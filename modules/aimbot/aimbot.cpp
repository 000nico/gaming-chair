#include "../modules.hpp"
#include "../../memory/memory.h"
#include <cmath>
#include <iostream>

void aimassist::handler() {
    pNtUserSendInput NtUserSendInput = NULL;
    HMODULE hWin32u = GetModuleHandleA("win32u.dll");
    NtUserSendInput = (pNtUserSendInput)GetProcAddress(hWin32u, "NtUserSendInput");

    while (true) {
        // MUY IMPORTANTE: El Sleep evita que el juego crashee por saturación
        Sleep(5); 

        if (!aimassist::enabled) continue;

        HWND game = FindWindowA(nullptr, "Counter-Strike 2");
        if (GetForegroundWindow() != game) continue;

        RECT rect = GetGameRect();
        int gameW = rect.right - rect.left;
        int gameH = rect.bottom - rect.top;

        float centerX = gameW / 2.0f;
        float centerY = gameH / 2.0f;

        for (auto& p : g_players) {
            if (p.team == localTeam && !esp::teams) continue;
            if (p.health <= 0) continue;

            Vector3 headPos = p.pos;
            headPos.z += 67.0f;
            Vector2 screen;

            if (!WorldToScreen(viewMatrix, headPos, screen, gameW, gameH)) continue;

            float dx = screen.x - centerX;
            float dy = screen.y - centerY;
            
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist < aimassist::radio /*&& dist > aimassist::deadZone*/) {
                
                float moveX = dx / aimassist::smoothing;
                float moveY = dy / aimassist::smoothing;

                INPUT input = { 0 };
                input.type = INPUT_MOUSE;
                input.mi.dwFlags = MOUSEEVENTF_MOVE; 
                input.mi.dx = (LONG)moveX;
                input.mi.dy = (LONG)moveY;

                NtUserSendInput(1, &input, sizeof(INPUT));
                
                break; 
            }
        }
    }
}