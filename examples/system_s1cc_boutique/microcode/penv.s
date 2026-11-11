;; SYSTEM-S1CC host microcode: the pitch envelope. Template syntax: make_s1cc.py frag()

.fragment penv_core v pe pf prm
;; pitch envelope AD (the host ADSR's math, adsr, proven on the device, with the gate at 1 and sustain 0): latch
;; cleared at a note start (r1 = 1), attack flag clamp(256 (1 - latch)), level toward 1.25 attack at rate
;; kD + attack (kA - kD) -> r0, stored at word pe; pf = the latch; prm = the decay rate's parameter C0xx (the attack
;; rate at prm + 1). Registers r0..r3 = a, b, c, d
; voice {v}: pitch envelope a: the ADSR's latch form with gate 1, sustain 0
ld r0, [{pe + 1:04x}]
ld r2, [{pf + 1:04x}]
mov r2, r2
;; latch *= 1 - d
mad r2, r2, -r1
;; latch = 1 once the level passed 0.999: 256 (256 (e - 0.999)) in two clamped steps (one product would pass the
;; DSP's +-512)
addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1
st r2, [{pf:04x}]
;; attack flag c = clamp(256 (1 - latch)): 1 until the latch is full. With the attack 1 - latch itself a latch
;; caught half way in the 0.999 band (slow attacks) left the target 1.25 (1 - latch) under the level and the
;; envelope stuck there (2026-10-28, on the device: it stayed at a value, never fell to 0)
subxy r2, r2, #1.0
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
ld r1, [c0{prm:02x}]
ld r3, [c0{prm + 1:02x}]
mov r3, r3
;; rate = kD + attack (kA - kD)
subxy r3, r1, r3
mad r1, r2, r3
;; target 1.25 attack
mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1
min r0, r0, #1.0
st r0, [{pe:04x}]


.fragment penv_b v hb
;; the pitch envelope of voice v on DSP B (MOD_DSP=B), after the LFO: penv_core with the note start stored by
;; lfo_b -> word pe (COLOR MOD; CMOD_IDX: the level in the COLOR MOD table) and bank 5 PEB+v (-> pitch on DSP A)
ld r1, [{hb["bd"]:04x}]
.include penv_core v=v, pe=cmt(2, v) if cmod_idx() else hb["pe"], pf=hb["pf"], prm=B6_PENV
.if PITCH_MOD
    st r0, [{PEB[v]:04x}]
.endif
