/* C-S1JX region 0 (master): the JX-3P's own microcode, relocated by s1jx.py, on the SYSTEM-1 master DSPs (voices 0
 * and 1: MAIN2 + MAIN4 on DSP A, MAIN0 + DCO1 + our output tail on DSP B), the key assigner (4-voice poly, the one
 * of C-S1V4: examples/system_1/model.c) and the model object (load the master programs, VOLUME).
 * Docs: docs/examples_system_1_jx_README.md. The microcode comes from the user's plugins/JX-3P at build time.
 */
#define JX_CHIP 0
#include "jx.h"

#define NV 4

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
    if (a->v[v].vel) {
        vset(ID_VEL + v, 0);
        jx_gate(v, 0);
    }
    a->v[v].vel = 0;
}

static void play(struct assign *a, int v, int note, int vel)
{
    struct voice *x = &a->v[v];
    if (!jx.ready)
        jx_send_state();
    if (x->note != note)
        vset(ID_NOTE + v, note);
    if (!(x->vel == vel && vel))
        vset(ID_VEL + v, vel);
    jx_pitch(v, note);                  /* voices 0, 1 play here (master DSPs); jx.h drops the slave's writes */
    jx_gate(v, vel);
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

/* ---- model object: master DSP programs, state, VOLUME ------------------------------------------------- */

struct model { const void *const *vt; };

/* the sets as the SYSTEM-1 PLUG-OUTs queue them on the master in load (main, section ends, close). Never the
   frame header at 0x0000: Roland's engines keep it in their tables but queue from the next entry; queuing it
   hangs the device (C-S1JX7, 2026-10-22) */
static struct s1_prog set_master_a[] = {
    { 0x0100, MC_MA_MAIN_BYTES, mc_MA_MAIN },
    { 0x6DF2, MC_END_BYTES, mc_END },
    { 0x7FF2, MC_CLOSE_A01D_BYTES, mc_CLOSE_A01D },
    { 0, 0, 0 },
};
static struct s1_prog set_master_b[] = {
    { 0x5FF2, MC_END_BYTES, mc_END },
    { 0x6FF2, MC_END_BYTES, mc_END },
    { 0x1000, MC_MB_MAIN_BYTES, mc_MB_MAIN },
    { 0x7FF2, MC_CLOSE_A004_BYTES, mc_CLOSE_A004 },
    { 0, 0, 0 },
};

#ifdef JX_V4_PROGS
/* C-S1JX4 (V4_PROGS=1 build.sh): C-S1V4's DSP programs (measured good) with our ARM code and writes, to tell our
   relocated Roland programs from the rest */
#include "../system_1/build/s1_programs.h"
static struct s1_prog set_master_test[] = {
    { 0x1000, MC_S1MM1_BYTES, mc_S1MM1 },
    { 0, 0, 0 },
};
#elif defined(JX_SLAVE_TEST)
/* C-S1JXS (SLAVE_TEST=1 build.sh): the master runs only a minimal program (as C-S1V4's) that plays the slave's voices
   2 and 3; nothing of the JX-3P on the master DSPs. To tell a hang of the master programs from the rest. */
static struct s1_prog set_master_test[] = {
    { 0x1000, MC_MB_TEST_BYTES, mc_MB_TEST },
    { 0, 0, 0 },
};
#endif

static int m_nop(struct model *m) { (void)m; return 0; }

enum { ID_VOLUME = 0x100 };
static int volume = 255;

static void set_volume(int v)                       /* (v / 255)^2 -> C01C of master DSP B (the tail) */
{
    volume = v < 0 ? 0 : v > 255 ? 255 : v;
    float g = (float)volume / 255.0f;
    ((env_fn)SVC(0xEC))(1, M_VOLUME, fbits(g * g));
}

static void m_param(struct model *m, int id, int flag, int value)       /* vt+0x30 */
{
    (void)m; (void)flag;
    if (id == ID_VOLUME)
        set_volume(value);
}

static void m_load(struct model *m)                                      /* vt+0x08 */
{
    (void)m;
#if defined(JX_SLAVE_TEST) || defined(JX_V4_PROGS)
    ((s1_queue_fn)SVC(0x34))(set_master_test);
#else
    ((s1_queue_fn)SVC(0x30))(set_master_a);
    ((s1_queue_fn)SVC(0x34))(set_master_b);
#endif
    set_volume(volume);
    jx_send_state();
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
        jx_init();
        created = 1;
    }
}
static void destroy(void) {}
static void *get_assigner(void) { return &assigner; }
static void *get_model(void) { return &model; }

#ifndef S1_NAME
#define S1_NAME "C-S1JX"
#endif
#ifndef S1_MODEL_ID
#define S1_MODEL_ID 0x0011
#endif

__attribute__((section(".syninfo"), used))
const struct syninfo1 syninfo = SYNINFO1(S1_NAME, S1_MODEL_ID, 0x0001, create, destroy, get_assigner, get_model);

__attribute__((section(".patches"), used))
const uint32_t patches[NPATCH][128] = { { 0 } };
