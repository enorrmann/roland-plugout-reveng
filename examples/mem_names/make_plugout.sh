#!/bin/sh
# Build JX-3P-MEM-ALL: stock JX-3P whose region 0 init hook writes 7 bytes of
# OS memory, 8 bytes each from SRC, SRC+8 and SRC+16 (default 0x60000000), as
# 16 hex chars over the names of slots 1, 2 and 3 respectively.
#   [SRC=0x60000000] [NAME="MEM ALL"] examples/mem_names/make_plugout.sh [OUT_DIR]
# Install in slot 3: python3 tools/plugout_tool.py install plugins/JX-3P-MEM-ALL --slot 3 --no-patches
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
SRC=${SRC:-0x60000000}
OUT=${1:-$ROOT/plugins/JX-3P-MEM-ALL}
BASE=0x61800000; CAVE_OFF=0x100; INIT_OFF=0x11823c
B="python3 $ROOT/tools/plugout_build.py"
WORK=$(mktemp -d); trap 'rm -rf "$WORK"' EXIT
cd "$HERE"
arm-none-eabi-as -mcpu=cortex-a9 -mthumb --defsym SRC=$SRC --defsym CAVE_OFF=$CAVE_OFF \
    --defsym INIT_OFF=$INIT_OFF hook.s -o hook.o
arm-none-eabi-ld -Ttext=$(printf 0x%x $((BASE + CAVE_OFF))) --entry=hook hook.o -o hook.elf
arm-none-eabi-objcopy -O binary -j .text hook.elf hook.bin
arm-none-eabi-objdump -d hook.elf | tail -n +7
echo "hook: $(wc -c < hook.bin) bytes"
$B unpack "$ROOT/plugins/JX-3P/Script/Code8_1.Dat" "$WORK/p" \
    --relocs-from "$ROOT/plugins/JX-3P/Script/Code8_2.Dat" >/dev/null
$B set-id "$WORK/p" --name "${NAME:-MEM ALL}"
$B patch "$WORK/p" --region 0 --at 0x61800100 --file "$HERE/hook.bin"
$B patch "$WORK/p" --region 0 --at 0x61800024 --hex 01018061 --reloc 0x61800024
$B plugin "$WORK/p" --template "$ROOT/plugins/JX-3P-MEM" -o "$OUT"
echo "done: $OUT"
