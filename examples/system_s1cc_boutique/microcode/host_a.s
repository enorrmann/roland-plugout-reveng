;; SYSTEM-S1CC host microcode: slave DSP A around the kernel. Template syntax: make_s1cc.py frag()

.fragment head_a
; s1m0: slave DSP A main @0x0100: {NV} voices x s1cc kernel {KERNEL} DSP A -> A151+v{" (voice pairs interleaved, PAIRS=1)" if PAIRS else ""}


.fragment host_a v h
;; the host's DSP A part of voice v, before the kernel; four registers r0..r3 = a, b, c, d (pairs: r4..r7 for the
;; odd voice), longer lived values in same-sample words of the voice block (h: name -> word):
;; * gate and trigger copies for DSP B (its envelopes, LFO and pitch envelope), from their loads
;; * MOD_DSP=A: the pitch envelope here (a = its level)
;; * -> pitch: y = P lf + E pe + R octaves (LFO TO PITCH, PITCH ENV AMOUNT, OSC1 RANGE 64'..2' = -3..+2; lf and pe
;;   from DSP B, one sample late); pitch + y,
;;   dt 2^y = (e^(y ln2 / 8))^8 (Taylor to the 4th power, three squarings: ~2 cents at 6 octaves), rdt 2^-y (to the
;;   3rd: it only sets a polyBLEP's width) -> the words the kernel reads as s1_pitch / s1_increment,
;;   s1_dt, s1_rdt; DSP B's pitch straight to its word (HOSTB)
;; * sub oscillator (square one octave down) -> bank 5 SUBB+v: COLOR MOD source (DSP B), SUB of the mixer
;; * MOD_DSP=A: the colors here, the envelopes from DSP B (ENVX); else they come from DSP B (colors_b), the kernel
;;   reads them in bank 5 (COLB)
; voice {v}: host (DSP A): gate / trigger copies for DSP B
.if PENV and not MOD_B
    ld r0, [{TRIG[v]:04x}]
    ld r2, [{GATE[v]:04x}]
    ld r1, [{h["pt"] + 1:04x}]
    mov r0, r0
    .if GT_B
        st r0, [{TB[v]:04x}]
        st r2, [{GB[v]:04x}]
    .endif
    st r0, [{h["pt"]:04x}]
    subxy r1, r1, r0
    mul r1, r1, r1
    .include penv_core v=v, pe=h["pe"], pf=h["pf"], prm=A6_PENV
.elif GT_B
    ld r0, [{TRIG[v]:04x}]
    ld r2, [{GATE[v]:04x}]
    mov r0, r0
    st r0, [{TB[v]:04x}]
    st r2, [{GB[v]:04x}]
.endif
.if PITCH_MOD
    ; voice {v}: -> pitch b = y = P lf + E pe + OSC1 RANGE octaves
    ld r1, [{LFB[v]:04x}]
    ld r2, [c0{A6_LPITCH:02x}]
    .if MOD_B
        ld r0, [{PEB[v]:04x}]
    .endif
    mov r2, r2
    mul r1, r1, r2
    ld r2, [c0{A6_PAMT:02x}]
    mov r2, r2
    mad r1, r0, r2
    ld r2, [c0{A6_RANGE1:02x}]
    mov r2, r2
    addxy r1, r1, r2
    .if "pitch" in PITCH_MOD
        ld r2, [{PITCH[v]:04x}]
        mov r2, r2
        addxy r2, r2, r1
        ;; the kernel's word (DSP A reads it) and / or DSP B's (INB)
        .if "pitchm" in h
            st r2, [{h["pitchm"]:04x}]
        .endif
        .if "pitch" in HOSTB
            st r2, [{INB0 + 8 * KDEF.inputs_b.index("pitch") + v:04x}]
        .endif
    .endif
    ;; 2^y = (e^(y ln2 / 8))^8: Taylor of e^z on |z| <= 0.52 (y up to +-6 octaves: RANGE, PITCH ENV, LFO), then
    ;; three squarings; dt to z^4 (~2 cents at 6 octaves), rdt (a polyBLEP's width) to z^3
    .for n, src, sign in (("dt", DT, 1.0), ("rdt", RDT, -1.0))
        .if n in PITCH_MOD
            mul r2, r1, #{sign * math.log(2) / 8!r}
            .if n == "dt"
                mov r0, #0.041666666666666664
                ymacd r0, r2, #0.16666666666666666
                ymacd r0, r2, #0.5
                ymacd r0, r2, #1.0
                ymacd r0, r2, #1.0
            .else
                mov r0, #0.16666666666666666
                ymacd r0, r2, #0.5
                ymacd r0, r2, #1.0
                ymacd r0, r2, #1.0
            .endif
            mul r0, r0, r0
            mul r0, r0, r0
            mul r0, r0, r0
            ld r2, [{src[v]:04x}]
            mov r2, r2
            mul r2, r2, r0
            st r2, [{h[n + "m"]:04x}]
        .endif
    .endfor
.endif
.if "sp" in h
    ; voice {v}: sub oscillator: a square one octave down (COLOR MOD source, SUB of the mixer)
    ld r1, [{h["sp"] + 1:04x}]
    ld r2, [{DT[v]:04x}]
    mov r2, r2
    addxy r2, r1, r2
    ph_wrap r1, r3, r2
    st r1, [{h["sp"]:04x}]
    mul r1, r1, #256.0
    ph_clamp r1, r3, -r1
    st r1, [{SUBB[v]:04x}]
.endif
.if not MOD_B
    .for i in COLOR_USED
        ; voice {v}: color {i} = w0 + w1 lf + w2 pe + w3 fe + w4 ae + w5 sub (c)
        ld r2, [c0{A6_COL[i]:02x}]
        .for w, src in ((1, LFB[v]), (2, h["pe"]), (3, ENVX[1][v]), (4, ENVX[0][v]), (5, SUBB[v]))
            ld r1, [{src:04x}]
            ld r3, [c0{A6_COL[i] + w:02x}]
            mov r3, r3
            mad r2, r1, r3
        .endfor
        st r2, [{h["color%d" % i]:04x}]
    .endfor
.endif


.fragment mixer v h
;; HOST_MIX, after the kernel: kernel (its output word xo) + SUB x sub + NOISE x noise -> x of DSP B
; voice {v}: mixer
ld r0, [{h["xo"]:04x}]
ld r1, [{SUBB[v]:04x}]
ld r2, [c0{A6_SUBMIX:02x}]
mov r2, r2
mad r0, r1, r2
ld r1, [{lfo_block("A")["nz"]:04x}]
ld r2, [c0{A6_NOISEMIX:02x}]
mov r2, r2
mad r0, r1, r2
st r0, [{XV[v]:04x}]


.fragment gate_copy v
;; gate and trigger for the host's envelopes on DSP B (with the host LFO, host_a stores them from its loads)
ld r0, [{GATE[v]:04x}]
st r0, [{GB[v]:04x}]
ld r1, [{TRIG[v]:04x}]
st r1, [{TB[v]:04x}]
