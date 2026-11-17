/* lfo_check.c: the JX-3P LFO of the ARM side (jx3p_arm.h arm_custom_tick / jl_lfo, Q24) against Roland's MAIN0 in the
 * emulator (tools/lfo_ref.py -> work/jx3p/lfo_ref.txt), sample by sample (README.md §3.5):
 *
 *   python3 sdk/s1cc/examples/jx3p_ported/tools/lfo_ref.py
 *   cc -O2 -Isdk/s1cc/examples/jx3p_ported -o /tmp/lfo_check sdk/s1cc/examples/jx3p_ported/tools/lfo_check.c \
 *       && /tmp/lfo_check work/jx3p/lfo_ref.txt
 *
 * Per case: the largest difference over the first 12000 samples when each sample may match Roland's within +-2
 * samples (the step instants), and over the whole second the relative difference of the period (rising zero
 * crossings) and the peaks. Beyond ~12000 samples the step instants drift apart by more than 2 samples: the phase step
 * in the DSP's rounding against ours (Q31), ~0.4 sample per cycle at RATE 175, 1.7e-4 relative at RATE 60. RANDOM is skipped (Roland's takes
 * the master's noise word, 0 in the emulator).
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define NV 8
#define Q1 (1 << 24)
#define B5_DT 0x40
#define B5_RDT 0x48
#define B5_PITCH 0x30
#define P_ENV0 0x80
#define P_ARM0 0xa0
enum { ST_IDLE, ST_ATT, ST_DEC, ST_REL };
enum { C_LFO_SHAPE, C_LFO_RATE, C_LFO_DELAY, C_N };
static uint8_t hlfo_knob[C_N];
static int lfo_kt;
static int32_t qmul(int32_t a, int32_t b) { return (int32_t)(((int64_t)a * b) >> 24); }
static void b5(int idx, uint32_t bits) { (void)idx; (void)bits; }
static void ramp(int id, int32_t q) { (void)id; (void)q; }
static int cust_bend;
#include "jx3p_arm.h"

static double period(const double *x, int n)
{
    int a = -1, b = -1, c = 0;
    for (int i = 1; i < n; i++)
        if (x[i - 1] < 0 && x[i] >= 0) { if (a < 0) a = i; b = i; c++; }
    return c > 1 ? (double)(b - a) / (c - 1) : 0;
}

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "work/jx3p/lfo_ref.txt", "r");
    if (!f) { perror("lfo_ref.txt"); return 1; }
    static const int s1_shape[3] = { 0, 3, 5 };        /* JX-3P WAVEFORM -> the SYSTEM-1 LFO SHAPE that maps to it */
    char line[256];
    double worst = 0;
    while (fgets(line, sizeof line, f)) {
        int w, rate, delay, kt, n;
        if (sscanf(line, "case %d %d %d %d %d", &w, &rate, &delay, &kt, &n) != 5)
            continue;
        double *r = malloc(sizeof(double) * n), *o = malloc(sizeof(double) * n);
        for (int i = 0; i < n; i++) {
            if (!fgets(line, sizeof line, f)) return 2;
            r[i] = atof(line);
        }
        hlfo_knob[C_LFO_SHAPE] = (uint8_t)s1_shape[w];
        hlfo_knob[C_LFO_RATE] = (uint8_t)rate;
        hlfo_knob[C_LFO_DELAY] = (uint8_t)delay;
        lfo_kt = 1;
        jl_p = 0, jl_c = 0, jl_held = 0;
        arm_custom_gate(0, 1);
        /* Roland's first output (sample 0) is 0, its first value at sample 1: one sample of delay */
        o[0] = 0;
        for (int i = 1; i < n; i++) {
            arm_custom_tick(1);
            o[i] = jl_lfo(0) / (double)Q1;
        }
        if (w == 2) {
            printf("wave %d rate %3d delay %3d: RANDOM, Roland's is the master's noise (0 here): skipped\n", w, rate, delay);
            continue;
        }
        double e = 0, pk_r = 0, pk_o = 0;
        int at = 0;
        for (int i = 2; i < n - 2; i++) {
            pk_r = fmax(pk_r, fabs(r[i]));
            pk_o = fmax(pk_o, fabs(o[i]));
            if (i >= 12000)
                continue;
            double m = 1e9;
            for (int l = -2; l <= 2; l++)
                m = fmin(m, fabs(o[i] - r[i + l]));
            if (m > e) { e = m; at = i; }
        }
        double po = period(o, n), pr = period(r, n);
        printf("wave %d rate %3d delay %3d: max |ARM - Roland| %.2e (sample %d); period %.1f / %.1f (%.1e); peak %.4f / %.4f\n",
               w, rate, delay, e, at, po, pr, pr > 0 ? fabs(po / pr - 1) : 0.0, pk_o, pk_r);
        worst = fmax(worst, e);
        free(r);
        free(o);
    }
    printf("worst %.2e\n", worst);
    return 0;
}
