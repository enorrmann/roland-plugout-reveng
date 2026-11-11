/* bq_osc2x.h: the JP-08's saw (oscillator program variant 2) and pulse (variant 3) at two sub-samples per sample
 * instead of four (README.md §9).
 *
 * Roland's programs compute each oscillator at four sub-samples (176.4 kHz on the Boutique, 384 kHz as lifted onto
 * the SYSTEM-1's 96 kHz), each sub-sample: the phase, the shape with its rounded edge, a two-pole low-pass (biquad);
 * then a 28-tap FIR decimates the four sub-samples to one sample. Here the per-sub-sample arithmetic is Roland's
 * (bq_jp08_host_voice.h, instructions 354..467 and 486..619: the same phase and reset rules, the same shape, the same
 * biquad coefficients), at two sub-samples (192 kHz, more than the Boutique's 176.4 kHz), and the 28-tap FIR is replaced by an
 * 11-tap FIR (below).
 *
 * The phase step per sub-sample (phase -1..1, two sub-samples per sample) is f / 96000: the host's s1_dt() (bq_host2x.c),
 * and the edge's width comes from its inverse s1_rdt(): no exponential and no reciprocal of the pitch here. */
#ifndef BQ_OSC2X_H
#define BQ_OSC2X_H

/* the decimator: 11 symmetric taps on the biquad's output at 192 kHz, least squares to a one-pole roll-off at
   12 kHz in 0..20 kHz (Roland's 28-tap FIR falls the same way: the two-sub-sample oscillator measured +1.3 dB at
   7 kHz, +2.5 dB at 10 kHz, +3.5 dB at 12.5 kHz brighter than the four-sub-sample one with a flat filter) and to 0 in
   76..96 kHz (-51.5 dB: the band that folds onto 0..20 kHz). a_j = the biquad's output j sub-samples ago (a0 = this
   sample's second sub-sample, a1 its first). README.md §9 */
#define BQ_H0 0.046144f
#define BQ_H1 0.096152f
#define BQ_H2 (-0.015874f)
#define BQ_H3 (-0.089596f)
#define BQ_H4 0.21705f
#define BQ_H5 0.480646f

typedef struct {
    float r;                     /* the running phase after the last sub-sample (Roland's sync master, free running) */
    float ph;                    /* the shape's phase after the last sub-sample (follows r: set to it at a wrap) */
    float x1, x2;                /* biquad: the last two inputs */
    float y1, y2;                /* biquad: the last two outputs (y1 = second sub-sample of the sample before) */
    float s1, s2, s3, s4, s5;    /* the second sub-sample's biquad output, 1..5 samples ago (a2, a4, .., a10) */
    float f1, f2, f3, f4;        /* the first sub-sample's, 1..4 samples ago (a3, a5, a7, a9) */
} bq_osc_t;

static inline __attribute__((always_inline)) float bq_hb(bq_osc_t *o, float a1, float a0)
{
    float y = BQ_H0 * (a0 + o->s5) + BQ_H1 * (a1 + o->f4) + BQ_H2 * (o->s1 + o->s4) + BQ_H3 * (o->f1 + o->f3)
            + BQ_H4 * (o->s2 + o->s3) + BQ_H5 * o->f2;
    o->s5 = o->s4; o->s4 = o->s3; o->s3 = o->s2; o->s2 = o->s1; o->s1 = a0;
    o->f4 = o->f3; o->f3 = o->f2; o->f2 = o->f1; o->f1 = a1;
    return y;
}

/* Roland's saw (variant 2): returns the oscillator's output word (what the program stores at 2018). inc = the phase
   step per sub-sample (-1..1 phase, two sub-samples: f / 96000), w = 1 / (4 inc) (the edge; from the host's s1_rdt,
   no reciprocal here). Roland limits the step to 0.249 (f < 23.9 kHz) */
static inline __attribute__((always_inline)) float bq_saw2x(bq_osc_t *o, float inc, float w)
{
    inc = fminf(inc, 0.249f);
    float r1 = s1_wrap(o->r + inc), r2 = s1_wrap(r1 + inc);       /* the running phase (A08B = 1: free running) */
    float p1 = SEL(K_LT0(r1 - o->r), r1, s1_wrap(o->ph + inc));
    float q = s1_wrap(p1 + inc);
    float p2 = SEL(K_LT0(q), r2, q);
    o->r = r2;
    float x1 = (s1_wrap(p1) + p1) * 0.5f * K_SIN(K_CLAMP(K_TRI(0.5f * p1 + 0.5f) * w) * 1.570796f);
    float x2 = (s1_wrap(p2) + p2) * 0.5f * K_SIN(K_CLAMP(K_TRI(0.5f * p2 + 0.5f) * w) * 1.570796f);
    float y1 = 1.5688642f * o->y1 - 0.7864105f * o->y2 + 0.054386564f * (x1 + 2.0f * o->x1 + o->x2);
    float y2 = 1.5688642f * y1 - 0.7864105f * o->y1 + 0.054386564f * (x2 + 2.0f * x1 + o->x1);
    o->ph = p2;
    o->x2 = x1; o->x1 = x2;
    o->y2 = y1; o->y1 = y2;
    return bq_hb(o, y1, y2) * 0.5037f;      /* x 0.5037: the level of Roland's four-sub-sample saw (measured) */
}

/* Roland's pulse (variant 3): pw = the program's input 200b (PWM x its source, 0..1.2); returns the word it stores
   at 201d. inc as bq_saw2x, w = 0.2 / inc. The two reciprocals of the edge's slope, 1 / (c - 1) and 1 / (c + 1), once
   per sample (Roland: per sub-sample; c changes with the pulse width only) */
static inline __attribute__((always_inline)) float bq_pulse2x(bq_osc_t *o, float inc, float w, float pw)
{
    inc = fminf(inc, 0.249f);
    float c = -0.9f * pw - 0.03f;
    float rn = K_RECIP(c - 1.0f), rp = K_RECIP(c + 1.0f);
    float r1 = s1_wrap(o->r + inc), r2 = s1_wrap(r1 + inc);       /* the running phase (A08D = 1) */
    float t = s1_wrap(o->ph + inc);
    float p1 = SEL(K_LT0(t), r1, t);
    float u = s1_wrap(p1 + inc);
    float p2 = SEL(K_LT0(r2 - r1), r2, SEL(K_Z(u), 0.0f, u));
    o->r = r2;
    float e1 = p1 + c, e2 = p2 + c;
    float v1 = s1_sign(e1) * K_CLAMP(K_TRI(SEL(K_LT0(e1), rn, rp) * e1) * w);
    float v2 = s1_sign(e2) * K_CLAMP(K_TRI(SEL(K_LT0(e2), rn, rp) * e2) * w);
    float x1 = -v1, x2 = -v2;
    float y1 = 1.5991099f * o->y1 - 0.75719583f * o->y2 + 0.03952148f * (x1 + 2.0f * o->x1 + o->x2);
    float y2 = 1.5991099f * y1 - 0.75719583f * o->y1 + 0.03952148f * (x2 + 2.0f * x1 + o->x1);
    o->ph = p2;
    o->x2 = x1; o->x1 = x2;
    o->y2 = y1; o->y1 = y2;
    return bq_hb(o, y1, y2) * 0.53f;        /* x 0.53: the level of Roland's pulse (measured: 0.507 at C7 .. 0.535 at C2) */
}

#endif
