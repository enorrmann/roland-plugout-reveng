# Open discrepancies — DSP ISA, assembler and disassembler

Scope: **only what is still unresolved.** Each entry gives the evidence, why it is not resolved, and what
would close it. Settled questions are deliberately absent; when one is closed it is removed from here (or
moved into `DSP_ISA_SPEC.md` if it changes the spec). The 2026-10-20 review that closed the earlier entries
(D1 coefficient operand, D3 round trip, D4 fixture errors) is written up in `asm/REVIEW.md`.

State at the time of writing (`asm/`, working tree, not committed):

```
python3 -m unittest discover -s tests   (in asm/)   74 tests, all pass, 1 skipped
python3 tests/roundtrip_report.py                  132 of 132 files identical (128 corpus_all + 4 corpus_s1)
python3 tests/coverage.py                          corpus_all 30313 of 30478 instructions decoded (99.5 %), 165 verbatim
```

Every decoded line is re-assembled before it is printed; a line that does not give back its own words is
kept verbatim, so the figures above mean "decoded and exactly reproducible", not just "matched a template".

---

## D1. The MAC coefficient's read position: byte C vs in-order after Y

**Status:** both branches of spec 4.4 are implemented and reproduce the corpus exactly. Only one branch has
been checked against the device.

**Evidence.**
- Spec 4.4: *"A MAC: Y at byte Y, W at byte W, C right after Y's immediate (or at byte c when Y is a
  register)."*
- `Y` immediate (`modeY ≠ 0`): **1151** corpus instances, read in order after Y. Its reading comes from the
  device-derived §4.4 example (`540c 1c12 af1b afe4 cfe6 7a2b 3000 0638 063d 2333 33c4 c008`, class 0x2a0):
  Y = 1.0 at byte 2, coefficient 233333 = 0.2 after skipping the two reference words, W = c4c008 at byte 11.
  The spec 4.4 derives this from the device (`re/dspverify.py`, 2026-10-17); the reassembly confirms the reading.
- `Y` register (`modeY = 0`): **2124** corpus instances, coefficient read at byte C. Example
  `2b09 2c8f 6fe2 7fe4 5983 0407 0403 a2aa aa00` (class 0x158): C = 6, coefficient `a2aaaa` = −1/6 (the
  listing: `#-0.16666603@6`). The value is plausible; no device check has been made.

**Why it is not resolved.** The byte-C branch rests on the spec's wording and on the reassembly of 2124
instructions, which proves the bits are read back the same way, not that the hardware reads them there.

**What closes it.** A probe on master DSP3 (the method of `re/dspverify.py`): a MAC with `modeY = 0`,
`op2 = 2`, a known register Y, and a coefficient word placed at the byte named by C. Compare the stored
result with `X·Y + W + c·Z`. Two different C bytes should give two different results.

---

## D2. Where the NOTES.md line for the byte rule is

**Status:** open, low risk.

**Evidence.** `DSP_ISA_SPEC.md` §4.4 gives the byte rule and cites `re/dspverify.py` ("~100 random
Roland instructions"). The `isa.json` evidence entries cite `NOTES.md` line numbers, which
`tests/test_decoding.py` checks only for existence and non-emptiness. No table entry cites the byte rule.

**What closes it.** Either add an evidence entry for the byte rule that points at a real `NOTES.md` line
(the existing test then checks it), or state that `DSP_ISA_SPEC.md` §4.4 is the sole source.

---

## D3. MAC Y and W: is bit 3 a negation?

**Status:** open. The table and the listing treat bit 3 as a negate; the spec says the measured MAC cases do
not show it.

**Evidence.**
- `DSP_ISA_SPEC.md` §5.2 (table, row "mY, Y"): *"register (bits 2–0; bit 3 was **not** a negation in the
  measured MAC cases)"*.
- `asm/isa.json`, MAC form: `Y` and `W` have `negate_bit: 3`. The listing therefore prints `-r7` for
  `2b09 2c8f …` (Y field `8f`), and `DSP_ISA.md` §6 reads the same word as `r7·r2`.
- The two readings give opposite signs for the same word, so one of them is wrong. The round trip is
  indifferent (the same bit is written back either way).

**What closes it.** A probe of a MAC with the Y (and W) field bit 3 set, register operand known, comparing
the result with `+r` and with `−r`. Until then the listing keeps the table's reading and the spec's
sentence stands.

---

## D4. Spec §12 layouts that the corpus contradicts

**Status:** open. The table keeps the layout that reassembles more corpus instances; the spec table lists
another.

**Evidence** (`python3 asm/tests/class_layouts.py`; instances in `work/corpus_all` and `corpus_s1`):

| class | spec §12 | table (kept) | instances re-assembling: spec / table |
|---|---|---|---|
| 0x0E0 | `CCC` | `CC` | 0 / 12 |
| 0x240 | `MAANCCC` | `MAACN` | 0 / 7 |
| 0x572 | `ATACTTRRRC` | `AAACTTRRRC` | 0 / 8 |
| 0x592 | `AATCCT` | `AABCCT` | 0 / 16 |

For 0x0E0 the table is `CC` because `1c04 0090 1a10 0003` is a jump and a condition with its slot word
(spec 5.4b); the spec's `CCC` has no slot position for that word.

**What closes it.** Decide per class whether the spec table or the instances are right, then correct the
spec §12 row (or the table). The 0x240, 0x572 and 0x592 instances are few (7–16); a probe on one of each
would settle the letters.

---

## D5. Unit A words with destination 8–15 outside class 0x048

**Status:** open; the listing keeps these words verbatim.

**Evidence.** Spec 6.1: *"D ≥ 8 (single A op, class 0x048): r(D−8) ← X + Y + r[op] … Not measured inside
multi-op classes."* The corpus has about 85 such words in multi-op classes (`0x2e0`, `0x188`, …); they were
decoded as ordinary ops until 2026-10-20, which lost the 8 (`0x8042` printed as `mov r0, r2`), so the plain
A forms now take D < 8 only.

**What closes it.** A probe of a multi-op class (for instance 0x2e0) with a D ≥ 8 word, comparing the
destination with `X + Y + r[op]`. Once measured, the variant can be added to `isa.json` with that class's
evidence, and the verbatim count drops.

---

## D6. Unit B words in classes 0x258 and 0x140 do not decode

**Status:** open. About 48 corpus instances (27 in 0x258, 21 in 0x140).

**Evidence.** The spec layout `MABTNR` for 0x258 re-assembles 65 of 111 instances; the others have a
transfer word where the layout has a B. In 0x140 (`MBTN`) the B position fails for `cd0e`-type words, which
are transfers (store to bank 5, mode `d0`). The layout is probably right for some instances and a different
pattern holds for the rest.

**What closes it.** Decode the failing instances by hand against the mode bytes and find the pattern that
separates them; then either a second layout for the class or a note in `DSP_ISA_SPEC.md` §12.

---

## D7. Hidden-register Y: byte offset or word count?

**Status:** open. The code reads Y as a byte offset, the same as an ALU op.

**Evidence.** Spec 9.0 says *"Y = … for immediates the number of words of the instruction's immediate stream
to skip"*. The example `1d06 4022 7fe0 c1fc 5100 0000` (h4 = 324) reads the value at byte 2 (word 1). A
word count of 2 would read byte 4. Both readings reproduce the corpus, so the corpus does not separate
them (the word reading was not tried).

**What closes it.** A probe of `40TY` with Y = 1 or 3 (odd): the byte reading lands mid-word, the word
reading does not. Also check against the spec's own example value.

---

## D8. Sequential reads over jump words

**Status:** open; [unknown] in the spec.

**Evidence.** Spec 4.4: a sequential read steps over the words a transfer uses as references. Nothing says
whether it also steps over a jump's offset word. The code steps over references only (`layout.stream_bits`,
`refs`), and that is what reproduces the corpus; no corpus instance has a sequential read crossing a jump
word, so the corpus cannot decide.

**What closes it.** A probe with a MAC coefficient (or a hidden-register load) whose sequential read crosses
a `jmp` word.

---

## D9. Register coefficient with bit 9 clear

**Status:** open. Six `…c0` variants (`mac_p1c0`, `mac_p1w2c0`, `mac_p1wc0`, `mac_p1p2c0`, `mac_p1p2wp2c0`,
`mac_p1p2wc0`) are `inferred`.

**Evidence.** Spec 5.2: bit 9 = 1 with a register coefficient. 143 corpus instances have a register
coefficient with bit 9 clear; the variants reproduce them exactly, and nothing else is known about them.

**What closes it.** A probe of a MAC with a register coefficient and bit 9 clear, comparing the result with
the register form.

---

## D10. Reads past the end of an instruction

**Status:** not a discrepancy; a representation gap. 20 corpus instructions read past their last word.

**Evidence.** Spec 4.4: the hardware reads the next instruction's words there. The listing cannot say what
those words are without the next instruction, so these lines stay verbatim.

**What closes it.** Nothing in the spec needs changing. A listing that keeps the next instruction's words
with the line would represent them; that is a tool change, not an ISA question.

---

## Not listed here, and why

- **Rejected lines, sizing and round-trip divergence** — all zero on the corpus after the 2026-10-20 review;
  see `asm/REVIEW.md`.
- **The coefficient operand missing from the listing** — fixed (`mac_*c1/c2/c3` now show `#value`).
- **Hidden values h4/h5, bit operations, `X9bY` flags** — settled in `DSP_ISA_SPEC.md` §8–§9.
