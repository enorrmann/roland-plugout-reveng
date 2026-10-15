#!/bin/sh
# Build region 0 (the model) in C.
#   examples/from_scratch/model/build.sh   -> model/build/region0_<base>.bin, relocs0.json
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
B=$HERE/build
mkdir -p "$B"
python3 "$HERE/params.py" "$B"
CFLAGS="-mcpu=cortex-m4 -mthumb -mfloat-abi=soft -Os -ffreestanding -fno-builtin -nostdlib \
 -fno-zero-initialized-in-bss -fno-common -Wall -Wextra -I$B ${PEEK:+-DPEEK}"
arm-none-eabi-gcc $CFLAGS -c "$HERE/model.c" -o "$B/model.o"
arm-none-eabi-gcc $CFLAGS -c "$HERE/storage.S" -Wa,-I"$B" -o "$B/storage.o"
# link at the slot 1 and slot 2 run addresses; the words that move are the relocations
for base in 0x61800000 0x61A00000; do
    arm-none-eabi-ld -T "$HERE/link.ld" --defsym BASE=$base "$B/model.o" "$B/storage.o" \
        -o "$B/model_$base.elf"
    arm-none-eabi-objcopy -O binary "$B/model_$base.elf" "$B/region0_$base.bin"
done
arm-none-eabi-size "$B/model_0x61800000.elf"
python3 - "$B" <<'PY'
import json, os, struct, sys
b = sys.argv[1]
a = open(os.path.join(b, 'region0_0x61800000.bin'), 'rb').read()
c = open(os.path.join(b, 'region0_0x61A00000.bin'), 'rb').read()
assert len(a) == len(c)
relocs = []
for o in range(0, len(a) - 3, 4):
    x, y = struct.unpack_from('<I', a, o)[0], struct.unpack_from('<I', c, o)[0]
    if x != y:
        assert (y - x) & 0xffffffff == 0x200000, (hex(o), hex(x), hex(y))
        relocs.append(o)
diff = [o for o in range(len(a)) if a[o] != c[o] and o - o % 4 not in relocs]
assert not diff, diff[:10]
json.dump(relocs, open(os.path.join(b, 'relocs0.json'), 'w'))
print(f'region 0: {len(a)} bytes, {len(relocs)} relocations')
PY
