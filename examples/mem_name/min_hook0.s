@ Minimal region 0 ("model") for a PLUG-OUT that only dumps memory.
@
@ The init function (SYNINFO +0x24) does two things and returns:
@   1. writes the 7 bytes at SRC as 14 hex digits over the name (image + 0x10)
@   2. returns the address of a descriptor whose callbacks all point to a stub
@      that returns 0 (the stock init returns a pointer to such a table of
@      function pointers; nothing real sits behind it here)
@
@ Position independent: the same bytes work in every slot. Assembled with
@   --defsym SRC=<address>  --defsym CAVE_OFF=<offset of this code in the image>
    .syntax unified
    .thumb
    .text
    .global hook
    .thumb_func
hook:
base:
    push    {r4-r6, lr}
    adr     r4, base
    bic     r4, r4, #1
    ldr     r5, =(CAVE_OFF - 0x10)
    subs    r4, r4, r5              @ r4 = &name[0]
    ldr     r1, =SRC
    movs    r2, #7                  @ 7 bytes -> 14 chars, name stays NUL terminated
1:  ldrb    r0, [r1], #1
    lsrs    r3, r0, #4
    bl      nib
    strb    r3, [r4], #1
    ands    r3, r0, #15
    bl      nib
    strb    r3, [r4], #1
    subs    r2, r2, #1
    bne     1b
    adr     r1, stub
    orr     r1, r1, #1              @ Thumb address of the stub
    adr     r0, desc
    str     r1, [r0, #4]
    str     r1, [r0, #8]
    str     r1, [r0, #24]
    pop     {r4-r6, pc}             @ r0 = descriptor

@ r3 = nibble -> ASCII hex
    .thumb_func
nib:
    cmp     r3, #10
    ite     lo
    addlo   r3, r3, #'0'
    addhs   r3, r3, #('A' - 10)
    bx      lr

    .thumb_func
stub:
    movs    r0, #0
    bx      lr
    .ltorg
    .align  2
desc:
    .space  32
