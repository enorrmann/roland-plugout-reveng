@ Region 0 "init" hook that reads A:/hola.txt from the SD card and reports it
@ over MIDI (OS CPU, same mechanism as midi_send/init_cc.s). Runs before the
@ stock init, so it works while the PLUG-OUT is being installed.
@
@ OS functions used (BASE_FIRMWARE, from work/os/m0_core0):
@   FOPEN  0x60057E54  fopen(path, mode)            -> FILE* (0 = error)
@   FREAD  0x60057EA8  fread(buf, size, count, fp)  -> items read
@   FCLOSE 0x60057E28  fclose(fp)
@   WRAPP / ROUTER / DTORW: same as init_cc.s (MIDI output router)
@
@ Witness, as CC on channel 16 (CC 102 = 0x66 is the stock pattern):
@   CC 0x67 value = bytes read (0x7F = fopen failed)
@   CC 0x68 value = each byte of the file, & 0x7F, in order
@ Assembled with -defsym CAVE_OFF, INIT_OFF, FOPEN, FREAD, FCLOSE, WRAPP,
@ ROUTER, DTORW, MASK.
    .syntax unified
    .thumb
    .text
    .global hook
    .thumb_func
hook:
base:
    push    {r0-r7, lr}            @ 36 bytes
    sub     sp, sp, #44            @ buf[0..31] at sp, FILE* at sp+32; 80 total

    adr     r0, path
    adr     r1, mode
    ldr     r4, =(FOPEN + 1)
    blx     r4                     @ fopen("A:/hola.txt", "r")
    str     r0, [sp, #32]
    cbnz    r0, opened
    movs    r0, #0x67              @ fopen failed: CC 0x67 = 0x7F
    movs    r1, #0x7F
    bl      send_cc
    b       done

opened:
    mov     r0, sp                 @ fread(buf, 1, 32, fp)
    movs    r1, #1
    movs    r2, #32
    ldr     r3, [sp, #32]
    ldr     r4, =(FREAD + 1)
    blx     r4
    mov     r6, r0                 @ r6 = bytes read

    movs    r0, #0x67              @ CC 0x67 = count
    mov     r1, r6
    bl      send_cc

    movs    r5, #0
send_loop:
    cmp     r5, r6
    bge     send_end
    ldrb    r1, [sp, r5]
    and     r1, r1, #0x7F
    movs    r0, #0x68
    bl      send_cc
    adds    r5, r5, #1
    b       send_loop
send_end:
    ldr     r0, [sp, #32]          @ fclose(fp)
    ldr     r4, =(FCLOSE + 1)
    blx     r4

done:
    add     sp, sp, #44
    adr.w   r12, base              @ original init, PC-relative: slot independent
    ldr     r5, =(INIT_OFF + 1 - CAVE_OFF)
    add     r12, r12, r5
    pop     {r0-r7, lr}
    bx      r12

@ send_cc(r0 = cc, r1 = value): one CC on cable 4, channel 16, through the router.
send_cc:
    push    {r4, r5, lr}           @ 12 bytes
    sub     sp, sp, #20            @ pkt at sp, Wrap object at sp+4: 32 total
    mov     r4, r0
    mov     r5, r1
    movw    r2, #0xBF4B            @ 4B BF: cable 4, CIN B, status CC ch 16
    orr     r2, r2, r4, lsl #16    @ controller
    orr     r2, r2, r5, lsl #24    @ value
    str     r2, [sp]
    add     r0, sp, #4
    mov     r1, sp
    ldr     r3, =(WRAPP + 1)
    blx     r3                     @ wrap_packet(&w, pkt)
    ldr     r0, =MASK
    add     r1, sp, #4
    ldr     r3, =(ROUTER + 1)
    blx     r3                     @ router(MASK, &w)
    add     r0, sp, #4
    ldr     r3, =(DTORW + 1)
    blx     r3                     @ Wrap dtor
    add     sp, sp, #20
    pop     {r4, r5, pc}

    .ltorg
path:
    .asciz  "A:/hola.txt"
mode:
    .asciz  "r"
