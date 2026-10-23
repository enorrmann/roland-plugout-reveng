/* C-S1JX, shared by both regions: the JX-3P parameter model (model_full.py -> build/model.h, evaluated as in
 * examples/from_scratch/engine/engine.c) and the four voices, with every DSP write of the SYSTEM-8 layout
 * (S8 DSP d, id) sent to where s1jx.py put that program on the SYSTEM-1:
 *
 *   bank 5 (B):      S8 DSP d, id -> b5_index[d][id - 0x20]: a bank-5 word of d's chip (0x0100FD6C(sel, obj, bits))
 *                    or, for parameters the program only reads, a bank-6 id (0x8000 | sel << 8 | id, FCEC)
 *                    (sel = word >> 8: A000+ / A100+; the same code on master and slave, system-1/DSP_FIT.md)
 *   bank 6 (P):      S8 DSP d, id -> SYSTEM-1 DSP b6_sel[d] of d's chip, id + b6_off[d], 0x0100FCEC(sel, id, bits)
 *
 * Even S8 DSPs (0, 2, 4: voices 0 and 1) are on the master, odd ones (1, 3, 5: voices 2 and 3) on the slave; each
 * region runs the whole model and keeps only its chip's writes (JX_CHIP). No FPU: soft float (libgcc).
 * Ramps (+0xA0 on the SYSTEM-8) are written immediately: the SYSTEM-1 ramp service FCE4 is not mapped yet.
 */
#include <stdint.h>
#include "syninfo.h"

typedef struct { uint16_t src, when, nwhen, expr, nexpr, t0; uint8_t nt; } mout_t;
typedef struct { uint8_t bank, dsp, ramp, flag; uint16_t id; } mtgt_t;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#include "model.h"                 /* vel_curve; the model's byte code only with JX_RUNTIME_MODEL */
#include "s1jx.h"
#pragma GCC diagnostic pop

#ifndef JX_CHIP
#error JX_CHIP: 0 = master (region 0), 1 = slave (region 1)
#endif
#define N(a) (sizeof(a) / sizeof((a)[0]))

typedef void (*bank_fn)(int sel, const uint32_t *obj, uint32_t value);
typedef void (*env_fn)(int sel, int id, uint32_t value);

static uint32_t fbits(float f) { union { float f; uint32_t u; } x = { .f = f }; return x.u; }

static void b5_write(int word, uint32_t bits)
{
    uint32_t obj[3] = { 0, 0, (uint32_t)(word & 0xFF) };
    ((bank_fn)SVC(0x16C))(word >> 8, obj, bits);
}

static void dsp_bank(int dsp, int id, float v)
{
    if ((dsp & 1) != JX_CHIP || id < 0x20 || id >= 0x50)
        return;
    uint16_t w = b5_index[dsp][id - 0x20];
    if (w == 0xFFFF)
        return;
    if (w & 0x8000)                                   /* a parameter the program only reads: bank 6 */
        ((env_fn)SVC(0xEC))((w >> 8) & 1, w & 0xFF, fbits(v));
    else
        b5_write(w, fbits(v));
}

__attribute__((unused)) static void dsp_param(int dsp, int id, float v)
{
    if ((dsp & 1) != JX_CHIP || id >= 0x100)
        return;
    ((env_fn)SVC(0xEC))(b6_sel[dsp], id + b6_off[dsp], fbits(v));
}

/* ---- parameter model ------------------------------------------------------------------------------------- */
/* Default build: the start-up state is a table computed on the PC (s1jx.state_table: jx_state0 / jx_state1) and
   copied here; no float maths and almost no time in load (the runtime evaluation, JX_RUNTIME_MODEL, was the
   suspect of the hang at PLUG-OUT selection, 2026-10-22). JX_RUNTIME_MODEL=1 keeps the evaluator for when the
   panel is mapped to JX-3P ids. */

typedef struct jx {
    int32_t val[NPARAM];
    uint8_t note[4], on[4];
    int ready;                     /* 0 until the first full state after a load */
} jx_t;

static jx_t jx;

static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

#ifdef JX_RUNTIME_MODEL
static int pidx(int id)
{
    int lo = 0, hi = NPARAM - 1;
    while (lo <= hi) {
        int m = (lo + hi) >> 1;
        if (param_id[m] == id) return m;
        if (param_id[m] < id) lo = m + 1; else hi = m - 1;
    }
    return -1;
}

enum { OP_CONST, OP_PARAM, OP_SLAVE, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MIN, OP_MAX, OP_GE, OP_LT,
       OP_SEL, OP_EXP2, OP_PICK, OP_KNOTS, OP_ABS, OP_FLOOR, OP_INTERP, OP_FPARAM };

static float floorf_(float x)
{
    float t = (float)(int32_t)x;
    return t > x ? t - 1.0f : t;
}

static float exp2f_(float x)
{
    if (x < -126.0f) return 0.0f;
    if (x > 127.0f) x = 127.0f;
    float n = floorf_(x + 0.5f);
    float f = (x - n) * 0.69314718f, t = 1.0f, s = 1.0f;
    for (int k = 1; k <= 7; k++) { t *= f / (float)k; s += t; }
    union { float f; uint32_t u; } e = { .u = (uint32_t)((int32_t)n + 127) << 23 };
    return s * e.f;
}

static float eval_expr(unsigned off, unsigned n)
{
    float st[24];
    int sp = 0;
    for (unsigned i = off; i < off + n; ) {
        unsigned op = mcode[i++];
        switch (op) {
        case OP_CONST: st[sp++] = mconst[mcode[i++]]; break;
        case OP_PARAM: st[sp++] = (float)jx.val[mcode[i++]]; break;
        case OP_FPARAM: {
            union { int32_t i; float f; } u = { .i = jx.val[mcode[i++]] };
            st[sp++] = u.f;
            break;
        }
        case OP_SLAVE: st[sp++] = 0.0f; break;          /* one SYSTEM-8 slave's worth of voices */
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
        case OP_KNOTS: i += 4; break;
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

static void run_outputs(unsigned first, unsigned count)
{
    for (unsigned o = first; o < first + count; o++) {
        const mout_t *m = &mout[o];
        if (m->nwhen && eval_expr(m->when, m->nwhen) == 0.0f)
            continue;
        float v = eval_expr(m->expr, m->nexpr);
        for (unsigned t = m->t0; t < m->t0 + m->nt; t++) {
            const mtgt_t *g = &mtgt[t];
            if (g->bank) dsp_bank(g->dsp, g->id, v);
            else dsp_param(g->dsp, g->id, v);
        }
    }
}

#endif /* JX_RUNTIME_MODEL */

/* ---- voices (engine.c note_pitch / note_gate; local voice lv 0..3 of one SYSTEM-8 slave) ------------------ */

static void active_flags(void)
{
    float any = (jx.on[0] | jx.on[1] | jx.on[2] | jx.on[3]) ? 1.0f : 0.0f;
    dsp_bank(0, 0x29, any); dsp_bank(0, 0x3b, any);
    dsp_bank(1, 0x29, any); dsp_bank(1, 0x3b, any);
}

static void jx_pitch(int lv, int note)
{
    int u = lv >> 1, k = lv & 1;
    jx.note[lv] = (uint8_t)(note & 0x7F);
    dsp_bank(u, 0x21 + 0x12 * k, (float)(jx.note[lv] - 12) / 12.0f);
}

static void jx_gate(int lv, int vel)
{
    int u = lv >> 1, k = lv & 1, d = 4 + u, s = 0xf * k - u;   /* DSP5 ids are one lower */
    vel = clampi(vel, 0, 127);
    if (vel > 0) {
        dsp_bank(d, 0x2a + s, 1.0f);
        dsp_bank(d, 0x29 + s, vel_curve[vel]);
        dsp_bank(d, 0x22 + s, (float)vel / 127.0f);
        dsp_bank(u, 0x22 + 0x12 * k, 1.0f);
        jx.on[lv] = 1;
    } else {
        dsp_bank(u, 0x22 + 0x12 * k, 0.0f);
        jx.on[lv] = 0;
    }
    active_flags();
}

/* ---- state ------------------------------------------------------------------------------------------------ */

static void jx_init(void)
{
#ifdef JX_RUNTIME_MODEL
    for (int i = 0; i < NPARAM; i++)
        jx.val[i] = 0;
    for (unsigned j = 0; j < N(jx_defaults); j++) {
        int i = pidx(jx_defaults[j].id);
        if (i >= 0)
            jx.val[i] = jx_defaults[j].v;
    }
#endif
    for (int v = 0; v < 4; v++) { jx.note[v] = 60; jx.on[v] = 0; }
    jx.ready = 0;
}

/* every output in the JX-3P's refresh order, what engine.c adds outside the model, and the voices */
#define JX_STATE_(c) jx_state##c
#define JX_STATE(c) JX_STATE_(c)
static void jx_send_state(void)
{
#ifdef JX_NO_STATE
    jx.ready = 1;                     /* NO_STATE=1 build.sh: no start-up writes at all (diagnostic) */
    return;
#endif
#ifdef JX_RUNTIME_MODEL
    run_outputs(CONST_FIRST, CONST_COUNT);
    for (unsigned j = 0; j < N(refresh_order); j++)
        run_outputs(src_first[refresh_order[j]], src_count[refresh_order[j]]);
    dsp_param(4, 0x4a, 1.0f);                                 /* e_unmute: MAIN4's output level */
#else
    for (unsigned j = 0; j < N(JX_STATE(JX_CHIP)); j++) {
        if (JX_STATE(JX_CHIP)[j].kind == 0)
            b5_write(JX_STATE(JX_CHIP)[j].idx, JX_STATE(JX_CHIP)[j].bits);
        else
            ((env_fn)SVC(0xEC))(JX_STATE(JX_CHIP)[j].sel, JX_STATE(JX_CHIP)[j].idx, JX_STATE(JX_CHIP)[j].bits);
    }
#endif
    for (int lv = 0; lv < 4; lv++) {
        jx_pitch(lv, jx.note[lv]);
        jx_gate(lv, jx.on[lv] ? 100 : 0);
    }
    jx.ready = 1;
}

/* a parameter change (panel / MIDI mapping to JX-3P ids: not done yet, docs) */
#ifdef JX_RUNTIME_MODEL
__attribute__((unused)) static void jx_param(int id, int value)
{
    int i = pidx(id);
    if (i < 0)
        return;
    jx.val[i] = value;
    run_outputs(src_first[i], src_count[i]);
}
#endif
