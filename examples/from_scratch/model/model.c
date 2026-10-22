/* Region 0 (the model) of the from-scratch PLUG-OUT: runs on the master BMC
 * (core 0, next to the OS). Freestanding, no library, no foreign code.
 *
 * What the OS needs from region 0 (see docs/NOTES.md):
 *   SYNINFO +0x24  init(): returns the descriptor; called many times
 *   SYNINFO +0x28  parameter record list (model_list.h, from params.py)
 *   base+0x8000    slot memory: patches and edit buffers (storage.bin)
 *   descriptor     [0] slot (OS writes it), [1] create, [2] destroy,
 *                  [3]/[4] assigner objects of part A/B, [5] 0, [6] select
 *   assigner       C++-style object, 17-entry vtable (see struct assign_vt)
 *
 * The assigner turns keys into voice parameters through the kernel's
 * parameter services at 0x01000400 (group 2 = voice level ids):
 *   0x429+v note, 0x43A+v velocity (0 = gate off), 0x44B+v portamento
 *   legato flag, 0x45C = voice with the highest sounding note, 0x465 bend,
 *   0x467 modulation, 0x46A expression; voices 4..7 for the upper split half.
 */
#include <stdint.h>
#include "model_list.h"

#define NV_MAX 8

/* kernel parameter services (0x01000400); the group argument is passed
 * through but get/set only use part and id */
struct os_param {
    int (*get)(int part, int group, int id);
    void (*set)(int part, int group, int id, int value);
    void (*flush)(int part, int group);
    void (*touch)(int part, int group, int id);
};
#define OS ((const struct os_param *)0x01000400)

enum { G_PART = 1, G_VOICE = 2, G_PATCH = 4 };
enum { ID_NOTE = 0x429, ID_VEL = 0x43A, ID_LEGATO = 0x44B, ID_TOP = 0x45C,
       ID_BEND = 0x465, ID_MOD = 0x467, ID_EXPR = 0x46A, ID_PART_22D = 0x22D,
       ID_PORTA_TIME = 0x6FD, ID_PORTA_MODE = 0x6FE, ID_ASSIGN = 0x6FF };
enum { POLY = 0, MONO = 1, UNISON = 2 };

struct voice {
    int8_t note;          /* -1 = none */
    uint8_t vel;          /* 0 = gate off */
    uint8_t held;         /* released while the hold pedal is down */
    uint8_t pad;
};

struct assign;
struct assign_vt {
    void (*reset)(struct assign *);                       /*  0 */
    void (*changed)(struct assign *, int group);          /*  1 */
    void (*note_off)(struct assign *, int note);          /*  2 */
    void (*note_on)(struct assign *, int note, int vel);  /*  3 */
    void (*cc)(struct assign *, int cc, int value);       /*  4 */
    void (*part_22d)(struct assign *, int value);         /*  5 */
    void (*bend)(struct assign *, int value);             /*  6 */
    void (*nop7)(struct assign *);                        /*  7 */
    void (*nop8)(struct assign *);                        /*  8 */
    void (*set)(struct assign *, int group, int id, int value);   /*  9 */
    int (*get)(struct assign *, int group, int id, int *value);   /* 10 */
    void (*touch)(struct assign *, int group, int id);    /* 11 */
    void (*flush)(struct assign *, int group);            /* 12 */
    void (*other_cc)(struct assign *, int cc, int value); /* 13 */
    void (*init)(struct assign *, int voices);            /* 14 */
    void (*set_voices)(struct assign *, int voices);      /* 15 */
    int (*voices)(struct assign *);                       /* 16 */
};

/* +4 (voice count) and +0xA0 (split mode) sit where the stock object keeps
 * them, in case the OS reads them directly */
struct assign {
    const struct assign_vt *vt;
    int n;                    /* +0x04 voices */
    uint32_t all;             /* voice mask */
    int key_assign;           /* POLY / MONO / UNISON (patch 0x6FF) */
    int porta_mode;           /* patch 0x6FE */
    int hold;
    int off_pending;          /* all-notes-off arrived while holding */
    uint32_t legato;          /* voices whose 0x44B flag is set */
    uint32_t glide;           /* voices that follow the next note's pitch */
    int top;                  /* last value sent to 0x45C */
    uint32_t keys[4];         /* keys down, bit per MIDI note */
    struct voice v[NV_MAX];
    int8_t order[NV_MAX];     /* most recently used first */
    uint8_t pad[0xA0 - 0x60];
    int mode;                 /* +0xA0 split mode 0..4 */
};
_Static_assert(__builtin_offsetof(struct assign, mode) == 0xA0, "layout");
_Static_assert(sizeof(struct assign) == 0xA4, "layout");

/* ---- parameter access ------------------------------------------------- */

/* split mode -> part: 0 whole, 1/3 lower, 2/4 upper */
static int part_of(const struct assign *a)
{
    static const uint8_t part[5] = { 0, 1, 2, 1, 2 };
    return (unsigned)a->mode < 5 ? part[a->mode] : 0;
}

static void m_set(struct assign *a, int g, int id, int v) { OS->set(part_of(a), (int16_t)g, (int16_t)id, v); }
static int m_get(struct assign *a, int g, int id, int *v) { *v = OS->get(part_of(a), (int16_t)g, (int16_t)id); return 1; }
static void m_touch(struct assign *a, int g, int id) { OS->touch(part_of(a), (int16_t)g, (int16_t)id); }
static void m_flush(struct assign *a, int g) { OS->flush(part_of(a), g); }

#ifdef VOICELOG
/* Diagnostic build (VOICELOG=1 model/build.sh, examples/from_scratch/voicelog/): every note and velocity the
 * assigner sets for a voice is sent to the PC as SysEx F0 7D 05 <kind> <voice> <value> <note> F7, kind 0 = note
 * (value = note), 1 = velocity (value = velocity, note = the note the voice plays), voice 0..7 = the global voice id
 * (slave 0: 0-3, slave 1: 4-7). Only the velocity message carries the note: a note message sent right before it
 * was not received. Same send path as PEEK. */
static int vlog_note;                                                  /* note of the voice being set */
static void vlog(int kind, int voice, int value)
{
    uint8_t buf[8] = { 0xF0, 0x7D, 0x05, (uint8_t)kind, (uint8_t)voice, (uint8_t)(value & 0x7F),
                       (uint8_t)(vlog_note & 0x7F), 0xF7 };
    uint32_t msg[4], wrap[4];
    ((void (*)(void *))0x600753CDu)(msg);                              /* Msg ctor */
    ((uint8_t *)msg)[6] = 4;                                           /* cable 4 */
    ((void (*)(void *, const void *, int))0x600754BDu)(msg, buf, 8);   /* set SysEx */
    ((void (*)(void *, void *))0x600754C7u)(wrap, msg);                /* wrap_msg */
    ((void (*)(int, void *))0x6000345Du)(0x1A20, wrap);                /* router: USB1 + DIN */
    ((void (*)(void *))0x600753F5u)(wrap);                             /* Wrap dtor */
    ((void (*)(void *))0x600753E1u)(msg);                              /* Msg dtor */
}
#endif

static void vset(struct assign *a, int id, int v)
{
#ifdef VOICELOG
    if (id >= ID_NOTE && id < ID_NOTE + 8) vlog(0, id - ID_NOTE, v);
    if (id >= ID_VEL && id < ID_VEL + 8) vlog(1, id - ID_VEL, v);
#endif
    a->vt->set(a, G_VOICE, id, v);
}
static int pget(struct assign *a, int id, int *v) { return a->vt->get(a, G_PATCH, id, v); }

/* voice ids: the upper split half (mode 2) uses voices 4..7 */
static int vid(const struct assign *a, int base, int v) { return base + v + (a->mode == 2 ? 4 : 0); }

/* ---- voices ------------------------------------------------------------- */

static void gate_off(struct assign *a, int v)          /* only if sounding */
{
    if (a->v[v].vel)
        vset(a, vid(a, ID_VEL, v), 0);
    a->v[v].vel = 0;
}

static void gate_off_always(struct assign *a, int v)
{
    vset(a, vid(a, ID_VEL, v), 0);
    a->v[v].vel = 0;
}

static void play(struct assign *a, int v, int note, int vel)
{
    struct voice *x = &a->v[v];
    if (x->note != note)
        vset(a, vid(a, ID_NOTE, v), note);
#ifdef VOICELOG
    vlog_note = note;
#endif
    if (!(x->vel == vel && vel))
        vset(a, vid(a, ID_VEL, v), vel);
    x->note = note; x->vel = vel; x->held = 0;
}

static void retune(struct assign *a, int v, int note)  /* pitch only, gate unchanged */
{
    if (a->v[v].note != note)
        vset(a, vid(a, ID_NOTE, v), note);
    a->v[v].note = note; a->v[v].held = 0;
}

/* apply fn to the voices of mask (bit v = voice v) */
#define FOR_VOICES(a, mask, v) \
    for (uint32_t m_ = (mask), v = 0; (int)v < (a)->n && m_; v++, m_ >>= 1) if (m_ & 1)

static void release(struct assign *a, uint32_t mask)
{
    FOR_VOICES(a, mask, v) gate_off(a, v);
}

static void release_always(struct assign *a, uint32_t mask)
{
    FOR_VOICES(a, mask, v) gate_off_always(a, v);
}

static void play_all(struct assign *a, uint32_t mask, int note, int vel)
{
    FOR_VOICES(a, mask, v) play(a, v, note, vel);
}

static void retune_all(struct assign *a, uint32_t mask, int note)
{
    FOR_VOICES(a, mask, v) retune(a, v, note);
}

static void legato_on(struct assign *a, uint32_t mask)
{
    FOR_VOICES(a, mask, v) vset(a, vid(a, ID_LEGATO, v), 1);
    a->legato |= mask;
}

static void legato_off(struct assign *a, uint32_t mask)
{
    FOR_VOICES(a, mask, v) vset(a, vid(a, ID_LEGATO, v), 0);
    a->legato &= ~mask;
}

static int none_sounding(const struct assign *a)
{
    for (int v = 0; v < a->n; v++)
        if (a->v[v].vel)
            return 0;
    return 1;
}

static void use(struct assign *a, int v)                /* move v to the front */
{
    for (int k = a->n - 1; k > 0; k--)
        if (a->order[k] == v) {
            for (; k > 0; k--)
                a->order[k] = a->order[k - 1];
            a->order[0] = v;
            return;
        }
}

/* tell the engine which sounding voice has the highest note. on = v was just
 * started (it wins ties; voices released under hold do not count) */
static void update_top(struct assign *a, int v, int on)
{
    int top = v, first = 1;
    for (int k = 0; k < a->n; k++) {
        const struct voice *x = &a->v[k];
        if (!x->vel)
            continue;
        if (!on && first) { top = k; first = 0; continue; }
        if (x->note > a->v[top].note && !(on && a->hold && x->held == 1))
            top = k;
    }
    if (a->top != top) {
        a->top = top;
        vset(a, ID_TOP, top + (a->mode == 2 ? 4 : 0));
    }
}

static void key_set(struct assign *a, int n, int down)
{
    uint32_t bit = 1u << (n & 31);
    if (down) a->keys[(n >> 5) & 3] |= bit;
    else a->keys[(n >> 5) & 3] &= ~bit;
}

static int lowest_key(const struct assign *a)
{
    for (int w = 0; w < 4; w++)
        if (a->keys[w])
            for (int b = 0; b < 32; b++)
                if (a->keys[w] & (1u << b))
                    return w * 32 + b;
    return -1;
}

static void clear_voices(struct assign *a)
{
    for (int v = 0; v < NV_MAX; v++) {
        a->v[v].note = -1; a->v[v].vel = 0; a->v[v].held = 0;
    }
    a->glide = 0;
    for (int w = 0; w < 4; w++)
        a->keys[w] = 0;
}

static void init_voices(struct assign *a, int n)
{
    if (n > NV_MAX) n = NV_MAX;
    a->n = n;
    a->all = 0;
    for (int v = 0; v < n; v++)
        a->all |= 1u << v;
    a->key_assign = a->porta_mode = a->hold = a->off_pending = 0;
    a->legato = 0;
    for (int k = 0; k < n; k++)
        a->order[k] = k;
    a->top = -1;
    clear_voices(a);
}

/* ---- key assign modes --------------------------------------------------- */

/* POLY: same note again > oldest free voice > oldest voice released under
 * hold > (portamento on: the newest voice, else the oldest one) */
static void poly(struct assign *a, int note, int vel, int porta)
{
    if (!vel) {
        if (a->hold) {
            for (int v = 0; v < a->n; v++)
                if (a->v[v].vel && a->v[v].note == note) {
                    a->v[v].held = 1;
                    update_top(a, v, 0);
                }
            return;
        }
        uint32_t mask = 0;
        int last = 0;
        for (int v = 0; v < a->n; v++)
            if (a->v[v].vel && a->v[v].note == note) {
                a->v[v].held = 0;
                mask |= 1u << v;
                last = v;
            }
        if (mask) {
            release(a, mask);
            update_top(a, last, 0);
        }
        return;
    }
    int same = -1, idle = -1, held = -1, v;
    for (int k = a->n - 1; k >= 0; k--) {
        int c = a->order[k];
        if (a->v[c].note == note) same = c;
        if (idle < 0 && !a->v[c].vel) idle = c;
        if (held < 0 && a->v[c].held == 1) held = c;
    }
    if (same >= 0) v = same;
    else if (idle >= 0) v = idle;
    else if (held >= 0) v = held;
    else v = porta ? a->order[0] : a->order[a->n - 1];

    if (a->porta_mode && porta) {
        /* first key of a phrase: every voice glides from here */
        if (none_sounding(a)) { legato_on(a, a->all); a->glide = a->all; }
        else legato_off(a, a->all);
        for (int k = 0; k < a->n; k++)
            if (k != v && (a->glide & (1u << k)))
                retune(a, k, note);
    }
    gate_off(a, v);
    play(a, v, note, vel);
    a->glide &= ~(1u << v);
    use(a, v);
    update_top(a, v, 1);
}

/* MONO (voice 0) and UNISON (all voices): last note wins; on release fall
 * back to the lowest key still down */
static void mono(struct assign *a, int note, int vel, int unison)
{
    uint32_t mask = unison ? a->all : 1;
    if (vel) {
        if (!a->v[0].vel || a->v[0].held == 1) {
            if (unison) release_always(a, a->all);
            else gate_off_always(a, 0);
        }
        play_all(a, mask, note, vel);
        use(a, 0);
        if (!unison)
            release(a, ~1u);
        update_top(a, 0, 1);
        return;
    }
    if (a->v[0].note != note)
        return;
    if (a->hold) {
        for (int k = 1; k < a->n; k++)
            if (a->v[k].vel) {
                for (int j = 0; j < a->n; j++)
                    if (a->v[j].vel && a->v[j].note == note)
                        a->v[j].held = 1;
                return;
            }
        int key = lowest_key(a);
        if (key >= 0) retune_all(a, mask, key);
        else a->v[0].held = 1;
        if (!unison)
            release(a, ~1u);
        return;
    }
    int key = lowest_key(a);
    if (key >= 0) {
        retune_all(a, mask, key);
        if (!unison)
            release(a, ~1u);
        return;
    }
    release(a, a->all);
    update_top(a, 0, 0);
}

static void key(struct assign *a, int note, int vel)
{
    int t;
    switch (a->key_assign) {
    case MONO:
        mono(a, note, vel, 0);
        break;
    case UNISON:
        if (pget(a, ID_PORTA_TIME, &t))
            mono(a, note, vel, 1);
        break;
    default:
        if (pget(a, ID_PORTA_TIME, &t))
            poly(a, note, vel, t != 0);
        break;
    }
}

/* ---- controllers -------------------------------------------------------- */

static void all_notes_off(struct assign *a)
{
    if (a->hold) {
        a->off_pending = 1;
        return;
    }
    release(a, a->all);
    clear_voices(a);
}

static void hold(struct assign *a, int on)
{
    if (on) {
        a->hold = 1;
        return;
    }
    a->hold = 0;
    uint32_t mask = 0;
    for (int v = 0; v < a->n; v++)
        if (a->v[v].held == 1) {
            a->v[v].held = 0;
            mask |= 1u << v;
        }
    release(a, mask);
    if (a->off_pending == 1) {
        release(a, a->all);
        clear_voices(a);
    }
    a->off_pending = 0;
}

static void all_sound_off(struct assign *a)
{
    hold(a, 0);
    all_notes_off(a);
}

static void modulation(struct assign *a, int v)
{
    vset(a, ID_MOD, v);
    a->vt->flush(a, G_PATCH);
}

static void reset_controllers(struct assign *a)
{
    modulation(a, 0);
    vset(a, ID_EXPR, 127);
    hold(a, 0);
    a->vt->bend(a, 0x2000);
}

static void read_key_assign(struct assign *a)
{
    int v;
    if (!pget(a, ID_ASSIGN, &v) || v == a->key_assign)
        return;
    all_sound_off(a);
    if (v == POLY)
        a->glide = 0;
    else if (v == MONO || v == UNISON)
        for (int w = 0; w < 4; w++)
            a->keys[w] = 0;
    a->key_assign = v;
}

static void read_porta_mode(struct assign *a)
{
    int v;
    if (pget(a, ID_PORTA_MODE, &v))
        a->porta_mode = v;
}

/* ---- vtable ------------------------------------------------------------- */

static void m_init(struct assign *a, int n) { init_voices(a, n); all_notes_off(a); }
static void m_reset(struct assign *a) { a->vt->init(a, a->n); }

static void m_set_voices(struct assign *a, int n)
{
    all_sound_off(a);
    a->vt->init(a, n);
    read_key_assign(a);
    read_porta_mode(a);
}

static void m_changed(struct assign *a, int group)
{
    if (group == G_PATCH) {
        read_key_assign(a);
        read_porta_mode(a);
    }
}

static void m_note_off(struct assign *a, int note)
{
    key_set(a, note, 0);
    key(a, note, 0);
}

static void m_note_on(struct assign *a, int note, int vel)
{
    key_set(a, note, vel != 0);
    key(a, note, vel);
}

#ifdef PEEK
/* Live memory peek / poke for DSP experiments (re/dsppeek.py), master side.
 * Region 0 runs on the OS CPU (master BMC core 0): it can read and write the
 * master's DSP memories and send MIDI through the OS router. Commands arrive
 * as CC1 values seq<<6 | 3<<4 | cmd (cmd field 3 is ignored by the slave
 * engine's DSPLIVE protocol, and these CC1s are not forwarded to the slaves);
 * the 14-bit data P is the last pitch bend, read back from voice id 0x465 (the OS writes the bend
 * there itself; the assigner's bend method is not called for MIDI bend).
 *   cmd 0 A_LO = P, 1 A_HI = P      address = 0x40000000 | A_HI << 16 | A_LO << 2
 *   cmd 2 D_LO = P, 3 D_MID = P     data bits 0-13, 14-27
 *   cmd 4 write: data = D_LO | D_MID << 14 | (P & 15) << 28 -> *address, address += 4
 *   cmd 5 read P words (1..16): reply F0 7D 02 <address: 8 nibbles> <words: 8 nibbles each> F7,
 *         address += 4 P
 *   cmd 6 ping: reply F0 7D 03 <P: 4 nibbles> F7
 * MIDI out = the sequence of the OS Identity handler (docs/BASE_FIRMWARE.md 7.12). */
static uint32_t peek_alo, peek_ahi, peek_dlo, peek_dmid;

static void sx_send(const uint8_t *buf, int len)
{
    uint32_t msg[4], wrap[4];                                          /* filled by the ctors */
    ((void (*)(void *))0x600753CDu)(msg);                              /* Msg ctor */
    ((uint8_t *)msg)[6] = 4;                                           /* cable 4 */
    ((void (*)(void *, const void *, int))0x600754BDu)(msg, buf, len); /* set SysEx */
    ((void (*)(void *, void *))0x600754C7u)(wrap, msg);                /* wrap_msg */
    ((void (*)(int, void *))0x6000345Du)(0x1A20, wrap);                /* router: USB1 + DIN */
    ((void (*)(void *))0x600753F5u)(wrap);                             /* Wrap dtor */
    ((void (*)(void *))0x600753E1u)(msg);                              /* Msg dtor */
}

static uint8_t *put_nibbles(uint8_t *b, uint32_t v, int n)
{
    while (n--)
        *b++ = (v >> (4 * n)) & 15;
    return b;
}

static volatile uint32_t *peek_addr(void)
{
    return (volatile uint32_t *)(0x40000000u | peek_ahi << 16 | peek_alo << 2);
}

static void peek_advance(int words)
{
    uint32_t w = (peek_ahi << 14 | peek_alo) + words;
    peek_alo = w & 0x3FFF;
    peek_ahi = (w >> 14) & 0x3FFF;
}

static void peek_cmd(struct assign *as, int cmd)
{
    uint8_t buf[3 + 8 + 16 * 8 + 1], *b = buf;
    int bend = 0;
    as->vt->get(as, G_VOICE, ID_BEND, &bend);
    uint32_t p = (uint32_t)(bend + 0x2000) & 0x3FFF;
    switch (cmd) {
    case 0: peek_alo = p; break;
    case 1: peek_ahi = p; break;
    case 2: peek_dlo = p; break;
    case 3: peek_dmid = p; break;
    case 4:
        *peek_addr() = peek_dlo | peek_dmid << 14 | (p & 15) << 28;
        __asm volatile ("dsb" ::: "memory");
        peek_advance(1);
        break;
    case 5: {
        int n = p < 1 ? 1 : p > 16 ? 16 : (int)p;
        volatile uint32_t *a = peek_addr();
        *b++ = 0xF0; *b++ = 0x7D; *b++ = 0x02;
        b = put_nibbles(b, (uint32_t)(uintptr_t)a, 8);
        for (int i = 0; i < n; i++)
            b = put_nibbles(b, a[i], 8);
        *b++ = 0xF7;
        sx_send(buf, b - buf);
        peek_advance(n);
        break;
    }
    case 6:
        *b++ = 0xF0; *b++ = 0x7D; *b++ = 0x03;
        b = put_nibbles(b, p, 4);
        *b++ = 0xF7;
        sx_send(buf, b - buf);
        break;
    }
}
#endif

static void m_cc(struct assign *a, int cc, int v)
{
#ifdef PEEK
    if (cc == 1 && (v & 0x30) == 0x30) {
        peek_cmd(a, v & 15);
        return;
    }
#endif
    switch (cc) {
    case 1: modulation(a, v); break;
    case 7: case 10: case 65: break;
    case 11: vset(a, ID_EXPR, v); break;
    case 64: hold(a, v); break;
    case 120: all_sound_off(a); break;
    case 121: reset_controllers(a); break;
    case 123: case 124: case 125: case 126: case 127: all_notes_off(a); break;
    default: a->vt->other_cc(a, cc, v); break;
    }
}

/* part parameter 0x22D: v (0..7) is stored as v + 8; the same value again
 * only touches it */
static void m_part_22d(struct assign *a, int v)
{
    int cur;
    if (!a->vt->get(a, G_PART, ID_PART_22D, &cur))
        return;
    if (v == cur)
        a->vt->touch(a, G_PART, ID_PART_22D);
    else if ((unsigned)v < 8)
        a->vt->set(a, G_PART, ID_PART_22D, v + 8);
    a->vt->flush(a, G_PART);
    a->vt->flush(a, 0);
}

static void m_bend(struct assign *a, int v)
{
    v -= 0x2000;
    if (v < -0x2000) v = -0x2000;
    else if (v >= 0x2000) v = 0x1FFF;
    vset(a, ID_BEND, v);
}

static void m_nop(struct assign *a) { (void)a; }
static void m_other_cc(struct assign *a, int cc, int v) { (void)a; (void)cc; (void)v; }
static int m_voices(struct assign *a) { return a->n; }

static const struct assign_vt assign_vt = {
    m_reset, m_changed, m_note_off, m_note_on, m_cc, m_part_22d, m_bend, m_nop, m_nop,
    m_set, m_get, m_touch, m_flush, m_other_cc, m_init, m_set_voices, m_voices,
};

/* split mode: 1/2 = lower/upper half with 4 voices each, else 8 voices */
static void set_mode(struct assign *a, int mode)
{
    if (a->mode == mode)
        return;
    a->vt->set_voices(a, mode == 1 || mode == 2 ? 4 : 8);
    a->mode = mode;
}

static void construct(struct assign *a)
{
    a->vt = &assign_vt;
    for (int w = 0; w < 4; w++)
        a->keys[w] = 0;
    a->mode = 0;
    init_voices(a, NV_MAX);
}

/* ---- descriptor and SYNINFO --------------------------------------------- */

static struct assign part_obj[2];
static struct assign *part_ptr[2];
static int constructed;

struct descriptor {
    int slot;
    void (*create)(void);
    void (*destroy)(void);
    struct assign *part[2];
    void *zero;
    void (*select)(int part, int mode);
};
static struct descriptor desc;

static void create(void)
{
    if (!constructed) {
        construct(&part_obj[0]);
        construct(&part_obj[1]);
        constructed = 1;
    }
    if (!part_ptr[0]) part_ptr[0] = &part_obj[0];
    if (!part_ptr[1]) part_ptr[1] = &part_obj[1];
}

static void destroy(void)
{
    part_ptr[0] = part_ptr[1] = 0;
}

static void select_part(int part, int mode)
{
    struct assign *a = part_ptr[part ? 1 : 0];
    if (a)
        set_mode(a, mode);
}

/* the OS calls this many times; it must keep returning the same descriptor */
static struct descriptor *plugout_init(void)
{
    desc.create = create;
    desc.destroy = destroy;
    desc.part[0] = part_ptr[0];
    desc.part[1] = part_ptr[1];
    desc.zero = 0;
    desc.select = select_part;
    return &desc;
}

struct syninfo {
    char magic[16];
    char name[16];
    uint16_t model_id, version;
    struct descriptor *(*init)(void);
    const void *list;
    char magic2[16];
};

__attribute__((section(".syninfo"), used))
const struct syninfo syninfo = {
    "SYSTEM8_SYNINFO", "C-SAW", 0x0008, 0x0001, plugout_init, model_list, "SYSTEM8_SYNINFO",
};
