#!/bin/sh
# SYSTEM-S1CC: a SYSTEM-1 PLUG-OUT that is only an s1cc kernel (docs/examples_system_s1cc_README.md). sh build.sh --help
# Needs arm-none-eabi-gcc/ld/objcopy and Python 3 with numpy. No Roland file is read.
usage() {
    cat <<'TXT'
usage: KERNEL=DIR [VAR=value ...] sh examples/system_s1cc/build.sh [--help]

Builds SYSTEM-S1CC for the SYSTEM-1 -> plugins/<NAME>/Script/Code1.Dat (and Code.Dat, the same file): the voice is
an s1cc kernel and nothing else (S1_DSP_A on the slave's DSP A, S1_DSP_B on its DSP B); the master only applies
VOLUME and sends the mix through the OS delay / reverb (docs/examples_system_s1cc_README.md).
Install: python3 tools/plugout_tool.py install plugins/<NAME> --slot 1 --force --no-patches -y
Then select the PLUG-OUT on the panel.

Panel: the kernel's S1_PARAM tables on their panel ids (sdk/s1cc/include/s1panel.h; row 128 until the control
moves); VOLUME, DELAY and REVERB as the OS does them. A kernel that calls s1_color also gets OSC1 / OSC2 COLOR with
COLOR MOD (manual, LFO, pitch env, filter env, amp env, sub osc) and the LFO SHAPE / RATE and PITCH ENV A / D
controls (docs/examples_system_s1cc_README.md §3.2); every other control does nothing.

Options (environment variables):
  KERNEL=DIR    required: an s1cc build directory (sdk/s1cc/s1cc build VOICE.c ... -o DIR)
  (the number of voices is not an option: the build takes the most, 8 down to 1, whose slave programs fit the
  measured limits, instructions per sample and the 6 KiB of the main program, and says how many at the end)
  BUDGET_A=n, BUDGET_B=n, MAIN_BYTES=n  the limits (default 703 / 708 instructions per sample, 6144 bytes: what ran
                on the device; past them it hung), only to try more at your own risk
  NAME=...      plug-out name shown on the device and plugins/ folder (default SYSTEM-S1CC); max 16 characters
  MODEL_ID=0x.. (default 0x0010), PARAMLOG=1 (model.c sends the parameters to the PC)
  TRY_FORMS=... op groups to test on the device before they go into asm/proven_forms.txt: 'all' or
                'mad:reg + mul:imm; ...' (the candidates: KERNEL=... python3 examples/system_s1cc/make_s1cc.py --forms);
                the build writes the ones it used to examples/system_s1cc/build/try_forms.txt; once the PLUG-OUT works
                on the device: python3 tools/proven_forms.py promote examples/system_s1cc/build/try_forms.txt --by NAME
  LFO_PITCH=0   the host LFO does not modulate the pitch / s1_dt / s1_rdt words (saves ~18 instructions per voice)
  LFO_LUT=1     the host LFO's shapes from bank-5 tables (sine, triangle, saw; square = clamp(256 saw)): ~3 more
                DSP B instructions per voice; the sine is a true sine (docs/examples_system_s1cc_README.md §3.6)
  ENV_CURVE=sq|cube|sqrt|smooth|lin   one shared curve table for every host envelope, level = C(ADSR): ~7 DSP B
                instructions per envelope

Examples:
  sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_env.c
  KERNEL=sdk/s1cc/build/simple_saw_env sh examples/system_s1cc/build.sh          (-> plugins/SYSTEM-S1CC)
  KERNEL=sdk/s1cc/build/simple_saw_env python3 examples/system_s1cc/make_s1cc.py --sim   (programs + emulator)
TXT
}
case "$1" in -h|--help) usage; exit 0 ;; "") ;; *) usage >&2; exit 2 ;; esac
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
B=$HERE/build
NAME=${NAME:-SYSTEM-S1CC}
MODEL_ID=${MODEL_ID:-0x0010}
[ -n "$KERNEL" ] || { usage >&2; exit 2; }
case $KERNEL in /*) ;; *) KERNEL=$(pwd)/$KERNEL ;; esac
export KERNEL
mkdir -p "$B"
unset VOICES
NV=8                                       # the most voices that fit (make_s1cc.py exits 3 when they do not)
while :; do
    set +e
    VOICES=$NV python3 "$HERE/make_s1cc.py" > "$B/make_s1cc.log" 2>&1
    rc=$?
    set -e
    if [ $rc = 0 ]; then break; fi
    if [ $rc != 3 ] || [ $NV = 1 ]; then cat "$B/make_s1cc.log"; exit 1; fi
    grep 'do not fit' "$B/make_s1cc.log"
    NV=$((NV - 1))
done
cat "$B/make_s1cc.log"
python3 "$ROOT/tools/check_optimized.py" "$HERE"/s1m0.s "$HERE"/s1m1.s $(ls "$HERE"/s1a?.s "$HERE"/s1b?.s 2>/dev/null) "$HERE"/s1mm1.s || true    # warnings: unoptimized microcode (docs/MICROCODE_OPTIMIZATION.md)
CFLAGS="-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -Os -ffreestanding -fno-builtin -nostdlib \
 -fno-zero-initialized-in-bss -fno-common -Wall -Wextra -I$HERE -I$B -DS1_NAME=\"$NAME\" -DS1_MODEL_ID=$MODEL_ID ${PARAMLOG:+-DPARAMLOG}"
for r in 0 1; do
    if [ $r = 0 ]; then SRC=model.c; BASE=0x60800000; CODE=0x9000; else SRC=engine.c; BASE=0x60200000; CODE=0x1000; fi
    arm-none-eabi-gcc $CFLAGS -c "$HERE/$SRC" -o "$B/region$r.o"
    arm-none-eabi-ld -T "$HERE/link.ld" --defsym BASE=$BASE --defsym CODE=$CODE "$B/region$r.o" -o "$B/region$r.elf"
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
# every DSP program must be in the ARM image exactly as the OS loader reads it: 32-bit little-endian words, the
# first halfword of each pair in the high half (2026-10-27: a uint16_t array reached the DSP with every pair
# swapped and hung the device when the PLUG-OUT was selected; the emulator, which runs the .hex words, cannot see it)
here = os.path.dirname(b)
sects = tuple(n for n in ('s1a2', 's1a3', 's1a4', 's1a5', 's1b2', 's1b3', 's1b4', 's1b5')
              if os.path.exists(os.path.join(here, n + '.hex')))
for r, names in ((1, ('s1m0', 's1m1', 's1c0', 's1c1') + sects), (0, ('s1mm1',))):
    img = open(os.path.join(b, f'region{r}.bin'), 'rb').read()
    for n in names:
        w = [int(x, 16) for x in open(os.path.join(here, n + '.hex')) if x.strip() and not x.startswith('#')]
        p = w + [0] * (len(w) & 1)
        blob = b''.join(struct.pack('<I', (p[i] << 16) | p[i + 1]) for i in range(0, len(p), 2))
        if img.find(blob) < 0:
            swapped = b''.join(struct.pack('<H', x) for x in w)
            sys.exit(f'region {r}: program {n} is not in the image as the OS loader reads it'
                     + (' (it is there with every halfword pair swapped: a uint16_t array)' if img.find(swapped) >= 0 else ''))
    print(f'region {r}: programs {", ".join(names)} as the OS loads them')
proj = os.path.join(b, 'proj')
os.makedirs(proj, exist_ok=True)
for r in (0, 1):
    os.replace(os.path.join(b, f'region{r}.bin'), os.path.join(proj, f'region{r}.bin'))
region = lambda r, load: {"file": f"region{r}.bin", "stamp": "26-1027_00:00:00", "magic": "SYSTEM-1:Plugout",
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
echo "$NAME: $NV voices"
