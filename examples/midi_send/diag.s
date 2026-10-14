@ Diagnostic probe: engine hook on the DSP parameter sender (0x60A0B41E).
@ On each panel move, calls ctor -> setSysEx -> wrap exactly as the Identity
@ handler, then writes 64 bytes of stack (buf + Msg + Wrap) to the preset NAME0
@ area (one nibble per byte, 128 bytes) so the host can read them with RQ1 and
@ verify the structures. Does NOT call the router (safe, no MIDI output attempt).
@
@ Layout of 128 nibble bytes at DEST (decoded = 64 bytes):
@   bytes  0..15  buf[16] (the SysEx template + padding)
@   bytes 16..23  Msg[8]  (after ctor + setSysEx)
@   bytes 24..35  Wrap[12] (after wrap)
@   bytes 36..39  marker 0xDEADBEEF (proves this code ran)
@   bytes 40..63  zero padding
@
@ Prologue/epilogue from verified mem_name/press.s.
@ Assembled with -defsym:
@   HOOK    0x60a07000
@   RET     0x60a0b425
@   LIT     0x60a0b470
@   DEST    0x60445957  (RAM image of NAME0 of preset 1, slot 3)
@   CTOR    0x600753cc
@   SETSX   0x600754bc
@   WRAP    0x600754c6
    .syntax unified
    .thumb
    .text
    .global press
    .thumb_func
press:
base:
    push    {r0-r7, lr}
    sub     sp, sp, #68            @ 64 data bytes + 4 for marker

    @ clear the 68-byte area
    mov     r0, sp
    movs    r1, #0
    movs    r2, #68
clr:
    strb    r1, [r0], #1
    subs    r2, r2, #1
    bne     clr

    @ buf[0..14] = SysEx template at sp+0
    adr     r1, tmpl
    mov     r0, sp
    movs    r2, #15
cp:
    ldrb    r3, [r1], #1
    strb    r3, [r0], #1
    subs    r2, r2, #1
    bne     cp

    @ Msg ctor at sp+16
    add     r0, sp, #16
    ldr     r4, =(CTOR + 1)
    blx     r4

    @ m[6] = 0 (port nibble)
    movs    r3, #0
    strb    r3, [sp, #(16 + 6)]

    @ setSysEx(&m, buf, 15) at sp+16
    add     r0, sp, #16
    mov     r1, sp
    movs    r2, #15
    ldr     r4, =(SETSX + 1)
    blx     r4

    @ wrap(&w, &m): w at sp+24, m at sp+16
    add     r0, sp, #24
    add     r1, sp, #16
    ldr     r4, =(WRAP + 1)
    blx     r4

    @ marker at sp+36..sp+39 = 0xDEADBEEF
    ldr     r3, =0xDEADBEEF
    str     r3, [sp, #36]

    @ ---- write 64 bytes from sp to DEST as nibbles (128 bytes) ----
    ldr     r5, =DEST
    mov     r1, sp
    movs    r2, #64
wrt:
    ldrb    r0, [r1], #1
    lsrs    r3, r0, #4
    strb    r3, [r5], #1
    ands    r3, r0, #15
    strb    r3, [r5], #1
    subs    r2, r2, #1
    bne     wrt

    add     sp, sp, #68
    @ ---- epilogue: verbatim from press.s ----
    adr.w   r12, base
    ldr     r1, =(LIT - HOOK)
    add     r1, r1, r12
    ldr     r1, [r1]
    ldr     r1, [r1]               @ displaced: r0 = [[LIT]]
    str     r1, [sp]
    ldr     r1, =(RET - HOOK)
    add     r12, r12, r1
    pop     {r0-r7, lr}
    push    {r3, lr}
    bx      r12
    .ltorg
    .align  2
tmpl:
    .byte   0xF0, 0x7D, 0x01, 0x00, 0x02, 0x03, 0x04, 0x05
    .byte   0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0xF7
