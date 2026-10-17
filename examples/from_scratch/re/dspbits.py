"""Unit-B ops with D < 8 (nibbles 0-b): bitwise ops on two's-complement fixed point, results modulo 512 (a negative
result reads as 512 + x: the old `7ffxxxxx` format). Master DSP3 probes with random registers, then every candidate
function is checked (2026-10-17).

    python3 dspbits.py [word ...]       # default 0001 0201 0401 0601 0801 0a01 0101
"""
import os, random, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspprobe

Q = 2 ** 24                      # fixed-point scale for the bit ops (values are multiples of 1/8 here)
VALS = [k / 8 for k in range(-7, 8)] + [1.5, -1.5, 3.25, -3.25]


def fx(v):
    return int(round(v * Q))


def un(i):                       # integer -> value modulo 512, as the DSP shows it
    return (i / Q) % 512


CANDS = {
    'x&y': lambda x, y: un(fx(x) & fx(y)), 'x|y': lambda x, y: un(fx(x) | fx(y)),
    'x^y': lambda x, y: un(fx(x) ^ fx(y)), 'x&~y': lambda x, y: un(fx(x) & ~fx(y)),
    '~x&y': lambda x, y: un(~fx(x) & fx(y)), '~(x&y)': lambda x, y: un(~(fx(x) & fx(y))),
    '~(x|y)': lambda x, y: un(~(fx(x) | fx(y))), '~(x^y)': lambda x, y: un(~(fx(x) ^ fx(y))),
    'x|~y': lambda x, y: un(fx(x) | ~fx(y)), '~x|y': lambda x, y: un(~fx(x) | fx(y)),
    '~x': lambda x, y: un(~fx(x)), '~y': lambda x, y: un(~fx(y)), 'x': lambda x, y: un(fx(x)), 'y': lambda x, y: un(fx(y)),
}


def main():
    words = sys.argv[1:] or ['0001', '0201', '0401', '0601', '0801', '0a01', '0101']
    rnd = random.Random(7)
    pk = Peek()
    sets = [[rnd.choice(VALS) for _ in range(8)] for _ in range(6)]
    for w in words:
        d, x = int(w, 16) >> 12, (int(w, 16) >> 6) & 7
        rows = []
        for regs in sets:
            after = dspprobe.probe(pk, [f'0a02 {w}'], regs)
            rows.append((regs, raw_value(after[d]), after[d]))
        ok = [n for n, f in CANDS.items()
              if all(abs(f(r[x], r[int(w, 16) & 7]) - v) < 1e-6 or abs(f(r[x], r[int(w, 16) & 7]) - 512 - v) < 1e-6
                     for r, v, _ in rows)]
        print(f'{w}: r{d} = {", ".join(ok) or "?"}   ' +
              '; '.join(f'({r[x]:+g},{r[int(w, 16) & 7]:+g})->{v:g}' for r, v, _ in rows[:4]), flush=True)


if __name__ == '__main__':
    main()
