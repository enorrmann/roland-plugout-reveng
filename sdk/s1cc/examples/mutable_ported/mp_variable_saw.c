/* mp_variable_saw.c: Plaits VariableSawOscillator: a saw whose ramp is split by a notch at pw, and a waveshape that
 * morphs the two ramps from a notched saw (both falling) to a triangle-like shape (the second rising): the sum and
 * the difference of two band-limited saws pw apart.
 * OSC1 COLOR = pw, OSC2 COLOR = waveshape, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float p = mp_advance(&ph, dt);
  float pw = fminf(fmaxf(0.5f + 0.48f * (s1_color(0) + s1_color(0) - 1.0f), 2.0f * dt), 1.0f - 2.0f * dt);
  float s0 = mp_saw_bl(p, dt, rdt), s1 = mp_saw_bl(p - pw, dt, rdt);
  float notch = 0.5f * (s0 + s1);                        /* two falling ramps */
  float pulse = s1 - s0 + pw + pw - 1.0f;
  /* waveshape: the second ramp turned up = notch + (pulse - ramp offset): crossfade of the two forms */
  float tri = notch + 0.5f * pulse;
  return (notch + (tri - notch) * s1_color(1)) * s1_param(level, 0);
}
