#!/usr/bin/env python3
"""rebuild.py -- link the generated sources into an image and check it.

This is the driver behind ``make``.  It assembles every unit in ``code/asm``
(the code layer) and ``data`` (the data layer), places the resulting objects
with ``build_le.py`` and compares the image with the reference dump.

    python3 tools/rebuild.py                     # full build + verify
    python3 tools/rebuild.py --jobs 8            # parallel assembly
    python3 tools/rebuild.py --keep              # keep the objects

The build is incremental: ``build/obj`` holds one ``.o`` per source file and
only sources newer than their object are assembled again.
"""
from __future__ import annotations

import argparse
import os
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)

import build_le      # noqa: E402

AS = "arm-none-eabi-as"
AS_FLAGS = ["-mthumb", "-mcpu=cortex-m7", "-mimplicit-it=always"]
BASE = 0x60000000
SIZE = 0x2CDA70


def sources(dirs):
    out = []
    for d in dirs:
        if not os.path.isdir(d):
            continue
        for name in sorted(os.listdir(d)):
            if name.endswith(".s"):
                out.append(os.path.join(d, name))
    return out


def obj_for(src, objdir):
    rel = os.path.relpath(src, ROOT).replace(os.sep, "_")
    return os.path.join(objdir, rel[:-2] + ".o")


def assemble_one(src, objdir):
    obj = obj_for(src, objdir)
    if os.path.exists(obj) and os.path.getmtime(obj) >= os.path.getmtime(src):
        return obj, None
    os.makedirs(os.path.dirname(obj), exist_ok=True)
    # -I <data dir>: the data units .incbin their byte files from there
    r = subprocess.run([AS] + AS_FLAGS + ["-I", os.path.dirname(src), "-o", obj, src],
                       capture_output=True, text=True)
    if r.returncode:
        return obj, r.stderr.strip().splitlines()[:2]
    return obj, None


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--code", default=os.path.join(HERE, "..", "code", "asm"))
    ap.add_argument("--data", default=os.path.join(HERE, "..", "data"))
    ap.add_argument("--obj", default=os.path.join(HERE, "..", "build", "obj"))
    ap.add_argument("-o", "--out", default=os.path.join(HERE, "..", "code", "build", "os_full.bin"))
    ap.add_argument("--reference", default=os.path.join(HERE, "..", "..", "dumps", "os_full.bin"))
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    ap.add_argument("--list", default="build/objects.txt")
    args = ap.parse_args(argv)

    srcs = sources([args.code, args.data])
    if not srcs:
        print("rebuild: no sources found (run gen_asm.py and gen_data.py first)")
        return 1
    print("rebuild: %d source unit(s)" % len(srcs))
    objs, errs = [], []
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for obj, err in pool.map(lambda s: assemble_one(s, args.obj), srcs):
            if err:
                errs.append((obj, err))
            else:
                objs.append(obj)
    if errs:
        print("rebuild: %d unit(s) failed to assemble" % len(errs))
        for obj, err in errs[:5]:
            print("  %s: %s" % (obj, err))
        return 1
    with open(args.list, "w") as f:
        f.write("\n".join(objs) + "\n")
    print("rebuild: assembling done, linking %d object(s)" % len(objs))
    os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)
    # the object list is passed in a response file: 9 400 paths exceed ARG_MAX
    rsp = os.path.join(os.path.dirname(os.path.abspath(args.list)), "objects.rsp")
    with open(rsp, "w") as f:
        f.write("\n".join(objs) + "\n")
    r = subprocess.run([sys.executable, os.path.join(HERE, "build_le.py"),
                        "-o", args.out, "--allow-holes", "@" + rsp])
    if r.returncode:
        return r.returncode
    return subprocess.run([sys.executable, os.path.join(HERE, "verify.py"),
                           args.out, args.reference]).returncode


if __name__ == "__main__":
    sys.exit(main())
