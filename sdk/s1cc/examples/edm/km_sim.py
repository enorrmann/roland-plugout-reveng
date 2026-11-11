"""S1_MASTER check without the device (docs/vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md §3.5): the master B
program SYSTEM-S1CC built from a kernel's S1_MASTER (examples/system_s1cc/build/s1mm1.s, make_s1cc.py master_km) on
the C emulator, with the delay memory and the bank-0 ring modelled:
  * bank 0 (0000..07ff) is a ring: a word stored at X in sample n is read at X + 1 in sample n + 1, at X in sample n;
  * line k (written at STR_W + 8k): its request (edm_call.s edm_km_block) takes the age the main stored at km_age(k)
    in the sample before (read at + 1) and gives x_q = w[n - d - 1 - q] at STR_RES + 8k + q (q = 0, 1), the relation
    of str1_sim.py / pl_sim.py (MP-EDMSTR / MP-PLSTRING tuned on the device with it);
  * the master parameters as region 0 writes them (model.c STR_KM): the voice's S1_VOICE_PARAM rows at its note and
    the knobs' steps, the S1_PARAM columns at their knob, the immediates, the gates.
Two checks:
  1. the program against the native reference (the C on the PC): voice 0 gets the reference's own excitation (ref.raw,
     the slave's output) and its output must follow ref.raw.m sample by sample;
  2. one string per voice (notes 36 .. 91, a noise burst each): each pitch against its note, in cents.

    sdk/s1cc/s1cc build sdk/s1cc/examples/edm/edm_plstring.c
    KERNEL=sdk/s1cc/build/edm_plstring sh examples/system_s1cc/build.sh
    python3 sdk/s1cc/examples/edm/km_sim.py [KERNEL_DIR] [name=knob ...]   (name.a / name.b: a voice table's knobs)"""
import importlib.util
import os
import subprocess
import sys

import numpy as np

R = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..'))
sys.path[:0] = [R + '/asm', R + '/examples/from_scratch/emu']
import asm  # noqa: E402
import dspref  # noqa: E402

args = [a for a in sys.argv[1:] if '=' not in a]
knobs = dict(a.split('=', 1) for a in sys.argv[1:] if '=' in a)
KDIR = os.path.abspath(args[0] if args else os.environ.get('KERNEL', R + '/sdk/s1cc/build/edm_plstring'))
os.environ['KERNEL'] = KDIR
spec = importlib.util.spec_from_file_location('mk', R + '/examples/system_s1cc/make_s1cc.py')
mk = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mk)
if not mk.KM:
    raise SystemExit(f'km_sim.py: {KDIR} has no S1_MASTER')

A = asm.Assembler()
prog = []
for line in open(R + '/examples/system_s1cc/build/s1mm1.s'):
    line = line.strip()
    if line and not line.startswith(';'):
        r = asm.Assembler(A.table).assemble(line + '\n')
        assert r.ok, (line, r.errors)
        prog.append(r.words)
NV, LPV = mk.str_nv(), mk.STR_LPV
ids = mk.km_ids()
off, size = mk.km_words()
OUT = [mk.KM_S + size * v + off['OUT'] for v in range(NV)]
AGE = [mk.km_age(k) for k in range(NV * LPV)]


def knob(name, default=128):
    return int(knobs.get(name, default))


def params(mem, notes, gate):
    """the master parameters as region 0 writes them (model.c STR_KM)"""
    mem[0xC000 + mk.M_VOLUME] = 1.0
    for t in mk.KDEF.vparams:
        (pa, pb), cnt, vals = mk.KTAB[t['name']]
        a = knob(t['name'] + '.a') >> 5 if pa else 4
        b = knob(t['name'] + '.b') >> 5 if pb else 4
        for v, n in enumerate(notes):
            row = (n * 8 + a) * 8 + b
            for i in range(cnt):
                mem[0xC000 + ids['vcol'][(t['name'], i)] + v] = vals[row * cnt + i] / t['scales'][i]
    for key, i in ids['single'].items():
        if key[0] == 'P':
            p = next(q for q in mk.KDEF.mparams if q['name'] == key[1])
            _, cnt, vals = mk.KTAB[key[1]]
            mem[0xC000 + i] = vals[knob(key[1]) * cnt + key[2]] / p['scale']
        else:
            mem[0xC000 + i] = float(key[1])
    for v in range(NV):
        mem[0xC000 + ids['gate'] + v] = gate


def run(notes, xin, n):
    """n samples; xin(s, v): voice v's input on 6006 + v; returns the voices' outputs [NV, n] and the lines' writes"""
    m = dspref.Machine(context='master_dsp4_efdc')
    mem = m.mem
    params(mem, notes, 1.0)
    w = np.zeros((NV * LPV, n))
    ring, X, s_ = {}, {}, [0]
    out = np.zeros((NV, n))
    req = {}                                        # line -> d of this sample's request

    def acc(op, ad, val):
        s = s_[0]
        if ad < 0x0800:                             # the bank-0 ring
            key = (ad - s) & 0x7FF
            if op == 'w':
                ring[key] = val
                if ad in OUT:
                    out[OUT.index(ad), s] = val
                return val
            return ring.get(key, 0.0)
        if mk.STR_RES <= ad < mk.STR_RES + 8 * NV * LPV and op == 'r':
            k, q = divmod(ad - mk.STR_RES, 8)
            i = s - req[k] - 1 - q
            return w[k, i] if 0 <= i < n else 0.0
        if mk.STR_W <= ad < mk.STR_W + 8 * NV * LPV and op == 'w' and (ad - mk.STR_W) % 8 == 0:
            w[(ad - mk.STR_W) // 8, s] = val
            return val
        if op == 'r':
            return X.get(ad, mem.get(ad))
        mem[ad] = val
        return val
    pid = m.add(prog)
    m.set_access(acc)
    for s in range(n):
        s_[0] = s
        for k in range(NV * LPV):                   # the request block: the age stored in the sample before
            req[k] = int(round(ring.get((AGE[k] + 1 - s) & 0x7FF, 0.0) * 16384))
        for v in range(NV):
            X[0x6006 + v] = xin(s, v)
        m.run_program(pid)
    return out, w


# 1. against the native reference (voice 0, note 57; the reference's ref writes ref.raw and ref.raw.m)
N1, NOTE = 20000, 57
ref = os.path.join(KDIR, 'ref.raw')
kargs = [f'{k}={v}' for k, v in knobs.items()]
subprocess.run([os.path.join(KDIR, 'ref'), 'ref', ref, str(NOTE), str(N1)] + kargs, check=True)
xb = np.fromfile(ref, dtype='<f4').reshape(-1, 2)[:, 1]
ym = np.fromfile(ref + '.m', dtype='<f4')
out, _ = run([NOTE] * NV, lambda s, v: float(xb[s]) if v == 0 else 0.0, N1)
y = out[0]
err = y - ym
snr = 10 * np.log10(np.sum(ym ** 2) / max(np.sum(err ** 2), 1e-30))
print(f'vs the native reference (note {NOTE}, {N1} samples): SNR {snr:.1f} dB, max |err| {np.max(np.abs(err)):.2e}, '
      f'peak {np.max(np.abs(y)):.3f} / ref {np.max(np.abs(ym)):.3f}')

# 2. one string per voice: pitch against the note
N2 = 30000
notes = [36, 48, 60, 67, 79, 91][:NV]
rng = np.random.default_rng(4)
burst = rng.uniform(-0.5, 0.5, (200, NV))
out, w = run(notes, lambda s, v: float(burst[s, v]) if s < 200 else 0.0, N2)
res = []
for v in range(NV):
    seg = out[v, 6000:]
    f0 = 440 * 2 ** ((notes[v] - 69) / 12)
    if seg.std() < 1e-6:
        res.append('%d:dead' % notes[v])
        continue
    F = np.abs(np.fft.rfft(seg * np.hanning(len(seg)), 16 * len(seg)))
    fr = np.fft.rfftfreq(16 * len(seg), 1 / 96000)
    band = (fr > f0 * 0.8) & (fr < f0 * 1.2)
    p = fr[np.argmax(F * band)]
    res.append('%d:%+.1fc' % (notes[v], 1200 * np.log2(p / f0)))
print('pitches: ' + '  '.join(res))
