/*
 * From-scratch PLUG-OUT engine (region 1, runs on the slave BMCs).
 *
 * Our own ARM code; the only foreign data are the DSP programs (microcode),
 * taken by gen_tables.py from the user's JX-3P: the main program of each of the
 * six DSPs and the SAW program in both oscillator slots. The engine registers
 * them, sends the start-up state and turns notes into DSP parameter writes.
 * Interface: CUSTOM_PLUGOUTS.md "DSP microcode in the engine",
 * examples/from_scratch/NOTES.md.
 */
#include <stdint.h>

typedef struct { uint32_t addr, n1, n2; const void *src; } dspdesc_t;
typedef struct { uint8_t off, isdesc; uint16_t v; } regcall_t;
typedef struct { uint8_t unit; uint16_t id; float v; } pset_t;
typedef struct { uint8_t unit; uint16_t id; uint8_t ramp, flag; uint32_t sp; float v; } rampset_t;
typedef struct { uint8_t type; uint16_t a, b, c, d; uint32_t e, f, g, h; float v; } swapop_t;
typedef struct { uint8_t grp; uint16_t id; uint16_t first, n; } ext_t;        /* id outside the patch */
typedef struct { uint8_t bank, dsp; uint16_t id; uint16_t curve; } extout_t; /* one DSP write */
typedef struct { float a, b; int16_t tab; } curve_t;                         /* a*v+b, or curve_tab[tab][v] */

#include "tables.h"

#define N(a) (sizeof(a) / sizeof((a)[0]))

/* OS service table of the slave (s_boot, copied to 0x0113FC00) */
#define OS(off) (*(void *volatile *)(0x0113FC00u + (off)))
typedef void (*reg_fn)(const void *);                                   /* +0x28..+0x6C */
typedef void (*ramp_fn)(int dsp, int id, int ramp, int flag, int sp, float v); /* +0xA0 */
typedef void (*set_fn)(int dsp, int id, float v);                       /* +0xA4 */
typedef void (*bank_fn)(int dsp, const uint32_t *id, float v);          /* +0xB0 */
typedef void (*glob_fn)(float v);                                       /* +0xBC */
typedef int (*slave_fn)(void);                                          /* +0xF0 */
typedef int (*get_fn)(int part, int grp, int id);                       /* +0x00 */
typedef void (*sync_fn)(void);                                          /* +0xC0: wait for DSP frame sync */
typedef void (*swap_fn)(int dsp, const void *prog, const void *data, void *ctx, void *ctx2,
                        uint32_t slot, uint32_t task, uint32_t x0, uint32_t x1);  /* +0x70 */

static void p_ramp(const rampset_t *r) { ((ramp_fn)OS(0xA0))(r->unit, r->id, r->ramp, r->flag, r->sp, r->v); }
static void p_set(int dsp, int id, float v) { ((set_fn)OS(0xA4))(dsp, id, v); }
static void p_bank(int dsp, uint32_t id, float v) { ((bank_fn)OS(0xB0))(dsp, &id, v); }
static int slave_index(void) { return ((slave_fn)OS(0xF0))() ? 1 : 0; }

/* ---- engine object --------------------------------------------------------- */

typedef struct engine {
    const void *const *vt;
    int slave, first;              /* slave 0: voices 0-3, slave 1: voices 4-7 */
    uint8_t note[4], on[4];
} engine_t;

/* ids outside the patch (part level, controllers, on/off switches, ...): the
   emulator could only record their responses with value 0, so the engine reads
   the real value (OS +0x00) and maps it through the learned curves */
static float curve_eval(const curve_t *c, int v)
{
    if (c->tab < 0) return c->a * (float)v + c->b;
    if (v < 0) v = 0;
    if (v > 255) v = 255;
    return curve_tab[c->tab][v];
}

static void apply_ext(const ext_t *x, int v)
{
    for (unsigned j = 0; j < x->n; j++) {
        const extout_t *o = &extout[x->first + j];
        float y = curve_eval(&curves[o->curve], v);
        if (y != y) continue;                  /* not written by the JX-3P for this value */
        if (o->bank) p_bank(o->dsp, o->id, y);
        else p_set(o->dsp, o->id, y);
    }
}

static const ext_t *ext_table(engine_t *e, unsigned *n)
{
    *n = e->slave ? N(ext1) : N(ext0);
    return e->slave ? ext1 : ext0;
}

static void send_ext(engine_t *e)
{
    unsigned n;
    const ext_t *x = ext_table(e, &n);
    for (unsigned i = 0; i < n; i++)
        apply_ext(&x[i], ((get_fn)OS(0x00))(0, x[i].grp, x[i].id));
}

/* start the slot programs (DCO1/DCO2 on DSP2/3, effect on DSP4): +0x70 loads the
   program into its slot and enables the slot's task bit, as the JX-3P does when
   the waveform / effect parameters arrive */
static uint32_t swap_ctx[8] = { 0, 0, 0xFFFFFFFFu };

static void start_slots(void)
{
    for (unsigned i = 0; i < N(swap_ops); i++) {
        const swapop_t *o = &swap_ops[i];
        if (o->type == 0)
            ((ramp_fn)OS(0xA0))(o->a, o->b, o->c, o->d, o->e, o->v);
        else if (o->type == 1)
            ((sync_fn)OS(0xC0))();
        else
            ((swap_fn)OS(0x70))(o->a, &descs[o->b], &descs[o->c], swap_ctx, swap_ctx,
                                o->e, o->f, o->g, o->h);
    }
}

static void send_state(engine_t *e)
{
    const pset_t *a = e->slave ? state_a1 : state_a0;
    const pset_t *b = e->slave ? state_b1 : state_b0;
    unsigned na = e->slave ? N(state_a1) : N(state_a0);
    unsigned nb = e->slave ? N(state_b1) : N(state_b0);
    for (unsigned i = 0; i < na; i++) p_set(a[i].unit, a[i].id, a[i].v);
    for (unsigned i = 0; i < nb; i++) p_bank(b[i].unit, b[i].id, b[i].v);
    for (unsigned i = 0; i < N(state_bc0); i++)
        ((glob_fn)OS(0xBC))(e->slave ? state_bc1[i] : state_bc0[i]);
    send_ext(e);
}

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
    if (vel > 127) vel = 127;
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

static void last_voice(engine_t *e, int lv)
{
    for (int j = 0; j < 8; j++) {              /* same order as the JX-3P: from 0x3e + first */
        int i = (j + e->first) & 7;
        p_bank(5, 0x3e + i, i == lv ? 1.0f : 0.0f);
    }
}

/* vtable entries (called by the slave as obj->vt[n](obj, ...)) */
static int  e_nop(void) { return 0; }
static void e_init(engine_t *e) { e->slave = slave_index(); e->first = 4 * e->slave; }
static void e_load(engine_t *e)                                         /* vt+0x08 */
{
    (void)e;
    for (unsigned i = 0; i < N(regcalls); i++) {
        const regcall_t *r = &regcalls[i];
        ((reg_fn)OS(r->off))(r->isdesc ? (const void *)&descs[r->v] : (const void *)(uintptr_t)r->v);
    }
}
static void e_start(engine_t *e) { send_state(e); start_slots(); }       /* vt+0x0C */
static void e_mute(engine_t *e)                                          /* vt+0x10 */
{
    (void)e;
    for (unsigned i = 0; i < N(osc_mute); i++) p_ramp(&osc_mute[i]);
}
static void e_unmute(engine_t *e)                                        /* vt+0x18 */
{
    (void)e;
    for (unsigned i = 0; i < N(osc_unmute); i++) p_ramp(&osc_unmute[i]);
}
static void e_refresh(engine_t *e, int all) { if (all) send_state(e); }  /* vt+0x24 */
static void e_param(engine_t *e, int id, int zero, int value)            /* vt+0x30 */
{
    (void)zero;
    int v;
    if (id >= 0x429 && id < 0x429 + 8) {
        v = id - 0x429 - e->first;
        if (v >= 0 && v < 4) note_pitch(e, v, value);
    } else if (id >= 0x43A && id < 0x43A + 8) {
        v = id - 0x43A - e->first;
        if (v >= 0 && v < 4) note_gate(e, v, value);
    } else if (id == 0x45C) {
        v = value - e->first;
        if (v >= 0 && v < 4) last_voice(e, v);
    } else {
        unsigned n;
        const ext_t *x = ext_table(e, &n);
        for (unsigned i = 0; i < n; i++)
            if (x[i].id == id) apply_ext(&x[i], value);
    }
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
