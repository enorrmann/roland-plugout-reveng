/* xmod_check.c: DCO2 CROSS MOD of our oscillators (jx3p_osc.h jx_mark / jx_saw_x) against Roland's DCO2 saw program
 * lifted to C (jx3p_dco2_saw.h), natively (README.md §4.3):
 *
 *   cc -O2 -Isdk/s1cc/tools -Isdk/s1cc/examples/jx3p_ported -o /tmp/xmod_check \
 *       sdk/s1cc/examples/jx3p_ported/tools/xmod_check.c -lm && /tmp/xmod_check
 *
 * DCO1 is Roland's phase at four sub-samples (span 2, step 2^(pitch1 - 4.75) x 0.0022916666), whose wrap mark (the
 * sub-sample, 1..4, of the last wrap: 0.25 .. 1) DCO2 reads one sample later, as MAIN2 hands it (A022 x the mark).
 * Ours: DCO1's phase per sample and jx_mark. For SYNC and METAL at several DCO1 / DCO2 pitches: the harmonics of
 * DCO1's period (the synced sound repeats at it) of both, dB re Roland's strongest, and the SNR after alignment.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define DSPLIFT_K_NATIVE
static inline float s1_exp2(float x) { return exp2f(x); }
static float in[8], rout;
#define KP(a) 0.0f
#define KIN(a) in[(a) - 0xa0d0]
#define KOUT(a, v) do { if ((a) == 0x2221) rout = (v); } while (0)
#define KE(a) 0.0f
#include "dsplift_k.h"
#include "jx3p_dco2_saw.h"
#define JX_SAW_GAIN 0.5f
#define JX_PULSE_GAIN (-0.9f)
#define JX_SQ_GAIN 0.33f
#define JX_SQ_SPIKE 0.5f
#include "jx3p_osc.h"

#ifndef XSAW
#define XSAW jx_saw_xl
#endif
#define N 96000
static float r[N], u[N];

static double harm(const float *x, double f, int k)
{
    double re = 0, im = 0;
    for (int i = N / 2; i < N; i++) {
        double w = 0.5 - 0.5 * cos(2 * M_PI * (i - N / 2) / (N / 2)), ph = 2 * M_PI * k * f * i / 96000.0;
        re += w * x[i] * cos(ph);
        im += w * x[i] * sin(ph);
    }
    return sqrt(re * re + im * im);
}

int main(void)
{
    static const float cases[][3] = { { 4.75f, 5.30f, 1 }, { 4.75f, 6.10f, 1 }, { 3.75f, 4.45f, 1 }, { 4.75f, 5.30f, 2 },
                                      { 4.75f, 6.10f, 2 }, { 4.75f, 4.75f, 0 } };
    for (unsigned c = 0; c < sizeof cases / sizeof *cases; c++) {
        float p1 = cases[c][0], p2 = cases[c][1];
        int mode = (int)cases[c][2];
        float xm = mode >= 1, mt = mode == 2;
        /* Roland: DCO1 at four sub-samples */
        float inc1 = fminf(exp2f(p1 - 4.75f) * 0.0022916666f, 0.249f), ph1 = 0, mark = 0;
        in[0] = p2; in[1] = xm; in[2] = mt; in[4] = 0;
        jx_osc_t o1 = { 0 }, o2 = { 0 };
        float dt1 = (float)(440.0 * exp2(p1 - 4.75) / 96000.0), rd1 = 1.0f / dt1;
        float dt2 = (float)(440.0 * exp2(p2 - 4.75) / 96000.0), rd2 = 1.0f / dt2;
        float k = 0;
        for (int i = 0; i < N; i++) {
            in[3] = xm * mark;                        /* the mark of the sample before (A022 x) */
            mark = 0;
            for (int j = 1; j <= 4; j++) {
                float q = ph1 + inc1;
                if (q >= 1.0f) { q -= 2.0f; mark = 0.25f * j; }
                ph1 = q;
            }
            jx3p_dco2_saw();
            r[i] = rout;
            /* ours */
            u[i] = XSAW(&o2, dt2, rd2, xm * k, xm, mt);
            jx_phase(&o1, dt1);
            k = jx_mark(o1.p, dt1, rd1);
        }
        double f = 440.0 * exp2(p1 - 4.75);
        int per = (int)(96000.0 / f) + 1, best = 0;
        double be = 1e30;
        for (int lag = 0; lag < per + 8; lag++) {
            double e = 0;
            for (int i = N / 2; i < N / 2 + 9600; i++)
                e += (r[i] - u[i - lag]) * (r[i] - u[i - lag]);
            if (e < be) be = e, best = lag;
        }
        double e = 0, sr = 0;
        for (int i = N / 2; i < N; i++) {
            e += (r[i] - u[i - best]) * (r[i] - u[i - best]);
            sr += r[i] * r[i];
        }
        double h[13], hu[13], hm = 0;
        for (int kk = 1; kk <= 12; kk++) {
            h[kk] = harm(r, f, kk);
            hu[kk] = harm(u, f, kk);
            hm = fmax(hm, h[kk]);
        }
        printf("%s DCO1 %.2f DCO2 %.2f: SNR %.1f dB (lag %d)\n   Roland:", mode == 1 ? "SYNC " : mode == 2 ? "METAL" : "OFF  ",
               p1, p2, 10 * log10(sr / (e + 1e-30)), best);
        for (int kk = 1; kk <= 12; kk++) printf(" %6.1f", 20 * log10(h[kk] / hm + 1e-12));
        printf("\n   ours:  ");
        for (int kk = 1; kk <= 12; kk++) printf(" %6.1f", 20 * log10(hu[kk] / hm + 1e-12));
        /* bands of a third of an octave (METAL is not periodic at DCO1's period): energy, ours - Roland, dB */
        printf("\n   bands (ours - Roland, dB, 100 Hz .. 12.8 kHz):");
        for (double lo = 100; lo < 12800; lo *= 1.587401) {
            double er = 0, eu = 0;
            for (int i = N / 2; i < N; i++) { (void)i; }
            int K = 4096;
            for (int b = (int)(lo * K / 96000.0); b < (int)(lo * 1.587401 * K / 96000.0) + 1; b++) {
                double rr = 0, ri = 0, ur = 0, ui = 0;
                for (int i = 0; i < K; i++) {
                    double w = 0.5 - 0.5 * cos(2 * M_PI * i / K), a = -2 * M_PI * b * i / K;
                    rr += w * r[N / 2 + i] * cos(a); ri += w * r[N / 2 + i] * sin(a);
                    ur += w * u[N / 2 + i] * cos(a); ui += w * u[N / 2 + i] * sin(a);
                }
                er += rr * rr + ri * ri; eu += ur * ur + ui * ui;
            }
            printf(" %+.1f", 10 * log10((eu + 1e-30) / (er + 1e-30)));
        }
        printf("\n");
    }
    return 0;
}
