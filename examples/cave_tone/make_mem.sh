#!/bin/sh
# Build plugins/JX-3P-MEM-TONE: stock JX-3P whose EFFECT TONE method (shared by
# OD/DS/FZ, 0x60A0AC54) is replaced by mem_hook.s: TONE = n copies the 4 bytes at
# BASE + n*STRIDE into the RAM word CUTH. With CUTH = the RAM address of the live
# EFFECT TONE value, the knob overwrites itself with OS memory and
# `read_param.py --edit --watch` prints the word (LE: b0 = low byte) per step.
#
#   BASE=0x60000000 CUTH=<ram addr of TONE> [STRIDE=4] examples/cave_tone/make_mem.sh [OUT_DIR]
# CUTH must be known first (scan of the edit buffer with preset_fill NEEDLE32).
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
OUT=${1:-$ROOT/plugins/JX-3P-MEM-TONE}
: "${CUTH:?set CUTH (RAM address of VCF CUTOFF FREQ H of preset 1, slot 3)}"
BASE=${BASE:-0x60000000}
HOOK=0x60a07000
METHOD=0x60a0ac54
B="python3 $ROOT/tools/plugout_build.py"
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$HERE"
A="-mcpu=cortex-a9 -mthumb -mfpu=vfpv3-d16 -mfloat-abi=hard"
arm-none-eabi-as $A --defsym BASE=$BASE --defsym CUTH=$CUTH --defsym STRIDE=${STRIDE:-4} mem_hook.s -o mem_hook.o
arm-none-eabi-as $A engine.s -o engine.o
arm-none-eabi-ld -Ttext=$HOOK --entry=tone_hook mem_hook.o engine.o -o mem_hook.elf
arm-none-eabi-objcopy -O binary -j .text mem_hook.elf mem_hook.bin
printf '\t.syntax unified\n\t.thumb\n\t.text\n\tb.w\t%s\n' "$HOOK" > mem_entry.s
arm-none-eabi-as $A mem_entry.s -o mem_entry.o
arm-none-eabi-ld -Ttext=$METHOD --entry=$METHOD mem_entry.o -o mem_entry.elf
arm-none-eabi-objcopy -O binary -j .text mem_entry.elf mem_entry.bin
$B unpack "$ROOT/plugins/JX-3P/Script/Code8_1.Dat" "$WORK/p" \
    --relocs-from "$ROOT/plugins/JX-3P/Script/Code8_2.Dat" >/dev/null
$B set-id "$WORK/p" --name "JX-3P MEM TONE" --version 1.07
$B patch "$WORK/p" --region 1 --at $HOOK --file mem_hook.bin
$B patch "$WORK/p" --region 1 --at $METHOD --file mem_entry.bin
$B plugin "$WORK/p" --template "$ROOT/plugins/JX-3P-MEM" -o "$OUT" >/dev/null
echo "plugin: $OUT  BASE=$BASE CUTH=$CUTH"
