#!/bin/sh
# Build press.bin (engine hook, linked at the cave in region 1) and sender.bin
# (the B.W that redirects the DSP parameter sender to it). Slot-1 addresses.
#   SRC=0x0113FCB0 ./engine_build.sh
#   TEST=1           build the visible-name test hook ("MEM HOOK n")
#   HANG=1           build a hook that spins forever (is the sender reached at all?)
#   DEST=<address>   RAM image of NAME0 of preset 1 (default 0x60445957, slot 3)
set -e
cd "$(dirname "$0")"
SRC=${SRC:-0x60000000}
HOOK=${HOOK:-0x60a07000}        # zero area of region 1 (same cave as cave_tone)
NAME=0x61800010                 # region 0 SYNINFO name, slot 1
SENDER=0x60a0b41e               # CDSPJx3p parameter sender (see CUSTOM_PLUGOUTS.md)
arm-none-eabi-as -mcpu=cortex-a9 -mthumb \
    --defsym SRC=$SRC --defsym DEST=${DEST:-0x60445957} --defsym HOOK=$HOOK --defsym NAME=$NAME \
    ${TEST:+--defsym TEST=1} ${HANG:+--defsym HANG=1} --defsym COUNTER=0x70 --defsym RET=0x60a0b425 --defsym LIT=0x60a0b470 press.s -o press.o
arm-none-eabi-ld -Ttext=$HOOK --entry=press press.o -o press.elf
arm-none-eabi-objcopy -O binary -j .text press.elf press.bin
# 4-byte entry patch: b.w press
printf '\t.syntax unified\n\t.thumb\n\t.text\n\tb.w\t%s\n' "$HOOK" > sender.s
arm-none-eabi-as -mcpu=cortex-a9 -mthumb sender.s -o sender.o
arm-none-eabi-ld -Ttext=$SENDER --entry=$SENDER sender.o -o sender.elf
arm-none-eabi-objcopy -O binary -j .text sender.elf sender.bin
arm-none-eabi-objdump -d press.elf sender.elf
