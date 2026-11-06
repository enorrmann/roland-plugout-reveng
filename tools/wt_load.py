"""Load a wavetable frame into a running SYSTEM-S1CC bank-5 PLUG-OUT over USB -- no reinstall (docs/MUTABLE_MEMORY.md
§4.5). The plug-out must be built with SL_LOAD=1 (a SL_WT=b5 kernel, e.g. sdk/s1cc/examples/wavetable/jp8_saw.c):

    SL_LOAD=1 WT_WAV=".../Jupiter 8 Saw.wav" KERNEL=sdk/s1cc/build/jp8_saw sh examples/system_s1cc/build.sh
    python3 tools/plugout_tool.py install plugins/JP8-SAW5 --slot 1 --force --no-patches -y   # then select it

Then, while it plays:
    python3 tools/wt_load.py --saw                       # a band-limited saw (a built-in test shape)
    python3 tools/wt_load.py --sine                      # or --square / --tri / --pulse 0.25
    python3 tools/wt_load.py mywave.wav                  # one frame of a WAV (--frame K, --frame-size N)
    python3 tools/wt_load.py --revert                    # hand the frame back to OSC1 COLOR

Transport (re/dsplive.py's live-poke protocol, ported to the SYSTEM-1; engine.c wt_load.h): per word a pitch bend
carries 14 data bits and a CC1 (modulation) carries the 7-bit command. Our region 0 forwards pitch bend -> voice
parameter 0x130 and CC1 -> 0x131, and both reach region 1, which writes bank 5 A080.. with FD6C. One MIDI client at
a time ([[one-device-client]]): do not run another SYSTEM-1 tool meanwhile.

UNPROVEN on hardware as of 2026-11-06 (that bend / CC1 reach region 1). Uses only the kernel `wave` module and numpy.
"""
import argparse
import os
import re
import struct
import subprocess
import sys
import time

N_POINTS = 127                                    # points per frame (+ 1 guard = 128 bank-5 words A080..A0FF)


def find_port(hint='SYSTEM-1'):
    """the raw ALSA rawmidi device of the SYSTEM-1 (/dev/snd/midiCxDy), from `amidi -l`"""
    out = subprocess.run(['amidi', '-l'], capture_output=True, text=True).stdout
    for m in re.finditer(r'^(?:IO|O)\s+hw:(\d+),(\d+)(?:,\d+)?\s+(.*)$', out, re.M):
        if hint in m.group(3):
            return '/dev/snd/midiC%sD%s' % (m.group(1), m.group(2))
    # fall back to /proc/asound/cards (as re/dsplive.py), matching the card name
    key = hint.replace('-', '')
    for line in open('/proc/asound/cards'):
        w = line.split()
        if len(w) > 1 and w[1].lstrip('[').startswith(key):
            return '/dev/snd/midiC%sD0' % w[0]
    raise SystemExit('wt_load.py: no %s MIDI port (amidi -l); pass --port /dev/snd/midiCxDy' % hint)


class Link:
    """the frame-load transport on one raw MIDI device"""

    def __init__(self, dev, ch=0, delay=0.010):
        self.f = open(dev, 'wb', buffering=0)
        self.ch, self.delay, self.seq = ch & 15, delay, 0
        self.cmd(2, 1)                            # a nop so the first real command is always a change

    def cmd(self, op, ext, data=0x2000):
        """pitch bend = data (14 bits), then CC1 = seq<<6 | op<<4 | ext (7 bits)"""
        assert 0 <= data < 0x4000 and 0 <= op < 4 and 0 <= ext < 16
        self.seq ^= 1
        self.f.write(bytes([0xE0 | self.ch, data & 0x7F, (data >> 7) & 0x7F,
                            0xB0 | self.ch, 1, self.seq << 6 | op << 4 | ext]))
        time.sleep(self.delay)

    def addr(self, i):
        self.cmd(0, 0, i & 0x3FFF)

    def sample(self, q15):
        x = (int(q15) * 4) & 0x3FFFF              # Q15 -> 18-bit value * 2^17 (device: sext18 >> 2 -> Q15)
        self.cmd(1, (x >> 14) & 0xF, x & 0x3FFF)

    def commit(self):
        self.cmd(2, 0)

    def clear(self):
        self.cmd(2, 2)

    def load(self, q15_points):
        """send the whole frame (points + a guard) and commit it"""
        pts = list(q15_points)
        pts.append(pts[0])                        # guard point: the reader interpolates past the last point
        self.addr(0)
        for v in pts:
            self.sample(max(-32768, min(32767, int(v))))
        self.commit()
        return len(pts)


def read_wav(path):
    """mono float samples of a WAV: PCM 8/16/24/32 or IEEE float32 (no dependency on soundfile)"""
    import numpy as np
    with open(path, 'rb') as fh:
        data = fh.read()
    if data[:4] != b'RIFF' or data[8:12] != b'WAVE':
        raise SystemExit('wt_load.py: %s is not a WAV' % path)
    fmt, ch, bits, samples = None, 1, 16, None
    i = 12
    while i + 8 <= len(data):
        cid, sz = data[i:i + 4], struct.unpack_from('<I', data, i + 4)[0]
        body = data[i + 8:i + 8 + sz]
        if cid == b'fmt ':
            fmt, ch, _, _, _, bits = struct.unpack_from('<HHIIHH', body, 0)
        elif cid == b'data':
            samples = body
        i += 8 + sz + (sz & 1)
    if fmt is None or samples is None:
        raise SystemExit('wt_load.py: %s has no fmt/data chunk' % path)
    if fmt == 3 and bits == 32:
        x = np.frombuffer(samples, '<f4').astype(np.float64)
    elif fmt == 1 and bits == 16:
        x = np.frombuffer(samples, '<i2').astype(np.float64) / 32768.0
    elif fmt == 1 and bits == 32:
        x = np.frombuffer(samples, '<i4').astype(np.float64) / 2147483648.0
    elif fmt == 1 and bits == 24:
        b = np.frombuffer(samples, np.uint8).reshape(-1, 3).astype(np.int32)
        v = b[:, 0] | b[:, 1] << 8 | b[:, 2] << 16
        x = np.where(v & 0x800000, v - 0x1000000, v).astype(np.float64) / 8388608.0
    elif fmt == 1 and bits == 8:
        x = (np.frombuffer(samples, np.uint8).astype(np.float64) - 128.0) / 128.0
    else:
        raise SystemExit('wt_load.py: unsupported WAV (fmt %d, %d-bit)' % (fmt, bits))
    return x[::ch] if ch > 1 else x               # left channel


def shape(name, pulse):
    """a built-in test waveform, one period of `frame_size` samples"""
    import numpy as np
    t = np.linspace(0.0, 1.0, 2048, endpoint=False)
    if name == 'sine':
        return np.sin(2 * np.pi * t)
    if name == 'saw':
        return 2.0 * t - 1.0
    if name == 'square':
        return np.where(t < 0.5, 1.0, -1.0)
    if name == 'tri':
        return 1.0 - 4.0 * np.abs(np.round(t) - t)
    if name == 'pulse':
        return np.where(t < pulse, 1.0, -1.0)
    raise SystemExit('wt_load.py: unknown shape %s' % name)


def make_frame(wave, harmonics, peak):
    """band-limit to `harmonics` and resample to 127 points + normalize to `peak` -- the make_s1cc.py sl_frames() math,
    for one frame (int16 Q15)"""
    import numpy as np
    wave = np.asarray(wave, dtype=np.float64)
    n = len(wave)
    h = min(harmonics, N_POINTS // 2)
    X = np.fft.rfft(wave)[:h + 1] * (N_POINTS / n)
    X[0] = 0.0                                     # drop DC
    y = np.fft.irfft(X, N_POINTS)
    m = np.abs(y).max()
    if m > 0:
        y = y / m * peak
    return [int(round(v * 32767)) for v in y]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    src = ap.add_mutually_exclusive_group()
    src.add_argument('wav', nargs='?', help='a WAV file (one frame of it)')
    src.add_argument('--sine', action='store_const', const='sine', dest='shape')
    src.add_argument('--saw', action='store_const', const='saw', dest='shape')
    src.add_argument('--square', action='store_const', const='square', dest='shape')
    src.add_argument('--tri', action='store_const', const='tri', dest='shape')
    src.add_argument('--pulse', type=float, metavar='DUTY', help='a pulse of this duty (0..1)')
    src.add_argument('--revert', action='store_true', help='hand the frame back to OSC1 COLOR (clear the load)')
    ap.add_argument('--frame', type=int, default=0, help='which frame of the WAV (default 0)')
    ap.add_argument('--frame-size', type=int, default=0, help='samples per frame (default: 2048 if it divides, else all)')
    ap.add_argument('--harmonics', type=int, default=N_POINTS // 2, help='band-limit to this many harmonics (default 63)')
    ap.add_argument('--peak', type=float, default=0.8, help='normalize the frame to this peak (default 0.8)')
    ap.add_argument('--port', help='raw MIDI device (default: the SYSTEM-1, from amidi -l)')
    ap.add_argument('--channel', type=int, default=5, metavar='N',
                    help="MIDI channel 1..16 the SYSTEM-1 receives on (default 5, this device's; set it to yours)")
    ap.add_argument('--delay', type=float, default=0.010, help='seconds between messages (default 0.010)')
    ap.add_argument('--dry-run', action='store_true', help='build the frame and print it, send nothing')
    a = ap.parse_args()

    if not a.revert:
        if a.pulse is not None:
            wave = shape('pulse', a.pulse)
        elif a.shape:
            wave = shape(a.shape, 0.5)
        elif a.wav:
            wave = read_wav(a.wav)
            fs = a.frame_size or (2048 if len(wave) % 2048 == 0 and len(wave) >= 2048 else len(wave))
            off = a.frame * fs
            if off + fs > len(wave):
                raise SystemExit('wt_load.py: frame %d (size %d) is past the end of %s (%d samples)'
                                 % (a.frame, fs, a.wav, len(wave)))
            wave = wave[off:off + fs]
        else:
            ap.error('give a WAV, a built-in shape (--saw/--sine/--square/--tri/--pulse), or --revert')
        frame = make_frame(wave, a.harmonics, a.peak)
        print('frame: %d points, peak %d, harmonics %d' % (len(frame), max(abs(v) for v in frame), a.harmonics))
        if a.dry_run:
            print(', '.join(str(v) for v in frame))
            return

    if not 1 <= a.channel <= 16:
        raise SystemExit('wt_load.py: --channel is 1..16')
    dev = a.port or find_port()
    print('MIDI channel %d (change with --channel N), port %s' % (a.channel, dev))
    link = Link(dev, a.channel - 1, a.delay)
    if a.revert:
        link.clear()
        print('reverted: OSC1 COLOR drives the frame again')
    else:
        n = link.load(frame)
        print('loaded %d words (A080..A0%02X)' % (n, 0x80 + n - 1))


if __name__ == '__main__':
    main()
