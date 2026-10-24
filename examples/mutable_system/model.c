/* MUTABLE-SYSTEM region 0: C-S1SY's model unchanged (examples/system_1_synth/model.c).
 * C-S1SY region 0 (the model; the same as C-S1V8's, examples/system_1/model.c): runs on the SYSTEM-1 master BMC, next to OS 1.30. Freestanding, no library, no
 * foreign code. Port of the C-V4 model (examples/from_scratch/model/model.c) to the SYSTEM-1 interface
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

#ifdef PARAMLOG
/* FX diagnostic (system-1/FX_CHAIN.md §6): the parameter blocks of the master DSPs as the OS's set_param writes them
 * (0x01008D26: sel 0 -> 0x4014C000, sel 1 -> 0x4016C000, + 16 id; words +0 mode, +4 / +8 value, +C 0). Sent as
 * param_log(0x3000 + (sel << 8) + id, word index, word) for B C000..C03F and A C000..C00F. */
static void fx_dump(void)
{
    for (int sel = 1; sel >= 0; sel--)
        for (int id = 0; id < (sel ? 0x40 : 0x10); id++) {
            volatile uint32_t *blk = (volatile uint32_t *)((sel ? 0x4016C000u : 0x4014C000u) + 16u * (uint32_t)id);
            for (int k = 0; k < 4; k++)
                param_log(0x3000 + (sel << 8) + id, k, (int)blk[k]);
        }
}
#endif

static void m_param(struct model *m, int id, int flag, int value)       /* vt+0x30 */
{
    (void)m; (void)flag;
#ifdef PARAMLOG
    param_log(id, flag, value);
    /* every control except the notes / velocities, the synth's own panel and the LFO ids */
    int known = (id >= 0x110 && id < 0x130) || (id >= 0x160 && id <= 0x18A);
    if (!known && id != ID_VOLUME && ((value & 63) == 0 || value == 255))
        fx_dump();
#endif
    if (id == ID_VOLUME)
        set_volume(value);
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
#define S1_NAME "MUTABLE-SYSTEM"
#endif
#ifndef S1_MODEL_ID
#define S1_MODEL_ID 0x0010
#endif

__attribute__((section(".syninfo"), used))
const struct syninfo1 syninfo = SYNINFO1(S1_NAME, S1_MODEL_ID, 0x0001, create, destroy, get_assigner, get_model);

/* the patch memory the OS copies at install: NPATCH patches of 128 words, all zero (our engine reads none) */
__attribute__((section(".patches"), used))
const uint32_t patches[NPATCH][128] = { { 0 } };
