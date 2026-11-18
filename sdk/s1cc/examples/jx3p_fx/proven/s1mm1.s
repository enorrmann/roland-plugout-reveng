; JX-3P-S1-FX master B (s1mm1), FX_WET=1.5 FX_DEPTH=1.5, ran on the SYSTEM-1 2026-11-18 (12 pairs, 8 voices; user: no hangs).
; Frozen copy for asm/s1rules.py PROVEN_PROGRAMS (master-B shapes: bank-2 loads / stores of the lines and requests).
;; FX_KERNEL: the master effect
ld r0, [6006]
ld r1, [0301]
mul r2, r0, #0.5
ld r4, [0303]
addxy r3, r1, r2
mul r5, r4, #0.5
st r1, [0302]
addxy r6, r3, r5
ld r1, [c030]
ld r7, [0305]
mul r2, r6, r1
ld r3, [0307]
ld r5, [c03b]
addxy r4, r7, r2
mul r6, r3, r5
st r7, [0306]
addxy r1, r4, r6
ld r7, [c043]
ld r3, [c043]
ld r6, [c034]
mul r2, r1, r7
mul r5, r1, r3
ld r4, [0309]
mul r7, r1, r6
ld r3, [030b]
st r2, [0304]
st r5, [0308]
addxy r2, r4, r7
mul r5, r3, #0.5
st r4, [030a]
addxy r1, r2, r5
ld r4, [c030]
ld r6, [030d]
mul r7, r1, r4
ld r2, [030f]
ld r5, [c03b]
addxy r3, r6, r7
mul r1, r2, r5
ld r2, [c043]
addxy r4, r3, r1
st r6, [030e]
ld r6, [c043]
mul r5, r4, r2
ld r3, [0311]
ld r1, [c03c]
mul r7, r4, r6
st r5, [0310]
mul r6, r3, r1
ld r2, [0313]
ld r5, [c035]
st r7, [030c]
addxy r7, r4, r6
mul r3, r2, r5
ld r4, [c042]
addxy r1, r7, r3
ld r2, [c042]
mul r6, r1, r4
mul r5, r1, r2
st r6, [0312]
ld r4, [0315]
ld r6, [0317]
st r5, [4040]
ld r7, [c042]
addxy r2, r4, r6
ld r5, [c045]
mul r3, r1, r7
addxy r1, r2, r5
st r3, [4048]
mov r7, r1
ld r4, [c020]
ph_wrap r7, r3, r7
ld r5, [c039]
mov r3, r7
st r7, [0314]
sign r3, r3
st r0, [0300]
mul r2, r7, r3
mul r6, r3, r4
mul r1, r2, r5
ld r4, [c024]
st r6, [0316]
addxy r6, r1, r4
ld r7, [4080]
ld r3, [4081]
ld r1, [c023]
st r6, [032c]
st r6, [032d]
ld r5, [4082]
mul r4, r2, r1
ld r6, [c022]
subxy r3, r7, r3
addxy r1, r4, r6
mul r5, r5, r3
ld r4, [4088]
ld r6, [4089]
st r1, [032e]
st r1, [032f]
addxy r3, r5, r7
ld r1, [408a]
subxy r5, r4, r6
mov r0, r0
mul r7, r1, r5
ld r1, [0319]
addxy r6, r7, r4
subxy r5, r1, r3
ld r7, [c031]
mov r0, r0
mul r4, r5, r7
ld r5, [031b]
addxy r3, r1, r4
ld r1, [c031]
subxy r7, r5, r3
st r3, [0318]
mul r4, r7, r1
ld r7, [031d]
addxy r3, r5, r4
subxy r1, r7, r6
ld r5, [c031]
st r3, [031a]
mul r4, r1, r5
ld r1, [031f]
addxy r6, r7, r4
ld r7, [c031]
subxy r5, r1, r6
st r6, [031c]
mul r4, r5, r7
ld r5, [6008]
ld r7, [0321]
addxy r6, r1, r4
subxy r1, r7, r5
ld r4, [0323]
ld r7, [c040]
st r5, [0320]
mul r5, r4, r7
ld r7, [c041]
addxy r4, r1, r5
ld r5, [c041]
mul r1, r4, r7
mul r7, r4, r5
ld r5, [c033]
st r1, [0322]
st r7, [0324]
ld r1, [0325]
mul r7, r4, r5
ld r5, [0327]
st r1, [0326]
addxy r4, r1, r7
mul r1, r5, #0.5
ld r5, [0329]
addxy r7, r4, r1
ld r4, [c025]
st r5, [032a]
mul r1, r7, r4
ld r4, [032b]
addxy r7, r5, r1
ld r5, [c03a]
st r6, [031e]
mul r1, r4, r5
ld r5, [c044]
addxy r4, r7, r1
ld r1, [c03d]
mul r7, r4, r5
addxy r5, r2, r1
st r7, [0328]
ld r1, [c032]
mul r7, r5, r4
addxy r5, r2, r1
ld r1, [c01d]
mul r2, r5, r4
ld r4, [c021]
ld r5, [c01e]
mul r2, r2, r4
mov r0, r0
addxy r4, r6, r2
mul r2, r5, r0
mul r6, r1, r4
ld r0, [c038]
addxy r4, r2, r6
mul r6, r7, r0
mul r5, r4, #2.0
addxy r4, r3, r6
st r5, [0331]
mul r5, r1, r4
mov r0, r0
addxy r7, r2, r5
mov r1, r1
mul r0, r7, #2.0
mov r2, r2
st r0, [0330]
ld r6, [0330]
ld r5, [0331]
ld r7, [c01c]
mov r3, r3
mul r6, r6, r7
mul r5, r5, r7
mov r3, r3
st r6, [2000]
st r5, [2002]
ld r2, [2001]
ld r3, [2003]
mov r4, r4
st r2, [a118]
st r3, [a119]
mov r2, #0.0
st r2, [a11a]
st r2, [a11b]
st r2, [a11c]
st r2, [a11d]
