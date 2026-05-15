#!/bin/bash

mkdir -p bin2

x86_64-w64-mingw32-g++ \
    -O1 \
    -c "gui payload.cpp" \
    -o bin2/gui_payload.o

echo "=== External references ==="
objdump -d bin2/gui_payload.o | grep "call\|jmp"

objcopy -O binary \
    --only-section=.text \
    bin2/gui_payload.o \
    bin2/gui_payload.bin

xxd -i bin2/gui_payload.bin | \
sed 's/unsigned char/static unsigned char/g; s/unsigned int/static unsigned int/g' \
> bin2/gui_payload.h

echo "Done - $(wc -c < bin2/gui_payload.bin) bytes"

rm -f bin2/gui_payload.o bin2/gui_payload.bin