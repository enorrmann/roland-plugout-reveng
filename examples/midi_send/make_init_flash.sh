#!/bin/sh
# Build plugins/JX-3P-INIT-FLASH: stock JX-3P whose region 0 init first runs
# init_flash.s (OS CPU): COUNT SysEx messages F0 7D 01 <offset> <LEN bytes of
# QSPI flash as nibbles> F7 (offset = SRC + i*LEN) through the OS router, then
# "FLASH OK" over its own name, then the stock init. Normally driven by
# bulk_dump.py --flash. Default: the boot loader, flash 0x0 (BASE_FIRMWARE §8.14).
#
#   [SRC=0] [LEN=32] [COUNT=64] [MASK=0x1A20] examples/midi_send/make_init_flash.sh [OUT_DIR]
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
OUT=${1:-$ROOT/plugins/JX-3P-INIT-FLASH}
BASE=0x61800000; CAVE_OFF=0x100; INIT_OFF=0x11823c
B="python3 $ROOT/tools/plugout_build.py"
WORK=$(mktemp -d); trap 'rm -rf "$WORK"' EXIT
cd "$WORK"
arm-none-eabi-as -mcpu=cortex-a9 -mthumb --defsym CAVE_OFF=$CAVE_OFF --defsym INIT_OFF=$INIT_OFF \
    --defsym ROUTER=0x6000345c --defsym DTORW=0x600753f4 --defsym MASK=${MASK:-0x1A20} \
    --defsym FREAD=0x6004306a \
    --defsym SRC=${SRC:-0} --defsym LEN=${LEN:-32} --defsym COUNT=${COUNT:-64} \
    --defsym CTOR=0x600753cc --defsym SETSX=0x600754bc --defsym WRAPM=0x600754c6 --defsym DTORM=0x600753e0 \
    "$HERE/init_flash.s" -o h.o
arm-none-eabi-ld -Ttext=$(printf 0x%x $((BASE + CAVE_OFF))) --entry=hook h.o -o h.elf
for bad in .rodata .data .bss; do
    if arm-none-eabi-objdump -h h.elf | grep -q "$bad"; then
        echo "ERROR: hook has a $bad section" >&2; exit 1
    fi
done
arm-none-eabi-objcopy -O binary -j .text h.elf "$HERE/init_flash.bin"
arm-none-eabi-objdump -d h.elf | tail -n +7
echo "hook: $(wc -c < "$HERE/init_flash.bin") bytes"
$B unpack "$ROOT/plugins/JX-3P/Script/Code8_1.Dat" p \
    --relocs-from "$ROOT/plugins/JX-3P/Script/Code8_2.Dat" >/dev/null
$B set-id p --name "INIT FLASH" --version 1.22
$B patch p --region 0 --at 0x61800100 --file "$HERE/init_flash.bin"
$B patch p --region 0 --at 0x61800024 --hex 01018061 --reloc 0x61800024
$B plugin p --template "$ROOT/plugins/JX-3P-MEM" -o "$OUT" >/dev/null
echo "plugin: $OUT  SRC=${SRC:-0} LEN=${LEN:-32} COUNT=${COUNT:-64} MASK=${MASK:-0x1A20}"
