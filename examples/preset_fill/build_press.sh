#!/bin/sh
# Build press.bin (engine hook linked at the region 1 cave, slot-1 addresses)
# and sender.bin (the B.W at the sender entry). Overrides: DEST SRC LEN MODE.
set -e
cd "$(dirname "$0")"
HOOK=${HOOK:-0x60a07000}
SENDER=0x60a0b41e
DEFS=""
for v in DEST SRC LEN MODE; do
    eval "x=\${$v:-}"
    [ -n "$x" ] && DEFS="$DEFS -D$v=$x"
done
arm-none-eabi-gcc -mcpu=cortex-a9 -mthumb -Os -ffreestanding -fno-builtin \
    -fno-jump-tables -fno-stack-protector -fno-pic -nostdlib $DEFS \
    -c fill.c -o fill.o
arm-none-eabi-as -mcpu=cortex-a9 -mthumb --defsym HOOK=$HOOK \
    --defsym RET=0x60a0b425 --defsym LIT=0x60a0b470 press.s -o press.o
arm-none-eabi-ld -Ttext=$HOOK --entry=press press.o fill.o -o press.elf
arm-none-eabi-objcopy -O binary -j .text press.elf press.bin
printf '\t.syntax unified\n\t.thumb\n\t.text\n\tb.w\t%s\n' "$HOOK" > sender.s
arm-none-eabi-as -mcpu=cortex-a9 -mthumb sender.s -o sender.o
arm-none-eabi-ld -Ttext=$SENDER --entry=$SENDER sender.o -o sender.elf
arm-none-eabi-objcopy -O binary -j .text sender.elf sender.bin
echo "press.bin: $(wc -c < press.bin) bytes"
