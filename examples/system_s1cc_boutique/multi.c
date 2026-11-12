/* SYSTEM-S1CC multi (build_multi.sh, examples/system_s1cc/README.md §9): region 1 of a PLUG-OUT whose OSC1 WAVE
 * selector (0..5) chooses the kernel, or (KMAP2, README.md §12) the pair OSC1 WAVE x OSC2 WAVE. Each position is a whole SYSTEM-S1CC engine (engine.c built with
 * -DS1_SUB=k<i>_engine against that kernel's s1_programs.h: its programs, knob tables and host), exactly the engine
 * a single-kernel build has. This file is only the engine the OS sees, and forwards every call to the active one.
 *
 *   load (vt+0x08)    the active engine's load (programs on DSP A / DSP B, sections, tables)
 *   param (vt+0x30)   0x167 (OSC1 WAVE): another kernel -> the old engine's deinit (its ARM tick hook out), the new
 *                     engine's load (FC44 off for all sections, FC30 / FC34 mains, FC40 sections: what load does),
 *                     then every control value received so far replayed to it, then its refresh (notes from the OS);
 *                     any other id: recorded, and forwarded to the active engine
 *   the rest          forwarded to the active engine
 *
 * Every engine is built with the same number of voices (the voice allocator of region 0 is one, model.c NV).
 * The DSP memory is not cleared at a switch: the new kernel starts from the old one's bank-0 words (phases, filter
 * states); kernels whose state can grow without bound would need a reset there (open, §9). */
#include <stdint.h>
#include "syninfo.h"
#include "multi_map.h"     /* build_multi.sh: NK, the k<i>_engine declarations, kmap[6] = engine index per position */

#define WAVE_ID 0x167                                   /* OSC1 WAVE, selector 0..5 (s1panel.h S1_OSC1_WAVE) */
#define WAVE2_ID 0x16C                                  /* OSC2 WAVE (KMAP2: the kernel of the pair, README.md §12) */

typedef struct sub { const void *const *vt; } sub_t;
typedef void (*v_fn)(sub_t *e);
typedef int (*i_fn)(sub_t *e);
typedef int (*fwd_fn)(sub_t *e, int a, int b, int c);
typedef void (*param_fn)(sub_t *e, int id, int flag, int value);
typedef void (*refresh_fn)(sub_t *e, int all);

typedef struct multi {
    const void *const *vt;
} multi_t;

static sub_t *subs[NK];
static int active, loaded, wave1, wave2;
static int placed;                                      /* SECT_SWITCH: the kernel whose sections are on the DSPs */
static int wanted;                                      /* SECT_SWITCH: the pair the WAVE knobs ask for */
static int pending;                                     /* SECT_SWITCH: a WAVE value came with flag != 0 */
static int16_t last_val[0x100];                         /* control ids 0x100..0x1FF: last value, -1 = none yet */
static int8_t last_flag[0x100];

static sub_t *cur(void) { return subs[active]; }

static void m_init(multi_t *m)                                   /* vt+0x00 */
{
    (void)m;
#ifdef SECT_SWITCH
    subs[0] = (sub_t *)k_engine[0]();                   /* the only engine: the others are sections (sect_only.c) */
    ((v_fn)subs[0]->vt[0])(subs[0]);
#else
    for (int i = 0; i < NK; i++) {
        subs[i] = (sub_t *)k_engine[i]();
        ((v_fn)subs[i]->vt[0])(subs[i]);
    }
#endif
    for (int i = 0; i < 0x100; i++) { last_val[i] = -1; last_flag[i] = 0; }
#ifdef SECT_SWITCH
    active = 0;
    placed = 0;
    wanted = 0;
#elif defined(KMAP2)
    active = kmap2[0][0];
#else
    active = kmap[0];
#endif
    wave1 = wave2 = 0;
    loaded = 0;
}

static int m_deinit(multi_t *m)                                  /* vt+0x04 */
{
    (void)m;
    loaded = 0;
    return ((i_fn)cur()->vt[1])(cur());
}

static void m_load(multi_t *m)                                   /* vt+0x08 */
{
    (void)m;
    ((v_fn)cur()->vt[2])(cur());
    loaded = 1;
#ifdef SECT_SWITCH
    placed = 0;          /* load placed k0's sections. Never FC40 here: FC40 waits for the DSP, which is not running
                            yet during load (JP-08-S1W, 2026-11-12: FC40 from load hung the device at every select);
                            the wanted pair is placed at the next WAVE value with flag 0 (the path measured on
                            JP-08-WTEST4): until a WAVE knob moves, k0's pair sounds */
#endif
}

#ifdef SECT_SWITCH
/* SECT_SWITCH (README.md §12): engine k0 stays active for everything; a WAVE change places the chosen kernel's voice
   sections (FC40) over the running ones. The mains, the ARM side, the notes do not change */

static void select_kernel(int k)
{
    wanted = k;
    if (k == placed || !loaded)
        return;
    placed = k;
    k_sections[k]();
}
#else
static void select_kernel(int k)
{
    if (k == active)
        return;
    if (!loaded) {                                      /* not selected yet: the next load takes it */
        active = k;
        return;
    }
    ((i_fn)cur()->vt[1])(cur());                        /* old: its tick hook out */
    active = k;
    ((v_fn)cur()->vt[2])(cur());                        /* new: its programs and tables */
    for (int i = 0; i < 0x100; i++) {                   /* the panel as it is now (not the notes: refresh reads them) */
        int id = 0x100 + i;
        if (last_val[i] < 0 || id == WAVE_ID || id == WAVE2_ID || (id >= 0x110 && id < 0x130))
            continue;
        ((param_fn)cur()->vt[12])(cur(), id, last_flag[i], last_val[i]);
    }
    ((refresh_fn)cur()->vt[9])(cur(), 1);
}
#endif

static void m_param(multi_t *m, int id, int flag, int value)     /* vt+0x30 */
{
    (void)m;
    if (id >= 0x100 && id < 0x200) {
        last_val[id - 0x100] = (int16_t)value;
        last_flag[id - 0x100] = (int8_t)flag;
    }
    int v = value < 0 ? 0 : value > 5 ? 5 : value;
#ifdef KMAP2
    if (id == WAVE_ID || id == WAVE2_ID) {
        if (id == WAVE_ID)
            wave1 = v;
        else
            wave2 = v;
#ifdef SECT_SWITCH
        if (flag != 0) {                                /* the stock synth reloads only with flag 0 (0x6004ecc6
                                                           cbnz r1). TEST (2026-11-12, OSC2 WAVE seemed never to
                                                           reload): another flag only asks; the next note on places it */
            pending = 1;
            return;
        }
#endif
        select_kernel(kmap2[wave1][wave2]);
        return;
    }
#else
    if (id == WAVE_ID) {
        select_kernel(kmap[v]);
        return;
    }
#endif
    ((param_fn)cur()->vt[12])(cur(), id, flag, value);
}

/* the other slots: the active engine's, with the arguments as they come (r0 = the engine, r1..r3) */
#define FWD(n) static int m_fwd##n(multi_t *m, int a, int b, int c) \
    { (void)m; return ((fwd_fn)cur()->vt[n])(cur(), a, b, c); }
#ifdef SECT_SWITCH
typedef int (*get_fn)(int g, int id);
static int m_fwd9(multi_t *m, int a, int b, int c)              /* vt+0x24 refresh (notes) */
{
    (void)m;
    int r = ((fwd_fn)cur()->vt[9])(cur(), a, b, c);
    if (pending && loaded) {                            /* a WAVE value with flag != 0: placed at a note on, when the
                                                           DSPs surely run (never in load: FC40 waits for the DSP) */
        for (int v = 0; v < 8; v++)
            if (((get_fn)SVC(0x00))(2, 0x120 + v) > 0) {
                pending = 0;
                select_kernel(kmap2[wave1][wave2]);
                break;
            }
    }
    return r;
}
#else
FWD(9)
#endif
FWD(3) FWD(4) FWD(5) FWD(6) FWD(7) FWD(8) FWD(10) FWD(11) FWD(13) FWD(14) FWD(15) FWD(16) FWD(17) FWD(18)
FWD(19) FWD(20) FWD(21)

static const void *const multi_vt[22] = {
    [0] = (const void *)m_init, [1] = (const void *)m_deinit, [2] = (const void *)m_load,
    [3] = (const void *)m_fwd3, [4] = (const void *)m_fwd4, [5] = (const void *)m_fwd5, [6] = (const void *)m_fwd6,
    [7] = (const void *)m_fwd7, [8] = (const void *)m_fwd8, [9] = (const void *)m_fwd9,
    [10] = (const void *)m_fwd10, [11] = (const void *)m_fwd11,
    [12] = (const void *)m_param,
    [13] = (const void *)m_fwd13, [14] = (const void *)m_fwd14, [15] = (const void *)m_fwd15,
    [16] = (const void *)m_fwd16, [17] = (const void *)m_fwd17, [18] = (const void *)m_fwd18,
    [19] = (const void *)m_fwd19, [20] = (const void *)m_fwd20, [21] = (const void *)m_fwd21,
};

static multi_t multi = { .vt = multi_vt };

/* ---- exports ------------------------------------------------------------------------------------ */

static void create(void) {}
static void destroy(void) {}
static void *get_none(void) { return 0; }
static void *get_engine(void) { return &multi; }

#ifndef S1_NAME
#define S1_NAME "S1CC-MULTI"
#endif
#ifndef S1_MODEL_ID
#define S1_MODEL_ID 0x0011
#endif

__attribute__((section(".syninfo"), used))
const struct syninfo1 syninfo = SYNINFO1(S1_NAME, S1_MODEL_ID, 0x0001, create, destroy, get_none, get_engine);
