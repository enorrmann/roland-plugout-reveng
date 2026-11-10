#!/bin/sh
# idx_probe.sh: MP-EDMIDX on the SYSTEM-1 (docs/vol6_reference_designs_applications/mutable_memory.md §3.3.6): one note (a dry burst), 12 s of USB audio.
# Left = dry + 0.7 x tapreq(base 272 + age 48000 - p), right = dry + 0.7 x tapreq(base 272, fixed address).
# Prints: the left echo's lag (48000 + a few = the OS convention works on our line), and what the right adds to the
# dry: a copy of the dry at a small lag (tapreq relative to the write position) or a slow step signal (absolute).
D=${OUT:-/tmp}; W=$D/idx_probe.wav; P=${PORT:-hw:3,0,0}; N=${1:-60}
arecord -q -D hw:SYSTEM1,0 -f S32_LE -r 96000 -c 2 -d 12 "$W" &
sleep 0.3
amidi -p $P -S "$(printf '94 %02x 64' $N)"; sleep 0.5
amidi -p $P -S "$(printf '84 %02x 00' $N)"
wait
python3 - "$W" <<'PY'
import sys, numpy as np, wave
w = wave.open(sys.argv[1])
x = np.frombuffer(w.readframes(w.getnframes()), dtype='<i4').reshape(-1, 2).astype(float) / 2**31
L, R = x[:, 0], x[:, 1]; fs = 96000
on = int(np.argmax(abs(L) > 0.2 * abs(L).max()))
print('onset %.3f s, peak L %.4f R %.4f' % (on / fs, abs(L).max(), abs(R).max()))
dry = L[on - 200:on + 4000]
seg = L[on - 200:on + 200000]
c = np.correlate(seg[1000:], dry, 'valid')
lag = 1000 + int(np.argmax(abs(c)))
print('left: strongest copy of the burst %d samples after it (expected ~48000), gain %.3f'
      % (lag - 0, c[lag - 1000] / np.dot(dry, dry)))
d = R - L                                   # the right tap minus the left tap (the dry cancels)
pre = d[on - 200:on + 40000]                # before the left echo: only the right tap
c2 = np.correlate(pre, dry[:2000], 'valid')
k = int(np.argmax(abs(c2)))
print('right - left before 0.4 s: rms %.5f, best copy of the burst at %d samples (gain %.3f)'
      % (pre.std(), k - 200, c2[k] / np.dot(dry[:2000], dry[:2000])))
blk = d[on:on + 10 * fs].reshape(-1, fs // 10)
print('right - left, mean per 0.1 s over 10 s (steps = absolute address):')
print(' '.join('%+.4f' % v for v in blk.mean(axis=1)))
PY
