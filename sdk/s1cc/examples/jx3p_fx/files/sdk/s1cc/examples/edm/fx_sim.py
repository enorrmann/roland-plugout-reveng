"""FX_KERNEL check without the device (examples/system_s1cc_boutique README.md §1.9): the master B program SYSTEM-S1CC-
BOUTIQUE built with a master effect (OUTDIR/s1mm1.s, make_s1cc.py master_fx) on the C emulator, with the rest modelled
as km_sim.py does for S1_MASTER:
  * bank 0 (0000..07ff) is a ring: a word stored at X in sample n is read at X + 1 in sample n + 1, at X in sample n;
  * line k (written at STR_W + 8k): its request (edm_call.s edm_km_block) takes the age a the main stored at km_age(k)
    in the sample before (read at + 1) and gives x_q = w[n - floor(a) - 1 - q] at STR_RES + 8k + q (q = 0, 1) and the
    fraction floor(64 (a - floor a)) / 64 at + 2 (the hardware's, 1/64 truncated; s1rt.c models the same);
  * the master parameters as region 0 writes them (model.c STR_KM, MASTER_FX): the S1_PARAM columns at their knob,
    the immediates; VOLUME 1;
  * 6006 = the effect's input, 6008 = the noise word (s1_fx_noise): the native reference's own sequences.
The check: the program's left (FX_SEND 2000) and right (2002) against the native reference (ref.raw.m / ref.raw.r of
the effect's s1cc build: S1_MASTER on its S1_DSP_A test signal), sample by sample, SNR.

    sdk/s1cc/s1cc build sdk/s1cc/examples/jx3p_ported/jx3p_fx.c
    KERNEL=<a voice build> FX_KERNEL=sdk/s1cc/build/jx3p_fx OUTDIR=DIR python3 examples/system_s1cc_boutique/make_s1cc.py
    python3 sdk/s1cc/examples/edm/fx_sim.py DIR [N] [name=knob ...]       (KERNEL / FX_KERNEL as for the build)"""
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
if not args:
    raise SystemExit(__doc__)
OUTDIR = os.path.abspath(args[0])
N = int(args[1]) if len(args) > 1 else 300000
spec = importlib.util.spec_from_file_location('mk', R + '/examples/system_s1cc_boutique/make_s1cc.py')
mk = importlib.util.module_from_spec(spec)
sys.argv = [sys.argv[0]]                          # make_s1cc.py reads no arguments on import
spec.loader.exec_module(mk)
if not mk.MFX:
    raise SystemExit('fx_sim.py: FX_KERNEL is not set (the build\'s effect)')
FXDIR = mk.FX_KERNEL

prog = []
A = asm.Assembler()
for line in open(os.path.join(OUTDIR, 's1mm1.s')):
    line = line.strip()
    if line and not line.startswith(';'):
        r = asm.Assembler(A.table).assemble(line + '\n')
        assert r.ok, (line, r.errors)
        prog.append(r.words)
LINES = mk.STR_LPV
ids = mk.km_ids()
AGE = [mk.km_age(k) for k in range(LINES)]
SEND = mk.FX_SEND


def params(mem):
    mem[0xC000 + mk.M_VOLUME] = 1.0
    for key, i in ids['single'].items():
        if key[0] == 'P':
            p = next(q for q in mk.MK.mparams if q['name'] == key[1])
            _, cnt, vals = mk.MTAB[key[1]]
            mem[0xC000 + i] = vals[int(knobs.get(key[1], 128)) * cnt + key[2]] / p['scale']
        else:
            mem[0xC000 + i] = float(key[1])


def noise_seq(n):
    """s1rt.c s1_fx_noise: x = 1664525 x + 1013904223 (mod 2^32) from 22222, as int32 / 2^31"""
    out, x = np.zeros(n), 22222
    for i in range(n):
        x = (x * 1664525 + 1013904223) & 0xFFFFFFFF
        out[i] = (x - (1 << 32) if x >= 1 << 31 else x) / 2147483648.0
    return out


def run(xin, nz, n):
    m = dspref.Machine(context='master_dsp4_efdc')
    mem = m.mem
    params(mem)
    w = np.zeros((LINES, n))
    ring, X, s_ = {}, {}, [0]
    out = np.zeros((2, n))
    req = {}

    def acc(op, ad, val):
        s = s_[0]
        if ad < 0x0800:
            key = (ad - s) & 0x7FF
            if op == 'w':
                ring[key] = val
                return val
            return ring.get(key, 0.0)
        if mk.STR_RES <= ad < mk.STR_RES + 8 * LINES and op == 'r':
            k, q = divmod(ad - mk.STR_RES, 8)
            a = req[k]
            if q == 2:
                return np.floor((a - np.floor(a)) * 64) / 64
            i = s - int(np.floor(a)) - 1 - q
            return w[k, i] if 0 <= i < n else 0.0
        if mk.STR_W <= ad < mk.STR_W + 8 * LINES and op == 'w' and (ad - mk.STR_W) % 8 == 0:
            w[(ad - mk.STR_W) // 8, s] = val
            return val
        if op == 'w' and ad in SEND:
            out[SEND.index(ad), s] = val
        if op == 'r':
            return X.get(ad, mem.get(ad))
        mem[ad] = val
        return val
    pid = m.add(prog)
    m.set_access(acc)
    for s in range(n):
        s_[0] = s
        for k in range(LINES):                      # the request block: the age stored in the sample before
            req[k] = ring.get((AGE[k] + 1 - s) & 0x7FF, 0.0) * 16384
        X[0x6006] = xin[s]
        X[mk.FX_NOISE] = nz[s]
        m.run_program(pid)
    return out


ref = os.path.join(OUTDIR, 'fx_ref.raw')
kargs = [f'{k}={v}' for k, v in knobs.items()]
subprocess.run([os.path.join(FXDIR, 'ref'), 'ref', ref, '57', str(N)] + kargs, check=True)
xb = np.fromfile(ref, dtype='<f4').reshape(-1, 2)[:, 1]
yl, yr = np.fromfile(ref + '.m', dtype='<f4'), np.fromfile(ref + '.r', dtype='<f4')
out = run(xb, noise_seq(N), N)
for name, y, yref in (('left', out[0], yl), ('right', out[1], yr)):
    err = y - yref
    snr = 10 * np.log10(np.sum(yref ** 2) / max(np.sum(err ** 2), 1e-30))
    wet = yref - xb                                  # the effect's own part (dry = the input)
    werr = 10 * np.log10(np.sum(wet ** 2) / max(np.sum(err ** 2), 1e-30))
    print(f'{name}: SNR {snr:.1f} dB (of the effect alone: {werr:.1f} dB), max |err| {np.max(np.abs(err)):.2e}, '
          f'peak {np.max(np.abs(y)):.3f} / ref {np.max(np.abs(yref)):.3f}')
np.save(os.path.join(OUTDIR, 'fx_sim.npy'), np.vstack([out, [yl, yr]]))
