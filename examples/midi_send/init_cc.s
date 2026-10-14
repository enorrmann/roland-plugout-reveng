@ Region 0 "init" hook (same mechanism as mem_names/hook.s: SYNINFO init ->
@ cave 0x61800100 -> original init). Region 0 runs on the OS CPU (the region 1
@ engine runs on another processor and hangs on any OS call, see
@ BASE_FIRMWARE §7.9), so this is the place to call the MIDI output router.
@
@ Sends ONE CC exactly as the panel path does after its Tx rewrite:
@   pkt = 4B BF 66 01  (cable 4, CIN B; CC 102 = 1 on channel 16)
@   wrap_packet(&w, pkt); router(MASK, &w); Wrap dtor(&w)
@ then, as a witness that the router returned, writes "INIT CC OK" + NUL over
@ the PLUG-OUT name of its own slot (PC-relative), and runs the original init.
@ Assembled with -defsym CAVE_OFF, INIT_OFF, WRAPP, ROUTER, DTORW, MASK.
    .syntax unified
    .thumb
    .text
    .global hook
    .thumb_func
hook:
base:
    push    {r0-r7, lr}            @ 36 bytes
    sub     sp, sp, #20            @ +20 = 56: sp stays 8-byte aligned
    ldr     r0, =0x0166BF4B        @ bytes 4B BF 66 01
    str     r0, [sp]
    add     r0, sp, #4
    mov     r1, sp
    ldr     r4, =(WRAPP + 1)
    blx     r4                     @ wrap_packet(&w, pkt)
    ldr     r0, =MASK
    add     r1, sp, #4
    ldr     r4, =(ROUTER + 1)
    blx     r4                     @ router(MASK, &w)
    add     r0, sp, #4
    ldr     r4, =(DTORW + 1)
    blx     r4                     @ Wrap dtor
    add     sp, sp, #20

    adr.w   r5, base               @ witness: own name = image + 0x10
    ldr     r4, =(CAVE_OFF - 0x10)
    subs    r5, r5, r4
    adr     r1, msg
    movs    r2, #11                @ "INIT CC OK" + NUL
1:  ldrb    r0, [r1], #1
    strb    r0, [r5], #1
    subs    r2, r2, #1
    bne     1b

    adr.w   r12, base              @ original init, PC-relative: slot independent
    ldr     r5, =(INIT_OFF + 1 - CAVE_OFF)
    add     r12, r12, r5
    pop     {r0-r7, lr}
    bx      r12
    .ltorg
msg:
    .asciz  "INIT CC OK"
