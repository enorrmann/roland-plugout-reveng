/* native_new.c: the native ops measured on the SYSTEM-8 on 2026-10-29, as s1cc uses them (docs/NATIVE_OPS.md):
 * s1_mac2 with a constant factor (the MAC's immediate coefficient K) and with two (Y immediate and K), s1_sqrt and
 * s1_rsqrt (sqrtsplit + Newton). The voice is a mix of the results; tests/test_s1cc.py compares it sample by sample
 * with the C reference (SNR) and checks the ops are there (the kernel listing).
 *
 * Build:   sdk/s1cc/s1cc build sdk/s1cc/examples/native_ops/native_new.c --regs 0,1,2,3,4,5,6,7 -o sdk/s1cc/build/native_new */
#include "s1dsp.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
    float dt = s1_dt();
    float p = ph + dt;
    if (p >= 1.0f) p -= 1.0f;
    ph = p;
    float saw = p + p - 1.0f;
    float x = 0.25f + p;                                  /* 0.25 .. 1.25 */
    float a = s1_mac2(saw, x, 0.375f, saw, 0.0f);         /* saw x + K saw: mac_p1wc3 */
    float b = s1_mac2(saw, 0.625f, -0.25f, x, p);         /* Y immediate and K, with W: mac_p1p2c3 */
    float c = s1_sqrt(x) - 0.75f;                         /* 0.5 .. 1.118 */
    float d = 0.5f * s1_rsqrt(x) - 0.6f;                  /* 0.447 .. 1 */
    return (0.3f * a + 0.3f * b + 0.4f * c + 0.4f * d) * s1_param(level, 0) * s1_gate();
}
