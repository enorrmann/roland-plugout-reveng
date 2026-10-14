#!/bin/sh
# Is 0x60445957 the cell behind RQ1 of NAME0 (preset 1, slot 3)?
#
#   1. send the JX-3P bank (all 64 presets: a partial send leaves the device in
#      "receiving") with preset 1 renamed "ZZZZ 1 [ZZZZ]" (patches only, no firmware)
#   2. RQ1 NAME0 must now read that name
#   3. read 7 bytes at DEST through the init-hook name channel
#
# Step 2 must already read "ZZZZ 1 [ZZZZ]" (otherwise the bank write did not
# change the leaf and the test says nothing). Expect the nibbles 05 0A 05 0A ...
# If the name still shows "String 1" nibbles (05 03 07 04 07 02 06) the host
# write lands in another cell and DEST is not the live preset name.
#
#   examples/mem_name/patch_cell_test.sh [slot] [dest] [plugin]
#   plugin: any JX-3P based folder (default JX-3P-CAVE-NAME). Step 1 uses
#           --skip-check: a MEM plug-out rewrites its own name with hex, so the
#           installer would never recognise the slot as holding it
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
SLOT=${1:-3}
DEST=${2:-0x60445957}
PLUGIN=${3:-$ROOT/plugins/JX-3P-CAVE-NAME}
BANK=$(mktemp /tmp/zzbank.XXXXXX)
trap 'rm -f "$BANK"' EXIT
python3 - "$ROOT/plugins/JX-3P/Patch/1 Preset.bin" "$BANK" <<'PY'
import sys
b = bytearray(open(sys.argv[1], "rb").read())
old, new = b"String 1 [Str1] ", b"ZZZZ 1 [ZZZZ]   "
nib = lambda t: b"".join(bytes([c >> 4, c & 15]) for c in t)
# the record name field and the NAME0 leaf (nibbles) are separate in the bank;
# the device serves the leaf
for a, z in ((old, new), (nib(old), nib(new))):
    i = b.index(a)
    assert b.count(a) == 1
    b[i:i + len(a)] = z
open(sys.argv[2], "wb").write(b)
PY
echo "== 1. send the bank with the renamed preset 1 to slot $SLOT"
python3 "$ROOT/tools/plugout_tool.py" install "$PLUGIN" --slot "$SLOT" \
    --patches-only --bank "$BANK" --skip-check --device-type 2 -y
sleep 5
echo "== 2. RQ1 NAME0"
python3 "$ROOT/tools/patch_read.py" --slot "$SLOT" --patches 0
echo "== 3. 7 bytes at $DEST (installs JX-3P-MEM in slot $SLOT, no patches)"
python3 "$ROOT/examples/mem_name/dump_loop.py" --slot "$SLOT" --start "$DEST" --count 7 \
    -o "$BANK.dump" --no-resume -y
xxd "$BANK.dump"; rm -f "$BANK.dump"
