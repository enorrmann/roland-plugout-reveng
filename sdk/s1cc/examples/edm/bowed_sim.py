"""EDM_STR_BOWED check without the device (docs/MUTABLE_MEMORY.md §3.3.5): build/s1mm1.s (microcode/edm_str.s
master_bowed) on the C emulator; 6 lines (neck, bridge per voice), the tapreq results and the bank-0 ring modelled as
in str1_sim.py; a steady bow (BOW, default 0.16) on 3 voices; prints each voice's pitch against the note.
    KERNEL=sdk/s1cc/build/edm_bowed sh examples/system_s1cc/build.sh && python3 sdk/s1cc/examples/edm/bowed_sim.py"""
import importlib.util, os, sys
import numpy as np
R = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..'))
sys.path[:0] = [R + '/asm', R + '/examples/from_scratch/emu']
import asm, dspref
os.environ.setdefault('KERNEL', R + '/sdk/s1cc/build/edm_bowed'); os.environ['EDM_STR_BOWED'] = '1'
spec = importlib.util.spec_from_file_location('mk', R + '/examples/system_s1cc/make_s1cc.py')
mk = importlib.util.module_from_spec(spec); spec.loader.exec_module(mk)
A = asm.Assembler(); prog = []
for l in open(R + '/examples/system_s1cc/build/s1mm1.s'):
    l = l.strip()
    if l and not l.startswith(';'):
        r = asm.Assembler(A.table).assemble(l + '\n'); assert r.ok, (l, r.errors); prog.append(r.words)
NV, N = 3, int(os.environ.get("N", "40000"))
BOW = float(os.environ.get('BOW', '0.16')); K = int(os.environ.get('K', '3')); KNOB = int(os.environ.get('KNOB', '128'))
notes = [int(x) for x in os.environ.get('NOTES', '43 55 67').split()]
m = dspref.Machine(context='master_dsp4_efdc'); mem = m.mem; mem[0xC01C] = 1.0
ages = []
for v, n in enumerate(notes):
    an, gn, ab, gb = mk.bowed_loop(n, K, float(os.environ['SHARE']) if 'SHARE' in os.environ else None)
    ages += [an, ab]
    mem[0xC020 + 2 * v] = gn; mem[0xC020 + 2 * v + 1] = gb; mem[0xC040 + v] = float(os.environ['S']) if 'S' in os.environ else (172 - KNOB / 2) / 32
LAT = int(os.environ.get('LAT', '1'))   # one more sample per line than the table model: measured on hw 2026-11-04
HAIR = float(os.environ.get('HAIR', '0.08')); SWAP = os.environ.get('SWAP') == '1'   # x1 the newer sample
SIGN = float(os.environ.get('SIGN', '1')); rng = np.random.default_rng(7); nz = [0.0] * 3
w = np.zeros((2 * NV, N)); out = np.zeros(N); st = {}; cur = {}; n_ = [0]; X = {}
def acc(op, ad, val):
    if 0x4080 <= ad < 0x40B0 and op == 'r':
        k, q = divmod(ad - 0x4080, 8); i = n_[0] - ages[k] - 1 - LAT - (1 - q if SWAP and q < 2 else q)
        return w[k, i] if i >= 0 else 0.0
    if 0x4040 <= ad < 0x4070 and op == 'w':
        w[(ad - 0x4040) // 8, n_[0]] = val; return val
    if 0x0300 <= ad < 0x0360:
        if op == 'w': cur[ad] = val; return val
        return st.get(ad - 1, 0.0)
    if op == 'r': return X.get(ad, mem.get(ad))
    mem[ad] = val; return val
pid = m.add(prog); m.set_access(acc)
for s in range(N):
    n_[0] = s
    for v in range(NV):
        nz[v] += 0.12 * (rng.uniform(-1, 1) - nz[v])              # edm_bowed.c: the hair's noise, +-8 %
        X[0x6006 + v] = SIGN * BOW * min(1.0, s / 2000) * (1 + HAIR * nz[v])
    m.run_program(pid); st.update(cur); cur.clear(); out[s] = mem.get(0x2000)
for v in range(NV):
    seg = w[2 * v + 1, N // 2:] - w[2 * v + 1, N // 2:].mean(); f0 = 440 * 2 ** ((notes[v] - 69) / 12)
    if seg.std() < 1e-6:
        print('note %d: no oscillation' % notes[v]); continue
    Xf = np.abs(np.fft.rfft(seg * np.hanning(len(seg)), 8 * len(seg))); fr = np.fft.rfftfreq(8 * len(seg), 1 / 96000)
    pk = lambda k: Xf[(fr > f0 * k * 0.95) & (fr < f0 * k * 1.05)].max()
    mk_ = (fr > f0 * 0.8) & (fr < f0 * 1.2); p = fr[np.argmax(Xf * mk_)]
    print('note %d  %+.1f cents  rms %.3f  H2 %.0f H3 %.0f H4 %.0f dB' % (notes[v], 1200 * np.log2(p / f0), seg.std(),
          20 * np.log10(pk(2) / pk(1)), 20 * np.log10(pk(3) / pk(1)), 20 * np.log10(pk(4) / pk(1))))
# the period of the sum, by autocorrelation (the 3 notes are octaves apart: the lowest one's period, if it sounds)
for v in range(NV):
    x = w[2 * v + 1, N // 2:] - w[2 * v + 1, N // 2:].mean()
    ac = np.fft.irfft(np.abs(np.fft.rfft(x, 4 * len(x))) ** 2)[:6000]
    P = 96000 / (440 * 2 ** ((notes[v] - 69) / 12)); lag = 20 + np.argmax(ac[20:int(P * 1.5)])
    X2 = np.abs(np.fft.rfft(x * np.hanning(len(x)), 8 * len(x))); f2 = np.fft.rfftfreq(8 * len(x), 1 / 96000)
    f0 = 96000 / P; h = [X2[(f2 > f0 * k * 0.97) & (f2 < f0 * k * 1.03)].max() for k in (1, 2, 3)]
    mode = 'fundamental' if h[0] > h[1] else 'OCTAVE (H2 %+.0f dB over H1)' % (20 * np.log10(h[1] / h[0]))
    print('note %d: period %.1f samples (note %.1f): %s  H2 %+.0f H3 %+.0f dB' % (notes[v], lag, P, mode,
          20 * np.log10(h[1] / h[0]), 20 * np.log10(h[2] / h[0])))
o = out[N // 2:] - out[N // 2:].mean()
Xo = np.abs(np.fft.rfft(o * np.hanning(len(o)), 8 * len(o))); fo = np.fft.rfftfreq(8 * len(o), 1 / 96000)
for v in range(NV):
    f0 = 440 * 2 ** ((notes[v] - 69) / 12)
    hs = [Xo[(fo > f0 * k * 0.98) & (fo < f0 * k * 1.02)].max() for k in range(1, 7)]
    print('output near note %d H1..6: %s' % (notes[v], ' '.join('%+.0f' % (20 * np.log10(h / max(hs))) for h in hs)))
print('send rms %.3f' % out[N // 2:].std())
