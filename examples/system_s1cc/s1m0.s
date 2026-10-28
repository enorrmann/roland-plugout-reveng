mov r5, #0.0 | mov r6, #1.0 | ld r0, [03b3] | ld r2, [03b1] | ld r4, [c040]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | ld r1, [0309]
ph_wrap r0, r3, r0
addxy r2, r2, r0 | st r0, [03b2] | ld r0, [0305]
ph_wrap r2, r3, r2 | ld r3, [a060]
subxy r0, r0, r3 | st r2, [03b0]
mul r0, r0, r0 | st r3, [0304] | ld r2, [03b0]
mad r1, r1, -r0
addxy r7, r1, r4
ph_wrap r1, r3, r7 | ld r3, [030b]
subxy r7, r1, r7 | subxy r4, r3, r2 | ld r2, [030d] | st r1, [0308]
mul r4, r4, r7
mad r3, r4, #0.5 | ld r4, [c041]
subxy r7, r2, r3 | st r3, [030a]
mad r2, r7, r4 | ld r4, [c047]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c046] | st r2, [030c]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c044]
mad r7, r1, r4 | ld r4, [c045]
mad r7, r3, r4 | ld r4, [c043]
ph_tri r3, r3, -r1 | ld r1, [0301]
mul r2, r3, r3 | mad r7, r3, r4 | ld r4, [c042]
mul r2, r2, #-0.5
add r2, #1.5
mul r2, r2, r3
mad r7, r2, r4 | ld r2, [0303]
addxy r4, r1, #-0.999
mul r4, r4, #256.0 | mad r2, r2, -r0
mul r4, r4, #256.0
max r4, r4, r5 | ld r5, [c050]
min r4, r4, r6
max r2, r2, r4 | ld r4, [c049]
subxy r3, r2, #1.0 | st r2, [0302]
mul r0, r3, r4 | ld r4, [c048]
mad r0, r2, r4 | mul r4, r3, #1.25 | ld r2, [0307]
subxy r4, r1, r4 | add r7, #0.5
mad r1, r4, r0 | ld r0, [a040]
min r1, r1, r6 | ld r6, [c051] | ld r4, [a170]
mad r5, r7, r6 | ld r6, [c052] | st r1, [0300]
addxy r0, r2, r0
ph_wrap r2, r3, r0 | ld r0, [a178]
mad r5, r1, r6 | mul r2, r2, #256.0 | ld r6, [c053] | st r2, [0306]
mad r5, r0, r6 | ph_clamp r2, r3, -r2 | ld r6, [c054]
mad r5, r4, r6 | ld r6, [c055]
mad r5, r2, r6 | ld r6, [c059]
st r5, [030e] | ld r5, [c058]
mad r5, r7, r6 | ld r6, [c05a] | ld r7, [0313]
mad r5, r1, r6 | ld r6, [c05b] | ld r1, [a048]
mad r5, r0, r6 | ld r6, [c05c] | ld r0, [a040]
mad r5, r4, r6 | ld r6, [c05d]
addxy r3, r0, #-0.004999999888241291
mul r4, r3, #5.479451911627053 | mad r5, r2, r6 | ld r2, [030e]
mov r3, r0 | st r5, [0310]
mad r3, r7, #0.5 | ld r6, [0310]
max r5, r2, r4 | st r6, [0314]
mul r2, r3, #2.0
ph_wrap r4, r3, r2
addxy r7, r4, #1.0 | st r4, [0312]
ph_wrap r3, r3, r7
mul r2, r0, #2.0
min r7, r3, r2
mul r0, r0, #-2.0
max r7, r7, r0
mul r7, r1, r7
mul r7, r7, #256.0
mul r7, r7, #32.0
max r6, r7, -r7
addxy r6, r6, #-2.0
mul r7, r7, r6
addxy r6, r7, r3
mov r7, r5
mad r7, r4, #-0.68493148895338163
addxy r3, r7, #-0.65753423000762279
mul r4, r3, #-1.4600000381469727
ph_wrap r7, r3, r4
min r3, r7, r2
max r4, r3, r0
mul r2, r1, r4
mul r3, r2, #256.0
mul r0, r3, #32.0
max r1, r0, -r0
addxy r4, r1, #-2.0
mul r2, r0, r4 | ld r4, [0314]
addxy r3, r2, r7
mov r0, r6
subxy r1, r3, r6
mad r0, r4, #-0.5
mad r5, r1, #-0.68493148895338163 | ld r1, [c070]
addxy r2, r0, #1.25 | ld r0, [a030]
addxy r7, r5, #0.027397258945758795 | st r0, [a1a0]
mul r3, r2, r7 | mov r5, #0.0 | ld r0, [a038] | ld r2, [03b0]
mad r6, r3, #-0.73000001907348633 | st r0, [a160] | ld r0, [031b]
mul r4, r1, r6 | mov r6, #1.0 | ld r3, [a061]
subxy r0, r0, r3 | ld r1, [a060] | st r3, [031a]
mul r0, r0, r0 | st r1, [a168] | ld r1, [031f]
mad r1, r1, -r0 | st r4, [a151] | ld r4, [c040]
addxy r7, r1, r4
ph_wrap r1, r3, r7 | ld r3, [0321]
subxy r7, r1, r7 | subxy r4, r3, r2 | ld r2, [0323] | st r1, [031e]
mul r4, r4, r7
mad r3, r4, #0.5 | ld r4, [c041]
subxy r7, r2, r3 | st r3, [0320]
mad r2, r7, r4 | ld r4, [c047]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c046] | st r2, [0322]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c044]
mad r7, r1, r4 | ld r4, [c045]
mad r7, r3, r4 | ld r4, [c043]
ph_tri r3, r3, -r1 | ld r1, [0317]
mul r2, r3, r3 | mad r7, r3, r4 | ld r4, [c042]
mul r2, r2, #-0.5
add r2, #1.5
mul r2, r2, r3
mad r7, r2, r4 | ld r2, [0319]
addxy r4, r1, #-0.999
mul r4, r4, #256.0 | mad r2, r2, -r0
mul r4, r4, #256.0
max r4, r4, r5 | ld r5, [c050]
min r4, r4, r6
max r2, r2, r4 | ld r4, [c049]
subxy r3, r2, #1.0 | st r2, [0318]
mul r0, r3, r4 | ld r4, [c048]
mad r0, r2, r4 | mul r4, r3, #1.25 | ld r2, [031d]
subxy r4, r1, r4 | add r7, #0.5
mad r1, r4, r0 | ld r0, [a041]
min r1, r1, r6 | ld r6, [c051] | ld r4, [a171]
mad r5, r7, r6 | ld r6, [c052] | st r1, [0316]
addxy r0, r2, r0
ph_wrap r2, r3, r0 | ld r0, [a179]
mad r5, r1, r6 | mul r2, r2, #256.0 | ld r6, [c053] | st r2, [031c]
mad r5, r0, r6 | ph_clamp r2, r3, -r2 | ld r6, [c054]
mad r5, r4, r6 | ld r6, [c055]
mad r5, r2, r6 | ld r6, [c059]
st r5, [0324] | ld r5, [c058]
mad r5, r7, r6 | ld r6, [c05a] | ld r7, [0329]
mad r5, r1, r6 | ld r6, [c05b] | ld r1, [a049]
mad r5, r0, r6 | ld r6, [c05c] | ld r0, [a041]
mad r5, r4, r6 | ld r6, [c05d]
addxy r3, r0, #-0.004999999888241291
mul r4, r3, #5.479451911627053 | mad r5, r2, r6 | ld r2, [0324]
mov r3, r0 | st r5, [0326]
mad r3, r7, #0.5 | ld r6, [0326]
max r5, r2, r4 | st r6, [032a]
mul r2, r3, #2.0
ph_wrap r4, r3, r2
addxy r7, r4, #1.0 | st r4, [0328]
ph_wrap r3, r3, r7
mul r2, r0, #2.0
min r7, r3, r2
mul r0, r0, #-2.0
max r7, r7, r0
mul r7, r1, r7
mul r7, r7, #256.0
mul r7, r7, #32.0
max r6, r7, -r7
addxy r6, r6, #-2.0
mul r7, r7, r6
addxy r6, r7, r3
mov r7, r5
mad r7, r4, #-0.68493148895338163
addxy r3, r7, #-0.65753423000762279
mul r4, r3, #-1.4600000381469727
ph_wrap r7, r3, r4
min r3, r7, r2
max r4, r3, r0
mul r2, r1, r4
mul r3, r2, #256.0
mul r0, r3, #32.0
max r1, r0, -r0
addxy r4, r1, #-2.0
mul r2, r0, r4 | ld r4, [032a]
addxy r3, r2, r7
mov r0, r6
subxy r1, r3, r6
mad r0, r4, #-0.5
mad r5, r1, #-0.68493148895338163 | ld r1, [c070]
addxy r2, r0, #1.25 | ld r0, [a031]
addxy r7, r5, #0.027397258945758795 | st r0, [a1a1]
mul r3, r2, r7 | mov r5, #0.0 | ld r0, [a039] | ld r2, [03b0]
mad r6, r3, #-0.73000001907348633 | st r0, [a161] | ld r0, [0331]
mul r4, r1, r6 | mov r6, #1.0 | ld r3, [a062]
subxy r0, r0, r3 | ld r1, [a061] | st r3, [0330]
mul r0, r0, r0 | st r1, [a169] | ld r1, [0335]
mad r1, r1, -r0 | st r4, [a152] | ld r4, [c040]
addxy r7, r1, r4
ph_wrap r1, r3, r7 | ld r3, [0337]
subxy r7, r1, r7 | subxy r4, r3, r2 | ld r2, [0339] | st r1, [0334]
mul r4, r4, r7
mad r3, r4, #0.5 | ld r4, [c041]
subxy r7, r2, r3 | st r3, [0336]
mad r2, r7, r4 | ld r4, [c047]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c046] | st r2, [0338]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c044]
mad r7, r1, r4 | ld r4, [c045]
mad r7, r3, r4 | ld r4, [c043]
ph_tri r3, r3, -r1 | ld r1, [032d]
mul r2, r3, r3 | mad r7, r3, r4 | ld r4, [c042]
mul r2, r2, #-0.5
add r2, #1.5
mul r2, r2, r3
mad r7, r2, r4 | ld r2, [032f]
addxy r4, r1, #-0.999
mul r4, r4, #256.0 | mad r2, r2, -r0
mul r4, r4, #256.0
max r4, r4, r5 | ld r5, [c050]
min r4, r4, r6
max r2, r2, r4 | ld r4, [c049]
subxy r3, r2, #1.0 | st r2, [032e]
mul r0, r3, r4 | ld r4, [c048]
mad r0, r2, r4 | mul r4, r3, #1.25 | ld r2, [0333]
subxy r4, r1, r4 | add r7, #0.5
mad r1, r4, r0 | ld r0, [a042]
min r1, r1, r6 | ld r6, [c051] | ld r4, [a172]
mad r5, r7, r6 | ld r6, [c052] | st r1, [032c]
addxy r0, r2, r0
ph_wrap r2, r3, r0 | ld r0, [a17a]
mad r5, r1, r6 | mul r2, r2, #256.0 | ld r6, [c053] | st r2, [0332]
mad r5, r0, r6 | ph_clamp r2, r3, -r2 | ld r6, [c054]
mad r5, r4, r6 | ld r6, [c055]
mad r5, r2, r6 | ld r6, [c059]
st r5, [033a] | ld r5, [c058]
mad r5, r7, r6 | ld r6, [c05a] | ld r7, [033f]
mad r5, r1, r6 | ld r6, [c05b] | ld r1, [a04a]
mad r5, r0, r6 | ld r6, [c05c] | ld r0, [a042]
mad r5, r4, r6 | ld r6, [c05d]
addxy r3, r0, #-0.004999999888241291
mul r4, r3, #5.479451911627053 | mad r5, r2, r6 | ld r2, [033a]
mov r3, r0 | st r5, [033c]
mad r3, r7, #0.5 | ld r6, [033c]
max r5, r2, r4 | st r6, [0340]
mul r2, r3, #2.0
ph_wrap r4, r3, r2
addxy r7, r4, #1.0 | st r4, [033e]
ph_wrap r3, r3, r7
mul r2, r0, #2.0
min r7, r3, r2
mul r0, r0, #-2.0
max r7, r7, r0
mul r7, r1, r7
mul r7, r7, #256.0
mul r7, r7, #32.0
max r6, r7, -r7
addxy r6, r6, #-2.0
mul r7, r7, r6
addxy r6, r7, r3
mov r7, r5
mad r7, r4, #-0.68493148895338163
addxy r3, r7, #-0.65753423000762279
mul r4, r3, #-1.4600000381469727
ph_wrap r7, r3, r4
min r3, r7, r2
max r4, r3, r0
mul r2, r1, r4
mul r3, r2, #256.0
mul r0, r3, #32.0
max r1, r0, -r0
addxy r4, r1, #-2.0
mul r2, r0, r4 | ld r4, [0340]
addxy r3, r2, r7
mov r0, r6
subxy r1, r3, r6
mad r0, r4, #-0.5
mad r5, r1, #-0.68493148895338163 | ld r1, [c070]
addxy r2, r0, #1.25 | ld r0, [a032]
addxy r7, r5, #0.027397258945758795 | st r0, [a1a2]
mul r3, r2, r7 | mov r5, #0.0 | ld r0, [a03a] | ld r2, [03b0]
mad r6, r3, #-0.73000001907348633 | st r0, [a162] | ld r0, [0347]
mul r4, r1, r6 | mov r6, #1.0 | ld r3, [a063]
subxy r0, r0, r3 | ld r1, [a062] | st r3, [0346]
mul r0, r0, r0 | st r1, [a16a] | ld r1, [034b]
mad r1, r1, -r0 | st r4, [a153] | ld r4, [c040]
addxy r7, r1, r4
ph_wrap r1, r3, r7 | ld r3, [034d]
subxy r7, r1, r7 | subxy r4, r3, r2 | ld r2, [034f] | st r1, [034a]
mul r4, r4, r7
mad r3, r4, #0.5 | ld r4, [c041]
subxy r7, r2, r3 | st r3, [034c]
mad r2, r7, r4 | ld r4, [c047]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c046] | st r2, [034e]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c044]
mad r7, r1, r4 | ld r4, [c045]
mad r7, r3, r4 | ld r4, [c043]
ph_tri r3, r3, -r1 | ld r1, [0343]
mul r2, r3, r3 | mad r7, r3, r4 | ld r4, [c042]
mul r2, r2, #-0.5
add r2, #1.5
mul r2, r2, r3
mad r7, r2, r4 | ld r2, [0345]
addxy r4, r1, #-0.999
mul r4, r4, #256.0 | mad r2, r2, -r0
mul r4, r4, #256.0
max r4, r4, r5 | ld r5, [c050]
min r4, r4, r6
max r2, r2, r4 | ld r4, [c049]
subxy r3, r2, #1.0 | st r2, [0344]
mul r0, r3, r4 | ld r4, [c048]
mad r0, r2, r4 | mul r4, r3, #1.25 | ld r2, [0349]
subxy r4, r1, r4 | add r7, #0.5
mad r1, r4, r0 | ld r0, [a043]
min r1, r1, r6 | ld r6, [c051] | ld r4, [a173]
mad r5, r7, r6 | ld r6, [c052] | st r1, [0342]
addxy r0, r2, r0
ph_wrap r2, r3, r0 | ld r0, [a17b]
mad r5, r1, r6 | mul r2, r2, #256.0 | ld r6, [c053] | st r2, [0348]
mad r5, r0, r6 | ph_clamp r2, r3, -r2 | ld r6, [c054]
mad r5, r4, r6 | ld r6, [c055]
mad r5, r2, r6 | ld r6, [c059]
st r5, [0350] | ld r5, [c058]
mad r5, r7, r6 | ld r6, [c05a] | ld r7, [0355]
mad r5, r1, r6 | ld r6, [c05b] | ld r1, [a04b]
mad r5, r0, r6 | ld r6, [c05c] | ld r0, [a043]
mad r5, r4, r6 | ld r6, [c05d]
addxy r3, r0, #-0.004999999888241291
mul r4, r3, #5.479451911627053 | mad r5, r2, r6 | ld r2, [0350]
mov r3, r0 | st r5, [0352]
mad r3, r7, #0.5 | ld r6, [0352]
max r5, r2, r4 | st r6, [0356]
mul r2, r3, #2.0
ph_wrap r4, r3, r2
addxy r7, r4, #1.0 | st r4, [0354]
ph_wrap r3, r3, r7
mul r2, r0, #2.0
min r7, r3, r2
mul r0, r0, #-2.0
max r7, r7, r0
mul r7, r1, r7
mul r7, r7, #256.0
mul r7, r7, #32.0
max r6, r7, -r7
addxy r6, r6, #-2.0
mul r7, r7, r6
addxy r6, r7, r3
mov r7, r5
mad r7, r4, #-0.68493148895338163
addxy r3, r7, #-0.65753423000762279
mul r4, r3, #-1.4600000381469727
ph_wrap r7, r3, r4
min r3, r7, r2
max r4, r3, r0
mul r2, r1, r4
mul r3, r2, #256.0
mul r0, r3, #32.0
max r1, r0, -r0
addxy r4, r1, #-2.0
mul r2, r0, r4 | ld r4, [0356]
addxy r3, r2, r7
mov r0, r6
subxy r1, r3, r6
mad r0, r4, #-0.5
mad r5, r1, #-0.68493148895338163 | ld r1, [c070]
addxy r2, r0, #1.25 | ld r0, [a033]
addxy r7, r5, #0.027397258945758795 | st r0, [a1a3]
mul r3, r2, r7 | mov r5, #0.0 | ld r0, [a03b] | ld r2, [03b0]
mad r6, r3, #-0.73000001907348633 | st r0, [a163] | ld r0, [035d]
mul r4, r1, r6 | mov r6, #1.0 | ld r3, [a064]
subxy r0, r0, r3 | ld r1, [a063] | st r3, [035c]
mul r0, r0, r0 | st r1, [a16b] | ld r1, [0361]
mad r1, r1, -r0 | st r4, [a154] | ld r4, [c040]
addxy r7, r1, r4
ph_wrap r1, r3, r7 | ld r3, [0363]
subxy r7, r1, r7 | subxy r4, r3, r2 | ld r2, [0365] | st r1, [0360]
mul r4, r4, r7
mad r3, r4, #0.5 | ld r4, [c041]
subxy r7, r2, r3 | st r3, [0362]
mad r2, r7, r4 | ld r4, [c047]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c046] | st r2, [0364]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c044]
mad r7, r1, r4 | ld r4, [c045]
mad r7, r3, r4 | ld r4, [c043]
ph_tri r3, r3, -r1 | ld r1, [0359]
mul r2, r3, r3 | mad r7, r3, r4 | ld r4, [c042]
mul r2, r2, #-0.5
add r2, #1.5
mul r2, r2, r3
mad r7, r2, r4 | ld r2, [035b]
addxy r4, r1, #-0.999
mul r4, r4, #256.0 | mad r2, r2, -r0
mul r4, r4, #256.0
max r4, r4, r5 | ld r5, [c050]
min r4, r4, r6
max r2, r2, r4 | ld r4, [c049]
subxy r3, r2, #1.0 | st r2, [035a]
mul r0, r3, r4 | ld r4, [c048]
mad r0, r2, r4 | mul r4, r3, #1.25 | ld r2, [035f]
subxy r4, r1, r4 | add r7, #0.5
mad r1, r4, r0 | ld r0, [a044]
min r1, r1, r6 | ld r6, [c051] | ld r4, [a174]
mad r5, r7, r6 | ld r6, [c052] | st r1, [0358]
addxy r0, r2, r0
ph_wrap r2, r3, r0 | ld r0, [a17c]
mad r5, r1, r6 | mul r2, r2, #256.0 | ld r6, [c053] | st r2, [035e]
mad r5, r0, r6 | ph_clamp r2, r3, -r2 | ld r6, [c054]
mad r5, r4, r6 | ld r6, [c055]
mad r5, r2, r6 | ld r6, [c059]
st r5, [0366] | ld r5, [c058]
mad r5, r7, r6 | ld r6, [c05a] | ld r7, [036b]
mad r5, r1, r6 | ld r6, [c05b] | ld r1, [a04c]
mad r5, r0, r6 | ld r6, [c05c] | ld r0, [a044]
mad r5, r4, r6 | ld r6, [c05d]
addxy r3, r0, #-0.004999999888241291
mul r4, r3, #5.479451911627053 | mad r5, r2, r6 | ld r2, [0366]
mov r3, r0 | st r5, [0368]
mad r3, r7, #0.5 | ld r6, [0368]
max r5, r2, r4 | st r6, [036c]
mul r2, r3, #2.0
ph_wrap r4, r3, r2
addxy r7, r4, #1.0 | st r4, [036a]
ph_wrap r3, r3, r7
mul r2, r0, #2.0
min r7, r3, r2
mul r0, r0, #-2.0
max r7, r7, r0
mul r7, r1, r7
mul r7, r7, #256.0
mul r7, r7, #32.0
max r6, r7, -r7
addxy r6, r6, #-2.0
mul r7, r7, r6
addxy r6, r7, r3
mov r7, r5
mad r7, r4, #-0.68493148895338163
addxy r3, r7, #-0.65753423000762279
mul r4, r3, #-1.4600000381469727
ph_wrap r7, r3, r4
min r3, r7, r2
max r4, r3, r0
mul r2, r1, r4
mul r3, r2, #256.0
mul r0, r3, #32.0
max r1, r0, -r0
addxy r4, r1, #-2.0
mul r2, r0, r4 | ld r4, [036c]
addxy r3, r2, r7
mov r0, r6
subxy r1, r3, r6
mad r0, r4, #-0.5
mad r5, r1, #-0.68493148895338163 | ld r1, [c070]
addxy r2, r0, #1.25 | ld r0, [a034]
addxy r7, r5, #0.027397258945758795 | st r0, [a1a4]
mul r3, r2, r7 | mov r5, #0.0 | ld r0, [a03c] | ld r2, [03b0]
mad r6, r3, #-0.73000001907348633 | st r0, [a164] | ld r0, [0373]
mul r4, r1, r6 | mov r6, #1.0 | ld r3, [a065]
subxy r0, r0, r3 | ld r1, [a064] | st r3, [0372]
mul r0, r0, r0 | st r1, [a16c] | ld r1, [0377]
mad r1, r1, -r0 | st r4, [a155] | ld r4, [c040]
addxy r7, r1, r4
ph_wrap r1, r3, r7 | ld r3, [0379]
subxy r7, r1, r7 | subxy r4, r3, r2 | ld r2, [037b] | st r1, [0376]
mul r4, r4, r7
mad r3, r4, #0.5 | ld r4, [c041]
subxy r7, r2, r3 | st r3, [0378]
mad r2, r7, r4 | ld r4, [c047]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c046] | st r2, [037a]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c044]
mad r7, r1, r4 | ld r4, [c045]
mad r7, r3, r4 | ld r4, [c043]
ph_tri r3, r3, -r1 | ld r1, [036f]
mul r2, r3, r3 | mad r7, r3, r4 | ld r4, [c042]
mul r2, r2, #-0.5
add r2, #1.5
mul r2, r2, r3
mad r7, r2, r4 | ld r2, [0371]
addxy r4, r1, #-0.999
mul r4, r4, #256.0 | mad r2, r2, -r0
mul r4, r4, #256.0
max r4, r4, r5 | ld r5, [c050]
min r4, r4, r6
max r2, r2, r4 | ld r4, [c049]
subxy r3, r2, #1.0 | st r2, [0370]
mul r0, r3, r4 | ld r4, [c048]
mad r0, r2, r4 | mul r4, r3, #1.25 | ld r2, [0375]
subxy r4, r1, r4 | add r7, #0.5
mad r1, r4, r0 | ld r0, [a045]
min r1, r1, r6 | ld r6, [c051] | ld r4, [a175]
mad r5, r7, r6 | ld r6, [c052] | st r1, [036e]
addxy r0, r2, r0
ph_wrap r2, r3, r0 | ld r0, [a17d]
mad r5, r1, r6 | mul r2, r2, #256.0 | ld r6, [c053] | st r2, [0374]
mad r5, r0, r6 | ph_clamp r2, r3, -r2 | ld r6, [c054]
mad r5, r4, r6 | ld r6, [c055]
mad r5, r2, r6 | ld r6, [c059]
st r5, [037c] | ld r5, [c058]
mad r5, r7, r6 | ld r6, [c05a] | ld r7, [0381]
mad r5, r1, r6 | ld r6, [c05b] | ld r1, [a04d]
mad r5, r0, r6 | ld r6, [c05c] | ld r0, [a045]
mad r5, r4, r6 | ld r6, [c05d]
addxy r3, r0, #-0.004999999888241291
mul r4, r3, #5.479451911627053 | mad r5, r2, r6 | ld r2, [037c]
mov r3, r0 | st r5, [037e]
mad r3, r7, #0.5 | ld r6, [037e]
max r5, r2, r4 | st r6, [0382]
mul r2, r3, #2.0
ph_wrap r4, r3, r2
addxy r7, r4, #1.0 | st r4, [0380]
ph_wrap r3, r3, r7
mul r2, r0, #2.0
min r7, r3, r2
mul r0, r0, #-2.0
max r7, r7, r0
mul r7, r1, r7
mul r7, r7, #256.0
mul r7, r7, #32.0
max r6, r7, -r7
addxy r6, r6, #-2.0
mul r7, r7, r6
addxy r6, r7, r3
mov r7, r5
mad r7, r4, #-0.68493148895338163
addxy r3, r7, #-0.65753423000762279
mul r4, r3, #-1.4600000381469727
ph_wrap r7, r3, r4
min r3, r7, r2
max r4, r3, r0
mul r2, r1, r4
mul r3, r2, #256.0
mul r0, r3, #32.0
max r1, r0, -r0
addxy r4, r1, #-2.0
mul r2, r0, r4 | ld r4, [0382]
addxy r3, r2, r7
mov r0, r6
subxy r1, r3, r6
mad r0, r4, #-0.5
mad r5, r1, #-0.68493148895338163 | ld r1, [c070]
addxy r2, r0, #1.25 | ld r0, [a035]
addxy r7, r5, #0.027397258945758795 | st r0, [a1a5]
mul r3, r2, r7 | mov r5, #0.0 | ld r0, [a03d] | ld r2, [03b0]
mad r6, r3, #-0.73000001907348633 | st r0, [a165] | ld r0, [0389]
mul r4, r1, r6 | mov r6, #1.0 | ld r3, [a066]
subxy r0, r0, r3 | ld r1, [a065] | st r3, [0388]
mul r0, r0, r0 | st r1, [a16d] | ld r1, [038d]
mad r1, r1, -r0 | st r4, [a156] | ld r4, [c040]
addxy r7, r1, r4
ph_wrap r1, r3, r7 | ld r3, [038f]
subxy r7, r1, r7 | subxy r4, r3, r2 | ld r2, [0391] | st r1, [038c]
mul r4, r4, r7
mad r3, r4, #0.5 | ld r4, [c041]
subxy r7, r2, r3 | st r3, [038e]
mad r2, r7, r4 | ld r4, [c047]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c046] | st r2, [0390]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c044]
mad r7, r1, r4 | ld r4, [c045]
mad r7, r3, r4 | ld r4, [c043]
ph_tri r3, r3, -r1 | ld r1, [0385]
mul r2, r3, r3 | mad r7, r3, r4 | ld r4, [c042]
mul r2, r2, #-0.5
add r2, #1.5
mul r2, r2, r3
mad r7, r2, r4 | ld r2, [0387]
addxy r4, r1, #-0.999
mul r4, r4, #256.0 | mad r2, r2, -r0
mul r4, r4, #256.0
max r4, r4, r5 | ld r5, [c050]
min r4, r4, r6
max r2, r2, r4 | ld r4, [c049]
subxy r3, r2, #1.0 | st r2, [0386]
mul r0, r3, r4 | ld r4, [c048]
mad r0, r2, r4 | mul r4, r3, #1.25 | ld r2, [038b]
subxy r4, r1, r4 | add r7, #0.5
mad r1, r4, r0 | ld r0, [a046]
min r1, r1, r6 | ld r6, [c051] | ld r4, [a176]
mad r5, r7, r6 | ld r6, [c052] | st r1, [0384]
addxy r0, r2, r0
ph_wrap r2, r3, r0 | ld r0, [a17e]
mad r5, r1, r6 | mul r2, r2, #256.0 | ld r6, [c053] | st r2, [038a]
mad r5, r0, r6 | ph_clamp r2, r3, -r2 | ld r6, [c054]
mad r5, r4, r6 | ld r6, [c055]
mad r5, r2, r6 | ld r6, [c059]
st r5, [0392] | ld r5, [c058]
mad r5, r7, r6 | ld r6, [c05a] | ld r7, [0397]
mad r5, r1, r6 | ld r6, [c05b] | ld r1, [a04e]
mad r5, r0, r6 | ld r6, [c05c] | ld r0, [a046]
mad r5, r4, r6 | ld r6, [c05d]
addxy r3, r0, #-0.004999999888241291
mul r4, r3, #5.479451911627053 | mad r5, r2, r6 | ld r2, [0392]
mov r3, r0 | st r5, [0394]
mad r3, r7, #0.5 | ld r6, [0394]
max r5, r2, r4 | st r6, [0398]
mul r2, r3, #2.0
ph_wrap r4, r3, r2
addxy r7, r4, #1.0 | st r4, [0396]
ph_wrap r3, r3, r7
mul r2, r0, #2.0
min r7, r3, r2
mul r0, r0, #-2.0
max r7, r7, r0
mul r7, r1, r7
mul r7, r7, #256.0
mul r7, r7, #32.0
max r6, r7, -r7
addxy r6, r6, #-2.0
mul r7, r7, r6
addxy r6, r7, r3
mov r7, r5
mad r7, r4, #-0.68493148895338163
addxy r3, r7, #-0.65753423000762279
mul r4, r3, #-1.4600000381469727
ph_wrap r7, r3, r4
min r3, r7, r2
max r4, r3, r0
mul r2, r1, r4
mul r3, r2, #256.0
mul r0, r3, #32.0
max r1, r0, -r0
addxy r4, r1, #-2.0
mul r2, r0, r4 | ld r4, [0398]
addxy r3, r2, r7
mov r0, r6
subxy r1, r3, r6
mad r0, r4, #-0.5
mad r5, r1, #-0.68493148895338163 | ld r1, [c070]
addxy r2, r0, #1.25 | ld r0, [a036]
addxy r7, r5, #0.027397258945758795 | st r0, [a1a6]
mul r3, r2, r7 | mov r5, #0.0 | ld r0, [a03e] | ld r2, [03b0]
mad r6, r3, #-0.73000001907348633 | st r0, [a166] | ld r0, [039f]
mul r4, r1, r6 | mov r6, #1.0 | ld r3, [a067]
subxy r0, r0, r3 | ld r1, [a066] | st r3, [039e]
mul r0, r0, r0 | st r1, [a16e] | ld r1, [03a3]
mad r1, r1, -r0 | st r4, [a157] | ld r4, [c040]
addxy r7, r1, r4
ph_wrap r1, r3, r7 | ld r3, [03a5]
subxy r7, r1, r7 | subxy r4, r3, r2 | ld r2, [03a7] | st r1, [03a2]
mul r4, r4, r7
mad r3, r4, #0.5 | ld r4, [c041]
subxy r7, r2, r3 | st r3, [03a4]
mad r2, r7, r4 | ld r4, [c047]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c046] | st r2, [03a6]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c044]
mad r7, r1, r4 | ld r4, [c045]
mad r7, r3, r4 | ld r4, [c043]
ph_tri r3, r3, -r1 | ld r1, [039b]
mul r2, r3, r3 | mad r7, r3, r4 | ld r4, [c042]
mul r2, r2, #-0.5
add r2, #1.5
mul r2, r2, r3
mad r7, r2, r4 | ld r2, [039d]
addxy r4, r1, #-0.999
mul r4, r4, #256.0 | mad r2, r2, -r0
mul r4, r4, #256.0
max r4, r4, r5 | ld r5, [c050]
min r4, r4, r6
max r2, r2, r4 | ld r4, [c049]
subxy r3, r2, #1.0 | st r2, [039c]
mul r0, r3, r4 | ld r4, [c048]
mad r0, r2, r4 | mul r4, r3, #1.25 | ld r2, [03a1]
subxy r4, r1, r4 | add r7, #0.5
mad r1, r4, r0 | ld r0, [a047]
min r1, r1, r6 | ld r6, [c051] | ld r4, [a177]
mad r5, r7, r6 | ld r6, [c052] | st r1, [039a]
addxy r0, r2, r0
ph_wrap r2, r3, r0 | ld r0, [a17f]
mad r5, r1, r6 | mul r2, r2, #256.0 | ld r6, [c053] | st r2, [03a0]
mad r5, r0, r6 | ph_clamp r2, r3, -r2 | ld r6, [c054]
mad r5, r4, r6 | ld r6, [c055]
mad r5, r2, r6 | ld r6, [c059]
st r5, [03a8] | ld r5, [c058]
mad r5, r7, r6 | ld r6, [c05a] | ld r7, [03ad]
mad r5, r1, r6 | ld r6, [c05b] | ld r1, [a04f]
mad r5, r0, r6 | ld r6, [c05c] | ld r0, [a047]
mad r5, r4, r6 | ld r6, [c05d]
addxy r3, r0, #-0.004999999888241291
mul r4, r3, #5.479451911627053 | mad r5, r2, r6 | ld r2, [03a8]
mov r3, r0 | st r5, [03aa]
mad r3, r7, #0.5 | ld r6, [03aa]
max r5, r2, r4 | st r6, [03ae]
mul r2, r3, #2.0
ph_wrap r4, r3, r2
addxy r7, r4, #1.0 | st r4, [03ac]
ph_wrap r3, r3, r7
mul r2, r0, #2.0
min r7, r3, r2
mul r0, r0, #-2.0
max r7, r7, r0
mul r7, r1, r7
mul r7, r7, #256.0
mul r7, r7, #32.0
max r6, r7, -r7
addxy r6, r6, #-2.0
mul r7, r7, r6
addxy r6, r7, r3
mov r7, r5
mad r7, r4, #-0.68493148895338163
addxy r3, r7, #-0.65753423000762279
mul r4, r3, #-1.4600000381469727
ph_wrap r7, r3, r4
min r3, r7, r2
max r4, r3, r0
mul r2, r1, r4
mul r3, r2, #256.0
mul r0, r3, #32.0
max r1, r0, -r0
addxy r4, r1, #-2.0
mul r2, r0, r4 | ld r4, [03ae]
addxy r3, r2, r7
subxy r1, r3, r6
mov r0, r6
mad r5, r1, #-0.68493148895338163 | ld r1, [c070]
mad r0, r4, #-0.5
addxy r2, r0, #1.25 | ld r0, [a037]
addxy r7, r5, #0.027397258945758795 | st r0, [a1a7]
mul r3, r2, r7 | ld r0, [a03f]
mad r6, r3, #-0.73000001907348633 | st r0, [a167]
mul r4, r1, r6 | ld r1, [a067]
st r4, [a158]
st r1, [a16f]
