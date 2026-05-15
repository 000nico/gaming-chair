#include "../modules.hpp"
#include "../../direct_x/direct_x.h"

void aimassist::render(RECT rect){
    int gameW = rect.right - rect.left;
    int gameH = rect.bottom - rect.top;

    float centerX = rect.left + gameW / 2.0f;
    float centerY = rect.top  + gameH / 2.0f;

    if(aimassist::renderRadio)
        DX_DrawCircle( centerX, centerY, aimassist::radio, 255, 255, 255, 255);
    if(aimassist::renderDeadZone)
        DX_DrawCircle( centerX, centerY, aimassist::deadZone, 255, 255, 255, 255);

    if(aimassist::deadZone >= aimassist::radio)
        aimassist::radio = aimassist::deadZone;
}