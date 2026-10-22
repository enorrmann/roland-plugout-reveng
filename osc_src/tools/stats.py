#!/usr/bin/env python3
"""stats.py -- what the reconstruction contains, in one screen.

    python3 tools/stats.py

Reports the code/data split of the image, how many units the code layer
recovered as instruction text versus as exact bytes, and the size of the
generated sources and objects.
"""
from __future__ import annotations

import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)


def count(path, pattern=None):
    if not os.path.isdir(path):
        return 0
    n = 0
    for name in os.listdir(path):
        if pattern and not name.endswith(pattern):
            continue
        n += 1
    return n


def byte_form(path):
    """True for a generated unit whose first body line is ``.byte``."""
    seen_label = False
    try:
        with open(path) as f:
            for line in f:
                if line.startswith("fn_") or line.startswith("d_"):
                    seen_label = True
                    continue
                if seen_label and line.strip():
                    return line.lstrip().startswith(".byte")
    except OSError:
        pass
    return False


def main():
    asm_dir = os.path.join(ROOT, "code", "asm")
    units = [os.path.join(asm_dir, n) for n in sorted(os.listdir(asm_dir))] \
        if os.path.isdir(asm_dir) else []
    text = sum(1 for u in units if not byte_form(u))
    print("code units   %6d  (%d instruction text, %d exact bytes)"
          % (len(units), text, len(units) - text))
    print("data units   %6d" % count(os.path.join(ROOT, "data"), ".s"))
    for d in ("code/asm", "data", "build", "code/build"):
        p = os.path.join(ROOT, d)
        if os.path.isdir(p):
            size = sum(os.path.getsize(os.path.join(p, n))
                       for n in os.listdir(p) if os.path.isfile(os.path.join(p, n)))
            print("%-12s %6.1f MiB" % (d, size / (1 << 20)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
