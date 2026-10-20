"""OSC1 COLOR test on S8LAB (slot 1, patch 0).

Writes the COLOR value with a DT1 (device address = patch address + OSC1 offset + 2), plays one note per value and
prints level and spectral centroid. At the end it restores the value it read first.

    python3 color_test.py 0 255 93      # values to test (the first one read is restored at the end)

COLOR is the second 2-nibble value of the OSC1 leaf (order from the SYSTEM-8 Script.xml: WAVE, COLOR, MOD, RANGE,
CROSS MOD); it is the parameter 0x6D5, between DCO1 WAVEFORM 0x6D4 and DCO1 FREQ MOD 0x6D6.
"""
import argparse, os, sys, time
import numpy as np

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
import patches as PT          # noqa: E402
import plugout as P           # noqa: E402
import plugout_tool as T      # noqa: E402
import dspaudio as da         # noqa: E402

SLOT, PATCH = 1, 0
PLUGIN = os.path.join(ROOT, 'plugins', 'SYSTEM-8')
LAYOUT = PT.ScriptLayout(os.path.join(PLUGIN, 'Script', 'Script.xml'))
OSC1 = next(x for x in LAYOUT.leaves['patch'] if x.path == 'OSC1')
COLOR = PT.patch_address(SLOT, PATCH, 0) + OSC1.offset + 2
WAVE_ADDR = PT.patch_address(SLOT, PATCH, 0) + OSC1.offset


def args():
    return argparse.Namespace(backend=None, port=None, device_id=P.DEFAULT_DEVICE_ID, gap_ms=0.0,
                              wire_pacing=False, verbose=False, timeout=1.5)


def read_bytes(addr, n):
    a = args()
    s, port = T.open_session(a)
    try:
        rq = P.build(P.RQ1, addr, P.addr_bytes(n), a.device_id)
        s._pump(); s._inbox.clear(); s.send(rq)
        got, end = [], time.monotonic() + a.timeout
        while time.monotonic() < end and not got:
            s._pump(); got, s._inbox = s._inbox, []; time.sleep(0.01)
        return P.sysex_data(got[0]) if got else None
    finally:
        port.close()


def write_value(v):
    a = args()
    s, port = T.open_session(a)
    try:
        data = bytes([(v >> 4) & 0xF, v & 0xF])
        s._pump(); s.send(P.build(P.DT1, COLOR, data, a.device_id)); time.sleep(0.3)
    finally:
        port.close()


def measure(name):
    p = da.record(name, note=45, secs=3.0, on=0.3, hold=2.0)
    x = np.fromfile(p, dtype='<f4').reshape(-1, 2).mean(1)
    seg = x[int(0.6 * 96000):int(2.4 * 96000)]
    rms = float(np.sqrt((seg ** 2).mean()))
    S = np.abs(np.fft.rfft(seg * np.hanning(len(seg))))
    f = np.fft.rfftfreq(len(seg), 1 / 96000)
    cen = float((S * f).sum() / max(S.sum(), 1e-12))
    return rms, cen


def main():
    vals = [int(x, 0) for x in sys.argv[1:]] or [0, 255]
    orig = read_bytes(COLOR, 2)
    orig_v = (orig[0] << 4 | orig[1]) if orig else None
    print(f'COLOR now: {orig_v} (bytes {orig.hex(" ") if orig else None})')
    try:
        for v in vals:
            write_value(v)
            rms, cen = measure(f'color_{v}')
            print(f'COLOR={v:3d}  rms={rms:.5f}  centroid={cen:8.1f} Hz')
    finally:
        if orig_v is not None:
            write_value(orig_v)
            print(f'restored COLOR={orig_v}')


if __name__ == '__main__':
    main()
