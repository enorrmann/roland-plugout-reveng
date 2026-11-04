/* cz.c: a Casio CZ voice (phase distortion) for SYSTEM-S1CC, after Michael Rickard's CZ-1 study
 * (https://www.kasploosh.com/cz/, "Casio CZ-1 Spelunking": basic waves, window functions, modulation) and the CZ
 * operation manuals. Spec and verification: docs/examples_casio_cz_README.md.
 *
 * Panel (CZ name -> SYSTEM-1 control):
 *   LINE 1 DCO1 wave   OSC1 WAVE   positions 0..4 = CZ WAVE 1..5 (saw, square, pulse, sine-pulse/"double sine",
 *                                  saw-pulse); position 5 = WAVE 6 (resonance, saw window)
 *   LINE 2 DCO2 wave   OSC2 WAVE   positions 0..4 = CZ WAVE 1..5; position 5 = WAVE 7 (resonance, triangle window)
 *   DCW (distortion)   OSC1 / OSC2 COLOR, with COLOR MOD: FILTER ENV = the CZ's DCW envelope, AMP ENV, PITCH ENV,
 *                      LFO, SUB OSC (the host's sources, docs/examples_system_s1cc_README.md §3.2)
 *   LINE SELECT 1+2'   OSC1 MIX / OSC2 MIX: the level of each line (2' = line 2 detuned)
 *   OCTAVE / DETUNE    OSC1 RANGE (both lines, host), OSC2 RANGE (line 2's octave against line 1), OSC2 FINE (+-1
 *                      semitone, 128 = in tune)
 *   RING modulation    SUB: line 2' -> line 2' x line 1, crossfaded (0 = none, 255 = the CZ's ring)
 *   NOISE modulation   NOISE: line 2' -> line 2' x noise, crossfaded
 *   DCA envelope       AMP A / D / S / R (host ADSR 0)
 *   DCW envelope       FILTER A / D / S / R (host ADSR 1): set COLOR MOD = FILTER ENV
 *   DCO (pitch) env    PITCH ENV AMOUNT / ATTACK / DECAY (host)
 *   vibrato            LFO TO PITCH (host LFO, with DELAY TIME)
 *   filter             FILTER CUTOFF / RESONANCE / ENV AMOUNT / KEY TRACK: the host-side SVF of mp_dsp_b.h (the CZ
 *                      has none: CUTOFF 255 + RESONANCE 0 is the plain CZ)
 *
 * One phase-distortion formula makes all the waves; the WAVE selector only picks its coefficients (a 6-word knob
 * table), so a voice pays for one wave, not for six. With p the oscillator's phase (0..1) and d the DCW (0..1):
 *   k = (1 + a d)^4 = 1 .. 32 (at most rdt/2: the CZ's DCW key follow), r ~ 1/k, B ~ 1/(2 - r)
 *   q = R p + o (1 - r)   with R = 1 + SQ + 15 F d (at most rdt/4),   n = floor(q),  h = q - n
 *   phi = min((KA k + F) h,  1 - (BG + BE) B + (BG B + BE) h,  1)
 *   y = -(1 - F win(p) - 2 SQ n) cos(2 pi (1 - SQ/2) phi)
 * d = 0 gives a sine for WAVE 1..5, as on the CZ. */
#include <math.h>
#include "../mutable_ported/include/mp_dsp_a.h"
#define MP_FILTER MP_SVF_LP
#include "../mutable_ported/include/mp_dsp_b.h"

/* coefficients of one wave: the knob table's 6 words. Everything else is derived from them in the voice:
     R0 = 1 + SQ (the square runs two half-cycles), S = 1 - SQ / 2, G = -2 SQ (the second half inverted),
     KB = F, R1 = 15 F, window = 1 - F p (OSC1) or 1 - F |2p - 1| (OSC2), CE = BG + BE.
   Fewer words keep fewer values live: 9 words (R0, S, G, R1 loaded) gave 195 instructions against 186, with more
   spills (4 registers per voice) */
enum { C_SQ, C_F, C_O, C_KA, C_BG, C_BE, C_N };

/* the CZ waves (kasploosh: bits 000 SAW, 001 SQUARE, 010 PULSE, 100 SINE-PULSE, 101 SAW-PULSE; windows 001 SAW and
   010 TRIANGLE on the multi-sine 110). Row 6 = the resonance wave: its window is the oscillator's (saw on OSC1 = CZ
   WAVE 6, triangle on OSC2 = CZ WAVE 7).
                     SQ  F   o     KA  BG  BE */
static const float cz_wave[6][C_N] = {
  /* 1 saw         */ {0, 0,  0,    1,  1,  0},
  /* 2 square      */ {1, 0,  0,    1,  0,  0},
  /* 3 pulse       */ {0, 0, -0.5f, 1,  0,  0},
  /* 4 sine-pulse  */ {0, 0,  0,    1,  0,  0},
  /* 5 saw-pulse   */ {0, 0,  0,    2,  0,  1},
  /* 6/7 resonance */ {0, 1,  0,    0,  0,  0},
};

/* selector 0..5; anything else (row 128, before the panel arrives) = position 0 */
static void cz_wave_row(int knob, float *out) {
  int w = knob >= 0 && knob <= 5 ? knob : 0;
  for (int i = 0; i < C_N; i++) out[i] = cz_wave[w][i];
}
S1_PARAM(wave1, S1_OSC1_WAVE, C_N) { cz_wave_row(knob, out); }
S1_PARAM(wave2, S1_OSC2_WAVE, C_N) { cz_wave_row(knob, out); }

/* line levels: two lines, half the level of one each */
S1_PARAM(level1, S1_OSC1_MIX, 1) { out[0] = 0.5f * knob / 255.0f; }
S1_PARAM(level2, S1_OSC2_MIX, 1) { out[0] = 0.5f * knob / 255.0f; }

/* line 2's pitch against line 1: OSC2 RANGE - OSC1 RANGE (the host already put OSC1 RANGE in s1_dt) and FINE; each
   table also has its inverse (line 2's 1/dt without a division) */
S1_PARAM(range1, S1_OSC1_RANGE, 2) { out[0] = powf(2.0f, -(knob <= 5 ? knob : 3)); out[1] = 1.0f / out[0]; }
S1_PARAM(range2, S1_OSC2_RANGE, 2) { out[0] = powf(2.0f, knob <= 5 ? knob : 3); out[1] = 1.0f / out[0]; }
S1_PARAM(fine2, S1_OSC2_FINE, 2) { out[0] = powf(2.0f, (knob - 128) / 128.0f / 12.0f); out[1] = 1.0f / out[0]; }

/* RING and NOISE modulation of line 2' (the host mixer is off: MIX=0 below, these knobs are ours). Row 128 (before
   the panel arrives) = 0, like the host's SUB / NOISE */
S1_PARAM(ring, S1_SUB_MIX, 2) { out[0] = knob == 128 ? 0.0f : knob / 255.0f; out[1] = 1.0f - out[0]; }
S1_PARAM(nmod, S1_NOISE_MIX, 2) { out[0] = knob == 128 ? 0.0f : knob / 255.0f; out[1] = 1.0f - out[0]; }

#define CZ_A 1.3784142f     /* DCW = 1: k = (1 + CZ_A)^4 = 32, the fast segment is 1/32 of the cycle */
#define CZ_KCAP 0.5f        /* k <= rdt / 2: the fast segment lasts at least 2 samples */
#define CZ_RES 15.0f        /* resonance: R = 1 + 15 DCW */
#define CZ_NOISE_K 7.31275f /* noise map n = wrap(K n + W) (make_s1cc.py NOISE_K) */
#define CZ_NOISE_W 0.6180339887f
#define CZ_RCAP 0.25f       /* resonance R <= rdt / 4: its sine stays under fs / 4 */

/* one CZ oscillator: phase *ph, step dt (rdt = 1/dt), DCW d (0..1), the wave's 6 coefficients c, tri = the
   resonance window (0 saw, 1 triangle; a constant) */
static inline float cz_osc(float *ph, float dt, float rdt, float d, const float *c, int tri) {
  float p = mp_advance(ph, dt);
  /* k = (1 + a d)^4: 1 .. 32 (a = 32^(1/4) - 1), about exponential, three ops; r = 1/k from a cubic for 1/(1 + a d)
     (0.5 %), to the 4th. The cap on k (high notes) does not move r: B and the pulse's centre are then slightly off,
     the waveform stays continuous (phi is a min of lines) */
  float e = 1.0f + CZ_A * d;
  float e2 = e * e;
  float k = fminf(e2 * e2, CZ_KCAP * rdt);
  float ri = 1.0f + d * (-1.27973f + d * (1.14161f - 0.44683f * d));
  float ri2 = ri * ri;
  float r = ri2 * ri2;
  float B = 0.49623f + r * (0.30271f + r * (-0.05572f + r * 0.25678f));    /* 1 / (2 - r), 0.2 % */
  float sq = c[C_SQ], f = c[C_F];
  float R = fminf(1.0f + sq + f * (CZ_RES * d), CZ_RCAP * rdt);
  float q = R * p + c[C_O] * (1.0f - r);
  float n = s1_floor(q);
  float h = q - n;
  float bg = c[C_BG], be = c[C_BE];
  float l1 = (c[C_KA] * k + f) * h;
  float l2 = (1.0f - (bg + be) * B) + (bg * B + be) * h;
  float phi = fminf(fminf(l1, l2), 1.0f);
  float win = tri ? fabsf(p + p - 1.0f) : p;
  float g = 1.0f - f * win - (sq + sq) * n;
  return -g * mp_cos(phi - 0.5f * sq * phi);
}

static float ph1, ph2, nw;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float c1[C_N], c2[C_N];
  for (int i = 0; i < C_N; i++) {
    c1[i] = s1_param(wave1, i);
    c2[i] = s1_param(wave2, i);
  }
  float kk = s1_param(range1, 0) * s1_param(range2, 0) * s1_param(fine2, 0);    /* line 2 / line 1 */
  float dt2 = mp_step(dt, kk);
  float rdt2 = rdt * s1_param(range1, 1) * s1_param(range2, 1) * s1_param(fine2, 1);
  float l1 = cz_osc(&ph1, dt, rdt, s1_color(0), c1, 0);
  float l2 = cz_osc(&ph2, dt2, rdt2, s1_color(1), c2, 1);
  /* line 2': x ((1 - ring) + ring line 1) x ((1 - nmod) + nmod noise): the CZ's ring / noise modulation,
     crossfaded. The noise: n = wrap(K n + W), the host LFO's noise map with a constant W (two ops) */
  float nz = s1_wrap(CZ_NOISE_K * nw + CZ_NOISE_W);
  nw = nz;
  l2 *= s1_param(ring, 1) + s1_param(ring, 0) * l1;
  l2 *= s1_param(nmod, 1) + s1_param(nmod, 0) * nz;
  return l1 * s1_param(level1, 0) + l2 * s1_param(level2, 0);
}

S1_BUILD("NAME=CZ-SYSTEM MIX=0");
S1_COMPILE("--regs 0,1,2,3");
