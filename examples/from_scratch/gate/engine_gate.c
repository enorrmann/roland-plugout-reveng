/*
 * Gate plug-out engine (region 1, slave BMCs). Our own ARM code and our own DSP microcode, no Roland
 * programs, no envelope, no LFO. The only signal path is DSP2 -> DSP4 (dsp/make_chain.py):
 *   DSP2 DCO1 slot: SAW at the pitch of bank 5 id 0x21, output to MXM Z 0x404 (DSP4 reads it as M 0x005)
 *   DSP4 DCO1 slot: SAW * gate of bank 5 id 0x22, output to IOMD 6020 / 6021 (the wave bus)
 * Every other slot is kept, empty (as in engine.c), so the OS runs the Roland layout around the two programs.
 * Notes: one monophonic voice per slave, the last key still held sounds, gate 1 while it is held.
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

/* DCO levels (0x32 DCO1, 0x33 DCO2) of DSP2 and DSP3, as mini/ and engine.c: muted before the swap, then ramped up */
static void dco_levels(int id, float v, int ramp)
{
    for (int d = 2; d <= 3; d++)
        for (int k = 0; k < 2; k++) {
            int last = !ramp && d == 3 && k == 1;
            ((ramp_fn)OS(0xA0))(d, id + 0x16 * k, ramp, last, last, v);
        }
}

static void p_bank(int dsp, uint32_t id, float v) { ((bank_fn)OS(0xB0))(dsp, &id, v); }
static int slave_index(void) { return ((slave_fn)OS(0xF0))() ? 1 : 0; }

enum { SLOT_MAIN = 0x0100, SLOT_DCO1 = 0x6000, SLOT_DCO2 = 0xB000, SLOT_FX = 0xB000, SLOT_X = 0x8000 };
#define PROG(a, name) { a, MC_##name##_BYTES, MC_##name##_BYTES, mc_##name, { 0, 0 } }
#define EMPTY(a) { a, 0xe, 0xe, mc_empty, { 0, 0 } }
#define DATA(a, n) { a, n, n, zero_data, { 0, 0 } }
#define D8 DATA(0x0008, 8)
#define D10 DATA(0x0010, 8)
#define DFX DATA(0x16e8, 8)

static const uint32_t mc_empty[4] = { 0x0a02ff70, 0x0a02ff70, 0x0b0200f8, 0x00010000 };
static const uint32_t zero_data[8];

static dspdesc_t main_prog[6] = { EMPTY(SLOT_MAIN), EMPTY(SLOT_MAIN), EMPTY(SLOT_MAIN),
                                  EMPTY(SLOT_MAIN), EMPTY(SLOT_MAIN), EMPTY(SLOT_MAIN) };
static dspdesc_t main_data[6] = { DATA(0, 8), DATA(0, 8), DATA(0, 8), DATA(0, 8), DATA(0, 8), DATA(0, 8) };
/* every slot of the Roland layout stays (dsp/ rule: a removed component leaves its slot empty) */
static dspdesc_t x_empty = EMPTY(SLOT_X);                     /* DSP0, DSP1: envelope/LFO slot, empty */
static dspdesc_t dco1_empty = EMPTY(SLOT_DCO1);
static dspdesc_t dco2_empty = EMPTY(SLOT_DCO2);
static dspdesc_t dco2_null_prog = PROG(SLOT_DCO2, DCO2NULL);   /* DSP2/3 DCO2: our SAW, no output */
static dspdesc_t fx_empty = EMPTY(SLOT_FX);                   /* DSP4 effect slot, empty */
static dspdesc_t osc_prog = PROG(SLOT_DCO1, OSC);             /* DSP2 DCO1: SAW */
static dspdesc_t gate_prog = PROG(SLOT_DCO1, GATE);           /* DSP4 DCO1: gate */
static dspdesc_t osc_data = DATA(0x0008, 0x20);               /* ring state of the saw at 0x670 */
static dspdesc_t gate_data = DATA(0x0008, 8);
static dspdesc_t dco1_data = D8, dco2_data = D10, fx_data = DFX;
#ifdef T_VOICE5             /* diagnostic: the mini voice (our own, dsp/voice.hex) on DSP5 DCO1 as in C-MINI2 */
static dspdesc_t voice_prog = PROG(SLOT_DCO1, VOICE);
static dspdesc_t voice_data = DATA(0x0008, 0x20);
#endif
static uint32_t swap_ctx[8] = { 0, 0, 0xFFFFFFFFu };

typedef struct { const void *const *vt; int slave, first; int note[4]; int cur; } engine_t;

static void e_init(engine_t *e) { e->slave = slave_index(); e->first = 4 * e->slave; e->cur = -1; }
static int  e_nop(void) { return 0; }

static void e_load(engine_t *e)                                         /* vt+0x08 */
{
    /* same slots as engine.c: DSP0/1 X, DSP2/3 DCO1+DCO2, DSP4 DCO1+FX, DSP5 DCO1. Slots, then programs, then data,
       DSP by DSP (OS +0x28 / +0x40 / +0x58 + 4 * dsp). The programs we run are swapped in by e_start. */
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
        const dspdesc_t *p[3] = { &main_prog[d], d < 2 ? &x_empty : &dco1_empty, 0 };
        if (d == 2 || d == 3) { p[1] = &dco1_empty; p[2] = &dco2_empty; }
        if (d == 4) p[2] = &fx_empty;
        for (int s = 0; s < nslot[d]; s++)
            ((reg_fn)OS(0x40 + 4 * d))(p[s]);
    }
    for (int d = 0; d < 6; d++) {
        const dspdesc_t *p[3] = { &main_data[d], 0, 0 };
        if (d == 2 || d == 3) { p[1] = &dco1_data; p[2] = &dco2_data; }
        if (d == 4) { p[1] = &dco1_data; p[2] = &fx_data; }
        if (d == 5) p[1] = &dco1_data;
        for (int s = 0; s < nslot[d]; s++)
            ((reg_fn)OS(0x58 + 4 * d))(p[s]);
    }
}

static void e_start(engine_t *e)                                         /* vt+0x0C */
{
    (void)e;
    /* start from zero, as send_state() of engine.c does for the voices: pitch and gate of each voice */
    p_bank(2, 0x21, 0.0f); p_bank(2, 0x22, 0.0f);
    p_bank(4, 0x21, 0.0f); p_bank(4, 0x22, 0.0f);
    p_bank(5, 0x21, 0.0f); p_bank(5, 0x22, 0.0f);
    /* slot programs only run once +0x70 has enabled their task bit: the empty ones too, as in engine.c */
    dco_levels(0x32, 0.0f, 0);
    dco_levels(0x33, 0.0f, 0);
    ((sync_fn)OS(0xC0))(3, 0, 0, 1);
    for (int d = 2; d <= 3; d++) {
        ((swap_fn)OS(0x70))(d, &dco1_empty, &dco1_data, swap_ctx, swap_ctx, SLOT_DCO1, 2, 0, 0);
        ((swap_fn)OS(0x70))(d, &dco2_null_prog, &dco2_data, swap_ctx, swap_ctx, SLOT_DCO2, 3, 0, 0);
    }
    ((swap_fn)OS(0x70))(4, &fx_empty, &fx_data, swap_ctx, swap_ctx, SLOT_FX, 3, 0x1F0, 0x80);
#ifdef T_VOICE5
    ((swap_fn)OS(0x70))(5, &voice_prog, &voice_data, swap_ctx, swap_ctx, SLOT_DCO1, 2, 0, 0);
#endif
    ((sync_fn)OS(0xC0))(3, 0, 0, 1);
#ifndef T_NO_OSC_PROG       /* diagnostic: DSP2 DCO1 stays empty */
    ((swap_fn)OS(0x70))(2, &osc_prog, &osc_data, swap_ctx, swap_ctx, SLOT_DCO1, 2, 0, 0);
    /* DSP3 DCO1 too: an empty DCO slot on DSP2/3 silenced the synth (WAVE_BUS 4g); our SAW fills it */
    ((swap_fn)OS(0x70))(3, &osc_prog, &osc_data, swap_ctx, swap_ctx, SLOT_DCO1, 2, 0, 0);
#endif
#ifndef T_NO_GATE_PROG      /* diagnostic: DSP4 DCO1 stays empty */
    ((swap_fn)OS(0x70))(4, &gate_prog, &gate_data, swap_ctx, swap_ctx, SLOT_DCO1, 2, 0, 0);
#endif
    dco_levels(0x32, 1.0f, 3);
    dco_levels(0x33, 1.0f, 3);
}
static void e_mute(engine_t *e) { (void)e; }                            /* vt+0x10 */
static void e_unmute(engine_t *e) { (void)e; }                          /* vt+0x18 */
static void e_refresh(engine_t *e, int all) { (void)e; (void)all; }     /* vt+0x24 */

/* monophonic voice: the last key of this slave's voices that is still held sounds */
static void note_on(engine_t *e, int v, int note)
{
    e->cur = v;
    p_bank(2, 0x21, (float)(note - 12) / 12.0f);     /* pitch: 440 Hz at note 69 (p = 4.75) */
    p_bank(4, 0x22, 1.0f);                           /* gate open */
}
static void note_off(engine_t *e, int v)
{
    if (e->cur == v) {
        e->cur = -1;
        p_bank(4, 0x22, 0.0f);                       /* gate closed */
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

/* ---- exports: descriptor and SYNINFO ------------------------------------------------ */

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
