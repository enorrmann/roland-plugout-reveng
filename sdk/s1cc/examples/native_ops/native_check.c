/* native_check.c: the DSP's native ops (s1dsp.h "Native ops", docs/S1CC.md §4.5) checked on the synth by ear.
 * Each op is compared with an identity that does not use it; the voice is a sawtooth times (1 - clamp(K error)):
 * a clean saw at full level when every selected op is right, quieter / silent / rough when one is wrong.
 * OSC1 COLOR picks the ops in the check: knob 0..27 all of them, then one each 28 steps:
 *   28 s1_recip   56 s1_log2 + s1_exp   84 s1_floor   112 s1_sign   140 s1_mac2   168 s1_sum3   196 s1_ldexp (shl)
 *   224 s1_exp2 over 18 octaves (expsplit) + s1_ceil
 * OSC1 MIX = level. Every op runs whatever the knob says (the knob only weights its error), so a build that hangs
 * the device hangs with any knob position.
 *
 * Build:   sdk/s1cc/s1cc build sdk/s1cc/examples/native_ops/native_check.c --regs 0,1,2,3,4,5,6,7 -o sdk/s1cc/build/native_check --check
 *          KERNEL=sdk/s1cc/build/native_check NAME=NATIVE-CHECK sh examples/system_s1cc/build.sh */
#include <math.h>
#include "s1dsp.h"

#define NOPS 8
S1_PARAM(sel, S1_OSC1_COLOR, NOPS) {
    int region = knob / 28;                       /* 0 = all, 1..8 = one op */
    for (int k = 0; k < NOPS; k++)
        out[k] = region == 0 || region == k + 1 ? 1.0f : 0.0f;
}
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
    float dt = s1_dt();
    float p = ph + dt;
    if (p >= 1.0f) p -= 1.0f;
    ph = p;
    float saw = p + p - 1.0f;
    float x = 0.25f + p;                          /* 0.25 .. 1.25, above 0 for 1/x and log2 */
    float y = 4.0f * p;                           /* 0 .. 4: floor steps */
    float fr = 0.5f * s1_wrap(y + y - 1.0f) + 0.5f;   /* frac(y) without floor */
    float e[NOPS];
    e[0] = s1_recip(x) * x - 1.0f;
    e[1] = s1_exp(s1_log2(x) * 0.69314718f) - x;
    e[2] = y - s1_floor(y) - fr;
    e[3] = s1_sign(saw) * saw - fabsf(saw);
    e[4] = s1_mac2(saw, x, dt, y, p) - (saw * x + dt * y + p);
    e[5] = s1_sum3(saw, x, y) - (saw + x + y);
    e[6] = s1_ldexp(saw * dt, 12) - saw * dt * 4096.0f;
    float t = 17.8f * p - 9.0f;                   /* 2^-9 .. 2^8.8: wide enough for the exp split */
    e[7] = s1_log2(s1_exp2(t)) - t + s1_ceil(y) + s1_floor(-y);
    float err = 0.0f;
    for (int k = 0; k < NOPS; k++)
        err += s1_param(sel, k) * fabsf(e[k]);
    return saw * s1_param(level, 0) * (1.0f - s1_clamp(64.0f * err)) * s1_gate();
}
