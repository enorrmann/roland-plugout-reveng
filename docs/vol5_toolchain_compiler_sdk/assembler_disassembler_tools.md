# Volume V — Chapter 4: The Microcode Toolchain — Assembler, Disassembler, Scheduler, Rules and Checks

> Sources: the module docstrings of `asm/*.py`, the data files of `asm/`, `tools/doccheck.py` and
> `tools/check_optimized.py`. Usage and parameters of every script: [TOOLS.md](../TOOLS.md). Review of the assembler's
> rules: [REVIEW.md](../vol7_cleanroom_evidence_archive/REVIEW.md). The ISA they implement:
> [Vol. II ch. 3](../vol2_dsp_core_isa/instruction_word_encoding.md) / [ch. 4](../vol2_dsp_core_isa/instruction_set_reference.md).

## 1. The pieces

| file | role (from its docstring) |
|---|---|
| `asm/isa.json` | the ISA table: forms, variants, field positions, selectors, fixed bits, status tags, classes and layouts; the mnemonic index is in [Vol. II ch. 4](../vol2_dsp_core_isa/instruction_set_reference.md) §15 |
| `asm/isa.py` | "The ISA table as objects. Pure interpreter: nothing here knows a mnemonic." `isa.Table.load()`, `table.classes[0x048]`, `table.mnemonics['mul']`, `table.constraints` |
| `asm/codec.py` | "Generic codec: the only place that knows how a table entry maps to words." `codec.decode(form, words)`, `codec.encode(form, variant, args)`; a word is only decoded when the chosen variant explains every field |
| `asm/layout.py` | program layout rules shared by the disassembler and the assembler: a class's *prefix* is what the units fill; the rest is the slot array |
| `asm/slots.py` | the slot array of one instruction (spec §4.3): N words first, references, the immediate bit stream; each word has at most one owner, a conflicting second claim is reported, never resolved by order |
| `asm/asm.py` | assembler: dspasm source → program words |
| `asm/disasm.py` | disassembler: program words → dspasm source |
| `asm/dspsched.py` | list scheduler for our microcode (SYSTEM-1 and SYSTEM-8) |
| `asm/dspbuild.py` | general build pipeline: phase-wrap rewrite, schedule, assemble, verify in the emulator |
| `asm/s1rules.py` | the SYSTEM-1 access rules R2–R10, checked on scheduled programs |
| `asm/roland_forms.py` | which op groups Roland's own programs put in one instruction (evidence before a device test) |
| `asm/proven_forms.txt` | op groups that shared one instruction in builds proven on the SYSTEM-1 (64 groups at the date of reading) |
| `asm/bad_forms.txt` | op groups that hung the SYSTEM-1 (or that the DSP does not run) |
| `asm/master_b_forms.txt` | single op forms that ran in a main program of the SYSTEM-1's master DSP B |
| `asm/proven_addresses.txt` | addresses (plain `ld` / `st`) that builds which ran on the device used, per DSP |
| `asm/tests/` | `test_asm.py`, `test_corpus.py`, `test_decoding.py`, `test_regressions.py`, `test_spec_examples.py`, `class_layouts.py`, `coverage.py`, `roundtrip_report.py`, `baseline.json` |
| `tools/doccheck.py` | every ```` ```dsp ```` block of the documents must assemble |
| `tools/check_optimized.py` | warnings `manual-wrap`, `filler`, `transfer-only`, `fusable` ([Vol. III ch. 6](../vol3_voice_allocation_scheduling/microcode_optimization.md) §2) |

## 2. Assembler (`asm/asm.py`)

```python
result = Assembler().assemble(text)
result.words        # flat 16-bit words in execution order
result.errors       # [(line, message)] — rejected, never guessed
result.notes        # [(line, message)] — assembled but flagged
```

The language is what `disasm.program` writes back:

```text
label:  unit [| unit ...]          ; comment
mac_p1p2 r0, r0, r1, r2, r1
raw @0x060 0fe0 0201              ; verbatim body words, optional class
```

Directives, each on the line before the instruction it changes:

| directive | meaning |
|---|---|
| `.class 0x0040` | the class of the next instruction (needed when two classes fit the units, or for a `raw` line) |
| `.free @n 0xMASK` | bits of word n (0 = header) that the variant leaves free and that the listing restores |
| `.slot @k 0xNNNN` | slot word k, which no operand reads (spec §4.4) |
| `.words 0xNNNN ...` | words kept as they are (a truncated tail) |

Immediates (spec §4.4): `#v@b:w` is a value read at byte b of the slot array, w bits wide. A MAC coefficient `#v@c` is
read at byte c (its C field; + 8 in the `...r` variants, bit 9 set), Y a register or an immediate (measured 2026-10-29,
[Vol. II ch. 8](../vol2_dsp_core_isa/isa_discrepancies.md) D8). An immediate with no byte is placed at the first byte
where its bits agree with the stream (a coefficient: bytes 0–7, written into C).

Confidence policy, enforced in the assembler: a line whose variant is not in `Table.EMIT_DEFAULT` (exact, measured) is
rejected unless `allow_inferred=True` is passed, which admits `inferred` and `fitted`. `raw` words and `.words` are not
emitted by default either. Ambiguity is rejected: if more than one class can hold the units, or a field that must be
preserved has no source, the line fails with the reason.

## 3. Disassembler (`asm/disasm.py`)

```python
text, report = disasm.program(words, table)
```

Its "honesty rules":
* a word the table does not explain becomes `raw` with its words verbatim — never an invented mnemonic (spec §14);
* `raw` also carries the class when the class would not follow from the units, so re-assembling always reproduces the
  original header;
* each line ends with a status tag (`exact`, `measured`, `inferred`, `fitted`, `raw`);
* `report` counts what happened and lists every instruction that stayed verbatim, with the reason;
* each line is re-assembled before it is printed; a line that does not give back its own words is kept verbatim
  (`report["reassembly"]`; on the corpus it is 0).

Result on the corpus (spec §13, 2026-10-20): every decoded line re-assembles and every file of the corpus (132 files)
comes back identical (`python3 -m unittest discover -s tests` in `asm/`).

## 4. Scheduler (`asm/dspsched.py`)

```python
import dspsched
packed = dspsched.schedule(lines)          # one-unit lines (with "mov rN, rN" fillers) -> instruction texts
```

Input: the plain program the generators write (one op or one transfer per line, a load delay as a `mov rN, rN` filler,
`;` comments). Output: the same in fewer instructions (independent ops share an instruction, loads and stores go into
op instructions, fillers disappear, units reordered within the dependencies). Semantics kept: every unit reads before
any writes; a loaded register is usable from the next instruction on; a store reads the register as the instruction
starts. The dependency distances, the per-instruction limits and the priority rule are listed in
[Vol. III ch. 6](../vol3_voice_allocation_scheduling/microcode_optimization.md) §5 (the same text as the docstring).
`dspsched.schedule(lines, load_proven())` fuses only the groups of `asm/proven_forms.txt`; a group of
`asm/bad_forms.txt` is never used, not even with `TRY_FORMS=all`.

## 5. Build pipeline (`asm/dspbuild.py`)

```python
import dspbuild
b = dspbuild.build(lines, dspbuild.S1)        # framing S1 (SYSTEM-1 section) or S8 (SYSTEM-8 slot / main)
b.words, b.text, b.plain_words                # optimized words with the framing, its text, the plain words
dspbuild.write('s1m0', out_dir, b)            # s1m0.s / s1m0.hex + check_optimized warnings
```

`build()` runs, in order (each step can be turned off for one call, or for every build with `DSP_OPT=0`):
1. **phwrap**: the manual 11-op phase wrap of the early programs becomes `ph_wrap x, r3, x` when the temporary is dead
   afterwards; same result except for a phase of exactly 1.0 (manual 1.0, `ph_wrap` −1.0);
2. **schedule**: `asm/dspsched.py`;
3. **assemble**, with the framing words around the code;
4. **verify** (`DSP_VERIFY=0` to skip): the scheduled program against the plain one in the emulator's C core, on 4
   random states for 6 samples each; every register, flag and memory word must be equal. Verified programs are
   remembered by their words (`asm/build/verified.txt`).

Command line: `python3 asm/dspbuild.py FILE.s [--s1|--s8] [-o DIR]`. (The docstring cites "[Vol. III ch. 6](../vol3_voice_allocation_scheduling/microcode_optimization.md)
§6"; the original of that document, now [Vol. III ch. 6](../vol3_voice_allocation_scheduling/microcode_optimization.md),
has sections §1–§5 only: the pipeline is documented here and in the docstring.)

## 6. SYSTEM-1 access rules (`asm/s1rules.py`)

Breaking one of them hung the device (measured by on-device bisection, [examples/system_1_jx/README.md](../../examples/system_1_jx/README.md) §5):

| rule | content | status in the docstring |
|---|---|---|
| R2 | bank 5: at most 1 load and 1 store per instruction | measured |
| R3 | bank 0: at most 2 loads and 2 stores per instruction | measured |
| R4 | bank 6: at most 2 loads per instruction | measured |
| R5 | one bank-5 access per instruction (load or store) | "our rule, inferred"; stricter than Roland (corpus count 2026-11-03 over 13845 SYSTEM-1 instructions: 1 load + 1 store 5 times, 2 stores 11 times only in the OS's frame headers, 2 loads never) |
| R6 | at most 2 loads of banks 5 + 6 + IOMD per instruction | inferred |
| R7 | a slave DSP stores to bank 5 only in `A100..A1FF` (`A000..A0FF` is what the ARM writes) | — |
| R8 | ops sharing an instruction: only the groups proven on the device (`asm/proven_forms.txt`) | — |
| R9 | never a group that `asm/bad_forms.txt` forbids | — |
| R10 | a slave DSP never accesses bank 5 at `A200` or above (`ldb5` / `FD6C` reach only `A000..A1FF` reliably; measured 2026-11-06) | measured |

Plus a warning for an instruction shape that is not part of a program that ran on the device (`PROVEN_PROGRAMS`:
C-S1V8, C-S1SY13). Use: `s1rules.check('s1m0', lines)` or `python3 asm/s1rules.py FILE.s ...`.

`asm/bad_forms.txt` (first entries, 2026-10-28): two unit-B ops in one instruction hung the SYSTEM-1 (MP-CSAW-H2 hung,
MP-CSAW-G2 ran); Roland fuses A+B and A+A+B. The MAC with an immediate coefficient beside an A op also hung the SYSTEM-1
(2026-10-29, `asm/roland_forms.py` docstring; [Vol. II ch. 10](../vol2_dsp_core_isa/native_ops.md) §5).

## 7. Evidence lists kept up to date by the builds

| list | updated by | used by |
|---|---|---|
| `asm/proven_forms.txt` | `tools/proven_forms.py promote BUILD/try_forms.txt --by PLUGOUT` after a `TRY_FORMS` build ran on the device | `dspsched` (fusion), `s1rules` R8 |
| `asm/proven_addresses.txt` | `make_s1cc.py --promote-addresses NAME` after a build ran on the device | `make_s1cc.py` warns for any other address |
| `asm/master_b_forms.txt` | by hand after a master-B build ran | `make_s1cc.py` warns for any other form in `s1mm1` |
| `work/roland_forms.json` (local, from Roland's code) | `asm/roland_forms.py` | evidence for a group before a device test |

## 8. Documentation check (`tools/doccheck.py`)

A ```` ```dsp ```` block is DSP assembly in the syntax of `asm/`: one instruction per line, `;` comments. Blocks are
assembled one by one with `asm.Assembler`; a block that fails is reported with its file and line and the exit code is
1. ```` ```c ```` blocks are not compiled. In this documentation suite every ```` ```dsp ```` block assembles
(`python3 tools/doccheck.py docs/vol*/*.md docs/vol*/*/*.md system-1/*.md`, run on 2026-11-10).
