/* mp_z.c: Plaits ZOscillator (the grain engine's formant voice): a formant sine restarted at every half period of
 * the note, under a falling raised-cosine window and a contour, with the CZ-style phase shift / offset modes. The
 * formant phase is frac(d r) of the half-period phase d (no state of its own). Plaits puts a BLEP on the restart
 * jump; here it is naive (the window is ~0 there, the jump is the mode's offset only).
 * OSC1 COLOR = formant ratio (1 .. 16 x twice the note), OSC2 COLOR = mode, carrier shape 0.25 (half way to the
 * plain cosine contour), OSC1 MIX = level. DSP B: mp_dsp_b.h. The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt();
  float c = mp_advance(&ph, dt);                         /* carrier phase */
  float d = mp_frac(c + c);                              /* discontinuity phase, 2 f0 */
  float f = mp_frac(d * s1_exp2(4.0f * s1_color(0)));    /* formant phase, reset with d */
  float mode = s1_color(1);
  float ramp_down = 0.5f + 0.5f * mp_cos(0.5f * d);      /* 1 -> 0 over the half period */
  /* modes: offset 1 / -sin(shift) / ~0, phase shift 0.25 .. 0.75 (Plaits' three thirds) */
  float shift = mode < 0.333f ? 0.25f + 1.5f * mode : 0.997f - 0.75f * mode;
  float off = mode < 0.333f ? 1.0f : mode < 0.666f ? -mp_sin(mp_frac(shift)) : 0.001f;
  float disc = mp_sin(mp_frac(f + shift));
  /* shape 0.25: contour 1 + (cos - 1) / 2, the window halved on the second half */
  float contour = 0.5f + 0.5f * mp_cos(c);
  float rd = c >= 0.5f ? 0.5f * ramp_down : ramp_down;
  return ((rd * (off + disc) - off) * contour) * s1_param(level, 0);
}
