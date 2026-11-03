ld r0, [0353] | ld r2, [0351] | ld r6, [a039]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | st r6, [a161]
ph_wrap r0, r3, r0 | ld r4, [a061] | ld r6, [c04b]
addxy r2, r2, r0 | st r0, [0352] | ld r0, [a060] | ld r3, [c057]
ph_wrap r2, r3, r2 | ld r1, [a120] | ld r7, [c057]
st r2, [0350] | ld r2, [a038]
st r2, [a160] | ld r2, [c04b]
mul r1, r1, r2 | ld r5, [a121] | ld r2, [c05e]
mul r5, r5, r6 | st r0, [a168] | ld r6, [c05e]
st r4, [a169]
ld r0, [a128]
mad r1, r0, r2 | ld r4, [a129] | ld r2, [c05f]
mad r5, r4, r6 | addxy r1, r1, r2 | ld r6, [c05f]
addxy r5, r5, r6 | mov r0, #0.041666666666666664 | ld r2, [a030]
addxy r2, r2, r1 | mov r4, #0.041666666666666664 | ld r6, [a031]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [0300]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [030a]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a040]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a041]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mul r2, r1, #-0.08664339756999316 | st r2, [0302] | ld r1, [0307]
mul r6, r5, #-0.08664339756999316 | mov r0, #0.16666666666666666 | st r6, [030c] | ld r5, [0311]
mov r4, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a048]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a049]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | st r2, [0304] | ld r2, [a040]
addxy r2, r1, r2 | st r6, [030e] | ld r6, [a041]
addxy r6, r5, r6
ph_wrap r1, r3, r2 | ld r2, [c056]
ph_wrap r5, r3, r6 | st r1, [0306] | ld r6, [c056]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [0310]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a130] | ld r1, [0350]
st r5, [a131] | ld r5, [0350]
ld r0, [a130]
mul r0, r0, r2 | ld r4, [a131] | ld r2, [c077]
mul r4, r4, r6 | mad r0, r1, r3 | ld r6, [c077] | ld r1, [0304]
mad r4, r5, r7 | ld r3, [c079] | ld r5, [030e] | st r1, [0188]
mul r2, r2, r3 | ld r7, [c079] | st r5, [019c]
mul r6, r6, r7 | ld r3, [c072] | st r0, [0308] | ld r0, [0302]
mul r2, r2, r3 | ld r7, [c072] | st r4, [0312] | ld r4, [030c]
mul r6, r6, r7 | ld r3, [c078]
ld r7, [c078]
ld r1, [c07a]
mul r3, r3, r1 | ld r5, [c07a]
mul r7, r7, r5 | ld r1, [c073]
mul r3, r3, r1 | mul r1, r0, r2 | ld r5, [c073]
mul r7, r7, r5 | mul r5, r4, r6
shl r2, r1, 10
shl r6, r5, 10 | min r1, r2, #0.25 | ld r2, [0188]
min r5, r6, #0.25 | ld r6, [019c] | st r1, [018c]
mul r3, r2, r3 | mul r7, r6, r7 | st r5, [01a0]
mul r3, r3, #256.0 | mul r7, r7, #256.0
addxy r1, r0, #-0.004999999888241291 | addxy r5, r4, #-0.004999999888241291 | st r3, [018a] | ld r3, [a138]
mul r1, r1, #5.479451911627053 | mul r5, r5, #5.479451911627053 | st r7, [019e] | ld r7, [a139]
max r3, r3, r1 | ld r1, [0181]
max r7, r7, r5 | st r3, [018e] | ld r5, [0195]
mov r3, r0 | mov r7, r4 | st r7, [01a2]
mad r3, r1, #0.5 | mad r7, r5, #0.5
mul r1, r3, #2.0 | mul r5, r7, #2.0
ph_wrap r3, r3, r1
ph_wrap r7, r3, r5 | st r3, [0180]
addxy r1, r3, #1.0 | addxy r5, r7, #1.0 | st r7, [0194]
st r3, [0190] | st r7, [01a4]
ph_wrap r1, r3, r1
ph_wrap r5, r3, r5
mul r3, r0, #2.0 | mul r7, r4, #2.0
min r2, r1, r3
min r6, r5, r7
mul r0, r0, #-2.0 | mul r4, r4, #-2.0
max r2, r2, r0 | st r0, [0192] | ld r0, [0188]
max r6, r6, r4 | st r4, [01a6] | ld r4, [019c]
mul r2, r0, r2 | mul r6, r4, r6
shl r2, r2, 13
shl r6, r6, 13
absadd r0, r2, #-2.0
absadd r4, r6, #-2.0
mul r2, r2, r0 | mul r6, r6, r4
addxy r0, r2, r1 | addxy r4, r6, r5 | ld r2, [018e] | ld r6, [01a2]
mov r1, r2 | mov r5, r6 | ld r2, [0190] | ld r6, [01a4]
mad r1, r2, #-0.68493148895338163 | mad r5, r6, #-0.68493148895338163
addxy r2, r1, #-0.65753423000762279 | addxy r6, r5, #-0.65753423000762279
mul r1, r2, #-1.4600000381469727 | mul r5, r6, #-1.4600000381469727
ph_wrap r2, r3, r1
ph_wrap r6, r3, r5 | min r1, r2, r3 | ld r3, [0192]
min r5, r6, r7 | ld r7, [01a6]
max r1, r1, r3 | ld r3, [0188]
max r5, r5, r7 | ld r7, [019c]
mul r3, r3, r1 | mul r7, r7, r5
shl r1, r3, 13
shl r5, r7, 13
absadd r3, r1, #-2.0
absadd r7, r5, #-2.0
mul r1, r1, r3 | mul r5, r5, r7
addxy r3, r1, r2 | addxy r7, r5, r6 | ld r2, [018e] | ld r6, [01a2]
subxy r1, r3, r0 | subxy r5, r7, r4
mad r2, r1, #-0.68493148895338163 | mad r6, r5, #-0.68493148895338163
addxy r3, r0, #1.0 | addxy r7, r4, #1.0
addxy r1, r2, #0.027397258945758795 | addxy r5, r6, #0.027397258945758795
mul r2, r3, r1 | mul r6, r7, r5 | ld r3, [c075]
mad r0, r2, #-0.73000001907348633 | mad r4, r6, #-0.73000001907348633 | ld r7, [c075]
mul r1, r3, r0 | mul r5, r7, r4 | ld r3, [018c] | ld r7, [01a0] | ld r2, [a140]
addxy r0, r3, #-0.0024999999441206455 | addxy r4, r7, #-0.0024999999441206455 | ld r6, [a141] | st r1, [018e]
mul r0, r0, #10.958903823254106 | mul r4, r4, #10.958903823254106 | st r5, [01a2]
max r2, r2, r0 | ld r0, [0183]
max r6, r6, r4 | ld r4, [0197] | st r2, [0192]
mov r1, r3 | mov r5, r7 | st r6, [01a6]
mad r1, r0, #0.25 | mad r5, r4, #0.25
mul r0, r1, #4.0 | mul r4, r5, #4.0
ph_wrap r1, r3, r0
ph_wrap r5, r3, r4 | st r1, [0182]
addxy r0, r1, #1.0 | addxy r4, r5, #1.0 | st r5, [0196]
st r1, [0188] | st r5, [019c]
ph_wrap r0, r3, r0
ph_wrap r4, r3, r4
mul r1, r3, #4.0 | mul r5, r7, #4.0
min r2, r0, r1
min r6, r4, r5
mul r3, r3, #-4.0 | mul r7, r7, #-4.0
max r2, r2, r3 | st r3, [018c] | ld r3, [018a]
max r6, r6, r7 | st r7, [01a0] | ld r7, [019e]
mul r2, r3, r2 | mul r6, r7, r6
shl r2, r2, 15
shl r6, r6, 15
absadd r3, r2, #-1.0
absadd r7, r6, #-1.0
mac_p1wc3 r2, r2, r3, #0.25, r0 | ld r3, [0192]
mac_p1wc3 r6, r6, r7, #0.25, r4 | ld r7, [01a6]
mov r0, r3 | mov r4, r7 | ld r3, [0188] | ld r7, [019c]
mad r0, r3, #-0.68493148895338163 | mad r4, r7, #-0.68493148895338163
addxy r3, r0, #-0.65753423000762279 | addxy r7, r4, #-0.65753423000762279
mul r0, r3, #-1.4600000381469727 | mul r4, r7, #-1.4600000381469727
ph_wrap r3, r3, r0
ph_wrap r7, r3, r4 | min r0, r3, r1 | ld r1, [018c]
min r4, r7, r5 | ld r5, [01a0]
max r0, r0, r1 | ld r1, [018a]
max r4, r4, r5 | ld r5, [019e]
mul r1, r1, r0 | mul r5, r5, r4
shl r0, r1, 15
shl r4, r5, 15
absadd r1, r0, #-1.0
absadd r5, r4, #-1.0
mac_p1wc3 r0, r0, r1, #0.25, r3 | ld r3, [0192]
mac_p1wc3 r4, r4, r5, #0.25, r7 | ld r7, [01a6]
subxy r1, r0, r2 | subxy r5, r4, r6
mad r1, r3, #-0.36500000953674316 | mad r5, r7, #-0.36500000953674316
addxy r0, r2, #0.25 | addxy r4, r6, #0.25
addxy r3, r1, #-0.0099999997764825821 | addxy r7, r5, #-0.0099999997764825821
mac_p1wc3 r1, r0, r3, #0.5, r2 | ld r0, [c076] | ld r3, [018e]
mac_p1wc3 r5, r4, r7, #0.5, r6 | ld r4, [c076] | ld r7, [01a2]
mac_p1wc3 r2, r1, r0, #0.125, r3 | ld r1, [0308] | ld r0, [a1b8]
mac_p1wc3 r6, r5, r4, #0.125, r7 | ld r5, [0312] | ld r4, [a1b9]
mad r2, r1, #0.25 | mad r6, r5, #0.25 | ld r3, [a1d0]
mul r1, r0, r3 | ld r7, [a1d1] | st r2, [0192] | ld r3, [c071]
mul r5, r4, r7 | st r6, [01a6] | ld r2, [0300] | ld r7, [c071]
ld r6, [030a] | st r1, [018e] | st r5, [01a2]
addxy r2, r2, #-4.75 | addxy r6, r6, #-4.75 | ld r1, [c074]
mul r1, r1, r2 | ld r5, [c074] | ld r0, [a1c0]
mul r5, r5, r6 | ld r4, [a1c1]
mac_p1wc3 r2, r0, r3, #0.125, r1 | ld r0, [a1d8] | ld r3, [c070]
mac_p1wc3 r6, r4, r7, #0.125, r5 | ld r4, [a1d9] | ld r7, [c070]
mad r2, r0, #0.125 | mad r6, r4, #0.125
mul r1, r2, #0.17328679513998632 | mul r5, r6, #0.17328679513998632
mov r0, #0.041666666666666664 | mov r4, #0.041666666666666664
ymacd r0, r1, #0.16666666666666666 | ymacd r4, r5, #0.16666666666666666
ymacd r0, r1, #0.5 | ymacd r4, r5, #0.5
ymacd r0, r1, #1.0 | ymacd r4, r5, #1.0
ymacd r0, r1, #1.0 | ymacd r4, r5, #1.0
mul r2, r0, r0 | mul r6, r4, r4
mul r1, r2, r2 | mul r5, r6, r6
mul r0, r1, r1 | mul r4, r5, r5
mul r2, r0, r0 | mul r6, r4, r4
mul r1, r2, r2 | mul r5, r6, r6
mul r0, r1, #6.103515625e-05 | mul r4, r5, #6.103515625e-05 | ld r1, [c07b]
mul r2, r3, r0 | mul r6, r7, r4 | ld r5, [c07b] | st r1, [018a]
min r3, r2, #3.3569336665095761e-05 | st r5, [019e] | ld r2, [0185]
min r7, r6, #3.3569336665095761e-05 | shl r0, r3, 14 | ld r3, [0187] | ld r6, [0199]
shl r4, r7, 14 | ld r7, [019b]
mul r1, r0, r3 | mul r5, r4, r7
addxy r1, r1, r2 | addxy r5, r5, r6
mov r2, r1 | mov r6, r5 | st r1, [018c] | ld r1, [0192]
mad r2, r1, #-0.25 | st r5, [01a0] | ld r5, [01a6]
mad r6, r5, #-0.25 | ld r1, [018a] | ld r5, [019e]
mac_p1wc3 r2, r1, r3, #0.5, r2
mac_p1wc3 r6, r5, r7, #0.5, r6
mac_p1wc3 r2, r0, r2, #-0.5, r3 | ld r3, [018c]
mac_p1wc3 r6, r4, r6, #-0.5, r7 | ld r7, [01a0] | st r0, [018c]
mac_p1wc3 r3, r0, r2, #-0.5, r3 | st r4, [01a0]
mac_p1wc3 r7, r4, r6, #-0.5, r7 | st r3, [0188]
mul r0, r3, #-2.0 | mul r4, r7, #-2.0 | st r7, [019c]
st r0, [0184] | st r4, [0198]
mov r0, r3 | mov r4, r7 | ld r3, [0192] | ld r7, [01a6]
mad r0, r3, #0.125 | mad r4, r7, #0.125
mac_p1wc3 r3, r1, r2, #0.5, r0 | ld r1, [018c]
mac_p1wc3 r7, r5, r6, #0.5, r4 | ld r5, [01a0]
mac_p1wc3 r0, r1, r3, #-0.5, r2 | ld r3, [018e] | ld r2, [0188]
mac_p1wc3 r4, r5, r7, #-0.5, r6 | ld r7, [01a2] | ld r6, [019c]
mul r1, r0, #4.0 | mul r5, r4, #4.0
mul r0, r3, r2 | mul r4, r7, r6 | st r1, [0186] | ld r2, [a03a]
mul r1, r0, #-32.0 | mul r5, r4, #-32.0 | st r5, [019a] | ld r6, [a03b]
st r1, [a151] | ld r3, [c057]
st r5, [a152] | ld r7, [c057]
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
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [0314]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [031e]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a042]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a043]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mul r2, r1, #-0.08664339756999316 | st r2, [0316] | ld r1, [031b]
mul r6, r5, #-0.08664339756999316 | mov r0, #0.16666666666666666 | st r6, [0320] | ld r5, [0325]
mov r4, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a04a]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a04b]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | st r2, [0318] | ld r2, [a042]
addxy r2, r1, r2 | st r6, [0322] | ld r6, [a043]
addxy r6, r5, r6
ph_wrap r1, r3, r2 | ld r2, [c056]
ph_wrap r5, r3, r6 | st r1, [031a] | ld r6, [c056]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [0324]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a132] | ld r1, [0350]
st r5, [a133] | ld r5, [0350]
ld r0, [a132]
mul r0, r0, r2 | ld r4, [a133] | ld r2, [c077]
mul r4, r4, r6 | mad r0, r1, r3 | ld r6, [c077] | ld r1, [0318]
mad r4, r5, r7 | ld r3, [c079] | ld r5, [0322] | st r1, [01b0]
mul r2, r2, r3 | ld r7, [c079] | st r5, [01c4]
mul r6, r6, r7 | ld r3, [c072] | st r0, [031c] | ld r0, [0316]
mul r2, r2, r3 | ld r7, [c072] | st r4, [0326] | ld r4, [0320]
mul r6, r6, r7 | ld r3, [c078]
ld r7, [c078]
ld r1, [c07a]
mul r3, r3, r1 | ld r5, [c07a]
mul r7, r7, r5 | ld r1, [c073]
mul r3, r3, r1 | mul r1, r0, r2 | ld r5, [c073]
mul r7, r7, r5 | mul r5, r4, r6
shl r2, r1, 10
shl r6, r5, 10 | min r1, r2, #0.25 | ld r2, [01b0]
min r5, r6, #0.25 | ld r6, [01c4] | st r1, [01b4]
mul r3, r2, r3 | mul r7, r6, r7 | st r5, [01c8]
mul r3, r3, #256.0 | mul r7, r7, #256.0
addxy r1, r0, #-0.004999999888241291 | addxy r5, r4, #-0.004999999888241291 | st r3, [01b2] | ld r3, [a13a]
mul r1, r1, #5.479451911627053 | mul r5, r5, #5.479451911627053 | st r7, [01c6] | ld r7, [a13b]
max r3, r3, r1 | ld r1, [01a9]
max r7, r7, r5 | st r3, [01b6] | ld r5, [01bd]
mov r3, r0 | mov r7, r4 | st r7, [01ca]
mad r3, r1, #0.5 | mad r7, r5, #0.5
mul r1, r3, #2.0 | mul r5, r7, #2.0
ph_wrap r3, r3, r1
ph_wrap r7, r3, r5 | st r3, [01a8]
addxy r1, r3, #1.0 | addxy r5, r7, #1.0 | st r7, [01bc]
st r3, [01b8] | st r7, [01cc]
ph_wrap r1, r3, r1
ph_wrap r5, r3, r5
mul r3, r0, #2.0 | mul r7, r4, #2.0
min r2, r1, r3
min r6, r5, r7
mul r0, r0, #-2.0 | mul r4, r4, #-2.0
max r2, r2, r0 | st r0, [01ba] | ld r0, [01b0]
max r6, r6, r4 | st r4, [01ce] | ld r4, [01c4]
mul r2, r0, r2 | mul r6, r4, r6
shl r2, r2, 13
shl r6, r6, 13
absadd r0, r2, #-2.0
absadd r4, r6, #-2.0
mul r2, r2, r0 | mul r6, r6, r4
addxy r0, r2, r1 | addxy r4, r6, r5 | ld r2, [01b6] | ld r6, [01ca]
mov r1, r2 | mov r5, r6 | ld r2, [01b8] | ld r6, [01cc]
mad r1, r2, #-0.68493148895338163 | mad r5, r6, #-0.68493148895338163
addxy r2, r1, #-0.65753423000762279 | addxy r6, r5, #-0.65753423000762279
mul r1, r2, #-1.4600000381469727 | mul r5, r6, #-1.4600000381469727
ph_wrap r2, r3, r1
ph_wrap r6, r3, r5 | min r1, r2, r3 | ld r3, [01ba]
min r5, r6, r7 | ld r7, [01ce]
max r1, r1, r3 | ld r3, [01b0]
max r5, r5, r7 | ld r7, [01c4]
mul r3, r3, r1 | mul r7, r7, r5
shl r1, r3, 13
shl r5, r7, 13
absadd r3, r1, #-2.0
absadd r7, r5, #-2.0
mul r1, r1, r3 | mul r5, r5, r7
addxy r3, r1, r2 | addxy r7, r5, r6 | ld r2, [01b6] | ld r6, [01ca]
subxy r1, r3, r0 | subxy r5, r7, r4
mad r2, r1, #-0.68493148895338163 | mad r6, r5, #-0.68493148895338163
addxy r3, r0, #1.0 | addxy r7, r4, #1.0
addxy r1, r2, #0.027397258945758795 | addxy r5, r6, #0.027397258945758795
mul r2, r3, r1 | mul r6, r7, r5 | ld r3, [c075]
mad r0, r2, #-0.73000001907348633 | mad r4, r6, #-0.73000001907348633 | ld r7, [c075]
mul r1, r3, r0 | mul r5, r7, r4 | ld r3, [01b4] | ld r7, [01c8] | ld r2, [a142]
addxy r0, r3, #-0.0024999999441206455 | addxy r4, r7, #-0.0024999999441206455 | ld r6, [a143] | st r1, [01b6]
mul r0, r0, #10.958903823254106 | mul r4, r4, #10.958903823254106 | st r5, [01ca]
max r2, r2, r0 | ld r0, [01ab]
max r6, r6, r4 | ld r4, [01bf] | st r2, [01ba]
mov r1, r3 | mov r5, r7 | st r6, [01ce]
mad r1, r0, #0.25 | mad r5, r4, #0.25
mul r0, r1, #4.0 | mul r4, r5, #4.0
ph_wrap r1, r3, r0
ph_wrap r5, r3, r4 | st r1, [01aa]
addxy r0, r1, #1.0 | addxy r4, r5, #1.0 | st r5, [01be]
st r1, [01b0] | st r5, [01c4]
ph_wrap r0, r3, r0
ph_wrap r4, r3, r4
mul r1, r3, #4.0 | mul r5, r7, #4.0
min r2, r0, r1
min r6, r4, r5
mul r3, r3, #-4.0 | mul r7, r7, #-4.0
max r2, r2, r3 | st r3, [01b4] | ld r3, [01b2]
max r6, r6, r7 | st r7, [01c8] | ld r7, [01c6]
mul r2, r3, r2 | mul r6, r7, r6
shl r2, r2, 15
shl r6, r6, 15
absadd r3, r2, #-1.0
absadd r7, r6, #-1.0
mac_p1wc3 r2, r2, r3, #0.25, r0 | ld r3, [01ba]
mac_p1wc3 r6, r6, r7, #0.25, r4 | ld r7, [01ce]
mov r0, r3 | mov r4, r7 | ld r3, [01b0] | ld r7, [01c4]
mad r0, r3, #-0.68493148895338163 | mad r4, r7, #-0.68493148895338163
addxy r3, r0, #-0.65753423000762279 | addxy r7, r4, #-0.65753423000762279
mul r0, r3, #-1.4600000381469727 | mul r4, r7, #-1.4600000381469727
ph_wrap r3, r3, r0
ph_wrap r7, r3, r4 | min r0, r3, r1 | ld r1, [01b4]
min r4, r7, r5 | ld r5, [01c8]
max r0, r0, r1 | ld r1, [01b2]
max r4, r4, r5 | ld r5, [01c6]
mul r1, r1, r0 | mul r5, r5, r4
shl r0, r1, 15
shl r4, r5, 15
absadd r1, r0, #-1.0
absadd r5, r4, #-1.0
mac_p1wc3 r0, r0, r1, #0.25, r3 | ld r3, [01ba]
mac_p1wc3 r4, r4, r5, #0.25, r7 | ld r7, [01ce]
subxy r1, r0, r2 | subxy r5, r4, r6
mad r1, r3, #-0.36500000953674316 | mad r5, r7, #-0.36500000953674316
addxy r0, r2, #0.25 | addxy r4, r6, #0.25
addxy r3, r1, #-0.0099999997764825821 | addxy r7, r5, #-0.0099999997764825821
mac_p1wc3 r1, r0, r3, #0.5, r2 | ld r0, [c076] | ld r3, [01b6]
mac_p1wc3 r5, r4, r7, #0.5, r6 | ld r4, [c076] | ld r7, [01ca]
mac_p1wc3 r2, r1, r0, #0.125, r3 | ld r1, [031c] | ld r0, [a1ba]
mac_p1wc3 r6, r5, r4, #0.125, r7 | ld r5, [0326] | ld r4, [a1bb]
mad r2, r1, #0.25 | mad r6, r5, #0.25 | ld r3, [a1d2]
mul r1, r0, r3 | ld r7, [a1d3] | st r2, [01ba] | ld r3, [c071]
mul r5, r4, r7 | st r6, [01ce] | ld r2, [0314] | ld r7, [c071]
ld r6, [031e] | st r1, [01b6] | st r5, [01ca]
addxy r2, r2, #-4.75 | addxy r6, r6, #-4.75 | ld r1, [c074]
mul r1, r1, r2 | ld r5, [c074] | ld r0, [a1c2]
mul r5, r5, r6 | ld r4, [a1c3]
mac_p1wc3 r2, r0, r3, #0.125, r1 | ld r0, [a1da] | ld r3, [c070]
mac_p1wc3 r6, r4, r7, #0.125, r5 | ld r4, [a1db] | ld r7, [c070]
mad r2, r0, #0.125 | mad r6, r4, #0.125
mul r1, r2, #0.17328679513998632 | mul r5, r6, #0.17328679513998632
mov r0, #0.041666666666666664 | mov r4, #0.041666666666666664
ymacd r0, r1, #0.16666666666666666 | ymacd r4, r5, #0.16666666666666666
ymacd r0, r1, #0.5 | ymacd r4, r5, #0.5
ymacd r0, r1, #1.0 | ymacd r4, r5, #1.0
ymacd r0, r1, #1.0 | ymacd r4, r5, #1.0
mul r2, r0, r0 | mul r6, r4, r4
mul r1, r2, r2 | mul r5, r6, r6
mul r0, r1, r1 | mul r4, r5, r5
mul r2, r0, r0 | mul r6, r4, r4
mul r1, r2, r2 | mul r5, r6, r6
mul r0, r1, #6.103515625e-05 | mul r4, r5, #6.103515625e-05 | ld r1, [c07b]
mul r2, r3, r0 | mul r6, r7, r4 | ld r5, [c07b] | st r1, [01b2]
min r3, r2, #3.3569336665095761e-05 | st r5, [01c6] | ld r2, [01ad]
min r7, r6, #3.3569336665095761e-05 | shl r0, r3, 14 | ld r3, [01af] | ld r6, [01c1]
shl r4, r7, 14 | ld r7, [01c3]
mul r1, r0, r3 | mul r5, r4, r7
addxy r1, r1, r2 | addxy r5, r5, r6
mov r2, r1 | mov r6, r5 | st r1, [01b4] | ld r1, [01ba]
mad r2, r1, #-0.25 | st r5, [01c8] | ld r5, [01ce]
mad r6, r5, #-0.25 | ld r1, [01b2] | ld r5, [01c6]
mac_p1wc3 r2, r1, r3, #0.5, r2
mac_p1wc3 r6, r5, r7, #0.5, r6
mac_p1wc3 r2, r0, r2, #-0.5, r3 | ld r3, [01b4]
mac_p1wc3 r6, r4, r6, #-0.5, r7 | ld r7, [01c8] | st r0, [01b4]
mac_p1wc3 r3, r0, r2, #-0.5, r3 | st r4, [01c8]
mac_p1wc3 r7, r4, r6, #-0.5, r7 | st r3, [01b0]
mul r0, r3, #-2.0 | mul r4, r7, #-2.0 | st r7, [01c4]
st r0, [01ac] | st r4, [01c0]
mov r0, r3 | mov r4, r7 | ld r3, [01ba] | ld r7, [01ce]
mad r0, r3, #0.125 | mad r4, r7, #0.125
mac_p1wc3 r3, r1, r2, #0.5, r0 | ld r1, [01b4]
mac_p1wc3 r7, r5, r6, #0.5, r4 | ld r5, [01c8]
mac_p1wc3 r0, r1, r3, #-0.5, r2 | ld r3, [01b6] | ld r2, [01b0]
mac_p1wc3 r4, r5, r7, #-0.5, r6 | ld r7, [01ca] | ld r6, [01c4]
mul r1, r0, #4.0 | mul r5, r4, #4.0
mul r0, r3, r2 | mul r4, r7, r6 | st r1, [01ae] | ld r2, [a03c]
mul r1, r0, #-32.0 | mul r5, r4, #-32.0 | st r5, [01c2] | ld r6, [a03d]
st r1, [a153] | ld r3, [c057]
st r5, [a154] | ld r7, [c057]
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
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [0328]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [0332]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a044]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a045]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mul r2, r1, #-0.08664339756999316 | st r2, [032a] | ld r1, [032f]
mul r6, r5, #-0.08664339756999316 | mov r0, #0.16666666666666666 | st r6, [0334] | ld r5, [0339]
mov r4, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a04c]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a04d]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | st r2, [032c] | ld r2, [a044]
addxy r2, r1, r2 | st r6, [0336] | ld r6, [a045]
addxy r6, r5, r6
ph_wrap r1, r3, r2 | ld r2, [c056]
ph_wrap r5, r3, r6 | st r1, [032e] | ld r6, [c056]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [0338]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a134] | ld r1, [0350]
st r5, [a135] | ld r5, [0350]
ld r0, [a134]
mul r0, r0, r2 | ld r4, [a135] | ld r2, [0328]
mul r4, r4, r6 | mad r0, r1, r3 | ld r6, [0332] | st r2, [a1e4]
mad r4, r5, r7 | st r0, [0330] | st r6, [a1e5] | ld r0, [032a]
st r4, [033a] | ld r4, [0334] | st r0, [a1b4]
ld r1, [0330] | ld r5, [033a] | ld r2, [a03e]
ld r6, [a03f] | ld r3, [032c] | ld r7, [0336]
st r4, [a1b5]
st r1, [a1cc]
st r5, [a1cd]
st r2, [a166] | ld r2, [c04b]
st r6, [a167] | ld r6, [c04b]
ld r0, [a066]
ld r4, [a067]
ld r1, [a126]
mul r1, r1, r2 | ld r5, [a127] | ld r2, [c05e]
mul r5, r5, r6 | st r0, [a16e] | ld r6, [c05e]
st r4, [a16f]
ld r0, [a12e]
mad r1, r0, r2 | ld r4, [a12f] | ld r2, [c05f]
mad r5, r4, r6 | addxy r1, r1, r2 | ld r6, [c05f]
addxy r5, r5, r6 | mov r0, #0.041666666666666664 | ld r2, [a036]
addxy r2, r2, r1 | mov r4, #0.041666666666666664 | ld r6, [a037]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [033c] | st r3, [a1ec]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [0346] | st r7, [a1ed]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5 | ld r3, [c057]
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0 | ld r7, [c057]
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a046]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a047]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mul r2, r1, #-0.08664339756999316 | st r2, [033e] | ld r1, [0343]
mul r6, r5, #-0.08664339756999316 | mov r0, #0.16666666666666666 | st r6, [0348] | ld r5, [034d]
mov r4, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a04e]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a04f]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | st r2, [0340] | ld r2, [a046]
addxy r2, r1, r2 | st r6, [034a] | ld r6, [a047]
addxy r6, r5, r6
ph_wrap r1, r3, r2 | ld r2, [c056]
ph_wrap r5, r3, r6 | st r1, [0342] | ld r6, [c056]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [034c]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a136] | ld r1, [0350]
st r5, [a137] | ld r5, [0350]
ld r0, [a136]
mul r0, r0, r2 | ld r4, [a137] | ld r2, [033c]
mul r4, r4, r6 | mad r0, r1, r3 | ld r6, [0346] | ld r3, [0340]
mad r4, r5, r7 | st r0, [0344] | ld r0, [033e] | st r2, [a1e6]
st r4, [034e] | ld r4, [0348] | st r0, [a1b6]
ld r1, [0344] | ld r5, [034e]
ld r7, [034a] | st r4, [a1b7]
st r1, [a1ce]
st r5, [a1cf]
st r6, [a1e7]
st r3, [a1ee]
st r7, [a1ef]
