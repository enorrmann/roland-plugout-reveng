"""The DSP's native ops (shifts, SFU, Newton step, 3-operand add, two-product MAC) on the SYSTEM-8 master, exact,
against the C reference (emu/dspref.py): before s1cc and the SYSTEM-1 host use them (docs/DSP_HARDWARE.md §5).

    python3 dspnative.py [TEST ...] [-n 24] [--list] [--save FILE]   (needs the PEEK PLUG-OUT selected, dsppeek.py ping)
    python3 dspnative.py --check FILE [TEST ...]                      (no device: the reference against saved cases)

Each case loads r0..r7 with RAW operands, runs the op and stores r0..r7 to bank 5 (master DSP3, as dspexact.py), in
two forms: with an empty instruction between the op and the stores, and with the stores right after it (a result
that is not ready in the next instruction shows up there). Every stored register is compared with the reference.
"""
import argparse, os, random, sys
from fractions import Fraction as F
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'emu'))
sys.path.insert(0, os.path.join(HERE, '..', '..', '..', 'asm'))
import dspref                                   # noqa: E402
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


def bop_lo(d, nib, x, y, mode=0):
    """unit B word with D < 8: nibble nib (X bit 2 in its low bit), X bits 1-0, mode, Y"""
    return d << 12 | (nib | (x >> 2)) << 8 | (x & 3) << 6 | mode << 4 | y


def b_imm(d, nib, x, mode):
    """unit B op with an immediate Y (mode 1 / 2 / 3: 16 / 24 / 32 bits) read at byte 0 of the slot words"""
    def f(r):
        k = raw(r.uniform(-4, 4))
        imm = [k >> 16] if mode == 1 else [k >> 16, (k & 0xFF00) if mode == 2 else k & 0xFFFF]
        return [0x0A02 + len(imm), bop_lo(d, nib, x, 0, mode)] + imm
    return f


def mac_yi(op1, m):
    """r2 = r0 #Y + r4 r3 (+ r5): Y an immediate (mode m) at byte 2, register coefficient C = r4"""
    def f(r):
        k = raw(r.uniform(-4, 4))
        imm = [k >> 16] if m == 1 else [k >> 16, (k & 0xFF00) if m == 2 else k & 0xFFFF]
        w1 = 2 << 12 | op1 << 9 | 0 << 6 | m << 4 | 2
        w2 = 3 << 12 | 1 << 9 | 4 << 6 | 5
        return [0x0800 | (3 + len(imm)), w1, w2] + imm
    return f


def mac_k8(op1):
    """r2 = r0 r1 + K r3 with op2 = 011: K (imm16) at byte C + 8 = 10 (C = 2); random words before it"""
    def f(r):
        k = raw(r.uniform(-4, 4))
        w1 = 2 << 12 | op1 << 9 | 0 << 6 | 1
        w2 = 3 << 12 | 3 << 9 | 2 << 6 | 0
        tail = [r.randrange(0x10000) for _ in range(4)] + [k >> 16, r.randrange(0x10000)]
        return [0x0800 | (3 + len(tail)), w1, w2] + tail
    return f


def mac(d, op1, x, y, z, op2, c, w):
    """MAC, register operands (§5.4): w1 = D op1 X mY=0 Y, w2 = Z op2 C mW=0 W (header 0803 as dspexact mac2)"""
    return [0x0803, d << 12 | op1 << 9 | x << 6 | y, z << 12 | op2 << 9 | c << 6 | w]


def kwords(r, m, c, n=1):
    """slot words with a random coefficient at byte c of the stream (the stream starts with the n N words)"""
    k = raw(r.uniform(-4, 4)) >> (32 - 8 * (m + 1)) << (32 - 8 * (m + 1))
    words = [r.randrange(0x10000) for _ in range(c // 2 - n)] + [k >> 16, k & 0xFFFF]
    return words + [r.randrange(0x10000)]


def mac_k(d, op1, x, y, z, m, c, w):
    """MAC with an immediate coefficient (m = 1/2/3: 16/24/32 bits) at byte c; class 0x040 with the slot words
    (the header counts every word), random words around the coefficient"""
    def f(r):
        w1, w2 = mac(d, op1, x, y, z, m << 1, c, w)[1:]
        tail = kwords(r, m, c)
        return [0x0800 | (3 + len(tail)), w1, w2] + tail
    return f


# name: (instruction words or a function rnd -> words, operand generator)
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
    'sqrtsplit_neg': (sfu(2, 1, 0), gen_exp),                         # both signs, 2^-20 .. 2^6
    'expsplit': (sfu(2, 1, 0, True), gen_any(-12, 12)),
    'log2split': (sfu(2, 2, 0), gen_exp),
    'log2split_big': (sfu(2, 2, 0), gen_any(-500, -0.125)),       # Y < 0: Y / 512 on the 2^-28 grid too
    'sign': (sfu(2, 2, 0, True), gen_exp),
    'ceil': (sfu(2, 3, 0), gen_any(-50, 50)),
    'floor': (sfu(2, 3, 0, True), gen_any(-50, 50)),
    # unit B, D < 8: |X| + Y, Y - |X| (absadd / subabs), X in r0..r7, Y a register or an immediate (2026-10-29)
    **{f'absadd_x{x}': ([0x0A02, bop_lo(2, 0xC, x, 3)], gen_any(-4, 4)) for x in (1, 5)},
    **{f'subabs_x{x}': ([0x0A02, bop_lo(2, 0xE, x, 3)], gen_any(-4, 4)) for x in (1, 6)},
    'absadd_neg': ([0x0A02, bop_lo(2, 0xC, 1, 11)], gen_any(-4, 4)),          # |r1| - r3
    **{f'absadd_imm{m}': (b_imm(2, 0xC, 1, m), gen_any(-4, 4)) for m in (1, 2, 3)},
    **{f'subabs_imm{m}': (b_imm(2, 0xE, 5, m), gen_any(-4, 4)) for m in (1, 2, 3)},
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
    # MAC, immediate coefficient K, register Y (DSP_ISA_DISCREPANCIES.md D1: K at byte C): per-case words
    **{f'mac_p1wk{m}c{c}': (mac_k(2, 4, 0, 1, 3, m, c, 0), gen_any(-4, 4)) for m in (1, 2, 3) for c in (2, 4, 6)},
    **{f'mac_n1wk{m}': (mac_k(2, 5, 0, 1, 3, m, 2, 0), gen_any(-4, 4)) for m in (1, 2, 3)},
    **{f'mac_p1p2k{m}': (mac_k(2, 6, 0, 1, 3, m, 2, 5), gen_any(-4, 4)) for m in (1, 2, 3)},
    **{f'mac_n1p2k{m}': (mac_k(2, 7, 0, 1, 3, m, 2, 5), gen_any(-4, 4)) for m in (1, 2, 3)},
    'mac_p1p2k2h': (mac_k(10, 6, 0, 1, 3, 2, 2, 5), gen_any(-4, 4)),
    # immediate Y (byte 2) with a register coefficient (op2 = 001): X #Y + rC rZ (+ rW)
    **{f'mac_p1w_yi{m}': (mac_yi(4, m), gen_any(-4, 4)) for m in (1, 2, 3)},
    **{f'mac_p1p2_yi{m}': (mac_yi(6, m), gen_any(-4, 4)) for m in (1, 3)},
    # op2 = 011: an imm16 coefficient at byte C + 8 (class sweeps, 2026-10-29)
    'mac_p1wk1r': (mac_k8(4), gen_any(-4, 4)),
    # unknown forms, kept raw for the analysis (the reference may disagree): B op e with bits 7-6 = 10, f5Y counter
    'raw_b_e10': ([0x0A02, 0xAE81], gen_any(-4, 4)),
    # the e10 form = a division seed with a numerator (2026-10-29: r(d) <- rX sign(Y) 2^-e, r(d+1) <- R[m], rY <- m,
    # X = r0 with d = 2, Y = r1): where does X come from? other d / Y / mode / negated Y
    'raw_e10_d3y5': ([0x0A02, 0xBE85], gen_any(-4, 4)),
    'raw_e10_d5y2': ([0x0A02, 0xDE82], gen_any(-4, 4)),
    'raw_e10_d0y6': ([0x0A02, 0x8E86], gen_any(-4, 4)),
    'raw_e10_m1': ([0x0A02, 0xAE91], gen_any(-4, 4)),
    'raw_e10_yneg': ([0x0A02, 0xAE89], gen_any(-4, 4)),
    # modo 1: the numerator r(d) or r(2 mode)? and modes 2 / 3
    'raw_e10_m1d6': ([0x0A02, 0xEE95], gen_any(-4, 4)),
    'raw_e10_m1d3': ([0x0A02, 0xBE95], gen_any(-4, 4)),
    'raw_e10_m2': ([0x0A02, 0xAEA1], gen_any(-4, 4)),
    'raw_e10_m3': ([0x0A02, 0xAEB1], gen_any(-4, 4)),
    'raw_f53': ([0x0A02, 0xAF53], gen_any(-4, 4)),
    'mac_p1p2k2nz': (mac_k(2, 6, 0, 1, 11, 2, 2, 13), gen_any(-4, 4)),  # -Z, -W
    # two MACs in one instruction (class 0x080, MMNN, the form Roland uses: one product each): both read the
    # registers before the instruction (the second reads the old r2); a P2 term in either MAC writes nothing
    'mac2x': ([0x1005, 0x2401, 0x6482, 0x0005, 0x0007], gen_any(-4, 4)),          # r2 = r0 r1 + r5; r6 = r2 r2 + r7
    'mac2xn': ([0x1005, 0x2601, 0x5683, 0x0005, 0x000F], gen_any(-4, 4)),         # r2 = -r0 r1 + r5; r5 = -r2 r3 - r7
    'mac2xh': ([0x1005, 0xA401, 0x6482, 0x0005, 0x0007], gen_any(-4, 4)),         # D >= 8 without P2: r2 kept
    'mac_p1h': ([0x0803, 0xA401, 0x0005], gen_any(-4, 4)),                         # the same in one MAC
    'mac_n1wh': (mac(10, 5, 0, 1, 3, 1, 4, 5), gen_any(-4, 4)),                    # with P2: (-r0 r1 + r4 r3) / 2
    'mac2xi': (lambda r: [0x1007, 0x2414, 0x6496, 0x0005, 0x0007] + [raw(r.uniform(-4, 4)) >> 16 for _ in range(2)],
               gen_any(-4, 4)),                                                   # Y = imm16 at bytes 4 and 6
    'mac2xp2': ([0x1005, 0x2C01, 0x6482, 0x3305, 0x0007], gen_any(-4, 4)),        # P2 in the first: no writes
}


def program(cases, gap):
    P = [[0x0A02, 0xFF70]]
    for k, (words, regs) in enumerate(cases):
        P += [movimm(r, w) for r, w in sorted(regs.items())] + [[1], words] + ([[1]] if gap else [])
        P += [A.st(r, 0xA000 + OUT + 8 * k + r) for r in range(8)]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return [w for i in P for w in i]


_M = []


def expect(words, regs):
    """the reference's registers after the instruction, and its disassembly (asm/disasm.py)"""
    if not _M:
        _M.append(dspref.Machine(context='master_dsp3'))
    m = _M[0]
    m.reset(context='master_dsp3', regs=[float(val(regs[k])) for k in range(8)])
    m.run(list(words))
    return m.regs, disasm(words)


def disasm(words):
    import disasm as D
    import isa
    if not hasattr(disasm, 'table'):
        disasm.table = isa.Table.load()
    try:
        return D.disassemble_insn(disasm.table, list(words)).text.replace('\n', '; ')
    except Exception as e:                       # noqa: BLE001
        return '?? %s' % e


def report(records):
    """compare captured cases with the reference: one line per test and gap form, the first difference"""
    import collections
    groups = collections.OrderedDict()
    for r in records:
        groups.setdefault(r['test'], {True: [], False: []})[r['gap']].append(r)
    nbad = 0
    for n, by in groups.items():
        res = {}
        for gap, rs in by.items():
            bad, tot, first = 0, 0, None
            for r in rs:
                regs = dict(enumerate(r['regs']))
                exp, _ = expect(r['words'], regs)
                for k in range(8):
                    g = float(val(r['got'][k]))
                    tot += 1
                    if g != exp[k]:
                        bad += 1
                        if first is None:
                            first = (k, [float(val(regs[i])) for i in range(8)], g, exp[k], r['words'])
            res[gap] = (bad, tot, first)
            nbad += bad
        print(f'{n:11s} gap: {res[True][0]}/{res[True][1]} wrong   no gap: {res[False][0]}/{res[False][1]} wrong',
              flush=True)
        for gap in (True, False):
            f = res[gap][2]
            if f:
                print(f'   {"gap" if gap else "no gap"}: r{f[0]} hw {f[2]!r} emulator {f[3]!r}; in ' +
                      ' '.join(f'{x:.6g}' for x in f[1]) + '; ' + ' '.join(f'{w:04x}' for w in f[4]))
    return nbad


def main():
    import json
    ap = argparse.ArgumentParser()
    ap.add_argument('tests', nargs='*')
    ap.add_argument('-n', type=int, default=24)
    ap.add_argument('--list', action='store_true')
    ap.add_argument('--sim', action='store_true', help='only the emulator (no device): print one case per test')
    ap.add_argument('--save', help='also write the raw cases (JSON lines) to this file')
    ap.add_argument('--check', help='no device: compare the reference with the cases saved in this file')
    a = ap.parse_args()
    names = a.tests or list(TESTS)
    if a.check:
        records = [json.loads(x) for x in open(a.check) if x.strip()]
        records = [r for r in records if not a.tests or r['test'] in a.tests]
        return 1 if report(records) else 0
    if a.list or a.sim:
        rnd = random.Random(1)
        for n in names:
            words, gen = TESTS[n]
            words = words(rnd) if callable(words) else words
            regs = gen(rnd)
            r, txt = expect(words, regs)
            print(f'{n:11s} {" ".join(f"{w:04x}" for w in words):16s} {txt}')
            if a.sim:
                print('   in  ' + ' '.join(f'{float(val(regs[k])):.6g}' for k in range(8)))
                print('   out ' + ' '.join(f'{x:.6g}' for x in r))
        return 0
    from dsppeek import Peek
    from dspmaster import Session
    rnd = random.Random(1)
    out = open(a.save, 'a') if a.save else None
    records = []
    with Session(Peek(), size=400) as ses:
        for n in names:
            words, gen = TESTS[n]
            rec = []
            for gap in (True, False):
                for b in range(0, a.n, PER_LOAD):
                    cases = [((words(rnd) if callable(words) else words), gen(rnd))
                             for _ in range(min(PER_LOAD, a.n - b))]
                    ses.load(program(cases, gap))
                    got = ses.pk.read(BANK5 + 4 * OUT, 8 * len(cases))
                    for k, (words_k, regs) in enumerate(cases):
                        rec.append({'test': n, 'gap': gap, 'words': list(words_k),
                                    'regs': [regs[i] for i in range(8)], 'got': list(got[8 * k:8 * k + 8])})
            if out:
                out.write(''.join(json.dumps(r) + '\n' for r in rec))
                out.flush()
            report(rec)
            records += rec
    return 0


if __name__ == '__main__':
    sys.exit(main())
