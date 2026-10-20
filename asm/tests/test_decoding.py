"""Regression tests for the three decoding outcomes the table must keep apart.

* **confirmed**: a word whose encoding the spec marks [exact]/[measured]. It
  must decode to one specific mnemonic, and it must keep doing so — these are
  the words a regression would silently corrupt.
* **ambiguous**: a word that more than one table entry could claim. The
  decoder must pick the one the spec's own section assigns it (nibble 0xF is
  split between the special-function and f4..f7 families by bits 7-6).
* **unknown**: a word no entry explains. It must stay verbatim, and the reason
  must say which field went unexplained.

    python3 tests/test_decoding.py
"""

import os
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))

import codec  # noqa: E402
import disasm  # noqa: E402
import isa  # noqa: E402

TABLE = isa.Table.load()
ROOT_SPEC = os.path.dirname(os.path.dirname(HERE))


def decode_unit(word, mnemonic_of_form):
    """decode one word through the form that owns it -> (mnemonic, reason)"""
    for name in mnemonic_of_form:
        v, why = codec.decode(TABLE.form_for(name), [word])
        if v is not None:
            return v.mnemonic, why
    return None, why


# ---------------------------------------------------------------------------
# confirmed encodings, each traceable to the spec or to NOTES.md
# ---------------------------------------------------------------------------

CONFIRMED_A = [
    (0x3C01, "addxy", "spec 11.1: r3 <- r0 + r1"),
    (0x4086, "newton", "spec 6.2: Newton step of the reciprocal"),
    (0x1050, "mov", "spec 11.3: r1 <- 0.5 (imm16)"),
    (0x1430, "mul", "spec 11.2: r1 <- r0 * 0.5 (imm32)"),
    (0x0102, "addh4", "spec 6.1: op 0, sub-code 4 (X = 4) reads h4"),
]

CONFIRMED_B = [
    (0x3C42, "absadd", "spec 6.3: rD <- |rX| + Y"),
    (0x8E09, "cmp", "spec 6.3: cmp, flags only (8e09 = r0 - r1)"),
    (0xAE10, "cmp", "spec 6.3: cmp with an immediate"),
    (0xB4C0, "max", "spec 6.3: max(r3, r0) measured"),
    (0xD150, "min", "spec 6.3: d150 = min(r5, imm 1.0)"),
    (0xD550, "max", "spec 6.3: d550 = max(r5, 0)"),
    (0xCF06, "recipseed", "spec 6.4: reciprocal seed"),
    (0x9F0D, "recipseed", "spec 6.4: the reciprocal-seed form with Y bit 3"),
    (0xCF16, "expsplit", "spec 6.4: cf16 is the sqrt split, mode bits select it"),
    (0x8F19, "expsplit", "spec 6.4: exp split"),
    (0x8F29, "sign", "spec 6.4: sign"),
    (0x8F39, "negmask", "spec 6.4: negative mask"),
    (0xCF36, "negmask", "spec 6.4: cf36 = ceiling (mode 3)"),
]

CONFIRMED_MARKERS = [
    (0xFF70, "f7", "spec 6.4: ff70 = program start/end marker"),
    (0xEF75, "f7", "spec 6.4: ef75 opens a per-voice section"),
    (0xEF74, "f7", "spec 6.4: ef74 closes a per-voice section"),
]

CONFIRMED_MAC = [
    ((0x0C0A, 0x9281), "mac_p1p2", "spec 11.5: linear interpolation"),
    ((0x0809, 0x5280), "mac_p1w", "spec 7: r0 <- r0*r1 + r2*r5 (op1 = 4: P2)"),
    ((0x3CCA, 0x1CB6), "mac_p1p2c3",
     "spec 7: Horner of the exponential (op1 = 6, coefficient mode 3 = imm32)"),
]


class Confirmed(unittest.TestCase):
    def test_unit_a(self):
        for word, want, why in CONFIRMED_A:
            with self.subTest(word="%04x" % word, why=why):
                got, reason = decode_unit(word, ["mul"])
                self.assertEqual(got, want, reason)

    def test_unit_b(self):
        unit_b = ["min", "recipseed", "f4"]
        for word, want, why in CONFIRMED_B:
            with self.subTest(word="%04x" % word, why=why):
                got, reason = decode_unit(word, unit_b)
                self.assertEqual(got, want, reason)

    def test_section_markers(self):
        unit_b = ["min", "recipseed", "f4"]
        for word, want, why in CONFIRMED_MARKERS:
            with self.subTest(word="%04x" % word, why=why):
                got, reason = decode_unit(word, unit_b)
                self.assertEqual(got, want, reason)
                # a marker must never claim an effect
                self.assertIn("no effect", TABLE.mnemonics[want].result)

    def test_mac(self):
        for (w1, w2), want, why in CONFIRMED_MAC:
            with self.subTest(words="%04x %04x" % (w1, w2), why=why):
                v, reason = codec.decode(TABLE.form_for("mac_p1"), [w1, w2])
                self.assertIsNotNone(v, reason)
                self.assertEqual(v.mnemonic, want)


# ---------------------------------------------------------------------------
# ambiguous words: the spec picks the family, so must we
# ---------------------------------------------------------------------------

class Ambiguous(unittest.TestCase):
    """Nibble 0xF is shared by the special functions (bits 7-6 = 00) and the
    f4..f7 forms (bits 7-6 = 01). The spec splits them by those bits; a word
    must land in exactly one family."""

    def test_nibble_f_split(self):
        for word, want_family in ((0xCF06, "sfu"), (0x8F19, "sfu"), (0x9F0D, "sfu"),
                                  (0xCF36, "sfu"), (0xFF70, "f_words"),
                                  (0xEF75, "f_words")):
            with self.subTest(word="%04x" % word):
                hits = []
                for name in ("min", "recipseed", "f4"):
                    form = TABLE.form_for(name)
                    v, _ = codec.decode(form, [word])
                    if v is not None:
                        hits.append(form.name)
                self.assertEqual(hits, [want_family],
                                 "%04x decoded by %r" % (word, hits))

    def test_min_versus_max(self):
        """min and max differ in one nibble bit: a wrong match would swap them."""
        self.assertEqual(decode_unit(0xD150, ["min"])[0], "min")
        self.assertEqual(decode_unit(0xD550, ["min"])[0], "max")


# ---------------------------------------------------------------------------
# unknown words stay verbatim, with the reason
# ---------------------------------------------------------------------------

class Unknown(unittest.TestCase):
    def test_unknown_unit_b_stays_raw(self):
        # D < 8 with a nibble the table has no bit-op for
        text, report = disasm.program([0x0A03, 0x7B10, 0x0000], TABLE)
        line = [l for l in text.splitlines()
                if l and not l.startswith(";") and not l.lstrip().startswith(".")][0]
        self.assertTrue(line.strip().startswith("raw "), line)
        self.assertEqual(report["raw"], 1)
        self.assertIn("no mnemonic", report["verbatim"][0][2])

    def test_reason_names_the_unexplained_field(self):
        # 0x7B10: nibble 0xB has no entry, so no variant can claim it
        v, why = codec.decode(TABLE.form_for("min"), [0x7B10])
        self.assertIsNone(v)
        self.assertTrue(why)
        self.assertIn("no variant matches", why)

    def test_class_outside_the_table(self):
        """A header whose length cannot fit is reported, not guessed."""
        text, report = disasm.program([0xFFFF, 0x0000], TABLE)
        self.assertEqual(report["decoded"], 0)
        self.assertEqual(len(report["verbatim"]), 1)
        self.assertIn("truncated", report["verbatim"][0][2])
        self.assertIn(".words ffff 0000", text)     # kept as words, not dropped

    def test_length_zero_cannot_continue(self):
        text, report = disasm.program([0x0000, 0x3C01], TABLE)
        self.assertEqual(report["decoded"], 0)
        self.assertIn("length 0", report["verbatim"][0][2])
        self.assertIn(".words 0000 3c01", text)      # nothing after it is lost

    def test_truncated_stream(self):
        text, report = disasm.program([0x0904, 0x1430], TABLE)   # claims 4 words
        self.assertEqual(report["decoded"], 0)
        self.assertEqual(len(report["verbatim"]), 1)


if __name__ == "__main__":
    unittest.main()


# ---------------------------------------------------------------------------
# confidence policy: what may be emitted, and on what evidence
# ---------------------------------------------------------------------------

class Confidence(unittest.TestCase):
    """The assembler must not confuse a syntactically recognised variant with
    one whose semantics is confirmed (spec: [exact] = zero error on random
    operands, [measured] = consistent but limited, [inferred] = derived)."""

    def test_every_variant_has_evidence(self):
        for form in TABLE.forms:
            for v in form.variants:
                with self.subTest(form=form.name, mnemonic=v.mnemonic):
                    self.assertTrue(v.evidence,
                                    "%s/%s has no evidence" % (form.name, v.mnemonic))

    def test_evidence_points_at_a_real_section(self):
        """every citation resolves: a '## N' heading of the spec, or a line of
        NOTES.md, whichever the evidence names."""
        sources = {}
        for rel in ("docs/DSP_ISA_SPEC.md",
                    "docs/NOTES.md"):
            path = os.path.join(ROOT_SPEC, rel)
            if os.path.exists(path):
                sources[rel] = open(path, encoding="utf-8").read()
        for form in TABLE.forms:
            for v in form.variants:
                for ev in v.evidence:
                    with self.subTest(form=form.name, mnemonic=v.mnemonic,
                                      section=ev["section"]):
                        text = sources.get(ev["source"])
                        self.assertIsNotNone(text, "unknown evidence source %r" % ev["source"])
                        if "SPEC" in ev["source"]:
                            heading = "## %s." % ev["section"] if ev["section"] != "5.4b"                                 else "## 5.4b."
                            alt = "### %s " % ev["section"]
                            self.assertTrue(heading in text or alt in text
                                            or "## %s" % ev["section"] in text,
                                            "the spec has no section %r" % ev["section"])
                        else:
                            # a NOTES citation is a line number: it must exist
                            lines = text.splitlines()
                            n = ev.get("line")
                            self.assertIsNotNone(n, "NOTES citation without a line")
                            self.assertLessEqual(n, len(lines),
                                                 "NOTES.md has no line %d" % n)
                            self.assertTrue(lines[n - 1].strip(),
                                            "NOTES.md line %d is empty" % n)

    def test_status_is_known(self):
        for form in TABLE.forms:
            for v in form.variants:
                with self.subTest(mnemonic=v.mnemonic):
                    self.assertIn(v.status, isa.STATUSES)

    def test_mac_confidence_follows_the_spec(self):
        """op2 coefficient mode decides the status (spec 5.2: mode 1 is
        'presumably imm16 [inferred]', modes 2/3 are measured)."""
        for form in TABLE.forms:
            if form.name != "mac":
                continue
            for v in form.variants:
                mode = v.when.get("op2")
                creg = v.when.get("coefreg", 0)
                with self.subTest(mnemonic=v.mnemonic, mode=mode, coefreg=creg):
                    if mode == 0 and creg == 0 and "c0" in v.mnemonic:
                        # bit 9 clear (spec 5.2 documents bit 9 = 1): measured on the
                        # device for op1 4 and 6/7; the immediate-Y and op1 2/3/5
                        # forms are reproduced by the corpus only
                        self.assertIn(v.status, ("measured", "inferred"))
                    elif mode == 0 and creg == 0:
                        self.assertIn(v.status, ("exact", "measured"))
                    elif mode == 0 and creg == 1:
                        # register coefficient: documented in spec 5.2, but no
                        # measured case has bit 9 set, so it stays inferred
                        self.assertEqual(v.status, "inferred")
                    elif mode == 1:
                        # spec 5.2 marks mode 1 as "presumably imm16 [inferred]"
                        self.assertEqual(v.status, "inferred")
                    else:
                        # modes 2/3 are measured (NOTES 933-934), but a set
                        # coefreg bit was never exercised by a measurement
                        self.assertIn(v.status, ("measured", "inferred"))

    def test_emission_policy(self):
        """exact/measured by default, inferred only on request, unknown never."""
        for form in TABLE.forms:
            for v in form.variants:
                with self.subTest(mnemonic=v.mnemonic, status=v.status):
                    if v.status in ("exact", "measured"):
                        self.assertTrue(TABLE.emittable(v))
                    elif v.status in isa.EMIT_OPT_IN:
                        self.assertFalse(TABLE.emittable(v))
                        self.assertTrue(TABLE.emittable(v, allow_inferred=True))
                    else:
                        self.assertFalse(TABLE.emittable(v, allow_inferred=True))
