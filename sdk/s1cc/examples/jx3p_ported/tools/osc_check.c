/* osc_check.c: our oscillators (jx3p_osc.h) against Roland's DCO programs lifted to C (jx3p_dco1_saw.h,
 * jx3p_dco1_sqr.h), natively, sample by sample (README.md §4). One waveform per build (the lifts share cell names):
 *
 *   cc -O2 -Isdk/s1cc/tools -Isdk/s1cc/examples/jx3p_ported -DWAVE=0 -o /tmp/oc0 \
 *       sdk/s1cc/examples/jx3p_ported/tools/osc_check.c -lm && /tmp/oc0
 *
 * WAVE 0 SAW (saw program, wave word 0), 1 SQUARE (sqr program), 2 PULSE (saw program, wave word 1). For each
 * pitch: the lag that best aligns the two (Roland's FIR delays ~3.5 samples), the error after it (SNR, dB), the
 * first harmonics of both (dB re Roland's fundamental), and Roland's peak and DC. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define DSPLIFT_K_NATIVE
static inline float s1_exp2(float x) { return exp2f(x); }
static inline float s1_wrap(float x);
static float in[8], rout;
#define KP(a) 0.0f
#define KIN(a) in[(a) - 0xa0d0]
#define KOUT(a, v) do { if ((a) == 0x221f) rout = (v); } while (0)
#define KE(a) 0.0f
#include "dsplift_k.h"
#ifndef WAVE
#define WAVE 0
#endif
#if WAVE == 1
#include "jx3p_dco1_sqr.h"
#define ROLAND jx3p_dco1_sqr
#else
#include "jx3p_dco1_saw.h"
#define ROLAND jx3p_dco1_saw
#endif
/* the output gains and signs (fitted here: the FIR of the lifts inverts) */
#define JX_SAW_GAIN 0.5f
#define JX_PULSE_GAIN (-0.9f)
#define JX_SQ_GAIN 0.33f
#define JX_SQ_SPIKE 0.5f
#include "jx3p_osc.h"

#define N 96000
static float r[N], u[N];

static double harm(const float *x, double f, int k)    /* |X(k f)| over the last half, Hann window */
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
    static const float pitches[] = { 0.75f, 2.75f, 4.75f, 6.75f, 8.25f };   /* octaves, 4.75 = A 440 */
    for (unsigned j = 0; j < sizeof pitches / sizeof *pitches; j++) {
        float pitch = pitches[j];
        double f = 440.0 * exp2(pitch - 4.75);
        in[0] = pitch;
        in[4] = WAVE == 2 ? 1.0f : 0.0f;
        jx_osc_t o = { 0 };
        float dt = (float)(f / 96000.0), rd = 1.0f / dt;
        for (int i = 0; i < N; i++) {
            ROLAND();
            r[i] = rout;
#if WAVE == 0
            u[i] = jx_saw(&o, dt, rd);
#elif WAVE == 1
            u[i] = jx_square(&o, dt, rd);
#else
            u[i] = jx_pulse(&o, dt, rd);
#endif
        }
        /* our phase starts at 0, Roland's at its own: align by the lag (0..period) with the least error */
        int per = (int)(96000.0 / f) + 1, best = 0;
        double be = 1e30, sr = 0;
        for (int lag = 0; lag < per + 8 && lag < 4000; lag++) {
            double e = 0;
            for (int i = N / 2; i < N / 2 + 9600; i++)
                e += (r[i] - u[i - lag]) * (r[i] - u[i - lag]);
            if (e < be)
                be = e, best = lag;
        }
        double e = 0, dc = 0, pk = 0;
        for (int i = N / 2; i < N; i++) {
            e += (r[i] - u[i - best]) * (r[i] - u[i - best]);
            sr += r[i] * r[i];
            dc += r[i];
            pk = fmax(pk, fabs(r[i]));
        }
        printf("WAVE %d pitch %.2f (%.0f Hz): lag %d SNR %.1f dB, Roland peak %.3f DC %.4f\n", WAVE, pitch, f, best,
               10 * log10(sr / (e + 1e-30)), pk, dc / (N / 2));
        double h1 = harm(r, f, 1);
        printf("   harm  Roland:");
        for (int k = 1; k <= 8 && k * f < 48000; k++)
            printf(" %6.1f", 20 * log10(harm(r, f, k) / h1 + 1e-12));
        printf("\n         ours:  ");
        for (int k = 1; k <= 8 && k * f < 48000; k++)
            printf(" %6.1f", 20 * log10(harm(u, f, k) / h1 + 1e-12));
        printf("\n");
    }
    return 0;
}
