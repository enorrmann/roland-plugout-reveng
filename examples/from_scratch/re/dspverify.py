"""Verify instruction semantics (and so the template) on master DSP3 against dspisa's simulation (2026-10-17).
For each instruction: memory references are moved to 0x700.. and preloaded, r0..r7 get random values, the flags of
groups 1-5 are set by cmp + X9bY, then the instruction runs; registers and every written reference are compared
with dspisa.run on the same state.

    python3 dspverify.py "954c 2476 ..." ["..." ...] [-n 3]
    python3 dspverify.py --inferred [-n 3]       # one Roland instance of every class marked 'inferred'
"""
import argparse, glob, os, random, sys
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value
from dspmaster import Session, words_of
import dspasm as A, dspisa, dspdis

VALS = [k / 8 for k in range(-7, 8) if k] + [1.5, -1.25, 2.5]
OUT, MEMOUT, REF0 = 0x780, 0x790, 0x700


def layout(ins):
    """template letters (with the slot area resolved) and the indices (in ins) of fe-referenced slot words"""
    t = dspisa.template(ins)
    body = ins[1:]
    last_t = max([i for i, k in enumerate(t) if k == 'T'], default=-1)
    refs = []
    if last_t >= 0:
        nN = t.count('M')
        slots = list(range(last_t + 1, len(body)))
        for i, k in enumerate(t[:last_t + 1]):
            w = body[i]
            if k == 'T' and (w >> 4) & 0xFF == 0xFE and (w & 15) >> 1 < len(slots):
                refs.append(1 + slots[(w & 15) >> 1])
    return t, sorted(set(refs))


def flag_prefix(rnd):
    """cmp r0 + imm into groups 1..5 with random signs (r0 = 0 first)"""
    P = [A.a('move', 0, imm=0.0, bits=16)]
    for x in (0x9, 0xA, 0xB, 0xC, 0xD):
        v = rnd.choice([1.0, -1.0, 0.0])
        P.append([0x0D8 << 5 | 4, 0x8E10, x << 12 | 0x9B1, A.raw(v) >> 16])
    return P + [[1], [1]]


def case(ins, rnd):
    t, refs = layout(ins)
    ins = list(ins)
    mem = {}
    pre = []
    addrs = []
    for k, i in enumerate(refs):
        # same bank as the original reference: loads per instruction are limited per bank (bank 0: 2, bank 5: 1)
        a = {0x0: REF0, 0xA: 0xA3F0, 0xC: 0xC3F0}.get(ins[i] >> 12, REF0) + k
        ins[i] = a
        addrs.append(a)
        v = rnd.choice(VALS)
        mem[a] = v
        pre += [A.a('move', 7, imm=v, bits=16), A.st(7, a)]
    regs = [rnd.choice(VALS) for _ in range(8)]
    P = [[0x0A02, 0xFF70]] + pre + flag_prefix(rnd)
    P += [A.a('move', r, imm=v, bits=16) for r, v in enumerate(regs)] + [[1], ins, [1], [1]]
    P += [A.st(r, OUT + r) for r in range(8)]
    for k, a in enumerate(addrs):
        P += [A.ld(0, a), [1], [1], A.st(0, MEMOUT + k)]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    # simulation of the same program up to the instruction
    dspisa.FLAGS = None
    dspisa.PENDING = {}
    sregs, smem = [0.0] * 8, dict(mem)
    for p in P[1:P.index(ins) + 3]:
        sregs, smem, txt = dspisa.run(list(p), sregs, smem)
    return P, regs, mem, sregs, smem, addrs, txt


def check(ses, ins, n, rnd):
    bad = 0
    for _ in range(n):
        P, regs, mem, sregs, smem, refs, txt = case(ins, rnd)
        ses.load([w for i in P for w in i])
        got = [raw_value(x) for x in ses.read(OUT, 8)]
        gotm = [raw_value(x) for x in ses.read(MEMOUT, len(refs))] if refs else []
        diffs = [f'r{r}: hw {got[r]:+.6g} sim {sregs[r]:+.6g}' for r in range(8) if abs(got[r] - sregs[r]) > 1e-4 * max(1, abs(got[r]))]
        diffs += [f'[{a:04x}]: hw {gotm[k]:+.6g} sim {smem.get(a, 0):+.6g}' for k, a in enumerate(refs)
                  if abs(gotm[k] - smem.get(a, 0)) > 1e-4 * max(1, abs(gotm[k]))]
        if diffs:
            bad += 1
            print(f'   MISMATCH regs {" ".join(f"{v:+g}" for v in regs)}: ' + '; '.join(diffs))
            print(f'            sim: {txt}')
    return bad


def inferred_instances():
    cls = {}
    for line in open(os.path.join(HERE, 'dsp_templates.txt')):
        w = line.split()
        if len(w) > 4 and w[0].startswith('0x') and w[2] in ('inferred', 'verified'):
            cls[int(w[0], 16)] = [int(x, 16) for x in w[4:]]
    return cls


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ins', nargs='*')
    ap.add_argument('--inferred', action='store_true')
    ap.add_argument('-n', type=int, default=3)
    a = ap.parse_args()
    items = [(f'{i[0] >> 5:#05x}', i) for i in inferred_instances().values()] if a.inferred else \
            [('', [int(x, 16) for x in s.split()]) for s in a.ins]
    rnd = random.Random(11)
    with Session(Peek(), size=400) as ses:
        for name, ins in items:
            t, refs = layout(ins)
            bad = check(ses, ins, a.n, rnd)
            print(f'{name} {" ".join(f"{x:04x}" for x in ins)}  [{t}]  -> {"OK" if not bad else f"{bad}/{a.n} MISMATCH"}', flush=True)


if __name__ == '__main__':
    main()
