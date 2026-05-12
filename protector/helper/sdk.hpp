#include <windows.h>

extern bool problemDebuggerPresent, crashInstruction;
extern HANDLE mainThread;
namespace sdk{
    bool enableSeDebugPrivilege();
    DWORD WINAPI protectedThread(LPVOID param);
}

typedef NTSTATUS(NTAPI* fnNtSetInformationProcess_t)(
    HANDLE ProcessHandle,
    ULONG  ProcessInformationClass,
    PVOID  ProcessInformation,
    ULONG  ProcessInformationLength
);

typedef NTSTATUS(NTAPI* fnNtQueryInformationProcess_t)(
    HANDLE ProcessHandle,
    ULONG  ProcessInformationClass,
    PVOID  ProcessInformation,
    ULONG  ProcessInformationLength,
    PULONG ReturnLength
);

typedef NTSTATUS (NTAPI* pNtSetInformationThread)(
    HANDLE ThreadHandle,
    ULONG ThreadInformationClass,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength
);