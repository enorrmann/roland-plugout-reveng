/* jx3p_chorus.h: the S8 JX-3P PLUG-OUT's "JX-3P CHORUS" (EFFECT TYPE 2, Roland's DSP4 slot program fx_ch), as a
 * plain C model at 96 kHz. README.md §8. Not wired into the PLUG-OUT yet (where it runs: §8.4).
 *
 * Source: Roland's program (tools/make_chorus.py: listing work/jx3p/chorus/fx_ch.s, lift jx3p_chorus_lift.c) and the
 * words Roland's engine computes for it (jx3p_chorus_tables.h, the same tool). Instruction numbers = the lift's.
 *
 *   input     m = (L + R) / 2 x C1F0                                                         (26, 27)        [code]
 *             two biquad lowpasses b = 0.07921255, 0.1584244, 0.07921255; a1 = 1.21644402, a2 = -0.533294
 *             (-3 dB 12.4 kHz each: the BBD's anti-alias filter)                              (27..32)        [code]
 *             a highpass y = 0.999019 (x - x1) + 0.9980383 y1 (~30 Hz)                        (30, 33)        [code]
 *   LFO       p += C1F5 + d, wrapped into [-1, 1); d = 2^-21 against the sign of p             (35..41)        [code;
 *             the sign of d: inferred]; tri_L = 1 - |p|, tri_R = |p| (antiphase; `bic` reads r0 = |p| at the
 *             instruction's start)                                                            (42, 43)        [code]
 *             C1F5 = 9.6495e-6: p crosses [-1, 1) once per LFO period (|p|: 1 -> 0 -> 1), 2 / C1F5 =
 *             207 265 samples = 2.159 s (0.463 Hz); d makes the halves 4.9 % slower / faster   [code + engine]
 *   BBD clock x = 1.30823231 C1F4 tri + 0.449236 samples per clock (C1F4 = 1)                 (48, 49, 56, 57) [code]
 *             the line is written once per clock (1..4 clocks per sample, the input interpolated between
 *             samples) and read 384 clocks back (0.0234375 x 16384 units)                     (75..280)       [code]
 *             -> here a delay line read at 384 x samples (172.5 .. 674.7 samples = 1.80 .. 7.03 ms), linear
 *             interpolation; the clocking's own aliasing is not modelled                                      [approx]
 *   output    the tap through a two-pole lowpass of C1F7 = 0.23046267 (two one-poles y += C1F7 (x - y);
 *             Roland's stage has one instruction the disassembler does not decode)  (281..306)      [inferred]
 *             L / R = dry + wet_L / wet_R x C1F2 x 1.1 (EFFECT LEVEL; dry: C1F1 = C1F3 = 1)  (18..25)        [code]
 *   not modelled: the BBD hiss (EFFECT TONE -> C1F6: noise x (tri + 0.3) x C1F6, 76..87), the fade-in after a type
 *   change ([0647] / [0648], 10..17).
 */
#ifndef JX3P_CHORUS_H
#define JX3P_CHORUS_H
#include <math.h>
#include "jx3p_chorus_tables.h"

#define JXC_LEN 1024                        /* > 384 x 1.757 + 2 */
#define JXC_STAGES 384.0f                   /* BBD clocks between write and read (0.0234375 x 16384) */
#define JXC_X0 0.449236f                    /* samples per BBD clock at tri = 0 (instruction 48) */
#define JXC_XK 1.30823231f                  /* x per unit of tri x C1F4 (instruction 48) */
#define JXC_NUDGE 4.76837158e-7f            /* 2^-21 (instruction 40) */
#define JXC_WET 1.1f                        /* the wet gain's factor (instruction 19) */

typedef struct { float x1, x2, y1, y2; } jxc_bq_t;
typedef struct {
    float line[JXC_LEN];
    int w;                                  /* next write position */
    jxc_bq_t bq[2];
    float hx1, hy1;                         /* the highpass */
    float p, d;                             /* the LFO phase, its nudge */
    float oL[2], oR[2];                     /* the output stages */
} jx_chorus_t;

static inline void jx_chorus_init(jx_chorus_t *c)
{
    for (int i = 0; i < JXC_LEN; i++) c->line[i] = 0.0f;
    c->w = 0;
    c->bq[0] = c->bq[1] = (jxc_bq_t){ 0, 0, 0, 0 };
    c->hx1 = c->hy1 = 0.0f;
    c->p = 0.0f; c->d = JXC_NUDGE;
    c->oL[0] = c->oL[1] = c->oR[0] = c->oR[1] = 0.0f;
}

static inline float jxc_biquad(jxc_bq_t *s, float x)
{
    float y = 0.07921255f * x + 0.1584244f * s->x1 + 0.07921255f * s->x2 + 1.21644402f * s->y1 - 0.533294f * s->y2;
    s->x2 = s->x1; s->x1 = x; s->y2 = s->y1; s->y1 = y;
    return y;
}

/* the line read d samples back (fractional), as s1_line_tap's two neighbours + a linear interpolation */
static inline float jxc_read(const jx_chorus_t *c, float d)
{
    int k = (int)d;
    float g = d - (float)k;
    float a = c->line[(c->w - 1 - k) & (JXC_LEN - 1)], b = c->line[(c->w - 2 - k) & (JXC_LEN - 1)];
    return a + g * (b - a);
}

/* one sample: in L / R, EFFECT LEVEL 0..255 (the panel value); out L / R. dl / dr (optional): the delays used */
static inline void jx_chorus(jx_chorus_t *c, float inL, float inR, int level, float *outL, float *outR,
                             float *dl, float *dr)
{
    float m = 0.5f * (inL + inR) * JXC_C1F0;
    m = jxc_biquad(&c->bq[1], jxc_biquad(&c->bq[0], m));
    float h = 0.999019f * (m - c->hx1) + 0.9980383f * c->hy1;
    c->hx1 = m; c->hy1 = h;
    c->line[c->w] = h;
    c->w = (c->w + 1) & (JXC_LEN - 1);
    float p = c->p + JXC_C1F5 + c->d;
    p = p >= 1.0f ? p - 2.0f : p < -1.0f ? p + 2.0f : p;
    c->p = p;
    c->d = p >= 0.0f ? -JXC_NUDGE : JXC_NUDGE;
    float ap = fabsf(p);
    float xl = JXC_XK * JXC_C1F4 * (1.0f - ap) + JXC_X0, xr = JXC_XK * JXC_C1F4 * ap + JXC_X0;
    float del = JXC_STAGES * xl - 1.0f, der = JXC_STAGES * xr - 1.0f;   /* read(d) = d + 1 samples back */
    float wl = jxc_read(c, del), wr = jxc_read(c, der);
    c->oL[0] += JXC_C1F7 * (wl - c->oL[0]); c->oL[1] += JXC_C1F7 * (c->oL[0] - c->oL[1]);
    c->oR[0] += JXC_C1F7 * (wr - c->oR[0]); c->oR[1] += JXC_C1F7 * (c->oR[0] - c->oR[1]);
    float g = jxc_level[level & 255] * JXC_WET;
    float dry = JXC_C1F1 * JXC_C1F3 + (1.0f - JXC_C1F1);
    *outL = dry * inL + g * c->oL[1];
    *outR = dry * inR + g * c->oR[1];
    if (dl) *dl = del + 1.0f;
    if (dr) *dr = der + 1.0f;
}
#endif
