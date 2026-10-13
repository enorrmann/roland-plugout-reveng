#!/bin/sh
# Build the JX-3P-MEM-STEP PLUG-OUT: a stock JX-3P with two hooks.
#
#   region 0 (model)   SYNINFO init  -> hook.s   : at init, name := hex of 7 bytes at SRC
#   region 1 (engine)  param sender  -> press.s  : every panel parameter change
#                                                  advances the address by 7 and
#                                                  rewrites the name
#
# Usage:   examples/mem_name/make_plugout.sh [SRC] [OUT_DIR]
#   SRC      first address to read (default 0x60000000, start of the OS image)
#   OUT_DIR  plugin folder to create (default plugins/JX-3P-MEM-STEP)
#
# Needs: arm-none-eabi-as/ld/objcopy, python3. Then install in slot 3:
#   python3 tools/plugout_tool.py install plugins/JX-3P-MEM-STEP --slot 3 --dry-run
#   python3 tools/plugout_tool.py install plugins/JX-3P-MEM-STEP --slot 3
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
export SRC=${1:-0x60000000}
OUT=${2:-$ROOT/plugins/JX-3P-MEM-STEP}
B="python3 $ROOT/tools/plugout_build.py"
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

echo "== 1. assemble hooks (SRC=$SRC)"
"$HERE/build.sh" >/dev/null            # hook.bin   region 0 init hook
"$HERE/engine_build.sh" >/dev/null     # press.bin  engine hook, sender.bin  B.W

echo "== 2. unpack stock JX-3P (slot 1 addresses; relocs from the slot 2 build)"
$B unpack "$ROOT/plugins/JX-3P/Script/Code8_1.Dat" "$WORK/p" \
    --relocs-from "$ROOT/plugins/JX-3P/Script/Code8_2.Dat" >/dev/null

echo "== 3. identity"
$B set-id "$WORK/p" --name "JX-3P MEM STEP"

echo "== 4. region 0: init hook in the zero area, SYNINFO init -> hook"
$B patch "$WORK/p" --region 0 --at 0x61800100 --file "$HERE/hook.bin"
$B patch "$WORK/p" --region 0 --at 0x61800024 --hex 01018061 --reloc 0x61800024

echo "== 5. region 1: press hook in the cave, sender entry -> B.W press"
$B patch "$WORK/p" --region 1 --at 0x60a07000 --file "$HERE/press.bin"
$B patch "$WORK/p" --region 1 --at 0x60a0b41e --file "$HERE/sender.bin"

echo "== 6. plugin folder -> $OUT (slots 1..3)"
$B plugin "$WORK/p" --template "$ROOT/plugins/JX-3P-MEM" -o "$OUT"
echo "done. Install with: python3 tools/plugout_tool.py install $OUT --slot 3"
