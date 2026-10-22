#!/usr/bin/env python3
"""gen_c.py -- Ghidra's decompilation of ``m0_core0`` as C that compiles.

Six passes over one Ghidra export (``ghidra/Dump.java``, driven by
``tools/ghidra_dump.py``; ``make ghidra`` runs both):

1. preflight   the dump itself: coverage of the reference image against the
               listing and the sweep, duplicate function entries, and the
               jump targets Ghidra did not follow.  Run it first: a
               mis-decompiled function shows up here rather than as a compiler
               error 10 000 lines on.
2. data        the image's bytes the C does not spell out as expressions.
               The sweep's code bytes are a *code image* (``.text``, relinked
               from the generated functions); everything else is data
               (``data/``, the ``.osc_data`` section, RAM at run time).  The
               holes the code image leaves -- literal-pool words the
               decompiler folded away -- are carried in the declaration table
               and patched back into ``.text`` at their addresses.
3. enum        the group (``g_``) and function (``fn_``) enumerations, which
               key every target in the passes below.
4. dtable      the declaration table ``osc_d.h``: one ``OSC_*`` macro per
               function and per place, so every fixed address is typed.
5. code        the C functions (``code/``), one .c per contiguous run.
6. fwd         the prototypes, so that a call is declared before it is used.

The passes are text in, text out; each reads only what it needs, and the order
the decompiled functions arrive in never reaches their text.  ``--func`` keeps
the loop short while the rewriter is tuned.

Addresses are ``m0 | g`` (``tools/segment_m0.py``).  The output is C11 and
needs no headers to parse.
"""
from __future__ import annotations

import argparse
import os
import pickle
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

BASE = 0x60000000
M0_END = 0x60130E00
TEXT_END = 0x60094800
GROUP = 0x2F0
HEADER_GROUP = 0x180           # the gap before a table's literal pool, where
                               # the declaration table's macros live
SEP = re.compile(r"^///// ([0-9a-fA-F]+) (\S+)$")


def log(msg):
    print("gen_c: " + msg, file=sys.stderr)


# --------------------------------------------------------------- addresses ---
def g_of(off):
    return (off // GROUP) & 0xFF


def m0_addr(off):
    """``m0 | g`` for an image offset: the group and the offset inside it."""
    return "%07X%02X" % (g_of(off), off % GROUP)


def group_start(off):
    """Where the group's literal pool begins (the C header of the group)."""
    g = g_of(off)
    return g * GROUP - HEADER_GROUP


def group_end(off):
    return group_start(off) + HEADER_GROUP + GROUP


# --------------------------------------------------------------- the dump ----
class Dump:
    def __init__(self, path):
        self.path = path
        self.units = self._units(os.path.join(path, "units.txt"))
        self.funcs = self._funcs(os.path.join(path, "funcs.txt"))
        self.decomp = self._decomp(os.path.join(path, "decomp.txt"))

    @staticmethod
    def _units(path):
        out = {}
        with open(path) as f:
            for line in f:
                parts = line.rstrip("\n").split(" ", 4)
                if len(parts) < 4:
                    continue
                try:
                    addr, ln = int(parts[0], 16), int(parts[1])
                    raw = bytes.fromhex(parts[2])
                except ValueError:
                    continue
                out.setdefault(addr, (raw, parts[3] == "D"))
        return out

    @staticmethod
    def _funcs(path):
        out = []
        with open(path) as f:
            for line in f:
                p = line.split()
                if len(p) == 2:
                    try:
                        out.append((int(p[0], 16), p[1]))
                    except ValueError:
                        pass
        return out

    @staticmethod
    def _decomp(path):
        out, cur, buf = [], None, []
        with open(path, errors="replace") as f:
            for line in f:
                m = SEP.match(line.rstrip("\n"))
                if m:
                    if cur is not None:
                        out.append((cur[0], cur[1], "".join(buf)))
                    cur, buf = (int(m.group(1), 16), m.group(2)), []
                elif cur is not None:
                    buf.append(line)
        if cur is not None:
            out.append((cur[0], cur[1], "".join(buf)))
        return out


def image_bytes():
    with open(os.path.join(ROOT, "..", "dumps", "os_full.bin"), "rb") as f:
        return f.read()


def sweep():
    path = os.path.join(ROOT, "build", "seg.pkl")
    if not os.path.exists(path):
        return None
    with open(path, "rb") as f:
        return pickle.load(f)


# ------------------------------------------------------------------- data ----
class Layout:
    """Which bytes the C expresses and which the data layer carries.

    Byte ``h`` of the image (offset from 0x60000000) is *data* -- carried by
    the data layer instead of being spelled by a decompiled function -- when
    it is after the text, or when neither the sweep nor Ghidra read it as
    code.  (Only Ghidra's code can be decompiled; it follows paths the linear
    sweep cannot, so the two sources are merged, not compared.)
    """

    def __init__(self, d, img, sw):
        self.img = img
        self.d = d
        n = M0_END - BASE
        mask = sw["mask"] if sw is not None else b"\x01" * n
        covered = bytearray(n)
        for (a, (raw, _isd)) in d.units.items():
            o = a - BASE
            if 0 <= o < n and o + len(raw) <= n:
                covered[o:o + len(raw)] = b"\x01" * len(raw)
        text_end = TEXT_END - BASE
        self.text_end = text_end
        data = bytearray(n)
        for h in range(text_end):
            if mask[h] == 0 and covered[h] == 0:
                data[h] = 1
        data[text_end:] = b"\x01" * (n - text_end)
        self.mask = mask
        self.is_data = data
        # runs of data, one pass over the map
        runs = []
        start = None
        for h in range(n):
            if data[h] and start is None:
                start = h
            elif not data[h] and start is not None:
                runs.append((start, h))
                start = None
        if start is not None:
            runs.append((start, n))
        self.data_runs = runs
        # a gap at or before a group's header belongs to the run that follows
        # it, a gap after the header to the run before: two groups never share
        # objects
        merged = []
        for a, b in runs:
            hdr = group_start(a)
            if hdr < a and not (merged and merged[-1][1] > hdr):
                a = hdr
            if merged and a <= merged[-1][1]:
                merged[-1][1] = max(merged[-1][1], b)
            else:
                merged.append([a, b])
        self.runs = [tuple(r) for r in merged]
        self.holes = sum(1 for h in range(text_end) if data[h])


# ---------------------------------------------------------------- preflight --
def preflight(d, img, sw):
    starts = sorted({a for a, _ in d.funcs})
    log("preflight: %d listing units, %d function entries, %d decompiled"
        % (len(d.units), len(starts), len(d.decomp)))
    log("preflight: %d entries with several names" % (len(d.funcs) - len(starts)))
    n = M0_END - BASE
    covered = bytearray(n)
    for (a, (raw, _isd)) in d.units.items():
        o = a - BASE
        if 0 <= o < n and o + len(raw) <= n:
            covered[o:o + len(raw)] = b"\1" * len(raw)
    holes = [i for i in range(n) if not covered[i]]
    log("preflight: %d byte(s) of m0_core0 not in the listing" % len(holes))
    if sw is not None:
        mask = sw["mask"]
        diff = sum(1 for i in range(n) if (mask[i] == 1) != (covered[i] == 1))
        log("preflight: %d byte(s) where Ghidra's code/data split differs from "
            "the sweep" % diff)
    missing = [a for a, _ in d.funcs if a not in d.units]
    if missing:
        log("preflight: %d entries with no instruction (first %#x)"
            % (len(missing), missing[0]))
    noargs = sum(1 for _a, _n, t in d.decomp
                 if re.search(r"\bvoid\s+\w+\s*\(\s*void\s*\)", t[:400]))
    log("preflight: %d decompiled function(s) take no argument" % noargs)
    return not holes and not missing


# ------------------------------------------------------------- the passes ----
def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("passes", nargs="+", help="preflight data enum dtable code fwd | all")
    ap.add_argument("--dump", required=True, help="ghidra/Dump.java output directory")
    ap.add_argument("--out", default=os.path.join(ROOT, "c"))
    ap.add_argument("--func", type=lambda s: int(s, 0), help="one function (iteration)")
    args = ap.parse_args(argv)
    passes = args.passes
    if "all" in passes:
        passes = ["preflight", "data", "enum", "dtable", "code", "fwd"]
    d = Dump(args.dump)
    img = image_bytes()
    sw = sweep()
    if "preflight" in passes:
        preflight(d, img, sw)
    if "data" in passes:
        lay = Layout(d, img, sw)
        log("data: %d run(s) of data, %d hole(s) in the code image"
            % (len(lay.data_runs), lay.holes))
        for a, b in lay.data_runs[:40]:
            print("  %#x..%#x %d" % (a, b, b - a))
    return 0


if __name__ == "__main__":
    sys.exit(main())
