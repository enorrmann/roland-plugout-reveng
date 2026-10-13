@ Engine hook: the DSP parameter sender (0x60A0B41E, every panel parameter
@ change) first calls fill_main(), then runs its 3 displaced instructions and
@ continues at 0x60A0B424. See ../mem_name/press.s for the displacement.
@ Arguments r0-r3 and s0 are preserved (fill_main uses no VFP). Position
@ independent. -defsym HOOK/RET/LIT as in ../mem_name/engine_build.sh.
    .syntax unified
    .thumb
    .text
    .global press
    .thumb_func
press:
base:
    push    {r0-r6, lr}             @ 32 bytes: AAPCS stack alignment
    bl      fill_main
    adr.w   r12, base
    ldr     r1, =(LIT - HOOK)
    add     r1, r1, r12
    ldr     r1, [r1]
    ldr     r1, [r1]                @ displaced: r0 = [[LIT]]
    str     r1, [sp]                @ ...delivered through the saved r0 slot
    ldr     r1, =(RET - HOOK)
    add     r12, r12, r1            @ r12 = RET (Thumb bit included)
    pop     {r0-r6, lr}
    push    {r3, lr}                @ displaced: push {r3, lr}
    bx      r12
    .ltorg
