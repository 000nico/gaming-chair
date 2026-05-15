#include "../modules.hpp"
#include "../../memory/memory.h"
#include <cmath>
#include <iostream>

Vector2 getBezierPoint(float t, Vector2 p0, Vector2 p1, Vector2 p2) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;

    Vector2 p;
    p.x = uu * p0.x + 2 * u * t * p1.x + tt * p2.x;
    p.y = uu * p0.y + 2 * u * t * p1.y + tt * p2.y;
    return p;
}

void aimassist::handler() {
    pNtUserSendInput NtUserSendInput = NULL;
    HMODULE hWin32u = GetModuleHandleA("win32u.dll");
    NtUserSendInput = (pNtUserSendInput)GetProcAddress(hWin32u, "NtUserSendInput");

    while (true) {
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

            bool isInsideTarget = (dist < aimassist::radio);
            bool passesDeadZone = !aimassist::deadZoneEnabled || (dist > aimassist::deadZone);

            if (isInsideTarget && passesDeadZone) {
                Vector2 origin = { 0, 0 };
                Vector2 target = { dx, dy };
                
                static float curveStrength = 0.5f; 
                Vector2 controlPoint = {
                    (target.x / 2.0f) + (rand() % 20 - 10), 
                    (target.y / 2.0f) + (rand() % 20 - 10)
                };

                float t = 1.0f / aimassist::smoothing; 
                Vector2 nextStep = getBezierPoint(t, origin, controlPoint, target);
                                                                                                            
                INPUT input = { 0 };
                input.type = INPUT_MOUSE;
                input.mi.dwFlags = MOUSEEVENTF_MOVE; 
                input.mi.dx = (LONG)nextStep.x;
                input.mi.dy = (LONG)nextStep.y;

                NtUserSendInput(1, &input, sizeof(INPUT));
                
                break; 
            }
        }
    }
}