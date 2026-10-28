/* SYSTEM-S1CC region 1 (the engine): runs on the SYSTEM-1 slave BMC (docs/examples_system_s1cc_README.md).
 * MUTABLE-SYSTEM's engine (examples/mutable_system/engine.c) without C-S1SY's panel: only what a kernel compiled by
 * s1cc needs. Freestanding, no library, no floating point (floats are bit patterns from make_s1cc.py tables).
 *
 *   load (vt+0x08)    voice sections of the OS off, then our programs on DSP A and DSP B; the knob tables
 *   param (vt+0x30)   0x110+v note -> pitch A030+v, s1_dt A040+v, s1_rdt A048+v; 0x120+v velocity -> gate A038+v and, at a note start, the
 *                     trigger A060+v toggles (s1_trig); any other id: the kernel's S1_PARAM table with that
 *                     panel id -> DSP A parameters C070.. and / or DSP B C060..
 *                     HOST_ENVS (the kernel uses s1_env): AMP / FILTER A, D, S, R -> the host envelopes' DSP B
 *                     parameters (C040.., C044..; make_s1cc.py tables, defaults until a fader moves)
 *                     HOST_LFO (the kernel reads the pitch, s1_lfo_filter / s1_lfo_amp or s1_color): LFO SHAPE /
 *                     RATE / DELAY TIME / TO PITCH / TO FILTER / TO AMP, PITCH ENV A / D, OSC1 / OSC2 COLOR and COLOR
 *                     MOD -> DSP A parameters C040.. (make_s1cc.py hlfo_voice)
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

/* ---- the host (HOST_LFO): LFO (DSP B, hlfo_voice_b), pitch envelope, COLOR MOD, mixer (DSP A, hlfo_voice) -------- */

#if HOST_LFO
enum { C_LFO_SHAPE, C_LFO_RATE, C_LFO_DELAY, C_LFO_PITCH, C_LFO_FILTER, C_LFO_AMP, C_PENV_A, C_PENV_D,
       C_COLOR0, C_COLOR1, C_MOD0, C_MOD1, C_SUB, C_NOISE, C_PENV_AMT, C_RANGE1, C_N };   /* make_s1cc.py HLFO's order */
static uint8_t hlfo_knob[C_N];

static void hlfo_color(int i)          /* weights (manual, LFO, pitch env, filter env, amp env, sub): knob x source */
{
    int k = hlfo_knob[C_COLOR0 + i], src = hlfo_knob[C_MOD0 + i] > 5 ? 5 : hlfo_knob[C_MOD0 + i];
    for (int j = 0; j < 6; j++) {      /* the LFO and the sub are -1..1: knob x (0.5 + 0.5 x) */
        uint32_t w = src == 1 || src == 5 ? (j == 0 || j == src ? half_bits[k] : F_ZERO)
                                          : j == src ? level_bits[k] : F_ZERO;
        b6(0, (i ? A6_COL1 : A6_COL0) + j, w);
    }
}

static void hlfo_apply(int c)
{
    int k = hlfo_knob[c];
    switch (c) {
    case C_LFO_SHAPE:                  /* alpha, beta, K, w (make_s1cc.py LFO_SHAPES); S&H: the glide follows at once */
    case C_LFO_RATE: {
        int sh = hlfo_knob[C_LFO_SHAPE] > 5 ? 5 : hlfo_knob[C_LFO_SHAPE], r = hlfo_knob[C_LFO_RATE];
        for (int j = 0; j < 4; j++)    /* the LFO runs on DSP B (sel 1) */
            b6(1, B6_LWAVE + j, lfo_shape_bits[4 * sh + j]);
        b6(1, B6_LRATE, lfo_rate_bits[r]);
        b6(1, B6_LSG, sh == 5 ? F_ONE : lfo_sg_bits[r]);
        break;
    }
    case C_LFO_DELAY: b6(1, B6_LDELAY, lfo_delay_bits[k]); break;
    case C_LFO_PITCH: b6(0, A6_LPITCH, lfo_pitch_bits[k]); break;
    case C_LFO_FILTER: b6(1, B6_LFILT, lfo_filt_bits[k]); break;
    case C_LFO_AMP:
        b6(1, B6_LAMP, lfo_amp_bits[k]);
        b6(1, B6_LAMPABS, lfo_ampabs_bits[k]);
        break;
    case C_PENV_AMT: b6(0, A6_PAMT, penv_amt_bits[k]); break;   /* PITCH ENV AMOUNT -> pitch (octaves) */
    case C_RANGE1: b6(0, A6_RANGE1, range_bits[k > 5 ? 5 : k]); break;   /* OSC1 RANGE 64'..2' -> -3..+2 octaves */
    case C_PENV_A: b6(0, A6_PENV + 1, attack_bits[k]); break;
    case C_PENV_D: b6(0, A6_PENV, decay_bits[k]); break;
    case C_COLOR0: case C_MOD0: hlfo_color(0); break;
    case C_COLOR1: case C_MOD1: hlfo_color(1); break;
    case C_SUB: b6(0, A6_SUBMIX, half_bits[k]); break;           /* the host mixer: SUB and NOISE levels 0 .. 0.5 */
    case C_NOISE: b6(0, A6_NOISEMIX, half_bits[k]); break;
    default: break;
    }
}

static void hlfo_set(int id, int value)
{
    for (int c = 0; c < C_N; c++)
        if (hlfo_panel[c] == id) {
            hlfo_knob[c] = (uint8_t)(value < 0 ? 0 : value > 255 ? 255 : value);
            hlfo_apply(c);
        }
}

static void hlfo_all(void)
{
    for (int c = 0; c < C_N; c++)
        hlfo_apply(c);
}
#else
static void hlfo_set(int id, int value) { (void)id; (void)value; }
static void hlfo_all(void) {}
#endif

/* ---- sections (make_s1cc.py SECTIONS: the voices a DSP's main program does not hold) ------------------------------ */

#if NSECT
/* a section is placed the way the stock synth places its voice programs: FC40(index, program set, three data sets),
   here with no data (the stock synth: 0x6004ecb4 -> FC40(i, voice set, data set, empty, empty); FC40 = 0x60003F22
   queues them with the section number on its own queue, 0x600602d8). A program queued with FC30 at 0x1900 instead
   was copied but did not run (2026-10-28) */
typedef void (*place_fn)(int index, const struct s1_prog *prog, const struct s1_prog *d1, const struct s1_prog *d2,
                         const struct s1_prog *d3);
static struct s1_prog set_none[] = {
    { 0, 0, 0 },
};
#endif

static void section_on(void)          /* FC44(index, 1) clears the section's "off" bit (0x60003F96). Again at every
                                          refresh and note on, as the knob tables: cheap, and safe if the OS cleared it */
{
#if NSECT
    for (int i = 0; i < NSECT; i++)
        ((slot_fn)SVC(0x44))(sect_index[i], 1);
#endif
}

/* ---- engine object ------------------------------------------------------------------------------ */

typedef struct engine {
    const void *const *vt;
    uint8_t note[NV], on[NV], trig[NV];
} engine_t;

static void voice_note_words(engine_t *e, int v)      /* pitch, s1_dt, s1_rdt of the voice's note */
{
    b5(B5_PITCH + v, pitch_bits[e->note[v]]);
    b5(B5_DT + v, dt_bits[e->note[v]]);
    b5(B5_RDT + v, rdt_bits[e->note[v]]);
}

static void voice_pitch(engine_t *e, int v, int note)
{
    e->note[v] = (uint8_t)(note & 0x7F);
    voice_note_words(e, v);
}

static void voice_gate(engine_t *e, int v, int vel)
{
    int on = vel > 0;
    if (on) {
        voice_note_words(e, v);                        /* the note may have arrived after the gate */
        e->trig[v] ^= 1;                               /* s1_trig: toggles at every note start */
        b5(B5_TRIG + v, e->trig[v] ? F_ONE : F_ZERO);
        kparam_all();                                  /* again at every note on, in case a load lost them */
        henv_all();
        hlfo_all();
        section_on();
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
#if HOST_LFO
    for (int c = 0; c < C_N; c++)
        hlfo_knob[c] = hlfo_default[c];
#endif
}

static void e_load(engine_t *e)                                  /* vt+0x08 */
{
    for (int i = 0; i < 8; i++)
        ((slot_fn)SVC(0x44))(i, 0);
    ((s1_queue_fn)SVC(0x30))(set_a);
    ((s1_queue_fn)SVC(0x34))(set_b);
#if NSECT
    for (int i = 0; i < NSECT; i++)            /* the sections of both DSPs, after the mains */
        ((place_fn)SVC(0x40))(sect_index[i], sect_set[i], set_none, set_none, set_none);
#endif
    section_on();
    for (int v = 0; v < NV; v++)
        b5(B5_TRIG + v, e->trig[v] ? F_ONE : F_ZERO);
    kparam_all();
    henv_all();
    hlfo_all();
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
    hlfo_all();
    section_on();
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
    hlfo_set(id, value);                       /* COLOR: a kernel table too, if the kernel has one on it */
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
