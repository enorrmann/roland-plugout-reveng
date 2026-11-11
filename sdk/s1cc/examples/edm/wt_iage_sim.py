"""WT_IAGE_TEST check without the device (docs/vol6_reference_designs_applications/mutable_memory.md §4.4, the read
noise): examples/system_s1cc/build/s1mm1.s (edm_wt.s master_wt_s with WT_IAGE_TEST=1) on the C emulator, the three
reads the selector picks (C048 / C049 / C04A one-hot): 0 = the tapreq's fraction with a fractional age (every build
before), 1 / 2 = our fraction 1 - q of one / two samples before with a whole age. Model (wt_sim.py's):
  * the loop line already holds the table (loop of WT_LEN samples, WT_R = w[n - WT_LEN]); no fill (index slots never
    match);
  * a request with the age a (samples) stored in sample n gives in sample n + 1 x0 = w[n - A], x1 = w[n - A - 1],
    fraction (a - A) truncated to 1/64, A = floor(a) (LAG=2: in sample n + 2);
  * the bank-0 ring: stored at X in sample n, read at X + 1 in sample n + 1;
  * voice 0 plays NOTE on frame FRAME (slave word 6008), its output on 600E.
Prints, per read, the non-harmonic part of the held note (least squares on every harmonic of the exact f0) in dB.
With the model's lag (1) read 0 and read 1 must agree, read 2 must be worse; on the device the read that comes out
clean says which lag is true and whether the fractional age was the noise.
    WT_WAV=".../80's PWM.wav" WT_LINES=0 WT_IAGE_TEST=1 NAME=WT-IAGE-T KERNEL=sdk/s1cc/build/wt_edm_cmod \\
        sh examples/system_s1cc/build.sh && python3 sdk/s1cc/examples/edm/wt_iage_sim.py"""
import math
import os
import re
import sys

import numpy as np

R = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..'))
sys.path[:0] = [R + '/asm', R + '/examples/from_scratch/emu']
import asm  # noqa: E402
import dspref  # noqa: E402

B = R + '/examples/system_s1cc/build/'
H = open(B + 's1_programs.h').read()
assert '#define WT_IAGE_TEST 1' in H, 'build with WT_IAGE_TEST=1 first'
L = int(re.search(r'#define WT_LEN (\d+)u', H).group(1))
F = int(re.search(r'#define WT_F (\d+)u', H).group(1))
tab = np.array([int(x) for x in re.search(r'wt_table\[\d+\] = \{([^}]*)\}', H).group(1).replace('\n', ' ').split(',')],
               float) / 65536.0                       # wt_fill: int16 x 2^-16 (peak 0.5)
A = asm.Assembler()
prog = []
for line in open(B + 's1mm1.s'):
    line = line.strip()
    if line and not line.startswith(';'):
        r = asm.Assembler(A.table).assemble(line + '\n')
        assert r.ok, (line, r.errors)
        prog.append(r.words)
NOTE, FRAME = int(os.environ.get('NOTE', '48')), int(os.environ.get('FRAME', '12'))
LAG = int(os.environ.get('LAG', '1'))
N = int(os.environ.get('N', '40000'))
WT_S, WT_AGEB, WT_RES, WT_W, WT_R, WT_M2S = 0x0300, 0x0340, 0x4080, 0x4040, 0x4041, 0x600E
NV = int(re.search(r'#define WT_NV (\d+)', H).group(1))


def run(mode):
    m = dspref.Machine(context='master_dsp4_efdc')
    mem = m.mem
    dt = 440.0 * 2 ** ((NOTE - 69) / 12) * F / 96000.0          # table samples per sample
    p = {0x1C: 1.0, 0x38: (L - 0.5) / 16384, 0x39: L / 32768, 0x36: 0.0, 0x3E: 0.0, 0x3F: 0.0,
         0x3A: -1.0, 0x3B: 0.0, 0x3C: -1.0, 0x3D: 0.0}
    for v in range(NV):
        p[0x30 + v] = math.floor(dt) / 16384 if v == 0 else 0.0
        p[0x40 + v] = dt - math.floor(dt) if v == 0 else 0.0
    for k in range(3):
        p[0x48 + k] = 1.0 if k == mode else 0.0
    for k, x in p.items():
        mem[0xC000 + k] = x
    off = L + 4096
    w = np.zeros(off + N)
    w[:off] = tab[(np.arange(off) - off) % L]                     # the loop holds the table: w[off + n] ~ c = n
    ring, X, s_ = {}, {}, [0]
    pend = {}                                                     # sample -> {voice: (A, frac)}
    out = np.zeros(N)

    def acc(op, ad, val):
        n = s_[0]
        if ad < 0x0800:
            key = (ad - n) & 0x7FF
            if op == 'w':
                ring[key] = val
                return val
            return ring.get(key, 0.0)
        if ad == WT_R and op == 'r':
            return w[off + n - L]
        if ad == WT_W and op == 'w':
            w[off + n] = val
            return val
        if WT_RES <= ad < WT_RES + 8 * NV and op == 'r':
            v, q = divmod(ad - WT_RES, 8)
            a, fr = pend.get(n, {}).get(v, (0, 0.0))
            if q == 2:
                return fr
            i = off + n - LAG - a - q                             # w[n' - A - q], n' = the request's sample
            return w[i] if 0 <= i < len(w) else 0.0
        if WT_M2S <= ad < WT_M2S + 8 and op == 'w':
            if ad == WT_M2S:
                out[n] = val
            return val
        if op == 'r':
            return X.get(ad, mem.get(ad))
        mem[ad] = val
        return val
    pid = m.add(prog)
    m.set_access(acc)
    X[0x6008] = FRAME * F / 16384
    for n in range(N):
        s_[0] = n
        m.run_program(pid)
        for v in range(NV):                                       # the request block: the age stored this sample
            a = ring.get((WT_AGEB + 4 * v + 1 - n) & 0x7FF, 0.0) * 16384
            A_ = math.floor(a + 1e-9)
            pend.setdefault(n + LAG, {})[v] = (A_, math.floor((a - A_) * 64) / 64)
    return out, dt


def nonharmonic_db(y, f0):
    seg = y[len(y) // 2:]
    t = np.arange(len(seg))
    K = int(48000 / f0)
    cols = [np.ones_like(seg, dtype=float)]
    for k in range(1, K + 1):
        cols += [np.cos(2 * np.pi * k * f0 * t / 96000), np.sin(2 * np.pi * k * f0 * t / 96000)]
    M = np.array(cols).T
    c, *_ = np.linalg.lstsq(M, seg, rcond=None)
    res = seg - M @ c
    return 10 * np.log10(np.sum(res ** 2) / max(np.sum((seg - seg.mean()) ** 2), 1e-30))


outs = {}
for mode in (0, 1, 2):
    y, dt = run(mode)
    outs[mode] = y
    f0 = 440.0 * 2 ** ((NOTE - 69) / 12)
    print(f'read {mode} ({["tapreq fraction, fractional age", "1 - q one sample before", "1 - q two samples before"][mode]}): '
          f'non-harmonic {nonharmonic_db(y, f0):.1f} dB, rms {np.sqrt(np.mean(y[N // 2:] ** 2)):.4f}')
d = outs[0][N // 2:] - outs[1][N // 2:]
print(f'read 0 vs read 1: {10 * np.log10(np.sum(d ** 2) / max(np.sum(outs[0][N // 2:] ** 2), 1e-30)):.1f} dB (model lag {LAG})')
