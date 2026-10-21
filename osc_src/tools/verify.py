#!/usr/bin/env python3
"""verify.py -- compare a rebuilt image with the reference dump.

    python3 tools/verify.py code/build/os_full.bin ../dumps/os_full.bin

Reports the size, the SHA-256 of both files and, when they differ, the first
mismatching byte and a per-64 KiB summary.  Exit status is 0 only when the two
files are identical.
"""
from __future__ import annotations

import argparse
import hashlib
import sys


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("rebuilt")
    ap.add_argument("reference", nargs="?", default="../dumps/os_full.bin")
    ap.add_argument("--quiet", action="store_true")
    args = ap.parse_args(argv)

    with open(args.rebuilt, "rb") as f:
        got = f.read()
    with open(args.reference, "rb") as f:
        ref = f.read()
    same = got == ref
    if not args.quiet:
        print("%-40s %9d bytes  sha256 %s" % (args.rebuilt, len(got), sha256(args.rebuilt)))
        print("%-40s %9d bytes  sha256 %s" % (args.reference, len(ref), sha256(args.reference)))
    if same:
        print("verify: IDENTICAL")
        return 0
    n = min(len(got), len(ref))
    diffs = [i for i in range(n) if got[i] != ref[i]]
    print("verify: DIFFERENT -- %d of %d byte(s) differ" % (len(diffs) + abs(len(got) - len(ref)), max(len(got), len(ref))))
    if diffs:
        first = diffs[0]
        print("  first mismatch at +0x%05x: rebuilt %02x, reference %02x"
              % (first, got[first], ref[first]))
        # per 64 KiB summary
        for base in range(0, n, 0x10000):
            block = diffs_index(diffs, base, min(base + 0x10000, n))
            if block:
                print("  +0x%06x..0x%06x : %d byte(s) differ"
                      % (base, min(base + 0x10000, n), block))
    return 1


def diffs_index(diffs, lo, hi):
    return sum(1 for d in diffs if lo <= d < hi)


if __name__ == "__main__":
    sys.exit(main())
