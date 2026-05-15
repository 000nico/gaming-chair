#include "../modules.hpp"
#include <iostream>
#include <windows.h>
#include <random>
#include <algorithm>

void triggerbot::handler() {
    pNtUserSendInput NtUserSendInput = NULL;

    HMODULE hWin32u = GetModuleHandleA("win32u.dll");
    if (!hWin32u) return;
    NtUserSendInput = (pNtUserSendInput)GetProcAddress(hWin32u, "NtUserSendInput");
    if (!NtUserSendInput) return;

    std::mt19937 rng(std::random_device{}());

    HWND game = nullptr;
    DWORD lastWndCheck = 0;

    while (true) {
        Sleep(1);

        if (!triggerbot::enabled) continue;
        if (isAiming <= 0) continue;

        DWORD now = GetTickCount();
        if (!game || (now - lastWndCheck) > 2000) {
            game = FindWindowA(nullptr, "Counter-Strike 2");
            lastWndCheck = now;
        }
        if (!game || GetForegroundWindow() != game) continue;

        int delay;
        if (!triggerbot::randomization) {
            delay = (int)triggerbot::delay;
        } else {
            std::uniform_int_distribution<int> dist(-30, 30);
            delay = std::max(0, (int)triggerbot::delay + dist(rng));
        }

        if (delay > 0)
            Sleep(delay);

        if (!triggerbot::enabled) continue;
        if (isAiming <= 0) continue;
        if (GetForegroundWindow() != game) continue;

        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        NtUserSendInput(1, &input, sizeof(INPUT));

        input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        NtUserSendInput(1, &input, sizeof(INPUT));
    }
}