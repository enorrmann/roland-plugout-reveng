#!/usr/bin/env python3
"""segment.py -- split os_full.bin into its *code* and its *data* bytes.

ARM Thumb-2 is self-synchronising: real code decodes from its first
instruction and keeps decoding until a literal pool or a data table
interrupts it, while data decoded as code almost never stays decodable for
long.  A linear Capstone sweep (``CS_MODE_THUMB | CS_MODE_MCLASS``, i.e. the
Cortex-M profile) over the whole image is therefore a reliable classifier --
provided the sweep is allowed to re-synchronise: a block that does not decode
is skipped byte by byte, which is what the CPU does when it walks into data.

Everything the sweep decodes is code; everything else is data.  The result is
cached so the generators do not re-sweep the image each time.

    python3 tools/segment.py                          # summary
    python3 tools/segment.py --cache build/seg.pkl    # cache for the generators

Requires capstone with the Cortex-M mode (capstone >= 5.0).
"""
from __future__ import annotations

import argparse
import os
import pickle
import sys

import capstone as cs

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
DEFAULT_IMAGE = os.path.join(ROOT, "..", "dumps", "os_full.bin")
BASE = 0x60000000


def sweep(img: bytes, base: int = BASE):
    """Linear Thumb-2 sweep.

    Returns (mask, insns): ``mask[i]`` is 1 for every byte covered by a decoded
    instruction; ``insns`` maps image offset -> (size, mnemonic, operands).
    """
    md = cs.Cs(cs.CS_ARCH_ARM, cs.CS_MODE_THUMB | cs.CS_MODE_MCLASS)
    n = len(img)
    mask = bytearray(n)
    insns = {}
    a = 0
    while a < n - 1:
        decoded = False
        for ins in md.disasm(img[a:a + 4], base + a):
            mask[a:a + ins.size] = b"\x01" * ins.size
            insns[a] = (ins.size, ins.mnemonic, ins.op_str)
            a += ins.size
            decoded = True
            break
        if not decoded:
            a += 1
    return mask, insns


def runs(mask: bytearray, value: int = 0):
    """Maximal runs of bytes whose mask equals ``value``: [(start, end), ...]."""
    out = []
    n = len(mask)
    i = 0
    while i < n:
        if bool(mask[i]) == bool(value):
            j = i
            while j < n and bool(mask[j]) == bool(value):
                j += 1
            out.append((i, j))
            i = j
        else:
            i += 1
    return out


def load(image=DEFAULT_IMAGE):
    with open(image, "rb") as f:
        return f.read()


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("image", nargs="?", default=DEFAULT_IMAGE)
    ap.add_argument("--cache", help="write (mask, runs, instruction table) here")
    args = ap.parse_args(argv)
    img = load(args.image)
    mask, insns = sweep(img)
    code_runs = runs(mask, 1)
    data_runs = runs(mask, 0)
    print(f"image    {len(img)} bytes at 0x{BASE:08X}")
    print(f"code     {sum(mask)} bytes ({100 * sum(mask) / len(img):.2f} %),"
          f" {len(insns)} instructions in {len(code_runs)} blocks")
    print(f"data     {len(img) - sum(mask)} bytes in {len(data_runs)} blocks")
    if args.cache:
        with open(args.cache, "wb") as f:
            pickle.dump({"base": BASE, "mask": bytes(mask), "code_runs": code_runs,
                         "data_runs": data_runs, "insns": insns}, f)
        print(f"cache    {args.cache}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
