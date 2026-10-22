#!/usr/bin/env python3
"""
fnmatch.py -- find a function of one SYSTEM-1 OS build in another build.

The bytes at ADDR (N bytes) in IMAGE_A are turned into a pattern where the
PC-relative parts are wildcards (32-bit BL/B.W, `ldr rX, [pc, #imm]`, `adr`), and the
pattern is searched in IMAGE_B (halfword aligned). Both images load at 0x60000000.

    python3 system-1/fnmatch.py work/sys1_v120/app_master.bin work/sys1_v130/app_master.bin \
        0x60006606:28 0x60006534:64

Used to carry addresses found in 1.20 (the plug-in's image) over to 1.30 (the device).
"""
from __future__ import annotations

import re
import struct
import sys

BASE = 0x60000000


def pattern(b: bytes) -> bytes:
    out, i = b"", 0
    while i + 1 < len(b):
        h = struct.unpack_from("<H", b, i)[0]
        if h >> 11 in (0x1D, 0x1E, 0x1F):           # 32-bit Thumb-2 instruction
            out += b".{4}" if h >> 11 == 0x1E else re.escape(b[i:i + 4])
            i += 4
            continue
        if h >> 11 in (0x09, 0x14):                 # ldr literal / adr: offset byte is a wildcard
            out += b"." + re.escape(b[i + 1:i + 2])
        else:
            out += re.escape(b[i:i + 2])
        i += 2
    return out


def find(a: bytes, b: bytes, addr: int, n: int) -> list[int]:
    pat = pattern(a[addr - BASE:addr - BASE + n])
    return [BASE + m.start() for m in re.finditer(pat, b, re.S) if m.start() % 2 == 0]


def main() -> int:
    if len(sys.argv) < 4:
        print(__doc__)
        return 2
    a = open(sys.argv[1], "rb").read()
    b = open(sys.argv[2], "rb").read()
    for spec in sys.argv[3:]:
        addr, n = spec.split(":")
        hits = find(a, b, int(addr, 16), int(n))
        print(f"{int(addr, 16):#010x} ({n} bytes): " + (", ".join(f"{h:#010x}" for h in hits[:8]) or "not found"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
