/* mp_swarm.c: Plaits swarm engine: a swarm of saws, each under its own bell envelope, detuned around the note and
 * drifting at random (Plaits: 8 grains of saw / sine restarted at random; here 4 band-limited saws). The windows are
 * triangles of free-running phases at four rates (no random restart: a rare compare costs ~8 ops per grain once it may not be
 * folded), so they overlap like the grains of the original; each saw's detune is a fixed spread plus one slowly
 * low-passed noise (with a sign and weight per saw), which gives the swarm's random beating.
 * OSC1 COLOR = window rate (slow .. fast grains), OSC2 COLOR = detune spread, OSC1 MIX = level.
 * DSP B: mp_dsp_b.h. The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

static const float kSpread[4] = { -1.0f, -0.33f, 0.33f, 1.0f };
static const float kRate[4] = { 1.0f, 1.31f, 1.73f, 2.11f };       /* window rates: never in step */
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.75f * knob / 255.0f) * 0.4f; }

static float ph[4], env[4] = { 0.0f, 0.25f, 0.5f, 0.75f }, drift, w;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt(), y = 0.0f;
  float de = s1_exp2(5.0f * s1_color(0)) * (1.0f / (0.8f * MP_FS));   /* 0.8 s .. 25 ms windows */
  float sp = 0.03f * s1_color(1);
  float n = mp_noise(&w);
  drift += (n - drift) * 0.001f;                        /* a few Hz of random drift, x the saw's own sign */
  for (int i = 0; i < 4; i++) {
    float r = 1.0f + sp * (kSpread[i] + drift * kSpread[3 - i]);
    float e = mp_advance(&env[i], de * kRate[i]);
    float g = 1.0f - fabsf(e + e - 1.0f);                 /* triangle window 0 .. 1 .. 0 */
    float d = dt * r;
    y += g * mp_saw_bl(mp_advance(&ph[i], d), d, rdt * (2.0f - r));   /* 1/r ~ 2 - r near 1 */
  }
  return y * s1_param(level, 0);
}
