#!/usr/bin/env python3
"""gen_functions.py -- the list of units the code layer is built from.

A *unit* is one function start: the reset entry, every call target (``bl`` /
``blx``), every tail-call target (``b.w``), and every function Ghidra found.
The list is sorted, so the generator can give each unit the span up to the next
one.

    python3 tools/gen_functions.py --cache build/seg.pkl -o build/functions.txt

``--ghidra FILE`` adds addresses exported by ``ghidra/Export.java`` (one hex
address per line); without it the list is built from the sweep alone, which is
what ``make`` uses so a clean checkout needs no Ghidra.
"""
from __future__ import annotations

import argparse
import os
import pickle
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

BASE = 0x60000000
END = BASE + 0x2CDA70
# the reset handler; the image has no other root
ENTRY = (BASE,)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--cache", default="build/seg.pkl")
    ap.add_argument("--ghidra", action="append", default=[],
                    help="extra entry addresses (one hex per line), repeatable")
    ap.add_argument("-o", "--out", default="build/functions.txt")
    args = ap.parse_args(argv)

    with open(args.cache, "rb") as f:
        insns = pickle.load(f)["insns"]
    funcs = set(ENTRY)
    for off, (size, mnem, ops) in insns.items():
        if mnem not in ("bl", "blx", "b.w", "b"):
            continue
        if not ops.startswith("#"):
            continue
        try:
            tgt = int(ops[1:], 16)
        except ValueError:
            continue
        if BASE <= tgt < END:
            funcs.add(tgt & ~1)
    for path in args.ghidra:
        with open(path) as f:
            for line in f:
                line = line.strip()
                if line:
                    funcs.add(int(line, 16) & ~1)
    funcs = sorted(funcs)
    os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)
    with open(args.out, "w") as f:
        f.write("\n".join("%x" % a for a in funcs) + "\n")
    print("gen_functions: %d unit(s) -> %s" % (len(funcs), args.out))
    return 0


if __name__ == "__main__":
    sys.exit(main())
