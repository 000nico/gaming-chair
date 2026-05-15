#include<windows.h>

namespace antiDebug {
    bool setDebugPortCheck();
    void crash();
    bool threadHideFromDebugger();
    bool rdtsc();
    bool deletePe();
    bool parentProcessCheck();
    bool scyllaCheck();
    bool hookCheck();
    bool exceptionThrow();
    bool DoS();
}

typedef NTSTATUS(NTAPI* fnNtSetInformationProcess_t)(
    HANDLE ProcessHandle,
    ULONG  ProcessInformationClass,
    PVOID  ProcessInformation,
    ULONG  ProcessInformationLength
);

typedef struct _PBI {
    NTSTATUS ExitStatus;
    PVOID PebBaseAddress;
    ULONG_PTR AffinityMask;
    LONG BasePriority;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR InheritedFromUniqueProcessId;
} PBI;

typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

extern "C" {
    NTSTATUS NTAPI NtCreateSection(
        PHANDLE            SectionHandle,
        ACCESS_MASK        DesiredAccess,
        PVOID              ObjectAttributes,
        PLARGE_INTEGER     MaximumSize,
        ULONG              SectionPageProtection,
        ULONG              AllocationAttributes,
        HANDLE             FileHandle
    );

    NTSTATUS NTAPI NtMapViewOfSection(
        HANDLE             SectionHandle,
        HANDLE             ProcessHandle,
        PVOID* BaseAddress,
        ULONG_PTR          ZeroBits,
        SIZE_T             CommitSize,
        PLARGE_INTEGER     SectionOffset,
        PSIZE_T            ViewSize,
        SECTION_INHERIT    InheritDisposition,
        ULONG              AllocationType,
        ULONG              Win32Protect
    );
}

