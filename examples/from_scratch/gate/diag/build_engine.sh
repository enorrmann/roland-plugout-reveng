#!/bin/sh
# Build the from-scratch engine into a plugin folder.
#   examples/from_scratch/engine/build.sh [plugins/JX-3P] [out-plugin-dir]
# Both regions are ours: region 0 from ../model (C), region 1 from engine.c.
# The JX-3P plugin folder only supplies the DSP microcode, copied by
# programs.py (static, SHA-256 checked); the parameter model is ours
# (model_spec.py -> gen_model.py). MODEL=jx: stock JX-3P region 0 and its
# Script.xml / Patch/ (for comparisons).
set -e
HERE=/home/emilio/git/roland-plugout-reveng/examples/from_scratch/gate/diag/../../engine
ROOT=$(cd "$HERE/../../.." && pwd)
SRC_PLUGIN=${1:-$ROOT/plugins/JX-3P}
NAME=${NAME:-C-SAW}
OUT_PLUGIN=${2:-$ROOT/plugins/$NAME}
B=$HERE/build
mkdir -p "$B"
MODEL=${MODEL:-c}
[ "$MODEL" = jx ] || "$HERE/../model/build.sh"
python3 "$HERE/programs.py" "$SRC_PLUGIN" "$B"
python3 "$HERE/gen_model.py" "$B"
if [ "$MODEL" = jx ] && [ ! -f "$B/proj/region0.bin" ]; then
    python3 "$ROOT/tools/plugout_build.py" unpack "$SRC_PLUGIN/Script/Code8_1.Dat" "$B/proj" \
        --relocs-from "$SRC_PLUGIN/Script/Code8_2.Dat"
fi
CFLAGS="-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -ffreestanding \
 -fno-builtin -nostdlib -fno-zero-initialized-in-bss -fno-common -Wall -Wextra -I$B"
if [ -n "$DSPTEST" ]; then CFLAGS="$CFLAGS -DDSPTEST"; fi
if [ -n "$DSPLIVE" ]; then CFLAGS="$CFLAGS -DDSPLIVE"; fi
if [ -n "$WAVETABLE" ]; then CFLAGS="$CFLAGS -DWAVETABLE"; fi
if [ -n "$MORPH" ]; then CFLAGS="$CFLAGS -DMORPH"; fi
arm-none-eabi-gcc $CFLAGS $EXTRA_DEFS -c "/home/emilio/git/roland-plugout-reveng/examples/from_scratch/gate/diag/engine_m4e.c" -o "$B/engine.o"
# link at the slot 1 and slot 2 run addresses; the words that move are the relocations
for base in 0x60A00000 0x60C00000; do
    arm-none-eabi-ld -T "$HERE/link.ld" --defsym BASE=$base "$B/engine.o" -o "$B/engine_$base.elf"
    arm-none-eabi-objcopy -O binary "$B/engine_$base.elf" "$B/engine_$base.bin"
done
arm-none-eabi-size "$B/engine_0x60A00000.elf"
python3 - "$B" "$MODEL" "$HERE/../model/build" "$NAME" <<'PY'
import json, os, shutil, struct, sys
b, model, mb, name = sys.argv[1:5]
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
if model == 'jx':
    shutil.copytree(os.path.join(b, 'proj'), proj)          # stock region 0 and metadata
    r = json.load(open(os.path.join(proj, 'relocs.json')))
else:
    # our own project: container metadata written here, both regions ours.
    # Region header fields: format magic, a build stamp in the stock
    # "YY-MMDD_HH:MM:SS" form, the container tool version the stock files carry,
    # and the load / entry words of the SYSTEM-8 format.
    os.makedirs(proj)
    def region(f, load):
        return {'file': f, 'stamp': '26-1014_00:00:00', 'magic': 'SYSTEM-8:Plugout',
                'tool': '0.0114FA', 'load': load, 'entry': load}
    json.dump({'format': 1, 'device': 'SYSTEM-8', 'slot': 1, 'name': name, 'model_id': 8,
               'version': '0.01', 'records': 4096,
               'regions': [region('region0.bin', '0x60800000'), region('region1.bin', '0x60200000')]},
              open(os.path.join(proj, 'plugout.json'), 'w'), indent=2)
    shutil.copy(os.path.join(mb, 'region0_0x61800000.bin'), os.path.join(proj, 'region0.bin'))
    r = {'0': json.load(open(os.path.join(mb, 'relocs0.json')))}
    print(f"region 0: ours ({len(r['0'])} relocations)")
open(os.path.join(proj, 'region1.bin'), 'wb').write(a)
r['1'] = relocs
json.dump(r, open(os.path.join(proj, 'relocs.json'), 'w'))
print(f'region 1: {len(a)} bytes, {len(relocs)} relocations')
PY
BT="python3 $ROOT/tools/plugout_build.py"
$BT set-id "$B/projC" --name "$NAME" --version 0.01
$BT show "$B/projC"
# MODEL=jx: Script.xml and the JX-3P preset banks for the patch transfer.
# Ours: only the firmware; the 64 INIT patches are in region 0.
rm -rf "$OUT_PLUGIN/Patch" "$OUT_PLUGIN/Script/Script.xml"   # left over from older builds
if [ "$MODEL" = jx ]; then
    $BT plugin "$B/projC" --template "$SRC_PLUGIN" -o "$OUT_PLUGIN"
else
    $BT plugin "$B/projC" -o "$OUT_PLUGIN"
fi
