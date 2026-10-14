@ EFFECT TONE replacement (as hook.c: tone inverted) that ALSO writes the PLUG-OUT
@ name (region 0 image + 0x10, found PC-relative so it works in every slot) as
@ "MEM TONE vv" + NUL, vv = TONE value in hex. Tests whether `info` re-reads the
@ slot name at run time. Run address: cave 0x60A07000 (slot 1).
@   -defsym HOOK=0x60a07000 -defsym NAME=0x61800010
    .syntax unified
    .thumb
    .text
    .global tone_hook
    .thumb_func
tone_hook:
base:
    push    {r0-r6, lr}
    adr.w   r4, base                @ run address of this code (even: not the function symbol)
    ldr     r5, =(HOOK - NAME)
    subs    r4, r4, r5              @ r4 = &name[0]
    adr     r5, tname
    movs    r6, #15
1:  ldrb    r0, [r5], #1
    strb    r0, [r4], #1
    subs    r6, r6, #1
    bne     1b
    movs    r0, #0
    strb    r0, [r4]                @ NUL (name field is 16 bytes)
    subs    r4, r4, #6              @ char 9 of the name
    ldr     r6, [sp, #8]            @ saved r2 = value
    lsrs    r5, r6, #4
    cmp     r5, #10
    ite     lo
    addlo   r5, r5, #'0'
    addhs   r5, r5, #('A' - 10)
    strb    r5, [r4], #1
    ands    r6, r6, #15
    cmp     r6, #10
    ite     lo
    addlo   r6, r6, #'0'
    addhs   r6, r6, #('A' - 10)
    strb    r6, [r4]
    pop     {r0-r6, lr}
    @ ---- original body of cave_tone/hook.c ----
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
    .align  2
tname:
    .ascii  "MEM TONE 00    "
