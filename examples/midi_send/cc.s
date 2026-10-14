@ MIDI CC probe: engine hook on the DSP parameter sender (0x60A0B41E).
@ On every panel move it sends ONE CC exactly as the OS panel path does after
@ its Tx rewrite (router 0x60003660..0x600036EE): USB-MIDI packet with cable 4,
@ wrap_packet, router(0x1A20), Wrap dtor. Packet = 4B BF 66 01 (CC 102 = 1 on
@ channel 16) so it cannot be confused with the panel's own CCs.
@
@ Liveness/diagnostic: after wrap_packet and BEFORE the router it writes
@ pkt[4] + Wrap[12] (decoded bytes 0..15) and the marker 0xDEADBEEF (decoded
@ bytes 36..39) as nibbles over NAME0 of preset 1 (DEST, slot 3); read back with
@ read_diag.py. Marker present + no CC = hook runs, router drops the packet.
@
@ Prologue/epilogue copied verbatim from the verified mem_name/press.s.
@ Assembled with -defsym: HOOK, RET, LIT (as probe.s), WRAPP (0x600753ea),
@ ROUTER (0x6000345c), DTORW (0x600753f4), DEST (0x60445957).
    .syntax unified
    .thumb
    .text
    .global press
    .thumb_func
press:
base:
    push    {r0-r7, lr}
    sub     sp, sp, #16            @ pkt[4] @sp, Wrap[12] @sp+4

    ldr     r3, =0x0166BF4B        @ bytes 4B BF 66 01 (little endian)
    str     r3, [sp]
    add     r0, sp, #4
    mov     r1, sp
    ldr     r4, =(WRAPP + 1)
    blx     r4                     @ wrap_packet(&w, pkt)

    ldr     r5, =DEST              @ pkt + Wrap -> NAME0 as nibbles
    mov     r1, sp
    movs    r2, #16
1:  ldrb    r0, [r1], #1
    lsrs    r3, r0, #4
    strb    r3, [r5], #1
    ands    r3, r0, #15
    strb    r3, [r5], #1
    subs    r2, r2, #1
    bne     1b
    ldr     r5, =(DEST + 72)       @ marker DEADBEEF at decoded bytes 36..39
    ldr     r0, =0xEFBEADDE        @ emitted MSB first -> bytes EF BE AD DE
    movs    r2, #8
2:  lsrs    r3, r0, #28            @ high nibble first
    strb    r3, [r5], #1
    lsls    r0, r0, #4
    subs    r2, r2, #1
    bne     2b
    movw    r0, #0x1A20
    add     r1, sp, #4
    ldr     r4, =(ROUTER + 1)
    blx     r4                     @ router(0x1A20, &w)
    add     r0, sp, #4
    ldr     r4, =(DTORW + 1)
    blx     r4                     @ Wrap dtor

    add     sp, sp, #16
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
