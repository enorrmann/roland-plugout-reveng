/* chorus_check.c: the JX-3P CHORUS model (jx3p_chorus.h) natively (README.md §8.3): the delays it sweeps, the LFO's
 * period, L / R in antiphase, the wet path's gain over frequency, and a demo pair of WAVs (a saw chord dry and through
 * the chorus) for listening.
 *
 *   X=sdk/s1cc/examples/jx3p_ported; cc -O2 -I$X -o /tmp/chorus_check $X/tools/chorus_check.c -lm && \
 *       /tmp/chorus_check work/jx3p/chorus
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "jx3p_chorus.h"

#define FS 96000

static void wav(const char *path, const float *l, const float *r, int n)
{
    FILE *f = fopen(path, "wb");
    if (!f) { perror(path); exit(1); }
    uint32_t data = (uint32_t)n * 8, riff = 36 + data, fmt = 16, rate = FS, byterate = FS * 8;
    uint16_t tag = 3, ch = 2, align = 8, bits = 32;
    fwrite("RIFF", 1, 4, f); fwrite(&riff, 4, 1, f); fwrite("WAVEfmt ", 1, 8, f); fwrite(&fmt, 4, 1, f);
    fwrite(&tag, 2, 1, f); fwrite(&ch, 2, 1, f); fwrite(&rate, 4, 1, f); fwrite(&byterate, 4, 1, f);
    fwrite(&align, 2, 1, f); fwrite(&bits, 2, 1, f); fwrite("data", 1, 4, f); fwrite(&data, 4, 1, f);
    for (int i = 0; i < n; i++) { fwrite(&l[i], 4, 1, f); fwrite(&r[i], 4, 1, f); }
    fclose(f);
    printf("wrote %s\n", path);
}

int main(int argc, char **argv)
{
    static jx_chorus_t c;
    int ok = 1;
    /* 1. the delays over 12 s (silence in) */
    jx_chorus_init(&c);
    int n = 12 * FS, ups = 0, last_up = -1, first_up = -1;
    double mn = 1e9, mx = -1e9, sum_err = 0, prev = 0;
    for (int i = 0; i < n; i++) {
        float L, R, dl, dr;
        jx_chorus(&c, 0, 0, 255, &L, &R, &dl, &dr);
        mn = fmin(mn, dl); mx = fmax(mx, dl);
        sum_err = fmax(sum_err, fabs((dl + dr) - (2 * JXC_STAGES * JXC_X0 + JXC_STAGES * JXC_XK * JXC_C1F4)));
        double mid = JXC_STAGES * (JXC_X0 + 0.5 * JXC_XK * JXC_C1F4);
        if (i && prev < mid && dl >= mid) { if (first_up < 0) first_up = i; last_up = i; ups++; }
        prev = dl;
    }
    double period = ups > 1 ? (double)(last_up - first_up) / (ups - 1) / FS : 0;
    printf("delay L: %.1f .. %.1f samples = %.3f .. %.3f ms\n", mn, mx, 1e3 * mn / FS, 1e3 * mx / FS);
    printf("LFO period %.4f s (%.4f Hz); expected 2 / C1F5 = %.4f s\n", period, period ? 1 / period : 0,
           2.0 / JXC_C1F5 / FS);
    printf("L + R delay constant within %.3g samples (antiphase)\n", sum_err);
    ok &= fabs(mn - JXC_STAGES * JXC_X0) < 1 && fabs(mx - JXC_STAGES * (JXC_X0 + JXC_XK * JXC_C1F4)) < 1;
    ok &= fabs(period - 2.0 / JXC_C1F5 / FS) < 0.01 && sum_err < 0.01;
    /* 2. the wet path's gain (EFFECT LEVEL 255, wet = out - in), a sine per frequency, after 1 s */
    printf("wet gain (L), EFFECT LEVEL 255:");
    static const float fr[] = { 50, 200, 1000, 3000, 6000, 10000, 15000 };
    for (unsigned j = 0; j < sizeof fr / sizeof *fr; j++) {
        jx_chorus_init(&c);
        double acc = 0;
        for (int i = 0; i < 2 * FS; i++) {
            float x = sinf(2 * (float)M_PI * fr[j] * i / FS), L, R;
            jx_chorus(&c, x, x, 255, &L, &R, 0, 0);
            if (i >= FS) acc += (double)(L - x) * (L - x);
        }
        printf("  %g Hz %.1f dB", fr[j], 10 * log10(acc / FS / 0.5 + 1e-30));
    }
    printf("\n");
    /* 3. demo: C major saw chord (C4 E4 G4), 6 s, dry and through the chorus at EFFECT LEVEL 192 */
    if (argc > 1) {
        int m = 6 * FS;
        float *dl = malloc(sizeof(float) * m), *dr = malloc(sizeof(float) * m);
        float *cl = malloc(sizeof(float) * m), *cr = malloc(sizeof(float) * m);
        static const double f0[] = { 261.63, 329.63, 392.0 };
        double ph[3] = { 0 };
        jx_chorus_init(&c);
        for (int i = 0; i < m; i++) {
            float x = 0;
            for (int k = 0; k < 3; k++) {
                ph[k] += f0[k] / FS;
                ph[k] -= floor(ph[k]);
                x += (float)(0.2 * (2 * ph[k] - 1));
            }
            dl[i] = dr[i] = x;
            jx_chorus(&c, x, x, 192, &cl[i], &cr[i], 0, 0);
        }
        char p[1024];
        snprintf(p, sizeof p, "%s/demo_dry.wav", argv[1]); wav(p, dl, dr, m);
        snprintf(p, sizeof p, "%s/demo_chorus.wav", argv[1]); wav(p, cl, cr, m);
    }
    printf("%s\n", ok ? "OK" : "FAIL");
    return ok ? 0 : 1;
}
