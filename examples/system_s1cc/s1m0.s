ld r0, [03c3] | ld r2, [03c1] | ld r6, [a039]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | st r6, [a161]
ph_wrap r0, r3, r0 | ld r4, [a061] | ld r6, [c04b]
addxy r2, r2, r0 | st r0, [03c2] | ld r0, [a060] | ld r3, [c076]
ph_wrap r2, r3, r2 | ld r1, [a120] | ld r7, [c076]
st r2, [03c0] | ld r2, [a038]
st r2, [a160] | ld r2, [c04b]
mul r1, r1, r2 | ld r5, [a121] | ld r2, [c05e]
mul r5, r5, r6 | st r0, [a168] | ld r6, [c05e]
st r4, [a169]
ld r0, [a128]
mad r1, r0, r2 | ld r4, [a129] | ld r2, [c05f]
mad r5, r4, r6 | addxy r1, r1, r2 | ld r6, [c05f]
addxy r5, r5, r6 | mov r0, #0.041666666666666664 | ld r2, [a030]
addxy r2, r2, r1 | mov r4, #0.041666666666666664 | ld r6, [a031]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [a1b0]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [a1b1]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a040]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a041]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mul r2, r1, #-0.08664339756999316 | st r2, [0300] | ld r1, [0305]
mul r6, r5, #-0.08664339756999316 | mov r0, #0.16666666666666666 | st r6, [0318] | ld r5, [031d]
mov r4, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a048]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a049]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0 | ld r0, [0300]
mul r6, r6, r4 | st r2, [0302] | ld r2, [a040]
addxy r2, r1, r2 | st r6, [031a] | ld r6, [a041]
addxy r6, r5, r6 | ld r4, [0318]
ph_wrap r1, r3, r2 | ld r2, [c074]
ph_wrap r5, r3, r6 | st r1, [0304] | ld r6, [c074]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [031c]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a130] | ld r1, [0302]
mul r2, r2, r3 | mul r6, r6, r7 | st r5, [a131] | ld r5, [031a] | st r1, [030c]
ld r3, [c070] | st r5, [0324]
mul r2, r2, r3 | ld r7, [c070]
mul r6, r6, r7 | ld r3, [c075]
ld r7, [c075]
ld r1, [c077]
mul r3, r3, r1 | ld r5, [c077]
mul r7, r7, r5 | ld r1, [c071]
mul r3, r3, r1 | mul r1, r0, r2 | ld r5, [c071]
mul r7, r7, r5 | mul r5, r4, r6
min r2, r1, #0.00390625 | ld r1, [030c]
min r6, r5, #0.00390625 | ld r5, [0324] | st r2, [0310]
mul r3, r1, r3 | mul r7, r5, r7 | st r6, [0328]
addxy r2, r0, #-0.004999999888241291 | addxy r6, r4, #-0.004999999888241291 | st r3, [030e] | ld r3, [a138]
mul r2, r2, #5.479451911627053 | mul r6, r6, #5.479451911627053 | st r7, [0326] | ld r7, [a139]
max r3, r3, r2 | ld r2, [0309]
max r7, r7, r6 | st r3, [0312] | ld r6, [0321]
mov r3, r0 | mov r7, r4 | st r7, [032a]
mad r3, r2, #0.5 | mad r7, r6, #0.5
mul r2, r3, #2.0 | mul r6, r7, #2.0
ph_wrap r3, r3, r2
ph_wrap r7, r3, r6 | st r3, [0308]
addxy r2, r3, #1.0 | addxy r6, r7, #1.0 | st r7, [0320]
st r3, [0314] | st r7, [032c]
ph_wrap r2, r3, r2
ph_wrap r6, r3, r6
mul r3, r0, #2.0 | mul r7, r4, #2.0
min r1, r2, r3
min r5, r6, r7
mul r0, r0, #-2.0 | mul r4, r4, #-2.0
max r1, r1, r0 | st r0, [0316] | ld r0, [030c]
max r5, r5, r4 | st r4, [032e] | ld r4, [0324]
mul r1, r0, r1 | mul r5, r4, r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #32.0 | mul r5, r5, #32.0
max r0, r1, -r1
max r4, r5, -r5
addxy r0, r0, #-2.0 | addxy r4, r4, #-2.0
mul r1, r1, r0 | mul r5, r5, r4
addxy r0, r1, r2 | addxy r4, r5, r6 | ld r1, [0312] | ld r5, [032a]
mov r2, r1 | mov r6, r5 | ld r1, [0314] | ld r5, [032c]
mad r2, r1, #-0.68493148895338163 | mad r6, r5, #-0.68493148895338163
addxy r1, r2, #-0.65753423000762279 | addxy r5, r6, #-0.65753423000762279
mul r2, r1, #-1.4600000381469727 | mul r6, r5, #-1.4600000381469727
ph_wrap r1, r3, r2
ph_wrap r5, r3, r6
min r2, r1, r3 | ld r3, [0316]
min r6, r5, r7 | ld r7, [032e]
max r2, r2, r3 | ld r3, [030c]
max r6, r6, r7 | ld r7, [0324]
mul r3, r3, r2 | mul r7, r7, r6
mul r2, r3, #256.0 | mul r6, r7, #256.0
mul r3, r2, #32.0 | mul r7, r6, #32.0
max r2, r3, -r3
max r6, r7, -r7
addxy r2, r2, #-2.0 | addxy r6, r6, #-2.0
mul r3, r3, r2 | mul r7, r7, r6
addxy r2, r3, r1 | addxy r6, r7, r5 | ld r1, [0312] | ld r5, [032a]
subxy r3, r2, r0 | subxy r7, r6, r4
mad r1, r3, #-0.68493148895338163 | mad r5, r7, #-0.68493148895338163
addxy r2, r0, #1.0 | addxy r6, r4, #1.0
addxy r3, r1, #0.027397258945758795 | addxy r7, r5, #0.027397258945758795
mul r1, r2, r3 | mul r5, r6, r7 | ld r2, [c072]
mad r0, r1, #-0.73000001907348633 | mad r4, r5, #-0.73000001907348633 | ld r6, [c072]
mul r3, r2, r0 | mul r7, r6, r4 | ld r2, [0310] | ld r6, [0328] | ld r1, [a140]
addxy r0, r2, #-3.9062499126885086e-05 | addxy r4, r6, #-3.9062499126885086e-05 | ld r5, [a141] | st r3, [0312]
mul r0, r0, #256.0 | mul r4, r4, #256.0 | st r7, [032a]
mul r0, r0, #2.7397259558135265 | mul r4, r4, #2.7397259558135265
max r1, r1, r0 | ld r0, [030b]
max r5, r5, r4 | ld r4, [0323] | st r1, [0316]
mov r3, r2 | mov r7, r6 | st r5, [032e]
mad r3, r0, #0.00390625 | mad r7, r4, #0.00390625
mul r0, r3, #256.0 | mul r4, r7, #256.0
ph_wrap r3, r3, r0
ph_wrap r7, r3, r4 | st r3, [030a]
addxy r0, r3, #1.0 | addxy r4, r7, #1.0 | st r7, [0322]
st r3, [030c] | st r7, [0324]
ph_wrap r0, r3, r0
ph_wrap r4, r3, r4
mul r3, r2, #256.0 | mul r7, r6, #256.0
min r1, r0, r3
min r5, r4, r7
mul r2, r2, #-256.0 | mul r6, r6, #-256.0
max r1, r1, r2 | st r2, [0310] | ld r2, [030e]
max r5, r5, r6 | st r6, [0328] | ld r6, [0326]
mul r1, r2, r1 | mul r5, r6, r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #8.0 | mul r5, r5, #8.0
max r2, r1, -r1
max r6, r5, -r5
addxy r2, r2, #-1.0 | addxy r6, r6, #-1.0
mul r1, r1, r2 | mul r5, r5, r6 | ld r2, [0316] | ld r6, [032e]
mad r1, r0, #0.25 | mad r5, r4, #0.25
mov r0, r2 | mov r4, r6 | ld r2, [030c] | ld r6, [0324]
mad r0, r2, #-0.68493148895338163 | mad r4, r6, #-0.68493148895338163
addxy r2, r0, #-0.65753423000762279 | addxy r6, r4, #-0.65753423000762279
mul r0, r2, #-1.4600000381469727 | mul r4, r6, #-1.4600000381469727
ph_wrap r2, r3, r0
ph_wrap r6, r3, r4
min r0, r2, r3 | ld r3, [0310]
min r4, r6, r7 | ld r7, [0328]
max r0, r0, r3 | ld r3, [030e]
max r4, r4, r7 | ld r7, [0326]
mul r3, r3, r0 | mul r7, r7, r4
mul r0, r3, #256.0 | mul r4, r7, #256.0
mul r3, r0, #256.0 | mul r7, r4, #256.0
mul r0, r3, #8.0 | mul r4, r7, #8.0
max r3, r0, -r0
max r7, r4, -r4
addxy r3, r3, #-1.0 | addxy r7, r7, #-1.0
mul r0, r0, r3 | mul r4, r4, r7
mad r0, r2, #0.25 | mad r4, r6, #0.25 | ld r2, [0316] | ld r6, [032e]
subxy r3, r0, r1 | subxy r7, r4, r5
mad r3, r2, #-0.36500000953674316 | mad r7, r6, #-0.36500000953674316
addxy r0, r1, #0.25 | addxy r4, r5, #0.25
addxy r2, r3, #-0.0099999997764825821 | addxy r6, r7, #-0.0099999997764825821
mul r3, r0, r2 | mul r7, r4, r6 | ld r0, [c073]
mad r3, r1, #0.5 | mad r7, r5, #0.5 | ld r4, [c073] | ld r1, [0312] | ld r5, [032a]
mul r2, r3, r0 | mul r6, r7, r4
mad r2, r1, #0.125 | mad r6, r5, #0.125 | ld r1, [a130]
mul r3, r2, #4.0 | mul r7, r6, #4.0 | ld r5, [a131]
st r3, [0306] | st r7, [031e]
ld r0, [0306] | ld r4, [031e] | ld r2, [c056]
mad r0, r1, r2 | ld r6, [c056] | ld r1, [03c0]
mad r4, r5, r6 | ld r5, [03c0] | ld r2, [c057]
mad r0, r1, r2 | ld r6, [c057] | ld r2, [a03a]
mad r4, r5, r6 | ld r6, [a03b] | ld r3, [c076]
st r0, [a151] | ld r7, [c076]
st r4, [a152]
st r2, [a162] | ld r2, [c04b]
st r6, [a163] | ld r6, [c04b]
ld r0, [a062]
ld r4, [a063]
ld r1, [a122]
mul r1, r1, r2 | ld r5, [a123] | ld r2, [c05e]
mul r5, r5, r6 | st r0, [a16a] | ld r6, [c05e]
st r4, [a16b]
ld r0, [a12a]
mad r1, r0, r2 | ld r4, [a12b] | ld r2, [c05f]
mad r5, r4, r6 | addxy r1, r1, r2 | ld r6, [c05f]
addxy r5, r5, r6 | mov r0, #0.041666666666666664 | ld r2, [a032]
addxy r2, r2, r1 | mov r4, #0.041666666666666664 | ld r6, [a033]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [a1b2]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [a1b3]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a042]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a043]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mul r2, r1, #-0.08664339756999316 | st r2, [0330] | ld r1, [0335]
mul r6, r5, #-0.08664339756999316 | mov r0, #0.16666666666666666 | st r6, [0348] | ld r5, [034d]
mov r4, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a04a]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a04b]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0 | ld r0, [0330]
mul r6, r6, r4 | st r2, [0332] | ld r2, [a042]
addxy r2, r1, r2 | st r6, [034a] | ld r6, [a043]
addxy r6, r5, r6 | ld r4, [0348]
ph_wrap r1, r3, r2 | ld r2, [c074]
ph_wrap r5, r3, r6 | st r1, [0334] | ld r6, [c074]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [034c]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a132] | ld r1, [0332]
mul r2, r2, r3 | mul r6, r6, r7 | st r5, [a133] | ld r5, [034a] | st r1, [033c]
ld r3, [c070] | st r5, [0354]
mul r2, r2, r3 | ld r7, [c070]
mul r6, r6, r7 | ld r3, [c075]
ld r7, [c075]
ld r1, [c077]
mul r3, r3, r1 | ld r5, [c077]
mul r7, r7, r5 | ld r1, [c071]
mul r3, r3, r1 | mul r1, r0, r2 | ld r5, [c071]
mul r7, r7, r5 | mul r5, r4, r6
min r2, r1, #0.00390625 | ld r1, [033c]
min r6, r5, #0.00390625 | ld r5, [0354] | st r2, [0340]
mul r3, r1, r3 | mul r7, r5, r7 | st r6, [0358]
addxy r2, r0, #-0.004999999888241291 | addxy r6, r4, #-0.004999999888241291 | st r3, [033e] | ld r3, [a13a]
mul r2, r2, #5.479451911627053 | mul r6, r6, #5.479451911627053 | st r7, [0356] | ld r7, [a13b]
max r3, r3, r2 | ld r2, [0339]
max r7, r7, r6 | st r3, [0342] | ld r6, [0351]
mov r3, r0 | mov r7, r4 | st r7, [035a]
mad r3, r2, #0.5 | mad r7, r6, #0.5
mul r2, r3, #2.0 | mul r6, r7, #2.0
ph_wrap r3, r3, r2
ph_wrap r7, r3, r6 | st r3, [0338]
addxy r2, r3, #1.0 | addxy r6, r7, #1.0 | st r7, [0350]
st r3, [0344] | st r7, [035c]
ph_wrap r2, r3, r2
ph_wrap r6, r3, r6
mul r3, r0, #2.0 | mul r7, r4, #2.0
min r1, r2, r3
min r5, r6, r7
mul r0, r0, #-2.0 | mul r4, r4, #-2.0
max r1, r1, r0 | st r0, [0346] | ld r0, [033c]
max r5, r5, r4 | st r4, [035e] | ld r4, [0354]
mul r1, r0, r1 | mul r5, r4, r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #32.0 | mul r5, r5, #32.0
max r0, r1, -r1
max r4, r5, -r5
addxy r0, r0, #-2.0 | addxy r4, r4, #-2.0
mul r1, r1, r0 | mul r5, r5, r4
addxy r0, r1, r2 | addxy r4, r5, r6 | ld r1, [0342] | ld r5, [035a]
mov r2, r1 | mov r6, r5 | ld r1, [0344] | ld r5, [035c]
mad r2, r1, #-0.68493148895338163 | mad r6, r5, #-0.68493148895338163
addxy r1, r2, #-0.65753423000762279 | addxy r5, r6, #-0.65753423000762279
mul r2, r1, #-1.4600000381469727 | mul r6, r5, #-1.4600000381469727
ph_wrap r1, r3, r2
ph_wrap r5, r3, r6
min r2, r1, r3 | ld r3, [0346]
min r6, r5, r7 | ld r7, [035e]
max r2, r2, r3 | ld r3, [033c]
max r6, r6, r7 | ld r7, [0354]
mul r3, r3, r2 | mul r7, r7, r6
mul r2, r3, #256.0 | mul r6, r7, #256.0
mul r3, r2, #32.0 | mul r7, r6, #32.0
max r2, r3, -r3
max r6, r7, -r7
addxy r2, r2, #-2.0 | addxy r6, r6, #-2.0
mul r3, r3, r2 | mul r7, r7, r6
addxy r2, r3, r1 | addxy r6, r7, r5 | ld r1, [0342] | ld r5, [035a]
subxy r3, r2, r0 | subxy r7, r6, r4
mad r1, r3, #-0.68493148895338163 | mad r5, r7, #-0.68493148895338163
addxy r2, r0, #1.0 | addxy r6, r4, #1.0
addxy r3, r1, #0.027397258945758795 | addxy r7, r5, #0.027397258945758795
mul r1, r2, r3 | mul r5, r6, r7 | ld r2, [c072]
mad r0, r1, #-0.73000001907348633 | mad r4, r5, #-0.73000001907348633 | ld r6, [c072]
mul r3, r2, r0 | mul r7, r6, r4 | ld r2, [0340] | ld r6, [0358] | ld r1, [a142]
addxy r0, r2, #-3.9062499126885086e-05 | addxy r4, r6, #-3.9062499126885086e-05 | ld r5, [a143] | st r3, [0342]
mul r0, r0, #256.0 | mul r4, r4, #256.0 | st r7, [035a]
mul r0, r0, #2.7397259558135265 | mul r4, r4, #2.7397259558135265
max r1, r1, r0 | ld r0, [033b]
max r5, r5, r4 | ld r4, [0353] | st r1, [0346]
mov r3, r2 | mov r7, r6 | st r5, [035e]
mad r3, r0, #0.00390625 | mad r7, r4, #0.00390625
mul r0, r3, #256.0 | mul r4, r7, #256.0
ph_wrap r3, r3, r0
ph_wrap r7, r3, r4 | st r3, [033a]
addxy r0, r3, #1.0 | addxy r4, r7, #1.0 | st r7, [0352]
st r3, [033c] | st r7, [0354]
ph_wrap r0, r3, r0
ph_wrap r4, r3, r4
mul r3, r2, #256.0 | mul r7, r6, #256.0
min r1, r0, r3
min r5, r4, r7
mul r2, r2, #-256.0 | mul r6, r6, #-256.0
max r1, r1, r2 | st r2, [0340] | ld r2, [033e]
max r5, r5, r6 | st r6, [0358] | ld r6, [0356]
mul r1, r2, r1 | mul r5, r6, r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #8.0 | mul r5, r5, #8.0
max r2, r1, -r1
max r6, r5, -r5
addxy r2, r2, #-1.0 | addxy r6, r6, #-1.0
mul r1, r1, r2 | mul r5, r5, r6 | ld r2, [0346] | ld r6, [035e]
mad r1, r0, #0.25 | mad r5, r4, #0.25
mov r0, r2 | mov r4, r6 | ld r2, [033c] | ld r6, [0354]
mad r0, r2, #-0.68493148895338163 | mad r4, r6, #-0.68493148895338163
addxy r2, r0, #-0.65753423000762279 | addxy r6, r4, #-0.65753423000762279
mul r0, r2, #-1.4600000381469727 | mul r4, r6, #-1.4600000381469727
ph_wrap r2, r3, r0
ph_wrap r6, r3, r4
min r0, r2, r3 | ld r3, [0340]
min r4, r6, r7 | ld r7, [0358]
max r0, r0, r3 | ld r3, [033e]
max r4, r4, r7 | ld r7, [0356]
mul r3, r3, r0 | mul r7, r7, r4
mul r0, r3, #256.0 | mul r4, r7, #256.0
mul r3, r0, #256.0 | mul r7, r4, #256.0
mul r0, r3, #8.0 | mul r4, r7, #8.0
max r3, r0, -r0
max r7, r4, -r4
addxy r3, r3, #-1.0 | addxy r7, r7, #-1.0
mul r0, r0, r3 | mul r4, r4, r7
mad r0, r2, #0.25 | mad r4, r6, #0.25 | ld r2, [0346] | ld r6, [035e]
subxy r3, r0, r1 | subxy r7, r4, r5
mad r3, r2, #-0.36500000953674316 | mad r7, r6, #-0.36500000953674316
addxy r0, r1, #0.25 | addxy r4, r5, #0.25
addxy r2, r3, #-0.0099999997764825821 | addxy r6, r7, #-0.0099999997764825821
mul r3, r0, r2 | mul r7, r4, r6 | ld r0, [c073]
mad r3, r1, #0.5 | mad r7, r5, #0.5 | ld r4, [c073] | ld r1, [0342] | ld r5, [035a]
mul r2, r3, r0 | mul r6, r7, r4
mad r2, r1, #0.125 | mad r6, r5, #0.125 | ld r1, [a132]
mul r3, r2, #4.0 | mul r7, r6, #4.0 | ld r5, [a133]
st r3, [0336] | st r7, [034e]
ld r0, [0336] | ld r4, [034e] | ld r2, [c056]
mad r0, r1, r2 | ld r6, [c056] | ld r1, [03c0]
mad r4, r5, r6 | ld r5, [03c0] | ld r2, [c057]
mad r0, r1, r2 | ld r6, [c057] | ld r2, [a03c]
mad r4, r5, r6 | ld r6, [a03d] | ld r3, [c076]
st r0, [a153] | ld r7, [c076]
st r4, [a154]
st r2, [a164] | ld r2, [c04b]
st r6, [a165] | ld r6, [c04b]
ld r0, [a064]
ld r4, [a065]
ld r1, [a124]
mul r1, r1, r2 | ld r5, [a125] | ld r2, [c05e]
mul r5, r5, r6 | st r0, [a16c] | ld r6, [c05e]
st r4, [a16d]
ld r0, [a12c]
mad r1, r0, r2 | ld r4, [a12d] | ld r2, [c05f]
mad r5, r4, r6 | addxy r1, r1, r2 | ld r6, [c05f]
addxy r5, r5, r6 | mov r0, #0.041666666666666664 | ld r2, [a034]
addxy r2, r2, r1 | mov r4, #0.041666666666666664 | ld r6, [a035]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [a1b4]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [a1b5]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a044]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a045]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mul r2, r1, #-0.08664339756999316 | st r2, [0360] | ld r1, [0365]
mul r6, r5, #-0.08664339756999316 | mov r0, #0.16666666666666666 | st r6, [0378] | ld r5, [037d]
mov r4, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a04c]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a04d]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0 | ld r0, [0360]
mul r6, r6, r4 | st r2, [0362] | ld r2, [a044]
addxy r2, r1, r2 | st r6, [037a] | ld r6, [a045]
addxy r6, r5, r6 | ld r4, [0378]
ph_wrap r1, r3, r2 | ld r2, [c074]
ph_wrap r5, r3, r6 | st r1, [0364] | ld r6, [c074]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [037c]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a134] | ld r1, [0362]
mul r2, r2, r3 | mul r6, r6, r7 | st r5, [a135] | ld r5, [037a] | st r1, [036c]
ld r3, [c070] | st r5, [0384]
mul r2, r2, r3 | ld r7, [c070]
mul r6, r6, r7 | ld r3, [c075]
ld r7, [c075]
ld r1, [c077]
mul r3, r3, r1 | ld r5, [c077]
mul r7, r7, r5 | ld r1, [c071]
mul r3, r3, r1 | mul r1, r0, r2 | ld r5, [c071]
mul r7, r7, r5 | mul r5, r4, r6
min r2, r1, #0.00390625 | ld r1, [036c]
min r6, r5, #0.00390625 | ld r5, [0384] | st r2, [0370]
mul r3, r1, r3 | mul r7, r5, r7 | st r6, [0388]
addxy r2, r0, #-0.004999999888241291 | addxy r6, r4, #-0.004999999888241291 | st r3, [036e] | ld r3, [a13c]
mul r2, r2, #5.479451911627053 | mul r6, r6, #5.479451911627053 | st r7, [0386] | ld r7, [a13d]
max r3, r3, r2 | ld r2, [0369]
max r7, r7, r6 | st r3, [0372] | ld r6, [0381]
mov r3, r0 | mov r7, r4 | st r7, [038a]
mad r3, r2, #0.5 | mad r7, r6, #0.5
mul r2, r3, #2.0 | mul r6, r7, #2.0
ph_wrap r3, r3, r2
ph_wrap r7, r3, r6 | st r3, [0368]
addxy r2, r3, #1.0 | addxy r6, r7, #1.0 | st r7, [0380]
st r3, [0374] | st r7, [038c]
ph_wrap r2, r3, r2
ph_wrap r6, r3, r6
mul r3, r0, #2.0 | mul r7, r4, #2.0
min r1, r2, r3
min r5, r6, r7
mul r0, r0, #-2.0 | mul r4, r4, #-2.0
max r1, r1, r0 | st r0, [0376] | ld r0, [036c]
max r5, r5, r4 | st r4, [038e] | ld r4, [0384]
mul r1, r0, r1 | mul r5, r4, r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #32.0 | mul r5, r5, #32.0
max r0, r1, -r1
max r4, r5, -r5
addxy r0, r0, #-2.0 | addxy r4, r4, #-2.0
mul r1, r1, r0 | mul r5, r5, r4
addxy r0, r1, r2 | addxy r4, r5, r6 | ld r1, [0372] | ld r5, [038a]
mov r2, r1 | mov r6, r5 | ld r1, [0374] | ld r5, [038c]
mad r2, r1, #-0.68493148895338163 | mad r6, r5, #-0.68493148895338163
addxy r1, r2, #-0.65753423000762279 | addxy r5, r6, #-0.65753423000762279
mul r2, r1, #-1.4600000381469727 | mul r6, r5, #-1.4600000381469727
ph_wrap r1, r3, r2
ph_wrap r5, r3, r6
min r2, r1, r3 | ld r3, [0376]
min r6, r5, r7 | ld r7, [038e]
max r2, r2, r3 | ld r3, [036c]
max r6, r6, r7 | ld r7, [0384]
mul r3, r3, r2 | mul r7, r7, r6
mul r2, r3, #256.0 | mul r6, r7, #256.0
mul r3, r2, #32.0 | mul r7, r6, #32.0
max r2, r3, -r3
max r6, r7, -r7
addxy r2, r2, #-2.0 | addxy r6, r6, #-2.0
mul r3, r3, r2 | mul r7, r7, r6
addxy r2, r3, r1 | addxy r6, r7, r5 | ld r1, [0372] | ld r5, [038a]
subxy r3, r2, r0 | subxy r7, r6, r4
mad r1, r3, #-0.68493148895338163 | mad r5, r7, #-0.68493148895338163
addxy r2, r0, #1.0 | addxy r6, r4, #1.0
addxy r3, r1, #0.027397258945758795 | addxy r7, r5, #0.027397258945758795
mul r1, r2, r3 | mul r5, r6, r7 | ld r2, [c072]
mad r0, r1, #-0.73000001907348633 | mad r4, r5, #-0.73000001907348633 | ld r6, [c072]
mul r3, r2, r0 | mul r7, r6, r4 | ld r2, [0370] | ld r6, [0388] | ld r1, [a144]
addxy r0, r2, #-3.9062499126885086e-05 | addxy r4, r6, #-3.9062499126885086e-05 | ld r5, [a145] | st r3, [0372]
mul r0, r0, #256.0 | mul r4, r4, #256.0 | st r7, [038a]
mul r0, r0, #2.7397259558135265 | mul r4, r4, #2.7397259558135265
max r1, r1, r0 | ld r0, [036b]
max r5, r5, r4 | ld r4, [0383] | st r1, [0376]
mov r3, r2 | mov r7, r6 | st r5, [038e]
mad r3, r0, #0.00390625 | mad r7, r4, #0.00390625
mul r0, r3, #256.0 | mul r4, r7, #256.0
ph_wrap r3, r3, r0
ph_wrap r7, r3, r4 | st r3, [036a]
addxy r0, r3, #1.0 | addxy r4, r7, #1.0 | st r7, [0382]
st r3, [036c] | st r7, [0384]
ph_wrap r0, r3, r0
ph_wrap r4, r3, r4
mul r3, r2, #256.0 | mul r7, r6, #256.0
min r1, r0, r3
min r5, r4, r7
mul r2, r2, #-256.0 | mul r6, r6, #-256.0
max r1, r1, r2 | st r2, [0370] | ld r2, [036e]
max r5, r5, r6 | st r6, [0388] | ld r6, [0386]
mul r1, r2, r1 | mul r5, r6, r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #8.0 | mul r5, r5, #8.0
max r2, r1, -r1
max r6, r5, -r5
addxy r2, r2, #-1.0 | addxy r6, r6, #-1.0
mul r1, r1, r2 | mul r5, r5, r6 | ld r2, [0376] | ld r6, [038e]
mad r1, r0, #0.25 | mad r5, r4, #0.25
mov r0, r2 | mov r4, r6 | ld r2, [036c] | ld r6, [0384]
mad r0, r2, #-0.68493148895338163 | mad r4, r6, #-0.68493148895338163
addxy r2, r0, #-0.65753423000762279 | addxy r6, r4, #-0.65753423000762279
mul r0, r2, #-1.4600000381469727 | mul r4, r6, #-1.4600000381469727
ph_wrap r2, r3, r0
ph_wrap r6, r3, r4
min r0, r2, r3 | ld r3, [0370]
min r4, r6, r7 | ld r7, [0388]
max r0, r0, r3 | ld r3, [036e]
max r4, r4, r7 | ld r7, [0386]
mul r3, r3, r0 | mul r7, r7, r4
mul r0, r3, #256.0 | mul r4, r7, #256.0
mul r3, r0, #256.0 | mul r7, r4, #256.0
mul r0, r3, #8.0 | mul r4, r7, #8.0
max r3, r0, -r0
max r7, r4, -r4
addxy r3, r3, #-1.0 | addxy r7, r7, #-1.0
mul r0, r0, r3 | mul r4, r4, r7
mad r0, r2, #0.25 | mad r4, r6, #0.25 | ld r2, [0376] | ld r6, [038e]
subxy r3, r0, r1 | subxy r7, r4, r5
mad r3, r2, #-0.36500000953674316 | mad r7, r6, #-0.36500000953674316
addxy r0, r1, #0.25 | addxy r4, r5, #0.25
addxy r2, r3, #-0.0099999997764825821 | addxy r6, r7, #-0.0099999997764825821
mul r3, r0, r2 | mul r7, r4, r6 | ld r0, [c073]
mad r3, r1, #0.5 | mad r7, r5, #0.5 | ld r4, [c073] | ld r1, [0372] | ld r5, [038a]
mul r2, r3, r0 | mul r6, r7, r4
mad r2, r1, #0.125 | mad r6, r5, #0.125 | ld r1, [a134]
mul r3, r2, #4.0 | mul r7, r6, #4.0 | ld r5, [a135]
st r3, [0366] | st r7, [037e]
ld r0, [0366] | ld r4, [037e] | ld r2, [c056]
mad r0, r1, r2 | ld r6, [c056] | ld r1, [03c0]
mad r4, r5, r6 | ld r5, [03c0] | ld r2, [c057]
mad r0, r1, r2 | ld r6, [c057]
mad r4, r5, r6 | st r0, [a155]
st r4, [a156]
