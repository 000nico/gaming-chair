// esp.cpp
#include "../modules.hpp"
#include "../../direct_x/direct_x.h"
#include "../../overlay/duplication/duplication.h"
#include "../../gui/imgui/imgui.h"

void esp::handler(RECT rect){
    int gameW = rect.right - rect.left;
    int gameH = rect.bottom - rect.top;

    for(auto& p : g_players){
        if(p.team == localTeam && !esp::teams) continue;

        Vector3 feetPos = { p.pos.x, p.pos.y, p.pos.z - 8.0f  };
        Vector3 headPos = { p.pos.x, p.pos.y, p.pos.z + 70.0f };

        Vector2 screenFeet, screenHead;
        if(!WorldToScreen(viewMatrix, feetPos, screenFeet, gameW, gameH)) continue;
        if(!WorldToScreen(viewMatrix, headPos, screenHead, gameW, gameH)) continue;

        screenFeet.x += rect.left; screenFeet.y += rect.top;
        screenHead.x += rect.left; screenHead.y += rect.top;

        float h = screenFeet.y - screenHead.y;
        if(h <= 5 || h > 500) continue;

        float w = h / 2.5f;
        float x = screenHead.x - w / 2.0f;
        float y = screenHead.y;

        if(esp::enableBoxes){
            DX_DrawRect(dx, x - 1, y - 1, w + 2, h + 2, 0, 0, 0, 255, 1.0f);
            DX_DrawRect(dx, x + 1, y + 1, w - 2, h - 2, 0, 0, 0, 255, 1.0f);
            DX_DrawRect(dx, x, y, w, h, esp::color.r, esp::color.g, esp::color.b, esp::color.a, 1.0f);
        }

        if(esp::enableHealthESP){
            float barH = h * (p.health * 0.01f);
            DX_DrawLine(dx, x - 4, y, x - 4, y + h, 50, 50, 50, esp::healthColor.a, 2.0f);
            DX_DrawLine(dx, x - 4, y + h, x - 4, y + h - barH, esp::healthColor.r, esp::healthColor.g, esp::healthColor.b, esp::healthColor.a, 2.0f);
        }

        if(esp::enableNameESP){
            float textW = ImGui::CalcTextSize(p.name).x;
            float textX = x + w / 2.0f - textW / 2.0f;
            DX_DrawText(textX - 1, y - 14, p.name, 0, 0, 0, 255);
            DX_DrawText(textX + 1, y - 14, p.name, 0, 0, 0, 255);
            DX_DrawText(textX,     y - 13, p.name, 0, 0, 0, 255);
            DX_DrawText(textX,     y - 15, p.name, 0, 0, 0, 255);
            DX_DrawText(textX,     y - 14, p.name, esp::nameColor.r, esp::nameColor.g, esp::nameColor.b, esp::nameColor.a);
        }
    }
}