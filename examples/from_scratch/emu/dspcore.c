/* C core of the DSP emulator and the reference of the DSP's semantics (docs/EMULATOR.md; the Python model
 * re/dspisa.py and chain.py Slave were archived 2026-10-29).
 * dspdec.h decodes every instruction once (dspcomp.py calls it); this file interprets the decoded form, or calls the
 * native translation of a program (dspjit.py) when one is attached. Both use the inline functions of dspcore.h.
 *
 * Inside one sample the DSPs run in groups (sl_set_groups): one thread per group, the DSPs of a group in model
 * order. A group must hold every DSP that shares an IOMD word with another (dspjit.groups decides it from the
 * programs); MXM and the pair window are latched, so the result does not depend on the threads.
 *
 *     cc -O2 -ffp-contract=off -shared -fPIC -pthread -o dspcore.so dspcore.c -lm   (cchain.py does this)
 */
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

#include "dspcore.h"

#include "dspdec.h"

/* ------------------------------------------------------------------ one instruction */
static int exec_insn(Slave *s, Dsp *d, const Prog *p, int k, int *jump)
{
    const Insn *in = &p->insn[k];
    if (in->err) return -2;
    uint64_t skip = 0;
    for (int i = 0; i < in->ncond; i++) {
        const Cond *c = &p->conds[in->cond0 + i];
        int cond = (int)((d->flags >> c->cc) & 1) != c->neg;
        int o = cond ? c->op_t : c->op_f;
        if (o >= 0) skip |= 1ull << o;
    }
    double regs[8], out[8];
    memcpy(regs, d->regs, sizeof regs);
    memcpy(out, regs, sizeof out);
    uint64_t done = 0;
    double rv[64];
    int rz[64];
    uint32_t um = 0, uv = 0;
    for (int i = 0; i < in->nop; i++) {
        if (skip >> i & 1) continue;
        const Op *o = &p->ops[in->op0 + i];
        Res res = {0, 0, 0};
        switch (o->kind) {
        case K_A:
            if (a_op(o->w, o->imm[0], d, regs, out, &res)) return -1;
            break;
        case K_B: b_op(o->w, o->imm[0], d, regs, out, &res); break;
        case K_M: mac(o->w, o->w2, o->imm[0], o->imm[1], o->imm[2], regs, out); break;
        case K_J: *jump = o->a; break;
        case K_HID:
            if (o->b == 0) d->hidden[o->a] = pymod(regs[o->c & 7], 512);
            else d->hidden[o->a] = o->imm[0];
            break;
        case K_X28: {
            int64_t ia = ((int64_t)(d->hidden[6] * 16384) + o->b) & 0x7FF;
            if (o->a < 8) out[o->a] = rd(s, d, ia);
            else wr(s, d, ia, regs[o->a - 8]);
            break;
        }
        case K_XB5: {
            int64_t ia = o->c ? (int64_t)(d->hidden[o->c] * 16384) + o->b : o->b;
            ia += 0xA000;
            if (o->a < 8) out[o->a] = rd(s, d, ia);
            else wr(s, d, ia, regs[o->a - 8]);
            break;
        }
        case K_XFLAG:
            if (o->a && o->b < 64 && (done >> o->b & 1)) {
                uint32_t sh = 4 * (uint32_t)o->a;
                um |= 0xFu << sh;
                uv = (uv & ~(0xFu << sh)) | flag_bits(rv[o->b], rz[o->b]) << sh;
            }
            break;
        case K_XMEM:
            if (o->a < 8) out[o->a] = rd(s, d, o->b);
            else wr(s, d, o->b, regs[o->a - 8]);
            break;
        case K_XTAP: tap(s, d, regs[o->a & 7]); break;
        }
        if (o->kind <= K_M) {
            for (int r = 0; r < 8; r++) out[r] = quant(out[r]);
            if (!res.set) {
                int dest = (o->w >> 12) & 7;
                res.v = out[dest], res.z = out[dest] == 0;
            }
            rv[o->unit] = res.v, rz[o->unit] = res.z;
            done |= 1ull << o->unit;
        }
    }
    d->flags = (d->flags & ~d->pmask) | d->pval;
    d->pmask = um, d->pval = uv;
    memcpy(d->regs, out, sizeof out);
    return 0;
}

static int run_prog(Slave *s, Dsp *d, int pid)
{
    const Prog *p = &s->progs[pid];
    if (p->native) {
        int e = p->native(s, d);
        if (e) snprintf(d->err, sizeof d->err, "native program %d (DSP%d): error %d", pid, d->n, e);
        return e;
    }
    int i = 0, steps = 0;
    while (i < p->ninsn) {
        if (++steps > MAXSTEPS) {
            snprintf(d->err, sizeof d->err, "endless loop in program %d (DSP%d)", pid, d->n);
            return -1;
        }
        int jump = -2;
        int e = exec_insn(s, d, p, i, &jump);
        if (e) {
            snprintf(d->err, sizeof d->err, "%s at instruction %d of program %d (DSP%d)",
                     e == -1 ? "newton with r7 (no register r8)" : "undecodable instruction", i, pid, d->n);
            return -1;
        }
        d->ninstr++;
        if (s->trace) s->trace(d->n, i, d->regs, d->flags, d->pmask, d->pval);
        if (jump == -1) {
            snprintf(d->err, sizeof d->err, "jump to no instruction start at %d of program %d", i, pid);
            return -1;
        }
        i = jump >= 0 ? jump : i + 1;
    }
    return 0;
}

static int run_group(Slave *s, int g)
{
    for (int j = 0; j < s->ngroup[g]; j++) {
        Dsp *d = &s->d[s->group[g][j]];
        for (int q = 0; q < d->nprogs; q++)
            if (run_prog(s, d, d->progs[q])) return -1;
    }
    return 0;
}

/* ------------------------------------------------------------------ end of sample */
static void latch_into(Slave *s, int only)    /* chain.py Slave._latch */
{
    for (int r = 0; r < NDSP; r++)
        if (only < 0 || r == only) s->d[r].in_gen++;
    for (int w = 0; w < NDSP; w++) {
        Dsp *dw = &s->d[w];
        for (int k = 0; k < dw->nout; k++) {
            int off = dw->out_list[k];
            double v = dw->mxm_out[off];
            int z = off & 0x7FF;
            if (z >= 0x400 && z < 0x600) {
                int i = (z - 0x400 + s->mxm_shift) & 0x1FF;
                for (int r = 2 * (w / 2 + 1); r <= 2 * (w / 2 + 1) + 1; r++)
                    if (r < NDSP && (only < 0 || r == only)) {
                        int a = (w & 1 ? 0x200 : 0) + i;
                        s->d[r].mxm_in[a] = v;
                        s->d[r].mxm_in_st[a] = s->d[r].in_gen;
                    }
            } else if (z < 0x400) {
                int r = (w / 2 - 1) * 2 + (z >= 0x200 ? 1 : 0);
                if (r >= 0 && (only < 0 || r == only)) {
                    int a = 0x600 + (z & 0x1FF);
                    s->d[r].mxm_in[a] = v;
                    s->d[r].mxm_in_st[a] = s->d[r].in_gen;
                }
            }
        }
    }
}

static void merge_dp(Slave *s)                /* chain.py dp_next: the last write in DSP order wins */
{
    if (s->dp_pairs && s->dp_delay) {
        int b = (int)(s->t & 1);
        for (int j = 0; j < s->norder; j++) {
            Dsp *d = &s->d[s->order[j]];
            int pair = d->n >> 1;
            for (int k = 0; k < d->nhi; k++) {
                int i = d->hi_list[k];
                s->dp[pair][b][i] = d->own_hi[i];
                s->dp_st[pair][b][i] = s->t + 1;
            }
        }
    }
    for (int n = 0; n < NDSP; n++) s->d[n].nhi = 0;
}

static void edm_write(Slave *s)                /* the DMA: the window word 0x300 of this sample into the EDM */
{
    if (!s->edm) return;
    if (s->flat) {
        s->edm[s->d[0].t % s->edm_len] = s->flat[0x4300];
        return;
    }
    if (s->edm_dsp < 0) return;
    Dsp *d = &s->d[s->edm_dsp];
    s->edm[d->t % s->edm_len] = d->bank2[d->t & 1][0x300];
}

static void end_sample(Slave *s)
{
    edm_write(s);
    if (s->mxm_delay) latch_into(s, -1);
    merge_dp(s);
    for (int n = 0; n < NDSP; n++) {
        s->d[n].out_gen++;
        s->d[n].nout = 0;
    }
    s->t++;
    for (int n = 0; n < NDSP; n++) s->d[n].t = s->t;
}

static int first_error(Slave *s)
{
    for (int n = 0; n < NDSP; n++)
        if (s->d[n].err[0]) {
            snprintf(s->err, sizeof s->err, "%s", s->d[n].err);
            return -1;
        }
    return 0;
}

/* ------------------------------------------------------------------ threads: one per group 1.., spin barrier */
typedef struct {
    Slave *s;
    int g;
    atomic_int *go, *done, *stop;
} Worker;

static void spin_pause(int *k)
{
    if (++*k > 2000) {
        sched_yield();
        *k = 0;
    }
#if defined(__x86_64__) || defined(__i386__)
    __builtin_ia32_pause();
#endif
}

static void *worker(void *arg)
{
    Worker *w = arg;
    int seen = 0;
    for (;;) {
        int k = 0, g;
        while ((g = atomic_load_explicit(w->go, memory_order_acquire)) == seen && !atomic_load(w->stop))
            spin_pause(&k);
        if (atomic_load(w->stop)) break;
        seen = g;
        run_group(w->s, w->g);
        atomic_fetch_add_explicit(w->done, 1, memory_order_release);
    }
    return NULL;
}

/* ------------------------------------------------------------------ API */
Slave *sl_new(int ring1, int mxm_shift, int mxm_delay, int dp_pairs, int dp_delay, const int *order, int norder,
              const int *shared, int nshared, const int *shared5, int nshared5, const int *own_params, int nown)
{
    Slave *s = calloc(1, sizeof *s);
    s->ring1 = ring1, s->mxm_shift = mxm_shift, s->mxm_delay = mxm_delay;
    s->dp_pairs = dp_pairs, s->dp_delay = dp_delay;
    s->norder = norder;
    memcpy(s->order, order, norder * sizeof *order);
    for (int n = 0; n < NDSP; n++) {
        Dsp *d = &s->d[n];
        d->n = n;
        d->bank1 = d->bank1_own;
        d->bank5 = d->bank5_own;
        d->params = s->params;
        d->in_gen = 1, d->out_gen = 1;
    }
    for (int k = 0; k < nshared; k++) s->d[shared[2 * k + 1]].bank1 = s->d[shared[2 * k]].bank1;
    for (int k = 0; k < nshared5; k++) s->d[shared5[2 * k + 1]].bank5 = s->d[shared5[2 * k]].bank5;
    for (int k = 0; k < nown; k++) s->d[own_params[k]].params = s->d[own_params[k]].params_own;
    s->pipe_a = s->pipe_b = -1;
    s->ngroups = 1;                           /* default: everything on the calling thread, model order */
    s->ngroup[0] = norder;
    memcpy(s->group[0], order, norder * sizeof *order);
    return s;
}

/* the delay line (Slave.edm): DSP `dsp` has the window, base and latency as measured (324, 2), len EDM words */
void sl_set_edm(Slave *s, int dsp, double base, int lat, int64_t len)
{
    free(s->edm);
    s->edm = len > 0 ? calloc(len, sizeof *s->edm) : NULL;
    s->edm_len = len, s->edm_base = base, s->edm_lat = lat, s->edm_dsp = dsp;
}

void sl_free(Slave *s)
{
    free(s->edm);
    free(s->flat);
    for (int k = 0; k < s->nprogs; k++) {
        free(s->progs[k].insn);
        free(s->progs[k].ops);
        free(s->progs[k].conds);
    }
    free(s->progs);
    free(s);
}

int sl_add_prog(Slave *s, int ninsn, const Insn *insn, int nops, const Op *ops, int nconds, const Cond *conds)
{
    s->progs = realloc(s->progs, (s->nprogs + 1) * sizeof *s->progs);
    Prog *p = &s->progs[s->nprogs];
    p->ninsn = ninsn;
    p->native = NULL;
    p->insn = malloc((ninsn + 1) * sizeof *insn);
    p->ops = malloc((nops + 1) * sizeof *ops);
    p->conds = malloc((nconds + 1) * sizeof *conds);
    memcpy(p->insn, insn, ninsn * sizeof *insn);
    memcpy(p->ops, ops, nops * sizeof *ops);
    memcpy(p->conds, conds, nconds * sizeof *conds);
    return s->nprogs++;
}

void sl_set_native(Slave *s, int pid, native_fn f) { s->progs[pid].native = f; }

void sl_set_trace(Slave *s, void (*f)(int, int, const double *, uint32_t, uint32_t, uint32_t)) { s->trace = f; }

void sl_set_progs(Slave *s, int dsp, const int *ids, int n)
{
    s->d[dsp].nprogs = n;
    memcpy(s->d[dsp].progs, ids, n * sizeof *ids);
}

/* groups: flat list of DSP numbers, sizes[g] each; every DSP of the model order exactly once, in model order inside
 * a group. Returns -1 (and keeps one group) if the model needs one thread. */
int sl_set_groups(Slave *s, const int *dsps, const int *sizes, int ngroups)
{
    if (!s->mxm_delay || (s->dp_pairs && !s->dp_delay)) return -1;
    s->ngroups = ngroups;
    for (int g = 0, k = 0; g < ngroups; g++) {
        s->ngroup[g] = sizes[g];
        for (int j = 0; j < sizes[g]; j++) s->group[g][j] = dsps[k++];
    }
    return 0;
}

int sl_ngroups(Slave *s) { return s->ngroups; }

/* state of one DSP: regs[8], flags, pending mask and values (32 condition bits), hidden[8] */
void sl_get_state(Slave *s, int dsp, double *regs, uint32_t *flags, uint32_t *pmask, uint32_t *pval, double *hidden)
{
    Dsp *d = &s->d[dsp];
    memcpy(regs, d->regs, sizeof d->regs);
    *flags = d->flags, *pmask = d->pmask, *pval = d->pval;
    memcpy(hidden, d->hidden, sizeof d->hidden);
}

void sl_set_state(Slave *s, int dsp, const double *regs, uint32_t flags, uint32_t pmask, uint32_t pval,
                  const double *hidden)
{
    Dsp *d = &s->d[dsp];
    memcpy(d->regs, regs, sizeof d->regs);
    d->flags = flags, d->pmask = pmask, d->pval = pval;
    memcpy(d->hidden, hidden, sizeof d->hidden);
}

double *sl_iomd(Slave *s) { return s->iomd; }
double *sl_params(Slave *s) { return s->params; }
double *sl_params_dsp(Slave *s, int dsp) { return s->d[dsp].params; }
double *sl_bank5(Slave *s, int dsp) { return s->d[dsp].bank5; }
double *sl_bank0(Slave *s, int dsp) { return s->d[dsp].bank0; }
double *sl_bank1(Slave *s, int dsp) { return s->d[dsp].bank1; }
int64_t sl_t(Slave *s) { return s->t; }
const char *sl_error(Slave *s) { return s->err; }

int64_t sl_ninstr(Slave *s)
{
    int64_t n = 0;
    for (int k = 0; k < NDSP; k++) n += s->d[k].ninstr;
    return n;
}

int64_t sl_ninstr_dsp(Slave *s, int dsp) { return s->d[dsp].ninstr; }

double sl_quant(double v) { return quant(v); }

/* quant against quant_ref on n values of every binary exponent -1080..1030 and the edges of each range (random
 * mantissas from `seed`); returns the number of differences (bit patterns), the first one in *bad */
/* pymod (fast paths) against pymod_ref (fmod) for random x near the fast-path edges and m in {0.5, 1, 2, 512};
 * returns the number of differing results (bits), the first bad x in *bad */
int64_t sl_pymod_check(int64_t n, uint64_t seed, double *bad)
{
    static const double ms[4] = {0.5, 1.0, 2.0, 512.0};
    int64_t diff = 0;
    uint64_t x = seed | 1;
    for (int64_t k = 0; k < n; k++) {
        x ^= x << 13, x ^= x >> 7, x ^= x << 17;
        double m = ms[x & 3];
        double u = (double)(x >> 11) / 9007199254740992.0;          /* [0, 1) */
        double e = ldexp(1.0, -(int)((x >> 3) % 60));
        double cases[10] = {(u * 6 - 3) * m, u * e, -u * e, m, -m, 2 * m, -2 * m, nextafter(m, 0),
                            nextafter(-m, 0), nextafter(2 * m, 0)};
        for (int c = 0; c < 10; c++) {
            double a = pymod(cases[c], m), r = pymod_ref(cases[c], m);
            if (memcmp(&a, &r, sizeof a)) {
                if (!diff) *bad = cases[c];
                diff++;
            }
        }
    }
    double sp[] = {0.0, -0.0, 4.9e-324, -4.9e-324};
    for (unsigned c = 0; c < sizeof sp / sizeof *sp; c++)
        for (int j = 0; j < 4; j++) {
            double a = pymod(sp[c], ms[j]), r = pymod_ref(sp[c], ms[j]);
            if (memcmp(&a, &r, sizeof a)) {
                if (!diff) *bad = sp[c];
                diff++;
            }
        }
    return diff;
}

int64_t sl_quant_check(int64_t n, uint64_t seed, double *bad)
{
    int64_t diff = 0;
    uint64_t x = seed | 1;
    for (int64_t k = 0; k < n; k++) {
        x ^= x << 13, x ^= x >> 7, x ^= x << 17;
        int e = (int)(x % 2111) - 1080;
        double v = ldexp(1.0 + (double)(x >> 12) / 4503599627370496.0, e);
        double cases[6] = {v, -v, nextafter(v, 0), -nextafter(v, INFINITY), ldexp(1.0, e), -ldexp(1.0, e)};
        for (int c = 0; c < 6; c++) {
            double a = quant(cases[c]), r = quant_ref(cases[c]);
            if (memcmp(&a, &r, sizeof a) && !(a != a && r != r)) {
                if (!diff) *bad = cases[c];
                diff++;
            }
        }
    }
    double sp[] = {0.0, -0.0, INFINITY, -INFINITY, NAN, 512.0, -512.0, 0.125, 2.0, 32.0, 4.9e-324};
    for (unsigned c = 0; c < sizeof sp / sizeof *sp; c++) {
        double a = quant(sp[c]), r = quant_ref(sp[c]);
        if (memcmp(&a, &r, sizeof a) && !(a != a && r != r)) {
            if (!diff) *bad = sp[c];
            diff++;
        }
    }
    return diff;
}

static int mxm_list(Dsp *d, int in, int *addr, double *val, int max)
{
    int n = 0;
    if (!in) {
        for (int k = 0; k < d->nout && n < max; k++, n++) {
            addr[n] = 0x8000 + d->out_list[k];
            val[n] = d->mxm_out[d->out_list[k]];
        }
        return n;
    }
    for (int off = 0; off < 0x2000 && n < max; off++)
        if (d->mxm_in_st[off] == d->in_gen) {
            addr[n] = 0x8000 + off;
            val[n++] = d->mxm_in[off];
        }
    return n;
}

/* MXM writes of the running sample (before the latch) / latched inputs of one DSP; returns the count */
int sl_mxm_out(Slave *s, int dsp, int *addr, double *val, int max) { return mxm_list(&s->d[dsp], 0, addr, val, max); }
int sl_mxm_in(Slave *s, int dsp, int *addr, double *val, int max) { return mxm_list(&s->d[dsp], 1, addr, val, max); }

static int step_serial(Slave *s)
{
    for (int j = 0; j < s->norder; j++) {
        int dn = s->order[j];
        Dsp *d = &s->d[dn];
        if (!s->mxm_delay) latch_into(s, dn);
        for (int q = 0; q < d->nprogs; q++)
            if (run_prog(s, d, d->progs[q])) return first_error(s);
    }
    end_sample(s);
    return 0;
}

/* rec_addr: an IOMD word (0x6000..0x7FFF), or (dsp + 1) << 16 | address = that word as DSP dsp reads it (bank 5
 * of a DSP, for example) */
static void record(Slave *s, int64_t k, const int *rec_addr, int nrec, double *rec)
{
    for (int j = 0; j < nrec; j++) {
        int a = rec_addr[j];
        rec[k * nrec + j] = a >> 16 ? rd(s, &s->d[(a >> 16) - 1], a & 0xFFFF) : s->iomd[a & 0x1FFF];
    }
}

/* ------------------------------------------------------------------ pipeline: stage A one sample ahead */
/* Exact when (checked by cchain.py before sl_set_pipeline): stage A is one DSP that shares bank 5 with pipe_b only,
 * touches no IOMD word, MXM or pair window of the others, and reads no bank-5 word stage B writes; both stages use
 * static bank-5 addresses. Then stage B of sample t sees exactly the words A wrote in sample t. */
void sl_set_pipeline(Slave *s, int a, int b, const int *wa, int nwa, const int *wb, int nwb)
{
    s->pipe_a = a, s->pipe_b = b;
    s->npipe_wa = nwa, s->npipe_wb = nwb;
    memcpy(s->pipe_wa, wa, nwa * sizeof *wa);
    memcpy(s->pipe_wb, wb, nwb * sizeof *wb);
}

typedef struct {
    Slave *s;
    atomic_int *go, *done, *stop;
} PipeWorker;

static int run_dsp(Slave *s, int dn)
{
    Dsp *d = &s->d[dn];
    for (int q = 0; q < d->nprogs; q++)
        if (run_prog(s, d, d->progs[q])) return -1;
    return 0;
}

static void *pipe_worker(void *arg)
{
    PipeWorker *w = arg;
    int seen = 0;
    for (;;) {
        int k = 0, g;
        while ((g = atomic_load_explicit(w->go, memory_order_acquire)) == seen && !atomic_load(w->stop))
            spin_pause(&k);
        if (atomic_load(w->stop)) break;
        seen = g;
        run_dsp(w->s, w->s->pipe_a);
        atomic_store_explicit(w->done, 1, memory_order_release);
    }
    return NULL;
}

static void record_split(Slave *s, int64_t k, const int *rec_addr, int nrec, double *rec, int stage_a)
{
    for (int j = 0; j < nrec; j++) {
        int a = rec_addr[j], dsp = (a >> 16) - 1;
        if ((dsp == s->pipe_a) != stage_a) continue;
        rec[k * nrec + j] = a >> 16 ? rd(s, &s->d[dsp], a & 0xFFFF) : s->iomd[a & 0x1FFF];
    }
}

static int run_stage_b(Slave *s)
{
    for (int j = 0; j < s->norder; j++)
        if (s->order[j] != s->pipe_a && run_dsp(s, s->order[j])) return -1;
    return 0;
}

static int sl_step_pipe(Slave *s, int n, const int *rec_addr, int nrec, double *rec)
{
    Dsp *A = &s->d[s->pipe_a], *B = &s->d[s->pipe_b];
    double *shared = A->bank5;
    memcpy(s->pipe_buf, shared, sizeof s->pipe_buf);
    B->bank5 = s->pipe_buf;
    atomic_int go = 0, done = 0, stop = 0;
    PipeWorker wk = {s, &go, &done, &stop};
    pthread_t th;
    pthread_create(&th, NULL, pipe_worker, &wk);
    int err = 0;
    A->t = s->t;                                         /* prologue: stage A alone, sample t */
    if (run_dsp(s, s->pipe_a)) err = -1;
    record_split(s, 0, rec_addr, nrec, rec, 1);
    for (int k = 0; k < n && !err; k++) {
        for (int j = 0; j < s->npipe_wa; j++) s->pipe_buf[s->pipe_wa[j]] = shared[s->pipe_wa[j]];
        int ahead = k + 1 < n;
        if (ahead) {                                     /* stage A: sample t + 1 on the worker */
            A->t = s->t + 1;
            atomic_store_explicit(&done, 0, memory_order_relaxed);
            atomic_fetch_add_explicit(&go, 1, memory_order_release);
        }
        if (run_stage_b(s)) err = -1;                    /* stage B: sample t here */
        if (ahead) {
            int spins = 0;
            while (!atomic_load_explicit(&done, memory_order_acquire)) spin_pause(&spins);
        }
        if (err || first_error(s)) {
            err = -1;
            break;
        }
        record_split(s, k, rec_addr, nrec, rec, 0);
        if (ahead) record_split(s, k + 1, rec_addr, nrec, rec, 1);
        end_sample(s);                                   /* s->t + 1; every d->t = s->t (A's included) */
    }
    atomic_store(&stop, 1);
    pthread_join(th, NULL);
    for (int j = 0; j < s->npipe_wb; j++) shared[s->pipe_wb[j]] = s->pipe_buf[s->pipe_wb[j]];
    B->bank5 = shared;
    return err ? first_error(s), -1 : 0;
}

/* chain.py Slave.step, n times; with `rec` the IOMD words rec_addr[0..nrec) are stored after each sample. With more
 * than one group and n >= 64 the groups run on their own threads. */
int sl_step(Slave *s, int n, const int *rec_addr, int nrec, double *rec)
{
    if (s->pipe_a >= 0 && n >= 64) {
        static double dummy;
        return sl_step_pipe(s, n, rec_addr, nrec, rec ? rec : &dummy);
    }
    if (s->ngroups <= 1 || n < 64) {
        for (int k = 0; k < n; k++) {
            if (step_serial(s)) return -1;
            record(s, k, rec_addr, nrec, rec);
        }
        return 0;
    }
    atomic_int go = 0, done = 0, stop = 0;
    pthread_t th[NDSP];
    Worker wk[NDSP];
    for (int g = 1; g < s->ngroups; g++) {
        wk[g] = (Worker){s, g, &go, &done, &stop};
        pthread_create(&th[g], NULL, worker, &wk[g]);
    }
    int err = 0;
    for (int k = 0; k < n && !err; k++) {
        atomic_store_explicit(&done, 0, memory_order_relaxed);
        atomic_fetch_add_explicit(&go, 1, memory_order_release);
        run_group(s, 0);
        int spins = 0;
        while (atomic_load_explicit(&done, memory_order_acquire) != s->ngroups - 1) spin_pause(&spins);
        if (first_error(s)) {
            err = -1;
            break;
        }
        end_sample(s);
        record(s, k, rec_addr, nrec, rec);
    }
    atomic_store(&stop, 1);
    for (int g = 1; g < s->ngroups; g++) pthread_join(th[g], NULL);
    return err;
}

/* ------------------------------------------------------------------ machine: one DSP, flat memory (tools) */
/* the simulator the tools use (make_*.py, dspasm.py, dspnative.py, sim_main.py; dspref.Machine): DSP 0 of a Slave
 * in machine mode. Programs: dsp_machine_add (decoded once), dsp_machine_run; single instructions: dsp_exec1. */
Slave *dsp_machine_new(void)
{
    static const int order[1] = {0};
    Slave *s = sl_new(0, 0, 0, 0, 0, order, 1, NULL, 0, NULL, 0, NULL, 0);
    s->flat = calloc(0x10000, sizeof *s->flat);
    return s;
}

double *dsp_machine_mem(Slave *s) { return s->flat; }

void dsp_machine_set_access(Slave *s, double (*f)(int, int64_t, double)) { s->access = f; }

/* decode a program (instruction k = words[starts[k] .. starts[k + 1]), the last one to nwords) and add it; jumps
 * resolved to instruction indexes (-1: no instruction starts there). Returns the program id, or -1. Undecodable
 * instructions fail only if they run. */
int dsp_machine_add(Slave *s, const uint16_t *words, int nwords, const int *starts, int ninsn)
{
    Insn *insn = calloc(ninsn ? ninsn : 1, sizeof *insn);
    int cap = 64 * (ninsn ? ninsn : 1);
    Op *ops = calloc(cap, sizeof *ops);
    Cond *conds = calloc(cap, sizeof *conds);
    int32_t *joff = calloc(DMAX, sizeof *joff);
    int no = 0, nc = 0;
    for (int k = 0; k < ninsn; k++) {
        int end = k + 1 < ninsn ? starts[k + 1] : nwords, n = 0, m = 0;
        int e = dsp_decode(words + starts[k], end - starts[k], ops + no, DMAX, &n, conds + nc, DMAX, &m, joff);
        insn[k] = (Insn){no, e ? 0 : n, nc, e ? 0 : m, e ? 1 : 0};
        if (e) continue;
        for (int i = 0; i < n; i++)
            if (ops[no + i].kind == K_J) {
                int tgt = end + joff[i], idx = -1;
                for (int j = 0; j < ninsn; j++)
                    if (starts[j] == tgt) { idx = j; break; }
                ops[no + i].a = idx;
            }
        no += n, nc += m;
    }
    int pid = sl_add_prog(s, ninsn, insn, no, ops, nc, conds);
    free(insn), free(ops), free(conds), free(joff);
    return pid;
}

/* run program pid once on DSP 0; 0 or -1 (dsp error: sl_error) */
int dsp_machine_run(Slave *s, int pid)
{
    Dsp *d = &s->d[0];
    d->err[0] = 0;
    if (run_prog(s, d, pid)) {
        snprintf(s->err, sizeof s->err, "%s", d->err);
        return -1;
    }
    return 0;
}

/* one instruction on DSP 0: 0, -1 (newton with r7), -2 (undecodable); *jumped = 1 and *jump_off = the offset in
 * words from the instruction end when it has a jump that runs */
int dsp_exec1(Slave *s, const uint16_t *ins, int nins, int *jumped, int *jump_off)
{
    Op ops[DMAX];
    Cond conds[DMAX];
    int32_t joff[DMAX];
    int n = 0, m = 0;
    *jumped = 0;
    int e = dsp_decode(ins, nins, ops, DMAX, &n, conds, DMAX, &m, joff);
    if (e) return -2;
    for (int i = 0; i < n; i++)
        if (ops[i].kind == K_J) ops[i].a = i;          /* the op index: its offset is joff[i] */
    Insn in = {0, n, 0, m, 0};
    Prog p = {1, &in, ops, conds, NULL};
    int jump = -2;
    e = exec_insn(s, &s->d[0], &p, 0, &jump);
    if (e) return e;
    s->d[0].ninstr++;
    if (jump >= 0) *jumped = 1, *jump_off = joff[jump];
    return 0;
}

/* the end of a sample on the machine: the delay line's DMA (flat[4300] into the EDM), then t + 1 */
void dsp_machine_end_sample(Slave *s)
{
    edm_write(s);
    s->t++;
    s->d[0].t = s->t;
}
