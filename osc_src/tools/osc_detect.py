#!/usr/bin/env python3
"""osc_detect.py -- the space model the C layer needs, derived from the dump.

The image is the *run-time* picture of the master BMC's core 0.  Its addresses
fall into a few spaces, and the C layer has to treat each one differently:

    code        functions and their literal pools      (inside the text)
    rodata      tables, strings, jump tables           -> linked at its address
    bss         zeros a start-up routine cleared       -> .bss, no contents
    data        initialised variables                  -> copied from ROM at
                                                          start-up, as the OS does
    mmio        device registers                       -> absolute, never copied

Reconstructing the copy means knowing what is initialised, which is exactly
the distinction ARM builds express with the *copies in the image*: a constant
is a byte in the image that code reaches through a literal pool ("only looked
up").  Everything else in the data area is either zero or carried to RAM by a
start-up routine.  ``not-in-image`` therefore cannot be a state for a place
that the image holds, so:

    only looked up (never written) -> read-only constant   (G1)
    read *and* written             -> variable              (G2)

and a word holding a data address in the writable space is an *absolute*
pointer into RAM (or a deliberate alias into a constant table), which is why
the image has to be linked ``-N`` and cannot be made relocatable without
changing meaning.

    python3 tools/osc_detect.py ../dumps/os_full.bin --report

The report is a measurement of one image, not a rule: it is how the tables in
``gen_c.py`` were derived and how a change to them is checked.
"""
from __future__ import annotations

import argparse
import bisect
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

BASE = 0x60000000
M0_END = 0x60130E00
TEXT_END = 0x60094800
GROUP = 0x2F0
MMIO = ((0x40000000, 0x60000000), (0xE0000000, 0xE0100000))
RAM = ((0x01000000, 0x01200000), (0x20000000, 0x20400000))


def space(v):
    if BASE <= v < BASE + M0_END - BASE:
        return "image"
    for lo, hi in MMIO:
        if lo <= v < hi:
            return "mmio"
    for lo, hi in RAM:
        if lo <= v < hi:
            return "ram"
    return "other"


def group_of(off):
    return off // GROUP


def load(path):
    with open(path, "rb") as f:
        return f.read()


def scan(img, base=BASE):
    """Classify every read of an image address by the code (word accesses)."""
    import capstone as cs
    from capstone import arm as A
    md = cs.Cs(cs.CS_ARCH_ARM, cs.CS_MODE_THUMB | cs.CS_MODE_MCLASS)
    md.detail = True
    md.skipdata = True
    reads, writes = set(), set()
    tgt = None
    for ins in md.disasm(img, base):
        mn = ins.mnemonic
        if mn == ".byte":
            continue
        if mn in ("ldr", "ldr.w") and len(ins.operands) == 2:
            op = ins.operands[1]
            if op.type == A.ARM_OP_MEM and op.mem.base == A.ARM_REG_PC and op.mem.index == 0:
                a = ((ins.address + 4) & ~3) + op.mem.disp
                o = a - base
                if 0 <= o <= len(img) - 4:
                    tgt = int.from_bytes(img[o:o + 4], "little")
                    if base <= tgt < base + len(img):
                        reads.add(tgt)
            elif op.type == A.ARM_OP_MEM and op.mem.base in (A.ARM_REG_R0, A.ARM_REG_R1):
                pass
        elif mn in ("ldr", "ldr.w") and len(ins.operands) <= 2:
            pass
        if mn in ("str", "str.w", "strb", "strh") and ins.operands:
            v = ins.operands[-1] if ins.operands[-1].type == A.ARM_OP_IMM else None
            if v is not None and base <= v.imm < base + len(img):
                writes.add(v.imm)
    return reads, writes


def walk_groups(img, reads, writes, text_end=TEXT_END):
    """{group: kind} for every group of the data area."""
    kinds = {}
    for off in range(text_end - BASE, len(img), GROUP):
        chunk = img[off:off + GROUP]
        if not chunk:
            continue
        addrs = range(BASE + off, BASE + off + len(chunk))
        r = any(a in reads for a in addrs)
        w = any(a in writes for a in addrs)
        if r and w:
            kinds[off] = "var"
        elif r:
            kinds[off] = "const"
        elif not any(chunk):
            kinds[off] = "zero"
        else:
            kinds[off] = "opaque"
    return kinds


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("image", nargs="?", default=os.path.join(ROOT, "..", "dumps", "os_full.bin"))
    ap.add_argument("--report", action="store_true")
    args = ap.parse_args(argv)
    img = load(args.image)
    reads, writes = scan(img)
    kinds = walk_groups(img, reads, writes)
    if args.report:
        from collections import Counter
        print("words read by code      %6d" % len(reads))
        print("words written by code   %6d" % len(writes))
        print("groups (%#x B)          %6d  %s" % (GROUP, len(kinds),
                                                    Counter(kinds.values())))
        # bytes per kind
        bytal = Counter()
        for off, k in kinds.items():
            bytal[k] += len(img[off:off + GROUP])
        print("bytes per kind          " + ", ".join("%s=%d" % kv for kv in sorted(bytal.items())))
    return 0


if __name__ == "__main__":
    sys.exit(main())
