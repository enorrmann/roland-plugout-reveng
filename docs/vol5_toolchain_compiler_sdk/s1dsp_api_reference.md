# Volume V — Chapter 2: The s1cc SDK Header (`s1dsp.h`, `s1panel.h`) — API Reference

> **Scope:** SYSTEM-1.

> Written from the headers `sdk/s1cc/include/s1dsp.h` and `sdk/s1cc/include/s1panel.h`. What each part does when
> compiled is [ch. 1](s1cc_compiler_internals.md) §3 (program model and intrinsics) and §7.6 (native ops); where the
> header and that chapter differ, it is said below.

## 1. Program model (macros)

| macro | expands to | meaning |
|---|---|---|
| `S1_DSP_A` | `float s1_dsp_a(void)` (with `extern "C"` in C++) | the voice on the slave's DSP A, every sample, every voice; returns the voice signal (≈ −1…1) |
| `S1_DSP_B` | `float s1_dsp_b(float x)` | optional, the voice on the slave's DSP B; `x` = the host's voice signal |
| `S1_INIT` | `void s1_init(void)` | optional; runs on the PC only, before the reference and each profiling scenario; the state it leaves is the DSP's initial state |
| `S1_PARAM(name, panel_id, count)` | a `static` function `s1_param_fn_<name>(int knob, float *out)` and a descriptor `s1_param_desc_<name>` in section `s1_params` | knob table: computed on the PC for knob = 0..255; on the synth the ARM writes the knob's row into DSP parameters (row 128 at load, then whenever the control moves) |
| `s1_param(name, i)` | `s1_param_get(&s1_param_desc_<name>, i)` | value `i` of the knob table |
| `S1_NOTE_RANGE(lo, hi)` | `const int s1_note_range[2]` | MIDI notes the voice is played at (default 0..127); the profiling only runs those |
| `S1_BUILD(opts)` | `const char s1_build_opts[]` | host build options written to the kernel as `.option KEY VALUE` |
| `S1_COMPILE(opts)` | `const char s1_compile_opts[]` | `s1cc lower` options, applied before the command line's |

The descriptor type:

```c
typedef struct s1_param_desc {
    const char *name;
    int panel_id;
    int count;
    void (*fn)(int knob, float *out);
} s1_param_desc;
```

Attributes: `S1_IN` = `pure, nothrow` (reads the voice's inputs, writes no memory: a store before the call reaches a load
after it); `S1_FN` = `const, nothrow` (a function of its arguments only).

## 2. Intrinsics (as declared)

| declaration | attr. | meaning (header comment) | where usable |
|---|---|---|---|
| `float s1_pitch(void)` | `S1_IN` | pitch of the voice in octaves (4.75 = A 440; LFO and range applied by the host) | A, B |
| `float s1_increment(void)` | `S1_IN` | phase step for a phase −1..1: 2 f / 96000 (exp2 of the pitch on the DSP) | A, B |
| `float s1_dt(void)`, `float s1_rdt(void)` | `S1_IN` | f / 96000 (phase step for a phase 0..1) and its inverse, from per-note tables the host writes with the pitch (SYSTEM-S1CC: two loads, no exp2, no division) | A, B |
| `float s1_gate(void)`, `float s1_trig(void)` | `S1_IN` | gate 0 / 1; trigger toggles 0 / 1 at every note start | A, B |
| `float s1_param_get(const s1_param_desc *, int)` | `S1_IN` | used through `s1_param(name, i)` | A, B |
| `void s1_export(int i, float v)`, `float s1_import(int i)` | —, `S1_IN` | a value from `S1_DSP_A` to `S1_DSP_B` ("same sample" in the header; on the device it can arrive one sample later, [ch. 1](s1cc_compiler_internals.md) §3.1) | export in A, import in B |
| `float s1_ldb5(int i)` | `S1_IN` | `S1_DSP_A` only: word i (0..126) of the wavetable frame in bank 5 (`A080..`) that the host / USB loader writes; one `hset` + `ldb5` (an indexed bank-5 load); the native reference and `check` read a band-limited saw stand-in frame | A |
| `float s1_wrap(float)`, `s1_clamp`, `s1_tri` | `S1_FN` | x wrapped into [−1, 1); limited to [−1, 1]; 1 − 2 \|wrap(−x − 1.5)\| | A, B |
| `float s1_exp2(float)`, `s1_exp` | `S1_FN` | 2^x, e^x | A, B |
| `float s1_range(float x, float lo, float hi)` | `S1_FN` | x, declaring its range (lo, hi constants; the profile must reach the call) | A, B |
| `float s1_floor(float)`, `s1_ceil` | `S1_FN` | SFU floor / ceil, 1 op (\|x\| < 511) | A, B |
| `float s1_sign(float)` | `S1_FN` | SFU sign: −1, 0, +1, 1 op | A, B |
| `float s1_abs(float)` | `S1_FN` | \|x\|, 1 op (unit B) | A, B |
| `float s1_recip(float)` | `S1_FN` | 1/x: `recipseed` + 3 `newton`, 4 ops, 1e-8 | A, B |
| `float s1_log2(float)` | `S1_FN` | log2 x, x > 0: `log2split` + degree 6, 9 ops, 5e-6 | A, B |
| `float s1_ldexp(float x, int k)` | `S1_FN` | x 2^k, k constant: free (a scale) or one `shl` / `shr` | A, B |
| `float s1_sum3(float, float, float)` | `S1_FN` | a + b + c: `sum8`, 1 op | A, B |
| `float s1_mac2(float a, float b, float c, float d, float e)` | `S1_FN` | a b + c d + e: one MAC op; a constant factor becomes the MAC's immediate coefficient | A, B |
| `float s1_sqrt(float)`, `s1_rsqrt` | `S1_FN` | √x, 1/√x, x > 0: `sqrtsplit` + 2 Newton steps, 11 / 13 ops, 2e-7 | A, B |
| `float s1_env(int i)` | `S1_IN` | `S1_DSP_B` only: host envelope i, 0..1 (0 = AMP, 1 = FILTER); the native reference and `check` give the gate | B |
| `float s1_color(int i)` | `S1_IN` | `S1_DSP_A` only: OSC1 (0) / OSC2 (1) COLOR, 0..1, with its COLOR MOD source; reference and `check`: the knob (`color0=K` / `color1=K`, default 128) | A |
| `float s1_lfo_filter(void)`, `float s1_lfo_amp(void)` | `S1_IN` | host LFO → FILTER (octaves −4..+4) and → AMP (gain 0..1); on A and B; reference and `check`: 0 and 1 | A, B |
| `float s1_host_mix(void)` | `S1_IN` | only `s1cc fuse` calls it: the host's SUB / NOISE between A and B | (fused voice) |

"A / B only" restrictions are enforced by the compiler with errors E114/E115 (export/import), E117 (`s1_env` in A) and
E119 (`s1_color` in B) ([ch. 1](s1cc_compiler_internals.md) §9.2.2, §9.2.7).

### 2.1 Notes

* **`s1_exp2` / `s1_exp` cost** (as `lower.cpp` builds it): up to ±8 octaves of range, Taylor + 4 squarings, 10 ops;
  wider (up to 20 octaves), `expsplit` + a degree-6 polynomial, 10 ops, 6·10⁻⁸ ([ch. 1](s1cc_compiler_internals.md)
  §3, §7.3, §7.6).
* **`s1_ldb5(i)`** (`S1_DSP_A` only): word i (0..126) of the wavetable frame in bank 5 (`A080`…), the frame that the
  host or the USB loader writes; one `hset` + `ldb5` (an indexed bank-5 load), the only way to read a run-time table
  by a computed index. Its use: [Vol. VI ch. 7](../vol6_reference_designs_applications/mutable_memory.md) §4.5.

## 3. Panel ids (`s1panel.h`)

The SYSTEM-1 panel controls as parameter ids for `S1_PARAM(name, panel_id, count)`. Measured with PARAMLOG on OS 1.30,
2026-10-23 ([examples/system_1_synth/README.md](../../examples/system_1_synth/README.md) §8). Knobs and faders send
0..255; selectors send 0..5. Not in the header: VOLUME (`0x100`) goes to the master (region 0), not to the voice; the
HPF cutoff sends nothing.

| group | macro | id | note in the header |
|---|---|---|---|
| LFO | `S1_LFO_SHAPE` | `0x160` | selector 0..5: sine, triangle, saw, square, sample & glide, sample & hold |
| | `S1_LFO_FADE` | `0x161` | delay / fade time |
| | `S1_LFO_RATE` | `0x162` | |
| | `S1_LFO_TO_PITCH` | `0x163` | |
| | `S1_LFO_TO_FILTER` | `0x164` | |
| | `S1_LFO_TO_AMP` | `0x165` | |
| oscillators | `S1_OSC1_WAVE` | `0x167` | selector 0..5 |
| | `S1_OSC1_COLOR` | `0x168` | |
| | `S1_OSC1_COLOR_MOD` | `0x169` | selector 0..5: manual, LFO, pitch env, filter env, amp env, sub osc |
| | `S1_OSC1_RANGE` | `0x16A` | selector 0..5: 64' .. 2' |
| | `S1_OSC2_WAVE` | `0x16C` | |
| | `S1_OSC2_COLOR` | `0x16D` | |
| | `S1_OSC2_COLOR_MOD` | `0x16E` | |
| | `S1_OSC2_RANGE` | `0x16F` | |
| | `S1_OSC2_FINE` | `0x170` | 128 = centre |
| mixer | `S1_OSC1_MIX` | `0x174` | |
| | `S1_OSC2_MIX` | `0x175` | |
| | `S1_SUB_MIX` | `0x176` | |
| | `S1_NOISE_MIX` | `0x177` | |
| pitch envelope | `S1_PITCH_ENV_AMOUNT` | `0x17A` | |
| | `S1_PITCH_ENV_ATTACK` | `0x17B` | |
| | `S1_PITCH_ENV_DECAY` | `0x17C` | |
| filter | `S1_FILTER_CUTOFF` | `0x17D` | |
| | `S1_FILTER_RESONANCE` | `0x17F` | |
| | `S1_FILTER_ENV_AMOUNT` | `0x181` | bipolar: 128 = 0 |
| | `S1_FILTER_ATTACK` | `0x182` | |
| | `S1_FILTER_DECAY` | `0x183` | |
| | `S1_FILTER_SUSTAIN` | `0x184` | |
| | `S1_FILTER_RELEASE` | `0x185` | |
| | `S1_FILTER_KEY_TRACK` | `0x186` | |
| amplifier | `S1_AMP_ATTACK` | `0x187` | |
| | `S1_AMP_DECAY` | `0x188` | |
| | `S1_AMP_SUSTAIN` | `0x189` | |
| | `S1_AMP_RELEASE` | `0x18A` | |
| effects | `S1_REVERB_AMOUNT` | `0x18D` | |
| | `S1_DELAY_AMOUNT` | `0x18E` | |
| | `S1_DELAY_TIME` | `0x18F` | |

In MUTABLE-SYSTEM the host program (C-S1SY) also uses some of these controls; OSC1 / OSC2 COLOR are the free ones. In
SYSTEM-S1CC there is no host: all of them are the kernel's.

## 4. Minimal example

From [ch. 1](s1cc_compiler_internals.md) §7.6 (it uses only declared intrinsics):

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

The complete annotated minimal example is [sdk/s1cc/examples/simple_saw_wrapped/README.md](../../sdk/s1cc/examples/simple_saw_wrapped/README.md).
