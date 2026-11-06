/* wt_c.c: a wavetable oscillator written entirely in C, reading the bank-5 frame with s1_ldb5 (docs/MUTABLE_MEMORY.md
 * §4.5). The frame (127 points + a guard copy of point 0) is written by the host / USB loader (tools/wt_load.py);
 * SL_LOAD=1 gives it a default ramp saw until one is sent. Pitch modulation is free: s1_dt() already carries the
 * host's LFO -> pitch and RANGE. Manipulate the sample in C: it is just `a` and `b` below.
 *
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/wavetable/wt_c.c
 *         KERNEL=sdk/s1cc/build/wt_c sh examples/system_s1cc/build.sh      (-> plugins/WT-C)
 */
#include "s1dsp.h"

S1_DSP_A {
    static float phase;                    /* 0 .. 1 (voice state, one ring word) */
    float inc = 440.0f * s1_exp2(s1_pitch() - 4.75f) * (1.0f / 96000.0f);   /* f / 96000 (pitch: LFO + RANGE in it) */
    phase += inc;
    if (phase >= 1.0f)                     /* the exact wrap s1cc recognises (ph_wrap) */
        phase -= 1.0f;
    float x = phase * 127.0f;              /* position in the 127-point frame */
    int k = (int)x;                        /* 0 .. 126 */
    float f = x - (float)k;                /* fractional part */
    float a = s1_ldb5(k);
    float b = s1_ldb5(k + 1);              /* k+1 up to 127 = the guard (copy of point 0) */
    return a + (b - a) * f;                /* linear interpolation (raw oscillator) */
}

/* DSP B: the host AMP envelope as the VCA (SYSTEM-S1CC builds the ADSR in assembly because the kernel uses s1_env;
 * s1_env(0) = AMP ATTACK/DECAY/SUSTAIN/RELEASE, 0..1). This gates the voice properly -- no drone. */
S1_DSP_B {
    return x * s1_env(0);
}

S1_BUILD("NAME=WT-C SL_LOAD=1");
