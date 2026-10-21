/*
 * Minimal from-scratch PLUG-OUT engine (region 1, slave BMCs). Our own ARM code, no Roland data:
 * the DSP programs are ours (../dsp/voice.s, assembled by asm/asm.py), every other DSP runs an
 * empty program. Notes of this slave's four voices drive one monophonic voice of DSP5:
 *   bank 5 id 0x21 = pitch p = (note - 12) / 12   (f = 440 * 2^(p - 4.75))
 *   bank 5 id 0x22 = gate (1 while a key of this voice is held)
 * Region 0 (../model) is ours too; its parameter list is the JX-3P one for now.
 */
#include <stdint.h>

typedef struct { uint32_t addr, n1, n2; const void *src; uint32_t os[2]; } dspdesc_t;

#include "programs.h"

#define OS(off) (*(void *volatile *)(0x0113FC00u + (off)))
typedef void (*reg_fn)(const void *);                                   /* +0x28..+0x6C */
typedef void (*ramp_fn)(int dsp, int id, int ramp, int flag, int sp, float v); /* +0xA0 */
typedef void (*bank_fn)(int dsp, const uint32_t *id, float v);          /* +0xB0 */
typedef int (*slave_fn)(void);                                          /* +0xF0 */
typedef void (*sync_fn)(int a, int b, int c, int d);                    /* +0xC0: wait for the DSP frame */
typedef void (*swap_fn)(int dsp, const void *prog, const void *data, void *ctx, void *ctx2,
                        uint32_t slot, uint32_t task, uint32_t x0, uint32_t x1);  /* +0x70 */

static void p_bank(int dsp, uint32_t id, float v) { ((bank_fn)OS(0xB0))(dsp, &id, v); }
static int slave_index(void) { return ((slave_fn)OS(0xF0))() ? 1 : 0; }

enum { SLOT_MAIN = 0x0100, SLOT_DCO1 = 0x6000, SLOT_DCO2 = 0xB000, SLOT_FX = 0xB000, SLOT_X = 0x8000 };
#define PROG(a, name) { a, MC_##name##_BYTES, MC_##name##_BYTES, mc_##name, { 0, 0 } }
#define EMPTY(a) { a, 0xe, 0xe, mc_empty, { 0, 0 } }
#define DATA(a, n) { a, n, n, zero_data, { 0, 0 } }

static const uint32_t zero_data[8];
#ifdef ROLAND_MAIN     /* diagnostic: Roland's main programs (engine build) */
static dspdesc_t main_prog[6] = { PROG(SLOT_MAIN, MAIN0), PROG(SLOT_MAIN, MAIN1), PROG(SLOT_MAIN, MAIN2),
                                  PROG(SLOT_MAIN, MAIN3), PROG(SLOT_MAIN, MAIN4), PROG(SLOT_MAIN, MAIN5) };
#else
static dspdesc_t main_prog[6] = { EMPTY(SLOT_MAIN), EMPTY(SLOT_MAIN), EMPTY(SLOT_MAIN),
                                  EMPTY(SLOT_MAIN), EMPTY(SLOT_MAIN), EMPTY(SLOT_MAIN) };
#endif
static dspdesc_t x_empty[2] = { EMPTY(SLOT_X), EMPTY(SLOT_X) };                /* DSP0, DSP1 */
static dspdesc_t dco1_empty[6] = { EMPTY(SLOT_DCO1), EMPTY(SLOT_DCO1), EMPTY(SLOT_DCO1),
                                   EMPTY(SLOT_DCO1), EMPTY(SLOT_DCO1), EMPTY(SLOT_DCO1) };
static dspdesc_t dco2_empty[6] = { EMPTY(SLOT_DCO2), EMPTY(SLOT_DCO2), EMPTY(SLOT_DCO2),
                                   EMPTY(SLOT_DCO2), EMPTY(SLOT_DCO2), EMPTY(SLOT_DCO2) };
static dspdesc_t fx_empty = EMPTY(SLOT_FX);                                   /* DSP4 effect slot */
static dspdesc_t voice = PROG(SLOT_DCO1, VOICE);           /* DSP5 DCO1 slot: the voice */
#define D8 DATA(0x0008, 8)
#define D10 DATA(0x0010, 8)
#define DFX DATA(0x16e8, 8)
static dspdesc_t main_data[6] = { DATA(0, 8), DATA(0, 8), DATA(0, 8), DATA(0, 8), DATA(0, 8), DATA(0, 8) };
static dspdesc_t dco1_data[6] = { D8, D8, D8, D8, D8, D8 };
static dspdesc_t dco2_data[6] = { D10, D10, D10, D10, D10, D10 };
static dspdesc_t fx_data = DFX;
static dspdesc_t voice_data = DATA(0x0008, 0x20);
static uint32_t swap_ctx[8] = { 0, 0, 0xFFFFFFFFu };

typedef struct { const void *const *vt; int slave, first; int note[4]; int cur; } engine_t;

static void e_init(engine_t *e) { e->slave = slave_index(); e->first = 4 * e->slave; e->cur = -1; }
static void e_load(engine_t *e)                                         /* vt+0x08 */
{
    /* same topology as engine.c: DSP2/3 main + DCO1 + DCO2, DSP4 main + DCO1 + FX, DSP5 main + DCO1 (voice).
       Slots, then programs, then data, DSP by DSP (OS +0x28 / +0x40 / +0x58 + 4 * dsp) */
    static const uint16_t slots[6][3] = {
        { SLOT_MAIN, SLOT_X }, { SLOT_MAIN, SLOT_X }, { SLOT_MAIN, SLOT_DCO1, SLOT_DCO2 },
        { SLOT_MAIN, SLOT_DCO1, SLOT_DCO2 }, { SLOT_MAIN, SLOT_DCO1, SLOT_FX }, { SLOT_MAIN, SLOT_DCO1 },
    };
    static const uint8_t nslot[6] = { 2, 2, 3, 3, 3, 2 };
    (void)e;
    for (int d = 0; d < 6; d++)
        for (int s = 0; s < nslot[d]; s++)
            ((reg_fn)OS(0x28 + 4 * d))((const void *)(uintptr_t)slots[d][s]);
    for (int d = 0; d < 6; d++) {
        const dspdesc_t *p[3] = { &main_prog[d], 0, 0 };
        if (d < 2) p[1] = &x_empty[d];
        if (d == 2 || d == 3) { p[1] = &dco1_empty[d]; p[2] = &dco2_empty[d]; }
        if (d == 4) { p[1] = &dco1_empty[d]; p[2] = &fx_empty; }
        if (d == 5) p[1] = &voice;
        for (int s = 0; s < nslot[d]; s++)
            ((reg_fn)OS(0x40 + 4 * d))(p[s]);
    }
    for (int d = 0; d < 6; d++) {
        const dspdesc_t *p[3] = { &main_data[d], 0, 0 };
        if (d == 2 || d == 3) { p[1] = &dco1_data[d]; p[2] = &dco2_data[d]; }
        if (d == 4) { p[1] = &dco1_data[d]; p[2] = &fx_data; }
        if (d == 5) p[1] = &voice_data;
        for (int s = 0; s < nslot[d]; s++)
            ((reg_fn)OS(0x58 + 4 * d))(p[s]);
    }
}

/* level of the oscillator slots (0x32 DCO1, 0x33 DCO2), and the start of their programs: as engine.c */
static void dco_levels(int id, float v, int ramp)
{
    for (int d = 2; d <= 3; d++)
        for (int k = 0; k < 2; k++) {
            int last = !ramp && d == 3 && k == 1;
            ((ramp_fn)OS(0xA0))(d, id + 0x16 * k, ramp, last, last, v);
        }
}

static void swap_dco(int which)
{
    int id = which == 1 ? 0x32 : 0x33;
    dco_levels(id, 0.0f, 0);
    ((sync_fn)OS(0xC0))(3, 0, 0, 1);
    for (int d = 2; d <= 3; d++)
        if (which == 1)
            ((swap_fn)OS(0x70))(d, &dco1_empty[d], &dco1_data[d], swap_ctx, swap_ctx, SLOT_DCO1, 2, 0, 0);
        else
            ((swap_fn)OS(0x70))(d, &dco2_empty[d], &dco2_data[d], swap_ctx, swap_ctx, SLOT_DCO2, 3, 0, 0);
    dco_levels(id, 1.0f, 3);
}

static void e_start(engine_t *e)                                         /* vt+0x0C */
{
    (void)e;
    /* slot programs only run once +0x70 has enabled their task bit */
    swap_dco(1);
    swap_dco(2);
    ((swap_fn)OS(0x70))(4, &fx_empty, &fx_data, swap_ctx, swap_ctx, SLOT_FX, 3, 0x1F0, 0x80);
    ((sync_fn)OS(0xC0))(3, 0, 0, 1);
    ((swap_fn)OS(0x70))(5, &voice, &voice_data, swap_ctx, swap_ctx, SLOT_DCO1, 2, 0, 0);
}
static int  e_nop(void) { return 0; }
static void e_mute(engine_t *e) { (void)e; }                            /* vt+0x10 */
static void e_unmute(engine_t *e) { (void)e; }                          /* vt+0x18 */
static void e_refresh(engine_t *e, int all) { (void)e; (void)all; }     /* vt+0x24 */

/* monophonic voice: the last key of this slave's voices that is still held sounds */
static void note_on(engine_t *e, int v, int note)
{
    e->cur = v;
    p_bank(5, 0x21, (float)(note - 12) / 12.0f);
    p_bank(5, 0x22, 1.0f);
}
static void note_off(engine_t *e, int v)
{
    if (e->cur == v) {
        e->cur = -1;
        p_bank(5, 0x22, 0.0f);
    }
}

static void e_param(engine_t *e, int id, int zero, int value)            /* vt+0x30 */
{
    (void)zero;
    int v;
    if (id >= 0x429 && id < 0x429 + 8) {                 /* pitch (note) of voice id - 0x429 */
        v = id - 0x429 - e->first;
        if (v >= 0 && v < 4) e->note[v] = value;
        return;
    }
    if (id >= 0x43A && id < 0x43A + 8) {                 /* velocity of voice id - 0x43A: 0 = key up */
        v = id - 0x43A - e->first;
        if (v < 0 || v >= 4) return;
        if (value > 0) note_on(e, v, e->note[v]);
        else note_off(e, v);
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

/* ---- exports: descriptor and SYNINFO (as engine.c) ------------------------------- */

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
