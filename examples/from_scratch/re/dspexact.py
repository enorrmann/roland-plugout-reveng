"""Bit-exact arithmetic of the DSP (2026-10-17): random full-precision RAW operands, the op, the RAW result read
exactly from bank 5 (master DSP3), compared with exact rational models of the RAW encoding and rounding.

    python3 dspexact.py add mul mac ... [-n 64]

RAW: value = +-M / 2^(32 - 4s), s = 0..3 (bits 30-29), M = 29 bits; the encoder picks the smallest s that fits.
"""
import argparse, os, random, sys
from fractions import Fraction as F
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
from dsppeek import Peek
from dspmaster import Session, words_of
import dspasm as A

BANK5 = 0x41122000
OUT = 0x200


def val(w):
    s = (w >> 29) & 3
    v = F(w & 0x1FFFFFFF, 2 ** (32 - 4 * s))
    return -v if w >> 31 else v


def enc(v, mode):
    """exact rational -> RAW word with rounding mode: trunc, floor, near (half away), even (half to even)"""
    neg = v < 0
    a = -v if neg else v
    for s in range(4):
        sc = 2 ** (32 - 4 * s)
        x = a * sc
        if mode == 'trunc':
            M = int(x)
        elif mode == 'floor':
            M = int(x) if not neg or x == int(x) else int(x) + 1      # floor of the signed value
        elif mode == 'near':
            M = int(x + F(1, 2))
        else:
            M = round(x)
        if M < 2 ** 29:
            return (neg << 31 | s << 29 | M) if M else 0
    return None


def rnd_raw(rnd, smax=3, smin=0):
    s = rnd.randint(smin, smax)
    M = rnd.randint(2 ** 25, 2 ** 29 - 1) if s else rnd.randint(1, 2 ** 29 - 1)
    return rnd.randint(0, 1) << 31 | s << 29 | M


def movimm(r, w):
    """rR = imm32 (the RAW word w)"""
    return [0x0904, (r << 12) | 0x0070, w >> 16, w & 0xFFFF]


# op: (instruction words, operand generator, exact function of the input values)
OPS = {
    'add': ([0x0902, 0x2C01], lambda r: (rnd_raw(r, 2), rnd_raw(r, 2), 0), lambda a, b, c: a + b),
    'mul': ([0x0902, 0x2401], lambda r: (rnd_raw(r, 2), rnd_raw(r, 1), 0), lambda a, b, c: a * b),
    'mac': ([0x0902, 0x2601], lambda r: (rnd_raw(r, 1), rnd_raw(r, 1), rnd_raw(r, 1)), lambda a, b, c: c + a * b),
    # MAC unit: 0803 2c01 3305 = r2 = r0*r1 + r3*r4 + r5 ; D >= 8 form ac01: r2 = (...)/2
    'mac2': ([0x0803, 0x2C01, 0x3305], lambda r: {k: rnd_raw(r, 1) for k in range(6)},
             lambda v: v[0] * v[1] + v[3] * v[4] + v[5]),
    'mac2h': ([0x0803, 0xAC01, 0x3305], lambda r: {k: rnd_raw(r, 1) for k in range(6)},
              lambda v: (v[0] * v[1] + v[3] * v[4] + v[5]) / 2),
    'lerp': ([0x0803, 0x2C0A, 0x9281], lambda r: {0: rnd_raw(r, 1), 1: rnd_raw(r, 1), 2: rnd_raw(r, 1)},
             lambda v: v[1] + v[2] * (v[0] - v[1])),
    'addbig': ([0x0902, 0x2C01], lambda r: (rnd_raw(r, 3, 3), rnd_raw(r, 3, 3), 0),
               lambda a, b, c: (lambda t: (abs(t) % 512) * (1 if t >= 0 else -1))(a + b)),
    'sub': ([0x0902, 0x2E01], lambda r: (rnd_raw(r, 2), rnd_raw(r, 2), 0), lambda a, b, c: b - a),
}


def program(op, cases):
    ins = OPS[op][0]
    P = [[0x0A02, 0xFF70]]
    for k, regs in enumerate(cases):
        if len(regs) == 3:
            regs = dict(zip((0, 1, 2), regs))
        P += [movimm(r, w) for r, w in regs.items()] + [[1], ins, [1], A.st(2, 0xA000 + OUT + k)]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return [w for i in P for w in i]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ops', nargs='+')
    ap.add_argument('-n', type=int, default=64)
    a = ap.parse_args()
    rnd = random.Random(1)
    with Session(Peek(), size=400) as ses:
        for op in a.ops:
            _, gen, fn = OPS[op]
            score = {m: 0 for m in ('trunc', 'floor', 'near', 'even')}
            bad, tot = [], 0
            for b in range(0, a.n, 12):
                cases = [gen(rnd) for _ in range(min(12, a.n - b))]
                ses.load(program(op, cases))
                got = ses.pk.read(BANK5 + 4 * OUT, len(cases))
                for regs, g in zip(cases, got):
                    if isinstance(regs, dict):
                        exact = fn({r: val(w) for r, w in regs.items()})
                        x, y, z = regs.get(0, 0), regs.get(1, 0), regs.get(3, 0)
                    else:
                        x, y, z = regs
                        exact = fn(val(x), val(y), val(z))
                    if abs(exact) >= 512:
                        continue
                    tot += 1
                    hit = [m for m in score if enc(exact, m) == g]
                    for m in hit:
                        score[m] += 1
                    if not hit:
                        bad.append((x, y, z, g, enc(exact, 'trunc'), float(exact)))
            print(f'{op}: {tot} cases; matches per rounding model: {score}', flush=True)
            for x, y, z, g, t, e in bad[:4]:
                print(f'   {x:08x} {y:08x} {z:08x} -> hw {g:08x}  trunc-model {t:08x}  exact {e:.10g}  '
                      f'(diff {(int(g & 0x1FFFFFFF) - int(t & 0x1FFFFFFF)) if t is not None else "?"} lsb)')


if __name__ == '__main__':
    main()
