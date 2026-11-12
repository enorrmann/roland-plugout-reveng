#!/bin/sh
# SYSTEM-S1CC multi: one SYSTEM-1 PLUG-OUT with up to six s1cc kernels, OSC1 WAVE (0..5) chooses which one runs
# (examples/system_s1cc/README.md §9). sh build_multi.sh --help
usage() {
    cat <<'TXT'
usage: KERNELS="[POS:]DIR ..." [VAR=value ...] sh examples/system_s1cc/build_multi.sh [--help]

Each kernel is built as SYSTEM-S1CC builds it alone (make_s1cc.py: its programs, knob tables, host), all with the
same number of voices (the fewest any of them fits, or VOICES=n). Region 1 holds one engine per kernel (engine.c
with -DS1_SUB) and multi.c, which forwards the OS's calls to the selected one; moving OSC1 WAVE reloads the DSP
programs of the kernel of that position and replays the panel to it.

  KERNELS   s1cc build directories in WAVE order; POS:DIR puts one at a position (0..5). A position with no
            kernel plays the one of the position before it (position 0: the first kernel given)
            A,B:DIR puts one at OSC1 WAVE = A and OSC2 WAVE = B (wave pairs: both selectors choose the kernel);
            a pair with no kernel plays the first kernel given (README.md §12)
            KERNELS="sdk/s1cc/build/mp_saw sdk/s1cc/build/mp_square 2:sdk/s1cc/build/mp_phase_distortion"
  SECT_SWITCH=1  (TEST) every kernel built with VOICE_SECT_TEST=1 (the voices in sections, the mains only the host:
            the same in every kernel, checked); a WAVE change re-places only the chosen kernel's sections (FC40),
            engine k0 stays active (mains, ARM side, notes). README.md §12
  VOICES=n  the voices of every kernel (default: the fewest of their VOICES=auto)
  NAME=...  plug-out name (default S1CC-MULTI), MODEL_ID=0x.. (default 0x0011), PARAMLOG=1
  The other make_s1cc.py variables (BUDGET_*, LFO_PITCH, ARM_ENV, ...) apply to every kernel; each kernel's own
  S1_BUILD options stay its own.

Install: python3 tools/plugout_tool.py install plugins/<NAME> --slot 1 --force --no-patches -y
TXT
}
case "$1" in -h|--help) usage; exit 0 ;; "") ;; *) usage >&2; exit 2 ;; esac
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
B=${OUTDIR:-$HERE/build_multi}
[ -n "$KERNELS" ] || { usage >&2; exit 2; }
NAME=${NAME:-S1CC-MULTI}
MODEL_ID=${MODEL_ID:-0x0011}
[ "$PARAMLOG" = 0 ] && PARAMLOG=
rm -rf "$B"
mkdir -p "$B"

# positions -> kernel directories (k0, k1, ... in the order given; the same directory twice is one kernel)
dirs=""
two=""
first=""
nk=0
pos=0
map=""
for item in $KERNELS; do
    case $item in
        [0-5],[0-5]:*) pos=${item%%:*}; item=${item#*:}; two=1 ;;
        [0-5]:*) pos=${item%%:*}; item=${item#*:} ;;
    esac
    case $item in /*) ;; *) item=$ROOT/$item ;; esac
    [ -f "$item/voice.s1k" ] || [ -f "$(pwd)/$item/voice.s1k" ] || { echo "build_multi.sh: no $item/voice.s1k" >&2; exit 2; }
    k=-1; i=0
    for d in $dirs; do [ "$d" = "$item" ] && k=$i; i=$((i + 1)); done
    if [ $k -lt 0 ]; then dirs="$dirs $item"; k=$nk; nk=$((nk + 1)); fi
    case $pos in *,*) ;; *) [ $pos -le 5 ] || { echo "build_multi.sh: more than six positions" >&2; exit 2; } ;; esac
    [ -n "$first" ] || first=$k
    map="$map $pos:$k"
    case $pos in *,*) ;; *) pos=$((pos + 1)) ;; esac
done

# the voices: the fewest of the kernels' own (VOICES=auto each), unless VOICES=n
run_make() {   # kernel-dir outdir voices
    KERNEL=$1 OUTDIR=$2 VOICES=$3 VOICE_SECT_TEST=${SECT_SWITCH:+1} python3 "$HERE/make_s1cc.py" > "$2.log" 2>&1 || { cat "$2.log"; echo "build_multi.sh: $1 does not build" >&2; exit 1; }
}
nv_of() { sed -n 's/^#define NV \([0-9]*\)$/\1/p' "$1/s1_programs.h"; }
if [ -z "$VOICES" ] || [ "$VOICES" = auto ]; then
    VOICES=8
    i=0
    for d in $dirs; do
        mkdir -p "$B/k$i"
        run_make "$d" "$B/k$i" auto
        n=$(nv_of "$B/k$i")
        echo "k$i $(basename "$d"): $n voices alone"
        [ "$n" -lt "$VOICES" ] && VOICES=$n
        i=$((i + 1))
    done
fi
# SECT_SWITCH: every kernel's voice block padded to the largest (KV_WORDS), so the mains are the same program
if [ -n "$SECT_SWITCH" ] && [ -z "$KV_WORDS" ]; then
    KV_WORDS=0
    i=0
    for d in $dirs; do
        mkdir -p "$B/k$i"
        run_make "$d" "$B/k$i" "$VOICES"
        w=$(sed -n 's/^kv_words \([0-9]*\) .*/\1/p' "$B/k$i.log")
        [ -n "$w" ] && [ "$w" -gt "$KV_WORDS" ] && KV_WORDS=$w
        i=$((i + 1))
    done
    echo "SECT_SWITCH: voice blocks padded to $KV_WORDS words"
    export KV_WORDS
    rm -f "$B"/k*/s1_programs.h
fi
i=0
for d in $dirs; do
    mkdir -p "$B/k$i"
    if [ ! -f "$B/k$i/s1_programs.h" ] || [ "$(nv_of "$B/k$i")" != "$VOICES" ]; then run_make "$d" "$B/k$i" "$VOICES"; fi
    grep -E '^(s1m0|s1m1) ' "$B/k$i.log" | tr '\n' ' ' | sed "s|^|k$i $(basename "$d"): |"; echo
    python3 "$ROOT/tools/check_optimized.py" "$B/k$i"/s1m0.s "$B/k$i"/s1m1.s $(ls "$B/k$i"/s1a?.s "$B/k$i"/s1b?.s 2>/dev/null) > "$B/k$i.opt" 2>&1 || true
    if [ $i -gt 0 ] && [ "$(grep -v "^#" "$B/k0/s1mm1.hex")" != "$(grep -v "^#" "$B/k$i/s1mm1.hex")" ]; then
        echo "build_multi.sh: k$i has another master program than k0 (region 0 holds one)" >&2; exit 1
    fi
    if [ -n "$SECT_SWITCH" ] && [ $i -gt 0 ]; then
        for p in s1m0 s1m1 s1c0 s1c1; do
            [ "$(grep -v "^#" "$B/k0/$p.hex")" = "$(grep -v "^#" "$B/k$i/$p.hex")" ] || { echo "build_multi.sh: SECT_SWITCH: k$i's $p differs from k0's (only the sections may)" >&2; exit 1; }
        done
        [ "$(ls "$B/k0" | grep -E '^s1[ab][2-5]\.hex$')" = "$(ls "$B/k$i" | grep -E '^s1[ab][2-5]\.hex$')" ] || { echo "build_multi.sh: SECT_SWITCH: k$i has other sections than k0" >&2; exit 1; }
    fi
    i=$((i + 1))
done

# multi_map.h: the engines and the WAVE map (a position without a kernel takes the one before it)
{
    echo "/* generated by build_multi.sh */"
    echo "#define NK $nk"
    i=0; while [ $i -lt $nk ]; do echo "void *k${i}_engine(void); void k${i}_engine_sections(void);"; i=$((i + 1)); done
    [ -n "$SECT_SWITCH" ] && echo '#define SECT_SWITCH 1'
    printf 'static void (*const k_sections[NK])(void) = {'
    i=0; while [ $i -lt $nk ]; do printf ' k%s_engine_sections,' $i; i=$((i + 1)); done
    echo ' };'
    printf 'static void *(*const k_engine[NK])(void) = {'
    i=0; while [ $i -lt $nk ]; do printf ' k%s_engine,' $i; i=$((i + 1)); done
    echo ' };'
    if [ -n "$two" ]; then
        # kmap2[OSC1 WAVE][OSC2 WAVE]; a pair without a kernel: the first kernel given
        echo '#define KMAP2 1'
        printf 'static const uint8_t kmap2[6][6] = {'
        a=0
        while [ $a -le 5 ]; do
            printf ' {'
            b2=0
            while [ $b2 -le 5 ]; do
                k=$first
                for e in $map; do [ "${e%%:*}" = "$a,$b2" ] && k=${e#*:}; done
                printf ' %s,' "$k"
                b2=$((b2 + 1))
            done
            printf ' },'
            a=$((a + 1))
        done
        echo ' };'
    else
    printf 'static const uint8_t kmap[6] = {'
    last=$(echo $map | sed 's/^\([0-9]\):\([0-9]*\).*/\2/')
    p=0
    while [ $p -le 5 ]; do
        for e in $map; do [ "${e%%:*}" = $p ] && last=${e#*:}; done
        printf ' %s,' "$last"
        p=$((p + 1))
    done
    echo ' };'
    fi
} > "$B/multi_map.h"
cat "$B/multi_map.h" | sed -n 's/^static const uint8_t kmap\[6\] = /WAVE -> kernel: /p; s/^static const uint8_t kmap2\[6\]\[6\] = /OSC1 x OSC2 WAVE -> kernel: /p'

CFLAGS="-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -Os -ffreestanding -fno-builtin -nostdlib \
 -fno-zero-initialized-in-bss -fno-common -Wall -Wextra -I$HERE -DS1_NAME=\"$NAME\" -DS1_MODEL_ID=$MODEL_ID ${PARAMLOG:+-DPARAMLOG}"
# each kernel's ARM_CUSTOM (its S1_BUILD option, as build.sh reads it): the kernel's own ARM side (README.md §11).
# Without it engine.c runs the host's generic ADSR and writes its raw levels into the words the kernel reads as its
# ARM values (the JP-08: the filter's g, the VCA gain): the multi builds of 2026-11-12 had broken envelopes so
arm_custom_of() {   # kernel-dir
    a=$(sed -n "s/^\.option ARM_CUSTOM //p" "$1/voice.s1k")
    [ -n "$a" ] || return 0
    case $a in /*) ;; *) a=$ROOT/$a ;; esac
    [ -f "$a" ] || { echo "build_multi.sh: ARM_CUSTOM $a: no such file" >&2; exit 2; }
    echo "-DARM_CUSTOM=\"$a\""
}
objs=""
i=0
for d in $dirs; do eval "acust_$i='$(arm_custom_of "$d")'"; i=$((i + 1)); done
i=0
while [ $i -lt $nk ]; do
    eval "acust=\$acust_$i"
    src=$HERE/engine.c
    [ -n "$SECT_SWITCH" ] && [ $i -gt 0 ] && src=$HERE/sect_only.c        # only its sections (README.md §12)
    arm-none-eabi-gcc $CFLAGS $acust -I"$B/k$i" -DS1_SUB=k${i}_engine -c "$src" -o "$B/k$i.o"
    objs="$objs $B/k$i.o"
    i=$((i + 1))
done
# every engine object exports only its k<i>_engine (all else is static): a clash would link the wrong tables
for o in $objs; do
    g=$(arm-none-eabi-nm -g --defined-only "$o" | awk '{print $3}' | grep -v '^k[0-9]*_engine\(_sections\)\?$' || true)
    [ -z "$g" ] || { echo "build_multi.sh: $o exports $g" >&2; exit 1; }
done
arm-none-eabi-gcc $CFLAGS -I"$B" -c "$HERE/multi.c" -o "$B/multi.o"
arm-none-eabi-ld -T "$HERE/link.ld" --defsym BASE=0x60200000 --defsym CODE=0x1000 "$B/multi.o" $objs -o "$B/region1.elf"
arm-none-eabi-gcc $CFLAGS $acust_0 -I"$B/k0" -c "$HERE/model.c" -o "$B/region0.o"
arm-none-eabi-ld -T "$HERE/link.ld" --defsym BASE=0x60800000 --defsym CODE=0x9000 "$B/region0.o" -o "$B/region0.elf"
for r in 0 1; do
    arm-none-eabi-objcopy -O binary "$B/region$r.elf" "$B/region$r.bin"
    arm-none-eabi-size "$B/region$r.elf"
done
python3 - "$B" "$NAME" "$MODEL_ID" "$nk" <<'PY'
import json, os, struct, sys
b, name, mid, nk = sys.argv[1], sys.argv[2], int(sys.argv[3], 0), int(sys.argv[4])
for r, base in ((0, 0x60800000), (1, 0x60200000)):
    img = open(os.path.join(b, f'region{r}.bin'), 'rb').read()
    assert img[:16] == b'SYSTEM1_SYNINFO\0' and img[0x1B4:0x1C4] == b'SYSTEM1_SYNINFO\0', 'SYNINFO'
    for off in (0x24, 0x28, 0x2C, 0x30):
        p = struct.unpack_from('<I', img, off)[0]
        assert p == 0 or (p & 1 and base <= p < base + len(img)), (r, hex(off), hex(p))
    print(f'region {r}: {len(img)} bytes, SYNINFO ok')
# every kernel's programs in region 1 as the OS loader reads them (32-bit words, first halfword high: build.sh)
def blob(path):
    w = [int(x, 16) for x in open(path) if x.strip() and not x.startswith('#')]
    p = w + [0] * (len(w) & 1)
    return b''.join(struct.pack('<I', (p[i] << 16) | p[i + 1]) for i in range(0, len(p), 2))
img1 = open(os.path.join(b, 'region1.bin'), 'rb').read()
img0 = open(os.path.join(b, 'region0.bin'), 'rb').read()
for k in range(nk):
    kd = os.path.join(b, f'k{k}')
    names = [n for n in ('s1m0', 's1m1', 's1c0', 's1c1', 's1a2', 's1a3', 's1a4', 's1a5', 's1b2', 's1b3', 's1b4', 's1b5')
             if os.path.exists(os.path.join(kd, n + '.hex'))]
    for n in names:
        if img1.find(blob(os.path.join(kd, n + '.hex'))) < 0:
            sys.exit(f'region 1: k{k} program {n} is not in the image as the OS loader reads it')
    print(f'region 1: k{k} programs {", ".join(names)} as the OS loads them')
if img0.find(blob(os.path.join(b, 'k0', 's1mm1.hex'))) < 0:
    sys.exit('region 0: program s1mm1 missing')
proj = os.path.join(b, 'proj')
os.makedirs(proj, exist_ok=True)
for r in (0, 1):
    os.replace(os.path.join(b, f'region{r}.bin'), os.path.join(proj, f'region{r}.bin'))
region = lambda r, load: {"file": f"region{r}.bin", "stamp": "26-1103_00:00:00", "magic": "SYSTEM-1:Plugout",
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
echo "$NAME: $nk kernels, $VOICES voices"
