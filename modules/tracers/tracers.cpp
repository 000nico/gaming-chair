// tracers.cpp
#include "../modules.hpp"
#include "../../direct_x/direct_x.h"
#include "../../overlay/duplication/duplication.h"
#include "../../gui/imgui/imgui.h"
#include <cmath>

void tracers::handler(RECT rect){
    int gameW = rect.right - rect.left;
    int gameH = rect.bottom - rect.top;

    float centerX, centerY;
    
    if(tracers::style == 0 || centered){
        centerX = rect.left + gameW / 2.0f + 1;
        centerY = rect.top  + gameH / 2.0f + 7;
    } else {
        centerX = rect.left + gameW / 2.0f;
        centerY = rect.top  + gameH;
    }

    for(auto& p : g_players){
        if(p.team == localTeam && !tracers::teams) continue;

        Vector2 screen;
        if(!WorldToScreen(viewMatrix, p.pos, screen, gameW, gameH)) continue;

        screen.x += rect.left;
        screen.y += rect.top;

        if(tracers::style == 1){
            DX_DrawLine(dx, centerX, centerY, screen.x, screen.y,
                tracers::color.r, tracers::color.g, tracers::color.b, tracers::color.a, 1.0f);
        } else {
            float dx_ = screen.x - centerX;
            float dy_ = screen.y - centerY;
            float len = sqrtf(dx_ * dx_ + dy_ * dy_);
            if(len < 0.0001f) continue;
            DX_DrawArrow(dx,
                centerX + (dx_ / len) * tracers::offset,
                centerY + (dy_ / len) * tracers::offset,
                screen.x, screen.y,
                tracers::color.r, tracers::color.g, tracers::color.b, tracers::color.a);
        }
    }
}