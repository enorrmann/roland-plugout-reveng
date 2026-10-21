@ Region 0 "init" hook: forward every SysEx that the host sends to the slaves (2026-10-21).
@
@ The host SysEx receiver is the C++ object at OBJ (master RAM, 0x20082008). When a SysEx is complete the
@ reassembler calls vtable[1] of the object, which is `bx lr` in the stock firmware (0x01103906), so the
@ message is dropped there. This hook:
@   1. copies the 16 vtable words of the object (VT_OLD = 0x0113EC30) into a new table in the cave,
@      with entry [1] replaced by fwd;
@   2. points the object at the new table;
@   3. fwd(obj): reads the length at obj+0x208 and the message at obj+8, and sends it as USB-MIDI packets on
@      cable 8 through the OS router (the same path as the CC in init_cc.s, not the image sender 0x010125C1);
@   4. writes "SX FWD OK" over its own name (witness that the hook ran), then runs the original init.
@
@ Region 0 runs on the OS CPU, where the router is safe to call (see init_cc.s).
@ Assembled with -defsym CAVE_OFF, INIT_OFF, WRAPP, ROUTER, DTORW, MASK, VT_OLD, OBJ.
    .syntax unified
    .thumb
    .text
    .global hook
    .thumb_func
hook:
base:
    push    {r0-r7, lr}
    sub     sp, sp, #20            @ keeps sp 8-byte aligned

    @ 1. new vtable = copy of VT_OLD (16 words) with [1] = fwd
    ldr     r4, =VT_OLD
    adr     r5, vt_new
    movs    r6, #16
1:  ldr     r0, [r4], #4
    str     r0, [r5], #4
    subs    r6, r6, #1
    bne     1b
    adr     r5, vt_new
    adr     r0, fwd
    orr     r0, r0, #1             @ Thumb address
    str     r0, [r5, #4]

    @ 2. object vtable pointer = vt_new
    ldr     r4, =OBJ
    adr     r0, vt_new
    str     r0, [r4]

    @ 2b. the second receiver built by the same constructor (0x60134708): object A at OBJ2 = 0x20081DFC,
    @ its stock vtable VT_OLD2 = 0x0113E49C (vtable[1] = bx lr as well). Same treatment, its own table.
    ldr     r4, =VT_OLD2
    adr     r5, vt_new2
    movs    r6, #16
7:  ldr     r0, [r4], #4
    str     r0, [r5], #4
    subs    r6, r6, #1
    bne     7b
    adr     r5, vt_new2
    adr     r0, fwd
    orr     r0, r0, #1
    str     r0, [r5, #4]
    ldr     r4, =OBJ2
    adr     r0, vt_new2
    str     r0, [r4]

    add     sp, sp, #20

    @ 3. witness: own name = image + 0x10
    adr.w   r5, base
    ldr     r4, =(CAVE_OFF - 0x10)
    subs    r5, r5, r4
    adr     r1, msg
    movs    r2, #10                @ "SX FWD OK" + NUL
2:  ldrb    r0, [r1], #1
    strb    r0, [r5], #1
    subs    r2, r2, #1
    bne     2b

    adr.w   r12, base              @ original init, PC-relative: slot independent
    ldr     r5, =(INIT_OFF + 1 - CAVE_OFF)
    add     r12, r12, r5
    pop     {r0-r7, lr}
    bx      r12

@ fwd(obj): r0 = obj. Sends obj[0x208] bytes from obj+8 as USB-MIDI packets on cable 8.
@ Packet = cable<<4 | CIN in byte 0, up to 3 message bytes after it. CIN 4 = SysEx continues, 5/6/7 = SysEx ends
@ with 1/2/3 bytes. Each packet: wrap_packet(&w, pkt); router(MASK, &w); Wrap dtor(&w).
    .thumb_func
fwd:
    push    {r4-r8, lr}
    sub     sp, sp, #20            @ pkt at sp (4 bytes), Wrap at sp+4 (12 bytes), padded
    mov     r4, r0                 @ obj

    @ witness: CC 102 = 1 on channel 16 (4B BF 66 01, cable 4), as init_cc.s. If the host sees it after each
    @ SysEx, fwd ran and the router output reached the host.
    ldr     r0, =0x0166BF4B
    str     r0, [sp]
    add     r0, sp, #4
    mov     r1, sp
    ldr     r7, =(WRAPP + 1)
    blx     r7
    ldr     r0, =MASK
    add     r1, sp, #4
    ldr     r7, =(ROUTER + 1)
    blx     r7
    add     r0, sp, #4
    ldr     r7, =(DTORW + 1)
    blx     r7

    ldrh    r5, [r4, #0x208]       @ bytes left
    add     r6, r4, #8             @ next message byte
3:  cbz     r5, 9f
    movs    r8, #3                 @ bytes in this packet
    movs    r7, #4                 @ CIN 4: more bytes follow
    cmp     r5, #3
    bhi     4f
    mov     r8, r5                 @ last packet: 1..3 bytes
    add     r7, r7, r5            @ CIN 5/6/7
4:  movs    r0, #0x80              @ cable 8 in the high nibble
    orr     r0, r0, r7
    movs    r2, #0
5:  cmp     r2, r8
    bge     6f
    ldrb    r1, [r6, r2]
    add     r3, r2, #1
    lsl     r3, r3, #3             @ shift = 8 * (k + 1)
    lsl     r1, r1, r3
    orr     r0, r0, r1
    adds    r2, r2, #1
    b       5b
6:  str     r0, [sp]
    add     r0, sp, #4
    mov     r1, sp
    ldr     r7, =(WRAPP + 1)
    blx     r7                     @ wrap_packet(&w, pkt)
    ldr     r0, =MASK
    add     r1, sp, #4
    ldr     r7, =(ROUTER + 1)
    blx     r7                     @ router(MASK, &w)
    add     r0, sp, #4
    ldr     r7, =(DTORW + 1)
    blx     r7                     @ Wrap dtor
    add     r6, r6, r8
    subs    r5, r5, r8
    b       3b
9:  add     sp, sp, #20
    pop     {r4-r8, pc}

    .ltorg
msg:
    .asciz  "SX FWD OK"
    .align  2
vt_new:
    .space  64                     @ 16 words, filled by the hook at run time
vt_new2:
    .space  64
