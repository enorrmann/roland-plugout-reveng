#!/usr/bin/env python3
"""build_c.py -- compile and link the C layer of ``m0_core0``.

The C tree is written by ``tools/gen_c.py``::

    c/osc.h          types, the OSC_P accessors, the decompiler's helpers
    c/code/fn_*.c    one file per function (generated)
    c/data/          the image's bytes the C does not spell out (generated)
    c/osc_d.h        a macro per fixed address (generated)
    c/fwd.h          a prototype per function (generated)
    c/config.h       which parts to compile

``arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb`` compiles it bare-metal.  This
tool runs the compiler over the tree and collects the diagnostics per source,
because the interesting number is not "does it compile" but *which* functions
do not and why:

    python3 tools/build_c.py --all             # every file
    python3 tools/build_c.py --one 0x600001C2  # one function, full output
    python3 tools/build_c.py --link            # compile + link an image

Nothing here needs Ghidra: the C is in the tree.
"""
from __future__ import annotations

import argparse
import os
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
C = os.path.join(ROOT, "c")

CC = "arm-none-eabi-gcc"
CFLAGS = ["-mcpu=cortex-m7", "-mthumb", "-mfloat-abi=soft", "-std=gnu11",
          "-ffreestanding", "-fno-common", "-fno-strict-aliasing",
          # the decompiled C is C89-style and works on 32-bit words: these
          # keep the compiler from rejecting it for things the ABI does not
          # care about (an int used as a pointer, a pointer as an int)
          "-Wno-int-conversion", "-Wno-incompatible-pointer-types",
          "-Wno-pointer-sign", "-Wno-implicit-function-declaration",
          "-fno-delete-null-pointer-checks"]


def sources():
    out = []
    for sub in ("code", "data"):
        d = os.path.join(C, sub)
        if not os.path.isdir(d):
            continue
        out += [os.path.join(d, n) for n in sorted(os.listdir(d)) if n.endswith(".c")]
    return out


def compile_one(src, objdir=None):
    cmd = [CC] + CFLAGS + ["-I", C]
    obj = None
    if objdir:
        rel = os.path.relpath(src, C).replace(os.sep, "_")
        obj = os.path.join(objdir, rel[:-2] + ".o")
        cmd += ["-c", "-o", obj]
    else:
        cmd += ["-fsyntax-only"]
    r = subprocess.run(cmd + [src], capture_output=True, text=True)
    return r.returncode, r.stderr, obj


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--all", action="store_true", help="compile every file")
    ap.add_argument("--one", type=lambda s: int(s, 0), help="one function address")
    ap.add_argument("--link", action="store_true", help="link an image")
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    ap.add_argument("-o", "--out", default=os.path.join(C, "build", "m0_core0.bin"))
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args(argv)
    if not (args.all or args.one is not None or args.link):
        ap.error("give --all, --one ADDR or --link")
    files = sources()
    if args.one is not None:
        want = "fn_%08X" % args.one
        files = [f for f in files if os.path.basename(f)[:-2] == want]
    errors = {}
    objs = []
    objdir = os.path.join(C, "build", "obj")
    os.makedirs(objdir, exist_ok=True)
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for src, (rc, err, obj) in zip(files, pool.map(lambda s: compile_one(s, objdir),
                                                       files)):
            if rc:
                errors[os.path.basename(src)] = err
            elif obj:
                objs.append(obj)
    print("build_c: %d file(s): %d compiled, %d with errors"
          % (len(files), len(files) - len(errors), len(errors)))
    for src, err in sorted(errors.items()):
        first = [l for l in err.splitlines() if " error:" in l]
        print("  %s: %d error(s)%s" % (src, len(first), (" -- " + first[0]) if first else ""))
        if args.verbose:
            print(err)
    if args.link and objs:
        with tempfile.NamedTemporaryFile("w", suffix=".rsp", delete=False) as f:
            f.write("\n".join(objs))
            rsp = f.name
        cmd = [CC] + CFLAGS + ["-nostdlib", "-nostartfiles",
                               "-Wl,--unresolved-symbols=ignore-all",
                               "-T", os.path.join(C, "m0_core0.ld"),
                               "-o", args.out, "@" + rsp]
        r = subprocess.run(cmd, capture_output=True, text=True)
        print("build_c: link %s -> %s" % ("ok" if r.returncode == 0 else "FAILED", args.out))
        if r.returncode:
            print(r.stderr[-4000:])
        return r.returncode
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
