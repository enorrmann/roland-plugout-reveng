"""Tests of the C core (cchain.py / dspcore.c / dspdec.h), the DSP reference since 2026-10-29 (the Python model
re/dspisa.py + chain.py was archived after the port was checked against it on the whole corpus: docs/EMULATOR.md §1.1).

    python3 test_core.py                 all tests
    python3 test_core.py insn            only the tests whose name contains insn
    python3 test_core.py --update-golden write the golden file (only after a measured change of the semantics)

* test_insn_golden: every distinct instruction of the local corpus (work/corpus/*.bin, work/corpus_all/*.txt: Roland
  programs, study only, never copied) as a one-instruction program on DSP2, from two random states (registers,
  flags, pending flags, hidden registers, banks 0/1/5, IOMD, parameters); the hash of the state after one sample
  must equal the golden file work/golden_insn.txt (local, like the corpus; hashes only, no instruction words).
* test_jx3p_chain: Roland's JX-3P mains and DCO slots (engine/build/programs.h, local) with random bank-5 inputs:
  native code against the interpreter, sample by sample, the whole state.
* test_native_corpus: every corpus program translated to native code (dspjit.py) against the interpreter, on DSP2
  from random states, a few samples: every register, flag, memory word equal (or both fail).
* test_native_threads_jx3p: the JX-3P with native code and threads against the interpreter on one thread, 4000
  samples with gate-like bank-5 changes: IOMD and the whole state equal.
* test_ours: v3 / v4 (ours) with a note, 300 samples: IOMD 6020 audible, interpreter = native code.
* test_speed: the JX-3P slave runs in real time (native + threads).
"""
import os
import random
import sys
import time
import traceback

import hashlib

import cchain
import chain
import listing
import voices

ROOT = chain.ROOT
CORPUS = [os.path.join(ROOT, 'work', 'corpus'), os.path.join(ROOT, 'work', 'corpus_all')]
GOLDEN = os.path.join(ROOT, 'work', 'golden_insn.txt')


class Skip(Exception):
    pass


def corpus_instructions():
    import glob
    import dspdis
    import dspmc
    seen = set()
    for d in CORPUS:
        for f in sorted(glob.glob(os.path.join(d, '*.bin')) + glob.glob(os.path.join(d, '*.txt'))):
            w = dspmc.blob_words(f) if f.endswith('.bin') else [int(t, 16) for t in open(f).read().split()]
            for _, ins in dspdis.instructions(w):
                if tuple(ins) not in seen:
                    seen.add(tuple(ins))
                    yield list(ins)


def rnd_value(r):
    return r.choice([0.0, 0.5, -0.25, 1.0, r.uniform(-1, 1), r.uniform(-8, 8), r.uniform(-600, 600)])


class State:
    """one random starting state for a cchain.Slave"""

    POOL = []

    def __init__(self, r):
        """registers, flags and hidden registers fresh; the memory contents from a pool of 4 (building them is the
        slow part)"""
        self.regs = [rnd_value(r) for _ in range(8)]
        self.flags = ''.join(r.choice('01') for _ in range(32))
        self.pending = {g: ''.join(r.choice('01') for _ in range(4)) for g in r.sample(range(1, 6), r.randint(0, 2))}
        self.hidden = {4: 288.0, 5: r.random(), 6: r.randrange(0, 0x800) / 16384, 7: r.randrange(0, 0x400) / 16384}
        while len(State.POOL) < 4:
            State.POOL.append(([rnd_value(r) for _ in range(chain.RING)], [rnd_value(r) for _ in range(chain.RING)],
                               [rnd_value(r) for _ in range(0x400)], [rnd_value(r) for _ in range(0x100)],
                               [rnd_value(r) for _ in range(0x400)]))
        self.bank0, self.bank1, self.bank5, self.iomd, self.params = r.choice(State.POOL)

    def c(self, s, n):
        d = s.d[n]
        d.set_state(self.regs, self.flags, self.pending, self.hidden)
        d.bank0[:], d.bank1[:], d.bank5[:] = self.bank0, self.bank1, self.bank5
        s.iomd.a[:0x100] = self.iomd
        s.params.a[:0x400] = self.params


def snapshot_c(s, only=None):
    out = []
    for d in (s.d if only is None else [s.d[only]]):
        regs, flags, pending, hidden = d.state()
        out.append((regs, flags, pending, hidden, list(d.bank0), list(d.bank1), list(d.bank5),
                    dict(sorted(d.mxm_in().items()))))
    mx = [dict(sorted(d.mxm_in().items())) for d in s.d]
    return out, mx, list(s.iomd.a)


def first_diff(a, b, path=''):
    if not path and repr(a) == repr(b):      # fast path (repr tells -0.0 from 0.0)
        return None
    if type(a) is not type(b) and not (isinstance(a, (int, float)) and isinstance(b, (int, float))):
        return f'{path}: {a!r} != {b!r}'
    if isinstance(a, (list, tuple)):
        if len(a) != len(b):
            return f'{path}: length {len(a)} != {len(b)}'
        for k, (x, y) in enumerate(zip(a, b)):
            e = first_diff(x, y, f'{path}[{k}]')
            if e:
                return e
        return None
    if isinstance(a, dict):
        if a.keys() != b.keys():
            return f'{path}: keys {sorted(a)} != {sorted(b)}'
        for k in a:
            e = first_diff(a[k], b[k], f'{path}[{k!r}]')
            if e:
                return e
        return None
    if isinstance(a, float) and repr(a) != repr(float(b)):      # repr: -0.0 differs from 0.0
        return f'{path}: {a!r} != {b!r}'
    if not isinstance(a, float) and a != b:
        return f'{path}: {a!r} != {b!r}'
    return None


def one_instruction(ins, st, dsp=2, native=False):
    """the state after one instruction from state st on DSP `dsp` (snapshot_c), or ('error', name)"""
    s = cchain.Slave({dsp: (chain.Program(ins, 'one', strict=False), [])}, native=native, threads=False)
    st.c(s, dsp)
    try:
        s.step()
        return snapshot_c(s, dsp)
    except (RuntimeError, ValueError) as e:
        return ('error', type(e).__name__)


def golden_line(ins, results):
    """hashes only (the corpus is Roland's code, it is not copied): instruction, results"""
    h = lambda x: hashlib.sha1(x.encode()).hexdigest()[:16]  # noqa: E731
    return h(' '.join(f'{w:04x}' for w in ins)) + ' ' + h(repr(results))


def test_quant():
    """the branch-free quant of dspcore.h against its plain form (C, every exponent, edges) and against the
    definition (DSP_HARDWARE.md §3, exact rationals: random values of the ranges the DSP uses)"""
    import ctypes as C
    L = cchain.lib()
    bad = C.c_double()
    n = L.sl_quant_check(2000000, 12345, C.byref(bad))
    assert n == 0, (n, bad.value)
    r = random.Random(5)
    for _ in range(200000):
        v = r.choice([r.uniform(-1, 1), r.uniform(-600, 600), r.uniform(-1e-3, 1e-3), r.uniform(-40, 40)])
        v = r.choice([v, float.fromhex(v.hex())])
        assert repr(L.sl_quant(v)) == repr(quant_def(v)), v
    for e in range(-40, 12):
        for v in (2.0 ** e, -(2.0 ** e), 2.0 ** e * (1 - 2 ** -53), 2.0 ** e * (1 + 2 ** -52)):
            assert repr(L.sl_quant(v)) == repr(quant_def(v)), v


def quant_def(v):
    """RAW encoding by its definition: the smallest s = 0..3 with floor(|v| 2^(32-4s)) < 2^29, truncated toward
    zero; |v| >= 512 kept"""
    from fractions import Fraction
    import math
    if v == 0:
        return v
    m = Fraction(abs(v))
    for s in range(4):
        M = math.floor(m * 2 ** (32 - 4 * s))
        if M < 2 ** 29:
            return math.copysign(float(Fraction(M, 2 ** (32 - 4 * s))), v)
    return v


def test_pymod():
    """the fast paths of dspcore.h pymod against its fmod form (bits), and against Python's float % (wrap1 of
    ph_wrap / ph_tri)"""
    import ctypes as C
    L = cchain.lib()
    bad = C.c_double()
    n = L.sl_pymod_check(2000000, 777, C.byref(bad))
    assert n == 0, (n, bad.value)


def test_insn_golden():
    if not any(os.path.isdir(d) for d in CORPUS):
        raise Skip('work/corpus missing')
    r = random.Random(7)
    lines = []
    for ins in corpus_instructions():
        lines.append(golden_line(ins, [one_instruction(ins, State(r)) for _ in range(2)]))
    if UPDATE_GOLDEN:
        open(GOLDEN, 'w').write('\n'.join(lines) + '\n')
        print(f'    {len(lines)} instructions written to {GOLDEN}')
        return
    if not os.path.exists(GOLDEN):
        raise Skip(f'{GOLDEN} missing (--update-golden)')
    gold = dict(x.split() for x in open(GOLDEN).read().split('\n') if x)
    bad = [x for x in lines if gold.get(x.split()[0]) != x.split()[1]]
    print(f'    {len(lines)} distinct instructions, {len(bad)} differ from the golden file')
    assert not bad


JX_SLOTS = {2: ('DCO1_SAW', 'DCO2_SAW'), 3: ('DCO1_SAW', 'DCO2_SAW')}


def jx_programs():
    progs = {}
    slot = {name: chain.Program(listing.words(name), name) for name in ('DCO1_SAW', 'DCO2_SAW')}
    for n in range(6):
        progs[n] = (chain.Program(listing.words(f'MAIN{n}'), f'MAIN{n}'), [slot[x] for x in JX_SLOTS.get(n, ())])
    return progs


def test_jx3p_chain(n=12):
    if not listing.available():
        raise Skip('engine/build/programs.h missing (run engine/build.sh)')
    r = random.Random(3)
    progs = jx_programs()
    sp, sc = cchain.Slave(progs, native=False, threads=False), cchain.Slave(progs)   # interpreter / native code
    for dsp in range(6):
        for i in range(0x80):
            v = r.uniform(-1, 1)
            sp.p_bank(dsp, i, v)
            sc.p_bank(dsp, i, v)
    for t in range(n):
        sp.step()
        sc.step()
        e = first_diff(snapshot_c(sp), snapshot_c(sc))
        assert e is None, f'sample {t}: {e}'
    print(f'    {n} samples, {sc.ninstr // n} instructions per sample, equal')


def corpus_programs():
    import glob
    import dspmc
    for d in CORPUS:
        for f in sorted(glob.glob(os.path.join(d, '*.bin')) + glob.glob(os.path.join(d, '*.txt'))):
            w = dspmc.blob_words(f) if f.endswith('.bin') else [int(t, 16) for t in open(f).read().split()]
            yield os.path.basename(f), chain.Program(w, os.path.basename(f))


def test_native_corpus(samples=3, states=3):
    if not any(os.path.isdir(d) for d in CORPUS):
        raise Skip('work/corpus missing')
    import dspjit
    progs = list(corpus_programs())
    dspjit.build([p for _, p in progs])                 # compile all at once (parallel), then each is cached
    r = random.Random(11)
    bad = []
    for name, p in progs:
        for _ in range(states):
            st = State(r)
            res = []
            for native in (False, True):
                s = cchain.Slave({2: (p, [])}, native=native, threads=False)
                st.c(s, 2)
                try:
                    s.step(samples)
                    res.append(snapshot_c(s))
                except RuntimeError as e:
                    res.append(('error',))
            e = first_diff(res[0], res[1])
            if e:
                bad.append((name, e))
                break
    for name, e in bad[:10]:
        print('   ', name, e)
    print(f'    {len(progs)} programs x {states} states x {samples} samples, {len(bad)} differ')
    assert not bad


def test_native_threads_jx3p(n=4000):
    if not listing.available():
        raise Skip('engine/build/programs.h missing (run engine/build.sh)')
    progs = jx_programs()
    a = cchain.Slave(progs, native=False, threads=False)
    b = cchain.Slave(progs)
    assert b.groups and len(b.groups) > 1, b.groups
    r = random.Random(4)
    for dsp in range(6):
        for i in range(0x80):
            v = r.uniform(-1, 1)
            a.p_bank(dsp, i, v)
            b.p_bank(dsp, i, v)
    words = (0x6020, 0x6021, 0x6060, 0x6061)
    for k in range(n // 1000):                          # bank-5 changes between blocks, as gates and pitches
        for _ in range(8):
            dsp, i, v = r.randrange(6), r.randrange(0x80), r.choice([0.0, 1.0, r.uniform(-1, 1)])
            a.p_bank(dsp, i, v)
            b.p_bank(dsp, i, v)
        oa, ob = a.run(1000, words), b.run(1000, words)
        assert oa == ob, f'block {k}: IOMD differs'
    e = first_diff(snapshot_c(a), snapshot_c(b))
    assert e is None, e
    print(f'    {n} samples, groups {b.groups}, equal')


def test_ours():
    for which in ('v3', 'v4'):
        sp = cchain.Slave(voices.programs(which), native=False, threads=False)
        sc = cchain.Slave(voices.programs(which))
        ep, ec = voices.Engine(sp), voices.Engine(sc)
        for e in (ep, ec):
            e.pitch(1, 69)
            e.note_on(0, 81)
        a = [(sp.step(), sp.iomd.get(0x6020, 0.0))[1] for _ in range(300)]
        b = sc.run(300, (0x6020,))[0x6020]
        assert a == b and chain.peak(a) > 0.9, which


def test_s1_pipeline(n=6000):
    """the two-stage pipeline (cchain.pipeline_plan: slave A one sample ahead on its own thread) against the serial
    run: C-S1SY step 11 with notes and the LFO, every bank-5 word of the three DSPs and the output, sample by sample
    (run() blocks with events between them, as make_synth.simulate)"""
    os.environ['STEP'] = '11'
    sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'system_1_synth'))
    import make_synth as ms
    progs = ms.sources()
    model = chain.Model(order=(0, 1, 2), shared5=((0, 1),), own_params=(0, 1, 2))
    panel = dict(ms.panel_defaults(), lfo_rate=220, lfo_pitch=200, lfo_filt=60, lfo_amp=90, lfo_wave=5,
                 cutoff=150, res=120, lfo_delay=30)
    words = [(2, 0xA118), (0, 0xA151), (1, 0xA150), 0x6006]
    outs = []
    for threads in (False, True):
        P = {k: chain.Program(progs[k][0], k) for k in ('s1m0', 's1m1', 's1mm1')}
        s = cchain.Slave({0: (P['s1m0'], []), 1: (P['s1m1'], []), 2: (P['s1mm1'], [])}, model, threads=threads)
        assert bool(s.pipeline) == threads
        b5 = ms.sim_b5(panel)
        for d, prm in zip(s.d, ms.sim_params(b5, panel)):
            d.params.update(prm)
        for k, v in b5.items():
            s.d[0].bank5[k & 0x3FF] = v
        rec = {w: [] for w in words}
        for k, t in enumerate(range(0, n, 1000)):
            ms.sim_event(b5, k % 8, 50 + k)
            for a in (ms.PITCH[k % 8], ms.GATE[k % 8], ms.TRIG[k % 8]):
                s.d[0].bank5[a & 0x3FF] = b5[a]
            r = s.run(1000, words)
            for w in words:
                rec[w] += r[w]
        outs.append((rec, [list(s.d[i].bank5) for i in range(3)], [list(s.d[i].bank0) for i in range(3)]))
    assert outs[0] == outs[1]


def test_s1_sched(n=4000):
    """asm/dspsched.py (C-S1SY step 12) against the unscheduled programs: every bank-5 and bank-0 word of the three
    DSPs and the output, sample by sample, with notes and the LFO"""
    os.environ['STEP'] = '12'
    sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'system_1_synth'))
    import importlib
    import make_synth as ms
    ms = importlib.reload(ms)
    model = chain.Model(order=(0, 1, 2), shared5=((0, 1),), own_params=(0, 1, 2))
    panel = dict(ms.panel_defaults(), lfo_rate=220, lfo_pitch=200, lfo_filt=60, lfo_amp=90, lfo_wave=4,
                 cutoff=150, res=120, lfo_delay=30, osc1_wave=1, osc2_wave=2)
    outs = []
    for progs in (ms.sources(True), ms.sources(False)):
        P = {k: chain.Program(progs[k][0], k) for k in ('s1m0', 's1m1', 's1mm1')}
        s = cchain.Slave({0: (P['s1m0'], []), 1: (P['s1m1'], []), 2: (P['s1mm1'], [])}, model)
        b5 = ms.sim_b5(panel)
        for d, prm in zip(s.d, ms.sim_params(b5, panel)):
            d.params.update(prm)
        for k, v in b5.items():
            s.d[0].bank5[k & 0x3FF] = v
        rec = []
        for k, t in enumerate(range(0, n, 500)):
            ms.sim_event(b5, k % 8, 50 + k if k % 3 else None)
            for a in (ms.PITCH[k % 8], ms.GATE[k % 8], ms.TRIG[k % 8]):
                s.d[0].bank5[a & 0x3FF] = b5[a]
            rec += s.run(500, [(2, 0xA118)])[(2, 0xA118)]
            rec.append([list(s.d[i].bank5) for i in range(3)])
            rec.append([[s.d[i].bank0[(a - s.t) & 0x7FF] for a in range(0x200, 0x500)] for i in range(2)])
        outs.append(rec)
    assert outs[0] == outs[1]
    os.environ['STEP'] = '11'


def test_s8_poly_opt(n=12000):
    """SYSTEM-8 C-MINI2 voice (dsp/make_poly.py): the --opt variant (ph_wrap + asm/dspsched.py) against the same
    voice unscheduled, bit for bit (every bank-0 / bank-5 word and 6020 / 6021); and against poly.hex (manual wrap,
    tested on the device): the same pitch and level of each voice"""
    sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'dsp'))
    import make_poly as mp
    progs = {'sched': mp.words(mp.source_opt())[0], 'plain': mp.words(mp.source_opt(scheduled=False))[0],
             'orig': mp.words(mp.source())[0]}
    res = {}
    for name, w in progs.items():
        s = cchain.Slave({5: (chain.Program(w, name), [])})
        pitches = [3.75, 4.75, 4.0, 5.25]
        for k in range(mp.NV):
            s.p_bank(5, mp.PITCH[k] & 0x3FF, pitches[k])
        out = {}
        for k in range(mp.NV):                     # voice k alone, then all four
            for j in range(mp.NV):
                s.p_bank(5, mp.GATE[j] & 0x3FF, 1.0 if j == k else 0.0)
            out[k] = s.run(n, (0x6020, 0x6021))
        res[name] = (out, [list(s.d[5].bank5)], [list(s.d[5].bank0)])
    assert res['sched'] == res['plain']
    for k in range(4):
        a, b = res['sched'][0][k][0x6020][n // 2:], res['orig'][0][k][0x6020][n // 2:]
        fa, fb = chain.freq(a), chain.freq(b)
        assert abs(fa - fb) < 0.01 * fb and abs(chain.peak(a) - chain.peak(b)) < 0.01, (k, fa, fb)


def test_speed():
    sc = cchain.Slave(voices.programs('v4'))
    t0 = time.perf_counter()
    sc.run(20000)
    tc = (time.perf_counter() - t0) / 20000
    print(f'    v4: C {tc * 1e6:.2f} us/sample, {sc.ninstr / 20000 / tc / 1e6:.0f} M instr/s')
    if not listing.available():
        return
    s = cchain.Slave(jx_programs())
    t0 = time.perf_counter()
    s.run(48000, ())
    rt = 0.5 / (time.perf_counter() - t0)
    print(f'    JX-3P slave (native, threads {s.groups}): {rt:.2f} x real time, {s.ninstr / 0.5 / rt / 1e6:.0f} M instr/s')
    assert rt >= 1


def main(sel):
    tests = [(k, v) for k, v in globals().items() if k.startswith('test_') and callable(v)]
    if sel:
        tests = [(k, v) for k, v in tests if any(s in k for s in sel)]
    bad = 0
    for k, fn in tests:
        t0 = time.time()
        try:
            fn()
            print(f'PASS {k} ({time.time() - t0:.1f} s)', flush=True)
        except Skip as e:
            print('SKIP', k, '-', e, flush=True)
        except Exception:
            bad += 1
            print('FAIL', k, flush=True)
            traceback.print_exc()
    print(f'{len(tests) - bad}/{len(tests)} passed or skipped')
    return 1 if bad else 0


UPDATE_GOLDEN = False

if __name__ == '__main__':
    UPDATE_GOLDEN = '--update-golden' in sys.argv
    sys.exit(main([a for a in sys.argv[1:] if not a.startswith('--')]))
