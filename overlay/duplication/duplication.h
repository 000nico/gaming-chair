#include "../../direct_x/direct_x.h"
#include <iostream>
#include <atomic>

extern DX dx;
namespace duplication {
    int handler(HWND hwnd);
    bool exit(bool debug);
    inline std::atomic<bool> running = true;
}