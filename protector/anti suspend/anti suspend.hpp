#include <windows.h>

extern HANDLE tHandle, mainThread;
extern HANDLE watchdogThread, eHandle;
extern LPVOID payloadBaseAddr, structBaseAddr;
namespace antiSuspend {
    bool bypassFreeze();
    bool resumeMainThread();
    bool watchDog();
    void checkWatchDog();
}

typedef NTSTATUS (NTAPI* pNtCreateThreadEx)(
    PHANDLE ThreadHandle,
    ACCESS_MASK DesiredAccess,
    PVOID ObjectAttributes,
    HANDLE ProcessHandle,
    PVOID StartRoutine,
    PVOID Argument,
    ULONG CreateFlags,
    SIZE_T ZeroBits,
    SIZE_T StackSize,
    SIZE_T MaximumStackSize,
    PVOID AttributeList
);

typedef NTSTATUS (NTAPI* pNtResumeThread)(
    HANDLE ThreadHandle,
    PULONG PreviousSuspendCount
);

typedef BOOL (WINAPI* pDuplicateHandle)(
    HANDLE   hSourceProcessHandle,
    HANDLE   hSourceHandle,
    HANDLE   hTargetProcessHandle,
    LPHANDLE lpTargetHandle,
    DWORD    dwDesiredAccess,
    BOOL     bInheritHandle,
    DWORD    dwOptions
);

struct watchdogStruct{
    HANDLE mainThreadHandle;
    HANDLE protectThreadHandle;

    pDuplicateHandle dup;
    pNtResumeThread resume;
};