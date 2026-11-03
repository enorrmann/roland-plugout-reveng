/* mp_phase_distortion.c: Casio CZ phase distortion (Plaits' phase distortion engine, its basic shape): the phase
 * runs fast up to a knee, slow after it, and drives a cosine: from sine to saw-like.
 * OSC1 COLOR = knee (distortion), with its COLOR MOD source (s1_color); OSC1 MIX = level. DSP B: mp_dsp_b.h. */
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"


/* RANGE selectors 0..5 = 64' .. 2' = -3 .. +2 octaves (anything else, as the row 128 before the panel arrives: 8') */
static float range_oct(int knob) { return knob <= 5 ? knob - 3.0f : 0.0f; }
/* DCO2's ratio to the host's dt and its inverse (for 1/dt, no division on the DSP), one table per control */
S1_PARAM(range1, S1_OSC1_RANGE, 2) { out[0] = powf(2.0f, -range_oct(knob)); out[1] = 1.0f / out[0]; }
S1_PARAM(range2, S1_OSC2_RANGE, 2) { out[0] = powf(2.0f, range_oct(knob)); out[1] = 1.0f / out[0]; }
S1_PARAM(fine2, S1_OSC2_FINE, 2) { out[0] = powf(2.0f, (knob - 128) / 128.0f / 12.0f); out[1] = 1.0f / out[0]; }

S1_PARAM(level1, S1_OSC1_MIX, 1) { out[0] = 0.5f * knob / 255.0f; }    /* two oscillators: half the level of one */
S1_PARAM(level2, S1_OSC2_MIX, 1) { out[0] = 0.5f * knob / 255.0f; }

static float ph1;
static float ph2;

static inline float render_oscillator(float *phase, float dt, float color, float level)
{
    float p = mp_advance(phase, dt);

    float a = s1_exp2(4.0589f * color);

    float b = 0.97436f + color *
              (-2.03726f + color *
              (4.05872f + color *
              (-3.9256f + color * 1.45078f)));

    float w = fminf(p * a, 1.0f - b * (1.0f - p));

    return -mp_cos(w) * level;
}

S1_DSP_A {
    float dt = mp_dt();

    float k = s1_param(range1, 0) *
              s1_param(range2, 0) *
              s1_param(fine2, 0);

    float dt2 = mp_step(dt, k);

    /* Oscillator 1 */
    float y = render_oscillator(
        &ph1,
        dt,
        s1_color(0),
        s1_param(level1, 0)
    );

    /* Oscillator 2 */
    y += render_oscillator(
        &ph2,
        dt2,
        s1_color(1),
        s1_param(level2, 0)
    );

    return y;
}