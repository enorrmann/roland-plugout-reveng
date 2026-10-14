/*
 * Region 0 init hook (C part): find where the device keeps the name of the
 * first preset ("String 1 [Str1]" of JX-3P bank 1) in RAM, report where, then
 * write the first LEN bytes of the OS image from there.
 *
 *   scan   LO..HI byte by byte for "String 1" as plain ASCII (form 1) or as one
 *          nibble per byte, the way RQ1 returns it (form 2). The needle is
 *          stored XOR 0xA5 so this image never contains it in plain.
 *   report (REPORT=1: instead, the 7 bytes at the match after the fill) 7 bytes over the PLUG-OUT name as 14 hex digits (shown by `info`):
 *            addr[4] (big endian, first match, 0 = none) | count[2] | form[1]
 *   fill   at the first match: MODE 0 = the raw OS bytes,
 *                              MODE 1 = each byte as two nibble bytes (hi, lo).
 *
 * Build-time -D defaults: LO HI SRC LEN MODE (see build.sh).
 */
typedef unsigned char u8;
typedef unsigned int u32;

#ifndef LO
#define LO   0x602CE000u    /* just past the OS image (0x60000000..0x602CDA70) */
#endif
#ifndef HI
#define HI   0x60800000u    /* slot 0 engine */
#endif
#ifndef SRC
#define SRC  0x60000000u
#endif
#ifndef LEN
#define LEN  70u
#endif
#ifndef MODE
#define MODE 0
#endif
#ifndef REPORT
#define REPORT 0            /* 1: name = 7 bytes read back at the match after the fill */
#endif

#define K 0xA5
#define W(a, b, c, d) ((((u32)(d) << 24) | ((u32)(c) << 16) | ((u32)(b) << 8) | (a)) ^ 0xA5A5A5A5u)
/* "String 1" as immediates (no .rodata: nothing slot-relative to relocate) */
#define ENC0 W('S', 't', 'r', 'i')
#define ENC1 W('n', 'g', ' ', '1')

static u8 hexd(u32 v) { v &= 15; return v < 10 ? '0' + v : 'A' - 10 + v; }

void scan_main(u8 *name)
{
    u32 w[2];
    u8 *n = (u8 *)w, r[7];
    u32 count = 0, first = 0, form = 0, i, j;

    w[0] = ENC0;
    w[1] = ENC1;
    __asm__ volatile("" : "+r"(w[0]), "+r"(w[1]));   /* keep the folding off */
    w[0] ^= 0xA5A5A5A5u;
    w[1] ^= 0xA5A5A5A5u;

    for (const volatile u8 *p = (const volatile u8 *)LO;
         p < (const volatile u8 *)HI; p++) {
        u32 f = 0;
#ifdef NEEDLE32              /* native little endian 32-bit word, form 3 */
        if (p[0] == (u8)NEEDLE32) {
            f = 3;
            for (j = 1; j < 4; j++)
                if (p[j] != (u8)(NEEDLE32 >> (8 * j))) { f = 0; break; }
        }
#elif defined(NEEDLE8)              /* 8-nibble value (e.g. VCF CUTOFF FREQ H), form 2 */
        if (p[0] == (u8)(NEEDLE8 >> 28)) {
            f = 2;
            for (j = 1; j < 8; j++)
                if (p[j] != ((NEEDLE8 >> (28 - 4 * j)) & 15)) { f = 0; break; }
        }
#else
        if (p[0] == n[0]) {
            f = 1;
            for (i = 1; i < 8; i++)
                if (p[i] != n[i]) { f = 0; break; }
        } else if (p[0] == (u8)(n[0] >> 4)) {
            f = 2;
            for (j = 0; j < 16; j++) {
                u8 want = (j & 1) ? (n[j >> 1] & 15) : (n[j >> 1] >> 4);
                if (p[j] != want) { f = 0; break; }
            }
        }
#endif
        if (f) {
            if (!count) { first = (u32)p; form = f; }
            count++;
        }
    }

    if (first) {
        volatile u8 *dst = (volatile u8 *)first;
        const volatile u8 *src = (const volatile u8 *)SRC;
        for (i = 0; i < LEN; i++) {
#if MODE == 1
            dst[2 * i]     = src[i] >> 4;
            dst[2 * i + 1] = src[i] & 15;
#else
            dst[i] = src[i];
#endif
        }
    }

    if (count > 0xFFFF)
        count = 0xFFFF;
#if REPORT
    if (first) {
        const volatile u8 *q = (const volatile u8 *)first;
        for (i = 0; i < 7; i++)
            r[i] = q[i];
    } else
#endif
    {
    r[0] = first >> 24; r[1] = first >> 16; r[2] = first >> 8; r[3] = first;
    r[4] = count >> 8;  r[5] = count;       r[6] = form;
    }
    for (i = 0; i < 7; i++) {
        name[2 * i]     = hexd(r[i] >> 4);
        name[2 * i + 1] = hexd(r[i]);
    }
    name[14] = 0;
}
