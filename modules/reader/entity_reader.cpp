#include "../modules.hpp"
#include "../../proxy/proxy/proxy.h"
#include <string>
#include <iostream>
#include <windows.h>
#include <cstdint>
#include <tlhelp32.h>
#include <vector>
#pragma comment(lib, "winmm.lib")

uintptr_t dwEntityList = 0x24D4E80;
uintptr_t hPlayerPawn = 0x90C;
uintptr_t iHealth = 0x34C;
uintptr_t m_iszPlayerName = 0x6F4;
uintptr_t m_vOldOrigin = 0x1390;
uintptr_t dwViewMatrix = 0x2334850;
uintptr_t m_iTeamNum = 0x3EB;
uintptr_t dwLocalPlayerPawn = 0x205A700;
uintptr_t m_pGameSceneNode = 0x330;
uintptr_t m_modelState = 0x150;
uintptr_t m_iIDEntIndex = 0x344C;
uintptr_t m_pAimPunchServices = 0x1490;
uintptr_t m_unpredictableBaseAngle = 0xA4;
std::vector<Player> g_players;

float viewMatrix[16] = {};
int localTeam = 0;
QAngle punchAngle;
static float lastPunchX = 0.f;
static float lastPunchY = 0.f;

uintptr_t GetModuleBase(DWORD pid, const char* moduleName) {
    uintptr_t moduleBase = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 moduleEntry;
        moduleEntry.dwSize = sizeof(moduleEntry);
        if (Module32First(snapshot, &moduleEntry)) do {
            if (!_stricmp(moduleEntry.szModule, moduleName)) {
                moduleBase = (uintptr_t)moduleEntry.modBaseAddr;
                break;
            }
        } while (Module32Next(snapshot, &moduleEntry));
        CloseHandle(snapshot);
    }
    return moduleBase;
}

void reader::handler() {
    HWND window = FindWindowA(nullptr, "Counter-Strike 2");
    DWORD pid;
    GetWindowThreadProcessId(window, &pid);
    uintptr_t client = GetModuleBase(pid, "client.dll");

    timeBeginPeriod(1);
    std::cout << "[reader] ready" << std::endl;

    while (true) {
        std::vector<Player> players;
        players.reserve(64);
        
        uintptr_t entityList = proxy::read<uintptr_t>(client + dwEntityList);
        if (!entityList) { Sleep(10); continue; }
        
        uintptr_t listEntry = proxy::read<uintptr_t>(entityList + 0x10);
        if (!listEntry) continue;
        //std::cout << listEntry << std::endl;
        uintptr_t localPawn = proxy::read<uintptr_t>(client + dwLocalPlayerPawn);
        //std::cout << localPawn << std::endl;
        if (!localPawn) continue;
        
        localTeam = proxy::read<int>(localPawn + m_iTeamNum);
        uint32_t aimHandle = proxy::read<uint32_t>(localPawn + m_iIDEntIndex);
        isAiming = 0;

        uintptr_t aimPunchServices = proxy::read<uintptr_t>(localPawn + 0x1490);

        float newPunchX = proxy::read<float>(aimPunchServices + 0x60);
        float newPunchY = proxy::read<float>(aimPunchServices + 0x64);

        float deltaX = newPunchX - lastPunchX;
        float deltaY = newPunchY - lastPunchY;

        float expected = punchDeltaX.load();
        while (!punchDeltaX.compare_exchange_weak(expected, expected + deltaX));

        expected = punchDeltaY.load();
        while (!punchDeltaY.compare_exchange_weak(expected, expected + deltaY));

        lastPunchX = newPunchX;
        lastPunchY = newPunchY;

        

        for (int i = 0; i < 16; i++)
            viewMatrix[i] = proxy::read<float>(client + dwViewMatrix + i * sizeof(float));

        for (int i = 0; i < 64; i++) {
            uintptr_t currentController = proxy::read<uintptr_t>(listEntry + (i * 0x70));
            if (!currentController) continue;

            uint32_t pawnHandle = proxy::read<uint32_t>(currentController + hPlayerPawn);
            if (!pawnHandle) continue;

            uintptr_t listEntry2 = proxy::read<uintptr_t>(entityList + (0x8 * ((pawnHandle & 0x7FFF) >> 9) + 0x10));
            if (!listEntry2) continue;

            uintptr_t currentPawn = proxy::read<uintptr_t>(listEntry2 + (0x70 * (pawnHandle & 0x1FF)));
            if (!currentPawn || currentPawn == localPawn) continue;

            int team = proxy::read<int>(currentPawn + m_iTeamNum);
            if (team != 2 && team != 3) continue;

            int health = proxy::read<int>(currentPawn + iHealth);
            if (health <= 0 || health > 100) continue;
            //std::cout << health << std::endl;

            Player p = {};
            p.health = health;
            p.team = team;
            p.pos = proxy::read<Vector3>(currentPawn + m_vOldOrigin);

            for (int j = 0; j < 16; j++)
                p.name[j] = proxy::read<char>(currentController + m_iszPlayerName + j);

            uintptr_t gameScene = proxy::read<uintptr_t>(currentPawn + m_pGameSceneNode);
            if (gameScene) {
                uintptr_t boneArray = proxy::read<uintptr_t>(gameScene + m_modelState + 0x80);
                if (boneArray) {
                    for (int b = 0; b < 29; b++)
                        p.bones[b] = proxy::read<Vector3>(boneArray + b * 32);
                }
            }

            if ((pawnHandle & 0x7FFF) == aimHandle) {
                isAiming = 1;
            }

            players.push_back(p);
        }

        g_players = std::move(players);
        //Sleep(1);
    }
    timeEndPeriod(1);
}