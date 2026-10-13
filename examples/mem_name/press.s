@ Engine-side hook: every call to the DSP parameter sender (0x60A0B41E, used by
@ every knob / button / switch the panel moves) first advances a counter, then
@ writes the 7 bytes at SRC + 7*counter, as 14 hex digits, over the PLUG-OUT
@ name in region 0's SYNINFO (image + 0x10), then runs the displaced original
@ instructions and continues the sender at 0x60A0B424.
@
@ Displaced from the sender entry (4 bytes, replaced by a B.W to `press`):
@     60a0b41e  ldr  r0, [pc, #0x50]    @ r0 = [0x60A0B470] (literal)
@     60a0b420  push {r3, lr}
@     60a0b422  ldr  r0, [r0]
@ Register contract of the sender (r0 ignored, r1..r3 + s0 are arguments) is
@ kept: r0-r7, lr are saved and the VFP registers are never touched.
@ Assembled with -defsym:
@   SRC        first address to read           (default 0x60000000)
@   HOOK       run address of this code        (cave in region 1, slot 1)
@   NAME       run address of the name         (region 0 image + 0x10, slot 1)
@   RET        0x60A0B425  (sender after the displaced instructions, Thumb bit set)
@   LIT        0x60A0B470  (literal word the displaced ldr read)
    .syntax unified
    .thumb
    .text
    .global press
    .thumb_func
press:
base:
    push    {r0-r7, lr}
    adr.w   r4, base               @ r4 = run address of this code
    adr     r5, counter
    ldr     r6, [r5]
    adds    r6, r6, #1
    str     r6, [r5]                @ counter++
    movs    r7, #7
    muls    r6, r7, r6              @ r6 = 7 * counter
    ldr     r7, =SRC
    adds    r1, r7, r6              @ r1 = SRC + 7*counter
    ldr     r7, =(HOOK - NAME)
    subs    r4, r4, r7              @ r4 = &name[0] (PC-relative: any slot)
    movs    r2, #7
1:  ldrb    r0, [r1], #1
    lsrs    r3, r0, #4
    bl      nib
    strb    r3, [r4], #1
    ands    r3, r0, #15
    bl      nib
    strb    r3, [r4], #1
    subs    r2, r2, #1
    bne     1b
    movs    r3, #0
    strb    r3, [r4]                @ keep the name NUL terminated
    adr.w   r12, base
    ldr     r1, =(LIT - HOOK)
    add     r1, r1, r12
    ldr     r1, [r1]
    ldr     r1, [r1]                @ displaced: r0 = [[LIT]]
    str     r1, [sp]                @ ...delivered through the saved r0 slot
    ldr     r1, =(RET - HOOK)
    add     r12, r12, r1            @ r12 = RET (Thumb bit set below)
    pop     {r0-r7, lr}             @ restore the sender's arguments
    push    {r3, lr}                @ displaced: push {r3, lr}
    bx      r12

@ r3 = nibble -> ASCII hex
nib:
    cmp     r3, #10
    ite     lo
    addlo   r3, r3, #'0'
    addhs   r3, r3, #('A' - 10)
    bx      lr
    .ltorg
    .align  2
counter:
    .word   0
