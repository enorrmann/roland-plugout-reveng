"""Check a wavetable oscillator on the device by USB audio: play a scale, compare the harmonics of every note
with the table's (re/wav2dco.py), report the notes where another program sounds.

    python3 wtcheck.py NAME cycle.wav [-n 32] [--cycle 2048 --frame K] [--notes 48 60]
"""
import argparse, os, sys, time
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dspaudio as da   # noqa: E402
import dspown           # noqa: E402
import wav2dco          # noqa: E402

H = 10


def table(path, n, cycle=None, frame=0):
    x = wav2dco.read_wav(path)
    if cycle:
        x = x[frame * cycle:(frame + 1) * cycle]
    y = wav2dco.resample_cycle(x, n)
    return y - y.mean()


def expected(y):
    """harmonic magnitudes 1..H of the linear interpolation of the table (sinc^2 droop), relative to the largest"""
    n = len(y)
    k = np.arange(1, H + 1)
    Y = np.abs(np.fft.rfft(y, n))
    Y = np.array([Y[i] if i <= n // 2 else 0 for i in k]) * np.sinc(k / n) ** 2
    return Y / Y.max()


def measured(x, f0):
    F = np.abs(np.fft.rfft(x * np.hanning(len(x)), 8 * len(x)))
    fr = np.fft.rfftfreq(8 * len(x), 1 / da.RATE)
    a = np.array([F[np.abs(fr - f0 * k) < 3].max() for k in range(1, H + 1)])
    return a / a.max()


def norm(v):
    return v / v.max() if v.max() > 0 else v


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('name')
    ap.add_argument('wav')
    ap.add_argument('-n', type=int, default=32)
    ap.add_argument('--cycle', type=int)
    ap.add_argument('--frame', type=int, default=0)
    ap.add_argument('--notes', type=int, nargs=2, default=[48, 60])
    ap.add_argument('--harmonics', type=int, default=H, help='compare harmonics 1..HARMONICS (the patch filter damps the high ones)')
    a = ap.parse_args()
    e = expected(table(a.wav, a.n, a.cycle, a.frame))[:a.harmonics]
    e = e / e.max()               # compare shapes over the same harmonics (the patch filter damps the high ones)
    ok = 0
    for note in range(*a.notes):
        x = da.load(da.record(f'{a.name}_n{note}', note, 0, 2.0))[40000:150000, 0]
        x = x - x.mean()
        f0 = dspown.freq(x) if x.std() > 1e-5 else 0.0
        # the DCO range is not known here: take the octave (-3..+1) whose harmonics fit the table best
        cands = [(np.abs(norm(measured(x, 440 * 2 ** ((note - 69) / 12 + o))[:a.harmonics]) - e).max(), o) for o in range(-3, 2)] \
            if x.std() > 1e-5 else [(1.0, 0)]
        err, o = min(cands)
        m = norm(measured(x, 440 * 2 ** ((note - 69) / 12 + o))[:a.harmonics]) if x.std() > 1e-5 else np.zeros(H)
        good = err < 0.05
        ok += good
        print(f'note {note}: f0 {f0:8.2f} oct {o:+d} rms {x.std():.4f} max harmonic error {err:.3f} {"OK" if good else "WRONG"}'
              f'  meas {" ".join(f"{v:.2f}" for v in m[:6])}', flush=True)
        time.sleep(0.3)
    print(f'{a.name}: {ok} of {a.notes[1] - a.notes[0]} notes match the table; expected {" ".join(f"{v:.2f}" for v in e[:6])}')


if __name__ == '__main__':
    main()
