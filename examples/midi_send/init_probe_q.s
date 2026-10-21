@ Region 0 "init" probe (2026-10-21): counts what enters the master's MIDI input queue, readable live.
@
@ The only producer of the queue at 0x20080370 is the wrapper at 0x601F4D66 (push {r4,lr}; ldr r0,[r0]; bl 0x601F4A6C;
@ ldmia; movs r0,#4; b.w 0x601F24D8). Every wrap that enters the queue goes through it, whatever the port. This hook
@ replaces its first 10 bytes (nop; ldr.w pc,[pc,#0]; .word qhook) and qhook does the same work, then calls the
@ producer and returns to the same tail:
@   type = wrap[7] >> 1: 3 = SysEx message, 4 = USB-MIDI packet (CC, notes, ...).
@
@ Live witness: the counts are written as nibbles at DEST, the RAM image of NAME0 of preset 1 (slot 3), the same
@ layout diag.s used, so `read_diag.py` reads them by RQ1 (no reinstall):
@   decoded bytes 0..3  = SysEx count (u32, little endian)
@   decoded bytes 4..7  = USB-MIDI packet count
@   decoded bytes 8..11 = 0xDEADBEEF (set by init: proves the hook ran)
@
@ Assembled with -defsym CAVE_OFF, INIT_OFF. Addresses are the stock OS ones (no relocation).
    .syntax unified
    .thumb
    .text
    .global hook
    .thumb_func
hook:
base:
    push    {r0-r7, lr}

    @ patch the producer wrapper: 0x601F4D66 = nop; ldr.w pc,[pc,#0]; .word qhook+1
    ldr     r4, =WRAPPER
    ldr     r0, =0xBF00            @ nop (bytes 00 BF)
    strh    r0, [r4]
    ldr     r0, =0xF000F8DF        @ ldr.w pc,[pc,#0] (bytes DF F8 00 F0)
    str     r0, [r4, #2]
    adr     r0, qhook
    orr     r0, r0, #1
    str     r0, [r4, #6]

    @ witness: counts start at zero, marker DEADBEEF
    adr     r2, count_sx
    movs    r0, #0
    str     r0, [r2]
    str     r0, [r2, #4]
    ldr     r0, =0xDEADBEEF
    str     r0, [r2, #8]
    bl      publish

    adr.w   r12, base              @ original init, PC-relative
    ldr     r5, =(INIT_OFF + 1 - CAVE_OFF)
    add     r12, r12, r5
    pop     {r0-r7, lr}
    bx      r12

@ qhook: r0 = the object of the wrapper (as the wrapper received it). Replaces 0x601F4D66..0x601F4D6F.
    .thumb_func
qhook:
    push    {r4, lr}
    ldr     r0, [r0, #0]           @ displaced: wrap pointer
    mov     r4, r0
    ldrb    r1, [r4, #7]
    lsrs    r1, r1, #1             @ type
    adr     r2, count_sx
    cmp     r1, #3
    beq     2f
    adr     r2, count_pkt
    cmp     r1, #4
    bne     3f
2:  ldr     r3, [r2]
    adds    r3, r3, #1
    str     r3, [r2]
3:  bl      publish

    mov     r0, r4
    ldr     r3, =(PRODUCER + 1)
    blx     r3                     @ displaced bl: producer(wrap)

    pop     {r4, lr}               @ wrapper tail: ldmia; movs r0,#4; b.w 0x601F24D8
    movs    r0, #4
    ldr     r3, =(TAIL + 1)
    bx      r3

@ publish: writes the 12 bytes at count_sx as 24 nibble bytes at DEST (what diag.s does for its 64 bytes).
@ Clobbers r0-r3, r5.
    .thumb_func
publish:
    push    {r4, lr}
    adr     r4, count_sx
    ldr     r5, =DEST
    movs    r2, #12
4:  ldrb    r0, [r4], #1
    lsrs    r1, r0, #4
    strb    r1, [r5], #1
    ands    r1, r0, #15
    strb    r1, [r5], #1
    subs    r2, r2, #1
    bne     4b
    pop     {r4, pc}

    .ltorg
    .align  2
count_sx:
    .word   0
count_pkt:
    .word   0
    .word   0
