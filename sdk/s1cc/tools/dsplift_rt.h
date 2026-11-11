/* dsplift_rt.h: runtime of the C that sdk/s1cc/tools/dsplift.py writes (docs/vol5_toolchain_compiler_sdk/dsplift.md).
 *
 * The semantics are those of the emulator's C core (examples/from_scratch/emu/dspcore.h, the reference), restated
 * here so the lifted C compiles alone: units, SFU, flags with one instruction of latency, hidden registers.
 * Differences, by design:
 *   - the MAC is computed in doubles (dspcore.h first tries the exact 128-bit path: 1 LSB in ~1/1000 MACs);
 *   - the RAW quantisation after each A / B / M unit only with -DDSPLIFT_QUANT;
 *   - memory: DSPLIFT_FLAT (the emulator's machine mode: one flat array, bank 5 folded to 1024 words, bank 0 not a
 *     ring) or, by default, banks with the bank-0 ring (a word stored at X in a sample is read at X + 1 in the next);
 *   - tap requests (Xa9Y) and the bank-4 MXM are not modelled (reads give 0, writes go to onwrite only).
 * Build the lifted C with -ffp-contract=off (an FMA changes the bits, as in the emulator). */
#ifndef DSPLIFT_RT_H
#define DSPLIFT_RT_H
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../examples/from_scratch/emu/qmask.h"

typedef struct LiftM {
    double r[8];                  /* registers */
    uint32_t F, PM, PV;           /* condition bits; pending bits (mask, values) of the instruction before */
    double H[8];                  /* hidden registers h4..h7 */
    int64_t t;                    /* sample counter (bank-0 ring, bank-2 pages) */
#ifdef DSPLIFT_FLAT
    double flat[0x10000];
#else
    double bank0[0x800], bank1[0x800], bank2[2][0x2000], iomd[0x2000], bank5[0x800], params[0x2000], bank7[0x2000];
#endif
    void (*onwrite)(struct LiftM *, int64_t a, double v);   /* optional: sees every store */
} LiftM;

static inline double l_quant(double v)
{
    uint64_t b;
    memcpy(&b, &v, 8);
    b &= QMASK[(b >> 52) & 0x7FF];
    if ((b & 0x7FFFFFFFFFFFFFFFull) > 0x7FF0000000000000ull) b = 0x7FF8000000000000ull;
    memcpy(&v, &b, 8);
    return v;
}
static inline double pymod(double x, double m)          /* Python's x % m */
{
    double r = fmod(x, m);
    if (r != 0) {
        if ((m < 0) != (r < 0)) r += m;
    } else
        r = copysign(0.0, m);
    return r;
}
static inline double pymin(double x, double y) { return y < x ? y : x; }
static inline double pymax(double x, double y) { return y > x ? y : x; }
static inline double wrap1(double v) { return pymod(v + 1, 2) - 1; }
static inline double recip_seed(double m)
{
    int k = (int)((m - 0.5) * 128);
    if (k > 63) k = 63;
    return floor(4096 / (0.5 + (double)(2 * k + 1) / 256) + 0.5) / 4096;
}
static const uint32_t L_EXP_ROM[26] = {
    0x00000018, 0x00000041, 0x000000b2, 0x000001e3, 0x00000522, 0x00000df3, 0x000025ec, 0x00006715,
    0x00011835, 0x0002f9af, 0x00081679, 0x0015fc21, 0x003bc2d7, 0x00a27290, 0x01b993fe, 0x04b0556e,
    0x0cbed866, 0x222a5554, 0x25e2d58d, 0x30000000, 0x42b7e151, 0x4763992e, 0x5415e5bf, 0x63699205,
    0x69469c4c, 0x7936dc56};
static inline double exp_rom(double n)
{
    int i = (int)n + 19;
    if (i < 0) return 0.0;
    if (i >= 26) return 536870911.0 / 1048576.0;
    uint32_t w = L_EXP_ROM[i];
    int s = (w >> 29) & 3;
    double v = (double)(w & 0x1FFFFFFF) / ldexp(1.0, 32 - 4 * s);
    return w >> 31 ? -v : v;
}
static const int L_RSQRT_Q[48] = {504, 490, 476, 464, 452, 442, 432, 422, 414, 406, 398, 391, 384, 377, 371, 365,
                                  359, 354, 349, 344, 339, 334, 330, 326, 322, 318, 314, 311, 307, 304, 300, 297,
                                  294, 291, 288, 285, 283, 280, 277, 275, 272, 270, 268, 266, 263, 261, 259, 257};
static inline void l_sfu(int w, const double *regs, double *out)
{
    int d = (w >> 12) - 8, mode = (w >> 4) & 3, y = w & 15;
    double Y = regs[y & 7];
    if (mode == 0 && (y & 8)) {                            /* atanhsplit */
        if (Y + 1) {
            int e;
            double m = frexp((Y + 1) / 2, &e);
            out[d] = (Y - 1) / 2 * ldexp(1.0, -e);
            out[y & 7] = m;
            out[(d + 1) & 7] = recip_seed(m);
        }
    } else if (mode == 0) {                                /* recipseed */
        if (Y) {
            int e;
            double m = frexp(fabs(Y), &e);
            out[d] = e > -9 ? copysign(ldexp(1.0, -e), Y) : 0.0;
            out[(d + 1) & 7] = recip_seed(m);
            out[y & 7] = m;
        }
    } else if (mode == 1 && !(y & 8)) {                    /* sqrtsplit */
        if (Y) {
            double m = fabs(Y);
            int k = 0;
            while (m >= 4) m /= 4, k--;
            while (m < 1) m *= 4, k++;
            int q = (int)((m - 1) * 16);
            if (q > 47) q = 47;
            out[y & 7] = m;
            out[(d + 1) & 7] = L_RSQRT_Q[q] / 512.0;
            out[(d + 3) & 7] = copysign(ldexp(1.0, -k), Y);
        }
    } else if (mode == 2 && !(y & 8)) {                    /* log2split */
        if (Y > 0) {
            double e = floor(log2(Y / 0.75));
            out[d] = e;
            out[y & 7] = Y / ldexp(1.0, (int)e);
        } else if (Y < 0) {
            double e = floor(log2((Y + 512) / 0.75));
            out[d] = e;
            out[y & 7] = floor(Y * ldexp(1.0, 32 - (int)e)) * 0x1p-32;
        }
    } else if (mode == 1) {                                /* expsplit */
        double fl = floor(Y);
        out[d] = exp_rom(fl);
        out[y & 7] = Y - fl;
    } else if (mode == 2)
        out[d] = Y ? copysign(1.0, Y) : 0.0;               /* sign */
    else if (y & 8)
        out[d] = floor(Y);
    else
        out[d] = ceil(Y);
}
static inline void l_divseed(int w, const double *regs, double *out)
{
    int d = w >> 12, mode = (w >> 4) & 3, y = w & 15;
    double R = regs[y & 7], N = (y & 8) ? R : regs[2 * mode];
    if (R) {
        int e;
        double m = frexp(fabs(R), &e);
        out[d & 7] = e > -9 ? floor(N * copysign(ldexp(1.0, 32 - e), R)) * 0x1p-32 : 0.0;
        out[(d + 1) & 7] = recip_seed(m);
        out[y & 7] = m;
    }
}
static inline double l_bitop(int k1, double X, double Y)
{
    const double Q = 8388608.0;
    int64_t xi = (int64_t)nearbyint(X * Q), yi = (int64_t)nearbyint(Y * Q), r = 0;
    switch (k1) {
    case 0: r = xi & yi; break;
    case 2: r = ~xi & yi; break;
    case 4: r = xi | yi; break;
    case 6: r = ~xi | yi; break;
    case 8: r = xi ^ yi; break;
    case 0xA: r = ~(xi ^ yi); break;
    }
    return pymod((double)r / Q, 512);
}
static inline double l_shift(double X, int sh) { return pymod(floor(X * ldexp(1.0, sh + 32)) * 0x1p-32 + 512, 1024) - 512; }
static inline uint32_t flag_bits(double v, int z)       /* !Z, !N, !V, N or Z */
{
    int n = v < 0, ov = fabs(v) >= 512;
    return (uint32_t)(!z) | (uint32_t)(!n) << 1 | (uint32_t)(!ov) << 2 | (uint32_t)(n || z) << 3;
}

#ifdef DSPLIFT_FLAT
static inline int64_t l_key(int64_t a) { return a >= 0xA000 && a < 0xC000 ? (0xA000 | (a & 0x3FF)) : a; }
static inline double RD(LiftM *M, int64_t a)
{
    a = l_key(a);
    return a >= 0 && a < 0x10000 ? M->flat[a] : 0.0;
}
static inline void WR(LiftM *M, int64_t a, double v)
{
    if (M->onwrite) M->onwrite(M, a, v);
    a = l_key(a);
    if (a >= 0 && a < 0x10000) M->flat[a] = v;
}
#else
static inline int64_t l_ring(LiftM *M, int64_t a) { return (a - M->t) & 0x7FF; }
#ifdef DSPLIFT_RING1                 /* dsplift.py --bank1-ring: bank 1 is a ring as bank 0 (the Boutique BQ DSPs) */
#define L_B1(M, off) l_ring(M, (off) & 0x7FF)
#else
#define L_B1(M, off) ((off) & 0x7FF)
#endif
static inline double RD(LiftM *M, int64_t a)
{
    if (a < 0) return 0;
    int64_t off = a & 0x1FFF;
    switch (a >> 13) {
    case 0: return M->bank0[l_ring(M, off & 0x7FF)];
    case 1: return M->bank1[L_B1(M, off)];
    case 2: return M->bank2[M->t & 1][off];
    case 3: return M->iomd[off];
    case 5: return M->bank5[off & 0x7FF];
    case 6: return M->params[off];
    case 7: return M->bank7[off];
    default: return 0;
    }
}
static inline void WR(LiftM *M, int64_t a, double v)
{
    if (M->onwrite) M->onwrite(M, a, v);
    if (a < 0) return;
    int64_t off = a & 0x1FFF;
    switch (a >> 13) {
    case 0: M->bank0[l_ring(M, off & 0x7FF)] = v; break;
    case 1: M->bank1[L_B1(M, off)] = v; break;
    case 2: M->bank2[M->t & 1][off] = v; break;
    case 3: M->iomd[off] = v; break;
    case 5: M->bank5[off & 0x7FF] = v; break;
    default: break;
    }
}
#endif

/* a delay-tap request without a delay line (as the emulator without an EDM): the two samples and the fraction
 * read 0, written to bank 2 at [h5 .. h5 + 2] */
static inline void TAP(LiftM *M, double a)
{
    (void)a;
    int64_t dest = (int64_t)(M->H[5] * 16384) & 0x1FFF;
    for (int k = 0; k < 3; k++) WR(M, 0x4000 + ((dest + k) & 0x1FFF), 0.0);
}

/* the macros the lifted code uses inside one unit */
#ifdef DSPLIFT_QUANT
#define QUANT(n) do { for (int _q = 0; _q < 8; _q++) n[_q] = l_quant(n[_q]); } while (0)
#else
#define QUANT(n) do { } while (0)
#endif
#define RESET() int rs = 0; double rvv = 0; int rzz = 0
#define RES(x) (rs = 1, rvv = (x), rzz = (x) == 0)
#define RESV(x) do { double _v = (x); rs = 1, rvv = _v, rzz = _v == 0; } while (0)
#define RESZ(v, z) (rs = 1, rvv = (v), rzz = (z))
#ifdef DSPLIFT_NO_OVERFLOW_WRAP                 /* A-unit results past +-512 kept (a DSP that saturates; -D option) */
#define AWRAP(e) (e)
#else
#define AWRAP(e) ({ double _r = (e); if (fabs(_r) >= 512) { rs = 1; rvv = _r; _r = copysign(fmod(fabs(_r), 512), _r); rzz = _r == 0; } _r; })
#endif
#define DONE(u, dest) do { if (!rs) { rvv = n[dest]; rzz = n[dest] == 0; } rv[u] = rvv; rz[u] = rzz; dn |= 1ull << (u); } while (0)
#define SETFLAGS(a, b) do { if (dn >> (b) & 1) { uint32_t _sh = 4 * (a); um |= 0xFu << _sh; uv = (uv & ~(0xFu << _sh)) | flag_bits(rv[b], rz[b]) << _sh; } } while (0)
#define SFU(w) l_sfu(w, r, n)
#define DIVSEED(w) l_divseed(w, r, n)
#define FREXP(d, y, X) do { int _e = -31; double _m = 0, _x = (X); if (_x) _m = frexp(_x, &_e); n[d] = _m; n[y] = _e; } while (0)
#define SHIFT(X, sh) l_shift(X, sh)
#define BITOP(k, X, Y) l_bitop(k, X, Y)
#define F5C(M) (fmod((double)(M)->t, 512.0 * 16384) / 16384)
#define ABORT(s) do { fprintf(stderr, "%s\n", s); abort(); } while (0)
#endif
