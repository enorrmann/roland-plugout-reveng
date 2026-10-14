@ Region 0 "init" hook: QSPI flash dump over SysEx (init_bulk.s, but the data
@ comes from flash instead of memory). For each of COUNT messages it reads LEN
@ bytes at flash offset SRC + i*LEN with the OS flash read
@ FREAD(0, 0, &{offset, length, dst}) (BASE_FIRMWARE §8.14) into the stack and
@ sends them in the same format as init_bulk.s:
@
@   F0 7D 01 <flash offset as 8 nibbles> <LEN bytes as nibbles> F7
@
@ then writes "FLASH OK" over its own name (or "FLSH ERR" and stops if a read
@ fails) and runs the original init. Host side: bulk_dump.py --flash.
@ Assembled with -defsym CAVE_OFF, INIT_OFF, SRC, LEN (<= 32), COUNT, MASK,
@ FREAD, CTOR, SETSX, WRAPM, ROUTER, DTORW, DTORM.
    .syntax unified
    .thumb
    .text
    .set    BUFSZ, 80              @ 3 + 8 + 2*32 + 1 = 76 max
    .set    DAT, BUFSZ             @ flash data[32] at sp + 80
    .set    REQ, DAT + 32          @ {offset, length, dst} at sp + 112
    .set    MSG, REQ + 16          @ Msg[8]  at sp + 128
    .set    WRP, MSG + 8           @ Wrap[12] at sp + 136
    .set    FRAME, WRP + 20        @ 156; with the 52-byte push: 208, aligned
    .global hook
    .thumb_func
hook:
base:
    push    {r0-r11, lr}           @ 52 bytes
    sub     sp, sp, #FRAME
    ldr     r8, =SRC               @ r8 = current address
    ldr     r7, =COUNT             @ r7 = messages left
next:
    str     r8, [sp, #REQ]         @ read LEN bytes of flash at r8 into DAT
    movs    r0, #LEN
    str     r0, [sp, #(REQ + 4)]
    add     r0, sp, #DAT
    str     r0, [sp, #(REQ + 8)]
    movs    r0, #0
    movs    r1, #0
    add     r2, sp, #REQ
    ldr     r4, =(FREAD + 1)
    blx     r4
    cmp     r0, #0
    bne     fail

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
    add     r1, sp, #DAT           @ data, 2 nibbles per byte
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
    adr     r1, msg
    b       done
fail:
    adr     r1, err
done:
    add     sp, sp, #FRAME

    adr.w   r5, base               @ witness: own name = image + 0x10
    ldr     r4, =(CAVE_OFF - 0x10)
    subs    r5, r5, r4
    movs    r2, #9                 @ 8 chars + NUL
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
    .align  2
msg:
    .asciz  "FLASH OK"
    .align  2
err:
    .asciz  "FLSH ERR"
