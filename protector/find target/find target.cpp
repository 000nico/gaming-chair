#include <windows.h>
#include <vector>
#include "find target.hpp"

#define _SYSTEM_HANDLE_INFORMATION 0x10

std::vector<DWORD> findTarget::find(bool revokeHandle) {
    std::vector<DWORD> targets;

    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    auto NtQuerySystemInformation =
        (PNtQuerySystemInformation)GetProcAddress(hNtdll, "NtQuerySystemInformation");

    ULONG size = 0x10000;
    BYTE* buffer = nullptr;
    NTSTATUS status;

    while (true) {
        buffer = new BYTE[size];

        status = NtQuerySystemInformation(
            _SYSTEM_HANDLE_INFORMATION,
            buffer,
            size,
            nullptr
        );

        if (status == 0) break;

        delete[] buffer;

        if (status == 0xC0000004) {
            size *= 2;
            continue;
        }

        return {};
    }

    auto info = (SYSTEM_HANDLE_INFORMATION*)buffer;

    for (ULONG i = 0; i < info->Count; i++) {

        DWORD pid = info->Handles[i].UniqueProcessId;

        HANDLE process = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pid);
        if (!process) continue;

        HANDLE dup = nullptr;

        if (!DuplicateHandle(process,
            (HANDLE)(ULONG_PTR)info->Handles[i].HandleValue,
            GetCurrentProcess(),
            &dup,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS)) {

            CloseHandle(process);
            continue;
        }

        if (GetProcessId(dup) == GetCurrentProcessId()) {
            targets.push_back(pid);

            if(revokeHandle){
                DuplicateHandle(process, (HANDLE)(ULONG_PTR)info->Handles[i].HandleValue, GetCurrentProcess(), &dup, DUPLICATE_SAME_ACCESS, FALSE, DUPLICATE_CLOSE_SOURCE);
                CloseHandle(dup);
            }
        }

        CloseHandle(dup);
        CloseHandle(process);
    }

    delete[] buffer;
    return targets;
}