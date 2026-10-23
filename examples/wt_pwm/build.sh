#!/bin/sh
# Build the PLUG-OUT C-PWM: the DCO1 wavetable is six frames of "80's PWM.wav" (Echo Sound Works Core Tables,
# Analog), 128 points each, compiled into the DSP programs (prog/dco1_w0..5.txt, made with re/wav2dco.py
# --cycle 2048 --frame 0,6,12,18,24,30 -n 128). DCO2 keeps the six AKWF programs of the C-WAVE build.
#   examples/wt_pwm/build.sh          -> plugins/C-PWM
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
ENGINE=$ROOT/examples/from_scratch/engine
WT2=$ROOT/examples/from_scratch/re/dspexp/own/wt2
OWN=""
for i in 0 1 2 3 4 5; do
    OWN="$OWN,dco1_w$i=$HERE/prog/dco1_w$i.txt,dco2_w$i=$WT2/dco2_w$i.txt"
done
OWN=${OWN#,}
python3 "$ROOT/tools/check_optimized.py" "$HERE"/prog/dco1_w*.txt "$WT2"/dco2_w*.txt || true    # warnings: unoptimized microcode (docs/MICROCODE_OPTIMIZATION.md)
DSPTEST= WAVETABLE=1 NAME=C-PWM OWN_PROGS="$OWN" "$ENGINE/build.sh" "$ROOT/plugins/JX-3P" "$ROOT/plugins/C-PWM"
echo "built $ROOT/plugins/C-PWM"
