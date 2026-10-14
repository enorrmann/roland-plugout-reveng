#!/bin/sh
# Build the JX-3P-MIDI-SEND PLUG-OUT: stock JX-3P with the probe hook in the
# engine (region 1). Region 0 is left untouched (the name stays readable; we do
# NOT rely on the name for liveness -- it is only writable at init). On every
# panel move the probe emits one SysEx per router bit 0..NBITS-1.
#
#   [NBITS=6] [NAME="MIDI SEND"] examples/midi_send/make_plugout.sh [OUT_DIR]
#
# Then install in slot 3 and capture the ports with run_probe.py.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
OUT=${1:-$ROOT/plugins/JX-3P-MIDI-SEND}
B="python3 $ROOT/tools/plugout_build.py"
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

echo "== 1. assemble probe (NBITS=${NBITS:-6})"
"$HERE/build.sh" >/dev/null

echo "== 2. unpack stock JX-3P (slot 1 addresses; relocs from the slot 2 build)"
$B unpack "$ROOT/plugins/JX-3P/Script/Code8_1.Dat" "$WORK/p" \
    --relocs-from "$ROOT/plugins/JX-3P/Script/Code8_2.Dat" >/dev/null

echo "== 3. identity"
$B set-id "$WORK/p" --name "${NAME:-MIDI SEND}"

echo "== 4. region 1: probe hook in the cave, sender entry -> B.W press"
$B patch "$WORK/p" --region 1 --at 0x60a07000 --file "$HERE/probe.bin"
$B patch "$WORK/p" --region 1 --at 0x60a0b41e --file "$HERE/sender.bin"

echo "== 5. plugin folder -> $OUT (slots 1..3)"
$B plugin "$WORK/p" --template "$ROOT/plugins/JX-3P-MEM" -o "$OUT"
echo "done. Install + capture with:"
echo "  python3 examples/midi_send/run_probe.py --slot 3"
