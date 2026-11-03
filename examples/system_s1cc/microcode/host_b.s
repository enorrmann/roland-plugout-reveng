;; SYSTEM-S1CC host microcode: slave DSP B around the kernel. Template syntax: make_s1cc.py frag()

.fragment head_b
;; the mix of the sample before's voice outputs (B outputs: ring S+1; A outputs as they are) x GAIN x C000 (the
;; volume) -> 6006 / 6007, the other outputs 0; then the noise of the host LFOs' sample & hold
; s1m1: slave DSP B main @0x0100: mix of {NV} voices x {GAIN!r} x C000 -> 6006 / 6007{"; per voice s1cc kernel DSP B" if HAS_B else ""}
mov r7, #0.0
.for v in range(NV)
    ld r{v & 1}, [{(B_OUT[v] + 1) if HAS_B else XV[v]:04x}]
    addxy r7, r7, r{v & 1}
.endfor
.if ARM_DIAG
    ;; the ARM's callback toggle, audible
    ld r6, [c11f]
    mad r7, r6, #0.1
.endif
ld r6, [c000]
mul r7, r7, #{GAIN!r}
mul r7, r7, r6
st r7, [{OUT_L:04x}]
st r7, [{OUT_R:04x}]
mov r0, #0.0
.for x in OUT_ZERO
    st r0, [{x:04x}]
.endfor
.if HOST_LFO
    .include noise dsp="B", G=lfo_block("B")
.endif


.fragment host_env v i e lvl
;; host envelope i of voice v: e = its words (level, latch at e + 2, last trigger at e + 4), lvl = where the level
;; goes (CURVE: luts_all makes C(level) from it). ARM_ENV: the ARM's envelope, ramped by the hardware
.if ARM_ENV
    ; voice {v}: host envelope {i} from the ARM (ARM_ENV)
    ld r0, [c{P_ENV0 + 8 * i + v:03x}]
    st r0, [{lvl:04x}]
.else
    ; voice {v}: host envelope {i}
    .include adsr e=lvl, latch=e + 2, pt=e + 4, gate=GB[v], trig=TB[v], prm=0xC000 + H6_ENV[i]
.endif
.if CMOD and not MOD_B and not CURVE
    ;; its level for COLOR MOD on DSP A (CURVE: luts_all)
    st r0, [{ENVX[i][v]:04x}]
.endif


.fragment adsr e latch pt gate trig prm
;; C-S1SY's ADSR (examples/mutable_system/make_mutable.py step_adsr, proven on the device) in four registers,
;; r0..r3 (PAIRS=1 runs the odd voice's copy in r4..r7) -> r0 (stored at e). prm: sustain, attack, decay, release
;; rate. latch = 1 once the attack reached 1; target 1.25 (attack) / S (decay) / 0 (release); a change of trig
;; clears the latch (new note). The attack is g - latch: it follows the gate itself, so a trigger and a gate that
;; arrive apart lose nothing. The same math as step_adsr, except: the end of the attack, 65536 (e - 0.999), is two
;; clamped steps of 256 (one product passes the DSP's +-512); the attack is clamp(256 (g - latch)), 1 while the
;; latch is not full (with g - latch a latch caught half way in the 0.999 band, slow attacks, kept the envelope
;; stuck: 2026-10-28, the pitch envelope on the device); the level is loaded again for the last step instead of
;; holding a register
ld r0, [{e + 1:04x}]
ld r1, [{latch + 1:04x}]
ld r2, [{trig:04x}]
ld r3, [{pt + 1:04x}]
mov r3, r3
st r2, [{pt:04x}]
;; d = (trig - last trig)^2
subxy r3, r3, r2
mul r3, r3, r3
;; latch *= 1 - d
subxy r3, r3, #1.0
mul r1, r1, r3
;; two clamped steps: no overflow
addxy r2, r0, #-0.999
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
max r1, r1, r2
ld r3, [{gate:04x}]
mov r3, r3
mul r1, r1, r3
st r1, [{latch:04x}]
;; attack = clamp(256 (g - latch))
subxy r2, r1, r3
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
;; 1 - g
subxy r3, r3, #1.0
;; rate = (1 - g) kR + attack kA + latch kD
ld r0, [{prm + 3:04x}]
mov r0, r0
mul r3, r3, r0
ld r0, [{prm + 1:04x}]
mov r0, r0
mad r3, r2, r0
ld r0, [{prm + 2:04x}]
mov r0, r0
mad r3, r1, r0
;; target
ld r0, [{prm:04x}]
mov r0, r0
mul r1, r1, r0
mad r1, r2, #1.25
ld r0, [{e + 1:04x}]
mov r0, r0
subxy r1, r0, r1
mad r0, r1, r3
min r0, r0, #1.0
st r0, [{e:04x}]
