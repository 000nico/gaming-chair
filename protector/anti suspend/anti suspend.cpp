#include <Windows.h>
#include "anti suspend.hpp"
#include "../helper/sdk.hpp"
#include "../anti debug/anti debug.hpp"
#include "watchdog payload/bin/watchdog.h"
#include <iostream>

// https://secret.club/2021/01/04/thread-stuff.html
#define THREAD_CREATE_FLAGS_BYPASS_PROCESS_FREEZE 0x40
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#define NtCurrentProcess() ((HANDLE)-1)

HANDLE tHandle, mainThread;
HANDLE watchdogThread, eHandle;
LPVOID payloadBaseAddr, structBaseAddr;

bool antiSuspend::bypassFreeze(){
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if(!hNtdll) return false;
    pNtCreateThreadEx NtCreateThreadEx = (pNtCreateThreadEx)GetProcAddress(hNtdll, "NtCreateThreadEx");
    if(!NtCreateThreadEx) return false;


    NTSTATUS status = NtCreateThreadEx(&tHandle,
    MAXIMUM_ALLOWED,
    NULL,
    NtCurrentProcess(),
    (PVOID)&sdk::protectedThread,
    NULL,
    THREAD_CREATE_FLAGS_BYPASS_PROCESS_FREEZE,
    0, 0, 0, NULL);
        
    if (!NT_SUCCESS(status)) {
        std::cout << "NtCreateThreadEx failed: " << std::hex << status << "\n";
        return false;
    }

    return true;
}

bool antiSuspend::resumeMainThread(){
    static pNtResumeThread NtResumeThread = (pNtResumeThread)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtResumeThread");

    if(!NtResumeThread) return 0;

    ULONG previousCount = 0;
    NtResumeThread(mainThread, &previousCount);

    if(previousCount > 0){
        // you can do something in here, this code executes if someone tried to suspend your process
    }
        
    return true;
}

bool antiSuspend::watchDog(){
    HWND eHWND = GetShellWindow();
    DWORD pid;
    GetWindowThreadProcessId(eHWND, &pid);
    eHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if(!hNtdll) return false;

    // alloc pages
    payloadBaseAddr = VirtualAllocEx(eHandle, NULL, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    structBaseAddr = VirtualAllocEx(eHandle, NULL, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if(!payloadBaseAddr | !structBaseAddr) return false;

    // duplicate handles for explorer
    HANDLE hRemoteMain = NULL, hRemoteProtect = NULL;
    DuplicateHandle(GetCurrentProcess(), mainThread, eHandle, &hRemoteMain, 0, 0, 2);
    DuplicateHandle(GetCurrentProcess(), tHandle, eHandle, &hRemoteProtect, 0, 0, 2);
    
    // fill struct
    watchdogStruct ws = {};
    ws.resume = (pNtResumeThread)GetProcAddress(hNtdll, "NtResumeThread");
    ws.mainThreadHandle = hRemoteMain;
    ws.protectThreadHandle = hRemoteProtect;

    if(!WriteProcessMemory(eHandle, structBaseAddr, &ws, sizeof(ws), 0)) return false;
    if(!WriteProcessMemory(eHandle, payloadBaseAddr, bin_watchdog_bin, bin_watchdog_bin_len, 0)) return false;

    // create thread
    watchdogThread = CreateRemoteThread(eHandle, NULL, 0, (LPTHREAD_START_ROUTINE)payloadBaseAddr, structBaseAddr, 0, NULL);
    if(!watchdogThread) return false;

    return true;
}

void antiSuspend::checkWatchDog(){
    DWORD exitCode = 0;
    GetExitCodeThread(watchdogThread, &exitCode);
    if(exitCode != STILL_ACTIVE){
        antiDebug::crash();
    }
}