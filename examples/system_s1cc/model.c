/* SYSTEM-S1CC region 0: MUTABLE-SYSTEM's model (examples/mutable_system/model.c: C-S1SY's, with VOLUME sent again at
 * every note), unchanged; NV voices from s1_programs.h (VOICES=n). examples/system_s1cc/README.md.
 * C-S1SY region 0 (the model; the same as C-S1V8's, examples/system_1/model.c): runs on the SYSTEM-1 master BMC, next to OS 1.30. Freestanding, no library, no
 * foreign code. Port of the C-V4 model (examples/from_scratch/model/model.c) to the SYSTEM-1 interface
 * (examples/system_1/README.md section 3):
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
#if defined(EDM_LINES) && defined(STR_NV) && STR_NV < NV
#undef NV
#define NV STR_NV                                             /* EDM_STR_NV: the assigner plays only the strings */
#endif

#if defined(EDM_WT) && WT_SLAVE && WT_NV < NV
#undef NV
#define NV WT_NV                                              /* WT_SLAVE: the assigner plays only the table's voices */
#endif

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

#ifdef EDM_LINES
static void edm_str_note(int v, int note);
static void edm_str_off(int v);
#endif
static void gate_off(struct assign *a, int v)
{
#ifdef EDM_LINES
    edm_str_off(v);
#endif
    if (a->v[v].vel)
        vset(ID_VEL + v, 0);
    a->v[v].vel = 0;
}

#if defined(EDM_WT) && WT_SLAVE
static void wt_voice(int v, int note);
#endif
static void play(struct assign *a, int v, int note, int vel)
{
    struct voice *x = &a->v[v];
#ifdef EDM_LINES
    if (vel)
        edm_str_note(v, note);
#endif
#if defined(EDM_WT) && WT_SLAVE
    if (vel)
        wt_voice(v, note);                                    /* the master's voice v = the slave's voice v */
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
#if defined(EDM_WT) && !WT_SLAVE
static void wt_note_on(int note, int vel);
#endif
static void a_note_off(struct assign *a, int note)
{
#if defined(EDM_WT) && !WT_SLAVE
    wt_note_on(note, 0);                                      /* note offs come here, not as a velocity 0 */
#endif
    key_set(a, note, 0);
    key(a, note, 0);
}
/* VOLUME again at every note: m_load writes it right after queueing the master DSP B program, and the queued load can
   land later and leave C01C at 0 (silence until the knob moves, observed 2026-10-27). One FCEC write per note. */
static void resend_volume(void);
#ifdef EDM_KS
/* EDM_KS (docs/vol6_reference_designs_applications/mutable_memory.md §3.3): the OS's delay on master DSP B as a Karplus-Strong string. At every note:
   the delay times the effect reads from its bank 5 (A001 left, A002 right; docs/vol1_system_architecture/bus_interconnect.md §16.3, program 0x0)
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
/* EDM_GLIDE (docs/vol6_reference_designs_applications/mutable_memory.md §3.3.2): the OS keeps its master-B program set in its own RAM and the effect
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
/* EDM_TAP (docs/vol6_reference_designs_applications/mutable_memory.md §3.3): the OS's delay only writes the line (our master B main reads it): delay on
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
/* EDM_CALL (docs/vol6_reference_designs_applications/mutable_memory.md §3.3.3): our tapreq inside the OS effect at master B 0x0, the one program whose
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
    unsigned n = 0;
    if (h != EDM_SITE_FNV) {
        int ours = prog_hw(EDM_SITE) == edm_call_site[0] && prog_hw(EDM_SITE + 2) == edm_call_site[1];
        uint32_t g = 0x811c9dc5u;                             /* the OS's words, kept at the head of the old block */
        for (unsigned i = 0; i < 5; i++)
            g = (g ^ prog_hw(EDM_BLOCK + 2 * i)) * 0x01000193u;
        if (!ours || g != EDM_SITE_FNV) {
            edm_call_state = 3;
            return;
        }
        /* another build's call already there (power on since): the OS's words back at the site first, so that
           nothing calls the block while it is rewritten (2026-11-04: MP-EDMIDX then MP-EDMSTRT without power off) */
        for (unsigned i = 0; i < 5; i++)
            buf[i] = (uint16_t)prog_hw(EDM_BLOCK + 2 * i);
        unsigned keep0 = prog_hw(EDM_SITE + 10);
        __asm volatile ("cpsid i" ::: "memory");
        prog_word(EDM_SITE, buf[0], buf[1]);
        prog_word(EDM_SITE + 4, buf[2], buf[3]);
        prog_word(EDM_SITE + 8, buf[4], keep0);
        __asm volatile ("cpsie i" ::: "memory");
        n = 5;
    } else
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
/* EDM_ECF (docs/vol6_reference_designs_applications/mutable_memory.md §3.3.4): our own delay-memory lines. Which bank-2 word a DSP's transfers move to
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
#ifdef EDM_LINES
/* EDM_LINES (docs/vol6_reference_designs_applications/mutable_memory.md §3.3.5): a Karplus-Strong string per voice on master B (microcode/edm_str.s), each
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
#ifdef STR_COMB
static uint8_t comb_knob[2] = { 128, 192 };                   /* OSC1 COLOR, OSC2 COLOR */
static void str_fb(int v) { str_prm(STR_PFB + v, str_res_bits[comb_knob[1]]); }
#elif defined(STR_KM)
static void str_fb(int v) { str_prm(KM_GATE + v, str_gate[v] ? F_ONE : F_ZERO); }   /* s1_gate() of S1_MASTER */
#else
static void str_fb(int v) { str_prm(STR_PFB + v, str_gate[v] ? str_hold_bits[str_knob[0]] : str_rel_bits[str_knob[1]]); }
#endif
#ifdef STR_ONE
/* EDM_STR_ONE: one request per voice; the request's age and g of each voice's note for the CUTOFF step (the one-pole's
   phase delay is in the table: a new step retunes the voices) */
static int8_t str_note[NV];
#if defined(STR_KM)
/* KM (S1_MASTER, make_s1cc.py km_header): the kernel's master parameters. Per voice its S1_VOICE_PARAM rows (note,
   the steps of their two knobs), at every note and when one of those knobs changes step; the S1_PARAM columns of
   the master from their knobs, and the immediates master B must not take as immediates, at load and at every note
   (str_bright, as the other EDM_LINES parameters: the queued program may land after load) */
#define KM_PANELS 16
static uint16_t km_panel[KM_PANELS];                          /* the knobs seen (panel ids), 128 until they move */
static uint8_t km_val[KM_PANELS];
static int km_knob(int panel)
{
    for (int i = 0; i < KM_PANELS; i++)
        if (km_panel[i] == panel)
            return km_val[i];
    return 128;
}
static void str_tune(int v)
{
    unsigned n = (unsigned)(str_note[v] & 0x7F);
    for (int c = 0; c < KM_NV; c++) {
        unsigned a = km_v_pa[c] ? (unsigned)km_knob(km_v_pa[c]) >> 5 : 4u;
        unsigned b = km_v_pb[c] ? (unsigned)km_knob(km_v_pb[c]) >> 5 : 4u;
        str_prm(km_v_id[c] + v, km_vbits[km_v_off[c] + (n * KM_STEPS + a) * KM_STEPS + b]);
    }
}
static void str_bright(void)
{
    for (int i = 0; i < KM_NP; i++)
        str_prm(km_p_id[i], km_pbits[km_p_off[i] + (unsigned)km_knob(km_p_panel[i])]);
    for (int i = 0; i < KM_NK; i++)
        str_prm(km_k_id[i], km_k_bits[i]);
}
static void km_param(int id, int value)                       /* a knob: its master columns, the voices' rows */
{
    int used = 0, i = 0, old = 128;
    for (int k = 0; k < KM_NP; k++)
        used |= km_p_panel[k] == id;
    for (int c = 0; c < KM_NV; c++)
        used |= km_v_pa[c] == id || km_v_pb[c] == id;
    if (!used || !id)                                         /* only the knobs of the kernel's master tables */
        return;
    while (i < KM_PANELS && km_panel[i] && km_panel[i] != id)
        i++;
    if (i == KM_PANELS)
        return;
    if (km_panel[i] == id)
        old = km_val[i];
    km_panel[i] = (uint16_t)id;
    km_val[i] = (uint8_t)value;
    if (edm_str_state != 1)
        return;
    for (int k = 0; k < KM_NP; k++)
        if (km_p_panel[k] == id)
            str_prm(km_p_id[k], km_pbits[km_p_off[k] + (unsigned)value]);
    if (value >> 5 != old >> 5)
        for (int c = 0; c < KM_NV; c++)
            if (km_v_pa[c] == id || km_v_pb[c] == id) {
                for (int v = 0; v < NV; v++)
                    str_tune(v);
                break;
            }
}
#elif defined(STR_BOWED)
/* EDM_STR_BOWED (Braids BOWED, master_bowed): lines 2v (neck) and 2v + 1 (bridge) of voice v: their ages and g for
   the note and the bow position (OSC2 COLOR, 8 steps); the friction slope s from OSC1 COLOR (Braids' TIMBRE) */
static uint8_t bow_knob[2] = { 128, 128 };                    /* OSC1 COLOR, OSC2 COLOR */
static void str_tune(int v)
{
    unsigned i = (unsigned)(bow_knob[1] >> 5) * 128u + (unsigned)(str_note[v] & 0x7F);
    str_prm(STR_PAGE0 + 2 * v, bow_na_bits[i]);
    str_prm(STR_PF + 2 * v, bow_ng_bits[i]);
    str_prm(STR_PAGE0 + 2 * v + 1, bow_ba_bits[i]);
    str_prm(STR_PF + 2 * v + 1, bow_bg_bits[i]);
    str_prm(STR_PB + v, bow_s_bits[bow_knob[0]]);
}
static void str_bright(void) {}
#elif defined(STR_BLOWN)
/* EDM_STR_BLOWN (Braids BLOWN + a bore loss, master_blown): per voice the bore's age, g and loss b of its note and
   OSC2 COLOR step, and the body filter's coefficient at the note; no feedback parameter (the reed) */
static uint8_t blown_color = 128;                             /* OSC2 COLOR */
static void str_tune(int v)
{
    unsigned n = (unsigned)(str_note[v] & 0x7F), i = (unsigned)(blown_color >> 5) * 128u + n;
    str_prm(STR_PAGE0 + v, blown_age_bits[i]);
    str_prm(STR_PF + v, blown_g_bits[i]);
    str_prm(STR_PAP + v, blown_b_bits[i]);
    str_prm(STR_PB + v, blown_c_bits[n]);
}
static void str_bright(void) {}
#elif defined(STR_PL)
/* EDM_STR_PL (Plaits' string, master_pstr): per voice the request's age, g, the damping one-poles' b and the
   allpasses' a of its note, FILTER CUTOFF step (8) and OSC2 COLOR dispersion step (8), from make_s1cc.py pl_tables */
static uint8_t pl_disp = 0;                                   /* OSC2 COLOR */
static void str_tune(int v)
{
    unsigned i = (((unsigned)(pl_disp >> 5) * PL_CSTEPS) + (unsigned)(str_knob[2] >> 5)) * 128u
                 + (unsigned)(str_note[v] & 0x7F);
    str_prm(STR_PAGE0 + v, pl_age_bits[i]);
    str_prm(STR_PF + v, pl_g_bits[i]);
    str_prm(STR_PB + v, pl_b_bits[i]);
    str_prm(STR_PAP + v, pl_a_bits[i]);
}
static void str_bright(void) {}
#elif defined(STR_COMB)
/* EDM_STR_COMB: the comb's delay = the period of note + OSC1 COLOR ((knob - 128) / 2 semitones), no loop filter (the
   last CUTOFF step, b = 1); OSC2 COLOR = the feedback, the same for every voice and whatever the gate */
static void str_tune(int v)
{
    int n = str_note[v] + ((comb_knob[0] - 128) >> 1);
    n = n < 0 ? 0 : n > 127 ? 127 : n;
    unsigned i = (STR1_STEPS - 1u) * 128u + (unsigned)n;
    str_prm(STR_PAGE0 + v, str1_age_bits[i]);
    str_prm(STR_PF + v, str1_g_bits[i]);
}
static void str_bright(void) { str_prm(STR_PA, str1_b_bits[STR1_STEPS - 1]); }
#else
static void str_tune(int v)
{
    unsigned i = (unsigned)(str_knob[2] >> 4) * 128u + (unsigned)(str_note[v] & 0x7F);
    str_prm(STR_PAGE0 + v, str1_age_bits[i]);
    str_prm(STR_PF + v, str1_g_bits[i]);
}
static void str_bright(void) { str_prm(STR_PA, str1_b_bits[str_knob[2] >> 4]); }
#endif
#else
static void str_bright(void)
{
    str_prm(STR_PA, str_a_bits[str_knob[2]]);
    str_prm(STR_PC, str_c_bits[str_knob[2]]);
}
#endif

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
#if STR_TAP
        /* STR_TAP: only the write entries (entries STR_SLOT0 + k, a line k per voice, two with STR_LPV 2); the
           reads are the OS effect's tapreq block */
        for (int k = STR_LPV * v; k < STR_LPV * (v + 1); k++) {
            str_map[k][0] = 0x80000000u | (STR_A0 + (uint32_t)k * STR_DA);
            str_map[k][1] = (STR_W + 8u * (uint32_t)k) << 16;
        }
#ifdef STR_ONE
        str_note[v] = 60;
        str_tune(v);
#else
        str_prm(STR_PAGE0 + v, str_age_bits[60]);
        str_prm(STR_PAGE2 + v, str_age2_bits[60]);
#endif
#else
        str_map[5 * v][0] = 0x80000000u | (STR_A0 + (uint32_t)v * STR_DA);
        str_map[5 * v][1] = (STR_W + 8u * (uint32_t)v) << 16;
        for (int k = 0; k < 4; k++) {
            str_map[5 * v + 1 + k][0] = str_rd[v][k][0];
            str_map[5 * v + 1 + k][1] = str_rd[v][k][1];
        }
#endif
        str_rd_set[v][0].addr = 8u * (STR_SLOT0 + 5u * (uint32_t)v + 1u);
        str_rd_set[v][0].bytes = sizeof str_rd[v];
        str_rd_set[v][0].src = str_rd[v];
    }
#if STR_TAP
    str_map[STR_LPV * NV][0] = str_map[STR_LPV * NV][1] = 0;
    str_set[0].bytes = 8 * (STR_LPV * NV + 1);
#else
    str_map[5 * NV][0] = str_map[5 * NV][1] = 0;
#endif
    ((s1_queue_fn)SVC(0x3C))(str_set);
    str_bright();
    edm_str_state = 1;
}
static void edm_str_note(int v, int note)                     /* gate on, note: retune voice v */
{
    if (edm_str_state != 1 || v < 0 || v >= NV)
        return;
#ifdef STR_ONE
    str_note[v] = (int8_t)(note & 0x7F);
    str_tune(v);
#elif STR_TAP
    str_prm(STR_PAGE0 + v, str_age_bits[note & 0x7F]);   /* the tapreq ages: plain parameters, written live */
    str_prm(STR_PAGE2 + v, str_age2_bits[note & 0x7F]);
#elif defined(EDM_STR_DIRECT)
    str_reads(v, note);
    /* the data queue is drained only when the PLUG-OUT is selected (OS 1.30 0x6001ECA2 empties the queues, queues its
       own sets, calls load, then loads them all): a set queued at a note is never applied (measured 2026-11-04,
       MP-EDMSTRL). Instead the address word of the voice's four read entries, stored directly: one 32-bit store
       per entry, the word / length half untouched, addresses inside the span the load mapped */
    for (int k = 0; k < 4; k++)
        ECF_T[2 * (STR_SLOT0 + 5 * v + 1 + k)] = str_rd[v][k][0];
#else
    str_reads(v, note);
    ((s1_queue_fn)SVC(0x3C))(str_rd_set[v]);
#endif
#ifndef STR_ONE
    str_prm(STR_PF + v, str_f_bits[note & 0x7F]);
#endif
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
#ifdef STR_KM
    if ((unsigned)value <= 255)
        km_param(id, value);
    return;
#endif
#ifdef STR_BOWED
    if ((id == 0x168 || id == 0x16D) && (unsigned)value <= 255) {    /* OSC1 COLOR: pressure, OSC2 COLOR: position */
        int k = id == 0x16D, old = bow_knob[k];
        bow_knob[k] = (uint8_t)value;
        if (edm_str_state == 1 && (k == 0 ? value != old : value >> 5 != old >> 5))
            for (int v = 0; v < NV; v++)
                str_tune(v);
    }
    return;
#endif
#ifdef STR_BLOWN
    if (id == 0x16D && (unsigned)value <= 255) {                     /* OSC2 COLOR: the bore's loss, 8 steps */
        int old = blown_color >> 5;
        blown_color = (uint8_t)value;
        if (edm_str_state == 1 && value >> 5 != old)
            for (int v = 0; v < NV; v++)
                str_tune(v);
    }
    return;
#endif
#ifdef STR_PL
    if (id == 0x16D && (unsigned)value <= 255) {                     /* OSC2 COLOR: dispersion */
        int old = pl_disp >> 5;
        pl_disp = (uint8_t)value;
        if (edm_str_state == 1 && value >> 5 != old)
            for (int v = 0; v < NV; v++)
                str_tune(v);
        return;
    }
    if (id == 0x17D && (unsigned)value <= 255) {                     /* FILTER CUTOFF: 8 steps */
        int old = str_knob[2] >> 5;
        str_knob[2] = (uint8_t)value;
        if (edm_str_state == 1 && value >> 5 != old)
            for (int v = 0; v < NV; v++)
                str_tune(v);
        return;
    }
#endif
#ifdef STR_COMB
    if ((id == 0x168 || id == 0x16D) && (unsigned)value <= 255) {     /* OSC1 COLOR: comb pitch, OSC2 COLOR: feedback */
        comb_knob[id == 0x16D] = (uint8_t)value;
        if (edm_str_state == 1)
            for (int v = 0; v < NV; v++) {
                if (id == 0x168)
                    str_tune(v);
                else
                    str_fb(v);
            }
        return;
    }
#endif
    int k = id == 0x188 ? 0 : id == 0x18A ? 1 : id == 0x17D ? 2 : -1;
    if (k < 0 || (unsigned)value > 255)
        return;
    int step = str_knob[2] >> 4;
    str_knob[k] = (uint8_t)value;
    if (edm_str_state != 1)
        return;
    if (k == 2) {
#ifdef STR_ONE
        if (value >> 4 == step)                               /* the same tuning step: nothing to write */
            return;
        for (int v = 0; v < NV; v++)
            str_tune(v);
#else
        (void)step;
#endif
        str_bright();
    } else
        for (int v = 0; v < NV; v++)
            str_fb(v);
}
#endif
#if defined(PARAMLOG) && defined(EDM_LINES)
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
#if defined(PARAMLOG) && defined(EDM_PORT)
/* EDM_PORT (docs/vol6_reference_designs_applications/mutable_memory.md §3.3.7): the ARM's indirect port to the delay memory, the sequence of the OS 1.30
   boot self-test 0x6001D51A (disassembled 2026-11-04), here WITHOUT its framing (it parks the 4 DSP maps and the 24
   routes first, 0x0100A211 / 0x01008BDF): the DSPs keep running. Word n: [0x40100050] = 0x01000000 + 4n,
   [0x4010007C] |= 0x100, wait [0x40100008] bit 30 = 0; write: [0x40100800] = data; read: [0x40100054] |= 0x20000000,
   wait [0x40100054] bit 31 = 0, data = [0x40100800]; then [0x4010007C] &= ~0x100, wait bit 30 again. Every wait is
   bounded (a stuck port counts a timeout instead of hanging the ARM).
   Log at every note: 0x3E00 state (flag 0 notes, 1 timeouts); 0x3E40 + k: word k * 0x10000 (k = 0..63, 4M words:
   the self-test checks address lines up to n = 2^21); 0x3E80 + i: the test block EDM_PORT_W0 + i (16 words).
   EDM_PORT_WRITE (EDM_PORT=w): at the first note, 64 words value 0x5A0000 + i at EDM_PORT_W0 + i, read back at once
   (0x3EC0 + i) */
static void param_log(int id, int flag, int value);
#define EP ((volatile uint32_t *)0x40100000u)
#define EP_DATA (*(volatile uint32_t *)0x40100800u)
#ifndef EDM_PORT_W0
#define EDM_PORT_W0 0x380000u                                 /* far from the OS lines if the port counts from 0 */
#endif
#define EP_SPIN 200000
static int ep_notes, ep_timeouts;
static void ep_wait(volatile uint32_t *r, uint32_t bit)
{
    for (int i = 0; i < EP_SPIN; i++)
        if (!(*r & bit))
            return;
    ep_timeouts++;
}
static void ep_open(uint32_t n)
{
    EP[0x50 / 4] = 0x01000000u + 4u * n;
    EP[0x7C / 4] |= 0x100u;
    ep_wait(&EP[0x08 / 4], 1u << 30);
}
static void ep_close(void)
{
    EP[0x7C / 4] &= ~0x100u;
    ep_wait(&EP[0x08 / 4], 1u << 30);
}
static uint32_t ep_read(uint32_t n)
{
    ep_open(n);
    EP[0x54 / 4] |= 0x20000000u;
    ep_wait(&EP[0x54 / 4], 1u << 31);
    uint32_t d = EP_DATA;
    ep_close();
    return d;
}
#ifdef EDM_PORT_WRITE
static void ep_write(uint32_t n, uint32_t d)
{
    ep_open(n);
    EP_DATA = d;
    ep_close();
}
#endif
#ifdef EDM_PORT_P
/* EDM_PORT=p (§3.3.7): only the controller word +0x48 (the counter that moved at 96 kHz in MP-EDMPORTD), no port
   access: is reading it harmless, how fast does it move, which way. Per note: +0x48 (0x3F60), again after ~10 ms of
   the ARM's cycle counter (0x3F61), the cycles elapsed (0x3F62; DWT CYCCNT, 156 MHz measured on the S1) */
#define DEMCR_M (*(volatile uint32_t *)0xE000EDFCu)
#define DWT_CTRL_M (*(volatile uint32_t *)0xE0001000u)
#define CYCCNT_M (*(volatile uint32_t *)0xE0001004u)
static void edm_port_probe(void)
{
    DEMCR_M |= 1u << 24;
    DWT_CTRL_M |= 1u;
    uint32_t c0 = CYCCNT_M, p0 = EP[0x48 / 4];
    while (CYCCNT_M - c0 < 1560000u)
        ;
    uint32_t p1 = EP[0x48 / 4], c1 = CYCCNT_M;
    param_log(0x3F60, 0, (int)p0);
    param_log(0x3F61, 0, (int)p1);
    param_log(0x3F62, 0, (int)(c1 - c0));
    ep_notes++;
    param_log(0x3E00, 0, ep_notes);
}
#elif defined(EDM_PORT_DIAG)
/* EDM_PORT=d (§3.3.7): MP-EDMPORTR silenced the device until power off (2026-11-04). Diagnostic: the controller's 32
   words 0x40100000..0x4010007C before (0x3F00 + i) and after (0x3F20 + i) ONE read of word 0 (0x3F40), with 0x50, 0x54,
   0x7C put back to their values before the read */
static void edm_port_probe(void)
{
    uint32_t r50 = EP[0x50 / 4], r54 = EP[0x54 / 4], r7c = EP[0x7C / 4];
    for (int i = 0; i < 32; i++)
        param_log(0x3F00 + i, 0, (int)EP[i]);
    uint32_t d = ep_read(0);
    EP[0x50 / 4] = r50;
    EP[0x54 / 4] = r54;
    EP[0x7C / 4] = r7c;
    for (int i = 0; i < 32; i++)
        param_log(0x3F20 + i, 0, (int)EP[i]);
    param_log(0x3F40, 0, (int)d);
    ep_notes++;
    param_log(0x3E00, 0, ep_notes);
    param_log(0x3E00, 1, ep_timeouts);
}
#else
static void edm_port_probe(void)
{
#ifdef EDM_PORT_WRITE
    if (ep_notes == 0) {
        for (uint32_t i = 0; i < 64; i++)
            ep_write(EDM_PORT_W0 + i, 0x5A0000u + i);
        for (int i = 0; i < 64; i++)
            param_log(0x3EC0 + i, 0, (int)ep_read(EDM_PORT_W0 + (uint32_t)i));
    }
#endif
    ep_notes++;
    for (int k = 0; k < 64; k++)
        param_log(0x3E40 + k, 0, (int)ep_read((uint32_t)k * 0x10000u));
    for (int i = 0; i < 16; i++)
        param_log(0x3E80 + i, 0, (int)ep_read(EDM_PORT_W0 + (uint32_t)i));
    param_log(0x3E00, 0, ep_notes);
    param_log(0x3E00, 1, ep_timeouts);
}
#endif
#endif
#if defined(PARAMLOG) && defined(EDM_WT)
static void param_log(int id, int flag, int value);
#endif
#ifdef EDM_WT
/* EDM_WT (docs/vol6_reference_designs_applications/mutable_memory.md §3.3.8): a wavetable stored whole (WT_LEN samples, s1_programs.h wt_table) in the
   delay memory of master B, in one own line that loops on itself (microcode/edm_wt.s). At load: the line's write and
   re-circulation read entries in the map table (FC3C, after the OS's 34, as EDM_ECF) and the request block in the OS
   effect (edm_call). At the first note region 0 fills the loop: it follows the delay memory's sample counter
   (controller word 0x40100048, -1 per sample; only READ: the port itself mutes the device, §3.3.7) and, sample by
   sample, puts table sample k in master B parameter WT_PSTG (FCEC; our main reads master bank 5 as 0, §3.3.8),
   k = samples since it changed WT_PSYNC; a sample it missed
   (interrupts) is put on a later lap, when its place comes round again. The OS stays blocked while it fills (one lap =
   WT_LEN / 96000 s, at most WT_LAPS laps). */
#define WT_CTR (*(volatile uint32_t *)0x40100048u)
#define WT_MAP ((volatile uint32_t *)0x4016E000u)
#define WT_LAPS 8u
#ifndef WT_BLOCK_MS
#define WT_BLOCK_MS 500u                                      /* the most one note blocks the OS while filling */
#endif
#define WT_CYC (*(volatile uint32_t *)0xE0001004u)
#if WT_LINES
/* WT_LINES (make_s1cc.py): + the loop read at index c + WT_F (the next frame, same phase) and the writes of the lines
   (the table mixed toward the next frame) above the loop (an older age reads a higher address) */
static const uint32_t wt_map[WT_LSUB + 3][2] = {
    { 0x80000000u | WT_A, WT_W << 16 },
    { WT_A + WT_RDIST, WT_R << 16 },
    { WT_A + WT_RDIST - WT_F, WT_RN << 16 },
    WT_LINE_MAP,                                              /* lines 1 .. WT_LSUB - 1 (make_s1cc.py wt_loff) */
    { 0, 0 },
};
#else
static const uint32_t wt_map[3][2] = {
    { 0x80000000u | WT_A, WT_W << 16 },                       /* [WT_W] -> WT_A */
    { WT_A + WT_RDIST, WT_R << 16 },                          /* [WT_R] <- WT_A + WT_LEN - 2: a loop of WT_LEN (distance + 2, measured) */
    { 0, 0 },
};
#endif
static struct s1_prog wt_set[] = { { 8 * WT_SLOT0, sizeof wt_map, wt_map }, { 0, 0, 0 } };
static uint8_t wt_done[(WT_LEN + 7u) / 8u];
static int wt_state;                                          /* 0 loaded, 1 filled, 3 map table unknown */
#ifdef WT_POLL
static uint32_t wt_filled, wt_span, wt_cycles;
#endif
typedef void (*wt_fn)(int sel, int id, uint32_t value);
static void wt_b5(int idx, uint32_t bits)                     /* master bank 5 A100 + idx (FD6C sel 1) */
{
    uint32_t obj[3] = { 0, 0, (uint32_t)idx };
    ((void (*)(int, const uint32_t *, uint32_t))SVC(0x16C))(1, obj, bits);
}
static void wt_prm(int id, uint32_t bits) { ((wt_fn)SVC(0xEC))(1, id, bits); }
static uint32_t wt_i2f(int32_t x, int sh)                     /* x 2^-sh as IEEE-754 bits, |x| < 2^24 */
{
    if (!x)
        return 0;
    uint32_t sg = x < 0 ? 0x80000000u : 0, a = x < 0 ? (uint32_t)-x : (uint32_t)x;
    int e = 31 - __builtin_clz(a);
    return sg | (uint32_t)(e - sh + 127) << 23 | ((a << (23 - e)) & 0x7FFFFFu);
}
static void wt_color(int knob)                                /* OSC1 COLOR: frame f (0 .. FRAMES - 2) and the mix */
{
#if WT_SLAVE
    wt_prm(WT_PFRAME, wt_i2f((int32_t)((((uint32_t)knob * (WT_FRAMES - 1u) + 127u) / 255u) * WT_F), 14));   /* nearest */
    return;
#endif
    uint32_t q = (uint32_t)knob * (WT_FRAMES - 1u), f = q / 255u, r = q % 255u;
    if (f > WT_FRAMES - 2u) {
        f = WT_FRAMES - 2u;
        r = 255u;
    }
    wt_prm(WT_PFRAME, wt_i2f((int32_t)(f * WT_F), 14));
    wt_prm(WT_PMIX, wt_i2f((int32_t)(r * 65536u / 255u), 16));
}
#if WT_IAGE_TEST
/* WT_IAGE_TEST (edm_wt.s master_wt_s): the read under test, one-hot at C048 / C049 / C04A: 0 = the tapreq's fraction
   (fractional age, as before), 1 / 2 = our fraction 1 - q of one / two samples before (whole age). OSC2 COLOR */
static uint8_t wt_mode;
static void wt_sel(void)
{
    for (int k = 0; k < 3; k++)
        wt_prm(WT_PSEL0 + k, k == wt_mode ? F_ONE : F_ZERO);
}
#endif
static void wt_limits(void)                                   /* edm_wt.s: c's limit and the age's wrap, exact */
{
    wt_prm(WT_PLIM, wt_i2f((int32_t)(2u * WT_LEN - 1u), 15));  /* (WT_LEN - 1/2) / 16384 */
    wt_prm(WT_PHALF, wt_i2f((int32_t)WT_LEN, 15));              /* WT_LEN / 32768 */
}
static void wt_load(void)                                     /* m_load */
{
    wt_limits();
    for (int v = 0; v < WT_NV; v++) {
#if !WT_SLAVE
        wt_prm(WT_PGAIN0 + v, 0);                             /* (WT_SLAVE: the slave's amp; C033.. are phase steps) */
#endif
        wt_prm(WT_PDT0 + v, WT_IAGE_TEST ? wt_dti_bits[60] : wt_dt_bits[60]);
#if WT_IAGE_TEST
        wt_prm(WT_PDTF0 + v, wt_dtf_bits[60]);
#endif
    }
#if WT_IAGE_TEST
    wt_sel();
#endif
#if WT_DIAG == 4
    wt_prm(WT_PSTG, 0x3F800000u);                             /* the main reads it (edm_wt.s WT_DIAG=4); no fill */
    wt_state = 1;
    return;
#endif
    wt_color(0);
    if (WT_MAP[0] != 0x80400000u || WT_MAP[1] != 0x00010000u || WT_MAP[2 * 33] != 0x0049b0dau
        || WT_MAP[2 * 33 + 1] != 0x002d0000u) {
        wt_state = 3;
        return;
    }
    ((s1_queue_fn)SVC(0x3C))(wt_set);
    wt_state = 0;
}
#if WT_FILLIDX && WT_FILLCOV
/* WT_FILLCOV (default; proven 2026-11-10 as WT_FILL2_TEST, WT-FILL4-T: the loop exact, -97.7 dB against the sine):
   the index-gated fill by COVERAGE. WT-DIAG-SIN-T (a sine table, the loop on the left
   channel) measured the old fill's table: ~470 samples per lap left at 0 (0.7 %), in residue-3 runs: its passes were
   counted in fill time, not in indices covered (a block per note, c runs on between notes), so some indices were
   never sent. Here a round is one residue class r of k mod 3 and ends only when every index of the class was sent
   (wt_done: sent this round); rounds r = 0, 1, 2, three times, lead d = 3. As in the old fill a pair
   goes only for k = the first index of the class >= t + d (k 3 apart, a slot reused ~6 samples later, after c passed
   its old index): the pair is not atomic (value, then index), and a slot reused every sample (the first WT-FILL2-T,
   one pair per sample) wrote neighbours' values: 20 % of the table wrong. A sent pair is lost now and then: the second
   build (6 rounds, d = 3 then 5) left 246 zeros per lap, 96 samples apart (the ARM's 1 ms tick), and d = 5 lets a slot
   be reused before c reaches its index (k up to t + 7, reuse at t + 6): hence d = 3 only and three rounds per class (the
   tick's phase against c moves every lap: 63488 is not a multiple of 96). Same master B program */
#define WT_F2ROUNDS 9u
static uint32_t wt_c0, wt_p0, wt_round, wt_nsent, wt_pairs;
static void wt_fill(void)
{
    *(volatile uint32_t *)0xE000EDFCu |= 1u << 24;
    *(volatile uint32_t *)0xE0001000u |= 1u;
    if (!wt_c0) {                                             /* the first note: the sync (c = 0) */
        static uint32_t sync;
        sync ^= 0x3F800000u;
        wt_prm(WT_PSYNC, sync);
        wt_c0 = WT_CYC;
        wt_p0 = WT_CTR;
        if (!wt_c0)
            wt_c0 = 1;
        for (unsigned i = 0; i < sizeof wt_done; i++)
            wt_done[i] = 0;
    }
    uint32_t cref = WT_CYC, tref = (wt_p0 - WT_CTR) & 0x7FFFFFu;
    uint32_t start = cref, last = 0xFFFFFFFFu, lastk = 0xFFFFFFFFu, slot = 0, sent = 0;
    while (WT_CYC - start < WT_BLOCK_MS * 156000u && wt_round < WT_F2ROUNDS) {
        uint32_t t = tref + (WT_CYC - cref) / 1625u;          /* samples since the sync */
        if (t == last)
            continue;
        last = t;
        uint32_t r = wt_round % 3u, d = 3u;
        uint32_t k = (t + d) % WT_LEN;                        /* the table index c reaches d samples on */
        k += (r + 3u - k % 3u) % 3u;                          /* the first index of class r from there */
        if (k >= WT_LEN)
            k = r;                                            /* past the table's end: the class's first */
        if (k == lastk)
            continue;
        lastk = k;
        if (wt_done[k >> 3] & (1u << (k & 7)))
            continue;
        wt_prm(slot ? WT_PV1 : WT_PV0, wt_i2f(wt_table[k], 16 - WT_SCALE2));   /* the value first, then the index */
        wt_prm(slot ? WT_PI1 : WT_PI0, wt_i2f((int32_t)k, 14));
        slot ^= 1u;
        sent++;
        wt_done[k >> 3] |= (uint8_t)(1u << (k & 7));
        if (++wt_nsent >= (WT_LEN - r + 2u) / 3u) {         /* the indices of class r: all sent */                             /* the class covered: the next round */
            wt_nsent = 0;
            wt_round++;
            for (unsigned i = 0; i < sizeof wt_done; i++)
                wt_done[i] = 0;
        }
    }
    wt_pairs += sent;
    if (wt_round >= WT_F2ROUNDS)
        wt_state = 1;
#ifdef PARAMLOG
    param_log(0x3F80, 0, (int)sent);
    param_log(0x3F81, 0, (int)wt_pairs);
    param_log(0x3F82, 0, (int)wt_round);
#endif
}
#elif WT_FILLIDX
/* WT_FILLIDX (make_s1cc.py): index-gated fill. Master B writes the loop only when its counter c equals the index of a
   slot (C03A/C03B, C03C/C03D: index, value), so a late, early or lost write is dropped instead of landing elsewhere
   (WT-DIAG2, 2026-11-05: the timed fill lost ~7 % and later notes overwrote good samples). Each lap of filling ("pass")
   sends one residue class of k mod 3 (the ARM needs ~2 samples per pair), k = the first of the class >= t + D, so a
   pair is still in its slot (alternating, ~6 samples) when c reaches k for a latency 0 .. D. Passes: residues 0, 1, 2
   with D = 3, then again with D = 6, then once more with D = 3 (re-sends are harmless: same value, same place) */
#define WT_FPASSES 9u
static uint32_t wt_c0, wt_p0, wt_ftime, wt_pairs;
static void wt_fill(void)
{
    *(volatile uint32_t *)0xE000EDFCu |= 1u << 24;
    *(volatile uint32_t *)0xE0001000u |= 1u;
    if (!wt_c0) {                                             /* the first note: the sync (c = 0) */
        static uint32_t sync;
        sync ^= 0x3F800000u;
        wt_prm(WT_PSYNC, sync);
        wt_c0 = WT_CYC;
        wt_p0 = WT_CTR;
        if (!wt_c0)
            wt_c0 = 1;
    }
    uint32_t cref = WT_CYC, tref = (wt_p0 - WT_CTR) & 0x7FFFFFu;
    uint32_t start = cref, last = tref, lastk = 0xFFFFFFFFu, slot = 0, sent = 0;
    while (WT_CYC - start < WT_BLOCK_MS * 156000u && wt_ftime < WT_FPASSES * WT_LEN) {
        uint32_t t = tref + (WT_CYC - cref) / 1625u;          /* samples since the sync */
        if (t == last)
            continue;
        wt_ftime += t - last;                                 /* samples spent filling: the pass */
        last = t;
        uint32_t pass = wt_ftime / WT_LEN, r = pass % 3u, d = (pass / 3u) % 3u == 1u ? 6u : 3u;
        uint32_t k = t + d;
        k += (r + 3u - k % 3u) % 3u;
        if (k == lastk)
            continue;
        lastk = k;
        k %= WT_LEN;
        wt_prm(slot ? WT_PV1 : WT_PV0, wt_i2f(wt_table[k], 16 - WT_SCALE2));   /* the value first, then the index (WT_SCALE2: 2^-15 = x2 in the EDM) */
        wt_prm(slot ? WT_PI1 : WT_PI0, wt_i2f((int32_t)k, 14));
        slot ^= 1u;
        sent++;
    }
    wt_pairs += sent;
    if (wt_ftime >= WT_FPASSES * WT_LEN)
        wt_state = 1;
#ifdef PARAMLOG
    param_log(0x3F80, 0, (int)sent);
    param_log(0x3F81, 0, (int)wt_pairs);
    param_log(0x3F82, 0, (int)wt_ftime);
#endif
}
#elif !defined(WT_POLL)
/* WT_POLL undefined (default since MP-EDMWT hung at its first note, 2026-11-05, polling the counter word without a
   pause): the time from the ARM's cycle counter, 1625 cycles per sample (156 MHz / 96 kHz), the delay-memory counter
   read only at the start and at the end (0x3F84: its samples minus the cycle counter's, the drift). One lap per note
   at most: the fill goes on at the next notes (the DSP's c keeps counting, k = samples since the sync) */
static uint32_t wt_c0, wt_p0, wt_filled_all;
static void wt_fill(void)
{
    *(volatile uint32_t *)0xE000EDFCu |= 1u << 24;
    *(volatile uint32_t *)0xE0001000u |= 1u;
    if (!wt_filled_all && !wt_c0) {                           /* the first note: the sync */
        for (unsigned i = 0; i < sizeof wt_done; i++)
            wt_done[i] = 0;
        static uint32_t sync;
        sync ^= 0x3F800000u;
        wt_prm(WT_PSYNC, sync);
        wt_c0 = WT_CYC;
        wt_p0 = WT_CTR;
        if (!wt_c0)
            wt_c0 = 1;
    }
    /* the time base again from the delay memory's counter (one read): the two clocks drift ~3 ppm (58 samples in
       20 s, 2026-11-05); then the cycle counter for at most WT_BLOCK_MS (the OS is blocked meanwhile) */
    uint32_t cref = WT_CYC, tref = (wt_p0 - WT_CTR) & 0x7FFFFFu;
    uint32_t start = cref, last = 0xFFFFFFFFu, filled = 0, par = 0;
    while (WT_CYC - start < WT_BLOCK_MS * 156000u && wt_filled_all < WT_LEN) {
        uint32_t t = tref + (WT_CYC - cref) / 1625u;          /* samples since the sync */
        if (t == last)
            continue;
        last = t;
        uint32_t k = t % WT_LEN;
        if (wt_done[k >> 3] & (1u << (k & 7)))
            continue;
        par ^= 1u;
        wt_prm(WT_PSTG, wt_i2f(32 * (int32_t)wt_table[k] + (par ? 1 : -1), 21));
        wt_done[k >> 3] |= (uint8_t)(1u << (k & 7));
        filled++;
        wt_filled_all++;
    }
    uint32_t t = tref + (WT_CYC - cref) / 1625u, ps = (wt_p0 - WT_CTR) & 0x7FFFFFu;
    if (wt_filled_all >= WT_LEN)
        wt_state = 1;
#ifdef PARAMLOG
    param_log(0x3F80, 0, (int)filled);                        /* table samples written this note */
    param_log(0x3F81, 0, (int)wt_filled_all);                 /* in all */
    param_log(0x3F82, 0, (int)t);                             /* samples since the sync, by the cycle counter */
    param_log(0x3F84, 0, (int)(ps - t));                      /* the delay memory's counter minus that */
    param_log(0x3F83, 0, (int)WT_LEN);
#endif
}
#else
static void wt_fill(void)                                     /* first note: the table into the loop */
{
    for (unsigned i = 0; i < sizeof wt_done; i++)
        wt_done[i] = 0;
    *(volatile uint32_t *)0xE000EDFCu |= 1u << 24;            /* DWT CYCCNT on (only for the report) */
    *(volatile uint32_t *)0xE0001000u |= 1u;
    uint32_t c0 = WT_CYC, p = WT_CTR, guard = 0;
    while (WT_CTR == p && ++guard < 100000u)                  /* right after the counter moves */
        ;
    static uint32_t sync;
    sync ^= 0x3F800000u;
    wt_b5(WT_SYNC, sync);                                     /* the DSP's c restarts at 0 here */
    uint32_t ps = WT_CTR, last = ps, filled = 0, par = 0, span = 0;
    while (filled < WT_LEN) {
        uint32_t q = WT_CTR;
        if (q == last) {
            if (++guard > 400000000u)                         /* the counter stopped: give up */
                break;
            continue;
        }
        last = q;
        span = (ps - q) & 0x7FFFFFu;                          /* samples since the sync (23-bit counter) */
        if (span >= WT_LAPS * WT_LEN)
            break;
        uint32_t k = span % WT_LEN;
        if (wt_done[k >> 3] & (1u << (k & 7)))
            continue;
        par ^= 1u;                                            /* a change every write, even for equal samples */
        wt_b5(WT_STG, wt_i2f(32 * (int32_t)wt_table[k] + (par ? 1 : -1), 21));
        wt_done[k >> 3] |= (uint8_t)(1u << (k & 7));
        filled++;
    }
    wt_filled = filled;
    wt_span = span;
    wt_cycles = WT_CYC - c0;
    wt_state = 1;
#ifdef PARAMLOG
    param_log(0x3F80, 0, (int)wt_filled);                     /* table samples written */
    param_log(0x3F81, 0, (int)wt_span);                       /* samples the fill took */
    param_log(0x3F82, 0, (int)wt_cycles);
    param_log(0x3F83, 0, (int)WT_LEN);
#endif
}
#endif
#if WT_SLAVE
static void wt_voice(int v, int note)                         /* region 0's assigner gave voice v the note */
{
    if (v >= 0 && v < WT_NV)                                  /* the note's step FIRST: the fill's parameters queue
                                                                 up behind it (it came 40-350 ms late, 2026-11-05) */
    {
        wt_prm(WT_PDT0 + v, WT_IAGE_TEST ? wt_dti_bits[note & 0x7F] : wt_dt_bits[note & 0x7F]);
#if WT_IAGE_TEST
        wt_prm(WT_PDTF0 + v, wt_dtf_bits[note & 0x7F]);
#endif
    }
    wt_limits();                                              /* again: a write in load can land before the program */
#if WT_IAGE_TEST
    wt_sel();
#endif
    if (wt_state == 0)
        wt_fill();                                            /* the first notes fill the table (<= WT_BLOCK_MS each) */
}
#endif
#if !WT_SLAVE
static int8_t wt_vnote[WT_NV] = { -1, -1, -1 };
static unsigned wt_next;
static void wt_note_on(int note, int vel)                     /* the voice of a note: a free one, else the oldest */
{
#ifdef PARAMLOG
    if (vel) {                                                /* 0x3F90 state; 0x3F91 + 2i: map entries 33..36 */
        param_log(0x3F90, 0, wt_state);
        for (int i = 0; i < 4; i++) {
            param_log(0x3F91 + 2 * i, 0, (int)WT_MAP[2 * (33 + i)]);
            param_log(0x3F92 + 2 * i, 0, (int)WT_MAP[2 * (33 + i) + 1]);
        }
    }
#endif
    if (!vel) {
        for (int v = 0; v < WT_NV; v++)
            if (wt_vnote[v] == note) {
                wt_vnote[v] = -1;
                wt_prm(WT_PGAIN0 + v, 0);
            }
        return;
    }
    if (wt_state == 0)
        wt_fill();
    int v = -1;
    for (int i = 0; i < WT_NV && v < 0; i++)
        if (wt_vnote[(wt_next + (unsigned)i) % WT_NV] < 0)
            v = (int)((wt_next + (unsigned)i) % WT_NV);
    if (v < 0)
        v = (int)(wt_next % WT_NV);
    wt_next = (unsigned)v + 1u;
    wt_vnote[v] = (int8_t)note;
    wt_prm(WT_PDT0 + v, wt_dt_bits[note & 0x7F]);
    wt_prm(WT_PGAIN0 + v, 0x3ECCCCCDu);                       /* 0.4 */
}
#endif
#endif
static void a_note_on(struct assign *a, int note, int vel)
{
#if defined(EDM_WT) && !WT_SLAVE
    wt_note_on(note, vel);
#endif
#if defined(PARAMLOG) && defined(EDM_PORT)
    if (vel)
        edm_port_probe();
#endif
#if defined(PARAMLOG) && defined(EDM_CALL)
    if (vel)
        edm_call_dump();
#endif
#if defined(PARAMLOG) && defined(EDM_ECF)
    if (vel)
        edm_ecf_dump();
#endif
#if defined(PARAMLOG) && defined(EDM_LINES)
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

#ifdef MC_S1MA0_BYTES
/* MA_MAIN: our master DSP A main at 0x0100 (empty), as the stock PLUG-OUTs load theirs (FC30 {0x100, ...}): the OS's
   output program at 0x82 runs on into 0x0100, which otherwise keeps what an earlier mode left there (the stock
   synth's modulation, writing 600E-6021 to the slave): docs/vol6_reference_designs_applications/mutable_memory.md §4.3 */
static struct s1_prog set_master_a[] = {
    { 0x0100, MC_S1MA0_BYTES, mc_S1MA0 },
    { 0, 0, 0 },
};
#endif

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
/* m_load's write of C01C lands before the OS loads the master-B program, which leaves it 0: silent until the VOLUME
   knob moved (2026-11-05, IDX-PROBE: no notes, so the note-on resend never ran). The panel values that m_load asks
   for (resend_panel) come back through m_param after the load: the volume goes again with the first VOL_AGAIN of
   them */
#define VOL_AGAIN 64
static int vol_again;

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
/* FX diagnostic (docs/vol1_system_architecture/bus_interconnect.md §16.7): the parameter blocks of the master DSPs as the OS's set_param writes them
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
    if (vol_again > 0) {
        vol_again--;
        resend_volume();
    }
#ifdef PARAMLOG
    param_log(id, flag, value);
    /* every control except the notes / velocities, the synth's own panel and the LFO ids */
    int known = (id >= 0x110 && id < 0x130) || (id >= 0x160 && id <= 0x18A);
    if (!known && id != ID_VOLUME && ((value & 63) == 0 || value == 255))
        fx_dump();
#endif
    if (id == ID_VOLUME)
        set_volume(value);
#ifdef EDM_WT
    if (id == 0x168)                                          /* OSC1 COLOR: the position in the table */
        wt_color(value);
#if WT_IAGE_TEST
    if (id == 0x16D && (unsigned)value <= 255) {              /* OSC2 COLOR: the read under test */
        wt_mode = value < 86 ? 0 : value < 171 ? 1 : 2;
        wt_sel();
    }
#endif
#endif
#ifdef EDM_LINES
    edm_str_param(id, value);
#endif
}
/* the panel's current values for the engine: the OS sends a control to the slave's engine (vt+0x30) only when it
   moves, so on entering the PLUG-OUT the kernel's tables, envelopes and LFO kept their defaults (CUTOFF and
   RESONANCE at 128 whatever the knobs said, 2026-10-28). touch + flush makes the OS send them all again with their
   values. The synth's controls 0x160..0x18F are patch words (patch id 0x160 + k) of group 3, as 0x18A..0x190
   (docs/vol1_system_architecture/bus_interconnect.md §16.1). [measured 2026-10-28: MP-CSAW reads CUTOFF / RESONANCE as they are when selected] */
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
#ifdef MC_S1MA0_BYTES
    ((s1_queue_fn)SVC(0x30))(set_master_a);
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
#ifdef EDM_LINES
    edm_str();
#endif
#ifdef EDM_WT
    wt_load();                                                /* (the request block: edm_call above, EDM_CALL) */
#endif
    set_volume(volume);
    vol_again = VOL_AGAIN;
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
