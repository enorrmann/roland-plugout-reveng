@ Region 0 init entry: calls scan_main(&name) and continues with the original
@ init. Position independent (name and init are found PC-relative), so the same
@ bytes work in every slot. -defsym CAVE_OFF / INIT_OFF as in ../mem_name.
    .syntax unified
    .thumb
    .text
    .global entry
    .thumb_func
entry:
base:
    push    {r0-r3, r4, lr}         @ 24 bytes: AAPCS stack alignment
    adr.w   r0, base
    ldr     r1, =(CAVE_OFF - 0x10)
    subs    r0, r0, r1              @ r0 = &name[0]
    bl      scan_main
    adr.w   r12, base               @ original init, PC-relative
    ldr     r1, =(INIT_OFF + 1 - CAVE_OFF)
    add     r12, r12, r1
    pop     {r0-r3, r4, lr}
    bx      r12
    .ltorg
