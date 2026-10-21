#!/bin/sh
# Gate plug-out = engine/build.sh with gate/engine_gate.c instead of engine.c. Programs: DSP2 SAW and DSP4 gate
# (dsp/make_chain.py -> dsp/dsp2_osc.hex, dsp/dsp4_gate.hex). engine/ is not modified.
#   examples/from_scratch/gate/build.sh [src-plugin-dir] [out-plugin-dir]
set -e
GATE=$(cd "$(dirname "$0")" && pwd)
ENG=$GATE/../engine
NAME=${NAME:-C-GATE}
export NAME
python3 "$GATE/../dsp/make_chain.py" > /dev/null
sed -e "s|^HERE=.*|HERE=$ENG|" -e "s|\"\$HERE/engine.c\"|\"$GATE/engine_gate.c\"|" -e "s|arm-none-eabi-gcc \$CFLAGS -c|arm-none-eabi-gcc \$CFLAGS \$EXTRA_DEFS -c|" "$ENG/build.sh" > "$GATE/build_engine.sh"
OWN_PROGS=${OWN_PROGS:-osc=$GATE/../dsp/dsp2_osc.hex,gate=$GATE/../dsp/dsp4_gate.hex,dco2null=$GATE/dco2_null.hex} sh "$GATE/build_engine.sh" "$@"
