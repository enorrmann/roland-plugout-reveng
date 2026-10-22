"""Tests of the C-S1JX microcode transformation (s1jx.py), each one on its own, no hardware.

    python3 test_s1jx.py            # all
    python3 test_s1jx.py units      # only the tests whose name contains 'units'

Needs plugins/JX-3P (the microcode is read from it, as the build does). What each test proves:

* identity: dspreloc.Prog decodes every transfer of the six mains and the DCO programs (literal addresses only) and
  relocate(identity) gives the same words back.
* units: every unit (MAIN0-5, DCO1@2, DCO1@3), original vs relocated, alone in one emulated DSP. Their external words
  (everything that is not their own ring memory: bank-5 parameters, MXM, pair window, bank-1 DCO interface, IOMD)
  are moved by the harness to the same own bank-5 words in both, and get the same random inputs every sample;
  bank 6 gets the same random parameters at the original and at the offset ids. The stores must be equal bit for
  bit for 3000 samples (NaN included). Proves: the ring offsets, the bank-6 offsets and the external map are a
  pure renaming. It does not test the timing between units (one-sample latencies, see the doc).
* scramble: every unit (relocated), with and without one of our programs before it that sets r0-r7 and the five
  flag groups to other values. Equal stores = the unit does not depend on the registers / flags another program
  leaves, so two units can share a DSP (on the SYSTEM-8 each main had its own DSP).
* memory: ring ranges of the units of one chip do not overlap (with the ring reach of each program), bank-6 ids of
  one DSP do not overlap, nor do the OS's; bank-5 words have one owner; IOMD within the pools.
* sizes: each program location fits its zone (slave main 0x100-0x18FF, sections 0x1800, master B 0x1000-0x5FF1,
  master A 0x100-0x6DF1).
* budget: instructions executed per sample on each SYSTEM-1 DSP (emulator, random inputs) <= 925 (master A: 925 -
  40 for the OS output program).
"""
import os
import random
import struct
import sys
import traceback

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import s1jx as J  # noqa: E402
import dspreloc as R  # noqa: E402
import chain  # noqa: E402
import cchain  # noqa: E402

_PLAN = None
BUDGET = 925
SCRAMBLE = ['mov r0, #0.3', 'mov r1, #-0.7', 'mov r2, #0.11', 'mov r3, #0.9', 'mov r4, #-0.25', 'mov r5, #0.6',
            'mov r6, #-0.45', 'mov r7, #0.8', 'cmp r0, r0, -r1 | setflags 1, 1', 'cmp r1, r1, -r0 | setflags 2, 1',
            'cmp r1, r1, -r0 | setflags 3, 1', 'cmp r0, r0, -r1 | setflags 4, 1', 'cmp r1, r1, -r0 | setflags 5, 1']


def plan():
    global _PLAN
    if _PLAN is None:
        words = J.roland_programs()
        _PLAN = (J.Plan(words), words)
    return _PLAN


def ring(a):
    return a >> 12 in (0x0, 0x2)


# ---------------------------------------------------------------- harness
def harness(p, key_of):
    """relocate Prog p: every external ref -> own bank-5 word of its key (same key, same word); ring and bank-6 refs
    stay. Returns (words, {key: index}, loaded keys, stored keys)"""
    keys, ld, st = {}, set(), set()
    for r in p.refs:
        k = key_of(r)
        if k is None:
            continue
        keys.setdefault(k, None)
        (ld if r.op == 'ld' else st).add(k)
    index = {k: 0x040 + i for i, k in enumerate(sorted(keys, key=repr))}
    assert len(index) <= 0x3C0, len(index)

    def f(r):
        k = key_of(r)
        return r.addr if k is None else 0xA000 | index[k]
    return p.relocate(f), index, ld, st


def key_original(pl, u):
    def k(r):
        new = pl.map(u, r)
        if new >> 12 == 0xC or (ring(r.addr) and ring(new)):
            return None
        return new
    return k


def key_relocated(r):
    return None if ring(r.addr) or r.addr >> 12 == 0xC else r.addr


def run(progs, index, ld, st, b6ids, seed, n=3000):
    """progs: [word lists] in one DSP; inputs = loaded-only keys; returns the stored words' bits per sample"""
    s = cchain.Slave({0: (chain.Program(progs[0], 'p0'), [chain.Program(w, f'p{i + 1}') for i, w in
                                                          enumerate(progs[1:])])}, native=False, threads=False)
    rng = random.Random(seed)
    inputs = sorted(ld - st, key=repr)
    outs = sorted(st, key=repr)
    rec = []
    for t in range(n):
        if t % 250 == 0:
            for i in b6ids:
                s.params[0xC000 + i[1]] = rng.choice([0.0, 1.0, rng.uniform(-1, 1)]) if True else 0
        for k in inputs:
            s.p_bank(0, index[k], rng.choice([0.0, 1.0, rng.uniform(-1, 1)]))
        s.step()
        rec.append(tuple(struct.pack('<d', s.d[0].bank5[index[k]]) for k in outs))
    return rec


def b6_pairs(pl, u, orig):
    """[(original id, id as the program reads it)] for the bank-6 ids of unit u"""
    off = pl.b6off.get(u, 0)
    ids = sorted({a & 0xFFF for a in pl.fp[u].get(0xC, (set(), set()))[0]})
    return [(i, i if orig else i + off) for i in ids]


def unit_pair(u, scramble=False):
    pl, words = plan()
    p0 = pl.p[u]
    p1 = R.Prog(pl.reloc[u], u + "'")
    w0, i0, ld0, st0 = harness(p0, key_original(pl, u))
    w1, i1, ld1, st1 = harness(p1, key_relocated)
    return pl, (w0, i0, ld0, st0), (w1, i1, ld1, st1)


def same_inputs(a, b, b6a, b6b, seed):
    """run a and b with the same random sequence: the keys must match one to one (sorted order)"""
    return run([a[0]], a[1], a[2], a[3], b6a, seed), run([b[0]], b[1], b[2], b[3], b6b, seed)


# ---------------------------------------------------------------- tests
def test_identity():
    pl, words = plan()
    for u, p in pl.p.items():
        assert not p.other, (u, p.other[:3])
        assert p.relocate(lambda r: r.addr) == p.words, u


def test_units():
    pl, words = plan()
    for u in J.UNITS:
        _, a, b = unit_pair(u)
        # the keys of both harnesses must correspond in sorted order: original keys are relocated addresses already
        assert sorted(a[1], key=repr) == sorted(b[1], key=repr), (u, set(a[1]) ^ set(b[1]))
        ra, rb = same_inputs(a, b, b6_pairs(pl, u, True), b6_pairs(pl, u, False), seed=hash(u) & 0xFFFF)
        bad = next((t for t, (x, y) in enumerate(zip(ra, rb)) if x != y), None)
        assert bad is None, f'{u}: stores differ from sample {bad}'
        print(f'    {u}: {len(a[3])} stored words equal for {len(ra)} samples')


def test_scramble():
    pl, words = plan()
    import asm
    r = asm.Assembler().assemble('\n'.join(SCRAMBLE) + '\n')
    assert r.ok, r.errors
    scr = [0x0A02, 0xFF70] + r.words + [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001]
    for u in J.UNITS:
        w, idx, ld, st = harness(R.Prog(pl.words[u], u), key_relocated)
        b6 = b6_pairs(pl, u, False)
        a = run([w], idx, ld, st, b6, seed=7)
        b = run([scr, w], idx, ld, st, b6, seed=7)
        bad = next((t for t, (x, y) in enumerate(zip(a, b)) if x != y), None)
        assert bad is None, f'{u}: depends on the registers / flags left by the program before it (sample {bad})'


def test_handoff():
    """S8 order (main, then its DCO slot, same DSP, no glue) vs SYSTEM-1 (main + our r2/r3 store, another program
    that scrambles the registers, our load + DCO): the DCO's stores are equal bit for bit"""
    pl, words = plan()
    import asm
    r = asm.Assembler().assemble('\n'.join(SCRAMBLE) + '\n')
    scr = [0x0A02, 0xFF70] + r.words + [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001]
    for dco, main in J.HANDOFF.items():
        a = [R.Prog(pl.reloc[main], main), R.Prog(pl.reloc[dco], dco)]
        b = [R.Prog(pl.words[main], main), R.Prog(scr, 'scr'), R.Prog(pl.words[dco], dco)]
        keys = {}
        for p in a + b:
            for x in p.refs:
                k = key_relocated(x)
                if k is not None:
                    keys.setdefault(k, None)
        index = {k: 0x040 + i for i, k in enumerate(sorted(keys, key=repr))}
        f = lambda x: x.addr if key_relocated(x) is None else 0xA000 | index[key_relocated(x)]
        wa = [p.relocate(f) for p in a]
        wb = [p.relocate(f) for p in b]
        ld = {key_relocated(x) for p in a + b for x in p.refs if x.op == 'ld'} - {None}
        st_main = {key_relocated(x) for p in (a[0], b[0]) for x in p.refs if x.op == 'st'} - {None}
        st_dco = {key_relocated(x) for x in a[1].refs if x.op == 'st'} - {None}
        b6 = b6_pairs(pl, main, False)
        ra = run(wa, index, ld, st_main | st_dco, b6, seed=11)
        rb = run(wb, index, ld, st_main | st_dco, b6, seed=11)
        cols = [i for i, k in enumerate(sorted(st_main | st_dco, key=repr)) if k in st_dco]
        bad = next((t for t, (x, y) in enumerate(zip(ra, rb)) if [x[i] for i in cols] != [y[i] for i in cols]), None)
        assert bad is None, f'{dco}: DCO outputs differ from sample {bad}'
        print(f'    {main} -> {dco}: {len(cols)} DCO outputs equal for {len(ra)} samples')


def test_memory():
    pl, words = plan()
    for chip in (0, 1):
        for bank, offs in ((0x0, pl.b0off), (0x2, pl.b1off)):
            spans = []
            for u in J.UNITS:
                if J.CHIP[J.PLACE[u][0]] != chip:
                    continue
                a = sorted(r.addr & 0xFFF for r in R.Prog(pl.words[u], u).refs if r.addr >> 12 == bank)
                if not a:
                    continue
                reach = pl.p[u].ring_reach(bank) or 0
                spans.append((a[0], a[-1] + reach + 1, u))
                assert a[-1] + reach < 0x800, (u, hex(a[-1]))
            spans.sort()
            for (lo1, hi1, u1), (lo2, hi2, u2) in zip(spans, spans[1:]):
                assert hi1 <= lo2, f'chip {chip} bank {bank:x}: {u1} {lo1:#x}-{hi1:#x} overlaps {u2} {lo2:#x}'
    for s1 in range(4):
        used = {}
        for u in J.UNITS:
            if J.PLACE[u][0] != s1:
                continue
            for _, i in b6_pairs(pl, u, False):
                assert i not in J.B6_RESERVED[s1], (u, hex(i))
                assert used.setdefault(i, u) == u, f'{J.S1_NAMES[s1]}: C{i:03x} used by {used[i]} and {u}'
    for chip in (0, 1):
        assert len(pl.b5_owner[chip]) == len(set(pl.b5[chip].values()))
        assert not set(pl.b5_owner[chip]) & J.B5_RESERVED[chip]
        assert max(pl.b5_owner[chip]) < J.B5_TOP
    down = [a for a in pl.iomd.values() if a in J.IOMD_DOWN]
    up = [a for a in pl.iomd.values() if a in J.IOMD_UP]
    assert len(down) + len(up) == len(pl.iomd), pl.iomd
    print(f'    IOMD: {len(down)} of {len(J.IOMD_DOWN)} down, {len(up)} of {len(J.IOMD_UP)} up')


LIMITS = {(J.MA, 0x100): 0x6DF2, (J.MB, 0x1000): 0x5FF2, (J.SA, 0x100): 0x1900, (J.SA, 0x1900): 0x3100,
          (J.SB, 0x100): 0x1900, (J.SB, 0x1900): 0x3100}


def test_sizes():
    pl, words = plan()
    for s1, addr, units, w in pl.programs():
        end = addr + 2 * len(w)
        assert end <= LIMITS[(s1, addr)], f'{J.S1_NAMES[s1]} @{addr:#x}: ends at {end:#x}'
        print(f'    {J.S1_NAMES[s1]:9} @{addr:#06x}: {2 * len(w):5d} bytes, ends {end:#06x} '
              f'(limit {LIMITS[(s1, addr)]:#06x})')


def test_budget():
    pl, words = plan()
    per = {}
    for s1, addr, units, w in pl.programs():
        per.setdefault(s1, []).append(w)
    for s1, ws in per.items():
        progs = [chain.Program(x, 'p') for x in ws]
        s = cchain.Slave({0: (progs[0], progs[1:])}, native=False, threads=False)
        rng = random.Random(3)
        worst = 0
        for t in range(1500):
            if t % 200 == 0:
                for i in range(0x200):
                    s.p_bank(0, i, rng.choice([0.0, 1.0, rng.uniform(-1, 1)]))
                for i in range(0x200):
                    s.params[0xC000 + i] = rng.choice([0.0, 1.0, rng.uniform(-1, 1)])
            a = s.ninstr_dsp(0)
            s.step()
            worst = max(worst, s.ninstr_dsp(0) - a)
        limit = BUDGET - (40 if s1 == J.MA else 0)
        print(f'    {J.S1_NAMES[s1]:9}: {worst} instructions per sample (limit {limit})')
        assert worst <= limit, (J.S1_NAMES[s1], worst)


def main(sel):
    tests = [(k, v) for k, v in globals().items() if k.startswith('test_') and callable(v)]
    if sel:
        tests = [(k, v) for k, v in tests if any(s in k for s in sel)]
    bad = 0
    for k, fn in tests:
        try:
            fn()
            print('PASS', k, flush=True)
        except Exception:
            bad += 1
            print('FAIL', k, flush=True)
            traceback.print_exc()
    print(f'{len(tests) - bad} of {len(tests)} passed')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
