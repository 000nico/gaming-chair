#include<windows.h>
#include "../anti suspend.hpp"

void watchDogPayload(watchdogStruct* data){
    while(true){
        data->resume(data->mainThreadHandle, NULL);
        data->resume(data->protectThreadHandle, NULL);
    }
}