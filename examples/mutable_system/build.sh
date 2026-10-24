#!/bin/sh
# MUTABLE-SYSTEM: C-S1SY13F with Braids' SAW_SWARM (docs/examples_mutable_system_README.md). sh build.sh --help
# Needs arm-none-eabi-gcc/ld/objcopy, Python 3 with numpy, and the Mutable Instruments eurorack checkout (EURORACK,
# default ~/git/eurorack: the waveshaper table is fitted from braids/resources.cc). No Roland file is read.
usage() {
    cat <<'TXT'
usage: [VAR=value ...] sh examples/mutable_system/build.sh [--help]

Builds MUTABLE-SYSTEM for the SYSTEM-1 -> plugins/<NAME>/Script/Code1.Dat (and Code.Dat, the same file):
C-S1SY13F (examples/system_1_synth: LFO, ADSRs, 4-pole low-pass, OS delay / reverb) with Braids' SAW_SWARM
(7 detuned saws, tanh waveshaper, high-pass) in place of DCO1 + DCO2, ported with sdk/mport
(docs/examples_mutable_system_README.md, docs/MUTABLE_SDK.md).
Install: python3 tools/plugout_tool.py install plugins/<NAME> --slot 1 --force --no-patches -y
Then select the PLUG-OUT on the panel.

Panel: OSC1 COLOR = TIMBRE (detune of the swarm), OSC2 COLOR = COLOR (high-pass, open at 0), OSC1 RANGE =
octave, OSC1 MIX = level; the OSC2 controls and both wave selectors do nothing. Everything after the oscillator
is C-S1SY13's.

Options (environment variables):
  KERNEL=DIR    a voice compiled from C / C++ by s1cc (sdk/s1cc/s1cc build ... -o DIR) instead of the oscillator:
                its stage A on DSP A, stage B first in the voice of DSP B, its S1_PARAM tables on their panel ids
                (docs/S1CC.md §12, e.g. KERNEL=sdk/s1cc/build/braids_saw_swarm_s1 NAME=MUTABLE-S1CC)
  SWARM=0|1     1 (default without KERNEL): SAW_SWARM written in Python (sdk/mport); 0: C-S1SY's DCO1 + DCO2
  WS_DEGREE=n   degree of the odd polynomial that replaces Braids' ws_moderate_overdrive table (default 9:
                21 LSB worst case; 7: 97 LSB, 2 instructions per voice less)
  STEP=n        C-S1SY step the example builds on (default 13; examples/system_1_synth/build.sh --help)
  NAME=...      plug-out name shown on the device and plugins/ folder (default MUTABLE-SYSTEM); max 16 characters
  GMAX=x, RESCOMP=1, RES_MAX=x, VOICES=n, MODEL_ID=0x.., PARAMLOG=1, PAD=n, PACK=0|1, SCHED=0|1, FUSE=0|1
                as in examples/system_1_synth/build.sh (SWARM schedules with the proven op groups only,
                asm/proven_forms.txt, like FOLD and step 14)

Examples:
  sh examples/mutable_system/build.sh                                 (-> plugins/MUTABLE-SYSTEM)
  python3 examples/mutable_system/make_mutable.py --sim --pack-check  (DSP programs + emulator checks only)
  python3 sdk/mport/cli.py check saw_swarm                            (the port against Braids' C++)
TXT
}
case "$1" in -h|--help) usage; exit 0 ;; "") ;; *) usage >&2; exit 2 ;; esac
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
B=$HERE/build
NAME=${NAME:-MUTABLE-SYSTEM}
MODEL_ID=${MODEL_ID:-0x0010}
mkdir -p "$B"
python3 "$HERE/make_mutable.py"
python3 "$ROOT/tools/check_optimized.py" "$HERE"/s1m0.s "$HERE"/s1m1.s "$HERE"/s1mm1.s || true    # warnings: unoptimized microcode (docs/MICROCODE_OPTIMIZATION.md)
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
