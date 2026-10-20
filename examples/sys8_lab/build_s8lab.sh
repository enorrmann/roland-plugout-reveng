#!/bin/sh
# Build the SYSTEM-8 lab PLUG-OUT (plugins/S8LAB) from the stock SYSTEM-8 v1.32 firmware, and install it in PLG1.
#   examples/sys8_lab/build_s8lab.sh          # build + check against the firmware (no hardware)
#   examples/sys8_lab/build_s8lab.sh install  # also send it to slot 1 (replaces what is there)
#
# Source: work/sys8_proj (regions p0m = region 0, p0s = region 1, unpacked from firmware/sys8_sys_v132 with
# tools/sys8fw.py). The only change is the identity: name S8LAB, model id 0x20, version 0.01.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
PB="python3 $ROOT/tools/plugout_build.py"
SRC=$ROOT/work/sys8_proj
PROJ=$ROOT/work/s8lab_proj
OUT=$ROOT/plugins/S8LAB

# 1. the regions must be the stock v1.32 ones
cmp "$SRC/region0.bin" "$ROOT/work/sys8_v132/p1y01_p0m.region0.bin"
cmp "$SRC/region1.bin" "$ROOT/work/sys8_v132/p1y01_p0s.region0.bin"

# 2. identity + pack
rm -rf "$PROJ"
cp -r "$SRC" "$PROJ"
$PB set-id "$PROJ" --name S8LAB --model-id 32 --version 0.01
mkdir -p "$OUT/Script"
$PB pack "$PROJ" --slot 1 -o "$OUT/Script/Code8_1.Dat" | tail -1
echo "built $OUT/Script/Code8_1.Dat"

# 3. install (optional): firmware only, the patches already in PLG1 stay
if [ "$1" = install ]; then
    python3 "$ROOT/tools/plugout_tool.py" install "$OUT" --slot 1 --no-patches
    python3 "$ROOT/tools/plugout_tool.py" info
fi
