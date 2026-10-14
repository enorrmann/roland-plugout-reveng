#!/bin/sh
# Diagnostic hybrid (see hybrid.c): JX-3P engine + ours, chosen per slave.
#   examples/from_scratch/engine/hybrid.sh ["{load,start,mute,refresh,notes,params}"]
# default CFG1 = our note handling only. Run build.sh first (tables, proj).
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../../.." && pwd)
B=$HERE/build
CFG=${1:-"{0,0,0,0,1,0}"}
CFLAGS="-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -ffreestanding \
 -fno-builtin -nostdlib -fno-zero-initialized-in-bss -fno-common -Wall -I$B -I$HERE"
for jx in 0x60A00000 0x60C00000; do
    base=$(printf '0x%X' $((jx + 0x60000)))
    arm-none-eabi-gcc $CFLAGS -DJXBASE=$jx "-DCFG1=$CFG" -c "$HERE/hybrid.c" -o "$B/hyb_$jx.o"
    arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -c "$HERE/fwd.S" -o "$B/fwd.o"
    arm-none-eabi-ld -T "$HERE/link.ld" --defsym BASE=$base "$B/hyb_$jx.o" "$B/fwd.o" -o "$B/hyb_$jx.elf"
    arm-none-eabi-objcopy -O binary "$B/hyb_$jx.elf" "$B/hyb_$jx.bin"
done
INIT=$(arm-none-eabi-nm "$B/hyb_0x60A00000.elf" | awk '$3=="hyb_init"{print $1}')
python3 - "$B" "$INIT" <<'PY'
import json, os, shutil, struct, sys
b, init = sys.argv[1], int(sys.argv[2], 16) | 1
a = open(os.path.join(b, 'hyb_0x60A00000.bin'), 'rb').read()
c = open(os.path.join(b, 'hyb_0x60C00000.bin'), 'rb').read()
ours = []
for o in range(0, len(a) - 3, 4):
    x, y = struct.unpack_from('<I', a, o)[0], struct.unpack_from('<I', c, o)[0]
    if x != y:
        assert (y - x) & 0xffffffff == 0x200000, (hex(o), hex(x), hex(y))
        ours.append(o)
jx = bytearray(open(os.path.join(b, 'proj', 'region1.bin'), 'rb').read())
assert len(jx) <= 0x54E40 <= 0x60000
img = jx + bytes(0x60000 - len(jx)) + a
struct.pack_into('<I', img, 0x24, init)           # SYNINFO init -> hyb_init
proj = os.path.join(b, 'projH')
if os.path.exists(proj):
    shutil.rmtree(proj)
shutil.copytree(os.path.join(b, 'proj'), proj)
open(os.path.join(proj, 'region1.bin'), 'wb').write(img)
r = json.load(open(os.path.join(proj, 'relocs.json')))
assert 0x24 in r['1']
r['1'] = sorted(r['1'] + [0x60000 + o for o in ours])
json.dump(r, open(os.path.join(proj, 'relocs.json'), 'w'))
print(f'hybrid region 1: {len(img)} bytes, init {init:#x}, {len(ours)} + JX relocations')
PY
BT="python3 $ROOT/tools/plugout_build.py"
$BT set-id "$B/projH" --name "C-HYB" --version 0.01 >/dev/null
$BT plugin "$B/projH" --template "$ROOT/plugins/JX-3P" -o "$ROOT/plugins/C-HYB" | tail -1
