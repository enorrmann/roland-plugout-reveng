#!/bin/sh
# Build plugins/SD-TXT: stock JX-3P whose region 0 init first runs sd_txt.s
# (OS CPU): fopen/fread/fclose of A:/hola.txt, then CC witnesses over the MIDI
# router (see sd_txt.s), then the stock init.
#
#   [MASK=0x1A20] examples/sd_txt/make_sd_txt.sh [OUT_DIR]
# Install with --no-patches and capture the ports during the install (see
# docs/examples_sd_txt_README.md for the install and capture commands).
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
OUT=${1:-$ROOT/plugins/SD-TXT}
BASE=0x61800000; CAVE_OFF=0x100; INIT_OFF=0x11823c
B="python3 $ROOT/tools/plugout_build.py"
WORK=$(mktemp -d); trap 'rm -rf "$WORK"' EXIT
cd "$WORK"
arm-none-eabi-as -mcpu=cortex-a9 -mthumb \
    --defsym CAVE_OFF=$CAVE_OFF --defsym INIT_OFF=$INIT_OFF \
    --defsym FOPEN=0x60057e54 --defsym FREAD=0x60057ea8 --defsym FCLOSE=0x60057e28 \
    --defsym WRAPP=0x600753ea --defsym ROUTER=0x6000345c --defsym DTORW=0x600753f4 \
    --defsym MASK=${MASK:-0x1A20} "$HERE/sd_txt.s" -o h.o
arm-none-eabi-ld -Ttext=$(printf 0x%x $((BASE + CAVE_OFF))) --entry=hook h.o -o h.elf
for bad in .rodata .data .bss; do
    if arm-none-eabi-objdump -h h.elf | grep -q "$bad"; then
        echo "ERROR: hook has a $bad section" >&2; exit 1
    fi
done
arm-none-eabi-objcopy -O binary -j .text h.elf "$HERE/sd_txt.bin"
echo "hook: $(wc -c < "$HERE/sd_txt.bin") bytes"
$B unpack "$ROOT/plugins/JX-3P/Script/Code8_1.Dat" p \
    --relocs-from "$ROOT/plugins/JX-3P/Script/Code8_2.Dat" >/dev/null
$B set-id p --name "SD TXT" --version 0.01
$B patch p --region 0 --at 0x61800100 --file "$HERE/sd_txt.bin"
$B patch p --region 0 --at 0x61800024 --hex 01018061 --reloc 0x61800024
$B plugin p --template "$ROOT/plugins/JX-3P-MEM" -o "$OUT" >/dev/null
echo "plugin: $OUT"
