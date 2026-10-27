/* s1dsp.h: the C / C++ SDK of s1cc, the compiler from C / C++ to the microcode of the Roland BMC DSPs
 * (docs/S1CC.md). One source, compiled once by clang to LLVM IR, is used three ways:
 *   - translated to microcode by s1cc (the s1_* functions below are intrinsics there);
 *   - compiled natively with sdk/s1cc/runtime/s1rt.c: the reference the microcode is checked against, and the
 *     knob tables (S1_PARAM functions run on the PC, for knob = 0..255);
 *   - compiled natively with profiling calls inserted by s1cc: value ranges, overflows, tabulated functions.
 *
 * Program model. Each slave voice runs through two DSPs in series, A then B; the code of S1_DSP_A / S1_DSP_B is the
 * program of ONE voice, copied 8 times (8 voices, each with its own copy of the state). When each part runs:
 * docs/S1CC.md §3.1.
 *   S1_DSP_A { ... return y; }    slave DSP A, every sample, every voice (playing or not): the voice signal
 *                                   (about -1..1), in place of the host's DCO1 + DCO2. All the oscillators go here
 *   S1_DSP_B { ... return y; }    optional, slave DSP B, every sample, every voice; x = the host's voice signal
 *                                   (what S1_DSP_A returned, after the host's OSC1 MIX / tremolo). B is the second
 *                                   DSP, not a second oscillator: a filter / shaper of x; then the host's VCA, filter
 *   file-scope variables (int or float, not const) = state of the voice (one ring word each; constant-index
 *                                   arrays are fine: clang splits them)
 *   file-scope const arrays = tables: a constant index is folded; a computed one becomes part of a function that
 *                                   s1cc fits (polynomial, exp2 of a polynomial, with saturation)
 *   S1_PARAM(name, panel_id, count) { out[0..count-1] = f(knob); }   a knob table: runs on the PC at build time,
 *                                   for knob = 0..255. On the synth the ARM writes the knob's row into DSP
 *                                   parameters: row 128 at load, then whenever the control moves.
 *                                   panel_id: a SYSTEM-1 control, S1_OSC1_COLOR ... (s1panel.h, included here)
 *   S1_INIT    { obj.Init(); ... }  optional: runs on the PC only, before the reference and every profiling
 *                                   scenario; never on the DSP. The values it leaves in the state are the voice's
 *                                   initial state on the DSP too (a first-sample correction, docs/S1CC.md §11.1).
 *                                   Not a note-on: for that, s1_trig() in the voice
 *
 * Intrinsics:
 *   s1_pitch()            pitch of the voice in octaves (4.75 = A 440; LFO and range applied by the host)
 *   s1_increment()        phase step of that pitch for a phase -1..1: 2 f / 96000
 *   s1_gate(), s1_trig()  gate 0 / 1; trigger: toggles 0 / 1 at every note start
 *   s1_param(name, i)     value i of the knob table `name` (a DSP parameter C0xx the ARM writes)
 *   s1_export(i, v), s1_import(i)   a value from S1_DSP_A to S1_DSP_B, same sample
 *   s1_wrap(x)            x wrapped into [-1, 1)         s1_clamp(x)   x limited to [-1, 1]
 *   s1_tri(x)             1 - 2 |wrap(-x - 1.5)|         s1_exp2(x)    2^x
 *   s1_range(x, lo, hi)   x, declaring its range: lo and hi go to s1cc as constants, and the profile must reach
 *                         the call (a branch that always takes the same way leaves it unseen: docs/S1CC.md §3.2)
 */
#ifndef S1DSP_H_
#define S1DSP_H_

#ifdef __cplusplus
#define S1_EXTERN extern "C"
#else
#define S1_EXTERN
#endif

#include "s1panel.h"

typedef struct s1_param_desc {
    const char *name;
    int panel_id;
    int count;
    void (*fn)(int knob, float *out);
} s1_param_desc;

S1_EXTERN float s1_pitch(void);
S1_EXTERN float s1_increment(void);
S1_EXTERN float s1_gate(void);
S1_EXTERN float s1_trig(void);
S1_EXTERN float s1_param_get(const s1_param_desc *param, int i);
S1_EXTERN void s1_export(int i, float v);
S1_EXTERN float s1_import(int i);
S1_EXTERN float s1_wrap(float x);
S1_EXTERN float s1_clamp(float x);
S1_EXTERN float s1_tri(float x);
S1_EXTERN float s1_exp2(float x);
S1_EXTERN float s1_range(float x, float lo, float hi);

#define s1_param(name, i) s1_param_get(&s1_param_desc_##name, (i))

#define S1_PARAM(name, panel_id, count)                                                                      \
    static void s1_param_fn_##name(int knob, float *out);                                                    \
    __attribute__((used, section("s1_params"))) const s1_param_desc s1_param_desc_##name =                   \
        { #name, panel_id, count, s1_param_fn_##name };                                                      \
    static void s1_param_fn_##name(int knob, float *out)

#define S1_DSP_A S1_EXTERN float s1_dsp_a(void)
#define S1_INIT S1_EXTERN void s1_init(void)
#define S1_DSP_B S1_EXTERN float s1_dsp_b(float x)

#endif
