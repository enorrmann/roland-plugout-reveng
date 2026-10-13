@ Region 0 "init" hook: copies SRC_LEN bytes of OS memory, as hex text, over the
@ PLUG-OUT name in the SYNINFO block (image base + 0x10), then jumps to the
@ original init. Position independent (the name is found PC-relative), so the
@ same bytes work in every slot. Assembled with -defsym:
@   SRC        address to read          (default 0x60000000, OS image start)
@   CAVE_OFF   offset of this code from the image base
@   INIT_OFF   offset of the original init function (Thumb) from the image base
    .syntax unified
    .thumb
    .text
    .global hook
    .thumb_func
hook:
base:
    push    {r0-r5, lr}
    adr     r4, base
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
    adr.w   r12, base               @ original init, PC-relative: slot independent
    ldr     r5, =(INIT_OFF + 1 - CAVE_OFF)
    add     r12, r12, r5
    pop     {r0-r5, lr}
    bx      r12

@ r3 = nibble -> ASCII hex
nib:
    cmp     r3, #10
    ite     lo
    addlo   r3, r3, #'0'
    addhs   r3, r3, #('A' - 10)
    bx      lr
    .ltorg
