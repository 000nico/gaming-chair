#include "../modules.hpp"
#include "../../memory/memory.h"
#include <iostream>

void rcs::handler(){
    pNtUserSendInput NtUserSendInput = nullptr;
    HMODULE hWin32u = GetModuleHandleA("win32u.dll");
    if (!hWin32u) { std::cout << "[rcs] win32u.dll not found" << std::endl; return; }
    NtUserSendInput = (pNtUserSendInput)GetProcAddress(hWin32u, "NtUserSendInput");
    if (!NtUserSendInput) { std::cout << "[rcs] NtUserSendInput not found" << std::endl; return; }

    std::cout << "[rcs] ready" << std::endl;

    HWND  game         = nullptr;
    DWORD lastWndCheck = 0;

    while (true) {
        Sleep(1);

        DWORD now = GetTickCount();
        if (!game || (now - lastWndCheck) > 2000) {
            game         = FindWindowA(nullptr, "Counter-Strike 2");
            lastWndCheck = now;
        }

        if (!game || GetForegroundWindow() != game) continue;

        if (rcs::enabled) {
            float dx = punchDeltaX.exchange(0.f);
            float dy = punchDeltaY.exchange(0.f);

            LONG moveX = static_cast<LONG>(-dx * 2.0f);
            LONG moveY = static_cast<LONG>(-dy * 2.0f);

            std::cout << "[rcs] delta: " << dx << " | " << dy << " move: " << moveX << " | " << moveY << std::endl;

            if (moveX != 0 || moveY != 0) {
                INPUT input      = {};
                input.type       = INPUT_MOUSE;
                input.mi.dwFlags = MOUSEEVENTF_MOVE;
                input.mi.dx      = moveX;
                input.mi.dy      = moveY;
                NtUserSendInput(1, &input, sizeof(INPUT));
            }
        } else {
            punchDeltaX.exchange(0.f);
            punchDeltaY.exchange(0.f);
        }
    }
}