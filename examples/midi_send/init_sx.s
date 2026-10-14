@ Region 0 "init" hook (as init_cc.s, verified): sends ONE SysEx with LEN bytes
@ of OS memory at SRC, using the same sequence as the OS Identity handler
@ (0x60005A32): Msg ctor, Msg[6] = port, set SysEx, wrap_msg, router, dtors.
@
@   F0 7D 01 <SRC as 8 nibbles, MSB first> <LEN bytes as 2 nibbles each,
@   high nibble first> F7                      (7D = non-commercial ID)
@
@ Then writes "INIT SX OK" over its own name (router returned) and runs the
@ original init. Port nibble Msg[6] = 4: the cable the panel path uses and the
@ one that reached USB1 + DIN with mask 0x1A20 in init_cc.s.
@ Assembled with -defsym CAVE_OFF, INIT_OFF, SRC, LEN (<= 32), MASK, and the
@ OS addresses CTOR, SETSX, WRAPM, ROUTER, DTORW, DTORM.
    .syntax unified
    .thumb
    .text
    .set    BUFSZ, 80              @ 3 + 8 + 2*32 + 1 = 76 max
    .set    MSG, BUFSZ             @ Msg[8]  at sp + 80
    .set    WRP, BUFSZ + 8         @ Wrap[12] at sp + 88
    .set    FRAME, BUFSZ + 24      @ 104; with the 36-byte push: 140 -> pad
    .global hook
    .thumb_func
hook:
base:
    push    {r0-r7, lr}            @ 36 bytes
    sub     sp, sp, #(FRAME + 4)   @ 36 + 108 = 144: 8-byte aligned
    mov     r5, sp                 @ r5 = write pointer into buf
    movs    r0, #0xF0
    strb    r0, [r5], #1
    movs    r0, #0x7D
    strb    r0, [r5], #1
    movs    r0, #0x01
    strb    r0, [r5], #1
    ldr     r0, =SRC               @ address, 8 nibbles MSB first
    movs    r2, #8
1:  lsrs    r3, r0, #28
    strb    r3, [r5], #1
    lsls    r0, r0, #4
    subs    r2, r2, #1
    bne     1b
    ldr     r1, =SRC               @ data, 2 nibbles per byte
    movs    r2, #LEN
2:  ldrb    r0, [r1], #1
    lsrs    r3, r0, #4
    strb    r3, [r5], #1
    and     r3, r0, #15
    strb    r3, [r5], #1
    subs    r2, r2, #1
    bne     2b
    movs    r0, #0xF7
    strb    r0, [r5], #1
    mov     r6, sp
    subs    r6, r5, r6             @ r6 = SysEx length

    add     r0, sp, #MSG
    ldr     r4, =(CTOR + 1)
    blx     r4                     @ Msg ctor(&m)
    movs    r0, #4
    strb    r0, [sp, #(MSG + 6)]   @ m[6] = port 4
    add     r0, sp, #MSG
    mov     r1, sp
    mov     r2, r6
    ldr     r4, =(SETSX + 1)
    blx     r4                     @ set SysEx(&m, buf, len)
    add     r0, sp, #WRP
    add     r1, sp, #MSG
    ldr     r4, =(WRAPM + 1)
    blx     r4                     @ wrap_msg(&w, &m)
    ldr     r0, =MASK
    add     r1, sp, #WRP
    ldr     r4, =(ROUTER + 1)
    blx     r4                     @ router(MASK, &w)
    add     r0, sp, #WRP
    ldr     r4, =(DTORW + 1)
    blx     r4                     @ Wrap dtor
    add     r0, sp, #MSG
    ldr     r4, =(DTORM + 1)
    blx     r4                     @ Msg dtor
    add     sp, sp, #(FRAME + 4)

    adr.w   r5, base               @ witness: own name = image + 0x10
    ldr     r4, =(CAVE_OFF - 0x10)
    subs    r5, r5, r4
    adr     r1, msg
    movs    r2, #11                @ "INIT SX OK" + NUL
3:  ldrb    r0, [r1], #1
    strb    r0, [r5], #1
    subs    r2, r2, #1
    bne     3b

    adr.w   r12, base              @ original init, PC-relative
    ldr     r5, =(INIT_OFF + 1 - CAVE_OFF)
    add     r12, r12, r5
    pop     {r0-r7, lr}
    bx      r12
    .ltorg
msg:
    .asciz  "INIT SX OK"
