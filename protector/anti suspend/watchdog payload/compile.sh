#!/bin/bash
mkdir -p bin
x86_64-w64-mingw32-g++ \
    -O1 \
    -nostdlib \
    -fno-stack-protector \
    -fno-exceptions \
    -fno-rtti \
    -fPIC \
    -c watchdog.cpp -o bin/watchdog.o
objcopy -O binary --only-section=.text bin/watchdog.o bin/watchdog.bin
xxd -i bin/watchdog.bin | sed 's/unsigned char/static unsigned char/g; s/unsigned int/static unsigned int/g' > bin/watchdog.h
echo "ready - $(wc -c < bin/watchdog.bin) bytes"