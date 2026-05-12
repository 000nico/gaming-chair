#include "../modules.hpp"
#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <ctime>

#define MOUSE_LEFT_BUTTON_DOWN   0x0001
#define MOUSE_LEFT_BUTTON_UP     0x0002
#define max(a, b) (((a) > (b)) ? (a) : (b))

void triggerbot::handler() {
    pNtUserSendInput NtUserSendInput = NULL;

    HMODULE hWin32u = GetModuleHandleA("win32u.dll");
    NtUserSendInput = (pNtUserSendInput)GetProcAddress(hWin32u, "NtUserSendInput");

    srand(time(NULL));

    HWND game = FindWindowA(nullptr, "Counter-Strike 2");

    while (true) {
        if (triggerbot::enabled) {
            if (isAiming > 0) { 
                //std::cout << "aiming: " << isAiming << std::endl;

                if(!triggerbot::randomization)
                    Sleep(triggerbot::delay);
                else
                    Sleep(max(rand() % 60 + (int)triggerbot::delay - 30, 0)); // 30ms between the delay
                
                if(GetForegroundWindow() == game){
                    INPUT input = {0};
                    input.type = INPUT_MOUSE;
                    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                    NtUserSendInput(1, &input, sizeof(INPUT));

                    input = {0};
                    input.type = INPUT_MOUSE;
                    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                    NtUserSendInput(1, &input, sizeof(INPUT));

                    Sleep(1); 
                    
                }
            }
        }
        Sleep(1); 
    }
}