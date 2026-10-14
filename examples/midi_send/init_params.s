@ Region 0 "init" hook: read OS parameters and send them over SysEx.
@ For each u32 entry (part << 24 | group << 16 | id) of the table at TAB (NENT
@ entries) it calls the OS get callback [0x01000400](part, group, id, &v) and
@ stores v at VALS + 4*i (0xDEADBEEF first, so an unfilled entry shows), then
@ sends VALS like init_bulk.s: COUNT messages F0 7D 01 <addr> <LEN bytes> F7.
@ Host side: params_dump.py. Diagnostic only.
    .syntax unified
    .thumb
    .text
    .set    BUFSZ, 80              @ 3 + 8 + 2*32 + 1 = 76 max
    .set    MSG, BUFSZ             @ Msg[8]  at sp + 80
    .set    WRP, BUFSZ + 8         @ Wrap[12] at sp + 88
    .set    FRAME, BUFSZ + 28      @ 108; with the 52-byte push: 160, aligned
    .global hook
    .thumb_func
hook:
base:
    push    {r0-r11, lr}           @ 52 bytes
    sub     sp, sp, #FRAME
    ldr     r9, =TAB               @ fill VALS with get() results
    ldr     r10, =VALS
    ldr     r11, =NENT
0:  ldr     r0, =0xDEADBEEF
    str     r0, [r10]
    ldr     r3, [r9], #4
    lsrs    r0, r3, #24            @ part
    ubfx    r1, r3, #16, #8        @ group
    uxth    r2, r3                 @ id
    mov     r3, r10                @ &v
    ldr     r4, =0x01000400
    ldr     r4, [r4]
    blx     r4                     @ get(part, group, id, &v)
    add     r10, r10, #4
    subs    r11, r11, #1
    bne     0b
    ldr     r8, =VALS              @ r8 = current address
    ldr     r7, =COUNT             @ r7 = messages left
next:
    mov     r5, sp                 @ r5 = write pointer into buf
    movs    r0, #0xF0
    strb    r0, [r5], #1
    movs    r0, #0x7D
    strb    r0, [r5], #1
    movs    r0, #0x01
    strb    r0, [r5], #1
    mov     r0, r8                 @ address, 8 nibbles MSB first
    movs    r2, #8
1:  lsrs    r3, r0, #28
    strb    r3, [r5], #1
    lsls    r0, r0, #4
    subs    r2, r2, #1
    bne     1b
    mov     r1, r8                 @ data, 2 nibbles per byte
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

    add     r8, r8, #LEN
    subs    r7, r7, #1
    bne     next
    add     sp, sp, #FRAME

    adr.w   r5, base               @ witness: own name = image + 0x10
    ldr     r4, =(CAVE_OFF - 0x10)
    subs    r5, r5, r4
    adr     r1, msg
    movs    r2, #7                 @ "PRM OK" + NUL
3:  ldrb    r0, [r1], #1
    strb    r0, [r5], #1
    subs    r2, r2, #1
    bne     3b

    adr.w   r12, base              @ original init, PC-relative
    ldr     r5, =(INIT_OFF + 1 - CAVE_OFF)
    add     r12, r12, r5
    pop     {r0-r11, lr}
    bx      r12
    .ltorg
msg:
    .asciz  "PRM OK"
