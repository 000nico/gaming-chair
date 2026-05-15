#include <windows.h>
#include <iostream>
#include <thread>
#include <cstdlib>
#include <timeapi.h>

#include "proxy/proxy/proxy.h"
#include "overlay/overlay/overlay.hpp"
#include "overlay/duplication/duplication.h"
#include "memory/memory.h"
#include "gui/gui.h"
#include "modules/modules.hpp"
#include "gui/zdraw/demo/global.hpp"

HWND overlayHwnd = NULL;
OverlayInstance overlayInst;

OverlayInstance guiInst;
//HWND guiHwnd = NULL;
bool destroyed=false;

void exit(){
    proxy::exit(TRUE);
    overlay::exit(TRUE, overlayInst);
    duplication::exit(TRUE);
    overlay::exitGui(TRUE, guiInst);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
    //HWND hWnd = GetConsoleWindow();
    //ShowWindow(hWnd, SW_HIDE); 

    timeBeginPeriod(1);

    int width  = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    std::atexit(exit);

    if(overlay::initGui(TRUE, &guiHwnd, guiInst) != 1) return 0;

    if(IsWindow(guiHwnd))
        std::cout << "[gui] hwnd : " << guiHwnd << std::endl;
    else  
        std::cout << "[gui] error" << std::endl;
    
   
    if(proxy::init(QUERY_INFORMATION | OPERATION | READ | WRITE | DUP_HANDLE, "Counter-Strike 2", TRUE) != 1) return 0;
    if(overlay::init(TRUE, &overlayHwnd, overlayInst) != 1) return 0;

    std::cout << "[overlay] hwnd : " << overlayHwnd << std::endl;
    
    std::thread(duplication::handler, overlayHwnd).detach();
    SetWindowPos(overlayHwnd, HWND_TOPMOST, 0, 0, width, height, SWP_NOACTIVATE);

    //std::thread(gui::manage).detach();
    std::thread(reader::handler).detach();
    std::thread(zEntry::zMain).detach();
    std::thread(triggerbot::handler).detach();
    std::thread(aimassist::handler).detach();
    std::thread(rcs::handler).detach();

    while (true) {
        if(!FindWindowA(nullptr, "Counter-Strike 2") || destroyed) break;
        Sleep(50);
    }

    timeEndPeriod(1);

    exit();

    return 0;
}
