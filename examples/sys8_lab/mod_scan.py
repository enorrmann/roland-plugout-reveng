"""Scan MOD ID values on S8LAB (slot 1, patch 0) for audible modulation.

For each value k: MOD ID 1 = k, MOD PRM 1 = 255 (EXTEND block, 2 nibble bytes per value), one held note,
then the variation over time of level and centroid in 50 ms windows. The EXTEND block is restored at the end.

    python3 mod_scan.py 0 16        # values from 0 up to (not including) 16
"""
import sys
import numpy as np
import color_test as C
import plugout as P
import plugout_tool as T
import patches as PT

EXT = next(x for x in C.LAYOUT.leaves['patch'] if x.path == 'EXTEND')
EXT_ADDR = PT.patch_address(C.SLOT, C.PATCH, 0) + EXT.offset
# value offsets inside EXTEND (2 bytes each): MOD ID 1..3, MOD PRM 1..3
ID1, PRM1 = 0, 6


def write(addr, v):
    a = C.args()
    s, port = T.open_session(a)
    try:
        s._pump(); s.send(P.build(P.DT1, addr, bytes([(v >> 4) & 0xF, v & 0xF]), a.device_id)); 
    finally:
        port.close()
    import time; time.sleep(0.25)


def modulation(name):
    import dspaudio as da
    p = da.record(name, note=45, secs=3.5, on=0.3, hold=2.8)
    x = np.fromfile(p, dtype='<f4').reshape(-1, 2).mean(1)[int(0.5 * 96000):int(2.8 * 96000)]
    W = 4800
    rms, cen = [], []
    for i in range(len(x) // W):
        s = x[i * W:(i + 1) * W]
        r = float(np.sqrt((s ** 2).mean()))
        S = np.abs(np.fft.rfft(s * np.hanning(W))); f = np.fft.rfftfreq(W, 1 / 96000)
        rms.append(r); cen.append(float((S * f).sum() / max(S.sum(), 1e-12)))
    rms, cen = np.array(rms), np.array(cen)
    return float(rms.mean()), float(rms.std() / max(rms.mean(), 1e-9)), float(cen.std() / max(cen.mean(), 1e-9))


def main():
    lo, hi = (int(sys.argv[1]), int(sys.argv[2])) if len(sys.argv) > 2 else (0, 16)
    orig = C.read_bytes(EXT_ADDR, EXT.size)
    print('EXTEND', EXT.size, 'bytes', orig.hex(' ') if orig else None, flush=True)
    try:
        base = None
        for k in range(lo, hi):
            write(EXT_ADDR + ID1, k)
            write(EXT_ADDR + PRM1, 255)
            mean, rel_rms, rel_cen = modulation(f'mod_{k}')
            print(f'MOD ID={k:3d}  level={mean:.5f}  rms_mod={rel_rms:.3f}  centroid_mod={rel_cen:.3f}', flush=True)
    finally:
        if orig:
            a = C.args(); s, port = T.open_session(a); s._pump()
            s.send(P.build(P.DT1, EXT_ADDR, bytes(orig), a.device_id)); port.close()
            print('EXTEND restored', flush=True)


if __name__ == '__main__':
    main()
