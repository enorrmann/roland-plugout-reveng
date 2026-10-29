# Discrepancies — DSP ISA, assembler and disassembler

What is still open, and the device results that closed the others. A closed entry is removed from the open
list; its result is written into `DSP_ISA_SPEC.md` (or `DSP_ISA.md`). Settled questions are not repeated.

Status 2026-10-29: 3 items open (D4, D6, D10); D8 closed 2026-10-29. Everything else below is closed by a device probe or by a
code change with a passing test.

## Device session 2026-10-20

SYSTEM-8 1.32, lab plugin (`plugins/C-DSPT`, DSPTEST + DSPLIVE + PEEK) in slot 3, PLG3 selected. Probes:
`re/review_probes.py`, run on master DSP3 through `dspprobe` (the idle master context). Results:
`work/review_probes.json`: 24 probes, 24 match the expected value (the class 0x048 probes of D5 were run before the script; their numbers are in the table). The value compared is the result register
(r0, or r1 for the spec's 540c instruction). Registers: r0 .5, r1 .25, r2 .125, r3 −.375, r4 .0625,
r5 −.75, r6 .875, r7 −.0625.

| item | probe | result |
|---|---|---|
| D1 | `mac_p1wc2` with Y = r2 and the coefficient 0.5 at byte 2, 4 and 6 (C = 2, 4, 6) | r0 = X·Y + 0.5·Z in all three: the coefficient is read at byte C |
| D3 (Y) | `mac_p1 r0, r1, −r2, r3` and `… r2, r3` | the same result: bit 3 of a register Y is **not** a negation |
| D3 (W) | `mac_p1 r0, r1, r2, −r3` and `… r3` | −r3 negates: W's bit 3 **is** a negation (as spec 5.2 says) |
| D5 | `1503 8a43 4c42`, the AA class, with the D ≥ 8 op 0..7 | r0 = r1 + r3 + r[op] for all eight values |
| D5 | `0902 8043` … (class 0x048, four op values) | r0 = X + Y + r[op] (earlier session) |
| D7 | `hset h5` with Y = 1 and Y = 0 | the value is read at byte 1 (0x3456), not word 1 (0x5678) |
| D9 | op1 = 6, bit 9 set / clear (`mac_p1p2`, `mac_p1p2c0`) | set: X·Y + W + C·Z; clear: X + W + Z (no products) |
| D9 | op1 = 4, bit 9 clear (`mac_p1wc0`) | X·Y + C·Z: products kept |
| D8 | the spec's 540c instruction, coefficient word changed (2333, 2800, 2400, 2c00) | r1 = r0 + c·r7 + W for all four: the coefficient is read after Y, at slot 4 |

## Code changes made in this session (`asm/`, all tests pass, corpus 132/132 identical)

- `isa.json`: the MAC Y operand has no negation; bit 3 of a register Y is a free bit (`disasm.py` and
  `asm.py` restore it with `.free`). W keeps its negation.
- `isa.json`: new variant `sum8` (alu_a, D ≥ 8): r(D−8) ← X + Y + r[op]. It takes a register operand `R`
  (the op field). Measured in classes 0x048 and 0x0a8.
- `isa.json`: `mac_p1p2c0` and `mac_p1p2wp2c0` (op1 6 and 7, bit 9 clear, register Y) are the no-product form
  X + W + Z. The immediate-Y forms with bit 9 clear are now separate variants `…c0y` (inferred, products kept as
  before). `mac_p1wc0` is measured (op1 4).
- `tests/test_decoding.py`, `tests/test_regressions.py`, `tests/test_spec_examples.py`: expectations updated to
  the above. The NOTES.md evidence entry for the byte rule (D2) is on `add3` and `mac_p1wc2`; the evidence test
  checks it.
- `isa.json` `$doc`: the MAC Y and W notes.

## Closed

### D1. The MAC coefficient's read position with a register Y
Closed by the probe above: the coefficient is read at byte C (bytes 2, 4 and 6). Spec 4.4 now says so with
the measurement.

### D2. NOTES.md line for the byte rule
Closed: `isa.json` evidence entries on `add3` and `mac_p1wc2` cite `docs/NOTES.md`, line 837,
which states the byte rule. `tests/test_decoding.py` checks the line exists.

### D3. Is bit 3 of a MAC Y or W a negation?
Closed: Y — no (probe); W — yes (probe). Code and spec now agree with the device.

### D5. Unit A with destination 8–15
Closed for classes 0x048 and 0x0a8 (device, all op values). The listing prints the form as `sum8` (it was kept
verbatim before). Other multi-op classes are not yet probed; the same rule is expected to hold there, which is
a test to run rather than a discrepancy to argue about.

### D7. Hidden-register Y: byte or word?
Closed: byte (probe). Spec 9.0 and `DSP_ISA.md` say so.

### D9. Bit 9 of a MAC word
Closed: bit 9 switches the product terms for op1 = 6 with op2 = 000 (spec 7, "no products", now measured).
The `c0` names were kept; their meaning is written in their `$doc`/evidence.

## Open

### D4. Spec §12 layouts that the corpus contradicts
The table keeps the layout that reassembles more corpus instances:

| class | spec §12 | table | instances reassembling: spec / table |
|---|---|---|---|
| 0x240 | `MAANCCC` | `MAACN` | 0 / 7 |
| 0x572 | `ATACTTRRRC` | `AAACTTRRRC` | 0 / 8 |
| 0x592 | `AATCCT` | `AABCCT` | 0 / 16 |

Reassembly does not say which letter is right. Closing needs a probe per class that tells an ALU op from a
condition (or the other letter pairs): for example one instance of 0x592 with the table's `B` at position 2
replaced by an `A`, and the register result compared with the listing. Not done in this session.

### D6. Unit B words in classes 0x258 and 0x140
11 instances in 0x258 and 9 in 0x140 stay verbatim (corpus_all + corpus_s1). In each, position 2 (0x258) or 1
(0x140) holds a transfer word (`cd0f`, `cd0e`: store to bank 5, mode `d0`), where the layout has an ALU op.
Trying the other letter pattern decodes them without a raw fallback, but the immediates come out as values like
3.7e-09, i.e. noise. So the letter is not the whole story: the immediate or array placement of these instances
differs. Not resolved.

### D8. Where the MAC coefficient is read (closed 2026-10-29)
Rule, measured on the SYSTEM-8 master (re/dspnative.py `mac_*k*`, and class sweeps: a Roland instruction of each
of 19 MAC classes with transfers as a template, the other units neutralised, marker words in the slot array, C
swept 0/2/4/6, bit 9 of N clear and set, Y a register and an immediate): **the immediate coefficient is read at byte
C of the slot array (the N words included), plus 8 when bit 9 of the second word is set**. Y (an immediate) at byte
Y, W at byte W, as before. The old rule ("right after Y's immediate") matched the spec's 540c instruction only
because it has C = 0 with bit 9 set (byte 8, which is also where "after Y, over the two references" lands).
Plain-layout results that looked inexplicable here (the coefficient 1.5 = the N word) were C = 0: the assembler
left C at 0 when no `@` was given. A byte past the end of the instruction reads what follows it (D10): code must
not use it. 20 forms (`mac_p1wk*`, `mac_n1wk*`, `mac_p1p2k*`, `mac_n1p2k*`, /2, -Z/-W) exact on 1536 cases each.
Code: `emu/dspdec.h` `mac_coef_byte`; `asm/asm.py` places the coefficient at a byte 0-7 and writes it into C;
`asm/disasm.py` reads it there. 50 corpus instructions decode to a different coefficient than before.

### D10. Reads past the end of an instruction
Not an ISA question. 20 corpus instructions read past their last word; the hardware reads the next
instruction's words there. The listing keeps those lines verbatim. A tool would need the next instruction to
represent them; that is listing work, not spec work.

## Not listed here

- Rejected lines, sizing, round-trip divergence: zero on the corpus after the 2026-10-20 review (`docs/REVIEW.md`).
- The coefficient missing from the listing: fixed (`mac_*` with a coefficient show `#value`).
