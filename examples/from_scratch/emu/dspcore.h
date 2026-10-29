/* Shared by the interpreter (dspcore.c) and the native translations of dspjit.py: the slave state and the
 * instruction semantics (the DSP reference since 2026-10-29), as inline functions. A translated program calls the same functions
 * with constant words, so the two paths cannot disagree on what an instruction does.
 *
 * Every arithmetic step is exact as measured, with Python's float modulo and min/max argument order (the archived
 * Python model's, kept bit for bit).
 * Build with -ffp-contract=off: a fused multiply-add would change the bits (every product is rounded).
 */
#ifndef DSPCORE_H
#define DSPCORE_H
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "qmask.h"

#define RING 0x800
#define NDSP 6
#define MAXSTEPS 100000
#define INL static inline __attribute__((always_inline))

typedef struct Slave Slave;

typedef struct Dsp {
    int n;                        /* DSP number 0..5 */
    double regs[8];
    uint32_t flags;               /* bit c = condition c */
    uint32_t pmask, pval;         /* pending flags (one instruction of latency): bits to replace, their values */
    double hidden[8];             /* h4..h7 */
    double bank0[RING];
    double bank1_own[RING];
    double *bank1;
    double bank5_own[0x400];
    double bank2[2][0x2000];      /* bank 2: two pages alternating by sample (a store is read back 2 samples later);
                                   * the delay line's DMA window: [4300] -> EDM, tap results [h5 ..] (DSP_HARDWARE §8.4) */
    double *bank5;                /* bank5_own, or the partner's (Model.shared5: one bank 5 for two DSPs) */
    double *params;               /* bank 6: the slave's, or params_own (Model.own_params: one per DSP) */
    double params_own[0x2000];
    double own_hi[0x400];         /* bank 5 0x400-0x7FF written in this sample */
    int64_t own_hi_st[0x400];     /* written in sample st - 1 */
    int hi_list[0x400];           /* offsets written in this sample, in order */
    int nhi;
    double mxm_in[0x2000];
    int64_t mxm_in_st[0x2000];
    int64_t in_gen;
    double mxm_out[0x2000];
    int64_t mxm_out_st[0x2000];
    int64_t out_gen;
    int out_list[0x2000];         /* written MXM offsets of this sample, in order */
    int nout;
    int progs[16];
    int nprogs;
    int64_t ninstr;
    int64_t t;                    /* sample this DSP is running: the slave's, one ahead in the pipeline stage A */
    char err[200];
} Dsp;

typedef int (*native_fn)(Slave *s, Dsp *d);

typedef struct {
    int ninsn;
    struct Insn *insn;
    struct Op *ops;
    struct Cond *conds;
    native_fn native;
} Prog;

struct Slave {
    int ring1, mxm_shift, mxm_delay, dp_pairs, dp_delay;
    int order[NDSP], norder;
    Dsp d[NDSP];
    double iomd[0x2000];
    double params[0x2000];
    double dp[3][2][0x400];       /* pair window: dp_delay 1 = two buffers by sample parity; 0 = buffer 0 */
    int64_t dp_st[3][2][0x400];
    int64_t t;
    Prog *progs;
    int nprogs;
    /* debugging: called after every instruction by the interpreter, and by native code built with trace=True */
    void (*trace)(int dsp, int insn, const double *regs, uint32_t flags, uint32_t pmask, uint32_t pval);
    /* pipeline (sl_set_pipeline): DSP pipe_a runs sample t + 1 on its own thread while the others run sample t;
     * pipe_b (its shared-bank-5 partner) reads pipe_buf, where the bank-5 words pipe_a writes (pipe_wa) are copied
     * at each barrier; the words the others write (pipe_wb) go back to the shared bank 5 at the end of sl_step */
    int pipe_a, pipe_b;
    int pipe_wa[0x400], npipe_wa, pipe_wb[0x400], npipe_wb;
    double pipe_buf[0x400];
    int group[NDSP][NDSP];        /* thread groups: DSPs in execution order; group 0 runs on the calling thread */
    int ngroup[NDSP], ngroups;
    char err[256];
    /* machine mode (dsp_machine_new: one DSP and flat memory, the tools' simulator): every address a word of
     * flat[0x10000], bank 5 (A000-BFFF) folded to its 1024 words; access(op 'r' / 'w', address, value), when set,
     * takes every access instead (sim_main.py: refuse unmeasured banks, inputs and outputs) */
    double *flat;
    double (*access)(int op, int64_t a, double v);
    /* the delay line (DSP_HARDWARE.md §8.4, DSP_USOS.md §1; master DSP4 slot f7dc, measured 2026-10-17): the EDM
     * (edm_len words), written at the end of each sample with bank-2 word 0x300 of DSP edm_dsp (-1: no window);
     * a tap request with address A reads x[n - edm_lat - floor(k)] and the one before it, k = (A - edm_base) 16384,
     * and the fraction of k truncated to 1/64. Machine mode: the window is flat[4300], dsp_machine_end_sample */
    double *edm;
    int64_t edm_len;
    double edm_base;
    int edm_lat, edm_dsp;
};

typedef struct { int set; double v; int z; } Res;

/* ------------------------------------------------------------------ numbers */
/* quant: the smallest s whose floor(m 2^(32-4s)) < 2^29, i.e. m < 2^(4s-3), truncates m to a multiple of
 * 2^-(32-4s); m >= 512 (and inf, nan) is kept. s follows from the binary exponent E of m (m in [2^E, 2^(E+1))):
 * s = clamp((E + 7) >> 2, 0, 3). Without branches: the magnitude decides s, and a branch on it is mispredicted
 * most of the time (it was 90 % of the run time). quant(quant(x)) = quant(x). quant_ref is the plain form
 * (test_core.py compares the two). */

/* Truncating |m| to a multiple of 2^-p (p = 32 - 4s) is clearing the mantissa bits below 2^-p: with m in
 * [2^E, 2^(E+1)) those are the k = 52 - p - E = 20 + 4s - E lowest bits of the IEEE word. k <= 0: nothing to clear;
 * k > 52: m < 2^-p, the result is a zero with the sign of v; k = 63 clears every bit but the sign, which
 * is that zero. |v| >= 512, inf and nan (E >= 9) are kept: k = 0. Same bits as the scale-and-convert form (the
 * integer conversion of m 2^p is its floor); test_core.py test_quant checks it against quant_ref and the definition. */
/* Every NaN leaves quant as one canonical NaN (+qNaN). The DSP has no NaN (only random test states with inf reach
 * one), but C leaves open which NaN an op with NaN operands returns, and compilers commute operands: the sign
 * of the NaN then depended on the optimisation (it reached `sign` and gave -5 instead of -3 in p0s_003661c, the
 * "GCC 14 miscompile" of EMULATOR.md §3.6, also with clang). After quant the result no longer depends on it. */
#define QNAN_BITS UINT64_C(0x7FF8000000000000)

INL double quant(double v)
{
    uint64_t b;
    memcpy(&b, &v, sizeof b);
    b &= QMASK[(b >> 52) & 0x7FF];            /* qmask.h (gen_qmask.py): the k rule above, one mask per exponent */
    b = (b & UINT64_C(0x7FFFFFFFFFFFFFFF)) > UINT64_C(0x7FF0000000000000) ? QNAN_BITS : b;
    memcpy(&v, &b, sizeof v);
    return v;
}

static inline double quant_ref(double v)
{
    double m = fabs(v);
    if (m != m) {
        uint64_t b = QNAN_BITS;
        memcpy(&v, &b, sizeof v);
        return v;
    }
    if (v == 0 || m >= 512.0) return v;
    if (m < 0.125) return copysign(floor(m * 4294967296.0) * (1.0 / 4294967296.0), v);
    if (m < 2.0) return copysign(floor(m * 268435456.0) * (1.0 / 268435456.0), v);
    if (m < 32.0) return copysign(floor(m * 16777216.0) * (1.0 / 16777216.0), v);
    return copysign(floor(m * 1048576.0) * (1.0 / 1048576.0), v);
}

static inline double pymod_ref(double x, double m);

/* e^n of the SFU's exp split, read from the device (EXP_ROM): RAW words for n = -19 .. 6 */
static const uint32_t EXP_ROM[26] = {
    0x00000018, 0x00000041, 0x000000b2, 0x000001e3, 0x00000522, 0x00000df3, 0x000025ec, 0x00006715,
    0x00011835, 0x0002f9af, 0x00081679, 0x0015fc21, 0x003bc2d7, 0x00a27290, 0x01b993fe, 0x04b0556e,
    0x0cbed866, 0x222a5554, 0x25e2d58d, 0x30000000, 0x42b7e151, 0x4763992e, 0x5415e5bf, 0x63699205,
    0x69469c4c, 0x7936dc56 };
#define RAW_MAX (536870911.0 / 1048576.0)

static inline double exp_rom(double n)
{
    int i = (int)n + 19;
    if (i < 0) return 0.0;
    if (i >= 26) return RAW_MAX;
    uint32_t w = EXP_ROM[i];
    int s = (w >> 29) & 3;
    double v = (double)(w & 0x1FFFFFFF) / ldexp(1.0, 32 - 4 * s);
    return w >> 31 ? -v : v;
}

/* Python float %. Fast paths for m > 0 and -m <= x < 2m (ph_wrap / ph_tri of a phase: nearly every call), with the
 * bits of the fmod path: fmod(x, m) = x for |x| < m and = x - m (exact, Sterbenz) for m <= x < 2m; a negative x
 * gets + m rounded as below, a zero result is +0. test_core.py test_pymod compares the two paths. */
INL double pymod(double x, double m)
{
    if (m > 0) {
        if (x > 0 && x < m) return x;
        if (x >= m && x < 2 * m) return x - m;
        if (x < 0 && x >= -m) {
            double r = x + m;
            return r == 0 ? 0.0 : r;
        }
    }
    return pymod_ref(x, m);
}

static inline double pymod_ref(double x, double m)
{
    double r = fmod(x, m);
    if (r != 0) {
        if ((m < 0) != (r < 0)) r += m;
    } else
        r = copysign(0.0, m);
    return r;
}

INL double recip_seed(double m)
{
    int k = (int)((m - 0.5) * 128);
    if (k > 63) k = 63;
    return floor(4096 / (0.5 + (double)(2 * k + 1) / 256) + 0.5) / 4096;
}

/* the sqrt split's seed ROM: ~1/sqrt(m) for m in [1, 4), 48 bins of 1/16, 9 bits = round(512 / sqrt(bin middle))
 * (384 raw cases on the master, 2026-10-29; the 8-bit table read before was half of it, one bit short) */
static const int RSQRT_Q[48] = {504, 490, 476, 464, 452, 442, 432, 422, 414, 406, 398, 391, 384, 377, 371, 365, 359,
                                354, 349, 344, 339, 334, 330, 326, 322, 318, 314, 311, 307, 304, 300, 297, 294, 291,
                                288, 285, 283, 280, 277, 275, 272, 270, 268, 266, 263, 261, 259, 257};

INL double rsqrt_seed(double m)
{
    int k = (int)((m - 1) * 16);
    if (k > 47) k = 47;
    return RSQRT_Q[k] / 512.0;
}

INL double pymin(double x, double y) { return y < x ? y : x; }
INL double pymax(double x, double y) { return y > x ? y : x; }

/* ------------------------------------------------------------------ memory (chain.py Slave._access) */
INL int64_t ring(const Dsp *d, int64_t a) { return (a - d->t) & (RING - 1); }   /* d->t = s->t outside sl_step */

INL int64_t flat_key(int64_t a) { return a >= 0xA000 && a < 0xC000 ? (0xA000 | (a & 0x3FF)) : a; }

INL double rd(Slave *s, Dsp *d, int64_t a)
{
    if (s->flat) {
        if (s->access) return s->access('r', a, 0.0);
        a = flat_key(a);
        return a >= 0 && a < 0x10000 ? s->flat[a] : 0.0;
    }
    if (a < 0) return 0.0;
    int64_t bank = a >> 13, off = a & 0x1FFF;
    switch (bank) {
    case 0: return d->bank0[ring(d, off & 0x7FF)];
    case 1: return d->bank1[s->ring1 ? ring(d, off & 0x7FF) : (off & 0x7FF)];
    case 2: return d->bank2[d->t & 1][off];
    case 3: return s->iomd[off];
    case 4:
        if (d->mxm_in_st[off] == d->in_gen) return d->mxm_in[off];
        if (a >= 0x8400 && a < 0x8600 && d->mxm_out_st[off] == d->out_gen) return d->mxm_out[off];
        return 0.0;
    case 5: {
        int i = off & 0x7FF;
        if (i < 0x400) return d->bank5[i];
        i -= 0x400;
        if (d->own_hi_st[i] == s->t + 1) return d->own_hi[i];
        if (!s->dp_pairs) return 0.0;
        int pair = d->n >> 1;
        if (!s->dp_delay) return s->dp[pair][0][i];
        int b = (int)((s->t - 1) & 1);
        return s->dp_st[pair][b][i] == s->t ? s->dp[pair][b][i] : 0.0;
    }
    case 6: return d->params[off];
    default: return 0.0;
    }
}

/* a pair-window write reaches the pair's buffer at the end of the sample (dspcore.c merge_dp), in DSP order, as
 * chain.py's dp_next; with dp_delay 0 it is immediate (and the DSPs run on one thread) */
INL void wr(Slave *s, Dsp *d, int64_t a, double v)
{
    if (s->flat) {
        if (s->access) { s->access('w', a, v); return; }
        a = flat_key(a);
        if (a >= 0 && a < 0x10000) s->flat[a] = v;
        return;
    }
    if (a < 0) return;
    int64_t bank = a >> 13, off = a & 0x1FFF;
    switch (bank) {
    case 0: d->bank0[ring(d, off & 0x7FF)] = v; break;
    case 1: d->bank1[s->ring1 ? ring(d, off & 0x7FF) : (off & 0x7FF)] = v; break;
    case 2: d->bank2[d->t & 1][off] = v; break;
    case 3: s->iomd[off] = v; break;
    case 4:
        if (d->mxm_out_st[off] != d->out_gen) {
            d->mxm_out_st[off] = d->out_gen;
            d->out_list[d->nout++] = (int)off;
        }
        d->mxm_out[off] = v;
        break;
    case 5: {
        int i = off & 0x7FF;
        if (i < 0x400) { d->bank5[i] = v; break; }
        i -= 0x400;
        if (d->own_hi_st[i] != s->t + 1) {
            d->own_hi_st[i] = s->t + 1;
            d->hi_list[d->nhi++] = i;
        }
        d->own_hi[i] = v;
        if (s->dp_pairs && !s->dp_delay) s->dp[d->n >> 1][0][i] = v;
        break;
    }
    default: break;
    }
}

/* ------------------------------------------------------------------ units (a_op / b_op / sfu / mac) */
/* each returns nonzero where the op cannot run; immY / immC / immW = the unit's immediates (dspdec.h) */
INL int a_op(int w, double immY, Dsp *dsp, const double *regs, double *out, Res *res)
{
    int d = w >> 12, op = (w >> 9) & 7, x = (w >> 6) & 7, mode = (w >> 4) & 3, y = w & 15;
    double Y = mode ? immY : (y & 8 ? -regs[y & 7] : regs[y & 7]);
    double D = regs[d & 7], X = regs[x];
    if (d >= 8) {
        out[d - 8] = X + Y + regs[op];
        res->set = 1, res->v = out[d - 8], res->z = out[d - 8] == 0;
        return 0;
    }
    if (op == 0) {
        if (x == 1) out[d] = Y;
        else if (x == 2) {
            if (d == 7) return -1;                 /* newton with r7: no r8 */
            out[d] = D * regs[d + 1];
            out[d + 1] = 2 - regs[d + 1] * regs[y & 7];
            out[y & 7] = regs[d + 1] * regs[y & 7];
        } else if (x == 6 || x == 7) out[d] = D + Y;
        else if (x == 4 || x == 5) out[d] = D + Y + dsp->hidden[x];
        return 0;
    }
    double r = 0;
    switch (op) {
    case 1: r = D + X + Y; break;
    case 2: r = X * Y; break;
    case 3: r = D + X * Y; break;
    case 4: r = Y + X * D; break;
    case 5: r = Y - X * D; break;
    case 6: r = X + Y; break;
    case 7: r = Y - X; break;
    }
    out[d] = r;
    if (fabs(r) >= 512) {
        out[d] = copysign(fmod(fabs(r), 512), r);
        res->set = 1, res->v = r, res->z = out[d] == 0;
    }
    return 0;
}

INL void sfu(int w, const double *regs, double *out)
{
    int d = (w >> 12) - 8, mode = (w >> 4) & 3, y = w & 15;
    double Y = regs[y & 7];
    if (mode == 0 && (y & 8)) {
        double X = Y;
        if (X + 1) {
            int e;
            double m = frexp((X + 1) / 2, &e);
            double k = ldexp(1.0, -e);
            out[d] = (X - 1) / 2 * k;
            out[y & 7] = m;
            out[(d + 1) & 7] = recip_seed(m);
        }
    } else if (mode == 0) {
        if (Y) {
            int e;
            double m = frexp(fabs(Y), &e);
            out[d] = e > -9 ? copysign(ldexp(1.0, -e), Y) : 0.0;   /* 2^-e >= 512 gives 0 */
            out[(d + 1) & 7] = recip_seed(m);
            out[y & 7] = m;                                        /* and |Y|'s mantissa */
        }
    } else if (mode == 1 && !(y & 8)) {
        if (Y) {
            double m = fabs(Y);
            int k = 0;
            while (m >= 4) m /= 4, k--;
            while (m < 1) m *= 4, k++;
            out[y & 7] = m;
            out[(d + 1) & 7] = rsqrt_seed(m);
            out[(d + 3) & 7] = copysign(ldexp(1.0, -k), Y);       /* r(d+3), measured 2026-10-29 (not r7) */
        }
    } else if (mode == 2 && !(y & 8)) {
        if (Y > 0) {
            double e = floor(log2(Y / 0.75));
            out[d] = e;
            out[y & 7] = Y / ldexp(1.0, (int)e);
        } else if (Y < 0) {                    /* Y read in two's complement: e from Y + 512; m = Y / 2^e rounded toward
                                                * -inf at 2^-32, then truncated by its RAW encoding (120/120 raw cases on
                                                * the master, 2026-10-29; was e = 9, m = Y / 512: right for |Y| < 115) */
            double e = floor(log2((Y + 512) / 0.75));
            out[d] = e;
            out[y & 7] = floor(Y * ldexp(1.0, 32 - (int)e)) * 0x1p-32;
        }
    } else if (mode == 1) {
        double fl = floor(Y);
        out[d] = exp_rom(fl);
        out[y & 7] = Y - fl;
    } else if (mode == 2) {
        out[d] = Y ? copysign(1.0, Y) : 0.0;
    } else if (y & 8) {
        out[d] = floor(Y);                                         /* floor (was read as -(Y < 0)) */
    } else {
        out[d] = ceil(Y);
    }
}

INL double wrap1(double v) { return pymod(v + 1, 2) - 1; }

INL void b_op(int w, double immY, Dsp *dsp, const double *regs, double *out, Res *res)
{
    int d = w >> 12, b = (w >> 8) & 15, x = (w >> 6) & 7, mode = (w >> 4) & 3, y = w & 15;
    if (d >= 8 && b == 0xF) {
        int b76 = (w >> 6) & 3;
        if (b76 == 0) { sfu(w, regs, out); return; }
        if (b76 == 1) {
            int sub = (w >> 4) & 3, yy = w & 15;
            if (sub == 0) out[d & 7] = regs[yy & 7];
            else if (sub == 2 && yy >= 12) out[d & 7] = pymod(regs[d & 7] + dsp->hidden[yy - 8], 512);
            else if (sub == 1 && yy != 12) out[d & 7] = 0.0;
            else if (sub == 1) out[d & 7] = fmod((double)dsp->t, 512.0 * 16384) / 16384;   /* f5c: write position */
        }
        return;
    }
    double Y = mode ? immY : (y & 8 ? -regs[y & 7] : regs[y & 7]);
    double X = regs[x];
    if (d >= 8 && b == 0xE) {
        double D = regs[d & 7];
        int sub = (w >> 6) & 3;
        if (sub == 0) { res->set = 1, res->v = D + Y, res->z = D + Y == 0; }
        else if (sub == 1) out[d & 7] = Y;
        else if (sub == 3) {
            double R = regs[y & 7], v = 0;
            int neg = y & 8;
            switch (mode) {
            case 0: v = neg ? D - R : D + R; break;
            case 1: v = neg ? pymax(-1.0, pymin(1.0, R)) : fabs(R); break;
            case 2: v = neg ? 1 - 2 * fabs(wrap1(-R - 1.5)) : wrap1(R); break;
            case 3: v = neg ? -R : pymod(R, 2) / 2; break;
            }
            out[d & 7] = v;
        }
        return;
    }
    int k0 = d >> 3, k1 = b & 0xE;
    if (k0 == 0 && k1 <= 0xA) {
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
        out[d & 7] = pymod((double)r / Q, 512);
        return;
    }
    if (k0 == 1 && k1 == 0xC) {
        if (mode) {
            int sh = mode == 1 ? y : mode == 2 ? -y : -16 - y;
            /* two's complement with 32 fraction bits: the shift rounds toward -inf at 2^-32, wraps in [-512, 512);
             * the result's RAW encoding then truncates toward zero (quant in exec_insn). 672/672 raw cases on the
             * master, 2026-10-29 (was quant_floor: right only when the result is on the 2^-32 grid) */
            out[d & 7] = pymod(floor(X * ldexp(1.0, sh + 32)) * 0x1p-32 + 512, 1024) - 512;
        } else {
            int e = -31;
            double m = 0.0;
            if (X) m = frexp(X, &e);
            out[d & 7] = m;
            out[y & 7] = (double)e;
        }
        return;
    }
    double v;
    if (k0 == 0 && k1 == 0xC) v = fabs(X) + Y;
    else if (k0 == 0 && k1 == 0xE) v = Y - fabs(X);
    else if (k0 == 1 && k1 == 0x0) v = pymin(X, Y);
    else if (k0 == 1 && k1 == 0x4) v = pymax(X, Y);
    else if (k0 == 1 && k1 == 0x2) v = pymin(-X, Y);
    else if (k0 == 1 && k1 == 0x6) v = pymax(-X, Y);
    else if (k0 == 1 && k1 == 0x8) v = fabs(X);
    else if (k0 == 1 && k1 == 0xA) v = fabs(Y);
    else if (k0 == 1 && k1 == 0xE) v = Y;
    else return;
    out[d & 7] = v;
    if (k0 == 1 && k1 <= 6) {
        res->set = 1, res->v = v, res->z = (k1 & 2 ? -X : X) == Y;
    }
}

/* a RAW-grid value (a multiple of 2^-32 below 2^20) as an integer in units of 2^-32; 0 if it is not one */
INL int grid32(double v, int64_t *n)
{
    double t = v * 4294967296.0;
    if (!(fabs(v) < 1048576.0) || t != floor(t)) return 0;
    *n = (int64_t)t;
    return 1;
}

/* the MAC exactly (2026-10-29): the hardware sums the exact products and truncates once (§3); in doubles two
 * 29-bit mantissas make 58 bits and a rounding could cross a grid point (1 LSB in ~1/1000 cases, re/dspnative.py
 * mac_n1p2 / Roland instructions with a MAC). Operands on the RAW grid -> 128-bit integers in units of 2^-64;
 * the result truncated toward 0 on its RAW grid. 0: not all operands on the grid (the double path) */
INL int mac_exact(double X, double Y, double Z, double C, double W, int op1, int half, double *res)
{
    int64_t x, y, z, c, w;
    if (!grid32(X, &x) || !grid32(Y, &y) || !grid32(Z, &z) || !grid32(C, &c) || !grid32(W, &w)) return 0;
    __int128 p1 = (__int128)x * y, r = op1 & 1 ? -p1 : p1;           /* units of 2^-64 */
    if (op1 & 2) r += (__int128)w << 32;
    if (op1 & 4) r += (__int128)z * c;
    int neg = r < 0;
    unsigned __int128 m = neg ? -(unsigned __int128)r : (unsigned __int128)r;
    int sh = half ? 65 : 64;                        /* value = m 2^-sh */
    for (int s = 0; s < 4; s++) {                   /* the smallest s whose truncated mantissa fits 29 bits */
        int q = sh - (32 - 4 * s);                  /* drop q bits: units of 2^-(32 - 4 s) */
        unsigned __int128 M = m >> q;
        if (M < ((unsigned __int128)1 << 29)) {
            double v = ldexp((double)(uint64_t)M, -(32 - 4 * s));
            *res = neg ? -v : v;
            return 1;
        }
    }
    double v = ldexp((double)m, -sh);               /* |v| >= 512: as the double path (kept, no grid) */
    *res = neg ? -v : v;
    return 1;
}

INL void mac(int w1, int w2, double immY, double immC, double immW, const double *regs, double *out)
{
    int d = w1 >> 12, op1 = (w1 >> 9) & 7, x = (w1 >> 6) & 7, ym = (w1 >> 4) & 3, y = w1 & 15;
    int z = w2 >> 12, op2 = (w2 >> 9) & 7, c = (w2 >> 6) & 7, wm = (w2 >> 4) & 3, wf = w2 & 15;
    if (op1 < 2 || (d >= 8 && op1 < 4)) return;    /* /2 only with a P2 term; op2 = 011 = imm16 at byte C + 8 (2026-10-29) */
    double Y = ym ? immY : regs[y & 7];
    double C = (op2 >> 1) ? immC : regs[c];
    double W = wm ? immW : (wf & 8 ? -regs[wf & 7] : regs[wf & 7]);
    double Z = z & 8 ? -regs[z & 7] : regs[z & 7];
    double X = regs[x], one = 1.0;
    const double *f1 = &Y, *f2 = &C;               /* P1 = X f1, P2 = Z f2 */
    if (op2 == 0 && op1 >= 6) {                     /* sum mode: P1 = X (or X Y with an immediate Y), P2 = Z */
        if (!ym) f1 = &one;
        f2 = &one;
    }
    double r;
    if (!mac_exact(X, *f1, Z, *f2, W, op1, d >= 8, &r)) {
        double P1 = *f1 * X, P2 = *f2 * Z;
        r = (op1 & 1 ? -P1 : P1) + (op1 & 2 ? W : 0.0) + (op1 & 4 ? P2 : 0.0);
        if (d >= 8) r /= 2;
    }
    out[d & 7] = r;
}

/* a delay-line tap request (Xa9Y) with the address A: two neighbouring samples and the fraction to [h5 .. h5 + 2] */
INL void tap(Slave *s, Dsp *d, double A)
{
    double x0 = 0, x1 = 0, fr = 0;
    if (s->edm) {
        double k = (A - s->edm_base) * 16384.0, kf = floor(k);
        fr = floor((k - kf) * 64) / 64;
        int64_t D = (int64_t)kf + s->edm_lat, m = d->t - D;        /* x[m]: written at the end of sample m */
        if (kf >= 0 && D < s->edm_len && m >= 0) x0 = s->edm[m % s->edm_len];
        if (kf >= 0 && D + 1 < s->edm_len && m - 1 >= 0) x1 = s->edm[(m - 1) % s->edm_len];
    }
    int64_t dest = (int64_t)(d->hidden[5] * 16384) & 0x1FFF;
    wr(s, d, 0x4000 + dest, x0);
    wr(s, d, 0x4000 + ((dest + 1) & 0x1FFF), x1);
    wr(s, d, 0x4000 + ((dest + 2) & 0x1FFF), fr);
}

/* flags of a unit result for X9bY: !Z, !N, !V (|v| >= 512), N or Z (bit 0 first) */
INL uint32_t flag_bits(double v, int z)
{
    int n = v < 0, ov = fabs(v) >= 512;
    return (uint32_t)(!z) | (uint32_t)(!n) << 1 | (uint32_t)(!ov) << 2 | (uint32_t)(n || z) << 3;
}

#endif
