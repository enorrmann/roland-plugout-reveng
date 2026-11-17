/* arm_check.c: the ARM side (jx3p_arm.h, integer) against Roland's voice lifted to C (jx3p_voice.h, float), natively
 * (README.md §3.3). For each case of work/jx3p/arm_vectors.txt (tools/make_arm.py --vectors: knob values as the ARM
 * reads them and the S8 PLUG-OUT's words for the same settings) the lifted voice runs 4000 samples with constant
 * inputs (its smoothing settles) and gives the filter's g and both DCO pitches; the ARM code gives its g, s1_dt
 * and DCO2 / DCO1. The lift's three values come out through probes added by sed:
 *
 *   X=sdk/s1cc/examples/jx3p_ported; T=$(mktemp -d)
 *   python3 $X/tools/make_arm.py --vectors
 *   sed -e '/float brilliance_2 = /a KOUT(0x7777, brilliance_2);' -e '/float dco_5 = /a KOUT(0x7778, dco_5);' \
 *       -e '/float dco2 = /a KOUT(0x7779, dco2);' $X/jx3p_voice.h > $T/jx3p_voice_probe.h
 *   cc -O2 -I$T -Isdk/s1cc/tools -I$X -o $T/arm_check $X/tools/arm_check.c -lm && $T/arm_check work/jx3p/arm_vectors.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* ---- Roland's voice ---- */
#define DSPLIFT_K_NATIVE
static inline float s1_exp2(float x) { return exp2f(x); }
static float W[0x10000];
static float r_g, r_p1, r_p2;
#define KP(a) W[(a) & 0xffff]
#define KIN(a) W[(a) & 0xffff]
#define KOUT(a, v) do { switch (a) { case 0x7777: r_g = (v); break; case 0x7778: r_p1 = (v); break; \
                                      case 0x7779: r_p2 = (v); break; } } while (0)
#define KE(a) 0.0f
#include "dsplift_k.h"
#include "jx3p_voice_probe.h"

/* ---- the ARM (engine.c's helpers, as there) ---- */
#define NV 8
#define Q1 (1 << 24)
#define B5_DT 0x40
#define B5_RDT 0x48
#define B5_PITCH 0x30
#define P_ENV0 0x80
#define P_ARM0 0xa0
static int32_t qmul(int32_t a, int32_t b) { return (int32_t)(((int64_t)a * b) >> 24); }
static uint32_t b5w[0x100];
static int32_t rampw[0x100];
static void b5(int idx, uint32_t bits) { b5w[idx] = bits; }
static void ramp(int id, int32_t q) { rampw[id] = q; }
enum { ST_IDLE, ST_ATT, ST_DEC, ST_REL };
static int cust_bend, cust_mod;
enum { C_LFO_SHAPE, C_LFO_RATE, C_LFO_DELAY, C_N };   /* engine.c's host LFO knobs, as jx3p_arm.h reads them */
static uint8_t hlfo_knob[C_N];
static int lfo_kt;
#define ARM_S1_OCT 0                                   /* Roland's S8 scale (jx3p_arm.h) */
#include "jx3p_arm.h"

static float bits_f(uint32_t b) { float f; memcpy(&f, &b, 4); return f; }

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "work/jx3p/arm_vectors.txt", "r");
    if (!f) { perror("arm_vectors.txt"); return 1; }
    char line[256];
    uint8_t k[ARM_CUSTOM_NK];
    int note = 60, n = 0;
    int32_t e1 = 0, lf = 0;
    double eg = 0, ep1 = 0, er = 0, erd = 0;
    while (fgets(line, sizeof line, f)) {
        if (line[0] == 'K') {
            int v[25];
            char *s = line + 1;
            for (int i = 0; i < 25; i++)
                v[i] = (int)strtol(s, &s, 10);
            for (int i = 0; i < ARM_CUSTOM_NK; i++)    /* 0..13 the knobs, 14 / 15 VCA (0: the default), 16..21 */
                k[i] = (uint8_t)(i < 14 ? v[i] : i < 16 ? 0 : v[19 + (i - 16)]);
            note = v[14], e1 = v[15], lf = v[16], cust_bend = v[17], cust_mod = v[18];
            memset(W, 0, sizeof W);
        } else if (line[0] == 'W') {
            unsigned a;
            double x;
            sscanf(line + 2, "%x %lf", &a, &x);
            W[a] = (float)x;
        } else if (line[0] == 'E') {
            for (int i = 0; i < 4000; i++)
                jx3p_voice();
            /* the case's LFO word (A0A5) through the JX-3P LFO's state: SINE held = lf, DELAY 0 (x (0 + 1 - 0)) */
            jl_held = lf;
            jl_fade[0] = 0;
            arm_custom_voice(0, 0, e1, lf, note, k, 0);
            double g = rampw[P_ENV0 + 8] / (double)Q1;
            double dt1 = bits_f(b5w[B5_DT]), rdt1 = bits_f(b5w[B5_RDT]) * 16384.0;
            double dt2 = bits_f(b5w[B5_PITCH]), irat = rampw[P_ARM0] / (double)Q1 * 256.0;
            /* Roland's step per sub-sample is 2^(p - 4.75) x 0.0022916666 for a phase -1..1 at 384 kHz: f / 96000 =
               2 x that (four sub-samples, span 2) */
            double r1 = 2.0 * exp2(r_p1 - 4.75) * 0.0022916666, r2 = 2.0 * exp2(r_p2 - 4.75) * 0.0022916666;
            double dg = fabs(g - r_g), d1 = 1200.0 * fabs(log2(dt1 / r1)), d2 = 1200.0 * fabs(log2(dt2 / r2));
            double dr = fabs(rdt1 * dt1 - 1.0) + fabs(irat * dt2 / dt1 - 1.0);
            if (dg > eg) eg = dg;
            if (d1 > ep1) ep1 = d1;
            if (d2 > er) er = d2;
            if (dr > erd) erd = dr;
            if (dg > 2e-3 || d1 > 0.5 || d2 > 0.5)
                printf("case %d: g %.6f (Roland %.6f)  DCO1 %.3f cents  DCO2 %.3f cents\n", n, g, r_g, d1, d2);
            n++;
        }
    }
    printf("%d cases: g max |diff| %.2e; DCO1 max %.3f cents; DCO2 max %.3f cents; rdt, dt1/dt2 word max rel %.2e\n",
           n, eg, ep1, er, erd);
    return 0;
}
