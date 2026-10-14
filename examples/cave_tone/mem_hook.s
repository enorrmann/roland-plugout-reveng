@ EFFECT TONE replacement (as hook.c: tone inverted) that ALSO copies 4 bytes of
@ OS memory at BASE + TONE value (0..255) into the RAM of VCF CUTOFF FREQ H
@ (CUTH, native 32-bit word; the OS keeps it as raw bytes, RQ1 shows it as 8
@ nibbles of the little endian value: b0 = low byte). The host sets TONE = n,
@ then reads CUTOFF H with RQ1 -> the 4 bytes at BASE + n.
@ Run address: cave 0x60A07000. Assembled with -defsym:
@   BASE   first address read (TONE 0 reads BASE + 0)
@   CUTH   RAM address of the register that receives the 4 bytes: the EFFECT TONE
@          value of the live edit buffer (TONE mode) or any other int8x4 (see make_mem.sh)
@   STRIDE bytes advanced per TONE step (default 4: step n = word n, no overlap)
@ r0 = self, r1 = arg, r2 = value (as in hook.c). Integer registers only.
    .syntax unified
    .thumb
    .text
    .ifndef STRIDE
    .set    STRIDE, 4
    .endif
    .global tone_hook
    .thumb_func
tone_hook:
    push    {r0-r6, lr}
    ldr     r4, =BASE
    movs    r3, #STRIDE
    mla     r4, r2, r3, r4          @ r2 = TONE value: address BASE + value * STRIDE
    ldr     r5, =CUTH
    movs    r6, #4
1:  ldrb    r0, [r4], #1            @ native word, copied byte by byte (CUTH may be
    strb    r0, [r5], #1            @ odd): RQ1 then returns the 32-bit LE value
    subs    r6, r6, #1
    bne     1b
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
