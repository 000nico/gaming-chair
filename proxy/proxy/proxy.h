// proxy.h
#pragma once
#include <windows.h>

// ─────────────────────────────────────────────
// extern variables
// ─────────────────────────────────────────────
extern DWORD       targetPid;
extern HANDLE      readThread;
extern uintptr_t   readBufferAddress;
extern uintptr_t   structAddress;
extern LPVOID      shellCodeReadRegion;
extern LPVOID      readBufferRegion;
extern LPVOID      structRegion;

// ─────────────────────────────────────────────
// NtQuerySystemInformation
// ─────────────────────────────────────────────
typedef NTSTATUS (NTAPI *PNtQuerySystemInformation)(
    ULONG  SystemInformationClass,
    PVOID  SystemInformation,
    ULONG  SystemInformationLength,
    PULONG ReturnLength
);

typedef struct {
    USHORT UniqueProcessId;
    USHORT CreatorBackTraceIndex;
    UCHAR  ObjectTypeIndex;
    UCHAR  HandleAttributes;
    USHORT HandleValue;
    PVOID  Object;
    ULONG  GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct {
    ULONG Count;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1];
} SYSTEM_HANDLE_INFORMATION;

// ─────────────────────────────────────────────
// access types
// ─────────────────────────────────────────────
enum accessType {
    TERMINATE         = 0x0001,
    CREATE_THREAD     = 0x0002,
    OPERATION         = 0x0008,
    READ              = 0x0010,
    WRITE             = 0x0020,
    DUP_HANDLE        = 0x0040,
    QUERY_INFORMATION = 0x0400,
    ALL_ACCESS        = 0x1FFFFF
};

// ─────────────────────────────────────────────
// payload function pointers
// ─────────────────────────────────────────────
typedef NTSTATUS (NTAPI *fnNtReadVirtualMemory_t)(
    HANDLE,
    PVOID,
    PVOID,
    SIZE_T,
    PSIZE_T
);

// ─────────────────────────────────────────────
// payload struct
// ─────────────────────────────────────────────
struct payloadStruct {
    uintptr_t                  handleAddress;
    uintptr_t                  targetAddress;
    SIZE_T                     size;

    uintptr_t                  readBuffer;
    fnNtReadVirtualMemory_t    read;
    volatile int               readSignal;
};

// ─────────────────────────────────────────────
// payload namespace
// ─────────────────────────────────────────────
namespace payload {

    int load(
        DWORD     pid,
        uintptr_t readShellCodeBaseAddress,
        uintptr_t handleAddress,
        uintptr_t readBufferAddress,
        uintptr_t structBaseAddress
    );

    template<typename T>
    T read(DWORD pid, uintptr_t address, uintptr_t addressOfReadBuffer, uintptr_t structBaseAddress) {
        HANDLE targetHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

        payloadStruct pd = {};
        ReadProcessMemory(targetHandle, (LPVOID)structBaseAddress, &pd, sizeof(pd), 0);
        pd.targetAddress = address;
        pd.readSignal    = 1;
        pd.size          = sizeof(T);
        WriteProcessMemory(targetHandle, (LPVOID)structBaseAddress, &pd, sizeof(pd), 0);

        int signal = 1;
        while (signal) {
            ReadProcessMemory(targetHandle, (LPVOID)structBaseAddress, &pd, sizeof(pd), 0);
            signal = pd.readSignal;
        }

        T result{};
        ReadProcessMemory(targetHandle, (LPVOID)addressOfReadBuffer, &result, sizeof(T), 0);

        CloseHandle(targetHandle);
        return result;
    }
}

// ─────────────────────────────────────────────
// proxy namespace
// ─────────────────────────────────────────────
namespace proxy {

    bool enableSeDebugPrivilege();

    int findProcessWithHandle(
        ULONG    accessMask,
        LPCSTR   windowName,
        DWORD*   processPIDBuffer,
        HANDLE*  duplicatedHandleBuffer,
        USHORT*  originalHandleValue
    );

    int findAddressWithHandle(
        HANDLE     handle,
        DWORD      pid,
        uintptr_t* foundAtAddress,
        LPCSTR     windowName
    );

    int prepareCodeInjection(
        DWORD   pid,
        LPVOID* rShellCodeBaseAddress,
        LPVOID* rBufferBaseAddress,
        LPVOID* structBaseAddress
    );

    int init(ULONG accessMask, LPCSTR windowName, bool debug);
    int exit(bool debug);

    template<typename T>
    T read(uintptr_t address) {
        return payload::read<T>(targetPid, address, readBufferAddress, structAddress);
    }
}