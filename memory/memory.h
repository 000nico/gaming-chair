// memory.h
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "../overlay/overlay/overlay.hpp"

extern HWND overlayHwnd;
extern HWND guiHwnd;
extern void exit();
extern bool destroyed;
// +rep https://www.unknowncheats.me/forum/counter-strike-2-a/749047-cs-update-bone-indexes.html
enum BoneIndices : int {
	ORIGIN = 0,
	PELVIS = 1,
	SPINE0 = 2,
	SPINE1 = 3,
	SPINE2 = 4,
	NECK = 6,
	HEAD = 7,
	CLAVICLE_L = 8,
	SHOULDER_L = 9,
	ELBOW_L = 10,
	HAND_L = 11,
	CLAVICLE_R = 12, 
	SHOULDER_R = 13,
	ELBOW_R = 14,
	HAND_R = 15,
	HIP_L = 17,
	KNEE_L = 18,
	FOOT_HEEL_L = 19,
	HIP_R = 20,
	KNEE_R = 21,
	FOOT_HEEL_R = 22,
	CHEST = 23,
	GUN = 24,
	EYE_L = 25,
	EYE_R = 26,
	RANDOM = 27,
	CVJ_BONE = 28,
	FOOT_TOES_L_T = 74,
	FOOT_TOES_R_T = 77,
	FOOT_TOES_L_CT = 81,
	FOOT_TOES_R_CT = 86,
	BONE_MAX = 128
};

struct bone_link { int a, b; };
 
static bone_link skeleton_links[] = {
    // Spine
    { PELVIS, SPINE1 },
    { SPINE1, SPINE2 },
    { SPINE2, CHEST },
    { CHEST, NECK },
    { NECK,  HEAD },

    // Left arm
    { NECK,       SHOULDER_L },
    { SHOULDER_L, ELBOW_L },
    { ELBOW_L,    HAND_L },

    // Right arm
    { NECK,       SHOULDER_R },
    { SHOULDER_R, ELBOW_R },
    { ELBOW_R,    HAND_R },

    // Left leg T
    { PELVIS,      HIP_L },
    { HIP_L,       KNEE_L },
    { KNEE_L,      FOOT_HEEL_L },
    { FOOT_HEEL_L, FOOT_TOES_L_T },

    // Right leg T
    { PELVIS,      HIP_R },
    { HIP_R,       KNEE_R },
    { KNEE_R,      FOOT_HEEL_R },
    { FOOT_HEEL_R, FOOT_TOES_R_T },

    // Left leg CT
    { FOOT_HEEL_L, FOOT_TOES_L_CT },

    // Right leg CT
    { FOOT_HEEL_R, FOOT_TOES_R_CT },
};

struct Vector2 {
    float x, y;
};

struct Vector3 {
    float x, y, z;
};

extern int localTeam;   

struct Player {
    char name[16];
    int health, team;
    Vector3 pos;
    Vector3 bones[BONE_MAX];
};

extern std::vector<Player> g_players;

namespace reader {
    void handler();
}

bool WorldToScreen(const float* viewMatrix, Vector3 worldPos, Vector2& screenPos, int screenW, int screenH);
RECT GetGameRect();

typedef struct _MOUSE_INPUT_DATA {
    USHORT UnitId;
    USHORT Flags;
    USHORT ButtonFlags;
    USHORT ButtonData;
    ULONG  RawButtons;
    LONG   LastX;
    LONG   LastY;
    ULONG  ExtraInformation;
} MOUSE_INPUT_DATA, *PMOUSE_INPUT_DATA;

typedef BOOL (NTAPI* pNtUserInjectMouseInput)(PMOUSE_INPUT_DATA MouseData, ULONG Count);


typedef UINT(NTAPI* pNtUserSendInput)(
    UINT    cInputs,
    LPINPUT pInputs,
    int     cbSize
);
