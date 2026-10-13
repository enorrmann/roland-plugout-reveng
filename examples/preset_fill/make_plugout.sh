#!/bin/sh
# Build plugins/JX-3P-FILL: a stock JX-3P whose region 0 init hook (scanfill.c)
# locates the first preset's name in RAM and writes OS bytes over it.
#
#   examples/preset_fill/make_plugout.sh [OUT_DIR]
# Environment (see build.sh): LO HI SRC LEN MODE REPORT, e.g.  MODE=1 make_plugout.sh
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
OUT=${1:-$ROOT/plugins/JX-3P-FILL}
B="python3 $ROOT/tools/plugout_build.py"
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

"$HERE/build.sh"
$B unpack "$ROOT/plugins/JX-3P/Script/Code8_1.Dat" "$WORK/p" \
    --relocs-from "$ROOT/plugins/JX-3P/Script/Code8_2.Dat" >/dev/null
$B set-id "$WORK/p" --name "JX-3P FILL" --version 1.06
# the hook goes into the zero area at image + 0x100 (slot 1 address 0x61800100)
python3 - "$WORK/p/region0.bin" "$HERE/hook.bin" <<'PY'
import sys
r = open(sys.argv[1], "rb").read()
n = len(open(sys.argv[2], "rb").read())
assert not any(r[0x100:0x100 + n]), "cave at 0x100 is not empty"
PY
$B patch "$WORK/p" --region 0 --at 0x61800100 --file "$HERE/hook.bin"
$B patch "$WORK/p" --region 0 --at 0x61800024 --hex 01018061 --reloc 0x61800024
$B plugin "$WORK/p" --template "$ROOT/plugins/JX-3P-MEM" -o "$OUT" >/dev/null
echo "plugin: $OUT  (install with run_test.py or plugout_tool.py install --slot N)"
