"""Which voice got each note, and whether that voice is heard: one run, measured, not inferred.

    python3 voicelog.py                       # notes 48,52,55,60,64,67,72,76 one by one
    python3 voicelog.py --groups 48,52,55,60 64,67,72,76
    python3 voicelog.py --groups 60 --hold 3

Needs the region 0 built with VOICELOG=1 (model/build.sh) in the selected plug-out (PLG3). Region 0 sends one SysEx
per voice assignment on the device's MIDI 1: F0 7D 05 <kind> <voice> <value> F7 (kind 0 = note, 1 = velocity,
voice 0..7 = global voice id; slave 0 owns 0-3, slave 1 owns 4-7). This script:
  1. sends the note on / off bytes to the device MIDI 1 port (as play_test.py),
  2. reads the SysEx back from the device MIDI 1 port with its timestamps,
  3. records the audio (hw:SYSTEM8,0, 96 kHz, 2 channels, float),
  4. for each note: the voice the region assigned, and the level and pitch heard in its hold window.
Only one device tool at a time (docs/ memory: one device client).
"""
import argparse
import os
import subprocess
import tempfile
import threading
import time

import numpy as np

PRE, POST = 0.5, 0.6          # seconds before the first note and after the last release, for the baseline
RAW_FMT = ['-t', 'raw', '-f', 'FLOAT_LE', '-r', '96000', '-c', '2']


def device_node(sub):
    """ALSA rawmidi node of the SYSTEM-8, subdevice 0 (MIDI 1, SysEx log) or 1 (MIDI 2, the plug-out port)"""
    for line in open('/proc/asound/cards'):
        w = line.split()
        if len(w) > 1 and w[1] == '[SYSTEM8':
            return f'/dev/snd/midiC{w[0]}D{sub}'
    raise SystemExit('SYSTEM-8 not found')


class SysexLog(threading.Thread):
    """reads MIDI 1 and keeps the voice-log messages with their time (time.monotonic)"""

    def __init__(self, node):
        super().__init__(daemon=True)
        self.f = open(node, 'rb', buffering=0)
        self.events = []                              # (t, kind, voice, value)
        self.buf = bytearray()
        self.stop = False

    def run(self):
        while not self.stop:
            b = self.f.read(1)
            if not b:
                break
            now = time.monotonic()
            x = b[0]
            if x >= 0xF8:                             # real-time bytes (clock) may sit inside a SysEx: skip them
                continue
            if x == 0xF0:
                self.buf = bytearray([x])
            elif self.buf and x < 0x80:
                self.buf.append(x)
            elif self.buf and x == 0xF7:
                self.buf.append(x)
                m = self.buf
                if len(m) == 8 and m[1:3] == b'\x7d\x05':
                    self.events.append((now, m[3], m[4], m[5], m[6]))     # time, kind, voice, value, note
                self.buf = bytearray()
            else:
                self.buf = bytearray()                # any other status byte: not our message


def record(path, seconds):
    return subprocess.Popen(['arecord', '-D', 'hw:SYSTEM8,0', '-d', str(int(seconds) + 1), *RAW_FMT, path],
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def play(groups, hold, rest, ch, out):
    """writes note on / off to the plug-out port; returns [(t_on, t_off, [notes])] on the monotonic clock"""
    marks = []
    for g in groups:
        t_on = time.monotonic()
        for n in g:
            out.write(bytes([0x90 | ch, n, 100]))
        time.sleep(hold)
        t_off = time.monotonic()
        for n in g:
            out.write(bytes([0x80 | ch, n, 0]))
        marks.append((t_on, t_off, g))
        time.sleep(rest)
    return marks


def level(seg, rate=96000):
    f = np.fft.rfftfreq(len(seg), 1 / rate)
    mag = np.abs(np.fft.rfft(seg * np.hanning(len(seg))))
    return f, mag


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--groups', nargs='+', default=None,
                    help='each argument is one group of notes played together, e.g. 48,52 60 (default: one by one)')
    ap.add_argument('--hold', type=float, default=1.5)
    ap.add_argument('--rest', type=float, default=1.0)
    ap.add_argument('--ch', type=int, default=0)
    a = ap.parse_args()
    if a.groups is None:
        groups = [[n] for n in (48, 52, 55, 60, 64, 67, 72, 76)]
    else:
        groups = [[int(n) for n in g.split(',')] for g in a.groups]
    total = PRE + len(groups) * (a.hold + a.rest) + POST

    wav = os.path.join(tempfile.mkdtemp(prefix='voicelog-'), 'audio.raw')
    log = SysexLog(device_node(0))
    log.start()
    rec = record(wav, total)
    time.sleep(0.5)
    t0 = time.monotonic()
    time.sleep(PRE)
    out = open(device_node(0), 'wb', buffering=0)
    marks = play(groups, a.hold, a.rest, a.ch, out)
    time.sleep(POST)
    out.close()
    if rec.wait() != 0 or not os.path.exists(wav):
        raise SystemExit('arecord failed on hw:SYSTEM8,0 (device busy?)')
    time.sleep(0.3)
    log.stop = True

    raw = open(wav, 'rb').read()
    x = np.frombuffer(raw[:len(raw) // 8 * 8], dtype='<f4').reshape(-1, 2).astype(float)
    s = x[:, 1] if np.abs(x[:, 1]).max() >= np.abs(x[:, 0]).max() else x[:, 0]
    rate = 96000

    def idx(t):
        return int((t - t0 + 0.0) * rate) if t else 0

    base = s[:int(PRE * rate * 0.9)]
    base_rms = max(np.sqrt((base ** 2).mean()), 1e-5)     # digital silence before the notes: absolute floor
    print('audio: %.1f s, baseline rms %.6f (channel %s)' % (len(s) / rate, base_rms, 'R' if s is x[:, 1] else 'L'))
    print('sysex voice log: %d messages' % len(log.events))
    for t, kind, voice, value, note in log.events:
        print('   log t=%+.3f kind=%d voice=%d value=%d note=%d' % (t - t0, kind, voice, value, note))
    print()
    print('%-22s %-7s %-6s %-6s %-9s %-9s %-7s %s' % ('group', 'note', 'voice', 'slave', 'expect Hz', 'heard Hz',
                                                    'rms x', 'audible'))
    rows = []
    for t_on, t_off, g in marks:
        for n in g:
            ev = [e for e in log.events if e[1] == 1 and e[3] > 0 and e[4] == n and t_on - 0.05 <= e[0] <= t_on + 0.5]
            voice = ev[0][2] if ev else None
            seg = s[int((t_on - t0 + 0.3) * rate): int((t_off - t0 - 0.1) * rate)]
            exp = 440 * 2 ** ((n - 69) / 12)
            if len(seg) < 4096:
                continue
            f, mag = level(seg)
            band = (f > exp * 2 ** (-0.04)) & (f < exp * 2 ** (0.04))
            heard = f[band][np.argmax(mag[band])] if band.any() else 0.0
            rms = np.sqrt((seg ** 2).mean())
            ratio = rms / base_rms
            ok = ratio > 3 and abs(1200 * np.log2(heard / exp)) < 60 if heard else False
            rows.append((voice, n, ok))
            print('%-22s %-7d %-6s %-6s %-9.1f %-9.1f %-7.1f %s' % (
                str(g), n, '-' if voice is None else voice, '-' if voice is None else voice // 4,
                exp, heard, ratio, 'YES' if ok else 'no'))
    print()
    for sl in (0, 1):
        r = [ok for v, _, ok in rows if v is not None and v // 4 == sl]
        if r:
            print('slave %d: %d of %d notes audible' % (sl, sum(r), len(r)))
    nv = [n for v, n, _ in rows if v is None]
    if nv:
        print('notes with no voice log message:', nv)


if __name__ == '__main__':
    main()
