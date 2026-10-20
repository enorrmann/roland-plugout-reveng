"""Whole-patch write test on S8LAB (slot 1, patch 0): the patch struct (640 bytes) is read, the OSC1 X-MOD IN cord
is changed (PAT_NAME1 + 104), the struct is written back in 128-byte DT1 blocks (as PROTOCOL section 6 does), and the
sound is measured live and after a program change (PC 1, PC 0). The original 640 bytes are written back at the end.

    python3 patch_write_test.py 13
"""
import os, subprocess, sys, time
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
import numpy as np
import cords_test as X
import color_test as C
import plugout as P
import plugout_tool as T
import dspaudio as da
import mod_scan as M

PATCH_ADDR = X.BASE
SIZE = 640
CHUNK = 128


import patches as PT
LAYOUT = PT.ScriptLayout(os.path.join(ROOT, 'plugins', 'SYSTEM-8', 'Script', 'Script.xml'))
LEAVES = LAYOUT.leaves['patch']          # (offset, size) per leaf, as the device answers them


def read_patch():
    out = bytearray()
    for lf in LEAVES:
        d = None
        for _ in range(4):                       # the reply can be lost: retry
            d = C.read_bytes(PATCH_ADDR + lf.offset, lf.size)
            if d is not None and len(d) >= lf.size:
                break
            time.sleep(0.2)
        if d is None or len(d) < lf.size:
            raise SystemExit(f'short read of {lf.path} at {lf.offset:#x}: {None if d is None else len(d)}')
        out += d[:lf.size]
    return bytes(out)


def write_patch(data):
    a = C.args()
    s, port = T.open_session(a)
    try:
        for lf in LEAVES:
            s._pump(); s.send(P.build(P.DT1, PATCH_ADDR + lf.offset, data[lf.offset:lf.offset + lf.size], a.device_id))
            time.sleep(0.05)
    finally:
        port.close()
    time.sleep(0.5)


def spread(name):
    p = da.record(name, note=45, secs=3.5, on=0.3, hold=2.8)
    x = np.fromfile(p, dtype='<f4').reshape(-1, 2).mean(1)[int(0.5 * 96000):int(2.8 * 96000)]
    S = np.abs(np.fft.rfft(x * np.hanning(len(x)))); f = np.fft.rfftfreq(len(x), 1 / 96000)
    c = (S * f).sum() / S.sum()
    return float(np.sqrt(((f - c) ** 2 * S).sum() / S.sum()))


def pc(n):
    port = da.midi_port()
    subprocess.run(['amidi', '-p', port, '-S', 'C0 %02x' % n]); time.sleep(1.0)


def main():
    v = int(sys.argv[1]) if len(sys.argv) > 1 else 13
    orig = read_patch()
    assert len(orig) == SIZE
    print('patch read', len(orig), 'bytes; X-MOD IN =', orig[256 + 104:256 + 112].hex(' '), flush=True)
    new = bytearray(orig)
    new[256 + 104:256 + 112] = X.encode(v, 8)
    try:
        write_patch(bytes(new))
        print('written; X-MOD IN now', read_patch()[256 + 104:256 + 112].hex(' '), flush=True)
        print(f'live, cord={v}: spread={spread("pw_live"):8.1f} Hz', flush=True)
        pc(1); pc(0)
        print(f'after PC 1/0, cord={v}: spread={spread("pw_pc"):8.1f} Hz', flush=True)
    finally:
        write_patch(orig)
        print('restored patch', flush=True)


if __name__ == '__main__':
    main()
