/* bq_host2x.c: bq_host.c with the oscillators at two sub-samples (bq_osc2x.h, README.md §9): the voice program without
 * Roland's oscillator programs (make_voice.py --host --osc2x), the oscillators in C.
 *
 * bq_host.c: the Roland Boutique JP-08 voice for SYSTEM-S1CC with the host's envelopes and LFO (README.md §8).
 *
 * The same lifted voice as bq.c (bq_MODEL_host_voice.h: tools/make_voice.py --host), without Roland's ADSRs, LFO and
 * pre program: SYSTEM-S1CC's ARM_ENV / ARM_LFO (examples/system_s1cc/README.md §3.7) take their place:
 *   AMP A D S R    -> s1_env(0) in the cell of Roland's ENV2 (the VCA)
 *   FILTER A D S R -> s1_env(1) in the cell of ENV1 (filter envelope; FILTER ENV AMOUNT is Roland's FLT ENV MOD)
 *   LFO RATE, SHAPE, FADE, TO PITCH, TO FILTER, TO AMP, PITCH ENV: the host's (s1_pitch carries LFO -> PITCH and the
 *                  pitch envelope; s1_lfo_filter() is added to the cutoff in octaves; s1_lfo_amp() multiplies the voice)
 *   OSC2 COLOR + COLOR MOD -> the pulse width (Roland's PWM x its source): s1_color(1)
 * The whole voice is one function: SYSTEM-S1CC's fused build places whole voices on DSP A and DSP B. */
#include <math.h>
#include "s1dsp.h"

#define BQ_PANEL
#include "dsplift_k.h"
#include "bq_osc2x.h"
#include "bq_jp08_host_tables.h"

static float out_voice, osc2_pw, osc1_out, osc2_out;
static bq_osc_t osc1, osc2;

/* OSC2 TUNE as factors of the frequency: 2^tune and 2^-tune (the voice adds C046 octaves to the pitch; here they
   multiply the host's step, computed when the knob moves) */
S1_PARAM(tune2f, S1_OSC2_FINE, 2) { out[0] = exp2f(jp08_tune2[knob][0]); out[1] = 1.0f / out[0]; }
/* OSC2 RANGE: the host puts OSC1 RANGE into s1_pitch / s1_dt (64' .. 2' = -1 .. +4 octaves, position k -> k - 1, rows
   above 5 -> 5: examples/system_s1cc/README.md §3.5), and both oscillators take s1_dt: OSC2 takes OSC1's range out
   and puts its own in, the same way: 2^(k2 - k1) (the same position = the same octave). The JP-08's ARM puts VCO-2's
   range in the per-note factor A089 (2^octaves), not in a parameter word (C055 is the LOW switch: not mapped) */
S1_PARAM(range1f, S1_OSC1_RANGE, 3) { int k = knob > 5 ? 5 : knob; out[0] = exp2f((float)-(k - 1)); out[1] = 1.0f / out[0];
                                      out[2] = (float)(k - 1); }
S1_PARAM(range2f, S1_OSC2_RANGE, 2) { int k = knob > 5 ? 5 : knob; out[0] = exp2f((float)(k - 1)); out[1] = 1.0f / out[0]; }

static inline float bq_in(int a)
{
    switch (a) {
    case 0xa080: return s1_pitch() - s1_param(range1f, 2); /* in A080 pitch: the note, without the host's OSC1 RANGE
                                                            (Roland's filter follows the key, the ranges are A088 / A089) */
    case 0xa081: return s1_range(s1_gate(), 0.0f, 1.0f);  /* in A081 gate */
    case 0xa0f0: return s1_env(1);                        /* in A0F0 env_filter: FILTER A D S R */
    case 0xa0f1: return s1_env(0);                        /* in A0F1 env_amp: AMP A D S R */
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
#include "bq_jp08_host2x_voice.h"
#pragma float_control(pop)

#ifndef BQ_GAIN
#define BQ_GAIN 12.0f
#endif

/* the oscillators' steps from the host (README.md §9): Roland's step at two sub-samples is f / 96000 = s1_dt() (LFO,
   pitch envelope and range already in it), the edges' widths from s1_rdt(); OSC1 adds CROSS MOD (C04D octaves x OSC2's
   output of the sample before, as Roland's c_093), one short exponential */
S1_DSP_A {
    jp08_host2x_voice();
    float dt = s1_dt(), rdt = s1_rdt(), x = KP(0x4d) * osc2_out;
    osc1_out = bq_saw2x(&osc1, dt * s1_exp2(x), 0.25f * rdt * s1_exp2(-x));
    float r2 = s1_param(range2f, 0) * s1_param(range1f, 0), r2i = s1_param(range2f, 1) * s1_param(range1f, 1);   /* 2^(k2 - k1), 2^(k1 - k2) */
    osc2_out = bq_pulse2x(&osc2, dt * s1_param(tune2f, 0) * r2, 0.2f * rdt * s1_param(tune2f, 1) * r2i, osc2_pw);
    return out_voice * BQ_GAIN * s1_lfo_amp();
}

S1_DSP_B { return x; }

S1_BUILD("NAME=JP-08-S1X MIX=0");
S1_COMPILE("--regs 0,1,2,3,4,5,6,7");
