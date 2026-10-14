@ EFFECT TONE replacement (as cave_tone/hook.c: tone inverted, the verified
@ code-cave hook) that FIRST sends one MIDI CC through the OS output router,
@ exactly as the panel path does after its Tx rewrite (BASE_FIRMWARE §7.7):
@   pkt = 4B BF 66 vv   (cable 4, CIN B; CC 102 on channel 16, vv = TONE >> 1)
@   wrap_packet(&w, pkt); router(0x1A20, &w); Wrap dtor(&w)
@ Outcomes when moving TONE with OD/DS/FZ selected:
@   tone inverted + BF 66 vv on a port  -> CC send works
@   tone inverted, no CC                -> hook runs, router drops the packet
@   freeze                              -> the send sequence faults
@ v1 (no gate) hung the SYSTEM-8 as soon as slot 3 was SELECTED: preset load
@ calls the TONE method too, and the router call from that context hangs. So
@ the send now only happens when TONE == GATE (default 255, knob at max).
@ v2 (gate 255, router 0x1A20): selection OK, inverted tone OK, HANG at 255
@ with no CC on any port. STAGE (default 2) bisects it:
@   STAGE=0  only call a pure OS-image function: 0x600059AA(sp, 0x7F)
@            (MIDI class dev-id check; with r1 = 0x7F it returns 1 at once)
@   STAGE=1  wrap_packet + Wrap dtor only (no router) -> HANGS too (v1.11)
@   STAGE=2  + router(MASK)
@ Run address: cave 0x60A07000 (slot 1). r0 = self, r1 = arg, r2 = value.
@ Assembled with -defsym WRAPP (0x600753ea), ROUTER (0x6000345c),
@ DTORW (0x600753f4), MASK (default 0x1A20).
    .syntax unified
    .thumb
    .text
    .ifndef MASK
    .set    MASK, 0x1A20
    .endif
    .ifndef STAGE
    .set    STAGE, 2
    .endif
    .ifndef GATE
    .set    GATE, 255
    .endif
    .global tone_hook
    .thumb_func
tone_hook:
    cmp     r2, #GATE
    bne     body                   @ not at the gate value: inverted tone only
    push    {r0-r6, lr}
    sub     sp, sp, #16            @ pkt[4] @sp, Wrap[12] @sp+4
    .if STAGE == 0
    mov     r0, sp
    movs    r1, #0x7F
    ldr     r4, =0x600059AB
    blx     r4                     @ pure OS function, returns r0 = 1
    .else
    lsrs    r3, r2, #1             @ vv = TONE (0..255) >> 1
    lsls    r3, r3, #24
    ldr     r0, =0x0066BF4B        @ bytes 4B BF 66 00
    orrs    r3, r3, r0
    str     r3, [sp]
    add     r0, sp, #4
    mov     r1, sp
    ldr     r4, =(WRAPP + 1)
    blx     r4                     @ wrap_packet(&w, pkt)
    .if STAGE >= 2
    ldr     r0, =MASK
    add     r1, sp, #4
    ldr     r4, =(ROUTER + 1)
    blx     r4                     @ router(MASK, &w)
    .endif
    add     r0, sp, #4
    ldr     r4, =(DTORW + 1)
    blx     r4                     @ Wrap dtor
    .endif
    add     sp, sp, #16
    pop     {r0-r6, lr}
body:
    @ ---- original body of cave_tone/hook.c (tone inverted) ----
    push    {r4, r5, r6, lr}
    mov     r4, r0
    mov     r5, r1
    movs    r0, #0
    movs    r1, #16
    bl      prm_curve
    ldr     r2, [r4, #20]
    mov     r3, r5
    ldr     r1, [r4, #4]
    movs    r0, #0
    pop     {r4, r5, r6, lr}
    vneg.f32 s0, s0
    b.w     dsp_send
    .ltorg
