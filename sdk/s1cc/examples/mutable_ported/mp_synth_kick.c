/* mp_synth_kick.c: Plaits synthetic bass drum: a sine whose frequency is pushed up by an FM envelope at the note
 * start (fast decay), a body envelope, a click transient, the whole through a one-pole "tone" low-pass. The body
 * decay is the AMP envelope here; the FM envelope and the click are the kernel's.
 * OSC1 COLOR = FM envelope amount, OSC2 COLOR = tone (dark .. bright + click), OSC1 MIX = level.
 * DSP B: mp_dsp_b.h. The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph, fm, fm_lp, tr, lp, last_trig;

S1_DSP_A {
  float trig = s1_trig(), dt = mp_dt();
  if (trig != last_trig) {
    fm = 1.0f;
    tr = 1.0f;
    ph = 0.25f;                                          /* Plaits starts the body at a peak */
  }
  last_trig = trig;
  fm = fm * (1.0f - 1.0f / (0.012f * MP_FS));            /* 12 ms */
  fm_lp += (fm - fm_lp) * 0.1f;
  tr = tr * (1.0f - 1.0f / (0.005f * MP_FS));
  float p = mp_advance(&ph, mp_step(dt, 1.0f + 3.5f * s1_color(0) * fm_lp));
  float body = mp_sin(p);
  float tone = s1_color(1);
  float mix = body + tr * tone * 0.5f;
  /* tone: 4 f0 x 2^(9 tone), at most 1 */
  lp += (mix - lp) * fminf(4.0f * dt * s1_exp2(9.0f * tone), 1.0f);
  return lp * s1_param(level, 0);
}
