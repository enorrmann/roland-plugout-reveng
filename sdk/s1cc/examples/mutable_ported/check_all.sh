#!/bin/sh
# check_all.sh [kernel.c ...]: every mutable_ported kernel (default: mp_*.c here) built by s1cc and checked against its
# native reference (kernel in r0..r3: voice pairs), then SYSTEM-S1CC's programs with the most voices that fit (as
# build.sh: instructions per sample, 6 KiB per program, sections).
# One line per kernel: A / B instructions per voice, SNR of A and B (10 ms, note 57, knobs at 128), and the slave
# programs' totals. Work files in ${OUT:-/tmp/mutable_ported}.
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../../../.." && pwd)
OUT=${OUT:-/tmp/mutable_ported}
mkdir -p "$OUT"
[ $# -gt 0 ] || set -- "$HERE"/mp_*.c
for src in "$@"; do
    n=$(basename "$src" .c)
    log="$OUT/$n.log"
    if ! "$ROOT/sdk/s1cc/s1cc" build "$src" --regs-a 0,1,2,3 --regs-b 0,1,2,3 -o "$OUT/$n" --check > "$log" 2>&1; then
        echo "$n: FAIL $(grep -oE 'error E[0-9]+.{0,100}' "$log" | head -1)"
        continue
    fi
    a=$(grep '^DSP A:.*SNR' "$log" | sed -E 's/.*-> ([0-9]+) instructions; SNR ([-0-9.]+) dB.*/\1 \2/')
    b=$(grep '^DSP B:.*SNR' "$log" | sed -E 's/.*-> ([0-9]+) instructions; SNR ([-0-9.]+) dB.*/\1 \2/')
    mkdir -p "$OUT/$n.plug"
    nv=8                                   # the most voices that fit, as build.sh (voice pairs, sections)
    while [ $nv -ge 1 ]; do
        r=$(cd "$ROOT" && KERNEL="$OUT/$n" VOICES=$nv OUTDIR="$OUT/$n.plug" python3 examples/system_s1cc/make_s1cc.py 2>&1)
        echo "$r" | grep -q 'do not fit' || break
        nv=$((nv - 1))
    done
    tot=$(echo "$r" | awk '/^s1m0 /{a=$2} /^s1a[2-5] /{as=as "+" $2} /^s1m1 /{b=$2} /^s1b[2-5] /{bs=bs "+" $2} END{print "A " a as ", B " b bs}')
    set -- $a
    aa="A $1/voice ${2} dB"
    set -- $b
    echo "$n: $aa, B $1/voice ${2} dB; $nv voices: $tot; $(grep -oE 'W9[0-9]+' "$log" | sort -u | tr '\n' ' ')"
done
