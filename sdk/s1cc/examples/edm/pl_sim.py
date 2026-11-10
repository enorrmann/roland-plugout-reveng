"""EDM_STR_PL check without the device (docs/vol6_reference_designs_applications/mutable_memory.md §3.3.5): build/s1mm1.s (microcode/edm_str.s
master_pstr) on the C emulator with the tapreq results and the bank-0 ring modelled as in str1_sim.py; d, g, b, a from
make_s1cc.py pl_loop for a few cutoff / dispersion steps. Prints each string's pitch against the note.
    KERNEL=sdk/s1cc/build/edm_plstring_asm sh examples/system_s1cc/build.sh && python3 sdk/s1cc/examples/edm/pl_sim.py"""
import importlib.util, os, sys
import numpy as np
R = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..'))
sys.path[:0] = [R + '/asm', R + '/examples/from_scratch/emu']
import asm, dspref
os.environ.setdefault('KERNEL', R + '/sdk/s1cc/build/edm_plstring_asm'); os.environ['EDM_STR_PL'] = '1'
spec = importlib.util.spec_from_file_location('mk', R + '/examples/system_s1cc/make_s1cc.py')
mk = importlib.util.module_from_spec(spec); spec.loader.exec_module(mk)
A = asm.Assembler(); prog = []
for l in open(R + '/examples/system_s1cc/build/s1mm1.s'):
    l = l.strip()
    if l and not l.startswith(';'):
        r = asm.Assembler(A.table).assemble(l + '\n'); assert r.ok, (l, r.errors); prog.append(r.words)
NV, N = 6, 30000
notes = [36, 48, 60, 67, 79, 91]
for k, j in ((7, 0), (0, 0), (7, 7), (3, 4)):
    m = dspref.Machine(context='master_dsp4_efdc'); mem = m.mem; mem[0xC01C] = 1.0
    lp = [mk.pl_loop(n, k, j) for n in notes]
    for v, (d, g, b, a) in enumerate(lp):
        mem[0xC020 + v] = g; mem[0xC040 + v] = b; mem[0xC048 + v] = a; mem[0xC028 + v] = 0.9995
    w = np.zeros((NV, N)); st = {}; cur = {}; n = [0]; X = {}
    rng = np.random.default_rng(4)
    def acc(op, ad, val):
        if 0x4080 <= ad < 0x40C0 and op == 'r':
            v, q = divmod(ad - 0x4080, 8); i = n[0] - lp[v][0] - 1 - q
            return w[v, i] if i >= 0 else 0.0
        if 0x4040 <= ad < 0x4080 and op == 'w':
            w[(ad - 0x4040) // 8, n[0]] = val; return val
        if 0x0300 <= ad < 0x0360:
            if op == 'w': cur[ad] = val; return val
            return st.get(ad - 1, 0.0)
        if op == 'r': return X.get(ad, mem.get(ad))
        mem[ad] = val; return val
    pid = m.add(prog); m.set_access(acc)
    for s in range(N):
        n[0] = s
        for v in range(NV): X[0x6006 + v] = rng.uniform(-0.5, 0.5) if s < 200 else 0.0
        m.run_program(pid); st.update(cur); cur.clear()
    out = []
    for v in range(NV):
        seg = w[v, 6000:]; f0 = 440 * 2 ** ((notes[v] - 69) / 12)
        if seg.std() < 1e-5:
            out.append('%d:dead' % notes[v]); continue
        Xf = np.abs(np.fft.rfft(seg * np.hanning(len(seg)), 16 * len(seg))); fr = np.fft.rfftfreq(16 * len(seg), 1 / 96000)
        mk_ = (fr > f0 * 0.8) & (fr < f0 * 1.2); p = fr[np.argmax(Xf * mk_)]
        out.append('%d:%+.1fc' % (notes[v], 1200 * np.log2(p / f0)))
    print('cutoff step %d dispersion step %d  ' % (k, j) + '  '.join(out))
