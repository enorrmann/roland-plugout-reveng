;; SYSTEM-S1CC host microcode: the noise sources and the host LFO (DSP B). Template syntax: make_s1cc.py frag()

.fragment noise dsp G
;; a DSP's global noise source (MUTABLE-SYSTEM step 11 step_noise, proven on the device): n = wrap(K n + w), w a
;; Weyl counter. DSP A: the mixer's NOISE; DSP B: the LFOs' sample & hold. G: lfo_block(dsp)
; host: noise n = wrap(K n + w), w = Weyl counter (DSP {dsp})
ld r2, [{G["nz"] + 1:04x}]
ld r0, [{G["w"] + 1:04x}]
mov r1, r1
add r0, #{NOISE_W!r}:32
ph_wrap r0, r3, r0
st r0, [{G["w"]:04x}]
mul r2, r2, #{NOISE_K!r}
addxy r2, r2, r0
ph_wrap r2, r3, r2
st r2, [{G["nz"]:04x}]


.fragment lfo_b v hb G
;; the host LFO of voice v on DSP B, before its envelopes; four registers r0..r3 = a, b, c, d (pairs: r4..r7 for the
;; odd voice); hb: the voice's host words on DSP B, G: lfo_block('B')
;; * note start d: the trigger copy changed; the phase restarts there only with LFO KEY TRIG (B6_LKT = 1)
;; * LFO (MUTABLE-SYSTEM step 11 step_lfo_voice, proven on the device): phase restarted at the note start, own S&H
;;   of DSP B's noise at each wrap, own sample & glide g += k (h - g) (k = 1: sample & hold);
;;   lfo = w g + ph_clamp(K p) + t (alpha + beta t^2), t the triangle of the phase p (B6_LSG .. : k, alpha, beta,
;;   K, w per shape); fade f 0 -> 1 from the gate copy (DELAY TIME), lf = f lfo -> bank 5 LFB+v for DSP A
;; * -> amp: 1 + f (M lf - |M|) (tremolo down from full level; M < 0 the inverted LFO) -> s1_lfo_amp()
;; * -> filter: F lf octaves -> s1_lfo_filter()
;; ARM_LFO: lf (x fade) and the fade come from the ARM, ramped by the hardware
; voice {v}: host LFO (DSP B), b = d = 1 at a note start
ld r0, [{TB[v]:04x}]
ld r1, [{hb["bpt"] + 1:04x}]
mov r0, r0
st r0, [{hb["bpt"]:04x}]
subxy r1, r1, r0
mul r1, r1, r1
.if PENV and MOD_B
    st r1, [{hb["bd"]:04x}]
.endif
.if ARM_LFO
    ; voice {v}: LFO from the ARM (ARM_LFO): d = lf, a = fade
    ld r3, [c{P_LFO0 + v:03x}]
    ld r0, [c{P_FADE0 + v:03x}]
.else
    ; voice {v}: LFO phase a, 0 at a note start; c = 2 at a wrap
    ld r0, [{hb["lph"] + 1:04x}]
    ld r2, [c0{B6_LRATE:02x}]
    ld r3, [c0{B6_LKT:02x}]
    mov r0, r0
    mul r1, r1, r3
    mad r0, r0, -r1
    addxy r2, r0, r2
    ph_wrap r0, r3, r2
    st r0, [{hb["lph"]:04x}]
    subxy r2, r0, r2
    ; voice {v}: sample & hold b of the noise at each wrap
    ld r1, [{hb["lsh"] + 1:04x}]
    ld r3, [{G["nz"]:04x}]
    mov r3, r3
    subxy r3, r1, r3
    mul r3, r3, r2
    mad r1, r3, #0.5
    st r1, [{hb["lsh"]:04x}]
    ; voice {v}: sample & glide d += k (b - d)
    ld r2, [c0{B6_LSG:02x}]
    ld r3, [{hb["lsg"] + 1:04x}]
    mov r3, r3
    subxy r1, r3, r1
    mad r3, r1, r2
    st r3, [{hb["lsg"]:04x}]
    .if LFO_LUT
        .include lfo_shape_lut v=v, hb=hb
    .else
        ; voice {v}: lfo d = w g + ph_clamp(K p) + t (alpha + beta t^2)
        ld r1, [c0{B6_LWAVE + 3:02x}]
        mov r1, r1
        mul r3, r3, r1
        ld r1, [c0{B6_LWAVE + 2:02x}]
        mov r1, r1
        mul r2, r0, r1
        ph_clamp r2, r3, -r2
        addxy r3, r3, r2
        ph_tri r0, r3, -r0
        mul r2, r0, r0
        ld r1, [c0{B6_LWAVE + 1:02x}]
        mov r1, r1
        mul r2, r2, r1
        ld r1, [c0{B6_LWAVE:02x}]
        mov r1, r1
        addxy r2, r2, r1
        mad r3, r0, r2
    .endif
    ; voice {v}: fade a (0 -> 1 from the gate copy, DELAY TIME), d = lf = lfo x fade -> DSP A
    ld r0, [{hb["fade"] + 1:04x}]
    ld r1, [c0{B6_LDELAY:02x}]
    ld r2, [{GB[v]:04x}]
    mov r2, r2
    addxy r0, r0, r1
    mul r0, r0, r2
    min r0, r0, #1.0
    st r0, [{hb["fade"]:04x}]
    mul r3, r3, r0
.endif
st r3, [{LFB[v]:04x}]
.if cmod_idx()
    st r3, [{cmt(1, v):04x}]
.endif
; voice {v}: -> amp 1 + f (M lf - |M|), -> filter F lf (for the kernel)
ld r1, [c0{B6_LAMP:02x}]
ld r2, [c0{B6_LAMPABS:02x}]
mov r1, r1
mul r1, r3, r1
mad r1, r0, -r2
add r1, #1.0
st r1, [{hb["lfa"]:04x}]
ld r1, [c0{B6_LFILT:02x}]
mov r1, r1
mul r1, r3, r1
st r1, [{hb["lff"]:04x}]
.if PENV and MOD_B
    .include penv_b v=v, hb=hb
.endif


.fragment lfo_shape_lut v hb
;; LFO_LUT: d = w g + clamp(amp T), T = the shape's table at x = (p + 1) N / 2 from the sample before (luts_all
;; reads x after the voices; B6_LWAVE = h of the table's first word, + 1 = amp, + 3 = w; LFO_SHAPES): amp 1 the
;; table itself, 256 on the saw the square (as the formula's ph_clamp(256 p)), 0 for S&G / S&H; a = the phase p
;; (then free), d = the sample & glide value g
; voice {v}: lfo d = w g + clamp(amp T) (LFO_LUT: T from luts_all, one sample late)
ld r1, [c0{B6_LWAVE + 3:02x}]
mov r1, r1
mul r3, r3, r1
mul r2, r0, #{float(LUT_LN // 2)!r}
addxy r2, r2, #{float(LUT_LN // 2)!r}
st r2, [{hb["lx"]:04x}]
ld r0, [{hb["ly"] + 1:04x}]
ld r1, [c0{B6_LWAVE + 1:02x}]
mov r1, r1
mul r0, r0, r1
ph_clamp r0, r3, -r0
addxy r3, r3, r0
