#!/bin/sh
# Slave-1 output search (docs/log/WAVE_BUS_LOG.md §4q): mini's engine with a fixed drone per slave (440 Hz slave 0,
# 660 Hz slave 1) and our voice written to every IOMD word 6000-601F (make_voice.py, DRONE + ALLOUT).
#   NAME=C-S1W examples/from_scratch/gate/diag/build_s1w.sh
set -e
DIAG=$(cd "$(dirname "$0")" && pwd)
FS=$DIAG/../..
ENG=$FS/engine
NAME=${NAME:-C-S1W}
export NAME
(cd "$FS/dsp" && DRONE=1 ALLOUT=6000-601F python3 make_voice.py > /dev/null)
cp "$FS/dsp/voice.hex" "$FS/re/dspexp/own/voice.txt"
sed -e "s|^HERE=.*|HERE=$ENG|" -e "s|\"\$HERE/engine.c\"|\"$DIAG/engine_s1w.c\"|" -e "s|arm-none-eabi-gcc \$CFLAGS -c|arm-none-eabi-gcc \$CFLAGS \$EXTRA_DEFS -c|" "$ENG/build.sh" > "$DIAG/build_s1w_engine.sh"
OWN_PROGS=voice=$FS/re/dspexp/own/voice.txt sh "$DIAG/build_s1w_engine.sh" "$@"
