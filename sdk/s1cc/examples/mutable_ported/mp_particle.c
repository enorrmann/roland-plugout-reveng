/* mp_particle.c: Braids PARTICLE_NOISE (also Plaits' particle engine): random clicks of decaying noise, each
 * retuning three resonant band-passes around the note at random, so every particle rings at its own pitches.
 * Braids' two-pole resonators are the double-sampled SVF of mp_filter.h here (mp_twin_peaks.c says why).
 * OSC1 COLOR = density, OSC2 COLOR = pitch scatter, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
S1_COMPILE("--no-fold-branches");          /* a particle is rare: the profiling may never see one (W909) */
#include "include/mp_filter.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float w, amp, na, nb;
static struct mp_svf f1, f2, f3;

S1_DSP_A {
  float dt = mp_dt();
  float n = mp_noise(&w), ra = mp_noise(&w), rb = mp_noise(&w);   /* drawn every sample, kept at a particle */
  /* Braids: (1024 + COLOR) / 2^23 per 2 samples: 1 .. 32 in 10^4 per sample */
  float th = 1.0f - 0.0002f - 0.0062f * s1_color(0);
  if (n > th) {
    amp = 1.0f;
    na = ra;
    nb = rb;
  }
  float x = n * amp * 0.06f;
  amp = amp * 0.988f;
  float sc = s1_color(1), q = 0.03f;                   /* Braids r = 0.996 */
  /* the three pitches: Braids' + 0x600 / 0x980 / 0x790 (12 / 19 / 15 semitones) and the random offsets */
  float y = mp_svf_process(&f1, x, 6.2832f * mp_step(dt, 2.0f * s1_exp2(1.5f * sc * na)), q).bp;
  y += mp_svf_process(&f2, x, 6.2832f * mp_step(dt, 2.997f * s1_exp2(sc * na)), q).bp;
  y += mp_svf_process(&f3, x, 6.2832f * mp_step(dt, 2.378f * s1_exp2(0.5f * sc * nb)), q).bp;
  return mp_soft(y) * s1_param(level, 0);
}
