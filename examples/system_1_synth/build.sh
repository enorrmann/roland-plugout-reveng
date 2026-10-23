#!/bin/sh
# C-S1SY: C-S1V8 + DCO2, ADSR VCA, low-pass filter, filter ADSR (docs/examples_system_1_synth_README.md). sh build.sh --help
# Needs arm-none-eabi-gcc/ld/objcopy and Python 3. No Roland file is read: all code and microcode are ours.
usage() {
    cat <<'TXT'
usage: [VAR=value ...] sh examples/system_1_synth/build.sh [--help]

Builds the C-S1SY PLUG-OUT for the SYSTEM-1 -> plugins/<NAME>/Script/Code1.Dat (and Code.Dat, the same file).
Install: python3 tools/plugout_tool.py install plugins/<NAME> --slot 1 --force --no-patches -y
Then select the PLUG-OUT on the panel.

Options (environment variables):
  STEP=n        incremental step (default 8). Each one adds one thing to the previous one:
                  1 DCO2 (second saw, +7 cents)          2 OSC1/OSC2 levels from the panel
                  3 per-voice transport DSP A -> B       4 amp ADSR (DSP B)
                  5 OSC ranges + OSC2 fine tune          6 4-pole low-pass (cutoff, resonance, key track)
                  7 waves: selector position 2 = square, 3 = triangle, others saw
                  8 filter ADSR (faders 0x182..0x185) and env amount (0x181)
                  9 LFO on DSP A: rate 0x162, shape 0x160, delay 0x161, -> pitch 0x163 / filter 0x164 /
                    amp 0x165 (works on the device, 2026-10-23; DSP A 693 instructions)
                 10 the saw with ph_wrap (Roland's form) instead of the 11-op manual wrap: DSP A 517
                 11 LFO per voice (own phase restarted at each note, own S&H), -> filter / -> amp bipolar
                    (128 = 0, below = inverted LFO); op fusion on (FUSE): DSP A 576, B 603 (works on the device)
                  0 the first all-at-once design (silent on the device; reference only)
  FUSE=0|1      fuse independent ops into one instruction as Roland (default 1 from STEP 11, docs/
                MICROCODE_OPTIMIZATION.md); every build runs tools/check_optimized.py on the programs
  PACK=0|1      pack loads / stores into the ops' instructions as Roland does (default 1). PACK=0 = one unit
                per instruction: STEP=7 PACK=0 is over the time budget of slave DSP A and hangs the device
  NAME=...      plug-out name shown on the device and plugins/ folder (default C-S1SY); max 16 characters
  GMAX=x        filter coefficient limit (default 0.7, steps >= 6): caps the cutoff (0.7 = 10.7 kHz). Above
                ~0.85 the loop oscillates at 48 kHz (Nyquist) with full resonance: inaudible, but it eats the
                filter's headroom (emulator, 2026-10-23). It does not give audible self-oscillation
  RESCOMP=1     resonance compensated by cutoff (steps >= 6): the filter self-oscillates with full resonance
                from ~80 Hz to ~2.8 kHz cutoff, as an analog ladder (works on the device 2026-10-23; the
                oscillation keeps sounding after note off, and needs oscillator level to start; DSP B 708
                instead of 660 instructions = most tried). Without it it never self-oscillates
  RES_MAX=x     feedback at the top of the resonance knob (default 3.6; 4.4 with RESCOMP; 4 = threshold)
  VOICES=n      voices (default 8; the OS forwards at most 8)
  MODEL_ID=0x.. model id in the SYNINFO (default 0x0010)
  PARAMLOG=1    diagnostic: region 0 sends every parameter the OS hands it to the PC as SysEx F0 7D 06;
                capture with python3 examples/system_1/paramlog.py -t 90 --raw (select the PLUG-OUT first)
  PAD=n         budget test: n harmless fillers at the end of slave DSP A's program
  STAGE=n       only with STEP=0: how much of the all-at-once design (1..4)

Examples:
  sh examples/system_1_synth/build.sh                                (STEP=8, packed -> plugins/C-S1SY)
  GMAX=0.9 NAME=C-S1SYG sh examples/system_1_synth/build.sh
  RESCOMP=1 NAME=C-S1SYR sh examples/system_1_synth/build.sh
  PARAMLOG=1 NAME=C-S1SYL sh examples/system_1_synth/build.sh
  STEP=7 python3 examples/system_1_synth/make_synth.py --sim       (DSP programs + simulation only)
TXT
}
case "$1" in -h|--help) usage; exit 0 ;; "") ;; *) usage >&2; exit 2 ;; esac
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
B=$HERE/build
NAME=${NAME:-C-S1SY}
MODEL_ID=${MODEL_ID:-0x0010}
mkdir -p "$B"
python3 "$HERE/make_synth.py"
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
