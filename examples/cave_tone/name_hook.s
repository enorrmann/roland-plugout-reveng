@ EFFECT TONE method replacement (as hook.c: tone inverted) that ALSO writes the
@ 16-char name "MEM TONE vv" over NAME0 of preset 1 (DEST, one nibble per byte),
@ vv = the TONE value (0..255) as two hex digits. Run address: cave 0x60A07000.
@ r0 = self, r1 = arg, r2 = value (as in hook.c). Integer registers only; s0 and
@ the arguments are preserved until the original body below.
    .syntax unified
    .thumb
    .text
    .global tone_hook
    .thumb_func
tone_hook:
    push    {r0-r6, lr}
    ldr     r4, =DEST
    adr     r5, tname
    movs    r6, #16
1:  ldrb    r0, [r5], #1
    lsrs    r3, r0, #4
    strb    r3, [r4], #1
    ands    r3, r0, #15
    strb    r3, [r4], #1
    subs    r6, r6, #1
    bne     1b
    subs    r4, r4, #14             @ nibble pair of char 9
    .macro  HEXCHAR reg             @ ASCII hex digit of nibble `reg` as 2 nibbles
    cmp     \reg, #10
    bcs     2f
    movs    r3, #3
    strb    r3, [r4], #1
    strb    \reg, [r4], #1
    b       3f
2:  movs    r3, #4
    strb    r3, [r4], #1
    subs    \reg, \reg, #9
    strb    \reg, [r4], #1
3:
    .endm
    ldr     r6, [sp, #8]            @ saved r2 = value
    lsrs    r5, r6, #4
    HEXCHAR r5
    ands    r6, r6, #15
    HEXCHAR r6
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
    .ascii  "MEM TONE 00     "
