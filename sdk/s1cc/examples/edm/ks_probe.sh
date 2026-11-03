#!/bin/sh
# ks_probe.sh NOTE [secs]: play NOTE on the SYSTEM-1 (MIDI channel 5, the device setting here; PORT=hw:X,Y,Z) while capturing USB audio; prints the pitch found
D=${OUT:-/tmp}; N=${1:-48}; T=${2:-3}
W=$D/ks_$N.wav
arecord -q -D hw:SYSTEM1,0 -f S32_LE -r 96000 -c 2 -d $((T + 4)) "$W" &
sleep 0.3
amidi -p ${PORT:-hw:3,0,0} -S "$(printf '94 %02x 64' $N)"
sleep 1.0
amidi -p ${PORT:-hw:3,0,0} -S "$(printf '84 %02x 00' $N)"
wait
python3 - "$W" "$N" <<'PY'
import sys, numpy as np, wave
w = wave.open(sys.argv[1]); n = int(sys.argv[2])
x = np.frombuffer(w.readframes(w.getnframes()), dtype='<i4').reshape(-1, 2)[:, 0].astype(float) / 2**31
fs = 96000
print('peak %.4f  rms %.4f' % (abs(x).max(), x.std()))
# the SYSTEM-1's USB audio can lag seconds behind MIDI (2.7 s measured 2026-11-03): times are from the note's onset
on = np.argmax(abs(x) > 0.2 * abs(x).max()) / fs
print('onset %.3f s' % on)
for t0 in (0.1, 0.5, 1.2, 1.9):
    s = x[int((on + t0) * fs):int((on + t0) * fs) + 16384]
    if len(s) < 16384: break
    s = s - s.mean()
    ac = np.fft.irfft(abs(np.fft.rfft(s, 65536)) ** 2)[:20000]
    lag = 30 + np.argmax(ac[30:])
    print('t=%.1f s  rms %.4f  period %d samples (%.2f Hz)' % (t0, s.std(), lag, fs / lag))
print('expected period %.1f samples (%.2f Hz)' % (fs / (440 * 2 ** ((n - 69) / 12)), 440 * 2 ** ((n - 69) / 12)))
PY
