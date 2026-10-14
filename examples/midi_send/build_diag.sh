#!/bin/sh
# Build the diagnostic probe: dumps Msg/Wrap state to preset NAME0 area.
set -e
cd "$(dirname "$0")"
HOOK=${HOOK:-0x60a07000}
SENDER=0x60a0b41e
arm-none-eabi-as -mcpu=cortex-a9 -mthumb \
    --defsym HOOK=$HOOK --defsym RET=0x60a0b425 --defsym LIT=0x60a0b470 \
    --defsym DEST=0x60445957 \
    --defsym CTOR=0x600753cc --defsym SETSX=0x600754bc --defsym WRAP=0x600754c6 \
    diag.s -o diag.o
arm-none-eabi-ld -Ttext=$HOOK --entry=press diag.o -o diag.elf
for bad in .rodata .data .bss; do
    if arm-none-eabi-objdump -h diag.elf | grep -q "$bad"; then
        echo "ERROR: diag has a $bad section" >&2; exit 1
    fi
done
arm-none-eabi-objcopy -O binary -j .text diag.elf diag.bin
# reuse sender.bin from the main probe build (same patch point)
if [ ! -f sender.bin ]; then
    printf '\t.syntax unified\n\t.thumb\n\t.text\n\tb.w\t%s\n' "$HOOK" > sender.s
    arm-none-eabi-as -mcpu=cortex-a9 -mthumb sender.s -o sender.o
    arm-none-eabi-ld -Ttext=$SENDER --entry=$SENDER sender.o -o sender.bin.elf
    arm-none-eabi-objcopy -O binary -j .text sender.bin.elf sender.bin
fi
echo "diag.bin: $(stat -c%s diag.bin) bytes"
arm-none-eabi-objdump -d diag.elf
