@ MIDI send probe v2: engine hook on the DSP parameter sender (0x60A0B41E).
@ On every panel move it builds a 15-byte SysEx and hands it to the OS MIDI
@ transmit sequence (ctor -> set SysEx -> wrap -> router -> dtors, exactly as
@ the Identity handler at 0x60005A32).
@
@ v1 tried router bits 0..5 individually: none reached a physical port. The
@ router analysis shows those are internal buses; the actual MIDI output goes
@ through masks 0x800/0x1000 (logical port tags) which recurse into the router
@ with base mask 0x00081603. This version sends twice: once with mask 0x1000,
@ once with 0x800, each tagged differently in byte 3. The Identity handler
@ also sets m[6] = port & 15; we set it to 0 (unknown port) then 9 (the port
@ that triggers 0x800 in the router), then try port 8 (triggers 0x1000 in the
@ recursion).
@
@ Prologue/epilogue are copied verbatim from the verified mem_name/press.s.
@ Assembled with -defsym:
@   HOOK    run address of this code (cave in region 1, slot 1) = 0x60a07000
@   RET     0x60a0b425
@   LIT     0x60a0b470
@   CTOR    0x600753cc  Msg ctor(&m)
@   SETSX   0x600754bc  set SysEx(&m, buf, len)
@   WRAP    0x600754c6  wrap(&w, &m)
@   ROUTER  0x6000345c  router(mask, &w)
@   DTORW   0x600753f4  dtor(&w)
@   DTORM   0x600753e0  dtor(&m)
    .syntax unified
    .thumb
    .text
    .global press
    .thumb_func
press:
base:
    push    {r0-r7, lr}
    sub     sp, sp, #52            @ scratch: buf[16] @sp, Msg @sp+16, Wrap @sp+32

    @ ===== send #1: mask 0x1000, tag=0x10 =====
    adr     r1, tmpl
    mov     r0, sp
    movs    r2, #15
1:  ldrb    r3, [r1], #1
    strb    r3, [r0], #1
    subs    r2, r2, #1
    bne     1b
    movs    r3, #0x10              @ tag = 0x10 -> mask 0x1000
    strb    r3, [sp, #3]

    add     r0, sp, #16
    ldr     r4, =(CTOR + 1)
    blx     r4
    movs    r3, #0                 @ m[6] = 0
    strb    r3, [sp, #(16 + 6)]
    add     r0, sp, #16
    mov     r1, sp
    movs    r2, #15
    ldr     r4, =(SETSX + 1)
    blx     r4
    add     r0, sp, #32
    add     r1, sp, #16
    ldr     r4, =(WRAP + 1)
    blx     r4
    mov     r0, #0x1000            @ mask = 0x1000
    add     r1, sp, #32
    ldr     r4, =(ROUTER + 1)
    blx     r4
    add     r0, sp, #32
    ldr     r4, =(DTORW + 1)
    blx     r4
    add     r0, sp, #16
    ldr     r4, =(DTORM + 1)
    blx     r4

    @ ===== send #2: mask 0x800, tag=0x08 =====
    adr     r1, tmpl
    mov     r0, sp
    movs    r2, #15
2:  ldrb    r3, [r1], #1
    strb    r3, [r0], #1
    subs    r2, r2, #1
    bne     2b
    movs    r3, #0x08              @ tag = 0x08 -> mask 0x800
    strb    r3, [sp, #3]

    add     r0, sp, #16
    ldr     r4, =(CTOR + 1)
    blx     r4
    movs    r3, #9                 @ m[6] = 9 (port 9 -> 0x800 path in router)
    strb    r3, [sp, #(16 + 6)]
    add     r0, sp, #16
    mov     r1, sp
    movs    r2, #15
    ldr     r4, =(SETSX + 1)
    blx     r4
    add     r0, sp, #32
    add     r1, sp, #16
    ldr     r4, =(WRAP + 1)
    blx     r4
    mov     r0, #0x800             @ mask = 0x800
    add     r1, sp, #32
    ldr     r4, =(ROUTER + 1)
    blx     r4
    add     r0, sp, #32
    ldr     r4, =(DTORW + 1)
    blx     r4
    add     r0, sp, #16
    ldr     r4, =(DTORM + 1)
    blx     r4

    @ ===== send #3: mask 0x1A20 (the base mask the router itself uses), tag=0x1A =====
    adr     r1, tmpl
    mov     r0, sp
    movs    r2, #15
3:  ldrb    r3, [r1], #1
    strb    r3, [r0], #1
    subs    r2, r2, #1
    bne     3b
    movs    r3, #0x1A              @ tag = 0x1A -> mask 0x1A20
    strb    r3, [sp, #3]

    add     r0, sp, #16
    ldr     r4, =(CTOR + 1)
    blx     r4
    movs    r3, #0
    strb    r3, [sp, #(16 + 6)]
    add     r0, sp, #16
    mov     r1, sp
    movs    r2, #15
    ldr     r4, =(SETSX + 1)
    blx     r4
    add     r0, sp, #32
    add     r1, sp, #16
    ldr     r4, =(WRAP + 1)
    blx     r4
    movw    r0, #0x1A20            @ mask = 0x1A20 (bits 5,9,11,12)
    add     r1, sp, #32
    ldr     r4, =(ROUTER + 1)
    blx     r4
    add     r0, sp, #32
    ldr     r4, =(DTORW + 1)
    blx     r4
    add     r0, sp, #16
    ldr     r4, =(DTORM + 1)
    blx     r4

    add     sp, sp, #52
    @ ---- epilogue: verbatim from press.s ----
    adr.w   r12, base
    ldr     r1, =(LIT - HOOK)
    add     r1, r1, r12
    ldr     r1, [r1]
    ldr     r1, [r1]               @ displaced: r0 = [[LIT]]
    str     r1, [sp]               @ ...delivered through the saved r0 slot
    ldr     r1, =(RET - HOOK)
    add     r12, r12, r1           @ r12 = RET (Thumb bit already set)
    pop     {r0-r7, lr}            @ restore the sender's arguments
    push    {r3, lr}               @ displaced: push {r3, lr}
    bx      r12
    .ltorg
    .align  2
tmpl:
    .byte   0xF0, 0x7D, 0x01, 0x00, 0x02, 0x03, 0x04, 0x05
    .byte   0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0xF7
