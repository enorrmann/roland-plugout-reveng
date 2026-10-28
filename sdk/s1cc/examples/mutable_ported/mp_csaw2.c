/* mp_csaw2.c: two DCOs, each Braids CSAW (mp_csaw.c's saw with a notch), the second one's pitch with its own RANGE
 * and FINE TUNE: the first test of a second DCO.
 *   DCO1: the note (with LFO TO PITCH, PITCH ENV and OSC1 RANGE, which the host applies to s1_dt), OSC1 COLOR = notch
 *         width, OSC1 MIX = level
 *   DCO2: the same note x 2^(OSC2 RANGE - OSC1 RANGE + FINE): the host already put OSC1 RANGE in s1_dt, DCO2 takes it
 *         out again; OSC2 COLOR = its notch width, OSC2 MIX = its level, FINE TUNE +-1 semitone (128 = in tune)
 * Both COLORs follow their COLOR MOD source (s1_color); the notch depth stays in the middle (mp_csaw's at knob 128).
 * DSP B: mp_dsp_b.h. */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level1, S1_OSC1_MIX, 1) { out[0] = 0.5f * knob / 255.0f; }    /* two oscillators: half the level of one */
S1_PARAM(level2, S1_OSC2_MIX, 1) { out[0] = 0.5f * knob / 255.0f; }

/* RANGE selectors 0..5 = 64' .. 2' = -3 .. +2 octaves (anything else, as the row 128 before the panel arrives: 8') */
static float range_oct(int knob) { return knob <= 5 ? knob - 3.0f : 0.0f; }
/* DCO2's ratio to the host's dt and its inverse (for 1/dt, no division on the DSP), one table per control */
S1_PARAM(range1, S1_OSC1_RANGE, 2) { out[0] = powf(2.0f, -range_oct(knob)); out[1] = 1.0f / out[0]; }
S1_PARAM(range2, S1_OSC2_RANGE, 2) { out[0] = powf(2.0f, range_oct(knob)); out[1] = 1.0f / out[0]; }
S1_PARAM(fine2, S1_OSC2_FINE, 2) { out[0] = powf(2.0f, (knob - 128) / 128.0f / 12.0f); out[1] = 1.0f / out[0]; }

/* Braids CSAW as mp_csaw.c, one oscillator: phase *ph, step dt and its inverse, notch width c (0..1), depth d.
 * Two band-limited saws: s0 with the wrap's edge, s1 with the notch's; the notch indicator [p < pw] and the ramp
 * come out of them band-limited, and v = ramp (1 - I) + depth I */
static inline float csaw(float *ph, float dt, float rdt, float c, float d) {
  float pw = fmaxf(0.02f + 0.73f * c, 4.0f * dt);
  float p = mp_advance(ph, dt);
  float s0 = mp_saw_bl(p, dt, rdt), s1 = mp_saw_bl(p - pw, dt, rdt);
  float in = pw - 0.5f * (s0 - s1);
  float ramp = 0.5f * s0 + 0.5f;
  float v = ramp + (d - ramp) * in;
  return v + v - 1.0f;
}

static float ph1, ph2;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float k = s1_param(range1, 0) * s1_param(range2, 0) * s1_param(fine2, 0);           /* DCO2 / DCO1 */
  float rk = s1_param(range1, 1) * s1_param(range2, 1) * s1_param(fine2, 1);
  float dt2 = mp_step(dt, k), rdt2 = rdt * rk;
  float y = csaw(&ph1, dt, rdt, s1_color(0), 0.0f) * s1_param(level1, 0);
  y += csaw(&ph2, dt2, rdt2, s1_color(1), 0.0f) * s1_param(level2, 0);
  return y;
}
