#!/bin/sh
# Build probe.bin (engine hook, linked at the cave in region 1) and sender.bin
# (the B.W that redirects the DSP parameter sender to it). Slot-1 addresses, as
# mem_name/engine_build.sh. OS 1.32 send-sequence addresses are baked in here.
#   NBITS=6 ./build.sh      try router bits 0..5 (default)
#   NBITS=1 ./build.sh      only bit 0 (minimal first test)
#   PROBE=cc ./build.sh     assemble cc.s (one CC per panel move) instead of probe.s
set -e
cd "$(dirname "$0")"
HOOK=${HOOK:-0x60a07000}        # zero area of region 1 (same cave as cave_tone)
SENDER=0x60a0b41e               # CDSPJx3p parameter sender
arm-none-eabi-as -mcpu=cortex-a9 -mthumb \
    --defsym HOOK=$HOOK --defsym RET=0x60a0b425 --defsym LIT=0x60a0b470 \
    --defsym CTOR=0x600753cc --defsym SETSX=0x600754bc --defsym WRAP=0x600754c6 \
    --defsym ROUTER=0x6000345c --defsym DTORW=0x600753f4 --defsym DTORM=0x600753e0 \
    --defsym WRAPP=0x600753ea --defsym DEST=0x60445957 \
    ${PROBE:-probe}.s -o probe.o
arm-none-eabi-ld -Ttext=$HOOK --entry=press probe.o -o probe.elf
# reject slot-relative data sections (as mem_scan/scan.py does)
for bad in .rodata .data .bss; do
    if arm-none-eabi-objdump -h probe.elf | grep -q "$bad"; then
        echo "ERROR: probe has a $bad section (slot-relative data): not allowed" >&2
        exit 1
    fi
done
arm-none-eabi-objcopy -O binary -j .text probe.elf probe.bin
# 4-byte entry patch: b.w press
printf '\t.syntax unified\n\t.thumb\n\t.text\n\tb.w\t%s\n' "$HOOK" > sender.s
arm-none-eabi-as -mcpu=cortex-a9 -mthumb sender.s -o sender.o
arm-none-eabi-ld -Ttext=$SENDER --entry=$SENDER sender.o -o sender.bin.elf
arm-none-eabi-objcopy -O binary -j .text sender.bin.elf sender.bin
echo "probe.bin: $(stat -c%s probe.bin) bytes"
arm-none-eabi-objdump -d probe.elf
