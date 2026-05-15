// proxy.cpp
#include <vector>
#include <windows.h>
#include <wingdi.h>
#include <winnt.h>
#include <winuser.h>
#include <iostream>
#include <cstdint>

#include "proxy.h"

#include "../payload/read/bin/shellcodeRead.h"

#pragma comment(lib, "ntdll")

uintptr_t readBufferAddress = 0;
uintptr_t structAddress = 0;
DWORD targetPid = 0;
HANDLE readThread = 0;
LPVOID shellCodeReadRegion = 0;
LPVOID readBufferRegion = 0;
LPVOID structRegion = 0;

bool proxy::enableSeDebugPrivilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp = { 1 };
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) return false;
    LookupPrivilegeValueA(NULL, "SeDebugPrivilege", &tp.Privileges[0].Luid);
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
    CloseHandle(hToken);
    return GetLastError() == ERROR_SUCCESS;
}

int proxy::findProcessWithHandle(ULONG accessMask, LPCSTR windowName, DWORD* processPIDBuffer, HANDLE* duplicatedHandleBuffer, USHORT* originalHandleValue){
    HWND hwnd = FindWindowA(nullptr, windowName);

    if(!hwnd)
        return 0;

    DWORD windowPid;
    GetWindowThreadProcessId(hwnd, &windowPid);

    if(!windowPid)
        return 0;

    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    PNtQuerySystemInformation NtQuerySystemInformation = (PNtQuerySystemInformation)GetProcAddress(hNtdll, "NtQuerySystemInformation");

    if(!NtQuerySystemInformation)
        return 0;

    BYTE buffer[1048576];
    NTSTATUS NtQSI = NtQuerySystemInformation(0x10, buffer, sizeof(buffer), NULL);

    if(!NtQSI)
        return 0;

    SYSTEM_HANDLE_INFORMATION* SystemHandleInformation = (SYSTEM_HANDLE_INFORMATION*)buffer;

    for(int i = 0;  i < SystemHandleInformation->Count; i++){
        if((SystemHandleInformation->Handles[i].GrantedAccess & accessMask) == accessMask){
            HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, SystemHandleInformation->Handles[i].UniqueProcessId);

            HANDLE duplicatedHandle = NULL;
            
            DuplicateHandle(handle, (HANDLE)(ULONG_PTR)SystemHandleInformation->Handles[i].HandleValue, GetCurrentProcess(), &duplicatedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);

            DWORD pid = GetProcessId(duplicatedHandle);

            if(GetProcessId(duplicatedHandle) == windowPid){
                if(duplicatedHandleBuffer)
                    *duplicatedHandleBuffer = duplicatedHandle;

                if(processPIDBuffer)
                    *processPIDBuffer = SystemHandleInformation->Handles[i].UniqueProcessId;

                if(originalHandleValue)
                    *originalHandleValue = SystemHandleInformation->Handles[i].HandleValue;
                    
                targetPid = SystemHandleInformation->Handles[i].UniqueProcessId;
                CloseHandle(handle);
                return 1;
            }
            if(duplicatedHandle) 
                CloseHandle(duplicatedHandle);

            CloseHandle(handle);
        }
    }
    return 0;
}

int proxy::findAddressWithHandle(HANDLE handle, DWORD pid, uintptr_t* foundAtAddress, LPCSTR windowName){
    HANDLE targetHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    ULONG_PTR address = (ULONG_PTR)sysInfo.lpMinimumApplicationAddress;
    ULONG_PTR maxAddress = (ULONG_PTR)sysInfo.lpMaximumApplicationAddress;

    MEMORY_BASIC_INFORMATION mbi;
    std::vector<BYTE> buffer;

    HWND gameHWND = FindWindowA(nullptr, windowName);

    if(!gameHWND)
        return 0;

    DWORD gamePID;
    GetWindowThreadProcessId(gameHWND, &gamePID);

    if(!gamePID)
        return 0;
                 
    while(address < maxAddress){
        VirtualQueryEx(targetHandle, (LPCVOID)address, &mbi, sizeof(mbi));

        if(mbi.State == MEM_COMMIT && !(mbi.Protect & PAGE_GUARD) && !(mbi.Protect & PAGE_NOACCESS)){
            buffer.resize(mbi.RegionSize);
            SIZE_T bytesRead = 0;

            if(ReadProcessMemory(targetHandle, (LPCVOID)address, buffer.data(), mbi.RegionSize, &bytesRead)){
                for (SIZE_T i = 0; i + sizeof(ULONG_PTR) <= bytesRead; i += sizeof(ULONG_PTR)) {
                    HANDLE found = *(HANDLE*)(buffer.data() + i);

                    if (found == handle) {
                        HANDLE dupedHandle = NULL;
                        BOOL ok = DuplicateHandle(targetHandle, found, GetCurrentProcess(), &dupedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);

                        if(ok && dupedHandle){ 
                            if(gamePID == GetProcessId(dupedHandle)){
                                if(foundAtAddress){
                                    *foundAtAddress = address + i;

                                    CloseHandle(dupedHandle); 
                                    CloseHandle(targetHandle);
                                    return 1;
                                }
                            }
                            CloseHandle(dupedHandle); 
                        }
                    }
                }
            }
        }
        address += mbi.RegionSize;
    }

    CloseHandle(targetHandle);
    return 0;
}

int proxy::prepareCodeInjection(DWORD pid, LPVOID* rShellCodeBaseAddress, LPVOID* rBufferBaseAddress, LPVOID* structBaseAddress){
    HANDLE targetHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    shellCodeReadRegion = VirtualAllocEx(targetHandle, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    readBufferRegion    = VirtualAllocEx(targetHandle, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    structRegion        = VirtualAllocEx(targetHandle, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if(!shellCodeReadRegion || !readBufferRegion || !structRegion)
        return 0;

    readBufferAddress = (uintptr_t)readBufferRegion;
    structAddress     = (uintptr_t)structRegion;

    if(rShellCodeBaseAddress)
        *rShellCodeBaseAddress = shellCodeReadRegion;

    if(rBufferBaseAddress)
        *rBufferBaseAddress = readBufferRegion;

    if(structBaseAddress)
        *structBaseAddress = structRegion;
    
    CloseHandle(targetHandle);
    return 1;
}

int payload::load(DWORD pid, uintptr_t readShellCodeBaseAddress, uintptr_t handleAddress, uintptr_t readBufferAddress, uintptr_t structBaseAddress){
    HANDLE targetHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    if(!WriteProcessMemory(targetHandle, (LPVOID)readShellCodeBaseAddress, bin_shellcodeRead_bin, bin_shellcodeRead_bin_len, 0))
        return 0;

    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    payloadStruct pd = {};
    pd.handleAddress = handleAddress;

    pd.readBuffer = readBufferAddress;
    pd.readSignal = 0;
    pd.read = (fnNtReadVirtualMemory_t) GetProcAddress(ntdll, "NtReadVirtualMemory");

    if(!WriteProcessMemory(targetHandle, (LPVOID)structBaseAddress, &pd, sizeof(pd), 0))
        return 0;

    readThread = CreateRemoteThread(targetHandle, NULL, 0, (LPTHREAD_START_ROUTINE)readShellCodeBaseAddress, (LPVOID)structBaseAddress, 0, NULL);

    CloseHandle(targetHandle);

    if(!readThread)
        return 0;

    return 1;
}

int proxy::init(ULONG accessMask, LPCSTR windowName, bool debug){
    if(!enableSeDebugPrivilege()){
        if(debug)
            std::cout << "[proxy] failed at enabling privilege" << std::endl;

        return 2;
    }

    else if(debug){
        std::cout << "[proxy] privilege enabled" << std::endl;
    }

    DWORD targetProcessPID;
    USHORT originalHandle;
    if(!proxy::findProcessWithHandle(accessMask, windowName, &targetProcessPID, 0, &originalHandle)){
        if(debug)
            std::cout << "[proxy] failed at finding a process with the desired handle" << std::endl;

        return 3;
    }

    else if(debug){
        std::cout << "[proxy] found the handle " << originalHandle << " in the process with PID " << targetProcessPID << std::endl; 
    }

    uintptr_t handleAddress;
    if(!proxy::findAddressWithHandle((HANDLE)(uintptr_t)originalHandle, targetProcessPID, &handleAddress, windowName)){
        if(debug)
            std::cout << "[proxy] failed at finding the address of the handle" << std::endl;

        return 4;
    }
    
    else if(debug){
        std::cout << "[proxy] found the handle " << originalHandle << " in the address 0x" << std::hex << handleAddress << std::dec << " of the process with PID " << targetProcessPID << std::endl;
    }

    LPVOID readShellCodeBaseAddress, readBufferBaseAddress, structBaseAddress;
    if(!proxy::prepareCodeInjection(targetProcessPID, &readShellCodeBaseAddress, &readBufferBaseAddress, &structBaseAddress)){
        if(debug)
            std::cout << "[proxy] failed at preparing the code injection" << std::endl;
            
        return 5;
    }

    else if(debug){
        std::cout << "[proxy] successfully allocated memory pages in the process with PID " << targetProcessPID << std::endl;
        std::cout << "[proxy] read shell code base address: " << readShellCodeBaseAddress << std::endl;
        std::cout << "[proxy] read buffer base address: " << readBufferBaseAddress << std::endl;
        std::cout << "[proxy] struct base address: " << structBaseAddress << std::endl;
    }

    if(!payload::load(targetProcessPID, (uintptr_t)readShellCodeBaseAddress, handleAddress, (uintptr_t)readBufferBaseAddress, (uintptr_t)structBaseAddress)){
        if(debug)
            std::cout << "[payload] failed at injecting the payload" << std::endl;
        return 6;
    }

    else if(debug){
        std::cout << "[payload] successfully writted shell code" << std::endl;
        std::cout << "[payload] successfully writted struct" << std::endl;
        std::cout << "[payload] successfully created read thread" << std::endl;
        std::cout << "[payload] successfully injected" << std::endl;
    }

    return 1;
}

int proxy::exit(bool debug){
    if(readThread){
        TerminateThread(readThread, 0);
        CloseHandle(readThread);
        readThread = 0;
    }

    if(targetPid){
        HANDLE targetHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetPid);
        if(targetHandle){
            if(shellCodeReadRegion) VirtualFreeEx(targetHandle, shellCodeReadRegion, 0, MEM_RELEASE);
            if(readBufferRegion)    VirtualFreeEx(targetHandle, readBufferRegion, 0, MEM_RELEASE);
            if(structRegion)        VirtualFreeEx(targetHandle, structRegion, 0, MEM_RELEASE);
            CloseHandle(targetHandle);

            if(debug) std::cout << "[exit] memory freed in PID " << targetPid << std::endl;
        } else {
            if(debug) std::cout << "[exit] target process already dead, skipping VirtualFreeEx" << std::endl;
        }
    }

    shellCodeReadRegion = 0;
    readBufferRegion    = 0;
    structRegion        = 0;
    readBufferAddress   = 0;
    structAddress       = 0;
    targetPid           = 0;

    if(debug) std::cout << "[exit] clean" << std::endl;
    return 1;
}