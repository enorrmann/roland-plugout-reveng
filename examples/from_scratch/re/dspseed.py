"""SFU seeds bit-exact (2026-10-17): recip seed `cf06` (r4 = sign 2^-e, r5 ~ 1/m for r6 = m 2^e) and sqrt split
`cf16` (r6 <- m in [1,4), r5 ~ 1/sqrt m, r7 = sign 2^-k): sample the mantissa range and print the seed as RAW
words, to find the table (step width, values).

    python3 dspseed.py recip|rsqrt [-n 256] > work/seed_recip.txt
"""
import argparse, os, sys
from fractions import Fraction as F
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
from dsppeek import Peek
from dspmaster import Session
import dspasm as A
from dspexact import movimm, val

BANK5, OUT = 0x41122000, 0x200


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('kind', choices=['recip', 'rsqrt'])
    ap.add_argument('-n', type=int, default=256)
    a = ap.parse_args()
    if a.kind == 'recip':          # m in [0.5, 1): RAW s=1 (value = M / 2^28), M in [2^27, 2^28)
        xs = [(1 << 29) | ((1 << 27) + (k << 27) // a.n) for k in range(a.n)]
        ins, outs = [0x0A02, 0xCF06], (4, 5)
    else:                          # m in [1, 4): RAW s=1, M in [2^28, 2^30) -> s=2 for >= 2: use values 1..4
        xs = [A.raw(1 + 3 * k / a.n) for k in range(a.n)]
        ins, outs = [0x0A02, 0xCF16], (5, 6, 7)
    with Session(Peek(), size=400) as ses:
        for b in range(0, len(xs), 10):
            chunk = xs[b:b + 10]
            P = [[0x0A02, 0xFF70]]
            for k, x in enumerate(chunk):
                P += [movimm(6, x), [1], ins, [1]] + [A.st(r, 0xA000 + OUT + 4 * k + j) for j, r in enumerate(outs)]
            P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
            ses.load([w for i in P for w in i])
            got = ses.pk.read(BANK5 + 4 * OUT, 4 * len(chunk))
            for k, x in enumerate(chunk):
                r = got[4 * k:4 * k + len(outs)]
                print(f'{x:08x} {float(val(x)):.9f} ' + ' '.join(f'{w:08x} {float(val(w)):.9f}' for w in r), flush=True)


if __name__ == '__main__':
    main()
