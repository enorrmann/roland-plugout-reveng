#!/usr/bin/env python3
"""ghidra_dump.py -- drive ``ghidra/Dump.java`` over the image.

Two passes over one Ghidra project:

``analyze``   import the image, auto-analyse it.  The import is the expensive
              part (minutes for the 1.2 MB ``m0_core0`` text, and the same
              again for the decompiler's first call on each function), so the
              project is kept in ``work/ghidra/`` and reused.
``dump``      open the analysed project and write ``units.txt``, ``funcs.txt``
              and ``decomp.txt`` into ``--out``; ``gen_c.py`` reads those.

Ghidra needs a JDK and a writable configuration directory.  The system Ghidra
config (``~/.config/ghidra``) can be read-only or hold a stale path; a private
``XDG_CONFIG_HOME`` under ``work/`` avoids that, which is why the wrapper sets
it instead of relying on the default.

    python3 tools/ghidra_dump.py analyze
    python3 tools/ghidra_dump.py dump            # m0_core0 [0x60000000 0x60130E00]
    python3 tools/ghidra_dump.py dump --part m1_tcm

Requires Ghidra 11.x (``--ghidra``, default ``~/ghidras/ghidra_11.3.2_PUBLIC``);
nothing here is needed for the byte-exact rebuild, only for the C layer.
"""
from __future__ import annotations

import argparse
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
REPO = os.path.dirname(ROOT)

# The six programs of the image (docs/BASE_FIRMWARE.md §8.1): (offset, size),
# both relative to the dump.
PARTS = {
    "m0_core0": (0x000000, 0x130E00),
    "m1_tcm": (0x130E00, 0x42BBC),
    "m1_sdram": (0x173A00, 0x7D74C),
    "s_boot": (0x1F1200, 0x1D328),
    "s_data": (0x20E600, 0x7D114),
    "subcpu": (0x28B730, 0x4D68),
}
BASE = 0x60000000
DEFAULT_GHIDRA = os.path.expanduser("~/ghidras/ghidra_11.3.2_PUBLIC")


def env():
    e = dict(os.environ)
    e.pop("JAVA_HOME", None)          # let the launcher find java on PATH
    e["XDG_CONFIG_HOME"] = os.path.join(REPO, "work", "ghidra_cfg")
    os.makedirs(e["XDG_CONFIG_HOME"], exist_ok=True)
    return e


def headless(ghidra, *args):
    exe = os.path.join(ghidra, "support", "analyzeHeadless")
    if not os.path.exists(exe):
        raise SystemExit("ghidra_dump: %s not found (pass --ghidra)" % exe)
    return subprocess.run([exe] + list(args), env=env()).returncode


def analyze(args):
    part = args.part
    off, size = PARTS[part]
    proj = os.path.join(REPO, "work", "ghidra")
    raw = os.path.join(REPO, "work", "c_dump", part + ".bin")
    os.makedirs(os.path.dirname(raw), exist_ok=True)
    if not os.path.exists(raw):
        with open(os.path.join(REPO, "dumps", "os_full.bin"), "rb") as f:
            f.seek(off)
            data = f.read(size)
        with open(raw, "wb") as f:
            f.write(data)
        print("ghidra_dump: %s (%d bytes) -> %s" % (part, len(data), raw))
    os.makedirs(proj, exist_ok=True)
    return headless(args.ghidra, proj, "osc", "-import", raw,
                    "-processor", "ARM:LE:32:Cortex",
                    "-loader", "BinaryLoader", "-loader-baseAddr", hex(BASE),
                    "-scriptPath", os.path.join(ROOT, "ghidra"),
                    "-preScript", "SetBase.java",
                    "-analysisTimeoutPerFile", str(args.timeout))


def dump(args):
    part = args.part
    off, size = PARTS[part]
    start, end = BASE + off, BASE + off + size
    proj = os.path.join(REPO, "work", "ghidra")
    raw = os.path.join(REPO, "work", "c_dump", part + ".bin")
    out = os.path.abspath(args.out)
    return headless(args.ghidra, proj, "osc", "-process", part + ".bin",
                    "-noanalysis", "-scriptPath", os.path.join(ROOT, "ghidra"),
                    "-postScript", "Dump.java", out, "%x" % start, "%x" % end)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--ghidra", default=DEFAULT_GHIDRA)
    ap.add_argument("--part", default="m0_core0", choices=sorted(PARTS))
    ap.add_argument("--timeout", type=int, default=7200,
                    help="analysis timeout per file, seconds")
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("analyze")
    d = sub.add_parser("dump")
    d.add_argument("-o", "--out", default=os.path.join(ROOT, "build", "ghidra"))
    args = ap.parse_args(argv)
    if args.cmd == "analyze":
        return analyze(args)
    return dump(args)


if __name__ == "__main__":
    sys.exit(main())
