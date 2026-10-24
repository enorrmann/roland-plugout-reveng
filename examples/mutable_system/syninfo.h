/* SYSTEM1_SYNINFO: the 0x28C-byte block at the start of both regions of a SYSTEM-1 PLUG-OUT.
 *
 * Layout from the SYSTEM-1 PLUG-OUTs (SH-2, SH-101, PROMARS) and from how OS 1.30 reads it (master: install
 * 0x600A95F8, mode switch 0x6001E494, patch load 0x600A9528; slave: install 0x60004D1E, engine switch 0x6004E1A8).
 * The OS checks the magic at 0x000 and 0x1B4, calls create at install and get_b (the object it runs) at every
 * switch to the PLUG-OUT; on the master it also takes get_a (the key assigner). The tables only configure the
 * panel; our values are our own choice (docs/examples_system_1_README.md section 3.1).
 */
#include <stdint.h>

#define S1_MAGIC "SYSTEM1_SYNINFO"

struct s1_ctl { uint16_t id, on; uint32_t value; };      /* panel control id 0xC0..0xEC: enabled, value */
struct s1_sel { uint16_t id, on; uint8_t allow[4]; };    /* patch selector id: allowed positions 0..3 */

struct syninfo1 {
    char magic[16];                 /* 0x000 */
    char name[16];                  /* 0x010: the name the device shows (region 0) */
    uint16_t model_id, version;     /* 0x020 */
    void (*create)(void);           /* 0x024 */
    void (*destroy)(void);          /* 0x028 */
    void *(*get_a)(void);           /* 0x02C master: key assigner; slave: unused (0) */
    void *(*get_b)(void);           /* 0x030 master: model object; slave: engine object */
    struct s1_ctl ctl[39];          /* 0x034 */
    struct s1_sel sel[9];           /* 0x16C */
    char magic2[16];                /* 0x1B4: checked at install */
    char magic3[16];                /* 0x1C4: the next section is valid */
    int16_t button[7][2];           /* 0x1D4: panel button -> patch id (0x160 + word), -1 = none */
    struct s1_sel sel2[3];          /* 0x1F0 */
    char magic4[16];                /* 0x208: the patch count below is valid */
    uint8_t zero[0x60];             /* 0x218 */
    uint32_t patches;               /* 0x278: patches at region 0 + 0x1000, 0x200 bytes each */
    char magic5[16];                /* 0x27C */
};
_Static_assert(__builtin_offsetof(struct syninfo1, ctl) == 0x34, "layout");
_Static_assert(__builtin_offsetof(struct syninfo1, sel) == 0x16C, "layout");
_Static_assert(__builtin_offsetof(struct syninfo1, magic2) == 0x1B4, "layout");
_Static_assert(__builtin_offsetof(struct syninfo1, button) == 0x1D4, "layout");
_Static_assert(__builtin_offsetof(struct syninfo1, sel2) == 0x1F0, "layout");
_Static_assert(__builtin_offsetof(struct syninfo1, magic4) == 0x208, "layout");
_Static_assert(__builtin_offsetof(struct syninfo1, patches) == 0x278, "layout");
_Static_assert(sizeof(struct syninfo1) == 0x28C, "layout");

#define NPATCH 64

/* every panel control on; 0xC0 = 4 as in the 4-voice SH-2 (meaning not known) */
#define CTL(id) { id, 1, (id) == 0xC0 ? 4 : 0xFF }
#define CTLS \
    CTL(0xC0), CTL(0xC1), CTL(0xC2), CTL(0xC3), CTL(0xC4), CTL(0xC5), CTL(0xC6), CTL(0xC7), CTL(0xC8), CTL(0xC9), \
    CTL(0xCA), CTL(0xCB), CTL(0xCC), CTL(0xCD), CTL(0xCE), CTL(0xCF), CTL(0xD0), CTL(0xD1), CTL(0xD2), CTL(0xD3), \
    CTL(0xD4), CTL(0xD5), CTL(0xD6), CTL(0xD7), CTL(0xD8), CTL(0xD9), CTL(0xDA), CTL(0xDB), CTL(0xDC), CTL(0xDD), \
    CTL(0xDE), CTL(0xDF), CTL(0xE0), CTL(0xE1), CTL(0xE2), CTL(0xE3), CTL(0xE4), CTL(0xE5), CTL(0xEC)
/* selectors: every position allowed */
#define SEL(id) { id, 1, { 1, 1, 1, 1 } }
#define SELS SEL(0x171), SEL(0x172), SEL(0x178), SEL(0x179), SEL(0x17E), SEL(0x191), SEL(0x166), SEL(0x195), SEL(0x192)

#define SYNINFO1(name_, id_, ver_, create_, destroy_, get_a_, get_b_) {                                    \
    S1_MAGIC, name_, id_, ver_, create_, destroy_, get_a_, get_b_, { CTLS }, { SELS }, S1_MAGIC, S1_MAGIC,  \
    { { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 } },                \
    { { 0, 0, { 0 } }, { 0, 0, { 0 } }, { 0, 0, { 0 } } }, S1_MAGIC, { 0 }, NPATCH, S1_MAGIC }

/* a program set: {program memory address in bytes, length in bytes, source}, ended by {0, 0, 0}; queued to a DSP
   with the OS service 0x0100FC30 (DSP A) / 0x0100FC34 (DSP B) from the load method */
struct s1_prog { uint32_t addr, bytes; const void *src; };

#define SVC(off) (*(void *volatile *)(0x0100FC00u + (off)))
typedef void (*s1_queue_fn)(struct s1_prog *set);
