"""Convert a single-cycle WAV into an oscillator program for the DCO1/DCO2 slot of the SYSTEM-8 DSPs.

    python3 wav2dco.py cycle.wav [-n 64] [-o prog.txt] [--slot dco2] [--check]

The cycle is resampled to N points (FFT resampling = band-limited, drops harmonics above N/2), DC removed, peak
normalised to --level; the program (re/dspasm.py `wavetable`) interpolates linearly between the points, with
the points hardcoded as immediates (the DSP has no indexed reads). Load it live with
`python3 dspown.py load prog.txt` (lab plugin: only the DCO2 slot is live-pokable, so use --slot dco2 for that).
"""
import argparse, os, sys
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dspasm   # noqa: E402

# measured 2026-10-16: a DSP runs ~930-950 instructions per sample (slaves by audio, master DSP3 with
# re/dspbudget.py; an A/A/B instruction costs the same as a single-op one). Main2/3 takes 189, so the two DCO slots
# of a DSP share ~750 (CUSTOM_OSC.md). Over budget voice B fails and the device may need a power cycle.
BUDGET = 750
MARGIN = 40             # the edge is not sharp (925-945); stay this far below it


def read_wav(path):
    """mono float samples of a PCM (8/16/24/32-bit) or IEEE float (32/64-bit) WAV; channels averaged"""
    data = open(path, 'rb').read()
    if data[:4] != b'RIFF' or data[8:12] != b'WAVE':
        raise SystemExit(f'{path}: not a WAV file')
    fmt = body = None
    o = 12
    while o + 8 <= len(data):
        cid, n = data[o:o + 4], int.from_bytes(data[o + 4:o + 8], 'little')
        if cid == b'fmt ':
            fmt = data[o + 8:o + 8 + n]
        elif cid == b'data':
            body = data[o + 8:o + 8 + n]
        o += 8 + n + (n & 1)
    if fmt is None or body is None:
        raise SystemExit(f'{path}: no fmt / data chunk')
    tag, ch, bits = int.from_bytes(fmt[0:2], 'little'), int.from_bytes(fmt[2:4], 'little'), \
        int.from_bytes(fmt[14:16], 'little')
    if tag == 0xFFFE:                                   # WAVE_FORMAT_EXTENSIBLE: sub-format in the GUID
        tag = int.from_bytes(fmt[24:26], 'little')
    sw = bits // 8
    body = body[:len(body) - len(body) % (sw * ch)]
    if tag == 3 and sw in (4, 8):
        x = np.frombuffer(body, '<f4' if sw == 4 else '<f8').astype(float)
    elif tag == 1 and sw == 1:
        x = (np.frombuffer(body, np.uint8).astype(float) - 128) / 128
    elif tag == 1 and sw == 2:
        x = np.frombuffer(body, '<i2').astype(float) / 32768
    elif tag == 1 and sw == 3:
        b = np.frombuffer(body, np.uint8).reshape(-1, 3).astype(np.int32)
        x = ((b[:, 0] | b[:, 1] << 8 | b[:, 2] << 16) << 8 >> 8) / 2 ** 23
    elif tag == 1 and sw == 4:
        x = np.frombuffer(body, '<i4').astype(float) / 2 ** 31
    else:
        raise SystemExit(f'{path}: unsupported WAV format (tag {tag}, {bits} bits)')
    return x.reshape(-1, ch).mean(axis=1)


def resample_cycle(x, n):
    """periodic FFT resampling of one cycle to n points"""
    X = np.fft.rfft(x)
    m = n // 2 + 1
    Y = np.zeros(m, complex)
    k = min(len(X), m)
    Y[:k] = X[:k]
    if n < len(x):
        Y[n // 2] = Y[n // 2].real                     # Nyquist bin of the new grid must be real
    return np.fft.irfft(Y, n) * (n / len(x))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('wav')
    ap.add_argument('-n', type=int, default=32, help='table points (default 32; ~4 instructions per point for both voices)')
    ap.add_argument('--slot', default='dco2', choices=['dco1', 'dco2'])
    ap.add_argument('--level', type=float, default=0.9)
    ap.add_argument('--keep-dc', action='store_true')
    ap.add_argument('--cycle', type=int, help='cycle length in samples (default: the whole file)')
    ap.add_argument('--frame', type=int, default=0, help='with --cycle: take frame FRAME (multi-frame wavetables)')
    ap.add_argument('--pad-mid', type=int, default=0, help='no-ops between voice A and B (time budget probe)')
    ap.add_argument('--pad-mid-aab', type=int, default=0, help='A/A/B fillers between voice A and B')
    ap.add_argument('--check', action='store_true', help='simulate and compare with the table')
    ap.add_argument('-o')
    a = ap.parse_args()
    x = read_wav(a.wav)
    if a.cycle:
        x = x[a.frame * a.cycle:(a.frame + 1) * a.cycle]
        if len(x) < a.cycle:
            raise SystemExit(f'{a.wav}: no frame {a.frame} of {a.cycle} samples')
    y = resample_cycle(x, a.n)
    if not a.keep_dc:
        y -= y.mean()
    y *= a.level / np.max(np.abs(y))
    P = dspasm.wavetable(a.slot, [round(float(v), 6) for v in y], pad_mid=a.pad_mid, pad_mid_aab=a.pad_mid_aab)
    nwords = sum(len(i) for i in P)
    rest = BUDGET - MARGIN - len(P)
    print(f'{a.wav}: {len(x)} samples -> {a.n} points, program {nwords} words, {len(P)} instructions; '
          + (f'leaves ~{rest} for the other DCO slot (with a margin of {MARGIN})' if rest > 0 else
             'TOO LONG for the per-sample budget even with the other DCO empty (voice B fails)'), file=sys.stderr)
    t = dspasm.text(P, f'wavetable from {os.path.basename(a.wav)}, {a.n} points, {a.slot} (re/wav2dco.py)')
    (open(a.o, 'w').write(t) if a.o else print(t, end=''))
    if a.check:
        words = [int(w, 16) for l in t.splitlines() for w in l.split('#')[0].split()]
        pitch = 4.75 - 3                                 # 55 Hz -> 1745 samples/cycle at 96 kHz
        out = dspasm.simulate(words, pitch, 1745 + 100, a.slot)
        per = round(96000 / (440 * 2 ** (pitch - 4.75)))
        sim = np.array(out[100:100 + per])
        ref = np.interp(np.linspace(0, a.n, per, endpoint=False), np.arange(a.n + 1), np.append(y, y[0]))
        # simulated cycle starts at an arbitrary phase: compare with best circular shift
        errs = [np.max(np.abs(np.roll(sim, s) - ref)) for s in range(0, per, 1)]
        print(f'simulated vs ideal interpolation: max error {min(errs):.5f}', file=sys.stderr)


if __name__ == '__main__':
    main()
