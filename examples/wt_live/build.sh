#!/bin/sh
# Build the dedicated wavetable PLUG-OUT C-WTLIVE (plugins/C-WTLIVE): the from-scratch engine with
# DSPTEST + DSPLIVE and only the silent test program mute.txt in the DCO2 slot. It is not the lab
# plugin C-DSPT: nothing here builds or installs the ISA test batches.
#   examples/wt_live/build.sh
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
ENGINE=$ROOT/examples/from_scratch/engine
DSPTEST=1 DSPLIVE=1 MC_TEST="t0=$HERE/mute.txt,t1=$HERE/mute.txt" NAME=C-WTLIVE \
    "$ENGINE/build.sh" "$ROOT/plugins/JX-3P" "$ROOT/plugins/C-WTLIVE"
echo "built $ROOT/plugins/C-WTLIVE"
