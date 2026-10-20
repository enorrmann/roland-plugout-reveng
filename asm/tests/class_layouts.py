"""Compare the class layouts of isa.json with the spec's section 12 table.

    python3 tests/class_layouts.py        # one line per class that differs

For each class where the two layouts differ, both are tried on the corpus
instances of that class and the number that re-assemble exactly is printed.
That is the evidence used to choose between them (2026-10-20 review). It is a
self-consistency measure: a layout that reproduces the words is not thereby
proved to give the right meaning to each letter.
"""

import glob
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
ROOT = os.path.dirname(os.path.dirname(HERE))

import asm  # noqa: E402
import disasm  # noqa: E402
import isa  # noqa: E402
import layout  # noqa: E402

SPEC = os.path.join(ROOT, "examples", "from_scratch", "DSP_ISA_SPEC.md")
PER_CLASS = 150          # instances tried per class


def spec_layouts():
    """class -> layout string of the spec's section 12 table"""
    out = {}
    for line in open(SPEC, encoding="utf-8"):
        m = re.match(r"^\| `(0x[0-9a-f]{3})` \| `[0-9a-f]{4}` \| `([A-Z]+)` \(\d+\)", line)
        if m:
            out[int(m.group(1), 16)] = m.group(2)
    return out


def instances(table_dirs=("corpus_all", "corpus_s1")):
    """class -> up to PER_CLASS instances (lists of words)"""
    out = {}
    for d in table_dirs:
        for path in sorted(glob.glob(os.path.join(ROOT, "work", d, "*.txt"))):
            words = [int(t, 16) for t in open(path).read().split()]
            i = 0
            while i < len(words) and words[i] & 0x1F:
                n = words[i] & 0x1F
                cls = words[i] >> 5
                if len(out.setdefault(cls, [])) < PER_CLASS:
                    out[cls].append(words[i:i + n])
                i += n
    return out


def reassembling(table, cls, insts):
    """how many instances of `cls` the listing reproduces"""
    assembler = asm.Assembler(table, allow_inferred=True)
    ok = 0
    for ins in insts:
        d = disasm.disassemble_insn(table, ins)
        if d.status == "raw":
            continue
        r = assembler.assemble(".class %#06x\n%s" % (cls, "\n".join(
            ".free @%d %#06x" % (p + 1, m) for p, m in d.free) + "\n" + d.text))
        if not r.errors and r.words == ins:
            ok += 1
    return ok


def main():
    table = isa.Table.load()
    spec = spec_layouts()
    insts = instances()
    for cls in sorted(spec):
        if cls not in table.classes or cls not in insts:
            continue
        mine = table.classes[cls].layout
        theirs = spec[cls].rstrip("I")
        if layout.prefix_length(mine) == layout.prefix_length(theirs) and \
                mine[:layout.prefix_length(mine)] == theirs[:layout.prefix_length(theirs)]:
            continue
        a = reassembling(table, cls, insts[cls])
        saved = table.classes[cls].layout
        table.classes[cls].layout = theirs
        b = reassembling(table, cls, insts[cls])
        table.classes[cls].layout = saved
        print("%#05x  n=%3d  isa %-14s re-assembles %3d  | spec %-14s re-assembles %3d"
              % (cls, len(insts[cls]), mine, a, theirs, b))
    return 0


if __name__ == "__main__":
    sys.exit(main())
