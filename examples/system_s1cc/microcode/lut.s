;; SYSTEM-S1CC host microcode: bank-5 table lookups (LFO_LUT / ENV_CURVE, luts_all). Template syntax: make_s1cc.py frag()

.fragment lut_head n
; table lookups (LUT): {n} per sample, h6 / h7 alternating
.if LFO_LUT
    ;; h of the LFO shape's table
    ld r3, [c0{B6_LWAVE:02x}]
    ld r7, [c0{B6_LWAVE:02x}]
.endif


.fragment lut_job x t d src dsts scale base h
;; one lookup: x = the word src (x scale), d = T(x) (lut_read), stored at each of dsts
ld {x}, [{src:04x}]
.if scale
    mul {x}, {x}, #{scale!r}
.endif
.include lut_read x=x, t=t, d=d, base=base, h=h
.for a in dsts
    st {d}, [{a:04x}]
.endfor


.fragment lut_read x t d base h
;; d = T(x) from the table at base (x in table points): fl = floor x, fr = x - fl, h = word of fl, two indexed
;; loads, d = T0 + fr (T1 - T0). x is overwritten (fr), t is a scratch register; base: an immediate (float) or a
;; register holding h of the table's first word; h: 6 (ldb5) or 7 (ldb5h7)
floor {t}, {x}
subxy {x}, {t}, {x}
mul {t}, {t}, #{2.0 ** -14!r}
.if isinstance(base, float)
    addxy {t}, {t}, #{base!r}
.else
    addxy {t}, {t}, {base}
.endif
hset h{h}, {t}
{"ldb5" if h == 6 else "ldb5h7"} {d}, 0
{"ldb5" if h == 6 else "ldb5h7"} {t}, 1
subxy {t}, {d}, {t}
mad {d}, {x}, {t}
