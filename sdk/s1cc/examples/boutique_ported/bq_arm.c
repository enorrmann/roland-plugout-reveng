/* bq_arm.c: bq_host2x.c with the filter's coefficient and the VCA's curve computed on the ARM (SYSTEM-S1CC-BOUTIQUE
 * ARM_CUSTOM, bq_jp08_arm.h by tools/make_arm.py; the voice: make_voice.py --host --osc2x --arm; README.md §11): the
 * ARM writes g and the VCA gain into the envelope words every 1 ms, the kernel reads them as s1_env(1), s1_env(0).
 * Build with examples/system_s1cc_boutique/build.sh.
 *
 * bq_host2x.c: bq_host.c with the oscillators at two sub-samples (bq_osc2x.h, README.md §9): the voice program without
 * Roland's oscillator programs (make_voice.py --host --osc2x), the oscillators in C.
 *
 * bq_host.c: the Roland Boutique JP-08 voice for SYSTEM-S1CC with the host's envelopes and LFO (README.md §8).
 *
 * The same lifted voice as bq.c (bq_MODEL_host_voice.h: tools/make_voice.py --host), without Roland's ADSRs, LFO and
 * pre program: SYSTEM-S1CC's ARM_ENV / ARM_LFO (examples/system_s1cc/README.md §3.7) take their place:
 *   AMP A D S R    -> s1_env(0) in the cell of Roland's ENV2 (the VCA)
 *   FILTER A D S R -> s1_env(1) in the cell of ENV1 (filter envelope; FILTER ENV AMOUNT is Roland's FLT ENV MOD)
 *   LFO RATE, SHAPE, FADE, TO FILTER: the host's (s1_lfo_filter() is added to the cutoff in octaves).
 *                  bq_arm.c (README.md §13): no host LFO -> PITCH, pitch envelope or LFO -> AMP (LFO_PITCH=0); LFO TO
 *                  PITCH, PITCH ENV AMOUNT and LFO TO AMP are the JP-08's OSC LFO MOD, OSC ENV MOD, AMP LFO MOD on the ARM
 *   OSC2 COLOR + COLOR MOD -> the pulse width (Roland's PWM x its source): s1_color(1)
 * The whole voice is one function: SYSTEM-S1CC's fused build places whole voices on DSP A and DSP B. */
#include <math.h>
#include "s1dsp.h"

#define BQ_PANEL_HOST
#include "dsplift_k.h"
#include "bq_osc2x.h"
#include "bq_jp08_tables.h"

/* the ARM's values, smoothed: they change every 1 ms (a step every 96 samples); a one-pole of ~1 ms takes the steps'
   1 kHz buzz away (measured without: -52 dB under a fast filter-envelope sweep, README.md §11) */
#define ARM_SMOOTH 0.0104f
static float sm_g, sm_amp;
static float out_voice, osc2_pw, osc1_out, osc2_out;
static bq_osc_t osc1 __attribute__((used)), osc2 __attribute__((used));   /* used: LLVM keeps every field (S1_FIXED_STATES) */
/* the state words in declaration order with all fields (s1dsp.h): the wave pairs' programs (README.md §12) keep the
   filter, the envelopes' smoothing and each oscillator's state at the same words, so a program change while a note
   plays continues them instead of reading another program's values (the click of 2026-11-12) */
S1_FIXED_STATES;

/* OSC2 TUNE as factors of the frequency: 2^tune and 2^-tune (the voice adds C046 octaves to the pitch; here they
   multiply the host's step, computed when the knob moves) */
S1_PARAM(tune2f, S1_OSC2_FINE, 2) { out[0] = exp2f(jp08_tune2[knob][0]); out[1] = 1.0f / out[0]; }
/* OSC1 / OSC2 RANGE and the pitch modulation (README.md §13): the ARM (bq_jp08_arm.h arm_custom_pitch) writes the
   note words: s1_dt / s1_rdt = OSC1's step (note, OSC1 RANGE, LFO x OSC LFO MOD + ENV1 x OSC ENV MOD when OSC FREQ MOD
   DST includes VCO-1) and s1_pitch = 4.75 + OSC2 over OSC1 in octaves (OSC2 RANGE and its own modulation; 4.75 so
   that s1cc's profile, which gives the note's pitch, sees A 440 = the same step), as the JP-08 (lifted
   instructions 211..231). The build has LFO_PITCH=0: no SYSTEM-1 LFO -> PITCH, no pitch envelope */

static inline float bq_in(int a)
{
    switch (a) {
    case 0xa080: return 0.0f;                             /* in A080 pitch: KEY TRACK is the ARM's (u), the oscillators
                                                            take the ARM's steps: unused */
    case 0xa081: return s1_range(s1_gate(), 0.0f, 1.0f);  /* in A081 gate */
    case 0xa0f5: return sm_g;                            /* the filter's g (ARM: knobs, FILTER envelope, note, LFO) */
    case 0xa0f6: return sm_amp;                          /* the VCA gain (ARM: Roland's curve of the AMP envelope) */
    case 0xa0f2: return s1_lfo_filter();                  /* in A0F2 lfo_filter: octaves */
    case 0xa090: return s1_color(1);                      /* in A090 pwm */
    case 0xa0f3: return osc1_out;                         /* in A0F3: OSC1 of the sample before */
    case 0xa0f4: return osc2_out;                         /* in A0F4: OSC2 of the sample before */
    default: return 0.0f;
    }
}

#define KP(a) jp08_kp((a) & 0xff)
#define KIN(a) bq_in(a)
#define KOUT(a, v) do { switch (a) { case 0x203a: out_voice = (v); break; case 0x200b: osc2_pw = (v); break; } } while (0)
#define KE(a) 0.0f
#pragma float_control(precise, off, push)
#include "bq_jp08_arm_voice.h"
#pragma float_control(pop)

/* the wave pair (README.md §12): BQ_OSC1 0 sine, 1 triangle, 2 saw, 3 pulse, 4 square, 5 noise (ours); BQ_OSC2 0 sine,
   1 saw, 2 pulse */
#ifndef BQ_OSC1
#define BQ_OSC1 2
#endif
#ifndef BQ_OSC2
#define BQ_OSC2 2
#endif
/* the triangle's frequency word (Roland's c_1ef = 2^(pitch - 4.75) x A088) from the step: inc = f / 96000 and
   c_1ef = f / 440 x A088, so c_1ef = inc x 96000 / 440 x 0.459375 */
#define BQ_TRI_FREQ (96000.0f / 440.0f * 0.459375f)
/* OSC1 WAVE 4 (square): the pulse program with the width input 0 [inferred: the JP-08 ARM writes no parameter word
   for position 3 / 4 (tools/bq_fw.py, 2026-11-12), so the difference must be the per-note PWM inputs; TEST] */
#define BQ_SQUARE_PW 0.0f

#ifndef BQ_GAIN
#define BQ_GAIN 12.0f
#endif

/* the oscillators' steps from the host (README.md §9): Roland's step at two sub-samples is f / 96000 = s1_dt() (LFO,
   pitch envelope and range already in it), the edges' widths from s1_rdt(); OSC1 adds CROSS MOD (C04D octaves x OSC2's
   output of the sample before, as Roland's c_093), one short exponential */
S1_DSP_A {
    sm_g += ARM_SMOOTH * (s1_env(1) - sm_g);
    sm_amp += ARM_SMOOTH * (s1_env(0) - sm_amp);
    jp08_arm_voice();
    float dt = s1_dt(), rdt = s1_rdt(), x = KP(0x4d) * osc2_out;
    float d2 = s1_pitch() - 4.75f;                        /* the ARM: OSC2 over OSC1 in octaves (range, modulation) */
    float inc1 = dt * s1_exp2(x), rdt1 = rdt * s1_exp2(-x);
    float inc2 = dt * s1_param(tune2f, 0) * s1_exp2(d2), rdt2 = rdt * s1_param(tune2f, 1) * s1_exp2(-d2);
    /* the waveform pair of this program: BQ_OSC1 / BQ_OSC2 = the JP-08's OSC1 / OSC2 WAVE position (Roland's program
       of that position, README.md §12); one program per pair (waves/*.c), the PLUG-OUT changes the program when a
       WAVE knob moves (SYSTEM-S1CC-BOUTIQUE build_multi.sh SECT_SWITCH). Default: OSC1 saw (2), OSC2 pulse (2) */
#if BQ_OSC1 == 0
    osc1_out = bq_sine2x(&osc1, inc1);
#elif BQ_OSC1 == 1
    osc1_out = bq_tri2x(&osc1, inc1, inc1 * BQ_TRI_FREQ);
#elif BQ_OSC1 == 2
    osc1_out = bq_saw2x(&osc1, inc1, 0.25f * rdt1);
#elif BQ_OSC1 == 3
    osc1_out = bq_pulse2x(&osc1, inc1, 0.2f * rdt1, osc2_pw);
#elif BQ_OSC1 == 4
    osc1_out = bq_pulse2x(&osc1, inc1, 0.2f * rdt1, BQ_SQUARE_PW);
#elif BQ_OSC1 == 5
    osc1_out = bq_noise(&osc1);                         /* not Roland's noise yet (bq_osc2x.h, README.md §12) */
#else
#error "BQ_OSC1: 0..5"
#endif
#if BQ_OSC2 == 0
    osc2_out = bq_sine2x(&osc2, inc2);
#elif BQ_OSC2 == 1
    osc2_out = bq_saw2x(&osc2, inc2, 0.25f * rdt2);
#elif BQ_OSC2 == 2
    osc2_out = bq_pulse2x(&osc2, inc2, 0.2f * rdt2, osc2_pw);
#else
#error "BQ_OSC2: 0..2"
#endif
    return out_voice * BQ_GAIN;                         /* AMP LFO MOD: the ARM's VCA gain (README.md §13) */
}

S1_DSP_B { return x; }

S1_BUILD("NAME=JP-08-S1A MIX=0 LFO_PITCH=0 ARM_CUSTOM=sdk/s1cc/examples/boutique_ported/bq_jp08_arm.h PANEL=sdk/s1cc/examples/boutique_ported/bq_arm.panel");
S1_COMPILE("--regs 0,1,2,3,4,5,6,7");
