# Open discrepancies — DSP ISA, assembler and disassembler

Scope: **only what is still unresolved.** Each entry gives the evidence, why it is not resolved, and
what would close it. Settled questions are deliberately absent; when one is closed it should be removed
from here (or moved into `DSP_ISA_SPEC.md` if it changes the spec).

State at the time of writing (`asm/`, working tree, not committed):

```
python3 tests/…                   52 tests, 4 failing, 1 skipped
python3 tests/roundtrip_report.py identical 0   value 128   length 0
```

---

## D1. A MAC's coefficient operand is not printed, so the instruction cannot be reassembled

**Status:** diagnosed, fix half-designed, **not applied**.

**Evidence.** `2b09 2c8f 6fe2 7fe4 5983 0407 0403 a2aa aa00` (spec §4.3, second example).
`w2 = 5983` decodes as `mac_p1p2c2`: `op2 = 2` (coefficient mode imm24), `coefreg`/bit 9 = 0.
The listing is

```
mac_p1p2c2 r2, r2, -r7, r5, r3 | ld r6, [0407]@1 | ld r7, [0403]@2
```

— five operands, no coefficient — so the assembler re-emits **8 words** with `5803` where the original
has **9 words** with `5983` and the two `I` words `a2aa aa00`. The coefficient operand is missing from
`shows` in the 36 `mac_*c1/c2/c3` (and `…c1r/c2r/c3r`) variants in `isa.json`; `mac_text` (in
`asm/disasm.py`) also never renders it, because it only maps `Y`/`W` to a mode field.

**Why it is not resolved.** Rendering the operand is only half: its *position* in the immediate stream
(see D2) has to be right first, or a round trip reproduces a wrong word with the right shape. Doing the
representation half alone produces `5803`/16 words — worse than today's silent 8-word output in a
different way.

**What closes it.** (a) the offset rule of D2; (b) add `C` to `shows` for those variants and render it
from `op2` (`1`/`2`/`3` = imm16/24/32, `0` = register); (c) the assembler must accept `#value@BYTE:WIDTH`
for it and let it drive the single immediate stream; (d) a regression test asserting the exact 9 words.

---

## D2. Where a MAC's immediate coefficient is read: `modeY == 0` vs `modeY != 0`

**Status:** rule derived and confirmed on one example, **not implemented**.

**Evidence.** Spec §4.4: *"A MAC: **Y at byte Y, W at byte W** (their fields), **C right after Y's
immediate** (or at byte c when Y is a register)"*. The second clause is the one that applies to D1's
case: `w1 = 2c8f` has `modeY = 0`, i.e. **Y is a register**, so the coefficient is read at **byte `c`**
= byte 6 of the slot array = slot 3 = `a2aa`; 24 bits from there = `a2aaaa` = −1/6.

The other clause (`Y at byte Y`, then `C` sequentially after it) was confirmed end-to-end on the
elided §4.4 example, located at `work/corpus_all/p0s_003743c.txt` word 87:

```
540c 1c12 af1b afe4 cfe6 7a2b 3000 0638 063d 2333 33c4 c008      class 0x2a0, MBTTNRRIIII
slot array = 7a2b 3000 0638 063d 2333 33c4 c008     refs = slots 2 (0638) and 3 (063d)
modeY = 1 (Y imm16, Y = 2) ; op2 = 5 -> cm = 2 (imm24), c = 0 ; modeW = 2 (imm16), W = 11

Y  at byte 2                    -> 30000000                        spec: 3000        ✔
C  sequential after Y, skipping -> 23333300 = 0.2                   spec: 233333      ✔
   reference slots 2 and 3
W  at byte 11                   -> c4c00000 = -4.75                 spec: c4c008      ✔
```

**Why it is not resolved.** The `modeY == 0` branch has no example in the spec and no second instance
in the corpus that has been checked, so `byte c` rests on one instruction plus the spec's wording. The
code still calls `stream.take(0, …)` for MAC operands — it implements neither branch, which is why the
`540c` test currently reports `#418.6875` instead of `0.2`.

**What closes it.** Find a second instruction with `modeY == 0`, `op2 != 0` in the corpus and check its
coefficient against the same derivation; then implement both branches:
`modeY == 0 → take(c * 8, bits)`, else `take(Y * 8, bits)` for Y and a **sequential** read for C.

---

## D3. `roundtrip_report.py`: every corpus file diverges by value, none by length

**Status:** open; cause not established beyond "rejected lines".

**Evidence.**

```
identical    0 files
value      128 files
length       0 files
  value    word 111  JUPITER-8_1_0017d80.txt    2919 -> 2742 words, 14 rejected
  value    word 378  SH-2_1_0013510.txt          847 ->  811 words,  0 rejected
  value    word  17  SH-2_1_0013bb0.txt         1728 -> 1689 words,  0 rejected
```

Sizing is fine (0 length divergences), but every file still diverges in value. Files with **0 rejected
lines** also lose words (e.g. `SH-2_1_0013510.txt`: 847 → 811), so rejections alone do not explain it.

**Why it is not resolved.** The first divergence is reported, but not *why* that word changed. A
word-count drop with no rejected lines means an instruction is being emitted with fewer words than it
was read with — the same shape as D1 — but this has not been attributed one word at a time.

**What closes it.** For one 0-rejection file, walk the first divergence back to the instruction that
produced it, and record whether it is D1 (missing operand), a different missing operand, or a slot
assignment. Then decide whether it is one defect or several.

---

## D4. Two of the four failing tests are fixture errors, not code defects

**Status:** open, trivial, but it makes the suite's signal unreliable until fixed.

**Evidence.** `tests/test_spec_examples.py`:

- `test_sequential_read_skips_reference_words` asserts `take(2*8, 16) == 0x30000000` on
  `[0x3000, 0x0638, 0x063D, 0x2333]` — byte 2 of that array is `0638`, not `3000`. The expectation is
  wrong (byte 2 is `3000` only in the real `540c` array, where slot 0 is `7a2b`).
- `test_immediate_stream_is_the_whole_slot_array` asserts the listing contains `0.1875`; no immediate
  in `540c` has that value — Y is `3000` = 0.1875 only as a top-16-bit pattern, and the listing prints
  the coefficient, not Y.

**Why it is not resolved.** They were written while the MAC path still returned wrong operands, and I
stopped rather than tune assertions to whatever the code happened to print — an assertion fitted to the
bug is worse than no test.

**What closes it.** Correct both expectations against the derivations above (the `540c` values), and
keep `test_worked_examples`'s `4.4` row failing until D1/D2 are implemented.

---

## D5. Line number for §4.4's `540c` example in `NOTES.md` — unverified

**Status:** open, low risk.

**Evidence.** The spec cites measurements for the byte rule (`re/dspverify.py`, "verified on ~100 random
Roland instructions"), and `isa.json` evidence entries cite `NOTES.md` line numbers, which
`test_decoding.py` checks only for existence and non-emptiness. No entry in the table cites the byte rule
itself, so nothing verifies that the rule is recorded where the table can point at it.

**What closes it.** Either add an evidence entry for the byte rule pointing at a real `NOTES.md` line
(and let the existing test check it), or state that `DSP_ISA_SPEC.md` §4.4 is the sole source.

---

## Not listed here, and why

- **The coefficient being absent from `shows`** — that is D1's cause, not a separate discrepancy.
- **Rejected lines in the corpus** (up to 14 per file) — they are visible in the report and are being
  reduced; they are not a discrepancy between spec and implementation. They do interact with D3.
- **Sizing** — 0 length divergences in the report; considered closed unless D3 shows otherwise.
