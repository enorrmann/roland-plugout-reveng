#!/bin/sh
# Build the from-scratch engine into a plugin folder.
#   examples/from_scratch/engine/build.sh [plugins/JX-3P] [out-plugin-dir]
# The model (region 0) is still the stock JX-3P one; region 1 is ours.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../../.." && pwd)
SRC_PLUGIN=${1:-$ROOT/plugins/JX-3P}
OUT_PLUGIN=${2:-$ROOT/plugins/C-SAW}
B=$HERE/build
mkdir -p "$B"
python3 "$HERE/gen_tables.py" "$SRC_PLUGIN" "$B"
CFLAGS="-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -ffreestanding \
 -fno-builtin -nostdlib -fno-zero-initialized-in-bss -fno-common -Wall -Wextra -I$B"
arm-none-eabi-gcc $CFLAGS -c "$HERE/engine.c" -o "$B/engine.o"
# link at the slot 1 and slot 2 run addresses; the words that move are the relocations
for base in 0x60A00000 0x60C00000; do
    arm-none-eabi-ld -T "$HERE/link.ld" --defsym BASE=$base "$B/engine.o" -o "$B/engine_$base.elf"
    arm-none-eabi-objcopy -O binary "$B/engine_$base.elf" "$B/engine_$base.bin"
done
arm-none-eabi-size "$B/engine_0x60A00000.elf"
python3 - "$B" <<'PY'
import json, os, shutil, struct, sys
b = sys.argv[1]
a = open(os.path.join(b, 'engine_0x60A00000.bin'), 'rb').read()
c = open(os.path.join(b, 'engine_0x60C00000.bin'), 'rb').read()
assert len(a) == len(c)
relocs = []
for o in range(0, len(a) - 3, 4):
    x, y = struct.unpack_from('<I', a, o)[0], struct.unpack_from('<I', c, o)[0]
    if x != y:
        assert (y - x) & 0xffffffff == 0x200000, (hex(o), hex(x), hex(y))
        relocs.append(o)
diff = [o for o in range(len(a)) if a[o] != c[o] and o - o % 4 not in relocs]
assert not diff, diff[:10]
proj = os.path.join(b, 'projC')
if os.path.exists(proj):
    shutil.rmtree(proj)
shutil.copytree(os.path.join(b, 'proj'), proj)
open(os.path.join(proj, 'region1.bin'), 'wb').write(a)
r = json.load(open(os.path.join(proj, 'relocs.json')))
r['1'] = relocs
json.dump(r, open(os.path.join(proj, 'relocs.json'), 'w'))
print(f'region 1: {len(a)} bytes, {len(relocs)} relocations')
PY
BT="python3 $ROOT/tools/plugout_build.py"
$BT set-id "$B/projC" --name "C-SAW" --version 0.01
$BT show "$B/projC"
$BT plugin "$B/projC" --template "$SRC_PLUGIN" -o "$OUT_PLUGIN"
