;; SYSTEM-S1CC host microcode for a fused kernel (s1cc --fused). Template syntax: make_s1cc.py frag()

.fragment kcopy v pairs
;; the inputs voice v's kernel reads on the other DSP: each (source, bank-5 slot KX0 + 8 j + v) of make_s1cc.py
;; k_copies, after the host made it; the kernel reads the slot (this sample or the one before: the DSPs race)
; voice {v}: inputs for the fused kernel on the other DSP
.for j, src, dst in [(j, s, d) for j, (s, d) in enumerate(pairs)]
    ld r{j % 4}, [{src:04x}]
    st r{j % 4}, [{dst:04x}]
.endfor


.fragment mix_term v h
;; the host mixer's term for voice v's fused kernel, before it (the kernel adds it between its A and B parts:
;; s1_host_mix(), s1cc fuse): SUB x sub + NOISE x noise -> h["hm"] (microcode/host_a.s mixer, without the kernel's
;; output); 0 without the host mixer (MIX=0, or no host LFO)
.if HOST_MIX
    ; voice {v}: SUB / NOISE for the fused kernel
    ld r0, [{SUBB[v]:04x}]
    ld r2, [c0{A6_SUBMIX:02x}]
    ld r1, [{lfo_block("A")["nz"]:04x}]
    ld r3, [c0{A6_NOISEMIX:02x}]
    mul r0, r0, r2
    mad r0, r1, r3
    st r0, [{h["hm"]:04x}]
.else
    mov r0, #0.0
    st r0, [{h["hm"]:04x}]
.endif
