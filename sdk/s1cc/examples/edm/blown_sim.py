"""EDM_STR_BLOWN check without the device (docs/MUTABLE_MEMORY.md §3.3.5): build/s1mm1.s (microcode/edm_str.s
master_blown) on the C emulator, the tapreq results and the bank-0 ring modelled as in str1_sim.py, a steady breath
0.8 + 0.6 noise on every voice; prints each bore's pitch against the note and the odd / even harmonic balance.
    KERNEL=sdk/s1cc/build/edm_blown sh examples/system_s1cc/build.sh && python3 sdk/s1cc/examples/edm/blown_sim.py"""
import os, sys
import numpy as np
R = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..'))
sys.path[:0] = [R + '/asm', R + '/examples/from_scratch/emu']
import asm, dspref
import importlib.util
os.environ.setdefault('KERNEL', R + '/sdk/s1cc/build/edm_blown'); os.environ['EDM_STR_BLOWN'] = '1'
spec = importlib.util.spec_from_file_location('mk', R + '/examples/system_s1cc/make_s1cc.py')
mk = importlib.util.module_from_spec(spec); spec.loader.exec_module(mk)
K = int(os.environ.get('K', '3'))            # the loss step (OSC2 COLOR >> 5)
A = asm.Assembler(); prog = []
for l in open(R + '/examples/system_s1cc/build/s1mm1.s'):
    l = l.strip()
    if l and not l.startswith(';'):
        r = asm.Assembler(A.table).assemble(l + '\n'); assert r.ok, (l, r.errors); prog.append(r.words)
NV, N = 6, 40000
P0 = float(os.environ.get("P0", "0.45"))   # the breath pressure (edm_blown.c: 0.35 .. 0.9)
notes = [40, 48, 55, 60, 67, 74]
m = dspref.Machine(context='master_dsp4_efdc'); mem = m.mem; mem[0xC01C] = 1.0
dg = []
for v, n in enumerate(notes):
    d, g, b = mk.blown_loop(n, K)
    dg.append((d, g))
    mem[0xC020 + v] = g; mem[0xC048 + v] = b
    mem[0xC040 + v] = min(30.0 * 2 ** (n / 12) / 4096, 0.98)
w = np.zeros((NV, N)); body = np.zeros((NV, N)); st = {}; cur = {}; n_ = [0]; X = {}
rng = np.random.default_rng(5)
def acc(op, ad, val):
    if 0x4080 <= ad < 0x40C0 and op == 'r':
        v, q = divmod(ad - 0x4080, 8); i = n_[0] - dg[v][0] - 1 - q
        return w[v, i] if i >= 0 else 0.0
    if 0x4040 <= ad < 0x4080 and op == 'w':
        w[(ad - 0x4040) // 8, n_[0]] = val; return val
    if 0x0300 <= ad < 0x0360:
        if op == 'w':
            cur[ad] = val
            if (ad - 0x0300) % 16 == 2: body[(ad - 0x0300) // 16, n_[0]] = val
            return val
        return st.get(ad - 1, 0.0)
    if op == 'r': return X.get(ad, mem.get(ad))
    mem[ad] = val; return val
pid = m.add(prog); m.set_access(acc)
for s in range(N):
    n_[0] = s
    for v in range(NV): X[0x6006 + v] = P0 * (1 + 0.05 * rng.uniform(-1, 1))
    m.run_program(pid); st.update(cur); cur.clear()
for v in range(NV):
    seg = w[v, 20000:] - w[v, 20000:].mean(); f0 = 440 * 2 ** ((notes[v] - 69) / 12)
    Xf = np.abs(np.fft.rfft(seg * np.hanning(len(seg)), 8 * len(seg))); fr = np.fft.rfftfreq(8 * len(seg), 1 / 96000)
    pk = lambda k: Xf[(fr > f0 * k * 0.95) & (fr < f0 * k * 1.05)].max()
    mk = (fr > f0 * 0.8) & (fr < f0 * 1.2); p = fr[np.argmax(Xf * mk)]
    print('note %d  %+.1f cents  rms %.3f  H2 %.0f H3 %.0f H5 %.0f H7 %.0f dB  body rms %.3f' % (
        notes[v], 1200 * np.log2(p / f0), seg.std(), 20 * np.log10(pk(2) / pk(1)), 20 * np.log10(pk(3) / pk(1)),
        20 * np.log10(pk(5) / pk(1)), 20 * np.log10(pk(7) / pk(1)), body[v, 20000:].std()))
