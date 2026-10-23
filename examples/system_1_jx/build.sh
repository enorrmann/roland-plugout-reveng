#!/bin/sh
# C-S1JX: the JX-3P's own DSP microcode (one DCO, 4 voices) on the SYSTEM-1 (docs/examples_system_1_jx_README.md).
#   examples/system_1_jx/build.sh            -> plugins/C-S1JX/Script/Code1.Dat (and Code.Dat, the same file)
#   SLAVE_TEST=1 NAME=C-S1JXS ...             diagnostic: JX-3P on the slave only, minimal master program (model.c)
#   RUNTIME_MODEL=1 ...                       the parameter model evaluated on the ARM (soft float) instead of the table
#   V4_PROGS=1 NAME=C-S1JX4 ...              diagnostic: C-S1V4's DSP programs with our ARM code (model.c, engine.c)
#   NO_MAIN1=1 ...                           diagnostic: slave B main = an empty program instead of MAIN1
#   NOJUMP=UNIT[,UNIT] ...                   diagnostic: the unit's jumps neutralized (s1jx.nojump)
#   TRUNC=UNIT:lo:hi ...                     diagnostic: only instructions lo..hi-1 of a unit (s1jx.truncate)
#   NO_STATE=1 ...                           diagnostic: no start-up writes
#   SEC_A_EMPTY=1 ...                        diagnostic: an empty program of ours in the slave A section
#   NO_MAIN5=1 ...                           diagnostic: slave A main = an empty program instead of MAIN5
#   NO_SEC_A=1 ...                           diagnostic: slave A section (MAIN3) left off
#   NO_SEC_B=1 ...                            diagnostic: slave B section (DCO1 of voices 2/3) left off (engine.c)
# Needs plugins/JX-3P (the microcode is read from it at build time; build/ and the plugin folder hold Roland bytes:
# never commit them), arm-none-eabi-gcc/ld/objcopy and Python 3. Run test_s1jx.py before installing.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
S1=$ROOT/examples/system_1
B=$HERE/build
NAME=${NAME:-C-S1JX}
MODEL_ID=${MODEL_ID:-0x0010}
mkdir -p "$B"
python3 "$HERE/s1jx.py"
[ -n "$V4_PROGS" ] && python3 "$S1/make_s1.py" >/dev/null
CFLAGS="-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -Os -ffreestanding -fno-builtin -nostdlib \
 -fno-zero-initialized-in-bss -fno-common -Wall -Wextra -I$HERE -I$B -I$S1 -DS1_NAME=\"$NAME\" -DS1_MODEL_ID=$MODEL_ID ${SLAVE_TEST:+-DJX_SLAVE_TEST} ${NO_SEC_B:+-DJX_NO_SEC_B} ${RUNTIME_MODEL:+-DJX_RUNTIME_MODEL} ${V4_PROGS:+-DJX_V4_PROGS} ${NO_STATE:+-DJX_NO_STATE} ${NO_MAIN1:+-DJX_NO_MAIN1} ${NO_SEC_A:+-DJX_NO_SEC_A} ${NO_MAIN5:+-DJX_NO_MAIN5} ${SEC_A_EMPTY:+-DJX_SEC_A_EMPTY}"
LIBGCC=$(arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -print-libgcc-file-name)
for r in 0 1; do
    if [ $r = 0 ]; then SRC=model.c; BASE=0x60800000; CODE=0x9000; else SRC=engine.c; BASE=0x60200000; CODE=0x1000; fi
    arm-none-eabi-gcc $CFLAGS -c "$HERE/$SRC" -o "$B/region$r.o"
    arm-none-eabi-ld -T "$S1/link.ld" --defsym BASE=$BASE --defsym CODE=$CODE "$B/region$r.o" "$LIBGCC" \
        -o "$B/region$r.elf"
    arm-none-eabi-objcopy -O binary "$B/region$r.elf" "$B/region$r.bin"
    arm-none-eabi-size "$B/region$r.elf"
done
python3 - "$B" "$NAME" "$MODEL_ID" <<'PY'
import json, os, struct, sys
b, name, mid = sys.argv[1], sys.argv[2], int(sys.argv[3], 0)
for r, base in ((0, 0x60800000), (1, 0x60200000)):
    img = open(os.path.join(b, f'region{r}.bin'), 'rb').read()
    assert img[:16] == b'SYSTEM1_SYNINFO\0' and img[0x1B4:0x1C4] == b'SYSTEM1_SYNINFO\0', 'SYNINFO'
    for off in (0x24, 0x28, 0x2C, 0x30):
        p = struct.unpack_from('<I', img, off)[0]
        assert p == 0 or (p & 1 and base <= p < base + len(img)), (r, hex(off), hex(p))
    print(f'region {r}: {len(img)} bytes, SYNINFO ok')
proj = os.path.join(b, 'proj')
os.makedirs(proj, exist_ok=True)
for r in (0, 1):
    os.replace(os.path.join(b, f'region{r}.bin'), os.path.join(proj, f'region{r}.bin'))
region = lambda r, load: {"file": f"region{r}.bin", "stamp": "26-1022_00:00:00", "magic": "SYSTEM-1:Plugout",
                          "tool": "0.015370", "load": load, "entry": load}
meta = {"format": 1, "device": "SYSTEM-1", "slot": 1, "name": name, "model_id": mid, "version": "0.01",
        "records": 2048, "regions": [region(0, "0x60800000"), region(1, "0x60200000")]}
json.dump(meta, open(os.path.join(proj, 'plugout.json'), 'w'), indent=2)
PY
OUT=$ROOT/plugins/$NAME/Script
mkdir -p "$OUT"
python3 "$ROOT/tools/plugout_build.py" pack "$B/proj" -o "$OUT/Code1.Dat"
cp "$OUT/Code1.Dat" "$OUT/Code.Dat"
python3 "$ROOT/tools/plugout_build.py" show "$OUT/Code1.Dat"
echo "plugin folder: $ROOT/plugins/$NAME"
