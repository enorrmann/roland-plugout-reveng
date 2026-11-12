# Volume V — Chapter 8: `dsplift` — DSP Microcode to Plain C

> **Scope:** both (any program of the DSP ISA: SYSTEM-8, SYSTEM-1, and the Boutique `BQ2` firmware of the TB-03).

Specification. State: 2026-11-11, version 1.2.

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
| `--names FILE` | `--kernel`: names of parameters, inputs, outputs, cells, values, section headings; unused outputs; constant inputs (§6.4) |
| `--consts FILE` | `--kernel`: memory words that are constants in the caller's setup, lines `ADDR[-ADDR] VALUE` (hex addresses) (§6.4) |
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
A unit skipped by a condition sets nothing. A `mov` (unit A, op 0, x 1: rD = Y) sets no group and takes no number: the next
arithmetic unit of the instruction is "first" (2026-11-11). Evidence: the TB-03 voice's cutoff map (instructions
250–271) is a piecewise polynomial whose segment tests are written `mov rA, rB | addxy rC, x, #-edge`; with the
`mov` counted, the tests read a constant and the map was not monotonic (filter cut-off never below ~4 kHz, knobs
moving it by 1 octave); without it the map rises monotonically, and the filter sweeps ~100 Hz … several kHz with
CUTOFF and ENV MOD as a TB-303's, DECAY shaping the sweep (lift, frequency response measured by injecting a sine
at the filter input). Example (TB-03 voice, a piecewise polynomial): `mac_p1 r3, … |
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
Roland's operations as they are, and as C a person reads. It needs a program without jumps (a straight line; `--bq2`
control words that only call or return are fine) and writes `#include "dsplift_k.h"` (runtime, `sdk/s1cc/tools`; the
s1cc driver adds that directory to the include path) and one `static inline void NAME(void)` (or `NAME_a` /
`NAME_b`, §6.3). The emitter is `sdk/s1cc/tools/dsplift_c.py`.

### 6.1 Form

The program is run symbolically, instruction by instruction, with the semantics of the units (all units read the
registers of the instruction start, loads land at its end, condition bits arrive one instruction later, a unit a
condition may skip is blended by a 0 / 1 weight). What it computes is an expression graph, written as C:

| element | rule |
|---|---|
| values | one `float NAME = expression;` per value that is used more than once, stored in a bank-0 cell, named in the names file, or longer than 100 characters inlined; everything else is inlined. The comment after the line is the instruction that makes the value |
| default names | a cell's value: `c_X` (X = the bank-0 address); a parameter / input: `p_cXXX` / `in_aXXX`; a condition bit: `<value>_ge0`, … ; any other value: `tK` (K = the instruction), `tK_2` … for more in one instruction |
| bank 0 | the ring as delays: a cell's value at the start of a sample is what the nearest touched address below held one or more samples ago. Statics `X_zK` (with a name) or `D_X_K`: what cell X held K samples ago. A cell the program never writes only delays its source: its reads are the source further back (`saw_s0_z3`, not a chain of copies) |
| memory | `KP(a)` parameter words (`C000…`), `KIN(a)` bank-5 inputs, `KOUT(a, v)` bank-5 outputs (a statement where the program writes), `KE(a)` bank 7: macros the including file defines (`KARGS_A` / `KARGS_B`: extra arguments of the two parts). Named words use `#define P_NAME` / `IN_NAME` / `OUT_NAME`. Bank-5 words the program writes are its own state |
| registers, flags | not in the C: they are the edges of the graph. A register or condition bit read before it is written in the sample is a `static` (`NAME_rN`, `NAME_fC`; not with `--split`) |
| only what is needed | what reaches no output (`KOUT` not named `unused`, the hand-over, live statics) is left out, across samples too: a delay chain exists only while a needed value reads it |
| no RAW rounding | plain float arithmetic: what s1cc compiles is compared with the lift in doubles |

Exact simplifications only (the evaluation tree of every operation is kept, nothing is reassociated): float32
constant folding; `x · 1`, `x + 0`, `x − 0`, `x · 0 = 0` (the DSP has no inf / NaN); `x + (−y) → x − y`,
`x + (−c) · y → x − c · y`; `1 − (1 − b) → b` for 0 / 1 values; a blend whose weight is 0, or 1 between 0 / 1 values;
shared subexpressions; `pymod` and the condition bits of constants. A delay chain that only ever holds 0 is 0: the
lifter assumes it for all chains, runs, keeps those that are written 0, and runs again until nothing changes (the
greatest fixed point; statics start at 0, so by induction they stay 0). A blend with weight 1 between values that are
not 0 / 1 would not be exact; it is counted and reported (none in the TB-03 voice).

Idioms are macros of `dsplift_k.h` that expand to the expression the operations compute, so the C means the same
whether a macro or its expansion is read:

| macro | expression | from |
|---|---|---|
| `SEL(w, a, b)` | `b + w (a − b)`, w = 0 / 1: w ? a : b | a unit a condition may skip (§6.2) |
| `LERP(a, b, t)` | `a + t (b − a)` | one-pole steps, crossfades |
| `XFADE(a, b, w)` | `w b + a − w a` | the same crossfade as a MAC computes it (products in either order: exact) |
| `K_NZ`, `K_Z`, `K_GE0`, `K_LT0`, `K_LE0`, `K_GT0`, `K_LE0VZ`, `K_GT0VZ` | the condition bits and their complements `1 − bit` (§6.2) | condition words |
| `K_RECIP(y)` | `s1_sign(y) · s1_recip(max(|y|, 2⁻⁹))` | recipseed and its newton steps |
| `K_CLAMP(x)` | `max(−1, min(1, x))` | unit B clamp |
| `K_TRI(x)` | `1 − 2 |wrap(−x − 1.5)|` | unit B's triangle |
| `K_SIN(x)`, `K_EXP(x)` | Roland's polynomials: sin x for \|x\| ≤ π/2; e^x (expsplit: e^floor(x) times the series of the fraction) | recognised by building the macro's expression on the candidate argument: the node must be that expression |

The SFU parts are intrinsics: `recipseed` → `K_RECIP` (the newton steps after it fold away: r(D+1) = rY = 1),
`expsplit` → `s1_exp(floor Y)` and the fraction, `sign` / `floor` / `ceil` → `s1_sign` / `s1_floor` / `s1_ceil`;
`sqrtsplit` is not supported.

### 6.2 Conditions without compares

s1cc compiles a float compare into clamped arithmetic; near a tie (an envelope at its turning point) its select gave
a value that was neither side (TB-03, 2026-11-11). `--kernel` therefore makes the condition bits from the exact SFU
sign of the unit's result:

| bit | macro | expression |
|---|---|---|
| `!Z` | `K_NZ(z)` | `s1_abs(s1_sign(z))` |
| `!N` | `K_GE0(v)` | `fminf(1, 1 + s1_sign(v))` |
| `N or Z` | `K_LE0VZ(v, z)`, `K_LE0(v)` when v = z | `1 − fmaxf(s1_sign(v), 0) · s1_abs(s1_sign(z))` |

(v = the value whose sign gives N; z = the value whose zero gives Z: for `min` / `max` z = X − Y.) A unit a
condition may skip is blended: `n = SEL(w, new, old)`. A nested `SEL(K_LT0(x), a, SEL(K_GE0(x), b, c))` is not
simplified to `SEL(K_LT0(x), a, b)`: `c + 1 · (b − c)` need not be `b` in float.

The A unit's ±512 wrap is left out (plain arithmetic) except at the instructions named by `--overflow-wrap`
(`KOVERFLOW`: `|r| mod 512` with the sign of r). Find them with the lift (`dsplift_rt.h` without
`DSPLIFT_NO_OVERFLOW_WRAP` against it with). The TB-03 voice needs none.

### 6.3 Two DSPs (`--split K`)

A program longer than one slave DSP's budget (703 / 708 instructions) is cut: instructions 0…K−1 become `NAME_a`
(call it from `S1_DSP_A`), K… `NAME_b` (`S1_DSP_B`).

| item | rule |
|---|---|
| hand-over | what part B needs of part A's state (registers, condition bits, bank-0 cells) A sends with `s1_export(slot, v)`, B reads with `s1_import(slot)`, named as in A; one slot per distinct value, slots from `--export-base` (0), count `NAME_SLOTS` (a `#define`). B sees them one sample late (the slave ring): part B is delayed by one sample as a whole |
| constants | a value that is a constant at the end of A is written into B (no slot); so is a value `--import-ranges` measured constant |
| delay chains | each chain belongs to the part that writes its cell last |
| B → A | a cell A reads from a chain B owns would be a value going back from B to A: SYSTEM-S1CC has no path. The tool stops, except for the cells named in `--zero-cells` (known 0 in the caller's setup) |
| `--import-ranges FILE` | lines `NAME LO HI` (NAME = `r4`, `f5`, `c_033`, …), measured on the lift: LO = HI is a constant, the others give s1cc the range of the import |
| no state between samples | registers or condition bits live from one sample to the next: no `--split` |

### 6.4 Names and constants (`--names`, `--consts`)

The names file is what is known of the program; the C is regenerated from it (`dsplift_c.Names`). Lines (`#`
comments):

| line | effect |
|---|---|
| `param C019 cutoff [knob]` | the parameter's name (`KP(P_CUTOFF)`, a local `cutoff`); `knob` = set by the panel: may not be a constant |
| `in A004 pitch`, `out A100 voice` | an input's / output's name |
| `unused A101 A102` | outputs the including file ignores: not computed |
| `const A003 1`, `const A104-A109 0` | inputs that are constants in the caller's setup |
| `cell 041 flt_env` | the value stored in bank-0 cell 041 (and its chain: `flt_env_z1`) |
| `value t331 saw_inc` | renames the value the C calls `t331` (a name that no longer exists is a warning) |
| `section 312 saw oscillator` | a heading above the values from instruction 312 on |
| `section @osc1_freq*,@osc1_phase* OSC1` | a heading above the first value whose name matches one of the globs: the same file serves programs whose instruction numbers differ |
| `include FILE` | another names file (path relative to this one) |

`--names` may be given several times: the files are read in order, a later line wins.

**Automatic cell names: `dsplift_names.py`, step 1** (2026-11-12). The lifter names nothing by itself; a separate
step writes the names file it reads, so the file can be read and corrected by hand in between:

```sh
python3 sdk/s1cc/tools/dsplift_names.py OUT_names.txt --names SEEDS.txt [the --kernel arguments]   # step 1
python3 sdk/s1cc/tools/dsplift.py OUT.h --kernel --names OUT_names.txt [the same arguments]         # step 2
```

`OUT_names.txt` = the seed files (includes inlined, in order) and then a `cell X NAME` line for every bank-0 cell
they leave unnamed (`dsplift_c.auto_names`), named `GROUP_ROLE`:

| part | rule, first that applies |
|---|---|
| GROUP | 1. the named cell (names file) the value flows into first, when exactly one group is at that distance; 2. the group of the named parameters it reads, the most frequent (`env1_a` → `env1`: a parameter name's first word); 3. the groups of the cells it reads (to a fixed point); 4. the named inputs it reads |
| ROLE | `phase` (wraps and reads its own value of the sample before), `lp` (`LERP` from its own value of before), `state` (reads itself otherwise), `fir` (eight or more delayed cells), `prev` (holds the value of a lower cell: one sample later; named after that cell), `in` (a copy of a delayed or handed-over value), `flag` (0 / 1), `sq` / `exp` / `recip` / `sin` / `tri` / `sign` / `abs` (the outer operation); none: the group alone, numbered |

A cell that only copies a parameter or an input gets no name (the value's own name serves). Running step 1 again
overwrites `OUT_names.txt`: a correction that should stay goes into a seed file. So the seeds are only the parameters
(generated from the panel tables in `boutique_ported/tools/make_voice.py`) and the musical role of a few cells per
stage (`boutique_ported/bq_names_cells.txt`: 36 seeds, 54 more cells named automatically in the JP-08
voice).

**Names do not change the code** [measured 2026-11-12]. A value longer than 100 characters inlined gets a name of its
own (§6.1), and s1cc fuses a multiply and an add only inside one statement, so a cut costs instructions. The cut is
therefore measured on the text without names (`Emitter.canon`: parameters as `KP(0xC05F)`, delay cells as
`D_000_1`): the names files change only identifiers. s1cc units of the four JP-08 builds with the names files and with
none: `bq.c` 553 / 449, `bq_host.c` 797, `bq_host2x.c` 468, `bq_arm.c` 388, the same. (Before the canonical measure,
long names cut more often.)

`--consts FILE`: lines `ADDR[-ADDR] VALUE` (hex), the same as `const`; for the parameters the caller's init sets and
no knob moves. With constants the parts of a program its setup switches off fold away (§6.1: `x · 0`, delay chains
of 0).

### 6.5 Checks

The C keeps every operation's tree, so it computes what the per-instruction form of version 1.1 computed. Example: the
TB-03 voice (751 instructions, `--split 415`), compiled natively (clang, `-ffp-contract=off`), 100 000 samples with
knobs, notes, gate, accent and slide changing [native, 2026-11-11]:

| C | against the per-instruction form |
|---|---|
| no names, no constants | all five outputs bit-identical, also with every parameter and input random |
| `tb03_names.txt` and the TB-03 init as constants | the voice output `A100` bit-identical in the TB-03 setup |

| | per-instruction form (1.1) | this form |
|---|---|---|
| `tb03_voice.h` | 13 663 lines | 357 lines (383 without `--names` / `--consts`) |
| values A → B | 24 | 9 |
| s1cc: DSP A / DSP B instructions | 497 / 427 | 468 / 410 |

Example: [`sdk/s1cc/examples/tb03_ported`](../../sdk/s1cc/examples/tb03_ported/README.md). Compiled by s1cc it matches
the lift within 76–80 dB on sustained notes, 34–48 dB on sequences with accent and slide.

## 7. Files

| file | what it is |
|---|---|
| `sdk/s1cc/tools/dsplift.py` | the translator and `--check` |
| `sdk/s1cc/tools/dsplift_c.py` | the `--kernel` C: symbolic run, expression graph, idioms, names, automatic names (§6) |
| `sdk/s1cc/tools/dsplift_names.py` | step 1 before `--kernel`: a whole names file, seeds + automatic cell names (§6.4) |
| `sdk/s1cc/tools/dsplift_rt.h` | the runtime of the lifted C: memory, SFU, flags, the macros of the units |
| `sdk/s1cc/tools/dsplift_k.h` | the runtime of `--kernel` C: the idiom macros (`SEL`, `LERP`, `XFADE`, condition bits, `K_RECIP`, `K_SIN`, `K_EXP`, …), `KOVERFLOW` (`-DDSPLIFT_K_NATIVE`: libm instead of `s1dsp.h`) |
| `sdk/s1cc/tests/test_dsplift.py` | the test (§3) |
| `examples/from_scratch/emu/dspcore.h`, `dspdec.h` | the reference it follows (decoding: `dspcomp.decode_program`) |
