// this code is not being used but im lazy to remove it from the build.sh

#include "gui.h"
#include <windows.h>
#include "../overlay/overlay/overlay.hpp"
#include "../modules/modules.hpp"
#include "zdraw/demo/render/render.hpp"
#include <dwmapi.h>

//OverlayInstance guiInst;
HWND gameHwnd = nullptr;
bool menuOpen = false;


static void sync_to_game( HWND game )
{
    RECT rc{};
    POINT origin{ 0, 0 };
    GetClientRect( game, &rc );
    ClientToScreen( game, &origin );

    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    SetWindowPos( guiHwnd, HWND_TOPMOST, origin.x, origin.y, w, h, SWP_NOACTIVATE );
}

void gui::manage()
{

    HWND game = FindWindowA( nullptr, "Counter-Strike 2" );
    if ( !game ) return;

    SetLayeredWindowAttributes( guiHwnd, RGB(0,0,0), 0, LWA_COLORKEY );

    ShowWindow( guiHwnd, SW_HIDE );

    while ( true )
    {
        if ( GetAsyncKeyState( VK_INSERT ) & 1 )
        {
            menuOpen = !menuOpen;

            if ( menuOpen )
            {
                sync_to_game( game );
                ShowWindow( guiHwnd, SW_SHOW );
                SetWindowPos( guiHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
                SetForegroundWindow( guiHwnd );
                ClipCursor( nullptr );
                while ( ShowCursor( TRUE ) < 0 );
            }
            else
            {
                ShowWindow( guiHwnd, SW_HIDE );
                while ( ShowCursor( FALSE ) >= 0 );
            }
        }

        if ( menuOpen )
            sync_to_game( game );

        Sleep( 5 );
    }
}