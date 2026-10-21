#!/usr/bin/env python3
"""gen_data.py -- emit the non-code bytes of the image as C sources.

Everything the Thumb-2 sweep does not decode is data (``tools/segment.py``):
literal-pool words the sweep could not read as instructions, switch tables,
string blobs and the all-``0xFF`` padding.  Each run becomes a named C array in
``.rodata``/``.data`` plus the assembly that gives it its address:

    section "rodata.<addr>"   one section per run, named after the run's
                              address in the image

The address travels in the section name, not in ``. = ...``: ``as`` always lays
the sections of an object end to end, so an absolute address can only be
applied when the object is placed -- ``build_le.py`` does that by reading the
address out of the section name.  That also keeps every object small.

    python3 tools/gen_data.py --cache build/seg.pkl -o data
"""
from __future__ import annotations

import argparse
import os
import pickle
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

BASE = 0x60000000
CHUNK = 0x10000


def classify(img, start, end):
    """Label one data run: fill, zero, text, pointers or plain bytes."""
    seg = img[start:end]
    if all(b == 0xFF for b in seg):
        return "fill"
    if all(b == 0 for b in seg):
        return "zero"
    if sum(1 for b in seg if 0x20 <= b < 0x7F or b in (9, 10, 13, 0)) >= 0.9 * len(seg):
        return "text"
    if len(seg) % 4 == 0:
        words = struct.unpack("<%dI" % (len(seg) // 4), seg)
        if all(BASE <= w < BASE + len(img) for w in words):
            return "pointers"
    return "bytes"


def c_array(name, seg, kind):
    """A C definition of one run (the bytes are also in ``<name>.bin``)."""
    head = "/* %d bytes, %s */" % (len(seg), kind)
    if kind == "pointers":
        vals = struct.unpack("<%dI" % (len(seg) // 4), seg)
        body = ",\n".join("    " + ", ".join("0x%08x" % v for v in vals[i:i + 4])
                          for i in range(0, len(vals), 4))
        return "%s\nconst unsigned int __attribute__((section(\".rodata\"))) %s[%d] = {\n%s\n};\n" \
            % (head, name, len(vals), body)
    body = ",\n".join("    " + ", ".join("0x%02x" % b for b in seg[i:i + 16])
                      for i in range(0, len(seg), 16))
    const = "const " if kind != "zero" else ""
    return "%s\n%sunsigned char __attribute__((section(\".rodata\"))) %s[%d] = {\n%s\n};\n" \
        % (head, const, name, len(seg), body)


def asm_run(name, addr, size, kind):
    lines = ['\t.section .data.%08x,"aw",%%progbits' % addr,
             "\t.globl %s" % name, "%s:" % name]
    if kind == "fill":
        lines.append("\t.fill %d, 1, 0xFF" % size)
    elif kind == "zero":
        lines.append("\t.space %d, 0" % size)
    else:
        lines.append("\t.incbin \"%s.bin\"" % name)
    return "\n".join(lines)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--cache", default="build/seg.pkl")
    ap.add_argument("--image", default=os.path.join("..", "dumps", "os_full.bin"))
    ap.add_argument("-o", "--out", default="data")
    ap.add_argument("--chunk", type=lambda s: int(s, 0), default=CHUNK)
    args = ap.parse_args(argv)

    with open(args.cache, "rb") as f:
        seg = pickle.load(f)
    with open(args.image, "rb") as f:
        img = f.read()
    os.makedirs(args.out, exist_ok=True)
    encoded = os.path.abspath(args.out)

    runs = seg["data_runs"]
    units = 0
    for off in range(0, len(img), args.chunk):
        end = min(off + args.chunk, len(img))
        sub = [(a, b) for a, b in runs if off <= a and b <= end]
        if not sub:
            continue
        unit = "data_%02x" % (off // args.chunk)
        cs = ["/* data of os_full.bin, image offsets 0x%06x..0x%06x */" % (off, end),
              "#include <stdint.h>", ""]
        ss = ["\t.syntax unified", "\t.thumb", ""]
        for a, b in sub:
            kind = classify(img, a, b)
            name = "d_%08x" % a
            cs.append(c_array(name, img[a:b], kind))
            ss.append(asm_run(name, BASE + a, b - a, kind))
            with open(os.path.join(args.out, name + ".bin"), "wb") as f:
                f.write(img[a:b])
        with open(os.path.join(args.out, unit + ".c"), "w") as f:
            f.write("\n".join(cs))
        with open(os.path.join(args.out, unit + ".s"), "w") as f:
            f.write("\n".join(ss) + "\n")
        units += 1
    print("gen_data: %d unit(s), %d run(s), %d bytes"
          % (units, len(runs), len(img) - sum(seg["mask"])))
    print("gen_data: include %s in the assembler search path (-I)" % encoded)
    return 0


if __name__ == "__main__":
    sys.exit(main())
