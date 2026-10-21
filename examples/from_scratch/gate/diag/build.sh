#!/bin/sh
# Diagnostic build: mini/engine_mini.c (the one that plays) + a constant on DSP4 DCO1 (engine_d4.c).
#   NAME=C-D4 examples/from_scratch/gate/diag/build.sh
set -e
DIAG=$(cd "$(dirname "$0")" && pwd)
FS=$DIAG/../..
ENG=$FS/engine
NAME=${NAME:-C-D4}
export NAME
sed -e "s|^HERE=.*|HERE=$ENG|" -e "s|\"\$HERE/engine.c\"|\"${ENGINE:-$DIAG/engine_d4.c}\"|" -e "s|arm-none-eabi-gcc \$CFLAGS -c|arm-none-eabi-gcc \$CFLAGS \$EXTRA_DEFS -c|" "$ENG/build.sh" > "$DIAG/build_engine.sh"
OWN_PROGS=voice=$FS/dsp/poly.hex,const4=$DIAG/const4.hex,osc=$FS/dsp/dsp2_osc.hex,dco2null=$FS/gate/dco2_null.hex sh "$DIAG/build_engine.sh" "$@"
