#include "memory.h"
#include <vector>

bool WorldToScreen(const float* viewMatrix, Vector3 worldPos, Vector2& screenPos, int screenW, int screenH)
{
    float clipX = worldPos.x * viewMatrix[0]  + worldPos.y * viewMatrix[1]  + worldPos.z * viewMatrix[2]  + viewMatrix[3];
    float clipY = worldPos.x * viewMatrix[4]  + worldPos.y * viewMatrix[5]  + worldPos.z * viewMatrix[6]  + viewMatrix[7];
    float clipW = worldPos.x * viewMatrix[12] + worldPos.y * viewMatrix[13] + worldPos.z * viewMatrix[14] + viewMatrix[15];

    if (clipW < 0.1f)
        return false; // behind

    float ndcX = clipX / clipW;
    float ndcY = clipY / clipW;

    screenPos.x = (screenW / 2.0f) + (ndcX * screenW / 2.0f);
    screenPos.y = (screenH / 2.0f) - (ndcY * screenH / 2.0f);

    return true;
}


RECT GetGameRect(){
    HWND window = FindWindowA(nullptr, "Counter-Strike 2");
    if(!window) return {};

    RECT client = {};
    GetClientRect(window, &client);

    POINT origin = {0, 0};
    ClientToScreen(window, &origin);

    RECT r = {};
    r.left   = origin.x;
    r.top    = origin.y;
    r.right  = origin.x + client.right;
    r.bottom = origin.y + client.bottom;
    return r;
}