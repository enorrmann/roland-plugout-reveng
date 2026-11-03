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
shl r2, r1, 10
shl r6, r5, 10 | min r1, r2, #0.25 | ld r2, [030c]
min r5, r6, #0.25 | ld r6, [0324] | st r1, [0310]
mul r3, r2, r3 | mul r7, r6, r7 | st r5, [0328]
mul r3, r3, #256.0 | mul r7, r7, #256.0
addxy r1, r0, #-0.004999999888241291 | addxy r5, r4, #-0.004999999888241291 | st r3, [030e] | ld r3, [a138]
mul r1, r1, #5.479451911627053 | mul r5, r5, #5.479451911627053 | st r7, [0326] | ld r7, [a139]
max r3, r3, r1 | ld r1, [0309]
max r7, r7, r5 | st r3, [0312] | ld r5, [0321]
mov r3, r0 | mov r7, r4 | st r7, [032a]
mad r3, r1, #0.5 | mad r7, r5, #0.5
mul r1, r3, #2.0 | mul r5, r7, #2.0
ph_wrap r3, r3, r1
ph_wrap r7, r3, r5 | st r3, [0308]
addxy r1, r3, #1.0 | addxy r5, r7, #1.0 | st r7, [0320]
st r3, [0314] | st r7, [032c]
ph_wrap r1, r3, r1
ph_wrap r5, r3, r5
mul r3, r0, #2.0 | mul r7, r4, #2.0
min r2, r1, r3
min r6, r5, r7
mul r0, r0, #-2.0 | mul r4, r4, #-2.0
max r2, r2, r0 | st r0, [0316] | ld r0, [030c]
max r6, r6, r4 | st r4, [032e] | ld r4, [0324]
mul r2, r0, r2 | mul r6, r4, r6
shl r2, r2, 13
shl r6, r6, 13
absadd r0, r2, #-2.0
absadd r4, r6, #-2.0
mul r2, r2, r0 | mul r6, r6, r4
addxy r0, r2, r1 | addxy r4, r6, r5 | ld r2, [0312] | ld r6, [032a]
mov r1, r2 | mov r5, r6 | ld r2, [0314] | ld r6, [032c]
mad r1, r2, #-0.68493148895338163 | mad r5, r6, #-0.68493148895338163
addxy r2, r1, #-0.65753423000762279 | addxy r6, r5, #-0.65753423000762279
mul r1, r2, #-1.4600000381469727 | mul r5, r6, #-1.4600000381469727
ph_wrap r2, r3, r1
ph_wrap r6, r3, r5 | min r1, r2, r3 | ld r3, [0316]
min r5, r6, r7 | ld r7, [032e]
max r1, r1, r3 | ld r3, [030c]
max r5, r5, r7 | ld r7, [0324]
mul r3, r3, r1 | mul r7, r7, r5
shl r1, r3, 13
shl r5, r7, 13
absadd r3, r1, #-2.0
absadd r7, r5, #-2.0
mul r1, r1, r3 | mul r5, r5, r7
addxy r3, r1, r2 | addxy r7, r5, r6 | ld r2, [0312] | ld r6, [032a]
subxy r1, r3, r0 | subxy r5, r7, r4
mad r2, r1, #-0.68493148895338163 | mad r6, r5, #-0.68493148895338163
addxy r3, r0, #1.0 | addxy r7, r4, #1.0
addxy r1, r2, #0.027397258945758795 | addxy r5, r6, #0.027397258945758795
mul r2, r3, r1 | mul r6, r7, r5 | ld r3, [c072]
mad r0, r2, #-0.73000001907348633 | mad r4, r6, #-0.73000001907348633 | ld r7, [c072]
mul r1, r3, r0 | mul r5, r7, r4 | ld r3, [0310] | ld r7, [0328] | ld r2, [a140]
addxy r0, r3, #-0.0024999999441206455 | addxy r4, r7, #-0.0024999999441206455 | ld r6, [a141] | st r1, [0312]
mul r0, r0, #10.958903823254106 | mul r4, r4, #10.958903823254106 | st r5, [032a]
max r2, r2, r0 | ld r0, [030b]
max r6, r6, r4 | ld r4, [0323] | st r2, [0316]
mov r1, r3 | mov r5, r7 | st r6, [032e]
mad r1, r0, #0.25 | mad r5, r4, #0.25
mul r0, r1, #4.0 | mul r4, r5, #4.0
ph_wrap r1, r3, r0
ph_wrap r5, r3, r4 | st r1, [030a]
addxy r0, r1, #1.0 | addxy r4, r5, #1.0 | st r5, [0322]
st r1, [030c] | st r5, [0324]
ph_wrap r0, r3, r0
ph_wrap r4, r3, r4
mul r1, r3, #4.0 | mul r5, r7, #4.0
min r2, r0, r1
min r6, r4, r5
mul r3, r3, #-4.0 | mul r7, r7, #-4.0
max r2, r2, r3 | st r3, [0310] | ld r3, [030e]
max r6, r6, r7 | st r7, [0328] | ld r7, [0326]
mul r2, r3, r2 | mul r6, r7, r6
shl r2, r2, 15
shl r6, r6, 15
absadd r3, r2, #-1.0
absadd r7, r6, #-1.0
mac_p1wc3 r2, r2, r3, #0.25, r0 | ld r3, [0316]
mac_p1wc3 r6, r6, r7, #0.25, r4 | ld r7, [032e]
mov r0, r3 | mov r4, r7 | ld r3, [030c] | ld r7, [0324]
mad r0, r3, #-0.68493148895338163 | mad r4, r7, #-0.68493148895338163
addxy r3, r0, #-0.65753423000762279 | addxy r7, r4, #-0.65753423000762279
mul r0, r3, #-1.4600000381469727 | mul r4, r7, #-1.4600000381469727
ph_wrap r3, r3, r0
ph_wrap r7, r3, r4 | min r0, r3, r1 | ld r1, [0310]
min r4, r7, r5 | ld r5, [0328]
max r0, r0, r1 | ld r1, [030e]
max r4, r4, r5 | ld r5, [0326]
mul r1, r1, r0 | mul r5, r5, r4
shl r0, r1, 15
shl r4, r5, 15
absadd r1, r0, #-1.0
absadd r5, r4, #-1.0
mac_p1wc3 r0, r0, r1, #0.25, r3 | ld r3, [0316]
mac_p1wc3 r4, r4, r5, #0.25, r7 | ld r7, [032e]
subxy r1, r0, r2 | subxy r5, r4, r6
mad r1, r3, #-0.36500000953674316 | mad r5, r7, #-0.36500000953674316
addxy r0, r2, #0.25 | addxy r4, r6, #0.25
addxy r3, r1, #-0.0099999997764825821 | addxy r7, r5, #-0.0099999997764825821
mac_p1wc3 r1, r0, r3, #0.5, r2 | ld r0, [c073] | ld r3, [0312]
mac_p1wc3 r5, r4, r7, #0.5, r6 | ld r4, [c073] | ld r7, [032a]
mac_p1wc3 r2, r1, r0, #0.125, r3 | ld r3, [c076]
mac_p1wc3 r6, r5, r4, #0.125, r7 | ld r7, [c076]
mul r1, r2, #4.0 | mul r5, r6, #4.0 | ld r2, [c056]
st r1, [0306] | st r5, [031e]
ld r0, [0306] | ld r4, [031e] | ld r1, [a130]
mad r0, r1, r2 | ld r5, [a131] | ld r6, [c056]
mad r4, r5, r6 | ld r1, [03c0] | ld r2, [c057]
mad r0, r1, r2 | ld r5, [03c0] | ld r6, [c057]
mad r4, r5, r6 | ld r2, [a03a]
ld r6, [a03b]
st r0, [a151]
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
shl r2, r1, 10
shl r6, r5, 10 | min r1, r2, #0.25 | ld r2, [033c]
min r5, r6, #0.25 | ld r6, [0354] | st r1, [0340]
mul r3, r2, r3 | mul r7, r6, r7 | st r5, [0358]
mul r3, r3, #256.0 | mul r7, r7, #256.0
addxy r1, r0, #-0.004999999888241291 | addxy r5, r4, #-0.004999999888241291 | st r3, [033e] | ld r3, [a13a]
mul r1, r1, #5.479451911627053 | mul r5, r5, #5.479451911627053 | st r7, [0356] | ld r7, [a13b]
max r3, r3, r1 | ld r1, [0339]
max r7, r7, r5 | st r3, [0342] | ld r5, [0351]
mov r3, r0 | mov r7, r4 | st r7, [035a]
mad r3, r1, #0.5 | mad r7, r5, #0.5
mul r1, r3, #2.0 | mul r5, r7, #2.0
ph_wrap r3, r3, r1
ph_wrap r7, r3, r5 | st r3, [0338]
addxy r1, r3, #1.0 | addxy r5, r7, #1.0 | st r7, [0350]
st r3, [0344] | st r7, [035c]
ph_wrap r1, r3, r1
ph_wrap r5, r3, r5
mul r3, r0, #2.0 | mul r7, r4, #2.0
min r2, r1, r3
min r6, r5, r7
mul r0, r0, #-2.0 | mul r4, r4, #-2.0
max r2, r2, r0 | st r0, [0346] | ld r0, [033c]
max r6, r6, r4 | st r4, [035e] | ld r4, [0354]
mul r2, r0, r2 | mul r6, r4, r6
shl r2, r2, 13
shl r6, r6, 13
absadd r0, r2, #-2.0
absadd r4, r6, #-2.0
mul r2, r2, r0 | mul r6, r6, r4
addxy r0, r2, r1 | addxy r4, r6, r5 | ld r2, [0342] | ld r6, [035a]
mov r1, r2 | mov r5, r6 | ld r2, [0344] | ld r6, [035c]
mad r1, r2, #-0.68493148895338163 | mad r5, r6, #-0.68493148895338163
addxy r2, r1, #-0.65753423000762279 | addxy r6, r5, #-0.65753423000762279
mul r1, r2, #-1.4600000381469727 | mul r5, r6, #-1.4600000381469727
ph_wrap r2, r3, r1
ph_wrap r6, r3, r5 | min r1, r2, r3 | ld r3, [0346]
min r5, r6, r7 | ld r7, [035e]
max r1, r1, r3 | ld r3, [033c]
max r5, r5, r7 | ld r7, [0354]
mul r3, r3, r1 | mul r7, r7, r5
shl r1, r3, 13
shl r5, r7, 13
absadd r3, r1, #-2.0
absadd r7, r5, #-2.0
mul r1, r1, r3 | mul r5, r5, r7
addxy r3, r1, r2 | addxy r7, r5, r6 | ld r2, [0342] | ld r6, [035a]
subxy r1, r3, r0 | subxy r5, r7, r4
mad r2, r1, #-0.68493148895338163 | mad r6, r5, #-0.68493148895338163
addxy r3, r0, #1.0 | addxy r7, r4, #1.0
addxy r1, r2, #0.027397258945758795 | addxy r5, r6, #0.027397258945758795
mul r2, r3, r1 | mul r6, r7, r5 | ld r3, [c072]
mad r0, r2, #-0.73000001907348633 | mad r4, r6, #-0.73000001907348633 | ld r7, [c072]
mul r1, r3, r0 | mul r5, r7, r4 | ld r3, [0340] | ld r7, [0358] | ld r2, [a142]
addxy r0, r3, #-0.0024999999441206455 | addxy r4, r7, #-0.0024999999441206455 | ld r6, [a143] | st r1, [0342]
mul r0, r0, #10.958903823254106 | mul r4, r4, #10.958903823254106 | st r5, [035a]
max r2, r2, r0 | ld r0, [033b]
max r6, r6, r4 | ld r4, [0353] | st r2, [0346]
mov r1, r3 | mov r5, r7 | st r6, [035e]
mad r1, r0, #0.25 | mad r5, r4, #0.25
mul r0, r1, #4.0 | mul r4, r5, #4.0
ph_wrap r1, r3, r0
ph_wrap r5, r3, r4 | st r1, [033a]
addxy r0, r1, #1.0 | addxy r4, r5, #1.0 | st r5, [0352]
st r1, [033c] | st r5, [0354]
ph_wrap r0, r3, r0
ph_wrap r4, r3, r4
mul r1, r3, #4.0 | mul r5, r7, #4.0
min r2, r0, r1
min r6, r4, r5
mul r3, r3, #-4.0 | mul r7, r7, #-4.0
max r2, r2, r3 | st r3, [0340] | ld r3, [033e]
max r6, r6, r7 | st r7, [0358] | ld r7, [0356]
mul r2, r3, r2 | mul r6, r7, r6
shl r2, r2, 15
shl r6, r6, 15
absadd r3, r2, #-1.0
absadd r7, r6, #-1.0
mac_p1wc3 r2, r2, r3, #0.25, r0 | ld r3, [0346]
mac_p1wc3 r6, r6, r7, #0.25, r4 | ld r7, [035e]
mov r0, r3 | mov r4, r7 | ld r3, [033c] | ld r7, [0354]
mad r0, r3, #-0.68493148895338163 | mad r4, r7, #-0.68493148895338163
addxy r3, r0, #-0.65753423000762279 | addxy r7, r4, #-0.65753423000762279
mul r0, r3, #-1.4600000381469727 | mul r4, r7, #-1.4600000381469727
ph_wrap r3, r3, r0
ph_wrap r7, r3, r4 | min r0, r3, r1 | ld r1, [0340]
min r4, r7, r5 | ld r5, [0358]
max r0, r0, r1 | ld r1, [033e]
max r4, r4, r5 | ld r5, [0356]
mul r1, r1, r0 | mul r5, r5, r4
shl r0, r1, 15
shl r4, r5, 15
absadd r1, r0, #-1.0
absadd r5, r4, #-1.0
mac_p1wc3 r0, r0, r1, #0.25, r3 | ld r3, [0346]
mac_p1wc3 r4, r4, r5, #0.25, r7 | ld r7, [035e]
subxy r1, r0, r2 | subxy r5, r4, r6
mad r1, r3, #-0.36500000953674316 | mad r5, r7, #-0.36500000953674316
addxy r0, r2, #0.25 | addxy r4, r6, #0.25
addxy r3, r1, #-0.0099999997764825821 | addxy r7, r5, #-0.0099999997764825821
mac_p1wc3 r1, r0, r3, #0.5, r2 | ld r0, [c073] | ld r3, [0342]
mac_p1wc3 r5, r4, r7, #0.5, r6 | ld r4, [c073] | ld r7, [035a]
mac_p1wc3 r2, r1, r0, #0.125, r3 | ld r3, [c076]
mac_p1wc3 r6, r5, r4, #0.125, r7 | ld r7, [c076]
mul r1, r2, #4.0 | mul r5, r6, #4.0 | ld r2, [c056]
st r1, [0336] | st r5, [034e]
ld r0, [0336] | ld r4, [034e] | ld r1, [a132]
mad r0, r1, r2 | ld r5, [a133] | ld r6, [c056]
mad r4, r5, r6 | ld r1, [03c0] | ld r2, [c057]
mad r0, r1, r2 | ld r5, [03c0] | ld r6, [c057]
mad r4, r5, r6 | ld r2, [a03c]
ld r6, [a03d]
st r0, [a153]
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
shl r2, r1, 10
shl r6, r5, 10 | min r1, r2, #0.25 | ld r2, [036c]
min r5, r6, #0.25 | ld r6, [0384] | st r1, [0370]
mul r3, r2, r3 | mul r7, r6, r7 | st r5, [0388]
mul r3, r3, #256.0 | mul r7, r7, #256.0
addxy r1, r0, #-0.004999999888241291 | addxy r5, r4, #-0.004999999888241291 | st r3, [036e] | ld r3, [a13c]
mul r1, r1, #5.479451911627053 | mul r5, r5, #5.479451911627053 | st r7, [0386] | ld r7, [a13d]
max r3, r3, r1 | ld r1, [0369]
max r7, r7, r5 | st r3, [0372] | ld r5, [0381]
mov r3, r0 | mov r7, r4 | st r7, [038a]
mad r3, r1, #0.5 | mad r7, r5, #0.5
mul r1, r3, #2.0 | mul r5, r7, #2.0
ph_wrap r3, r3, r1
ph_wrap r7, r3, r5 | st r3, [0368]
addxy r1, r3, #1.0 | addxy r5, r7, #1.0 | st r7, [0380]
st r3, [0374] | st r7, [038c]
ph_wrap r1, r3, r1
ph_wrap r5, r3, r5
mul r3, r0, #2.0 | mul r7, r4, #2.0
min r2, r1, r3
min r6, r5, r7
mul r0, r0, #-2.0 | mul r4, r4, #-2.0
max r2, r2, r0 | st r0, [0376] | ld r0, [036c]
max r6, r6, r4 | st r4, [038e] | ld r4, [0384]
mul r2, r0, r2 | mul r6, r4, r6
shl r2, r2, 13
shl r6, r6, 13
absadd r0, r2, #-2.0
absadd r4, r6, #-2.0
mul r2, r2, r0 | mul r6, r6, r4
addxy r0, r2, r1 | addxy r4, r6, r5 | ld r2, [0372] | ld r6, [038a]
mov r1, r2 | mov r5, r6 | ld r2, [0374] | ld r6, [038c]
mad r1, r2, #-0.68493148895338163 | mad r5, r6, #-0.68493148895338163
addxy r2, r1, #-0.65753423000762279 | addxy r6, r5, #-0.65753423000762279
mul r1, r2, #-1.4600000381469727 | mul r5, r6, #-1.4600000381469727
ph_wrap r2, r3, r1
ph_wrap r6, r3, r5 | min r1, r2, r3 | ld r3, [0376]
min r5, r6, r7 | ld r7, [038e]
max r1, r1, r3 | ld r3, [036c]
max r5, r5, r7 | ld r7, [0384]
mul r3, r3, r1 | mul r7, r7, r5
shl r1, r3, 13
shl r5, r7, 13
absadd r3, r1, #-2.0
absadd r7, r5, #-2.0
mul r1, r1, r3 | mul r5, r5, r7
addxy r3, r1, r2 | addxy r7, r5, r6 | ld r2, [0372] | ld r6, [038a]
subxy r1, r3, r0 | subxy r5, r7, r4
mad r2, r1, #-0.68493148895338163 | mad r6, r5, #-0.68493148895338163
addxy r3, r0, #1.0 | addxy r7, r4, #1.0
addxy r1, r2, #0.027397258945758795 | addxy r5, r6, #0.027397258945758795
mul r2, r3, r1 | mul r6, r7, r5 | ld r3, [c072]
mad r0, r2, #-0.73000001907348633 | mad r4, r6, #-0.73000001907348633 | ld r7, [c072]
mul r1, r3, r0 | mul r5, r7, r4 | ld r3, [0370] | ld r7, [0388] | ld r2, [a144]
addxy r0, r3, #-0.0024999999441206455 | addxy r4, r7, #-0.0024999999441206455 | ld r6, [a145] | st r1, [0372]
mul r0, r0, #10.958903823254106 | mul r4, r4, #10.958903823254106 | st r5, [038a]
max r2, r2, r0 | ld r0, [036b]
max r6, r6, r4 | ld r4, [0383] | st r2, [0376]
mov r1, r3 | mov r5, r7 | st r6, [038e]
mad r1, r0, #0.25 | mad r5, r4, #0.25
mul r0, r1, #4.0 | mul r4, r5, #4.0
ph_wrap r1, r3, r0
ph_wrap r5, r3, r4 | st r1, [036a]
addxy r0, r1, #1.0 | addxy r4, r5, #1.0 | st r5, [0382]
st r1, [036c] | st r5, [0384]
ph_wrap r0, r3, r0
ph_wrap r4, r3, r4
mul r1, r3, #4.0 | mul r5, r7, #4.0
min r2, r0, r1
min r6, r4, r5
mul r3, r3, #-4.0 | mul r7, r7, #-4.0
max r2, r2, r3 | st r3, [0370] | ld r3, [036e]
max r6, r6, r7 | st r7, [0388] | ld r7, [0386]
mul r2, r3, r2 | mul r6, r7, r6
shl r2, r2, 15
shl r6, r6, 15
absadd r3, r2, #-1.0
absadd r7, r6, #-1.0
mac_p1wc3 r2, r2, r3, #0.25, r0 | ld r3, [0376]
mac_p1wc3 r6, r6, r7, #0.25, r4 | ld r7, [038e]
mov r0, r3 | mov r4, r7 | ld r3, [036c] | ld r7, [0384]
mad r0, r3, #-0.68493148895338163 | mad r4, r7, #-0.68493148895338163
addxy r3, r0, #-0.65753423000762279 | addxy r7, r4, #-0.65753423000762279
mul r0, r3, #-1.4600000381469727 | mul r4, r7, #-1.4600000381469727
ph_wrap r3, r3, r0
ph_wrap r7, r3, r4 | min r0, r3, r1 | ld r1, [0370]
min r4, r7, r5 | ld r5, [0388]
max r0, r0, r1 | ld r1, [036e]
max r4, r4, r5 | ld r5, [0386]
mul r1, r1, r0 | mul r5, r5, r4
shl r0, r1, 15
shl r4, r5, 15
absadd r1, r0, #-1.0
absadd r5, r4, #-1.0
mac_p1wc3 r0, r0, r1, #0.25, r3 | ld r3, [0376]
mac_p1wc3 r4, r4, r5, #0.25, r7 | ld r7, [038e]
subxy r1, r0, r2 | subxy r5, r4, r6
mad r1, r3, #-0.36500000953674316 | mad r5, r7, #-0.36500000953674316
addxy r0, r2, #0.25 | addxy r4, r6, #0.25
addxy r3, r1, #-0.0099999997764825821 | addxy r7, r5, #-0.0099999997764825821
mac_p1wc3 r1, r0, r3, #0.5, r2 | ld r0, [c073] | ld r3, [0372]
mac_p1wc3 r5, r4, r7, #0.5, r6 | ld r4, [c073] | ld r7, [038a]
mac_p1wc3 r2, r1, r0, #0.125, r3
mac_p1wc3 r6, r5, r4, #0.125, r7
mul r1, r2, #4.0 | mul r5, r6, #4.0 | ld r2, [c056]
st r1, [0366] | st r5, [037e]
ld r0, [0366] | ld r4, [037e] | ld r1, [a134]
mad r0, r1, r2 | ld r5, [a135] | ld r6, [c056]
mad r4, r5, r6 | ld r1, [03c0] | ld r2, [c057]
mad r0, r1, r2 | ld r5, [03c0] | ld r6, [c057]
mad r4, r5, r6 | st r0, [a155]
st r4, [a156]
