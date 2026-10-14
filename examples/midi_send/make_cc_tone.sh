#!/bin/sh
# Build plugins/JX-3P-CC-TONE: stock JX-3P whose EFFECT TONE method (OD/DS/FZ,
# 0x60A0AC54) jumps to cc_tone.s in the region 1 cave: each TONE move sends
# BF 66 vv through the OS router, then runs the verified inverted-tone body
# (the audible inversion proves the hook ran).
#
# Only TONE == GATE (default 255) sends: an ungated v1 hung at slot selection.
# STAGE=1: no router call (wrap + dtor only), see cc_tone.s.
#   [STAGE=2] [MASK=0x1A20] [GATE=255] examples/midi_send/make_cc_tone.sh [OUT_DIR]
# Install in slot 3, capture with run_probe.py --no-install, select an OD/DS/FZ
# effect and move TONE.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
OUT=${1:-$ROOT/plugins/JX-3P-CC-TONE}
HOOK=0x60a07000
METHOD=0x60a0ac54
B="python3 $ROOT/tools/plugout_build.py"
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
A="-mcpu=cortex-a9 -mthumb -mfpu=vfpv3-d16 -mfloat-abi=hard"
cd "$WORK"
arm-none-eabi-as $A --defsym WRAPP=0x600753ea --defsym ROUTER=0x6000345c \
    --defsym DTORW=0x600753f4 --defsym MASK=${MASK:-0x1A20} --defsym GATE=${GATE:-255} --defsym STAGE=${STAGE:-2} "$HERE/cc_tone.s" -o h.o
arm-none-eabi-as $A "$ROOT/examples/cave_tone/engine.s" -o e.o
arm-none-eabi-ld -Ttext=$HOOK --entry=tone_hook h.o e.o -o h.elf
for bad in .rodata .data .bss; do
    if arm-none-eabi-objdump -h h.elf | grep -q "$bad"; then
        echo "ERROR: hook has a $bad section" >&2; exit 1
    fi
done
arm-none-eabi-objcopy -O binary -j .text h.elf "$HERE/cc_tone.bin"
printf '\t.syntax unified\n\t.thumb\n\t.text\n\tb.w\t%s\n' "$HOOK" > j.s
arm-none-eabi-as $A j.s -o j.o
arm-none-eabi-ld -Ttext=$METHOD --entry=$METHOD j.o -o j.elf
arm-none-eabi-objcopy -O binary -j .text j.elf j.bin
arm-none-eabi-objdump -d h.elf | sed -n '/<tone_hook>:/,$p'
$B unpack "$ROOT/plugins/JX-3P/Script/Code8_1.Dat" p \
    --relocs-from "$ROOT/plugins/JX-3P/Script/Code8_2.Dat" >/dev/null
$B set-id p --name "JX-3P CC TONE" --version 1.1${STAGE:-2}
$B patch p --region 1 --at $HOOK --file "$HERE/cc_tone.bin"
$B patch p --region 1 --at $METHOD --file j.bin
$B plugin p --template "$ROOT/plugins/JX-3P-MEM" -o "$OUT" >/dev/null
echo "plugin: $OUT  STAGE=${STAGE:-2} MASK=${MASK:-0x1A20} GATE=${GATE:-255}"
