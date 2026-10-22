#!/bin/sh
# solov2 plug-out = engine/build.sh with solov2/engine_solov2.c: four SAW voices per slave, oscillators in DSP2 / DSP3
# (DCO1 slot), gate mix in DSP4 main, as Roland's layout (make_solov2.py). engine/ is not modified.
#   examples/from_scratch/solov2/build_solov2.sh [src-plugin-dir] [out-plugin-dir]
set -e
SOLOV2=$(cd "$(dirname "$0")" && pwd)
FS=$SOLOV2/..
ENG=$FS/engine
NAME=${NAME:-C-SOLOV2}
export NAME
python3 "$SOLOV2/make_solov2.py" > /dev/null
sed -e "s|^HERE=.*|HERE=$ENG|" -e "s|\"\$HERE/engine.c\"|\"$SOLOV2/engine_solov2.c\"|" -e "s|arm-none-eabi-gcc \$CFLAGS -c|arm-none-eabi-gcc \$CFLAGS \$EXTRA_DEFS -c|" "$ENG/build.sh" > "$SOLOV2/build_engine.sh"
OWN_PROGS=${OWN_PROGS:-solov2osc=$SOLOV2/solov2_osc.hex,solov2osc2=$SOLOV2/solov2_osc2.hex,solov2gate=$SOLOV2/solov2_gate.hex,dco2null=$FS/gate/dco2_null.hex} sh "$SOLOV2/build_engine.sh" "$@"
