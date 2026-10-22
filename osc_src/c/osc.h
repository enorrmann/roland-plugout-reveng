/* osc.h -- the types the decompiled SYSTEM-8 OS (m0_core0) is written in.
 *
 * Ghidra's `undefined` widths are the operands of a 32-bit Cortex-M core.
 * Every fixed address the code touches has a name (osc_d.h) and one accessor:
 *
 *   OSC_P4(D4_01003E0)  the 4-byte place at that address, as an lvalue
 *   OSC_FN_1C2          a function of the image, by address
 *
 * The name *is* the address: that is what lets the generated C link at -- and
 * mean -- the image's own addresses.
 */
#ifndef OSC_H
#define OSC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef unsigned char undefined;
typedef unsigned char undefined1;
typedef unsigned char byte;
typedef unsigned char uchar;
typedef unsigned short undefined2;
typedef unsigned int undefined4;
typedef uint64_t undefined8;
typedef long long longlong;
typedef unsigned long long ulonglong;

typedef unsigned int uint;
typedef int32_t bool32;
typedef unsigned short ushort;
typedef void (*code)(void);         /* a function pointer, Ghidra's `code` */

/* the floating-point specials the decompiler names */
#define INFINITY (__builtin_inff())
#define NAN (__builtin_nanf(""))

#include "osc_d.h"

/* --------------------------------------------------------------- places */
#define OSC_P1(x) (*(volatile uint8_t  *)(uintptr_t)(x))
#define OSC_P2(x) (*(volatile uint16_t *)(uintptr_t)(x))
#define OSC_P4(x) (*(volatile uint32_t *)(uintptr_t)(x))
#define OSC_P8(x) (*(volatile uint64_t *)(uintptr_t)(x))
/* the same for a place whose width the decompiler did not fix (a word) */
#define OSC_PA(x) (*(volatile uint32_t *)(uintptr_t)(x))
/* a call through a word the decompiler could not type: the function whose
 * address the word holds, called with whatever the call site passes */
/* a call through a word (a function pointer read from memory): C cannot
 * dereference a non-pointer, so this stands in for it.  It is the only thing
 * in the generated C that is not a plain operation, and a reader can find
 * every indirect call by searching for it. */
#define OSC_CALL(fn) (*(void (*)())&(uint32_t){(uint32_t)(uintptr_t)(fn)})
/* a call through the word at an address (`(**(code **)x)(a)`) */
#define OSC_CALL_AT(p) OSC_CALL(*(uint32_t *)(uintptr_t)(p))
/* an address of the kernel's RAM for the OS: not part of this image */
#define OSC_RAM(x) (*(volatile uint32_t *)(uintptr_t)(x))

/* ------------------------------------------- decompiler helper functions */
/* Ghidra's ``PACK``/``SUB``/``CONCAT``/``CARRY`` family: the arithmetic the
 * core does on neighbouring fields of a register, written out. */
#define SUB41(a, b) ((uint32_t)(((uint32_t)(a)) - ((uint32_t)(b))))
#define SUB21(a, b) ((uint32_t)(((uint32_t)(a)) - ((uint32_t)(b))))
#define CARRY4(a, b) ((uint32_t)(((uint32_t)(a)) < ((uint32_t)(b))))
#define SCARRY4(a, b) ((uint32_t)(((int32_t)(a)) > (int32_t)(0x7fffffff - (uint32_t)(b))))
#define SBORROW4(a, b) ((uint32_t)(((int32_t)(a)) < (int32_t)(0x80000000u + (uint32_t)(b))))
#define CONCAT11(a, b) ((uint32_t)((((uint32_t)(a)) << 8) | ((uint32_t)(b) & 0xffu)))
#define CONCAT12(a, b) ((uint32_t)((((uint32_t)(a)) << 16) | ((uint32_t)(b) & 0xffffu)))
#define CONCAT13(a, b) ((uint32_t)((((uint32_t)(a)) << 24) | ((uint32_t)(b) & 0xffffffu)))
#define CONCAT21(a, b) ((uint32_t)((((uint32_t)(a)) << 8) | ((uint32_t)(b) & 0xffu)))
#define CONCAT22(a, b) ((uint32_t)((((uint32_t)(a)) << 16) | ((uint32_t)(b) & 0xffffu)))
#define CONCAT31(a, b) ((uint32_t)((((uint32_t)(a)) << 24) | ((uint32_t)(b) & 0xffffffu)))
#define CONCAT44(a, b) ((((uint64_t)(uint32_t)(a)) << 32) | ((uint32_t)(b)))
/* the decompiler's SUB<n><m>: the low n bytes of (a - b) */
#define SUB168(a, b) ((uint64_t)((uint64_t)(a) - (uint64_t)(b)))
#define ZEXT21(a) ((uint32_t)((uint32_t)(a) & 0xffffu))
#define SEXT21(a) ((uint32_t)(int32_t)(int16_t)(a))
#define LZCOUNT(a) ((uint32_t)((a) ? __builtin_clz((uint32_t)(a)) : 32u))
#define ABS(a) ((int32_t)((a) < 0 ? -(int32_t)(a) : (int32_t)(a)))

/* Ghidra's naming for the decompiler's own helpers; the expressions stay in
 * the body and these aliases only make them readable */
#define LZCOUNT4(a) LZCOUNT(a)

/* a jump into the middle of a function.  C cannot take a label's address, so
 * the target of such an indirect jump is a place of its own: it stops the
 * program (and names the address) instead of jumping into the unknown. */
#define LAB_01004dac osc_bad_jump

/* ---------------------------------------------- instructions Ghidra noted */
/* System and coprocessor instructions the decompiler could not express.
 * They stop the program instead of returning a made-up value. */
void osc_stop(const char *what);
void osc_bad_jump(void);
static inline uint32_t osc_unsupported(const char *what)
{
    osc_stop(what);
    for (;;) { }
}
static inline uint32_t isCurrentModePrivileged(void) { return osc_unsupported("isCurrentModePrivileged"); }
static inline void setBasePriority(uint32_t v) { (void)v; osc_unsupported("setBasePriority"); }
static inline void setMainStackPointer(uint32_t v) { (void)v; osc_unsupported("setMainStackPointer"); }
static inline void DataSynchronizationBarrier(void) { (void)osc_unsupported("dsb"); }
static inline void ExclusiveAccess(void) { (void)osc_unsupported("clrex"); }
static inline uint32_t hasExclusiveAccess(void) { return osc_unsupported("ldrex"); }
static inline uint32_t halt_baddata(void) { return osc_unsupported("undef"); }
static inline uint32_t int3(void) { return osc_unsupported("bkpt"); }
static inline uint32_t coprocessor_load(void) { return osc_unsupported("mrc"); }
static inline uint32_t coprocessor_store(void) { return osc_unsupported("mcr"); }
static inline uint32_t coprocessor_moveto2(void) { return osc_unsupported("mcr"); }
static inline uint32_t coprocessor_storelong(void) { return osc_unsupported("mcrr"); }
static inline uint32_t VectorSignedToFloat(uint32_t v) { (void)v; return osc_unsupported("vcvt"); }
static inline uint32_t VectorFloatToSignedRound(uint32_t v) { (void)v; return osc_unsupported("vcvt"); }
static inline uint32_t VectorFloatToUnsigned(uint32_t v) { (void)v; return osc_unsupported("vcvt"); }
static inline uint32_t VectorUnsignedToFloat(uint32_t v) { (void)v; return osc_unsupported("vcvt"); }
static inline uint32_t VectorMax(uint32_t a, uint32_t b) { (void)a; (void)b; return osc_unsupported("vmax"); }
static inline uint32_t UnsignedSaturate(uint32_t v) { (void)v; return osc_unsupported("usat"); }
static inline uint32_t UnsignedDoesSaturate(uint32_t v) { (void)v; return osc_unsupported("usat"); }

#endif /* OSC_H */
