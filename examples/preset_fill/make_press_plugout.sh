#!/bin/sh
# Build plugins/JX-3P-PRESS: stock JX-3P + the engine sender hook (press.s).
#   examples/preset_fill/make_press_plugout.sh [OUT_DIR]     (env: DEST SRC LEN MODE)
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
OUT=${1:-$ROOT/plugins/JX-3P-PRESS}
B="python3 $ROOT/tools/plugout_build.py"
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

"$HERE/build_press.sh"
$B unpack "$ROOT/plugins/JX-3P/Script/Code8_1.Dat" "$WORK/p" \
    --relocs-from "$ROOT/plugins/JX-3P/Script/Code8_2.Dat" >/dev/null
$B set-id "$WORK/p" --name "JX-3P PRESS" --version 1.06
python3 - "$WORK/p/region1.bin" "$HERE/press.bin" <<'PY'
import sys
r = open(sys.argv[1], "rb").read()
n = len(open(sys.argv[2], "rb").read())
assert not any(r[0x7000:0x7000 + n]), "cave at 0x7000 is not empty"
PY
$B patch "$WORK/p" --region 1 --at 0x60a07000 --file "$HERE/press.bin"
$B patch "$WORK/p" --region 1 --at 0x60a0b41e --file "$HERE/sender.bin"
$B plugin "$WORK/p" --template "$ROOT/plugins/JX-3P-MEM" -o "$OUT" >/dev/null
echo "plugin: $OUT"
