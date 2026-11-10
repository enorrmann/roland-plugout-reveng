;; SYSTEM-S1CC host microcode: the master, the close programs, the section heads. Template syntax: make_s1cc.py frag()

.fragment master
;; C-S1SY13's master B (examples/mutable_system make_mutable.py step_m1, STEP 13)
; s1mm1: master DSP B @0x1000: 6006 / 6007 x volume (C01C) -> OS effect send 2000 / 2002; effect return 2001 / 2003 -> A118 / A119
ld r0, [{OUT_L:04x}]
ld r1, [{OUT_R:04x}]
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
.if M2S_PROBE
; M2S_PROBE (docs/vol6_reference_designs_applications/mutable_memory.md §4.3): master B -> slave words 600E + k, k = 0..7: 1.0 for even k, 0.5 for odd
mov r4, #1.0
mov r5, #0.5
.for k in range(8)
    st r{4 + k % 2}, [{0x600E + k:04x}]
.endfor
.endif


.fragment close word
;; the close program at 0x7FF2: frame counter E000 -> A004 (DSP A) / A005 (DSP B), as every SYSTEM-1 PLUG-OUT
ld r0, [e000]
st r0, [{word:04x}]


.fragment section_head name dsp a
; {name}: slave DSP {dsp} section @0x{a:04x}, after the main
