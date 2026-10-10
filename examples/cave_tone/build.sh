#!/bin/sh
# Build hook.bin, linked at the code cave in region 1 (slot 1 addresses).
set -e
CAVE=${CAVE:-0x60a07000}
cd "$(dirname "$0")"
FLAGS="-mcpu=cortex-a9 -mthumb -mfpu=vfpv3-d16 -mfloat-abi=hard -Os -ffreestanding -fno-builtin -nostdlib"
arm-none-eabi-gcc $FLAGS -c hook.c -o hook.o
arm-none-eabi-as -mcpu=cortex-a9 -mthumb engine.s -o engine.o
arm-none-eabi-ld -Ttext=$CAVE --entry=tone_hook hook.o engine.o -o hook.elf
arm-none-eabi-objcopy -O binary -j .text hook.elf hook.bin
arm-none-eabi-objdump -d hook.elf
