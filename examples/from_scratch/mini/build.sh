#!/bin/sh
# Minimal from-scratch PLUG-OUT = engine/build.sh with mini/engine_mini.c (engine.c with our DSP5 voice) instead of
# engine.c. The script is generated from engine/build.sh (engine.c and its build are not modified).
#   examples/from_scratch/mini/build.sh [out-plugin-dir]
#   OPT=1 NAME=C-MINI2O ...: the optimized voice poly_opt.hex (ph_wrap + asm/dspsched.py: 88 instead of 210
#                            instructions; same pitch and level in the emulator, emu/test_core.py test_s8_poly_opt;
#                            not yet tried on the device)
set -e
MINI=$(cd "$(dirname "$0")" && pwd)
ENG=$MINI/../engine
NAME=${NAME:-C-MINI2}
export NAME
if [ "${OPT:-0}" = 1 ]; then VOICE=$MINI/../dsp/poly_opt.hex; else VOICE=$MINI/../dsp/poly.hex; fi
python3 "$MINI/../dsp/make_poly.py" --opt > /dev/null
python3 "$MINI/../../../tools/check_optimized.py" "$VOICE" || true    # warnings: unoptimized microcode (docs/MICROCODE_OPTIMIZATION.md)
cp "$VOICE" "$MINI/../re/dspexp/own/voice.txt"
sed -e "s|^HERE=.*|HERE=$ENG|" -e "s|\"\$HERE/engine.c\"|\"$MINI/engine_mini.c\"|" -e "s|arm-none-eabi-gcc \$CFLAGS -c|arm-none-eabi-gcc \$CFLAGS \$EXTRA_DEFS -c|" "$ENG/build.sh" > "$MINI/build_engine.sh"
OWN_PROGS=${OWN_PROGS:-voice=voice.txt} sh "$MINI/build_engine.sh" "$@"
