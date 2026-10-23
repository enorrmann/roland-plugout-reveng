#!/bin/sh
# Build the PLUG-OUT C-MORPH: one wavetable position on DCO1 that morphs through six frames of "80's PWM.wav"
# with a triangle LFO (frame 0 -> 5 -> 0). The table (438 words) is written into bank 5 of DSP2/DSP3 at start;
# the DCO1 program is the player of re/wt5.py (--scan). DCO2 keeps the six AKWF programs of C-WAVE.
#   examples/wt_morph/build.sh          -> plugins/C-MORPH
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
ENGINE=$ROOT/examples/from_scratch/engine
WT2=$ROOT/examples/from_scratch/re/dspexp/own/wt2
python3 "$HERE/make.py"
python3 "$ROOT/tools/check_optimized.py" "$HERE"/prog/morph_dco1.txt "$WT2"/dco2_w*.txt || true    # warnings: unoptimized microcode (docs/MICROCODE_OPTIMIZATION.md)
OWN=""
for i in 0 1 2 3 4 5; do
    OWN="$OWN,dco1_w$i=$HERE/prog/morph_dco1.txt,dco2_w$i=$WT2/dco2_w$i.txt"
done
OWN=${OWN#,}
MORPH=1 WAVETABLE=1 NAME=C-MORPH OWN_PROGS="$OWN" "$ENGINE/build.sh" "$ROOT/plugins/JX-3P" "$ROOT/plugins/C-MORPH"
echo "built $ROOT/plugins/C-MORPH"
