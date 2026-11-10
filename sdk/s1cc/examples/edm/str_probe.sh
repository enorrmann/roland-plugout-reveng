#!/bin/sh
# str_probe.sh: MP-EDMSTR on the SYSTEM-1 (docs/vol6_reference_designs_applications/mutable_memory.md §3.3.5): six notes one after the other (each lands on
# another voice: the assigner takes the oldest free one), then a chord of three, while capturing USB audio; prints
# the pitch of each note against the expected one and the chord's three strongest peaks.
# MIDI channel 5 (the device setting here); PORT=hw:X,Y,Z; OUT=dir for the wav (default /tmp)
D=${OUT:-/tmp}; W=$D/str_probe.wav; P=${PORT:-hw:3,0,0}
NOTES="36 48 55 60 67 72"
arecord -q -D hw:SYSTEM1,0 -f S32_LE -r 96000 -c 2 -d 16 "$W" &
sleep 0.3
for n in $NOTES; do
    amidi -p $P -S "$(printf '94 %02x 64' $n)"; sleep 1.2
    amidi -p $P -S "$(printf '84 %02x 00' $n)"; sleep 0.3
done
amidi -p $P -S "94 3c 64 94 40 64 94 43 64"; sleep 2.0
amidi -p $P -S "84 3c 00 84 40 00 84 43 00"
wait
python3 - "$W" $NOTES <<'PY'
import sys, numpy as np, wave
w = wave.open(sys.argv[1]); notes = [int(x) for x in sys.argv[2:]]
x = np.frombuffer(w.readframes(w.getnframes()), dtype='<i4').reshape(-1, 2)[:, 0].astype(float) / 2**31
fs = 96000
print('peak %.4f  rms %.4f' % (abs(x).max(), x.std()))
# the USB audio lags the MIDI by up to ~2.7 s: everything is timed from the first onset
on = np.argmax(abs(x) > 0.2 * abs(x).max()) / fs
print('onset %.3f s' % on)
def peak(s, lo, hi):
    X = abs(np.fft.rfft(s * np.hanning(len(s)), 8 * len(s))); f = np.fft.rfftfreq(8 * len(s), 1 / fs)
    m = (f > lo) & (f < hi); return f[np.argmax(X * m)]
for i, n in enumerate(notes):
    t = on + 1.5 * i + 0.3
    s = x[int(t * fs):int((t + 0.8) * fs)]
    f0 = 440 * 2 ** ((n - 69) / 12)
    p = peak(s - s.mean(), 0.8 * f0, 1.25 * f0)
    print('note %3d  expected %8.2f Hz  measured %8.2f Hz  %+6.1f cents  rms %.4f' % (n, f0, p, 1200 * np.log2(p / f0), s.std()))
t = on + 1.5 * len(notes) + 0.3
s = x[int(t * fs):int((t + 1.2) * fs)]
X = abs(np.fft.rfft(s * np.hanning(len(s)), 4 * len(s))); f = np.fft.rfftfreq(4 * len(s), 1 / fs)
pk = [i for i in range(1, len(X) - 1) if X[i] > X[i - 1] and X[i] >= X[i + 1] and 100 < f[i] < 2000]
pk = sorted(pk, key=lambda i: -X[i])[:6]
print('chord 60 64 67 (261.63 329.63 392.00 Hz): peaks', ', '.join('%.2f' % f[i] for i in sorted(pk)))
PY
