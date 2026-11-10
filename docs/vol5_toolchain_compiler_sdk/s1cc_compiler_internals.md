# Volume V — Chapter 1: s1cc — C / C++ Compiler to the Microcode of Roland's BMCs

> **Scope:** SYSTEM-1 (the kernels run in SYSTEM-S1CC and MUTABLE-SYSTEM).

> §3 (program model, `s1dsp.h`) is complemented by [ch. 2](s1dsp_api_reference.md), which lists the SDK header.

Specification. State: 2026-10-24, version 1.0 (with the dated additions in the text).

s1cc translates the C or C++ code of a synth voice into the microcode of the DSPs of the SYSTEM-1 (the ISA is the same
on the SYSTEM-8). The code can be one's own or copied from another project: Mutable Instruments, stmlib, any portable
DSP. There is no need to rewrite the algorithm:
* clang compiles the code as it is to LLVM IR;
* s1cc (C++, LLVM 19 API) translates that IR;
* the same source is also compiled natively, as a reference, and `s1cc check` compares both versions in the
  repository's emulator.

It replaces `sdk/mport` ([Vol. V ch. 3](mport_mutable_sdk.md)) as the porting tool. There the components were rewritten by
hand in Python; mport stays as a study and as the SAW_SWARM bench.

Tags: **[measured]** on the device; **[sim]** in the emulator (C core); **[open]**.

## 1. Use

```sh
make -C sdk/s1cc                                                     # builds s1cc (needs LLVM 19)
sdk/s1cc/s1cc build VOICE.cc [OTHER.cc ...] [-I DIR] [-o OUTPUT]     # -> OUTPUT/voice.s1k (+ report)
sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_wrapped.c --check   # saw.c comes in by itself (§9)
sdk/s1cc/s1cc check OUTPUT [--note 57] [name=knob ...]               # emulator against the native reference
KERNEL=OUTPUT sh examples/mutable_system/build.sh                    # the voice in the complete synth (§12)
```

Example, Braids' SAW_SWARM from the original C++:

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/braids_saw_swarm_s1.cc -I sdk/mport/ref -I ~/git/eurorack
#   source: .../eurorack/braids/resources.cc (for resources.h; --no-auto to leave it out)
sdk/s1cc/s1cc check sdk/s1cc/build/braids_saw_swarm_s1 --note 45 ratio=200 hp_h=85
#   DSP A: 74 units -> 49 instructions; SNR 57.6 dB (10 ms) ...; bands port - ref (dB): -0.00 +0.00 ...
#   DSP B: 31 units -> 26 instructions; SNR 41.3 dB (10 ms) ...
```

`build --check` runs `check` when it finishes (note 57, knobs at 128), to see the instructions and the SNR without a
second command. `build` also accepts `--tol X` (tolerance of the fits, range metric, default 3·10⁻³), `--tollog X`
(metric in octaves, default 0.0005 = 0.6 cent: the increments are pitches), `--maxdeg N` (maximum degree of a
polynomial, 11) and `--regs LIST` (the registers the kernel may use, §1.1).

### 1.1 Registers (`--regs`)

`--regs-a LIST` / `--regs-b LIST`: the same for a single DSP. SYSTEM-S1CC with `PAIRS=1` needs the kernel in r0–r3
(`--regs-a 0,1,2,3 --regs-b 0,1,2,3`): the other half is the voice of the pair
([examples/system_s1cc/README.md](../../examples/system_s1cc/README.md) §4.2).

The DSP has 8 registers, `r0`–`r7`. `--regs` is the list of those that the kernel's register allocator may use (§7.5);
it does not touch the others. With fewer registers, more values do not fit and *spill*: they are stored in a temporary
word of bank 0 (`{T<k>}`, `.scratch` in the kernel) and loaded again. Each spill costs a transfer (almost always packed
for free) and, sometimes, an instruction and a word per voice.

| PLUG-OUT | `--regs` | why |
|---|---|---|
| MUTABLE-SYSTEM (`KERNEL=`) | `0,1,2,3,4,7` (the default) | the host program of C-S1SY has `r5` = 0 and `r6` = 1 during the whole voice (its ADSRs use them); the kernel runs in the middle and cannot overwrite them |
| SYSTEM-S1CC | `0,1,2,3,4,5,6,7` | there is no host that keeps anything in registers between one voice and another (the host's envelopes set `r5` / `r6` again before each voice) |

```sh
sdk/s1cc/s1cc build VOICE.c --regs 0,1,2,3,4,5,6,7      # for SYSTEM-S1CC
```

Example [sim, 2026-10-27], the ADSR in C of `simple_saw_env.c` in DSP B: with 6 registers, 5 spill words and 52
instructions per voice; with 8, 3 words and 50 instructions. `make_s1cc.py` warns when a kernel has spills and does not
use `r5` / `r6`. A kernel compiled with the 8 registers does not work in MUTABLE-SYSTEM (it would overwrite `r5` /
`r6`).

Dependencies:
* clang / clang++ and LLVM 19 (`llvm-link`, `llvm-config`, `libLLVM`);
* `make`;
* `check` uses Python 3 with numpy, because the repository's assembler, scheduler and emulator are in Python.

## 2. Files

| file | language | what it is |
|---|---|---|
| `sdk/s1cc/include/s1dsp.h` | C / C++ | the SDK: program model and intrinsics (§3) |
| `sdk/s1cc/include/s1panel.h` | C / C++ | the panel controls of the SYSTEM-1 as `S1_PARAM` ids (`S1_OSC1_COLOR`...), included by `s1dsp.h` (§3) |
| `sdk/s1cc/src/s1cc.h` | C++ | common structures: graph, profile, fits, kernel |
| `sdk/s1cc/src/graph.cpp` | C++ | IR → data graph (§4) |
| `sdk/s1cc/src/prof.cpp` | C++ | instrumentation for the profiling and clones of the islands (§5) |
| `sdk/s1cc/src/fit.cpp` | C++ | function fitting (§6) |
| `sdk/s1cc/src/lower.cpp` | C++ | graph → kernel: representation of the values and lowering (§7) |
| `sdk/s1cc/src/kernel.cpp` | C++ | values → registers (with spills) → lines of one unit (§7.5) |
| `sdk/s1cc/src/main.cpp` | C++ | `s1cc prof` / `lower` / `init` / `graph` |
| `sdk/s1cc/runtime/s1rt.c` | C | native runtime: intrinsics on the PC, tables, reference, profiling scenarios |
| `sdk/s1cc/s1cc` | sh | driver: `build`, `check` (§9) |
| `sdk/s1cc/Makefile` | make | builds `build/s1cc` |
| `sdk/s1cc/tools/s1kernel.py` | Python | reads a `.s1k` and places it at real addresses (examples, `check`) |
| `sdk/s1cc/tools/s1check.py` | Python | `s1cc check` (§10) |
| `sdk/s1cc/examples/` | C / C++ | `braids_saw_swarm.cc`, `braids_saw_swarm_s1.cc`, `va_saw_svf.c` (§11) |
| `sdk/s1cc/tests/test_s1cc.py` | Python | end-to-end tests (§13) |
| `sdk/s1cc/build/` | — | builds and outputs (not in git) |

## 3. Program model (`s1dsp.h`)

```c
#include "s1dsp.h"

S1_PARAM(cutoff, S1_OSC1_COLOR, 1) {       /* knob OSC1 COLOR -> table: runs on the PC, knob = 0..255 */
    out[0] = tanf(3.14159265f * 50.0f * powf(240.0f, knob / 255.0f) / 96000.0f);
}

static float phase, ic1, ic2;              /* state of the voice: one ring word each, 8 copies */

S1_DSP_A {                                 /* DSP A of the slave, every sample */
    phase += s1_increment() * 0.5f;
    if (phase >= 1.0f) phase -= 1.0f;
    return 2.0f * phase - 1.0f;
}

S1_DSP_B {                               /* optional: DSP B; x = the voice's signal from the host */
    float g = s1_param(cutoff, 0);
    ...
}
```

| element | rule |
|---|---|
| `S1_DSP_A` | mandatory; runs on DSP A of the slave; returns the voice's signal (≈ −1…1) |
| `S1_DSP_B` | optional; runs on DSP B; `x` = the voice's signal after the host's level and tremolo |
| non-`const` file-scope variables | state of the voice (int or float; arrays with a constant index work, clang splits them). A variable can only be used by one of the two functions |
| `const` arrays | tables: with a constant index they fold; with a computed index they go to an island (§4.3) |
| `S1_PARAM(name, panel_id, n)` | knob table: the function runs on the PC for knob = 0…255 and writes `out[0..n-1]`; the ARM writes it as the DSP's `C0xx` parameters when the control `panel_id` moves |
| `panel_id` | a SYSTEM-1 panel control, by name: `S1_OSC1_COLOR` (`0x168`), `S1_OSC2_COLOR` (`0x16D`), `S1_FILTER_CUTOFF`... (`include/s1panel.h`, `#define`, included by `s1dsp.h`). The ids are measured ([examples/system_1_synth/README.md](../../examples/system_1_synth/README.md) §8). OSC1 / OSC2 COLOR are the ones the C-S1SY host leaves free |
| loops | only with a constant number of iterations (clang unrolls them) |
| calls | only code that gets inlined (in the same file or in headers) and the intrinsics; libm: §7.3 |

Intrinsics:

| function | what it is |
|---|---|
| `s1_pitch()` | pitch of the voice in octaves (4.75 = A 440), with the host's LFO and range |
| `s1_increment()` | phase step of that pitch for a phase −1…1: 2·f/96000 |
| `s1_gate()`, `s1_trig()` | gate 0 / 1; trigger: alternates 0 / 1 at each start of a note |
| `s1_param(name, i)` | value i of an `S1_PARAM` table |
| `s1_export(i, v)` / `s1_import(i)` | a value from `S1_DSP_A` to `S1_DSP_B` through slot `i` (one bank-5 word per voice); when it arrives: §3.1 |
| `s1_wrap(x)`, `s1_clamp(x)`, `s1_tri(x)` | x wrapped to [−1, 1) / clipped to [−1, 1] / 1 − 2·\|wrap(−x − 1.5)\|: one op each (`ph_wrap` / `ph_clamp` / `ph_tri`) |
| `s1_exp2(x)` | 2ˣ (Taylor + squarings, 10 ops; with more than ±8 octaves of range, `expsplit` + polynomial, 10 ops: §7.6) |
| `s1_floor(x)`, `s1_ceil(x)`, `s1_sign(x)`, `s1_abs(x)` | one-op native ops: floor / ceiling (SFU, \|x\| < 511), sign −1 / 0 / +1 (SFU), \|x\| (§7.6) |
| `s1_recip(x)` | 1/x: `recipseed` + 3 `newton`, 4 ops, error 10⁻⁸ (the same as `a / x` does; §7.6) |
| `s1_exp(x)`, `s1_log2(x)` | eˣ (= `s1_exp2(x / ln 2)`); log₂ x for x > 0: `log2split` + polynomial of degree 6, 9 ops, 5·10⁻⁶ octaves (§7.6) |
| `s1_ldexp(x, k)` | x·2ᵏ with constant k: free (it changes the scale, §7.1); if it has to be materialised, one `shl` / `shr` (§7.6) |
| `s1_sum3(a, b, c)` | a + b + c in one op (`sum8`) when the three are registers (§7.6) |
| `s1_mac2(a, b, c, d, e)` | a·b + c·d + e in one MAC op (two products); `e` can be 0. A constant factor goes as an immediate coefficient of the MAC (one register less); two as well. With few registers, `mul` + `mad` (§7.6) |
| `s1_sqrt(x)`, `s1_rsqrt(x)` | √x, 1/√x for x > 0 (E608): `sqrtsplit` + 2 Newton steps, 11 / 13 ops, 2·10⁻⁷ (§7.6) |
| `s1_range(x, lo, hi)` | x, declaring its range (when the profiling does not see it) |
| `s1_dt()`, `s1_rdt()` | f/96000 of the note (step of a phase 0…1) and its inverse. The host writes them per voice from per-note tables (SYSTEM-S1CC: `A040+v`, `A048+v`; `s1_rdt` stored /16384): two loads, without `exp2` or division. It is the way of having `1/dt` (polyBLEP) without dividing (`s1_recip` costs 4 ops, §7.6) |
| `S1_BUILD("...")`, `S1_COMPILE("...")` | (declarations, at file level) options of the host and of the compiler written in the source (§3.3) |
| `S1_NOTE_RANGE(lo, hi)` | (declaration, at file level) the MIDI notes at which the voice is played; the profiling only uses those (by default 0…127). What only happens outside the range (the two edges of a pulse in one sample, a pitch limit) is not compiled (W908 / W909) |
| `s1_env(i)` | only in `S1_DSP_B`: envelope `i` of the host, 0…1 (SYSTEM-S1CC: 0 = AMP, 1 = FILTER; [examples/system_s1cc/README.md](../../examples/system_s1cc/README.md) §3.1). In the native reference and in `check`, the gate |
| `s1_color(i)` | only in `S1_DSP_A`: OSC1 (0) / OSC2 (1) COLOR, 0…1, with its COLOR MOD source (manual = the knob; LFO, PITCH ENV, FILTER ENV, AMP ENV, SUB OSC = knob × source; SYSTEM-S1CC builds them in assembler, [examples/system_s1cc/README.md](../../examples/system_s1cc/README.md) §3.2). Range declared 0…1 whatever the profile (the profiling sweeps it 0…1 on each note). In the native reference and in `check`, the knob: `color0=K` / `color1=K` (128 by default) |
| `s1_lfo_filter()`, `s1_lfo_amp()` | the host's per-voice LFO: octaves to add to the cutoff (LFO TO FILTER, ±4) and a gain 0…1 for the VCA (LFO TO AMP). In both DSPs (B receives them through `INB`). In the native reference and in `check`, 0 and 1; the profiling sweeps them over their range. The LFO → pitch needs no call: SYSTEM-S1CC modulates the words of `s1_pitch` / `s1_increment` / `s1_dt` / `s1_rdt` ([examples/system_s1cc/README.md](../../examples/system_s1cc/README.md) §3.3) |

### 3.1 When each part runs

An s1cc program runs in two places. On the **PC**, when compiling, as a native binary: the knob table, the reference and
the profiling (§5, §9). On the **synth**, as microcode of the slave DSPs. Each part runs in one or in both:

| part | on the PC (`s1cc build`) | on the synth |
|---|---|---|
| `S1_PARAM` | once per knob position, 0…255 | never: its table is used |
| `S1_INIT` | before the reference and before each profiling scenario | never: the state it leaves is used |
| `S1_DSP_A` | in the reference and the profiling, one voice | DSP A, every sample, 8 voices |
| `S1_DSP_B` | the same, after `S1_DSP_A` | DSP B, every sample, 8 voices |
| C functions called from the voice | inlined: they run where the voice runs | same |

**`S1_PARAM(name, panel_id, n) { out[0..n-1] = f(knob); }`**
* **PC.** `ref tables` calls the function for knob = 0, 1, …, 255 and stores the 256 rows of `n` values (`tables.txt`).
  It is native C: it can use `powf`, `tanf`, `s1_exp2` or whatever is needed, it costs 0 in the DSP.
* **Synth.** The plug-out carries the table. The slave's ARM (`engine.c kparam_set`) writes the row of the knob's
  position into `n` parameters of the DSP (`C070`… in A, `C060`… in B; one for the 8 voices). When the plug-out loads it
  writes row 128; afterwards, every time the control `panel_id` moves. **It does not take the real position of the knob
  at load time**: until it moves, it is 128.
* **In the voice.** `s1_param(name, i)` reads parameter `i`: one memory load, no computation.
* `panel_id`: `s1panel.h` (`S1_OSC1_COLOR`…).

**`S1_INIT { ... }`**
* **PC.** It runs before the first sample of the reference and before each profiling scenario: first the state goes
  back to the C initialisers (`__s1p_reset`) and then `S1_INIT` runs (`s1rt.c reset`).
* **Synth.** It never runs: it is not compiled to the DSP. s1cc reads the values it leaves in the state variables and
  makes the DSP start from them: with a correction on the first sample after the program is loaded (§11.1), except for
  the phases that stay at 0.
* **What for.** To initialise a library with its own function (`osc.Init()`, `saw_init(&s)`). Without `S1_INIT` the C
  initialisers hold (`static float x = 1.0f;`).
* It is not a "note on": it does not run at the start of a note. To reset something on each note, `s1_trig()` in the
  voice.

**`S1_DSP_A { ...; return y; }`**
* **Synth.** The code is compiled once and copied 8 times into the program of the slave's DSP A, one per voice, each
  copy with its state words (block of the voice: 16 words at most, state + temporaries). The 8 copies run every sample,
  at 96 kHz, **whether a note sounds or not**: the gate at 0 only changes the value of `s1_gate()`.
* **No jumps.** The DSP does not jump: the `if`s are computed on both sides and the result is chosen (`select`, §4.1);
  the loops are unrolled up to their profiled maximum (§9.2.5); the calls are inlined. The cost is the same in all
  samples.
* **Inputs**, per voice: `s1_pitch()` (with the host's LFO and OSC1 RANGE), `s1_increment()`, `s1_gate()`, `s1_trig()`
  (alternates 0 / 1 at each start of a note).
* **Output.** The returned value (≈ −1…1) replaces DCO1 + DCO2 of the host (C-S1SY). The host multiplies it by OSC1 MIX
  and by the LFO's tremolo and passes it to DSP B as the voice's input.
* **PC.** In the reference (`ref`, `refdsp`) and the profiling: a single voice, one call per sample.

**`S1_DSP_B { ...; return y; }`** (optional)
* **It is not oscillator 2.** B is **DSP B** of the pair of slave DSPs. DSP A does the oscillators (the host's two, OSC1
  and OSC2); DSP B, the filter and the envelopes. Several oscillators all go in `S1_DSP_A` (`simple_saw_wrapped`: two
  saws). `S1_DSP_B` is for processing A's signal: a filter, a waveshaper, a per-voice effect (the high-pass of
  SAW_SWARM in `braids_saw_swarm_s1.cc`). It also serves to split the work when `S1_DSP_A` does not fit in DSP A's
  instructions.
* **Synth.** The same as `S1_DSP_A`, on the slave's DSP B: 8 copies, every sample. It goes first in the voice of DSP B.
  `x` is the output of `S1_DSP_A` after OSC1 MIX and the tremolo. What it returns continues through the host's chain:
  VCA ADSR, filter ADSR and low-pass.
* **Communication.** `s1_export(i, v)` in `S1_DSP_A` and `s1_import(i)` here; `i` is the slot, one bank-5 word per
  voice. s1cc stores the value in A with its representation and reads it in B with the same one (`.export i s o`). In
  the reference and in the emulator it arrives in the same sample; on the device the two DSPs run at the same time and
  it can arrive one sample later (`make_s1cc.py` reports it per voice). Detail:
  [sdk/s1cc/examples/simple_saw_wrapped/README.md](../../sdk/s1cc/examples/simple_saw_wrapped/README.md) §8. State variables are not shared
  between the two (error E302).
* **Without `S1_DSP_B`**, the output of `S1_DSP_A` goes directly to the host's chain.
* **PC.** After each `S1_DSP_A`, with `x` = what it returned.

The above is MUTABLE-SYSTEM (§12), where the C-S1SY host surrounds the kernel. In SYSTEM-S1CC there is no host:
`s1_pitch()` arrives without LFO or RANGE, the output of `S1_DSP_A` goes directly to the mix (or to `S1_DSP_B`) and
there is no VCA or filter afterwards. The kernel has to apply its gate or its envelope
([examples/system_s1cc/README.md](../../examples/system_s1cc/README.md) §3).

Example explained part by part: [sdk/s1cc/examples/simple_saw_wrapped/README.md](../../sdk/s1cc/examples/simple_saw_wrapped/README.md).

### 3.2 Wrapping a library

* **Wrapper.** The library is compiled as it is, from its sources; a single file includes `s1dsp.h` and adapts it: a
  global instance (its state is the voice's), its `Init()` in `S1_INIT`, one sample per call in `S1_DSP_A` (if it
  renders blocks, with size 1) and its parameters from `s1_param`. If a method is private, the wrapper can use
  `#define private public` before the `#include` (`braids_wrapper_saw_swarm.cc`).
* **The sources** all go into `s1cc build` (`.c` / `.cc` of the library and its tables); what has no body gives E101
  (§9.2.2).
* **`s1_range(x, lo, hi)`.** Declares the range of a value that the profiling does not see. `lo` and `hi` have to be
  constants, and the profiling has to go through the call: a call inside a branch that no scenario takes declares
  nothing.
* Examples: [sdk/s1cc/examples/simple_saw_wrapped/README.md](../../sdk/s1cc/examples/simple_saw_wrapped/README.md) (minimal), `saw_wrapper.c`
  (note detection with `s1_trig`, `s1_range`), `braids_wrapper_*.cc` (a C++ class of Braids).

### 3.3 Options in the source (`S1_BUILD`, `S1_COMPILE`)

```c
S1_BUILD("NAME=MP-CSAW ARM_ENV=1 LFO_LUT=1 ENV_CURVE=sq");   /* host options -> .option in voice.s1k */
S1_COMPILE("--regs 0,1,2,3");                               /* options of s1cc lower */
```

| macro | what it does |
|---|---|
| `S1_BUILD("KEY=VALUE ...")` | `s1cc lower` copies each pair to `voice.s1k` as `.option KEY VALUE` (§7.4), in the given order. `KEY`: `A`–`Z`, `0`–`9`, `_`; `VALUE` without spaces, or between quotes (`NAME=\"MY SYNTH\"`). s1cc does not interpret the keys: the host validates them. SYSTEM-S1CC takes them as default values of its variables, and an environment variable wins ([examples/system_s1cc/README.md](../../examples/system_s1cc/README.md) §1.2) |
| `S1_COMPILE("--option value ...")` | options of `s1cc lower` (`--regs`, `--regs-a`, `--regs-b`, `--tol`, `--tollog`, `--maxdeg`, `--maxknots`, `--trace`, `--no-exact-init`, `--no-fold-branches`, `--no-recipseed`, `--no-fuse-mac`, `--no-native`, `--no-derive`; and `--fused` / `--no-fused`, §3.4). They are applied before those of the command line: `s1cc build ... --regs 0,1,2,3,4,5,6,7` wins. `voice.s1k` keeps them in `.compile` |

* One of each per voice, at file level (two `S1_BUILD` are a redefinition: E010).
* They are constants of the module (`s1_build_opts`, `s1_compile_opts`) that the driver leaves public; `lower` reads
  them from the IR.
* Errors: E012 (`S1_BUILD`), E013 (`S1_COMPILE`), §9.2.0.
* The driver's options (`-I`, `-o`, `--check`, `--no-auto`, `--debug`) do not go in `S1_COMPILE`: they are needed
  before compiling the C. The exception is `--fused` (§3.4): the driver looks for it in `S1_COMPILE` before profiling.

### 3.4 Fused voice (`--fused`, the default) [sim, 2026-11-03; on the device: MP-CSAW2-FE]

`s1cc build` compiles by default the whole voice, `S1_DSP_B(S1_DSP_A() + s1_host_mix())`, as **a single function**.
`--no-fused` (or `S1_COMPILE("--no-fused")`) gives the two programs of before (MUTABLE-SYSTEM needs them:
`make_mutable.py` rejects a fused kernel). The command line wins over `S1_COMPILE`. The kernel has a single program
(`.dsp A`) and the mark `.fused` (§7.4). The host can run it on either of the two DSPs: SYSTEM-S1CC splits the voices
between A and B ([examples/system_s1cc/README.md](../../examples/system_s1cc/README.md) §1.3).

* **Driver step.** After the loops (`voice.ll`), `s1cc fuse` creates
  `s1_dsp_a() { return s1_dsp_b(s1_dsp_a() + s1_host_mix()); }`, inlines both, deletes `s1_dsp_b` and marks the module
  (`__s1_fused`). Then `opt -O2` again. The profiling, the lowering and `refdsp` use the fused module; the native
  reference (`ref`) keeps the two functions.
* **What changes in the C.** Nothing: the same source compiles both ways. In the fused function:
  * `s1_import(i)` is the value that `s1_export(i)` gave before, in a register (without a bank-5 word). An import
    without an export before: E116;
  * `s1_env` and `s1_lfo_*` can be read in the whole voice (there is no E117);
  * a state variable can be used by all the code (there is no E302 between A and B).
* **What is gained.** A single scheduling and a single register allocation over the voice, without A's output or B's
  input. `mp_csaw2` (`--regs 0,1,2,3`): 111 instructions against 80 + 37 = 117 (−5 %), 6 spill words against 6 + 5.
  `simple_saw_hostenv`: 12 against 10 + 5. The SNR against the reference is the same (output B of `ref`, which is the one
  `check` compares).
* **Between A and B.** What the host did between the two DSPs (in SYSTEM-S1CC, the SUB / NOISE mixer) comes in as
  `s1_host_mix()`: an input that is 0 in the native reference, range −2…2, one addition in the kernel.

## 4. From the IR to the graph (`graph.cpp`)

### 4.1 Functions, blocks and memory

* **Module.** The linked module is read (`llvm-link` of all the sources, to have the initialisers of the tables),
  compiled with `clang -O2`.
* **Blocks.** The blocks of `s1_dsp_a` / `s1_dsp_b` are walked in topological order. A loop (backward edge) is an
  error.
* **If-conversion.** Each block has a predicate: the sum of the predicates of its incoming edges, which are mutually
  exclusive. Each edge takes the predicate of its block times the condition of the branch (or its negation). A `phi`
  becomes a chain of `select`s. A `store` in a conditional block becomes `state = select(predicate, new, previous)`.
* **State.** A state word is (global variable, offset). The first load reads the value of the previous sample; the
  following ones read the last value stored in the sample (forwarding in the graph). The final value is stored in the
  ring, always: a ring that is not rewritten loses the value.
* **`switch`.** It is a chain of `if`s: each case is an edge with predicate `cond == value`, and the default branch
  takes "no case"; the cases that go to the same block are added. clang turns an `if / else if` on an integer into a
  `switch` (`env.c`, version with states of 2026-10-27).
* **`s1_param` in a `?:`.** `c ? s1_param(a, i) : s1_param(b, i)` comes from clang as a single call on a `select` of the
  two descriptors. It is turned into the `select` of the two parameters. A parameter without its own profile record
  takes the range of its table, over all positions of the knob (`simple_saw_env.c`: attack / release).
* **Tables.** A load from a `const` global with a constant offset is folded (`ConstantFoldLoadFromConst`). With a
  computed index it is marked for an island.

### 4.2 Signedness of the integers

An LLVM integer has no sign: its view depends on the use.
* **Structural rule.** It is unsigned if an unsigned operation reads or produces it (`lshr`, `zext`, `uitofp`, `udiv`,
  `urem`, `umin`/`umax`, `icmp u*`, `fptoui`). The mark propagates towards the operands through `select`, `shl`, `add`,
  `sub`, `and`, `or`, `xor`, `trunc` and `mul`, and between a state word and its final value.
* **Profile.** In `lower.cpp` (`unsignedView`), for `add` / `sub` / `mul` / `shl` it decides the view in which the C
  operation did not overflow: `p>>19 − 28672` overflows unsigned and not signed, so it is signed. If it overflowed in
  both views (the phase of an oscillator) or in neither, the structural rule decides. For the rest, the signed view
  only wins if its range is much narrower.

### 4.3 Islands

Some nodes cannot be executed as they are in the DSP: table reads with a computed index, computed pointers, unknown
functions, integer divisions or remainders by variables, `or`, non-contiguous masks. For each one roots are searched:
the values that cut all its paths to the sources (dominators in the DAG of dependencies; the sources are inputs,
parameters and state), the highest first.

* **Island.** An island with root R is the nodes that depend only on R (and on constants). Its output is the function
  y = F(R).
* **Clones.** `s1cc prof` copies the island as a native function `double __s1p_isl_<k>(double x)`. The runtime samples
  it over the observed range of R, with 2049 points.
* **Choice.** The lowering fits F (§6) for each candidate root and keeps the first that meets the tolerance, or the one
  with the smallest error (with a warning).

Example [sim, 2026-10-24]: in SAW_SWARM the island of `Interpolate88(ws_moderate_overdrive, sample + 32768)`, which
clang leaves as `lshr`/`and`/`xor`/two reads/interpolation, has root `sample`. A polynomial of degree 7 comes out, with
an error of 1.2·10⁻³ of the range, in 7 ops.

### 4.4 Speculated values

* **The problem.** Clang turns small `if`s into `select`s and computes both branches in every sample. A value that only
  feeds one branch (for example the `t/dt` of the polyBLEP) then takes, in the samples in which it is not used, huge
  values: up to 10⁸.
* **Detection (`speculation`).** Those cones are marked: nodes whose users are all inside the cone, or are the `select`
  through that branch.
* **Profiling.** The profile also records their range only when the branch is taken (`__s1p_c`).
* **Lowering.** It uses that range and limits the input nodes of the cone to it (min/max). Without this the polyBLEP
  came out with ~20 dB of SNR; with this, with 68–81 dB [sim].
* **Booleans.** They are not limited (neither by speculation nor by the outliers): the profile records them with their
  signed view (true = −1) and in the DSP they are 0 / 1; limiting 1 to [−1, 0] made it false.

### 4.5 Graph transformations before the islands

All of them keep the value of the C (modulo 2^bits for the integers); the order of the nodes and the profile ids are the
same in `s1cc prof` and in `s1cc lower`, because both build the graph in the same way.

| transformation | where | what it does | example |
|---|---|---|---|
| `memset` / `memcpy` of constant size | `Builder::memop` | one write per word that the DSP reads or writes in the range; the others cost nothing | `Init()` of an object, `state_ = {}` |
| `memset` / `memcpy` of variable size or computed destination | `memcpyVar`, `call` | on a small variable (≤ 16 words): each word conditioned on its distance and on its candidate address (E310 otherwise) | `std::copy(&u[0], &u[7 - shift], &r[shift])` (Plaits StringSynth) |
| computed index into a small array | `ptrOf`, `load`, `store` | state, local or `const` arrays of ≤ 16 elements, or a `phi`/`select` of constants or of pointers to the same variable: the word is chosen with comparisons (`kSmallArray`). A `float` table of constants is read as a sum of steps e0 + Σ (e_k − e_k−1)·[idx ≥ k]: one exact integer comparison and one addition per element, and the comparisons of an index are shared between the tables it reads (2026-10-28: the chords of Plaits 290 → 139 instructions per voice). Out of range, the nearest end | `partial_phase[i]` of Braids; `kR0[c]` of `mp_chord.c` |
| `x ^ (c ? K : 0)` with the bits of `x` inside K | `xorMask` | `c ? K − x : x` | the triangle of Braids |
| integer sums that cancel | `affineSimplify` | affine form of `add`/`sub`/`~`/×const, also through a `select` (its two arms); if ≤ 2 terms remain, the node is rebuilt with them | `prev + (prev < cur ? cur − prev : ~(prev − cur))` = `cur` or `cur − 1` |
| `c ? x + y : x` | `affineSimplify` | `x + (c ? y : 0)`: one product by c and one addition, instead of subtracting the arms (not if `y` is constant: the phase wrap stays recognisable) | the `this_sample += …` of the BLEPs |
| knob-derived columns (§4.6) | `deriveKnobColumns` | after the islands | `intervals[parameter_ >> 9]` |
| state liveness | `markLive` | a state word is only stored if something live reads its old value | `strike_` in a shape that never strikes |
| demanded bits | `build` (LLVM `DemandedBits`) | each integer keeps how many low bits are used (`Node::demand`); the lowering can reduce it modulo 2^demand where a user overflows in C | `(int16_t)(t + 32768)` as `sext(t) ^ 0x7fff8000` |

### 4.6 Knob-derived columns

A value that depends only on an `S1_PARAM` table (and on constants) is a function of that knob, known exactly at its
256 positions. `prof sweep` (`runtime/s1rt.c`) sweeps each knob 0…255 with the others at 128 and records all the
values; `s1cc lower --sweep` turns each such value that feeds the rest of the voice (and costs ≥ 2 ops) into a table of
its own `knob~id`, with the same `panel_id`, that the ARM writes with the knob. There is no fit and no work in the DSP.
`tables.txt` receives those tables; `s1check` and `make_s1cc.py` give them the position of the base knob.

* **`--no-derive`** (also in `S1_COMPILE`) switches them off: each `s1_param` is read from its own DSP parameter. It
  serves for probes that measure the parameters one by one (`examples/arm_probe/ramp_probe.c`: 160 values of a table
  that, derived, stayed in a single column).

### 4.7 Indirect calls (`devirt.cpp`)

Before O2 the driver internalises the module (only `s1_dsp_a`, `s1_dsp_b`, `s1_init`, `s1_note_range` stay public): the
tables that nobody writes become constants (`fn_table_` of Braids) and a call through a table at a constant index
becomes direct. The intrinsics `s1_*` are `pure` / `const`, so a `store` before a call reaches the `load` after it
(`set_shape(X); Render()`), and the functions are aligned to 16 (`-falign-functions`): the virtual bit of a pointer to
member function folds. What stays indirect with a finite set of targets (a `phi`/`select`, a constant table at indices
that are a finite set of constants) is rewritten by `s1cc devirt`: the target is rebuilt with function constants and
the call is versioned into direct calls (`versionCallSite`); O2 again inlines them. The virtual branch of a pointer to
member function (it reads a vtable) does not count when the others give functions.

## 5. Profiling (`prof.cpp`, `runtime/s1rt.c`)

`s1cc prof` inserts, after each value of the voice functions:
* `__s1p_v(id, signed view, unsigned view)`: ranges;
* in integer `add` / `sub` / `mul` / `shl`, `__s1p_o(id, overflowed signed, overflowed unsigned)`: the exact 64-bit
  result compared with that of the operation;
* in the `select`s, `__s1p_c(id, branch taken, value)` for the speculated values.

It also adds the clones of the islands (`__s1p_isl_tab`) and `__s1p_reset()`, which brings the state back to its
initial values.

The runtime also records, at the start of each sample, the range of each state word (`r index min max` in
`profile.txt`, through `__s1p_state`). The value of the state at the start of the sample uses it when it has no record
of its own: when the first access of the sample is a conditional store, not a load (`if (start) stage = ATTACK;` before
reading `stage`). Without that range, a `select` with that value was left with the range of the other side alone, and a
comparison with it was resolved as a constant.

Scenarios (`s1rt.c prof`):
* each `S1_PARAM` table swept from 0 to 255 in steps of 17, with the others at 128;
* 24 random sets of knobs;
* all at 0 and all at 255.

Each one over 14 notes from 0 to 127, 2400 samples per note, with the gate closing halfway.

## 6. Function fitting (`fit.cpp`)

The domain is brought to t = ta·V + tb ∈ [−1, 1], with V the value of the root. The models that meet the tolerance are
tried, from the cheapest to the most expensive:

| model | cost (ops) | when |
|---|---|---|
| P(t), degree 1…`maxdeg` | degree + 1, plus 0–2 for the transformation of t | always |
| odd P (t·Q(t²)) | (degree − 1)/2 + 3 | symmetric domain and function |
| sign·2^Q(t), Q of degree 1…6 | degree + 11 | output of a single sign (frequencies, coefficients) |
| + clipping to the plateaus | + 1 per end | ≥ 2 % of the samples at the minimum or the maximum at one end (CLIP, end of a table) |

* **Least squares.** In the Chebyshev basis, converted to powers of t.
* **Metric.** 99.5th percentile of the error, so that one corner of a table does not decide the fit; the maximum is
  also reported.
* **Range metric.** |error| / range of the output, with tolerance `--tol`.
* **Metric in octaves.** If the output is positive and covers more than 3 octaves (a coefficient, a frequency):
  |log2(fit / y)| over the samples ≥ max/64, with tolerance `--tollog`.

`hinge` model (`fit.cpp`): `c0 + c1 t + Σ w_j max(0, t − k_j)`, a linear spline with uniform knots or at the breaks of
the samples, when no polynomial reaches the tolerance; 2 ops per knot, at most `--maxknots` (32). The default tolerance
in octaves is `--tollog 0.0005` (0.6 cent: the increments are pitches).

## 7. Lowering (`lower.cpp`)

### 7.1 Representation of the values

Each value V of the program lives in a register r with a scale s and an offset o: **V = (r − o)·s**.
* **What costs zero.** A bit shift or a constant factor changes s; adding a constant changes o. With that the fixed
  point of the integers (Q15 products, `>> 15`, `+ 32768`, the `uint32` phase of an oscillator) spends no
  instructions.
* **Normalisation.** |r| ≤ 16 is kept, and it is brought to ≤ 1 when it goes over (1 op). It is also rescaled upwards
  if |r| < 2⁻⁸: below 0.125 the DSP's grid is 2⁻³² absolute and a tiny register loses precision. It also holds for the
  result of `min` / `max`: a clipped value (the cutoff `w ≤ 0.55` of an `exp2` of ±10 octaves) is rescaled once when it
  is clipped, not at each product (SVF of mutable_ported: 39 instructions per voice on DSP B, SNR 85 dB; measured
  against the version without it, which took 42 and gave 57 dB; Braids wrappers between −21 and +4).
* **Scale of a parameter.** It is the power of 2 that covers its table; the ARM writes value / scale.
* **Scale of a state word.** Also a power of 2, except:
  * an unsigned integer that covers its whole range (a phase): s = 2ⁿ⁻¹, o = −1, that is r ∈ [−1, 1), ready for
    `ph_wrap`;
  * signed over its whole range: s = 2ⁿ⁻¹, o = 0.

### 7.2 Operations

| C / IR | microcode |
|---|---|
| `a + b`, `a - b` | r = rₐ + q·r_b, with q = s_b / sₐ (`addxy` / `subxy` / `mad`); o = oₐ + q·o_b. If it goes to a wrap, the base is the operand with the scale of the wrap |
| `a + c`, `a * c`, `a << n`, `a >> n`, `a / c` | only s and o change |
| `a * b` | materialises o = 0 in both (0–2 ops) and one `mul` |
| overflow of `add`/`sub`/`shl` in C (seen in the profile) | V mod 2ⁿ = 2ⁿ⁻¹·(wrap(V/2ⁿ⁻¹ − 1) + 1): `ph_wrap` plus 0–2 ops; signed, wrap(V/2ⁿ⁻¹) |
| overflow of `mul` in C | warning; the port computes without wrapping (in real code it is almost always a bug, like the one of Braids, [Vol. V ch. 3](mport_mutable_sdk.md) §7.3) |
| `x & (2ᵐ − 1)`, contiguous mask, `x ^ 2ᵇ`, `~x` | mod 2ᵐ; difference of two mods; + 2ᵇ mod 2ᵇ⁺¹; −x − 1 |
| `a < b` and the other comparisons | `ph_clamp(K·(a − b − θ))` in stages of gain ≤ 16. Integers: θ = ±0.5, exact. Floats: up to 2¹⁷ / range |
| `a == b` between integers | −\|a − b\| ≥ 0 (θ = −0.5). Until 2026-10-27 it was 0.5 − \|a − b\|, half-integer: with \|a − b\| = 1 it gave 0.5 |
| comparison of a binary value against a threshold between 0 and 1 | the value itself (or 1 − value), 0 ops. Binary = always 0 or 1: `s1_gate()`, `s1_trig()`, the constants 0 / 1, booleans, `select` / product / `min` / `max` of binaries, and a state whose initial and stored values are binary (a flag) |
| `a == b`, `a != b` between two binaries | (a − b)², 1 op |
| `c ? a : b` | two forms, and the one that emits fewer ops stays (dead code removes the other): (b − o_c·s_c·d) + r_c·(s_c·d) with d = a − b, one `mad` on the condition's register without turning it into 0 / 1; or b + c01·d with c01 = c materialised. With a constant d: c scaled (free) + one addition; with constant arms, affine in c (0 ops). 2026-10-27: ADSR of DSP B 46 → 43, Plaits 311 → 303 |
| `x < T ? x : x - K` (and the `>=` form) | detected: (T − K) + mod(x − (T − K), K), exact, if the profile leaves x in [T − K, T + K) |
| `min`, `max`, `CLIP`, `smin`/`smax` | 1 op per end |
| `fabs`, `abs` | `max r, r, -r` |
| `floor` | V − mod(V, 1) |
| casts | no cost. A `trunc` that wraps becomes a mod; a constant read as signed is corrected in `zext` |

### 7.2.1 Views of the integers and costs

* **View of an operand.** The operations that read the sign (`ashr`, `sext`, `sitofp`, `sdiv`, `smin`, `icmp.slt`…) or
  ignore it (`lshr`, `zext`, `uitofp`, `udiv`, `umin`, `icmp.ult`…) read the operand in their view: if its computed
  range goes out of it, it is reduced modulo 2^bits (`inV`, `toView`). The same in `add`/`sub`/`mul`/`shl` and in the
  arms of a `select`, against the view of the result (`needsView`). A wrap of a 32-bit value works at scale 2^31, where
  the DSP's 29-bit mantissa leaves steps of 8.
* **Islands.** The samples of the output are taken to the narrowest view; those of the root, to the view and modulus
  (`demand`) in which the DSP has the root; the booleans are 0 / 1 (`islandSamples`).
* **Small integers.** An integer division by a constant and an `fptosi` whose result has a profiled range ≤ 4096 are
  truncated (`floorLV` / `truncLV`, 2 ops): there 1 LSB is everything (`while (x < 0) { x += 1536; n++; }` which LLVM
  makes a division; `MAKE_INTEGRAL_FRACTIONAL`). The others, without floor (§8).
* **Variable shift.** `x << n` / `x >> n` = `x · exp2(±n)`.
* **`xor` with a constant.** By stretches of contiguous bits k…j: `x ^ c = x + c − 2 (x mod 2^(j+1)) + 2 (x mod 2^k)`.
* **Comparisons.** Exact up to a range of 2^19; wider (a 32-bit phase), a step of 2^−16 of the range (2 stages of
  `ph_clamp`). Floats: 2^−24.
* **Comparisons of a value against several thresholds.** The value is multiplied first and then each threshold is
  subtracted from it (`materialize(..., mulFirst)`): the product is the same in all of them and the kernel's CSE shares
  it. A table step costs 3 ops instead of 4.
* **Pending: `a·(1 − x)` with a tiny x** (a per-sample decay computed in the DSP) loses the precision of x; `a − a·x`
  does not (mp_bell / mp_kick, 2026-10-28: SNR 0 dB → 88 dB).
* **Constant comparisons (W909).** A comparison that always gave the same in the profiling is lowered as that constant
  before the liveness: the side it never takes costs nothing (`--no-fold-branches` keeps them).
* **Booleans.** `band` of two booleans with the same false level and the same orientation: `min` (or `max`), 1 op.
* **Selects.** `f = (1 + k) b − k a` with `k = o_c s_c`: with a negated condition (k = −1) it is `a`, without an op.
* **Same register.** Two values of the same register are added without an op (`(r − oa) sa + (r − ob) sb`); if they
  cancel, a constant.
* **Float phases.** A float state word in [0, 1) that ends in its wrap (`c ? x : x − 1`) is stored in [−1, 1) (s = 1/2,
  o = −1): the wrap is one `ph_wrap` and the load and the store do not convert. It starts at half a turn.
* **Kernel.** It removes repeated identical ops (CSE) and does not add offsets smaller than 2^−26 of the scale; an
  addition with a constant outside ±512 is done in steps (E701 before).
* **Reciprocal.** Shared per divisor (`recipCache`). With `recipseed` + 3 `newton` (§7.6) by default; `--no-recipseed`:
  normalisation by octaves with `ph_clamp` comparisons, cubic seed and Newton (`recipNorm`, experimental: less exact,
  more ops).

### 7.3 Functions and divisions

| function | how |
|---|---|
| `exp`, `exp2`, `pow(c, x)` | range up to ±8 octaves: degree-4 Taylor of e^(x·ln2/16) and 4 squarings, centred on the argument's range (the error goes down with x⁵). Wider (up to 20 octaves): `expsplit` + degree-6 polynomial (§7.6), 10 ops, 6·10⁻⁸, instead of one more squaring for each doubling of the range |
| `x / y`, `1/y` | if y comes from an `exp2` (`s1_increment`, frequencies): 2^(−argument), exact. Otherwise: y' = y/2ᵏ (k centres the range in octaves) and `recipseed` + 3 `newton` (§7.6), 4 ops, 10⁻⁸. `--no-recipseed`: normalisation by octaves with `mul`/`mad` (more ops) |
| `floor` (`floorf`), `s1_floor`, `s1_ceil` | SFU `floor` / `ceil`, one op, if the profiled range is within ±511; otherwise x − (x mod 1) |
| `sin`, `cos`, `tan`, `tanh`, `sinh`, `cosh`, `log`, `log2`, `log10`, `sqrt`, `atan`, `asin`, `acos`, `cbrt`, `pow(x, c)` | analytic fit over the profiled range of the argument (§6). The argument can depend on any number of values |
| others | island (§4.3) |

### 7.4 Kernel (`.s1k`)

```text
.fused                               --fused: only .dsp A, which is the whole voice S1_DSP_B(S1_DSP_A()) (§3.4)
.option <KEY> <value>                S1_BUILD: a host option (§3.3); the value is the rest of the line
.compile <options>                   S1_COMPILE: the options of lower that came from the source
.state <i> <A|B> <s> <o> <name>      state word i: V = (r - o) s
.param <name> <n> <scale>            S1_PARAM table (the ARM writes value / scale)
.export <i> <s> <o>                  s1_export i and its representation
.inputb <input>                      input that B needs from A (A copies it)
.dsp A <units>
.scratch <n>                         temporary words of the spill
<lines of one unit with markers {IN:pitch} {IN:gate} {IN:trig} {S<i>} {S<i>+1} {P:<name>:<i>}
 {X:<i>} {INB:<input>} {T<k>} {XIN} {OUT}>
.dsp B <units>
...
.end
```

The host replaces each marker with an address (`tools/s1kernel.py place`) and schedules with `asm/dspsched.py`.

### 7.5 Registers (`kernel.cpp`)

* **Allocation.** Linear scan; the result can take the register of an operand that dies in the same instruction. An
  accumulator of `mad`/`ymacd` that is read again is copied before.
* **Spilling.** If the registers are not enough, the value whose next use is furthest is stored in `{T<k>}` and loaded
  again before using it. It is a bank-0 word written and read in the same sample.
* **Constant folding.** Two consecutive multiplications by a constant are folded into one. An immediate outside
  [2⁻¹², 256] that is not a power of 2 is split into steps.
* **Clean-up.** The results that nobody uses are removed, and a filler `mov rS, rS` is put after a load whose register
  the next line uses.

### 7.6 Native ops of the DSP

Besides the usual ops (`mul`, `mad`, `addxy`, `ph_wrap`…), the DSP has special units (SFU, the two-product MAC, shifts,
three-operand addition; [Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §5). All the ones in this table were measured **exact** on
the SYSTEM-8 master (`examples/from_scratch/re/dspnative.py`, 2026-10-28 and -29: 24–96 random cases per op, with and
without an empty instruction before reading the result, against the emulator; the result can be used in the next
instruction). On the SYSTEM-1 none of them is tested yet: the PLUG-OUT `NATIVE-CHECK`
(`sdk/s1cc/examples/native_ops/native_check.c`) tests them by ear.

> Later state ([Vol. II ch. 10](../vol2_dsp_core_isa/native_ops.md) §5, "2026-10-29, late"): NATIVE-CHECK and NATIVE-CHECK2 then ran every
> single op of this list on the SYSTEM-1 without error, each alone in its instruction.

**Where each part is:**

| what | file |
|---|---|
| declaration and one line per intrinsic | `sdk/s1cc/include/s1dsp.h` (block «Native ops») |
| native reference (what the C gives on the PC: `check`, profiling) | `sdk/s1cc/runtime/s1rt.c` |
| recognition in the graph | `sdk/s1cc/src/graph.cpp` (`kS1Calls`, `Op` nodes with the name of the intrinsic) |
| lowering (range of the result, when to use the native op) | `sdk/s1cc/src/lower.cpp` (`s1_floor` … `s1_mac2`, `recip1`, `exp2LV`, `log2LV`) |
| emission and registers | `sdk/s1cc/src/kernel.cpp` (`Kernel::recip`, `expsplit`, `log2split`, `sqrtsplit`, `exp2split`, `floor`, `ceil`, `sign`, `shift`, `mac2`, `mac2k`, `macOf`, `sum3`; op `multi` for those that write more than one register; `sqrtsplit` looks for r(b+1) and r(b+3) free) |
| which registers each one reads and writes (for the scheduler) | `asm/dspsched.py` (`_native`) |
| encoding | `asm/isa.json` (SFU forms with bit 3 of Y, `floor`, `shl` with the shift as a number, MAC with register and immediate coefficient `measured`; the assembler places the immediate coefficient at a byte 0–7 and writes it into C) |
| semantics in the emulator | `examples/from_scratch/emu/dspcore.h` (the reference), `dspdec.h`, `dspjit.py` |

**Intrinsics** (all `float`, pure functions; used like any C function):

| intrinsic | DSP op | cost | limits |
|---|---|---|---|
| `s1_floor(x)`, `s1_ceil(x)` | SFU `floor` / `ceil` | 1 op | \|x\| < 511 (otherwise x − (x mod 1)) |
| `s1_sign(x)` | SFU `sign` | 1 op | −1, 0, +1 |
| `s1_abs(x)` | \|x\| (unit B) | 1 op | — |
| `s1_recip(x)` | `recipseed` + 3 × `newton` | 4 ops | error 10⁻⁸; x is scaled by 2ᵏ so that \|x'\| ≥ 2⁻⁹ and 1/x' ≤ 256; profiled range of more than 16 octaves: W904 |
| `s1_exp2(x)`, `s1_exp(x)` | Taylor (≤ ±8 octaves) or `expsplit` + degree 6 | 8–10 ops | result < 512 (2⁹); range of the argument ≤ 20 octaves for `expsplit` |
| `s1_log2(x)` | `log2split` + degree 6 in m − 1 | 9 ops | x > 0 over the whole profile (E607); 5·10⁻⁶ octaves |
| `s1_ldexp(x, k)` | none (scale) or `shl` / `shr` / `shrn` | 0–1 op | constant k (E606); `shl` wraps in two's complement if it goes past ±512 |
| `s1_sum3(a, b, c)` | `sum8` | 1 op | the three in registers |
| `s1_mac2(a, b, c, d, e)` | MAC `mac_p1p2` / `mac_p1w`; with a constant factor `mac_p1p2c3` / `mac_p1wc3` (32-bit immediate coefficient, at byte C of the slot area); with two, immediate Y as well | 1 op | one register per non-constant operand + 1 (otherwise `mul` + `mad`); W911 if the result can go past ±512 |
| `s1_sqrt(x)`, `s1_rsqrt(x)` | `sqrtsplit` (m ∈ [1, 4), 9-bit seed, ±2⁻ᵏ) + 2 Newton (each 3 `mul` + one MAC with immediates) | 11 / 13 ops | x > 0 over the whole profile (E608); 2·10⁻⁷; `s1_rsqrt` uses `s1_recip` of 2⁻ᵏ |

```c
#include "s1dsp.h"
static float ph;
S1_DSP_A {
    float p = ph + s1_dt();
    p -= s1_floor(p);                         /* frac in one op */
    ph = p;
    float g = s1_exp2(4.0f * s1_color(0) - 2.0f);   /* 2^-2 .. 2^2 */
    float w = s1_mac2(p, g, 1.0f - p, s1_color(1), 0.0f);  /* p g + (1 - p) c1: one op */
    return w * s1_recip(1.0f + g);            /* 1 / (1 + g): 4 ops */
}
```

**What s1cc does by itself** (without calling anything; `--no-native` switches it off, except `shl` and the division):

* multiplying by a constant larger than 256 → `shl` with the power of 2 (unit B, one op) and, if needed, a `mul` by the
  rest in [1, 2).;
* `x / y` with a variable y → `recipseed` + 3 `newton`;
* `floorf` → SFU `floor`;
* `exp2` / `exp` / `pow(c, x)` of a wide range → `expsplit`;
* `|a| + b` → `absadd`, `b − |a|` → `subabs` (b register or immediate: the `|u| − 2` of the polyBLEP), `(a + b) + c` →
  `sum8` (2026-10-29, tested on the SYSTEM-1);
* **fusion into the MAC** (2026-10-29; `--no-fuse-mac` switches it off): `acc + x·y` with `acc` made by a `mul`, and the
  sum of two products, come out in one MAC op instead of `mul` + `mad` / `mul` + `mul` + `addxy`. Only measured forms:
  Y and C registers, Y register with an immediate K, Y and K immediates (not an immediate Y with a register C). If the
  product is also used elsewhere, the `mul` stays and the MAC saves nothing.

**`recipseed` overwrites its input**: `recipseed rD, rY` leaves N = ±2⁻ᵉ in rD, the seed F ≈ 1/m in rD+1 **and m in
rY** [measured]. A division must therefore not read y again after `recipseed`. `Kernel::recip` uses those three
registers as `newton` expects them (Goldschmidt division, Roland's) and the allocator copies y first if it is read
afterwards (op `multi` with `clobber`).

**Verifying on the device** (SYSTEM-1, once per new op):

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/native_ops/native_check.c --regs 0,1,2,3,4,5,6,7 -o sdk/s1cc/build/native_check --check
KERNEL=sdk/s1cc/build/native_check NAME=NATIVE-CHECK sh examples/system_s1cc/build.sh
python3 tools/plugout_tool.py install plugins/NATIVE-CHECK --slot 1 --force --no-patches -y
```

Each op is compared with an identity that does not use it (1/x·x = 1, exp(log₂x·ln2) = x, floor + fraction = y,
sign(x)·x = \|x\|, MAC against a·b + c·d + e, `sum8` against two additions, `shl` against a multiplication, log₂(2ᵗ) = t
over 18 octaves with `expsplit`, ceil(y) + floor(−y) = 0). The voice is a saw × (1 − clamp(64·error)): a clean saw =
fine; lower, silent or harsh = some op wrong. OSC1 COLOR chooses which ops enter: 0–27 all, then one every 28 steps (28
`s1_recip`, 56 `s1_log2` + `s1_exp`, 84 `s1_floor`, 112 `s1_sign`, 140 `s1_mac2`, 168 `s1_sum3`, 196 `shl`, 224
`expsplit` + `s1_ceil`). If the device hangs when selecting it, it is an op or a group that the SYSTEM-1 does not
accept: switch off and on, and report it. In the emulator the whole PLUG-OUT gives SNR 99 dB against the native
reference (`make_s1cc.py --sim`).

## 8. Differences with the C, by design

| topic | behaviour |
|---|---|
| initial state | the ring starts at 0, that is V = −o·s. For a phase with o = −1 that is half a cycle (2³¹), not 0. Warning per word. `s1cc init` makes the `refdsp` reference that starts the same way, for `check` |
| roundings | the `>> n` and the conversions to integer do not round: the DSP computes in reals, with an error ≤ 1 LSB of the original integer |
| float comparisons | exact except in a band of 2⁻¹⁷ of the range around the threshold. The phase wrap is recognised and is exact |
| overflows of products in C | not reproduced (warning) |
| fitted functions | error ≤ tolerance over 99.5 % of the profiled range; outside the profiled range, no guarantee (`s1_range`) |

## 9. Driver (`s1cc build`)

0. **Automatic sources.** To the given sources is added the neighbouring source of each header they include: same
   name, same folder, `.c` / `.cc` / `.cpp` (`saw.h` → `saw.c`, `braids/resources.h` → `braids/resources.cc`). Then
   those of the new sources, until none appears. The headers come from `clang -MM` with the same options as the build:
   they are the ones the preprocessor really opens, without those of the system or of the SDK. Each added source is
   reported (`source: … (for saw.h; --no-auto to leave it out)`). `--no-auto`: only the given sources.
   * **Limit.** A function defined in a file with a name other than its header's is not found: E101 (§9.2.2). That
     source is passed by hand.
   * **Checked** [sim, 2026-10-27]: `simple_saw_wrapped`, `saw_wrapper`, `va_saw_svf`, `braids_saw_swarm` and
     `braids_saw_swarm_s1` give the same kernel with automatic sources as with the sources given by hand.
1. `clang(++) -O2 -S -emit-llvm -fno-math-errno -I include ...` of each source, and `llvm-link` → `voice.ll`.
2. `s1cc prof voice.ll -o prof.ll` (with `--fused`, first `s1cc fuse voice.ll` and `opt -O2`, §3.4).
3. `clang voice.ll runtime/s1rt.c -o ref` and `clang prof.ll runtime/s1rt.c -o prof`, native.
4. `ref tables tables.txt` (`S1_PARAM` tables) and `prof prof profile.txt` (scenarios, ranges, overflows, islands).
5. `s1cc lower voice.ll --profile profile.txt --tables tables.txt -o voice.s1k --report report.txt`: first the options
   of `S1_COMPILE`, then those of the command line; `S1_BUILD` → `.option` (§3.3).
6. `s1cc init voice.ll --kernel voice.s1k -o refdsp.ll` and `clang ... -o refdsp`: the reference that starts from the
   DSP's state.

`report.txt` includes the units per DSP, the spills, each island and fit (model, degree, error, cost), the discarded
candidates and the warnings.

### 9.1 Debugging (`--debug`)

`s1cc build ... --debug` (or `S1CC_DEBUG=1`) changes three things:

* **Debug information.** The sources are compiled with `-g`. Each error and warning names the file, the line and the
  C / C++ function of the instruction, and the inlining chain up to the voice:
  `digital_oscillator.cc:306 in braids::DigitalOscillator::RenderToy, inlined at braids_wrapper_digital.cc:28 in
  s1_dsp_a`. Without `--debug` the error shows the IR instruction and the voice's function.
* **Exact line.** In addition, each source is compiled at `-O0` (`OUTPUT/srcmap.ll`, through the variable
  `S1CC_SRCMAP`). With `-O2`, LLVM hoists the code of small `if`s to execute it without a branch (SimplifyCFG) and gives
  it the location of the `if`. When the `-O2` location of an instruction does not correspond to any instruction of its
  type in `-O0`, s1cc looks for it in `-O0` among those of that type in the same block, and shows that line. The error
  ends with the source line, a `^` under each candidate column and the note `LLVM moved this ...`.
* **Prior diagnosis.** Before the profiling, `s1cc diag linked.ll --remarks inline.txt` walks `s1_dsp_a` and
  `s1_dsp_b` and lists all the C functions it cannot compile, each with its reason and its places. The list stays in
  `OUTPUT/diag.txt` and also goes to stderr with the prefix `s1cc: debug:`. That way all the problems are seen even if
  the build stops at the first one.
* **LLVM's reasons.** `opt` runs with `-pass-remarks-missed=inline`. `OUTPUT/inline.txt` keeps why the inliner left each
  call, and `diag` adds that reason to the function.

What `diag` detects (they are the same rules the graph builder uses, `graph.cpp` `callProblem` / `instProblem`):

| problem | reason it reports |
|---|---|
| call to a function without a body | the file that defines it is not among the sources, or it is a library function s1cc does not know |
| call to a function that is defined but not inlined | `noinline`, `optnone`, variadic, recursive, or the inliner's reason |
| indirect call | function pointer or virtual method |
| LLVM intrinsic without an operation | its name (`llvm.*`) |
| `invoke` and other instructions | the instruction is not supported |
| `bitcast`, `ptrtoint`, `inttoptr`, `addrspacecast` | pointer used as a number or reinterpretation of type |

At the end, `diag` also shows the C functions that did get inlined into the voice (`inlined into s1_dsp_a:`). The body
of a function that was not inlined is also checked, because its problems will appear when it gets inlined. `diag` does
not see the errors that depend on the graph or the profile (ranges, islands with several inputs, computed pointers).
Those still stop the build in `lower`, but with the C location above.

Example, the Braids wrapper without `digital_oscillator.cc` among the sources:

```
s1cc: debug: S1_DSP_A (s1_dsp_a): 1 C function(s) s1cc cannot compile
s1cc: debug:   braids::DigitalOscillator::RenderToy
s1cc: debug:     E101: not inlined: it has no body in the module: the file that defines it is not among the build's sources (add it), or it is a library function s1cc does not know
s1cc: debug:       at sdk/s1cc/examples/braids_wrapper_digital.cc:28 in s1_dsp_a
s1cc: debug:   inlined into s1_dsp_a: braids::DigitalOscillator::set_parameters
```

With `--debug`, each error ends with a reference to its code in §9.2.

Example, the wrapper with `digital_oscillator.cc` (E501):

```
s1cc: error E501: a table read / function / division `xor` that depends on several values (osc.phase_, osc.phase_increment_, s1_param(color, 0)) at /home/emilio/git/eurorack/braids/digital_oscillator.cc:308 in braids::DigitalOscillator::RenderToy, inlined at sdk/s1cc/examples/braids_wrapper_digital.cc:28 in s1_dsp_a: only functions of one value can be tabulated
s1cc: debug: 308 |         held_sample = (((phase >> 24) ^ (x << 1)) & (~x)) + (x >> 1);
s1cc: debug:     |                                       ^              ^
s1cc: debug:       (LLVM moved this `xor` to line 306; in the C it is one of the marked ones)
```

Without `-O0`, LLVM gave line 306 (the `if (decimation_counter >= decimation_count)`). The state fields are named as in
the C (`osc.phase_`, thanks to the debug information of the struct), not as LLVM globals (`_ZL3osc+4`).

**Why there is no complete check of the C before compiling.** What `diag` detects (calls, instructions) does not depend
on the values and is decided before. The `E5xx` / `E6xx` errors do depend: `x ^ y` works if, after inlining and
propagating constants, one of the two is constant (`x` of a fixed mode in the wrapper) and fails if both vary. A check
on the C alone would only give false positives. That is why those errors come out in `lower`, with the exact line.

`S1CC_DEBUG=2` also dumps each node of the graph and its representation after the lowering
(`n12 fmul: %3 s ... V [...]`). `-g` does not change the kernel: with and without `--debug` the same `voice.s1k` comes
out (checked with `va_saw_svf.c` and `braids_saw_swarm_s1.cc`, 2026-10-27).

**Cost per C line.** `report.txt` ends each DSP with `cost of DSP S by C line (ops)`: the ops of the kernel that
survive dead-code elimination, by the C line that made them (with `--debug`, `file:line`; a synthetic node, like the
`select` of a `phi`, carries `(select)` and the line of an operand). One DSP instruction carries 1–3 ops, so the sum is
larger than the instruction count. `S1CC_OPS=1 build/s1cc lower …` prints each op with its operands and its line.

### 9.2 Errors and how to solve them

Each error and each warning has a fixed **code**: `s1cc: error E501: ...`, `s1cc: warning W905: ...`. The code
identifies the exact case; the text that follows changes with the names and values. To solve it, look for the code in
the tables below (`grep E501` in this chapter).
The errors stop the build; the warnings do not, and they also stay in `report.txt` with their code. With `--debug`
(§9.1), each message carries `file:line in function` with the inlining chain, and each error ends with
`s1cc: debug: cause and fix of E501: [Vol. V ch. 1](s1cc_compiler_internals.md) §9.2`.

| range | stage | § |
|---|---|---|
| `E0xx` | files, program structure, clang and linker (driver) | 9.2.0, 9.2.1 |
| `E1xx` | calls, inlining, `s1_*` intrinsics | 9.2.2 |
| `E2xx` | instructions | 9.2.3 |
| `E3xx` | memory and state | 9.2.4 |
| `E4xx` | loops | 9.2.5 |
| `E5xx` | islands and functions | 9.2.6 |
| `E6xx` | operations | 9.2.7 |
| `E7xx` | kernel (immediates, registers) | 9.2.8 |
| `W9xx` | warnings | 9.2.9 |

The codes are stable: a code does not change meaning; a new case receives a new number. Those of `E1xx` and `E2xx` also
come out in the `--debug` list (`E101: not inlined: ...`). In the source code, each `fail("Exxx", ...)` /
`warn("Wxxx", ...)` carries its own.

#### 9.2.0 Files and structure (`main.cpp`, `graph.cpp`)

| code | message | cause | solution |
|---|---|---|---|
| `E001` | `cannot read IN.ll` | the IR does not exist or is not valid (LLVM prints the detail before) | run `s1cc build`, not the subcommands by hand; check LLVM 19 |
| `E002` | `cannot read the tables` / `bad tables file` | `tables.txt` is missing or truncated | redo the whole build |
| `E003` | `cannot write F` | output directory without permissions or without space | another `-o` |
| `E004` | `init: state 'v' not in ORIG.ll` | `voice.ll` and `linked.ll` do not match (files of different builds) | redo the whole build |
| `E005` | `cannot read the profile` | the `prof` binary did not run or failed | redo the build; if it fails, run `OUTPUT/prof prof x.txt` and look at its error |
| `E012` | `S1_BUILD: 'x' is not KEY=VALUE` / `K given twice` / `the quote is not closed` / `K has no value` | the text of `S1_BUILD` is not a list of `KEY=VALUE` (§3.3) | keys in capitals (`ARM_ENV=1`), each one once; a value with spaces between `\"…\"` |
| `E013` | `S1_COMPILE: unknown option x` / `x is not a compiler option` / `x needs a value` | `S1_COMPILE` has something that is not an option of `lower` (a path, `--check`, a typo) | only the options of §3.3; those of the driver go on the command line |
| `E014` | `fused module with an s1_dsp_b` / `s1cc fuse: could not inline f` | the fused module keeps `s1_dsp_b`, or LLVM could not inline one of the two functions | run `s1cc build --fused`, not the subcommands by hand; if it persists, report it |
| `E007` | `pass pipeline: ...` | internal LLVM error when preparing the loops | report it |
| `E008` | `no S1_DSP_A (s1_dsp_a) in the module` | `S1_DSP_A` is missing, or it was defined inside a `namespace` / without `#include "s1dsp.h"` | define `S1_DSP_A { ... }` at file level |
| `E009` | `s1_dsp_a returns nothing` | the voice never reaches a `return` with a value | return the sample in all branches |

#### 9.2.1 Before s1cc: clang and the linker

| code | message | cause | solution |
|---|---|---|---|
| `E010` | errors of `clang` / `clang++` (`error: unknown type name`, `file not found`) | the C does not compile as native C | fix it like any compilation error. A `-I` is missing or a macro needs defining (`-DTEST` is set by the driver if `lib/stmlib` exists (`s1cc deps`)) |
| `E011` | `undefined reference to 'f'` (linking of `trips`, `ref` or `prof`) | `f` is called and not defined in any source | add to the build the `.c` / `.cc` that defines it. `--debug` already lists it before as `it has no body in the module` |

#### 9.2.2 Calls (`graph.cpp`, also in `diag`)

The voice becomes a single function without calls: everything it calls has to end up inlined. Only calls to the
`s1_*` intrinsics, to libm (§7.3) and to LLVM intrinsics with their own operation can remain.

| code | message | cause | solution |
|---|---|---|---|
| `E101` | `call to 'f' not inlined: it has no body in the module` | `f` is declared (header) but its file is not among the sources. Or it is a library function that s1cc does not know (`rand`, `printf`, `memcpy` with a variable size) | add the source. If it is from a library, replace it: noise with an LCG in a state variable, no `printf`. The driver only adds the source with the same name as the header (§9, step 0); if it is named differently, pass it to `s1cc build` |
| `E102`, `E103` | `... it is marked noinline` / `optnone` | attribute in the source (`__attribute__((noinline))`, `__attribute__((optnone))`, or a macro that sets them) | remove the attribute in the copy being compiled, or define the macro empty with `-D` |
| `E104` | `... it is variadic (...)` | function with `...` | a version with fixed arguments |
| `E105` | `... it is recursive` | the function calls itself (and it is not tail recursion, which LLVM turns into a loop) | write it as a loop of bounded iterations (§9.2.5) |
| `E106` | `... LLVM did not inline it (LLVM: ...)` | the inliner rejected it for another reason; the text in parentheses is that of `-pass-remarks-missed=inline` | according to that text. Usually: `static inline` or move it to a header |
| `E107` | `indirect call (function pointer or virtual method)` | a non-constant function pointer, or a `virtual` method that clang could not devirtualise | call the function directly. With a `switch` on a mode: `if` / `else` calling each function. For virtuals: call on the concrete type (`obj.Class::method()`) or mark the class `final` |
| `E108` | `intrinsic llvm.X is not supported` | an integer intrinsic without an operation in s1cc (`llvm.ctlz`, `llvm.bswap`, a large `llvm.memcpy`) | avoid the construct that generates it: `__builtin_clz` → comparison ladder; copy structs field by field |
| `E110` | `s1_*: argument N must be a constant` | index of `s1_param`, `s1_export`, `s1_import` computed | a literal or a compile-time constant |
| `E111` | `s1_param(name, i): index out of the table` | `i` ≥ `n` of `S1_PARAM(name, panel, n)` | enlarge `n` or fix the index |
| `E112`, `E113` | `s1_param: unknown table` / `not an S1_PARAM descriptor` | the first argument is not a name declared with `S1_PARAM` | use the macro's name, not a variable |
| `E114`, `E115` | `s1_export in S1_DSP_B` / `s1_import in S1_DSP_A` | the direction is only A → B | export in `S1_DSP_A`, import in `S1_DSP_B` |

#### 9.2.3 Instructions (`graph.cpp`, also in `diag`)

| code | message | cause | solution |
|---|---|---|---|
| `E203` | `instruction 'invoke' / 'landingpad' ...` | C++ exceptions | the driver compiles with `-fno-exceptions`. Remove `try` / `throw` from the voice's code |
| `E204` | `cast bitcast / ptrtoint / inttoptr is not supported` | a pointer used as a number, or type reinterpretation through a pointer (`*(int *)&f`) | work with the value: for the exponent of a float, `log2f`; for the rest, arithmetic or a table |
| `E205` | `unsupported constant` | vector constant or of an odd type (`long double`, `__int128`) | `float`, `double` or integers ≤ 64 bits |
| `E201` | `instruction 'X' is not supported` (others) | an IR instruction that the graph does not model (`atomicrmw`, `cmpxchg`, `fence`, `va_arg`, vectors) | remove atomics, `va_arg` and explicit vectors from the voice's code |
| `E206` | `value not available (pointer used as a number, or an unsupported instruction)` | a value that comes from something the graph does not model | look at the instruction in the message; almost always a pointer that leaves the voice or a vector |

#### 9.2.4 Memory and state (`graph.cpp`)

The state is the non-`const` file-scope variables. The tables are the `const` ones. There is no other memory (§3).

| code | message | cause | solution |
|---|---|---|---|
| `E301` | `state 'v' is not an int or a float` | file-scope variable of pointer type, non-separable struct or `long double` | keep indices instead of pointers; split the struct into scalar fields |
| `E302` | `state 'v' used by both S1_DSP_A and S1_DSP_B` | each state word lives on a single DSP | duplicate it, or pass the value with `s1_export` / `s1_import` |
| `E303` | `computed index into a state array: state arrays need constant indices` | `buf[i]` with a variable `i` in a state array (delay lines, circular buffers) | constant indices (shift the array by hand), or an unrolled loop. Long delay lines do not fit the model |
| `E304` | `computed index into a local array: use constant indices` | local array with a variable index | the same, or make it `const` if it is a table |
| `E305`, `E306` | `load from an unknown address` / `unsupported pointer (only file-scope variables and tables)` | a pointer that is not a global (argument, `malloc`, stored pointer) | access the global directly. An object by reference works if after inlining the reference points to a global |
| `E307` | `'t' has no definition in the module` | `extern const` table without its file (e.g. `resources.cc` of Braids) | add the source of the table |
| `E308` | `cannot fold the table read` | table read with a constant offset that LLVM cannot fold (table without a visible initialiser) | the same: the definition with its values has to be in the build |
| `E309` | `store to something that is not a state word` | write to a `const` table or to computed memory | write only to state variables |
| `E310` | `memset / memcpy of something that is not a variable with a constant offset` / `memset with a variable byte` / `... with a variable size: only ... a small variable` | `memset` / `memcpy` whose destination is not a variable, with a variable byte, or of variable size over more than 16 words | constant size, or a small variable (≤ 16 words, §4.6) |

#### 9.2.5 Loops (`loops.cpp`)

The driver profiles how many times each loop turns and unrolls it up to that maximum (`s1cc trips` / `unroll`, §9).

| code | message | cause | solution |
|---|---|---|---|
| `E401` | `a loop at ... never ran in the profiling scenarios` | the loop is in a branch that no scenario of §5 took, so its maximum is unknown | make the scenario reach it (range of the parameter, note), or give it a constant number of iterations |
| `E402` | `a loop at ... turns up to N times per sample: too long` | more than 64 turns per sample | reduce it (one turn per sample with the counter in the state), or rewrite it as a closed formula |
| `E403` | `cannot peel the loop at ...` | LLVM could not unroll it (complicated multiple exits) | simplify: a single exit condition, integer counter |
| `E404` | `loop in voice code (...)` | a loop was left after the unrolling (should not happen) | report it; meanwhile, unroll by hand |
| `E405` | `a loop of f without a preheader` | internal error of the loop's shape | report it with the source and `--debug` |

#### 9.2.6 Islands and functions (`graph.cpp`, `lower.cpp`, `fit.cpp`)

What the DSP does not execute directly (table reads with a computed index, `or`, non-contiguous masks, integer
divisions by variables, unknown functions) is turned into a function of **one** value and fitted (§4.3, §6).

| code | message | cause | solution |
|---|---|---|---|
| `E501` | `a table read / function / division that depends on several values ... only functions of one value can be tabulated` | the node depends on two or more independent sources (inputs, parameters, state) that do not go through a single intermediate value | see below |
| `E502` | `cannot tabulate ...: no single-value island could be sampled` | no candidate root had usable samples | the same as `island without samples` |
| `E503` | `island without samples (the profiling run never reached it)` | the code is in a branch that no scenario took | make some scenario reach it, or `s1_range` on the root |
| `E504` | `not translatable and not tabulated` | node marked as an island for which no island was chosen | report it with `--debug`; rewrite the expression |
| `E505` | `F: the argument's range is unknown (use s1_range)` | `sin`, `tanh`, `log`... on a value the profile never saw (branch not taken) | `s1_range(x, lo, hi)` on the argument |
| `E506` | `pow with two variables` | `pow(x, y)` with both variable | `exp2(y * log2(x))`, which s1cc does lower |

**Several values.** The message says which operation it is and on which values it depends (`s1_*` inputs, parameters
and state fields with their C name). With `--debug` it shows the exact line (§9.1). Real example, `RenderToy` of Braids
(`digital_oscillator.cc:308`, which depends on `osc.phase_`, `osc.phase_increment_` and `s1_param(color, 0)`):
`held_sample = (((phase >> 24) ^ (x << 1)) & (~x)) + (x >> 1)`, with `x` from a parameter and `phase` from the state.
The `xor` / `and` with a **variable** mask is not arithmetic, and it depends on two sources. Ways out, in order of
preference:
1. That one operand be constant: the `and` / `xor` with a constant are lowered as arithmetic (§7.2). If the parameter can
   be fixed (a mode, not a continuous knob), fix it.
2. Write the operation as arithmetic of the two values (additions, products, `min` / `max`, comparisons).
3. That it depend on a single intermediate value: if `x` takes few values, an `if` per value leaves in each branch a
   function of `phase` only, with its own island.
4. If none of that works, that algorithm does not fit s1cc's model.

#### 9.2.7 Operations (`lower.cpp`)

| code | message | cause | solution |
|---|---|---|---|
| `E601` | `OP: operand N must be a constant` | shift (`<<`, `>>`) by a variable amount; integer division or remainder by a variable that did not end up in an island | shift by a constant; for a variable `x << n`, `x * s1_exp2(n)` in float |
| `E603` | `'or' with a constant is not supported outside a tabulated function` | `x \| c` with `x` from several sources | `x + c` if the bits do not overlap; otherwise, make it depend on one value (island) |
| `E604` | `compare 'P' not supported` | odd comparison predicate (`ord`, `uno`: NaN checks) | remove the NaN checks: the DSP has no NaN |
| `E606` | `s1_ldexp: the exponent must be a constant` | `s1_ldexp(x, k)` with a variable k | a constant k; for a variable k, `x * s1_exp2(k)` |
| `E607` | `s1_log2: the argument must stay above 0` | the profiled range of the argument reaches 0 or is negative | `s1_range` or `fmaxf(x, eps)` in the C |
| `E608` | `s1_sqrt: the argument must stay above 0` (or `s1_rsqrt`) | the profiled range of the argument reaches 0 or is negative (`sqrtsplit` writes nothing with 0) | `s1_range` or `fmaxf(x, eps)` in the C |
| `E605` | `operation 'OP' not supported` | IR operation without a lowering in `lower.cpp` (integers of more than 64 bits, odd bit operations) | rewrite it with the operations of §7.2 |
| `E116` | `s1_import(i) without an s1_export(i)` | `S1_DSP_B` imports an index that `S1_DSP_A` never exports | add the `s1_export(i, v)` |
| `E117` | `s1_env in S1_DSP_A` | the host's envelopes run on DSP B | use `s1_env` in `S1_DSP_B`; for A to depend on an envelope, compute it in C ([sdk/s1cc/examples/simple_saw_wrapped/README.md](../../sdk/s1cc/examples/simple_saw_wrapped/README.md)) |
| `E118` | `s1_env(i): the host has envelopes 0 and 1` | index other than 0 (AMP) or 1 (FILTER) | `s1_env(0)` or `s1_env(1)` |
| `E119` | `s1_color in S1_DSP_B` | the host builds the colors on DSP A | read `s1_color` in `S1_DSP_A` and pass the value to B with `s1_export` / `s1_import` |
| `E120` | `s1_color(i): the panel has colors 0 (OSC1) and 1 (OSC2)` | index other than 0 or 1 | `s1_color(0)` or `s1_color(1)` |
| `E121` | `s1_ldb5 in S1_DSP_B …: the wavetable read is on DSP A` | `s1_ldb5` called in `S1_DSP_B` | read the frame in `S1_DSP_A` and pass the value with `s1_export` / `s1_import` |
| `E006` | `no table for S1_PARAM 'p' (tables file)` | old `tables.txt` or the table's function did not run | redo the whole build |

#### 9.2.8 Kernel (`kernel.cpp`)

| code | message | cause | solution |
|---|---|---|---|
| `E701` | `kernel: constant K outside the DSP range (+-512)` | a scale or offset outside the range of the immediates; it usually comes from a huge profiled range | `s1_range` on the value that explodes, or limit it in the C (`s1_clamp`) |
| `E702` | `kernel: a multiply by a non-finite constant` / `a non-finite constant` | an unbounded range (division by a value that reaches 0, `log(0)`) | `s1_range`, or protect the division in the C (`max(y, eps)`) |
| `E703` | `kernel: exp2 of a value spanning more than 2^11` | the argument of `exp2` / `exp` / `pow(c, x)` covers more than 2048 octaves | limit the argument (`s1_range` or `s1_clamp`) |
| `E704` | `kernel: out of registers (N usable) even with spilling` | too many values live at once | more registers with `--regs` if the host program allows it, or split the computation between `S1_DSP_A` and `S1_DSP_B` |
| `E705`, `E706` | `kernel: value read before it is defined` / `no free register pair for recipseed` | internal error (E706: with 4 registers or fewer the pair rD, rD+1 of `recipseed` can be missing) | report it with the source and `--debug`; E706: more registers with `--regs` |
| `E707` | `kernel: the argument of OP is still live` | internal error: a native op that overwrites its input (`recipseed`, `expsplit`, `log2split`) received a value that is read afterwards without the prior copy | report it with the source and `--debug` |

#### 9.2.9 Warnings

The build finishes, but the result can differ from the C. `s1cc check` measures how much.

| code | warning | what it means | what to do |
|---|---|---|---|
| `W901` | `the original overflows at ... (integer product); the port computes it without wrapping` | the integer product overflows in C and the DSP does not reproduce the overflow | nothing, if the overflow was accidental. If the algorithm depends on it (hash, noise), rewrite it with `&` / `%` by a constant. Measure where it happens: in the Braids ones it only happens where the filter is already unstable and it does not matter (§11) |
| `W902` | `truncation wraps at ...` | a `trunc` to a smaller integer overflows in the profile; the port reproduces the modulus | normally correct (phases). Check whether it was not intentional |
| `W903` | `division by a value in [lo, hi]: the port divides by max(y, c)` | the divisor reaches 0 or crosses the sign | protect the divisor in the C, or `s1_range` |
| `W904` | `division by a value over [...]: more than 16 octaves, the ends saturate` | divisor with too wide a range | bound the divisor |
| `W905` | `F at ...: the fit misses the tolerance (error E)` / `island at ...: the fit misses the tolerance` | the fit does not reach `--tol` / `--tollog` | a higher `--maxdeg`, a larger `--tol`, or a smaller range (`s1_range`) |
| `W906` | `DSP S: N values limited to the range of 99.99 % of their profiled samples` | values with rare peaks; they are clipped so as not to ruin the scale | nothing if `check` gives a good SNR; otherwise, `s1_range` on those values |
| `W907` | `state 'v' starts at 0 in the ring (...)` | with `--no-exact-init`, the state word does not start at its initial value | remove `--no-exact-init` |
| `W908` | `DSP S: N values no profiling scenario computed (a branch none of them took) are 0 on the DSP; the first at ...` | code in a branch that no profiling scenario took (a second turn of an unrolled loop, a corner case). Its range is unknown, and computing it from the ranges of its operands can give 10¹⁵, which ruins the scale of the values combined with it. It is computed as 0, at no cost | nothing, if that branch is not taken in practice (Plaits `VariableShapeOscillator`: the second turn of the BLEP's `while`, §11). If it is taken, make a scenario reach it or declare the ranges with `s1_range`; `--debug` gives the C line |
| `W909` | `DSP S: N compares gave one answer in every profiling scenario and are that constant` | a comparison always gave the same in the profiling (the shape change that `S1_INIT` already made, a limit no note reaches): it is lowered as that constant, and the side it never takes costs nothing | nothing, if the scenarios cover the use (all the knobs, the notes of `S1_NOTE_RANGE`). Otherwise, `--no-fold-branches` |
| `W911` | `s1_mac2 at ...: the result can pass +-512 (it wraps)` | the range of a·b + c·d + e reaches ±512 | scale the operands (`s1_ldexp`) or bound them (`s1_range`) |

## 10. Verification (`s1cc check`)

* **Bench.** DSP 0 of the emulator = the code of DSP A, DSP 1 = that of DSP B, a shared bank 5 as in the pair of slave
  DSPs.
* **Inputs.** Pitch at `A030`, gate `A038` = 1, trigger `A060` = 1.
* **Connections.** The output of A goes to `A151`, which is the input of B.
* **Memory.** State from `0x0100` and temporaries from `0x0300` in bank 0 of each DSP; parameters from `C040`.
* **Scheduling.** With the proven forms (`--all-forms` for all of them).
* **Comparison.** Against `refdsp`: SNR in the first 10 ms (sample by sample) and over all, levels per band and
  instructions.

## 11. Examples [sim, 2026-10-27]

| example | what it tests | A (instr.) | B (instr.) | SNR 10 ms A / B |
|---|---|---|---|---|
| `braids_saw_swarm.cc` | the original loop of RenderSawSwarm (integers, `uint32` phases, `Interpolate88`, Q15 SVF, `lut_svf_cutoff` by island) | 49 | 46 | 58 / 44–49 dB |
| `braids_saw_swarm_s1.cc` | the same, with the coefficient of the high-pass from the exported increment (fits in 8 voices) | 49 | 26 | 58 / 46–49 dB |
| `va_saw_svf.c` | float: polyBLEP (`if`, divisions by `dt`), Simper's SVF with `tanhf`, tables with `powf`/`tanf` | 84 | 31 | 68 / 54 dB |
| `plaits_variable_shape.cc` | Plaits' `VariableShapeOscillator` as it is, with the real stmlib (`s1cc deps`): BLEP in an unrolled `while`, division of the BLEP whose divisor crosses 0 (W903), a branch no scenario takes (W908) | 311 | — | 68–99 dB / — |
| `simple_saw_wrapped/` | the minimal example: a pure C library (naive saw, `saw.c`), two instances per voice, and its wrapper ([sdk/s1cc/examples/simple_saw_wrapped/README.md](../../sdk/s1cc/examples/simple_saw_wrapped/README.md)) | 30 | — | 95–112 dB / — |

Notes 45 and 57, knobs at 128. `tests/test_s1cc.py` uses these values with a margin (A ≤ 52 in the Braids ones). The
phases that start at 0 get no start correction (§11.1).

**Warning W901 of the Braids ones and the unstable zone of the high-pass** [sim, 2026-10-27]. The warning comes from
`f * bp` and `f * hp` of `S1_DSP_B` (`braids_saw_swarm_s1.cc` lines 50 and 53): `int32` products that wrap on Braids'
ARM and not in the port. Measured on the native reference with an overflow counter (1 s per case, ratio = 128):

| COLOR (`hp_h`) | note 33 | note 57 | note 81 |
|---|---|---|---|
| 0, 85, 128, 200 | 0 | 0 | 0 |
| 255 | 0 | 0 | 23 of 192 000 (0.012 %) |

`check`, DSP B:

| case | overflows | SNR 10 ms | peaks port / ref |
|---|---|---|---|
| COLOR 255, note 57 | 0 | 42.5 dB | 0.506 / 0.508 |
| COLOR 255, note 69 | 0 | 44.0 dB | 0.917 / 0.915 |
| COLOR 200, note 81 | 0 | 0.6 dB | 1.000 / 1.000 |
| COLOR 230, note 81 | 0 | −1.7 dB | 1.000 / 1.000 |
| COLOR 255, note 81 | 23 | −1.7 dB | 1.000 / 1.000 |

* **The difference is not the overflow.** With COLOR 200 and note 81 there is no overflow and the port already differs
  the same.
* **Cause.** With a high note and a high COLOR, `u` reaches Braids' limit (0.3927, `f` ≈ 0.77). With `lut_svf_damp[0]` =
  65534 (damping 2) Chamberlin's SVF is unstable there: it self-oscillates and both versions saturate at ±1. The signal is
  chaotic: a difference of 1 LSB of rounding (§8: the `>>` do not round the same) grows until the outputs do not look
  alike. Reproducing the overflow (2–3 instructions per product) would not bring the result closer, so it is not done.
* **Sound.** In that zone the levels per band differ a lot (note 81, COLOR 255: +20 dB, −16 dB, −6 dB in the first
  three bands). It is saturated self-oscillation in both, but with another balance of frequencies. For them to match
  over the whole range `u` would have to be limited below the stability limit, departing from Braids.
* **Outside that zone** there is no overflow and the port matches: 42–49 dB (table above and `tests/test_s1cc.py`).

**Plaits `VariableShapeOscillator` and W908** [sim, 2026-10-27]. Until 2026-10-27 the kernel came out silent (peak 0,
SNR 0 dB, 384 instructions):
* **Cause.** The BLEP runs in a `while (…) { if (!high_) …; if (high_) …; }` that s1cc unrolls to its profiled maximum.
  The second turn (registers 93–114) was taken by no scenario: its values have no range in the profile. s1cc computed
  them by intervals from their operands, and one is
  `t = (slave_phase_ - pw) / (previous_pw_ - pw + slave_frequency)`, with a divisor that crosses 0 (W903): ranges of up
  to 3·10¹⁵. The `select`s that join the paths of the `while` inherited that scale (1.6·10¹⁷), and the normal values
  (≈ 0…1) were left without precision: `next_sample` and the output at 0.
* **Why the speculation (§4.4) did not avoid it.** It only limits the cones of the IR's `select`s. The `select`s that come
  from turning blocks into predicates (the `phi`s) do not have it: their values are only profiled when the block runs,
  and that is enough, except when the block never runs.
* **Fix.** A value that no scenario computed is lowered as 0 and reported with W908 (§9.2.9). It is only used under a
  predicate that the profile never saw true.
* **Result.** 311 instructions; SNR 10 ms: 67.9 dB (note 45), 68.0 dB (57), 78.5 dB (69, `pw` 200, `shape` 50), 83.9 dB
  (81, `shape` 255), 99.0 dB (33, `pw` 0, `shape` 0). In the test with a margin.
* **It does not fit in the synth yet**: 311 instructions per voice × 8 and 21 spill words (the block of a voice admits
  16 words of state + temporaries, §12).

### 11.1 Exact start of the state (`.initflag`, `--no-exact-init`)

* **The problem.** The DSP's state ring starts at 0, and that is V = −o·s (§8). For a `uint32` phase (o = −1) it is half a
  cycle (2³¹), not the 0 of the C.
* **The correction.** If a state word does not start at 0 in the ring (because of the C initialiser or of `S1_INIT`),
  s1cc adds a first-sample flag (`{I}`: load, `1 − flag`, store 1) and one `mad` per word: r ← r + first·r₀. The kernel
  lists those words (`.init <index>`) and `s1cc init` leaves only those at their C value in the reference `refdsp`.
* **Cost.** The DSP does not jump: those instructions run in **every** sample, although they only change something in
  the first. With 7 words: 2 for the flag + 7 `mad` = 9 per sample.
* **When it acts.** Only once, on the first sample after the program is loaded into the DSP. Not on each note: as in the
  C, the state carries on between notes.
* **Phases: automatic.** A modular word (a `uint32` phase, o = −1) that starts at 0 in the C is **not** corrected: it
  starts shifted by half a cycle, a fixed phase offset of a free-running oscillator, which is not heard. `report.txt`
  notes it (`phase starting at 0, left at half a cycle`). That way SAW_SWARM saves 9 instructions (58 → 49) with the
  same SNR (57.8 dB). A phase that `S1_INIT` sets to another value is corrected.
* **The rest is corrected**, because the initial value is usually part of the algorithm: the seed of a noise generator
  (at 0 it stays at 0 forever), a level or a gain that start at 1, a filter whose initial state is not 0. Example [sim,
  2026-10-27]: `static float lvl = 1.0f; ... return saw * lvl;` sounds (peak 0.997); with `--no-exact-init` it stays
  silent (peak 0) and the warning W907 comes out.
* **`--no-exact-init`** also removes those corrections (one W907 per word).
* **`S1_INIT`** is optional. Without it the C initialisers hold (`static uint32_t phase[7];` = 0). What costs
  instructions is not `S1_INIT`: it is any initial value that the ring does not give by itself.

## 12. In the synth (`examples/mutable_system`, `KERNEL=`)

Two PLUG-OUTs take a kernel:

| PLUG-OUT | what it adds to the kernel | when to use it |
|---|---|---|
| MUTABLE-SYSTEM (`examples/mutable_system`, `KERNEL=`) | all of C-S1SY13: LFO and RANGE on the pitch, OSC1 MIX, tremolo, VCA and filter ADSR, low-pass | an oscillator that wants the rest of a synth |
| SYSTEM-S1CC (`examples/system_s1cc`, [examples/system_s1cc/README.md](../../examples/system_s1cc/README.md)) | nothing: mix, VOLUME and the OS's delay / reverb; `VOICES=1..8`; instruction cap | a complete voice in C (with its gate / envelope), or a kernel that does not fit next to the host |

What follows describes MUTABLE-SYSTEM.

* **Build.** `KERNEL=<output of s1cc> sh examples/mutable_system/build.sh` replaces the oscillator of C-S1SY13 by the
  kernel's DSP A code and puts its DSP B code at the start of the voice of DSP B.
* **Knobs.** Each `S1_PARAM` ends up on the knob of its `panel_id` (tables `kparam_*`, `engine.c kparam_set`).
* **Addresses.** [examples/mutable_system/README.md](../../examples/mutable_system/README.md) §8.

With `braids_saw_swarm_s1.cc` [sim]:
* DSP A 626 and DSP B 620 instructions. The hand version in Python used 638 / 564; the measured limits are 703 (ran; 826 hung) / 708 (ran).
* `--pack-check` and `--sim-check` identical on 7 panels.
* The synth's output matches that of the hand version to ±0.03 dB per band.

## 13. Tests

```sh
python3 -m unittest discover -s sdk/s1cc/tests      # ≈ 8 s: build + check of the three examples, thresholds of §11
```

## 14. Open

* **No kernel tested on the device.** The SAW_SWARM one is ready to install (§12).
* **Loops with a variable trip count.** Daisy-chains, searches: today they are an error.
* **Delays and tables read by phase in memory.** They wait for the computed index of the SYSTEM-1 (step 15Y of C-S1SY);
  today the tables go through islands, and the islands only serve for functions of one value.
* **Exact initial state.** Option to generate the code that sets the C's initial state (one flag and one `mad` per
  word).
* **Native ops on the SYSTEM-1** (complete plan: [Vol. II ch. 10](../vol2_dsp_core_isa/native_ops.md)). Measured exact on the SYSTEM-8 (§7.6);
  `NATIVE-CHECK` on the device is missing, and then testing the groups (`shl` with a `mul`, the MAC with an A op) with
  TRY_FORMS. `sqrtsplit` (1/√) was left without an exact model: it writes 2⁻ᵏ in r5 and its Q table does not match
  (`dspnative.py sqrtsplit`).
* **Costs.** Cheaper float compares with the DSP's flags (`X9bY`, tested on the SYSTEM-8, not on the SYSTEM-1) and
  fusion of more forms (list of proven forms).
* **stmlib.** Substitutes of `stmlib/dsp/*` (filter, polyblep, parameter_interpolator) to compile Plaits, Rings or
  Elements as they are.

> **Later state of some open items** (from the other chapters, not from this specification): kernels have run on the
> device since 2026-10-28 (e.g. MP-CSAW, MP-CSAW2 with 6 voices, NATIVE-CHECK / NATIVE-CHECK2:
> [examples/system_s1cc/README.md](../../examples/system_s1cc/README.md),
> [sdk/s1cc/examples/mutable_ported/README.md](../../sdk/s1cc/examples/mutable_ported/README.md),
> [Vol. II ch. 10](../vol2_dsp_core_isa/native_ops.md) §5); `sqrtsplit` got its exact model on 2026-10-29 (9-bit Q table, output r(d+3):
> [Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §5.3); the "exact initial state" item is what §11.1
> describes; indexed loads (`hset` + `X28Y`) run on the SYSTEM-1 since 2026-10-29 ([Vol. II ch. 10](../vol2_dsp_core_isa/native_ops.md) §2).
