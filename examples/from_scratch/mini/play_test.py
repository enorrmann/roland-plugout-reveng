"""Play notes on the SYSTEM-8 (MIDI bytes to the device) while recording its audio, then report per note.

    python3 play_test.py [--ch 0] [--notes 69,72,76]    (PLG3 must be the selected plugout)

Each note: note on, HOLD s, note off, REST s. The report gives, per note, the dominant frequency of the held
part and its level, against the expected 440 * 2^((n - 69) / 12) (vibrato of the LFO moves it by up to ±0.24
semitones). Needs the device MIDI node (the same one dsplive.py uses) and arecord on hw:SYSTEM8,0.
"""
import argparse
import os
import subprocess
import sys
import time

import numpy as np

HOLD, REST = 1.5, 1.0


def midi_dev():
    for line in open('/proc/asound/cards'):
        w = line.split()
        if len(w) > 1 and w[1] == '[SYSTEM8':
            return f'/dev/snd/midiC{w[0]}D0'
    raise SystemExit('SYSTEM-8 not found')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--ch', type=int, default=0)
    ap.add_argument('--notes', default='69,72,76')
    ap.add_argument('--raw', default='/tmp/claude-1000/play_test.raw')
    a = ap.parse_args()
    notes = [int(n) for n in a.notes.split(',')]
    total = len(notes) * (HOLD + REST) + 1.5
    rec = subprocess.Popen(['arecord', '-D', 'hw:SYSTEM8,0', '-f', 'FLOAT_LE', '-r', '96000', '-c', '2',
                            '-d', str(int(total + 1)), a.raw], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(0.5)
    dev = open(midi_dev(), 'wb', buffering=0)
    marks = []
    t0 = time.time()
    for n in notes:
        marks.append((time.time() - t0, n))
        dev.write(bytes([0x90 | a.ch, n, 100]))
        time.sleep(HOLD)
        dev.write(bytes([0x80 | a.ch, n, 0]))
        time.sleep(REST)
    dev.close()
    rec.wait()
    b = open(a.raw, 'rb').read()
    x = np.frombuffer(b[:len(b) // 8 * 8], dtype='<f4').reshape(-1, 2).astype(float)
    print('captured %.1f s, rms L %.5f R %.5f' % (len(x) / 96000, np.sqrt((x[:, 0] ** 2).mean()),
                                                 np.sqrt((x[:, 1] ** 2).mean())))
    ch = 1 if np.abs(x[:, 1]).max() >= np.abs(x[:, 0]).max() else 0
    s = x[:, ch]
    for k, (t, n) in enumerate(marks):
        seg = s[int((t + 0.2 + 0 * k) * 96000): int((t + HOLD - 0.1) * 96000)]
        if len(seg) < 4096:
            continue
        f = np.fft.rfftfreq(len(seg), 1 / 96000)
        S = np.abs(np.fft.rfft(seg * np.hanning(len(seg))))
        S[f < 60] = 0
        fp = f[np.argmax(S)]
        exp = 440 * 2 ** ((n - 69) / 12)
        print('note %3d: expect %7.1f Hz, peak %7.1f Hz (%+.2f semitones), rms %.4f' % (
            n, exp, fp, 12 * np.log2(fp / exp), np.sqrt((seg ** 2).mean())))


if __name__ == '__main__':
    sys.exit(main())
