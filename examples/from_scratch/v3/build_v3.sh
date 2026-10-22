#!/bin/sh
# v3 plug-out = engine/build.sh with v3/engine_v3.c: our own main programs (make_v3.py) in Roland's voice layout, our saw
# in the DCO2 slots of DSP2 / DSP3, our null program in the DCO1 slots. No Roland microcode is copied (checked by
# the signature test of docs/VOICE_ALLOCATION.md); engine/ is not modified.
#   examples/from_scratch/v3/build_v3.sh [src-plugin-dir] [out-plugin-dir]
set -e
V3=$(cd "$(dirname "$0")" && pwd)
FS=$V3/..
ENG=$FS/engine
NAME=${NAME:-C-V3}
export NAME
python3 "$V3/make_v3.py" > /dev/null
sed -e "s|^HERE=.*|HERE=$ENG|" -e "s|\"\$HERE/engine.c\"|\"$V3/engine_v3.c\"|" -e "s|arm-none-eabi-gcc \$CFLAGS -c|arm-none-eabi-gcc \$CFLAGS \$EXTRA_DEFS -c|" "$ENG/build.sh" > "$V3/build_engine.sh"
OWN_PROGS=${OWN_PROGS:-v3m0=$V3/v3m0.hex,v3m1=$V3/v3m1.hex,v3m2=$V3/v3m2.hex,v3m3=$V3/v3m3.hex,v3m4=$V3/v3m4.hex,v3m5=$V3/v3m5.hex,v3saw=$V3/v3saw.hex,dco2null=$FS/gate/dco2_null.hex} sh "$V3/build_engine.sh" "$@"
