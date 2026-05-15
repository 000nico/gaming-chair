#include <windows.h>
#include <iostream>
#include <winnt.h>

#include "overlay.hpp"
#include "../payload/bin/payload.h"
#include "../payload/bin2/gui_payload.h"
#include "../../gui/zdraw/demo/render/render.hpp"

DWORD getExplorerPID(){
    HWND hwnd = GetShellWindow();
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    return pid;
}


int allocateMemoryPages(OverlayInstance& inst){
    inst.overlayPageBuffer = VirtualAllocEx(
        inst.explorerHandle,
        NULL,
        0x1000,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );

    if(!inst.overlayPageBuffer)
        return 0;

    inst.structPageBuffer = VirtualAllocEx(
        inst.explorerHandle,
        NULL,
        0x1000,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );

    return inst.structPageBuffer != NULL;
}

int allocateGuiPages(OverlayInstance& inst){
    inst.overlayPageBuffer2 = VirtualAllocEx(
        inst.explorerHandle,
        NULL,
        0x1000,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );

    if(!inst.overlayPageBuffer2)
        return 0;

    inst.structPageBuffer2 = VirtualAllocEx(
        inst.explorerHandle,
        NULL,
        0x1000,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );

    return inst.structPageBuffer2 != NULL;
}

int injectPayload(OverlayInstance& inst){
    HMODULE user32   = GetModuleHandleA("user32.dll");
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    HMODULE ntdll    = GetModuleHandleA("ntdll.dll");

    if(!WriteProcessMemory(
        inst.explorerHandle,
        inst.overlayPageBuffer,
        bin_payload_bin,
        bin_payload_bin_len,
        NULL))
        return 0;

    overlayPayloadStruct ops{};
    wcscpy_s(ops.className,  L"overlay");
    wcscpy_s(ops.windowName, L"overlay");

    ops.hInstance        = NULL;
    ops.getModuleHandle  = (pGetModuleHandle)GetProcAddress(kernel32, "GetModuleHandleA");
    ops.createWindow     = (pCreateWindowInBand)GetProcAddress(user32, "CreateWindowInBand");
    ops.destroyWindow    = (pDestroyWindow)GetProcAddress(user32, "DestroyWindow");
    ops.showWindow       = (pShowWindow)GetProcAddress(user32, "ShowWindow");
    ops.sleep            = (pSleep)GetProcAddress(kernel32, "Sleep");
    ops.peekMessage      = (pPeekMessageW)GetProcAddress(user32, "PeekMessageW");
    ops.translateMessage = (pTranslateMessage)GetProcAddress(user32, "TranslateMessage");
    ops.dispatchMessage  = (pDispatchMessageW)GetProcAddress(user32, "DispatchMessageW");
    ops.memset           = (pRtlZeroMemory)GetProcAddress(ntdll, "RtlZeroMemory");
    ops.registerClass    = (pRegisterClassEx)GetProcAddress(user32, "RegisterClassExW");
    ops.defWindowProc    = (pDefWindowProcW)GetProcAddress(user32, "DefWindowProcW");
    ops.affinity         = (pSetWindowDisplayAffinity)GetProcAddress(user32, "SetWindowDisplayAffinity");
    ops.signal           = 0;

    WriteProcessMemory(inst.explorerHandle, inst.structPageBuffer, &ops, sizeof(ops), NULL);

    inst.remoteThread = CreateRemoteThread(
        inst.explorerHandle,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)inst.overlayPageBuffer,
        inst.structPageBuffer,
        0,
        NULL
    );

    return inst.remoteThread != NULL;
}

int injectGuiPayload(OverlayInstance& inst){
    HMODULE user32   = GetModuleHandleA("user32.dll");
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    HMODULE ntdll    = GetModuleHandleA("ntdll.dll");

    if(!WriteProcessMemory(
        inst.explorerHandle,
        inst.overlayPageBuffer2,
        bin2_gui_payload_bin,
        bin2_gui_payload_bin_len,
        NULL))
        return 0;

    overlayPayloadStruct ops{};
    wcscpy_s(ops.className,  L"gui_overlay");
    wcscpy_s(ops.windowName, L"gui_overlay");

    ops.hInstance        = NULL;
    ops.getModuleHandle  = (pGetModuleHandle)GetProcAddress(kernel32, "GetModuleHandleA");
    ops.createWindow     = (pCreateWindowInBand)GetProcAddress(user32, "CreateWindowInBand");
    ops.destroyWindow    = (pDestroyWindow)GetProcAddress(user32, "DestroyWindow");
    ops.showWindow       = (pShowWindow)GetProcAddress(user32, "ShowWindow");
    ops.sleep            = (pSleep)GetProcAddress(kernel32, "Sleep");
    ops.peekMessage      = (pPeekMessageW)GetProcAddress(user32, "PeekMessageW");
    ops.translateMessage = (pTranslateMessage)GetProcAddress(user32, "TranslateMessage");
    ops.dispatchMessage  = (pDispatchMessageW)GetProcAddress(user32, "DispatchMessageW");
    ops.memset           = (pRtlZeroMemory)GetProcAddress(ntdll, "RtlZeroMemory");
    ops.registerClass    = (pRegisterClassEx)GetProcAddress(user32, "RegisterClassExW");
    ops.defWindowProc    = (pDefWindowProcW)GetProcAddress(user32, "DefWindowProcW");
    ops.affinity         = (pSetWindowDisplayAffinity)GetProcAddress(user32, "SetWindowDisplayAffinity");
    ops.setWindowLongPtr = (pSetWindowLongPtrW)GetProcAddress(user32, "SetWindowLongPtrW");
    ops.getWindowLongPtr = (pGetWindowLongPtrW)GetProcAddress(user32, "GetWindowLongPtrW");
    ops.signal           = 0;

    WriteProcessMemory(
        inst.explorerHandle,
        inst.structPageBuffer2,
        &ops,
        sizeof(ops),
        NULL
    );

    inst.remoteThread2 = CreateRemoteThread(
        inst.explorerHandle,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)inst.overlayPageBuffer2,
        inst.structPageBuffer2,
        0,
        NULL
    );

    return inst.remoteThread2 != NULL;
}

int overlay::init(bool debug, HWND* returnHwnd, OverlayInstance& inst){
    DWORD pid = getExplorerPID();
    if(!pid) return 0;

    inst.explorerHandle = OpenProcess(
        PROCESS_VM_OPERATION |
        PROCESS_VM_WRITE |
        PROCESS_VM_READ |
        PROCESS_CREATE_THREAD,
        FALSE,
        pid
    );

    if(!inst.explorerHandle)
        return 0;

    if(!allocateMemoryPages(inst))
        return 0;

    if(!injectPayload(inst))
        return 0;

    Sleep(500);

    overlayPayloadStruct ops{};
    ReadProcessMemory(inst.explorerHandle, inst.structPageBuffer, &ops, sizeof(ops), 0);
    std::cout << "[overlay] ops.returnHwnd = " << ops.returnHwnd << std::endl;

    *returnHwnd = ops.returnHwnd;

    return 1;
}

int overlay::initGui(bool debug, HWND* returnHwnd, OverlayInstance& inst)
{
    DWORD pid = getExplorerPID();
    if (!pid) return 0;

    inst.explorerHandle = OpenProcess(
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE |
        PROCESS_VM_READ | PROCESS_CREATE_THREAD,
        FALSE, pid
    );

    if (!inst.explorerHandle) return 0;
    if (!allocateGuiPages(inst)) return 0;
    if (!injectGuiPayload(inst)) return 0;

    Sleep(200);

    overlayPayloadStruct ops{};
    ReadProcessMemory(inst.explorerHandle, inst.structPageBuffer2, &ops, sizeof(ops), nullptr);
    *returnHwnd = ops.returnHwnd;

    return 1;
}

int overlay::exit(bool debug, OverlayInstance& inst){
    overlayPayloadStruct ops{};
    ReadProcessMemory(inst.explorerHandle, inst.structPageBuffer, &ops, sizeof(ops), 0);

    ops.signal = -1;

    WriteProcessMemory(inst.explorerHandle, inst.structPageBuffer, &ops, sizeof(ops), 0);

    WaitForSingleObject(inst.remoteThread, 5000);
    TerminateThread(inst.remoteThread, 0);

    VirtualFreeEx(inst.explorerHandle, inst.overlayPageBuffer, 0, MEM_RELEASE);
    VirtualFreeEx(inst.explorerHandle, inst.structPageBuffer, 0, MEM_RELEASE);

    CloseHandle(inst.explorerHandle);

    return 1;
}

int overlay::exitGui(bool debug, OverlayInstance& inst){
    if(!inst.explorerHandle)
        return 0;

    overlayPayloadStruct ops{};
    ReadProcessMemory(
        inst.explorerHandle,
        inst.structPageBuffer2,
        &ops,
        sizeof(ops),
        nullptr
    );

    ops.signal = -1;

    WriteProcessMemory(
        inst.explorerHandle,
        inst.structPageBuffer2,
        &ops,
        sizeof(ops),
        nullptr
    );

    if(inst.remoteThread2)
    {
        WaitForSingleObject(inst.remoteThread2, 5000);
        TerminateThread(inst.remoteThread2, 0);
        CloseHandle(inst.remoteThread2);
        inst.remoteThread2 = nullptr;
    }

    if(inst.overlayPageBuffer2)
    {
        VirtualFreeEx(inst.explorerHandle, inst.overlayPageBuffer2, 0, MEM_RELEASE);
        inst.overlayPageBuffer2 = nullptr;
    }

    if(inst.structPageBuffer2)
    {
        VirtualFreeEx(inst.explorerHandle, inst.structPageBuffer2, 0, MEM_RELEASE);
        inst.structPageBuffer2 = nullptr;
    }

    if(debug)
        std::cout << "[overlay] gui exit succeeded\n";

    return 1;
}