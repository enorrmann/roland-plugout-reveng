"""Play a note on the SYSTEM-1 and record its audio over USB -- the capture rig used to check a PLUG-OUT on the
device (docs/vol5_toolchain_compiler_sdk/audio_capture.md). One MIDI client at a time: do not run another SYSTEM-1 tool at the same time,
and do not touch the keyboard while it records (your notes mix into the capture).

    python3 tools/s1_capture.py                         # note 60 on ch 5, 1.5 s -> harmonic summary
    python3 tools/s1_capture.py --note 67 --secs 3      # another note, longer
    python3 tools/s1_capture.py --cmd "python3 tools/wt_load.py --square"   # run that first, then capture
    python3 tools/s1_capture.py --raw cap.raw           # keep the raw audio (S32_LE, 96 kHz, 2 ch)
    python3 tools/s1_capture.py --list                  # just show the SYSTEM-1 MIDI + audio devices

The SYSTEM-1 receives on MIDI channel 5 here (--channel to change). Audio comes back on the USB audio input
(S32_LE, 96 kHz, stereo); the synth's output is on it only while the device plays.
"""
import argparse
import os
import re
import subprocess
import sys
import threading
import time

RATE = 96000
HINT = 'SYSTEM-1'


def find_midi():
    """the amidi port of SYSTEM-1 MIDI 1 (e.g. hw:3,0,0)"""
    out = subprocess.run(['amidi', '-l'], capture_output=True, text=True).stdout
    for line in out.splitlines():
        if HINT in line and 'MIDI 1' in line:
            return line.split()[1]
    for line in out.splitlines():                              # any SYSTEM-1 port
        if HINT in line:
            return line.split()[1]
    raise SystemExit('s1_capture.py: no %s MIDI port (amidi -l)' % HINT)


def find_audio():
    """the ALSA capture device of the SYSTEM-1 (hw:<card>,0)"""
    for line in open('/proc/asound/cards'):
        w = line.split()
        if len(w) > 1 and w[1].lstrip('[').startswith(HINT.replace('-', '')):
            return 'hw:%s,0' % w[0]
    return 'hw:CARD=SYSTEM1,DEV=0'


def classify(x):
    """harmonic summary of a mono signal: fundamental and h2..h6 relative to h1, and a saw/square/triangle guess"""
    import numpy as np
    nz = np.nonzero(np.abs(x) > x.std() * 0.1)[0]
    if len(nz) < RATE // 4:
        return 'silent (did a note play? right channel?)', {}
    a, b = nz[0], nz[-1]
    mid = x[a + (b - a) // 4: b - (b - a) // 4]                # the sustained middle, past attack / release
    mid = mid - mid.mean()
    X = np.abs(np.fft.rfft(mid * np.hanning(len(mid))))
    f = np.fft.rfftfreq(len(mid), 1.0 / RATE)
    f0 = f[1 + np.argmax(X[1:])]

    def h(k):
        m = (f >= f0 * k * 0.97) & (f <= f0 * k * 1.03)
        return X[m].max() if m.any() else 0.0
    h1 = h(1) or 1e-9
    amp = {k: h(k) / h1 for k in range(1, 7)}
    even, r3 = (amp[2] + amp[4]) / 2, amp[3]
    guess = ('saw' if even > 0.25 else 'square' if r3 > 0.22 else 'triangle' if r3 > 0.03 else 'sine/unclear')
    return guess, {'f0': round(f0, 1), **{'h%d' % k: round(amp[k], 3) for k in range(2, 7)}}


def capture(path, port, audio, note, ch, secs, on, hold, vel):
    import numpy as np

    def play():
        time.sleep(on)
        subprocess.run(['amidi', '-p', port, '-S', '%02x %02x %02x' % (0x90 | (ch - 1), note, vel)], check=False)
        time.sleep(hold)
        subprocess.run(['amidi', '-p', port, '-S', '%02x %02x 00' % (0x80 | (ch - 1), note)], check=False)
    t = threading.Thread(target=play)
    t.start()
    subprocess.run(['arecord', '-q', '-D', audio, '-d', str(int(np.ceil(secs))), '-f', 'S32_LE',
                    '-r', str(RATE), '-c', '2', '-t', 'raw', path], check=True)
    t.join()
    x = np.fromfile(path, '<i4').reshape(-1, 2).astype(np.float64) / 2147483648.0
    return x


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--note', type=int, default=60, help='MIDI note (default 60 = middle C)')
    ap.add_argument('--channel', type=int, default=5, metavar='N', help='MIDI channel 1..16 (default 5)')
    ap.add_argument('--vel', type=int, default=100, help='velocity (default 100)')
    ap.add_argument('--secs', type=float, default=1.5, help='capture length (default 1.5)')
    ap.add_argument('--on', type=float, default=0.3, help='delay before note-on (default 0.3)')
    ap.add_argument('--hold', type=float, default=1.0, help='note held (default 1.0)')
    ap.add_argument('--cc', help='control changes sent before the note, "CC=VALUE,..." (e.g. "74=127,71=0"; SYSTEM-S1CC '
                    'maps 74 cutoff, 71 resonance, 79 filter env amount, 75 filter decay: examples/system_s1cc/README.md §1.5)')
    ap.add_argument('--cmd', help='a shell command to run before the capture (e.g. a wt_load.py send)')
    ap.add_argument('--raw', help='keep the raw audio at this path (default: a temp file, deleted)')
    ap.add_argument('--port', help='amidi MIDI port (default: the SYSTEM-1)')
    ap.add_argument('--audio', help='ALSA capture device (default: the SYSTEM-1)')
    ap.add_argument('--list', action='store_true', help='show the SYSTEM-1 devices and exit')
    a = ap.parse_args()

    port = a.port or find_midi()
    audio = a.audio or find_audio()
    if a.list:
        print('MIDI :', port)
        print('audio:', audio)
        return
    if not 1 <= a.channel <= 16:
        raise SystemExit('s1_capture.py: --channel is 1..16')
    if a.cmd:
        print('$', a.cmd)
        subprocess.run(a.cmd, shell=True, check=False)
        time.sleep(0.2)

    if a.cc:
        for item in a.cc.split(','):
            c, v = (int(x) for x in item.split('='))
            if not (0 <= c <= 127 and 0 <= v <= 127):
                raise SystemExit('s1_capture.py: --cc values are 0..127')
            subprocess.run(['amidi', '-p', port, '-S', '%02x %02x %02x' % (0xB0 | (a.channel - 1), c, v)], check=False)
        print('sent CC', a.cc)
        time.sleep(0.2)

    path = a.raw or os.path.join(os.environ.get('TMPDIR', '/tmp'), 's1_capture_%d.raw' % os.getpid())
    print('capturing: note %d, channel %d, %.1f s (%s)' % (a.note, a.channel, a.secs, audio))
    x = capture(path, port, audio, a.note, a.channel, a.secs, a.on, a.hold, a.vel)
    peak = float(abs(x).max())
    guess, d = classify(x[:, 0] + x[:, 1])
    print('peak %.5f  ->  %s  %s' % (peak, guess, d))
    if a.raw:
        print('raw audio: %s (S32_LE, %d Hz, 2 ch)' % (a.raw, RATE))
    else:
        try:
            os.remove(path)
        except OSError:
            pass


if __name__ == '__main__':
    main()
