"""s1cc check: a built kernel on the emulator against the native reference of the same C / C++ (docs/S1CC.md §10).

    sdk/s1cc/s1cc check OUTDIR [--note 57] [--n 9600] [name=knob ...] [--show]

Bench: slave DSP A (emulator DSP 0) runs stage A, slave DSP B (DSP 1) stage B, one shared bank 5 (the SYSTEM-1
slave pair); pitch word at A030, gate A038 = 1, trigger A060 = 1 (one note start, as the reference); stage A's
output to A151 (= stage B's input), exports at A1A0 + i, inputs for B at A1B0 + i, B's output to A1C0; state words
in each DSP's bank 0 from 0x0100 (2 words each), scratch words from 0x0300; parameters from C040 in .param order, on both DSPs. Programs
scheduled with asm/dspsched.py (only the proven op groups unless --all-forms). Prints the SNR over the first
10 ms and over all, the band levels port - reference, and the instruction counts.
"""
import argparse
import os
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
for p in (os.path.join(ROOT, 'asm'), os.path.join(ROOT, 'examples', 'from_scratch', 're'),
          os.path.join(ROOT, 'examples', 'from_scratch', 'emu'), HERE):
    if p not in sys.path:
        sys.path.insert(0, p)
import numpy as np  # noqa: E402

import asm  # noqa: E402
import dspsched  # noqa: E402
from s1kernel import Kernel, read_tables  # noqa: E402

PITCH, GATE, TRIG, A_OUT, EXP0, INB0, B_OUT = 0xA030, 0xA038, 0xA060, 0xA151, 0xA1A0, 0xA1B0, 0xA1C0
STATE0, PARAM0, SCRATCH0 = 0x0100, 0x40, 0x0300


def layout(k):
    addr = {'IN:pitch': PITCH, 'IN:gate': GATE, 'IN:trig': TRIG, 'XIN': A_OUT}
    for s in k.states:
        addr[f'S{s["index"]}'] = STATE0 + 2 * s['index']
    i = PARAM0
    for p in k.params:
        for j in range(p['count']):
            addr[f'P:{p["name"]}:{j}'] = 0xC000 + i + j
        i += p['count']
    for e in k.exports:
        addr[f'X:{e}'] = EXP0 + e
    for j, name in enumerate(k.inputs_b):
        addr[f'INB:{name}'] = INB0 + j
    return addr


def program(lines, proven=True):
    code = dspsched.schedule(lines, dspsched.load_proven() if proven else None)
    r = asm.Assembler().assemble('\n'.join(code) + '\n')
    if not r.ok:
        raise SystemExit(r.errors)
    return [0x0A02, 0xFF71] + r.words + [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001], code


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def run_kernel(outdir, note, n, knobs, proven=True):
    import chain
    import cchain
    k = Kernel.read(os.path.join(outdir, 'voice.s1k'))
    tables = read_tables(os.path.join(outdir, 'tables.txt'))
    base = layout(k)
    progs, info = {}, {}
    for st, dsp in (('A', 0), ('B', 1)):
        if st not in k.stages:
            continue
        addr = dict(base, OUT=A_OUT if st == 'A' else B_OUT)
        for j in range(k.stages[st]['scratch']):
            addr[f'T{j}'] = SCRATCH0 + 2 * j
        if st == 'B':
            for j, name in enumerate(k.inputs_b):
                addr[f'IN:{name}'] = INB0 + j
        words, code = program(k.place(st, addr), proven)
        progs[dsp] = (chain.Program(words, st), [])
        info[st] = (k.stages[st]['units'], len(code))
    model = chain.Model(order=tuple(sorted(progs)), shared5=((0, 1),) if len(progs) == 2 else (),
                        own_params=tuple(sorted(progs)))
    sl = cchain.Slave(progs, model)
    for dsp in progs:
        i = PARAM0
        for p in k.params:
            for j, v in enumerate(k.param_values(p['name'], knobs.get(p['name'], 128), tables)):
                sl.d[dsp].params[0xC000 + i + j] = f32(v)
            i += p['count']
    b5 = sl.d[0].bank5
    b5[PITCH & 0x3FF] = f32((note - 12) / 12.0)
    b5[GATE & 0x3FF] = 1.0
    b5[TRIG & 0x3FF] = 1.0
    words = [(0, A_OUT)] + ([(1, B_OUT)] if 1 in progs else [])
    rec = sl.run(n, words)
    return np.array(rec[(0, A_OUT)]), (np.array(rec[(1, B_OUT)]) if 1 in progs else None), info


def run_ref(outdir, note, n, knobs):
    out = os.path.join(outdir, 'ref.raw')
    exe = os.path.join(outdir, 'refdsp')
    exe = exe if os.path.exists(exe) else os.path.join(outdir, 'ref')
    subprocess.run([exe, 'ref', out, str(note), str(n)]
                   + [f'{k}={v}' for k, v in knobs.items()], check=True)
    ab = np.fromfile(out, dtype='<f4').reshape(-1, 2)
    return ab[:, 0].astype(float), ab[:, 1].astype(float)


def snr(p, r):
    e = np.mean((p - r) ** 2)
    return float('inf') if e == 0 else 10 * np.log10(np.mean(r ** 2) / e)


def bands(x, fs=96000.0, edges=(20, 200, 1000, 4000, 12000, 40000)):
    x = np.asarray(x) - np.mean(x)
    w = np.hanning(len(x))
    s = np.abs(np.fft.rfft(x * w)) ** 2
    f = np.fft.rfftfreq(len(x), 1 / fs)
    return [10 * np.log10(np.sum(s[(f >= lo) & (f < hi)]) + 1e-30) for lo, hi in zip(edges, edges[1:])]


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('outdir')
    ap.add_argument('--note', type=float, default=57)
    ap.add_argument('--n', type=int, default=9600)
    ap.add_argument('--all-forms', action='store_true', help='schedule with every op group (not only the proven)')
    ap.add_argument('--show', action='store_true', help='print the scheduled programs')
    ap.add_argument('knobs', nargs='*', help='name=knob (0..255; default 128)')
    a = ap.parse_intermixed_args(argv)
    knobs = {kv.split('=')[0]: int(kv.split('=')[1]) for kv in a.knobs}
    pa, pb, info = run_kernel(a.outdir, a.note, a.n, knobs, not a.all_forms)
    ra, rb = run_ref(a.outdir, a.note, a.n, knobs)
    w = min(960, a.n)
    for st, p, r in (('A', pa, ra), ('B', pb, rb)):
        if p is None:
            continue
        units, ins = info[st]
        bp, br = bands(p), bands(r)
        print(f'stage {st}: {units} units -> {ins} instructions; SNR {snr(p[:w], r[:w]):.1f} dB (10 ms), '
              f'{snr(p, r):.1f} dB (all); peak {np.max(np.abs(p)):.3f} / ref {np.max(np.abs(r)):.3f}; '
              'bands port - ref (dB): ' + ' '.join(f'{x - y:+.2f}' for x, y in zip(bp, br)))


if __name__ == '__main__':
    main()
