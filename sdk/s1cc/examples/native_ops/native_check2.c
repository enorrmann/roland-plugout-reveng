/* native_check2.c: the native ops measured on the SYSTEM-8 on 2026-10-29 (docs/NATIVE_OPS.md), checked on the
 * SYSTEM-1 by ear, as native_check.c: each op is compared with an identity that does not use it; the voice is a
 * sawtooth times (1 - clamp(K error)): a clean saw at full level when every selected op is right, quieter / silent /
 * rough when one is wrong.
 * OSC1 COLOR picks the ops in the check: knob 0..27 all of them, then one each 28 steps:
 *   28 s1_sqrt (sqrtsplit + Newton)   56 s1_rsqrt (+ recipseed)   84 s1_mac2 with one constant (mac_p1wc3)
 *   112 s1_mac2 with two constants (mac_p1p2c3, Y immediate)   140 |u| - 2 (absadd with an immediate)
 *   168 a b + c d (the MAC fusion, mac_p1w)
 * OSC1 MIX = level. Every op runs whatever the knob says (the knob only weights its error), so a build that hangs
 * the device hangs with any knob position.
 *
 * Build:   sdk/s1cc/s1cc build sdk/s1cc/examples/native_ops/native_check2.c --regs 0,1,2,3,4,5,6,7 -o sdk/s1cc/build/native_check2 --check
 *          KERNEL=sdk/s1cc/build/native_check2 NAME=NATIVE-CHECK2 sh examples/system_s1cc/build.sh */
#include <math.h>
#include "s1dsp.h"

#define NOPS 6
S1_PARAM(sel, S1_OSC1_COLOR, NOPS) {
    int region = knob / 28;                       /* 0 = all, 1..6 = one op */
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
    float x = 0.25f + p;                          /* 0.25 .. 1.25, above 0 for the roots */
    float e[NOPS];
    float r = s1_sqrt(x);
    e[0] = r * r - x;
    float q = s1_rsqrt(x);
    e[1] = q * q * x - 1.0f;
    e[2] = s1_mac2(saw, x, 0.375f, p, 0.0f) - (saw * x + 0.375f * p);
    e[3] = s1_mac2(saw, 0.625f, -0.25f, x, p) - (0.625f * saw - 0.25f * x + p);
    float u = saw;                                 /* the polyBLEP's |u| - 2 (absadd #-2) against sign(u) u - 2 */
    e[4] = (fabsf(u) - 2.0f) * u - (s1_sign(u) * u - 2.0f) * u;
    e[5] = (saw * x + p * dt) - s1_sum3(saw * x, p * dt, 0.0f);
    float err = 0.0f;
    for (int k = 0; k < NOPS; k++)
        err += s1_param(sel, k) * fabsf(e[k]);
    return saw * s1_param(level, 0) * (1.0f - s1_clamp(64.0f * err)) * s1_gate();
}
