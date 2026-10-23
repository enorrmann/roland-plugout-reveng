/* C-S1JX region 1 (slave): the JX-3P's own microcode, relocated by s1jx.py, on the SYSTEM-1 slave DSPs (voices 2
 * and 3): DSP A main MAIN5, section 0x1900 MAIN3; DSP B main MAIN1, section 0x1900 DCO1. The voice ids come from the
 * OS for all four voices (0x110 + v note, 0x120 + v velocity); jx.h keeps the slave's writes.
 * Docs: docs/examples_system_1_jx_README.md.
 */
#define JX_CHIP 1
#include "jx.h"

typedef int (*get_fn)(int group, int id);
typedef void (*slot_fn)(int index, int on);

static int os_get(int g, int id) { return ((get_fn)SVC(0x00))(g, id); }

/* the sets as the SYSTEM-1 PLUG-OUTs queue them in load (main, section, close; never the frame header at 0x0000,
   which hangs the device: C-S1JX7, 2026-10-22) */
static struct s1_prog set_a[] = {
#ifdef JX_SEC_A_EMPTY
    { 0x1900, MC_EMPTY_BYTES, mc_EMPTY },          /* SEC_A_EMPTY=1 build.sh: our empty program in the section */
#else
    { 0x1900, MC_SA_SEC_BYTES, mc_SA_SEC },
#endif
#ifdef JX_NO_MAIN5
    { 0x0100, MC_EMPTY_BYTES, mc_EMPTY },          /* NO_MAIN5=1 build.sh: MAIN5 replaced by an empty main */
#else
    { 0x0100, MC_SA_MAIN_BYTES, mc_SA_MAIN },
#endif
    { 0x7FF2, MC_CLOSE_A004_BYTES, mc_CLOSE_A004 },
    { 0, 0, 0 },
};
static struct s1_prog set_b[] = {
#ifdef JX_NO_MAIN1
    { 0x0100, MC_EMPTY_BYTES, mc_EMPTY },          /* NO_MAIN1=1 build.sh: MAIN1 replaced by an empty main */
#else
    { 0x0100, MC_SB_MAIN_BYTES, mc_SB_MAIN },
#endif
    { 0x1900, MC_SB_SEC_BYTES, mc_SB_SEC },
    { 0x7FF2, MC_CLOSE_A005_BYTES, mc_CLOSE_A005 },
    { 0, 0, 0 },
};
#ifdef JX_V4_PROGS
/* C-S1JX4: C-S1V4's slave programs in our sets (main, close), no section */
#include "../system_1/build/s1_programs.h"
static struct s1_prog set_a4[] = {
    { 0x0100, MC_S1M0_BYTES, mc_S1M0 },
    { 0x7FF2, MC_CLOSE_A004_BYTES, mc_CLOSE_A004 },
    { 0, 0, 0 },
};
static struct s1_prog set_b4[] = {
    { 0x0100, MC_S1M1_BYTES, mc_S1M1 },
    { 0x7FF2, MC_CLOSE_A005_BYTES, mc_CLOSE_A005 },
    { 0, 0, 0 },
};
#endif
/* FC44 index -> section: 0 = A 0x1900, 2 = B 0x1900 (system-1/SH101_POLY.md section 2) */
enum { SEC_A_1900 = 0, SEC_B_1900 = 2 };

typedef struct engine { const void *const *vt; } engine_t;

static int e_nop(engine_t *e) { (void)e; return 0; }

static void e_init(engine_t *e) { (void)e; }                              /* vt+0x00 */

static void e_load(engine_t *e)                                           /* vt+0x08 */
{
    (void)e;
    for (int i = 0; i < 8; i++)
        ((slot_fn)SVC(0x44))(i, 0);
#ifdef JX_V4_PROGS
    ((s1_queue_fn)SVC(0x30))(set_a4);
    ((s1_queue_fn)SVC(0x34))(set_b4);
#else
    ((s1_queue_fn)SVC(0x30))(set_a);
    ((s1_queue_fn)SVC(0x34))(set_b);
#ifndef JX_NO_SEC_A
    ((slot_fn)SVC(0x44))(SEC_A_1900, 1);           /* NO_SEC_A=1 build.sh: left off (diagnostic) */
#endif
#ifndef JX_NO_SEC_B
    ((slot_fn)SVC(0x44))(SEC_B_1900, 1);           /* NO_SEC_B=1 build.sh: left off (DSP B time test) */
#endif
#endif
    jx_send_state();
}

static void e_refresh(engine_t *e, int all)                               /* vt+0x24 */
{
    (void)e; (void)all;
    for (int v = 0; v < 4; v++) {
        int vel = os_get(2, 0x120 + v);
        jx.note[v] = (uint8_t)(os_get(2, 0x110 + v) & 0x7F);
        jx.on[v] = vel > 0;
    }
    jx_send_state();
}

static void e_param(engine_t *e, int id, int flag, int value)             /* vt+0x30 */
{
    (void)e; (void)flag;
    if (!jx.ready)
        jx_send_state();
    if (id >= 0x110 && id < 0x114)
        jx_pitch(id - 0x110, value);
    else if (id >= 0x120 && id < 0x124)
        jx_gate(id - 0x120, value);
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

static int created;
static void create(void)
{
    if (!created) {
        jx_init();
        created = 1;
    }
}
static void destroy(void) {}
static void *get_none(void) { return 0; }
static void *get_engine(void) { return &engine; }

#ifndef S1_NAME
#define S1_NAME "C-S1JX"
#endif
#ifndef S1_MODEL_ID
#define S1_MODEL_ID 0x0011
#endif

__attribute__((section(".syninfo"), used))
const struct syninfo1 syninfo = SYNINFO1(S1_NAME, S1_MODEL_ID, 0x0001, create, destroy, get_none, get_engine);
