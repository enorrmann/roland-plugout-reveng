#!/bin/sh
# Build the dedicated wavetable PLUG-OUT C-WTLIVE (plugins/C-WTLIVE) from the from-scratch engine:
# DSPTEST + DSPLIVE, with the calibration test programs cal_d2/cal_d3 (re/dspexp/audio) in the DCO2
# slot of DSP2 and DSP3, so the DCO2 output is audible. DCO1 keeps its own saw. Only the test
# programs of the calibration batch are used; no other ISA batch is built.
#   examples/wt_live/build.sh
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
ENGINE=$ROOT/examples/from_scratch/engine
AUDIO=$ROOT/examples/from_scratch/re/dspexp/audio
python3 "$ROOT/tools/check_optimized.py" "$AUDIO"/cal_d2.txt "$AUDIO"/cal_d3.txt || true    # warnings: unoptimized microcode (docs/MICROCODE_OPTIMIZATION.md)
DSPTEST=1 DSPLIVE=1 MC_TEST="t0=$AUDIO/cal_d2.txt,t1=$AUDIO/cal_d3.txt" NAME=C-WTLIVE \
    "$ENGINE/build.sh" "$ROOT/plugins/JX-3P" "$ROOT/plugins/C-WTLIVE"
echo "built $ROOT/plugins/C-WTLIVE"
