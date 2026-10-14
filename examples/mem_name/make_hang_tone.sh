#!/bin/sh
# Two diagnostic PLUG-OUTs, both built on the verified EFFECT TONE method
# (OD/DS/FZ, 0x60A0AC54, slot 1 addresses). Each spins forever (b .) when the
# TONE knob is moved with an OVERDRIVE / DISTORTION / FUZZ effect selected:
#
#   JX-3P-HANG-INPLACE   `b .` written over the method entry (no cave)
#   JX-3P-HANG-CAVE      b.w to a `b .` in the region 1 cave 0x60A07000
#
# INPLACE hangs  -> the TONE knob reaches the method (the procedure is right)
# CAVE hangs too -> code in the cave executes; if only INPLACE hangs, the cave
#                   is not executable memory and hooks must be in place.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
B="python3 $ROOT/tools/plugout_build.py"
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
TONE=0x60a0ac54
CAVE=0x60a07000

printf '\t.syntax unified\n\t.thumb\n\t.text\n\tb.w\t%s\n' "$CAVE" > "$WORK/j.s"
arm-none-eabi-as -mcpu=cortex-a9 -mthumb "$WORK/j.s" -o "$WORK/j.o"
arm-none-eabi-ld -Ttext=$TONE --entry=$TONE "$WORK/j.o" -o "$WORK/j.elf"
arm-none-eabi-objcopy -O binary -j .text "$WORK/j.elf" "$WORK/j.bin"
JMP=$(od -An -tx1 -v "$WORK/j.bin" | tr -d ' \n')

build() {  # name dir patches...
    $B unpack "$ROOT/plugins/JX-3P/Script/Code8_1.Dat" "$WORK/p$2" \
        --relocs-from "$ROOT/plugins/JX-3P/Script/Code8_2.Dat" >/dev/null
    $B set-id "$WORK/p$2" --name "$1" >/dev/null
}

build "HANG INPLACE" a
$B patch "$WORK/pa" --region 1 --at $TONE --hex fee7 >/dev/null
$B plugin "$WORK/pa" --template "$ROOT/plugins/JX-3P-MEM" -o "$ROOT/plugins/JX-3P-HANG-INPLACE" >/dev/null

build "HANG CAVE" b
$B patch "$WORK/pb" --region 1 --at $CAVE --hex fee7 >/dev/null
$B patch "$WORK/pb" --region 1 --at $TONE --hex "$JMP" >/dev/null
$B plugin "$WORK/pb" --template "$ROOT/plugins/JX-3P-MEM" -o "$ROOT/plugins/JX-3P-HANG-CAVE" >/dev/null
echo "entry jump bytes: $JMP"
echo "plugins/JX-3P-HANG-INPLACE  plugins/JX-3P-HANG-CAVE"
