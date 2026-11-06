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

Options (environment variables; every one but KERNEL may also come from the kernel's C source,
S1_BUILD("NAME=MY-SYNTH ARM_ENV=1 ..."), docs/S1CC.md §3.3; an environment variable wins):
  KERNEL=DIR    required: an s1cc build directory (sdk/s1cc/s1cc build VOICE.c ... -o DIR)
  (the number of voices is not an option: make_s1cc.py VOICES=auto computes the most, 8 down to 1, whose slave
  programs fit the measured limits, instructions per sample and the 6 KiB of the main program, before it builds,
  and the build says how many at the end)
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
  SPLIT=auto|n  a kernel built with s1cc --fused: voices 0..n-1 run the whole voice on DSP A, the rest on DSP B;
                auto (default) balances the two DSPs (docs/examples_system_s1cc_README.md §1.3)
  EDM_STR=1     a Karplus-Strong string per voice in the delay memory of master B (docs/MUTABLE_MEMORY.md §3.3.5):
                the kernel makes the excitation; AMP DECAY / AMP RELEASE = feedback held / released, FILTER CUTOFF =
                brightness (sdk/s1cc/examples/edm/edm_string.c)
  EDM_WT=1      a wavetable stored whole in the delay memory of master B (docs/MUTABLE_MEMORY.md §3.3.8, §4.4):
                WT_WAV=file (Serum WAV, frames of 2048), filled at the first notes, OSC1 COLOR = position. The frames
                that fit are kept from the start, the rest dropped with a warning: at most 127 (the delay line), and
                as many as region 0 holds (the build retries with fewer, WT_MAXF=n to set it)
  EDM_PORT=r|w|d  probe of the ARM's indirect port to the delay memory (docs/MUTABLE_MEMORY.md §3.3.7), needs
                PARAMLOG=1: at every note region 0 reads 64 words over 4M + a test block and logs them; w: also
                writes 64 words at the first note (EDM_PORT_W0=n, default 0x380000) and reads them back;
                d: the port registers before / after one read, restored after it; p: only the counter +0x48
  MA_MAIN=0     do not load our (empty) master DSP A main at 0x0100 (default 1: load it, as the stock PLUG-OUTs do;
                without it master A runs what an earlier mode left there, which overwrites the master -> slave words
                600E..6021: docs/MUTABLE_MEMORY.md §4.3)
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
[ -n "$KERNEL" ] || { usage >&2; exit 2; }
case $KERNEL in /*) ;; *) KERNEL=$(pwd)/$KERNEL ;; esac
[ -f "$KERNEL/voice.s1k" ] || { echo "build.sh: no $KERNEL/voice.s1k (KERNEL=<s1cc build directory>)" >&2; exit 2; }
# S1_BUILD("NAME=... MODEL_ID=... PARAMLOG=1") in the kernel: the defaults; an environment variable wins
kopt() { sed -n "s/^\.option $1 //p" "$KERNEL/voice.s1k"; }
NAME=${NAME:-$(kopt NAME)}
NAME=${NAME:-SYSTEM-S1CC}
case $NAME in *" "*) echo "build.sh: NAME '$NAME' has a space (it goes into CFLAGS and the plugins/ folder)" >&2; exit 2 ;; esac
MODEL_ID=${MODEL_ID:-$(kopt MODEL_ID)}
MODEL_ID=${MODEL_ID:-0x0010}
[ -n "${PARAMLOG+x}" ] || PARAMLOG=$(kopt PARAMLOG)
[ "$PARAMLOG" = 0 ] && PARAMLOG=
# EDM_KS=1 (docs/MUTABLE_MEMORY.md §3.3): region 0 makes the OS's delay a Karplus string at the last note's period;
# EDM_SCALE = bank-5 A101 value per sample of delay (1/16384: the line's address unit, measured)
[ -n "${EDM_KS+x}" ] || EDM_KS=$(kopt EDM_KS)
[ "$EDM_KS" = 0 ] && EDM_KS=
EDM_SCALE=${EDM_SCALE:-0.00006103515625}
EDM_FB=${EDM_FB:-0.975}
# EDM_TAP=1 (make_s1cc.py option): master B reads the delay memory with its own taps; region 0 sets the OS's delay to
# write the line with the send and nothing else (feedback and its own level 0)
[ -n "${EDM_TAP+x}" ] || EDM_TAP=$(kopt EDM_TAP)
[ "$EDM_TAP" = 0 ] && EDM_TAP=
# EDM_CALL=1 (make_s1cc.py option): region 0 adds our tapreq to the OS effect at 0x0, live (model.c edm_call)
[ -n "${EDM_CALL+x}" ] || EDM_CALL=$(kopt EDM_CALL)
[ "$EDM_CALL" = 0 ] && EDM_CALL=
[ -n "$EDM_CALL" ] && EDM_TAP=1
# EDM_ECF=1 (make_s1cc.py option): two lines of our own in the delay memory; region 0 maps their bank-2 words in
# master B's map table (model.c edm_ecf, docs/MUTABLE_MEMORY.md §3.3.4)
[ -n "${EDM_ECF+x}" ] || EDM_ECF=$(kopt EDM_ECF)
[ "$EDM_ECF" = 0 ] && EDM_ECF=
# EDM_STR=1 (make_s1cc.py option): a Karplus string per voice in its own delay-memory line (model.c edm_str,
# microcode/edm_str.s, docs/MUTABLE_MEMORY.md §3.3.5)
[ -n "${EDM_STR+x}" ] || EDM_STR=$(kopt EDM_STR)
[ "$EDM_STR" = 0 ] && EDM_STR=
# EDM_GLIDE=1 (with EDM_KS): region 0 patches the effect's delay-time smoothing in the OS's copy of its master-B set
[ -n "${EDM_GLIDE+x}" ] || EDM_GLIDE=$(kopt EDM_GLIDE)
[ "$EDM_GLIDE" = 0 ] && EDM_GLIDE=
# EDM_WT=1 (make_s1cc.py option): a wavetable stored whole in the delay memory (model.c edm_wt, WT_WAV=file)
[ -n "${EDM_WT+x}" ] || EDM_WT=$(kopt EDM_WT)
[ "$EDM_WT" = 0 ] && EDM_WT=
# EDM_PORT=r|w (model.c edm_port_probe): read / write probe of the delay memory through the ARM's port
case "${EDM_PORT:-}" in ""|0) EDM_PORT= ;; r) ;; w) ;; d) ;; p) ;; *) echo "build.sh: EDM_PORT is r, w, d or p" >&2; exit 2 ;; esac
[ -z "$EDM_PORT" ] || [ -n "$PARAMLOG" ] || { echo "build.sh: EDM_PORT needs PARAMLOG=1" >&2; exit 2; }
export KERNEL
mkdir -p "$B"
# a failed build must leave nothing to install: the plug-out of an earlier build with this NAME goes first
rm -f "$ROOT/plugins/$NAME/Script/Code1.Dat" "$ROOT/plugins/$NAME/Script/Code.Dat"
# the most voices that fit: make_s1cc.py VOICES=auto computes them before it builds (exit 3: not even 1 fits)
rm -f "$B"/s1mb?.*                      # EDM_ENTRY's program: only when this build makes it
VOICES=auto OUTDIR="$B" python3 "$HERE/make_s1cc.py" > "$B/make_s1cc.log" 2>&1 || { cat "$B/make_s1cc.log"; exit 1; }
cat "$B/make_s1cc.log"
NV=$(sed -n 's/^#define NV \([0-9]*\)$/\1/p' "$B/s1_programs.h")
python3 "$ROOT/tools/check_optimized.py" "$B"/s1m0.s "$B"/s1m1.s $(ls "$B"/s1a?.s "$B"/s1b?.s 2>/dev/null) "$B"/s1mm1.s || true    # warnings: unoptimized microcode (docs/MICROCODE_OPTIMIZATION.md)
if [ -n "$EDM_KS" ]; then
    python3 - "$B/edm_ks.h" "$EDM_SCALE" "$EDM_FB" <<'PY'
import struct, sys
out, scale, fb = sys.argv[1], float(sys.argv[2]), float(sys.argv[3])
bits = lambda x: struct.unpack('<I', struct.pack('<f', x))[0]
per = [96000.0 / (440.0 * 2 ** ((n - 69) / 12)) - 3.0 for n in range(128)]   # 3 samples of loop latency (measured)
with open(out, 'w') as f:
    f.write('/* generated by build.sh (EDM_KS): A001 / A002 = period in samples x EDM_SCALE %g; feedback %g */\n' % (scale, fb))
    f.write('static const uint32_t edm_time_bits[128] = {\n')
    for i in range(0, 128, 8):
        f.write('    ' + ', '.join('0x%08x' % bits(min(p * scale, 35.0)) for p in per[i:i + 8]) + ',\n')
    f.write('};\n#define EDM_FB_BITS 0x%08xu\n#define EDM_ONE_BITS 0x3f800000u\n' % bits(fb))
PY
fi
CFLAGS="-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -Os -ffreestanding -fno-builtin -nostdlib \
 -fno-zero-initialized-in-bss -fno-common -Wall -Wextra -I$HERE -I$B -DS1_NAME=\"$NAME\" -DS1_MODEL_ID=$MODEL_ID ${PARAMLOG:+-DPARAMLOG} ${EDM_KS:+-DEDM_KS} ${EDM_TAP:+-DEDM_TAP} ${EDM_GLIDE:+-DEDM_GLIDE} ${EDM_CALL:+-DEDM_CALL} ${EDM_ECF:+-DEDM_ECF} ${EDM_STR:+-DEDM_STR} ${EDM_WT:+-DEDM_WT} ${EDM_STR_DIRECT:+-DEDM_STR_DIRECT} $([ "$EDM_CALL" = write ] && [ "${EDM_OSOFF:-1}" = 1 ] && echo -DEDM_CALL_W || true) ${EDM_GLIDE_NEW:+-DEDM_GLIDE_NEW=$EDM_GLIDE_NEW} ${EDM_PORT:+-DEDM_PORT} $([ "$EDM_PORT" = w ] && echo -DEDM_PORT_WRITE || true) $([ "$EDM_PORT" = d ] && echo -DEDM_PORT_DIAG || true) $([ "$EDM_PORT" = p ] && echo -DEDM_PORT_P || true) ${EDM_PORT_W0:+-DEDM_PORT_W0=$EDM_PORT_W0}"
for r in 0 1; do
    if [ $r = 0 ]; then SRC=model.c; BASE=0x60800000; CODE=0x9000; else SRC=engine.c; BASE=0x60200000; CODE=0x1000; fi
    arm-none-eabi-gcc $CFLAGS -c "$HERE/$SRC" -o "$B/region$r.o"
    arm-none-eabi-ld -T "$HERE/link.ld" --defsym BASE=$BASE --defsym CODE=$CODE "$B/region$r.o" -o "$B/region$r.elf"
    arm-none-eabi-objcopy -O binary "$B/region$r.elf" "$B/region$r.bin"
    rm -f "$B/region$r.o"
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
here = b                                       # make_s1cc.py OUTDIR: the programs are in build/ too
sects = tuple(n for n in ('s1a2', 's1a3', 's1a4', 's1a5', 's1b2', 's1b3', 's1b4', 's1b5')
              if os.path.exists(os.path.join(here, n + '.hex')))
entry = tuple(n for n in ('s1mb0', 's1mb5', 's1mb6', 's1mb7', 's1ma0') if os.path.exists(os.path.join(here, n + '.hex')))
for r, names in ((1, ('s1m0', 's1m1', 's1c0', 's1c1') + sects), (0, ('s1mm1',) + entry)):
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
if ! python3 "$ROOT/tools/plugout_build.py" pack "$B/proj" -o "$OUT/Code1.Dat" 2> "$B/pack.err"; then
    # EDM_WT: the table travels in region 0 (1024 blocks of 512 compressed bytes): when it does not fit, the last frames
    # go (4096 bytes each, as if they did not compress) and the build runs again; make_s1cc.py warns which
    over=$(sed -n 's/.*region 0 needs.*(\([0-9]*\) bytes too many.*/\1/p' "$B/pack.err")
    nf=$(sed -n 's/^#define WT_FRAMES \([0-9]*\)u.*/\1/p' "$B/s1_programs.h")
    if [ -n "$EDM_WT" ] && [ -n "$over" ] && [ -n "$nf" ] && [ "$nf" -gt 1 ]; then
        keep=$((nf - (over + 4095) / 4096 - 1))
        [ "$keep" -ge 1 ] || keep=1
        echo "build.sh: EDM_WT: region 0 is $over bytes too big with $nf frames: building again with $keep (WT_MAXF)" >&2
        WT_MAXF=$keep exec sh "$0" "$@"
    fi
    cat "$B/pack.err" >&2
    exit 1
fi
cat "$B/pack.err" >&2
cp "$OUT/Code1.Dat" "$OUT/Code.Dat"
python3 "$ROOT/tools/plugout_build.py" show "$OUT/Code1.Dat"
echo "plugin folder: $ROOT/plugins/$NAME"
echo "$NAME: $NV voices"
if [ -n "$EDM_WT" ]; then
    sed -n 's|^/\* EDM_WT (microcode/edm_wt.s, docs/MUTABLE_MEMORY.md §3.3.8): \(.*\) \*/$|wavetable: \1|p' "$B/s1_programs.h"
fi
