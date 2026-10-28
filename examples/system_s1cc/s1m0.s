mov r6, #1.0 | ld r0, [037b] | ld r2, [0379] | ld r4, [c040]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | ld r1, [0303]
ph_wrap r0, r3, r0
addxy r2, r2, r0 | st r0, [037a] | ld r0, [0301]
ph_wrap r2, r3, r2 | ld r3, [a060]
subxy r0, r0, r3 | st r2, [0378]
mul r0, r0, r0 | st r3, [0300] | ld r2, [0378]
mad r1, r1, -r0
addxy r7, r1, r4
ph_wrap r1, r3, r7 | ld r3, [0305]
subxy r7, r1, r7 | subxy r4, r3, r2 | ld r2, [0307] | st r1, [0302]
mul r4, r4, r7
mad r3, r4, #0.5 | ld r4, [c041]
subxy r7, r2, r3 | st r3, [0304]
mad r2, r7, r4 | ld r4, [c047]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c046] | st r2, [0306]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c044]
mad r7, r1, r4 | ld r4, [c045]
mad r7, r3, r4 | ld r4, [c043]
ph_tri r3, r3, -r1 | ld r1, [0313]
mul r2, r3, r3 | mad r7, r3, r4 | ld r4, [c042]
mul r2, r2, #-0.5
add r2, #1.5
mul r2, r2, r3 | ld r3, [a038]
mad r7, r2, r4 | ld r2, [0309] | ld r4, [c04a]
addxy r2, r2, r4 | ld r4, [c04d]
mul r2, r2, r3 | ld r3, [c04e]
min r2, r2, #1.0
mul r7, r7, r2 | st r2, [0308]
mul r5, r7, r4 | ld r4, [c04c]
mad r5, r2, -r3 | ld r3, [a030] | ld r2, [0315]
add r5, #1.0 | mad r2, r2, -r0
mul r5, r7, r4 | st r5, [0310] | ld r4, [c04b]
mul r4, r7, r4 | mov r5, #0.041666666666666664 | st r5, [030e]
addxy r3, r3, r4
mul r3, r4, #0.34657359027997264 | addxy r4, r1, #-0.999 | st r3, [030a]
ymacd r5, r3, #0.16666666666666666 | mul r4, r4, #256.0
ymacd r5, r3, #0.5 | mul r4, r4, #256.0
ymacd r5, r3, #1.0
ymacd r5, r3, #1.0 | ld r3, [a040]
mul r5, r5, r5
mul r3, r3, r5 | mov r5, #0.0
max r4, r4, r5 | st r3, [030c] | ld r5, [c050]
min r4, r4, r6
max r2, r2, r4 | ld r4, [c049]
subxy r3, r2, #1.0 | st r2, [0314]
mul r0, r3, r4 | ld r4, [c048]
mad r0, r2, r4 | mul r4, r3, #1.25 | ld r2, [0317]
subxy r4, r1, r4
mad r1, r4, r0 | ld r0, [a040]
min r1, r1, r6 | ld r6, [c051] | ld r4, [a170]
mad r5, r7, r6 | addxy r0, r2, r0 | ld r6, [c052] | st r1, [0312]
ph_wrap r2, r3, r0 | ld r0, [a178]
mad r5, r1, r6 | mul r2, r2, #256.0 | ld r6, [c053] | st r2, [0316] | ld r1, [031b]
mad r5, r0, r6 | ph_clamp r2, r3, -r2 | ld r6, [c054] | ld r0, [030c]
mad r5, r4, r6 | ld r6, [c055]
mad r5, r2, r6 | mov r6, #1.0
mov r2, r0 | st r5, [0318]
mad r2, r1, #0.5
mul r3, r2, #2.0
ph_wrap r4, r3, r3
addxy r7, r4, #1.0 | st r4, [031a]
ph_tri r1, r3, -r7
mul r2, r1, r1
addxy r3, r2, #-8.8404402339953698
mul r4, r2, r3 | ld r3, [0318]
addxy r7, r4, #21.601100021326904
addxy r4, r3, #-0.5 | mul r2, r1, r7
mul r1, r4, #0.17328679513998632 | mov r7, #0.041666666666666664
ymacd r7, r1, #0.16666666666666666
ymacd r7, r1, #0.5
ymacd r7, r1, #1.0
ymacd r7, r1, #1.0
mul r3, r7, r7
mul r4, r3, r3
mul r1, r4, r4
mul r7, r1, r1 | ld r1, [031d]
mul r3, r0, r7
min r4, r3, #0.25
mad r4, r1, #0.25
mul r0, r4, #4.0
ph_wrap r7, r3, r0
addxy r3, r7, #1.0 | st r7, [031c]
ph_tri r1, r3, -r3
mul r4, r1, r1
addxy r0, r4, #-8.8404402339953698
mul r7, r4, r0
addxy r3, r7, #21.601100021326904
mul r4, r1, r3 | ld r1, [c070]
mul r0, r2, r4 | ld r4, [0310]
mul r7, r0, #0.00390625 | ld r0, [030e] | st r4, [a1a0] | ld r4, [c040]
mul r3, r1, r7 | st r0, [a1a8] | ld r1, [030a]
mul r2, r3, #1.3530342883660893 | ld r0, [a038]
st r0, [a160] | ld r0, [031f]
st r1, [a1b0]
ld r3, [a061]
subxy r0, r0, r3 | ld r1, [a060] | st r3, [031e]
mul r0, r0, r0 | st r1, [a168] | ld r1, [0321]
mad r1, r1, -r0 | st r2, [a151] | ld r2, [0378]
addxy r7, r1, r4
ph_wrap r1, r3, r7 | ld r3, [0323]
subxy r7, r1, r7 | subxy r4, r3, r2 | ld r2, [0325] | st r1, [0320]
mul r4, r4, r7
mad r3, r4, #0.5 | ld r4, [c041]
subxy r7, r2, r3 | st r3, [0322]
mad r2, r7, r4 | ld r4, [c047]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c046] | st r2, [0324]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c044]
mad r7, r1, r4 | ld r4, [c045]
mad r7, r3, r4 | ld r4, [c043]
ph_tri r3, r3, -r1 | ld r1, [0331]
mul r2, r3, r3 | mad r7, r3, r4 | ld r4, [c042]
mul r2, r2, #-0.5
add r2, #1.5
mul r2, r2, r3 | ld r3, [a039]
mad r7, r2, r4 | ld r2, [0327] | ld r4, [c04a]
addxy r2, r2, r4 | ld r4, [c04d]
mul r2, r2, r3 | ld r3, [c04e]
min r2, r2, #1.0
mul r7, r7, r2 | st r2, [0326]
mul r5, r7, r4 | ld r4, [c04c]
mad r5, r2, -r3 | ld r3, [a031] | ld r2, [0333]
add r5, #1.0 | mad r2, r2, -r0
mul r5, r7, r4 | st r5, [032e] | ld r4, [c04b]
mul r4, r7, r4 | mov r5, #0.041666666666666664 | st r5, [032c]
addxy r3, r3, r4
mul r3, r4, #0.34657359027997264 | addxy r4, r1, #-0.999 | st r3, [0328]
ymacd r5, r3, #0.16666666666666666 | mul r4, r4, #256.0
ymacd r5, r3, #0.5 | mul r4, r4, #256.0
ymacd r5, r3, #1.0
ymacd r5, r3, #1.0 | ld r3, [a041]
mul r5, r5, r5
mul r3, r3, r5 | mov r5, #0.0
max r4, r4, r5 | st r3, [032a] | ld r5, [c050]
min r4, r4, r6
max r2, r2, r4 | ld r4, [c049]
subxy r3, r2, #1.0 | st r2, [0332]
mul r0, r3, r4 | ld r4, [c048]
mad r0, r2, r4 | mul r4, r3, #1.25 | ld r2, [0335]
subxy r4, r1, r4
mad r1, r4, r0 | ld r0, [a041]
min r1, r1, r6 | ld r6, [c051] | ld r4, [a171]
mad r5, r7, r6 | addxy r0, r2, r0 | ld r6, [c052] | st r1, [0330]
ph_wrap r2, r3, r0 | ld r0, [a179]
mad r5, r1, r6 | mul r2, r2, #256.0 | ld r6, [c053] | st r2, [0334] | ld r1, [0339]
mad r5, r0, r6 | ph_clamp r2, r3, -r2 | ld r6, [c054] | ld r0, [032a]
mad r5, r4, r6 | ld r6, [c055]
mad r5, r2, r6 | mov r6, #1.0
mov r2, r0 | st r5, [0336]
mad r2, r1, #0.5
mul r3, r2, #2.0
ph_wrap r4, r3, r3
addxy r7, r4, #1.0 | st r4, [0338]
ph_tri r1, r3, -r7
mul r2, r1, r1
addxy r3, r2, #-8.8404402339953698
mul r4, r2, r3 | ld r3, [0336]
addxy r7, r4, #21.601100021326904
addxy r4, r3, #-0.5 | mul r2, r1, r7
mul r1, r4, #0.17328679513998632 | mov r7, #0.041666666666666664
ymacd r7, r1, #0.16666666666666666
ymacd r7, r1, #0.5
ymacd r7, r1, #1.0
ymacd r7, r1, #1.0
mul r3, r7, r7
mul r4, r3, r3
mul r1, r4, r4
mul r7, r1, r1 | ld r1, [033b]
mul r3, r0, r7
min r4, r3, #0.25
mad r4, r1, #0.25
mul r0, r4, #4.0
ph_wrap r7, r3, r0
addxy r3, r7, #1.0 | st r7, [033a]
ph_tri r1, r3, -r3
mul r4, r1, r1
addxy r0, r4, #-8.8404402339953698
mul r7, r4, r0
addxy r3, r7, #21.601100021326904
mul r4, r1, r3 | ld r1, [c070]
mul r0, r2, r4 | ld r4, [032e]
mul r7, r0, #0.00390625 | ld r0, [032c] | st r4, [a1a1] | ld r4, [c040]
mul r3, r1, r7 | st r0, [a1a9] | ld r1, [0328]
mul r2, r3, #1.3530342883660893 | ld r0, [a039]
st r0, [a161] | ld r0, [033d]
st r1, [a1b1]
ld r3, [a062]
subxy r0, r0, r3 | ld r1, [a061] | st r3, [033c]
mul r0, r0, r0 | st r1, [a169] | ld r1, [033f]
mad r1, r1, -r0 | st r2, [a152] | ld r2, [0378]
addxy r7, r1, r4
ph_wrap r1, r3, r7 | ld r3, [0341]
subxy r7, r1, r7 | subxy r4, r3, r2 | ld r2, [0343] | st r1, [033e]
mul r4, r4, r7
mad r3, r4, #0.5 | ld r4, [c041]
subxy r7, r2, r3 | st r3, [0340]
mad r2, r7, r4 | ld r4, [c047]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c046] | st r2, [0342]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c044]
mad r7, r1, r4 | ld r4, [c045]
mad r7, r3, r4 | ld r4, [c043]
ph_tri r3, r3, -r1 | ld r1, [034f]
mul r2, r3, r3 | mad r7, r3, r4 | ld r4, [c042]
mul r2, r2, #-0.5
add r2, #1.5
mul r2, r2, r3 | ld r3, [a03a]
mad r7, r2, r4 | ld r2, [0345] | ld r4, [c04a]
addxy r2, r2, r4 | ld r4, [c04d]
mul r2, r2, r3 | ld r3, [c04e]
min r2, r2, #1.0
mul r7, r7, r2 | st r2, [0344]
mul r5, r7, r4 | ld r4, [c04c]
mad r5, r2, -r3 | ld r3, [a032] | ld r2, [0351]
add r5, #1.0 | mad r2, r2, -r0
mul r5, r7, r4 | st r5, [034c] | ld r4, [c04b]
mul r4, r7, r4 | mov r5, #0.041666666666666664 | st r5, [034a]
addxy r3, r3, r4
mul r3, r4, #0.34657359027997264 | addxy r4, r1, #-0.999 | st r3, [0346]
ymacd r5, r3, #0.16666666666666666 | mul r4, r4, #256.0
ymacd r5, r3, #0.5 | mul r4, r4, #256.0
ymacd r5, r3, #1.0
ymacd r5, r3, #1.0 | ld r3, [a042]
mul r5, r5, r5
mul r3, r3, r5 | mov r5, #0.0
max r4, r4, r5 | st r3, [0348] | ld r5, [c050]
min r4, r4, r6
max r2, r2, r4 | ld r4, [c049]
subxy r3, r2, #1.0 | st r2, [0350]
mul r0, r3, r4 | ld r4, [c048]
mad r0, r2, r4 | mul r4, r3, #1.25 | ld r2, [0353]
subxy r4, r1, r4
mad r1, r4, r0 | ld r0, [a042]
min r1, r1, r6 | ld r6, [c051] | ld r4, [a172]
mad r5, r7, r6 | addxy r0, r2, r0 | ld r6, [c052] | st r1, [034e]
ph_wrap r2, r3, r0 | ld r0, [a17a]
mad r5, r1, r6 | mul r2, r2, #256.0 | ld r6, [c053] | st r2, [0352] | ld r1, [0357]
mad r5, r0, r6 | ph_clamp r2, r3, -r2 | ld r6, [c054] | ld r0, [0348]
mad r5, r4, r6 | ld r6, [c055]
mad r5, r2, r6 | mov r6, #1.0
mov r2, r0 | st r5, [0354]
mad r2, r1, #0.5
mul r3, r2, #2.0
ph_wrap r4, r3, r3
addxy r7, r4, #1.0 | st r4, [0356]
ph_tri r1, r3, -r7
mul r2, r1, r1
addxy r3, r2, #-8.8404402339953698
mul r4, r2, r3 | ld r3, [0354]
addxy r7, r4, #21.601100021326904
addxy r4, r3, #-0.5 | mul r2, r1, r7
mul r1, r4, #0.17328679513998632 | mov r7, #0.041666666666666664
ymacd r7, r1, #0.16666666666666666
ymacd r7, r1, #0.5
ymacd r7, r1, #1.0
ymacd r7, r1, #1.0
mul r3, r7, r7
mul r4, r3, r3
mul r1, r4, r4
mul r7, r1, r1 | ld r1, [0359]
mul r3, r0, r7
min r4, r3, #0.25
mad r4, r1, #0.25
mul r0, r4, #4.0
ph_wrap r7, r3, r0
addxy r3, r7, #1.0 | st r7, [0358]
ph_tri r1, r3, -r3
mul r4, r1, r1
addxy r0, r4, #-8.8404402339953698
mul r7, r4, r0
addxy r3, r7, #21.601100021326904
mul r4, r1, r3 | ld r1, [c070]
mul r0, r2, r4 | ld r4, [034c]
mul r7, r0, #0.00390625 | ld r0, [034a] | st r4, [a1a2] | ld r4, [c040]
mul r3, r1, r7 | st r0, [a1aa] | ld r1, [0346]
mul r2, r3, #1.3530342883660893 | ld r0, [a03a]
st r0, [a162] | ld r0, [035b]
st r1, [a1b2]
ld r3, [a063]
subxy r0, r0, r3 | ld r1, [a062] | st r3, [035a]
mul r0, r0, r0 | st r1, [a16a] | ld r1, [035d]
mad r1, r1, -r0 | st r2, [a153] | ld r2, [0378]
addxy r7, r1, r4
ph_wrap r1, r3, r7 | ld r3, [035f]
subxy r7, r1, r7 | subxy r4, r3, r2 | ld r2, [0361] | st r1, [035c]
mul r4, r4, r7
mad r3, r4, #0.5 | ld r4, [c041]
subxy r7, r2, r3 | st r3, [035e]
mad r2, r7, r4 | ld r4, [c047]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c046] | st r2, [0360]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c044]
mad r7, r1, r4 | ld r4, [c045]
mad r7, r3, r4 | ld r4, [c043]
ph_tri r3, r3, -r1 | ld r1, [036d]
mul r2, r3, r3 | mad r7, r3, r4 | ld r4, [c042]
mul r2, r2, #-0.5
add r2, #1.5
mul r2, r2, r3 | ld r3, [a03b]
mad r7, r2, r4 | ld r2, [0363] | ld r4, [c04a]
addxy r2, r2, r4 | ld r4, [c04d]
mul r2, r2, r3 | ld r3, [c04e]
min r2, r2, #1.0
mul r7, r7, r2 | st r2, [0362]
mul r5, r7, r4 | ld r4, [c04c]
mad r5, r2, -r3 | ld r3, [a033] | ld r2, [036f]
add r5, #1.0 | mad r2, r2, -r0
mul r5, r7, r4 | st r5, [036a] | ld r4, [c04b]
mul r4, r7, r4 | mov r5, #0.041666666666666664 | st r5, [0368]
addxy r3, r3, r4
mul r3, r4, #0.34657359027997264 | addxy r4, r1, #-0.999 | st r3, [0364]
ymacd r5, r3, #0.16666666666666666 | mul r4, r4, #256.0
ymacd r5, r3, #0.5 | mul r4, r4, #256.0
ymacd r5, r3, #1.0
ymacd r5, r3, #1.0 | ld r3, [a043]
mul r5, r5, r5
mul r3, r3, r5 | mov r5, #0.0
max r4, r4, r5 | st r3, [0366] | ld r5, [c050]
min r4, r4, r6
max r2, r2, r4 | ld r4, [c049]
subxy r3, r2, #1.0 | st r2, [036e]
mul r0, r3, r4 | ld r4, [c048]
mad r0, r2, r4 | mul r4, r3, #1.25 | ld r2, [0371]
subxy r4, r1, r4
mad r1, r4, r0 | ld r0, [a043]
min r1, r1, r6 | ld r6, [c051] | ld r4, [a173]
mad r5, r7, r6 | addxy r0, r2, r0 | ld r6, [c052] | st r1, [036c]
ph_wrap r2, r3, r0 | ld r0, [a17b]
mad r5, r1, r6 | mul r2, r2, #256.0 | ld r6, [c053] | st r2, [0370] | ld r1, [0375]
mad r5, r0, r6 | ph_clamp r2, r3, -r2 | ld r6, [c054] | ld r0, [0366]
mad r5, r4, r6 | ld r6, [c055]
mad r5, r2, r6
mov r2, r0 | st r5, [0372]
mad r2, r1, #0.5
mul r3, r2, #2.0
ph_wrap r4, r3, r3
addxy r7, r4, #1.0 | st r4, [0374]
ph_tri r1, r3, -r7
mul r2, r1, r1
addxy r3, r2, #-8.8404402339953698
mul r4, r2, r3 | ld r3, [0372]
addxy r7, r4, #21.601100021326904
addxy r4, r3, #-0.5 | mul r2, r1, r7
mul r1, r4, #0.17328679513998632 | mov r7, #0.041666666666666664
ymacd r7, r1, #0.16666666666666666
ymacd r7, r1, #0.5
ymacd r7, r1, #1.0
ymacd r7, r1, #1.0
mul r3, r7, r7
mul r4, r3, r3
mul r1, r4, r4
mul r7, r1, r1 | ld r1, [0377]
mul r3, r0, r7
min r4, r3, #0.25
mad r4, r1, #0.25
mul r0, r4, #4.0
ph_wrap r7, r3, r0
addxy r3, r7, #1.0 | st r7, [0376]
ph_tri r1, r3, -r3
mul r4, r1, r1
addxy r0, r4, #-8.8404402339953698
mul r7, r4, r0
addxy r3, r7, #21.601100021326904
mul r4, r1, r3 | ld r1, [c070]
mul r0, r2, r4 | ld r4, [036a]
mul r7, r0, #0.00390625 | ld r0, [0368] | st r4, [a1a3]
mul r3, r1, r7 | ld r1, [0364] | st r0, [a1ab]
mul r2, r3, #1.3530342883660893 | st r1, [a1b3]
ld r0, [a03b]
ld r1, [a063]
st r2, [a154]
st r0, [a163]
st r1, [a16b]
