;; SYSTEM-S1CC master B with taps of the big delay memory (EDM_TAP=1, docs/MUTABLE_MEMORY.md §3.3). Template syntax:
;; make_s1cc.py frag(). Assembled one line per instruction, not scheduled (tapreq / f6 / hset are not in the scheduler):
;; every result waits one instruction (the spacers) before it is read.
;;
;; The OS's delay (program 0x0, left in place: replacing it hung the device, 2026-11-03) writes the line from the send
;; 2000 / 2002; this main reads it with its own taps: hset h4 = 256 (the line's base, as the OS), hset h5 = where the
;; results go (x0, x1, fraction at h5, h5 + 1, h5 + 2, read one sample later), f6 rD, -rS (address += base - rS),
;; tapreq rD, 0. Two forms of the address, one per side, to find which the hardware uses:
;;   left:  the age minus a position counter p += [E013], wrapped at the OS line's 35.15625 units (as the OS does)
;;   right: the age alone (relative, as on the SYSTEM-8)
;; Probes 2026-11-03: (1) age 0.5 s, results 4040 / 4043: no echo on either side; (2) the OS's result word 4005 read
;; here gives its 10 ms echo (bank-2 loads from this main work), our request (counter, 0.5 s, results 402E): none;
;; (3) this: both requests at the OS's own time A101 (10 ms with DELAY TIME at 0)

.fragment master_edm EDM_RES EDM_POS EDM_AGE EDM_LEN EDM_GAIN EDM_REQ EDM_WR
ld r0, [{OUT_L:04x}]
ld r1, [{OUT_R:04x}]
ld r7, [c{M_VOLUME:03x}]
mov r4, r4
mul r0, r0, r7
mul r1, r1, r7
mov r4, r4
st r0, [{FX_SEND[0]:04x}]
st r1, [{FX_SEND[1]:04x}]
.if EDM_WR
;; EDM_CALL=idx: the left send also into our own line (write word EDM_WR, mapped by model.c edm_ecf)
st r0, [{EDM_WR:04x}]
.endif
ld r2, [{FX_RET[0]:04x}]
ld r3, [{FX_RET[1]:04x}]
;; last sample's left tap: x0 + (x1 - x0) f
ld r4, [{EDM_RES[0]:04x}]
ld r5, [{EDM_RES[0] + 1:04x}]
ld r6, [{EDM_RES[0] + 2:04x}]
mov r7, r7
subxy r5, r4, r5
mov r7, r7
mul r5, r5, r6
mov r7, r7
addxy r4, r4, r5
mov r7, r7
mul r4, r4, #{EDM_GAIN}
mov r7, r7
addxy r2, r2, r4
;; last sample's right tap
ld r4, [{EDM_RES[1]:04x}]
ld r5, [{EDM_RES[1] + 1:04x}]
ld r6, [{EDM_RES[1] + 2:04x}]
mov r7, r7
subxy r5, r4, r5
mov r7, r7
mul r5, r5, r6
mov r7, r7
addxy r4, r4, r5
mov r7, r7
mul r4, r4, #{EDM_GAIN}
mov r7, r7
addxy r3, r3, r4
st r2, [{M_OUT[0]:04x}]
st r3, [{M_OUT[1]:04x}]
;; EDM_REQ 0 (EDM_CALL): the requests are made by the OS effect at 0x0 (edm_call.s); here only the reads above
.if EDM_REQ
;; the position counter p (bank 0: stored at a, read at a + 1 in the next sample)
ld r6, [e013]
ld r7, [{EDM_POS + 1:04x}]
mov r5, r5
addxy r6, r6, r7
mov r5, #{EDM_LEN}
mov r7, r7
subxy r5, r5, r6
mov r7, r7
sign r5, r5
mov r7, r7
addxy r5, r5, #1.0
mov r7, r7
mul r5, r5, #{EDM_LEN / 2}
mov r7, r7
subxy r6, r5, r6
mov r7, r7
st r6, [{EDM_POS:04x}]
;; this sample's requests, both at the OS delay's own time (bank 5 A101, the shared A1xx page)
ld r3, [a101]
hset h4, #256
hset h5, #{(EDM_RES[0] - 0x4000) / 16384}
mov r4, r3
mov r7, r7
f6 r4, -r6
mov r7, r7
tapreq r4, 0
mov r7, r7
mov r5, #0.0
hset h4, #256
hset h5, #{(EDM_RES[1] - 0x4000) / 16384}
mov r4, r3
mov r7, r7
f6 r4, -r5
mov r7, r7
tapreq r4, 0
mov r7, r7
.endif
mov r2, #0.0
.for x in M_ZERO
    st r2, [{x:04x}]
.endfor


.fragment edm_entry EDM_RES EDM_POS EDM_AGE EDM_LEN EDM_GAIN EDM_REQ EDM_STEP
.if EDM_STEP == 'head'
;; control (2026-11-03): only the 4-instruction head, as the stock PLUG-OUTs' 30-byte 0x0; the rest of the memory
;; stays the OS's effect program, which runs on from there
ld r0, [a000]
cmp r0, r0, #0
ld r0, [e021]
cond !12, 1, 0 | st r0, [a000]@0 | st r0, [a000]@1
.else
;; RECORD ONLY: make_s1cc.py refuses EDM_ENTRY. A PLUG-OUT must never queue 0x0000 (system-1/WAVE_BUS.md §5.3); these
;; probes did and hung / muted the device (docs/MUTABLE_MEMORY.md §3.3).
;; master DSP B program at 0x0 in place of the OS's delay + reverb (EDM_ENTRY=1). tapreq is served only here: from
;; the main at 0x1000 the requests gave nothing, while bank-2 loads worked (probes 2026-11-03, master_edm above).
;; 0x0 is master B's entry: first the calls every SYSTEM-1 master B entry makes (E021 -> A000; call the main
;; 0x1000 = word 0x0800 under condition 25 and the section ends 0x5FF2 / 0x6FF2 under 26 / 27; system-1/FX_CHAIN.md
;; §3); a program at 0x0 without them hung the device (2026-11-03). Then our code between the start / end markers,
;; then the jump to the close at 0x7FF2 (word 0x3FF9).
;; EDM_STEP 'pass': only the send into the line, back unchanged (probe A, after probe 4 shut the device down when
;; selected, 2026-11-03); 'taps': body (probe 4): the send 2000 / 2002 (the main's mix, this sample) into the line (4001); taps of EDM_AGE, left with
;; the position counter p (as master_edm), right with the age alone; out = send + gain x tap -> 2000 / 2002
ld r0, [a000]
cmp r0, r0, #0
ld r0, [e021]
cond !12, 1, 0 | st r0, [a000]@0 | st r0, [a000]@1
.slot @0 0800
cond 25, 10, 0
.class 0x0000
raw
.slot @0 2ff9
cond 26, 10, 0
.class 0x0000
raw
.slot @0 37f9
cond 27, 10, 0
.class 0x0000
raw
f7 r7, r1
.if EDM_STEP == 'pass'
;; probe A: no tapreq: the send into the line (4001) and back unchanged
ld r0, [2000]
ld r1, [2002]
mov r7, r7
st r0, [4001]
st r0, [2000]
st r1, [2002]
.else
ld r0, [2000]
ld r1, [2002]
mov r7, r7
st r0, [4001]
;; last sample's left tap: x0 + (x1 - x0) f
ld r4, [{EDM_RES[0]:04x}]
ld r5, [{EDM_RES[0] + 1:04x}]
ld r6, [{EDM_RES[0] + 2:04x}]
mov r7, r7
subxy r5, r4, r5
mov r7, r7
mul r5, r5, r6
mov r7, r7
addxy r4, r4, r5
mov r7, r7
mul r4, r4, #{EDM_GAIN}
mov r7, r7
addxy r0, r0, r4
;; last sample's right tap
ld r4, [{EDM_RES[1]:04x}]
ld r5, [{EDM_RES[1] + 1:04x}]
ld r6, [{EDM_RES[1] + 2:04x}]
mov r7, r7
subxy r5, r4, r5
mov r7, r7
mul r5, r5, r6
mov r7, r7
addxy r4, r4, r5
mov r7, r7
mul r4, r4, #{EDM_GAIN}
mov r7, r7
addxy r1, r1, r4
st r0, [2000]
st r1, [2002]
;; the position counter p (bank 0: stored at a, read at a + 1 in the next sample)
ld r6, [e013]
ld r7, [{EDM_POS + 1:04x}]
mov r5, r5
addxy r6, r6, r7
mov r5, #{EDM_LEN}
mov r7, r7
subxy r5, r5, r6
mov r7, r7
sign r5, r5
mov r7, r7
addxy r5, r5, #1.0
mov r7, r7
mul r5, r5, #{EDM_LEN / 2}
mov r7, r7
subxy r6, r5, r6
mov r7, r7
st r6, [{EDM_POS:04x}]
;; this sample's requests
hset h4, #256
hset h5, #{(EDM_RES[0] - 0x4000) / 16384}
mov r4, #{EDM_AGE}
mov r7, r7
f6 r4, -r6
mov r7, r7
tapreq r4, 0
mov r7, r7
mov r5, #0.0
hset h4, #256
hset h5, #{(EDM_RES[1] - 0x4000) / 16384}
mov r4, #{EDM_AGE}
mov r7, r7
f6 r4, -r5
mov r7, r7
tapreq r4, 0
mov r7, r7
.endif
f7 r7, r0
.slot @0 3ff9
cond 0, 8, 0
.class 0x0000
raw
.endif

.fragment master_direct
;; master B main for EDM_ENTRY probe B: the mix straight to the output (A118 / A119), and the send 2000 / 2002 as
;; always; the effect return is not used. Sound = this main runs under our entry at 0x0 (2026-11-03: with the
;; return path, probe A was silent)
ld r0, [{OUT_L:04x}]
ld r1, [{OUT_R:04x}]
ld r7, [c{M_VOLUME:03x}]
mov r4, r4
mul r0, r0, r7
mul r1, r1, r7
mov r4, r4
st r0, [{FX_SEND[0]:04x}]
st r1, [{FX_SEND[1]:04x}]
st r0, [{M_OUT[0]:04x}]
st r1, [{M_OUT[1]:04x}]
mov r2, #0.0
.for x in M_ZERO
    st r2, [{x:04x}]
.endfor
