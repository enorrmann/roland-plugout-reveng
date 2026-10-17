"""Unit-B nibble e, D >= 8, bits 7-6 = 11 with mode bits != 00 (2026-10-17): phase-shaping ops. Probes with random
registers on master DSP3 and tests candidate functions of X (= r(d)), Y (= rY with the bit-3 sign) and |Y|.

    python3 dspshape.py [word ...]
"""
import math, os, random, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspprobe

VALS = [0.25, 0.75, 1.25, 1.75, 2.5, 3.25, -0.25, -0.75, -1.25, -1.75, -2.5, -3.25, 0.5, -0.5, 0.125, -0.375]


def wrap(v):                       # into [-1, 1)
    return (v + 1) % 2 - 1


C = {
    'wrap(Y)': lambda x, y: wrap(y), 'wrap(X)': lambda x, y: wrap(x), 'wrap(X+Y)': lambda x, y: wrap(x + y),
    'tri(Y)': lambda x, y: 1 - 2 * abs(wrap(y - 1.5)), 'tri(X)': lambda x, y: 1 - 2 * abs(wrap(x - 1.5)),
    'tri(X+Y)': lambda x, y: 1 - 2 * abs(wrap(x + y - 1.5)),
    'ph(Y)': lambda x, y: (y % 2) / 2, 'ph(X)': lambda x, y: (x % 2) / 2, 'ph(X+Y)': lambda x, y: ((x + y) % 2) / 2,
    '|wrap(Y)|': lambda x, y: abs(wrap(y)), 'X+Y': lambda x, y: x + y, 'Y': lambda x, y: y,
    'sat(X+Y)': lambda x, y: max(-1, min(1, x + y)), 'sat(Y)': lambda x, y: max(-1, min(1, y)),
}


def main():
    words = sys.argv[1:]
    rnd = random.Random(5)
    pk = Peek()
    for w in words:
        wi = int(w, 16)
        d, y = (wi >> 12) & 7, wi & 15
        rows = []
        for _ in range(8):
            regs = [rnd.choice(VALS) for _ in range(8)]
            after = dspprobe.probe(pk, [f'0a02 {w}'], regs)
            X, Y = regs[d], (-regs[y & 7] if y & 8 else regs[y & 7])
            rows.append((X, Y, raw_value(after[d]), [raw_value(a) for a in after], regs))
        ok = [n for n, f in C.items() if all(abs(f(X, Y) - v) < 1e-6 for X, Y, v, _, _ in rows)]
        other = sorted({i for _, _, _, a, r in rows for i in range(8) if i != d and abs(a[i] - r[i]) > 1e-9})
        print(f'{w}: r{d} = {", ".join(ok) or "?"}' + (f'  (also changes r{other})' if other else '') + '   ' +
              ' '.join(f'({X:+g},{Y:+g})->{v:+g}' for X, Y, v, _, _ in rows), flush=True)


if __name__ == '__main__':
    main()
