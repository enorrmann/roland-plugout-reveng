#!/bin/sh
# Minimal from-scratch PLUG-OUT = engine/build.sh with mini/engine_mini.c (engine.c with our DSP5 voice) instead of
# engine.c. The script is generated from engine/build.sh (engine.c and its build are not modified).
#   examples/from_scratch/mini/build.sh [out-plugin-dir]
set -e
MINI=$(cd "$(dirname "$0")" && pwd)
ENG=$MINI/../engine
NAME=${NAME:-C-MINI2}
export NAME
python3 "$MINI/../dsp/make_poly.py" > /dev/null
python3 "$MINI/../../../tools/check_optimized.py" "$MINI"/../dsp/poly.hex || true    # warnings: unoptimized microcode (docs/MICROCODE_OPTIMIZATION.md)
cp "$MINI/../dsp/poly.hex" "$MINI/../re/dspexp/own/voice.txt"
sed -e "s|^HERE=.*|HERE=$ENG|" -e "s|\"\$HERE/engine.c\"|\"$MINI/engine_mini.c\"|" -e "s|arm-none-eabi-gcc \$CFLAGS -c|arm-none-eabi-gcc \$CFLAGS \$EXTRA_DEFS -c|" "$ENG/build.sh" > "$MINI/build_engine.sh"
OWN_PROGS=${OWN_PROGS:-voice=voice.txt} sh "$MINI/build_engine.sh" "$@"
