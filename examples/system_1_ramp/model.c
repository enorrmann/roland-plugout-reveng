/* C-S1RAMP region 0: C-S1V4's model (examples/system_1/model.c) plus the measurement of the parameter ramp service
 * FCE4 (docs/examples_system_1_ramp_README.md): LFO RATE (0x162) starts a ramp of master DSP B parameter C030 with
 * rate y = knob / 16; LFO DELAY TIME (0x161) sets the mode z = knob / 32, LFO TO PITCH (0x163) the direction
 * (< 128 up 0 -> 1, else down 1 -> 0), LFO SHAPE (0x160) how the ARM reads bank 5 (0 the window, 1 / 2 FD78 sel
 * 0 / 1, 3 / 4 FD74 sel 0 / 1; >= 3 also starts the end-of-ramp chain, chain() below); every parameter the model receives sends a report (SysEx F0 7D 07, read by
 * ramp.py) of what master DSP B measured (make_s1.py prog_m1). Freestanding, no library, no foreign code.
 * The rest as C-S1V4: Port of the C-V4 model (examples/from_scratch/model/model.c) to the SYSTEM-1 interface
 * (docs/examples_system_1_README.md section 3):
 *
 *   SYNINFO +0x24 create, +0x28 destroy, +0x2C the key assigner, +0x30 the model object
 *   assigner  9-entry vtable: reset, changed, note_off, note_on, cc, part 0x81, bend, -, -
 *   model     13-entry vtable: init, deinit, load (vt+0x08: queue the master DSP programs), param (vt+0x30:
 *             VOLUME), the rest unused
 *   patches   64 x 0x200 bytes at base + 0x1000 (word k = patch id 0x160 + k); ours are all zero
 *
 * The assigner sends, through the master's parameter services at 0x0100FC00 (group 2 = voice ids):
 *   0x110+v note, 0x120+v velocity (0 = gate off), 0x130 bend, 0x131 modulation, 0x134 expression; v = 0..3.
 */
#include <stdint.h>
#include "syninfo.h"
#include "s1_programs.h"

#ifndef NV
#define NV 4
#endif

/* master parameter services */
typedef int (*get_fn)(int group, int id);
typedef void (*set_fn)(int group, int id, int value);
typedef void (*flush_fn)(int group);
typedef void (*touch_fn)(int group, int id);
static int p_get(int g, int id) { return ((get_fn)SVC(0x00))(g, id); }
static void p_set(int g, int id, int v) { ((set_fn)SVC(0x04))(g, id, v); }
static void p_flush(int g) { ((flush_fn)SVC(0x08))(g); }
static void p_touch(int g, int id) { ((touch_fn)SVC(0x0C))(g, id); }

enum { G_PART = 1, G_VOICE = 2, G_PATCH = 4 };
enum { ID_NOTE = 0x110, ID_VEL = 0x120, ID_BEND = 0x130, ID_MOD = 0x131, ID_EXPR = 0x134,
       ID_PART_80 = 0x80, ID_PART_81 = 0x81 };

/* ---- key assigner: 4-voice poly (C-V4's POLY rule) ----------------------------------------- */

struct voice { int8_t note; uint8_t vel, held, pad; };

struct assign {
    const void *const *vt;
    int hold, off_pending;
    uint32_t keys[4];
    struct voice v[NV];
    int8_t order[NV];          /* most recently used first */
};

static void vset(int id, int v) { p_set(G_VOICE, id, v); }

static void gate_off(struct assign *a, int v)
{
    if (a->v[v].vel)
        vset(ID_VEL + v, 0);
    a->v[v].vel = 0;
}

static void play(struct assign *a, int v, int note, int vel)
{
    struct voice *x = &a->v[v];
    if (x->note != note)
        vset(ID_NOTE + v, note);
    if (!(x->vel == vel && vel))
        vset(ID_VEL + v, vel);
    x->note = note; x->vel = vel; x->held = 0;
}

static void use(struct assign *a, int v)
{
    for (int k = NV - 1; k > 0; k--)
        if (a->order[k] == v) {
            for (; k > 0; k--)
                a->order[k] = a->order[k - 1];
            a->order[0] = v;
            return;
        }
}

static void clear_voices(struct assign *a)
{
    for (int v = 0; v < NV; v++) {
        a->v[v].note = -1; a->v[v].vel = 0; a->v[v].held = 0;
        a->order[v] = v;
    }
    for (int w = 0; w < 4; w++)
        a->keys[w] = 0;
}

static void release_all(struct assign *a)
{
    for (int v = 0; v < NV; v++)
        gate_off(a, v);
}

/* same note again > oldest free voice > oldest voice released under hold > oldest voice */
static void key(struct assign *a, int note, int vel)
{
    if (!vel) {
        for (int v = 0; v < NV; v++)
            if (a->v[v].vel && a->v[v].note == note) {
                if (a->hold) a->v[v].held = 1;
                else gate_off(a, v);
            }
        return;
    }
    int same = -1, idle = -1, held = -1, v;
    for (int k = NV - 1; k >= 0; k--) {
        int c = a->order[k];
        if (a->v[c].note == note) same = c;
        if (idle < 0 && !a->v[c].vel) idle = c;
        if (held < 0 && a->v[c].held == 1) held = c;
    }
    if (same >= 0) v = same;
    else if (idle >= 0) v = idle;
    else if (held >= 0) v = held;
    else v = a->order[NV - 1];
    gate_off(a, v);                     /* a new key always starts with a fresh gate */
    play(a, v, note, vel);
    use(a, v);
}

static void key_set(struct assign *a, int n, int down)
{
    uint32_t bit = 1u << (n & 31);
    if (down) a->keys[(n >> 5) & 3] |= bit;
    else a->keys[(n >> 5) & 3] &= ~bit;
}

static void all_notes_off(struct assign *a)
{
    if (a->hold) { a->off_pending = 1; return; }
    release_all(a);
    clear_voices(a);
}

static void hold(struct assign *a, int on)
{
    if (on) { a->hold = 1; return; }
    a->hold = 0;
    for (int v = 0; v < NV; v++)
        if (a->v[v].held == 1) {
            a->v[v].held = 0;
            gate_off(a, v);
        }
    if (a->off_pending) {
        release_all(a);
        clear_voices(a);
    }
    a->off_pending = 0;
}

static void a_reset(struct assign *a)
{
    a->hold = a->off_pending = 0;
    release_all(a);
    clear_voices(a);
}
static void a_changed(struct assign *a, int group) { (void)a; (void)group; }
static void a_note_off(struct assign *a, int note) { key_set(a, note, 0); key(a, note, 0); }
static void a_note_on(struct assign *a, int note, int vel) { key_set(a, note, vel != 0); key(a, note, vel); }

static void a_bend(struct assign *a, int v)                  /* 0..0x3FFF -> -0x2000..0x1FFF */
{
    (void)a;
    v -= 0x2000;
    if (v < -0x2000) v = -0x2000;
    else if (v >= 0x2000) v = 0x1FFF;
    vset(ID_BEND, v);
}

static void a_cc(struct assign *a, int cc, int v)
{
    switch (cc) {
    case 1: vset(ID_MOD, v); p_flush(G_PATCH); break;
    case 11: vset(ID_EXPR, v); break;
    case 64: hold(a, v >= 64); break;
    case 120: hold(a, 0); all_notes_off(a); break;
    case 121: vset(ID_MOD, 0); vset(ID_EXPR, 127); hold(a, 0); a_bend(a, 0x2000); break;
    case 123: case 124: case 125: case 126: case 127: all_notes_off(a); break;
    default: break;
    }
}

/* part parameter 0x81, as the SYSTEM-1 PLUG-OUTs handle it: the same value again only touches it, a new one
   (0..7) is stored as v + 8; then 0x80 = 1 and both groups are flushed */
static void a_part81(struct assign *a, int v)
{
    (void)a;
    if (v == p_get(G_PART, ID_PART_81))
        p_touch(G_PART, ID_PART_81);
    else if ((unsigned)v < 8)
        p_set(G_PART, ID_PART_81, v + 8);
    p_set(G_PART, ID_PART_80, 1);
    p_flush(G_PART);
    p_flush(0);
}

static void a_nop(struct assign *a) { (void)a; }

static const void *const assign_vt[10] = {
    (const void *)a_reset, (const void *)a_changed, (const void *)a_note_off, (const void *)a_note_on,
    (const void *)a_cc, (const void *)a_part81, (const void *)a_bend, (const void *)a_nop, (const void *)a_nop, 0,
};

static struct assign assigner = { .vt = assign_vt };

/* ---- model object: loads master DSP B, nothing else ------------------------------------------- */

struct model { const void *const *vt; };

/* master DSP B, section at 0x1000: slave 6006/6007 -> A118/A119 (the OS's output program on DSP A reads them;
   the OS queues its own PLUG-OUT-mode sets, with the section ends, just before calling load) */
static struct s1_prog set_master_b[] = {
    { 0x1000, MC_S1MM1_BYTES, mc_S1MM1 },
    { 0, 0, 0 },
};

static int m_nop(struct model *m) { (void)m; return 0; }

/* VOLUME knob: id 0x100 (0..255, measured with PARAMLOG 2026-10-22) -> parameter C01C of master DSP B, which our
   program there multiplies the output by (the SH-2 does the same with its own level). FCEC(sel 1 = DSP B, id,
   IEEE float bits) is the master's immediate parameter write. */
typedef void (*env_fn)(int sel, int id, uint32_t value);
enum { ID_VOLUME = 0x100 };
static int volume = 255;                                         /* full until the OS sends the knob */

static void set_volume(int v)
{
    volume = v < 0 ? 0 : v > 255 ? 255 : v;
    ((env_fn)SVC(0xEC))(1, M_VOLUME, volume_bits[volume]);
}

#ifdef PARAMLOG
/* Diagnostic build (PARAMLOG=1 build.sh, examples/system_1/paramlog.py): every parameter the OS hands to the model
 * (vt+0x30: id, flag, value) also goes to the PC as SysEx F0 7D 06 <id: 2 x 7 bits> <flag> <value: 5 x 7 bits> F7.
 * Sent the way OS 1.30 sends its own DT1 replies (0x60019D2A: send(0x605F20D4, buf, len, cable 3, mask 1)); only
 * once the OS has made that object (its guard byte 0x6009BF08 bit 0), otherwise the message is dropped. */
static void param_log(int id, int flag, int value)
{
    if (!(*(volatile uint8_t *)0x6009BF08u & 1))
        return;
    uint32_t v = (uint32_t)value;
    uint8_t buf[12] = { 0xF0, 0x7D, 0x06, (uint8_t)((id >> 7) & 0x7F), (uint8_t)(id & 0x7F), (uint8_t)(flag & 0x7F),
                        (uint8_t)((v >> 28) & 0x0F), (uint8_t)((v >> 21) & 0x7F), (uint8_t)((v >> 14) & 0x7F),
                        (uint8_t)((v >> 7) & 0x7F), (uint8_t)(v & 0x7F), 0xF7 };
    ((void (*)(void *, const void *, int, int, int))0x600065B1u)((void *)0x605F20D4u, buf, 12, 3, 1);
}
#endif

/* ---- the ramp measurement ------------------------------------------------------------------------ */

/* FCE4(sel, id, target bits, y, z) (master 1.30 0x01008CD4): parameter block 0x4014C000 / 0x4016C000 + 16 id:
   [+0] = 0x80000000 | z << 28 | y << 24, [+4] = target; with z != 0 the OS first puts its end-of-ramp function
   in 0x20003534 + 4 (+0x404 for DSP B) + 4 id. FCEC writes [+0] = 0xE0000000, [+4] = [+8] = value, [+0xC] = 0. */
typedef void (*ramp_fn)(int sel, int id, uint32_t target, int y, int z);
enum { ID_RATE = 0x162, ID_FADE = 0x161, ID_TOPITCH = 0x163, ID_SHAPE = 0x160 };
enum { R_V = 0x30, R_RUN = 0x31, R_SGN = 0x32, R_WORDS = 11 };
#define B5P(i) (*(volatile uint32_t *)(0x40101000u + 4u * (i)))         /* bank 5 A000 + i, the FD6C / FD78 window */
typedef uint32_t (*b5rd_fn)(int sel, const uint32_t *obj);       /* FD74 (through 0x40040550), FD78 (direct) */
static uint32_t b5_via(int how, int i)
{
    uint32_t obj[3] = { 0, 0, 0 };
    switch (how) {
    case 1: obj[2] = (uint32_t)i; return ((b5rd_fn)SVC(0x178))(0, obj);
    case 2: obj[2] = (uint32_t)(i - 0x100); return ((b5rd_fn)SVC(0x178))(1, obj);
    case 3: obj[2] = (uint32_t)i; return ((b5rd_fn)SVC(0x174))(0, obj);
    case 4: obj[2] = (uint32_t)(i - 0x100); return ((b5rd_fn)SVC(0x174))(1, obj);
    default: return B5P(i);
    }
}
static int r_how;                                                 /* LFO SHAPE (0x160): which read, 0..4 */
#define B5(i) b5_via(r_how, (i))
#define PBLK(k) (*(volatile uint32_t *)(0x4016C000u + 16u * R_V + 4u * (k)))
#define CBTAB (*(volatile uint32_t *)(0x20003534u + 0x404u + 4u * R_V))
static int r_y = -1, r_z, r_down, r_case;

/* the ARM times the ramp itself (bank 5 reads from the master's ARM give 0, 2026-10-29): it polls the live value,
   parameter block +8, with the Cortex-M3 cycle counter (DWT CYCCNT) for at most R_MAXMS ms (SysTick LOAD + 1 =
   cycles per ms if the OS ticks at 1 kHz, reported); r_t[k] = cycles until the value crossed TH[k] */
#define DEMCR (*(volatile uint32_t *)0xE000EDFCu)
#define DWT_CTRL (*(volatile uint32_t *)0xE0001000u)
#define CYCCNT (*(volatile uint32_t *)0xE0001004u)
#define SYST_CSR (*(volatile uint32_t *)0xE000E010u)
#define SYST_LOAD (*(volatile uint32_t *)0xE000E014u)
enum { R_MAXMS = 2500 };
static const uint32_t th_bits[8] = { 0x3C23D70Au, 0x3DCCCCCDu, 0x3E800000u, 0x3F000000u, 0x3F400000u, 0x3F666666u, 0x3F7D70A4u, 0x3F7FBE77u };
static const uint32_t thd_bits[8] = { 0x3F7D70A4u, 0x3F666666u, 0x3F400000u, 0x3F000000u, 0x3E800000u, 0x3DCCCCCDu, 0x3C23D70Au, 0x3A83126Fu };   /* down: 1 - TH[k] */
static uint32_t r_t[8], r_total, r_polls, r_first;

static void put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)((v >> 28) & 0x0F); p[1] = (uint8_t)((v >> 21) & 0x7F); p[2] = (uint8_t)((v >> 14) & 0x7F);
    p[3] = (uint8_t)((v >> 7) & 0x7F); p[4] = (uint8_t)(v & 0x7F);
}

/* F0 7D 07 <case> <y> <z> <down> <how> <bank 5 A1E0..A1EA> <block +0..+C> <end-of-ramp table entry> <the marker A1EA
   read the 5 ways> <A118, A119: the audio master B writes> F7, 5 x 7 bits per word */
static void ramp_report(void)
{
    if (!(*(volatile uint8_t *)0x6009BF08u & 1))
        return;
    uint8_t buf[9 + 5 * (R_WORDS + 5 + 5 + 2)];
    uint8_t *p = buf;
    *p++ = 0xF0; *p++ = 0x7D; *p++ = 0x07;
    *p++ = (uint8_t)(r_case & 0x7F); *p++ = (uint8_t)(r_y & 0x7F); *p++ = (uint8_t)r_z; *p++ = (uint8_t)r_down;
    *p++ = (uint8_t)r_how;
    uint32_t w[R_WORDS] = { PBLK(2), r_total, r_t[0], r_t[1], r_t[2], r_t[3], r_t[4], r_t[5], r_t[6], r_t[7],
                            SYST_LOAD };
    for (int k = 0; k < R_WORDS; k++, p += 5)
        put32(p, w[k]);
    for (int k = 0; k < 4; k++, p += 5)
        put32(p, PBLK(k));
    put32(p, CBTAB); p += 5;
    uint32_t x[7] = { SYST_CSR, *(volatile uint32_t *)0xE000ED00u, DWT_CTRL, r_polls, r_first,
                      b5_via(r_how, 0x118), b5_via(r_how, 0x119) };
    for (int k = 0; k < 7; k++, p += 5)
        put32(p, x[k]);
    *p++ = 0xF7;
    ((void (*)(void *, const void *, int, int, int))0x600065B1u)((void *)0x605F20D4u, buf, (int)(p - buf), 3, 1);
}

static void ramp_start(int y)
{
    env_fn set = (env_fn)SVC(0xEC);
    uint32_t start_v = r_down ? 0x3F800000u : 0, target = r_down ? 0 : 0x3F800000u;
    set(1, R_RUN, 0);
    set(1, R_SGN, r_down ? 0xBF800000u : 0x3F800000u);
    set(1, R_V, start_v);
    set(1, R_RUN, 0x3F800000u);
    r_y = y; r_case++;
    DEMCR |= 1u << 24;
    DWT_CTRL |= 1u;
    uint32_t per_ms = (SYST_LOAD & 0xFFFFFFu) + 1, limit = per_ms * R_MAXMS;
    if (per_ms < 1000 || limit / R_MAXMS != per_ms)
        limit = 0x7FFFFFFFu;
    for (int k = 0; k < 8; k++)
        r_t[k] = 0xFFFFFFFFu;
    r_first = 0xFFFFFFFFu;
    r_polls = 0;
    uint32_t t0 = CYCCNT, t = 0;
    ((ramp_fn)SVC(0xE4))(1, R_V, target, y, r_z);
    int left = 8;
    while (left && (t = CYCCNT - t0) < limit) {
        uint32_t v = PBLK(2);
        r_polls++;
        if (r_first == 0xFFFFFFFFu && v != start_v)
            r_first = t;
        for (int k = 0; k < 8; k++)
            if (r_t[k] == 0xFFFFFFFFu && (r_down ? v <= thd_bits[k] : v >= th_bits[k])) {
                r_t[k] = t;
                left--;
            }
    }
    r_total = t;
}

/* ---- the end-of-ramp chain (LFO SHAPE >= 3) ------------------------------------------------------------
   FE3C(sel, id, fn) (master 1.30 SVC 0x23C -> 0x01008CBC) puts fn in the end-of-ramp table (0x20003534 + 4 / + 0x404
   + 4 id, only if the entry is free; returns 1 / 0); FE40(sel, id) frees it. The interrupt handler 0x0100B008 (DSP B)
   reads the finished id from a hardware register (bits 4-11) and calls table[id](id) (0x01008A4C), which does not
   free the entry. Here fn starts the opposite ramp (z = 1: the end raises the interrupt) and notes the time: a
   triangle 0 <-> 1 that runs with no ARM tick. Report F0 7D 08 (ramp.py). */
typedef int (*cbreg_fn)(int sel, int id, void (*fn)(int));
typedef int (*cbfree_fn)(int sel, int id);
static volatile uint32_t c_count, c_t[8], c_reg, c_free;
static volatile int c_on, c_dir;
static volatile uint32_t c_amp = 0x3F800000u;     /* the triangle's top: LFO TO PITCH < 64: 1, < 128: 0.5, < 192: 0.25, else 0.125 */

static void chain_cb(int id)
{
    uint32_t t = CYCCNT;
    c_t[c_count & 7] = t;
    c_count++;
    if (!c_on)
        return;
    c_dir ^= 1;
    ((ramp_fn)SVC(0xE4))(1, id, c_dir ? c_amp : 0, r_y < 0 ? 0 : r_y, 1);
}

static void chain(int on)
{
    if (on == c_on)
        return;
    if (on) {
        DEMCR |= 1u << 24;
        DWT_CTRL |= 1u;
        c_count = 0;
        ((env_fn)SVC(0xEC))(1, R_V, 0);
        c_reg = (uint32_t)((cbreg_fn)SVC(0x23C))(1, R_V, chain_cb);
        c_on = 1;
        c_dir = 1;
        ((ramp_fn)SVC(0xE4))(1, R_V, c_amp, r_y < 0 ? 0 : r_y, 1);
    } else {
        c_on = 0;
        c_free = (uint32_t)((cbfree_fn)SVC(0x240))(1, R_V);
    }
}

/* F0 7D 08 <on> <y> <count> <registered> <freed> <the last 8 times (cycles)> <table entry> F7, 5 x 7 bits per word */
static void chain_report(void)
{
    if (!(*(volatile uint8_t *)0x6009BF08u & 1))
        return;
    uint8_t buf[6 + 5 * 13];
    uint8_t *p = buf;
    *p++ = 0xF0; *p++ = 0x7D; *p++ = 0x08;
    *p++ = (uint8_t)(c_on | (c_amp == 0x3F800000u ? 0 : c_amp == 0x3F000000u ? 2 : c_amp == 0x3E800000u ? 4 : 6));
    *p++ = (uint8_t)(r_y & 0x7F);
    uint32_t w[13] = { c_count, c_reg, c_free, c_t[0], c_t[1], c_t[2], c_t[3], c_t[4], c_t[5], c_t[6], c_t[7], CBTAB,
                       SYST_LOAD };
    for (int k = 0; k < 13; k++, p += 5)
        put32(p, w[k]);
    *p++ = 0xF7;
    ((void (*)(void *, const void *, int, int, int))0x600065B1u)((void *)0x605F20D4u, buf, (int)(p - buf), 3, 1);
}

static void m_param(struct model *m, int id, int flag, int value)       /* vt+0x30 */
{
    (void)m; (void)flag;
#ifdef PARAMLOG
    param_log(id, flag, value);
#endif
    if (id == ID_VOLUME)
        set_volume(value);
    else if (id == ID_FADE)
        r_z = (value >> 5) & 7;
    else if (id == ID_TOPITCH) {
        r_down = value >= 128;
        c_amp = value < 64 ? 0x3F800000u : value < 128 ? 0x3F000000u : value < 192 ? 0x3E800000u : 0x3E000000u;
    }
    else if (id == ID_SHAPE) {
        r_how = value > 4 ? 4 : value;
        chain(value >= 3);
    }
    if (c_on || c_count) {                                          /* the chain: its own report */
        if (id == ID_RATE)
            r_y = value >> 4;                                       /* the next ramps' rate */
        chain_report();
        return;
    }
    ramp_report();                                                  /* the previous case, as it is now */
    if (id == ID_RATE && (value >> 4) != r_y)
        ramp_start(value >> 4);
}
static void m_load(struct model *m)                           /* vt+0x08 */
{
    (void)m;
    ((s1_queue_fn)SVC(0x34))(set_master_b);
    set_volume(volume);
}

static const void *const model_vt[13] = {
    (const void *)m_nop, (const void *)m_nop, (const void *)m_load, (const void *)m_nop, (const void *)m_nop,
    (const void *)m_nop, (const void *)m_nop, (const void *)m_nop, (const void *)m_nop, (const void *)m_nop,
    (const void *)m_nop, (const void *)m_nop, (const void *)m_param,
};

static struct model model = { model_vt };

/* ---- exports ------------------------------------------------------------------------------------ */

static int created;
static void create(void)
{
    if (!created) {
        clear_voices(&assigner);
        created = 1;
    }
}
static void destroy(void) {}
static void *get_assigner(void) { return &assigner; }
static void *get_model(void) { return &model; }

#ifndef S1_NAME
#define S1_NAME "C-S1RAMP"
#endif
#ifndef S1_MODEL_ID
#define S1_MODEL_ID 0x0010
#endif

__attribute__((section(".syninfo"), used))
const struct syninfo1 syninfo = SYNINFO1(S1_NAME, S1_MODEL_ID, 0x0001, create, destroy, get_assigner, get_model);

/* the patch memory the OS copies at install: NPATCH patches of 128 words, all zero (our engine reads none) */
__attribute__((section(".patches"), used))
const uint32_t patches[NPATCH][128] = { { 0 } };
