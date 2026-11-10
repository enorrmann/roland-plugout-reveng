# Volume III — Chapter 6: Optimized Microcode — Mandatory Rules (SYSTEM-1 and SYSTEM-8)

> **Scope:** both; the SYSTEM-1-only rules (proven forms, access rules) are marked.

Specification for all own DSP microcode, on both devices (the ISA is the same). Each instruction costs one cycle of
the sample's budget:
* SYSTEM-1 slave: maximum tested 708, hangs at 826 ([examples/system_1_synth/README.md](../../examples/system_1_synth/README.md) §4);
* SYSTEM-8: about 925–945 per DSP ([Vol. II ch. 4](../vol2_dsp_core_isa/instruction_set_reference.md)).

A wasted instruction is one voice, one waveform or one modulation less. How Roland does it:
[Vol. III ch. 7](roland_oscillators.md).

State: 2026-10-23.

## 1. Rules (mandatory)

1. **Phase operations in a single op.** Wrapping a phase is `ph_wrap rD, r3, rY` (Roland's form, X = r3). The
   triangle is `ph_tri rD, r3, -rY` and the square / clipping to ±1 is `ph_clamp rD, r3, -rY`. The chains `mul ×256` /
   `min` / `max` that do the same in 11 instructions are forbidden.
2. **Packed transfers.** The `ld`/`st` go into the instruction of an op (`pack()`, like Roland), within the
   per-instruction limits (`chain.access_violations`). A filler `mov rN, rN` is only written in the unpacked code and
   disappears when packing.
3. **Independent operations fused.** Two or three ops that do not depend on each other go in one instruction
   (`op1 | op2`, A+B, A+A+B). **Two ops of unit B in one instruction hang the SYSTEM-1** [measured 2026-10-28:
   `max + max`, `ph_wrap + ph_wrap`, `ph_clamp + ph_clamp`, `min + min`; `asm/bad_forms.txt`]. The groups that ran are
   in `asm/proven_forms.txt` (59), and new ones are tested with `TRY_FORMS`
   ([examples/system_s1cc/README.md](../../examples/system_s1cc/README.md) §4.1). All of them read before writing. The
   scheduler `asm/dspsched.py` does it (§5), reordering within the dependencies. `pack()` with `FUSE=1` is the previous
   version, only with neighbours.
4. **No repeated loads.** A value that is already in a register or in memory in the same sample is not loaded again
   (for example, the gate: one load per voice and per sample, which also serves for the copy towards DSP B).
5. **Bit-exact validation in the emulator.** Every optimization is compared with the unoptimized version in the C core
   before installing: `make_synth.py --pack-check` (≈ 3 s, identical samples). `--sim-check` (C core against the Python
   reference) is only needed when the emulator changes or a new instruction form appears ([TOOLS_GUIDE.md](../TOOLS_GUIDE.md)
   §1).
6. **Parameters on the registers' grid.** A parameter used right after it is loaded has to be on the quantisation grid
   (`on_grid()` in `make_synth.py`). Otherwise the packed and the unpacked versions differ by 1 LSB.
7. **No build without `check_optimized`.** The warnings are fixed or justified in the example's README.

## 2. `tools/check_optimized.py`

```sh
python3 tools/check_optimized.py examples/system_1_synth/s1m0.s     # .s, .hex or .txt (words)
python3 tools/check_optimized.py --strict FILE...                    # exit code 1 if there are warnings
python3 tools/check_optimized.py -q FILE...                          # one line per file
```

| warning | what it detects | correction |
|---|---|---|
| `manual-wrap` | repeated pairs `mul rX, rX, #256` + `min rX, rX, …` | rule 1 |
| `filler` | `mov rN, rN` alone in an instruction | rule 2 |
| `transfer-only` | instruction with only `ld`/`st` | rule 2: move it to a neighbouring op |
| `fusable` | two consecutive instructions of one op, independent and encodable as one (the assembler checks it) | rule 3 |

It is run, after generating the programs, by:

| device | build | programs it checks |
|---|---|---|
| SYSTEM-1 | `examples/system_1/build.sh` | `s1m0.s`, `s1m1.s`, `s1mm1.s` |
| SYSTEM-1 | `examples/system_1_synth/build.sh` | the same |
| SYSTEM-8 | `examples/from_scratch/gate/build.sh` | `dsp2_osc.hex`, `dsp4_gate.hex`, `dco2_null.hex` |
| SYSTEM-8 | `examples/from_scratch/mini/build.sh` | `poly.hex` |
| SYSTEM-8 | `examples/wt_morph/build.sh`, `examples/wt_pwm/build.sh` | `prog/*.txt`, `wt2/dco2_w*.txt` |
| SYSTEM-8 | `examples/wt_live/build.sh` | `cal_d2.txt`, `cal_d3.txt` |

`examples/from_scratch/engine/build.sh` is not modified (project rule). Its own programs arrive through `OWN_PROGS` /
`MC_TEST` from the scripts above, which are the ones that check them. `examples/system_1_jx` only relocates Roland's
microcode and is not checked.

State when the tool was created (2026-10-23):

| program | instructions | warnings |
|---|---|---|
| C-S1SY step 8, DSP A (installed) | 587 | 16 manual-wrap, 25 transfer-only, 49 fusable |
| C-S1SY step 11, DSP A (`FUSE=1`) | 576 | 0 manual-wrap, 49 transfer-only, 25 fusable (of 74 without `FUSE`) |
| C-S1SY step 11, DSP B (`FUSE=1`) | 603 | 62 transfer-only (the 6 zero writes of `6008`–`600D` and the loads of each voice) |
| C-S1V4 `s1m0` | 269 | 8 manual-wrap, 16 filler, 33 transfer-only, 9 fusable |
| SYSTEM-8 `poly.hex` (C-MINI2) | 210 | 5 manual-wrap, 9 filler, 33 transfer-only, 5 fusable |
| SYSTEM-8 `dsp2_osc.hex` | 35 | 1 manual-wrap, 2 filler, 4 transfer-only, 2 fusable |
| SYSTEM-8 wavetables `dco1_w0.txt` | 344 | 2 manual-wrap, 10 transfer-only, 7 fusable |

[open] `pack()` only fuses an op with the immediately previous instruction. 25 pairs remain in DSP A of step 11, and the
loose transfers are not moved towards the next op.

[open] The SYSTEM-8 programs (`make_v3`/`make_v4`, `make_chain`, `make_poly`, `wav2dco`) and C-S1V4 still have the
manual wrap: they have to be moved to `ph_wrap`, packed and fused, validated in the emulator and tested on the device.

## 3. Examples (C-S1SY, `make_synth.py`)

Saw with `ph_wrap` (step 10): phase += increment and wrap in two ops, instead of 13.

```dsp
addxy r1, r3, r1
ph_wrap r1, r3, r1
st r1, [0300]
```

Fused transfers and ops (step 11, output of `pack()` with `FUSE=1`): three ops in one instruction, the loads that the
next instruction uses.

```dsp
mad r7, r1, r4 | mul r3, r1, #256.0 | ld r4, [c062]
ph_clamp r3, r3, -r3
mad r7, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c060]
```

In the last instruction, `mad` reads the previous r3 (the square) and `ph_tri` writes the triangle into r3. All units
read before writing.

## 4. Measured cost

| C-S1SY step | DSP A (instr.) | DSP B (instr.) | what changes |
|---|---|---|---|
| 9 (global LFO, installed) | 693 | 652 | — |
| 10 (saw with `ph_wrap`) | 517 | 652 | −176: 11 instructions × 16 oscillators |
| 11 (per-voice LFO, without `FUSE`) | 649 | 652 | +132 for the per-voice LFO |
| 11 (with `FUSE`, by default) | 576 | 603 | −73 / −49 from the fusion |

`--sim-check` of step 11: C core = Python reference, and fused = unpacked, bit for bit (4 panels, two of them with
LFO) [sim, 2026-10-23].

## 5. Instruction scheduler (`asm/dspsched.py`)

Common to both devices. The program comes in as the generators write it: one op or one transfer per line, the delay
of a load as a filler `mov rN, rN` and comments with `;`. The same program comes out in fewer instructions:
* the independent ops go together (`op1 | op2 | op3`);
* the loads and stores go into the instructions of the ops;
* the fillers disappear;
* the units are reordered as far as the dependencies allow. `pack()` only joined a line with the instruction before,
  in program order.

```python
import dspsched
lines = dspsched.schedule(lines)       # list of instructions as text, ready for the assembler
```

Semantics it keeps, those of the emulator (measured on the device):
* all units of an instruction read before any of them writes;
* a loaded register can be used from the next instruction on;
* a store reads the register at the start of the instruction.

Dependencies (i before j in the program) and minimum distance in instructions:

| dependency | distance |
|---|---|
| reading a register that an op or a load wrote | 1 |
| writing a register that was read before | 0 (can go in the same instruction) |
| writing a register written before | 1 |
| same address (banks 0, 1 and 5 every 0x800, IOMD every 0x2000): load after store | 1 |
| same address: store after load | 0 |
| same address: two stores | 1 |
| a line it does not understand (another op, a condition) | barrier: own instruction, nothing crosses it |

Per-instruction limits: those tested on the device with `pack()` and `FUSE` (C-S1SY8 … C-S1SY11).
* At most 3 ops, each with a single destination (`OPS`).
* At most 3 transfers, without repeating an address: bank 0 ≤ 2 loads and ≤ 2 stores, the other banks ≤ 1 of each.
* The instruction has to assemble.

Priority: the longest dependency path to the end of the program (critical path) and, if equal, program order.

Results [sim, 2026-10-23]:

| program | before | with the scheduler | validation |
|---|---|---|---|
| SYSTEM-1, C-S1SY DSP A (step 12) | 640 (`pack()` + `FUSE`) | **553** | `--pack-check` identical on 5 panels; `test_s1_sched` (banks 0 and 5 of the three DSPs, word by word) |
| SYSTEM-1, C-S1SY DSP B (step 12) | 603 | **471** | same |
| SYSTEM-8, 4-voice voice of C-MINI2 (`make_poly.py --opt`: `ph_wrap` + scheduler) | 210 (`poly.hex`, manual wrap) | **88** | `test_s8_poly_opt`: scheduled = unscheduled bit for bit; same pitch and level as `poly.hex` |

`check_optimized` after the scheduler: C-S1SY A 2 `transfer-only` and 8 `fusable`, B 4 and 8; `poly_opt` 2
`transfer-only`. They are the ones that do not fit because of the port or template limits.

Validating with the whole state a difference of the simulation appeared, not of the scheduler. `simulate()` wrote the
pitch as a Python double and the ARM writes float32 (`pitch_bits`), which falls on the grid. When storing the freshly
loaded register without a unit in between, the raw value came out. Now `sim_event` writes float32, like the device
(rule 6).

Use:
* C-S1SY: `STEP=12` (or `SCHED=1` with another step) in `make_synth.py` / `build.sh`.
* C-MINI2: `OPT=1 NAME=C-MINI2O sh examples/from_scratch/mini/build.sh`; without `OPT` `poly.hex` is still used.

[open] Neither of the two variants has been tested on the device yet. C-S1V4 and C-V4 do not go through the scheduler
yet.

> Later state (from [examples/system_1_synth/README.md](../../examples/system_1_synth/README.md) §2, table of
> steps): step 12 (the scheduler instead of `pack()`, 553 / 471) is "[sim]"; step 13 keeps those counts and adds the OS
> delay and reverb, and step 13 + `FOLD` (C-S1SY13F, 585 / 471 / 7) "works" on the device (2026-10-23) and is the last
> build that works completely.
