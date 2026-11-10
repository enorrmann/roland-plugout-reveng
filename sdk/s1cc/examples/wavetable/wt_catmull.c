/* wt_c.c: a wavetable oscillator written entirely in C, reading the bank-5 frame with s1_ldb5
 * (docs/vol6_reference_designs_applications/mutable_memory.md §4.5). The 127-point frame (+ a guard copy of point 0) in bank 5 A080.. is written by the
 * host / USB loader (tools/wt_load.py); SL_LOAD=1 gives it a default ramp saw until one is sent. The sample is just
 * `a` and `b` below -- manipulate it in C. DSP B = the mutable_ported filter + AMP envelope (mp_dsp_b.h), as jp8_saw.
 *
 * Pitch: the phase advances by mp_dt() = s1_dt() (f / 96000, two loads, the host's LFO -> pitch and RANGE already in
 * it) -- no exp2, no division, unlike computing it from s1_pitch(). odd voices read with h7 / ldb5h7 so two concurrent
 * voices never share the one hidden index register (make_s1cc _h67, as SL_H67). --no-fused keeps every voice's
 * oscillator on slave DSP A, where the loader writes the frame (DSP B has its own, empty bank 5).
 *
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/wavetable/wt_c.c
 *         KERNEL=sdk/s1cc/build/wt_c sh examples/system_s1cc/build.sh          (-> plugins/WT-C)
 * Install: python3 tools/plugout_tool.py install plugins/WT-C --slot 1 --force --no-patches -y   (then select it)
 */


 #define MP_FILTER MP_LADDER

#include "../mutable_ported/include/mp_dsp_a.h"
#include "../mutable_ported/include/mp_dsp_b.h"

/* OSC1 MIX: the oscillator's level in the mixer. SUB MIX (0x176) and NOISE MIX (0x177) are the host's mixer, added
 * after this (MIX=1 below): x_DSP_B = OSC1 x osc1mix + SUB x submix + NOISE x noise. RANGE (0x16A) is already in the
 * pitch (the host scales s1_dt by it). */
S1_PARAM(osc1mix, S1_OSC1_MIX, 1) { out[0] = knob / 255.0f; }

static float ph;                               /* phase 0..1 (voice state) */


static inline float interpolate_cubic(float y0, float y1,
                                      float y2, float y3, float t)
{
    float a = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
    float b = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    float c = -0.5f * y0 + 0.5f * y2;

    return ((a * t + b) * t + c) * t + y1;
}

S1_DSP_A {
    float p = mp_advance(&ph, mp_dt());
    float x = p * 127.0f;

    int k = (int)x;
    float f = x - (float)k;

    float y0 = s1_ldb5(k - 1);
    float y1 = s1_ldb5(k);
    float y2 = s1_ldb5(k + 1);
    float y3 = s1_ldb5(k + 2);

    return interpolate_cubic(y0, y1, y2, y3, f)
           * s1_param(osc1mix, 0);
}

S1_BUILD("NAME=WT-C SL_LOAD=1 MIX=1");         /* MIX=1: the host's SUB (0x176) + NOISE (0x177) mixer */
S1_COMPILE("--regs 0,1,2,3 --no-fused");
