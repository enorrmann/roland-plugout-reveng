;; SYSTEM-S1CC host microcode: EDM_WT, a wavetable stored whole in the delay memory of master B
;; (docs/vol6_reference_designs_applications/mutable_memory.md §3.3.8). Template syntax: make_s1cc.py frag()
;;
;; One own line at WT_A: write word WT_W, read word WT_R at age WT_LEN - 1 (map table, model.c wt_load), so the line
;; is a loop of WT_LEN samples that keeps what it holds (each sample re-writes what it read). Region 0 fills it at the
;; first notes: it follows the delay memory's sample counter (controller word 0x40100048) and puts each table sample
;; into parameter C0{WT_PSTG:02x} (FCEC); a changed one is written instead of the re-circulated sample. C0{WT_PSYNC:02x} changed:
;; the loop counter c restarts at 0 (region 0 numbers the table samples from that moment: sample k is where c = k).
;; WT_NV voices, each two requests (frame f and f + 1, mixed by COLOR) through the request block in the OS effect.
;; Units: samples / 16384 (the tapreq age unit). Written for the master's main as it runs (one line per
;; instruction): a loaded value is used two instructions later, an op's result one instruction later (fillers: a
;; register moved onto itself). subxy rD, rX, Y = Y - rX. Only op forms that ran on master B (asm/master_b_forms.txt).
;; Bank 0 states (stored at a, read at a + 1 the next sample): WT_S c, + 2 the last C0{WT_PSTG:02x}, + 4 the last
;; C0{WT_PSYNC:02x}; voice v at WT_S + 8 + 4v: the phase p, + 2 the gain g; WT_AGEB + 4 (2v + j) (+ 1): the ages.
;; r0 = c and r7 = the voices' sum for the whole program

.fragment master_wt
;; c = c + 1, 0 at WT_LEN, 0 when the sync parameter changed
ld r0, [{WT_S + 1:04x}]
ld r1, [c{WT_PSYNC:03x}]
ld r2, [{WT_S + 5:04x}]
addxy r0, r0, #{2.0 ** -14!r}
;; the limit WT_LEN - 1/2 sample from parameter C0{WT_PLIM:02x} (region 0 writes it, exact float): immediates near
;; it ran wrong on master B (11.49609 = 4b7f as 5.748, WT-AGE2; with 11.5 = 4b80 a jump every 65 536 samples,
;; 2026-11-05), the emulator reads them right
ld r3, [c{WT_PLIM:03x}]
mov r7, r7
st r1, [{WT_S + 4:04x}]
subxy r2, r1, r2
subxy r3, r0, r3
mov r7, r7
sign r2, r2
sign r3, r3
mul r2, r2, r2
addxy r3, r3, #1.0
mov r7, r7
mul r3, r3, #0.5
mov r7, r7
mul r0, r0, r3
mov r3, #1.0
mov r7, r7
subxy r3, r2, r3
mov r7, r7
mul r0, r0, r3
mov r7, r7
st r0, [{WT_S:04x}]
;; the loop: w = x + m (s - x), m = 1 when the sample parameter changed since the last sample
ld r1, [c{WT_PSTG:03x}]
ld r2, [{WT_S + 3:04x}]
ld r3, [{WT_R:04x}]
mov r7, r7
st r1, [{WT_S + 2:04x}]
subxy r2, r1, r2
subxy r1, r3, r1
mov r7, r7
sign r2, r2
mov r7, r7
mul r2, r2, r2
mov r7, r7
mul r1, r1, r2
mov r7, r7
addxy r1, r1, r3
mov r7, r7
st r1, [{WT_W:04x}]
mov r7, #0.0
.for v in range(WT_NV)
    ;; voice {v}: phase p += dt (C0{WT_PDT0 + v:02x}), less one frame (WT_F) when it reaches it
    ld r4, [{WT_S + 8 + 4 * v + 1:04x}]
    ld r5, [c{WT_PDT0 + v:03x}]
    mov r3, #{WT_F / 16384!r}
    mov r6, r6
    addxy r4, r4, r5
    mov r6, r6
    subxy r3, r3, r4
    mov r6, r6
    sign r3, r3
    mov r6, r6
    addxy r3, r3, #1.0
    mov r6, r6
    mul r3, r3, #{WT_F / 32768!r}
    mov r6, r6
    subxy r4, r3, r4
    ld r5, [c{WT_PFRAME:03x}]
    st r4, [{WT_S + 8 + 4 * v:04x}]
    mov r6, r6
    addxy r4, r4, r5
    ;; its two requests' ages: a = c - (f + p) - WT_LAT, + WT_LEN when negative -> bank 0 (the request block reads + 1:
    ;; this sample's age if it runs after the main, the last sample's if before)
    .for j in (0, 1)
        .if j
            mov r6, r6
            addxy r4, r4, #{WT_F / 16384!r}
        .endif
        mov r6, r6
        subxy r6, r4, r0
        mov r5, r5
        addxy r6, r6, #{-(WT_LAT / 16384)!r}
        mov r5, r5
        mov r3, r6
        mov r5, r5
        sign r3, r3
        mov r5, r5
        mul r3, r3, #-1.0
        mov r5, r5
        addxy r3, r3, #1.0
        mov r5, r5
        mul r3, r3, #{WT_LEN / 32768!r}
        mov r5, r5
        addxy r6, r6, r3
        mov r5, r5
        st r6, [{WT_AGEB + 4 * (2 * v + j):04x}]
        st r6, [{WT_AGEB + 4 * (2 * v + j) + 1:04x}]
    .endfor
    ;; the last sample's two results, interpolated as master_edm: y_j = x0 + (x1 - x0) fr -> r1, r2
    .for j in (0, 1)
        ld r4, [{WT_RES + 8 * (2 * v + j):04x}]
        ld r5, [{WT_RES + 8 * (2 * v + j) + 1:04x}]
        ld r6, [{WT_RES + 8 * (2 * v + j) + 2:04x}]
        mov r3, r3
        subxy r5, r4, r5
        mov r3, r3
        mul r5, r5, r6
        mov r3, r3
        addxy r{1 + j}, r4, r5
    .endfor
    ;; y = y0 + m (y1 - y0) (m = C0{WT_PMIX:02x}, COLOR between the two frames); gain g -> its target C0{WT_PGAIN0 + v:02x} (the
    ;; gate) by a one-pole of WT_GK per sample (no clicks); sum += y g
    ld r5, [c{WT_PMIX:03x}]
    ld r4, [{WT_S + 8 + 4 * v + 3:04x}]
    subxy r2, r1, r2
    ld r6, [c{WT_PGAIN0 + v:03x}]
    mul r2, r2, r5
    mov r3, r3
    addxy r1, r1, r2
    subxy r6, r4, r6
    mov r3, r3
    mul r6, r6, #{WT_GK!r}
    mov r3, r3
    addxy r4, r4, r6
    mov r3, r3
    st r4, [{WT_S + 8 + 4 * v + 2:04x}]
    mul r1, r1, r4
    mov r3, r3
    addxy r7, r7, r1
.endfor
;; x VOLUME -> both sends (WT_DIAG=1: left = the loop itself, x 0.5: the whole table at a fixed speed)
ld r5, [c{M_VOLUME:03x}]
.if WT_DIAG == 1
ld r2, [{WT_R:04x}]
.else
mov r3, r3
.endif
mul r7, r7, r5
.if WT_DIAG == 1
mul r2, r2, #0.5
mov r3, r3
mul r2, r2, r5
mov r3, r3
st r2, [{FX_SEND[0]:04x}]
.else
mov r3, r3
st r7, [{FX_SEND[0]:04x}]
.endif
st r7, [{FX_SEND[1]:04x}]
ld r2, [{FX_RET[0]:04x}]
ld r3, [{FX_RET[1]:04x}]
mov r4, r4
st r2, [{M_OUT[0]:04x}]
st r3, [{M_OUT[1]:04x}]
mov r2, #0.0
.for x in M_ZERO
    st r2, [{x:04x}]
.endfor


.fragment master_wt_s
;; WT_SLAVE (docs/vol6_reference_designs_applications/mutable_memory.md §4.4): the loop as master_wt; each voice ONE request (frame f = COLOR's nearest,
;; WT_PFRAME) -> its sample to the slave by 600E + v (WT-M2S: arrives with our master A main, MA_MAIN); then the
;; slave's mix 6006 / 6007 x VOLUME -> the OS effect sends, as microcode/master.s master. Ages at WT_AGEB + 4v,
;; results WT_RES + 8v.
;; c = c + 1, 0 at WT_LEN, 0 when the sync parameter changed
ld r0, [{WT_S + 1:04x}]
ld r1, [c{WT_PSYNC:03x}]
ld r2, [{WT_S + 5:04x}]
addxy r0, r0, #{2.0 ** -14!r}
;; the limit WT_LEN - 1/2 sample from parameter C0{WT_PLIM:02x} (region 0 writes it, exact float): immediates near
;; it ran wrong on master B (11.49609 = 4b7f as 5.748, WT-AGE2; with 11.5 = 4b80 a jump every 65 536 samples,
;; 2026-11-05), the emulator reads them right
ld r3, [c{WT_PLIM:03x}]
mov r7, r7
st r1, [{WT_S + 4:04x}]
subxy r2, r1, r2
subxy r3, r0, r3
mov r7, r7
sign r2, r2
sign r3, r3
mul r2, r2, r2
addxy r3, r3, #1.0
mov r7, r7
mul r3, r3, #0.5
mov r7, r7
mul r0, r0, r3
mov r3, #1.0
mov r7, r7
subxy r3, r2, r3
mov r7, r7
mul r0, r0, r3
mov r7, r7
st r0, [{WT_S:04x}]
.if WT_FILLIDX
;; the loop, index-gated fill (WT_FILLIDX): slot j = (index ij, value vj) from the ARM; m_j = 1 - sign(ij - c)^2
;; (1 only when c is the slot's index); w = x + m0 (v0 - x) + m1 (v1 - x) (the ARM never puts one index in both)
ld r1, [c{WT_PI0:03x}]
ld r2, [c{WT_PI1:03x}]
ld r3, [{WT_R:04x}]
mov r7, r7
subxy r1, r0, r1
subxy r2, r0, r2
mov r7, r7
sign r1, r1
sign r2, r2
mov r7, r7
mul r1, r1, r1
mul r2, r2, r2
ld r4, [c{WT_PV0:03x}]
ld r5, [c{WT_PV1:03x}]
mul r1, r1, #-1.0
mul r2, r2, #-1.0
subxy r4, r3, r4
subxy r5, r3, r5
addxy r1, r1, #1.0
addxy r2, r2, #1.0
mov r7, r7
mul r4, r4, r1
mul r5, r5, r2
mov r7, r7
addxy r3, r3, r4
mov r7, r7
addxy r3, r3, r5
mov r7, r7
st r3, [{WT_W:04x}]
.if WT_LINES
;; WT_LINES: lines 1 .. WT_LSUB - 1 = the table mixed j / WT_LSUB toward the next frame (same phase), rebuilt every sample
;; from the loop: x = w (index c), xn = [{WT_RN:04x}] (index c + WT_F, a fixed map read); line j at WT_A + wt_loff(j)
ld r4, [{WT_RN:04x}]
mov r7, r7
subxy r4, r3, r4
mov r7, r7
mul r4, r4, #{1.0 / WT_LSUB!r}
mov r7, r7
addxy r5, r3, r4
mov r7, r7
st r5, [{WT_LW[0]:04x}]
.for i in range(1, WT_LSUB - 1)
addxy r5, r5, r4
mov r7, r7
st r5, [{WT_LW[i]:04x}]
.endfor
.endif
.else
;; the loop: w = x + m (s - x), m = 1 when the sample parameter changed since the last sample
ld r1, [c{WT_PSTG:03x}]
ld r2, [{WT_S + 3:04x}]
ld r3, [{WT_R:04x}]
mov r7, r7
st r1, [{WT_S + 2:04x}]
subxy r2, r1, r2
subxy r1, r3, r1
mov r7, r7
sign r2, r2
mov r7, r7
mul r2, r2, r2
mov r7, r7
mul r1, r1, r2
mov r7, r7
addxy r1, r1, r3
mov r7, r7
st r1, [{WT_W:04x}]
.endif
;; r2 = WT_LEN / 32768 (parameter C0{WT_PHALF:02x}: the age's wrap below adds 2 r2 = WT_LEN; not an immediate)
ld r2, [c{WT_PHALF:03x}]
.for v in range(WT_NV)
    ;; voice {v}: phase p += dt (C0{WT_PDT0 + v:02x}), less one frame (WT_F) when it reaches it
    ; @voice {v}
;; WT_IAGE_TEST (2026-11-10, the read-noise test): the phase in whole samples p (units, C0{WT_PDT0 + v:02x} = the
;; step's whole part) and its fraction q in [0, 1) (bank 0 WT_Q + 4v, C0{WT_PDTF0 + v:02x} = the step's fraction), so the
;; request's age can be whole (as MP-PLSTRING's, which reads clean) and the interpolation can use our own fraction
;; instead of the tapreq's (1/64, truncated). The selector (OSC2 COLOR, C0{WT_PSEL0:02x}..: one-hot w_hw, w_1, w_2)
;; picks: the tapreq's fraction with the fractional age (as before), or 1 - q of one or two samples before with the
;; whole age. carry / wrap at >=: 1 + s/2 - s^2/2 (s = sign; 1 at 0, as sign(0) = 0 must)
.if WT_IAGE_TEST
    ld r4, [{WT_S + 8 + 4 * v + 1:04x}]
    ld r5, [c{WT_PDT0 + v:03x}]
    ld r1, [{WT_Q + 4 * v + 1:04x}]
    ld r6, [c{WT_PDTF0 + v:03x}]
    mov r7, r7
    addxy r4, r4, r5
    st r1, [{WT_Q + 4 * v + 2:04x}]
    addxy r1, r1, r6
    mov r3, #-1.0
    mov r7, r7
    addxy r3, r3, r1
    mov r7, r7
    sign r3, r3
    mov r7, r7
    mov r5, r3
    mov r7, r7
    mul r5, r5, r5
    mov r7, r7
    subxy r3, r5, r3
    mov r7, r7
    mul r3, r3, #0.5
    mov r7, r7
    addxy r3, r3, #1.0
    mov r7, r7
    subxy r1, r3, r1
    mul r3, r3, #{2.0 ** -14!r}
    mov r7, r7
    st r1, [{WT_Q + 4 * v:04x}]
    addxy r4, r4, r3
    mov r3, #{WT_F / 16384!r}
    mov r7, r7
    subxy r3, r3, r4
    mov r7, r7
    sign r3, r3
    mov r7, r7
    mov r5, r3
    mov r7, r7
    mul r5, r5, r5
    mov r7, r7
    subxy r3, r5, r3
    mov r7, r7
    mul r3, r3, #0.5
    mov r7, r7
    addxy r3, r3, #1.0
    mov r7, r7
    mul r3, r3, #{WT_F / 16384!r}
    mov r7, r7
    subxy r4, r3, r4
.else
    ld r4, [{WT_S + 8 + 4 * v + 1:04x}]
    ld r5, [c{WT_PDT0 + v:03x}]
    mov r3, #{WT_F / 16384!r}
    mov r6, r6
    addxy r4, r4, r5
    mov r6, r6
    subxy r3, r3, r4
    mov r6, r6
    sign r3, r3
    ;; (mul only as `mul rX, rX, rX`: `mul r5, r3, r3` ran wrong on master B, 2026-11-05)
    ;; sign(0) = 0: with "+ 1.0" an exact hit (p + dt = WT_F) took HALF a frame off -> the voice stayed half a frame
    ;; off (a click, measured 2026-11-05 wt_long_235143: note 52, shift 1022 of 2048). s + s^2: 2 / 0 / 0
    mov r6, r6
    mov r5, r3
    mov r6, r6
    mul r5, r5, r5
    mov r6, r6
    addxy r3, r3, r5
    mov r6, r6
    mul r3, r3, #{WT_F / 32768!r}
    mov r6, r6
    subxy r4, r3, r4
.endif
.if WT_CMOD
    ld r5, [{wt_s2m(v):04x}]
.else
    ld r5, [c{WT_PFRAME:03x}]
.endif
.if WT_FLATCH
    ;; WT_FLATCH: the frame position is taken only when the phase wrapped (a new cycle starts: no splice, no jitter
    ;; inside a cycle) and approaches the new one by WT_FLK per cycle: fl += w (f - fl), w = WT_FLK at a wrap, else 0
    ;; (r3 = WT_F / 16384 = 0.125 at a wrap, else 0). The COLOR's jitter x 30 frames x 2048 made WT_CQ=0 pure noise
    ;; (WT-CONT, 2026-11-06)
    ld r6, [{WT_FL + 4 * v + 1:04x}]
    mul r3, r3, #{8.0 * WT_FLK!r}
    st r4, [{WT_S + 8 + 4 * v:04x}]
    subxy r5, r6, r5
    mov r7, r7
    mul r5, r5, r3
    mov r7, r7
    addxy r6, r6, r5
    mov r7, r7
    st r6, [{WT_FL + 4 * v:04x}]
    addxy r4, r4, r6
.else
    st r4, [{WT_S + 8 + 4 * v:04x}]
    mov r6, r6
    addxy r4, r4, r5
.endif
    ;; its request's age: a = c - (f + p) - WT_LAT - 256, + WT_LEN when negative, + 256 -> bank 0: ages 256 ..
    ;; WT_LEN + 256 (the copy one lap older), never near 0: a request at a few samples' age ticked once per lap
    ;; (WT-AGE3, 2026-11-05)
.if WT_AGEFIX
    ;; WT_AGEFIX probe: fixed ages (voice 0: WT_AGEFIX[0], voice 1: WT_AGEFIX[1], others the first)
    mov r6, #{(WT_AGEFIX[1] if v == 1 else WT_AGEFIX[0]) / 16384!r}
    mov r5, r5
.else
    mov r6, r6
    subxy r6, r4, r0
    mov r5, r5
    addxy r6, r6, #{-(WT_LAT / 16384)!r}
    mov r5, r5
    addxy r6, r6, #-0.015625
    mov r5, r5
    mov r3, r6
    mov r5, r5
    sign r3, r3
    ;; s^2 - s (2 / 0 / 0): with "1 - s" an age of exactly 0 added half the table (sign(0) = 0)
    mov r7, r7
    mov r5, r3
    mov r7, r7
    mul r5, r5, r5
    mul r3, r3, #-1.0
    mov r7, r7
    addxy r3, r3, r5
    mov r7, r7
    mul r3, r3, r2
    mov r5, r5
    addxy r6, r6, r3
    mov r5, r5
    addxy r6, r6, #0.015625
    mov r5, r5
.endif
.if WT_IAGE_TEST
    ;; the age requested: A - 2^-14 (1 + w_hw (q - 1)): w_hw = 1 the fractional age of before (A - q), else the whole
    ;; age A - 1 sample (its pair: ages A, A + 1 -> y = x0 + (1 - q) (x1 - x0))
    ld r3, [c{WT_PSEL0:03x}]
    addxy r5, r1, #-1.0
    mov r7, r7
    mul r5, r5, r3
    mov r7, r7
    addxy r5, r5, #1.0
    mov r7, r7
    mul r5, r5, #{-(2.0 ** -14)!r}
    mov r7, r7
    addxy r6, r6, r5
    mov r7, r7
.endif
    st r6, [{WT_AGEB + 4 * v:04x}]
    st r6, [{WT_AGEB + 4 * v + 1:04x}]
    ;; the last sample's result, interpolated: y = x0 + (x1 - x0) fr -> the slave
.if WT_IAGE_TEST
    ;; g = w_hw fr + w_1 (1 - q[n-1]) + w_2 (1 - q[n-2]): the pair read now was asked one or two samples ago
    ld r4, [{WT_RES + 8 * v:04x}]
    ld r5, [{WT_RES + 8 * v + 1:04x}]
    ld r6, [{WT_RES + 8 * v + 2:04x}]
    ld r3, [c{WT_PSEL0:03x}]
    mov r7, r7
    subxy r5, r4, r5
    mul r6, r6, r3
    ld r1, [{WT_Q + 4 * v + 1:04x}]
    ld r3, [c{WT_PSEL0 + 1:03x}]
    mov r7, r7
    mul r1, r1, r3
    addxy r6, r6, r3
    mov r7, r7
    subxy r6, r1, r6
    ld r1, [{WT_Q + 4 * v + 3:04x}]
    ld r3, [c{WT_PSEL0 + 2:03x}]
    mov r7, r7
    mul r1, r1, r3
    addxy r6, r6, r3
    mov r7, r7
    subxy r6, r1, r6
    mov r7, r7
    mul r5, r5, r6
    mov r7, r7
    addxy r1, r4, r5
    mov r3, r3
.else
    ld r4, [{WT_RES + 8 * v:04x}]
    ld r5, [{WT_RES + 8 * v + 1:04x}]
    ld r6, [{WT_RES + 8 * v + 2:04x}]
    mov r3, r3
    subxy r5, r4, r5
    mov r3, r3
    mul r5, r5, r6
    mov r3, r3
    addxy r1, r4, r5
    mov r3, r3
.endif
.if WT_VTAG
    ;; WT_VTAG probe: voice {v} at (v + 1) / 8 of its level (which voice a note got)
    mul r1, r1, #{(v + 1) / 8!r}
    mov r3, r3
.endif
    st r1, [{WT_M2S + v:04x}]
.endfor
; @voices end
;; the slave's mix x VOLUME -> the OS effect sends; the effect's return -> the OS output program (A118 / A119)
.if WT_AGEFIX
;; WT_AGEFIX: left = voice 0's read, right = voice 1's (what was sent to 600E / 600F), not the slave
ld r0, [{WT_M2S:04x}]
ld r1, [{WT_M2S + 1:04x}]
.elif WT_DIAG == 1
;; WT_DIAG=1 (WT_SLAVE): left = the loop itself (the table at a fixed speed, one sample per sample: compare with the
;; WAV), right = voice 0's read as sent to the slave (600E, as WT_AGEFIX); the slave's mix is not heard
ld r0, [{WT_R:04x}]
ld r1, [{WT_M2S:04x}]
.elif WT_DIAGA
;; WT_DIAGA: left = voice 0's read, right = the age voice 0 requested / 16 (units: 0 .. 0.72)
ld r0, [{WT_M2S:04x}]
ld r1, [{WT_AGEB + 1:04x}]
mov r4, r4
mov r4, r4
mul r1, r1, #0.0625
.else
ld r0, [{OUT_L:04x}]
ld r1, [{OUT_R:04x}]
.endif
ld r7, [c{M_VOLUME:03x}]
mov r4, r4
mul r0, r0, r7
mul r1, r1, r7
st r0, [{FX_SEND[0]:04x}]
st r1, [{FX_SEND[1]:04x}]
ld r2, [{FX_RET[0]:04x}]
ld r3, [{FX_RET[1]:04x}]
mov r4, r4
st r2, [{M_OUT[0]:04x}]
st r3, [{M_OUT[1]:04x}]
mov r2, #0.0
.for x in M_ZERO
    st r2, [{x:04x}]
.endfor
