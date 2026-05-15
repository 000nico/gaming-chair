#include "../modules.hpp"
#include "../../memory/memory.h"
#include <cmath>
#include <iostream>
#include <random>
#include <cfloat>
#include <algorithm>

static std::mt19937 rng(std::random_device{}());

Vector2 getBezierPoint(float t, Vector2 p0, Vector2 p1, Vector2 p2) {
    float u  = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    return {
        uu * p0.x + 2.0f * u * t * p1.x + tt * p2.x,
        uu * p0.y + 2.0f * u * t * p1.y + tt * p2.y
    };
}

void aimassist::handler() {
    pNtUserSendInput NtUserSendInput = nullptr;
    HMODULE hWin32u = GetModuleHandleA("win32u.dll");
    if (!hWin32u) return;
    NtUserSendInput = (pNtUserSendInput)GetProcAddress(hWin32u, "NtUserSendInput");
    if (!NtUserSendInput) return;

    HWND   game         = nullptr;
    DWORD  lastWndCheck = 0;

    std::uniform_real_distribution<float> jitterDist(-10.0f, 10.0f);

    while (true) {
        Sleep(5);

        if (!aimassist::enabled) continue;

        DWORD now = GetTickCount();
        if (!game || (now - lastWndCheck) > 2000) {
            game         = FindWindowA(nullptr, "Counter-Strike 2");
            lastWndCheck = now;
        }
        if (!game || GetForegroundWindow() != game) continue;

        RECT rect  = GetGameRect();
        int  gameW = rect.right  - rect.left;
        int  gameH = rect.bottom - rect.top;

        float centerX = gameW / 2.0f;
        float centerY = gameH / 2.0f;

        Player* bestTarget = nullptr;
        float   bestDist   = FLT_MAX;
        Vector2 bestScreen = {};

        for (auto& p : g_players) {
            if (p.team == localTeam && !esp::teams) continue;
            if (p.health <= 0)                      continue;

            Vector3 headPos  = p.pos;
            headPos.z       += 67.0f;
            Vector2 screen;

            if (!WorldToScreen(viewMatrix, headPos, screen, gameW, gameH)) continue;

            float dx   = screen.x - centerX;
            float dy   = screen.y - centerY;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist < aimassist::radio && dist < bestDist) {
                bestDist   = dist;
                bestTarget = &p;
                bestScreen = screen;
            }
        }

        if (!bestTarget) continue;

        float dx = bestScreen.x - centerX;
        float dy = bestScreen.y - centerY;
        float dist = bestDist;

        bool inDeadZone = aimassist::deadZoneEnabled && (dist <= aimassist::deadZone);
        if (inDeadZone) continue;

        constexpr float STOP_THRESHOLD = 1.5f; 
        if (dist < STOP_THRESHOLD) continue;

        float jitterScale = std::min(dist / aimassist::radio, 1.0f);
        Vector2 origin       = { 0.0f, 0.0f };
        Vector2 target       = { dx, dy };
        Vector2 controlPoint = {
            (dx / 2.0f) + jitterDist(rng) * jitterScale,
            (dy / 2.0f) + jitterDist(rng) * jitterScale
        };

        float t        = std::clamp(1.0f / aimassist::smoothing, 0.01f, 1.0f);
        Vector2 step   = getBezierPoint(t, origin, controlPoint, target);

        if (sqrtf(step.x * step.x + step.y * step.y) > dist) {
            step = target;
        }

        INPUT input    = {};
        input.type     = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        input.mi.dx    = static_cast<LONG>(step.x);
        input.mi.dy    = static_cast<LONG>(step.y);

        NtUserSendInput(1, &input, sizeof(INPUT));
    }
}