#!/bin/sh
# Roland-free test plug-out: engine_solo.c (mini's engine without Roland programs) + our oscillator with gate on DSP4
# (dsp/make_solo.py) + our null DCO programs on DSP2/3 (dco2_null.hex).
#   NAME=C-SOLO examples/from_scratch/gate/build_solo.sh
set -e
GATE=$(cd "$(dirname "$0")" && pwd)
FS=$GATE/..
ENG=$FS/engine
NAME=${NAME:-C-SOLO}
export NAME
python3 "$FS/dsp/make_solo.py" > /dev/null
sed -e "s|^HERE=.*|HERE=$ENG|" -e "s|\"\$HERE/engine.c\"|\"$GATE/engine_solo.c\"|" -e "s|arm-none-eabi-gcc \$CFLAGS -c|arm-none-eabi-gcc \$CFLAGS \$EXTRA_DEFS -c|" "$ENG/build.sh" > "$GATE/build_solo_engine.sh"
OWN_PROGS=solo=$FS/dsp/solo.hex,dco2null=$GATE/dco2_null.hex sh "$GATE/build_solo_engine.sh" "$@"
