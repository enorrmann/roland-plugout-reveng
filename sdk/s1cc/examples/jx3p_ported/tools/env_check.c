/* env_check.c: the JX-3P envelopes of the ARM side (jx3p_arm.h arm_custom_env, Q24) against Roland's MAIN0 in the
 * emulator (tools/env_ref.py -> work/jx3p/env_ref.txt), tick by tick (README.md §3.4):
 *
 *   python3 sdk/s1cc/examples/jx3p_ported/tools/env_ref.py
 *   cc -O2 -Isdk/s1cc/examples/jx3p_ported -o /tmp/env_check sdk/s1cc/examples/jx3p_ported/tools/env_check.c \
 *       && /tmp/env_check work/jx3p/env_ref.txt
 */
#include <stdio.h>
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
static int32_t qmul(int32_t a, int32_t b) { return (int32_t)(((int64_t)a * b) >> 24); }
static void b5(int idx, uint32_t bits) { (void)idx; (void)bits; }
static void ramp(int id, int32_t q) { (void)id; (void)q; }
static int cust_bend;
enum { C_LFO_SHAPE, C_LFO_RATE, C_LFO_DELAY, C_N };   /* engine.c's host LFO knobs, as jx3p_arm.h reads them */
static uint8_t hlfo_knob[C_N];
static int lfo_kt;
#include "jx3p_arm.h"

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "work/jx3p/env_ref.txt", "r");
    if (!f) { perror("env_ref.txt"); return 1; }
    char line[256];
    double worst = 0;
    while (fgets(line, sizeof line, f)) {
        int a, d, s, r, off, nt;
        if (sscanf(line, "case %d %d %d %d %d %d", &a, &d, &s, &r, &off, &nt) != 6)
            continue;
        uint8_t k[4] = { (uint8_t)s, (uint8_t)a, (uint8_t)d, (uint8_t)r };
        int32_t l = 0;
        int st = ST_ATT;
        double e = 0, el = 0;
        int at = 0;
        memset(arm_env_prev, 0, sizeof arm_env_prev);
        for (int t = 0; t < nt; t++) {
            double r1, r2;
            if (!fgets(line, sizeof line, f) || sscanf(line, "%lf %lf", &r1, &r2) != 2)
                return 2;
            if (t == off && st != ST_IDLE)
                st = ST_REL;
            /* one tick = 2 steps of 1 ms: the tick runs in the first step of the pair */
            st = arm_custom_env(1, 0, st, &l, k, t == 0 ? 1 : 2);
            double x = l / (double)Q1, dd = fabs(x - r1);
            if (dd > e) { e = dd; el = r1; at = t; }
        }
        printf("A %3d D %3d S %3d R %3d: max |ARM - Roland| %.2e (tick %d, Roland %.5f)\n", a, d, s, r, e, at, el);
        if (e > worst) worst = e;
    }
    printf("worst %.2e\n", worst);
    return 0;
}
