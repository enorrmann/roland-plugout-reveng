#!/bin/sh
# Build hook.bin for region 0, slot-1 addresses (image base 0x61800000).
#   SRC=0x0113FCB0 ./build.sh     read the OS service table instead of 0x60000000
set -e
cd "$(dirname "$0")"
SRC=${SRC:-0x60000000}
BASE=0x61800000
CAVE_OFF=${CAVE_OFF:-0x100}
INIT_OFF=0x11823c
arm-none-eabi-as -mcpu=cortex-a9 -mthumb \
    --defsym SRC=$SRC --defsym CAVE_OFF=$CAVE_OFF \
    --defsym INIT_OFF=$INIT_OFF hook.s -o hook.o
arm-none-eabi-ld -Ttext=$(printf 0x%x $((BASE + CAVE_OFF))) --entry=hook hook.o -o hook.elf
arm-none-eabi-objcopy -O binary -j .text hook.elf hook.bin
arm-none-eabi-objdump -d hook.elf
