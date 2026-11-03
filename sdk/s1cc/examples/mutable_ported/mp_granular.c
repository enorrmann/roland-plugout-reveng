/* mp_granular.c: Braids GRANULAR_CLOUD: four sine grains under bell windows, each at the note detuned at random.
 * Braids restarts an idle grain with a 1 in 4 chance per block; here the windows are free-running phases at four
 * rates (no random restart: a rare compare costs ~8 ops per grain once it may not be folded) and a grain's detune
 * is low-passed noise sampled while its window is near zero, so every grain comes back at another pitch.
 * OSC1 COLOR = grain length (200 .. 3 ms), OSC2 COLOR = pitch scatter, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

static const float kRate[4] = { 1.0f, 1.27f, 1.61f, 1.93f };
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.75f * knob / 255.0f) * 0.5f; }

static float ph[4], env[4] = { 0.0f, 0.25f, 0.5f, 0.75f }, ratio[4] = { 1.0f, 1.0f, 1.0f, 1.0f }, w;

S1_DSP_A {
  float dt = mp_dt(), y = 0.0f;
  float de = s1_exp2(6.0f * s1_color(0)) * (1.0f / (0.2f * MP_FS));
  float sc = s1_color(1);
  float n = mp_noise(&w);
  for (int i = 0; i < 4; i++) {
    float e = mp_advance(&env[i], de * kRate[i]);
    float g = mp_sin(0.5f * e);
    g = g * g;
    /* the detune follows the noise only while the window is closed (g ~ 0): (1 - g)^8 ~ 1 near the edges, 0 inside */
    float c = 1.0f - g, c2 = c * c, c4 = c2 * c2;
    ratio[i] += (s1_exp2(sc * n) - ratio[i]) * (c4 * c4 * 0.05f);
    y += g * mp_sin(mp_advance(&ph[i], mp_step(dt, ratio[i])));
  }
  return y * s1_param(level, 0);
}
