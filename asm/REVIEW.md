# Assembler and disassembler: review, fixes and tests

Reviewed 2026-10-20. Scope: `asm/` (`asm.py`, `disasm.py`, `codec.py`, `isa.py`,
`isa.json`, `layout.py`, `slots.py`) and its tests. The spec it implements is
`examples/from_scratch/DSP_ISA_SPEC.md`. Nothing was committed.

## Result

| | before the review | after |
|---|---|---|
| corpus files that re-assemble to the same words (corpus_all) | 0 of 128 | 128 of 128 |
| corpus_s1 files that re-assemble to the same words | not tested | 4 of 4 |
| lines the assembler rejects on the corpus | 441 | 0 |
| instructions decoded (corpus_all, share) | 98.1 % | 99.5 % |
| decoded instructions that fail to re-assemble | not measured | 0 (checked on every listing) |
| tests | 52; 4 failing, and the documented commands did not run | 74; all pass, 1 skipped (`test_decoding_share` needs `work/corpus`, which this machine does not have) |

The "before" decoding share counts lines the old code printed as decoded even
when its own assembler could not take them back, and some of those printed the
wrong operands (see defects 2, 3 and 10). The "after" share is different in
kind: a line is only printed as decoded if it re-assembles to its own words.

## How to run

```
cd asm
python3 -m unittest discover -s tests          # all tests, about 70 s (corpus round trip)
python3 tests/test_regressions.py             # one module
python3 tests/roundtrip_report.py [N]         # per file: identical / value / length
python3 tests/coverage.py [--check]           # decoding share by family and confidence
python3 tests/class_layouts.py                # isa.json layouts vs spec 12, by re-assembly
```

Run from `asm/`. `python3 -m unittest asm/tests/x.py` from the repository root
does not work: the directory `asm/` shadows the module `asm`.

## Defects found and fixed

Ordered by how much they mattered. "Corpus" is `work/corpus_all` and
`work/corpus_s1`.

1. **The disassembler was not safe to run.** `disasm.py` used `isa.TableError`
   without importing `isa`, so an error path raised `NameError`.

2. **Immediates were placed in the wrong words.** The assembler gave an
   instruction one immediate value and wrote it only into the slot words that no
   reference used. Spec 4.4 says the immediate stream is the whole slot array
   (references included), and that each operand reads it at its own byte. Both
   tools now use one rule (`layout.stream_bits`, `layout.array_positions`), and
   the assembler lays every immediate over the array in the order the hardware
   reads them. Bits that disagree with a word the array already fixes are an
   error; they are never overwritten.

3. **MAC immediates were misread.** The disassembler read a MAC's Y immediate at
   byte 0 and dropped the coefficient entirely. Now: Y at byte Y; the coefficient
   right after Y's immediate (skipping reference words), or at byte C when Y is a
   register; W at byte W. The spec 4.4 worked example now decodes to the values
   the spec gives (`#1@2:16, #0.2, r7, #-4.75…@11:24`). The old listing printed
   `#418.6875`.

4. **Immediate text did not read back to its bits.** Values were printed with
   `%.7g`, which is not exact for RAW values, and a 24-bit field such as 233333
   printed as raw bits. The disassembler now prints the shortest decimal that
   encodes back to the same bits (0.2 for 233333), and falls back to `raw.hhhh`
   only when none does. The assembler stores a stated width by truncation and
   notes a real loss.

5. **Hidden-register loads (spec 9.0).** Their values are integer words scaled by
   1/16384, not RAW numbers; the old code decoded them as RAW. The assembler also
   chose the first `hset` variant (h4) for every `hset`, so `hset h5` came back
   as `h4`. Fixed: integer values, and the variant is chosen by the register
   named. Y is the byte the value is read at (see open items).

6. **The assembler used the first variant of each mnemonic.** Several mnemonics
   name more than one variant (`ld` by mode, `hset` by register, MACs by
   coefficient and bit 9). The assembler now picks the variant whose operand kinds
   match what the source wrote (immediate or register, `hN`), and the result is
   still checked against the words.

7. **Class table errors.** The layouts in `isa.json` disagreed with spec 12 and
   with the corpus. 32 layouts changed (`git diff asm/isa.json` lists them, and
   each has a `$doc` note). A spec 12 layout was adopted only where it
   re-assembles more corpus instances than the old one (`tests/class_layouts.py`
   is the evidence). Examples:
   - 0x328 was `AABTT`; the spec and the corpus say `ABTTRR`. A transfer word had
     been read as an ALU word, which is why "body word 3 (B) has no mnemonic"
     appeared for it.
   - 0x0E0 is `CC`, not `CCC`: a jump and a condition, with the jump's slot word
     in the array (`1c04 0090 1a10 0003`, spec 5.4b).
   - 0x45A is `MMBTTTNN`: two MACs need two N words.
   - 0x160 and 0x318 lost their `body_lengths`: that filter rejected instruction
     sizes the SYSTEM-1 corpus has (body 8 for 0x160). The layouts alone now
     separate them.
   - 0x4AA (`MABBTTN`) and 0x60A (`ABCTTT`) were missing; both are in
     `re/dsp_templates.txt` as verified. The corpus had 12 instances of 0x4AA that
     matched no class.

   Adopting a spec layout is a judgment by reassembly, not by a measurement of
   the letters. A letter can be wrong and still reassemble. Where that matters,
   the class is listed in open items.

8. **The split between units and slot array was wrong.** `layout.letters_for`
   and `_core_of` removed the last `R` letters by count, so a layout with an `R`
   in the middle lost its tail; and a `C` or `R` after the last transfer was read
   as a unit. There is now one split point, `layout.prefix_length`: the units
   fill the prefix (up to the last T), and everything after it is the slot array.

9. **Unit B lost bit 2 of X.** The nibble at bits 11-8 holds `op*2 + X[2]`
   (spec 5.1). The X field write cleared it. Fixed in `codec.encode`.

10. **Unit A with destination 8..15 was decoded as an ordinary op.** `0x8042`
    came out as `mov r0, r2`, which lost the 8. Spec 6.1 gives D >= 8 its own
    meaning, measured only in class 0x048. The plain A forms now take D < 8, so
    the D >= 8 words stay verbatim (open items).

11. **Condition groups lost their 8.** `setflags` (X9bY) printed `r(X & 7)`, so
    groups 1 and 9 both came out as `setflags r1`. It now prints the group
    (X - 8), and the assembler writes 8 + group.

12. **`.free` was one mask for every word.** A bit that belongs to a unit's word
    was applied to every word whose mask allowed it, so it leaked into a MAC's N
    word. `.free` is now per word: `.free @n 0xMASK` (n counts the header as 0).

13. **Words a listing could not represent were lost.** A dump that ends inside an
    instruction, and a length-0 header, were printed as comments and the words
    after them were dropped. They are now kept as `.words`.

14. **Jumps.** The disassembler did not claim a jump's slot, so the offset word
    counted as a stream word. The assembler did not write the jump's slot field,
    so a jump always read slot 0, whatever slot its offset occupied. A jump can now
    name its slot (`jmp .+3@2`); the disassembler claims it and the assembler
    writes it.

15. **Stream words that no operand reads were not represented.** Nonzero (and
    trailing) words in the slot array that nothing reads were regenerated as
    zeros. They are now stated with `.slot @k 0xNNNN`.

16. **MAC variants with bit 9 clear were undecodable.** 143 corpus instances use a
    register coefficient with bit 9 clear, which the table had no variant for.
    Six variants `...c0` were added (status `inferred`; the evidence cites spec
    5.2 and says the corpus reproduces them). Two variants that shared a mnemonic
    with a different bit 9 were renamed `...b9` (`mac_p1b9`, `mac_p1w2b9`), since
    the text has to tell them apart; eight exact duplicates were removed.

17. **Immediate reads past the instruction.** The hardware reads the next
    instruction's words there (spec 4.4). The listing cannot say what those are,
    so such a line stays verbatim (20 corpus instructions).

18. **The listing now checks itself.** `disasm.program` assembles each decoded
    line back before printing it. If the words do not come back, the line is kept
    verbatim and counted in `report["reassembly"]`. On the corpus that count is 0.
    This is the guarantee the whole file rests on; the round-trip test checks the
    same thing from outside.

19. **Smaller things.** The assembler's `.class` check rejected a class outside
    the table even on a `raw` line, whose words are verbatim (now allowed; an
    instruction still gets the error). The "0.1875" expectation in
    `test_spec_examples` was wrong: the Y word 3000 is RAW 1.0. Unused helpers were
    removed (`canonical`, `shares_core`, `classify`, `_emit`, `field_value`,
    `immediate_mode`, `read_index`, `free_slots`, `slot_start` of the old layout,
    and others).

## Tests

| module | what it checks | tests |
|---|---|---|
| `test_asm.py` | the encoder against spec 11 words; confidence policy; class and `.free` directives; slot-array sizing | 26 |
| `test_regressions.py` | one case per defect above, each a round trip, with the text where the text was the defect | 20 |
| `test_spec_examples.py` | the spec's worked examples (4.3, 4.4, 5.4b, 9.0, ...) and the number format | 8 |
| `test_decoding.py` | decoding of the unit kinds, ambiguity, unknown words stay verbatim, evidence of every variant cites a real section | 16 |
| `test_corpus.py` | every corpus file: no crash, exact round trip, no line that fails to re-assemble, decoding share | 4 |
| `roundtrip_report.py` | per file, the first divergent word (a report, not a test) | - |
| `coverage.py` | decoding share by family and confidence; `--check` compares against `baseline.json` | - |
| `class_layouts.py` | isa.json layouts that differ from spec 12, with reassembly counts (a report) | - |

`baseline.json` holds the numbers that `coverage.py --check` compares against.
Current: 99.5 % of corpus_all and 99.7 % of corpus_s1 decode; 132 of 132 files
round-trip exactly.

## Open items

These are not verified and the code does not claim them.

- **Unit A with destination 8..15, outside class 0x048.** About 85 corpus
  instances stay verbatim. Spec 6.1 gives the single-op form (r(D-8) <- X + Y +
  r[op]); the multi-op use is not measured.
- **Unit B words in classes 0x258 and 0x140.** About 50 instances. The spec layout
  reassembles about 60 % of the 0x258 instances; the others probably belong to a
  different letter pattern. Not resolved.
- **Sequential reads skip only reference words.** Spec 4.4 says reference words
  are skipped. Whether a jump word is skipped is unknown; the code does not skip
  it.
- **Hidden-register Y.** Spec 9.0 says Y counts words to skip, but the example
  `1d06 4022 7fe0 c1fc 5100 0000` (value 324) reads its value at byte 2, which is
  word 1. The code reads Y as a byte offset, the same as an ALU op. The word
  reading was not tried against the corpus, so the two are not separated.
- **Register coefficient with bit 9 clear.** The six `...c0` variants reproduce the
  corpus; their meaning is not measured.
- **Coefficient read at byte C with a register Y.** Spec 4.4 gives this rule with
  a bracketed note. It reproduces the corpus; the meaning is not measured.
- **Layout letters.** A changed layout (defect 7) reassembles more corpus
  instances; that does not prove the letter for each position is right. Each
  changed class has a `$doc` note in `isa.json`.
- **Hardware.** Nothing here was run on the device. The checks are: the spec's
  worked examples, the corpus round trip, and the decoding agreement with
  `re/` measurements already recorded in the spec. A wrong meaning that
  reassembles consistently would still pass.
- **Runtime.** The full suite takes about 70 s, nearly all of it the corpus round
  trip in `test_corpus.py`.

## Notes for the next change

- A new layout or variant needs an evidence entry that cites a section of the
  spec (the test `test_evidence_points_at_a_real_section` checks it).
- `isa.json` is the source of truth; `layout.py` and `codec.py` apply it. A new
  rule belongs in `layout.py` if both directions need it.
- Keep the self-check in `disasm.program`. It is what keeps a bad table entry from
  producing a listing that does not match the words.
