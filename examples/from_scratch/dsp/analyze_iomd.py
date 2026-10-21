"""Peak analysis of a capture of the IOMD test (make_iomd_test.py): which tones reach the output.

    arecord -D hw:SYSTEM8,0 -f FLOAT_LE -r 96000 -c 2 -d 6 cap.raw
    python3 analyze_iomd.py cap.raw
Expected: 440 Hz (word 60B0), 660 Hz (60B1), 880 Hz (6020). Level per tone in dB relative to the strongest.
"""
import struct
import sys

import numpy as np

raw = open(sys.argv[1], 'rb').read()
x = np.frombuffer(raw[: len(raw) // 8 * 8], dtype='<f4').reshape(-1, 2)
for ch in (0, 1):
    s = x[:, ch].astype(np.float64)
    s = s[int(0.5 * 96000):]                       # skip the first 0.5 s
    rms = float(np.sqrt(np.mean(s ** 2)))
    f = np.fft.rfftfreq(len(s), 1 / 96000)
    spec = np.abs(np.fft.rfft(s * np.hanning(len(s))))
    print('channel %d: rms %.5f' % (ch, rms))
    for target in (440, 660, 880):
        m = (f > target * 0.97) & (f < target * 1.03)
        peak = f[m][np.argmax(spec[m])] if m.any() else float('nan')
        print('  near %4d Hz: peak %.1f Hz, level %.1f dB re full' % (
            target, peak, 20 * np.log10(spec[m].max() / (len(s) / 2) + 1e-12)))
