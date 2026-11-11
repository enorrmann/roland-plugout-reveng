;; SYSTEM-S1CC host microcode: COLOR MOD on DSP B. Template syntax: make_s1cc.py frag()
;; (MOD_DSP=A: the colors are part of host_a, host_a.s)

.fragment colors_b v hb e acc
;; COLOR MOD of voice v on DSP B, after its envelopes: color i = weights (C054 + 6 i ..) x (1, lf, pitch env,
;; filter env, amp env, sub) -> bank 5 COLB (s1_color(i) on DSP A, one sample late); the sub square from DSP A.
;; Both colors in one pass (acc: color -> register, r0 / r2): each source is loaded once for the two (the loads,
;; not the multiplies, set its length: two bank-6 loads and one bank-5 access per instruction). e: the voice's
;; block (amp envelope at e, filter envelope at e + 6)
; voice {v}: colors {", ".join("%d (%s)" % (i, acc[i]) for i in COLOR_USED)} = w0 + w1 lf + w2 pe + w3 fe + w4 ae + w5 sub
.for i in COLOR_USED
    ld {acc[i]}, [c0{B6_COL[i]:02x}]
.endfor
.for w, src in ((1, LFB[v]), (2, hb["pe"]), (3, e + 6), (4, e), (5, SUBB[v]))
    ld r1, [{src:04x}]
    .for i in COLOR_USED
        ld r3, [c0{B6_COL[i] + w:02x}]
        mov r3, r3
        mad {acc[i]}, r1, r3
    .endfor
.endfor
.for i in COLOR_USED
    st {acc[i]}, [{COLB[i][v]:04x}]
.endfor


.fragment colors_idx
;; COLOR MOD of every voice by indexed loads (CMOD_IDX), after all the voices: per color i h6 = the chosen source's
;; table row (parameter C054 + 6 i + 2, written by the ARM), w0 and w (C054 + 6 i, + 1); per voice
;; color = w0 + w src[h6 + 2 v] (ldidx + ymacd + st). The sources: the envelopes and the pitch envelope keep their
;; level in the table (cmt), the LFO is stored there too, the sub is copied (sub_to_cmt)
; COLOR MOD by indexed loads: per color h6 = the row of the chosen source, each voice w0 + w src[h6 + 2 v]
.for i in COLOR_USED
    ld r0, [c0{B6_COL[i] + 2:02x}]
    mov r0, r0
    hset h6, r0
    ld r1, [c0{B6_COL[i]:02x}]
    ld r2, [c0{B6_COL[i] + 1:02x}]
    mov r2, r2
    .for v in range(NV)
        ldidx r{3 + v % 4}, {2 * v}
        ymacd r{3 + v % 4}, r2, r1
        st r{3 + v % 4}, [{COLB[i][v]:04x}]
    .endfor
.endfor


.fragment sub_to_cmt v
;; CMOD_IDX: the sub square into the COLOR MOD table
ld r1, [{SUBB[v]:04x}]
st r1, [{cmt(5, v):04x}]
