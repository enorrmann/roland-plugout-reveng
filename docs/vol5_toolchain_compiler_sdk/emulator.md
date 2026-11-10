# Volume V — Chapter 5: Emulator of a Complete PLUG-OUT — What Exists, What Is Missing and in Which Order

> Paths `emu/…`, `re/…`, `engine/…`, `voicelog/…` are relative to `examples/from_scratch/`.

State: 2026-10-29 (the reference is the C: §1, §1.1; speed: §3.6.1). Step 1 (DSP core in C) and the speed (§3.6: one
JX-3P slave in real time, bit-identical to `chain.py`) are done; the rest is a plan. **To resume: §5.** Goal: run a
Roland PLUG-OUT (starting with the JX-3P) on the PC and obtain the same audio as the device, to study the microcode and
to test own programs without installing them.

Tags: **[measured]** = on the device; **[code]** = it is in our code; **[listing]** = read from the disassembled
microcode; **[documented]** = in another document of the repo; **[inferred]** = deduction without measurement.

**Microcode rule** ([S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §7): the emulator and its tests **may** run Roland's microcode. It is
loaded at run time from the user's local copy (`engine/build/programs.h`, `plugins/<NAME>`), as `engine/programs.py`
does, and no Roland bytes are kept in the repository. The prohibition only applies to our plug-outs.

## 1. What already exists

**The DSP reference is the C code** (user's decision, 2026-10-29): `emu/dspdec.h` (decoder), `emu/dspcore.h`
(semantics of each unit) and `emu/dspcore.c` (interpreter, slave, single-DSP machine). From Python it is only used
through `emu/dspref.py` (ctypes, without semantics of its own). The Python model (`re/dspisa.py`, the `Slave` of
`chain.py`, the front of `dspcomp.py`) was archived in `archive/` (§1.1); `re/dspisa.py` is a notice that fails when
imported.

| piece | what it does | limit |
|---|---|---|
| `examples/from_scratch/emu/dspdec.h` | class templates (`re/dsp_templates.txt`, `re/dsp_units_ab.txt`), slot area, immediate stream, units, conditions, jumps → `Op` / `Cond`; `dsp_check` (forms without a measured effect), `dsp_hidden_use` | — |
| `examples/from_scratch/emu/dspref.py` | `Machine` (one DSP, flat memory 0–FFFF, bank 5 folded to 1024 words: the simulator of the tools `make_*.py`, `dspasm.py`, `dspnative.py`, `dspverify.py`, `sim_main.py`), `layout`, `template`, `check`, `quant`, `raw_value`, flag contexts | — |
| `examples/from_scratch/emu/chain.py` | `Model` (the hypotheses of the slave), `Program`, `access_violations`, helpers | the emulator is `cchain` |
| `examples/from_scratch/emu/test_chain.py` | 20 tests; the model matches the measurement of C-V4 ([S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6.7), on the C core | — |
| `examples/from_scratch/emu/cchain.py` + `dspcore.c` + `dspcomp.py` | **C core**: `cchain.Slave` / `Bmc2`. `dspcomp.py` only packs what `dspdec.h` decodes. The `.so` builds itself in `emu/build/` (`dspref.lib`) | gaps of §3.3 |
| `examples/from_scratch/emu/dspjit.py` + `dspcore.h` | translation of each program to native code and groups of threads per sample (§3.6 point 2): one JX-3P slave in real time | GCC 14 `-O2` needs `-fno-gcse -fno-peephole2` (§3.6) |
| `examples/from_scratch/emu/test_core.py` | the 15 756 distinct instructions of the local corpus against the golden file `work/golden_insn.txt` (hashes only, local), JX-3P native against interpreter, the 212 programs of the corpus in native against the interpreter, native with threads against serial, `quant` against its definition, v3/v4, speed | the golden file is regenerated (`--update-golden`) only after a measured change of the semantics |
| `examples/from_scratch/re/emu1.py`, `tools/engine_emu.py` | region 1 (ARM engine) in Unicorn, OS services `0x0113FC00` intercepted | they only log the calls; they do not connect them to any DSP |
| `examples/from_scratch/re/emu0.py` | region 0 (model, assigner) in Unicorn | without MIDI input or link to the slaves |
| `examples/from_scratch/emu/listing.py` | reads Roland's programs from `programs.h` and disassembles them | static reading only |
| `voicelog/voicelog.py`, `re/dspaudio.py` | captures of the device for comparison | — |

> Note: the "GCC 14 `-O2` needs `-fno-gcse -fno-peephole2`" of the table above was resolved later the same week: it was
> the sign of a NaN, and the flags were removed (§3.6, §3.6.1).

### 1.1 The port to C (2026-10-29)

`archive/dspisa.py`, `archive/chain.py` (with its `Slave` in Python) and `archive/dspcomp_py.py` are the previous model,
kept only for validation. `archive/validate_port.py` compared the C with it over the 9791 distinct instructions of
`work/corpus_all`: template, slots, units, decoding and execution of one instruction from random states (registers,
flags, memory, jumps). Result: identical, except 50 whose MAC coefficient changed by the rule measured that day
([Vol. II ch. 8](../vol2_dsp_core_isa/isa_discrepancies.md) D8). `sim_main.py` gave the same negatives as before on MAIN0–5. The tools moved to
`dspref.Machine` and kept giving the same: `make_synth.py` and `make_mutable.py --sim-check` (the slave of `cchain`
against the flat-memory machine) identical, `make_*.py --sim` of `examples/from_scratch` with the same frequencies.

## 2. Proposed architecture

```
MIDI ─► region 0 (Unicorn, emu0) ─ note/velocity 0x429+v / 0x43A+v ─► region 1 ×2 slaves (Unicorn, emu1)
                                                                         │ OS services (§3.1)
                                                                         ▼
                              DSP0..DSP5 ×2 (C core, §3.6) ─► slave 0 6020/6021 ─► WAV
                              (+ later: wave bus → master's DSPs, §3.5)
```

## 3. What is missing

### 3.1 Joining the ARM with the DSPs (medium complexity, known)

The OS services that the ARM engine calls have to act on the DSP emulator [code, `engine.c` / `engine_v3.c`;
[Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) §2]:

| service | action in the emulator |
|---|---|
| `+0x28/+0x40/+0x58 + 4·dsp` | register slots, programs and data: copy the microcode to the DSP's slot |
| `+0x70` `swap` | change the program of a slot and activate its task (DCO1 = 2, DCO2 = 3, effect = 3) |
| `+0xB0` `p_bank` | write `A000 + id` of the DSP's bank 5 |
| `+0xA0` `p_ramp` | write an entry of bank 6 with a ramp (§3.2) |
| `+0x00` `get` | read the patch parameter (patch model in Python) |
| `+0xC0` `sync` | frame boundary: apply the change between samples |
| `+0xF0` | index of the slave |

### 3.2 Bank 6 (`Cnnn`): the parameter block with ramps (the most important and the least known)

* Roland's main programs read a lot from there: ≈ 2000 loads in the corpus ([Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) §3.1); e.g. the
  coefficients `C044`–`C048` of MAIN4's mix [listing]. Today `chain.py` returns 0.
* What the ARM writes is known: an entry of 16 bytes per id, `[+8] = mode<<28 | flag<<24 | ramp<<20 | 0x08000000`,
  `[+0xC] = target`; modes "00 disable, 10 auto ini, 01 timer, 11 env" [documented, [Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) §1 and §2.2].
* **How the value evolves sample by sample is not measured** (shape and duration of the ramp according to `ramp`, what
  `flag` and the modes do). Without that there are no envelopes or levels.
* Proposed measurement: with the master's lab (`re/dspmaster.py` / `dsppeek`), a program on DSP3 that copies a `Cnnn`
  word to DT0 every sample, and the ARM writing with `+0xA0` steps from 0 to 1 with different `ramp`, `flag` and modes;
  read the series and fit the model.

### 3.3 Fidelity of the instruction simulator with long programs

| gap | why it matters | how to close it |
|---|---|---|
| **Load latency for the ops**: `dspisa` makes a load visible to the next instruction; [Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §7.3 says the next op still sees the old value. Roland has `ld r0,[02b2] \| ld r1,[029a]` followed by `mul r0,r0,r1` | it changes chained results | test on the master (`re/dspverify.py` with two instructions) |
| Instructions "inferred"/"fitted" of the disassembly (MAIN4: 56 inferred, 10 fitted) | possibly different results | sweep with `re/dspverify.py` of the affected classes |
| Undecoded transfer modes: `9e`, `e0`/`e1`, `d8`/`d9`/`dc`/`dd` | memory accesses that do nothing today | `re/dspmodescan.py`, tests on the master |
| Bank 7 (`e013`, the JX reads it as a source) | noise, counter? | read the word on the master over several consecutive samples |
| Floating-point arithmetic with the truncation rule | accumulated rounding errors | core in integers with a 29-bit mantissa (§3.6) |
| Flags and jumps per DSP | already kept per DSP in `chain.py` | — |

> Note: on the load latency, see the reconciliation in [Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md)
> §7.3 (later measurements agree with the emulator: the next instruction sees the loaded value).

### 3.4 Bank 2: delay line and EDM

Chorus, delay and phaser use it. Measured in part ([Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §8.4: write `[4300]`, request `Xa9Y`, 6-bit
fraction, unit 16384 samples, line of `f7dc` in EDM). Missing in the emulator: the EDM/IDM memory, the per-sample DMA
and the window of each DSP/slot (the ARM connects it: `+0x310`).

### 3.5 Details of the slave still open, and the master

* Slave: whether the mailbox `A400` arrives in the same sample or in the next; order of the DSPs inside the sample;
  which words of slave 1 enter `6000`–`6003` of slave 0 ([S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6.6). For sounding they do not
  matter; for matching bit for bit, they do.
* Master (only if the final output is wanted): the master's DSPs with their microcode (global effects, master, reverb;
  it is in the OS, `m1_tcm`), region 0 with MIDI input and panel, and the wave bus slaves → master (correspondence
  unknown). Meanwhile, the reference output is `6020`/`6021` of slave 0.

### 3.6 Speed

`dspisa` in Python does ≈ 20 000 instructions/s (66 instructions of the saw in ≈ 3 ms). A Roland plug-out is
≈ 11 000 instructions per sample (6 DSPs × ≈ 930 × 2 slaves): **more than 10 hours per second of audio**. It needs:
1. A DSP core in C (interpreter), with the same interface as `chain.py` (banks, MXM, mailbox, IOMD), callable from
   Python (ctypes/cffi). Validate it against `dspisa` instruction by instruction and against `test_chain.py`.
2. Then, translate each program to native code once (there are almost no jumps) and parallelise per DSP: MXM is frozen
   per sample, so the DSPs of one sample are independent.
3. Skip DSPs without active voices.

**Point 1 done (2026-10-22)** [code, `emu/test_core.py`]: the JX-3P on one slave (MAIN0–5 + DCO1/DCO2 SAW on DSP2 and
DSP3, random bank 5) executes **3 621 instructions per sample** (fewer than the estimate above: there are jumps), and
the C core does ≈ 18 M instructions/s (one core, `-O2`): **1 s of audio of one slave ≈ 19 s**. Real time needs
≈ 0.35 G instr/s per slave, about 20 times more. The core is compiled with `-ffp-contract=off`: a fused multiply-add
(FMA) would change the bits with respect to `dspisa`, which rounds each product.

**Point 2 done (2026-10-22)** [code, `emu/test_core.py`]: **one JX-3P slave runs in real time** (1 s of audio in
≈ 0.8 s; ≈ 280 M instr/s), with the same bits as the interpreter and as `chain.py`.

* `emu/dspjit.py` translates each program to a C function: words as constants, jumps as `goto`, and the units call the
  same inline functions as the interpreter (`emu/dspcore.h`), so the semantics are in one place only. Each program is
  an object of its own, compiled in parallel and cached in `emu/build/jit/` by the hash of its source (JX-3P: ≈ 7 s the
  first time). `cchain.Slave(..., native=True, threads=True)` is the default.
* `quant` without branches (scale chosen by the IEEE exponent): the version with branches was ≈ 90 % of the time
  because of mispredicted branches. The translator only quantises the registers that can be unquantised (`quant` is
  idempotent).
* Threads: one group of DSPs per thread inside each sample (`dspjit.groups`). The DSPs that share an IOMD word or bank
  1 (`Model.shared1`) go together; with MXM in the same sample or the immediate pair window, everything goes in one
  thread. The pair window of bank 5 is merged at the end of the sample in the model's order, so the result does not
  depend on the threads. The JX-3P gives six groups; the ceiling is the most loaded DSP (DSP2/3, ≈ 840 instr.). The
  threads are used in `run()` / `step(n)` with n ≥ 64.
* **What looked like a GCC 14 bug** (`p0s_003661c`: r2 = −5 instead of −3 after instruction 86, in the third sample;
  it went away by removing `-fgcse` or `-fpeephole2`) **was the sign of a NaN** [measured, 2026-10-23].
  * The random states of the test bring inf, and NaNs appear.
  * C does not fix which NaN `a + b` returns with NaN operands, and the compiler can swap them. The `sign` op then turned
    that sign into ±1.
  * With clang 19 the same came out, in `p0s_003661c` and in `p0s_003743c`.
  * Fix: `quant` returns a canonical NaN (+qNaN). Also, the exact ops that negate do not skip the `quant` (§3.6.1). With
    that, gcc without those flags and clang give 0 differences over the 212 programs, and the flags were removed. The
    real DSP has no NaN.
* Debugging: `cchain.Slave(..., trace=f)` calls `f(dsp, instruction, regs, flags, pm, pv)` after each instruction, in
  the interpreter and in the native code (compiled separately with the calls).

**Point 3 not done, on purpose:** a DSP without voices keeps changing its state (envelopes, memory rings), so skipping
it cannot give the same bits; and it is no longer needed for the real time of one slave. For two slaves (step 8) the
exact option is to run them in parallel (12 threads) in C.

### 3.6.1 More speed (2026-10-23) and the model of the SYSTEM-1

Measured with C-S1SY step 11 (SYSTEM-1: slave A + slave B + master B, 1264 instructions per sample, native, one
thread, 96 000 samples = 1 s of audio). All the changes give the same bits: the whole `test_core.py` (instruction
corpus, JX-3P chain, 212 native programs against the interpreter) and `make_synth.py --sim-check`:

| change | 1 s of audio | M instr/s |
|---|---|---|
| before | 1.207 s | 101 |
| `pymod` with exact fast paths for −m ≤ x < 2m (`ph_wrap` / `ph_tri` of a phase; `test_pymod` against `fmod`) + `-fno-math-errno` | 0.872 s | 139 |
| `quant` as a bit mask of the mantissa, instead of scaling and converting to integer | 0.588 s | 206 |
| the mask precomputed per exponent: `qmask.h` (`gen_qmask.py`), one load and one AND | **0.341 s** | **356** |

* `quant` was ≈ 80 % of the time: it is in the dependency chain of each instruction. The mask clears the k = 20 + 4s − E
  low bits of the mantissa (k > 52 → a signed zero; |v| ≥ 512, inf, nan: nothing). It is the same rule as the scaling
  (the conversion to integer of m·2^p is its floor). `test_quant` compares it with `quant_ref` and with `dspisa.quant`.
* **Model of the SYSTEM-1 in a single `Slave`.**
  * `Model.shared5 = ((0, 1),)`: one bank 5 (`A000`–`A3FF`) for two DSPs, like the `A1xx` that slave A writes and B
    reads.
  * `Model.own_params = (0, 1, 2)`: one bank 6 per DSP (`FCEC(sel, …)`). `cchain.DspView.params` is the bank 6 of that
    DSP.
  * `dspjit.groups` puts in the same thread the DSPs that share bank 5. `chain.py` implements the two options the same
    way; `test_s1_shared5` compares the C with `chain.py` sample by sample.
  * `Slave.run(n, [(dsp, address)])` records words of any DSP (for example `A118` of the master), not only of IOMD.
* `make_synth.py simulate()` uses that model: one call to C per stretch between events, instead of a Python loop per
  sample. Before, it was two `Slave`s and copies of bank 5 in each sample. 1 s of audio: from 1.87 s to **0.43 s** (real
  time × 2.3), with 0.08 s of preparation.
* SYSTEM-8: the JX-3P slave (6 threads) goes from 1.23× to **2.05× real time** (170 M instr/s; `test_speed`).
* New tests: `test_pymod` (fast paths against `fmod`) and `test_s1_shared5` (C against `chain.py` with `shared5` /
  `own_params` and C-S1SY step 11). `test_core.py` passes 9/9 and `test_chain.py` 20/20 (2026-10-23).
* `dspcomp.decode_program` keeps the decoding in the `Program`: before, it was decoded three times per `Slave`
  (`compile_program`, `translate` and `groups`).

**Second round (2026-10-23, later)**, same bits (`test_core.py` 10/10, `--sim-check`). The complete simulation of the
SYSTEM-1 (`make_synth.simulate`, 1 s of audio):

| change | 1 s of audio | what it does |
|---|---|---|
| before the second round (one thread, model of the SYSTEM-1) | 0.57 s | — |
| **pipeline A ∥ B** (`cchain.pipeline_plan`, `sl_set_pipeline`) | 0.29 s | slave A computes sample t + 1 in its own thread while B and the master compute t |
| **no `quant` after the exact ops** (`dspjit.exact_dest`) | **0.24 s** (4× real time) | `min`/`max`/`abs`/`clamp`/`neg`/copies of unit B with inputs on the grid: the result is already on it |

* **Pipeline.** Each DSP carries its own ring clock (`Dsp.t`): that of stage A goes one sample ahead.
  * Stage B reads a buffer of the shared bank 5. At each barrier only the words that A writes are copied to it (40 in
    C-S1SY). When `sl_step` finishes, those written by the other DSPs (6) go back to the shared bank.
  * `pipeline_plan` only enables it if it gives the same bits:
    * a single `shared5` pair, with A before B;
    * latched MXM;
    * A does not touch IOMD, MXM, the pair window or a shared bank 1;
    * there are no indexed transfers to bank 5;
    * A does not read or write words that the others write.
  * `test_s1_pipeline` compares pipeline and serial, word by word, over 6000 samples with notes and LFO.
* **Exact ops.** Of the quantisations that remain, almost all are of freshly loaded registers, and they are mandatory: an
  op that uses a load reads the raw value and the register is quantised afterwards (§3.6.1 above; rule 6 of
  [Vol. III ch. 6](../vol3_voice_allocation_scheduling/microcode_optimization.md)). Without any `quant` it would be 0.13 s, but it would not be
  exact.
* **Compiler:** `DSP_CC=gcc|clang` (by default clang if it is installed). Same performance within the noise (S1: 0.24 s
  with both; JX-3P: 2.3× / 2.45× real time). Neither needs `-fno-gcse -fno-peephole2` any more (it was a NaN, §3.6). The
  compiler's name enters the hash of the cache and there is one `dspcore-<cc>.so` per compiler.
* **Faster Python reference, without changing its semantics:** `dspisa.layout()` caches the `Layout` per words, `Imm`
  keeps the bit string and `quant` is memoised (without the zeros: 0.0 and −0.0 are the same key). The reference goes
  from ≈ 83 to ≈ 30 ms per sample.
* **Simulations in parallel:** `make_synth.run_sims` runs each independent simulation in its own process (`SIM_JOBS`, by
  default all the cores). The JIT cache admits several processes at once (temporaries per PID) and the native code is
  compiled in the parent before the fork. `--sim-check`: from ≈ 2.5–3 min to **27 s**; `--sim`: 2.8 s.
* **Preparation:** `dspcomp.decode_program` keeps the decoding per words for the whole process: 0.084 → 0.037 s per
  simulation.

What remains in Python:
* (until 2026-10-29: the reference `dspisa`; archived, the reference is the C, §1.1);
* the preparation (≈ 0.04 s);
* `Bmc2` (the two slaves of the SYSTEM-8, a Python loop per sample; only `test_chain.py` uses it). Moving it to C, with
  the two slaves in parallel, is the next thing for the SYSTEM-8.

Possible next improvements, in order of profitability [inferred, not measured]:

1. ~~**No `quant` after the exact ops.**~~ Done (above). What follows is the original version of the idea: `min`, `max`,
   `mov rX`, `ph_clamp`, `ph_abs` and `ph_neg` return one of their inputs, or ±1, or −x. If the inputs are on the grid,
   the result is too. It needs an analysis in `dspjit.translate`. Without `quant` the time is 0.13 s instead of 0.34,
   although only part of the ops is exact.
2. **Live audio.** A player that runs `Slave.run()` in blocks of, for example, 256 samples and sends them to ALSA, with
   the notes by MIDI from the PC. That way an own PLUG-OUT can be heard without installing it.
3. **The two slaves of the SYSTEM-8 in parallel** (12 threads, §3.6 point 3).
4. **The ARM side** (`engine.c` compiled for the PC, with the OS services simulated) so that the MIDI messages reach the
   DSPs as on the device (§3.1).

### 3.7 Validation

Compare the render with captures of the device, note by note: frequency, level, envelope. References: C-SAWPOLY
(Roland's chain with our saws), the stock JX-3P in a slot, C-V4. Tools: `voicelog.py` (assignment + audio),
`re/dspaudio.py`.

**Native ops against the device** (`re/dspnative.py`, SYSTEM-8 master with the PEEK probe, 2026-10-28): each op with
random registers, with and without an empty instruction before storing, compared word by word with the reference (until
2026-10-28 `re/dspisa.py run`, from 2026-10-29 the C through `emu/dspref.py`). What was corrected in `emu/dspcore.h`
(and, before the port, in `dspisa.py`) and `emu/dspjit.py` (`writes`):

| op | before in the emulator | on the device |
|---|---|---|
| `shl` / `shr` / `shrn` | X·2^k without a limit, truncated toward 0 | two's complement with 32 fraction bits: rounds toward −∞ at 2^−32, wraps into [−512, 512); then the RAW encoding truncates toward 0 (2026-10-29: 672/672 raw cases; the "toward −∞ on the RAW grid" of 2026-10-28 was only right with the result on the 2^−32 grid) |
| `recipseed` | wrote r(d), r(d+1) | also rY ← m; r(d) = 0 if 2^−e ≥ 512 |
| `expsplit` | exact e^⌊Y⌋ | measured ROM (`EXP_ROM`, n = −19…6), 0 below, saturates above |
| SFU mode 3, bit 3 | −(Y < 0) | ⌊Y⌋ (`floor`) |
| `log2split`, Y < 0 | Y/512 truncated | Y read in two's complement: e = ⌊log2((Y + 512)/0.75)⌋, m = Y/2^e rounded toward −∞ at 2^−32 and truncated by the encoding (2026-10-29: 120/120 raw; e = 9 only for \|Y\| < 115) |
| `sqrtsplit` | wrote r7 ← ±2^−k, 8-bit seed | rY ← m = \|Y\|·4^k ∈ [1, 4), r(d+1) ← Q[⌊(m−1)·16⌋]/512 with Q = round(512/√(middle of the interval)) (9 bits), **r(d+3)** ← sign·2^−k (2026-10-29: 384 raw + 3072 in `dspnative`). With Y in r(d+1) or r(d+3) the writes mix bits: `dsp_check` marks it unmeasured |
| MAC with D ≥ 8 | /2 always | /2 only with a P2 term (op1 ≥ 4); without P2 it writes nothing (2026-10-29) |
| two MACs (class 0x080) | both write | a P2 term in either of the two: none writes (2026-10-29) |
| immediate coefficient of the MAC | after the immediate of Y (Y immediate) | at byte C of the slot array, +8 with bit 9 of N; Y register or immediate, with or without transfers (19 classes, 2026-10-29) |

After the changes: 0 differences in 96 cases per op (`dspnative.py -n 96`), `test_core.py` 11/11 (including the 212
Roland programs in the native translation).

## 4. Proposed order

| step | what | expected result |
|---|---|---|
| 1 | **done**: DSP core in C with the interface of `chain.py`; pass `test_chain.py` with it; plus native code + threads (§3.6) | 20/20 tests in ≈ 5 s (9 min 36 s before); `test_core.py`: 15 756 instructions and the JX-3P chain bit-identical to `chain.py`; one JX-3P slave at 1.23× real time |
| 2 | Loader of Roland's programs from `programs.h` / `region1.bin` (reuse `emu/listing.py`, `re/alloc_scan.py`) | MAIN0–5 and the DCOs of the JX-3P loaded in the emulator |
| 3 | Measure and model bank 6 (§3.2) | ramps and envelopes in the emulator |
| 4 | Resolve the load latency of the ops (§3.3) | `dspisa` and the C core corrected |
| 5 | Connect `re/emu1.py` to the DSPs (§3.1): Roland's ARM engine controls the emulator | a MIDI note → voice at `6020` |
| 6 | JX-3P without effects: render of the eight notes of `voicelog` and comparison with a capture | tuning and envelope equal to the capture |
| 7 | Bank 2 / delay line; effects of the JX-3P | chorus and delay |
| 8 | Region 0 in the loop (real assigner), several slaves | polyphony as on the device |
| 9 | (optional) Master: global-effects DSP and wave bus | final output |

Each step must have a test that can fail (like `test_chain.py`). Before each measurement on the device: one installation
at a time, the user selects PLG3 again, never two MIDI tools at once.

## 5. State as of 2026-10-22 and what is missing

### 5.1 Done (uncommitted; the user makes the commits)

| what | where | test |
|---|---|---|
| `dspisa.run()` split into `dspisa.Layout` (static part) + execution; same behaviour | `re/dspisa.py` | 148 722 executions of the corpus equal to the previous version (script of the session, not kept) |
| C core: interpreter + memory of `chain.py` | `emu/dspcore.h`, `emu/dspcore.c`, `emu/dspcomp.py`, `emu/cchain.py` | `test_core.py insn` (≈ 5 min), `jx3p`, `ours`, `quant` |
| translation to native + threads per DSP group | `emu/dspjit.py` | `test_core.py native` (≈ 2 min), `speed` |
| `test_chain.py` on the C core (`--py` = `chain.py`) | `emu/test_chain.py` | 20/20 in ≈ 5 s |
| debug trace per instruction | `cchain.Slave(..., trace=f)` | used to isolate the GCC failure |

Commands: `cd examples/from_scratch/emu; python3 test_chain.py; python3 test_core.py` (everything, ≈ 8 min; it needs
`work/corpus*` and `engine/build/programs.h` locally, if they are missing those tests are skipped).

### 5.2 Open in what is done

* **GCC 14 `-O2` failure** (§3.6): no cause found; avoided with `-fno-gcse -fno-peephole2`. If the compiler or the flags
  change, run `test_core.py native`. To follow it up: the case is `p0s_003661c` (local corpus), random state of
  `test_native_corpus` (seed 11), instruction 86 in the third sample; with the trace on it does not appear.
  > Resolved on 2026-10-23 (§3.6 above): it was the sign of a NaN; the flags were removed.
* Speed: the ceiling with threads is the most loaded DSP (DSP2/3, ≈ 840 instr./sample); `quant` and the memory accesses
  (`rd`/`wr`, with stamp checks) are what remains in the profile.
* Point 3 of §3.6 (skipping DSPs without voices) **is not done**: it cannot be bit-identical.
* Two slaves: `cchain.Bmc2` advances sample by sample from Python (without threads). Step 8 needs a `run` in C of
  several slaves with the link `Model.link`, in parallel (12 threads).
* `asm/` (format layer of the assembler) and `emu/dspdec.h` are still two decoders; unifying them is a separate task
  (compare both over the corpus first).

* **The complete chain of the JX-3P does not give valid audio** (2026-10-22, `examples/system_1_jx`,
  [examples/system_1_jx/README.md](../../examples/system_1_jx/README.md) §7): with the complete model and the INIT PATCH, the output `6020` of MAIN4 is 0,
  huge (511) or NaN depending on the cutoff (sweep 0–255). The filter coefficient comes from `expsplit` + `newton r2,
  #…` + a polynomial (instructions ≈ 302–347 of MAIN4) and gives values out of range (−9.6). DCO1 starts with the same
  sequence and, with the current semantics of `newton` ([Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §5.1: it reads `r2`·`r3`), it depends on the
  registers the previous program leaves. One of two things: `newton` with an immediate has other semantics, or Roland
  chains registers between programs. Unresolved; the tests of C-S1JX compare programs with each other (bit for bit) and
  do not depend on it.

* **Access limits per instruction** (2026-10-23): the interpreter does all the accesses of an instruction, but the
  hardware does not (bank 5 ≤ 1 load and ≤ 1 store, bank 0 ≤ 2 loads and ≤ 2 stores, bank 6 ≤ 2 loads). Programs that
  passed the emulator hung the SYSTEM-1. From now on `chain.Program` (and therefore `cchain`) **rejects** a program that
  breaks those limits (`chain.access_violations`, `STRICT`; `strict=False` only for test harnesses that do not go to the
  device). Checked against the installed builds: it rejects exactly those that hung for that reason
  (`examples/system_1_jx/test_s1jx.py` `device_history`).

### 5.3 Next: step 2 of §4 (loader of Roland's programs)

Today `test_core.jx_programs()` builds the JX-3P by hand: MAIN0–5 in DSP0–5 and `DCO1_SAW` + `DCO2_SAW` in the slots of
DSP2 and DSP3. Missing:

1. read from `programs.h` / `region1.bin` which programs there are and their type (main, DCO1, DCO2, effect) with
   `re/alloc_scan.py` / `emu/allplugouts.py`;
2. model the slots as the ARM engine registers them (`+0x28/+0x40/+0x58`, `+0x70 swap`: DCO1 = task 2, DCO2 = 3,
   effect = 3; §3.1), with the change of program between samples;
3. a test that can fail: for example, that the JX-3P loaded this way gives the same as `jx_programs()`, and that
   changing the waveform (SAW → SQR) changes the output.

Then: step 3 (bank 6, measurement on the device with the user), step 4 (load latency, measured), step 5 (`emu1.py`
connected to the core).

## 6. References

[S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6 (model of the slave, current emulator), [Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §7–§10 (memory,
communication, execution model), [Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) (services and banks of the ARM), [Vol. VI ch. 1](../vol6_reference_designs_applications/c_v4_system8_reference.md) (C-V4),
[NOTES.md](../vol7_cleanroom_evidence_archive/NOTES.md) ("Posibles próximos pasos: emulador", original plan with Unicorn and JIT).

