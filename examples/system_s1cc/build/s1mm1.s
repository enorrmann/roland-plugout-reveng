;; S1_MASTER voice 0
ld r5, [4080]
ld r6, [4081]
ld r1, [c030]
ld r2, [c020]
subxy r7, r5, r6
st r1, [0310]
st r1, [0311]
mul r1, r2, r7
ld r2, [0301]
addxy r6, r1, r5
ld r3, [c040]
subxy r7, r2, r6
ld r6, [0303]
mul r1, r3, r7
ld r4, [c048]
addxy r5, r1, r2
ld r0, [6006]
subxy r7, r6, r5
st r5, [0300]
mul r1, r3, r7
ld r5, [0305]
addxy r2, r1, r6
ld r7, [0307]
subxy r3, r5, r2
ld r5, [0309]
mul r1, r4, r3
st r2, [0302]
addxy r6, r1, r7
st r2, [0306]
subxy r2, r5, r6
ld r3, [030b]
mul r1, r4, r2
ld r5, [030d]
addxy r7, r1, r3
st r6, [0304]
st r6, [030a]
subxy r6, r5, r7
ld r5, [c01e]
mul r1, r4, r6
ld r4, [c01d]
st r7, [0308]
st r7, [030e]
ld r2, [030f]
ld r7, [c028]
subxy r6, r5, r4
addxy r3, r1, r2
mul r1, r7, r6
st r3, [030c]
addxy r2, r1, r5
st r3, [0312]
mul r4, r2, r3
mov r0, r0
addxy r7, r4, r0
mov r1, r1
st r7, [4040]
;; S1_MASTER voice 1
ld r5, [4088]
ld r6, [4089]
ld r1, [c031]
ld r2, [c021]
subxy r7, r5, r6
st r1, [0323]
st r1, [0324]
mul r1, r2, r7
ld r2, [0314]
addxy r6, r1, r5
ld r3, [c041]
subxy r7, r2, r6
ld r6, [0316]
mul r1, r3, r7
ld r4, [c049]
addxy r5, r1, r2
ld r0, [6007]
subxy r7, r6, r5
st r5, [0313]
mul r1, r3, r7
ld r5, [0318]
addxy r2, r1, r6
ld r7, [031a]
subxy r3, r5, r2
ld r5, [031c]
mul r1, r4, r3
st r2, [0315]
addxy r6, r1, r7
st r2, [0319]
subxy r2, r5, r6
ld r3, [031e]
mul r1, r4, r2
ld r5, [0320]
addxy r7, r1, r3
st r6, [0317]
st r6, [031d]
subxy r6, r5, r7
ld r5, [c01e]
mul r1, r4, r6
ld r4, [c01d]
st r7, [031b]
st r7, [0321]
ld r2, [0322]
ld r7, [c029]
subxy r6, r5, r4
addxy r3, r1, r2
mul r1, r7, r6
st r3, [031f]
addxy r2, r1, r5
st r3, [0325]
mul r4, r2, r3
mov r0, r0
addxy r7, r4, r0
mov r1, r1
st r7, [4048]
;; S1_MASTER voice 2
ld r5, [4090]
ld r6, [4091]
ld r1, [c032]
ld r2, [c022]
subxy r7, r5, r6
st r1, [0336]
st r1, [0337]
mul r1, r2, r7
ld r2, [0327]
addxy r6, r1, r5
ld r3, [c042]
subxy r7, r2, r6
ld r6, [0329]
mul r1, r3, r7
ld r4, [c04a]
addxy r5, r1, r2
ld r0, [6008]
subxy r7, r6, r5
st r5, [0326]
mul r1, r3, r7
ld r5, [032b]
addxy r2, r1, r6
ld r7, [032d]
subxy r3, r5, r2
ld r5, [032f]
mul r1, r4, r3
st r2, [0328]
addxy r6, r1, r7
st r2, [032c]
subxy r2, r5, r6
ld r3, [0331]
mul r1, r4, r2
ld r5, [0333]
addxy r7, r1, r3
st r6, [032a]
st r6, [0330]
subxy r6, r5, r7
ld r5, [c01e]
mul r1, r4, r6
ld r4, [c01d]
st r7, [032e]
st r7, [0334]
ld r2, [0335]
ld r7, [c02a]
subxy r6, r5, r4
addxy r3, r1, r2
mul r1, r7, r6
st r3, [0332]
addxy r2, r1, r5
st r3, [0338]
mul r4, r2, r3
mov r0, r0
addxy r7, r4, r0
mov r1, r1
st r7, [4050]
;; S1_MASTER voice 3
ld r5, [4098]
ld r6, [4099]
ld r1, [c033]
ld r2, [c023]
subxy r7, r5, r6
st r1, [0349]
st r1, [034a]
mul r1, r2, r7
ld r2, [033a]
addxy r6, r1, r5
ld r3, [c043]
subxy r7, r2, r6
ld r6, [033c]
mul r1, r3, r7
ld r4, [c04b]
addxy r5, r1, r2
ld r0, [6009]
subxy r7, r6, r5
st r5, [0339]
mul r1, r3, r7
ld r5, [033e]
addxy r2, r1, r6
ld r7, [0340]
subxy r3, r5, r2
ld r5, [0342]
mul r1, r4, r3
st r2, [033b]
addxy r6, r1, r7
st r2, [033f]
subxy r2, r5, r6
ld r3, [0344]
mul r1, r4, r2
ld r5, [0346]
addxy r7, r1, r3
st r6, [033d]
st r6, [0343]
subxy r6, r5, r7
ld r5, [c01e]
mul r1, r4, r6
ld r4, [c01d]
st r7, [0341]
st r7, [0347]
ld r2, [0348]
ld r7, [c02b]
subxy r6, r5, r4
addxy r3, r1, r2
mul r1, r7, r6
st r3, [0345]
addxy r2, r1, r5
st r3, [034b]
mul r4, r2, r3
mov r0, r0
addxy r7, r4, r0
mov r1, r1
st r7, [4058]
;; S1_MASTER voice 4
ld r5, [40a0]
ld r6, [40a1]
ld r1, [c034]
ld r2, [c024]
subxy r7, r5, r6
st r1, [035c]
st r1, [035d]
mul r1, r2, r7
ld r2, [034d]
addxy r6, r1, r5
ld r3, [c044]
subxy r7, r2, r6
ld r6, [034f]
mul r1, r3, r7
ld r4, [c04c]
addxy r5, r1, r2
ld r0, [600a]
subxy r7, r6, r5
st r5, [034c]
mul r1, r3, r7
ld r5, [0351]
addxy r2, r1, r6
ld r7, [0353]
subxy r3, r5, r2
ld r5, [0355]
mul r1, r4, r3
st r2, [034e]
addxy r6, r1, r7
st r2, [0352]
subxy r2, r5, r6
ld r3, [0357]
mul r1, r4, r2
ld r5, [0359]
addxy r7, r1, r3
st r6, [0350]
st r6, [0356]
subxy r6, r5, r7
ld r5, [c01e]
mul r1, r4, r6
ld r4, [c01d]
st r7, [0354]
st r7, [035a]
ld r2, [035b]
ld r7, [c02c]
subxy r6, r5, r4
addxy r3, r1, r2
mul r1, r7, r6
st r3, [0358]
addxy r2, r1, r5
st r3, [035e]
mul r4, r2, r3
mov r0, r0
addxy r7, r4, r0
mov r1, r1
st r7, [4060]
;; S1_MASTER voice 5
ld r5, [40a8]
ld r6, [40a9]
ld r1, [c035]
ld r2, [c025]
subxy r7, r5, r6
st r1, [036f]
st r1, [0370]
mul r1, r2, r7
ld r2, [0360]
addxy r6, r1, r5
ld r3, [c045]
subxy r7, r2, r6
ld r6, [0362]
mul r1, r3, r7
ld r4, [c04d]
addxy r5, r1, r2
ld r0, [600b]
subxy r7, r6, r5
st r5, [035f]
mul r1, r3, r7
ld r5, [0364]
addxy r2, r1, r6
ld r7, [0366]
subxy r3, r5, r2
ld r5, [0368]
mul r1, r4, r3
st r2, [0361]
addxy r6, r1, r7
st r2, [0365]
subxy r2, r5, r6
ld r3, [036a]
mul r1, r4, r2
ld r5, [036c]
addxy r7, r1, r3
st r6, [0363]
st r6, [0369]
subxy r6, r5, r7
ld r5, [c01e]
mul r1, r4, r6
ld r4, [c01d]
st r7, [0367]
st r7, [036d]
ld r2, [036e]
ld r7, [c02d]
subxy r6, r5, r4
addxy r3, r1, r2
mul r1, r7, r6
st r3, [036b]
addxy r2, r1, r5
st r3, [0371]
mul r4, r2, r3
mov r0, r0
addxy r7, r4, r0
mov r1, r1
st r7, [4068]
ld r6, [0312]
ld r0, [0325]
mov r7, r7
addxy r6, r6, r0
ld r0, [0338]
mov r7, r7
addxy r6, r6, r0
ld r0, [034b]
mov r7, r7
addxy r6, r6, r0
ld r0, [035e]
mov r7, r7
addxy r6, r6, r0
ld r0, [0371]
mov r7, r7
addxy r6, r6, r0
ld r7, [c01c]
mul r6, r6, #0.5
mov r3, r3
mul r6, r6, r7
mov r3, r3
st r6, [2000]
st r6, [2002]
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
