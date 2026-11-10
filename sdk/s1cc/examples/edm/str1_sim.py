"""EDM_STR_ONE check without the device (docs/vol6_reference_designs_applications/mutable_memory.md §3.3.5): build/s1mm1.s (microcode/edm_str.s
master_str1) on the C emulator; the tapreq results as measured on our line (a request at age d gives, read in the
next sample, x0 = w[n - d - 1], x1 = w[n - d - 2]), the bank-0 ring as stored at a / read at a + 1 the sample after;
d and g from make_s1cc.py str1_tune for each CUTOFF step. Prints each string's pitch against the note.
    KERNEL=sdk/s1cc/build/edm_string EDM_STR_ONE=1 sh examples/system_s1cc/build.sh && python3 sdk/s1cc/examples/edm/str1_sim.py"""
import math, os, sys
import numpy as np
R = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..'))
sys.path[:0] = [R + '/asm', R + '/examples/from_scratch/emu']
import asm, dspref

def tune(n, b):                                   # = make_s1cc.py str1_tune
    p = 96000.0 / (440.0 * 2 ** ((n - 69) / 12)); w = 2 * math.pi / p; z = complex(math.cos(-w), math.sin(-w))
    lo, hi = 0.0, p
    for _ in range(60):
        t = (lo + hi) / 2; d, g = int(t), t - int(t)
        h = ((1 - g) + g * z) * b / (1 - (1 - b) * z)
        if d + 1 + (-math.atan2(h.imag, h.real)) % (2 * math.pi) / w > p: hi = t
        else: lo = t
    t = (lo + hi) / 2; return int(t), t - int(t)

A = asm.Assembler(); prog = []
for l in open(R + '/examples/system_s1cc/build/s1mm1.s'):
    l = l.strip()
    if l and not l.startswith(';'):
        r = asm.Assembler(A.table).assemble(l + '\n'); assert r.ok, (l, r.errors); prog.append(r.words)
NV, N = 6, 40000
for b in (1.0, 0.5, 0.25):
    notes = [36, 48, 60, 67, 79, 91]
    m = dspref.Machine(context='master_dsp4_efdc'); mem = m.mem
    mem[0xC01D] = b; mem[0xC01C] = 1.0
    dg = [tune(n, b) for n in notes]
    for v in range(NV):
        mem[0xC020 + v] = dg[v][1]; mem[0xC028 + v] = 0.999
    w = np.zeros((NV, N)); st = {}; cur = {}; n = [0]; X = {}
    rng = np.random.default_rng(3)
    def acc(op, a, val):
        if 0x4080 <= a < 0x40C0 and op == 'r':
            v, k = divmod(a - 0x4080, 8); i = n[0] - dg[v][0] - 1 - k
            return w[v, i] if i >= 0 else 0.0
        if 0x4040 <= a < 0x4080 and op == 'w':
            w[(a - 0x4040) // 8, n[0]] = val; return val
        if 0x0300 <= a < 0x0310:
            if op == 'w': cur[a] = val; return val
            return st.get(a - 1, 0.0)
        if op == 'r': return X.get(a, mem.get(a))
        mem[a] = val; return val
    pid = m.add(prog); m.set_access(acc)
    for s in range(N):
        n[0] = s
        for v in range(NV): X[0x6006 + v] = rng.uniform(-0.5, 0.5) if s < 200 else 0.0
        m.run_program(pid); st.update(cur); cur.clear()
    out = []
    for v in range(NV):
        seg = w[v, 20000:]; f0 = 440 * 2 ** ((notes[v] - 69) / 12)
        Xf = np.abs(np.fft.rfft(seg * np.hanning(len(seg)), 16 * len(seg))); fr = np.fft.rfftfreq(16 * len(seg), 1 / 96000)
        mk = (fr > f0 * 0.8) & (fr < f0 * 1.2); p = fr[np.argmax(Xf * mk)]
        out.append('%d:%+.1fc' % (notes[v], 1200 * np.log2(p / f0)))
    print('b %.2f  ' % b + '  '.join(out))
