/* mp_csaw.c: Braids CSAW, the saw with a notch: the output sits at a "depth" level until the phase reaches the
 * notch width, then jumps to the ramp (polyBLEP at the notch and at the wrap).
 * OSC1 COLOR = notch width (TIMBRE), OSC2 COLOR = notch depth (COLOR), both with their COLOR MOD (s1_color), OSC1
 * MIX = level. DSP B: mp_dsp_b.h. */
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  /* Braids: pw = TIMBRE x 0.75; d = level of the notch, 0 = ramp start */
  float pw = fmaxf(0.02f + 0.73f * s1_color(0), 4.0f * dt), d = -0.125f + 0.25f * s1_color(1);
  float p = mp_advance(&ph, dt);
  /* two band-limited saws: s0 with the wrap's edge, s1 with the notch's; the notch indicator [p < pw] and the ramp
     come out of them band-limited, and v = ramp (1 - I) + depth I */
  float s0 = mp_saw_bl(p, dt, rdt), s1 = mp_saw_bl(p - pw, dt, rdt);
  float in = pw - 0.5f * (s0 - s1);
  float ramp = 0.5f * s0 + 0.5f;
  float v = ramp + (d - ramp) * in;
  return (v + v - 1.0f) * s1_param(level, 0);
}
