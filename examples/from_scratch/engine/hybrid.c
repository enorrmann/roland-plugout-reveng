/*
 * Diagnostic build: the stock JX-3P engine and ours in one region 1 image.
 * Ours sits at JXBASE + 0x60000; the JX-3P SYNINFO init points to hyb_init.
 * The slave gets a proxy object; per slave, cfg[] decides which engine handles
 * each part. Slave 0 = JX-3P only (control), slave 1 = the parts under test.
 */
#define HYBRID
#include "engine.c"

#ifndef JXBASE
#error JXBASE
#endif
#define JX_INIT (JXBASE + 0xFCCC + 1)

typedef struct { uint8_t load, start, mute, refresh, notes, params; } cfg_t;
#ifndef CFG1
#define CFG1 { 0, 0, 0, 0, 1, 0 }        /* round 1: our note handling only */
#endif
static const cfg_t cfg[2] = { { 0, 0, 0, 0, 0, 0 }, CFG1 };

typedef uint32_t *(*init_fn)(void);
typedef void (*v0_fn)(void *);
typedef void (*v1_fn)(void *, int);
typedef void (*v3_fn)(void *, int, int, int);

static uint32_t *jxD;
void *jx_obj;                                   /* used by fwd.S */
static uint32_t hdesc[8];
extern const void *const fwd_vt[28];            /* fwd.S: forwarders to the JX-3P object */
static const cfg_t *c;

#define JXVT(off) (*(void *const *)((*(const uint8_t *const *)jx_obj) + (off)))

static void sync_slot(void) { if (jxD) jxD[0] = hdesc[1]; }

static void h_init(void *p)                     /* vt+0x00: both */
{
    (void)p;
    sync_slot();
    ((v0_fn)JXVT(0x00))(jx_obj);
    e_init(&engine);
    c = &cfg[engine.slave];
}
static void h_load(void *p)    { (void)p; sync_slot(); if (c->load) e_load(&engine); else ((v0_fn)JXVT(0x08))(jx_obj); }
static void h_start(void *p)   { (void)p; if (c->start) e_start(&engine); else ((v0_fn)JXVT(0x0C))(jx_obj); }
static void h_mute(void *p)    { (void)p; if (c->mute) e_mute(&engine); else ((v0_fn)JXVT(0x10))(jx_obj); }
static void h_unmute(void *p, int x) { (void)p; if (c->mute) e_unmute(&engine); else ((v1_fn)JXVT(0x18))(jx_obj, x); }
static void h_refresh(void *p, int x) { (void)p; if (c->refresh) e_refresh(&engine, x); else ((v1_fn)JXVT(0x24))(jx_obj, x); }
static void h_param(void *p, int id, int z, int v)
{
    (void)p;
    int note = (id >= 0x429 && id < 0x431) || (id >= 0x43A && id < 0x442) || id == 0x45C;
    if (note ? c->notes : c->params)
        e_param(&engine, id, z, v);
    else
        ((v3_fn)JXVT(0x30))(jx_obj, id, z, v);
}

static const void *proxy_vt[28];
static engine_t proxy;                         /* only .vt is used by the slave */

static uint32_t *hyb_create(void)
{
    ((init_fn)jxD[1])();                        /* JX-3P create */
    jxD = ((init_fn)JX_INIT)();
    jx_obj = (void *)jxD[5];
    return &hdesc[1];
}

uint32_t *hyb_init(void)
{
    jxD = ((init_fn)JX_INIT)();
    if (!proxy.vt) {
        for (int i = 0; i < 28; i++)
            proxy_vt[i] = fwd_vt[i];
        proxy_vt[0x00 / 4] = (const void *)h_init;
        proxy_vt[0x08 / 4] = (const void *)h_load;
        proxy_vt[0x0C / 4] = (const void *)h_start;
        proxy_vt[0x10 / 4] = (const void *)h_mute;
        proxy_vt[0x18 / 4] = (const void *)h_unmute;
        proxy_vt[0x24 / 4] = (const void *)h_refresh;
        proxy_vt[0x30 / 4] = (const void *)h_param;
        proxy.vt = proxy_vt;
    }
    jx_obj = (void *)jxD[5];
    hdesc[2] = (uint32_t)(uintptr_t)hyb_create;
    hdesc[3] = jxD[2];
    hdesc[4] = jxD[3];
    hdesc[5] = jxD[4];
    hdesc[6] = (uint32_t)(uintptr_t)&proxy;
    return &hdesc[1];
}

/* keep the linker from dropping it: the build script patches its address into
   the JX-3P SYNINFO */
__attribute__((section(".syninfo"), used)) const void *const hyb_entry = (const void *)hyb_init;
