/* SYSTEM-S1CC region 1 (the engine): runs on the SYSTEM-1 slave BMC (docs/examples_system_s1cc_README.md).
 * MUTABLE-SYSTEM's engine (examples/mutable_system/engine.c) without C-S1SY's panel: only what a kernel compiled by
 * s1cc needs. Freestanding, no library, no floating point (floats are bit patterns from make_s1cc.py tables).
 *
 *   load (vt+0x08)    voice sections of the OS off, then our programs on DSP A and DSP B; the knob tables
 *   param (vt+0x30)   0x110+v note -> pitch A030+v; 0x120+v velocity -> gate A038+v and, at a note start, the
 *                     trigger A060+v toggles (s1_trig); any other id: the kernel's S1_PARAM table with that
 *                     panel id -> DSP A parameters C070.. and / or DSP B C060..
 *                     HOST_ENVS (the kernel uses s1_env): AMP / FILTER A, D, S, R -> the host envelopes' DSP B
 *                     parameters (C040.., C044..; make_s1cc.py tables, defaults until a fader moves)
 *   refresh (vt+0x24) voices from the current values, knob tables from the last values received
 */
#include <stdint.h>
#include "syninfo.h"
#include "s1_programs.h"

typedef int (*get_fn)(int group, int id);
typedef void (*slot_fn)(int index, int on);
typedef void (*bank_fn)(int sel, const uint32_t *obj, uint32_t value);
typedef void (*param_fn)(int sel, int id, uint32_t value);

static int os_get(int g, int id) { return ((get_fn)SVC(0x00))(g, id); }

static void b5(int idx, uint32_t bits)                        /* bank 5 word A000 + idx (FD6C) */
{
    uint32_t obj[3] = { 0, 0, (uint32_t)idx };
    ((bank_fn)SVC(0x16C))(0, obj, bits);
}

static void b6(int sel, int id, uint32_t bits) { ((param_fn)SVC(0xEC))(sel, id, bits); }   /* C000 + id (FCEC) */

/* ---- programs -------------------------------------------------------------------------------- */

static struct s1_prog set_a[] = {
    { 0x0100, MC_S1M0_BYTES, mc_S1M0 },
    { 0x7FF2, MC_S1C0_BYTES, mc_S1C0 },
    { 0, 0, 0 },
};
static struct s1_prog set_b[] = {
    { 0x0100, MC_S1M1_BYTES, mc_S1M1 },
    { 0x7FF2, MC_S1C1_BYTES, mc_S1C1 },
    { 0, 0, 0 },
};

/* ---- the kernel's knob tables (S1_PARAM): row 128 until the control moves -------------------------- */

#if KPARAMS
static uint8_t kparam_knob[KPARAMS];

static void kparam_apply(int k)
{
    const uint32_t *t = kparam_bits + kparam_off[k] + (uint32_t)kparam_knob[k] * kparam_count[k];
    for (int i = 0; i < kparam_count[k]; i++) {
        if (kparam_a[k] != 0xFF)
            b6(0, kparam_a[k] + i, t[i]);
        if (kparam_b[k] != 0xFF)
            b6(1, kparam_b[k] + i, t[i]);
    }
}

static void kparam_set(int id, int value)
{
    for (int k = 0; k < KPARAMS; k++)
        if (kparam_panel[k] == id) {
            kparam_knob[k] = (uint8_t)(value < 0 ? 0 : value > 255 ? 255 : value);
            kparam_apply(k);
        }
}

static void kparam_all(void)
{
    for (int k = 0; k < KPARAMS; k++)
        kparam_apply(k);
}
#else
static void kparam_set(int id, int value) { (void)id; (void)value; }
static void kparam_all(void) {}
#endif

/* ---- the host's envelopes (HOST_ENVS: the kernel uses s1_env): AMP and FILTER faders -> DSP B parameters ---- */

#if HOST_ENVS
static uint8_t henv_knob[8];                   /* (S, A, D, R) of envelope 0, then of envelope 1 */

static void henv_apply(int k)
{
    int j = k & 3;                                 /* parameter order: sustain level, attack, decay, release rate */
    const uint32_t *t = j == 0 ? level_bits : j == 1 ? attack_bits : decay_bits;
    b6(1, (k < 4 ? H6_ENV0 : H6_ENV1) + j, t[henv_knob[k]]);
}

static void henv_set(int id, int value)
{
    for (int k = 0; k < 8; k++)
        if (henv_panel[k] == id) {
            henv_knob[k] = (uint8_t)(value < 0 ? 0 : value > 255 ? 255 : value);
            henv_apply(k);
        }
}

static void henv_all(void)
{
    for (int k = 0; k < 8; k++)
        henv_apply(k);
}
#else
static void henv_set(int id, int value) { (void)id; (void)value; }
static void henv_all(void) {}
#endif

/* ---- engine object ------------------------------------------------------------------------------ */

typedef struct engine {
    const void *const *vt;
    uint8_t note[NV], on[NV], trig[NV];
} engine_t;

static void voice_pitch(engine_t *e, int v, int note)
{
    e->note[v] = (uint8_t)(note & 0x7F);
    b5(B5_PITCH + v, pitch_bits[e->note[v]]);
}

static void voice_gate(engine_t *e, int v, int vel)
{
    int on = vel > 0;
    if (on) {
        b5(B5_PITCH + v, pitch_bits[e->note[v]]);      /* the note may have arrived after the gate */
        e->trig[v] ^= 1;                               /* s1_trig: toggles at every note start */
        b5(B5_TRIG + v, e->trig[v] ? F_ONE : F_ZERO);
        kparam_all();                                  /* again at every note on, in case a load lost them */
        henv_all();
    }
    e->on[v] = on;
    b5(B5_GATE + v, on ? F_ONE : F_ZERO);
}

static int e_nop(engine_t *e) { (void)e; return 0; }

static void e_init(engine_t *e)                                  /* vt+0x00 */
{
    for (int v = 0; v < NV; v++) { e->note[v] = 69; e->on[v] = 0; e->trig[v] = 0; }
#if KPARAMS
    for (int k = 0; k < KPARAMS; k++)
        kparam_knob[k] = 128;
#endif
#if HOST_ENVS
    for (int k = 0; k < 8; k++)
        henv_knob[k] = henv_default[k];
#endif
}

static void e_load(engine_t *e)                                  /* vt+0x08 */
{
    for (int i = 0; i < 8; i++)
        ((slot_fn)SVC(0x44))(i, 0);
    ((s1_queue_fn)SVC(0x30))(set_a);
    ((s1_queue_fn)SVC(0x34))(set_b);
    for (int v = 0; v < NV; v++)
        b5(B5_TRIG + v, e->trig[v] ? F_ONE : F_ZERO);
    kparam_all();
    henv_all();
}

static void e_refresh(engine_t *e, int all)                      /* vt+0x24 */
{
    (void)all;
    for (int v = 0; v < NV; v++) {
        voice_pitch(e, v, os_get(2, 0x110 + v));
        int vel = os_get(2, 0x120 + v);
        if ((vel > 0) != e->on[v])
            voice_gate(e, v, vel);
    }
    kparam_all();
    henv_all();
}

static void e_param(engine_t *e, int id, int flag, int value)    /* vt+0x30 */
{
    (void)flag;
    if (id >= 0x110 && id < 0x110 + NV) {
        voice_pitch(e, id - 0x110, value);
        return;
    }
    if (id >= 0x120 && id < 0x120 + NV) {
        voice_gate(e, id - 0x120, value);
        return;
    }
    kparam_set(id, value);
    henv_set(id, value);                       /* a fader may drive both (a kernel table and a host envelope) */
}

static const void *const engine_vt[22] = {
    [0] = (const void *)e_init,
    [1] = (const void *)e_nop,
    [2] = (const void *)e_load,
    [3] = (const void *)e_nop, [4] = (const void *)e_nop, [5] = (const void *)e_nop, [6] = (const void *)e_nop,
    [7] = (const void *)e_nop, [8] = (const void *)e_nop,
    [9] = (const void *)e_refresh,
    [10] = (const void *)e_nop, [11] = (const void *)e_nop,
    [12] = (const void *)e_param,
    [13] = (const void *)e_nop, [14] = (const void *)e_nop, [15] = (const void *)e_nop, [16] = (const void *)e_nop,
    [17] = (const void *)e_nop, [18] = (const void *)e_nop, [19] = (const void *)e_nop, [20] = (const void *)e_nop,
    [21] = (const void *)e_nop,
};

static engine_t engine = { .vt = engine_vt };

/* ---- exports ------------------------------------------------------------------------------------ */

static void create(void) {}
static void destroy(void) {}
static void *get_none(void) { return 0; }
static void *get_engine(void) { return &engine; }

#ifndef S1_NAME
#define S1_NAME "SYSTEM-S1CC"
#endif
#ifndef S1_MODEL_ID
#define S1_MODEL_ID 0x0010
#endif

__attribute__((section(".syninfo"), used))
const struct syninfo1 syninfo = SYNINFO1(S1_NAME, S1_MODEL_ID, 0x0001, create, destroy, get_none, get_engine);
