;; EDM_STR (docs/MUTABLE_MEMORY.md §3.3.5): a Karplus-Strong string per voice in the delay memory, on master B's main
;; (0x1000). The slave sends voice v's excitation (the kernel's output) on IOMD 6006 + v (host_b.s head_b). Each voice
;; has its own line through master B's map table (region 0, model.c edm_str): write word W = STR_W + 8v at address
;; STR_A0 + v STR_DA, read words W + 1, W + 3, W + 5, W + 7 at that address + d, d + 1, d + 2, d + 3, so that this
;; sample's loads give y0..y3 = w[n - d - 1 .. n - d - 4] (the read is one sample later than its entry: 48001 for
;; 48000, §3.3.4). Per voice:
;;   F0 = a (y0 + y2) + c y1, F1 = a (y1 + y3) + c y2      (c = 1 - 2a: linear phase, 1 sample, brightness a)
;;   z  = F0 + f (F1 - F0)                                 (age d + 2 + f: the period)
;;   w  = x + fb z  -> W; the voice's output w
;; a C01D, c C01E (shared), f C020 + v, fb C028 + v (per voice) are written by region 0. Assembled one line per
;; instruction, as edm_ecf.s (bank-2 transfers are not in the scheduler's proven shapes): one instruction between a
;; result and its use, the spacers are `mov r3, r3` / `mov r7, r7` where nothing else fits.
;; Registers: r4 = a, r5 = c, r6 = sum of the voices; r0..r3, r7 per voice.

.fragment master_str
ld r4, [c{STR_PA:03x}]
ld r5, [c{STR_PC:03x}]
mov r6, #0.0
.for v in range(NV)
    ;; voice {v}
    ld r0, [{STR_W + 8 * v + 1:04x}]
    ld r1, [{STR_W + 8 * v + 3:04x}]
    ld r2, [{STR_W + 8 * v + 5:04x}]
    ld r3, [{STR_W + 8 * v + 7:04x}]
    addxy r0, r0, r2
    mul r7, r1, r5
    addxy r1, r1, r3
    mul r0, r0, r4
    mul r2, r2, r5
    mul r1, r1, r4
    addxy r0, r0, r7
    addxy r1, r1, r2
    ld r3, [c{STR_PF + v:03x}]
    subxy r1, r0, r1
    ld r2, [c{STR_PFB + v:03x}]
    mul r1, r1, r3
    ld r7, [{0x6006 + v:04x}]
    addxy r0, r0, r1
    mov r3, r3
    mul r0, r0, r2
    mov r3, r3
    addxy r0, r0, r7
    mov r3, r3
    st r0, [{STR_W + 8 * v:04x}]
    addxy r6, r6, r0
.endfor
;; the sum x STR_GAIN x VOLUME (C01C) -> the OS effect's send; its return -> the output, as master.s
ld r7, [c{M_VOLUME:03x}]
mul r6, r6, #{STR_GAIN}
mov r3, r3
mul r6, r6, r7
mov r3, r3
st r6, [{FX_SEND[0]:04x}]
st r6, [{FX_SEND[1]:04x}]
ld r2, [{FX_RET[0]:04x}]
ld r3, [{FX_RET[1]:04x}]
mov r4, r4
st r2, [{M_OUT[0]:04x}]
st r3, [{M_OUT[1]:04x}]
mov r2, #0.0
.for x in M_ZERO
    st r2, [{x:04x}]
.endfor
