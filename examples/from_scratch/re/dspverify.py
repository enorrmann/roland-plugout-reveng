"""Verify instruction semantics (and so the template) on master DSP3 against the C reference (emu/dspref.py).
For each instruction: memory references are moved to 0x700.. and preloaded, r0..r7 get random values, the flags of
groups 1-5 are set by cmp + X9bY, then the instruction runs; registers and every written reference are compared
with the reference on the same state.

    python3 dspverify.py "954c 2476 ..." ["..." ...] [-n 3]
    python3 dspverify.py --inferred [-n 3]       # one Roland instance of every class marked 'inferred'
    python3 dspverify.py --jumps                  # the jump probes (unconditional, 32 conditions, a loop)
    python3 dspverify.py ... --save FILE          # also keep the raw probes; --check FILE: no device, offline
"""
import argparse, glob, json, os, random, sys
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value
from dspmaster import Session, words_of
import dspasm as A, dspdis
sys.path.insert(0, os.path.join(HERE, '..', 'emu'))
import dspref
from dspnative import disasm

VALS = [k / 8 for k in range(-7, 8) if k] + [1.5, -1.25, 2.5]
OUT, MEMOUT, REF0 = 0x780, 0x790, 0x700


def layout(ins):
    """template letters (with the slot area resolved) and the indices (in ins) of fe-referenced slot words"""
    t = dspref.template(ins)
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
    return P, regs, mem, addrs


def simulate(P, mem, addrs):
    """the reference on the whole probe program (jumps followed; master DSP3's flag context): what it stores"""
    m = dspref.Machine(context='master_dsp3')
    m.mem.update({int(k): v for k, v in mem.items()})
    m.run_program([list(i) for i in P])
    return [m.mem.get(OUT + r) for r in range(8)], [m.mem.get(MEMOUT + k) for k in range(len(addrs))]


def compare(rec):
    """differences between a captured probe and the reference (empty: exact)"""
    sregs, smem = simulate(rec['P'], rec['mem'], rec['addrs'])
    got = [raw_value(x) for x in rec['got']]
    gotm = [raw_value(x) for x in rec['gotm']]
    diffs = [f'r{r}: hw {got[r]!r} sim {sregs[r]!r}' for r in range(8) if got[r] != sregs[r]]       # exact
    diffs += [f'[{a:04x}]: hw {gotm[k]!r} sim {smem[k]!r}' for k, a in enumerate(rec['addrs']) if gotm[k] != smem[k]]
    return diffs


def check(ses, ins, n, rnd, name, out):
    bad = 0
    for _ in range(n):
        P, regs, mem, refs = case(ins, rnd)
        ses.load([w for i in P for w in i])
        rec = {'name': name, 'ins': list(ins), 'P': [list(i) for i in P], 'mem': mem, 'addrs': refs,
               'regs': regs, 'got': list(ses.read(OUT, 8)), 'gotm': list(ses.read(MEMOUT, len(refs))) if refs else []}
        if out:
            out.write(json.dumps(rec) + '\n')
            out.flush()
        diffs = compare(rec)
        if diffs:
            bad += 1
            print(f'   MISMATCH regs {" ".join(f"{v:+g}" for v in regs)}: ' + '; '.join(diffs))
            print(f'            ins: {disasm(ins)}')
    return bad


def jump_probes():
    """jumps on the master (spec 5.4b): unconditional over one instruction; skipped by each condition 4-19 and its
    negation (the flags of groups 1-5 come from flag_prefix); a backward loop of three passes"""
    mov = A.a('move', 1, imm=0.5, bits=16)
    assert len(mov) == 3
    out = [('jmp +3', [0x0B03, 0x0090, 0x0003], [mov])]
    for cc in range(4, 20):
        for neg in (0, 1):
            out.append((f'jmp unless {"!" if neg else ""}cond{cc}', [0x1C04, 0x0090, cc << 9 | neg << 8 | 0x10, 0x0003],
                        [mov]))
    # loop: r0 += 1; flags of group 1 from r0 - 3; jump back while N (skipped when !N = cond 5)
    body = [A.a('add', 0, 0, imm=1.0, bits=16), [0x0D8 << 5 | 4, 0x8E10, 0x99B1, A.raw(-3.0) >> 16], [1]]
    n = sum(len(i) for i in body) + 4
    out.append(('loop 3', [0x1C04, 0x0090, 5 << 9 | 0x10, (-n) & 0xFFFF], body))
    return out


def program_probes():
    """probes that are whole programs (name, builder(rnd) -> P): indexed transfers (hset from a register, then
    bank 5 [h6 + Y] / [h7 + Y] loads and a store, bank 0 [h6 + Y] (X28Y)), and the load latency (an op right after
    a load, and after two loads in one instruction)"""
    NOP = [1]

    def table(rnd, base, n):
        """random values written to base .. base + n - 1 by the program itself"""
        P = []
        for k in range(n):
            P += [A.a('move', 7, imm=rnd.choice(VALS), bits=16), A.st(7, base + k)]
        return P

    def indexed5(rnd):
        i, j = rnd.randrange(0, 8), rnd.randrange(0, 8)
        P = [[0x0A02, 0xFF70]] + table(rnd, 0xA300, 12)
        P += [A.a('move', r, imm=rnd.choice(VALS), bits=16) for r in range(8)]
        P += [A.a('move', 4, imm=(0x300 + i) / 16384, bits=32), A.a('move', 5, imm=(0x300 + j) / 16384, bits=32), NOP,
              [0x0B02, 0x4084], [0x0B02, 0x40C5], NOP,                       # h6 = r4, h7 = r5
              [0x0C02, 0x0DA0], [0x0C02, 0x1DA1], [0x0C02, 0x2DB0], [0x0C02, 0x3DB2], NOP, NOP,
              [0x0C02, 0x8DA3], NOP, A.ld(6, 0xA303), NOP, NOP]              # [h6 + 3] <- r0, read back in r6
        return P

    def indexed0(rnd):
        i = rnd.randrange(0, 8)
        P = [[0x0A02, 0xFF70]] + table(rnd, 0x0700, 12)
        P += [A.a('move', r, imm=rnd.choice(VALS), bits=16) for r in range(8)]
        P += [A.a('move', 4, imm=(0x700 + i) / 16384, bits=32), NOP, [0x0B02, 0x4084], NOP,
              [0x0C02, 0x0280], [0x0C02, 0x1282], NOP, NOP]                  # r0 <- [h6 + 0], r1 <- [h6 + 2]
        return P

    def latency(rnd):
        P = [[0x0A02, 0xFF70]] + table(rnd, 0x0700, 2)
        P += [A.a('move', r, imm=rnd.choice(VALS), bits=16) for r in range(8)]
        P += [A.ld(1, 0x0700), A.a('add', 2, 1, imm=0.0, bits=16),          # r2 = r1 right after the load
              [0x1E05, 0x3FE0, 0x4FE2, 0x0700, 0x0701], A.a('mul', 5, 3, 4),  # two loads, then r5 = r3 r4
              NOP, NOP]
        return P
    return [('indexed bank 5', indexed5), ('indexed bank 0', indexed0), ('load latency', latency)]


def probe_case(build, rnd):
    P = build(rnd) + [A.st(r, OUT + r) for r in range(8)] + [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return P


def jump_case(jins, before, rnd):
    """flags, registers (r0 = 0 for the loop), the instructions `before` then the jump (a loop jumps back over
    them), the instruction after it (the one a forward jump skips), the stores"""
    regs = [rnd.choice(VALS) for _ in range(8)]
    regs[0] = 0.0
    P = [[0x0A02, 0xFF70]] + flag_prefix(rnd)
    P += [A.a('move', r, imm=v, bits=16) for r, v in enumerate(regs)] + [[1]]
    if jins[-1] & 0x8000:                               # backward: the body goes first
        P += before + [jins] + [[1], [1]]
    else:
        P += [jins] + before + [[1], [1]]
    P += [A.st(r, OUT + r) for r in range(8)]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return P, regs


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
    ap.add_argument('--jumps', action='store_true', help='the jump probes (jump_probes)')
    ap.add_argument('--programs', action='store_true', help='the program probes (program_probes)')
    ap.add_argument('--list-file', action='append', default=[], help='instructions, one per line (repeatable)')
    ap.add_argument('-n', type=int, default=3)
    ap.add_argument('--save', help='also write the raw probes (JSON lines) to this file')
    ap.add_argument('--check', help='no device: compare the reference with the probes saved in this file')
    a = ap.parse_args()
    if a.check:
        import collections
        res = collections.OrderedDict()
        for line in open(a.check):
            if line.strip():
                rec = json.loads(line)
                d = compare(rec)
                r = res.setdefault((rec['name'], tuple(rec['ins'])), [0, 0, None])
                r[0] += 1
                if d:
                    r[1] += 1
                    r[2] = r[2] or d
        for (name, ins), (n, bad, d) in res.items():
            print(f'{name} {" ".join(f"{x:04x}" for x in ins)}  -> {"OK" if not bad else f"{bad}/{n} MISMATCH: " + "; ".join(d)}')
        print(f'{sum(1 for v in res.values() if not v[1])}/{len(res)} exact')
        return
    items = [(f'{i[0] >> 5:#05x}', i) for i in inferred_instances().values()] if a.inferred else \
            [('', [int(x, 16) for x in s.split()]) for s in a.ins]
    for f in a.list_file:
        items += [('', [int(x, 16) for x in l.split()]) for l in open(f) if l.strip()]
    rnd = random.Random(11)
    out = open(a.save, 'a') if a.save else None
    with Session(Peek(), size=400) as ses:
        for name, ins in items:
            t, refs = layout(ins)
            bad = check(ses, ins, a.n, rnd, name, out)
            print(f'{name} {" ".join(f"{x:04x}" for x in ins)}  [{t}]  -> {"OK" if not bad else f"{bad}/{a.n} MISMATCH"}', flush=True)
        if a.programs:
            for name, build in program_probes():
                bad = 0
                for _ in range(a.n):
                    P = probe_case(build, rnd)
                    ses.load([w for i in P for w in i])
                    rec = {'name': name, 'ins': [], 'P': P, 'mem': {}, 'addrs': [], 'regs': [],
                           'got': list(ses.read(OUT, 8)), 'gotm': []}
                    if out:
                        out.write(json.dumps(rec) + '\n')
                        out.flush()
                    d = compare(rec)
                    if d:
                        bad += 1
                        print('   MISMATCH ' + '; '.join(d))
                print(f'{name}  -> {"OK" if not bad else f"{bad}/{a.n} MISMATCH"}', flush=True)
        if a.jumps:
            for name, jins, before in jump_probes():
                bad = 0
                for _ in range(a.n):
                    P, regs = jump_case(jins, before, rnd)
                    ses.load([w for i in P for w in i])
                    rec = {'name': name, 'ins': jins, 'P': P, 'mem': {}, 'addrs': [], 'regs': regs,
                           'got': list(ses.read(OUT, 8)), 'gotm': []}
                    if out:
                        out.write(json.dumps(rec) + '\n')
                        out.flush()
                    d = compare(rec)
                    if d:
                        bad += 1
                        print('   MISMATCH ' + '; '.join(d))
                print(f'{name} {" ".join(f"{x:04x}" for x in jins)}  -> {"OK" if not bad else f"{bad}/{a.n} MISMATCH"}', flush=True)


if __name__ == '__main__':
    main()
