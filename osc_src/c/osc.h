/* osc.h -- the types the decompiled SYSTEM-8 OS is written against.
 *
 * The image at 0x60000000 was recovered with Ghidra; every function kept the
 * types the decompiler inferred, which are these four undefined widths plus
 * the standard C integers.  A few Ghidra conventions need a definition to be
 * valid C:
 *
 *   undefined/undefined1/2/4/8   the byte at that place, loadable as 1/2/4/8
 *                                bytes little-endian (``undefined4 x;``)
 *   uint, ushort, uchar, byte    the fixed-width names the decompiler prints
 *   code                         a function pointer (Ghidra's cast of a call
 *                                through a value)
 *   the {pack,char} accessors    defined in osc.c: 8) 7) 6) 5) 4) 3) 2) 1)
 *                                ``SUB84(v, n)`` = byte n of v; the CONCAT and
 *                                ZEXT families mirror Ghidra's arithmetic on
 *                                neighbouring fields
 *
 * Data lives at absolute addresses: every byte of the image has a name,
 * ``D_<address>`` (``osc_d.h``), and the reads/writes the decompiler folded
 * back into the text address the same fixed location the original code did.
 */
#ifndef OSC_H
#define OSC_H

#include <stdint.h>
#include <stddef.h>

/* ------------------------------------------------------------- Ghidra types */
typedef unsigned char undefined;
typedef unsigned char undefined1;
typedef unsigned char byte;
typedef unsigned char uchar;
typedef signed char schar;
typedef unsigned short undefined2;
typedef short sword;
typedef unsigned int undefined4;
typedef int sdword;
typedef uint64_t undefined8;
typedef uint64_t qword;

#ifndef __UINT32_TYPE__
typedef unsigned int uint;
#endif
typedef unsigned short ushort;
typedef int bool_;          /* unused; keeps 'bool' meaning the C99 type */

typedef void (*code)(void);

/* --------------------------------------------------------------- the image */
#define OSC_BASE   0x60000000u
#define OSC_END    0x60130E00u      /* m0_core0: the master BMC core 0 OS */
#define TEXT_END   0x60094800u      /* code ends here; tables, strings, data */
#define OSC_RAM    0x20000000u      /* the OS's variables at run time */
#define OSC_KERNEL 0x01000000u      /* uT-Kernel, loaded at start-up */

/* every byte of the image as an lvalue, by address */
#define OSC8(a)  (*(volatile uint8_t  *)(uintptr_t)(a))
#define OSC16(a) (*(volatile uint16_t *)(uintptr_t)(a))
#define OSC32(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define OSCP(a)  (*(void * volatile *)(uintptr_t)(a))

/* an instruction the decompiler could not follow (see osc_c.sh) */
[[noreturn]] void osc_unsupported(uint32_t from, uint32_t target);

#endif /* OSC_H */
