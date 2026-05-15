#pragma once
#include <windows.h>

struct OverlayInstance {
    LPVOID overlayPageBuffer = NULL;
    LPVOID structPageBuffer  = NULL;
    HANDLE remoteThread      = NULL;
    HANDLE explorerHandle    = NULL;
    
    LPVOID overlayPageBuffer2 = NULL;
    LPVOID structPageBuffer2  = NULL;
    HANDLE remoteThread2      = NULL;
};

namespace overlay {
    int init(bool debug, HWND* returnHwnd, OverlayInstance& inst);
    int exit(bool debug, OverlayInstance& inst);
    int initGui(bool debug, HWND* returnHwnd, OverlayInstance& inst);
    int exitGui(bool debug, OverlayInstance& inst);
}

typedef HWND (WINAPI* pCreateWindowInBand)(
    DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName,
    DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
    HWND hWndParent, HMENU hMenu, HINSTANCE hInstance,
    LPVOID lpParam, DWORD dwBand
);

typedef BOOL    (WINAPI* pShowWindow)              (HWND hWnd, int nCmdShow);
typedef BOOL    (WINAPI* pDestroyWindow)           (HWND hWnd);
typedef VOID    (WINAPI* pSleep)                   (DWORD dwMilliseconds);
typedef BOOL    (WINAPI* pPeekMessageW)            (LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
typedef BOOL    (WINAPI* pTranslateMessage)        (const MSG* lpMsg);
typedef LRESULT (WINAPI* pDispatchMessageW)        (const MSG* lpMsg);
typedef LRESULT (WINAPI* pDefWindowProcW)          (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
typedef ATOM    (WINAPI* pRegisterClassEx)         (const WNDCLASSEXW* lpwcx);
typedef HMODULE (WINAPI* pGetModuleHandle)         (LPCSTR lpModuleName);
typedef void    (WINAPI* pRtlZeroMemory)           (void* dest, size_t length);
typedef BOOL    (WINAPI* pSetWindowDisplayAffinity)(HWND hwnd, DWORD dwAffinity);
typedef LONG_PTR (WINAPI* pSetWindowLongPtrW)(HWND, int, LONG_PTR);
typedef LONG_PTR (WINAPI* pGetWindowLongPtrW)(HWND, int);

struct overlayPayloadStruct {
    volatile int signal;

    pCreateWindowInBand       createWindow;
    pSleep                    sleep;
    pPeekMessageW             peekMessage;
    pTranslateMessage         translateMessage;
    pDispatchMessageW         dispatchMessage;
    pShowWindow               showWindow;
    pDestroyWindow            destroyWindow;
    pRegisterClassEx          registerClass;
    pGetModuleHandle          getModuleHandle;
    pDefWindowProcW           defWindowProc;
    pRtlZeroMemory            memset;
    pSetWindowDisplayAffinity affinity;

    WCHAR     className[32];
    WCHAR     windowName[32];
    HINSTANCE hInstance;
    HWND      returnHwnd;
    
    pSetWindowLongPtrW setWindowLongPtr;
    pGetWindowLongPtrW getWindowLongPtr;

    volatile UINT   pendingMsg;
    volatile WPARAM pendingWParam;
    volatile LPARAM pendingLParam;
    volatile int    msgReady;
};