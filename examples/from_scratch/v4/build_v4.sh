#!/bin/sh
# v4 plug-out = the v3 build (engine_v3.c, our saw and null slots) with the v4 main programs (make_v4.py) in place of
# v3m0..v3m5. Nothing of engine/ or v3/ is modified; no Roland microcode (emu/sigcheck.py).
#   examples/from_scratch/v4/build_v4.sh [src-plugin-dir] [out-plugin-dir]
#   V4B=1 ...    the fallback without the bank-1 ring (v4bm*), NAME defaults to C-V4B
set -e
V4=$(cd "$(dirname "$0")" && pwd)
FS=$V4/..
if [ -n "$V4B" ]; then
    python3 "$V4/make_v4.py" --no-ring > /dev/null
    P=v4bm
    NAME=${NAME:-C-V4B}
else
    python3 "$V4/make_v4.py" > /dev/null
    P=v4m
    NAME=${NAME:-C-V4}
fi
export NAME
OWN_PROGS=v3m0=$V4/${P}0.hex,v3m1=$V4/${P}1.hex,v3m2=$V4/${P}2.hex,v3m3=$V4/${P}3.hex,v3m4=$V4/${P}4.hex,v3m5=$V4/${P}5.hex,v3saw=$FS/v3/v3saw.hex,dco2null=$FS/gate/dco2_null.hex \
    sh "$FS/v3/build_v3.sh" "$@"
