/* Shared by the interpreter (dspcore.c) and the native translations of dspjit.py: the slave state of chain.py and
 * the instruction semantics of re/dspisa.py, as inline functions. A translated program calls the same functions
 * with constant words, so the two paths cannot disagree on what an instruction does.
 *
 * Every arithmetic step follows dspisa.py literally, including Python's float modulo and min/max argument order.
 * Build with -ffp-contract=off: a fused multiply-add would change the bits (dspisa rounds every product).
 */
#ifndef DSPCORE_H
#define DSPCORE_H
#include <math.h>
#include <stdint.h>
#include <string.h>

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
    double bank5[0x400];
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
    int group[NDSP][NDSP];        /* thread groups: DSPs in execution order; group 0 runs on the calling thread */
    int ngroup[NDSP], ngroups;
    char err[256];
};

typedef struct { int set; double v; int z; } Res;

/* ------------------------------------------------------------------ numbers (dspisa.py) */
/* dspisa.quant: the smallest s whose floor(m 2^(32-4s)) < 2^29, i.e. m < 2^(4s-3), truncates m to a multiple of
 * 2^-(32-4s); m >= 512 (and inf, nan) is kept. s follows from the binary exponent E of m (m in [2^E, 2^(E+1))):
 * s = clamp((E + 7) >> 2, 0, 3). Without branches: the magnitude decides s, and a branch on it is mispredicted
 * most of the time (it was 90 % of the run time). Scaling by a power of two is exact and m 2^(32-4s) < 2^29, so the
 * integer conversion is the floor of dspisa. quant(quant(x)) = quant(x). quant_ref is the plain form (test_core.py
 * compares the two). */
static const double QSCALE[4] = {4294967296.0, 268435456.0, 16777216.0, 1048576.0};
static const double QINV[4] = {1.0 / 4294967296.0, 1.0 / 268435456.0, 1.0 / 16777216.0, 1.0 / 1048576.0};

INL double quant(double v)
{
    uint64_t b;
    memcpy(&b, &v, sizeof b);
    int e = (int)((b >> 52) & 0x7FF) - 1023;
    int s = (e + 7) >> 2;
    s = s < 0 ? 0 : s > 3 ? 3 : s;
    double m = e >= 9 ? 0.0 : fabs(v);
    double q = copysign((double)(int64_t)(m * QSCALE[s]) * QINV[s], v);
    return e >= 9 ? v : q;
}

static inline double quant_ref(double v)
{
    double m = fabs(v);
    if (v == 0 || m >= 512.0 || m != m) return v;
    if (m < 0.125) return copysign(floor(m * 4294967296.0) * (1.0 / 4294967296.0), v);
    if (m < 2.0) return copysign(floor(m * 268435456.0) * (1.0 / 268435456.0), v);
    if (m < 32.0) return copysign(floor(m * 16777216.0) * (1.0 / 16777216.0), v);
    return copysign(floor(m * 1048576.0) * (1.0 / 1048576.0), v);
}

INL double pymod(double x, double m)      /* Python float % */
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

static const int RSQRT_Q[48] = {252, 245, 238, 232, 226, 221, 216, 211, 207, 203, 199, 195, 192, 188, 185, 182, 179,
                                177, 174, 172, 169, 167, 165, 163, 161, 159, 157, 155, 153, 152, 150, 148, 147, 145,
                                144, 142, 141, 140, 138, 137, 136, 135, 134, 133, 131, 130, 129, 128};

INL double rsqrt_seed(double m)
{
    int k = (int)((m - 1) * 16);
    if (k > 47) k = 47;
    return RSQRT_Q[k] / 256.0;
}

INL double pymin(double x, double y) { return y < x ? y : x; }
INL double pymax(double x, double y) { return y > x ? y : x; }

/* ------------------------------------------------------------------ memory (chain.py Slave._access) */
INL int64_t ring(const Slave *s, int64_t a) { return (a - s->t) & (RING - 1); }

INL double rd(Slave *s, Dsp *d, int64_t a)
{
    if (a < 0) return 0.0;
    int64_t bank = a >> 13, off = a & 0x1FFF;
    switch (bank) {
    case 0: return d->bank0[ring(s, off & 0x7FF)];
    case 1: return d->bank1[s->ring1 ? ring(s, off & 0x7FF) : (off & 0x7FF)];
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
    case 6: return s->params[off];
    default: return 0.0;
    }
}

/* a pair-window write reaches the pair's buffer at the end of the sample (dspcore.c merge_dp), in DSP order, as
 * chain.py's dp_next; with dp_delay 0 it is immediate (and the DSPs run on one thread) */
INL void wr(Slave *s, Dsp *d, int64_t a, double v)
{
    if (a < 0) return;
    int64_t bank = a >> 13, off = a & 0x1FFF;
    switch (bank) {
    case 0: d->bank0[ring(s, off & 0x7FF)] = v; break;
    case 1: d->bank1[s->ring1 ? ring(s, off & 0x7FF) : (off & 0x7FF)] = v; break;
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

/* ------------------------------------------------------------------ units (dspisa.py a_op / b_op / sfu / mac) */
/* each returns nonzero where dspisa raises; immY / immC / immW = the unit's immediates (dspcomp.py) */
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
            if (d == 7) return -1;                 /* dspisa: IndexError */
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
            out[d] = copysign(ldexp(1.0, -e), Y);
            out[(d + 1) & 7] = recip_seed(m);
        }
    } else if (mode == 1 && !(y & 8)) {
        if (Y) {
            double m = fabs(Y);
            int k = 0;
            while (m >= 4) m /= 4, k--;
            while (m < 1) m *= 4, k++;
            out[y & 7] = m;
            out[(d + 1) & 7] = rsqrt_seed(m);
            out[7] = copysign(ldexp(1.0, -k), Y);
        }
    } else if (mode == 2 && !(y & 8)) {
        if (Y > 0) {
            double e = floor(log2(Y / 0.75));
            out[d] = e;
            out[y & 7] = Y / ldexp(1.0, (int)e);
        } else if (Y < 0) {
            out[d] = 9.0;
            out[y & 7] = Y / 512;
        }
    } else if (mode == 1) {
        double fl = floor(Y);
        out[d] = exp(fl);
        out[y & 7] = Y - fl;
    } else if (mode == 2) {
        out[d] = Y ? copysign(1.0, Y) : 0.0;
    } else if (y & 8) {
        out[d] = Y < 0 ? -1.0 : 0.0;
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
            out[d & 7] = X * ldexp(1.0, sh);
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

INL void mac(int w1, int w2, double immY, double immC, double immW, const double *regs, double *out)
{
    int d = w1 >> 12, op1 = (w1 >> 9) & 7, x = (w1 >> 6) & 7, ym = (w1 >> 4) & 3, y = w1 & 15;
    int z = w2 >> 12, op2 = (w2 >> 9) & 7, c = (w2 >> 6) & 7, wm = (w2 >> 4) & 3, wf = w2 & 15;
    if (op1 < 2 || op2 == 3) return;
    double Y = ym ? immY : regs[y & 7];
    double C = (op2 >> 1) ? immC : regs[c];
    double W = wm ? immW : (wf & 8 ? -regs[wf & 7] : regs[wf & 7]);
    double Z = z & 8 ? -regs[z & 7] : regs[z & 7];
    double P1 = Y * regs[x], P2 = C * Z;
    if (op2 == 0 && op1 >= 6) {
        if (!ym) P1 = regs[x];
        P2 = Z;
    }
    double r = (op1 & 1 ? -P1 : P1) + (op1 & 2 ? W : 0.0) + (op1 & 4 ? P2 : 0.0);
    out[d & 7] = d >= 8 ? r / 2 : r;
}

/* flags of a unit result for X9bY: !Z, !N, !V (|v| >= 512), N or Z (bit 0 first) */
INL uint32_t flag_bits(double v, int z)
{
    int n = v < 0, ov = fabs(v) >= 512;
    return (uint32_t)(!z) | (uint32_t)(!n) << 1 | (uint32_t)(!ov) << 2 | (uint32_t)(n || z) << 3;
}

#endif
