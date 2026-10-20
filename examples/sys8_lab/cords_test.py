"""Patch-cord test on S8LAB (slot 1, patch 0): OSC1 X-MOD IN source + OSC1 CROSS MOD amount.

PAT_NAME1 holds the cords (int8x4 values, 8 bytes each, range 0..13, source index into matrixOut):
  OSC1 X-MOD IN at PAT_NAME1 + 104  -> absolute patch offset 256 + 104
OSC1 CROSS MOD is the last value of the OSC1 leaf: patch offset 14 + 8 = 22 (2 nibble bytes, 0..255).
Each run: set the source value and the amount, play one note, measure the level modulation; restore at the end.

    python3 cords_test.py 0 2 3       # source values to try (0 = OFF control)
"""
import sys
import numpy as np
import color_test as C
import mod_scan as M
import plugout as P
import plugout_tool as T
import patches as PT

BASE = PT.patch_address(C.SLOT, C.PATCH, 0)
XMOD_SRC = BASE + 256 + 104          # PAT_NAME1 'OSC1 X-MOD IN', 8 bytes
CROSS = BASE + 14 + 8                # OSC1 'OSC1 CROSS MOD', 2 bytes


def write_bytes(addr, data):
    a = C.args()
    s, port = T.open_session(a)
    try:
        s._pump(); s.send(P.build(P.DT1, addr, bytes(data), a.device_id))
    finally:
        port.close()
    import time; time.sleep(0.25)


def encode(v, n):
    """2-nibble-per-byte encoding of an int8x4 / int2x4 value: low nibble in the last byte"""
    return bytes([(v >> (4 * (n - 1 - i))) & 0xF for i in range(n)])


def main():
    srcs = [int(x, 0) for x in sys.argv[1:]] or [0, 2, 3]
    orig_src = C.read_bytes(XMOD_SRC, 8)
    orig_cross = C.read_bytes(CROSS, 2)
    print('X-MOD IN now', orig_src.hex(' ') if orig_src else None, '| CROSS MOD now', orig_cross.hex(' ') if orig_cross else None, flush=True)
    try:
        write_bytes(CROSS, encode(255, 2))
        for v in srcs:
            write_bytes(XMOD_SRC, encode(v, 8))
            mean, rel_rms, rel_cen = M.modulation(f'cords_{v}')
            print(f'source={v:2d} CROSS=255  level={mean:.5f}  rms_mod={rel_rms:.3f}  centroid_mod={rel_cen:.3f}', flush=True)
    finally:
        if orig_src:
            write_bytes(XMOD_SRC, orig_src)
        if orig_cross:
            write_bytes(CROSS, orig_cross)
        print('restored X-MOD IN and CROSS MOD', flush=True)


if __name__ == '__main__':
    main()
