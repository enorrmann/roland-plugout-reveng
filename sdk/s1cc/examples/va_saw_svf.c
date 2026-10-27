/* A float voice in plain C (docs/S1CC.md §11): band-limited saw (polyBLEP, as stmlib / Plaits), a state-variable
 * low-pass with tanh drive. Written like ordinary portable DSP code: branches, a libm call, a knob table. */
#include <math.h>
#include "s1dsp.h"

/* OSC1 COLOR: cutoff 50 Hz .. 12 kHz -> SVF coefficient g = tan(pi fc / fs) (computed on the PC) */
S1_PARAM(cutoff, S1_OSC1_COLOR, 1) {
    float fc = 50.0f * powf(240.0f, knob / 255.0f);
    out[0] = tanf(3.14159265f * fc / 96000.0f);
}

/* OSC2 COLOR: resonance -> damping 2 .. 0.1 */
S1_PARAM(damping, S1_OSC2_COLOR, 1) { out[0] = 2.0f - 1.9f * knob / 255.0f; }

static float phase;      /* 0..1 */
static float ic1, ic2;   /* SVF state (Cytomic / Simper form) */

static inline float poly_blep(float t, float dt) {
    if (t < dt) { t /= dt; return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
    return 0.0f;
}

S1_VOICE {
    float dt = s1_increment() * 0.5f;           /* our step is for a phase -1..1 */
    phase += dt;
    if (phase >= 1.0f) phase -= 1.0f;
    float saw = 2.0f * phase - 1.0f - poly_blep(phase, dt);
    return saw * 0.5f;
}

S1_VOICE_B {
    float g = s1_param(cutoff, 0), k = s1_param(damping, 0);
    float a1 = 1.0f / (1.0f + g * (g + k));     /* a function of the knobs only: tabulated */
    float v3 = tanhf(1.5f * x) - ic2;
    float v1 = a1 * ic1 + g * a1 * v3;
    float v2 = ic2 + g * v1;
    ic1 = 2.0f * v1 - ic1;
    ic2 = 2.0f * v2 - ic2;
    return v2;
}
