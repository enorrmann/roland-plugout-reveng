"""Per-file round-trip report: the first divergent word, not just the size.

    python3 tests/roundtrip_report.py [N]

Distinguishes the two failure kinds:
  length  the emitted words end early or late (a sizing error)
  value   a word differs in place (a slot or operand assignment error)
"""

import glob
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
ROOT = os.path.dirname(os.path.dirname(HERE))

import asm  # noqa: E402
import disasm  # noqa: E402
import isa  # noqa: E402


def main(limit=None):
    table = isa.Table.load()
    a = asm.Assembler(table, allow_inferred=True)
    rows = []
    for path in sorted(glob.glob(os.path.join(ROOT, "work", "corpus_all", "*.txt"))):
        words = [int(t, 16) for t in open(path).read().split()]
        text, _ = disasm.program(words, table, os.path.basename(path))
        r = a.assemble(text)
        first = next((i for i, (x, y) in enumerate(zip(words, r.words)) if x != y), None)
        if r.words == words:
            kind = "identical"
        elif first is None:
            kind = "length"
        elif first >= min(len(words), len(r.words)):
            kind = "length"
        else:
            kind = "value"
        rows.append((kind, first if first is not None else -1, os.path.basename(path),
                     len(words), len(r.words), r.rejected))

    for kind in ("identical", "value", "length"):
        group = [r for r in rows if r[0] == kind]
        print("%-10s %3d files" % (kind, len(group)))
    print()
    print("first divergence (value errors show where the misassignment starts):")
    shown = 0
    for kind, first, name, n, got, rej in rows:
        if kind == "identical":
            continue
        print("  %-8s word %-4d %-26s %d -> %d words, %d rejected"
              % (kind, first, name, n, got, rej))
        shown += 1
        if limit and shown >= limit:
            break


if __name__ == "__main__":
    main(int(sys.argv[1]) if len(sys.argv) > 1 else 10)
