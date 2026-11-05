;; SYSTEM-S1CC host microcode: EDM_WT, a wavetable stored whole in the delay memory of master B
;; (docs/MUTABLE_MEMORY.md §3.3.8). Template syntax: make_s1cc.py frag()
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
mov r3, #{WT_LEN / 16384 - 2.0 ** -15!r}
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
