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
.for v in range(str_nv())
    ;; voice {v}
    .if STR_TAP
        ;; STR_TAP: last sample's tapreq results (edm_call.s edm_str_block): x0, x1 at age d, x0, x1 at age d + 2
        ld r0, [{STR_RES + 8 * v:04x}]
        ld r1, [{STR_RES + 8 * v + 1:04x}]
        ld r2, [{STR_RES + 8 * v + 3:04x}]
        ld r3, [{STR_RES + 8 * v + 4:04x}]
    .else
        ld r0, [{STR_W + 8 * v + 1:04x}]
        ld r1, [{STR_W + 8 * v + 3:04x}]
        ld r2, [{STR_W + 8 * v + 5:04x}]
        ld r3, [{STR_W + 8 * v + 7:04x}]
    .endif
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


;; EDM_STR_ONE (STR_TAP with one request per voice; measured 2026-11-04: 6 of our tapreq per sample ran, 8 hung):
;; the block's request at age d gives x0, x1 (ages d, d + 1 in the main's terms: w[n - d - 1], w[n - d - 2]);
;;   z = x0 + g (x1 - x0)       g = C020 + v (region 0), the period's fraction with the filter's delay taken out
;;   s = s + b (z - s)          one-pole brightness, b = C01D; s in bank 0 STR1_S + 2v (stored at a, read a + 1)
;;   w = x + fb s -> W          period = d + 1 + g + phase delay of the two filters at the note (make_s1cc.py str1_tune)
.fragment master_str1
ld r4, [c{STR_PA:03x}]
mov r6, #0.0
.for v in range(str_nv())
    ;; voice {v}
    ld r0, [{STR_RES + 8 * v:04x}]
    ld r1, [{STR_RES + 8 * v + 1:04x}]
    ld r2, [c{STR_PF + v:03x}]
    ld r3, [{STR1_S + 2 * v + 1:04x}]
    subxy r1, r0, r1
    ld r7, [{0x6006 + v:04x}]
    mul r1, r1, r2
    mov r5, r5
    addxy r0, r0, r1
    mov r5, r5
    subxy r0, r3, r0
    mov r5, r5
    mul r0, r0, r4
    ld r2, [c{STR_PFB + v:03x}]
    addxy r3, r3, r0
    mov r5, r5
    mul r0, r3, r2
    st r3, [{STR1_S + 2 * v:04x}]
    addxy r0, r0, r7
    .if STR_COMB
        ;; EDM_STR_COMB (Braids SAW_COMB): the line gets x + res z, the output is x + z (b = 1: s = z)
        addxy r1, r3, r7
        st r0, [{STR_W + 8 * v:04x}]
        addxy r6, r6, r1
    .else
        mov r5, r5
        st r0, [{STR_W + 8 * v:04x}]
        addxy r6, r6, r0
    .endif
.endfor
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


;; EDM_STR_PL (Plaits physical_modelling/string.cc on the same lines, one request per voice as master_str1):
;;   z   = x0 + g (x1 - x0)                         g = C020 + v
;;   lp  = two one-poles b (C040 + v): Plaits' damping SVF at Q 0.5 is exactly two equal one-poles in series
;;   s   = three first-order allpasses a (C048 + v): y = a (u - y1) + x1, the stiffness dispersion without a second
;;         line (Plaits uses an allpass on a delay line: a second tapreq per voice, past the 6 per sample that run)
;;   w   = x + fb s -> W                            fb = C028 + v (AMP DECAY / RELEASE)
;; period = d + 1 + phase delay of all of it at the note (make_s1cc.py str_pl_tune). States in bank 0
;; STR1_S + 16 v: lp1, lp2, ap1 x / y, ap2 x / y, ap3 x / y (stored at a, read at a + 1).
.fragment master_pstr
mov r6, #0.0
.for v in range(str_nv())
    ;; voice {v}
    ld r0, [{STR_RES + 8 * v:04x}]
    ld r1, [{STR_RES + 8 * v + 1:04x}]
    ld r2, [c{STR_PF + v:03x}]
    ld r3, [{STR1_S + 16 * v + 1:04x}]
    subxy r1, r0, r1
    ld r4, [c{STR_PB + v:03x}]
    mul r1, r1, r2
    ld r5, [{STR1_S + 16 * v + 3:04x}]
    addxy r0, r0, r1
    ld r7, [{STR1_S + 16 * v + 5:04x}]
    subxy r0, r3, r0
    ld r2, [c{STR_PAP + v:03x}]
    mul r0, r0, r4
    ld r1, [{STR1_S + 16 * v + 7:04x}]
    addxy r3, r3, r0
    mov r4, r4
    subxy r0, r5, r3
    st r3, [{STR1_S + 16 * v:04x}]
    mul r0, r0, r4
    mov r4, r4
    addxy r5, r5, r0
    mov r4, r4
    st r5, [{STR1_S + 16 * v + 2:04x}]
    ;; allpass 1: u = r5, x1 = r7, y1 = r1
    subxy r0, r1, r5
    st r5, [{STR1_S + 16 * v + 4:04x}]
    mul r0, r0, r2
    ld r3, [{STR1_S + 16 * v + 9:04x}]
    addxy r0, r0, r7
    ld r1, [{STR1_S + 16 * v + 11:04x}]
    st r0, [{STR1_S + 16 * v + 6:04x}]
    ;; allpass 2: u = r0, x1 = r3, y1 = r1
    subxy r5, r1, r0
    st r0, [{STR1_S + 16 * v + 8:04x}]
    mul r5, r5, r2
    ld r7, [{STR1_S + 16 * v + 13:04x}]
    addxy r5, r5, r3
    ld r1, [{STR1_S + 16 * v + 15:04x}]
    st r5, [{STR1_S + 16 * v + 10:04x}]
    ;; allpass 3: u = r5, x1 = r7, y1 = r1
    subxy r0, r1, r5
    st r5, [{STR1_S + 16 * v + 12:04x}]
    mul r0, r0, r2
    ld r3, [c{STR_PFB + v:03x}]
    addxy r0, r0, r7
    ld r7, [{0x6006 + v:04x}]
    st r0, [{STR1_S + 16 * v + 14:04x}]
    mul r1, r0, r3
    mov r4, r4
    addxy r1, r1, r7
    mov r4, r4
    st r1, [{STR_W + 8 * v:04x}]
    ;; the output is the string alone, as Plaits (not the excitation: with a dark damping the burst covered it)
    addxy r6, r6, r0
.endfor
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


;; EDM_STR_BLOWN (Braids digital_oscillator.cc RenderBlown, the reed in the bore): one request per voice; the breath
;; pressure from the slave on 6006 + v. Braids' Q15 / Q12 constants as floats:
;;   z  = x0 + g (x1 - x0); h = z / 2
;;   lp += b (h + h[n-1] - lp)                        the bore's loss (not in Braids: b = C048 + v, OSC2 COLOR)
;;   dp = -0.95 lp - breath                           (kReflectionCoefficient, the bore's end; h[n-1] in bank 0)
;;   reed = clip(-0.3 dp + 0.7, -1, 1)               (kReedSlope, kReedOffset)
;;   out  = clip(dp reed + breath, -1, 1) -> W
;;   body += c (out - body)                           (lut_flute_body_filter at the note, c = C040 + v) -> output
;; the sign at the bore's end makes the period twice the loop: the line, the average and the loss = P / 2
;; (make_s1cc.py blown_loop). States: bank 0 STR1_S + 16 v (h[n-1]), + 2 (body), + 4 (lp).
.fragment master_blown
mov r6, #0.0
.for v in range(str_nv())
    ;; voice {v}
    ld r0, [{STR_RES + 8 * v:04x}]
    ld r1, [{STR_RES + 8 * v + 1:04x}]
    ld r2, [c{STR_PF + v:03x}]
    ld r3, [{STR1_S + 16 * v + 1:04x}]
    subxy r1, r0, r1
    ld r7, [{0x6006 + v:04x}]
    mul r1, r1, r2
    ld r2, [{STR1_S + 16 * v + 5:04x}]
    addxy r0, r0, r1
    ld r5, [c{STR_PAP + v:03x}]
    mul r0, r0, #0.5
    ld r4, [{STR1_S + 16 * v + 3:04x}]
    addxy r1, r0, r3
    st r0, [{STR1_S + 16 * v:04x}]
    ;; the bore's loss (2026-11-04: without it the reed made a square on the device): lp += b (r - lp), b = C048 + v
    subxy r1, r2, r1
    mov r6, r6
    mul r1, r1, r5
    ld r5, [c{STR_PB + v:03x}]
    addxy r2, r2, r1
    mov r3, #-1.0
    st r2, [{STR1_S + 16 * v + 4:04x}]
    mul r1, r2, #-0.95
    mov r6, r6
    subxy r1, r7, r1
    mov r6, r6
    mul r2, r1, #-0.3
    mov r6, r6
    addxy r2, r2, #0.7
    mov r6, r6
    min r2, r2, #1.0
    mov r6, r6
    max r2, r2, r3
    mov r6, r6
    mul r2, r1, r2
    mov r6, r6
    addxy r2, r2, r7
    mov r6, r6
    min r2, r2, #1.0
    mov r6, r6
    max r2, r2, r3
    mov r6, r6
    st r2, [{STR_W + 8 * v:04x}]
    subxy r0, r4, r2
    mov r6, r6
    mul r0, r0, r5
    mov r6, r6
    addxy r4, r4, r0
    mov r6, r6
    st r4, [{STR1_S + 16 * v + 2:04x}]
    addxy r6, r6, r4
.endfor
ld r7, [c{M_VOLUME:03x}]
mul r6, r6, #0.25
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


;; EDM_STR_BOWED (Braids digital_oscillator.cc RenderBowed): two lines per voice, k = 2v the neck (bow -> nut -> bow),
;; k = 2v + 1 the bridge (bow -> bridge -> bow), one request each (edm_str_block1 over str_lines()); 3 voices.
;;   nut = lerp(neck, gN), br = lerp(bridge, gB)            gN = C020 + 2v, gB = C020 + 2v + 1
;;   lp  = G br + P lp                                       (kBridgeLPGain / Pole1 0.4275 / 0.55 at Braids' 48 kHz,
;;                                                            moved to 96 kHz: P = sqrt(0.55), the same DC gain 0.95)
;;   dv  = bow + lp + nut                                    (bow - string velocity; bow from the slave on 6006 + v)
;;   f   = min(1, (|dv s| + 0.75)^-4)                        (lut_bowing_friction = STK's bow table; s = C040 + v,
;;                                                            Braids' p0 / 32; 1 / y: quadratic seed + 2 Newton steps)
;;   neck <- f dv - lp, bridge <- f dv - nut
;;   body: t = gin br + 1.843 y0 - 0.85 y1, out = t       (Braids' kBiquad poles moved from 48 kHz to 96 kHz:
;;                                                            493 Hz; as a low pass with unity gain, gin = 1 - p1 - p2:
;;                                                            Braids' band pass, out = t - y1, put the fundamental
;;                                                            15 dB under the 7th harmonic: metallic on hw 2026-11-04)
;; States: bank 0 STR1_S + 16 v: lp, + 2 y0, + 4 y1.
.fragment master_bowed
mov r6, #0.0
.for v in range(str_nv())
    ;; voice {v}
    ld r0, [{STR_RES + 16 * v:04x}]
    ld r1, [{STR_RES + 16 * v + 1:04x}]
    ld r2, [c{STR_PF + 2 * v:03x}]
    ld r3, [{STR_RES + 16 * v + 8:04x}]
    subxy r1, r0, r1
    ld r4, [{STR_RES + 16 * v + 9:04x}]
    mul r1, r1, r2
    ld r5, [c{STR_PF + 2 * v + 1:03x}]
    addxy r0, r0, r1
    subxy r4, r3, r4
    ld r7, [{STR1_S + 16 * v + 1:04x}]
    .if BOW_DIAG and v == 0
        ;; BOW_DIAG: voice 0's nut value (the neck line's read) to the right send, one sample later
        st r0, [0370]
    .endif
    mul r4, r4, r5
    mov r6, r6
    addxy r3, r3, r4
    mul r7, r7, #{BOW_LPP}
    mul r1, r3, #{BOW_LPG}
    mov r6, r6
    addxy r7, r7, r1
    ld r2, [{0x6006 + v:04x}]
    st r7, [{STR1_S + 16 * v:04x}]
    addxy r1, r7, r0
    ld r5, [c{STR_PB + v:03x}]
    addxy r2, r2, r1
    mov r6, r6
    mul r1, r2, r5
    mov r6, r6
    ;; |dv s| = max(x, -x) + 0.75 with proven ops (absadd #0.75 never ran on the S1: on hw the string went to the
    ;; octave, 2026-11-04, while the emulator kept the fundamental)
    mul r4, r1, #-1.0
    mov r6, r6
    max r1, r1, r4
    mov r6, r6
    addxy r1, r1, #0.75
    mov r6, r6
    min r1, r1, #4.75
    mov r6, r6
    mul r4, r1, #{BOW_SEED[2]}
    mov r6, r6
    addxy r4, r4, #{BOW_SEED[1]}
    mov r6, r6
    mul r4, r4, r1
    mov r6, r6
    addxy r4, r4, #{BOW_SEED[0]}
    mov r6, r6
    .for _ in range(2)
        mul r5, r1, r4
        mov r6, r6
        subxy r5, r5, #2.0
        mov r6, r6
        mul r4, r4, r5
        mov r6, r6
    .endfor
    mul r4, r4, r4
    mov r6, r6
    mul r4, r4, r4
    mov r6, r6
    min r4, r4, #1.0
    mov r6, r6
    mul r4, r4, r2
    mov r6, r6
    subxy r1, r7, r4
    subxy r5, r0, r4
    st r1, [{STR_W + 16 * v:04x}]
    st r5, [{STR_W + 16 * v + 8:04x}]
    ld r0, [{STR1_S + 16 * v + 3:04x}]
    ld r1, [{STR1_S + 16 * v + 5:04x}]
    mul r3, r3, #{BOW_GIN}
    mov r6, r6
    mul r0, r0, #{BOW_P1}
    mul r2, r1, #{BOW_P2}
    addxy r3, r3, r0
    ld r7, [{STR1_S + 16 * v + 3:04x}]
    addxy r3, r3, r2
    mov r6, r6
    mov r4, r3
    st r3, [{STR1_S + 16 * v + 2:04x}]
    st r7, [{STR1_S + 16 * v + 4:04x}]
    addxy r6, r6, r4
.endfor
ld r7, [c{M_VOLUME:03x}]
mul r6, r6, #{BOW_GAIN}
mov r3, r3
mul r6, r6, r7
mov r3, r3
st r6, [{FX_SEND[0]:04x}]
.if BOW_DIAG
    ld r5, [0371]
    mov r4, r4
    st r5, [{FX_SEND[1]:04x}]
.else
    st r6, [{FX_SEND[1]:04x}]
.endif
ld r2, [{FX_RET[0]:04x}]
ld r3, [{FX_RET[1]:04x}]
mov r4, r4
st r2, [{M_OUT[0]:04x}]
st r3, [{M_OUT[1]:04x}]
mov r2, #0.0
.for x in M_ZERO
    st r2, [{x:04x}]
.endfor
