@ Engine-side hook: every call to the DSP parameter sender (0x60A0B41E, used by
@ every knob / button / switch the panel moves) writes one chunk of OS memory
@ into the RAM image of the first preset's NAME0 leaf (DEST, one nibble per
@ byte, 128 bytes), where the host can read it back with RQ1 (tools/patch_read.py
@ style). Layout of the 128 nibble bytes (decoded: 64 bytes):
@
@     bytes 0..3    big endian address of the chunk (SRC + CHUNK * counter)
@     bytes 4..63   the 60 bytes at that address
@
@ The counter lives in region 0 (name + COUNTER, zero area, known writable),
@ starts at 0 and is incremented on every call; the first press reads SRC.
@
@ Displaced from the sender entry (4 bytes, replaced by a B.W to `press`):
@     60a0b41e  ldr  r0, [pc, #0x50]    @ r0 = [0x60A0B470] (literal)
@     60a0b420  push {r3, lr}
@     60a0b422  ldr  r0, [r0]
@ Register contract of the sender (r0 ignored, r1..r3 + s0 are arguments) is
@ kept: r0-r7, lr are saved and the VFP registers are never touched.
@ Assembled with -defsym:
@   SRC        first address to read           (default 0x60000000)
@   DEST       RAM address of NAME0 of preset 1 of slot 3 (0x60445957)
@   HOOK       run address of this code        (cave in region 1, slot 1)
@   NAME       run address of the plugout name (region 0 image + 0x10, slot 1)
@   COUNTER    offset of the counter from the name (image + 0x10 + COUNTER)
@   RET        0x60A0B425  (sender after the displaced instructions, Thumb bit set)
@   LIT        0x60A0B470  (literal word the displaced ldr read)
    .syntax unified
    .thumb
    .text
    .global press
    .thumb_func
press:
base:
.ifdef HANG
    b       .                       @ diagnostic: spin forever if the hook is reached
.endif
    push    {r0-r7, lr}
    adr.w   r4, base               @ r4 = run address of this code
    ldr     r7, =(HOOK - NAME)
    subs    r4, r4, r7              @ r4 = &name[0] (PC-relative: any slot)
    ldr     r6, [r4, #COUNTER]
    adds    r5, r6, #1
    str     r5, [r4, #COUNTER]      @ counter++ (region 0 is writable RAM)
.ifdef TEST
    @ TEST build: no memory read. Writes the 16-char name "MEM HOOK 0      "
    @ with char 9 replaced by the hex digit of the press number, so that a
    @ panel move is visible on the display / by RQ1 without decoding anything.
    ands    r6, r5, #15             @ r5 = counter + 1: press number, low nibble
    ldr     r5, =DEST
    adr     r1, tname
    movs    r2, #16
3:  ldrb    r0, [r1], #1
    lsrs    r3, r0, #4
    strb    r3, [r5], #1
    ands    r3, r0, #15
    strb    r3, [r5], #1
    subs    r2, r2, #1
    bne     3b
    subs    r5, r5, #14             @ nibble pair of char 9 (bytes 18, 19)
    cmp     r6, #10
    bcs     5f
    movs    r3, #3                  @ '0'..'9' = 0x30..0x39
    strb    r3, [r5], #1
    strb    r6, [r5]
    b       6f
5:  movs    r3, #4                  @ 'A'..'F' = 0x41..0x46
    strb    r3, [r5], #1
    subs    r6, r6, #9
    strb    r6, [r5]
6:
.else
    movs    r7, #60
    muls    r6, r7, r6              @ r6 = 60 * counter (before the increment)
    ldr     r7, =SRC
    adds    r1, r7, r6              @ r1 = chunk address
    ldr     r5, =DEST
    mov     r7, r1                  @ header: the address, 8 nibbles, MSB first
    movs    r2, #8
2:  lsrs    r3, r7, #28
    strb    r3, [r5], #1
    lsls    r7, r7, #4
    subs    r2, r2, #1
    bne     2b
    movs    r2, #60                 @ payload: each byte as two nibbles
1:  ldrb    r0, [r1], #1
    lsrs    r3, r0, #4
    strb    r3, [r5], #1
    ands    r3, r0, #15
    strb    r3, [r5], #1
    subs    r2, r2, #1
    bne     1b
.endif
    adr.w   r12, base
    ldr     r1, =(LIT - HOOK)
    add     r1, r1, r12
    ldr     r1, [r1]
    ldr     r1, [r1]                @ displaced: r0 = [[LIT]]
    str     r1, [sp]                @ ...delivered through the saved r0 slot
    ldr     r1, =(RET - HOOK)
    add     r12, r12, r1            @ r12 = RET (Thumb bit set below)
    pop     {r0-r7, lr}             @ restore the sender's arguments
    push    {r3, lr}                @ displaced: push {r3, lr}
    bx      r12
    .ltorg
.ifdef TEST
    .align  2
tname:
    .ascii  "MEM HOOK 0      "
.endif
