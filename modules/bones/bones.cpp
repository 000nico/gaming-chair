// bones.cpp
#include "../modules.hpp"
#include "../../direct_x/direct_x.h"
#include "../../overlay/duplication/duplication.h"
#include "../../gui/imgui/imgui.h"
#include <cmath>

void bones::handler(RECT rect){
    int gameW = rect.right - rect.left;
    int gameH = rect.bottom - rect.top;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    const ImU32 colHead   = IM_COL32(bones::headColor.r,   bones::headColor.g,   bones::headColor.b,   bones::headColor.a);
    const ImU32 colFilled = IM_COL32(bones::filledColor.r, bones::filledColor.g, bones::filledColor.b, bones::filledColor.a);

    for(auto& p : g_players){
        if(p.team == localTeam && !bones::teams) continue;

        for(int i = 0; i < (int)(sizeof(skeleton_links) / sizeof(skeleton_links[0])); i++){
            int b1 = skeleton_links[i].a;
            int b2 = skeleton_links[i].b;

            if(p.bones[b1].x == 0 && p.bones[b1].y == 0 && p.bones[b1].z == 0) continue;
            if(p.bones[b2].x == 0 && p.bones[b2].y == 0 && p.bones[b2].z == 0) continue;

            Vector2 s1, s2;
            if(!WorldToScreen(viewMatrix, p.bones[b1], s1, gameW, gameH)) continue;
            if(!WorldToScreen(viewMatrix, p.bones[b2], s2, gameW, gameH)) continue;

            s1.x += rect.left; s1.y += rect.top;
            s2.x += rect.left; s2.y += rect.top;

            DX_DrawLine(dx, s1.x, s1.y, s2.x, s2.y,
                bones::color.r, bones::color.g, bones::color.b, bones::color.a, 1.0f);
        }

        if(bones::head){
            Vector2 headScreen, neckScreen;
            if(!WorldToScreen(viewMatrix, p.bones[BoneIndices::HEAD], headScreen, gameW, gameH)) continue;
            if(!WorldToScreen(viewMatrix, p.bones[BoneIndices::NECK], neckScreen, gameW, gameH)) continue;

            headScreen.x += rect.left; headScreen.y += rect.top;
            neckScreen.x += rect.left; neckScreen.y += rect.top;

            float radius = fabsf(headScreen.y - neckScreen.y) * 1.2f;

            if(bones::filled){
                dl->AddCircleFilled(ImVec2(headScreen.x, headScreen.y), radius, colFilled);
            }
            dl->AddCircle(ImVec2(headScreen.x, headScreen.y), radius, colHead, 0, 1.5f);
        }
    }
}