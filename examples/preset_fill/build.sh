#!/bin/sh
# Build hook.bin for region 0 (slot-1 addresses, image base 0x61800000).
# Overrides (environment): LO HI SRC LEN MODE REPORT NEEDLE8, e.g.  MODE=1 ./build.sh
set -e
cd "$(dirname "$0")"
BASE=0x61800000
CAVE_OFF=${CAVE_OFF:-0x100}
INIT_OFF=0x11823c
DEFS=""
for v in LO HI SRC LEN MODE REPORT NEEDLE8 NEEDLE32; do
    eval "x=\${$v:-}"
    [ -n "$x" ] && DEFS="$DEFS -D$v=$x"
done
arm-none-eabi-gcc -mcpu=cortex-a9 -mthumb -Os -ffreestanding -fno-builtin \
    -fno-jump-tables -fno-stack-protector -fno-pic -nostdlib $DEFS \
    -c scanfill.c -o scanfill.o
arm-none-eabi-as -mcpu=cortex-a9 -mthumb --defsym CAVE_OFF=$CAVE_OFF \
    --defsym INIT_OFF=$INIT_OFF entry.s -o entry.o
arm-none-eabi-ld -Ttext=$(printf 0x%x $((BASE + CAVE_OFF))) --entry=entry \
    entry.o scanfill.o -o hook.elf
arm-none-eabi-objcopy -O binary -j .text hook.elf hook.bin
arm-none-eabi-objdump -d hook.elf | grep -c . >/dev/null
echo "hook.bin: $(wc -c < hook.bin) bytes"
