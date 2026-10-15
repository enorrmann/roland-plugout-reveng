/*
 * From-scratch PLUG-OUT engine (region 1, runs on the slave BMCs).
 *
 * Our own ARM code. The only foreign data are the DSP programs (microcode),
 * copied by programs.py from the user's JX-3P at build time. The engine
 * registers them, keeps the current parameter values and turns them into DSP
 * parameter writes with our parameter model (model_spec.py, compiled by
 * gen_model.py into byte code that eval_expr() interprets), swaps the
 * oscillator / effect programs when the waveforms or the effect type change,
 * and turns notes into pitch / gate writes.
 * Interface: CUSTOM_PLUGOUTS.md "DSP microcode in the engine",
 * examples/from_scratch/NOTES.md.
 */
#include <stdint.h>

/* program / data descriptor as in the JX-3P image: the OS uses the two trailing words (zero in the
   image; with a 4-word struct it overwrote the next array entry, and every odd entry was silent) */
typedef struct { uint32_t addr, n1, n2; const void *src; uint32_t os[2]; } dspdesc_t;
typedef struct { uint16_t src, when, nwhen, expr, nexpr, t0; uint8_t nt; } mout_t;
typedef struct { uint8_t bank, dsp, ramp, flag; uint16_t id; } mtgt_t;

#include "programs.h"
#include "model.h"

#define N(a) (sizeof(a) / sizeof((a)[0]))

/* OS service table of the slave (s_boot, copied to 0x0113FC00) */
#define OS(off) (*(void *volatile *)(0x0113FC00u + (off)))
typedef void (*reg_fn)(const void *);                                   /* +0x28..+0x6C */
typedef void (*ramp_fn)(int dsp, int id, int ramp, int flag, int sp, float v); /* +0xA0 */
typedef void (*set_fn)(int dsp, int id, float v);                       /* +0xA4 */
typedef void (*bank_fn)(int dsp, const uint32_t *id, float v);          /* +0xB0 */
typedef int (*slave_fn)(void);                                          /* +0xF0 */
typedef int (*get_fn)(int part, int grp, int id);                       /* +0x00 */
typedef void (*sync_fn)(int a, int b, int c, int d);                    /* +0xC0: wait for the DSP frame */
typedef void (*swap_fn)(int dsp, const void *prog, const void *data, void *ctx, void *ctx2,
                        uint32_t slot, uint32_t task, uint32_t x0, uint32_t x1);  /* +0x70 */

static void p_ramp(int dsp, int id, int ramp, int flag, int sp, float v) { ((ramp_fn)OS(0xA0))(dsp, id, ramp, flag, sp, v); }
static void p_set(int dsp, int id, float v) { ((set_fn)OS(0xA4))(dsp, id, v); }
static void p_bank(int dsp, uint32_t id, float v) { ((bank_fn)OS(0xB0))(dsp, &id, v); }
static int slave_index(void) { return ((slave_fn)OS(0xF0))() ? 1 : 0; }
static int os_get(int grp, int id) { return ((get_fn)OS(0x00))(0, grp, id); }

/* ---- programs ----------------------------------------------------------------- */

/* an empty slot program (execution order: two no-ops, end) and the zero data blocks */
static const uint32_t mc_empty[4] = { 0x0a02ff70, 0x0a02ff70, 0x0b0200f8, 0x00010000 };
static const uint32_t zero_data[8];

#define PROG(a, name) { a, MC_##name##_BYTES, MC_##name##_BYTES, mc_##name, { 0, 0 } }
#define EMPTY(a) { a, 0xe, 0xe, mc_empty, { 0, 0 } }
#define DATA(a, n) { a, n, n, zero_data, { 0, 0 } }
enum { SLOT_MAIN = 0x0100, SLOT_DCO1 = 0x6000, SLOT_DCO2 = 0xB000, SLOT_FX = 0xB000, SLOT_X = 0x8000 };

/* Every descriptor is used by one DSP only, and every program has its own data descriptor, as in
   the JX-3P image (DSP2 and DSP3 have separate tables): the OS links registered descriptors through
   their trailing words, and one descriptor shared by DSP2 and DSP3 silenced every other swap. */
static dspdesc_t main_prog[6] = {
    PROG(0x0100, MAIN0), PROG(0x0100, MAIN1), PROG(0x0100, MAIN2),
    PROG(0x0100, MAIN3), PROG(0x0100, MAIN4), PROG(0x0100, MAIN5),
};
static dspdesc_t empty_x[2] = { EMPTY(SLOT_X), EMPTY(SLOT_X) };              /* DSP0, DSP1 */
static dspdesc_t empty_6[2] = { EMPTY(SLOT_DCO1), EMPTY(SLOT_DCO1) };        /* DSP4, DSP5 */
/* DCO1 waveform 0 SAW, 1 SQR, 2.. PULSE (= SAW program + flag); DCO2 0 SAW, 1 SQR, 2 PULSE, 3.. NOISE;
   [0] DSP2, [1] DSP3 */
#define DCO1_PROGS { PROG(SLOT_DCO1, DCO1_SAW), PROG(SLOT_DCO1, DCO1_SQR), PROG(SLOT_DCO1, DCO1_SAW) }
#define DCO2_PROGS { PROG(SLOT_DCO2, DCO2_SAW), PROG(SLOT_DCO2, DCO2_SQR), PROG(SLOT_DCO2, DCO2_SAW), \
                     PROG(SLOT_DCO2, DCO2_NOISE) }
static dspdesc_t dco1_prog[2][3] = { DCO1_PROGS, DCO1_PROGS };
#ifdef DSPTEST
/* ISA test bench (re/dspaudio.py): no live swaps (they were unreliable from note events); the
   DCO2 slot of DSP2 runs test program t0, the one of DSP3 t1, whatever the waveform. Each test
   program has a voice A path (outputs [2221] [221d]) and a voice B path ([2229] [2225]). */
static dspdesc_t dco2_prog[2][4] = { { PROG(SLOT_DCO2, T0), PROG(SLOT_DCO2, T0), PROG(SLOT_DCO2, T0), PROG(SLOT_DCO2, T0) },
                                     { PROG(SLOT_DCO2, T1), PROG(SLOT_DCO2, T1), PROG(SLOT_DCO2, T1), PROG(SLOT_DCO2, T1) } };
#else
static dspdesc_t dco2_prog[2][4] = { DCO2_PROGS, DCO2_PROGS };
#endif
#define D8 DATA(0x0008, 8)
#define D10 DATA(0x0010, 8)
static dspdesc_t dco1_data[2][3] = { { D8, D8, D8 }, { D8, D8, D8 } };
static dspdesc_t dco2_data[2][4] = { { D10, D10, D10, D10 }, { D10, D10, D10, D10 } };
/* effect types 0 OD, 1 DS, 2 CHORUS, 3 FUZZ, 4 CRUSHER, 5 PHASER (DSP4) */
static dspdesc_t fx_prog[6] = { PROG(SLOT_FX, FX_OD), PROG(SLOT_FX, FX_DS), PROG(SLOT_FX, FX_CH),
                                PROG(SLOT_FX, FX_FZ), PROG(SLOT_FX, FX_CR), PROG(SLOT_FX, FX_PH) };
#define DFX DATA(0x16e8, 8)
static dspdesc_t fx_data[6] = { DFX, DFX, DFX, DFX, DFX, DFX };
static dspdesc_t data_0[6] = { DATA(0, 8), DATA(0, 8), DATA(0, 8), DATA(0, 8), DATA(0, 8), DATA(0, 8) };
static dspdesc_t data_8[6] = { D8, D8, D8, D8, DATA(0x0008, 0x20), DATA(0x0008, 0x20) };   /* slot 1 */

/* ---- engine object -------------------------------------------------------------- */

typedef struct engine {
    const void *const *vt;
    int slave, first;              /* slave 0: voices 0-3, slave 1: voices 4-7 */
    uint8_t note[4], on[4];
    int32_t val[NPARAM];           /* current parameter values */
} engine_t;

static engine_t engine;

static int pidx(int id)            /* parameter index of an id, -1 if not in the model */
{
    int lo = 0, hi = NPARAM - 1;
    while (lo <= hi) {
        int m = (lo + hi) >> 1;
        if (param_id[m] == id) return m;
        if (param_id[m] < id) lo = m + 1; else hi = m - 1;
    }
    return -1;
}

static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

/* ---- the model's byte code (ops as in expr.py) -------------------------------------- */

enum { OP_CONST, OP_PARAM, OP_SLAVE, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MIN, OP_MAX, OP_GE, OP_LT,
       OP_SEL, OP_EXP2, OP_PICK, OP_KNOTS, OP_ABS, OP_FLOOR, OP_INTERP, OP_FPARAM };

static float floorf_(float x)
{
    float t = (float)(int32_t)x;
    return t > x ? t - 1.0f : t;
}

static float exp2f_(float x)       /* 2^x: integer part in the exponent, e^(f ln 2) for |f| <= 0.5 */
{
    if (x < -126.0f) return 0.0f;
    if (x > 127.0f) x = 127.0f;
    float n = floorf_(x + 0.5f);
    float f = (x - n) * 0.69314718f, t = 1.0f, s = 1.0f;
    for (int k = 1; k <= 7; k++) { t *= f / (float)k; s += t; }
    union { float f; uint32_t u; } e = { .u = (uint32_t)((int32_t)n + 127) << 23 };
    return s * e.f;
}

static float eval_expr(const engine_t *e, unsigned off, unsigned n)
{
    float st[24];
    int sp = 0;
    for (unsigned i = off; i < off + n; ) {
        unsigned op = mcode[i++];
        switch (op) {
        case OP_CONST: st[sp++] = mconst[mcode[i++]]; break;
        case OP_PARAM: st[sp++] = (float)e->val[mcode[i++]]; break;
        case OP_FPARAM: {                    /* float parameter: the value's bits */
            union { int32_t i; float f; } u = { .i = e->val[mcode[i++]] };
            st[sp++] = u.f;
            break;
        }
        case OP_SLAVE: st[sp++] = (float)e->slave; break;
        case OP_EXP2: st[sp - 1] = exp2f_(st[sp - 1]); break;
        case OP_ABS: if (st[sp - 1] < 0) st[sp - 1] = -st[sp - 1]; break;
        case OP_FLOOR: st[sp - 1] = floorf_(st[sp - 1]); break;
        case OP_SEL: sp -= 2; st[sp - 1] = st[sp - 1] != 0.0f ? st[sp] : st[sp + 1]; break;
        case OP_PICK: {
            int k = mcode[i++];
            int j = clampi((int)st[sp - k - 1], 0, k - 1);
            st[sp - k - 1] = st[sp - k + j];
            sp -= k;
            break;
        }
        case OP_INTERP: {
            const float *xs = &mconst[mcode[i]], *ys = xs + mcode[i + 1];
            int k = mcode[i + 1];
            float x = st[sp - 1];
            i += 2;
            if (x <= xs[0]) { st[sp - 1] = ys[0]; break; }
            if (x >= xs[k - 1]) { st[sp - 1] = ys[k - 1]; break; }
            int j = 0;
            while (x >= xs[j + 1]) j++;
            st[sp - 1] = ys[j] + (ys[j + 1] - ys[j]) * ((x - xs[j]) / (xs[j + 1] - xs[j]));
            break;
        }
        case OP_KNOTS: i += 4; break;        /* not used by the model */
        default: {
            float b = st[--sp], a = st[sp - 1], r;
            switch (op) {
            case OP_ADD: r = a + b; break;
            case OP_SUB: r = a - b; break;
            case OP_MUL: r = a * b; break;
            case OP_DIV: r = a / b; break;
            case OP_MIN: r = a < b ? a : b; break;
            case OP_MAX: r = a > b ? a : b; break;
            case OP_GE: r = a >= b ? 1.0f : 0.0f; break;
            default: r = a < b ? 1.0f : 0.0f; break;
            }
            st[sp - 1] = r;
        }
        }
    }
    return st[0];
}

/* the outputs of one source; immediate = start-up / refresh (+0xA4, no ramps) */
static void run_outputs(engine_t *e, unsigned first, unsigned count, int immediate)
{
    for (unsigned o = first; o < first + count; o++) {
        const mout_t *m = &mout[o];
        if (m->nwhen && eval_expr(e, m->when, m->nwhen) == 0.0f)
            continue;
        float v = eval_expr(e, m->expr, m->nexpr);
        for (unsigned t = m->t0; t < m->t0 + m->nt; t++) {
            const mtgt_t *g = &mtgt[t];
            if (g->bank) p_bank(g->dsp, g->id, v);
            else if (immediate || g->ramp == 0xFF) p_set(g->dsp, g->id, v);
            else p_ramp(g->dsp, g->id, g->ramp, g->flag, 0, v);
        }
    }
}

static void run_source(engine_t *e, int i, int immediate)
{
    run_outputs(e, src_first[i], src_count[i], immediate);
}

/* ---- program swaps (as the JX-3P does when the waveforms / effect type change) ----------- */

static uint32_t swap_ctx[8] = { 0, 0, 0xFFFFFFFFu };

static void swap(int dsp, const dspdesc_t *prog, const dspdesc_t *data, uint32_t slot, uint32_t task,
                 uint32_t x0, uint32_t x1)
{
    ((swap_fn)OS(0x70))(dsp, prog, data, swap_ctx, swap_ctx, slot, task, x0, x1);
}

static void dco_levels(int id, float v, int ramp)     /* level of DCO1 (0x32) or DCO2 (0x33), 4 voices */
{
    for (int d = 2; d <= 3; d++)
        for (int k = 0; k < 2; k++) {
            int last = !ramp && d == 3 && k == 1;     /* the last mute ramp waits (flag 1) */
            p_ramp(d, id + 0x16 * k, ramp, last, last, v);
        }
}

static void swap_dco(engine_t *e, int which)
{
    int id = which == 1 ? 0x32 : 0x33;
    int w1 = clampi(e->val[P_DCO1_WAVE], 0, 2), w2 = clampi(e->val[P_DCO2_WAVE], 0, 3);
    dco_levels(id, 0.0f, 0);
    ((sync_fn)OS(0xC0))(3, 0, 0, 1);
    for (int d = 2; d <= 3; d++)
        if (which == 1)
            swap(d, &dco1_prog[d - 2][w1], &dco1_data[d - 2][w1], SLOT_DCO1, 2, 0, 0);
        else
            swap(d, &dco2_prog[d - 2][w2], &dco2_data[d - 2][w2], SLOT_DCO2, 3, 0, 0);
    dco_levels(id, 1.0f, 3);
}


static void swap_fx(engine_t *e)
{
    p_ramp(4, 0x4a, 3, 1, 1, 0.0f);
    ((sync_fn)OS(0xC0))(4, 0, 3, 1);
    int t = clampi(e->val[P_FX_TYPE], 0, 5);
    swap(4, &fx_prog[t], &fx_data[t], SLOT_FX, 3, 0x1F0, 0x80);
    p_ramp(4, 0x4a, 3, 0, 0, 1.0f);
    if (e->val[P_FX_TYPE] == 0) {                     /* OVERDRIVE: its two level ids */
        p_ramp(4, 0x1f3, 15, 0, 0, 1.0f);
        p_ramp(4, 0x1f2, 15, 0, 0, 1.0f);
    }
}

/* ---- voices ------------------------------------------------------------------------- */

static void active_flags(engine_t *e)
{
    float any = (e->on[0] | e->on[1] | e->on[2] | e->on[3]) ? 1.0f : 0.0f;
    p_bank(0, 0x29, any); p_bank(0, 0x3b, any);
    p_bank(1, 0x29, any); p_bank(1, 0x3b, any);
}

static void note_pitch(engine_t *e, int lv, int note)
{
    int u = lv >> 1, k = lv & 1;
    e->note[lv] = note;
    p_bank(u, 0x21 + 0x12 * k, (float)(note - 12) / 12.0f);
}

static void note_gate(engine_t *e, int lv, int vel)
{
    int u = lv >> 1, k = lv & 1, d = 4 + u, s = 0xf * k - u;   /* DSP5 ids are one lower */
    vel = clampi(vel, 0, 127);
    if (vel > 0) {
        p_bank(d, 0x2a + s, 1.0f);
        p_bank(d, 0x29 + s, vel_curve[vel]);
        p_bank(d, 0x22 + s, (float)vel / 127.0f);
        p_bank(u, 0x22 + 0x12 * k, 1.0f);
        e->on[lv] = 1;
    } else {
        p_bank(u, 0x22 + 0x12 * k, 0.0f);
        e->on[lv] = 0;
    }
    active_flags(e);
}

/* ---- state ---------------------------------------------------------------------------- */

static void read_params(engine_t *e)
{
    for (int i = 0; i < NPARAM; i++)
        e->val[i] = os_get(param_group[i], param_id[i]);
}

/* full refresh: every output with the current values, in the JX-3P's order (last write wins) */
static void send_state(engine_t *e)
{
    read_params(e);
    run_outputs(e, CONST_FIRST, CONST_COUNT, 1);
    for (unsigned j = 0; j < N(refresh_order); j++)
        run_source(e, refresh_order[j], 1);
    for (int lv = 0; lv < 4; lv++) {
        note_pitch(e, lv, os_get(2, 0x429 + e->first + lv));
        note_gate(e, lv, os_get(2, 0x43A + e->first + lv));
    }
}

/* vtable entries (called by the slave as obj->vt[n](obj, ...)) */
static int  e_nop(void) { return 0; }
static void e_init(engine_t *e) { e->slave = slave_index(); e->first = 4 * e->slave; }
static void e_load(engine_t *e)                                         /* vt+0x08 */
{
    /* slots, then programs, then data, DSP by DSP (OS +0x28 / +0x40 / +0x58 + 4 * dsp) */
    int w1 = clampi(os_get(4, 0x6d4), 0, 2), w2 = clampi(os_get(4, 0x6d9), 0, 3);
    int fx = clampi(os_get(4, 0x748), 0, 5);
    static const uint16_t slots[6][3] = {
        { SLOT_MAIN, SLOT_X }, { SLOT_MAIN, SLOT_X }, { SLOT_MAIN, SLOT_DCO1, SLOT_DCO2 },
        { SLOT_MAIN, SLOT_DCO1, SLOT_DCO2 }, { SLOT_MAIN, 0x6000, SLOT_FX }, { SLOT_MAIN, 0x6000 },
    };
    static const uint8_t nslot[6] = { 2, 2, 3, 3, 3, 2 };
    (void)e;
    for (int d = 0; d < 6; d++)
        for (int s = 0; s < nslot[d]; s++)
            ((reg_fn)OS(0x28 + 4 * d))((const void *)(uintptr_t)slots[d][s]);
    for (int d = 0; d < 6; d++) {
        const dspdesc_t *p[3] = { &main_prog[d], d < 2 ? &empty_x[d] : &empty_6[d - 4], 0 };
        if (d == 2 || d == 3) { p[1] = &dco1_prog[d - 2][w1]; p[2] = &dco2_prog[d - 2][w2]; }
        if (d == 4) p[2] = &fx_prog[fx];
        for (int s = 0; s < nslot[d]; s++)
            ((reg_fn)OS(0x40 + 4 * d))(p[s]);
    }
    for (int d = 0; d < 6; d++) {
        const dspdesc_t *p[3] = { &data_0[d], &data_8[d], 0 };
        if (d == 2 || d == 3) { p[1] = &dco1_data[d - 2][w1]; p[2] = &dco2_data[d - 2][w2]; }
        if (d == 4) p[2] = &fx_data[fx];
        for (int s = 0; s < nslot[d]; s++)
            ((reg_fn)OS(0x58 + 4 * d))(p[s]);
    }
}
static void e_start(engine_t *e)                                         /* vt+0x0C */
{
    send_state(e);
    /* slot programs only run once +0x70 has enabled their task bit */
    swap_dco(e, 1);
    swap_dco(e, 2);
    swap_fx(e);
}
static void e_mute(engine_t *e)                                          /* vt+0x10 */
{
    (void)e;
    for (int d = 2; d <= 3; d++)
        for (int k = 0; k < 2; k++) { p_ramp(d, 0x32 + 0x16 * k, 0, 0, 0, 0.0f); }
    for (int d = 2; d <= 3; d++)
        for (int k = 0; k < 2; k++) { p_ramp(d, 0x33 + 0x16 * k, 0, 0, 0, 0.0f); }
    p_ramp(4, 0x4a, 3, 0, 0, 0.0f);
}
static void e_unmute(engine_t *e)                                        /* vt+0x18 */
{
    for (int d = 2; d <= 3; d++)
        for (int k = 0; k < 2; k++) { p_ramp(d, 0x32 + 0x16 * k, 3, 0, 0, 1.0f); }
    for (int d = 2; d <= 3; d++)
        for (int k = 0; k < 2; k++) { p_ramp(d, 0x33 + 0x16 * k, 3, 0, 0, 1.0f); }
    p_ramp(4, 0x4a, 3, 0, 0, 1.0f);
    switch (clampi(e->val[P_FX_TYPE], 0, 5)) {          /* the effect program's own level */
    case 0: p_ramp(4, 0x1f3, 15, 0, 0, 1.0f); p_ramp(4, 0x1f2, 15, 0, 0, 1.0f); break;
    case 1: case 3: p_ramp(4, 0x1f2, 3, 0, 0, 1.0f); break;
    case 2: case 4: p_ramp(4, 0x1f0, 3, 0, 0, 1.0f); break;
    default: p_ramp(4, 0x1f7, 3, 0, 0, 1.0f); break;
    }
}
static void e_refresh(engine_t *e, int all) { if (all) send_state(e); }  /* vt+0x24 */
static void e_param(engine_t *e, int id, int zero, int value)            /* vt+0x30 */
{
    (void)zero;
    int v;
    if (id >= 0x429 && id < 0x429 + 8) {
        v = id - 0x429 - e->first;
        if (v >= 0 && v < 4) note_pitch(e, v, value);
        return;
    }
    if (id >= 0x43A && id < 0x43A + 8) {
        v = id - 0x43A - e->first;
        if (v >= 0 && v < 4) note_gate(e, v, value);
        else active_flags(e);                         /* a voice of the other slave */
        return;
    }
    int i = pidx(id);
    if (i < 0)
        return;
    e->val[i] = value;
    if (i == P_DCO1_WAVE) swap_dco(e, 1);
    else if (i == P_DCO2_WAVE) swap_dco(e, 2);
    else if (i == P_FX_TYPE) swap_fx(e);
    run_source(e, i, 0);
}

static const void *const engine_vt[28] = {
    [0x00 / 4] = (const void *)e_init,
    [0x04 / 4] = (const void *)e_nop,
    [0x08 / 4] = (const void *)e_load,
    [0x0C / 4] = (const void *)e_start,
    [0x10 / 4] = (const void *)e_mute,
    [0x14 / 4] = (const void *)e_nop,
    [0x18 / 4] = (const void *)e_unmute,
    [0x1C / 4] = (const void *)e_nop,
    [0x20 / 4] = (const void *)e_nop,
    [0x24 / 4] = (const void *)e_refresh,
    [0x28 / 4] = (const void *)e_nop,
    [0x2C / 4] = (const void *)e_nop,
    [0x30 / 4] = (const void *)e_param,
    [0x34 / 4] = (const void *)e_nop, [0x38 / 4] = (const void *)e_nop,
    [0x3C / 4] = (const void *)e_nop, [0x40 / 4] = (const void *)e_nop,
    [0x44 / 4] = (const void *)e_nop, [0x48 / 4] = (const void *)e_nop,
    [0x4C / 4] = (const void *)e_nop, [0x50 / 4] = (const void *)e_nop,
    [0x54 / 4] = (const void *)e_nop, [0x58 / 4] = (const void *)e_nop,
    [0x5C / 4] = (const void *)e_nop, [0x60 / 4] = (const void *)e_nop,
    [0x64 / 4] = (const void *)e_nop, [0x68 / 4] = (const void *)e_nop,
    [0x6C / 4] = (const void *)e_nop,
};

static engine_t engine = { .vt = engine_vt };

#ifndef HYBRID   /* hybrid.c includes this file and has its own exports */
/* ---- exports: descriptor and SYNINFO ------------------------------------- */

/* desc[1..] is what init returns: D[0] slot (set by the slave), D[1] create,
   D[2] destroy, D[3], D[4] unused, D[5] main object */
static uint32_t desc[8];

static uint32_t *e_create(void)
{
    e_init(&engine);
    return &desc[1];
}
static void e_destroy(void) {}

uint32_t *plugout_init(void)
{
    desc[2] = (uint32_t)(uintptr_t)e_create;
    desc[3] = (uint32_t)(uintptr_t)e_destroy;
    desc[4] = 0;
    desc[5] = 0;
    desc[6] = (uint32_t)(uintptr_t)&engine;
    return &desc[1];
}

static const uint32_t empty_list = 0xFFFFFFFFu;

struct syninfo {
    char magic[16], name[16];
    uint16_t model, version;
    uint32_t *(*init)(void);
    const void *ptr;
    char magic2[16];
    uint32_t pad[4];
};

__attribute__((section(".syninfo"), used))
const struct syninfo syninfo = {
    "SYSTEM8_SYNINFO", "SYSTEM-8", 0x000a, 0x0001,
    plugout_init, &empty_list,
    "SYSTEM8_SYNINFO", {0},
};
#endif
