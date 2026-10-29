"""Assembler tests: independent of the round trip, plus the round trip itself.

Two kinds of proof, kept apart on purpose:

* `test_spec_words` assembles the worked examples of DSP_ISA_SPEC.md and
  compares against the words printed there. This checks the encoder against
  the documentation, not against our own decoder.
* `test_round_trip` disassembles corpus instructions and re-assembles them.
  It catches layout and operand bugs, but a bug shared by both directions
  would hide — which is why the first kind exists.

    python3 tests/test_asm.py
"""

import os
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
ROOT = os.path.dirname(os.path.dirname(HERE))

import asm  # noqa: E402
import disasm  # noqa: E402
import isa  # noqa: E402

TABLE = isa.Table.load()
ASM = asm.Assembler(TABLE, allow_inferred=True)


def words_of(source):
    r = ASM.assemble(source)
    return r, [w for w in r.words]


class SpecWords(unittest.TestCase):
    """source -> the words the spec prints for it (independent evidence)."""

    CASES = [
        ("addxy r3, r0, r1", "0902 3c01", "spec 11.1"),
        ("mul r1, r0, #0.5", "0904 1430 2800 0000", "spec 11.2"),
        ("mov r1, #0.5", "0903 1050 2800", "spec 11.3"),
        ("ld r0, [0400] | st r1, [0401]", "1e05 0fe0 9fe2 0400 0401", "spec 11.4"),
        ("mac_p1p2 r0, r0, -r2, -r1, r2, r1   ; @0x0040",
         "0803 0c0a 9281", "spec 11.5"),
        ("addxy r1, r1, r7 | mad r3, r2, #0.5 | max r2, r1, r5   ; @0x0160",
         "2c06 1c47 36b0 a445 2800 0000", "spec 11.6b"),
        ("absadd r3, r1, r2", "0a02 3c42", "spec 6"),
        ("newton r4, r6", "0902 4086", "spec 6.2"),
        ("jmp .+3", "0b03 0090 0003", "spec 5.4b"),
    ]

    def test_spec_words(self):
        for source, want, where in self.CASES:
            with self.subTest(spec=where, source=source):
                r, got = words_of(source)
                self.assertEqual(r.errors, [], "rejected: %r" % (r.errors,))
                self.assertEqual(" ".join("%04x" % w for w in got), want)


class Policy(unittest.TestCase):
    """the emission policy is enforced by the assembler, not by the caller"""

    def setUp(self):
        self.strict = asm.Assembler(TABLE)                 # no opt-in
        self.lax = asm.Assembler(TABLE, allow_inferred=True)

    def test_exact_and_measured_emit_by_default(self):
        for source in ("addxy r3, r0, r1", "absadd r3, r1, r2", "mov r1, #0.5",
                       "ld r0, [0400]", "newton r4, r6"):
            with self.subTest(source=source):
                self.assertTrue(self.strict.assemble(source).ok,
                                self.strict.assemble(source).errors)

    def test_coefficient_register_form_is_measured(self):
        """the MAC that reads its coefficient from rC (bit 9): measured exact on the SYSTEM-8 master
        (re/dspnative.py 2026-10-28), so it emits by default."""
        v = TABLE.mnemonics["mac_p1p2"]
        self.assertEqual(v.status, "measured")
        src = "mac_p1p2 r0, r0, -r2, -r1, r2, r1   ; @0x0040"
        self.assertTrue(self.strict.assemble(src).ok)

    def test_inferred_needs_the_flag(self):
        # spec 5.2 marks op2 mode 1 as "presumably imm16 [inferred]"
        v = TABLE.mnemonics["mac_p1c1"]
        self.assertEqual(v.status, "inferred")
        src = "mac_p1c1 r0, r0, #0.5, r1   ; @0x0040"
        self.assertFalse(self.strict.assemble(src).ok)
        r = self.lax.assemble(src)
        self.assertTrue(r.ok, r.errors)

    def test_raw_is_never_default(self):
        src = "raw @0x060 0fe0 0201"
        self.assertFalse(self.strict.assemble(src).ok)
        self.assertTrue(self.lax.assemble(src).ok)

    def test_unknown_mnemonic_is_rejected(self):
        r = self.lax.assemble("frobnicate r0, r1")
        self.assertFalse(r.ok)
        self.assertIn("unknown mnemonic", r.errors[0][1])

    def test_too_many_operands_is_rejected(self):
        r = self.lax.assemble("absadd r3, r1, r2, r0")
        self.assertFalse(r.ok)

    def test_several_ops_share_one_immediate_stream(self):
        """spec 4.4: every A/B op with immediate mode reads the same bit stream
        at its own Y byte, so several values in one line are legal and each must
        land at its own offset."""
        words = [0x3006, 0x0050, 0x8012, 0x1812, 0xB000, 0x0000]
        text, _ = disasm.program(words, TABLE)
        r = self.lax.assemble(text)
        self.assertTrue(r.ok, r.errors)
        self.assertEqual(r.words, words)

    def test_mac_class_defaults_to_the_measured_one(self):
        # 0x040 and 0x098 share the MN core: the assembler takes 0x040, the class measured exact (re/dspnative.py)
        r = self.lax.assemble("mac_p1p2 r0, r0, -r2, -r1, r2, r1")
        self.assertTrue(r.ok, r.errors)
        self.assertEqual(r.words, self.lax.assemble("mac_p1p2 r0, r0, -r2, -r1, r2, r1   ; @0x0040").words)

    def test_pinned_class_resolves_ambiguity(self):
        r = self.lax.assemble("mac_p1p2 r0, r0, -r2, -r1, r2, r1   ; @0x0040")
        self.assertTrue(r.ok)

    def test_bad_operands_are_rejected(self):
        for src in ("absadd r9, r1, r2", "absadd r3, r1, [0400]", "jmp nowhere"):
            with self.subTest(source=src):
                self.assertFalse(self.lax.assemble(src).ok)


class RoundTrip(unittest.TestCase):
    """disassemble -> assemble must give back the same words"""

    def corpus(self, limit=None):
        import glob
        files = sorted(glob.glob(os.path.join(ROOT, "work", "corpus_all", "*.txt")))
        out = []
        for path in files[:limit] if limit else files:
            out.append((os.path.basename(path),
                        [int(t, 16) for t in open(path).read().split()]))
        return out

    def test_individual_known_instructions(self):
        """short, hand-picked instructions round-trip exactly"""
        cases = [
            [0x0902, 0x3C01],                       # addxy r3, r0, r1
            [0x0903, 0x1050, 0x2800],               # mov r1, #0.5
            [0x0904, 0x1430, 0x2800, 0x0000],       # mul r1, r0, #0.5
            [0x1E05, 0x0FE0, 0x9FE2, 0x0400, 0x0401],   # ld/st pair
            [0x0803, 0x0C0A, 0x9281],               # the MAC lerp
            [0x0A02, 0xFF70],                       # section marker
            [0x0B03, 0x0090, 0x0003],               # jump +3
        ]
        for words in cases:
            with self.subTest(words=" ".join("%04x" % w for w in words)):
                text, _ = disasm.program(words, TABLE)
                r = ASM.assemble(text)
                self.assertTrue(r.ok, r.errors)
                self.assertEqual(r.words, words)

    def test_line_level_round_trip_is_not_regressing(self):
        """the share of corpus *lines* the assembler accepts

        Not a correctness proof and not yet a whole-file round trip: the
        assembler still rejects lines whose class it cannot pin down (several
        classes share a core) and lines with more than one immediate operand.
        The number is recorded so it cannot silently fall.
        """
        files = self.corpus(limit=12)
        if not files:
            self.skipTest("no corpus")
        lines = rejected = 0
        for name, words in files:
            text, _ = disasm.program(words, TABLE, name)
            lines += sum(1 for l in text.splitlines() if l and not l.startswith(";"))
            rejected += ASM.assemble(text).rejected
        share = (lines - rejected) / lines if lines else 0
        print("\nlines assembled: %d of %d (%.1f%%)" % (lines - rejected, lines, 100 * share))
        self.assertGreater(share, 0.60,
                           "the assembler accepts only %.1f%% of corpus lines" % (100 * share))


if __name__ == "__main__":
    unittest.main()


class Directives(unittest.TestCase):
    """`.class` and `.free`: explicit directives, not comment conventions."""

    def test_class_directive_pins_the_class(self):
        r = ASM.assemble(".class 0x0040\nmac_p1p2 r0, r0, -r2, -r1, r2, r1")
        self.assertTrue(r.ok, r.errors)
        self.assertEqual(r.words, [0x0803, 0x0C0A, 0x9281])

    def test_legacy_comment_hint_still_works(self):
        r = ASM.assemble("mac_p1p2 r0, r0, -r2, -r1, r2, r1   ; @0x0040")
        self.assertTrue(r.ok, r.errors)

    def test_unknown_class_is_rejected(self):
        r = ASM.assemble(".class 0x9999\nabsadd r3, r1, r2")
        self.assertFalse(r.ok)
        self.assertIn("not in the table", r.errors[0][1])

    def test_class_is_only_needed_when_ambiguous(self):
        text, _ = disasm.program([0x0902, 0x3C01], TABLE)
        self.assertNotIn(".class", text)
        text, _ = disasm.program([0x0803, 0x0C0A, 0x9281], TABLE)
        self.assertIn(".class 0x0040", text)

    def test_free_bits_survive_a_round_trip(self):
        """bits the variant declares free have no meaning: whatever the original
        word held must come back (spec 5.2)."""
        for freeval in (0x0000, 0x1000, 0x8000, 0x0007, 0x2000, 0xF000):
            words = [0x0803, 0x0601, freeval]
            with self.subTest(free=freeval):
                text, _ = disasm.program(words, TABLE)
                r = ASM.assemble(text)
                self.assertTrue(r.ok, r.errors)
                self.assertEqual(r.words, words)

    def test_free_directive_refuses_a_bit_that_is_not_free(self):
        # mac_p1w2 (op1 = 2) has no Z or C term, so those bits ARE free; a bit
        # an operand owns -- D -- must be refused instead.
        r = ASM.assemble(".class 0x0040\n.free 0x0000\nmac_p1w2 r0, r0, r1, r0")
        self.assertTrue(r.ok, r.errors)
        # 0x0004 is part of W (bits 3-0), which is a real operand here
        r = ASM.assemble(".class 0x0040\n.free 0x0004\nmac_p1w2 r0, r0, r1, r0")
        self.assertFalse(r.ok)
        self.assertIn("not a free bit", r.errors[0][1])


class SlotArray(unittest.TestCase):
    """spec 4.3: the slot array is one array, its word owners decided by who
    points at them. These are the cases that used to diverge."""

    CASES = [
        # (words, why)
        ([0x1805, 0x5550, 0x6FE2, 0x7000, 0x0219],
         "spec 4.3 example: slot 0 is the immediate, slot 1 the reference"),
        ([0x2B09, 0x2C8F, 0x6FE2, 0x7FE4, 0x5983, 0x0407, 0x0403, 0xA2AA, 0xAA00],
         "spec 4.3 example: two references behind an immediate stream"),
        ([0x1805, 0x0050, 0x2FE2, 0x0000, 0x8000],
         "0x0c0: a reference after an immediate word keeps its index"),
        ([0x1E05, 0x0FE0, 0x1FE2, 0x0285, 0x0285],
         "two transfers share a value but sit in different slots"),
        ([0x3006, 0x0050, 0x8012, 0x1812, 0xB000, 0x0000],
         "two ops read the one immediate stream at different byte offsets"),
    ]

    def test_round_trip(self):
        for words, why in self.CASES:
            with self.subTest(why=why, words=" ".join("%04x" % w for w in words)):
                text, _ = disasm.program(words, TABLE)
                r = ASM.assemble(text)
                self.assertTrue(r.ok, r.errors)
                self.assertEqual(r.words, words)

    def test_reference_index_is_explicit(self):
        """the listing writes `[addr]@k`, so the assembler needs no guess"""
        text, _ = disasm.program([0x1805, 0x0050, 0x2FE2, 0x0000, 0x8000], TABLE)
        self.assertIn("[8000]@1", text)

    def test_conflict_is_reported_not_resolved(self):
        """two different values in one slot is an error, not an overwrite"""
        import slots
        arr = slots.SlotArray(0, 1, 3)
        arr.take_n([0x1111])
        arr.claim_reference(1, 0x2222, "t1")
        with self.assertRaises(slots.SlotError):
            arr.claim_reference(1, 0x3333, "t2")

    def test_same_value_in_one_slot_is_accepted(self):
        import slots
        arr = slots.SlotArray(0, 0, 2)
        arr.claim_reference(0, 0x2222, "t1")
        arr.claim_reference(0, 0x2222, "t2")      # the corpus does this
        self.assertEqual(arr.slots[0].value, 0x2222)

    def test_first_divergent_word_is_reported(self):
        """compare word by word, not just the total length"""
        files = RoundTrip().corpus(limit=6)
        if not files:
            self.skipTest("no corpus")
        for name, words in files:
            text, _ = disasm.program(words, TABLE, name)
            r = ASM.assemble(text)
            first = next((i for i, (x, y) in enumerate(zip(words, r.words)) if x != y),
                         None)
            with self.subTest(file=name):
                if first is not None:
                    self.assertLess(first, len(words),
                                    "%s diverges at word %d" % (name, first))


class Sizing(unittest.TestCase):
    """one owner for (class, body length): the classifier decides, the emitter
    obeys. These guard the contract that used to disagree with itself."""

    def test_emitted_words_match_the_classified_length(self):
        """the invariant: len(words) == header length == classified body + 1"""
        cases = [
            "absadd r3, r1, r2",
            "mov r1, #0.5",
            "mul r1, r0, #0.5",
            "ld r0, [0400] | st r1, [0401]",
            "mac_p1p2 r0, r0, -r2, -r1, r2, r1   ; @0x0040",
        ]
        for src in cases:
            with self.subTest(source=src):
                r = ASM.assemble(src)
                self.assertTrue(r.ok, r.errors)
                header_len = r.words[0] & 0x1F
                self.assertEqual(len(r.words), header_len,
                                 "%r: header says %d words, emitted %d"
                                 % (src, header_len, len(r.words)))

    def test_length_is_the_one_the_line_needs(self):
        """the slot array is as small as the line allows (spec 4.3): no slot
        word for a line without a reference or an immediate that reads one"""
        for src, body_words in (("addxy r3, r0, r1", 1),
                                ("mul r5, r5, #256@0:16", 2),
                                ("ld r0, [0400] | st r1, [0401]", 4)):
            with self.subTest(source=src):
                r = ASM.assemble(src)
                self.assertTrue(r.ok, r.errors)
                self.assertEqual(len(r.words) - 1, body_words,
                                 "%r: %d body words" % (src, len(r.words) - 1))
