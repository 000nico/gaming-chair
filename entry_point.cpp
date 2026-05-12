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

//OverlayInstance guiInst;
//HWND guiHwnd = NULL;

void exit(){
    proxy::exit(TRUE);
    overlay::exit(TRUE, overlayInst);
    duplication::exit(TRUE);
    //overlay::exitGui(TRUE, guiInst);
}

int main(){
    system("cls");

    timeBeginPeriod(1);

    int width  = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    std::atexit(exit);

    /*
    overlay::init(TRUE, &guiHwnd, guiInst);

    if(IsWindow(guiHwnd))
        std::cout << "[gui] hwnd : " << guiHwnd << std::endl;
    else  
        std::cout << "[gui] error" << std::endl;
    */
   
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

    while (true) {
    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(0x47) & 0x8000)) {
        break;
    }
    Sleep(50);
    }

    timeEndPeriod(1);

    exit();

    return 0;
}
