"""The DSP's native ops (shifts, SFU, Newton step, 3-operand add, two-product MAC) on the SYSTEM-8 master, exact,
against the emulator (re/dspisa.py): before s1cc and the SYSTEM-1 host use them (docs/DSP_HARDWARE.md §5).

    python3 dspnative.py [TEST ...] [-n 24] [--list]      (needs the PEEK PLUG-OUT selected, dsppeek.py ping)

Each case loads r0..r7 with RAW operands, runs the op and stores r0..r7 to bank 5 (master DSP3, as dspexact.py), in
two forms: with an empty instruction between the op and the stores, and with the stores right after it (a result
that is not ready in the next instruction shows up there). Every stored register is compared with dspisa.run.
"""
import argparse, os, random, sys
from fractions import Fraction as F
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
import dspisa                                   # noqa: E402
import dspasm as A                              # noqa: E402
from dspexact import BANK5, enc, val, movimm   # noqa: E402

OUT = 0x200
PER_LOAD = 6                                    # cases per program (8 stores each, the saved area holds 400 words)


def raw(x):
    """float -> RAW word (truncated), 0 for 0"""
    return enc(F(x), 'trunc') or 0


def gen_any(lo, hi, regs=range(8)):
    return lambda r: {k: raw(r.uniform(lo, hi)) for k in regs}


def gen_pos(lo, hi):
    return lambda r: {k: raw(r.uniform(lo, hi)) for k in range(8)}


def gen_exp(r):
    """mixed magnitudes, both signs: 2^-20 .. 2^6"""
    return {k: raw(r.choice((-1, 1)) * 2 ** r.uniform(-20, 6)) for k in range(8)}


def sfu(d, mode, y, neg=False):
    """SFU word, unit B (DSP_HARDWARE.md §5.3): D = 8 + d, nibble f, bits 7-6 = 00, mode, Y (bit 3 = the variant)"""
    return [0x0A02, (8 + d) << 12 | 0xF00 | mode << 4 | (8 if neg else 0) | y]


def bop(d, nib, mode, x, y):
    """unit B word with D >= 8 (§5.2): nibble nib (X bit 2 in its low bit), mode, X bits 1-0, Y"""
    return [0x0A02, (8 + d) << 12 | (nib | (x >> 2)) << 8 | (x & 3) << 6 | mode << 4 | y]


def mac(d, op1, x, y, z, op2, c, w):
    """MAC, register operands (§5.4): w1 = D op1 X mY=0 Y, w2 = Z op2 C mW=0 W (header 0803 as dspexact mac2)"""
    return [0x0803, d << 12 | op1 << 9 | x << 6 | y, z << 12 | op2 << 9 | c << 6 | w]


# name: (instruction words, operand generator)
TESTS = {
    # unit B, X * 2^Y with Y the 4-bit field (shl / shr / shrn)
    **{f'shl{k}': (bop(2, 0xC, 1, 0, k), gen_any(-4, 4)) for k in (1, 3, 8, 15)},
    **{f'shr{k}': (bop(2, 0xC, 2, 0, k), gen_any(-400, 400)) for k in (1, 5, 8, 15)},
    **{f'shrn{k}': (bop(2, 0xC, 3, 0, k), gen_any(-400, 400)) for k in (0, 7)},
    'frexp': (bop(2, 0xC, 0, 0, 1), gen_exp),                      # r2 <- m, r1 <- e of r0
    # SFU
    'recipseed': (sfu(2, 0, 0), gen_exp),
    'atanhsplit': (sfu(2, 0, 0, True), gen_pos(0.01, 100)),
    'sqrtsplit': (sfu(2, 1, 0), gen_pos(1e-4, 300)),
    'expsplit': (sfu(2, 1, 0, True), gen_any(-12, 12)),
    'log2split': (sfu(2, 2, 0), gen_exp),
    'sign': (sfu(2, 2, 0, True), gen_exp),
    'ceil': (sfu(2, 3, 0), gen_any(-50, 50)),
    'floor': (sfu(2, 3, 0, True), gen_any(-50, 50)),
    # unit A
    'newton': ([0x0902, 0x2081], gen_pos(0.1, 2)),                 # r2 <- r2 r3; r3 <- 2 - r3 r1; r1 <- r3 r1
    'add3': ([0x0902, 0x2201], gen_any(-100, 100)),                # r2 <- r2 + r0 + r1
    'sum8': ([0x0902, 0xA601], gen_any(-100, 100)),                # r2 <- r0 + r1 + r3
    # MAC, two products, register coefficients
    'mac_p1p2': (mac(2, 6, 0, 1, 3, 1, 4, 5), gen_any(-4, 4)),    # r2 <- r0 r1 + r5 + r4 r3 (dspexact mac2)
    'mac_p1w': (mac(2, 4, 0, 1, 3, 1, 4, 5), gen_any(-4, 4)),     # r2 <- r0 r1 + r4 r3
    'mac_n1w': (mac(2, 5, 0, 1, 3, 1, 4, 5), gen_any(-4, 4)),     # r2 <- -r0 r1 + r4 r3
    'mac_n1p2': (mac(2, 7, 0, 1, 3, 1, 4, 5), gen_any(-4, 4)),    # r2 <- -r0 r1 + r5 + r4 r3
    'mac_p1p2h': (mac(10, 6, 0, 1, 3, 1, 4, 5), gen_any(-4, 4)),  # r2 <- (r0 r1 + r5 + r4 r3) / 2
}


def program(words, cases, gap):
    P = [[0x0A02, 0xFF70]]
    for k, regs in enumerate(cases):
        P += [movimm(r, w) for r, w in sorted(regs.items())] + [[1], words] + ([[1]] if gap else [])
        P += [A.st(r, 0xA000 + OUT + 8 * k + r) for r in range(8)]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return [w for i in P for w in i]


def expect(words, regs):
    r, _, txt = dspisa.run(words, [float(val(regs[k])) for k in range(8)])
    return r, txt


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('tests', nargs='*')
    ap.add_argument('-n', type=int, default=24)
    ap.add_argument('--list', action='store_true')
    ap.add_argument('--sim', action='store_true', help='only the emulator (no device): print one case per test')
    a = ap.parse_args()
    names = a.tests or list(TESTS)
    if a.list or a.sim:
        rnd = random.Random(1)
        for n in names:
            words, gen = TESTS[n]
            regs = gen(rnd)
            r, txt = expect(words, regs)
            print(f'{n:11s} {" ".join(f"{w:04x}" for w in words):16s} {txt}')
            if a.sim:
                print('   in  ' + ' '.join(f'{float(val(regs[k])):.6g}' for k in range(8)))
                print('   out ' + ' '.join(f'{x:.6g}' for x in r))
        return
    from dsppeek import Peek
    from dspmaster import Session
    rnd = random.Random(1)
    summary = []
    with Session(Peek(), size=400) as ses:
        for n in names:
            words, gen = TESTS[n]
            res = {}
            for gap in (True, False):
                bad, tot, first = 0, 0, None
                for b in range(0, a.n, PER_LOAD):
                    cases = [gen(rnd) for _ in range(min(PER_LOAD, a.n - b))]
                    ses.load(program(words, cases, gap))
                    got = ses.pk.read(BANK5 + 4 * OUT, 8 * len(cases))
                    for k, regs in enumerate(cases):
                        exp, _ = expect(words, regs)
                        for r in range(8):
                            g = float(val(got[8 * k + r]))
                            tot += 1
                            if g != exp[r]:
                                bad += 1
                                if first is None:
                                    first = (r, [float(val(regs[i])) for i in range(8)], g, exp[r])
                res[gap] = (bad, tot, first)
            line = f'{n:11s} gap: {res[True][0]}/{res[True][1]} wrong   no gap: {res[False][0]}/{res[False][1]} wrong'
            print(line, flush=True)
            summary.append(line)
            for gap in (True, False):
                f = res[gap][2]
                if f:
                    print(f'   {"gap" if gap else "no gap"}: r{f[0]} hw {f[2]!r} emulator {f[3]!r}; in ' +
                          ' '.join(f'{x:.6g}' for x in f[1]))


if __name__ == '__main__':
    main()
