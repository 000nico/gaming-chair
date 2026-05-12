#include "anti debug.hpp"
#include "../helper/sdk.hpp"
#include <windows.h>
#include <winnt.h>
#include <intrin.h>
#include <psapi.h>
#include <tlhelp32.h>    // CreateToolhelp32Snapshot, PROCESSENTRY32
#include <winternl.h>    // PROCESS_BASIC_INFORMATION

#define processDebugPort 7
#define ThreadHideFromDebugger 0x11

bool antiDebug::setDebugPortCheck(){
    auto NtQueryInformationProcess = (fnNtQueryInformationProcess_t)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
    
    LONG_PTR debugPort = 0;
    NtQueryInformationProcess(GetCurrentProcess(), processDebugPort, &debugPort, sizeof(debugPort), nullptr);

    return debugPort != 0;
}

bool antiDebug::threadHideFromDebugger(){
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");pNtSetInformationThread NtSetInformationThread = (pNtSetInformationThread)GetProcAddress(hNtdll, "NtSetInformationThread");

    if (!hNtdll)
        return false;

    NTSTATUS status = NtSetInformationThread(GetCurrentThread(), ThreadHideFromDebugger, NULL, 0);

    if (!NtSetInformationThread)
        return false;

    if(!NT_SUCCESS(status))
        return false;

    return true;

}

bool antiDebug::rdtsc(){
    int cpuInfo[4];
    unsigned __int64 t1 = __rdtsc();
    __cpuidex(cpuInfo, 0, 0); // does nothing but forces cpu
    unsigned __int64 t2 = __rdtsc();

    return (t2 - t1) > 0x100000;
}

bool antiDebug::deletePe() {
    void* baseAddress = (void*)GetModuleHandleA(NULL);
    if (!baseAddress) return false;

    DWORD oldProtect;
    
    if (VirtualProtect(baseAddress, 4096, PAGE_READWRITE, &oldProtect)) {
        
        SecureZeroMemory(baseAddress, 4096);

        VirtualProtect(baseAddress, 4096, oldProtect, &oldProtect);
        
        return true;
    }

    return false;
}

void antiDebug::crash(){
    EmptyWorkingSet(GetCurrentProcess());

    __fastfail(7);
}

DWORD getParentPid(){
    PBI pbi = {};
    
    typedef NTSTATUS(NTAPI* fnNtQIP)(HANDLE, ULONG, PVOID, ULONG, PULONG);
    fnNtQIP NtQIP = (fnNtQIP)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
    
    NtQIP(GetCurrentProcess(), 0, &pbi, sizeof(pbi), NULL);
    
    return (DWORD)pbi.InheritedFromUniqueProcessId;
}

bool antiDebug::parentProcessCheck(){
    DWORD parentPid = getParentPid();

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe = {};
    pe.dwSize = sizeof(pe);

    Process32First(hSnap, &pe);
    do {
        if(pe.th32ProcessID == parentPid){
            CloseHandle(hSnap);
            const char* whitelist[] = {
                "explorer.exe",
                "cmd.exe",
                "powershell.exe",
                "WindowsTerminal.exe"
            };

            for(auto& w : whitelist){
                if(strstr(pe.szExeFile, w)) return false;
            }

            // cualquier otro es sospechoso
            problemDebuggerPresent = true;
            return true;
        }
    } while(Process32Next(hSnap, &pe));

    CloseHandle(hSnap);
    problemDebuggerPresent = true;
    return true;
}

bool antiDebug::scyllaCheck(){
    if(FindWindowA("ScyllaHide", NULL)){
        problemDebuggerPresent = true;
        return true;
    }
    
    else return false;
}

bool antiDebug::hookCheck(){
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");

    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");

    if(!ntdll || !kernel32) return false;

    const char* ntdllFuncs[] = {
        "NtQueryInformationProcess",
        "NtSetInformationProcess",
        "NtSetInformationThread",
        "NtQuerySystemInformation",
        "NtGetContextThread",
        "NtSetContextThread",
        "NtOpenProcess",
        "NtReadVirtualMemory",
        "NtWriteVirtualMemory",
        "NtResumeThread",
        "NtSuspendThread",
        "NtTerminateProcess",
        "NtCreateThreadEx"
    };

    for(int i = 0; i < 13; i++){
        FARPROC func = GetProcAddress(ntdll, ntdllFuncs[i]);
        if(!func) continue;
        BYTE first = *(BYTE*)func;
        if(first == 0xE9 || first == 0xFF){
            problemDebuggerPresent = true;
            return true;
        }
    }

    HMODULE kernelbase = GetModuleHandleA("kernelbase.dll");

    const char* kernel32Funcs[] = {
        "OpenProcess",
        "ReadProcessMemory",
        "WriteProcessMemory",
        "CreateRemoteThread",
        "VirtualAllocEx"
    };

    for(int i = 0; i < 5; i++){
        FARPROC func = GetProcAddress(kernel32, kernel32Funcs[i]);
        if(!func) continue;
        BYTE first = *(BYTE*)func;
        if(first == 0xE9 || first == 0xFF){
            problemDebuggerPresent = true;
            return true;
        }
    }

    return false;
}

LONG WINAPI exceptionHandler(EXCEPTION_POINTERS* ep){
    if(ep->ExceptionRecord->ExceptionCode == 0xC0000008){
        problemDebuggerPresent = true;
    }
    return EXCEPTION_CONTINUE_EXECUTION;
}

bool antiDebug::exceptionThrow(){
    PVOID handler = AddVectoredExceptionHandler(1, exceptionHandler);
    CloseHandle((HANDLE)0xDEADBEEF);
    RemoveVectoredExceptionHandler(handler);
    return problemDebuggerPresent;
}

bool antiDebug::DoS() {
    HANDLE section = nullptr;
    
    NTSTATUS status = NtCreateSection(&section, 
                                      SECTION_ALL_ACCESS, 
                                      nullptr, 
                                      nullptr, 
                                      PAGE_EXECUTE_READWRITE, 
                                      SEC_COMMIT, 
                                      INVALID_HANDLE_VALUE);

    if (status >= 0) { 
        void* base = nullptr;
        SIZE_T view_size = (12ull << 40); 
        
        status = NtMapViewOfSection(section, 
                                    GetCurrentProcess(), 
                                    &base, 
                                    0, 
                                    0, 
                                    nullptr, 
                                    &view_size, 
                                    ViewUnmap, 
                                    0x2000,   
                                    PAGE_READWRITE);
        
        CloseHandle(section);
        return (status >= 0);
    }
    
    return false;
}