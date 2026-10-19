"""The worked examples of DSP_ISA_SPEC.md, decoded by the table.

These are independent of our own encoder: the words are copied from the spec
(§11, §6, §7, §8, §9.0), so they catch a table that is self-consistently wrong.

    python3 -m unittest tests/test_spec_examples.py
"""

import os
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))

import disasm  # noqa: E402
import isa  # noqa: E402

TABLE = isa.Table.load()

# (words, expected substrings, where, meaning)
CASES = [
    ([0x0902, 0x3C01], ["addxy r3, r0, r1"], "11.1", "r3 <- r0 + r1"),
    ([0x0A02, 0x3C42], ["absadd r3, r1, r2"], "6", "r3 <- |r1| + r2"),
    ([0x0903, 0x1050, 0x2800], ["mov r1, #0.5"], "11.3", "r1 <- 0.5 (imm16)"),
    ([0x0904, 0x1430, 0x2800, 0x0000], ["mul r1, r0, #0.5"], "11.2", "r1 <- r0 * 0.5 (imm32)"),
    ([0x1E05, 0x0FE0, 0x9FE2, 0x0400, 0x0401], ["ld r0, [0400]", "st r1, [0401]"],
     "11.4", "r0 <- [0x0400]; [0x0401] <- r1"),
    ([0x0803, 0x0C0A, 0x9281], ["mac_p1p2 r0, r0, -r2, -r1, r2, r1"], "11.5",
     "r0 <- r1 + r2*(r0 - r1): Z = -r2, C = r1, W = r2"),
    ([0x2C06, 0x1C47, 0x36B0, 0xA445, 0x2800, 0x0000],
     ["addxy r1, r1, r7", "mad r3, r2, #0.5", "max r2, r1, r5"], "11.6b",
     "A/A/B: r1 += r7 | r3 += r2*0.5 | r2 = max(r1, r5)"),
    ([0x0902, 0x4086], ["newton"], "6.2", "Newton step of the reciprocal"),
    ([0x0B03, 0x0090, 0x0003], ["jmp .+3"], "5.4b", "jump over 3 words"),
    ([0x0B03, 0x4050, 0x0301], ["hset h5"], "9.0", "h5 <- 0x301/16384"),
    ([0x1C04, 0x0090, 0x1A10, 0x0003], ["jmp", "cond"], "5.4b",
     "jump +3 unless cond13"),
    # spec 4.3: the slot array rule, worked out in the text
    ([0x1805, 0x5550, 0x6FE2, 0x7000, 0x0219],
     ["mul r5, r5, #256", "ld r6, [0219]"], "4.3",
     "A(5550, imm16 from slot 0) T(6fe2: slot 1) | slot 0 = 7000 (= 256), slot 1 = 0219"),
    ([0x2B09, 0x2C8F, 0x6FE2, 0x7FE4, 0x5983, 0x0407, 0x0403, 0xA2AA, 0xAA00],
     ["ld r6, [0407]", "ld r7, [0403]"], "4.3",
     "two references behind an immediate stream"),
    # spec 4.4: the elided "540c …" example, found at work/corpus_all/p0s_003743c.txt.
    # Y at byte 2 = 3000; C right after it, skipping the two references = 233333
    # (0.2); W at byte 11 = c4c008 (-4.75). Those three values only come out if
    # the stream is the WHOLE slot array, references included.
    ([0x540C, 0x1C12, 0xAF1B, 0xAFE4, 0xCFE6, 0x7A2B, 0x3000, 0x0638, 0x063D, 0x2333, 0x33C4, 0xC008],
     ["0.2", "-4.75"], "4.4",
     "C = 0.2 after skipping the two references, W = -4.75 at byte 11"),
]

# Spec 6.4, line "{8+d}f7Y": the section markers ARE decoded — as f7, a form with
# no visible effect. They must be reported as such, never as something with
# semantics, and their "no effect" must be visible in the result text.
MARKER_CASES = [
    ([0x0A02, 0xFF70], "3", "slot program start/end marker (ff70)"),
    ([0x0A02, 0xEF75], "6.4", "per-voice section open (ef75)"),
    ([0x0A02, 0xEF74], "6.4", "per-voice section close (ef74)"),
]

# Words the table has no mnemonic for must stay verbatim.
RAW_CASES = [
    # a B word with D < 8, an op nibble the table has no entry for
    ([0x0A03, 0x7B10, 0x0000], "6.3", "unit-B word with an unknown op nibble"),
]


def first_line(text):
    """the instruction lines of a listing (a `.class` / `.free` directive may
    precede one)"""
    return [l for l in text.splitlines()
            if l and not l.startswith(";") and not l.lstrip().startswith(".")]


class SpecExamples(unittest.TestCase):
    def test_worked_examples(self):
        for words, needles, where, what in CASES:
            with self.subTest(spec=where, what=what):
                text, _ = disasm.program(words, TABLE, "spec %s" % where)
                line = first_line(text)[0].split(";")[0].strip()
                for needle in needles:
                    self.assertIn(needle, line,
                                  "%s (%s): got %r" % (what, where, line))

    def test_markers_decode_as_f7_without_effect(self):
        for words, where, what in MARKER_CASES:
            with self.subTest(spec=where, what=what):
                text, report = disasm.program(words, TABLE, where)
                line = first_line(text)[0].split(";")[0].strip()
                self.assertTrue(line.startswith("f7 "), "%s: got %r" % (what, line))
                variant = TABLE.mnemonics["f7"]
                self.assertIn("no effect", variant.result)
                self.assertEqual(report["decoded"], 1)

    def test_unknown_stays_verbatim(self):
        for words, where, what in RAW_CASES:
            with self.subTest(spec=where, what=what):
                text, report = disasm.program(words, TABLE, where)
                line = first_line(text)[0].split(";")[0].strip()
                self.assertTrue(line.startswith("raw "), "%s: got %r" % (what, line))
                self.assertEqual(report["raw"], 1)

    def test_raw_keeps_every_word(self):
        words = [0x0A03, 0x7B10, 0x0000]
        text, _ = disasm.program(words, TABLE)
        line = first_line(text)[0].split(";")[0].strip()
        self.assertTrue(line.startswith("raw "), line)
        for w in words[1:]:
            self.assertIn("%04x" % w, line)

    def test_status_tags_are_reported(self):
        text, report = disasm.program([0x0902, 0x3C01], TABLE)
        self.assertEqual(report["statuses"], {"exact": 1})
        self.assertIn("(exact)", text)

    def test_number_format(self):
        for value, word in ((0.5, 0x28000000), (1.0, 0x30000000), (-1.0, 0xB0000000),
                            (2.0, 0x42000000), (100.0, 0x66400000), (0.0, 0)):
            with self.subTest(value=value):
                self.assertEqual(TABLE.raw_encode(value), word)
                self.assertEqual(TABLE.raw_decode(word), value)
        self.assertAlmostEqual(TABLE.raw_decode(TABLE.raw_encode(1 / 192)), 1 / 192, places=9)

    def test_immediate_stream_is_the_whole_slot_array(self):
        """spec 4.4's `540c` example: the byte offsets index the slot array with
        its references in it, and only *sequential* reading skips them.

        If the stream were built from the I words alone (the old rule), Y at
        byte 2 would read 0 instead of `3000` and C would read 0 instead of
        `233333` — the test would pass only by accident, so check the values.
        """
        words = [0x540C, 0x1C12, 0xAF1B, 0xAFE4, 0xCFE6, 0x7A2B,
                 0x3000, 0x0638, 0x063D, 0x2333, 0x33C4, 0xC008]
        text, _ = disasm.program(words, TABLE, "spec 4.4")
        line = first_line(text)[0].split(";")[0].strip()
        # Y = 3000 = 0.1875 (byte 2), C = 233333 = 0.2 (after skipping bytes 4-7),
        # W = c4c008 = -4.75 (byte 11)
        self.assertIn("0.1875", line, line)     # Y, at byte 2
        self.assertIn("0.2", line, line)        # C, after the two references
        self.assertIn("-4.75", line, line)      # W, at byte 11

    def test_sequential_read_skips_reference_words(self):
        """an explicit byte offset does not skip; a sequential read does"""
        stream = disasm.ImmStream([0x3000, 0x0638, 0x063D, 0x2333], refs={1, 2})
        self.assertEqual(stream.take(2 * 8, 16), 0x30000000)      # byte 2, no skip
        stream.pos = 4 * 8                                        # read on at byte 4
        self.assertEqual(stream.take(None, 24), 0x23333300)       # skips 1 and 2


if __name__ == "__main__":
    unittest.main()
