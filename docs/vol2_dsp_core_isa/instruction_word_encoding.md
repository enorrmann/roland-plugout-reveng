# Volume II — Chapter 3: Instruction Encoding (DSP ISA specification, part 1: §1–§5)

> **Scope:** both (same ISA); measured on the SYSTEM-8 unless stated.

> Part 1 of the ISA specification (§1–§5), for assembler and compiler writers. Part 2 (§6–§14: ALU / MAC semantics,
> predication, transfers and memory, execution semantics, encoding examples, the template table, method and open
> items) is [Vol. II ch. 4](instruction_set_reference.md). A reference "§n" with n ≤ 5 points to this chapter, n ≥ 6
> to chapter 4.

Roland BMC DSP as used by the SYSTEM-8 (and its PLUG-OUTs). State 2026-10-20 (delay memory, hidden registers,
indexed transfers, buses, sqrt/log/frexp: §6.3–§9.1; assembler review of 2026-10-20: §4.3, §4.4, §5.2, §5.4b, §6.1,
§9.0, §12–§14). Open discrepancies with the probe that closes each: [Vol. II ch. 8](isa_discrepancies.md). Practical uses of each feature: [Vol. II ch. 9](operation_uses.md). Overview and history:
[Vol. II ch. 7](isa_overview.md). Hardware context: [Vol. I ch. 1](../vol1_system_architecture/soc_overview.md). Reference implementation of the
decoder and simulator: the C reference `emu/dspdec.h` + `emu/dspcore.h` (`emu/dspref.py`; `re/dspisa.py` archived 2026-10-29); measured template data: `re/dsp_templates.txt`, `re/dsp_units_ab.txt`.
ARM-side memory map of the DSP banks (static analysis): [Vol. II ch. 6](memory_banks_dma.md). First own program: [Vol. VI ch. 6](../vol6_reference_designs_applications/custom_osc.md).

Everything here comes from **measurements on the hardware** (exact register/memory read-back on a master DSP,
§13) unless marked otherwise. Status tags:

| tag | meaning |
|---|---|
| **[exact]** | measured, reproduced with random operands, error 0 |
| **[measured]** | measured on specific cases, consistent, not exhaustively swept |
| **[inferred]** | derived from measurements + corpus structure, not tested directly |
| **[unknown]** | not decoded — an assembler must not emit it (or only copy it verbatim from Roland code) |

---

## 1. Machine model

| resource | description | status |
|---|---|---|
| r0–r7 | 8 data registers, RAW number format (§2). The only registers transfers can load/store. **Shared by all programs of one DSP** within a sample: a slot program starts with whatever the previous program (main or other slot) left (measured: the DCO2 slot sees signal values left by the DCO1 slot) | [exact] / [measured] |
| hidden registers | h4–h7, loaded by C-position words `40TY` (§9.0). h4 = delay base (`f6c`) and the value A op 0 sub 4 adds, h5 = delay-tap result address (A op 0 sub 5), h6 = index of mode-`28` transfers, h7 unknown. Values are left by other tasks (master DSP4: h4 = 288, h5 = 0x105/16384; DSP3: 0 after a reboot): set them before use | [measured] |
| bank 0 (DT0) | data memory, 0x800 32-bit words per DSP, a **ring** addressed relative to a pointer that advances every sample (§9) | [measured] |
| other memories | reference prefix = bank (bits 15–13): bank 2 (`4nnn`) two pages alternating per sample, DMA window of the delay memory where the ARM wired it (§9.1); bank 3 (`60nn`) IOMD, not read back; bank 4 (`8nnn`) MXM inter-DSP bus, not kept to the next sample; bank 5 (`Annn`) plain persistent memory; bank 6 (`Cnnn`) parameters; bank 1 (`22nn`) mailbox (§9) | [measured] / bank 1, 6 [inferred] |
| delay memory | a ring advancing one sample per sample, addresses in units of 16384 samples; reached only through the bank-2 DMA (§9.1) | [measured] |
| condition bits (flags) | 16 bits = 4 groups of 4 flags, **set by `X9bY`** from the result of an op (§8), tested by C words; they persist (across programs and samples) until rewritten | [measured 2026-10-17] |
| program memory | per DSP, ARM address 0x41480000 + n·0x100000 + slot byte address | [exact] |
| time budget | slave DSP2/3: ≈ 930–950 single-op instructions per sample for main + both oscillator slots (main2/3 = 189): a DCO2 slot of 733 instructions ran, 753 did not (DCO1 slot 6). Master DSP3 (idle, `re/dspbudget.py`: K fillers before a ring counter): the counter runs with 900 fillers + 22 and stops with 920–950 → **≈ 925–945 instructions per sample on every DSP**. **An A/A/B instruction (class 0x160) costs the same as a single-op one** (same edge with A/A/B fillers, 3654 halfwords; not a memory limit). Over budget the end of the program (voice B) is lost; afterwards the device can stay unstable (wrong / no sound, LFO-like garbage) until a **power cycle** (seen twice on the slaves) | [measured] |

A **program** runs from its first word to its end instruction once per sample. **The DSP sample rate is 96 kHz**
(measured: ~96 000 executions/s on master DSP3; an oscillator computed with fs = 96 000 is in tune within ±1.2
cents on the slaves) **[measured]**. Registers do **not** survive between executions where other programs share the
DSP (they use the same registers); persistent state must live in memory (§9).

## 2. Number format (RAW)

32-bit sign-magnitude floating format **[exact]**:

```
bit 31     sign (1 = negative)
bits 30-29 exponent s (0..3)
bits 28-0  mantissa M (unsigned)
value = (-1)^sign * M / 2^(32 - 4*s)
```

| s | scale | range of |value| | resolution |
|---|---|---|---|
| 0 | 2^-32 | < 0.125 | 2^-32 |
| 1 | 2^-28 | < 2 | 2^-28 |
| 2 | 2^-24 | < 32 | 2^-24 |
| 3 | 2^-20 | < 512 | 2^-20 |

Encoding (what Roland's constants and the hardware use): take the **smallest s** with M = |v|·2^(32−4s) < 2^29,
M = round(...). Examples: 1.0 = `30000000`, 0.5 = `28000000`, 0.25 = `24000000`, −1.0 = `b0000000`,
2.0 = `42000000`, 100 = `66400000`, 1/192 = `01555555`, π/2 = `3921fb54`, ln 2 = `2b17217f`. Zero = `00000000`.
**Arithmetic is exact, then truncated toward zero** [exact, 2026-10-17, `re/dspexact.py`: 48/48 random
full-precision cases each for add, sub, mul, D + X·Y, the MAC (two products + W) and the MAC D ≥ 8 (/2)]: the
exact rational result is encoded once (no intermediate rounding) with the smallest s, M = ⌊|v|·2^(32−4s)⌋.
Results ≥ 512 are not representable (A sums wrap mod 512, §10). Words of the form `7ffxxxxx` are values just
below 512: negative results of the unit-B bit operations, shown modulo 512 (§6.3) [measured 2026-10-17].

The ARM side sees the same memory through three views (bank 0: IEEE float at +0, Q31 "SD31" at +0x2000, RAW at
+0x4000); the DSP itself only works in RAW.

## 3. Program image

* Instruction stream = sequence of 16-bit words **[exact]**.
* In program memory, each 32-bit little-endian word holds two stream words, **the upper half first**:
  `mem32[i] = stream[2i] << 16 | stream[2i+1]` **[exact]**.
* Slot programs (oscillator / effect slots loaded by the engine) start with `0a02 ff70` and end with
  `0a02 ff70 0b02 00f8 0001` (some omit the second `0a02 ff70`). A blob of 4n+2 bytes leaves its last `0000`
  unloaded; the final `0001` must be present **[exact]**.
* The master DSP3 main program has the form `0001 0001 <body> 0b03 0080 7fee 0001` **[measured]**.
* `0001` is a one-word no-op (header with length 1, class 0) **[exact]**.
* The start/end instructions are unit-B / C-class words whose full semantics are [unknown]: always copy them
  verbatim.

## 4. Instruction encoding

### 4.1 Header

```
header = class[15:5] | length[4:0]
length = number of 16-bit words of the instruction, header included (1..31)
```

**[exact]** (all 84 unique corpus programs tile exactly with this rule). The class is **not** a bit field: it is a
template identifier (§4.2). Only classes listed in §12 are known; any other class is [unknown].

### 4.2 Templates

A template fixes the meaning of every body position (position 1 = word after the header). Words carry **no** type
marks: the same 16-bit value means different things in different positions **[exact]**. Position letters:

| letter | meaning | format |
|---|---|---|
| `A` | ALU op, unit A | §5.1, semantics §6.1 |
| `B` | ALU op, unit B (same word layout as A) | §5.1, semantics §6.3 |
| `M` | MAC first word (w1) | §5.2, semantics §7 |
| `N` | MAC second word (w2) | §5.2 |
| `C` | condition word | §5.4, semantics §8 |
| `T` | transfer | §5.3, semantics §9 |
| `R` | reference (data address) | §5.5 |
| `I` | immediate bits | §4.4 |

General order **[measured on all classes]**:

```
[ A / B / M / C words ]  [ T words ]  [ slot array: N words first, then R words ]  [ I words ]
```

C words usually sit between the ops and the transfers; in a few templates the measurement put a C (or an
unidentified word) inside the slot area — see the notes in §12.

### 4.3 Slot array

The **slot array** = every word after the last `T`. Slot k = (k+1)-th word of that array. **N words come first**
(one per M). Among the remaining slots, the ones a memory transfer (mode `fe`) points to are **references**; the
ones nobody points to are the **immediate bit stream**, in order **[exact on the corpus]**. Transfers with another mode byte do not use a slot.
**Units and array (2026-10-20).** The units fill the positions up to the last `T` (for a class with no `T`: up to
its first `N`, `I` or `R`). Every position after that is a slot word, whatever letter the layout gives it: a `C` or
an `R` there is a slot word, not a unit. A slot word that no operand reads has no meaning to the listing, so the
listing states it with a `.slot` directive; an assembler has to reproduce it as well (`asm/layout.py`).

Example: `1805 5550 6fe2 7000 0219` = A(5550, imm16) T(6fe2: slot 1) | slot 0 = `7000` (the imm16 = 256),
slot 1 = `0219` (reference). In §12 the `R`/`I` letters after the transfers are therefore only the layout of the
most frequent instance; an assembler places references and immediates by this rule. A transfer addresses slot `Y >> 1` (§5.3). Example: `2b09 2c8f 6fe2 7fe4 5983 0407 0403 a2aa aa00`
= M(2c8f) T(6fe2: slot 1) T(7fe4: slot 2) | slot 0 = N(5983), slot 1 = R(0407), slot 2 = R(0403) | I(a2aa aa00).

### 4.4 Immediates

* An operand whose **mode** field (§5) is non-zero takes an immediate: mode 1 = 16 bits, mode 2 = 24 bits,
  mode 3 = 32 bits **[exact]**.
* The value is the **top** 16/24/32 bits of the RAW word (the rest are 0): imm16 `2800` = 0.5, imm24 `2c4000` =
  0.765625 **[exact]**.
* All immediates of an instruction are taken **in order from one continuous, MSB-first bit stream** made of the
  `I` words, without word alignment: two imm24 = 48 bits = 3 words (`800d 0002 2222` = `800d00` + `022222`)
  **[exact]**.
* If the instruction is too short, the hardware keeps reading into the **next instruction's words**
  (`0903 3060 2800` reads 8 bits of the next header) **[exact]**. An assembler must always emit
  `ceil(total_bits / 16)` I words, padding the last one with zeros.
* **Where an immediate starts [measured 2026-10-17 against the hardware with `re/dspverify.py`]**: the immediate stream is the **slot array** (N words first, references included; classes
  without transfers: the N and I words). An A/B op with an immediate reads it starting **Y bytes** into that array
  (Y = its 4-bit Y field): `2c25` (Y = 5) reads imm24 `43 00 00` = 3.0 from the middle of `eb43 0000`; `7054`
  (Y = 4) reads the third word; `f1e8` (Y = 8) the fifth. Y = 0 = the start. A MAC: **Y at byte Y, W at byte W** (their fields), **C right after Y's
  immediate** (or at byte c when Y is a register). Reading past the first word skips the words that a transfer
  uses as a reference (`540c … 7a2b 3000 0638 063d 2333 33c4 c008`: Y = `3000`, C = `233333` = 0.2 after
  skipping the two references, W at byte 11 = `c4c008` = −4.75 → r1 = r0 + 0.2·r7 − 4.75, exact). Verified on
  ~100 random Roland instructions (§13b).
* **Reads and references (2026-10-20).** A stream read over a reference word must agree with that word's bits; a
  disagreement is a conflict, and an assembler must refuse it rather than overwrite the word. An immediate read
  past the last word of the instruction takes the next instruction's words (the listing keeps such a line
  verbatim). An immediate without a named byte may be placed at the first byte whose bits agree with the stream.
* **Several A/B ops of one instruction share the immediate stream [exact, 2026-10-16, master DSP3; byte rule
  2026-10-17]**: each op that takes an immediate reads the stream at its own byte (above), with its own width; two
  reads of the same bits must agree. Measured with ops whose byte is the start: AB
  `1606 36b0 9410 2800 0000 4200` (A `r3 += r2·imm32`, B `r1 = max(r0, imm16)`) → A gets 0.5 (`28000000`), B gets
  0.5 (`2800`), not 2.0 (`4200`); `1605 3690 9410 2800 4200` (imm16 + imm16) → both 0.5; AAB
  `2c06 1c70 3682 a410 2800 4200` → A1 imm32 = `28004200` (0.500063), B imm16 = `2800`. A MAC's operands are consumed
  in order (combination MAC + A/B with immediates: [unknown]).

## 5. Word formats

### 5.1 ALU op word (units A and B)

```
 15   12 11  9  8   6  5  4  3   0
+-------+-----+------+-----+------+
|   D   | op  |  X   | mY  |  Y   |
+-------+-----+------+-----+------+
```

| field | bits | meaning |
|---|---|---|
| D | 15–12 | destination register 0–7; 8–15 = special variants (§6.1, §6.3) |
| op | 11–9 | operation (§6) |
| X | 8–6 | first source register r0–r7 (op 0: sub-code) |
| mY | 5–4 | mode of Y: 0 register, 1 imm16, 2 imm24, 3 imm32 |
| Y | 3–0 | (mY = 0) bit 3 = negate, bits 2–0 = register: Y = 8+n means −rn |

All fields **[exact]** for unit A. Note that the nibble usually called "B" (bits 11–8) = op·2 + X[2].

### 5.2 MAC words

```
w1:  D[15:12]  op1[11:9]  X[8:6]  mY[5:4]  Y[3:0]
w2:  Z[15:12]  op2[11:9]  C[8:6]  mW[5:4]  W[3:0]
```

| field | meaning | status |
|---|---|---|
| D | destination r0–7 (8–15 not measured) | [exact] |
| op1 | bit 0: negate P1; bit 1: add W; bit 2: add P2; op1 0/1 = no write | [exact] |
| X | register | [exact] |
| mY, Y | Y operand: register (bits 2–0) or immediate. Bit 3 of a register Y is **not** a negation: measured 2026-10-20 (`mac_p1`, Y = −r2 gives the same result as Y = r2). It is a free bit (`asm/`: restored with `.free`). Immediate modes use bits 3–0 as the byte (DISCREPANCIES D3) | [measured] |
| Z | register, bit 3 = negate | [exact] |
| op2 | bits 11–10 = mode of the coefficient c: 00 register C, 01 imm16 [inferred], 10 imm24, 11 imm32; bit 9 = 1 with a register coefficient (spec 5.2). Reassembly of the corpus: 1151 instances with an immediate coefficient read after Y, 2124 with Y a register and the coefficient at byte C (§4.4); with op1 = 6 or 7 and bit 9 clear there are no products: r ← ±X + W + Z (measured 2026-10-20); with op1 = 4 or 5 both products are kept (measured 2026-10-20, DISCREPANCIES D9) | [measured; 01 inferred] |
| C | coefficient register when op2 mode = 0. With an immediate coefficient C is free, except when Y is a register: then C is the byte the coefficient is read at (§4.4) | [exact; byte C measured on the device 2026-10-20 at bytes 2, 4 and 6 (DISCREPANCIES D1)] |
| mW, W | W operand: register (bit 3 = negate; measured 2026-10-20: `mac_p1` with W = −r3 gives X·Y − r3) or immediate | [exact; measured 2026-10-20] |

### 5.3 Transfer word

```
 15  12 11      4 3  0
+------+---------+----+
|  R   |  mode   | Y  |
+------+---------+----+
```

| field | meaning | status |
|---|---|---|
| R | 0–7: load rR ← memory; 8–15: store memory ← r(R−8) | [exact] |
| mode | `fe` = plain access to the referenced address; `28` indexed (§9.0); `a9` delay-tap request (§9.1). See table below | partly |
| Y | slot index = Y >> 1 (Y even in 99.97 % of the corpus; the 5 odd cases are [unknown]) | [exact] |

| mode byte | behaviour | status |
|---|---|---|
| `fe` `ff` `3e` `0e` `10` `12` | same address as `fe` | [measured] (use `fe`) |
| `da` `de` | **bank 5 indexed by h6** (2026-10-17): X 0–7 → rX ← bank5[⌊h6·16384⌋ + Y]; X 8–15 → store. No reference slot, plain memory (no ring), §9.0 | [exact, master DSP3; slaves by audio] |
| `db` `df` | **bank 5 indexed by h7** (same as `da` with h7) | [exact, master DSP3] |
| `d0`–`d7` | bank5[N·16 + Y] (N = low nibble of the mode byte): direct address 0x00–0x7F, no reference slot | [exact, master DSP3] |
| `d8` `d9` `dc` `dd` | bank 5 at an address that changes with time | [unknown] |
| `e0` `e1` `ed` `be` `8e` `ee` `ce` `7e` `fc` `fd` `f0` | other memory (reads 0 or other values where `fe` reads the word; `re/dspmodescan.py`: `80`–`8f`, `b0`–`bf`, `c0`–`cf` read bank5[0] or other values, `e0`–`ef` a constant, `90`–`af`, `f5`–`f7` load nothing) | [unknown] |
| `28` | **indexed**: rX ← bank0[⌊h6·16384⌋ + Y] (X 8–15: store); Y is an offset, no reference slot; ring-relative (§9.0) | [exact] |
| `a9` | **delay-tap request** with the read address in rX; results to bank 2 [h5..h5+2] (§9.1) | [measured] |
| `9b` | **set flags** (2026-10-17, master DSP3/DSP4, `re/dspx9bflags.py`): `X9bY` stores the flags of the result of **unit Y** of its own instruction into **condition group X−8** (X = 9/a/b → group 1/2/3 = conds 4–7/8–11/12–15; X = c is used by Roland before `cond0–3` but changed nothing on the master; X = 0–8, d–f: no effect). Semantics in §8. No register or memory written | [measured] |
| `9e` | no data register loaded, effect unknown | [unknown] |

### 5.4 Condition word (C)

```
 15 14 13        9  8     7     4  3     0
+-----+-----------+-----+--------+--------+
| 0 0 | cc (5 b)  | neg |  skipT |  skipF |
+-----+-----------+-----+--------+--------+
cond = flag[cc] XOR neg          (cc = 0..31, §8; 2026-10-17: the field is 5 bits, not 4)
if cond: unit skipT does not execute, else unit skipF does not execute  (0 / out of range: none)
bit 14 set (`40TY`): hidden-register load (§9.0); `009Y`: jump (§5.4b)
```

**Units** are numbered from 1 in position order: first the ops (A, B, M), then the transfers **[measured, classes
0x0B8, 0x168, 0x2F0 (incl. a transfer as unit 3): full 256-value sweeps on master DSP3]**. `x12` = "skip op 1 if
true, op 2 if false" is a select between two results written to the same register (`2d06 1050 1052 0e12 3000 0000`).
Class 0x570 (three ops + transfer) follows the same rule for fields 1–3 but skips **all** ops for field 0 and runs
all of them for some fields ≥ 8 when the condition is false **[measured, unexplained]**.

**Field details (2026-10-17, `re/dspcsweep.py`, `re/dspcreal.py` with real flags):**
* The chosen field (high nibble if the condition holds, low nibble if not) acts **only if bit 3 of the other
  field is 0** (`00c1` in class 0x148: cond0 false, but the high field c ≥ 8 cancels "skip unit 1").
* Field 0 / out of range: no effect in classes with 1–2 ops and in Roland's multi-C instructions, but in classes
  with **three A-unit ops and a single C** (0x2c8 AAAC, 0x570 AAACT, 0x240 MAAC) it **cancels all ops**. Roland's
  code never uses that case (checked on `b247…1220 0e14`, `b307…0930`, `ad08…0e12 1f30`).
* Out-of-range unit numbers are class-specific "slots": class 0x190 (A C C) field 2 also skips its only op
  (Roland: `3204 5044 2610 1f20`, 56 uses); in 0x56a (AAACCT) field 4 skipped ops 1–2 and fields 5/6/0 suppressed
  the load that writes the same register as op 3. Fewer than 0.5 % of Roland's instructions have such fields.

### 5.4b Jump (a unit in a C position) **[measured 2026-10-17, master DSP3 / DSP4 `efdc`, slaves by audio]**

```
009Y   jump by the signed 16-bit word number (Y >> 1) of the immediate stream / free slots (§4.3, §4.4),
       counted in 16-bit words from the END of the instruction (forward and backward)
```

* A jump word sits where templates put C words, and **counts as a unit** (ops, jumps, then transfers) for the C
  words of its instruction: `1c04 0090 1a10 0003` = "jump +3 unless cond13 (if cond13 skip unit 1)";
  `3b06 0090 0092 1812 0001 00cd` = two jumps (offsets in slots 0 and 1), cond12 picks one; `7106 fe08 0090 1820
  b9b1 0080` = cmp (unit 1), jump (unit 2) under cond12, flags (unit 3, transfer).
* Corpus: every Roland offset lands on an instruction boundary (JX main and DCO2 programs; a 4-way `switch` in
  `JX_b000_60a24bc8` whose branches all jump to one merge point). Roland only jumps forward.
* Measured: `0b03 0090 0003` skips the next 3-word instruction; offset 0 = no-op; a C word skips/keeps the jump
  by the flags. **Backward jumps work: loops** (`1c04 0090 1a10 fff5`: 3, 8, 16 iterations exactly; the
  offset is end-relative — an instruction placed right before the loop body ran once). On the slaves (DCO2 slot,
  audio, `re/dspflagsaw.py`) forward jumps and a 4-iteration loop give a SAW identical to the reference.
* **Cost** (master DSP3, `re/dspbudget.py skip|loop`): instructions jumped over cost **nothing** (a jump over
  1500 fillers, far beyond the ~930-instruction budget, kept the counter running); a loop of 4 instructions per
  iteration fits 180 iterations, not 185, with ~25 other instructions → **≈ 5 cycles per iteration: a taken jump
  costs about one extra cycle**. Untaken branches are free, so if/else costs only the branch executed.
* **Named slot (2026-10-20).** The offset word of a jump is the slot Y >> 1; the listing names it (`jmp .+3@k`, k =
  slot) and an assembler writes k into the jump's Y field. Whether a sequential read (§4.4) steps over a jump word is
  not measured (DISCREPANCIES D8).
* **Hazard**: a backward jump in a program that is being rewritten while it runs can land inside old code and loop
  forever (the slaves hung until a power cycle, 2026-10-17). Always switch to an empty program (start + `0b02 00f8`)
  first, write the body, the head last (`dspmaster.Session`, `dspown.load`).
* `0b02 00f8` (slot program end) in the middle of a DSP3 main program had no effect; `00c1`/`00c2` (S100, under C
  words) changed nothing visible **[unknown]**.

### 5.5 Reference word (R)

16-bit data address. **[exact]** for 0x0000–0x07FF = bank 0 word index. Prefixes seen in the corpus
**[inferred, static analysis]**: `02nn`/`04nn`/`06nn` local variables (bank 0), `22nn` mailbox between the main
program and the slot programs (`2200` = noise source, `2201` pitch, `2221`/`221d`/`2229`/`2225` slot outputs;
DCO2 extra inputs **[measured by audio]**: `2209`/`2216` = sync enabled (1 with Cross Mod SYNC or METAL),
`220a`/`2217` = cross-mod (METAL) enabled, `220b`/`2218` = sync pulse train (one impulse per DCO1 cycle),
`220c`/`2219` = 0 — written by the main program MAIN2/3, see [Vol. VI ch. 6](../vol6_reference_designs_applications/custom_osc.md) §2),
`C0nn` parameter block (ramped parameters written by the ARM, read-only for the DSP), `A0nn` bank 5,
`60e0`/`60f0`/`8400`/`8401` I/O-like addresses in master DSP0/1 routing programs, `84xx`/`a4xx`/`86xx` with
non-`fe` transfer modes (delay memory?) [unknown]. From the ARM side (`CEZUtilDSPDump` in the OS, static analysis, [Vol. II ch. 6](memory_banks_dma.md)) bits 15–13 of a reference look like the bank: 0 DT0, 1 DT1, 2 DT2, 3 IOMD, 4 MXM (inter-DSP audio matrix; a DSP pair writes its Z area, the next pair reads it in its M/O areas), 5 DPC, 6 = ENV **[inferred, not verified on hardware]**. `Xa9Y` only occurs in effect programs right after a `{8+d}f6c` B op, followed by loads from bank-2 `4001…` and a linear interpolation: probably a delay-line read pointer (WMEM, the WM2418-side wave/delay memory) **[hypothesis]**.
