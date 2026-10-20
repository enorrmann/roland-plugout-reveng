"""One test per defect the 2026-10-20 review found and fixed.

Each case is a word sequence taken from the corpus (or the spec) that the old
code got wrong. The check is the round trip -- disassemble, assemble, get the
same words back -- plus the text where the text itself was the defect.

    cd asm && python3 -m unittest discover -s tests

The findings are written up in asm/REVIEW.md.
"""

import os
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))

import asm  # noqa: E402
import disasm  # noqa: E402
import isa  # noqa: E402

TABLE = isa.Table.load()
ASM = asm.Assembler(TABLE, allow_inferred=True)


def listing(words):
    text, report = disasm.program(words, TABLE, "regression")
    return text, report


def body_lines(text):
    return [l for l in text.splitlines() if l and not l.startswith((";", "."))]


def round_trips(test, words):
    text, report = listing(words)
    r = ASM.assemble(text)
    test.assertEqual(r.errors, [], text)
    test.assertEqual(r.words, words, text)
    return text, report


class RoundTripCases(unittest.TestCase):
    """the defects, each one a line that must come back identical"""

    def test_missing_isa_import_in_disassembler(self):
        # the unit-B nibble path reached isa.TableError without the import
        round_trips(self, [0x0A03, 0xD550, 0x0000])            # max r5, r5, #0@0:16

    def test_unit_b_keeps_bit_two_of_x_in_the_nibble(self):
        # X[2] lives in the nibble of a unit-B word (spec 5.1); the X field must
        # not clear it (codec.encode wrote the field over it)
        text, _ = round_trips(self, [0x0A03, 0xD550, 0x0000])
        self.assertIn("max r5, r5, #0@0:16", text)

    def test_setflags_keeps_its_condition_group(self):
        # X9bY: the group is X - 8 (spec 5.3). The old listing printed r(X & 7),
        # which lost the 8 and gave 'setflags r1' for both 9 and 1
        text, _ = round_trips(self, [0x1803, 0x2C0A, 0x99B1])
        self.assertIn("setflags 1, 1", text)

    def test_hset_uses_the_register_it_names(self):
        # the assembler chose the first hset variant (h4) for every hset
        text, _ = round_trips(self, [0x0B03, 0x4050, 0x0101])
        self.assertIn("hset h5", text)

    def test_hidden_register_value_is_integer_over_16384(self):
        # spec 9.0: h4 = 0x410000 / 16384 = 260, read at byte 2
        text, _ = round_trips(self, [0x1D06, 0x4022, 0x7FE0, 0xC17C, 0x4100, 0x0000])
        self.assertIn("hset h4, #260@2:24", text)

    def test_mac_coefficient_immediate_spec_4_4(self):
        # the spec's own worked example: Y at byte 2, the coefficient right after
        # Y's immediate (over the two references), W at byte 11
        words = [0x540C, 0x1C12, 0xAF1B, 0xAFE4, 0xCFE6, 0x7A2B, 0x3000, 0x0638,
                 0x063D, 0x2333, 0x33C4, 0xC008]
        text, _ = round_trips(self, words)
        self.assertIn("#1@2:16, #0.2, r7", text)

    def test_mac_coefficient_with_register_y_is_read_at_byte_c(self):
        # spec 4.4 "or at byte c when Y is a register": the coefficient is read
        # at the byte the C field names (6), not after Y
        words = [0x2B09, 0x2C8F, 0x6FE2, 0x7FE4, 0x5983, 0x0407, 0x0403, 0xA2AA, 0xAA00]
        text, report = round_trips(self, words)
        self.assertIn("#-0.16666603@6", text)
        self.assertEqual(report["raw"], 0)

    def test_two_macs_need_two_n_words(self):
        # class 0x45a holds MMBTTTNNIII...: the table had one N
        round_trips(self, [0x8B4F, 0x3454, 0x2518, 0xEEE6, 0xDFEA, 0x1FEC, 0x5FEE,
                           0x0016, 0x0002, 0x800D, 0x0222, 0x3000, 0x040D, 0x0442, 0x043A])

    def test_class_0x328_layout_is_absb_tt_rr(self):
        # the table had AABTT: the B word of the first two was an A, a transfer
        # word read as B had no mnemonic
        self.assertEqual(TABLE.classes[0x328].layout[:5], "ABTTR")
        round_trips(self, [0x6507, 0x28C4, 0x8C30, 0xCFE0, 0x1FE2, 0x0229, 0xC027])

    def test_class_0xe0_is_a_jump_with_its_slot_word(self):
        # spec 5.4b: `1c04 0090 1a10 0003` = jump (slot 0 = 0003) unless cond13
        text, _ = round_trips(self, [0x1C04, 0x0090, 0x1A10, 0x0003])
        self.assertIn("jmp .+3@0", text)

    def test_class_0x4aa_is_known(self):
        # 12 corpus instances of 0x4aa had no class at all
        self.assertIn(0x4AA, TABLE.classes)

    def test_register_coefficient_with_bit_nine_clear(self):
        # the mac_p1 family with bit 9 clear decoded to nothing (143 instances)
        round_trips(self, [0x0807, 0x4D24, 0xB4A7, 0x3000, 0x051E, 0xB800, 0x0000])

    def test_free_bits_are_restored_per_word(self):
        # `.free` used to apply one mask to every word: a bit of the B word
        # leaked into the MAC's N word
        round_trips(self, [0x2808, 0x1652, 0xDF2A, 0x2FE6, 0x0014, 0x4200, 0x3000, 0x0611])

    def test_unread_stream_words_are_stated_with_slot(self):
        # a stream word that no operand reads is kept by `.slot @k`
        text, _ = round_trips(self, [0x1307, 0x65A2, 0x1210, 0x0025, 0xAFD7, 0x0A2F, 0xEB85])
        self.assertIn(".slot @2 afd7", text)

    def test_immediate_text_reads_back_to_the_same_bits(self):
        # 0.2 is the 24-bit field 233333 (its RAW word is 23333333): the text must
        # be the short decimal, not raw.233333
        text, _ = round_trips(self, [0x540C, 0x1C12, 0xAF1B, 0xAFE4, 0xCFE6, 0x7A2B,
                                     0x3000, 0x0638, 0x063D, 0x2333, 0x33C4, 0xC008])
        self.assertNotIn("raw.", text)

    def test_truncated_tail_is_kept(self):
        # the dump ends inside an instruction: its words are kept, not dropped
        text, report = listing([0x0904, 0x1430])
        round_trips(self, [0x0904, 0x1430])
        self.assertIn(".words 0904 1430", text)

    def test_length_zero_header_keeps_the_rest(self):
        text, _ = listing([0x0000, 0x3C01])
        self.assertIn(".words 0000 3c01", text)


class Policy(unittest.TestCase):
    """the table and the layout rules, as the review left them"""

    def test_unit_a_with_destination_eight_or_more_is_not_mov(self):
        # D >= 8 is a special A form (spec 6.1). The old table read 0x8042 as
        # `mov r0, r2` and lost the 8. It now stays verbatim until measured.
        text, report = listing([0x0902, 0x8042])
        self.assertIn("raw", text)
        self.assertNotIn("mov r0, r2", text)

    def test_wrong_bit_pattern_is_refused_not_guessed(self):
        r = ASM.assemble(".class 0x0040\n.free @1 0x0004\nmac_p1w2 r0, r0, r1, r0")
        self.assertFalse(r.ok)

    def test_slot_directive_refuses_a_word_a_read_uses(self):
        r = ASM.assemble(".class 0x0048\n.slot @0 1234\nmul r1, r0, #0.5@0:32")
        self.assertFalse(r.ok)


if __name__ == "__main__":
    unittest.main()
