#include "upp.hpp"

#include "helper/sdk.hpp"
#include "find target/find target.hpp"
#include "thread or dll protection/dll protection.hpp"
#include "anti debug/anti debug.hpp"
#include "anti suspend/anti suspend.hpp"

#include <iostream>
#include <vector>
#include <cstdlib>

bool crashInstruction = false;
bool problemDebuggerPresent = false;

void NTAPI TlsCallback(PVOID hModule, DWORD dwReason, PVOID pContext) {
    if (dwReason == DLL_PROCESS_ATTACH) {
       // upp::init(true);
    }
}
__attribute__((section(".CRT$XLB"), used))
PIMAGE_TLS_CALLBACK tls_cb_ptr = TlsCallback;

DWORD WINAPI sdk::protectedThread(LPVOID param){

    bool THFD = antiDebug::threadHideFromDebugger();
    bool PDCP = dllProtection::processDynamicCodePolicy();
    //bool PE = antiDebug::deletePe();
    bool PP = antiDebug::parentProcessCheck();
    bool WD = antiSuspend::watchDog();
    bool DOS = antiDebug::DoS();

    
    while(true){
        //MessageBeep(0xFFFFFFFF); // debug for checking if the thread got suspended

        // 1. find targets and revoke handles
        std::vector<DWORD> targets;
        targets = findTarget::find(true);
        std::cout << "hola" << std::endl;

        // 2. check if debugger present
        if(antiDebug::rdtsc() || antiDebug::setDebugPortCheck()){
            problemDebuggerPresent = true;
        }

        // crash if debugger present
        //if(problemDebuggerPresent && crashInstruction)
            //antiDebug::crash();

        // 3. resume our main thread periodically
        antiSuspend::resumeMainThread();

        // 4. check if our watchdog is alive
        antiSuspend::checkWatchDog();

        // 5. check for scylla
        antiDebug::scyllaCheck();

        // 6. check for hooks
        antiDebug::hookCheck();

        // 7. throw exception
        antiDebug::exceptionThrow();

        Sleep(1000);
    }
        
}
/*
bool upp::init(bool crash){
    atexit([]{ upp::exit(); });
    crashInstruction = crash;

    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &mainThread, 0, FALSE, DUPLICATE_SAME_ACCESS);

    // 1.
    //sdk::enableSeDebugPrivilege();

    // 2.
    antiSuspend::bypassFreeze();

    return true;
}

bool upp::exit(){
    TerminateThread(watchdogThread, 0);
    WaitForSingleObject(watchdogThread, 1000);

    VirtualFreeEx(eHandle, payloadBaseAddr, 0, MEM_RELEASE);
    VirtualFreeEx(eHandle, structBaseAddr, 0, MEM_RELEASE);

    CloseHandle(watchdogThread);
    CloseHandle(eHandle);

    return true;
}
    */