#!/bin/bash

mkdir -p bin

x86_64-w64-mingw32-g++ \
    -O1 \
    -nostdlib \
    -fno-stack-protector \
    -fno-exceptions \
    -fno-rtti \
    -fPIC \
    -c "overlay payload.cpp" -o bin/payload.o

echo "=== External references ===" 
objdump -d bin/payload.o | grep "call\|jmp"

objcopy -O binary \
    --only-section=.text \
    bin/payload.o bin/payload.bin

xxd -i bin/payload.bin | sed 's/unsigned char/static unsigned char/g; s/unsigned int/static unsigned int/g' > bin/payload.h

echo "Done - $(wc -c < bin/payload.bin) bytes"

rm -f bin/payload.o bin/payload.bin