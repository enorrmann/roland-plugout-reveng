/* bq_host.c: the Roland Boutique JP-08 voice for SYSTEM-S1CC with the host's envelopes and LFO (README.md §8).
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

#define BQ_PANEL_HOST
#include "bq_jp08_tables.h"

static float out_voice;

static inline float bq_in(int a)
{
    switch (a) {
    case 0xa080: return s1_pitch();                       /* bq_names_host.txt: in A080 pitch */
    case 0xa081: return s1_range(s1_gate(), 0.0f, 1.0f);  /* in A081 gate */
    case 0xa0f0: return s1_env(1);                        /* in A0F0 env_filter: FILTER A D S R */
    case 0xa0f1: return s1_env(0);                        /* in A0F1 env_amp: AMP A D S R */
    case 0xa0f2: return s1_lfo_filter();                  /* in A0F2 lfo_filter: octaves */
    case 0xa090: return s1_color(1);                      /* in A090 pwm */
    default: return 0.0f;
    }
}

#define KP(a) jp08_kp((a) & 0xff)
#define KIN(a) bq_in(a)
#define KOUT(a, v) do { out_voice = (v); } while (0)
#define KE(a) 0.0f
#pragma float_control(precise, off, push)
#include "bq_jp08_host_voice.h"
#pragma float_control(pop)

#ifndef BQ_GAIN
#define BQ_GAIN 20.0f
#endif

S1_DSP_A {
    jp08_host_voice();
    return out_voice * BQ_GAIN * s1_lfo_amp();
}

S1_DSP_B { return x; }

S1_BUILD("NAME=JP-08-S1H MIX=0");
S1_COMPILE("--regs 0,1,2,3,4,5,6,7");
