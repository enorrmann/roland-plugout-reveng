# Volume V — Chapter 3: `sdk/mport` — Porting Mutable Instruments DSP Code to the Microcode of Roland's BMCs

> **Scope:** SYSTEM-1 (MUTABLE-SYSTEM).

> For new ports the tool is s1cc ([ch. 1](s1cc_compiler_internals.md)); mport is the study and the SAW_SWARM bench.

Specification. State: 2026-10-24, version 1.0.

`sdk/mport` is a set of Python modules to bring oscillators, filters and effects from the Mutable Instruments repository
(`~/git/eurorack`, MIT) to our microcode. The ISA is the same on the SYSTEM-1 and on the SYSTEM-8 ([Vol. II ch. 7](../vol2_dsp_core_isa/isa_overview.md)).
It does four things:
1. Translates the per-sample loop into microcode (`kernel.py`).
2. Brings to the build what the original computes per block (`component.py`, `lut.py`).
3. Compiles the original to have a reference (`reference.py`).
4. Measures the port against that reference in the emulator (`bench.py`, `compare.py`, `cli.py check`).

First ported component: **Braids' SAW_SWARM** (§7), which the example MUTABLE-SYSTEM uses
([examples/mutable_system/README.md](../../examples/mutable_system/README.md)). What else can be ported:
[examples/mutable_system/CANDIDATES.md](../../examples/mutable_system/CANDIDATES.md).

Tags: **[measured]** on the device; **[sim]** in the emulator (C core); **[code]** read in Mutable's source; **[open]**.

## 1. Files

| file | what it is |
|---|---|
| `sdk/mport/kernel.py` | `Kernel`: the per-sample loop written with values; allocates registers and generates lines of one unit (§3) |
| `sdk/mport/mi.py` | Mutable's fixed-point conventions → our numeric model; `on_grid`; stmlib's LCG (§2) |
| `sdk/mport/lut.py` | reads the tables of `<module>/resources.cc`; `Interpolate88` / `Interpolate824`; fitting of polynomials (§4) |
| `sdk/mport/component.py` | `Component` and `Param`: what a ported component is (§5) |
| `sdk/mport/components/` | ported components (`REGISTRY` in `__init__.py`); `braids_saw_swarm.py` (§7) |
| `sdk/mport/reference.py` | compiles the original C++ with g++ and runs it (§6.1) |
| `sdk/mport/ref/braids_render.cc` | reference program: `braids::DigitalOscillator` with shape, pitch and parameters from the command line |
| `sdk/mport/ref/stmlib/` | minimal substitutes of stmlib (`stmlib.h`, `utils/dsp.h`, `utils/random.h`): the submodule is empty in the eurorack checkout |
| `sdk/mport/bench.py` | a component alone on one DSP of the emulator (§6.3) |
| `sdk/mport/compare.py` | port ↔ reference metrics: SNR, f0, RMS, peak, levels per band (§6.4) |
| `sdk/mport/cli.py` | command line: `list`, `check`, `lines`, `lut` (§8) |
| `sdk/mport/tests/test_mport.py` | tests (§9) |
| `sdk/mport/build/` | the compiled reference executable (not in git) |

Dependencies: Python 3 with numpy; g++ and the eurorack checkout for the reference (`EURORACK`, by default
`~/git/eurorack`); `asm/` and the emulator of `examples/from_scratch/emu/` of the repository.

## 2. Numeric model (`mi.py`)

Our DSP works with signed numbers up to ±512, a 29-bit mantissa and a result truncated toward zero
([Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md)). It runs at 96 kHz, the same frequency as Braids. Each Mutable quantity has a fixed
equivalent:

| Mutable (Braids / Plaits) | here | function |
|---|---|---|
| `int16` sample | float −1…1 (x = s / 32768) | — |
| `uint32` phase, saw = `phase >> 19` | phase −1…1 wrapped with `ph_wrap`: p = phase / 2³¹ − 1 | `phase_from_u32` |
| `uint32` phase increment | 2·f / fs | `increment(hz)` |
| `int16` pitch = MIDI note << 7 (1/128 semitone) | pitch word = (note − 12) / 12 octaves (4.75 = A 440) | `braids_pitch`, `pitch_word` |
| parameter (TIMBRE / COLOR) 0…32767 | SYSTEM-1 knob 0…255 → v · 32767 / 255 | `param`, `knob` |
| Q15 product `a * b >> 15` | `a · b` in float | — |
| `CLIP` to int16 | `ph_clamp` (to −1…1) | — |
| tables | polynomial (per sample) or ARM table (per block) | `lut.py` |
| `stmlib::Random` | `Lcg` (same constants) | — |

Grid rule: a parameter used in the instruction right after its load has to be on the register's grid (multiples of
2⁻³² below 0.125). Otherwise the packed and the unpacked versions differ by 1 LSB
([Vol. III ch. 6](../vol3_voice_allocation_scheduling/microcode_optimization.md), rule 6). `Param.values` applies `on_grid` to every value.

## 3. `Kernel`: the per-sample loop

### 3.1 Use

```python
from kernel import Kernel
k = Kernel('voice 0', regs=(0, 1, 2, 3, 4, 7))       # registers it may use; r5 / r6 belong to the host program
p = k.state(0x0400)                                  # state word: loads 0x0401 (previous sample), stores 0x0400
inc = k.increment(k.load(0xA030))                    # 2^(pitch − 4.75) · 2 · 440 / 96000
q = k.wrap(k.add(p.value, inc))
p.write(q)
k.store(q, 0xA151)
lines = k.lines()                                    # 'ld r0, [a030]', 'mov r1, r1', 'addxy r1, r0, #-4.75', ...
```

Lines of one unit come out, with fillers `mov rN, rN` after the loads. It is the same form that the repository's
generators write by hand, so they are scheduled and validated the same way: `asm/dspsched.py`, `--pack-check`,
`tools/check_optimized.py`.

### 3.2 Operations

x is a value. y is a value, a negated value (`-v`, bit 3 of the register field) or a number (immediate).

| method | result | instruction |
|---|---|---|
| `load(addr)`, `param(idx)` | memory word / parameter `C0idx` | `ld rD, [addr]` |
| `store(v, addr)` | — | `st rV, [addr]` |
| `state(addr)` → `Ring` | `.value` loads `addr + 1`, `.write(v)` stores `addr` | bank-0 ring |
| `mov(y)` | y | `mov rD, Y` |
| `mul(x, y)` | x·y | `mul rD, rX, Y` |
| `add(x, y)` | x + y | `addxy rD, rX, Y` |
| `sub(x, y)` | x − y | `subxy rD, rY, X` (the op computes Y − X) or `addxy` with −y |
| `mad(acc, x, y)` | acc + x·y (the result takes the register of acc) | `mad rD, rX, Y` |
| `ymac(acc, x, y)` | y + x·acc: one Horner step | `ymacd rD, rX, Y` |
| `min(x, y)`, `max(x, y)` | minimum / maximum | `min` / `max` |
| `wrap(x)` | x wrapped to [−1, 1) | `ph_wrap rD, r3, rX` |
| `tri(x)` | 1 − 2·\|wrap(−x − 1.5)\| | `ph_tri rD, r3, -rX` |
| `clamp(x)` | x clipped to [−1, 1] | `ph_clamp rD, r3, -rX` |
| `horner(x, c)` | c₀ + x·(c₁ + x·(c₂ + …)) | `mul` + `addxy` + `ymacd`… |
| `poly_odd(x, a)` | x·(a₁ + a₃x² + a₅x⁴ + …) | the same on x² |
| `exp2(x)` | 2ˣ for \|x\| ≤ 8: (e^(x·ln2/16))¹⁶, 4 Taylor terms | the form of `make_synth.py` |
| `increment(pitch)` | phase step of a pitch word | `exp2` + scale |
| `comment(text)` | line `; text` | — |

In the phase ops, the X = r3 is Roland's encoding: `ph_*` does not read register r3. Only op types that already ran on
the SYSTEM-1 are emitted, the set `OPS` of `dspsched`.

### 3.3 Register allocation

* **Linear scan in program order.** A value lives from its line to its last read.
* **Reuse.** All the units of an instruction read before they write. That is why a result can take the register of an
  operand that dies in that same line.
* **Order of the free registers.** The one that has been free longest is taken. That way there are fewer false
  dependencies (WAR/WAW) and the scheduler has more room.
* **Accumulators.** `mad` / `ymacd` write into the accumulator's register. If that value is read again later, the kernel
  makes a copy with `mov` before.
* **Errors.** If no free register is left: `KernelError` with the list of live values. There is no spilling to memory
  (§10).
* **Fillers.** `lines(fillers=True)` puts `mov rS, rS` after a load when the next line uses that register (load
  latency 1). The scheduler removes them.

### 3.4 Verified example

Bench program of SAW_SWARM, unscheduled: `python3 sdk/mport/cli.py lines saw_swarm --plain`. The start (increment) and
the first saw are shown:

```dsp
ld r0, [a030]
mov r1, r1
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
mov r3, #0.041666666666666664
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r5, r4, r4
mul r6, r5, r5
mul r7, r6, r6
mul r0, r7, #0.009166666666666667
ld r1, [c040]
mov r0, r0
mul r2, r0, r1
ld r3, [0101]
mov r0, r0
addxy r4, r3, r2
ph_wrap r5, r3, r4
st r5, [0100]
mul r6, r5, #0.125
```

## 4. Tables (`lut.py`)

Mutable's modules read their curves from `int16` / `uint16` tables of `resources.cc`, which `resources/*.py`
generate. Here they are treated according to when they are read:

| use of the table | what is done | example |
|---|---|---|
| **per sample** (waveshaper, sine, window) | polynomial fitted by least squares (`fit_odd`, `fit`); the error is given in LSB of the int16 output | `ws_moderate_overdrive` → odd polynomial of degree 9 |
| **per block** (coefficient that only depends on a knob or on the note) | evaluated in Python at build time, for the 256 values of the knob, and goes to an ARM table | `lut_svf_cutoff` → exact formula on the increment (§7.2) |

A per-phase read of a table in the DSP needs a computed index. On the SYSTEM-1 that is [open]: it is step 15 of C-S1SY,
with the taps silent.

API:

| function | what it does |
|---|---|
| `tables(module)` / `table(module, name)` | arrays of `<EURORACK>/<module>/resources.cc` |
| `interp88(t)` | `Interpolate88(t, x·32768 + 32768) / 32768` as a function of x ∈ [−1, 1) |
| `interp824(t)` | `Interpolate824(t, phase) / 32768` as a function of the phase ∈ [0, 1) |
| `fit_odd(f, degree, x_max)` | coefficients (a₁, a₃, …) and maximum error in LSB |
| `fit(f, degree, x_min, x_max)` | coefficients c₀… (order of `Kernel.horner`) and maximum error |
| `scaled_odd(c, s)` | the same polynomial for a scaled input |

`ws_moderate_overdrive` (tanh(2x), Braids) on x ∈ [−0.875; 0.875], which is the range of SAW_SWARM [sim]:

| degree | maximum error (LSB) | ops |
|---|---|---|
| 3 | 1858 | 3 |
| 5 | 434 | 4 |
| 7 | 97 | 5 |
| 9 | 21.6 | 6 |
| 11 | 4.8 | 7 |

## 5. Component (`component.py`)

A Mutable `Render()` function is split into two parts:

| part | in Mutable | here |
|---|---|---|
| **control** | what `Render()` computes once per block of 24 samples from `pitch_` and `parameter_[]` | `Param(name, fn, count)`: fn(knob 0…255) → float or tuple; `Param.table()` = 256 × count floats for the ARM, which writes them with `FCEC` into `C0xx` |
| **audio** | the per-sample loop | `Component.kernel(k, ins, prm, st)` → `{output: Value}`; `ins`: inputs (for example `inc`); `prm[name](i)`: value `i` of the parameter; `st[name]`: `Ring` |

Attributes of a component: `name`, `source` (file and function of origin), `params`, `state` (names of the state
words), `inputs`, `outputs` and `reference(note, knobs, n)`.

A component can give **parts**: methods with the same arguments that the host places on different DSPs. SAW_SWARM has
`osc()` (DSP A), `hp_f()` and `hp()` (DSP B).

## 6. Verification

Three independent comparisons:

```
original C++  ==(bit for bit)==  integer model in Python  --(float, without the original's bugs)-->  port in the emulator
```

### 6.1 C++ reference (`reference.py`, `ref/`)

* **Compilation.** `braids_render.cc` is compiled with `braids/digital_oscillator.cc` and `braids/resources.cc` of the
  checkout, unchanged. Command: `g++ -O2 -I sdk/mport/ref -I $EURORACK`. The executable is only rebuilt if a source is
  newer.
* **Use.** `braids_render SHAPE PITCH P1 P2 N [SEED] [BLOCK] [STRIKE]` writes N int16 samples to standard output.
  `BLOCK` is 24 by default, like Braids.
* **`STRIKE=0`.** The state starts at 0, with the phases of the saws at 2³¹ (our phase 0), instead of with the random
  phases of the first render. It is needed to compare sample by sample. To reach the private state, only the reference
  program uses `#define private public`.
* **stmlib substitutes.** They implement `CLIP`, `CONSTRAIN`, `Interpolate824/88/1022/115`, `Crossfade`, `Mix` and
  `Random` with stmlib's semantics: difference times fraction and shifts that truncate. They are verified only on the
  paths of SAW_SWARM, because the integer model matches the C++ bit for bit (§6.2). Plaits, Rings and Elements also
  need `stmlib/dsp/*` [open].

### 6.2 Integer model (`braids_saw_swarm.int_model`)

It is the original algorithm written in Python with integers: `ComputePhaseIncrement`, the interpolation of the
increments, `Interpolate88` / `Interpolate824` and the SVF with `>> 15`. It serves two things:
* proving that we understood the original: it is **bit-identical** to the C++ in all the cases tested [sim];
* isolating the original's bugs. `wrap=False` removes the int32 overflow of §7.3, and the port is compared with that
  version.

### 6.3 Bench in the emulator (`bench.py`)

A component alone on DSP 0 of the C core (`cchain.Slave`, `Model(order=(0,), own_params=(0,))`). Layout:

| what | where |
|---|---|
| pitch | bank 5 `A030`, float32 as the ARM writes it |
| parameters | `C040`… in the order of `Component.params` |
| state | bank 0 `0x0100`…, 2 words per name |
| outputs | bank 5 `A1F0`… |
| `inc` | `Kernel.increment(pitch)` |

`bench.run(comp, note, knobs, n, sched=True)` returns (samples, `{'instructions', 'units', 'program'}`). The bench
schedules with `dspsched` **without** the list of proven forms. That is why its instruction count is a lower bound of
what it costs in an example (§7.4).

### 6.4 Metrics (`compare.py`)

| metric | what it measures | when it is valid |
|---|---|---|
| `snr_db(port, ref)` | power of the reference over that of the error, sample by sample | in the first 10 ms; afterwards small phase differences accumulate |
| `band_levels(x)` | power in bands 20–200–1k–4k–12k–40k Hz | always; it does not depend on the phases |
| `f0`, `rms`, `peak` | fundamental by zero crossings, level | indicative (the crossings fail with waves rich in harmonics) |

## 7. Ported component: Braids' SAW_SWARM

Origin: `braids/digital_oscillator.cc`, `DigitalOscillator::RenderSawSwarm` [code]. Port:
`sdk/mport/components/braids_saw_swarm.py`.

### 7.1 Original

* **Per block:**
  * `detune = (p1 + 1024)² >> 9`;
  * increment of saw i = `inc(pitch_ + detune·(i − 3)/65536)`, for i = 0…6, interpolated between steps of 1/128 of a
    semitone;
  * `hp_cutoff = pitch_ + ((p2 − 10922)·24 >> 5)` if p2 < 10922, otherwise `(p2 − 10922)·12 >> 5`, clipped to 0…32767;
  * `f = lut_svf_cutoff[hp_cutoff]` (2·sin(π·min(fc/fs, 1/8))·32767) and `damp = lut_svf_damp[0] = 65534`.
* **Per sample:**
  * `phase_i += inc_i`;
  * `sample = −28672 + Σ(phase_i >> 19)`;
  * `sample = Interpolate88(ws_moderate_overdrive, sample + 32768)`;
  * `notch = sample − bp·damp >> 15`; `lp += f·bp >> 15` with `CLIP`; `hp = notch − lp`; `bp += f·hp >> 15`;
  * `out = CLIP(hp)`.
* **Strike:** at the start of a note, 6 of the 7 phases take random values.

### 7.2 Port

| part of the original | port |
|---|---|
| increments of the 7 saws | `inc` of the centre (from the host) × ratio `R_k = 2^(k·detune/(65536·128·12))`, k = −3…−1, 1…3: `Param('ratio', count=6)` |
| sum of saws | x = Σ p_k / 8, accumulated with `mad x, p, #0.125`; it is exactly `sample / 32768` |
| `ws_moderate_overdrive` | odd polynomial of degree 9 (`WS_DEGREE`), 21.6 LSB of maximum error |
| `lut_svf_cutoff` | u = clamp(inc·H, π·8.18 Hz/fs, π/8) with `H = π·2^(offset/12)/2` (`Param('hp')`); f = (2u − u³/3)·32767/32768 |
| SVF | `notch = mad(x, bp, −65534/32768)`, `lp = clamp(mad(lp, f, bp))`, `hp = notch − lp`, `bp = mad(bp, f, hp)`, `out = clamp(hp)` |
| state | 7 phases + `lp` + `bp` in bank-0 rings |

### 7.3 Differences with the original

| difference | cause | effect |
|---|---|---|
| no strike | the phases keep running between notes | only the first note of each voice after power-on starts with the phases aligned |
| no sync input | the SYSTEM-1 has no source that uses it | — |
| **does not reproduce Braids' int32 overflow** | in the original `(increment_b − increment_a) · detune_fractional` overflows from MIDI note ≈ 92, depending on the detune; the low saws are left with wrong increments [code + integer model] | in high notes the port differs from the C++ (SNR 7.6 dB at note 96) and matches the model without the bug (58.3 dB) |
| SVF in float | Braids truncates `f·bp >> 15` | with the high-pass very open (f ≈ 7·10⁻⁴) the lows below 200 Hz differ by up to 5 dB |
| exact `lut_svf_cutoff` | the original interpolates linearly between semitones | less than 0.1 % in f |
| almost unstable regime | with a high COLOR and high notes, f ≈ 0.765 and damp = 2 leave the poles near −1 | both versions diverge chaotically; only the per-band comparisons are valid |

### 7.4 Results

`python3 sdk/mport/cli.py check saw_swarm` (2026-10-24) [sim]. The SNR is measured in the first 10 ms; the bands are
port − C++ in 20–200–1k–4k–12k–40k Hz:

| note | ratio / hp (knobs) | C++ = model | SNR vs C++ | SNR vs model without overflow | bands (dB) |
|---|---|---|---|---|---|
| 45 | 200 / 85 | yes | 41.3 | 41.3 | −0.03 −0.01 0.00 0.00 0.00 |
| 57 | 30 / 170 | yes | 55.9 | 55.9 | −0.69 0.00 −0.01 −0.01 +0.17 |
| 69 | 128 / 128 | yes | 57.4 | 57.4 | −1.43 0.00 −0.03 +0.01 0.00 |
| 69 | 128 / 0 | yes | 31.9 | 31.9 | −5.25 0.00 −0.02 +0.01 0.00 |
| 81 | 255 / 100 | yes | 58.5 | 58.5 | +0.26 0.00 −0.01 0.00 −0.07 |
| 33 | 255 / 255 | yes | 49.0 | 49.0 | −0.02 −0.01 0.00 0.00 0.00 |
| 96 | 128 / 128 | yes | 7.6 | 58.3 | +9.38 +8.30 −1.07 −0.02 −0.02 |

Cost per voice:

| context | units | instructions |
|---|---|---|
| bench, everything on one DSP, fusion without restrictions | 86 | 48 |
| MUTABLE-SYSTEM, DSP A (increment + saws + waveshaper + level + tremolo), only proven forms | — | ≈ 46 |
| MUTABLE-SYSTEM, DSP B (coefficient + SVF) | — | ≈ 9 |

## 8. Command line (`cli.py`)

```sh
python3 sdk/mport/cli.py list                                        # ported components
python3 sdk/mport/cli.py check saw_swarm [-n 9600] [--window 960]    # table of §7.4 (≈ 1.4 s)
python3 sdk/mport/cli.py lines saw_swarm [--plain]                   # bench program, scheduled or not
python3 sdk/mport/cli.py lut braids ws_moderate_overdrive --odd 9 --xmax 0.875
python3 sdk/mport/cli.py lut braids lut_svf_cutoff --kind 824 --deg 5 --xmin 0 --xmax 1
```

`check` goes through the cases of `CASES` in `cli.py`, one per row. For each case it builds the reference (the first
time), runs the bench and, if the component has `model()`, checks that C++ = model.

## 9. Tests

```sh
python3 -m unittest discover -s sdk/mport/tests        # 7 tests, ≈ 1.2 s
```

| test | what it checks |
|---|---|
| `test_expressions` | a component of arbitrary expressions gives the same in the emulator as in Python (register allocation, `sub`, `mad`, `ymac`, `min`/`max`, `wrap`, ring) |
| `test_out_of_registers` | `KernelError` when no registers are left |
| `test_accumulator_copy` | `mad` on a value that is read again works on a copy |
| `test_sched_equals_plain` | the scheduled program gives the same samples as the unscheduled one, in fewer instructions |
| `test_overdrive_fit` | the degree-9 fit of `ws_moderate_overdrive` has less than 25 LSB of error |
| `test_int_model_is_the_cpp` | integer model = C++ bit for bit (4 cases; skipped without g++ or without the checkout) |
| `test_port_close_to_original` | SNR > 40 dB in 10 ms against the model without overflow, bands 200 Hz–4 kHz within ±0.5 dB, ≤ 50 instructions |

## 10. How to port a new component

1. **Read the original.** Separate what is computed per block from what is computed per sample, and note the tables and
   the state. Before inventing a mechanism, look at how Roland does it (project rule).
2. **Reference.** Add the component to `ref/` and, if needed, to the stmlib substitutes. Write `reference()`.
3. **Integer or float model of the original** (recommended). Confirm that it matches the C++ bit for bit and note in
   §7.3 the original's bugs that will not be copied.
4. **`Param` for each control value.** Compute it as the original does, in Python.
5. **Per-sample tables to polynomials** with `lut.fit_odd` / `fit`. Choose the degree according to the error in LSB.
6. **`kernel()` with `Kernel`**, split into parts if it will go on two DSPs.
7. **Register it** in `components/__init__.py` and add its cases to `CASES` of `cli.py`.
8. **Measure.** `cli.py check`, then `cli.py lines` (cost) and the tests.
9. **Example.** Integrate it into a copy of an example, like `examples/mutable_system`: `--sim`, `--pack-check`,
   `--sim-check` if a new instruction form appears, `check_optimized` and build. Only one new thing per installation.

## 11. Open

* **Register spilling.** Today a kernel with more than 6–8 live values fails (`KernelError`). It would need storing in
  memory words of the same sample.
* **Computed index** (tables read by phase, variable delays) on the SYSTEM-1: step 15Y of C-S1SY. It blocks wavetables,
  Karplus, comb and chorus.
* **SFU ops** (reciprocal, root, log, exp by seed): they are not in `Kernel`, because they had not been tested on the
  SYSTEM-1.
* **Fused forms.** `ph_wrap` does not appear in `asm/proven_forms.txt`. If a build with `SCHED` without restrictions
  works on the device, DSP A of MUTABLE-SYSTEM would go down from 638 to 598 instructions.
* **stmlib.** Substitutes of `stmlib/dsp/*` for references of Plaits, Rings and Elements, or clone the submodule.

> Later state of some open items (from other chapters): s1cc has register spilling ([ch. 1](s1cc_compiler_internals.md)
> §7.5); indexed loads (`hset` + `X28Y`) run on the SYSTEM-1 since 2026-10-29 and the SFU ops were tested there with
> NATIVE-CHECK / NATIVE-CHECK2 ([Vol. II ch. 10](../vol2_dsp_core_isa/native_ops.md) §5).
