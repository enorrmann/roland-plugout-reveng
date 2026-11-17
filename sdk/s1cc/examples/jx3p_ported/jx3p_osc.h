/* jx3p_osc.h: our own JX-3P oscillators (README.md §4), in place of Roland's DCO programs (jx3p_dco1_saw.h ..
 * jx3p_dco2_noise.h, the originals lifted to C and kept beside these; tools/osc_check.c compares the two natively).
 *
 * Roland's DCO programs run four sub-samples per sample (384 kHz), each with its phase, a rounded edge
 * (sin(pi/2 clamp(...))) and the reciprocal of the step, then a 28-tap decimation FIR and the output stage: about
 * 120 instructions per oscillator. The JX-3P's three DCO waveforms (the WAVEFORM positions, measured on the lifts
 * 2026-11-17, tools/osc_check.c):
 *   0 SAW     the saw program, wave word 0: the phase, Roland's tilt, x -0.5 (the FIR inverts: a rising saw)
 *   1 SQUARE  the sqr program (a slot program of its own): sign of the phase plus a spike 0.5 x 0.98^n after every
 *             edge (n = sub-samples since the edge), a one-pole / one-zero per sub-sample (0.79069895 y1 + 0.7385094 x
 *             - 0.52920836 x1), x 0.33
 *   2 PULSE   the saw program, wave word 1: a pulse of fixed width, 1.7 - 0.04 per sub-sample since the wrap,
 *             limited to 0..1 (1 for ~17 sub-samples, then a ramp to 0 over 25: ~78 us), Roland's tilt, x 0.9
 * Here one sample per sample: phase 0..1, step dt = f / 96000 and rd = 1 / dt from the ARM (jx3p_arm.h, every 1 ms,
 * the pitch modulation in it); every step edge corrected by a two-sample polynomial BLEP (PolyBLEP: residual -(1 -
 * t/dt)^2 after the edge, (1 - (1 - t)/dt)^2 before it, branch-free with fminf); the sub-sample filters as their
 * 96 kHz equivalents (poles and zeros to the 4th power: matched z, the same DC gain).
 * Not modelled yet: DCO2 CROSS MOD (sync, metal), the 28-tap FIR's roll-off above ~15 kHz. */
#ifndef JX3P_OSC_H
#define JX3P_OSC_H

typedef struct {
    float p;                    /* phase 0..1 */
    float a;                    /* DCO2 CROSS MOD: Roland's second phase (0..1) */
    float off;                  /* DCO2 CROSS MOD (lean): p = frac(a - off) */
    float x1, y1;               /* the output filter: last input, last output */
    float e;                    /* SQUARE: the spike after the last edge */
} jx_osc_t;

/* the PolyBLEP residual at phase t (0..1) of a falling step of 2 at t = 0 (as the saw 2t - 1 has): rd = 1 / dt,
   rm = rd - 1. With x = t rd: after the edge a = max(1 - x, 0) (= 1 - t/dt while t < dt), before it b = max(x - rm,
   0) (= 1 - (1 - t)/dt while t > 1 - dt); residual b^2 - a^2 = (b - a)(b + a) */
typedef struct { float r, a; } jx_blep_t;
static inline __attribute__((always_inline)) jx_blep_t jx_blep2(float t, float rd, float rm)
{
    float x = t * rd;
    float a = fmaxf(1.0f - x, 0.0f), b = fmaxf(x - rm, 0.0f);
    jx_blep_t r = { (b - a) * (b + a), a };
    return r;
}
static inline __attribute__((always_inline)) float jx_blep(float t, float rd)
{
    return jx_blep2(t, rd, rd - 1.0f).r;
}

/* the wraps as `x >= 1 ? x - 1 : x`: s1cc makes that an exact mod, one op (docs/vol5_toolchain_compiler_sdk/
   s1cc_compiler_internals.md §7.2) */
static inline __attribute__((always_inline)) float jx_phase(jx_osc_t *o, float dt)
{
    float p = o->p + dt;
    p = p >= 1.0f ? p - 1.0f : p;
    o->p = p;
    return p;
}

/* Roland's tilt (the saw program's last filter, the same coefficients, MAIN2 / DCO instruction 341) */
static inline __attribute__((always_inline)) float jx_tilt(jx_osc_t *o, float x)
{
    float y = 0.8532445f * o->y1 + 0.44227272f * x - 0.29551718f * o->x1;
    o->x1 = x;
    o->y1 = y;
    return y;
}

/* WAVEFORM 0: SAW */
static inline __attribute__((always_inline)) float jx_saw(jx_osc_t *o, float dt, float rd)
{
    float p = jx_phase(o, dt);
    return jx_tilt(o, (2.0f * p - 1.0f - jx_blep(p, rd)) * JX_SAW_GAIN);
}

/* WAVEFORM 2: PULSE, 1.66 - 0.16 x (sub-samples since the wrap = 4 p rd), limited to 0..1; the wrap's step of 1 */
static inline __attribute__((always_inline)) float jx_pulse_p(jx_osc_t *o, float p, float rd)
{
    jx_blep_t b = jx_blep2(p, rd, rd - 1.0f);
    float v = fminf(fmaxf(1.66f - 0.16f * p * rd, 0.0f), 1.0f) + 0.5f * b.r;
    return jx_tilt(o, v * JX_PULSE_GAIN);
}
static inline __attribute__((always_inline)) float jx_pulse(jx_osc_t *o, float dt, float rd)
{
    return jx_pulse_p(o, jx_phase(o, dt), rd);
}

/* WAVEFORM 1: SQUARE; its filter per sub-sample (pole 0.79069895, zero 0.52920836 / 0.7385094) at 96 kHz: pole
   0.79069895^4, zero (0.52920836 / 0.7385094)^4, DC gain 1 */
#define JX_SQ_A 0.39087784f
#define JX_SQ_Z 0.26369840f
#define JX_SQ_G 0.82711548f
#define JX_SQ_DECAY 0.92236816f    /* 0.98^4: the spike per sample */
/* the spike restarts at 1 in the first sample after an edge: there the BLEP's after-edge term a > 0 (sign 1, else 0)
   and e = max(e x 0.98^4, 1) = 1; between edges it decays (e < 1) */
static inline __attribute__((always_inline)) float jx_square_p(jx_osc_t *o, float p, float dt, float rd)
{
    /* both edges with one PolyBLEP: r = frac(2 p) wraps at each; L = the level now (+1 after the rising edge at p = 0,
       -1 after the falling one at 0.5): the edge just passed has the sign of L, the next one the other, so
       sq = L (1 - a^2 - b^2) with a, b the after- and before-edge terms of r (step dt' = 2 dt: rd' = rd / 2) */
    float r = 2.0f * p;
    r = r >= 1.0f ? r - 1.0f : r;
    float L = 1.0f - 2.0f * (2.0f * p - r);            /* 2 p - r = 0 before 0.5, 1 after: L = +1, -1 */
    jx_blep_t e = jx_blep2(r, 0.5f * rd, 0.5f * rd - 1.0f);
    float bb = e.r + e.a * e.a;                        /* b^2 (e.r = b^2 - a^2) */
    float s = L * (1.0f - e.a * e.a - bb);
    float sp = fmaxf(o->e * JX_SQ_DECAY, s1_sign(e.a));   /* the spike: 1 in the sample after an edge */
    o->e = sp;
    float x = s + JX_SQ_SPIKE * sp;                    /* x JX_SQ_GAIN in the filter's coefficients */
    float y = JX_SQ_A * o->y1 + (JX_SQ_G * JX_SQ_GAIN) * x - (JX_SQ_G * JX_SQ_GAIN * JX_SQ_Z) * o->x1;
    o->x1 = x;
    o->y1 = y;
    (void)dt;
    return y;
}
static inline __attribute__((always_inline)) float jx_square(jx_osc_t *o, float dt, float rd)
{
    return jx_square_p(o, jx_phase(o, dt), dt, rd);
}

/* DCO2 CROSS MOD (the DCO2 programs' instructions 24..32, 51..64, ...: README.md §4.3). DCO1 marks the sub-sample
   (1..4 of Roland's four) in which its phase wrapped; DCO2 takes it one sample later. Roland's DCO2 has a second phase
   a besides its phase p: per sub-sample a = (first sub-sample ? a A028 : a) (x A029 at the marked one) + step, and when
   a wraps p takes a's value; at the marked sub-sample p starts again (Roland's -1, here 0). CROSS MOD off (A028 = 0):
   a restarts every sample, never wraps, p runs free. SYNC (A028 = 1, A029 = 0): a starts again at Roland's 0 (here
   0.5): p runs half a cycle from the mark, then follows a. METAL (A029 = 1): a runs on.
   Here per sample, closed form (at most one mark and one wrap of a per sample): k = the mark (0 none), xm = A028, mt =
   A029. Returns p; *corr = the saw's correction (after-edge PolyBLEP) for the jumps of p a mark or a snap makes */

/* the mark of DCO1 this sample: the sub-sample (1..4) where its phase wrapped (p < dt), else 0: ceil(4 - 4 p / dt),
   which is <= 0 when it did not wrap (no comparison: s1cc compiles float comparisons at a high cost) */
static inline __attribute__((always_inline)) float jx_mark(float p, float dt, float rd)
{
    (void)dt;
    return s1_range(fmaxf(-s1_floor(4.0f * p * rd - 4.0f), 0.0f), 0.0f, 4.0f);
}

typedef struct { float p, corr, plain; } jx_px_t;
/* every selection by arithmetic on 0 / 1 values (xm, mt, snap, ev) */
static inline __attribute__((always_inline)) jx_px_t jx_phase_x(jx_osc_t *o, float dt, float rd, float k, float xm,
                                                               float mt)
{
    float d = 0.25f * dt;
    float a = 0.5f + xm * (o->a - 0.5f) + dt;          /* the first sub-sample's a x A028 (0 -> Roland's 0 = 0.5) */
    float aw = a >= 1.0f ? a - 1.0f : a;
    float snap = a - aw;                               /* 1 when a wrapped, else 0 */
    float m = o->p + dt;
    m = m >= 1.0f ? m - 1.0f : m;
    float pre = o->p + dt - aw;                        /* p where a wrapped (the snap drops it to 0) */
    m = m + snap * (aw - m);
    float t = fmaxf(1.0f - aw * rd, 0.0f);
    float c = snap * pre * t * t;
    /* a mark at sub-sample k: p from 0 there, a from 0.5 (SYNC) or on (METAL); then 5 - k sub-steps */
    float r = (5.0f - k) * d;
    float mb = o->p + (k - 1.0f) * d;                  /* p just before the mark */
    float q = 0.25f * (k - 1.0f);                      /* 1 - the time since the mark (samples) */
    float ms = mt * snap;
    float me = r + ms * (aw - r);
    float ae = 0.5f + r + mt * (aw - 0.5f - r);
    float ev = fminf(k, 1.0f);                         /* k is 0 or 1..4 */
    jx_px_t x;
    x.p = s1_range(m + ev * (me - m), 0.0f, 1.0f);      /* (declared: the selections hide it from s1cc) */
    o->p = x.p;
    o->a = s1_range(aw + ev * (ae - aw), 0.0f, 1.0f);
    x.corr = c + ev * (mb * q * q - c);
    x.plain = (1.0f - ev) * (1.0f - snap);             /* the normal wrap's BLEP only when nothing moved p */
    return x;
}

/* the same, lean (README.md §4.3): one phase a and an offset o, p = frac(a - o): a mark sets o so that p starts at 0
   (SYNC: a from 0.5, o = 0.5; METAL: a on), a's wrap clears o (p takes a). CROSS MOD off: p runs free. No BLEP for
   the jumps a mark or a snap makes (the wrap's BLEP on p) */
static inline __attribute__((always_inline)) float jx_phase_xl(jx_osc_t *o, float dt, float k, float xm, float mt)
{
    float a1 = 0.5f + xm * (o->a - 0.5f) + dt;
    float aw = a1 >= 1.0f ? a1 - 1.0f : a1;
    float of = o->off - o->off * (a1 - aw);            /* a wrapped: the offset goes */
    float r = (5.0f - k) * (0.25f * dt);
    float ev = fminf(k, 1.0f);
    float ae = 0.5f + r + mt * (aw - 0.5f - r);
    float an = aw + ev * (ae - aw);
    float on = of + ev * (ae - r - of);
    float mx = an - on;
    mx = mx < 0.0f ? mx + 1.0f : mx;
    float mf = o->p + dt;
    mf = mf >= 1.0f ? mf - 1.0f : mf;
    float m = mf + xm * (mx - mf);
    o->a = an;
    o->off = on;
    o->p = m;
    return m;
}
static inline __attribute__((always_inline)) float jx_saw_xl(jx_osc_t *o, float dt, float rd, float k, float xm, float mt)
{
    float p = jx_phase_xl(o, dt, k, xm, mt);
    return jx_tilt(o, (2.0f * p - 1.0f - jx_blep(p, rd)) * JX_SAW_GAIN);
}
static inline __attribute__((always_inline)) float jx_square_xl(jx_osc_t *o, float dt, float rd, float k, float xm,
                                                               float mt)
{
    return jx_square_p(o, jx_phase_xl(o, dt, k, xm, mt), dt, rd);
}
static inline __attribute__((always_inline)) float jx_pulse_xl(jx_osc_t *o, float dt, float rd, float k, float xm,
                                                              float mt)
{
    return jx_pulse_p(o, jx_phase_xl(o, dt, k, xm, mt), rd);
}

/* DCO2 SAW with CROSS MOD */
static inline __attribute__((always_inline)) float jx_saw_x(jx_osc_t *o, float dt, float rd, float k, float xm,
                                                           float mt)
{
    jx_px_t x = jx_phase_x(o, dt, rd, k, xm, mt);
    return jx_tilt(o, (2.0f * x.p - 1.0f - x.plain * jx_blep(x.p, rd) + 2.0f * x.corr) * JX_SAW_GAIN);
}

/* DCO2 SQUARE / PULSE with CROSS MOD: the phase of jx_phase_x, their own BLEPs (a mark's jump uncorrected) */
static inline __attribute__((always_inline)) float jx_square_x(jx_osc_t *o, float dt, float rd, float k, float xm,
                                                              float mt)
{
    return jx_square_p(o, jx_phase_x(o, dt, rd, k, xm, mt).p, dt, rd);
}
static inline __attribute__((always_inline)) float jx_pulse_x(jx_osc_t *o, float dt, float rd, float k, float xm,
                                                             float mt)
{
    return jx_pulse_p(o, jx_phase_x(o, dt, rd, k, xm, mt).p, rd);
}

/* DCO2 NOISE: ours, not Roland's (its program outputs 0; MAIN2 takes the master's noise word, IOMD 60E2): a Weyl
   sequence through a squaring hash (mutable_ported mp_noise), -1..1 */
static inline __attribute__((always_inline)) float jx_noise(jx_osc_t *o)
{
    float x = o->p + 0.6180339887f;
    x = x >= 1.0f ? x - 1.0f : x;
    o->p = x;
    return s1_wrap(x * x * 4093.0f);
}

#endif
