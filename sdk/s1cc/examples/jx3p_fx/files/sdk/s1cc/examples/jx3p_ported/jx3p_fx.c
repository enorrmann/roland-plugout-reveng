/* jx3p_fx.c: the JX-3P PLUG-OUT's "JX-3P CHORUS" (EFFECT TYPE 2, Roland's DSP4 slot program fx_ch) as SYSTEM-S1CC's
 * master effect (FX_KERNEL): S1_MASTER runs ONCE per sample on the voices' mix on master DSP B, after the voices and
 * before the OS delay / reverb, so it costs the voices nothing (8 voices stay 8). The CRUSHER knob is its level
 * (Roland's EFFECT LEVEL: the same patch word 0x18C, so Roland's presets bring their chorus level). README.md §8.5.
 *
 * The same chorus as jx3p_chorus.h (the native model, README.md §8, from Roland's program and engine words), written
 * with only what ran on master B (asm/master_b_forms.txt: add, sub, mul, max, min, mov, sign; no compare, floor,
 * abs or wrap):
 *   input     m = x (the mix, L = R), two biquad lowpasses (12.4 kHz), a highpass ~30 Hz             [code]
 *   lines     m into lines 0 (L) and 1 (R) of the delay memory (one request per line and sample)
 *   LFO       p += C1F5 + d, wrapped into [-1, 1) by a sign (p - 1 - (1 + sign(p - 1))), d = -2^-21 sign(p);
 *             |p| = p sign(p); tri_L = 1 - |p|, tri_R = |p|                                            [code]
 *   delay     384 (XK tri + X0) samples (the BBD's 384 clocks of 1 / x samples each); the request's own fraction
 *             (s1_line_tap k = 2, the hardware's 1/64, as Roland's program reads its taps) interpolates
 *   output    two one-poles of C1F7 on each tap; + the BBD hiss; L / R = x + level x 1.1 x wet       [code; output
 *             stage: inferred]
 *   hiss      Roland's (instructions 75..87, 42..46): white noise (the host's, s1_fx_noise) -> a one-pole highpass
 *             y = 0.8582945 (u - u1) + 0.716599 y1 (~5.1 kHz; 0.8582945 = (1 + 0.716599) / 2) -> a biquad lowpass
 *             0.0489201 (1, 2, 1) / (1, -1.4190722, 0.614753) (9.5 kHz), x C1F6 (tri + 0.3) per side, added to the
 *             wet signal. C1F6 fixed at EFFECT TONE JXC_HISS_TONE (below)                              [code; the
 *             filters' order and the noise's level: inferred]
 * Its S1_DSP_A is only the profile's test signal (a saw chord): SYSTEM-S1CC never runs it. */
#include "s1dsp.h"
#include "s1panel.h"
#include "jx3p_chorus.h"

S1_BUILD("NAME=JX-3P-FX");
/* --no-range-fold: the profile's notes are shorter than one LFO period (2.16 s): it never sees the phase wrap nor a
   negative phase, and s1cc would fold both signs of the LFO to constants (the LFO would run off its range) */
S1_COMPILE("--no-range-fold");

/* EFFECT TONE (the hiss's level) fixed (the SYSTEM-1 has no knob for it): the value Roland's JX-3P chorus presets use
   most (TONE 128 in 26 of the 105 EFFECT TYPE 2 patches of 1 / 2 Preset.bin; next: 0 in 12, 48 in 9). 2026-11-18: the lowest one Roland
   uses (10, 2 Preset.bin "SQ The Attack") was not audible on the device (user) */
#define JXC_HISS_TONE 128

/* CRUSHER = EFFECT LEVEL: Roland's C1F2 x 1.1 (the program's factor) */
S1_PARAM(fx_level, S1_CRUSHER, 1) { out[0] = jxc_level[knob & 255] * JXC_WET; }

/* ---- the profile's test signal (never on the device) ---- */
static float t0, t1, t2;
S1_DSP_A
{
  float dt = s1_dt();
  t0 += dt;
  t0 = t0 >= 1.0f ? t0 - 2.0f : t0;
  t1 += dt * 1.2599210f;
  t1 = t1 >= 1.0f ? t1 - 2.0f : t1;
  t2 += dt * 1.4983071f;
  t2 = t2 >= 1.0f ? t2 - 2.0f : t2;
  return 0.25f * (t0 + t1 + t2);
}

/* ---- the chorus (master B, once on the mix) ---- */
static float b0x1, b0x2, b0y1, b0y2, b1x1, b1x2, b1y1, b1y2;   /* the two input biquads */
static float hx1, hy1;                                         /* the input highpass */
static float p, nud;                                           /* the LFO: phase, its nudge */
static float oL0, oL1, oR0, oR1;                               /* the output stages */
static float nu1, ny1, nb1, nb2, nby1, nby2;                   /* the hiss: highpass, biquad */

#define JXC_B 0.07921255f        /* the input biquads: b = JXC_B (1, 2, 1), a1, a2 (instructions 27..32) */
#define JXC_A1 1.21644402f
#define JXC_A2 (-0.533294f)
#define JXH_P 0.716599f          /* the hiss's highpass pole (instruction 76) and gain (1 + p) / 2 (77) */
#define JXH_G 0.8582945f
#define JXH_B 0.0489201f         /* the hiss's biquad: b = JXH_B (1, 2, 1), a1, a2 (78..82) */
#define JXH_A1 1.4190722f
#define JXH_A2 (-0.614753f)
#define JXH_BIAS 0.3f            /* the hiss's level follows tri + 0.3 (instruction 44) */

S1_MASTER
{
  /* input: the BBD's anti-alias lowpasses and the highpass */
  float y0 = JXC_B * (x + 2.0f * b0x1 + b0x2) + JXC_A1 * b0y1 + JXC_A2 * b0y2;
  b0x2 = b0x1; b0x1 = x; b0y2 = b0y1; b0y1 = y0;
  float y1 = JXC_B * (y0 + 2.0f * b1x1 + b1x2) + JXC_A1 * b1y1 + JXC_A2 * b1y2;
  b1x2 = b1x1; b1x1 = y0; b1y2 = b1y1; b1y1 = y1;
  float h = 0.999019f * (y1 - hx1) + 0.9980383f * hy1;
  hx1 = y1; hy1 = h;
  s1_line_write(0, h);
  s1_line_write(1, h);

  /* LFO: p in [-1, 1), wrapped by a sign (p reaches 1 exactly only by chance: then it lands on 0) */
  float q = s1_range(p + (JXC_C1F5 + nud), -1.0f, 1.0001f);
  q = q - (1.0f + s1_sign(q - 1.0f));
  p = q;
  float sg = s1_sign(q);
  nud = -JXC_NUDGE * sg;
  float ap = q * sg;                                           /* |p| */

  /* the two taps: ages 384 x - 1 (the model's), the request's d one less (s1_line_tap: d + 1 + f) */
  float dl = (JXC_STAGES * JXC_XK * JXC_C1F4) * (1.0f - ap) + (JXC_STAGES * JXC_X0 - 2.0f);
  float dr = (JXC_STAGES * JXC_XK * JXC_C1F4) * ap + (JXC_STAGES * JXC_X0 - 2.0f);
  float l0 = s1_line_tap(0, dl, 0), l1 = s1_line_tap(0, dl, 1), lf = s1_line_tap(0, dl, 2);
  float r0 = s1_line_tap(1, dr, 0), r1 = s1_line_tap(1, dr, 1), rf = s1_line_tap(1, dr, 2);
  float wl = l0 + lf * (l1 - l0), wr = r0 + rf * (r1 - r0);
  oL0 += JXC_C1F7 * (wl - oL0); oL1 += JXC_C1F7 * (oL0 - oL1);
  oR0 += JXC_C1F7 * (wr - oR0); oR1 += JXC_C1F7 * (oR0 - oR1);

  /* the BBD hiss */
  float u = s1_fx_noise();
  float hp = JXH_G * (u - nu1) + JXH_P * ny1;
  nu1 = u; ny1 = hp;
  float hb = JXH_B * (hp + 2.0f * nb1 + nb2) + JXH_A1 * nby1 + JXH_A2 * nby2;
  nb2 = nb1; nb1 = hp; nby2 = nby1; nby1 = hb;
  float hs = hb * jxc_tone[JXC_HISS_TONE];
  float hl = hs * (1.0f - ap + JXH_BIAS), hr = hs * (ap + JXH_BIAS);

  float g = s1_param(fx_level, 0);
  s1_out_r(x + g * (oR1 + hr));
  return x + g * (oL1 + hl);
}
