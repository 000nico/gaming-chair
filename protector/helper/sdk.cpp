#include <windows.h>
#include "sdk.hpp"

bool sdk::enableSeDebugPrivilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp = { 1 };
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) return false;
    LookupPrivilegeValueA(NULL, "SeDebugPrivilege", &tp.Privileges[0].Luid);
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
    CloseHandle(hToken);
    return GetLastError() == ERROR_SUCCESS;
}