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
consts=$(mktemp)
trap 'rm -f "$tmp" "$consts"' EXIT
# the parameters no knob moves: the TB-03 init's values (tb03_tables.h tb03_init), constants of the lift (README §2)
python3 - "$HERE/tb03_tables.h" "$HERE/tb03_names.txt" > "$consts" <<'PY'
import re, sys
src = open(sys.argv[1]).read()
body = src[src.index('tb03_init[0x40] = {'):]
body = re.sub(r'/\*.*?\*/', '', body[body.index('{') + 1:body.index('};')])
vals = [float(x.rstrip('f')) for x in re.findall(r'-?[0-9.]+(?:e-?\d+)?f', body)]
assert len(vals) == 0x40
knobs = {int(w[1], 16) & 0x3F for w in (l.split() for l in open(sys.argv[2])) if w[:1] == ['param'] and w[-1] == 'knob'}
for i, v in enumerate(vals):
    if i not in knobs:
        print('C%03X %r' % (i, v))
PY
lift() {   # lift OUT.h NAMES CONSTS ARGS...: one run of dsplift (the TB-03 voice program)
    o=$1 n=$2 c=$3
    shift 3
    python3 "$ROOT/sdk/s1cc/tools/dsplift.py" "$tmp" --bq2 --kernel --import-ranges tb03_import_ranges.txt \
        --names "$n" --consts "$c" "$@" tb03_voice=../../../../work/tb03/progs/v_0022.hex@0x22
    { cat; tail -n +2 "$tmp"; } > "$HERE/$o"
    echo "make_voice.sh: $HERE/$o"
}
cd "$HERE"
# --split 415: instructions 0..414 on DSP A, 415..750 on DSP B (both fit the budgets);
# --zero-cells 0bd,0bf: two bank-0 cells part A reads that only part B writes, 0 in the TB-03 setup (README §2);
# --import-ranges: the measured range of every value A hands to B (README §2)
lift tb03_voice.h tb03_names.txt "$consts" --split 415 --zero-cells 0bd,0bf <<'H'
/* tb03_voice.h: the TB-03 voice program (BQ2:Appli 1.07, voice DSP 0x0022, 751 instructions) as C, in two parts
 * (tb03_voice_a: instructions 0..414 on DSP A, tb03_voice_b: 415..750 on DSP B), for the TB-03 setup: the
 * parameters no knob moves are the init's values (tb03_tables.h), names from tb03_names.txt. Written by
 * sdk/s1cc/tools/dsplift.py --bq2 --kernel from the firmware (tools/make_voice.sh); do not edit: README.md §2. */
H
