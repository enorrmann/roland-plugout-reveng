ld r0, [0343] | ld r2, [0341] | ld r1, [a120]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | ld r3, [c057]
ph_wrap r0, r3, r0 | ld r4, [c072]
addxy r2, r2, r0 | st r0, [0342] | ld r0, [a060]
ph_wrap r2, r3, r2 | st r0, [a168]
st r2, [0340] | ld r2, [a038]
st r2, [a160] | ld r2, [c04b]
mul r1, r1, r2 | ld r0, [a128] | ld r2, [c05e]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a030]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [0300] | ld r1, [0305]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a040]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0302] | ld r2, [a040]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c056]
mul r1, r1, #256.0 | st r1, [0304]
ph_clamp r1, r3, -r1
st r1, [a130] | ld r1, [0340]
ld r0, [a130]
mul r0, r0, r2 | ld r2, [c079]
mad r0, r1, r3 | ld r1, [c077]
mul r3, r1, r2 | st r0, [0306] | ld r0, [0302]
mul r7, r3, r4 | ld r3, [a138] | ld r4, [c075]
mul r1, r0, r7 | ld r7, [0181] | st r4, [0188]
mad r0, r7, #0.5
min r2, r1, #0.00390625
mul r1, r0, #2.0 | addxy r0, r3, #-0.49274435423331175
ph_wrap r7, r3, r1
mul r1, r0, #0.17583843797623297 | mov r0, #0.041666666666666664 | st r7, [0180]
mov r4, r0
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r1, r4, r4
mul r4, r1, r1
mul r1, r4, r4
mul r4, r1, r1 | addxy r1, r3, #-2.7058547537905633
mul r1, r3, r1
addxy r1, r1, #2.7976123302951308
mul r1, r3, r1
addxy r1, r1, #-1.4042515113978813
mul r3, r3, r1 | addxy r1, r7, #1.0
mul r4, r4, r1 | addxy r1, r3, #0.67161111008399732
addxy r3, r7, #-1.0
mul r7, r1, r3
mul r1, r7, #0.36269500851631165
addxy r3, r1, #0.5
min r7, r4, r3
addxy r1, r7, #0.125
mul r4, r1, #4.0
ph_tri r3, r3, -r4
mul r7, r3, r3
addxy r1, r7, #-8.8404402339953698
mul r4, r7, r1
addxy r7, r4, #21.601100021326904 | ld r4, [a140]
mul r1, r7, r3 | ld r3, [0183] | ld r7, [c076]
mad r2, r3, #0.00390625 | st r1, [018a]
mul r3, r2, #256.0 | mov r1, r0
ph_wrap r2, r3, r3
addxy r3, r4, #-0.49274435423331175 | st r2, [0182]
mul r3, r3, #0.17583843797623297
ymacd r1, r3, #0.16666666666666666
ymacd r1, r3, #0.5
ymacd r1, r3, #1.0
ymacd r1, r3, #1.0
mul r3, r1, r1
mul r1, r3, r3
mul r3, r1, r1
mul r1, r3, r3 | addxy r3, r4, #-2.7058547537905633
mul r3, r4, r3
addxy r3, r3, #2.7976123302951308
mul r3, r4, r3
addxy r3, r3, #-1.4042515113978813
mul r4, r4, r3 | addxy r3, r2, #1.0
mul r1, r1, r3 | addxy r3, r4, #0.67161111008399732
addxy r4, r2, #-1.0
mul r2, r3, r4
mul r3, r2, #0.36269500851631165
addxy r4, r3, #0.5
min r2, r1, r4
addxy r3, r2, #0.125
mul r1, r3, #4.0
ph_tri r4, r3, -r1
mul r2, r4, r4
addxy r3, r2, #-8.8404402339953698
mul r1, r2, r3
addxy r2, r1, #21.601100021326904 | ld r1, [0188]
mul r3, r2, r4 | ld r2, [018a]
mac_p1w r4, r1, r2, r7, r3 | ld r3, [a1b8] | ld r2, [0306]
mul r1, r4, #0.03125 | ld r7, [a1d0]
mul r4, r3, r7 | mad r1, r2, #-0.85969737115440648 | ld r7, [c074]
st r4, [018a] | ld r4, [0300] | ld r3, [c071]
addxy r4, r4, #-4.75 | ld r2, [a1c0]
mul r7, r7, r4
mac_p1wc3 r4, r2, r3, #0.125, r7 | ld r2, [a1d8] | ld r3, [c070]
mad r4, r2, #0.125
mul r7, r4, #0.17328679513998632
ymacd r0, r7, #0.16666666666666666
ymacd r0, r7, #0.5
ymacd r0, r7, #1.0
ymacd r0, r7, #1.0
mul r2, r0, r0
mul r4, r2, r2
mul r7, r4, r4
mul r0, r7, r7
mul r2, r0, r0 | ld r0, [c07b]
mul r4, r2, #6.103515625e-05
mul r7, r3, r4 | ld r4, [0185]
min r2, r7, #3.3569336665095761e-05 | ld r7, [0187]
shl r3, r2, 14
mul r2, r3, r7
addxy r2, r2, r4
mov r4, r2
mad r4, r1, #0.072700001299381256
mac_p1wc3 r4, r0, r7, #0.5, r4
mac_p1wc3 r4, r3, r4, #-0.5, r7
mac_p1wc3 r7, r3, r4, #-0.5, r2
mul r2, r7, #-2.0
mov r2, r7 | st r2, [0184]
mad r2, r1, #-0.036350000649690628
mac_p1wc3 r1, r0, r4, #0.5, r2
mac_p1wc3 r0, r3, r1, #-0.5, r4 | ld r3, [018a]
mul r2, r0, #4.0 | mul r1, r3, r7 | ld r0, [a061]
mul r4, r1, #-32.0 | st r2, [0186] | ld r2, [a039] | ld r3, [c057]
st r2, [a161] | ld r2, [c04b]
ld r1, [a121]
mul r1, r1, r2 | st r0, [a169] | ld r2, [c05e]
ld r0, [a129]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a031]
addxy r2, r2, r1 | st r4, [a151] | ld r4, [c072]
mul r2, r1, #0.08664339756999316 | st r2, [0308] | ld r1, [030d]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a041]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [030a] | ld r2, [a041]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c056]
mul r1, r1, #256.0 | st r1, [030c]
ph_clamp r1, r3, -r1
st r1, [a131] | ld r1, [0340]
ld r0, [a131]
mul r0, r0, r2 | ld r2, [c079]
mad r0, r1, r3 | ld r1, [c077]
mul r3, r1, r2 | st r0, [030e] | ld r0, [030a]
mul r7, r3, r4 | ld r3, [a139] | ld r4, [c075]
mul r1, r0, r7 | ld r7, [018d] | st r4, [0194]
mad r0, r7, #0.5
min r2, r1, #0.00390625
mul r1, r0, #2.0 | addxy r0, r3, #-0.49274435423331175
ph_wrap r7, r3, r1
mul r1, r0, #0.17583843797623297 | mov r0, #0.041666666666666664 | st r7, [018c]
mov r4, r0
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r1, r4, r4
mul r4, r1, r1
mul r1, r4, r4
mul r4, r1, r1 | addxy r1, r3, #-2.7058547537905633
mul r1, r3, r1
addxy r1, r1, #2.7976123302951308
mul r1, r3, r1
addxy r1, r1, #-1.4042515113978813
mul r3, r3, r1 | addxy r1, r7, #1.0
mul r4, r4, r1 | addxy r1, r3, #0.67161111008399732
addxy r3, r7, #-1.0
mul r7, r1, r3
mul r1, r7, #0.36269500851631165
addxy r3, r1, #0.5
min r7, r4, r3
addxy r1, r7, #0.125
mul r4, r1, #4.0
ph_tri r3, r3, -r4
mul r7, r3, r3
addxy r1, r7, #-8.8404402339953698
mul r4, r7, r1
addxy r7, r4, #21.601100021326904 | ld r4, [a141]
mul r1, r7, r3 | ld r3, [018f] | ld r7, [c076]
mad r2, r3, #0.00390625 | st r1, [0196]
mul r3, r2, #256.0 | mov r1, r0
ph_wrap r2, r3, r3
addxy r3, r4, #-0.49274435423331175 | st r2, [018e]
mul r3, r3, #0.17583843797623297
ymacd r1, r3, #0.16666666666666666
ymacd r1, r3, #0.5
ymacd r1, r3, #1.0
ymacd r1, r3, #1.0
mul r3, r1, r1
mul r1, r3, r3
mul r3, r1, r1
mul r1, r3, r3 | addxy r3, r4, #-2.7058547537905633
mul r3, r4, r3
addxy r3, r3, #2.7976123302951308
mul r3, r4, r3
addxy r3, r3, #-1.4042515113978813
mul r4, r4, r3 | addxy r3, r2, #1.0
mul r1, r1, r3 | addxy r3, r4, #0.67161111008399732
addxy r4, r2, #-1.0
mul r2, r3, r4
mul r3, r2, #0.36269500851631165
addxy r4, r3, #0.5
min r2, r1, r4
addxy r3, r2, #0.125
mul r1, r3, #4.0
ph_tri r4, r3, -r1
mul r2, r4, r4
addxy r3, r2, #-8.8404402339953698
mul r1, r2, r3
addxy r2, r1, #21.601100021326904 | ld r1, [0194]
mul r3, r2, r4 | ld r2, [0196]
mac_p1w r4, r1, r2, r7, r3 | ld r3, [a1b9] | ld r2, [030e]
mul r1, r4, #0.03125 | ld r7, [a1d1]
mul r4, r3, r7 | mad r1, r2, #-0.85969737115440648 | ld r7, [c074]
st r4, [0196] | ld r4, [0308] | ld r3, [c071]
addxy r4, r4, #-4.75 | ld r2, [a1c1]
mul r7, r7, r4
mac_p1wc3 r4, r2, r3, #0.125, r7 | ld r2, [a1d9] | ld r3, [c070]
mad r4, r2, #0.125
mul r7, r4, #0.17328679513998632
ymacd r0, r7, #0.16666666666666666
ymacd r0, r7, #0.5
ymacd r0, r7, #1.0
ymacd r0, r7, #1.0
mul r2, r0, r0
mul r4, r2, r2
mul r7, r4, r4
mul r0, r7, r7
mul r2, r0, r0 | ld r0, [c07b]
mul r4, r2, #6.103515625e-05
mul r7, r3, r4 | ld r4, [0191]
min r2, r7, #3.3569336665095761e-05 | ld r7, [0193]
shl r3, r2, 14
mul r2, r3, r7
addxy r2, r2, r4
mov r4, r2
mad r4, r1, #0.072700001299381256
mac_p1wc3 r4, r0, r7, #0.5, r4
mac_p1wc3 r4, r3, r4, #-0.5, r7
mac_p1wc3 r7, r3, r4, #-0.5, r2
mul r2, r7, #-2.0
mov r2, r7 | st r2, [0190]
mad r2, r1, #-0.036350000649690628
mac_p1wc3 r1, r0, r4, #0.5, r2
mac_p1wc3 r0, r3, r1, #-0.5, r4 | ld r3, [0196]
mul r2, r0, #4.0 | mul r1, r3, r7 | ld r0, [a062]
mul r4, r1, #-32.0 | st r2, [0192] | ld r2, [a03a] | ld r3, [c057]
st r2, [a162] | ld r2, [c04b]
ld r1, [a122]
mul r1, r1, r2 | st r0, [a16a] | ld r2, [c05e]
ld r0, [a12a]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a032]
addxy r2, r2, r1 | st r4, [a152] | ld r4, [c072]
mul r2, r1, #0.08664339756999316 | st r2, [0310] | ld r1, [0315]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a042]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0312] | ld r2, [a042]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c056]
mul r1, r1, #256.0 | st r1, [0314]
ph_clamp r1, r3, -r1
st r1, [a132] | ld r1, [0340]
ld r0, [a132]
mul r0, r0, r2 | ld r2, [c079]
mad r0, r1, r3 | ld r1, [c077]
mul r3, r1, r2 | st r0, [0316] | ld r0, [0312]
mul r7, r3, r4 | ld r3, [a13a] | ld r4, [c075]
mul r1, r0, r7 | ld r7, [0199] | st r4, [01a0]
mad r0, r7, #0.5
min r2, r1, #0.00390625
mul r1, r0, #2.0 | addxy r0, r3, #-0.49274435423331175
ph_wrap r7, r3, r1
mul r1, r0, #0.17583843797623297 | mov r0, #0.041666666666666664 | st r7, [0198]
mov r4, r0
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r1, r4, r4
mul r4, r1, r1
mul r1, r4, r4
mul r4, r1, r1 | addxy r1, r3, #-2.7058547537905633
mul r1, r3, r1
addxy r1, r1, #2.7976123302951308
mul r1, r3, r1
addxy r1, r1, #-1.4042515113978813
mul r3, r3, r1 | addxy r1, r7, #1.0
mul r4, r4, r1 | addxy r1, r3, #0.67161111008399732
addxy r3, r7, #-1.0
mul r7, r1, r3
mul r1, r7, #0.36269500851631165
addxy r3, r1, #0.5
min r7, r4, r3
addxy r1, r7, #0.125
mul r4, r1, #4.0
ph_tri r3, r3, -r4
mul r7, r3, r3
addxy r1, r7, #-8.8404402339953698
mul r4, r7, r1
addxy r7, r4, #21.601100021326904 | ld r4, [a142]
mul r1, r7, r3 | ld r3, [019b] | ld r7, [c076]
mad r2, r3, #0.00390625 | st r1, [01a2]
mul r3, r2, #256.0 | mov r1, r0
ph_wrap r2, r3, r3
addxy r3, r4, #-0.49274435423331175 | st r2, [019a]
mul r3, r3, #0.17583843797623297
ymacd r1, r3, #0.16666666666666666
ymacd r1, r3, #0.5
ymacd r1, r3, #1.0
ymacd r1, r3, #1.0
mul r3, r1, r1
mul r1, r3, r3
mul r3, r1, r1
mul r1, r3, r3 | addxy r3, r4, #-2.7058547537905633
mul r3, r4, r3
addxy r3, r3, #2.7976123302951308
mul r3, r4, r3
addxy r3, r3, #-1.4042515113978813
mul r4, r4, r3 | addxy r3, r2, #1.0
mul r1, r1, r3 | addxy r3, r4, #0.67161111008399732
addxy r4, r2, #-1.0
mul r2, r3, r4
mul r3, r2, #0.36269500851631165
addxy r4, r3, #0.5
min r2, r1, r4
addxy r3, r2, #0.125
mul r1, r3, #4.0
ph_tri r4, r3, -r1
mul r2, r4, r4
addxy r3, r2, #-8.8404402339953698
mul r1, r2, r3
addxy r2, r1, #21.601100021326904 | ld r1, [01a0]
mul r3, r2, r4 | ld r2, [01a2]
mac_p1w r4, r1, r2, r7, r3 | ld r3, [a1ba] | ld r2, [0316]
mul r1, r4, #0.03125 | ld r7, [a1d2]
mul r4, r3, r7 | mad r1, r2, #-0.85969737115440648 | ld r7, [c074]
st r4, [01a2] | ld r4, [0310] | ld r3, [c071]
addxy r4, r4, #-4.75 | ld r2, [a1c2]
mul r7, r7, r4
mac_p1wc3 r4, r2, r3, #0.125, r7 | ld r2, [a1da] | ld r3, [c070]
mad r4, r2, #0.125
mul r7, r4, #0.17328679513998632
ymacd r0, r7, #0.16666666666666666
ymacd r0, r7, #0.5
ymacd r0, r7, #1.0
ymacd r0, r7, #1.0
mul r2, r0, r0
mul r4, r2, r2
mul r7, r4, r4
mul r0, r7, r7
mul r2, r0, r0 | ld r0, [c07b]
mul r4, r2, #6.103515625e-05
mul r7, r3, r4 | ld r4, [019d]
min r2, r7, #3.3569336665095761e-05 | ld r7, [019f]
shl r3, r2, 14
mul r2, r3, r7
addxy r2, r2, r4
mov r4, r2
mad r4, r1, #0.072700001299381256
mac_p1wc3 r4, r0, r7, #0.5, r4
mac_p1wc3 r4, r3, r4, #-0.5, r7
mac_p1wc3 r7, r3, r4, #-0.5, r2
mul r2, r7, #-2.0
mov r2, r7 | st r2, [019c]
mad r2, r1, #-0.036350000649690628
mac_p1wc3 r1, r0, r4, #0.5, r2
mac_p1wc3 r0, r3, r1, #-0.5, r4 | ld r3, [01a2]
mul r2, r0, #4.0 | mul r1, r3, r7 | ld r0, [a063]
mul r4, r1, #-32.0 | st r2, [019e] | ld r2, [a03b] | ld r3, [c057]
st r2, [a163] | ld r2, [c04b]
ld r1, [a123]
mul r1, r1, r2 | st r0, [a16b] | ld r2, [c05e]
ld r0, [a12b]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a033]
addxy r2, r2, r1 | st r4, [a153] | ld r4, [c072]
mul r2, r1, #0.08664339756999316 | st r2, [0318] | ld r1, [031d]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a043]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [031a] | ld r2, [a043]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c056]
mul r1, r1, #256.0 | st r1, [031c]
ph_clamp r1, r3, -r1
st r1, [a133] | ld r1, [0340]
ld r0, [a133]
mul r0, r0, r2 | ld r2, [c079]
mad r0, r1, r3 | ld r1, [c077]
mul r3, r1, r2 | st r0, [031e] | ld r0, [031a]
mul r7, r3, r4 | ld r3, [a13b] | ld r4, [c075]
mul r1, r0, r7 | ld r7, [01a5] | st r4, [01ac]
mad r0, r7, #0.5
min r2, r1, #0.00390625
mul r1, r0, #2.0 | addxy r0, r3, #-0.49274435423331175
ph_wrap r7, r3, r1
mul r1, r0, #0.17583843797623297 | mov r0, #0.041666666666666664 | st r7, [01a4]
mov r4, r0
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r1, r4, r4
mul r4, r1, r1
mul r1, r4, r4
mul r4, r1, r1 | addxy r1, r3, #-2.7058547537905633
mul r1, r3, r1
addxy r1, r1, #2.7976123302951308
mul r1, r3, r1
addxy r1, r1, #-1.4042515113978813
mul r3, r3, r1 | addxy r1, r7, #1.0
mul r4, r4, r1 | addxy r1, r3, #0.67161111008399732
addxy r3, r7, #-1.0
mul r7, r1, r3
mul r1, r7, #0.36269500851631165
addxy r3, r1, #0.5
min r7, r4, r3
addxy r1, r7, #0.125
mul r4, r1, #4.0
ph_tri r3, r3, -r4
mul r7, r3, r3
addxy r1, r7, #-8.8404402339953698
mul r4, r7, r1
addxy r7, r4, #21.601100021326904 | ld r4, [a143]
mul r1, r7, r3 | ld r3, [01a7] | ld r7, [c076]
mad r2, r3, #0.00390625 | st r1, [01ae]
mul r3, r2, #256.0 | mov r1, r0
ph_wrap r2, r3, r3
addxy r3, r4, #-0.49274435423331175 | st r2, [01a6]
mul r3, r3, #0.17583843797623297
ymacd r1, r3, #0.16666666666666666
ymacd r1, r3, #0.5
ymacd r1, r3, #1.0
ymacd r1, r3, #1.0
mul r3, r1, r1
mul r1, r3, r3
mul r3, r1, r1
mul r1, r3, r3 | addxy r3, r4, #-2.7058547537905633
mul r3, r4, r3
addxy r3, r3, #2.7976123302951308
mul r3, r4, r3
addxy r3, r3, #-1.4042515113978813
mul r4, r4, r3 | addxy r3, r2, #1.0
mul r1, r1, r3 | addxy r3, r4, #0.67161111008399732
addxy r4, r2, #-1.0
mul r2, r3, r4
mul r3, r2, #0.36269500851631165
addxy r4, r3, #0.5
min r2, r1, r4
addxy r3, r2, #0.125
mul r1, r3, #4.0
ph_tri r4, r3, -r1
mul r2, r4, r4
addxy r3, r2, #-8.8404402339953698
mul r1, r2, r3
addxy r2, r1, #21.601100021326904 | ld r1, [01ac]
mul r3, r2, r4 | ld r2, [01ae]
mac_p1w r4, r1, r2, r7, r3 | ld r3, [a1bb] | ld r2, [031e]
mul r1, r4, #0.03125 | ld r7, [a1d3]
mul r4, r3, r7 | mad r1, r2, #-0.85969737115440648 | ld r7, [c074]
st r4, [01ae] | ld r4, [0318] | ld r3, [c071]
addxy r4, r4, #-4.75 | ld r2, [a1c3]
mul r7, r7, r4
mac_p1wc3 r4, r2, r3, #0.125, r7 | ld r2, [a1db] | ld r3, [c070]
mad r4, r2, #0.125
mul r7, r4, #0.17328679513998632
ymacd r0, r7, #0.16666666666666666
ymacd r0, r7, #0.5
ymacd r0, r7, #1.0
ymacd r0, r7, #1.0
mul r2, r0, r0
mul r4, r2, r2
mul r7, r4, r4
mul r0, r7, r7
mul r2, r0, r0 | ld r0, [c07b]
mul r4, r2, #6.103515625e-05
mul r7, r3, r4 | ld r4, [01a9]
min r2, r7, #3.3569336665095761e-05 | ld r7, [01ab]
shl r3, r2, 14
mul r2, r3, r7
addxy r2, r2, r4
mov r4, r2
mad r4, r1, #0.072700001299381256
mac_p1wc3 r4, r0, r7, #0.5, r4
mac_p1wc3 r4, r3, r4, #-0.5, r7
mac_p1wc3 r7, r3, r4, #-0.5, r2
mul r2, r7, #-2.0
mov r2, r7 | st r2, [01a8]
mad r2, r1, #-0.036350000649690628
mac_p1wc3 r1, r0, r4, #0.5, r2
mac_p1wc3 r0, r3, r1, #-0.5, r4 | ld r3, [01ae]
mul r2, r0, #4.0 | mul r1, r3, r7 | ld r0, [a064]
mul r4, r1, #-32.0 | st r2, [01aa] | ld r2, [a03c] | ld r3, [c057]
st r2, [a164] | ld r2, [c04b]
ld r1, [a124]
mul r1, r1, r2 | st r0, [a16c] | ld r2, [c05e]
ld r0, [a12c]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a034]
addxy r2, r2, r1 | st r4, [a154]
mul r2, r1, #0.08664339756999316 | st r2, [0320] | ld r1, [0325]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a044]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0322] | ld r2, [a044]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c056]
mul r1, r1, #256.0 | st r1, [0324]
ph_clamp r1, r3, -r1
st r1, [a134] | ld r1, [0340]
ld r0, [a134]
mul r0, r0, r2 | ld r2, [0320]
mad r0, r1, r3 | st r2, [a1e4] | ld r3, [c057]
st r0, [0326] | ld r0, [0322]
ld r1, [0326] | ld r2, [a03d]
st r0, [a1b4]
st r1, [a1cc]
st r2, [a165] | ld r2, [c04b]
ld r0, [a065]
ld r1, [a125]
mul r1, r1, r2 | st r0, [a16d] | ld r2, [c05e]
ld r0, [a12d]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a035]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [0328] | ld r1, [032d]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a045]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [032a] | ld r2, [a045]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c056]
mul r1, r1, #256.0 | st r1, [032c]
ph_clamp r1, r3, -r1
st r1, [a135] | ld r1, [0340]
ld r0, [a135]
mul r0, r0, r2 | ld r2, [0328]
mad r0, r1, r3 | st r2, [a1e5] | ld r3, [c057]
st r0, [032e] | ld r0, [032a]
ld r1, [032e] | ld r2, [a03e]
st r0, [a1b5]
st r1, [a1cd]
st r2, [a166] | ld r2, [c04b]
ld r0, [a066]
ld r1, [a126]
mul r1, r1, r2 | st r0, [a16e] | ld r2, [c05e]
ld r0, [a12e]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a036]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [0330] | ld r1, [0335]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a046]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0332] | ld r2, [a046]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c056]
mul r1, r1, #256.0 | st r1, [0334]
ph_clamp r1, r3, -r1
st r1, [a136] | ld r1, [0340]
ld r0, [a136]
mul r0, r0, r2 | ld r2, [0330]
mad r0, r1, r3 | st r2, [a1e6] | ld r3, [c057]
st r0, [0336] | ld r0, [0332]
ld r1, [0336] | ld r2, [a03f]
st r0, [a1b6]
st r1, [a1ce]
st r2, [a167] | ld r2, [c04b]
ld r0, [a067]
ld r1, [a127]
mul r1, r1, r2 | st r0, [a16f] | ld r2, [c05e]
ld r0, [a12f]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a037]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [0338] | ld r1, [033d]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a047]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [033a] | ld r2, [a047]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c056]
mul r1, r1, #256.0 | st r1, [033c]
ph_clamp r1, r3, -r1
st r1, [a137] | ld r1, [0340]
ld r0, [a137]
mul r0, r0, r2 | ld r2, [0338]
mad r0, r1, r3 | st r2, [a1e7]
st r0, [033e] | ld r0, [033a]
ld r1, [033e] | st r0, [a1b7]
st r1, [a1cf]
