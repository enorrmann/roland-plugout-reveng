#!/usr/bin/env python3
"""gen_decomp.py -- decompile the OS with Ghidra (headless).

This is the path from the reconstructed *bytes* to readable C.  It drives
Ghidra once over the image and then decompiles a list of functions, writing one
``.c`` file per function.  It is kept separate from the byte-exact rebuild on
purpose: the rebuild must not depend on Ghidra, and decompiled C does not
reproduce the original bytes, only their meaning.

    # 1. import at the right base and seed every function
    python3 tools/gen_decomp.py analyze --ghidra ~/ghidras/ghidra_11.3.2_PUBLIC
    # 2. decompile (all functions, or a list of addresses)
    python3 tools/gen_decomp.py decompile --ghidra ~/ghidras/ghidra_11.3.2_PUBLIC
    python3 tools/gen_decomp.py decompile --ghidra ~/ghidras/ghidra_11.3.2_PUBLIC \
        0x600001c2 0x60000000

Output: ``work/ghidra/`` (project) and ``decomp/<address>.c``.

Notes that cost time to learn, encoded here:

* The image must be imported at ``0x60000000``.  ``-loader-baseAddr`` sets the
  memory block but leaves the program's *image base* at 0, and the analyzer
  then fails to disassemble anything; the ``SetBase`` post-script fixes it.
* Ghidra only finds functions reachable from the entry point, so the call
  targets from ``tools/gen_functions.py`` are seeded explicitly (``Sweep``).
* Analysis of the full 2.9 MB image takes a few minutes and produces ~9 000
  functions.
"""
from __future__ import annotations

import argparse
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
IMAGE = os.path.join(ROOT, "..", "dumps", "os_full.bin")
SCRIPTS = os.path.join(ROOT, "ghidra")
BASE = 0x60000000


def headless(ghidra, *args):
    exe = os.path.join(ghidra, "support", "analyzeHeadless")
    if not os.path.exists(exe):
        raise SystemExit("gen_decomp: %s not found (pass --ghidra)" % exe)
    env = dict(os.environ)
    env.setdefault("XDG_CONFIG_HOME", os.path.join(ROOT, "build", "xdg"))
    os.makedirs(env["XDG_CONFIG_HOME"], exist_ok=True)
    # -scriptPath must be set before any -postScript/-preScript, and scripts
    # cannot be given arguments twice, so build the common prefix here
    return subprocess.run([exe] + list(args), env=env)


def analyze(args):
    proj = os.path.join(ROOT, "work", "ghidra")
    os.makedirs(proj, exist_ok=True)
    funcs = os.path.join(ROOT, "build", "functions.txt")
    if not os.path.exists(funcs):
        os.system("%s %s --cache %s -o %s" % (
            sys.executable, os.path.join(HERE, "gen_functions.py"),
            os.path.join(ROOT, "build", "seg.pkl"), funcs))
    subprocess.run([sys.executable, os.path.join(HERE, "segment.py"),
                    IMAGE, "--cache", os.path.join(ROOT, "build", "seg.pkl")],
                   check=True)
    r = headless(args.ghidra, proj, "osc", "-import", IMAGE,
                 "-processor", "ARM:LE:32:Cortex",
                 "-loader", "BinaryLoader", "-loader-baseAddr", hex(BASE),
                 "-scriptPath", SCRIPTS, "-preScript", "SetBase.java",
                 "-analysisTimeoutPerFile", "3600")
    return r.returncode


def decompile(args):
    proj = os.path.join(ROOT, "work", "ghidra")
    out = os.path.join(ROOT, "decomp")
    r = headless(args.ghidra, proj, "osc", "-process", "os_full.bin",
                 "-readOnly", "-noanalysis", "-scriptPath", SCRIPTS,
                 "-preScript", "DecompileOne.java",
                 *[hex(a) for a in args.addresses])
    return r.returncode


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--ghidra", default=os.path.expanduser("~/ghidras/ghidra_11.3.2_PUBLIC"))
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("analyze", help="import and analyse the image")
    d = sub.add_parser("decompile", help="decompile functions")
    d.add_argument("addresses", nargs="*", type=lambda s: int(s, 0))
    args = ap.parse_args(argv)
    if args.cmd == "analyze":
        return analyze(args)
    if not args.addresses:
        print("gen_decomp: give at least one address (see build/functions.txt)")
        return 1
    return decompile(args)


if __name__ == "__main__":
    sys.exit(main())
