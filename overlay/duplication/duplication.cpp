#include "../../direct_x/direct_x.h"
#include "duplication.h"
#include "../../memory/memory.h"
#include "../../modules/modules.hpp"
#include "../../gui/imgui/imgui.h"
#include "../../gui/imgui/imgui_impl_dx11.h"
#include "../../gui/imgui/imgui_impl_win32.h"
#include "../../gui/gui.h"
#include <iostream>
#include <windows.h>
#include <timeapi.h>

#pragma comment(lib, "winmm.lib")

DX dx;
HWND guiHwnd = NULL;
bool open = true;

int duplication::handler(HWND hwnd) {
    SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
    timeBeginPeriod(1);

    const int SCREEN_W = GetSystemMetrics(SM_CXSCREEN);
    const int SCREEN_H = GetSystemMetrics(SM_CYSCREEN);

    if (!DX_Init(dx, hwnd, SCREEN_W, SCREEN_H) || !DX_InitDuplication(dx)) {
        return 0;
    }

    ImGuiContext* overlayCtx = ImGui::CreateContext();
    ImGui::SetCurrentContext(overlayCtx);
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(dx.device.Get(), dx.ctx.Get());

    HWND window = FindWindowA(nullptr, "Counter-Strike 2");
    RECT rect = {};
    int frmCount = 0;

    auto getGameRect = [&]() -> RECT {
        RECT r = {};
        if (!window || !IsWindow(window)) {
            window = FindWindowA(nullptr, "Counter-Strike 2");
            if (!window) return r;
        }
        RECT client = {};
        if (!GetClientRect(window, &client)) return r;
        POINT origin = { 0, 0 };
        ClientToScreen(window, &origin);
        r.left = origin.x;
        r.top = origin.y;
        r.right = origin.x + client.right;
        r.bottom = origin.y + client.bottom;
        return r;
    };

    auto reinitDuplication = [&]() -> bool {
        dx.duplication.Reset();
        for (int i = 0; i < 5; i++) {
            Sleep(100);
            if (DX_InitDuplication(dx)) return true;
        }
        return false;
    };

    rect = getGameRect();
    std::cout << "[duplication] ready" << std::endl;

    while (duplication::running) {
        if (!dx.duplication) {
            if (!reinitDuplication()) break;
            continue;
        }

        DXGI_OUTDUPL_FRAME_INFO info = {};
        ComPtr<IDXGIResource> resource = {};

        HRESULT hr = dx.duplication->AcquireNextFrame(0, &info, resource.GetAddressOf());

        if (hr == DXGI_ERROR_ACCESS_LOST || hr == DXGI_ERROR_INVALID_CALL) {
            if (!reinitDuplication()) break;
            continue;
        }

        if (SUCCEEDED(hr)) {
            if (info.LastPresentTime.QuadPart != 0) {
                ComPtr<ID3D11Texture2D> tex;
                resource.As(&tex);
                dx.ctx->CopyResource(dx.copyTex.Get(), tex.Get());
                
                if (++frmCount % 60 == 0)
                    rect = getGameRect();
            }
            dx.duplication->ReleaseFrame();
        } 
        else if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
            continue; 
        }

        ImGui::SetCurrentContext(overlayCtx);
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        DX_BeginFrame(dx);
        DX_DrawTexture(dx, dx.copyTex.Get());

        if (esp::enabled) esp::handler(rect);
        if (tracers::enabled) tracers::handler(rect);
        if (bones::enabled) bones::handler(rect);
        if (aimassist::enabled) aimassist::render(rect);

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        
        DX_EndFrame(dx); 
    }

    ImGui::SetCurrentContext(overlayCtx);
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext(overlayCtx);
    SetWindowDisplayAffinity(hwnd, WDA_NONE);
    timeEndPeriod(1);

    return 1;
}

bool duplication::exit(bool debug) {
    duplication::running = false;
    DX_Shutdown(dx);
    return 1;
}