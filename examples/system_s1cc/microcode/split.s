;; SYSTEM-S1CC host microcode for a fused kernel (s1cc --fused). Template syntax: make_s1cc.py frag()

.fragment kcopy v pairs
;; the inputs voice v's kernel reads on the other DSP: each (source, bank-5 slot KX0 + 8 j + v) of make_s1cc.py
;; k_copies, after the host made it; the kernel reads the slot (this sample or the one before: the DSPs race)
; voice {v}: inputs for the fused kernel on the other DSP
.for j, src, dst in [(j, s, d) for j, (s, d) in enumerate(pairs)]
    ld r{j % 4}, [{src:04x}]
    st r{j % 4}, [{dst:04x}]
.endfor
