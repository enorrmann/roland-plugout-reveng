#!/bin/sh
# make_voice.sh: writes tb03_voice.h (the TB-03 voice program as s1cc C) from Roland's updater. README.md §2.
#
#   sh sdk/s1cc/examples/tb03_ported/tools/make_voice.sh [BQ2_UPD.BIN]
#
# BQ2_UPD.BIN (TB-03 1.07) is only needed when work/tb03/progs/v_0022.hex is not there yet (tools/tb03_extract.py).
set -e
HERE=$(cd "$(dirname "$0")/.." && pwd)
ROOT=$(cd "$HERE/../../../.." && pwd)
WORK=$ROOT/work/tb03
if [ ! -f "$WORK/progs/v_0022.hex" ]; then
    [ -n "$1" ] || { echo "make_voice.sh: no $WORK/progs/v_0022.hex: pass BQ2_UPD.BIN" >&2; exit 2; }
    python3 "$HERE/tools/tb03_extract.py" "$1" "$WORK"
fi
tmp=$(mktemp)
trap 'rm -f "$tmp"' EXIT
# --split 415: instructions 0..414 on DSP A, 415..750 on DSP B (both fit the budgets)
# --zero-cells 0bd,0bf: two bank-0 cells part A reads that only part B writes; 0 in the TB-03 setup (README §2)
# --import-ranges: the measured range of every value A hands to B (README §2)
cd "$HERE"
python3 "$ROOT/sdk/s1cc/tools/dsplift.py" "$tmp" --bq2 --kernel --split 415 --zero-cells 0bd,0bf \
    --import-ranges tb03_import_ranges.txt tb03_voice=../../../../work/tb03/progs/v_0022.hex@0x22
{
    echo '/* tb03_voice.h: the TB-03 voice program (BQ2:Appli 1.07, voice DSP 0x0022, 751 instructions) as C, in two parts'
    echo ' * (tb03_voice_a: instructions 0..414 on DSP A, tb03_voice_b: 415..750 on DSP B). Written by sdk/s1cc/tools/dsplift.py'
    echo ' * --bq2 --kernel from the firmware (tools/make_voice.sh); do not edit: README.md §2. */'
    tail -n +2 "$tmp"
} > "$HERE/tb03_voice.h"
echo "make_voice.sh: $HERE/tb03_voice.h"
