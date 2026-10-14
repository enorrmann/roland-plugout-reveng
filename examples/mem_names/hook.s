@ Region 0 "init" hook: reads 3 * 8 bytes of OS memory at SRC and writes them as
@ 16 hex characters each over the PLUG-OUT names of the three slots:
@
@     slot 1 name (0x61800010)  <-  SRC +  0 .. +7
@     slot 2 name (0x61A00010)  <-  SRC +  8 .. +15
@     slot 3 name (0x61C00010)  <-  SRC + 16 .. +23
@
@ then jumps to the original init. The 16 chars fill the whole 16-byte name
@ field (the id follows, there is no room for a NUL). The destinations are
@ absolute, so the same bytes work whichever slot the plugout is installed in.
@ Assembled with -defsym SRC, CAVE_OFF, INIT_OFF (as mem_name/hook.s).
    .syntax unified
    .thumb
    .text
    .global hook
    .thumb_func
hook:
base:
    push    {r0-r7, lr}
    ldr     r1, =SRC
    ldr     r5, =0x61800010         @ name of slot 1
    movs    r6, #3                  @ three slots
0:  mov     r4, r5
    movs    r2, #8                  @ 8 bytes -> 16 chars
1:  ldrb    r0, [r1], #1
    lsrs    r3, r0, #4
    bl      nib
    strb    r3, [r4], #1
    ands    r3, r0, #15
    bl      nib
    strb    r3, [r4], #1
    subs    r2, r2, #1
    bne     1b
    add     r5, r5, #0x200000       @ next slot (regions 0 are 2 MiB apart)
    subs    r6, r6, #1
    bne     0b
    adr.w   r12, base               @ original init, PC-relative: slot independent
    ldr     r5, =(INIT_OFF + 1 - CAVE_OFF)
    add     r12, r12, r5
    pop     {r0-r7, lr}
    bx      r12

@ r3 = nibble -> ASCII hex
nib:
    cmp     r3, #10
    ite     lo
    addlo   r3, r3, #'0'
    addhs   r3, r3, #('A' - 10)
    bx      lr
    .ltorg
