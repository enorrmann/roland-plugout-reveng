;; SYSTEM-S1CC host microcode: IDX_PROBE, what indexed loads read on the SYSTEM-1's slave DSP A when there are several
;; hset per sample (docs/vol6_reference_designs_applications/mutable_memory.md §4.2). Template syntax: make_s1cc.py frag(); scheduled (asm/dspsched.py).
;; Region 1 writes bank 5 A080 + i = (i + 1) / 128 (i = 0 .. 127). Each test k sets its own h, reads one word and
;; stores it at IDXP_R + 2k; a test of bank 0 reads, with ldidx, a constant the program stored the sample before.
;; Every result multiplies a saw of its own frequency (IDXP_F[k]); a reference saw (value 1) at IDXP_FREF; the sum
;; -> bank 5 A1F0 -> DSP B -> the output (host_b.s head_b). Recording: amplitude of tone k / reference = the value
;; read, (i + 1) / 128 -> the index i that the load really used.

.fragment idxp_head
; IDX_PROBE head: four ldb5 with h6, each its own hset (indices IDXP_HEAD)
.for k in range(len(IDXP_HEAD))
    mov r{k}, #{(0x80 + IDXP_HEAD[k] + 0.5) / 16384!r}
    hset h6, r{k}
    ldb5 r{4 + k}, 0
    st r{4 + k}, [{IDXP_R + 2 * k:04x}]
.endfor


.fragment idxp_tail
; IDX_PROBE tail: four ldb5 with h6, two ldb5h7 with h7 (IDXP_TAIL); bank 0: IDXP_B0 constants stored, read back the
; next sample by ldidx, each with its own hset, then a pair of ldidx (Y 0 and 1) under one hset; then the tones
.for k in range(len(IDXP_TAIL))
    mov r0, #{(0x80 + IDXP_TAIL[k][1] + 0.5) / 16384!r}
    hset h{IDXP_TAIL[k][0]}, r0
    {"ldb5" if IDXP_TAIL[k][0] == 6 else "ldb5h7"} r1, 0
    st r1, [{IDXP_R + 2 * (4 + k):04x}]
.endfor
.for j in range(len(IDXP_B0))
    mov r1, #{IDXP_B0[j][1]!r}
    st r1, [{IDXP_B0[j][0]:04x}]
.endfor
.for j in range(4)
    mov r0, #{(IDXP_B0[j][0] + 1 + 0.5) / 16384!r}
    hset h6, r0
    ldidx r1, 0
    st r1, [{IDXP_R + 2 * (10 + j):04x}]
.endfor
mov r0, #{(IDXP_B0[4][0] + 1 + 0.5) / 16384!r}
hset h6, r0
ldidx r1, 0
ldidx r2, 1
st r1, [{IDXP_R + 2 * 14:04x}]
st r2, [{IDXP_R + 2 * 15:04x}]
mov r7, #0.0
.for k in range(len(IDXP_F) + 1)
    ld r4, [{IDXP_P + 2 * k + 1:04x}]
    addxy r4, r4, #{2.0 * (IDXP_F + [IDXP_FREF])[k] / 96000!r}
    ph_wrap r4, r3, r4
    st r4, [{IDXP_P + 2 * k:04x}]
    .if k < len(IDXP_F)
        ld r5, [{IDXP_R + 2 * k + 1:04x}]
        mul r4, r4, r5
    .endif
    mul r4, r4, #0.03
    addxy r7, r7, r4
.endfor
st r7, [a1f0]


.fragment idxp2_tail
; IDX_PROBE=2: the wavetable voices' pattern, 8 times: hset h6 from a computed register, ldb5 Y 0 and Y 1 (indices
; IDXP2[k], IDXP2[k] + 1), results k: 2k and 2k + 1; then the tones
.for k in range(8)
    mov r6, #{IDXP2[k] / 16384!r}
    addxy r6, r6, #{(0x80 + 0.5) / 16384!r}
    hset h6, r6
    ldb5 r1, 0
    ldb5 r2, 1
    st r1, [{IDXP_R + 4 * k:04x}]
    st r2, [{IDXP_R + 4 * k + 2:04x}]
.endfor
mov r7, #0.0
.for k in range(len(IDXP_F) + 1)
    ld r4, [{IDXP_P + 2 * k + 1:04x}]
    addxy r4, r4, #{2.0 * (IDXP_F + [IDXP_FREF])[k] / 96000!r}
    ph_wrap r4, r3, r4
    st r4, [{IDXP_P + 2 * k:04x}]
    .if k < len(IDXP_F)
        ld r5, [{IDXP_R + 2 * k + 1:04x}]
        mul r4, r4, r5
    .endif
    mul r4, r4, #0.03
    addxy r7, r7, r4
.endfor
st r7, [a1f0]
