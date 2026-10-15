"""The DSP's internal number format ("RAW" view of its data banks).

Measured on hardware (2026-10-14): 74 floats written through the IEEE view of
master DSP4's bank 5 (DPC4 0x41124E00) and read back through the SD31 / RAW
views (bulk_dump.py --tri, CONV=...). All of them fit:

    bit 31      sign (sign-magnitude)
    bits 30-29  exponent s = 0..3, one step = x16
    bits 28-0   magnitude M (29 bits)
    value = +-M / 2^(32 - 4 s)

s = 0: [0, 0.125) (= the SD31 value x 2), 1: [0.125, 2), 2: [2, 32), 3: [32, 512).
The IEEE view converts both ways; SD31 is Q31 saturated to [-1, 1).
"""
import math


def decode(r):
    v = (r & 0x1FFFFFFF) / 2.0 ** (32 - 4 * ((r >> 29) & 3))
    return -v if r >> 31 else v


def encode(v):
    a = abs(v)
    s = 0
    while s < 3 and a >= 0.125 * 16 ** s:
        s += 1
    m = int(round(a * 2.0 ** (32 - 4 * s)))
    if m >= 1 << 29:
        if s == 3:
            m = (1 << 29) - 1
        else:
            s += 1
            m = int(round(a * 2.0 ** (32 - 4 * s)))
    return (0x80000000 if v < 0 else 0) | s << 29 | m


if __name__ == '__main__':
    import sys
    for a in sys.argv[1:]:
        if a.startswith('0x'):
            print(a, decode(int(a, 16)))
        else:
            print(a, f'{encode(float(a)):08x}')
