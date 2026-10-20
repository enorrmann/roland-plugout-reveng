"""Disassemble every program of the corpus and report what the table covers.

    python3 tests/test_corpus.py            # summary
    python3 tests/test_corpus.py -v         # per directory

The corpus (`work/corpus*`) is written by re/dsp_corpus*.py as flat 16-bit
words in execution order, so a program is a word list with no framing. This
test does not fail on `raw` output: an undecoded encoding is a finding, not
an error, and the number that matters (the share that decodes) is printed.

    python3 tests/test_corpus.py
"""

import glob
import os
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
ROOT = os.path.dirname(os.path.dirname(HERE))

import disasm  # noqa: E402
import isa  # noqa: E402

TABLE = isa.Table.load()
CORPUS = os.path.join(ROOT, "work")


def programs(pattern="corpus*"):
    """every corpus .txt file as a flat word list"""
    out = []
    for path in sorted(glob.glob(os.path.join(CORPUS, pattern, "*.txt"))):
        try:
            words = [int(tok, 16) for tok in open(path).read().split()]
        except ValueError:
            continue
        if words:
            out.append((path, words))
    return out


def disassemble_all(pattern="corpus*"):
    total = {"files": 0, "instructions": 0, "decoded": 0, "raw": 0, "statuses": {}}
    for path, words in programs(pattern):
        _, rep = disasm.program(words, TABLE, os.path.basename(path))
        total["files"] += 1
        total["instructions"] += rep["instructions"]
        total["decoded"] += rep["decoded"]
        total["raw"] += rep["raw"]
        for k, v in rep["statuses"].items():
            total["statuses"][k] = total["statuses"].get(k, 0) + v
    return total


def main(argv):
    verbose = "-v" in argv
    for pattern in ("corpus", "corpus_all", "corpus_s1"):
        if not os.path.isdir(os.path.join(CORPUS, pattern)):
            continue
        t = disassemble_all(pattern)
        if not t["files"]:
            continue
        share = 100.0 * t["decoded"] / t["instructions"] if t["instructions"] else 0
        print("%-12s %4d files %6d instructions  decoded %6d (%.1f%%)  raw %5d"
              % (pattern, t["files"], t["instructions"], t["decoded"], share, t["raw"]))
        print("%-12s status: %s" % ("", ", ".join("%s %d" % kv for kv in sorted(t["statuses"].items()))))
    return 0


class Corpus(unittest.TestCase):
    """the table must decode most of the corpus and never crash on it"""

    def test_no_crash(self):
        for path, words in programs("corpus*"):
            with self.subTest(file=os.path.basename(path)):
                disasm.program(words, TABLE, path)

    def test_round_trip_is_exact(self):
        """listing -> assembler -> the same words, for every program (review
        2026-10-20: 132 files; before the review 0 of 128 corpus files)"""
        import asm
        assembler = asm.Assembler(TABLE, allow_inferred=True)
        files = programs("corpus_all") + programs("corpus_s1")
        if not files:
            self.skipTest("no corpus")
        for path, words in files:
            with self.subTest(file=os.path.basename(path)):
                text, _ = disasm.program(words, TABLE, path)
                r = assembler.assemble(text)
                self.assertEqual(r.errors, [])
                self.assertEqual(r.words, words)

    def test_every_decoded_line_reassembles(self):
        """the listing never claims a line it cannot take back (disasm.program's
        own check): no line falls back to raw for that reason"""
        for path, words in programs("corpus_all") + programs("corpus_s1"):
            with self.subTest(file=os.path.basename(path)):
                _, rep = disasm.program(words, TABLE, path)
                self.assertEqual(rep.get("reassembly", 0), 0)

    def test_decoding_share(self):
        t = disassemble_all("corpus")
        if not t["instructions"]:
            self.skipTest("no corpus")
        share = t["decoded"] / t["instructions"]
        self.assertGreater(share, 0.95,
                           "only %.1f%% of the corpus decodes (%d of %d instructions)"
                           % (100 * share, t["decoded"], t["instructions"]))


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
