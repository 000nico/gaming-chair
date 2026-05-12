// readPayload.cpp
#include <cstdint>
#include <windows.h>
#include <immintrin.h>
#include "../../proxy/proxy.h"

void pReadMem(payloadStruct* data){
    while(true){
        if(data->readSignal){
            HANDLE handle = *(HANDLE*) data->handleAddress;
            data->read(handle, (PVOID)data->targetAddress, (LPVOID)data->readBuffer, data->size, nullptr);
            data->readSignal = 0;
        } else {
            _mm_pause();
        }
    }
}