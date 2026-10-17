# DSP instruction set — technical specification (for assembler / compiler writers)

Roland BMC DSP as used by the SYSTEM-8 (and its PLUG-OUTs). State 2026-10-16 (delay memory, hidden registers,
indexed transfers, buses, sqrt/log/frexp: §6.3–§9.1). Practical uses of each feature: `DSP_USOS.md`. Overview and history:
`DSP_ISA.md`, `NOTES.md`. Hardware context: `../../ARCHITECTURE.md`. Reference implementation of the
decoder and simulator: `re/dspisa.py`; measured template data: `re/dsp_templates.txt`, `re/dsp_units_ab.txt`.
ARM-side memory map of the DSP banks (static analysis): `DSP_ARM_SIDE.md`. First own program: `CUSTOM_OSC.md`.

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
ones nobody points to are the **immediate bit stream**, in order **[exact on the corpus: every decoding
inconsistency of the earlier "R then I" rule disappears]**. Transfers with another mode byte do not use a slot.
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
* **Where an immediate starts (2026-10-17, measured against the hardware with `re/dspverify.py`; supersedes the
  two bullets below)**: the immediate stream is the **slot array** (N words first, references included; classes
  without transfers: the N and I words). An A/B op with an immediate reads it starting **Y bytes** into that array
  (Y = its 4-bit Y field): `2c25` (Y = 5) reads imm24 `43 00 00` = 3.0 from the middle of `eb43 0000`; `7054`
  (Y = 4) reads the third word; `f1e8` (Y = 8) the fifth. Y = 0 = the start, which is why "every op reads from the
  start" held in the earlier tests. A MAC: **Y at byte Y, W at byte W** (their fields), **C right after Y's
  immediate** (or at byte c when Y is a register). Reading past the first word skips the words that a transfer
  uses as a reference (`540c … 7a2b 3000 0638 063d 2333 33c4 c008`: Y = `3000`, C = `233333` = 0.2 after
  skipping the two references, W at byte 11 = `c4c008` = −4.75 → r1 = r0 + 0.2·r7 − 4.75, exact). Verified on
  ~100 random Roland instructions (§13b).
* Consumption order (old, before the byte rule): within an A/B op: Y. Within a MAC: Y (w1), then c (w2
  op2 mode), then W (w2).
* **Several A/B ops of one instruction share the immediate [exact, 2026-10-16, master DSP3]**: every A/B op that
  takes an immediate reads it from the **start** of the bit stream, each with its own width. Measured: AB
  `1606 36b0 9410 2800 0000 4200` (A `r3 += r2·imm32`, B `r1 = max(r0, imm16)`) → A gets 0.5 (`28000000`), B gets
  0.5 (`2800`), not 2.0 (`4200`); `1605 3690 9410 2800 4200` (imm16 + imm16) → both 0.5; AAB
  `2c06 1c70 3682 a410 2800 4200` → A1 imm32 = `28004200` (0.500063), B imm16 = `2800`. The earlier observation
  "two A ops with immediates in class 0x0A8 len 5 made the program stop" was probably a consequence (the second
  op did not get the bits expected). An assembler gives an instruction **at most one immediate value** (several
  ops may use it, with their widths); a MAC's operands are consumed in order (combination MAC + A/B with
  immediates: [unknown]).

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
| mY, Y | Y operand: register (bits 2–0; bit 3 was **not** a negation in the measured MAC cases) or immediate | [measured] |
| Z | register, bit 3 = negate | [exact] |
| op2 | bits 11–10 = mode of the coefficient c (0: register C, 2: imm24, 3: imm32; 1 presumably imm16 [inferred]); bit 9 = 1 with a register coefficient | [measured] |
| C | coefficient register when op2 mode = 0 | [exact] |
| mW, W | W operand: register (bit 3 = negate) or immediate | [exact] |

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
| `e0` `e1` `db` `ed` `be` `8e` `ee` `de` `ce` `7e` `fc` `fd` `f0` | other memory (reads 0 where `fe` reads the word; writes not found in banks 0/1/2/4/5) | [unknown] |
| `28` | **indexed**: rX ← bank0[⌊h6·16384⌋ + Y] (X 8–15: store); Y is an offset, no reference slot; ring-relative (§9.0) | [exact] |
| `a9` | **delay-tap request** with the read address in rX; results to bank 2 [h5..h5+2] (§9.1) | [measured] |
| `9b` | **set flags** (2026-10-17, master DSP3/DSP4, `re/dspx9bflags.py`): `X9bY` stores the flags of the result of **unit Y** of its own instruction into **condition group X−8** (X = 9/a/b → group 1/2/3 = conds 4–7/8–11/12–15; X = c is used by Roland before `cond0–3` but changed nothing on the master; X = 0–8, d–f: no effect). Semantics in §8. No register or memory written | [measured] |
| `9e` | no data register loaded, effect unknown | [unknown] |
| (history) | Before 2026-10-17 `X9bY` was read as "write r(X−8) to port Y"; Roland uses Y = 1–3, usually in the same instruction as a unit-B op) does **not**: change r0–r7, change the condition bits (hypothesis from corpus statistics "port 1/2/3 → condition groups 4–7/8–11/12–15, read by a C word 2 instructions later": tested on master DSP3 with values ±0.5, 0, ±2, ±1/16, raw patterns 1/2/4/8/F, 0x000F0000, 0x7fffffff, 0xF0000000, distance 1–3, inside a unit-B instruction, and across the sample boundary; and on the slave DCO2 slot by audio — the groups never changed), change what `fe` loads read afterwards, redirect the result of the unit-B op of its instruction, set the hidden values of A op 0 sub 4/5, or reach the slot's audio output (noise written to ports 0–7 on the slaves: silence). `Xa9Y` loads nothing (ports 0–7 read back nothing, also with Roland's SAW running in the other slot). Also ruled out on 2026-10-16 (master DSP4 slot 0xF7DC and DSP3): condition bits in a context with delay hardware, h4–h6, delay-memory write, indexed bank-0 store; a unique marker written with `b9b1..3` is not found in any MXM window, IOMD (3 views), banks 0/1/2/5 of DSP3 or bank 5 of DSP2. (All those tests used an op whose result was constant, or read the wrong group.) | [superseded] |

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
`220c`/`2219` = 0 — written by the main program MAIN2/3, see `CUSTOM_OSC.md` §2),
`C0nn` parameter block (ramped parameters written by the ARM, read-only for the DSP), `A0nn` bank 5,
`60e0`/`60f0`/`8400`/`8401` I/O-like addresses in master DSP0/1 routing programs, `84xx`/`a4xx`/`86xx` with
non-`fe` transfer modes (delay memory?) [unknown]. From the ARM side (`CEZUtilDSPDump` in the OS, static analysis, `DSP_ARM_SIDE.md`) bits 15–13 of a reference look like the bank: 0 DT0, 1 DT1, 2 DT2, 3 IOMD, 4 MXM (inter-DSP audio matrix; a DSP pair writes its Z area, the next pair reads it in its M/O areas), 5 DPC, 6 = ENV **[inferred, not verified on hardware]**. `Xa9Y` only occurs in effect programs right after a `{8+d}f6c` B op, followed by loads from bank-2 `4001…` and a linear interpolation: probably a delay-line read pointer (WMEM, the WM2418-side wave/delay memory) **[hypothesis]**.

## 6. ALU semantics

All ops of one instruction **read their operands before any of them writes** (§10).

### 6.1 Unit A **[exact]**

D < 8:

| op | rD ← |
|---|---|
| 0 | sub-code in X (below) |
| 1 | D + X + Y |
| 2 | X · Y |
| 3 | D + X · Y |
| 4 | Y + X · D |
| 5 | Y − X · D |
| 6 | X + Y |
| 7 | Y − X |

(D = old value of rD, X = rX, Y = operand Y.)

op 0 sub-codes (X field):

| X | effect | status |
|---|---|---|
| 0, 3 | no effect | [exact] |
| 1 | rD ← Y (move, load immediate) | [exact] |
| 2 | Newton step: rD ← rD·r(D+1); r(D+1) ← 2 − r(D+1)·rY; rY ← r(D+1)·rY (all with old values) | [exact] |
| 4 | rD ← D + Y + h4 (hidden, context dependent) | [exact formula, h unknown] |
| 5 | rD ← D + Y + h5 | [exact formula, h unknown] |
| 6, 7 | rD ← D + Y | [exact] |

D ≥ 8 (single A op, class 0x048): r(D−8) ← X + Y + r[op] (the op field indexes a third register) **[exact]**.
Not measured inside multi-op classes.

### 6.2 Division

There is no divide op. Roland computes reciprocals with repeated op-0 sub-2 Newton steps (`0902 4086` ×3 in the
SAW oscillator) after an initial estimate.

### 6.3 Unit B

Same word layout. **[exact unless noted]**

| word form | rD ← |
|---|---|
| `D 0 X Y` (D < 8) | **rX AND Y** — bit ops on two's-complement fixed point, result **mod 512** (a negative result reads as 512 + x: the old "`7ffxxxxx`" format) [measured 2026-10-17, `re/dspbits.py`] |
| `D 2 X Y` (D < 8) | (NOT rX) AND Y |
| `D 4 X Y` (D < 8) | rX OR Y |
| `D 6 X Y` (D < 8) | (NOT rX) OR Y |
| `D 8 X Y` (D < 8) | rX XOR Y |
| `D a X Y` (D < 8) | NOT (rX XOR Y) |
| `D c X Y` (op 6, D < 8) | \|rX\| + Y |
| `D e X Y` (op 7, D < 8) | Y − \|rX\| |
| D = 8+d, op 0 | min(rX, Y) |
| D = 8+d, op 2 | max(rX, Y) |
| D = 8+d, op 4 | \|rX\| |
| D = 8+d, op 5 | \|Y\| |
| D = 8+d, nibble e, bits 7–6 = 00 | **nothing written**: flags of r(d) + Y (CMP / TST, for `X9bY`; `8e09` = cmp r0 − r1, `ae10` = cmp r2 with imm) [measured 2026-10-17] |
| D = 8+d, nibble e, bits 7–6 = 01 | Y (r(d) ← ±rY) [fitted] |
| D = 8+d, nibble e, bits 7–6 = 11 | **bits 5–4 select the function, Y bit 3 the variant** (no immediate) [measured 2026-10-17, `re/dspshape.py`]: 00: r(d) ± rY; 01: \|rY\| / clamp(rY, −1, 1); 10: wrap(rY) into [−1, 1) / triangle 1 − 2\|wrap(−rY − 1.5)\|; 11: (rY mod 2)/2 (phase 0..1) / −rY |
| D = 8+d, nibble c, modes 1–3 | **shifts** [measured 2026-10-17]: mode 1 r(d) ← X·2^Y, mode 2 X·2^−Y, mode 3 X·2^−(16+Y) (Y = the 4-bit field, not a register); mode 0 = frexp |
| D = 8+d, nibble e, bits 7–6 = 10 | writes r(d) and r(d+1) (r(d+1) ≈ 1.7–1.98, recip-seed-like); never used by Roland [unknown] |

X is 3 bits: the "B" nibble's low bit is X[2], so odd nibbles are the same ops with X in r4–r7 (measured
`d150` = min(r5, imm 1.0), `d550` = max(r5, 0), `e590` = max(r6, 0), `e190` = min(r6, 1.0): Roland's
`clamp(x, 0, 1)`). Y may be an immediate (mode bits). The ops with D < 8 and nibbles 0–a are bit operations (table above; the
"`7ffxxxxx`" values were negative results shown modulo 512) **[measured 2026-10-17]**. Some D ≥ 8 forms also change other registers.

Also measured (2026-10-16, D ≥ 8, random operands): nibble 2 = min(−X, Y), 6 = max(−X, Y) (bit 1 negates X);
nibble c = **frexp**: X = m·2^e, m ∈ [0.5, 1) → r(d) ← m, rY ← e (X = 0 → 0, −31).

### 6.4 Special-function unit (unit B, D = 8+d, nibble `f`, bits 7–6 = 00)

Selected by the mode bits (5–4) and Y bit 3; Y register = Y & 7. **[measured, random inputs]**

| mode | Y bit 3 | function | example |
|---|---|---|---|
| 00 | 0 | **reciprocal seed**: \|Y\| = 2^e·m (m in [0.5, 1)): r(d) ← sign(Y)·2^−e, r(d+1) ← ≈ 1/m (within ~1 %) | `cf06`: r4, r5 from r6; refined by three `0902 4086` Goldschmidt steps |
| 01 | (1) | **exp split**: r(d) ← e^⌊Y⌋, rY ← Y − ⌊Y⌋ | `8f19`: r0 = e^⌊r1⌋, r1 = frac(r1); Roland then evaluates e^frac by Horner |
| 10 | 1 | sign: r(d) ← ±1 | `8f29` |
| 11 | 1 | negative mask: r(d) ← −1 if Y < 0 else 0 | `8f39` |
| 11 | 0 | ceiling: r(d) ← ⌈Y⌉ | `cf36` |
| 01 | 0 | **sqrt split**: \|Y\| = m·4^−k, m ∈ [1, 4): rY ← m, r(d+1) ← ≈ 1/√m (~1 %), r7 ← sign(Y)·2^−k (seen with d = 4, Y = 6; r7 may be r(d+3) or r(Y+1)) → √\|x\| = r7·m·r(d+1) | `cf16`: 9 → m 2.25, r5 0.662, r7 2 [measured 2026-10-16] |
| 10 | 0 | **log2 split**: Y = m·2^e, m ∈ [0.75, 1.5): rY ← m, r(d) ← e; Y < 0: r(d) ← 9, rY ← Y/512 | `cf26`: 9 → e 3, m 1.125; 0.375 → e −1, m 0.75 [measured] |
| 00 | 1 | **atanh split** (for logarithms): x = rY, 2^k such that (x+1)/(2·2^k) ∈ [0.5, 1): r(d) ← (x−1)/(2·2^k), rY ← (x+1)/(2·2^k), r(d+1) ≈ 1/rY (~1 %) → r(d)·r(d+1) refined by Newton = (x−1)/(x+1); ln x = 2·atanh of it | `9f0d` after `8f25` log2 split, then `0902 1085` Newton (base engine, JUPITER-8) [measured 2026-10-17]; `8f09` same op |

**Seed ROMs [exact, 2026-10-17, `re/dspseed.py`]**: reciprocal seed (`cf06`, also used by the atanh split
`9f0d`): 64 entries, k = the 6 mantissa bits after the leading 1 of m ∈ [0.5, 1), R[k] = round(4096 / (0.5 +
(2k+1)/256)) / 4096 (64/64). Reciprocal square root seed (`cf16`): 48 entries, j = ⌊(m − 1)·16⌋ for m ∈ [1, 4),
value = Q[j]/256, Q = 252 245 238 232 226 221 216 211 207 203 199 195 192 188 185 182 179 177 174 172 169 167 165
163 161 159 157 155 153 152 150 148 147 145 144 142 141 140 138 137 136 135 134 133 131 130 129 128 (literal table;
no simple formula fits).

**Nibble `f`, bits 7–6 = 01** (sub-op in bits 5–4) **[measured 2026-10-17, master DSP3/DSP4, `re/dsphops.py`,
`re/dspf5.py`]**:

| form | function |
|---|---|
| `{8+d}f4Y` | rd ← r(Y & 7) (sign bit of Y ignored). Roland also uses it as rd ← rd (`bf4b`, `df4d`, `ff4f`, with `89ba`): no other effect found on the master |
| `{8+d}f5c` | rd ← **delay-ring write position**: counts down 1/16384 per sample (96 000 samples/s), mod 512, own value per DSP; independent of rd and h4. Not used by Roland |
| `{8+d}f5Y`, Y ≠ c | reads 0 on the master (`9f5d`: a per-DSP constant, 0.0007 on DSP3, 1.8746 on DSP4) and on the slaves' DCO2 slot (`cf54`, `cf55`: SAW × value = silence, by audio). Roland uses `cf54` (JX DCO2 slot, as a weight): probably set up by Roland's own main program **[unknown]**. `cf5c` is non-zero on the slaves too |
| `{8+d}f6Y`, Y = c–f | **rd ← (rd + h(Y − 8)) mod 512**: `f6c` + h4 (delay base, §9.1), `f6d` + h5, `f6e` + h6, `f6f` + h7 (JX: `af6f` = index + h7, wrapped at 8 with cmp/C). Y < 8: no change |
| `{8+d}f7Y` | **measured 2026-10-17: no effect on registers, flags, memory addressing (banks 0/5) or audio (slaves: a SAW with Roland-style `ef75`/`ef74` voice sections sounds identical); each costs one cycle (`dspbudget.py marker`: same budget edge as single ops); no ARM code refers to them**. Section markers. `0a02 ff70` = program start and end (SYSTEM-1 programs start with `ff71`); **`ef75` opens and `ef74` closes a per-voice section** (layout of every two-voice program: input stores, `ef75`, voice A, `ef74 ef75`, voice B, `ef74`, output stores, end). Their hardware role (scheduling / sync) is not visible to the DSP |

## 7. MAC semantics **[exact on the measured cases]**

```
P1 = Y · rX
P2 = c · Z          (c = rC or immediate per op2; Z = ±rZ)
rD ← (op1 & 1 ? −P1 : P1) + (op1 & 2 ? W : 0) + (op1 & 4 ? P2 : 0)
special case: op2 = 000 and op1 ∈ {6,7}: no products → rD ← ±rX + W + Z (2026-10-17: with an immediate Y the
P1 product stays: ±imm·rX + W + Z; op1 ∈ {4,5} keeps both products with op2 = 000; op2 = 001 always multiplies:
op2 bit 9 enables the products in the W modes) [measured, `re/dspmacfit.py`]
op1 ∈ {0,1}: no write
D ≥ 8: r(D−8) ← result / 2                                   [measured 2026-10-16, random operands]
op2 = 010: c = imm16 (modes of c: 00 rC, 01 imm16, 10 imm24, 11 imm32); op2 = 011: unknown (constant result)
```
An A op with an immediate inside a MAC class (0x088) reads the MAC's N word as its immediate (measured): do not
give it a constant of its own.

Measured examples:

| instruction | computes |
|---|---|
| `0803 0c0a 9281` | r0 ← r1 + r2·(r0 − r1) (linear interpolation) |
| `0803 0809 5280` | r0 ← r0·r1 + r2·r5 |
| `0807 3cca 1cb6 002e 3bc7 01a0 1a01` | r3 ← r3·r2 + (2/2835)·r1 + 2/315 |
| `1107 7522 6504 0025 800d 0002 2222` | r7 ← r4·(−1/5040) + 1/120 ; r6 ← r4·r4 (A op) |
| `2b09 2c8f 6fe2 7fe4 5983 0407 0403 a2aa aa00` | r2 ← r2·r7 + r3 − r5/6 ; r6 ← [0407] ; r7 ← [0403] |

## 8. Predication

A C word enables/disables an A/B op of its instruction according to one of 16 **condition bits** (§5.4). The
bits are **flags**, 4 groups of 4, written by `X9bY` transfers **[measured 2026-10-17, master DSP3 main and DSP4
`efdc`, `re/dspx9bflags.py`]**:

```
X9bY   (transfer word, mode byte 9b): group g = X − 8 (X = 9, a, b → g = 1, 2, 3), Y = unit (1-based, ops first)
       flags of group g ← flags of the result of unit Y of the SAME instruction
cond 4g+0 = !Z        (result ≠ 0)
cond 4g+1 = !N        (result ≥ 0)
cond 4g+2 = !V        (|result| < 512, no overflow)
cond 4g+3 = N or Z    (result ≤ 0)
```

* Z, N for an A op: of the written result (r0 = r2 + r2 with r2 = 256 → 512 wraps to 0 → Z and V). A results
  **wrap**: magnitude mod 512, sign kept (300 + 300 → 88, −300·2 → −88, 511·2 → 510) **[measured]**.
* Unit B min/max (measured `b4c0` = max(r3, r0)): N = sign of the result, but **Z = the two operands are equal**
  (max(−0.25, 0) = 0 gives "≠ 0"; max(−0.5, −0.5) gives `0011` = Z and N).
* Unit B nibble e with D ≥ 8 and bits 7–6 = 00 (`8e09`, `be0c`, `ae10`…) is a **compare**: flags of r(d) + Y,
  no register written (`be0c` = r3 − r4, `be04` = r3 + r4, `9e0c` = r1 − r4) — the usual partner of `X9bY`.
* Units: Y = Y-th op of the instruction in position order (A, B, M and jumps count): Y = 1, 2 (class 0x328
  `6505 7c0f be0c 99b1 b9b2`), 3 (`b48b 06c7 1505 ae0e b9b3 …`) and 4 (`904a 0cc0 0cca 6062 be0f 1a12 b9b4 …`,
  after a MAC and two A ops) measured.
* MAC results set flags like A results (`8f09 4448 5448 99b1 a9b2`: two MACs → groups 1 and 2); SFU too (`8f39`:
  flags of the written r(d)); min like max (Z = operands equal).
* **Confirmed on the slaves** by audio (DCO2 slot, 2026-10-17): a SAW whose wrap is `cmp r1 − 1` + `b9b1` + a C
  select (or a conditional jump) is identical to the min/max reference.
* **Latency: one instruction** (as loads): a C word in the instruction right after the `X9bY` still reads the old
  flags; from the second instruction on it reads the new ones. Roland always leaves one instruction in between.
* **Persistence**: flags are not reset between samples, programs or test sessions; a group keeps its value until
  the next `X9bY` on it. The "static context bits" measured before (DSP3 `0111111011101110`, DSP4 `efdc`
  `0111101111100111`, slaves DCO2 `0011011001100110`) were simply the last flags left by Roland's code.
* **Groups 4 and 5** (2026-10-17): the C condition field is 5 bits; **X = c → group 4 (conds 16–19), X = d →
  group 5 (conds 20–23)**, same 4-flag semantics (master DSP3, and slaves by audio: `c9b1` + cond 17, `d9b1` +
  cond 21 switch the level of a SAW like the `b9b1` + cond 13 control). Roland: `c9b2` → C `2310` (!cond 17),
  `d9b1` → C `2a10` (cond 21). So **5 writable groups (X = 9…d → groups 1…5)**.
* **Constant groups**: group 0 (conds 0–3) reads `0111` (the flags of a zero: cond 1/2/3 true, cond 0 false);
  conds 24–31 read `1111` (always true). X = 8, e, f: no effect. Usable as constants: cond 1 = always, !cond 1 =
  never; cond 24 = always.
* The "C words" with cc ≥ 24 that some measured templates show (`3000`, `3921`, `4826`, `c0xx`…) are immediates
  or references mislabelled by the template guess, not conditions.
* Old tests missed this because they used `1803 0040 b9bP` (unit 1 = `r0 = r0` with r0 = 0) and read groups that
  did not match X.

Consequences for a compiler: **data-dependent control is possible**: compute (or `cmp`) a value, `X9bY` it into a
group, one instruction of distance, then a C word picks which op of an instruction runs (e.g. `x12`: skip op 1 if
true, op 2 if false) or whether a **jump** (§5.4b) is taken: if/else, switch, and loops (backward jumps).

## 9. Transfers and memory

* Load: `rR ← mem[slot]`; store: `mem[slot] ← r(R−8)` with mode `fe` **[exact]**.
* Up to 6 transfers per instruction (classes 0x060…0x722) **[exact]**.
* **Read ports per bank [measured 2026-10-17]**: bank 0 serves **2 loads** per instruction (with 3 or more only
  the last one is done); bank 5 serves **1** (with 2 or more none is done); stores are not limited by this
  (2 stores + 2 loads work). Roland's code respects it (0x47a: 2 loads from bank 0 + 2 from bank 6).
* An op and a load writing the same register in one instruction: the values are **added** (A `r7 = 0.5` + load
  0.375 → 0.875; measured again 2026-10-17). Avoid.
* **Ring addressing [measured]**: a reference R addresses physical word (R + ptr) mod ring, and ptr moves by one
  word every sample. Within one sample, a value stored at R is read back at R. **A value stored at R is read at R + 1
  in the next sample** (R + k after k samples). This is how state and delay lines work: to keep a variable from one
  sample to the next, load it from S + 1 and store it to S. Loading from S reads the value stored at S − 1 one sample
  ago (on master DSP3, a counter loaded from S and stored to S did not accumulate; loaded from S + 1 it counted
  96 000 times per second). Verified on master DSP3 and by the oscillator on the slave DCO slots.
* Consequence for observation: the ARM sees physical words; a value stored every sample leaves a trail over the
  whole bank (2048 words in < 0.4 s).
* Persistence per bank (master DSP3, counter loaded from W and stored to W) **[measured]**: bank 0 ring (load S+1);
  **bank 2 (`4nnn`) = two pages alternating every sample** (a store is read back in the same sample and 2 samples
  later, not at W+1); bank 4 (`80nn`, M zone) does not keep values to the next sample; bank 5 (`Annn`) plain
  persistent memory; bank 3 (`60nn`) not read back. The ARM reads bank 5 coherently (DPC*n* RAW view), which makes
  it the best place for test outputs.
* Bank 0 can be read by the ARM (RAW view) but ARM writes to bank 0 do not stay; bank 5 is ARM-writable
  **[measured]**.

### 9.0 Hidden registers and indexed transfers **[measured, master DSP3 / DSP4, 2026-10-16]**

* **`40TY` in a C position** (classes 0x058, 0x0E8, …) loads a hidden register: T = bits 7–6 (0 → h4, 1 → h5,
  2 → h6, 3 → h7 (unread)), mode = bits 5–4 (0: rY, kept mod 512; 1: imm16; 2: imm24; 3: imm32 reads 0), Y = bits
  3–0 = register, or for immediates the number of words of the instruction's immediate stream to skip. Immediate
  values are integers = value·16384 (addresses): `0b03 4050 0301` → h5 = 0x301; `1d06 4022 7fe0 c1fc 5100 0000` →
  h4 = 0x510000/16384 = 324; `0b02 4080` → h6 = r0.
* h4 = delay base (`f6c`, §6.4) and the value A op 0 sub 4 adds; h5 = tap-result address (§9.1), added by A op 0
  sub 5. Defaults on master DSP4: h4 = 288, h5 = 0x105/16384.
* **Indexed transfer, mode byte `28`**: `X28Y`, X 0–7 → rX ← bank0[⌊h6·16384⌋ + Y]; X 8–15 → bank0[…] ← r(X−8).
  Truncated, mod 0x800, ring-relative like a normal reference; a reference word is not added; several per
  instruction (`1e04 0281 1282`). Usage (S-100): `r0 ← param`, `0b02 4080`, `0c02 0280`. Lets a program read tables
  from memory (wavetables) instead of immediates.

* Indexed addresses are **ring-relative** (a counter kept with load [h6+1] / store [h6] counts every sample), so
  `[S + k]` reads what was stored at S k samples ago: a variable delay of < 2048 samples on any DSP. A fixed table
  cannot live in bank 0.
* **MXM timing and semantics [measured 2026-10-17, `re/dspbussync.py`, `re/dspbusclk.py`, `re/dspmxm.py`,
  `re/dspmxsum.py`]**: latched once per sample (a reader sees one value for its whole program), latency exactly
  1 sample (checked with the common delay-position clock), last write in a sample wins (no summing), not kept
  (0 when nobody wrote that sample). DSP3 Z/S → DSP4 O; DSP4 O → DSP3 S. IOMD (`re/dspiomd.py`): I/O words with an
  owner DSP; non-owner writes vanish; owner writes persist; the master block holds live inputs. See
  `DSP_HARDWARE.md` §8.
* **Buses [measured, master]**: `[60nn]` = IOMD word nn (0x4113C000). MXM stores of DSP3: M → ARM window MXM2,
  O → MXM3, Z and S → MXM5; DSP4 loads DSP3's Z/S stores from its O zone (`83nn` ← DSP3 `85nn`/`87nn`, same nn).
  `X9bY` writes nothing visible in any of these windows.

### 9.1 Delay memory (bank-2 DMA) **[measured, master DSP4 slot 0xF7DC, 2026-10-16]**

Only where the ARM connected the task's bank-2 window to the delay memory (master DSP4 global-delay slot 0xF7DC,
window `4300`; nothing happens on master DSP3). Delay memory = a ring that advances one sample per sample;
addresses are in units of 16384 samples. Per sample:

| step | instruction | effect |
|---|---|---|
| write | `[4300] ← x` (any `fe` store) | x enters the delay ring at address 324 (fixed for this slot by the ARM) |
| base | `1d06 4022 7fe0 c1fc <imm32>` | f6c base ← raw imm >> 22 (`5100 0000` → 324). Other low bytes of `40xx`: unknown, avoid |
| address | `{8+d}f6c` | rd ← rd + base (§6.4) |
| destination | `0b03 4050 <a>` | the next request writes its results to bank 2 [4000 + a ..] (`0301` → `[4301..4303]`). **Without it the request is ignored** |
| request | `da90` (`0c02 3a90`, or `1b04 c510 3a90 0008`) | read at address rd |
| results | load `[a]`, `[a+1]`, `[a+2]` (same sample) | x[n − L − ⌊k⌋], x[n − L − ⌊k⌋ − 1], frac(k), with k = 16384·(rd − 324), L = 2–3 samples (program position) |

Interpolate with `0803 0c0a 9281`-style lerp: r0 + frac·(r1 − r0). With base = 324 the delay is 16384·u + L for
u = r before f6c (measured u = 0, 0.5, 1, 10, 50.648 → 3, 8196, 16388, 163844, 829826 samples; u = 100 out of the
line). Roland's global delay: `m1_tcm_f7dc_01134238`, two taps (`[4301..]`, `[4304..]`). Tool: `re/dsptap.py`,
`re/dsprdly.py`. A full ARM read of WMEM (0x41300000) with a test program in this slot hung the master once.

## 10. Execution semantics **[measured, master DSP3]**

| case | result |
|---|---|
| two ops in one instruction | both read old values (`1503 3c01 4cc1`: r4 uses old r3), independent of word order |
| A + B / A + A + B in one instruction (classes 0x0B0, 0x160) | all three read old values [exact, 2026-10-16]: `1603 1c42 a445` (A `r1 = r1 + r2`, B `r2 = max(r1, r5)`) → r1 = old r1 + old r2, r2 = max(old r1, r5); `2c06 1c47 36b0 a445 2800 0000` (A `r1 += r7`, A `r3 += r2·0.5`, B `r2 = max(r1, r5)`) exact. Used for one-instruction-per-point wavetables (`CUSTOM_OSC.md`) and verified by audio on the slaves |
| op + store in one instruction | the store writes the **old** register value |
| program → next program on the same DSP | registers carry over (no reset between the main and slot programs) |
| op + load in one instruction | the op reads the **old** value; the load result is visible to the next instruction |
| op and load writing the same register in one instruction | the register received the **sum** (seen once; avoid) |
| ALU op → next instruction | the result is visible (no latency) |
| `X9bY` → C word | **one instruction of latency** (like a load) [measured 2026-10-17] |
| A op overflow | magnitude wraps mod 512, sign kept; flag V set (§8) |
| **load → ALU op** | **one instruction of latency**: an op in the instruction right after the load still sees the old register; from the second instruction on it sees the loaded value (a no-op or any unrelated instruction in between is enough) |
| load → store of the loaded register | not characterised; keep one instruction in between |

## 11. Encoding examples

1. `r3 ← r0 + r1`: unit A, D=3, op=6, X=0, mY=0, Y=1 → `0011 110 000 00 0001` = `3c01`; template `A`
   (class 0x048), length 2 → header `0x048<<5 | 2` = `0902`. Instruction: `0902 3c01`.
2. `r1 ← r0 · 0.5`: D=1, op=2, X=0, mY=3 (imm32), Y=0 → `1430`; imm32 0.5 = `28000000` → 2 I words; length 4 →
   `0904 1430 2800 0000`.
3. `r1 ← 0.5` (imm16): op 0 sub 1: D=1, op=0, X=1, mY=1, Y=0 → `0001 000 001 01 0000` = `1050` → `0903 1050 2800`.
4. `r0 ← [0x0400]; [0x0401] ← r1`: template `TTRR` (0x0F0), length 5: T1 = `0fe0` (load r0, slot 0),
   T2 = `9fe2` (store r1, slot 1), R = `0400`, `0401` → `1e05 0fe0 9fe2 0400 0401`.
5. `r0 ← r1 + r2·(r0 − r1)`: MAC, template `MN` (0x040): w1 D=0 op1=6 X=0 mY=0 Y=r2 (corpus uses `0c0a`);
   w2 Z=−r1 (`9`), op2=001, C=r2, mW=0, W=r1 → `9281` → `0803 0c0a 9281`.
6. `r3 ← |r1| + r2` (unit B): template `B` (class 0x050), word D=3 op=6 X=1 Y=2 → `3c42` → `0a02 3c42`.
6b. Three ops in one instruction, template `AAB` (class 0x160): `r1 ← r1 + r7 | r3 ← r3 + r2·s | r2 ← max(r1, r5)`
   = A `1c47`, A op 3 imm32 `36b0`, B `a445`, one imm32 → `2c06 1c47 36b0 a445 <s hi> <s lo>` (`re/dspasm.py aab()`).

7. A complete program: `re/dspasm.py saw dco2` generates our own naive SAW for the DCO2 slot (pitch in `[2207]` /
   `[2214]`, `f = 440·2^(p − 4.75)`, fs 96 kHz, 2^x by Taylor + 4 squarings, phase kept in the ring (load S + 1,
   store S), wrap without branches: q = max(phase − 1, 0), four times q = min(256·q, 1), phase −= 2q). Installed
   in the C-SAW PLUG-OUT it is in tune within ±1.2 cents from note 36 to 96, harmonics 1/n. Full description:
   `CUSTOM_OSC.md`.

## 12. Template table (all classes of the corpus)

Measured with `re/dspslots.py` (+ `ab` pass for unit A vs B) and resolved with `re/dsptemplates.py` from each
class's most frequent length. Other lengths of a class differ in the number of trailing `I` words (verified for
the measured examples; [inferred] in general). Templates with a `C` inside the slot area, or several `C`, came
from the resolver's fallback and should be re-checked before an assembler emits them.

| class | header (len n) | measured template (len) | fixed part | lengths in corpus | uses | most frequent instruction |
|---|---|---|---|---|---|---|
| `0x0f0` | `1e05` | `TTRR` (5) | `TTRR` + I… | 5 | 2611 | `1e05 0fe0 1fe2 2200 2200` |
| `0x060` | `0c03` | `TR` (3) | `TR` + I… | 2, 3 | 2217 | `0c03 2fe0 020c` |
| `0x048` | `0902` | `A` (2) | `A` + I… | 2, 3, 4 | 1700 | `0902 4086` |
| `0x040` | `0803` | `MN` (3) | `MN` + I… | 3, 4, 5, 6, 7, 8 | 1537 | `0803 0809 5280` |
| `0x050` | `0a02` | `B` (2) | `B` + I… | 2, 3, 4 | 1066 | `0a02 ff70` |
| `0x0a0` | `1405` | `MTNR` (5) | `MTNR` + I… | 4, 5, 6, 7, 8, 9, 10 | 725 | `1404 0c4a 6a90 8280` |
| `0x0a8` | `1503` | `AA` (3) | `AA` + I… | 3, 4, 5, 6, 7 | 673 | `1503 2987 7505` |
| `0x088` | `1107` | `MANIII` (7) | `MAN` + I… | 4, 5, 6, 7, 8, 9 | 630 | `1107 7522 6504 0025 800d 0002 2222` |
| `0x0c0` | `1804` | `ATR` (4) | `ATR` + I… | 3, 4, 5, 6 | 617 | `1804 7dd0 99b1 0000` |
| `0x158` | `2b07` | `MTTNRR` (7) | `MTTNRR` + I… | 7, 8, 9, 10, 11 | 544 | `2b09 2c8f 6fe2 7fe4 5983 0407 0403 a2aa aa00` |
| `0x170` | `2e05` | `AATR` (5) | `AATR` + I… | 4, 5, 6, 7, 8, 9 | 398 | `2e04 1442 3d87 a9b2` |
| `0x1f0` | `3e07` | `TTTRRR` (7) | `TTTRRR` + I… | 7 | 388 | `3e07 7fe0 2fe2 3fe4 a025 c022 c024` |
| `0x270` | `4e0a` | `MATTNRRII` (10) | `MATTNRR` + I… | 10, 11, 12 | 341 | `4e0a 254f 34e6 afe2 6fe4 0002 040c 0439 3921 fb00` |
| `0x0d8` | `1b04` | `BTR` (4) | `BTR` + I… | 3, 4, 5, 6 | 315 | `1b04 ae10 b9b1 0000` |
| `0x1a0` | `3406` | `ATTRR` (6) | `ATTRR` + I… | 5, 6, 7, 8 | 312 | `3406 2c86 afe0 3fe2 0216 0218` |
| `0x0b8` | `1703` | `AC` (3) | `AC` + I… | 3, 4, 5 | 293 | `1704 1050 1810 3000` |
| `0x348` | `6908` | `ATTTRRR` (8) | `ATTTRRR` + I… | 7, 8, 9, 10 | 290 | `6908 3cd0 9fe2 bfe4 99b1 0000 0265 0267` |
| `0x2c0` | `5809` | `MTTTNRRR` (9) | `MTTTNRRR` + I… | 9, 10, 11, 12, 13 | 250 | `5809 048d 2fe2 1fe4 5fe6 0000 0207 0209 a025` |
| `0x188` | `3105` | `ABTR` (5) | `ABTR` + I… | 4, 5, 6, 7 | 235 | `3105 34c4 ee10 b9b2 c400` |
| `0x0b0` | `1603` | `AB` (3) | `AB` + I… | 3, 4, 5, 6, 7 | 229 | `1604 54d0 9ec7 7000` |
| `0x128` | `2508` | `MATNRII` (8) | `MATNR` + I… | 6, 7, 8, 9, 10, 11 | 215 | `2507 1c4f 2483 7fe4 f012 3000 a023` |
| `0x328` | `6507` | `ABTTRR` (7) | `ABTTRR` + I… | 5, 6, 7, 8, 9 | 215 | `6505 7c0f be0c 99b1 b9b2` |
| `0x2f8` | `5f07` | `AATTRR` (7) | `AATTRR` + I… | 5, 6, 7, 8, 9, 10, 11 | 214 | `5f05 7dc9 6c09 99b1 a9b2` |
| `0x168` | `2d06` | `AACII` (6) | `AAC` + I… | 4, 5, 6, 7, 8 | 172 | `2d06 1050 1052 0e12 3000 0000` |
| `0x2a0` | `5408` | `MBTTNRR` (8) | `MBTTNRR` + I… | 8, 9, 11 | 160 | `5408 6d89 8ec5 8fe2 5fe4 0280 022f 0230` |
| `0x3c0` | `7809` | `TTTTRRRR` (9) | `TTTTRRRR` + I… | 9 | 157 | `7809 afe0 bfe2 0fe4 1fe6 0210 0211 020e 020f` |
| `0x090` | `1206` | `MBNCI` (6) | `MBNC` + I… | 4, 5, 6, 7, 8, 9 | 143 | `1204 570d fde8 0005` |
| `0x1d0` | `3a06` | `BTTRR` (6) | `BTTRR` + I… | 5, 6, 7, 8 | 139 | `3a06 8e10 b9b1 0fe2 0000 0210` |
| `0x058` | `0b02` | `C` (2) | `C` + I… | 2, 3, 4 | 138 | `0b02 00f8` |
| `0x5c0` | `b809` | `AATTTRRR` (9) | `AATTTRRR` + I… | 9, 10, 11, 12, 13 | 122 | `b80a 2487 3ee4 afe0 bfe2 a9b2 0215 021c 0ccc cc00` |
| `0x238` | `470c` | `MMTTNNRRIII` (12) | `MMTTNNRR` + I… | 9, 11, 12, 14 | 116 | `4709 4794 57d6 99b1 a9b2 0000 0000 2800 2800` |
| `0x080` | `1005` | `MMNN` (5) | `MMNN` + I… | 5, 7, 8, 9, 11 | 95 | `1005 048e 14ce 0000 0001` |
| `0x160` | `2c07` | `AABIII` (7) | `AAB` + I… | 4, 5, 6, 7 | 80 | `2c07 1447 3490 ce72 7000 0000 0200` |
| `0x5c2` | `b84b` | `AATTTTRRRR` (11) | `AATTTTRRRR` + I… | 11, 12, 14 | 75 | `b84b 1447 2507 afe0 bfe2 6fe4 7fe6 0206 0207 c025 c022` |
| `0x5b8` | `b70a` | `AACTTRRII` (10) | `AACTTRR` + I… | 8, 9, 10, 11 | 71 | `b708 6dd0 6dd2 1312 99b1 99b2 3000 b000` |
| `0x140` | `2808` | `MBTNRCI` (8) | `MBTNRC` + I… | 6, 8, 9, 10, 11, 12 | 70 | `2808 25b4 bedb 7fe2 0002 0438 2d21 8015` |
| `0x4e0` | `9c0c` | `MATTTNRRRII` (12) | `MATTTNRRR` + I… | 10, 12, 13, 14 | 69 | `9c0e 38d2 7cbc 9fe6 4fe8 5fea 4500 5000 5000 021a 0216 0217 0000 0002` |
| `0x118` | `2307` | `MABNII` (7) | `MABN` + I… | 6, 7, 8 | 64 | `2307 4e8e 3e80 ee62 0382 3000 0000` |
| `0x258` | `4b0b` | `MABTNRIIII` (11) | `MABTNR` + I… | 8, 9, 10, 11, 12 | 56 | `4b0a 65f5 75c5 8e22 b9b3 0006 ca00 00d3 d254 1f00` |
| `0x2f0` | `5e08` | `AACTRII` (8) | `AACTR` + I… | 6, 7, 8 | 45 | `5e08 7dd0 34d2 1a10 4fe4 4200 7000 0402` |
| `0x198` | `3305` | `ACTR` (5) | `ACTR` + I… | 5, 6 | 44 | `3305 3040 1b10 7fe0 a024` |
| `0x2e0` | `5c08` | `AABTRCI` (8) | `AABTRC` + I… | 5, 6, 7, 8, 9, 10 | 44 | `5c06 6d40 3cc0 ae10 b9b3 a400` |
| `0x4c0` | `9809` | `MABTTNRR` (9) | `MABTTNRR` + I… | 9, 10, 11, 12, 15 | 44 | `980f 2976 7c3e beeb 9fe2 6fe4 1e80 0429 042b 0000 0000 049b a5e3 0000 0000` |
| `0x558` | `ab0b` | `MTTTTNRRRR` (11) | `MTTTTNRRRR` + I… | 11, 13, 14 | 42 | `ab0b 2c0b bfe2 3fe4 7fe6 4fe8 a2c2 0217 021b c023 c025` |
| `0x340` | `6807` | `ACTTRR` (7) | `ACTTRR` + I… | 7, 9 | 37 | `6809 5064 1f13 dfe0 5fe2 0406 0410 3b33 3300` |
| `0x638` | `c70c` | `ATTTTRRRRII` (12) | `ATTTTRRRR` + I… | 9, 10, 11, 12 | 36 | `c709 3585 dfe0 99b1 0fe2 1fe4 0256 0254 c035` |
| `0x5a0` | `b40b` | `AABTTRRIII` (11) | `AABTTRR` + I… | 8, 9, 11 | 36 | `b40b 4424 5427 fe41 8fe0 2fe2 0250 0251 43a3 2930 6396` |
| `0x398` | `7307` | `BTTTRR` (7) | `BTTTRR` + I… | 7, 8, 10 | 35 | `7307 8e09 8fe0 b9b1 4fe2 0415 0411` |
| `0x180` | `3006` | `ABCII` (6) | `ABC` + I… | 5, 6 | 32 | `3006 0050 8012 1812 b000 0000` |
| `0x598` | `b307` | `AABCTR` (7) | `AABCTR` + I… | 7, 9 | 32 | `b307 4d08 3ed0 8e41 0930 99b1 3000` |
| `0x610` | `c209` | `ABTTTRRR` (9) | `ABTTTRRR` + I… | 7, 9, 11 | 30 | `c20b 1c47 8426 7fe0 afe2 2fe4 021c 022c a02b c400 0000` |
| `0x1e8` | `3d07` | `CTTRRC` (7) | `CTTRRC` + I… | 5, 7 | 30 | `3d07 4050 0fe2 7fe4 0105 4101 0422` |
| `0x530` | `a60d` | `MBTTTNRRRCII` (13) | `MBTTTNRRRC` + I… | 12, 13 | 29 | `a60c 2538 eee6 1fe2 4fe4 dfe6 0002 0415 0416 0410 0000 0000` |
| `0x55a` | `ab4d` | `MTTTTTNRRRRR` (13) | `MTTTTTNRRRRR` + I… | 13, 15 | 29 | `ab4d 14cd bfe2 2fe4 3fe6 6fe8 7fea 0001 020d 0206 0207 c027 c028` |
| `0x5b0` | `b609` | `AACCTRII` (9) | `AACCTR` + I… | 7, 9 | 28 | `b607 3044 3045 1220 0e13 5fe0 040f` |
| `0x190` | `3204` | `ACC` (4) | `ACC` + I… | 4, 5, 6 | 26 | `3204 3044 1b10 1220` |
| `0x100` | `200a` | `MMBNNIIII` (10) | `MMBNN` + I… | 10 | 25 | `200a 44a6 54e9 e5d4 0004 0005 0000 2c4f 762f bd2f` |
| `0x478` | `8f0b` | `MMTTTNNRRR` (11) | `MMTTTNNRRR` + I… | 9, 11, 14 | 25 | `8f0e 04aa 14ed 2fe4 3fe6 7fe8 0000 0001 0608 0619 c1fc 2fe2 9d2f e29d` |
| `0x5f0` | `be08` | `ABBTTRR` (8) | `ABBTTRR` + I… | 7, 8, 10 | 24 | `be07 3503 de0e 9f2b dfe0 b9b2 0417` |
| `0x5a4` | `b48a` | `AABTTTTRR` (10) | `AABTTTTRR` + I… | 10, 11, 16 | 22 | `b48a 6d49 3cc0 ce0b 9fe0 c9b1 b9b3 5fe2 043b 0433` |
| `0x0e8` | `1d04` | `CTR` (4) | `CTR` + I… | 3, 4, 5, 6 | 21 | `1d04 0910 0fe0 0248` |
| `0x110` | `2207` | `MMTNNR` (7) | `MMTNNR` + I… | 7, 8, 10, 11 | 21 | `2207 154e 350f 7fe4 0001 0003 c036` |
| `0x568` | `ad08` | `AAACCII` (8) | `AAACC` + I… | 8, 9 | 20 | `ad08 7040 7050 3052 0e12 1f30 0000 0000` |
| `0x2c8` | `5909` | `AAACIIII` (9) | `AAAC` + I… | 8, 9 | 20 | `5909 1c43 1c70 65b4 0b12 0000 0000 3000 0000` |
| `0x220` | `440b` | `MMBTNNRIII` (11) | `MMBTNNR` + I… | 10, 11, 12 | 20 | `440b 6566 054f fe69 dfe4 0003 0000 0232 0141 b230 0000` |
| `0x6c8` | `d908` | `BBTTTRR` (8) | `BBTTTRR` + I… | 8 | 20 | `d908 ae10 8ee0 ffe2 b9b1 c9b2 0000 0252` |
| `0x098` | `1307` | `MNIIII` (7) | `MN` + I… | 4, 7 | 19 | `1307 2c22 0b10 9941 022a 1c02 2a1c` |
| `0x0e0` | `1c04` | `CCC` (4) | `CCC` + I… | 3, 4 | 16 | `1c04 0090 0810 0009` |
| `0x2e8` | `5d05` | `AACC` (5) | `AACC` + I… | 5, 7 | 16 | `5d05 3cc4 45c7 0f10 1e30` |
| `0x720` | `e40b` | `TTTTTRRRRR` (11) | `TTTTTRRRRR` + I… | 11 | 15 | `e40b 8fe0 9fe2 1fe4 2fe6 7fe8 0600 0601 0603 0604 c1f7` |
| `0x4b8` | `970b` | `MABCTNRIII` (11) | `MABCTNR` + I… | 11, 12 | 14 | `970b 2476 75c6 be52 1830 4fe4 0002 b000 0413 0000 0000` |
| `0x268` | `4d09` | `MACTNRII` (9) | `MACTNR` + I… | 7, 9 | 12 | `4d09 54cd 1c64 1a20 2fe2 0004 0407 4200 0000` |
| `0x584` | `b08c` | `AABBCTTRRCI` (12) | `AABBCTTRRC` + I… | 12 | 12 | `b08c 24b4 3cc0 be0d ee50 1864 b9b3 6fe2 3000 0404 3000 0000` |
| `0x59c` | `b38e` | `AABCTTTRRRCCI` (14) | `AABCTTTRRRCC` + I… | 14 | 12 | `b38e 4537 3064 ee0f 1820 dfe0 efe2 b9b3 0402 040b b000 002f ae14 7a00` |
| `0x4e2` | `9c4c` | `MATTTTNRRRR` (12) | `MATTTTNRRRR` + I… | 12, 14 | 10 | `9c4c 648f 1440 dfe2 0fe4 7fe6 2fe8 0006 0226 c028 a032 021a` |
| `0x208` | `410b` | `MMANNCCCII` (11) | `MMANNCCC` + I… | 11 | 10 | `410b 4566 4569 1454 1a12 0006 0006 3000 0009 6e00 2654` |
| `0x458` | `8b0a` | `MMBTTNNRR` (10) | `MMBTTNNRR` + I… | 10 | 9 | `8b0a 4654 5696 8f2b 99b1 a9b2 0003 0003 2800 2800` |
| `0x378` | `6f06` | `BBTTR` (6) | `BBTTR` + I… | 6, 7 | 9 | `6f06 ae10 8ee0 b9b1 c9b2 0000` |
| `0x570` | `ae09` | `AAACTRII` (9) | `AAACTR` + I… | 8, 9, 11 | 9 | `ae0b 4072 4043 24b6 1812 ffe0 0412 0000 0001 a2b8 51eb` |
| `0x2d8` | `5b08` | `AABIIII` (8) | `AAB` + I… | 5, 8 | 9 | `5b08 6050 4d32 bedb 1b10 0000 0978 d4fd` |
| `0x5a2` | `b44a` | `AABTTTRRR` (10) | `AABTTTRRR` + I… | 10, 11 | 9 | `b44a 4d10 75c7 ee12 9fe4 b9b3 99b1 a800 a800 0459` |
| `0x592` | `b247` | `AATCCT` (7) | `AATCCT` + I… | 7 | 8 | `b247 3044 3045 ee0f 1220 0e14 b9b3` |
| `0x490` | `920c` | `MAACTNRIIII` (12) | `MAACTNR` + I… | 10, 12 | 6 | `920c 75e6 5043 5052 1a23 ffe4 0029 3000 0231 4500 00c4 0000` |
| `0x5c4` | `b88d` | `AATTTTTRRRRR` (13) | `AATTTTTRRRRR` + I… | 13 | 6 | `b88d 0487 14c7 8fe0 9fe2 2fe4 3fe6 7fe8 060f 0620 060e 061f c1fd` |
| `0x722` | `e44d` | `TTTTTTRRRRRR` (13) | `TTTTTTRRRRRR` + I… | 13 | 6 | `e44d 8fe0 8fe2 afe4 9fe6 2fe8 0fea 8400 a400 0204 0200 8614 60e1` |
| `0x4c2` | `984c` | `MABTTTNRRRI` (12) | `MABTTTNRRR` + I… | 12, 14 | 6 | `984c 3412 6d94 fe16 9fe8 a9b2 b9b3 0003 2400 0000 0000 0269` |
| `0x5f2` | `be49` | `ABBTTTRR` (9) | `ABBTTTRR` + I… | 9 | 5 | `be49 0c07 de0a eede b9b2 1fe0 8fe2 0238 0243` |
| `0x590` | `b208` | `AABCIII` (8) | `AABC` + I… | 8 | 5 | `b208 3c20 5044 9ee1 2620 1f40 2800 0000` |
| `0x574` | `ae8e` | `AAACTTTRRRIII` (14) | `AAACTTTRRR` + I… | 12, 14 | 5 | `ae8e 7de8 5043 5050 1a23 ffe2 0fe4 2fe6 3000 0254 025a 0257 aa1d 2a00` |
| `0x130` | `260b` | `MBBNCCCCCI` (11) | `MBBNCCCCC` + I… | 11 | 4 | `260b 1472 aeda 803a 0036 1999 9999 2e66 6666 03fa 0945` |
| `0x512` | `a24d` | `MBBTTTNRRRII` (13) | `MBBTTTNRRR` + I… | 13 | 4 | `a24d 79a6 ae12 8ee0 b9b2 c9b3 dfe4 7a40 0000 022e 3000 0000 0000` |
| `0x572` | `ae4c` | `ATACTTRRRCI` (12) | `ATACTTRRRC` + I… | 12 | 4 | `ae4c 7de6 5043 5050 1a23 ffe2 1fe4 3000 0251 0256 aa1d 2a00` |
| `0x600` | `c007` | `ABCCTR` (7) | `ABCCTR` + I… | 7 | 4 | `c007 6044 9ee1 2310 1a30 dfe0 024b` |
| `0x63a` | `c74c` | `ATTTTTRRRRR` (12) | `ATTTTTRRRRR` + I… | 12 | 4 | `c74c 0406 8fe0 4fe2 5fe4 6fe6 7fe8 0264 024d 024e c039 c03a` |
| `0x318` | `6305` | `ABII` (5) | `AB` + I… | 5 | 4 | `6305 6044 9ee1 2310 1a30` |
| `0x2d0` | `5a09` | `AABBIIII` (9) | `AABB` + I… | 9 | 4 | `5a09 0422 7890 b525 ce45 3000 2800 0029 9999` |
| `0x178` | `2f08` | `ABBCCCI` (8) | `ABBCCC` + I… | 8 | 4 | `2f08 1470 aeda 8034 3000 0000 03fa 0945` |
| `0x1b8` | `3705` | `BBTR` (5) | `BBTR` + I… | 5 | 4 | `3705 ce10 aed1 b9b1 a400` |
| `0x612` | `c24c` | `ABTTTTRRRRI` (12) | `ABTTTTRRRR` + I… | 12 | 4 | `c24c 24b6 be0d b9b2 afe0 1fe2 4fe4 0411 0412 0413 3000 0000` |
| `0x4b4` | `9692` | `MABCCTTNRRIIIIIII` (18) | `MABCCTTNRR` + I… | 18 | 4 | `9692 2c7b 5052 ee68 1f25 1f36 5fe4 6fe6 4fc2 3000 0406 0410 3b33 3300 0000 0000 0000 0000` |
| `0x108` | `2109` | `MMNNIIII` (9) | `MMNN` + I… | 9 | 4 | `2109 140b 140b 0b12 0024 0027 2780 00a7 8000` |
| `0x240` | `4809` | `MAANCCCI` (9) | `MAANCCC` + I… | 8, 9 | 4 | `4809 1862 3040 3041 0b23 2940 0000 0000 0000` |
| `0x6e8` | `dd07` | `BCTTTR` (7) | `BCTTTR` + I… | 7 | 4 | `dd07 de0e 1830 b9b1 99bc 1fe0 062c` |
| `0x120` | `2405` | `MANI` (5) | `MAN` + I… | 5, 9 | 3 | `2405 0cc0 0cca 1a12 a007` |
| `0x59a` | `b34c` | `AABCTTRRCCI` (12) | `AABCTTRRCC` + I… | 12 | 2 | `b34c 4076 4050 ae12 1812 b9b3 7fe4 0000 d000 c007 0800 0000` |
| `0x60c` | `c18e` | `ABCTTTTRRRRCI` (14) | `ABCTTTTRRRRC` + I… | 14 | 2 | `c18e 4078 ae10 1810 efe2 b9b2 5fe4 6fe6 c400 042a 0406 c005 2200 0000` |
| `0x4ba` | `974e` | `MABCTTNRRCCCI` (14) | `MABCTTNRRCCC` + I… | 14 | 2 | `974e 5566 4079 ae12 1820 b9b3 6fe4 0006 c200 c006 8273 8124 0000 0000` |
| `0x310` | `6205` | `ABBT` (5) | `ABBT` + I… | 5 | 2 | `6205 3503 de0e 9f2b b9b2` |
| `0x388` | `7106` | `BCCTR` (6) | `BCCTR` + I… | 6 | 2 | `7106 fe08 0090 1820 b9b1 0080` |
| `0x1d8` | `3b06` | `CCCCC` (6) | `CCCCC` + I… | 6 | 2 | `3b06 0090 0092 1812 0001 00cd` |
| `0x482` | `904a` | `MAABCTNRI` (10) | `MAABCTNR` + I… | 10 | 2 | `904a 0cc0 0cca 6062 be0f 1a12 b9b4 a005 0008 0000` |
| `0x3b8` | `770a` | `CTTTRRRCC` (10) | `CTTTRRRCC` + I… | 10 | 2 | `770a 40a6 1fe0 3fe2 4fe4 0622 4001 4002 0000 0000` |
| `0x2b8` | `5709` | `MCTTNRRC` (9) | `MCTTNRRC` + I… | 9 | 2 | `5709 788d 4012 0fe4 2fe6 3300 0000 063f 0640` |
| `0x588` | `b10b` | `AABBTRCCII` (11) | `AABBTRCC` + I… | 11 | 1 | `b10b 2c03 4062 1075 eef6 3fe0 0625 0600 005f ffff ff00` |
| `0x566` | `acce` | `AAABCTTTRRRCI` (14) | `AAABCTTTRRRC` + I… | 14 | 1 | `acce 75e6 0042 0c8c be10 1a23 b9b4 efe2 3fe4 0000 0603 0605 0c86 0700` |
| `0x5a6` | `b4cc` | `AABTTTTTRRR` (12) | `AABTTTTTRRR` + I… | 12 | 1 | `b4cc 6d83 2d4a 8ec1 ffe0 99b1 a9b2 3fe2 1fe4 0223 0228 0224` |
| `0x320` | `6407` | `ABCTRI` (7) | `ABCTR` + I… | 7 | 1 | `6407 5050 ded9 0a12 3fe2 0000 a021` |
| `0x0c8` | `1904` | `BBI` (4) | `BB` + I… | 4 | 1 | `1904 ef75 8410 0000` |


## 13. Method (how to verify anything here)

`re/dsppeek.py` (PLUG-OUT region 0 built with `-DPEEK`: MIDI pitch bend + CC1 in, SysEx out) lets the PC write
any master memory and read it back. `re/dspmaster.py` loads a test program into a context (`DSPCTX=dsp3`: the idle
master DSP3 main program, default; `efdc`: master DSP4 slot 0xEFDC, shared with the global effects — writing its
data memory once silenced the whole instrument until a power cycle), the original is saved and restored
(`dspmaster.py --restore` repairs an interrupted run). `re/dspprobe.py` sets r0..r7, runs instruction lines and
stores r0..r7 to bank 0 [0780..0787], which is read back (majority of 7 reads). Because bank 0 is a ring (§9),
the stored values leave a trail and the ARM read returns the value stored at some recent sample: fine for constant
results, but measuring a value that changes per sample needs a coherent 16-word read (the ring rotates ~10 µs per
word) or a read-back through memory the program itself maintains. `re/dspfit.py`, `re/dspclass.py`, `re/dspnlfit.py` fit exact formulas over random operands;
`re/dspslots.py` maps template positions; `re/dsptemplates.py` builds §12. Only one tool may use the MIDI port at a
time.

## 13b. All Roland DSP programs (2026-10-17)

`re/dsp_corpus_all.py` extracts every program (descriptor tables; signature scan as a fallback) from the
SYSTEM-8 base engine (`base-engine/up/p1y01_{p0s,pl1m,pl1s,pl2m}`), the OS (`m1_tcm`, `m1_sdram`, `s_data`), all
nine SYSTEM-8 PLUG-OUTs (JUNO-60/106, JUPITER-4/8, JX-3P, PROMARS, SH-101, SH-2, SYSTEM-100: region 1; region 0
holds no DSP code) → **212 unique programs** (84 = the old corpus, 116 base engine, 12 from JUPITER-8 / PROMARS /
SH-2); `work/corpus_all` holds the 128 new ones. `re/dsp_s1.py`: the SYSTEM-1 images of SH-2 / PROMARS use the
same ISA (4 programs, start marker `ff71`, no descriptor table) → `work/corpus_s1`. The base engine adds 20 classes
not seen before; their templates were inferred from the instances (fe slots, `b9bY` units, C words, +2 per extra
T between neighbouring classes) and are marked `inferred` in `re/dsp_templates.txt` (not measured on hardware).

**Templates measured (2026-10-17)**: every inferred template was run on master DSP3 with Roland's own instance
(`re/dspverify.py --inferred`: references moved to scratch words of the same bank and preloaded, random
registers and flags, registers and written memory compared with `dspisa.run`); `re/dspslots.py` position maps
for 0x240, 0x248, 0x250, 0x478, 0x47a, 0x4aa, 0x4da. 19 of 20 agree in every trial (0x56a: only its out-of-range
C field, §5.4); the hand-fixed templates 0x090, 0x098, 0x130, 0x178, 0x208, 0x240, 0x572, 0x592 agree too.
Random regression over the whole corpus (100 instructions without jumps / special transfer modes, 2 random cases each): **100 / 100 agree** with `dspisa.py`.

Coverage (`python3 re/dspisa.py cov DIR`): old corpus 98.6 %, new programs 97.8 %, SYSTEM-1 98.6 %; **the only
words left are the section markers `ff70`/`ff71`/`ef75`/`ef74`** (no visible effect).

## 14. Open items (do not emit)

Clean, current list: `DSP_HARDWARE.md` §11 (2026-10-17). Everything decoded in the corpus of all Roland programs
(212 SYSTEM-8 + 4 SYSTEM-1) except the section markers, which have no effect (§6.4). Timing: every instruction
costs one cycle whatever it contains (1 or 2 MACs, up to 6 transfers; `re/dspbudget.py mac|mac2|xfer4|xfer6`),
a taken jump one more.
