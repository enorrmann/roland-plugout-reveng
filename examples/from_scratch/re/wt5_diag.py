"""Host-side diagnostic of what `wt5.py send` actually sends (no hardware needed, except --peek).

    python3 wt5_diag.py TABLE.wav [--frames K] [--points N] [--frame F] [--scan HZ]
    python3 wt5_diag.py TABLE.wav --peek        # + read bank 5 back from the master DSPs (needs the lab + dsppeek)

Four independent checks:

1. SIMULATOR. Builds the programs exactly as wt5.py does and runs them with dspasm.simulate (dspisa) into a memory
   preloaded with the same table stored at 0xA000 + BASE: the output must be the expected frame waveform
   (resample to N points, linear interpolation, between frames). This is what NOTES.md calls "simulador exacto".
2. SHADOW. Replays the program writes dspown.load would send (dspown's EMPTY head, runs against state.json, 32-word
   chunks, head one 32-bit word at a time, start word last) into a shadow of the DCO2 slot and decodes it: the DSP
   must see the same words wt5.py compiled (a truncated/torn head shows up here).
3. DATA. Shows the bank-5 stream wt5.py sends (one command per point, 18-bit fixed point 2^-17, RAW conversion of
   engine.c's live_raw18) and *where the engine stores it*: the slave's DPC RAW window 0x41114000 + 0x4000*(n-2)
   + 0x2000 - the same window whose writes kill the DSPs on the device (T1/T2 of re/wt5_isolate.py).
4. PEEK (optional). Reads bank 5 back from the master DSPs at 0x41404000 + n*0x100000 + 4*(A000+addr) and compares
   with the expected RAW words; reports how many words differ (0 = the table landed).
"""
import argparse, json, os, sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dsplive as L          # noqa: E402
import dspown                # noqa: E402
import dspasm as A           # noqa: E402
import wt5                   # noqa: E402

BASE = wt5.BASE
EMPTY_LEN = len(dspown.EMPTY)


def shadow_of(paths):
    """the DCO2 shadow words of DSP2 as dspown.load leaves them (state.json + the program files)"""
    st = L.load_state()
    w = dspown.words_of(paths[0])
    old = (st or {}).get('words', {}).get('2', [])
    new = [None] * EMPTY_LEN + w[EMPTY_LEN:]
    old = [None] * EMPTY_LEN + old[EMPTY_LEN:]
    sh = list(old)
    for start, chunk in L.runs(old, new):
        for k in range(len(chunk)):
            i = start + k
            if i >= len(sh):
                sh.append(None)
            sh[i] = chunk[k]
    for i in range(EMPTY_LEN):
        sh[i] = w[i]
    return [0 if x is None else x for x in sh], w


def sim(words, table_words, pitch=4.75, n=4000):
    """dspisa simulation with bank 5 holding the table at A000 + BASE"""
    import dspasm
    mem_tbl = {0xA000 | ((BASE + i) & 0x3FF): v for i, v in enumerate(table_words)}
    # dspasm.simulate builds its own memory: patch it by running the same loop with the table preloaded
    import dspisa
    ins_list = [ins for _, ins in __import__('dspdis').instructions(words)]
    slot = 'dco2'
    mem = {A.SLOTS[slot]['A'][0]: pitch, A.SLOTS[slot]['B'][0]: pitch}
    mem.update(mem_tbl)
    ring = [A.SLOTS[slot][v][2] for v in 'AB'] + [A.FOLD_SLOTS.get(slot, {}).get(v, (0, 0, 0, None))[3] for v in 'AB']
    pos, p = {}, 0
    for i, ins in enumerate(ins_list):
        pos[p] = i
        p += len(ins)
    starts = sorted(pos)
    out = []
    for _ in range(n):
        for s in ring:
            if s is not None and s in mem:
                mem[s + 1] = mem[s]
        regs = [0.0] * 8
        i, steps = 0, 0
        while i < len(ins_list):
            steps += 1
            if steps > 100000:
                raise RuntimeError('endless loop in the simulation')
            dspisa.JUMP = None
            regs, mem, _ = dspisa.run(list(ins_list[i]), regs, mem)
            if dspisa.JUMP is not None:
                i = pos[starts[i] + len(ins_list[i]) + dspisa.JUMP]
            else:
                i += 1
        out.append(mem.get(A.SLOTS[slot]['A'][1][0], 0.0))
    return np.array(out)


def index_of(out, table_words, N):
    """recover the table points the oscillator produced: out[2i] = table[i] and out[2i+1] between points
    (SAW phase increments 0.5 per sample at pitch 4.75: pos goes 0, 0.5, 1, 1.5, ... so best = the table points
    themselves read at the even samples), interpolated inside the frame"""
    return out


def raw18(x):
    """engine.c live_raw18: 18-bit two's complement -> DSP RAW (sign-magnitude, exponent 0/1)"""
    v = x - (1 << 18) if x & (1 << 17) else x
    if v < (1 << 14):
        return (0x80000000 if v < 0 else 0) | ((abs(v) << 15) & 0x7FFFFFFF)
    return (0x80000000 if v < 0 else 0) | (1 << 29) | ((abs(v) << 11) & 0x1FFFFFFF)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('wav')
    ap.add_argument('--frames', type=int, default=1)
    ap.add_argument('--points', type=int, default=None)
    ap.add_argument('--frame', type=float, default=0.0)
    ap.add_argument('--scan', type=float, default=None)
    ap.add_argument('--peek', action='store_true', help='read bank 5 back from the master DSPs (dsppeek.py)')
    ap.add_argument('--n', type=int, default=4000, help='simulated samples')
    a = ap.parse_args()
    K = a.frames
    N = a.points or (wt5.LIMIT - BASE) // K - 1
    frames, picks, F, fs = wt5.convert(a.wav, K, N)
    table = wt5.words_of_table(frames)
    print(f'{os.path.basename(a.wav)}: {F} frames of {fs}; {K} x {N} points, {len(table)} words, frames {picks}')

    # 1) programs, exactly as main() builds them
    progs = []
    for lo in range(0, len(table), 200) if False else ():
        pass
    progs.append(wt5.program(table, N, K, a.frame, a.scan))
    paths = []
    os.makedirs(wt5.OUT, exist_ok=True)
    for i, P in enumerate(progs):
        p = os.path.join(wt5.OUT, f'diag_{i}.txt')
        open(p, 'w').write(A.text(P, f'wt5_diag {a.wav}'))
        paths.append(p)
    print(f'program: {len(progs[-1])} instructions, {sum(len(x) for x in progs[-1])} words')

    # 2) simulator: voice A must be the selected frame (or the morph between k and k+1)
    words = dspown.words_of(paths[-1])
    out = sim(words, table, n=a.n)
    k = 0 if K == 1 else min(int(np.floor(a.frame)), K - 2)
    frac = 0.0 if K == 1 else a.frame - k
    ref = np.array(frames[k]) * (1 - frac) + np.array(frames[min(k + 1, K - 1)]) * frac
    # SAW at pitch 4.75 (440 Hz at 96 kHz) advances 2*440/96000 of the frame per sample: 2 samples -> 1 point
    step = 2 * 440.0 / 96000.0 * N
    idx = np.round(np.arange(len(out)) * step) % N
    err = np.abs(out - ref[idx.astype(int)])
    print(f'simulator: voice A vs frame {k} (weight {1 - frac:.2f}/{frac:.2f}): max |err| {err.max():.3e} '
          f'({err.max() / max(1e-9, np.abs(ref).max()):.2%} of peak)')
    if err.max() > 1e-2:
        print('  WARNING: the program + table do not reproduce the frame in the exact simulator')

    # 3) shadow the DSP will see
    sh, w = shadow_of(paths)
    same = sh[:len(w)] == w
    print(f'shadow (dspown.load over state.json): {"matches the compiled program" if same else "DIFFERS"} '
          f'({sum(1 for x, y in zip(sh[:len(w)], w) if x != y)} words differ of {len(w)})')

    # 4) the bank-5 data stream and where the engine puts it
    vals = [max(-(1 << 17), min((1 << 17) - 1, int(round(v * (1 << 17))))) & 0x3FFFF for v in table]
    raw = [raw18(v) for v in vals]
    want = [A.raw(v / 131072) for v in table]
    bad = [i for i, (x, y) in enumerate(zip(raw, want)) if x != y]
    print(f'bank 5 stream: {len(vals)} commands, index A000-relative {BASE:#05x}..{BASE + len(vals) - 1:#05x}; '
          f'RAW conversion {"equals dspasm.raw" if not bad else f"differs at {len(bad)} words"}')
    print('  engine.c stores it at 0x41114000 + 0x4000*(n-2) + 0x2000 + 4*index, index & 0x3FF  (DPC RAW view)')

    if a.peek:
        from dsppeek import Peek, raw_value
        pk = Peek()
        ok = 0
        for d in (2, 3):
            got = pk.read(0x41404000 + d * 0x100000 + 4 * (0xA000 + BASE), len(table))
            ok += sum(1 for g, e in zip(got, want) if g == e)
        print(f'peek (master DSP2/DSP3 bank 5 {"hit" if ok == 2 * len(want) else "MISMATCH"}): '
              f'{ok}/{2 * len(want)} words as expected')
        if ok != 2 * len(want):
            print('  the table is not in bank 5: the write went through the DPC window (degrades DSPs) '
                  'or nothing stored it')


if __name__ == '__main__':
    main()
