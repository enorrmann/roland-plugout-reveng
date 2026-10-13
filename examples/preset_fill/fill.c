/*
 * Engine-side fill, called from press.s on every DSP parameter send (knob,
 * button, switch). Writes LEN bytes of the OS image (SRC) at DEST, the RAM
 * image of the first preset name found by scanfill.c (0x60445957, one nibble
 * per byte). MODE 0: raw bytes, MODE 1: each byte as two nibble bytes (hi, lo),
 * which survives the 7-bit SysEx data of RQ1 and decodes exactly.
 * No VFP, no globals, no rodata: nothing slot-relative to relocate.
 */
typedef unsigned char u8;
typedef unsigned int u32;

#ifndef DEST
#define DEST 0x60445957u
#endif
#ifndef SRC
#define SRC  0x60000000u
#endif
#ifndef LEN
#define LEN  70u
#endif
#ifndef MODE
#define MODE 1
#endif

void fill_main(void)
{
    volatile u8 *dst = (volatile u8 *)DEST;
    const volatile u8 *src = (const volatile u8 *)SRC;
    u32 i;

    for (i = 0; i < LEN; i++) {
#if MODE == 1
        dst[2 * i]     = src[i] >> 4;
        dst[2 * i + 1] = src[i] & 15;
#else
        dst[i] = src[i];
#endif
    }
}
