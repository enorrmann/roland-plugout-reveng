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
 *   LUT_N             LFO_LUT / ENV_CURVE: the bank-5 tables (A000 + LUT0_IDX ..) at load and refresh
 *   ARM_ENV / ARM_LFO the host's ADSRs / LFO on this ARM: ramped DSP B parameters, stepped by a hook in the
 *                     slave OS's 1 ms cyclic handler (tick); deinit (vt+0x04) takes it out
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
#if CMOD_IDX
    /* indexed (make_s1cc.py colors_b_idx): w0, w, the source's offset; the LFO and the sub are -1..1 */
    uint32_t w0 = src == 0 ? level_bits[k] : src == 1 || src == 5 ? half_bits[k] : F_ZERO;
    uint32_t w = src == 0 ? F_ZERO : src == 1 || src == 5 ? half_bits[k] : level_bits[k];
    b6(M6_SEL, (i ? M6_COL1 : M6_COL0), w0);
    b6(M6_SEL, (i ? M6_COL1 : M6_COL0) + 1, w);
    b6(M6_SEL, (i ? M6_COL1 : M6_COL0) + 2, cmod_off_bits[src]);
    return;
#endif
    for (int j = 0; j < 6; j++) {      /* the LFO and the sub are -1..1: knob x (0.5 + 0.5 x) */
        uint32_t w = src == 1 || src == 5 ? (j == 0 || j == src ? half_bits[k] : F_ZERO)
                                          : j == src ? level_bits[k] : F_ZERO;
        b6(M6_SEL, (i ? M6_COL1 : M6_COL0) + j, w);
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
    case C_PENV_A: b6(M6_SEL, M6_PENV + 1, attack_bits[k]); break;
    case C_PENV_D: b6(M6_SEL, M6_PENV, decay_bits[k]); break;
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

/* ---- ARM_ENV / ARM_LFO: the host's ADSRs and LFO on this ARM (docs/examples_system_s1cc_README.md §3.7) ---------
   Each envelope and each voice's LFO is one DSP B parameter that the hardware ramps (FCE4 with y = 0: 4.00 ms per
   ramp, whatever the distance, docs/examples_system_1_ramp_README.md). The time base is the slave OS's cyclic handler
   (driver\cyclic_handle.c: the RTOS cyclic handler 0x01008983, period "1" (its time unit unknown), calls its only client, the function at
   0x20002BFC): load puts tick() there, which calls the OS's function first, measures the time since the last step
   with the cycle counter and advances every voice by that many ramps of 4 ms, with a ramp as long (2026-10-30: the end-of-ramp functions, FE3C, run on the master but
   are never called on the slave; the cyclic hook was heard on the device). Integer only (no FPU on the SYSTEM-1's
   ARM; the SYSTEM-8's has one): levels in Q24 (1 = 1 << 24), tables from make_s1cc.py. */

#if ARM_ENV || ARM_LFO || ARM_TICK
typedef void (*ramp_fn)(int sel, int id, uint32_t target, int y, int z);
typedef void (*cyc_fn)(int i);
#define Q1 (1 << 24)
#define CYC_CLIENT (*(volatile cyc_fn *)0x20002BFCu)

static uint32_t q24_bits(int32_t q)                    /* Q24 -> IEEE float bits */
{
    if (q == 0)
        return 0;
    uint32_t sign = q < 0 ? 0x80000000u : 0, m = (uint32_t)(q < 0 ? -q : q);
    int e = 31 - __builtin_clz(m);                     /* m = 1.f x 2^e */
    uint32_t frac = e > 23 ? m >> (e - 23) : m << (23 - e);
    return sign | (uint32_t)(e - 24 + 127) << 23 | (frac & 0x7FFFFFu);
}

static int32_t qmul(int32_t a, int32_t b) { return (int32_t)(((int64_t)a * b) >> 24); }

static int ramp_y;                                     /* the ramp's rate: as long as the time between two steps */

static void ramp(int id, int32_t q)                    /* to q in one ramp of ramp_y, no interrupt */
{
#if ARM_RAMP
    ((ramp_fn)SVC(0xE4))(1, id, q24_bits(q), ramp_y, 0);
#else
    b6(1, id, q24_bits(q));                            /* ARM_RAMP=0: at once (FCEC) */
#endif
}
#endif

#if ARM_ENV
/* C-S1SY's ADSR (make_s1cc.py adsr) one ramp at a time: attack toward 1.25 until 1, decay toward S, release toward
   0; the one-pole's factor over a ramp from the faders (env_att_seg / env_dec_seg) */
enum { ST_IDLE, ST_ATT, ST_DEC, ST_REL };
static volatile uint8_t env_st[2][NV];
static int32_t env_lvl[2][NV];                         /* the level the running ramp ends at */

static void env_step(int i, int v, int m)              /* m ramps of 4 ms at once (the time since the last step) */
{
    const uint8_t *k = &henv_knob[4 * i];              /* S, A, D, R */
    int32_t l = env_lvl[i][v], top = Q1 + Q1 / 4, sl = (int32_t)(((uint32_t)k[0] << 24) / 255u);
    switch (env_st[i][v]) {
    case ST_ATT:
        for (int j = 0; j < m; j++)
            l = top - qmul(top - l, (int32_t)env_att_seg[k[1]]);
        if (l >= Q1) {
            l = Q1;
            env_st[i][v] = ST_DEC;
        }
        break;
    case ST_DEC:
        for (int j = 0; j < m; j++)
            l = sl + qmul(l - sl, (int32_t)env_dec_seg[k[2]]);
        break;
    case ST_REL:
        for (int j = 0; j < m; j++)
            l = qmul(l, (int32_t)env_dec_seg[k[3]]);
        if (l < (Q1 >> 14)) {                          /* -84 dB: the last ramp, to 0 */
            l = 0;
            env_st[i][v] = ST_IDLE;
        }
        break;
    default:
        return;
    }
    env_lvl[i][v] = l;
    ramp(P_ENV0 + 8 * i + v, l);
}
#endif

#if ARM_LFO
/* the DSP version's LFO (make_s1cc.py hlfo_voice_b) one ramp at a time: phase restarted at the note start, S&H of a
   random value at each wrap, sample & glide, shapes sine t (1.5 - 0.5 t^2) / triangle / saw / square / S&G / S&H,
   fade from the gate (DELAY TIME); lf = lfo x fade -> P_LFO0 + v, fade -> P_FADE0 + v */
static uint32_t lfo_ph[NV], lfo_rnd = 0x12345678u;
static int32_t lfo_h[NV], lfo_g[NV], lfo_fade[NV];
static volatile uint8_t lfo_on[NV], lfo_restart[NV];

static int32_t lfo_shape(int sh, uint32_t ph, int v)
{
    int32_t p = (int32_t)ph >> 7;                      /* the phase -1..1 */
    int32_t t = p > Q1 / 2 ? 2 * Q1 - 2 * p : p < -Q1 / 2 ? -2 * Q1 - 2 * p : 2 * p;   /* ph_tri: +1 at 0.5 */
    switch (sh) {
    case 0: return qmul(t, Q1 + Q1 / 2 - qmul(qmul(t, t), Q1 / 2));
    case 1: return t;
    case 2: return p;
    case 3: return p > 0 ? Q1 : p < 0 ? -Q1 : 0;
    default: return lfo_g[v];
    }
}

static void lfo_step(int v, int m)                     /* m sixteenths of 4 ms at once */
{
    int sh = hlfo_knob[C_LFO_SHAPE] > 5 ? 5 : hlfo_knob[C_LFO_SHAPE], r = hlfo_knob[C_LFO_RATE];
    if (lfo_restart[v]) {                              /* a note start: the phase from 0 (as on DSP B) */
        lfo_restart[v] = 0;
        lfo_ph[v] = 0;
    }
    uint32_t old = lfo_ph[v];
    lfo_ph[v] = old + (lfo_seg_inc[r] >> 4) * (uint32_t)m;
    if ((int32_t)old >= 0 && (int32_t)lfo_ph[v] < 0) {   /* wrapped: a new random value */
        lfo_rnd = lfo_rnd * 1664525u + 1013904223u;
        lfo_h[v] = (int32_t)lfo_rnd >> 7;
    }
    if (sh == 5) {
        lfo_g[v] = lfo_h[v];
    } else {                                           /* the glide over m sixteenths (first order) */
        int32_t k = (int32_t)(((int64_t)lfo_sg_seg[r] * m) >> 4);
        lfo_g[v] += qmul(lfo_h[v] - lfo_g[v], k > Q1 ? Q1 : k);
    }
#if ARM_FADE
    if (lfo_on[v]) {
        lfo_fade[v] += (int32_t)((lfo_fade_seg[hlfo_knob[C_LFO_DELAY]] >> 4) * (uint32_t)m);
        if (lfo_fade[v] > Q1)
            lfo_fade[v] = Q1;
    } else {
        lfo_fade[v] = 0;
    }
#else
    lfo_fade[v] = Q1;                                  /* ARM_FADE=0: no DELAY TIME */
#endif
    ramp(P_FADE0 + v, lfo_fade[v]);
    ramp(P_LFO0 + v, qmul(lfo_shape(sh, lfo_ph[v], v), lfo_fade[v]));
}
#endif

#if ARM_ENV || ARM_LFO || ARM_TICK
static cyc_fn cyc_os;                                  /* the OS's client function, called first */
static volatile uint32_t tick_n;
#define DEMCR (*(volatile uint32_t *)0xE000EDFCu)
#define DWT_CTRL (*(volatile uint32_t *)0xE0001000u)
#define CYCCNT (*(volatile uint32_t *)0xE0001004u)
#define SYST_LOAD (*(volatile uint32_t *)0xE000E014u)
static uint32_t tick_last, tick_acc, seg_cycles, step_last, step_min;
/* the ramp durations of y = 0..15 in 1/100 ms (docs/examples_system_1_ramp_README.md §3) */
static const uint16_t y_cms[16] = { 398, 813, 1220, 1625, 2008, 2438, 2843, 3250, 3591, 4014, 4877, 5684, 6496, 7181,
                                    8025, 9744 };

static void tick(int i)                                /* the cyclic handler, every period */
{
    if (cyc_os)
        cyc_os(i);
    if (CYC_CLIENT != tick) {                          /* the OS's function replaced itself: keep its new one */
        cyc_os = CYC_CLIENT;
        CYC_CLIENT = tick;
    }
    uint32_t n = ++tick_n;
#if ARM_TICK
    if ((n & 3) == 0)                                  /* diagnostic: a buzz on DSP B (ARM_DIAG adds C11F) */
        b6(1, 0x11F, (n & 4) ? F_ONE : F_ZERO);
#endif
    /* the time since the last step, measured (the cyclic handler's period is not known), in sixteenths of 4 ms */
    uint32_t now = CYCCNT, unit = seg_cycles >> 4;
    tick_acc += now - tick_last;
    tick_last = now;
    int m = 0;
    while (tick_acc >= unit && m < 16 * 24) {
        tick_acc -= unit;
        m++;
    }
    if (tick_acc > 16 * 24 * unit)                     /* a long gap (load): forget it */
        tick_acc = 0;
#if ARM_RAMP
    if (m < 16)                                        /* with ramps: steps of 4 ms at least */
        { tick_acc += (uint32_t)m * unit; return; }
#endif
    if (!m)
        return;
    /* the ramp: shorter than the shortest recent time between steps (85 %), so that it ends before the next one (a
       new ramp before the end jumped: 2026-10-30, the ticks come irregularly); the minimum forgets slowly */
    uint32_t gap = now - step_last;
    step_last = now;
    step_min = gap < step_min ? gap : step_min + (step_min >> 5);
    uint32_t lim = step_min / (seg_cycles / 400) * 85 / 100;   /* in 1/100 ms (32-bit division: no library) */
    int y = 0;
    while (y < 15 && y_cms[y + 1] <= lim)
        y++;
    ramp_y = y;
    for (int v = 0; v < NV; v++) {
#if ARM_LFO
        lfo_step(v, m);
#endif
#if ARM_ENV
        env_step(0, v, (m + 8) >> 4);                  /* (whole ramps; envelope on the tick: next step) */
        env_step(1, v, (m + 8) >> 4);
#endif
    }
}

static void arm_start(void)                            /* load: clear, hook the cyclic handler */
{
    for (int v = 0; v < NV; v++) {
#if ARM_ENV
        for (int i = 0; i < 2; i++) {
            env_st[i][v] = ST_IDLE;
            env_lvl[i][v] = 0;
            b6(1, P_ENV0 + 8 * i + v, F_ZERO);
        }
#endif
#if ARM_LFO
        lfo_ph[v] = 0;
        lfo_fade[v] = lfo_g[v] = lfo_h[v] = 0;
        b6(1, P_LFO0 + v, F_ZERO);
        b6(1, P_FADE0 + v, F_ZERO);
#endif
    }
    DEMCR |= 1u << 24;                                 /* the cycle counter */
    DWT_CTRL |= 1u;
    uint32_t per_ms = (SYST_LOAD & 0xFFFFFFu) + 1;      /* SysTick at 1 kHz (the master: 156 000, 156 MHz) */
    seg_cycles = 4 * (per_ms >= 10000 && per_ms <= 1000000 ? per_ms : 156000u);
    tick_last = step_last = CYCCNT;
    tick_acc = 0;
    step_min = seg_cycles;
    if (CYC_CLIENT != tick) {
        cyc_os = CYC_CLIENT;
        CYC_CLIENT = tick;
    }
}

static void arm_stop(void)                             /* deinit: the OS's function back */
{
    if (CYC_CLIENT == tick)
        CYC_CLIENT = cyc_os;
}

static void arm_gate(int v, int on)
{
#if ARM_DIAG
    static int g;
    g ^= 1;
    b6(1, 0x11F, g ? F_ONE : F_ZERO);                  /* diagnostic: a click at every note on / off */
#endif
#if ARM_LFO
    if (on)
        lfo_restart[v] = 1;
    lfo_on[v] = (uint8_t)on;
#endif
#if ARM_ENV
    for (int i = 0; i < 2; i++)                        /* the next tick takes the voice on from its level */
        env_st[i][v] = on ? ST_ATT : env_st[i][v] == ST_IDLE ? ST_IDLE : ST_REL;
#endif
}
#else
static void arm_start(void) {}
static void arm_stop(void) {}
static void arm_gate(int v, int on) { (void)v; (void)on; }
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
    arm_gate(v, on);
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

#if LUT_N
static void lut_all(void)       /* LFO_LUT / ENV_CURVE: the bank-5 tables the DSP B reads with indexed loads */
{
    for (int i = 0; i < LUT_N; i++)
        b5(LUT0_IDX + i, lut_bits[i]);
}
#else
static void lut_all(void) {}
#endif

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
    lut_all();
    for (int v = 0; v < NV; v++)
        b5(B5_TRIG + v, e->trig[v] ? F_ONE : F_ZERO);
    kparam_all();
    henv_all();
    hlfo_all();
    arm_start();
}

static int e_deinit(engine_t *e)                                 /* vt+0x04: our end-of-ramp functions out */
{
    (void)e;
    arm_stop();
    return 0;
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
    lut_all();
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
    [1] = (const void *)e_deinit,
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
