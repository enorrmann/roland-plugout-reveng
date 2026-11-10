# Volume V — Chapter 5: Emulator of a Complete PLUG-OUT — What Exists, What Is Missing and in Which Order

> **Scope:** both: the DSP core is common; the slave chain models are SYSTEM-8, the build simulators (`--sim`) of the SYSTEM-1 PLUG-OUTs are SYSTEM-1.

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
| `examples/from_scratch/emu/dspjit.py` + `dspcore.h` | translation of each program to native code and groups of threads per sample (§3.6 point 2): one JX-3P slave at 2.3–2.45× real time | — |
| `examples/from_scratch/emu/test_core.py` | the 15 756 distinct instructions of the local corpus against the golden file `work/golden_insn.txt` (hashes only, local), JX-3P native against interpreter, the 212 programs of the corpus in native against the interpreter, native with threads against serial, `quant` against its definition, v3/v4, speed | the golden file is regenerated (`--update-golden`) only after a measured change of the semantics |
| `examples/from_scratch/re/emu1.py`, `tools/engine_emu.py` | region 1 (ARM engine) in Unicorn, OS services `0x0113FC00` intercepted | they only log the calls; they do not connect them to any DSP |
| `examples/from_scratch/re/emu0.py` | region 0 (model, assigner) in Unicorn | without MIDI input or link to the slaves |
| `examples/from_scratch/emu/listing.py` | reads Roland's programs from `programs.h` and disassembles them | static reading only |
| `voicelog/voicelog.py`, `re/dspaudio.py` | captures of the device for comparison | — |

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
| Instructions "inferred"/"fitted" of the disassembly (MAIN4: 56 inferred, 10 fitted) | possibly different results | sweep with `re/dspverify.py` of the affected classes |
| Undecoded transfer modes: `9e`, `e0`/`e1`, `d8`/`d9`/`dc`/`dd` | memory accesses that do nothing today | `re/dspmodescan.py`, tests on the master |
| Bank 7 (`e013`, the JX reads it as a source) | noise, counter? | read the word on the master over several consecutive samples |
| Floating-point arithmetic with the truncation rule | accumulated rounding errors | core in integers with a 29-bit mantissa (§3.6) |
| Flags and jumps per DSP | already kept per DSP in `chain.py` | — |

The load latency is modelled as measured: the next instruction sees the loaded value
([Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §7.3).

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

A Roland PLUG-OUT is ≈ 11 000 instructions per sample on the SYSTEM-8 (6 DSPs × ≈ 930 × 2 slaves); the JX-3P on one
slave (MAIN0–5 + DCO1/DCO2 SAW on DSP2 and DSP3) executes **3 621 instructions per sample** (there are jumps). Speed
reached, with the same bits as the interpreter [code, `emu/test_core.py`, 2026-10-23]:

| simulation | speed |
|---|---|
| JX-3P, one SYSTEM-8 slave (6 threads, native) | **2.3× (gcc) / 2.45× (clang) real time** |
| SYSTEM-1, C-S1SY (slave A + slave B + master B, 1264 instructions per sample; `make_synth.simulate`) | 1 s of audio in **0.24 s** (4× real time), with ≈ 0.04 s of preparation |
| `make_synth.py --sim-check` / `--sim` | 27 s / 2.8 s |

How:

* **C core** (`emu/dspcore.h` semantics, `emu/dspcore.c` interpreter), compiled with `-ffp-contract=off`: a fused
  multiply-add would change the bits (each product is rounded).
* **Native code**: `emu/dspjit.py` translates each program to a C function (words as constants, jumps as `goto`; the
  units call the same inline functions as the interpreter, so the semantics are in one place). Each program is an
  object of its own, compiled in parallel and cached in `emu/build/jit/` by the hash of its source and of the compiler
  (`DSP_CC=gcc|clang`, clang by default if installed; one `dspcore-<cc>.so` per compiler). `cchain.Slave(...,
  native=True, threads=True)` is the default.
* **`quant`** (the rounding of a register to the DSP grid) as a bit mask of the mantissa precomputed per exponent
  (`qmask.h`, `gen_qmask.py`: one load and one AND; it clears the k = 20 + 4s − E low bits; k > 52 → a signed zero;
  |v| ≥ 512, inf, nan: nothing); `test_quant` compares it with `quant_ref`. The translator only quantises registers that
  can be off the grid (`quant` is idempotent) and skips it after the exact ops (`dspjit.exact_dest`: `min`/`max`/`abs`/
  `clamp`/`neg`/copies of unit B with inputs on the grid). The quantisations of freshly loaded registers are mandatory:
  an op that uses a load reads the raw value and the register is quantised afterwards (rule 6 of
  [Vol. III ch. 6](../vol3_voice_allocation_scheduling/microcode_optimization.md)).
* **`pymod`** with exact fast paths for −m ≤ x < 2m (`ph_wrap` / `ph_tri` of a phase; `test_pymod` against `fmod`) and
  `-fno-math-errno`.
* **NaN**: `quant` returns a canonical NaN (+qNaN). C does not fix which NaN `a + b` returns with NaN operands and the
  compiler can swap them, so a `sign` op could turn the NaN's sign into ±1; the canonical NaN makes gcc and clang give
  0 differences over the 212 programs without special flags. The real DSP has no NaN [measured, 2026-10-23].
* **Threads**: one group of DSPs per thread inside each sample (`dspjit.groups`). The DSPs that share an IOMD word or
  bank 1 (`Model.shared1`) or bank 5 go together; with MXM in the same sample or the immediate pair window, everything
  goes in one thread. The pair window of bank 5 is merged at the end of the sample in the model's order, so the result
  does not depend on the threads. The JX-3P gives six groups; the ceiling is the most loaded DSP (DSP2/3, ≈ 840
  instr.). Threads are used in `run()` / `step(n)` with n ≥ 64.
* **Model of the SYSTEM-1 in a single `Slave`**: `Model.shared5 = ((0, 1),)` (one bank 5 `A000`–`A3FF` for two DSPs),
  `Model.own_params = (0, 1, 2)` (one bank 6 per DSP, `cchain.DspView.params`); `Slave.run(n, [(dsp, address)])`
  records words of any DSP (for example `A118` of the master). `make_synth.py simulate()` makes one call to C per
  stretch between events. `test_s1_shared5` compares it with the archived `chain.py` sample by sample.
* **Pipeline A ∥ B** (`cchain.pipeline_plan`, `sl_set_pipeline`): slave A computes sample t + 1 in its own thread while
  B and the master compute t; each DSP carries its own ring clock (`Dsp.t`). Stage B reads a buffer of the shared bank
  5; at each barrier only the words A writes are copied to it (40 in C-S1SY), and those written by the other DSPs go
  back to the shared bank. `pipeline_plan` enables it only if it gives the same bits: a single `shared5` pair with A
  before B, latched MXM, A not touching IOMD, MXM, the pair window or a shared bank 1, no indexed transfers to bank 5,
  and A not reading or writing words the others write. `test_s1_pipeline` compares pipeline and serial word by word
  over 6000 samples with notes and LFO.
* **Simulations in parallel**: `make_synth.run_sims` runs each independent simulation in its own process (`SIM_JOBS`,
  all cores by default); the JIT cache admits several processes (temporaries per PID) and the native code is compiled
  in the parent before the fork. `dspcomp.decode_program` keeps the decoding per words for the whole process (0.037 s
  of preparation per simulation).
* **Debugging**: `cchain.Slave(..., trace=f)` calls `f(dsp, instruction, regs, flags, pm, pv)` after each instruction,
  in the interpreter and in the native code (compiled separately with the calls).

Not done: skipping DSPs without voices (a DSP without voices keeps changing its state — envelopes, memory rings — so
skipping it cannot give the same bits). In Python remain the preparation (≈ 0.04 s) and `Bmc2` (the two slaves of the
SYSTEM-8, a Python loop per sample; only `test_chain.py` uses it).

Possible next improvements [inferred, not measured]:

1. **The two slaves of the SYSTEM-8 in C, in parallel** (12 threads, with the link `Model.link`).
2. **Live audio.** A player that runs `Slave.run()` in blocks of, for example, 256 samples and sends them to ALSA, with
   the notes by MIDI from the PC. That way an own PLUG-OUT can be heard without installing it.
3. **The ARM side** (`engine.c` compiled for the PC, with the OS services simulated) so that the MIDI messages reach the
   DSPs as on the device (§3.1).

### 3.7 Validation

Compare the render with captures of the device, note by note: frequency, level, envelope. References: C-SAWPOLY
(Roland's chain with our saws), the stock JX-3P in a slot, C-V4. Tools: `voicelog.py` (assignment + audio),
`re/dspaudio.py`.

**Native ops against the device** (`re/dspnative.py`, SYSTEM-8 master with the PEEK probe, 2026-10-28): each op with
random registers, with and without an empty instruction before storing, compared word by word with the C reference
(`emu/dspref.py`). The behaviour measured on the device and modelled in `emu/dspcore.h` and `emu/dspjit.py` (`writes`):

| op | on the device (= the model) |
|---|---|
| `shl` / `shr` / `shrn` | two's complement with 32 fraction bits: rounds toward −∞ at 2^−32, wraps into [−512, 512); then the RAW encoding truncates toward 0 (2026-10-29: 672/672 raw cases) |
| `recipseed` | writes r(d), r(d+1) and rY ← m; r(d) = 0 if 2^−e ≥ 512 |
| `expsplit` | measured ROM (`EXP_ROM`, n = −19…6), 0 below, saturates above |
| SFU mode 3, bit 3 | ⌊Y⌋ (`floor`) |
| `log2split`, Y < 0 | Y read in two's complement: e = ⌊log2((Y + 512)/0.75)⌋, m = Y/2^e rounded toward −∞ at 2^−32 and truncated by the encoding (2026-10-29: 120/120 raw; e = 9 only for \|Y\| < 115) |
| `sqrtsplit` | rY ← m = \|Y\|·4^k ∈ [1, 4), r(d+1) ← Q[⌊(m−1)·16⌋]/512 with Q = round(512/√(middle of the interval)) (9 bits), **r(d+3)** ← sign·2^−k (2026-10-29: 384 raw + 3072 in `dspnative`). With Y in r(d+1) or r(d+3) the writes mix bits: `dsp_check` marks it unmeasured |
| MAC with D ≥ 8 | /2 only with a P2 term (op1 ≥ 4); without P2 it writes nothing (2026-10-29) |
| two MACs (class 0x080) | a P2 term in either of the two: none writes (2026-10-29) |
| immediate coefficient of the MAC | at byte C of the slot array, +8 with bit 9 of N; Y register or immediate, with or without transfers (19 classes, 2026-10-29) |

Result: 0 differences in 96 cases per op (`dspnative.py -n 96`), `test_core.py` 11/11 (including the 212
Roland programs in the native translation).

## 4. Proposed order

| step | what | expected result |
|---|---|---|
| 1 | **done**: DSP core in C, native code + threads (§3.6); `test_chain.py` 20/20 in ≈ 5 s; `test_core.py`: the 15 756 distinct instructions of the corpus and the JX-3P chain bit-identical | — |
| 2 | Loader of Roland's programs from `programs.h` / `region1.bin` (reuse `emu/listing.py`, `re/alloc_scan.py`) | MAIN0–5 and the DCOs of the JX-3P loaded in the emulator |
| 3 | Measure and model bank 6 (§3.2) | ramps and envelopes in the emulator |
| 4 | **done**: load latency of the ops measured and modelled (§3.3) | — |
| 5 | Connect `re/emu1.py` to the DSPs (§3.1): Roland's ARM engine controls the emulator | a MIDI note → voice at `6020` |
| 6 | JX-3P without effects: render of the eight notes of `voicelog` and comparison with a capture | tuning and envelope equal to the capture |
| 7 | Bank 2 / delay line; effects of the JX-3P | chorus and delay |
| 8 | Region 0 in the loop (real assigner), several slaves | polyphony as on the device |
| 9 | (optional) Master: global-effects DSP and wave bus | final output |

Each step must have a test that can fail (like `test_chain.py`). Before each measurement on the device: one installation
at a time, the user selects PLG3 again, never two MIDI tools at once.

## 5. State and what is missing

### 5.1 Done

| what | where | test |
|---|---|---|
| C core: interpreter + memory of `chain.py` | `emu/dspcore.h`, `emu/dspcore.c`, `emu/dspcomp.py`, `emu/cchain.py` | `test_core.py insn` (≈ 5 min), `jx3p`, `ours`, `quant` |
| translation to native + threads per DSP group | `emu/dspjit.py` | `test_core.py native` (≈ 2 min), `speed` |
| `test_chain.py` on the C core (`--py` = `chain.py`) | `emu/test_chain.py` | 20/20 in ≈ 5 s |
| debug trace per instruction | `cchain.Slave(..., trace=f)` | used to isolate the GCC failure |

Commands: `cd examples/from_scratch/emu; python3 test_chain.py; python3 test_core.py` (everything, ≈ 8 min; it needs
`work/corpus*` and `engine/build/programs.h` locally, if they are missing those tests are skipped).

### 5.2 Open in what is done

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
  passed the emulator hung the SYSTEM-1. `chain.Program` (and therefore `cchain`) **rejects** a program that
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

Then: step 3 (bank 6, measurement on the device with the user), step 5 (`emu1.py` connected to the core).

## 6. References

[S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6 (model of the slave, current emulator), [Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §7–§10 (memory,
communication, execution model), [Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) (services and banks of the ARM), [Vol. VI ch. 1](../vol6_reference_designs_applications/c_v4_system8_reference.md) (C-V4),
[NOTES.md](../vol7_cleanroom_evidence_archive/NOTES.md) ("Posibles próximos pasos: emulador", original plan with Unicorn and JIT).

