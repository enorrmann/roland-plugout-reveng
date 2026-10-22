/* C-S1V4 region 1 (the engine): runs on the SYSTEM-1 slave BMC. Freestanding, no library, no foreign code, no
 * floating point (the SYSTEM-1 BMCs have no FPU: floats are precomputed bit patterns from make_s1.py).
 * Port of C-V4's engine (examples/from_scratch/v3/engine_v3.c) to the SYSTEM-1 interface
 * (docs/examples_system_1_README.md section 3):
 *
 *   load (vt+0x08)    voice sections of the OS off (0x0100FC44 i, 0 for i = 0..7), then our programs:
 *                     DSP A: s1m0 @0x0100 + close @0x7FF2 (0x0100FC30), DSP B: s1m1 @0x0100 + close (0x0100FC34)
 *   param (vt+0x30)   (id, 0, value) for every changed id of groups 0..3: 0x110+v note -> pitch word A030+v,
 *                     0x120+v velocity -> gate word A038+v (1.0 / 0.0); v = 0..3
 *   refresh (vt+0x24) the same from the current values (0x0100FC00 get)
 *
 * Bank 5 words are written with the OS service 0x0100FD6C (sel, obj, value): obj[2] = word index, sel 0 = A000+,
 * sel 1 = A100+, value = IEEE float bits (what the SYSTEM-1 PLUG-OUT engines pass).
 */
#include <stdint.h>
#include "syninfo.h"
#include "s1_programs.h"

#define NV 4

typedef int (*get_fn)(int group, int id);
typedef void (*slot_fn)(int index, int on);
typedef void (*bank_fn)(int sel, const uint32_t *obj, uint32_t value);

static int os_get(int g, int id) { return ((get_fn)SVC(0x00))(g, id); }

static void b5(int idx, uint32_t bits)                        /* bank 5 word A000 + idx */
{
    uint32_t obj[3] = { 0, 0, (uint32_t)idx };
    ((bank_fn)SVC(0x16C))(0, obj, bits);
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

/* ---- engine object ------------------------------------------------------------------------------ */

typedef struct engine {
    const void *const *vt;
    uint8_t note[NV], on[NV];
} engine_t;

static void voice_pitch(engine_t *e, int v, int note)
{
    e->note[v] = (uint8_t)(note & 0x7F);
    b5(B5_PITCH + v, pitch_bits[e->note[v]]);
}

static void voice_gate(engine_t *e, int v, int vel)
{
    e->on[v] = vel > 0;
    if (e->on[v])
        b5(B5_PITCH + v, pitch_bits[e->note[v]]);      /* the note may have arrived after the gate */
    b5(B5_GATE + v, e->on[v] ? F_ONE : F_ZERO);
}

static int e_nop(engine_t *e) { (void)e; return 0; }

static void e_init(engine_t *e)                                  /* vt+0x00 */
{
    for (int v = 0; v < NV; v++) { e->note[v] = 69; e->on[v] = 0; }
}

static void e_load(engine_t *e)                                  /* vt+0x08 */
{
    (void)e;
    for (int i = 0; i < 8; i++)
        ((slot_fn)SVC(0x44))(i, 0);
    ((s1_queue_fn)SVC(0x30))(set_a);
    ((s1_queue_fn)SVC(0x34))(set_b);
}

static void e_refresh(engine_t *e, int all)                      /* vt+0x24 */
{
    (void)all;
    for (int v = 0; v < NV; v++) {
        voice_pitch(e, v, os_get(2, 0x110 + v));
        voice_gate(e, v, os_get(2, 0x120 + v));
    }
}

static void e_param(engine_t *e, int id, int flag, int value)    /* vt+0x30 */
{
    (void)flag;
    if (id >= 0x110 && id < 0x110 + NV)
        voice_pitch(e, id - 0x110, value);
    else if (id >= 0x120 && id < 0x120 + NV)
        voice_gate(e, id - 0x120, value);
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
#define S1_NAME "C-S1V4"
#endif
#ifndef S1_MODEL_ID
#define S1_MODEL_ID 0x0010
#endif

__attribute__((section(".syninfo"), used))
const struct syninfo1 syninfo = SYNINFO1(S1_NAME, S1_MODEL_ID, 0x0001, create, destroy, get_none, get_engine);
