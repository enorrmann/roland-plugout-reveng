/* C-S1SY region 1 (the engine): runs on the SYSTEM-1 slave BMC. Freestanding, no library, no foreign code, no
 * floating point (no FPU: floats are bit patterns from make_synth.py tables, or built from integers by fbits()).
 * C-S1V8's engine (examples/system_1/engine.c) plus the panel (docs/examples_system_1_synth_README.md):
 *
 *   load (vt+0x08)    voice sections of the OS off, then our programs on DSP A and DSP B
 *   param (vt+0x30)   0x110+v note -> pitch A030+v, 0x120+v velocity -> gate A038+v and, at a note start, the
 *                     trigger A060+v toggles (the DSP envelopes restart); panel ids (PARAMLOG 2026-10-23):
 *                       0x167 / 0x16C  OSC1 / OSC2 wave (0..5)     0x174 / 0x175  OSC1 / OSC2 level
 *                       0x17D cutoff  0x17F resonance  0x181 env amount  0x186 key track
 *                       0x182..0x185 filter A D S R    0x187..0x18A amp A D S R          (all 0..255)
 *   refresh (vt+0x24) voices from the current values, panel words from the last values received
 */
#include <stdint.h>
#include "syninfo.h"
#include "s1_programs.h"

typedef int (*get_fn)(int group, int id);
typedef void (*slot_fn)(int index, int on);
typedef void (*bank_fn)(int sel, const uint32_t *obj, uint32_t value);

static int os_get(int g, int id) { return ((get_fn)SVC(0x00))(g, id); }

static void b5(int idx, uint32_t bits)                        /* bank 5 word A000 + idx */
{
    uint32_t obj[3] = { 0, 0, (uint32_t)idx };
    ((bank_fn)SVC(0x16C))(0, obj, bits);
}

/* IEEE bits of n * 2^e (n < 2^31) */
static __attribute__((unused)) uint32_t fbits(uint32_t n, int e)
{
    if (!n)
        return 0;
    int msb = 31 - __builtin_clz(n);
    uint32_t m = msb > 23 ? n >> (msb - 23) : n << (23 - msb);
    return (uint32_t)(msb + e + 127) << 23 | (m & 0x7FFFFF);
}

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

/* ---- panel ------------------------------------------------------------------------------------- */
/* STEP 0 = the all-at-once design (silent on hw); STEP 1 = C-S1V8's engine; STEP >= 2: the panel words the step
   uses (make_synth.py STEPS), written at load and again at every note on (in case a load loses them) */
#if STEP == 0

enum { P_W1, P_W2, P_L1, P_L2, P_CUT, P_RES, P_AMT, P_KT, P_FA, P_FD, P_FS, P_FR, P_AA, P_AD, P_AS, P_AR, P_N };

static const uint16_t panel_id[P_N] = {
    0x167, 0x16C, 0x174, 0x175, 0x17D, 0x17F, 0x181, 0x186,
    0x182, 0x183, 0x184, 0x185, 0x187, 0x188, 0x189, 0x18A,
};

/* until the knobs move: saw + saw/2, filter open, organ-like amp envelope with a short release */
static uint8_t panel[P_N] = { 0, 0, 255, 128, 255, 0, 0, 0, 0, 128, 0, 60, 0, 128, 255, 60 };

static void osc_words(int base, int wave, int level)
{
    if (wave > 5)
        wave = 5;
    for (int k = 0; k < 3; k++)              /* coef / 256 x level / 255 x 0.5 ~ coef x level x 257 x 2^-25 */
        b5(base + (k ? k + 1 : 0), fbits((uint32_t)wave_coef[wave][k] * (uint32_t)level * 257u, -25));
    b5(base + 1, wave_thr_bits[wave]);
}

static void env_words(int base, int a, int d, int s, int r)
{
    b5(base + 0, level_bits[s]);
    b5(base + 1, attack_bits[a]);
    b5(base + 2, decay_bits[d]);
    b5(base + 3, decay_bits[r]);
}

static void panel_apply(int p)
{
    const uint8_t *v = panel;
    switch (p) {
    case P_W1: case P_L1: osc_words(B5_OSC1, v[P_W1], v[P_L1]); break;
    case P_W2: case P_L2: osc_words(B5_OSC2, v[P_W2], v[P_L2]); break;
    case P_CUT: b5(B5_CUT, cutoff_bits[v[P_CUT]]); break;
    case P_RES: b5(B5_RES, res_bits[v[P_RES]]); b5(B5_COMP, comp_bits[v[P_RES]]); break;
    case P_AMT: b5(B5_AMT, amount_bits[v[P_AMT]]); break;
    case P_KT: b5(B5_KT, level_bits[v[P_KT]]); break;
    case P_FA: case P_FD: case P_FS: case P_FR: env_words(B5_FENV, v[P_FA], v[P_FD], v[P_FS], v[P_FR]); break;
    case P_AA: case P_AD: case P_AS: case P_AR: env_words(B5_AENV, v[P_AA], v[P_AD], v[P_AS], v[P_AR]); break;
    default: break;
    }
}

static void panel_all(void)
{
    b5(B5_OFF2, DCO2_OFF_BITS);
    for (int p = 0; p < P_N; p++)
        panel_apply(p);
}
#define TRIGGERS 1
#elif STEP >= 2
/* step 2: OSC levels -> bank 5 A040 / A041 (read by DSP A); step 4: amp ADSR -> parameters C040..C043 of DSP B
   (FCEC(1, id, bits), the way Roland's SH-2 hands panel values to a slave DSP); step 5: OSC ranges and OSC2 fine
   tune -> parameters C044..C046 of DSP A (FCEC(0, ...)) */
typedef void (*param_fn)(int sel, int id, uint32_t value);
static void b6(int sel, int id, uint32_t bits) { ((param_fn)SVC(0xEC))(sel, id, bits); }

enum { P_L1, P_L2, P_AA, P_AD, P_AS, P_AR, P_R1, P_R2, P_F2, P_N };
static const uint16_t panel_id[P_N] = { 0x174, 0x175, 0x187, 0x188, 0x189, 0x18A, 0x16A, 0x16F, 0x170 };
static uint8_t panel[P_N] = { 255, 128, 0, 128, 255, 60, 3, 3, 128 };

static void panel_apply(int p)
{
    const uint8_t *v = panel;
    switch (p) {
    case P_L1: b5(B5_LEVEL1, level_bits[v[P_L1]]); break;
    case P_L2: b5(B5_LEVEL2, level_bits[v[P_L2]]); break;
#if STEP >= 5
    case P_R1: b6(0, B6_RANGE1, range_bits[v[P_R1] > 5 ? 5 : v[P_R1]]); break;    /* step 5: DSP A parameters */
    case P_R2: b6(0, B6_RANGE2, range_bits[v[P_R2] > 5 ? 5 : v[P_R2]]); break;
    case P_F2: b6(0, B6_FINE2, fine_bits[v[P_F2]]); break;
#else
    case P_R1: case P_R2: case P_F2: break;
#endif
    default:
        if (STEP >= 4) {
            b6(1, B6_AENV + 0, level_bits[v[P_AS]]);
            b6(1, B6_AENV + 1, attack_bits[v[P_AA]]);
            b6(1, B6_AENV + 2, decay_bits[v[P_AD]]);
            b6(1, B6_AENV + 3, decay_bits[v[P_AR]]);
        }
        break;
    }
}

static void panel_all(void)
{
    for (int p = 0; p < P_N; p++)
        panel_apply(p);
}
#define TRIGGERS (STEP >= 4)
#else
#define P_N 0
static const uint16_t panel_id[1];
static uint8_t panel[1];
static void panel_apply(int p) { (void)p; }
static void panel_all(void) {}
#define TRIGGERS 0
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
        if (TRIGGERS) {
            e->trig[v] ^= 1;                           /* every note on restarts the envelopes */
            b5(B5_TRIG + v, e->trig[v] ? F_ONE : F_ZERO);
        }
        if (STEP >= 2)
            panel_all();
    }
    e->on[v] = on;
    b5(B5_GATE + v, on ? F_ONE : F_ZERO);
}

static int e_nop(engine_t *e) { (void)e; return 0; }

static void e_init(engine_t *e)                                  /* vt+0x00 */
{
    for (int v = 0; v < NV; v++) { e->note[v] = 69; e->on[v] = 0; e->trig[v] = 0; }
}

static void e_load(engine_t *e)                                  /* vt+0x08 */
{
    for (int i = 0; i < 8; i++)
        ((slot_fn)SVC(0x44))(i, 0);
    ((s1_queue_fn)SVC(0x30))(set_a);
    ((s1_queue_fn)SVC(0x34))(set_b);
    if (TRIGGERS)
        for (int v = 0; v < NV; v++)
            b5(B5_TRIG + v, e->trig[v] ? F_ONE : F_ZERO);
    panel_all();
}

static void e_refresh(engine_t *e, int all)                      /* vt+0x24 */
{
    (void)all;
    for (int v = 0; v < NV; v++) {
        voice_pitch(e, v, os_get(2, 0x110 + v));
        int vel = os_get(2, 0x120 + v);
        if (STEP == 1 || (vel > 0) != e->on[v])       /* STEP 1: exactly C-S1V8 */
            voice_gate(e, v, vel);
    }
    panel_all();
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
    for (int p = 0; p < P_N; p++)
        if (panel_id[p] == id) {
            panel[p] = (uint8_t)(value < 0 ? 0 : value > 255 ? 255 : value);
            panel_apply(p);
            return;
        }
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
#define S1_NAME "C-S1SY"
#endif
#ifndef S1_MODEL_ID
#define S1_MODEL_ID 0x0010
#endif

__attribute__((section(".syninfo"), used))
const struct syninfo1 syninfo = SYNINFO1(S1_NAME, S1_MODEL_ID, 0x0001, create, destroy, get_none, get_engine);
