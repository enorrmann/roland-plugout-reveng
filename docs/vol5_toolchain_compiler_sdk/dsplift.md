# Volume V — Chapter 8: `dsplift` — DSP Microcode to Plain C

> **Scope:** both (any program of the DSP ISA: SYSTEM-8, SYSTEM-1, and the Boutique `BQ2` firmware of the TB-03).

Specification. State: 2026-11-11, version 1.1.

`sdk/s1cc/tools/dsplift.py` translates a DSP program into one C function that does what the emulator's C core
(`examples/from_scratch/emu/dspcore.h`, the reference) does with it. It is the reverse direction of s1cc
([ch. 1](s1cc_compiler_internals.md)): s1cc goes from C to microcode, dsplift from microcode to C. Its use is
**porting**: an existing program (a Roland PLUG-OUT voice, a Boutique engine) is lifted, read, and rewritten by hand
as an s1cc kernel; the lifted C stays as the native oracle the rewrite is compared against.

Tags: **[sim]** measured against the emulator; **[A]** open.

## 1. Use

```sh
python3 sdk/s1cc/tools/dsplift.py OUT.c NAME=PROGRAM.hex [NAME=PROGRAM.hex ...]
python3 sdk/s1cc/tools/dsplift.py OUT.c NAME=PROGRAM.hex --check [--samples 64] [--seed 1]
python3 sdk/s1cc/tools/dsplift.py OUT.c --bq2 NAME=PROGRAM.hex@ADDRESS [...]   # Boutique BQ2 DSP (§5)
python3 sdk/s1cc/tools/dsplift.py OUT.h --kernel [--bq2] [--split K ...] NAME=PROGRAM.hex[@ADDRESS]   # s1cc C (§6)
cc -O2 -ffp-contract=off -I sdk/s1cc/tools -c OUT.c          # the lifted C compiles alone with its runtime
```

| argument | meaning |
|---|---|
| `OUT.c` | the C file to write: `#include "dsplift_rt.h"` and one `static void NAME(LiftM *M)` per program |
| `NAME=PROGRAM.hex` | the C function name and the program: its 16-bit words in execution order, hexadecimal, separated by blanks (the format of `work/corpus_all/*.txt`) |
| `--skip-lead` | skip a leading run of `0000` / `0001` words (the data words in front of the programs of a PLUG-OUT or OS program set, as `system-1/s1_progs.py words()`) |
| `--check` | compare the lift with the emulator (§3); not with `--bq2` (the emulator has no BQ2 model) |
| `--bq2` | the Boutique `BQ2` DSP (§5): implicit flags, control words, calls between programs. Each program needs its byte address in program memory: `NAME=PROGRAM.hex@0x0052` |
| `--kernel` | write s1cc-ready C instead of the machine-mode lift (§6) |
| `--split K`, `--zero-cells X,…`, `--export-base N`, `--import-ranges FILE` | `--kernel`: the program in two parts, DSP A and DSP B (§6.3) |
| `--overflow-wrap K,…` | `--kernel`: the instructions whose A-unit result reaches ±512 and must wrap as on the DSP (§6.2) |
| `--samples N`, `--seed S` | samples per program in `--check` (64) and the random seed (1) |

The words after the last instruction (a header of length 0: `0000` padding) are left out, with a note on stderr.

## 2. What the C looks like

One block per instruction, with its disassembly (`asm/disasm.py`) and its words as a comment:

```c
    /* 41: mac_p1 r3, r1, #256@2:16, #1.192e-07@8:32 | addxy r7, r1, #5e-10@12:32 | ld r4, [002a]@2 | ... */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0;
        { RESET(); n[3] = (0x1p+8 * r[1]) + (0x1p-23); QUANT(n); DONE(1, 3); }
        { RESET(); n[7] = AWRAP(r[1] + 0x1.12e0be826d695p-31); QUANT(n); DONE(2, 7); }
        { n[4] = RD(M, 0x002a); }
        ...
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
```

| element | rule (as `dspcore.c exec_insn`) |
|---|---|
| `r[8]`, `n[8]` | every unit reads `r` (the registers at the start of the instruction) and writes `n`; `n` becomes `r` at the end. A load writes `n`: the value is there in the next instruction |
| conditions | `sk` = the units a condition skips (`cond`): one test of `M->F` per condition word |
| flags | an X9bY transfer (`SETFLAGS`) takes the result of a unit of this instruction; the bits reach `M->F` at the end of the **next** instruction (`PM` / `PV`: one instruction of latency) |
| units | A (`add3`, `mul`, `mad`, …, `newton`, `sum8`, with the ±512 wrap `AWRAP` and its flags), B (`min` / `max` with their flags, `cmp`, `bic`, phase ops, shifts, bit ops, `frexp`, `divseed`, `f4` / `f6` / `f5c`), SFU (`recipseed`, `expsplit`, `sqrtsplit`, `log2split`, `atanhsplit`, `sign`, `floor`, `ceil`) and the MAC, each written out with its fields decoded (the immediates as exact hexadecimal literals) |
| jumps | `009Y` → `goto L<k>` |
| hidden registers | `M->H[4..7]` (`hset`, `f6`, the indexed transfers) |
| memory | `RD(M, a)` / `WR(M, a, v)` of the runtime (§4) |

## 3. Verification (`--check`) [sim, 2026-11-10]

`--check` builds the lifted C with `-DDSPLIFT_FLAT -DDSPLIFT_QUANT` (machine mode: flat memory as `dspref.Machine`,
RAW quantisation after every unit) and runs, for each program, the emulator and the lift side by side: the same
start state (`dspref.Machine` registers, condition bits, hidden registers), and before every sample random values in
every word the program loads (0, 1 or uniform −1…1; bank 0 only before the first sample: after it, it is the
program's own state). It prints the largest difference of the registers and of the 65 536 memory words.

| programs | result |
|---|---|
| `work/corpus_all/` (128 Roland programs: SYSTEM-8 / SYSTEM-1 PLUG-OUTs, dumps), 16 samples each | 128 / 128 equal to the emulator (registers and memory, max difference 0) |
| TB-03 voice program (`BQ2:Appli` 1.07, 751 instructions), 32 samples | registers equal; memory within 2.3·10⁻¹⁰ (the MAC, below) |

The only expected difference is the MAC: the emulator first computes it exactly on the RAW grid with 128-bit
integers (`mac_exact`); the lift uses the double path, which differs by one RAW LSB in about one MAC in a thousand.
A program the emulator cannot run (an undecodable instruction) is reported as not compared.

Test: `python3 -m unittest sdk/s1cc/tests/test_dsplift.py` (five corpus programs, the ones that exposed lifter
bugs: tap requests, a loop with a conditional jump; skipped without `work/corpus_all`).

## 4. Runtime (`dsplift_rt.h`)

| part | content |
|---|---|
| `LiftM` | `r[8]`, `F` / `PM` / `PV` (condition bits and the pending ones), `H[8]`, `t` (sample counter), the memory, `onwrite` (optional callback that sees every store: a trace of the state words) |
| memory, default | banks: bank 0 a ring of 2048 words (`(a − t) & 0x7FF`: a word stored at X in one sample is read at X + 1 in the next; the caller increments `t` once per sample), bank 1 (2048), bank 2 (two pages by sample parity), IOMD, bank 5 (2048), bank 6 (parameters), bank 7 (`E000`…, plain words: on the BQ2 the programs read `E000` / `E021`, §5) |
| memory, `-DDSPLIFT_FLAT` | the emulator's machine mode: one array of 65 536 words, bank 5 folded to 1024 words, bank 0 not a ring |
| `-DDSPLIFT_QUANT` | the RAW quantisation after every A / B / M unit (as the emulator); without it, plain doubles (what a port is compared with) |

Not modelled: the delay line (a tap request `Xa9Y` writes 0, 0, 0 to bank 2 at `[h5 … h5 + 2]`, as the emulator
without an EDM; warning), the bank-4 MXM (reads 0), the pair window and bank-1 sharing between DSPs. Build with `-ffp-contract=off` (an FMA changes the bits).

## 5. The Boutique `BQ2` DSP (`--bq2`) [L, static; not measured]

The Boutique TB-03 firmware (`BQ2_UPD.BIN` 1.07, module `BQ2:Appli`) carries its DSP programs as the SYSTEM-1 does:
`{address, bytes, pointer}` program sets in the ARM image (halfwords upper first, as §3 of
[Vol. II ch. 3](../vol2_dsp_core_isa/instruction_word_encoding.md)). Its DSP is a newer member of the family: the
instruction set decodes, with three differences the emulator does not model. `--bq2` implements the first two as
read from Roland's code; nothing of this is measured on a device (there is no BQ2 device in the project).

### 5.1 Implicit flags

The TB-03 programs test conditions 4…19 without a single `X9bY` transfer. In 87 of 130 conditions both readings
below agree; in the 43 where they differ only the second one is consistent (every `cmp`, in any position, feeds
group 3; `addxy r0, r0, #0` placed two instructions before a condition is a flag-setting idiom):

| unit of the instruction | sets condition group |
|---|---|
| first A or MAC unit | 1 (conditions 4–7) |
| second A or MAC unit | 2 (8–11) |
| first B unit (also `cmp`, SFU) | 3 (12–15) |
| second B unit | 4 (16–19) |

Same bits as `X9bY` (`!Z`, `!N`, `!V`, `N or Z`) and the same latency: the instruction after the next one sees them.
A unit skipped by a condition sets nothing. Example (TB-03 voice, a piecewise polynomial): `mac_p1 r3, … |
subxy r2, r0, #1` (group 2 = 1 − r0), one instruction, then `mov r3, r4 | cond !11, 1, 0`: r3 = second polynomial
when r0 ≥ 1.

### 5.2 Control words

C words whose field T (bits 7–4) is ≥ 8 are not conditions. They are **units**, numbered after the A / B / MAC ops and
before the transfers, so another condition word of the instruction can skip them (`0080 1810 0b8c`: the jump is
unit 1, `1810` = cond 12 skips it). Their own condition field (bits 15–8, `cc` and `neg`) gates them: a control word
runs when its condition is **false** (code 0 is false: `00a0` always runs; `32a0` = call unless condition 25, which
the ARM side sets: the effect slots). Field F (bits 3–0) is the argument; Y >> 1 picks a slot word (the last words
of the instruction).

| word | meaning | evidence |
|---|---|---|
| `008Y` + slot | jump to the word address in the slot (absolute; `3ff9` = byte `0x7ff2`, the closing program) | end of the main program; jumps inside `0x1216` all land on instruction starts |
| `00aY` + slot | call the program at that word address (`0029` = `0x0052`), back after its `00f8` | the main program of the second DSP is a list of calls |
| `00f8` | return | the last instruction of every called program |
| `00cY`, `00dY`, `00eY` | not known; no effect in `--bq2` (warning). A "skip Y instructions" reading gives nonsense in the voice program (it would skip the loads that feed the stores after it), so they are taken as side effects outside the data path (events to the ARM?) [A] | |

A program without `00f8` falls through into the next one in memory: `0x008c` → `0x00e2` → `0x0c44` (returns), and
`0x11cc` → `0x1216` → `0x1b80` (returns). The lifted programs are separate functions; the fall-through is the
caller's (call them in a row). `CALL(M, address)` in the lifted C dispatches to the program lifted at that address.

### 5.3 Classes without a template [A]

The voice program decodes completely (its class `0x0098`, a MAC under a condition, `MCN`, is in the decoder's
templates; `asm/disasm.py` prints it as `raw`, the C is right). The second DSP has 11 instructions of classes
without a template (`0x4a8`, `0x3b0`, `0x200`, `0x428` with `4300` words: the bank-2 window of the delay memory,
`0x5e0`, `0x0d0`, `0x390`): `ABORT` in the C and a warning; those instructions have to be decoded by hand.

The conditions of an instruction are taken from the decoder (`dspcomp.decode`) and found again among the body
words (the op words taken out first) to read their fields T and F: the disassembler's text is not used, since it
prints `raw` for classes it has no layout for.

Class `0x108` (`M M C N N`, a MAC pair under a condition) carries its condition like any other class. Before
2026-11-11 the lift (not `--kernel`) left that condition out for `--bq2`: the MACs ran always, and the lifted TB-03
voice — the reference of the port — was wrong where they are skipped. Fixed; `--kernel` had it right.

`newton rD, #imm` (no register Y): on the BQ2 the immediate stands for Y, (Y / 2)² as the register form.

## 6. s1cc kernels (`--kernel`) [sim, 2026-11-11]

`--kernel` writes the program as C that s1cc compiles ([ch. 1](s1cc_compiler_internals.md)), for a port that runs
Roland's operations as they are. It needs a program without jumps (a straight line; `--bq2` control words that only
call or return are fine) and writes `#include "dsplift_k.h"` (runtime, `sdk/s1cc/tools`; the s1cc driver adds that
directory to the include path) and one `static inline void NAME(void)` (or `NAME_a` / `NAME_b`, §6.3).

### 6.1 Form

| element | rule |
|---|---|
| registers | named values `r0…r7` (start of the instruction) and `n0…n7` (results); a register read before it is written in the sample is `static` (kept from the sample before, as on the DSP) |
| condition bits | floats 0 / 1, only for the groups a condition reads (`f4…`, pending `pm` / `pv` with the one-instruction latency) |
| bank 0 | the ring as delays: `c_x` = the cell this sample, `D_x_k` = what address x held k samples ago (a word stored at X is read at X + 1 in the next sample) |
| memory | `KP(a)` parameter words (`C000…`), `KIN(a)` bank-5 inputs, `KOUT(a, v)` bank-5 outputs, `KE(a)` bank 7: macros the including file defines (`KARGS_A` / `KARGS_B`: extra arguments of the two parts). Bank-5 words the program writes are its own state |
| SFU | intrinsics: `recipseed` → `s1_sign(Y) · s1_recip(max(|Y|, 2⁻⁹))` with r(D+1) = rY = 1 (the newton steps after it then keep the exact reciprocal), `expsplit` → `s1_exp(floor Y)` and the fraction, `sign` / `floor` / `ceil` → `s1_sign` / `s1_floor` / `s1_ceil`; `sqrtsplit` is not supported |
| no RAW rounding | plain float arithmetic: what s1cc compiles is compared with the lift in doubles |

### 6.2 Conditions without compares

s1cc compiles a float compare into clamped arithmetic; near a tie (an envelope at its turning point) its select gave
a value that was neither side (TB-03, 2026-11-11). `--kernel` therefore makes the condition bits from the exact SFU
sign of the unit's result:

| bit | expression |
|---|---|
| `!Z` | `s1_abs(s1_sign(z))` |
| `!N` | `fminf(1, 1 + s1_sign(v))` |
| `N or Z` | `1 − fmaxf(s1_sign(v), 0) · s1_abs(s1_sign(z))` |

(v = the value whose sign gives N; z = the value whose zero gives Z: for `min` / `max` z = X − Y.) A unit a
condition may skip is blended: `n = old + w · (new − old)`, w = 0 / 1.

The A unit's ±512 wrap is left out (plain arithmetic) except at the instructions named by `--overflow-wrap`
(`KOVERFLOW`: `|r| mod 512` with the sign of r). Find them with the lift (`dsplift_rt.h` without
`DSPLIFT_NO_OVERFLOW_WRAP` against it with). The TB-03 voice needs none.

### 6.3 Two DSPs (`--split K`)

A program longer than one slave DSP's budget (703 / 708 instructions) is cut: instructions 0…K−1 become `NAME_a`
(call it from `S1_DSP_A`), K… `NAME_b` (`S1_DSP_B`).

| item | rule |
|---|---|
| hand-over | what part B reads before it writes it (registers, condition bits, bank-0 cells) part A sends with `s1_export(slot, v)`, B reads with `s1_import(slot)`; slots from `--export-base` (0), count `NAME_SLOTS` (a `#define`). B sees them one sample late (the slave ring): part B is delayed by one sample as a whole |
| delay chains | each `D_x` belongs to the part that writes x last |
| B → A | a cell A reads from a chain B owns would be a value going back from B to A: SYSTEM-S1CC has no path. The tool stops, except for the cells named in `--zero-cells` (known 0 in the caller's setup) |
| `--import-ranges FILE` | lines `NAME LO HI`, measured on the lift: a value with LO = HI is a constant and is written into B (no slot), the others give s1cc the range of the import |
| no state between samples | registers or condition bits live from one sample to the next: no `--split` |

Example: the TB-03 voice (751 instructions, `--split 415`: 20 values handed over, 20 constants, two zero cells):
[`sdk/s1cc/examples/tb03_ported`](../../sdk/s1cc/examples/tb03_ported/README.md). Compiled by s1cc it matches the
lift within 76–80 dB on sustained notes, 34–48 dB on sequences with accent and slide.

## 7. Files

| file | what it is |
|---|---|
| `sdk/s1cc/tools/dsplift.py` | the translator and `--check` |
| `sdk/s1cc/tools/dsplift_rt.h` | the runtime of the lifted C: memory, SFU, flags, the macros of the units |
| `sdk/s1cc/tools/dsplift_k.h` | the runtime of `--kernel` C: the result / flag macros, `KOVERFLOW`, the SFU as intrinsics (`-DDSPLIFT_K_NATIVE`: libm instead of `s1dsp.h`) |
| `sdk/s1cc/tests/test_dsplift.py` | the test (§3) |
| `examples/from_scratch/emu/dspcore.h`, `dspdec.h` | the reference it follows (decoding: `dspcomp.decode_program`) |
