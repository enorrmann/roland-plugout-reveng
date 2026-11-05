"""EDM_WT check without the device (docs/MUTABLE_MEMORY.md §3.3.8): build/s1mm1.s (microcode/edm_wt.s master_wt) on
the C emulator with a model of the rest:
  * the loop line: a store to WT_W at sample n is w[n]; a load of WT_R gives w[n - WT_LEN] (read entry at age
    WT_LEN - 1, + 1 sample: 48000 -> 48001 measured, §3.3.4);
  * the requests: the ages stored to WT_AGE + j in sample n are requested by the block the same sample; the main reads
    in sample n + 1 x0 = w[n - a], x1 = w[n - a - 1], fraction a - floor(a) (a in samples);
  * region 0 (model.c wt_fill): changes WT_SYNC once, then puts table sample k = samples since then in WT_STG, each
    sample with probability 1 - MISS (interrupts), until every sample went in;
  * the bank-0 ring: stored at a, read at a + 1 the next sample.
Checks: the loop holds the table (each place c = k holds sample k); a held note at frame 0 and at a mid COLOR is the
table read at the note's pitch.
    KERNEL=sdk/s1cc/build/mp_saw EDM_WT=1 sh examples/system_s1cc/build.sh && python3 sdk/s1cc/examples/edm/wt_sim.py"""
import math, os, re, sys
import numpy as np
R = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..'))
sys.path[:0] = [R + '/asm', R + '/examples/from_scratch/emu']
import asm, dspref

B = R + '/examples/system_s1cc/build/'
H = open(B + 's1_programs.h').read()
L = int(re.search(r'#define WT_LEN (\d+)u', H).group(1))
F = int(re.search(r'#define WT_F (\d+)u', H).group(1))
NF = L // F
tab = np.array([int(x) for x in re.search(r'wt_table\[\d+\] = \{([^}]*)\}', H).group(1).replace('\n', ' ').split(',')],
               float) / 32767.0
A = asm.Assembler(); prog = []
for l in open(B + 's1mm1.s'):
    l = l.strip()
    if l and not l.startswith(';'):
        r = asm.Assembler(A.table).assemble(l + '\n'); assert r.ok, (l, r.errors); prog.append(r.words)

MISS = float(os.environ.get('MISS', '0.2'))
rng = np.random.default_rng(1)
m = dspref.Machine(context='master_dsp4_efdc'); mem = m.mem
mem[0xC01C] = 1.0
NTOT = L * 6 + 40000
w = np.zeros(NTOT); n = [0]; st = {}; cur = {}; X = {}; res = {}
age_now = {}; send = [0.0]

def acc(op, a, val):
    if a == 0x40C0 and op == 'w':
        w[n[0]] = val; return val
    if a == 0x40C1 and op == 'r':
        i = n[0] - L; return w[i] if i >= 0 else 0.0
    if 0x40D0 <= a < 0x40E0 and op == 'r':
        return res.get(a, 0.0)
    if a == 0x2000 and op == 'w':
        send[0] = val; return val
    if a in (0xA1C8, 0xA1C9) and op == 'w':
        age_now[a - 0xA1C8] = val; return val
    if 0x0300 <= a < 0x0310:
        if op == 'w': cur[a] = val; return val
        return st.get(a - 1, 0.0)
    if op == 'r': return X.get(a, mem.get(a))
    mem[a] = val; return val

pid = m.add(prog); m.set_access(acc)

def run(s):
    n[0] = s
    m.run_program(pid); st.update(cur); cur.clear()
    res.clear()                                           # this sample's requests -> next sample's results
    for j in (0, 1):
        a = age_now.get(j, 0.0) * 16384.0
        ai = math.floor(a); fr = a - ai
        res[0x40D0 + 8 * j] = w[s - ai] if 0 <= s - ai < NTOT else 0.0
        res[0x40D1 + 8 * j] = w[s - ai - 1] if 0 <= s - ai - 1 < NTOT else 0.0
        res[0x40D2 + 8 * j] = fr

# region 0's fill: the sync at sample S0, then sample k = s - S0 (mod L) when not missed
S0, done, par, s = 100, np.zeros(L, bool), 0, 0
X[0xA1C1] = 0.0; X[0xA1C0] = 0.0
mem[0xC033] = 0.0
while s < NTOT:
    if s == S0:
        X[0xA1C1] = 1.0
    elif s > S0 and not done.all():
        k = (s - S0) % L
        if not done[k] and rng.random() > MISS:
            par ^= 1
            X[0xA1C0] = (32 * round(tab[k] * 32767) + (1 if par else -1)) / 2 ** 21
            done[k] = True
    elif done.all():
        break
    run(s); s += 1
fill_end = s
laps = (fill_end - S0) / L
# the loop's content after the fill: place c = k (c = 0 at the sync sample) holds sample k, now and a lap later
for t0 in (fill_end, fill_end + L):
    while s < t0:
        run(s); s += 1
    got = np.array([w[s - L + i] for i in range(L)])
    k = (np.arange(s - L, s) - S0) % L
    err = np.max(np.abs(got - np.array([(32 * round(tab[i] * 32767)) / 2 ** 21 for i in k])))
    print(f'loop after {laps:.2f} laps of filling (+{(t0 - fill_end) // L} lap): max |error| {err:.2e}')
# a held note
def note(midi, frame_q, mixv, N=20000):
    global s
    f = 440 * 2 ** ((midi - 69) / 12)
    mem[0xC030] = f * F / 96000 / 16384
    mem[0xC031] = frame_q * F / 16384
    mem[0xC032] = mixv
    mem[0xC033] = 1.0
    out = []
    for _ in range(N):
        run(s); out.append(send[0]); s += 1
    y = np.array(out[4000:])
    ph = (np.arange(len(y)) * f * F / 96000) % F
    ref = (1 - mixv) * np.interp(ph, np.arange(F), tab[frame_q * F:(frame_q + 1) * F]) + \
        mixv * np.interp(ph, np.arange(F), tab[(frame_q + 1) * F:(frame_q + 2) * F])
    # the same wave at any phase: the best normalized correlation over one period of shifts
    c = max(np.dot(y, np.roll(ref, d)) for d in range(0, int(96000 / f), 2))
    corr = c / (np.linalg.norm(y) * np.linalg.norm(ref) + 1e-30)
    Y = np.abs(np.fft.rfft(y * np.hanning(len(y)), 8 * len(y))); fr = np.fft.rfftfreq(8 * len(y), 1 / 96000)
    p = fr[np.argmax(Y * (fr > f * 0.8) * (fr < f * 1.2))]
    rms = np.sqrt(np.mean(y ** 2))
    print(f'note {midi} frame {frame_q} mix {mixv}: pitch {1200 * np.log2(p / f):+.1f} cents, rms {rms:.4f}, shape correlation {corr:.4f}')
note(60, 0, 0.0)
note(48, NF // 2, 0.5)
