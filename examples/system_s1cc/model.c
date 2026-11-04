/* SYSTEM-S1CC region 0: MUTABLE-SYSTEM's model (examples/mutable_system/model.c: C-S1SY's, with VOLUME sent again at
 * every note), unchanged; NV voices from s1_programs.h (VOICES=n). docs/examples_system_s1cc_README.md.
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

enum { G_PART = 1, G_VOICE = 2, G_PANEL = 3, G_PATCH = 4 };
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

#ifdef EDM_STR
static void edm_str_note(int v, int note);
static void edm_str_off(int v);
#endif
static void gate_off(struct assign *a, int v)
{
#ifdef EDM_STR
    edm_str_off(v);
#endif
    if (a->v[v].vel)
        vset(ID_VEL + v, 0);
    a->v[v].vel = 0;
}

static void play(struct assign *a, int v, int note, int vel)
{
    struct voice *x = &a->v[v];
#ifdef EDM_STR
    if (vel)
        edm_str_note(v, note);
#endif
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
/* VOLUME again at every note: m_load writes it right after queueing the master DSP B program, and the queued load can
   land later and leave C01C at 0 (silence until the knob moves, observed 2026-10-27). One FCEC write per note. */
static void resend_volume(void);
#ifdef EDM_KS
/* EDM_KS (docs/MUTABLE_MEMORY.md §3.3): the OS's delay on master DSP B as a Karplus-Strong string. At every note:
   the delay times the effect reads from its bank 5 (A001 left, A002 right; system-1/FX_CHAIN.md §3, program 0x0)
   = the note's period (build.sh table, EDM_SCALE per sample), feedback C004 (the OS writes 0.6 with DELAY > 0),
   C001 = 1 (delay on), C002 = 1 (delay level). The OS writes them again when DELAY / DELAY TIME move. */
#include "edm_ks.h"
/* The effect's delay times are bank-5 words A101 (left) / A102 (right) (A103 = 2 x A101 likely its second use),
   in the shared A1xx page: FD6C(sel 1, idx). Measured 2026-11-03 with PARAMLOG (edm_dump): DELAY TIME at 0 reads
   A101 = 0.05847 = 958 / 16384 and the echoes came every 961 samples (3 samples of loop latency). FD6C(0, ...)
   (A0xx) and 0x4016A000 do nothing for the effect. */
typedef void (*b5_fn)(int sel, const uint32_t *obj, uint32_t value);
static void mb5(int idx, uint32_t bits)                       /* master bank 5 word A100 + idx (FD6C sel 1) */
{
    uint32_t obj[3] = { 0, 0, (uint32_t)idx };
    ((b5_fn)SVC(0x16C))(1, obj, bits);
}
#ifdef PARAMLOG
static void edm_dump(void);                                   /* below param_log */
#endif
static void edm_note(int note)
{
#ifdef PARAMLOG
    edm_dump();
#endif
    typedef void (*fx_fn)(int sel, int id, uint32_t value);
    mb5(1, edm_time_bits[note & 0x7F]);
    mb5(2, edm_time_bits[note & 0x7F]);
    ((fx_fn)SVC(0xEC))(1, 0x01, EDM_ONE_BITS);
    ((fx_fn)SVC(0xEC))(1, 0x02, EDM_ONE_BITS);
    ((fx_fn)SVC(0xEC))(1, 0x04, EDM_FB_BITS);
}
#endif
#ifdef EDM_GLIDE
/* EDM_GLIDE (docs/MUTABLE_MEMORY.md §3.3.2): the OS keeps its master-B program set in its own RAM and the effect
   (program 0x0, 0x552 bytes at 0x60097364, OS 1.30) smooths the delay time with r7 -> 0.9998875 (`mov r6, #0.9998875`,
   halfwords 2fff 8a00 in the word at 0x60097404). Patched to 0.5 (2800 0000, asm/asm.py) only if the word is the
   expected one (another OS version: untouched); takes effect whenever the OS queues its set again. */
#define EDM_GLIDE_WORD ((volatile uint32_t *)0x60097404)
/* The OS queues that set only at boot, so the RAM patch alone changes nothing (measured 2026-11-03). Master DSP B's
   program memory is mapped at 0x40170000 + byte address (read 2026-11-03: the effect at +0, our main at +0x1000), so
   the same word is also patched there, live: 0x40170000 + 0xA0. */
#define EDM_GLIDE_DSP ((volatile uint32_t *)0x401700A0)
#ifndef EDM_GLIDE_NEW
#define EDM_GLIDE_NEW 0x28000000u                             /* 0.5; 0x2fff8a00u = the same value (write test) */
#endif
static void edm_glide(void)
{
    if (*EDM_GLIDE_WORD == 0x2fff8a00u)
        *EDM_GLIDE_WORD = 0x28000000u;
    if (*EDM_GLIDE_DSP == 0x2fff8a00u)
        *EDM_GLIDE_DSP = EDM_GLIDE_NEW;
}
#endif
#ifdef EDM_TAP
/* EDM_TAP (docs/MUTABLE_MEMORY.md §3.3): the OS's delay only writes the line (our master B main reads it): delay on
   (C001 = 1), its own level C002 and feedback C004 at 0. At load and every note (the OS writes them again when
   DELAY moves) */
static void edm_writer(void)
{
    typedef void (*fx_fn)(int sel, int id, uint32_t value);
#ifdef EDM_CALL_W
    ((fx_fn)SVC(0xEC))(1, 0x01, 0);                           /* write probe: the OS delay writes nothing */
#else
    ((fx_fn)SVC(0xEC))(1, 0x01, 0x3f800000u);
#endif
    ((fx_fn)SVC(0xEC))(1, 0x02, 0);
    ((fx_fn)SVC(0xEC))(1, 0x04, 0);
}
#endif
#ifdef EDM_CALL
/* EDM_CALL (docs/MUTABLE_MEMORY.md §3.3.3): our tapreq inside the OS effect at master B 0x0, the one program whose
   requests the delay memory serves. Nothing is queued at 0x0: the words are written live into master B's program
   memory (0x40170000 + byte address, read 2026-11-03), only if the site holds what OS 1.30 has there (FNV-1a of its 5
   halfwords; any other content: untouched). The OS's two instructions at the site are copied from the DSP into the
   block, ahead of ours (microcode/edm_call.s, make_s1cc.py edm_call.h); power off / on restores the effect. */
#include "edm_call.h"
#define MB_PROG 0x40170000u
#define EDM_SITE 0x164u                                       /* byte address of the effect's second tapreq (1.30) */
#define EDM_SITE_FNV 0x418f2347u
#define EDM_BLOCK 0xC00u                                      /* free program memory (edm_call.s) */
static unsigned prog_hw(unsigned b)                           /* halfword at byte b: the upper half comes first */
{
    uint32_t w = *(volatile uint32_t *)(MB_PROG + (b & ~3u));
    return (b & 2) ? (w & 0xFFFF) : (w >> 16);
}
static void prog_word(unsigned b, unsigned hi, unsigned lo) { *(volatile uint32_t *)(MB_PROG + b) = (hi << 16) | lo; }
static int edm_call_state;                                    /* 1 patched now, 2 already there, 3 site unknown */
static void edm_call(void)
{
    uint16_t buf[5 + sizeof edm_call_block / 2 + 1];
    uint32_t h = 0x811c9dc5u;
    for (unsigned i = 0; i < 5; i++)
        h = (h ^ prog_hw(EDM_SITE + 2 * i)) * 0x01000193u;
    if (h != EDM_SITE_FNV) {
        edm_call_state = prog_hw(EDM_SITE) == edm_call_site[0] && prog_hw(EDM_SITE + 2) == edm_call_site[1] ? 2 : 3;
        return;
    }
    unsigned n = 0;
    for (unsigned i = 0; i < 5; i++)
        buf[n++] = (uint16_t)prog_hw(EDM_SITE + 2 * i);
    for (unsigned i = 0; i < sizeof edm_call_block / 2; i++)
        buf[n++] = edm_call_block[i];
    if (n & 1)
        buf[n++] = 0;
    for (unsigned i = 0; i < n; i += 2)                       /* the block first: nothing runs it yet */
        prog_word(EDM_BLOCK + 2 * i, buf[i], buf[i + 1]);
    /* then the site, 5 halfwords at 0x164..0x16D (words 0x164, 0x168 and the upper half of 0x16C), back to back */
    unsigned keep = prog_hw(EDM_SITE + 10);
    __asm volatile ("cpsid i" ::: "memory");
    prog_word(EDM_SITE, edm_call_site[0], edm_call_site[1]);
    prog_word(EDM_SITE + 4, edm_call_site[2], edm_call_site[3]);
    prog_word(EDM_SITE + 8, edm_call_site[4], keep);
    __asm volatile ("cpsie i" ::: "memory");
    edm_call_state = 1;
}
#endif
#ifdef EDM_ECF
/* EDM_ECF (docs/MUTABLE_MEMORY.md §3.3.4): our own delay-memory lines. Which bank-2 word a DSP's transfers move to
   or from which delay-memory address is set by the DSP's map table, not by the program: master B's at 0x4016E000
   (read 2026-11-03), 8 bytes per entry {bit 31 write | address, word << 16 | length}. OS 1.30 queues 34 entries
   there at power on (its data set {0x0, 0x110 bytes} from 0x60097928: [4001] -> 0x400000 for the delay, [400a] and
   the reverb's lines from 0x48CA00 up to 0x49B0DA) and a zero entry after them ({0x110, 8 bytes}); the free entries
   hold {0x807FFFFF, 0x007FFFFF}. In the DSP the read entries have no length (low half 0) and their address moved
   by a few samples from the OS's RAM copy (read 2026-11-03: 0049aa4c 002c23e9 -> 0049aa4a 002c0000). We add a
   write and a read entry per line after the OS's (microcode/edm_ecf.s reads and writes the words); the table stays
   until power off. Only if entries 0 and 33 are the OS's and 34..38 are zero, free or already ours. */
#define ECF ((volatile uint32_t *)0x4016E000u)
#define ECF_OS 34                                             /* the OS's entries */
#define ECF_A1 0x440000u                                      /* inside the OS delay's span 0x400000..0x48CA00 */
#define ECF_A2 0x460000u
#define ECF_D1 48000u
#define ECF_D2 24000u
/* the entries as the OS writes its own (RAM format: a read entry carries its length in the low half), loaded by the
   OS's data queue for master B (FC3C, the way it loads its table and the stock PLUG-OUTs load {0x110, 8 bytes}): a
   direct ARM store into the table while the DSP ran switched the SYSTEM-1 off (2026-11-03) */
static const uint32_t ecf_own[5][2] = {
    { 0x80000000u | ECF_A1, 0x34u << 16 },                    /* [4034] -> A1 */
    { ECF_A1 + ECF_D1, 0x35u << 16 | ECF_D1 },                /* [4035] <- A1 + D1 */
    { 0x80000000u | ECF_A2, 0x36u << 16 },                    /* [4036] -> A2 */
    { ECF_A2 + ECF_D2, 0x37u << 16 | ECF_D2 },                /* [4037] <- A2 + D2 */
    { 0, 0 },                                                 /* the zero entry after them, as the OS's at 0x110 */
};
static struct s1_prog ecf_set[] = { { 0x110, sizeof ecf_own, ecf_own }, { 0, 0, 0 } };
static int edm_ecf_state;                                     /* 1 queued now, 2 already there, 3 table unknown */
static void edm_ecf(void)
{
    if (ECF[0] != 0x80400000u || ECF[1] != 0x00010000u || ECF[2 * 33] != 0x0049b0dau || ECF[2 * 33 + 1] != 0x002d0000u) {
        edm_ecf_state = 3;
        return;
    }
    int ours = 1;
    for (int k = 0; k < 5; k++) {                             /* entries 34..38: zero, free or ours (low half 0) */
        uint32_t a = ECF[2 * (ECF_OS + k)], b = ECF[2 * (ECF_OS + k) + 1];
        int mine = a == ecf_own[k][0] && b == (ecf_own[k][1] & 0xFFFF0000u);
        if (!mine)
            ours = 0;
        if (!mine && (a | b) && !(a == 0x807fffffu && b == 0x007fffffu)) {
            edm_ecf_state = 3;
            return;
        }
    }
    if (ours) {
        edm_ecf_state = 2;
        return;
    }
    ((s1_queue_fn)SVC(0x3C))(ecf_set);
    edm_ecf_state = 1;
}
#endif
#ifdef EDM_STR
/* EDM_STR (docs/MUTABLE_MEMORY.md §3.3.5): a Karplus-Strong string per voice on master B (microcode/edm_str.s), each
   in its own delay-memory line mapped the way EDM_ECF maps its two (above): voice v's write entry [STR_W + 8v] ->
   STR_A0 + v STR_DA and four read entries [STR_W + 8v + 1, 3, 5, 7] <- that address + d .. d + 3, table entries
   STR_SLOT0 + 5v .. + 4, a zero entry after the last voice. All queued at load with FC3C; at every note the four
   read entries of the voice again (d of the note, the period's whole part) and its f (C020 + v) by FCEC. The
   feedback C028 + v: AMP DECAY's while the gate is on, AMP RELEASE's after; FILTER CUTOFF -> a / 1 - 2a. */
#define ECF_T ((volatile uint32_t *)0x4016E000u)
static uint32_t str_map[5 * NV + 1][2];
static struct s1_prog str_set[] = { { 8 * STR_SLOT0, sizeof str_map, str_map }, { 0, 0, 0 } };
static uint32_t str_rd[NV][4][2];                             /* per voice: its read entries, queued at every note */
static struct s1_prog str_rd_set[NV][2];
static uint8_t str_gate[NV], str_knob[3] = { 200, 100, 160 }; /* AMP DECAY, AMP RELEASE, FILTER CUTOFF */
static int edm_str_state;                                     /* 1 queued, 3 table unknown (not OS 1.30's) */
typedef void (*str_fcec_fn)(int sel, int id, uint32_t value);
static void str_prm(int id, uint32_t bits) { ((str_fcec_fn)SVC(0xEC))(1, id, bits); }

static void str_reads(int v, int note)                        /* voice v's read entries for note */
{
    uint32_t d = str_d[note & 0x7F], a = STR_A0 + (uint32_t)v * STR_DA;
    for (uint32_t k = 0; k < 4; k++) {
        str_rd[v][k][0] = a + d + k;
        str_rd[v][k][1] = (STR_W + 8u * (uint32_t)v + 1u + 2u * k) << 16 | (d + k);
    }
}
static void str_fb(int v) { str_prm(STR_PFB + v, str_gate[v] ? str_hold_bits[str_knob[0]] : str_rel_bits[str_knob[1]]); }
static void str_bright(void)
{
    str_prm(STR_PA, str_a_bits[str_knob[2]]);
    str_prm(STR_PC, str_c_bits[str_knob[2]]);
}

static void edm_str(void)                                     /* m_load */
{
    if (ECF_T[0] != 0x80400000u || ECF_T[1] != 0x00010000u || ECF_T[2 * 33] != 0x0049b0dau
        || ECF_T[2 * 33 + 1] != 0x002d0000u) {
        edm_str_state = 3;
        return;
    }
    for (int v = 0; v < NV; v++) {                            /* feedback 0 first: whatever the lines hold dies */
        str_gate[v] = 0;
        str_prm(STR_PFB + v, 0);
        str_prm(STR_PF + v, 0);
        str_reads(v, 60);
        str_map[5 * v][0] = 0x80000000u | (STR_A0 + (uint32_t)v * STR_DA);
        str_map[5 * v][1] = (STR_W + 8u * (uint32_t)v) << 16;
        for (int k = 0; k < 4; k++) {
            str_map[5 * v + 1 + k][0] = str_rd[v][k][0];
            str_map[5 * v + 1 + k][1] = str_rd[v][k][1];
        }
        str_rd_set[v][0].addr = 8u * (STR_SLOT0 + 5u * (uint32_t)v + 1u);
        str_rd_set[v][0].bytes = sizeof str_rd[v];
        str_rd_set[v][0].src = str_rd[v];
    }
    str_map[5 * NV][0] = str_map[5 * NV][1] = 0;
    ((s1_queue_fn)SVC(0x3C))(str_set);
    str_bright();
    edm_str_state = 1;
}
static void edm_str_note(int v, int note)                     /* gate on, note: retune voice v */
{
    if (edm_str_state != 1 || v < 0 || v >= NV)
        return;
    str_reads(v, note);
    ((s1_queue_fn)SVC(0x3C))(str_rd_set[v]);
    str_prm(STR_PF + v, str_f_bits[note & 0x7F]);
    str_bright();                                             /* again: the queued program may land after load */
    str_gate[v] = 1;
    str_fb(v);
}
static void edm_str_off(int v)
{
    if (edm_str_state != 1 || v < 0 || v >= NV || !str_gate[v])
        return;
    str_gate[v] = 0;
    str_fb(v);
}
static void edm_str_param(int id, int value)                  /* m_param: the three knobs */
{
    int k = id == 0x188 ? 0 : id == 0x18A ? 1 : id == 0x17D ? 2 : -1;
    if (k < 0 || (unsigned)value > 255)
        return;
    str_knob[k] = (uint8_t)value;
    if (edm_str_state != 1)
        return;
    if (k == 2)
        str_bright();
    else
        for (int v = 0; v < NV; v++)
            str_fb(v);
}
#endif
#if defined(PARAMLOG) && defined(EDM_STR)
static void param_log(int id, int flag, int value);
static void edm_str_dump(void)                                /* id 0x3D00: state; 0x3D01 + k: entries 33..44, 73..75 */
{
    param_log(0x3D00, 0, edm_str_state);
    static const int at[] = { 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 73, 74, 75 };
    for (unsigned i = 0; i < sizeof at / sizeof at[0]; i++) {
        param_log(0x3D01 + (int)i, 0, (int)ECF_T[2 * at[i]]);
        param_log(0x3D01 + (int)i, 1, (int)ECF_T[2 * at[i] + 1]);
    }
}
#endif
#if defined(PARAMLOG) && defined(EDM_ECF)
static void param_log(int id, int flag, int value);
static void edm_ecf_dump(void)                                /* id 0x3C00: state; 0x3C01 + k: entries 30..41, last 2 */
{
    param_log(0x3C00, 0, edm_ecf_state);
    static const int at[] = { 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 100, 766, 767 };
    for (unsigned i = 0; i < sizeof at / sizeof at[0]; i++) {
        param_log(0x3C01 + (int)i, 0, (int)ECF[2 * at[i]]);
        param_log(0x3C01 + (int)i, 1, (int)ECF[2 * at[i] + 1]);
    }
}
#endif
#if defined(PARAMLOG) && defined(EDM_CALL)
static void param_log(int id, int flag, int value);
static void edm_call_dump(void)                               /* id 0x3B00: state, site words, block words */
{
    param_log(0x3B00, 0, edm_call_state);
    for (int i = 0; i < 4; i++)
        param_log(0x3B01 + i, 0, (int)*(volatile uint32_t *)(MB_PROG + EDM_SITE + 4 * i));
    for (int i = 0; i < 12; i++)
        param_log(0x3B08 + i, 0, (int)*(volatile uint32_t *)(MB_PROG + EDM_BLOCK + 4 * i));
}
#endif
static void a_note_on(struct assign *a, int note, int vel)
{
#if defined(PARAMLOG) && defined(EDM_CALL)
    if (vel)
        edm_call_dump();
#endif
#if defined(PARAMLOG) && defined(EDM_ECF)
    if (vel)
        edm_ecf_dump();
#endif
#if defined(PARAMLOG) && defined(EDM_STR)
    if (vel)
        edm_str_dump();
#endif
#ifdef EDM_TAP
    if (vel)
        edm_writer();
#endif
#ifdef EDM_KS
    if (vel)
        edm_note(note);
#endif
    if (vel)
        resend_volume();
    key_set(a, note, vel != 0);
    key(a, note, vel);
}

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
#ifdef MC_S1MB0_BYTES
    /* EDM_ENTRY: our 0x0 (microcode/edm.s edm_entry) in place of the OS effect, with the ends it calls, in the order
       of the stock PLUG-OUTs' master B set (0x0, 0x5FF2, 0x6FF2, 0x1000, 0x7FF2) */
    { 0x0000, MC_S1MB0_BYTES, mc_S1MB0 },
#ifdef MC_S1MB5_BYTES
    { 0x5FF2, MC_S1MB5_BYTES, mc_S1MB5 },
    { 0x6FF2, MC_S1MB6_BYTES, mc_S1MB6 },
#endif
#endif
    { 0x1000, MC_S1MM1_BYTES, mc_S1MM1 },
#ifdef MC_S1MB7_BYTES
    { 0x7FF2, MC_S1MB7_BYTES, mc_S1MB7 },
#endif
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
static void resend_volume(void) { set_volume(volume); }

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

#if defined(PARAMLOG) && defined(EDM_KS)
/* EDM_KS diagnostic: at every note, before our writes, the words where the effect's delay time may live: bank 5 of
   master DSP A / B (blocks 0x40140000 / 0x40160000 + bank 5 x 0x2000, inferred), the FD6C window 0x40101000 and its
   sel 1..3 pages: param_log(0x3800 + region, word index, word) */
#ifdef EDM_GLIDE
/* EDM_GLIDE diagnostic (id 0x3900 + k): the patched word, the effect's first words, the OS's set descriptor and the
   copy of the four descriptors in its loader object (0x604BB3DC, built by 0x6001A316) */
static void glide_dump(void)
{
    static const uint32_t at[] = { 0x401700A0u, 0x60097404u, 0x60097364u, 0x60097368u, 0x60060278u, 0x6006027Cu };
    int k = 0;
    for (unsigned i = 0; i < sizeof at / sizeof at[0]; i++)
        param_log(0x3900 + k++, 0, (int)*(volatile uint32_t *)at[i]);
    for (int i = 0; i < 12; i++)
        param_log(0x3900 + k++, 0, (int)((volatile uint32_t *)0x604BB3DCu)[i]);
    /* candidate DSP program windows (read only): 4 words at each (id 0x3A00 + 8 * window + word) */
    static const uint32_t win[] = { 0x40170000u, 0x40171000u, 0x40172000u, 0x4016E000u, 0x40160000u, 0x40164000u,
                                    0x40150000u, 0x40151000u, 0x4014E000u, 0x40140000u, 0x40144000u };
    for (unsigned w = 0; w < sizeof win / sizeof win[0]; w++)
        for (int i = 0; i < 4; i++)
            param_log(0x3A00 + 8 * (int)w + i, 0, (int)((volatile uint32_t *)win[w])[i]);
}
#endif
static void edm_dump(void)
{
#ifdef EDM_GLIDE
    glide_dump();
#endif
    static const uint32_t base[6] = { 0x4016A000u, 0x4014A000u, 0x40101000u, 0x40101400u, 0x40101800u, 0x40101C00u };
    for (int r = 0; r < 6; r++)
        for (int k = 0; k < 6; k++)
            param_log(0x3800 + r, k, (int)((volatile uint32_t *)base[r])[k]);
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
#ifdef EDM_STR
    edm_str_param(id, value);
#endif
}
/* the panel's current values for the engine: the OS sends a control to the slave's engine (vt+0x30) only when it
   moves, so on entering the PLUG-OUT the kernel's tables, envelopes and LFO kept their defaults (CUTOFF and
   RESONANCE at 128 whatever the knobs said, 2026-10-28). touch + flush makes the OS send them all again with their
   values. The synth's controls 0x160..0x18F are patch words (patch id 0x160 + k) of group 3, as 0x18A..0x190
   (system-1/FX_CHAIN.md §1). [measured 2026-10-28: MP-CSAW reads CUTOFF / RESONANCE as they are when selected] */
enum { ID_PANEL0 = 0x160, ID_PANEL1 = 0x18F };

static void resend_panel(void)
{
    for (int id = ID_PANEL0; id <= ID_PANEL1; id++)
        p_touch(G_PANEL, id);
    p_flush(G_PANEL);
}

static void m_load(struct model *m)                           /* vt+0x08 */
{
    (void)m;
#ifdef EDM_GLIDE
    edm_glide();
#endif
    ((s1_queue_fn)SVC(0x34))(set_master_b);
#ifdef EDM_KS
    /* the line may hold anything (earlier programs, a loop saturated at full scale: MP-EDMKS hummed at 95 Hz after
       the probes, 2026-11-03): feedback 0 until the first note empties it */
    ((void (*)(int, int, uint32_t))SVC(0xEC))(1, 0x04, 0);
#endif
#ifdef EDM_TAP
    edm_writer();
#endif
#ifdef EDM_CALL
    edm_call();
#endif
#ifdef EDM_ECF
    edm_ecf();
#endif
#ifdef EDM_STR
    edm_str();
#endif
    set_volume(volume);
    resend_panel();
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
#ifdef EDM_GLIDE
        edm_glide();
#endif
        clear_voices(&assigner);
        created = 1;
    }
}
static void destroy(void) {}
static void *get_assigner(void) { return &assigner; }
static void *get_model(void) { return &model; }

#ifndef S1_NAME
#define S1_NAME "SYSTEM-S1CC"
#endif
#ifndef S1_MODEL_ID
#define S1_MODEL_ID 0x0010
#endif

__attribute__((section(".syninfo"), used))
const struct syninfo1 syninfo = SYNINFO1(S1_NAME, S1_MODEL_ID, 0x0001, create, destroy, get_assigner, get_model);

/* the patch memory the OS copies at install: NPATCH patches of 128 words, all zero (our engine reads none) */
__attribute__((section(".patches"), used))
const uint32_t patches[NPATCH][128] = { { 0 } };
