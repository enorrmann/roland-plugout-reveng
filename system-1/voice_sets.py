#!/usr/bin/env python3
"""
voice_sets.py -- DSP program sets of the SYSTEM-1 slave app and their cost per sample.

The slave app loads DSP programs in "sets". A set is a run of 12-byte entries
{u32 program address (bytes), u32 length (bytes), u32 pointer to the program}:

    0x0000  head (data at 0x0000, then code)   0x100  main part (empty in voice sets)
    0x1900  voice 0           0x3100 voice 1
    0x4900  voice 2           0x6100 voice 3
    0x7FF2  7-halfword stub that runs on into the head at 0x0000 (program memory
            wraps at 0x8000 bytes)

i.e. four voice slots of 0x1800 bytes in one program memory. Programs are stored as
32-bit words with the two halfwords swapped (as everywhere else in the corpus).
Cost = number of DSP instructions (1 cycle each, DSP_HARDWARE.md; budget about 925-945
per sample, the exact edge depends on the DSP).

    python3 system-1/voice_sets.py [work/sys1_v130/app_slave.bin] [--entries]
"""
from __future__ import annotations

import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, os.path.join(ROOT, "examples", "from_scratch", "re"))
import dspdis  # noqa: E402

BASE = 0x60000000
SLOTS = (0x0, 0x100, 0x1900, 0x3100, 0x4900, 0x6100, 0x7FF2)
VOICE_SLOTS = (0x1900, 0x3100, 0x4900, 0x6100)
BUDGET = 925


def halfwords(img: bytes, ptr: int, nbytes: int) -> list[int]:
    o = ptr - BASE
    w = []
    for p in range(o, o + (nbytes + 3) // 4 * 4, 4):
        lo, hi = struct.unpack_from("<2H", img, p)
        w += [hi, lo]
    return w[:nbytes // 2]


def count(w: list[int]) -> int:
    """instructions in w; leading 0000/0001 halfwords (data words before the code) are skipped"""
    i = 0
    while i < len(w) and w[i] in (0x0000, 0x0001):
        i += 1
    return len(list(dspdis.instructions(w[i:]))) if i < len(w) else 0


def ninstr(img: bytes, ptr: int, nbytes: int) -> int:
    return count(halfwords(img, ptr, nbytes)) if nbytes else 0


def find_sets(img: bytes) -> list[list[tuple[int, int, int, int]]]:
    """runs of >= 5 consecutive {addr, len, ptr} entries; each entry = (entry addr, addr, len, ptr)"""
    ent = []
    for o in range(0, len(img) - 12, 4):
        a, n, p = struct.unpack_from("<3I", img, o)
        if a in SLOTS and BASE <= p < BASE + len(img) and n < 0x2000 and n % 2 == 0:
            ent.append((BASE + o, a, n, p))
    sets, cur = [], []
    for e in ent:
        if cur and e[0] - cur[-1][0] != 12:
            sets.append(cur)
            cur = []
        cur.append(e)
    sets.append(cur)
    return [s for s in sets if len(s) >= 5]


def main() -> int:
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    path = args[0] if args else os.path.join(ROOT, "work", "sys1_v130", "app_slave.bin")
    img = open(path, "rb").read()
    sets = find_sets(img)
    print(f"{os.path.relpath(path, ROOT)}: {len(sets)} sets")
    print(f"{'set':>10}  {'voices 0-3 (instr)':<22} {'other':>5} {'4 voices':>8} {'6 voices*':>9}")
    worst4 = worst6 = 0
    for s in sets:
        by = {a: (n, p) for _, a, n, p in s}
        v = [ninstr(img, by[a][1], by[a][0]) if a in by else 0 for a in VOICE_SLOTS]
        # end stub (0x7FF2), head (0x0000) and main part (0x100), each counted on its own
        other = sum(ninstr(img, by[a][1], by[a][0]) for a in (0x7FF2, 0x0, 0x100) if a in by)
        t4 = sum(v) + other
        t6 = t4 + 2 * max(v)
        worst4, worst6 = max(worst4, t4), max(worst6, t6)
        print(f"{s[0][0]:#10x}  {str(v):<22} {other:>5} {t4:>8} {t6:>9}")
        if "--entries" in sys.argv:
            for e, a, n, p in s:
                print(f"            {e:#x}: addr {a:#06x} len {n:#05x} ptr {p:#x}")
    print(f"worst set: 4 voices {worst4} instr, 6 voices {worst6} instr (budget ~{BUDGET})")
    print("* 6 voices = 4 voices + 2 more copies of the largest voice program (estimate)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
