#include "../global.hpp"
#include "../../../memory/memory.h"
#include "../../../modules/modules.hpp"

namespace render {
	static HWND  cs2_hwnd{ nullptr };
	static HHOOK ll_mouse_hook{ nullptr };
	static bool  menu_open{ false };
	static HCURSOR arrow_cursor{ nullptr };

	static LRESULT CALLBACK ll_mouse_proc( int nCode, WPARAM wparam, LPARAM lparam )
	{
		if ( nCode == HC_ACTION )
		{
			if ( wparam == WM_MOUSEWHEEL )
			{
				const auto* ms    = reinterpret_cast<MSLLHOOKSTRUCT*>( lparam );
				const SHORT delta = static_cast<SHORT>( HIWORD( ms->mouseData ) );

				POINT pt{ ms->pt.x, ms->pt.y };
				ScreenToClient( window::hwnd, &pt );

				PostMessageW( window::hwnd, WM_MOUSEWHEEL,
				              MAKEWPARAM( 0, delta ),
				              MAKELPARAM( pt.x, pt.y ) );

				return 1;
			}

			if ( menu_open &&
			     ( wparam == WM_MOUSEMOVE   ||
			       wparam == WM_LBUTTONDOWN ||
			       wparam == WM_LBUTTONUP   ||
			       wparam == WM_RBUTTONDOWN ||
			       wparam == WM_RBUTTONUP ) )
			{
				SetCursor( arrow_cursor );
			}
		}

		return CallNextHookEx( ll_mouse_hook, nCode, wparam, lparam );
	}

	static HWND find_cs2( )
	{
		HWND h = FindWindowW( nullptr, L"Counter-Strike 2" );
		if ( !h )
			h = FindWindowW( L"SDL_app", nullptr );
		return h;
	}

	static void sync_to_cs2( )
	{
		if ( !cs2_hwnd || !IsWindow( cs2_hwnd ) )
		{
			cs2_hwnd = find_cs2( );
			if ( !cs2_hwnd )
				return;
		}

		RECT client{};
		GetClientRect( cs2_hwnd, &client );

		POINT tl{ 0, 0 };
		ClientToScreen( cs2_hwnd, &tl );

		const int x = tl.x;
		const int y = tl.y;
		const int w = client.right - client.left;
		const int h = client.bottom - client.top;

		RECT cur{};
		GetWindowRect( window::hwnd, &cur );

		if ( cur.left == x && cur.top == y &&
		     ( cur.right - cur.left ) == w &&
		     ( cur.bottom - cur.top ) == h )
			return;

		SetWindowPos( window::hwnd, HWND_TOPMOST, x, y, w, h, SWP_NOACTIVATE );
		directx::resize( static_cast<UINT>( w ), static_cast<UINT>( h ) );
	}

	static void set_menu_visible( bool visible )
	{
		menu_open = visible;

		if ( visible )
		{
			ClipCursor( nullptr );
			ReleaseCapture();

			// forzar liberación del raw input de CS2/SDL
			if ( cs2_hwnd && IsWindow( cs2_hwnd ) )
			{
				RAWINPUTDEVICE rid{};
				rid.usUsagePage = 0x01;
				rid.usUsage     = 0x02;
				rid.dwFlags     = RIDEV_REMOVE;
				rid.hwndTarget  = nullptr;
				RegisterRawInputDevices( &rid, 1, sizeof( rid ) );
			}

			LONG ex = GetWindowLongW( window::hwnd, GWL_EXSTYLE );
			ex &= ~WS_EX_TRANSPARENT;
			ex |=  WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW;
			ex &= ~WS_EX_APPWINDOW;
			SetWindowLongW( window::hwnd, GWL_EXSTYLE, ex );

			ShowWindow( window::hwnd, SW_SHOWNA );

			int show_count = ShowCursor( TRUE );
			while ( show_count < 0 )
				show_count = ShowCursor( TRUE );

			SetCursor( arrow_cursor );

			if ( !ll_mouse_hook )
				ll_mouse_hook = SetWindowsHookExW( WH_MOUSE_LL, ll_mouse_proc, nullptr, 0 );
		}
		else
		{
			if ( ll_mouse_hook )
			{
				UnhookWindowsHookEx( ll_mouse_hook );
				ll_mouse_hook = nullptr;
			}

			ShowWindow( window::hwnd, SW_HIDE );

			LONG ex = GetWindowLongW( window::hwnd, GWL_EXSTYLE );
			ex |=  WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW;
			ex &= ~WS_EX_APPWINDOW;
			SetWindowLongW( window::hwnd, GWL_EXSTYLE, ex );

			if ( cs2_hwnd && IsWindow( cs2_hwnd ) )
			{
				SetForegroundWindow( cs2_hwnd );
				SetFocus( cs2_hwnd );
			}

			int show_count = ShowCursor( FALSE );
			while ( show_count >= 0 )
				show_count = ShowCursor( FALSE );
		}
	}

	bool initialize( )
	{
		arrow_cursor = LoadCursorA( nullptr, IDC_ARROW );

		if ( !window::initialize( ) )  { std::printf( "failed to initialize window\n" );  return false; }
		if ( !directx::initialize( ) ) { std::printf( "failed to initialize directx\n" ); return false; }

		if ( !zdraw::initialize( directx::device, directx::device_context ) )
		{ std::printf( "failed to initialize zdraw\n" ); return false; }

		if ( !zui::initialize( window::hwnd ) )
		{ std::printf( "failed to initialize zui\n" ); return false; }

		menu::initialize( directx::device, directx::device_context );
		return true;
	}

	void loop( )
	{
		constexpr float clear_color[ 4 ]{ 0.0f, 0.0f, 1.0f / 255.0f, 1.0f };

		bool insert_was_down{ false };

		MSG msg{};

		while ( true )
		{
			while ( PeekMessageW( &msg, nullptr, 0, 0, PM_REMOVE ) )
			{
				if ( msg.message == WM_QUIT )
					goto exit_loop;

				TranslateMessage( &msg );
				DispatchMessageW( &msg );
			}

			if ( GetAsyncKeyState( VK_END ) & 1 )
			{
				if ( ll_mouse_hook )
				{
					UnhookWindowsHookEx( ll_mouse_hook );
					ll_mouse_hook = nullptr;
				}
				DestroyWindow( window::hwnd );
				break;
			}

			const bool insert_down = ( GetAsyncKeyState( VK_INSERT ) & 0x8000 ) != 0;
			if ( insert_down && !insert_was_down )
				set_menu_visible( !menu_open );
			insert_was_down = insert_down;

			if ( menu_open )
			{
				overlayPayloadStruct ops{};
				ReadProcessMemory( guiInst.explorerHandle, guiInst.structPageBuffer2,
				                   &ops, sizeof( ops ), nullptr );

				if ( ops.msgReady )
				{
					zui::process_wndproc_message( ops.pendingMsg, ops.pendingWParam, ops.pendingLParam );
					ops.msgReady = 0;
					WriteProcessMemory( guiInst.explorerHandle, guiInst.structPageBuffer2,
					                    &ops, sizeof( ops ), nullptr );
				}
			}

			sync_to_cs2( );

			directx::device_context->OMSetRenderTargets( 1, &directx::render_target_view, nullptr );
			directx::device_context->ClearRenderTargetView( directx::render_target_view, clear_color );

			zdraw::begin_frame( );
			if ( menu_open )
				menu::draw( );
			zdraw::end_frame( );

			menu::update( );

			directx::swap_chain->Present( 0, DXGI_PRESENT_DO_NOT_WAIT );

			MsgWaitForMultipleObjects( 0, nullptr, FALSE, 1, QS_ALLINPUT );
		}

		exit_loop:;
	}

	bool window::initialize( )
	{
		hwnd = guiHwnd;
		if ( !hwnd )
			return false;

		LONG ex = GetWindowLongW( hwnd, GWL_EXSTYLE );
		ex |=  WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED;
		ex &= ~WS_EX_APPWINDOW;
		SetWindowLongW( hwnd, GWL_EXSTYLE, ex );

		SetWindowLongPtrW( hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>( procedure ) );

		SetLayeredWindowAttributes( hwnd, RGB( 0, 0, 1 ), 0, LWA_COLORKEY );

		ShowWindow( hwnd, SW_HIDE );
		UpdateWindow( hwnd );

		return true;
	}

	long long __stdcall window::procedure( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
	{
		switch ( msg )
		{
			case WM_MOUSEACTIVATE:
				return MA_NOACTIVATE;

			case WM_ACTIVATE:
				return 0;

			case WM_SETFOCUS:
				return 0;

			case WM_SETCURSOR:
				SetCursor( arrow_cursor );
				return TRUE;

			case WM_MOUSEMOVE:
				if ( menu_open )
					SetCursor( arrow_cursor );
				return 0;

			case WM_SIZE:
				if ( wparam != SIZE_MINIMIZED )
					directx::resize( LOWORD( lparam ), HIWORD( lparam ) );
				break;

			case WM_DESTROY:
				PostQuitMessage( 0 );
				return 0;

			case WM_NCHITTEST:
				return HTCLIENT;
		}

		zui::process_wndproc_message( msg, wparam, lparam );

		return DefWindowProcW( hwnd, msg, wparam, lparam );
	}

	void directx::resize( UINT width, UINT height )
	{
		if ( !swap_chain || width == 0 || height == 0 )
			return;

		device_context->OMSetRenderTargets( 0, nullptr, nullptr );

		if ( render_target_view )
		{
			render_target_view->Release( );
			render_target_view = nullptr;
		}

		swap_chain->ResizeBuffers( 0, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0 );

		ID3D11Texture2D* back_buffer{};
		swap_chain->GetBuffer( 0, IID_PPV_ARGS( &back_buffer ) );
		device->CreateRenderTargetView( back_buffer, nullptr, &render_target_view );
		back_buffer->Release( );

		const D3D11_VIEWPORT vp
		{
			.TopLeftX = 0.0f,
			.TopLeftY = 0.0f,
			.Width    = static_cast<float>( width ),
			.Height   = static_cast<float>( height ),
			.MinDepth = 0.0f,
			.MaxDepth = 1.0f
		};
		device_context->RSSetViewports( 1, &vp );
	}

	bool directx::initialize( )
	{
		const DXGI_SWAP_CHAIN_DESC sc_desc
		{
			.BufferDesc  = { .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
			.SampleDesc  = { .Count = 1 },
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 2,
			.OutputWindow = window::hwnd,
			.Windowed    = TRUE,
			.SwapEffect  = DXGI_SWAP_EFFECT_DISCARD,
			.Flags       = 0
		};

		constexpr D3D_FEATURE_LEVEL feature_levels[]{ D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
		D3D_FEATURE_LEVEL selected{};

		if ( FAILED( D3D11CreateDeviceAndSwapChain(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
			feature_levels, _countof( feature_levels ),
			D3D11_SDK_VERSION,
			&sc_desc, &swap_chain, &device, &selected, &device_context ) ) )
			return false;

		ID3D11Texture2D* back_buffer{ nullptr };
		if ( FAILED( swap_chain->GetBuffer( 0, IID_PPV_ARGS( &back_buffer ) ) ) )
			return false;

		const auto hr = device->CreateRenderTargetView( back_buffer, nullptr, &render_target_view );
		back_buffer->Release( );

		if ( FAILED( hr ) )
			return false;

		RECT rc{};
		GetClientRect( window::hwnd, &rc );

		const D3D11_VIEWPORT vp
		{
			.TopLeftX = 0.0f,
			.TopLeftY = 0.0f,
			.Width    = static_cast<float>( rc.right  - rc.left ),
			.Height   = static_cast<float>( rc.bottom - rc.top  ),
			.MinDepth = 0.0f,
			.MaxDepth = 1.0f
		};
		device_context->RSSetViewports( 1, &vp );

		return true;
	}

} // namespace render