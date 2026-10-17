"""All-in-one WAV wavetable -> bank-5 oscillator on the slaves (2026-10-17): converts a WAV (Serum-style multi-frame
table, `clm ` chunk, or a single cycle; PCM or float) to K frames of N points and sends it live to bank 5 of DSP2/DSP3
of both slaves (DCO2 slot, through dsplive), then leaves the player running.

    python3 wt5.py send TABLE.wav                          # 1 frame (the first), max size: 437 points
    python3 wt5.py send TABLE.wav --frames 2 --frame 1     # 2 frames of 218 points, play the last one
    python3 wt5.py send TABLE.wav --frames 8 --frame 3.5   # 8 x 53, fixed morph between frames 3 and 4
    python3 wt5.py send TABLE.wav --frames 8 --scan 0.25   # LFO scans all frames (NOT verified by audio yet)
    python3 wt5.py send TABLE.wav ... --dry                # only write the programs (work/wt5/)
    python3 wt5.py info TABLE.wav                          # frames in the file, what would be used

Needs the slave lab (DSPLIVE plugin in slot 3, PLG3 selected) and no other MIDI tool running. Time (15 ms per MIDI
command): table 438 commands (~6.6 s) + player ~280 words (~4 s, less when only some words change); --code ~2 min.

Transfer (default, needs a lab built with the 2026-10-17 engine): the table goes as DATA, one MIDI command per point
(18-bit fixed point, 2^-17 steps) for DSP2 and DSP3 at once; the slave engine (DSPLIVE) converts each value to RAW
and stores it into bank 5 itself; then the player program is loaded (both DSPs per command, only the changed words).
--code: the old way for older labs: the table travels as fill programs (move + store per point; bank 5 takes ONE
store per instruction) of --chunk points each, run with the player for a moment, then the player alone.
Conversion: frames picked evenly over the file, FFT-resampled to N points (band-limited), DC removed, one common
peak of 0.9.
Layout: frame k at BASE + k*(N+1): N points + a copy of point 0 (wrap). Space 0x048..0x1FD = 438 words
(Roland's slave programs use A000-A047, A200-A205, A300-A303): K*(N+1) <= 438 (1 x 437, 2 x 218, 4 x 108, 8 x 53,
16 x 26).
Player per voice (~45 instructions): SAW phase -> pos in [0, N), idx = ceil(pos) - 1, frac = pos - idx (SFU
ceiling `cf36`), h6 = frame k, h7 = frame k+1, four bank-5 loads (`0daY`, `0dbY`), linear interpolation inside the
frame and between frames. Frame position fixed (--frame) or a triangle LFO (--scan HZ, state in bank 5 A1FE).
"""
import argparse, math, os, sys, time
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dspasm as A                          # noqa: E402
import wav2dco                              # noqa: E402
from dspflagsaw import cmp_imm, jump_unless_ge   # noqa: E402

BASE, LIMIT = 0x048, 0x1FE                  # table words [BASE, LIMIT)
LFO = 0xA1FE                                # LFO phase (bank 5, persistent)
U = 1 / 16384
NOP = [0x0001]
OUT = os.path.join(HERE, '..', '..', '..', 'work', 'wt5')


def frame_size(path):
    d = open(path, 'rb').read()
    i = d.find(b'clm ')
    if i >= 0 and d[i + 8:i + 11] == b'<!>':
        return int(d[i + 11:i + 15])
    return 2048


def convert(path, K, N, peak=0.9):
    x = wav2dco.read_wav(path)
    fs = frame_size(path)
    F = max(1, len(x) // fs)
    picks = sorted(set(int(round(v)) for v in np.linspace(0, F - 1, K))) if K > 1 else [0]
    fr = []
    for i in picks:
        y = wav2dco.resample_cycle(x[i * fs:(i + 1) * fs], N)
        fr.append(y - y.mean())
    m = max(np.abs(f).max() for f in fr) or 1.0
    return [f * (peak / m) for f in fr], picks, F, fs


def words_of_table(frames):
    out = []
    for f in frames:
        out += list(f) + [f[0]]
    return out


def fill_code(values, addr0):
    """move + store per point (bank 5 takes ONE store per instruction: with 2 or more none is done, 2026-10-17)"""
    P = []
    for i, v in enumerate(values):
        P += [A.a('move', 0, imm=float(v)), A.st(0, 0xA000 | (addr0 + i))]
    return P


def ceil(d, y):                             # SFU ceiling: r(d) = ceil(r(y)) (`cf36` = r4 <- ceil(r6))
    return [0x050 << 5 | 2, (8 + d) << 12 | 0xF30 | y]


def lerp(a, b, f, t):                       # r(a) = r(a) + r(f) * (r(b) - r(a)), t = temp
    return [A.a('sub', t, a, b), A.a('mac', a, f, t)]


def morph_pos(K, frame, scan, fs=96000.0):
    """r6 = frame index k (integer valued), r7 = weight of frame k+1, computed once per sample"""
    if K == 1:
        return [A.a('move', 6, imm=0.0), A.a('move', 7, imm=0.0)]
    if scan is None:
        k = min(int(math.floor(frame)), K - 2)
        return [A.a('move', 6, imm=float(k)), A.a('move', 7, imm=float(frame - k))]
    sub = A.a('add', 1, 1, imm=-2.0, bits=16)
    P = [A.ld(1, LFO), NOP, NOP,
         A.a('add', 1, 1, imm=2 * scan / fs), cmp_imm(1, -1.0), NOP, jump_unless_ge(len(sub)), sub, A.st(1, LFO),
         A.a('move', 7, imm=0.0), A.b('absadd', 0, 1, 7),              # r0 = |phase| in [0, 1]
         A.a('mul', 0, 0, imm=(K - 1) * 0.999)]                          # m in [0, K-1)
    P += [ceil(6, 0), A.a('sub', 7, 6, 0), A.a('add', 7, 7, imm=1.0),  # r7 = m - ceil(m) + 1
          A.a('add', 6, 6, imm=-1.0)]                                   # r6 = ceil(m) - 1
    return P


def voice(slot, v, N, fs=96000.0):
    pin, outs, state = A.SLOTS[slot][v]
    P = [A.ld(1, state + 1), A.ld(0, pin), A.a('add', 0, 0, imm=-4.75), A.a('mul', 2, 0, imm=math.log(2) / 16),
         A.a('move', 3, imm=1 / 24), A.a('ymacd', 3, 2, imm=1 / 6), A.a('ymacd', 3, 2, imm=0.5),
         A.a('ymacd', 3, 2, imm=1.0), A.a('ymacd', 3, 2, imm=1.0)]
    P += [A.a('mul', 3, 3, 3)] * 4 + [A.a('mul', 3, 3, imm=2 * 440.0 / fs)]
    sub = A.a('add', 1, 1, imm=-2.0, bits=16)
    P += [A.a('add', 1, 3, 1), cmp_imm(1, -1.0), NOP, jump_unless_ge(len(sub)), sub, A.st(1, state)]
    # pos in [0, N): idx = ceil(pos) - 1, frac = pos - idx (frac in (0, 1])
    P += [A.a('mul', 0, 1, imm=N / 2), A.a('add', 0, 0, imm=N / 2), ceil(2, 0),
          A.a('sub', 3, 2, 0), A.a('add', 3, 3, imm=1.0),                 # r3 = frac
          A.a('mul', 4, 6, imm=float(N + 1)), A.a('add', 4, 4, 2), A.a('add', 4, 4, imm=float(BASE - 1)),
          A.a('mul', 4, 4, imm=U), A.a('add', 5, 4, imm=(N + 1) * U),       # r4 = frame k, r5 = frame k+1
          [0x0B02, 0x4084], [0x0B02, 0x40C5], NOP,                          # h6 = r4, h7 = r5
          [0x0C02, 0x0DA0], [0x0C02, 0x1DA1], [0x0C02, 0x4DB0], [0x0C02, 0x5DB1], NOP, NOP]
    P += lerp(0, 1, 3, 2) + lerp(4, 5, 3, 2) + lerp(0, 4, 7, 2)
    P += [A.st(0, outs[0]), A.st(0, outs[1])]
    return P


def program(table, N, K, frame, scan, fill=None, slot='dco2'):
    P = [[0x0A02, 0xFF70]]
    if fill is not None:
        lo, hi = fill
        P += fill_code(table[lo:hi], BASE + lo)
    P += morph_pos(K, frame, scan)
    for v in 'AB':
        P += voice(slot, v, N)
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [0x0001]]
    return P


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cmd', choices=['send', 'info'])
    ap.add_argument('wav')
    ap.add_argument('--frames', type=int, default=1)
    ap.add_argument('--points', type=int, default=None, help='default: as many as fit')
    ap.add_argument('--frame', type=float, default=0.0)
    ap.add_argument('--scan', type=float, default=None, help='LFO rate (Hz) scanning all frames')
    ap.add_argument('--code', action='store_true', help='send the table as fill programs (labs built before the '
                    'bank-5 command, ~2 min)')
    ap.add_argument('--chunk', type=int, default=200, help='--code: table words written per fill program')
    ap.add_argument('--dry', action='store_true', help='only write the programs')
    a = ap.parse_args()
    K = a.frames
    N = a.points or (LIMIT - BASE) // K - 1
    if K * (N + 1) > LIMIT - BASE:
        raise SystemExit(f'{K} x {N} points do not fit: K*(N+1) <= {LIMIT - BASE}')
    frames, picks, F, fs = convert(a.wav, K, N)
    if a.cmd == 'info':
        print(f'{a.wav}: {F} frames of {fs}; using frames {picks} at {N} points ({K * (N + 1)} words)')
        return
    table = words_of_table(frames)
    os.makedirs(OUT, exist_ok=True)
    name = os.path.splitext(os.path.basename(a.wav))[0].replace(' ', '_')
    progs = []
    for lo in range(0, len(table), a.chunk) if a.code else ():
        progs.append(program(table, N, K, a.frame, a.scan, (lo, min(len(table), lo + a.chunk))))
    progs.append(program(table, N, K, a.frame, a.scan))
    paths = []
    for i, P in enumerate(progs):
        p = os.path.join(OUT, f'{name}_{"play" if i == len(progs) - 1 else f"fill{i}"}.txt')
        open(p, 'w').write(A.text(P, f'wt5 {a.wav} K={K} N={N} frames {picks}'))
        words = sum(len(x) for x in P)
        print(f'{os.path.basename(p)}: {len(P)} instructions, {words} words')
        if len(P) > 600 or words > 4096:
            raise SystemExit('over the safe size (600 instructions / 4096 words): lower --chunk')
        paths.append(p)
    if a.dry:
        return
    import dspown
    t0 = time.time()
    if not a.code:              # the table as data: one MIDI command per point, both DSPs (engine DSPLIVE bank-5 mode)
        import dsplive
        link = dsplive.Link()
        link.write_b5(BASE, table)
        link.center()
        print(f'table sent: {len(table)} words in {time.time() - t0:.1f} s')
    for p in paths:
        dspown.load(p)
        time.sleep(0.3)
    print(f'done in {time.time() - t0:.1f} s')
    print(f'table in bank 5 [{BASE:#x}..{BASE + len(table):#x}), playing {os.path.basename(paths[-1])}')


if __name__ == '__main__':
    main()
