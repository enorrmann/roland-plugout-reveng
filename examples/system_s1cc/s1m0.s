ld r0, [033f] | ld r2, [033d] | ld r1, [a120]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | ld r4, [c070]
ph_wrap r0, r3, r0
addxy r2, r2, r0 | st r0, [033e] | ld r0, [a060]
ph_wrap r2, r3, r2 | st r0, [a168]
st r2, [033c] | ld r2, [a038]
st r2, [a160] | ld r2, [c04b]
mul r1, r1, r2 | ld r0, [a128] | ld r2, [c05e]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a030]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [a1b0] | ld r1, [0303]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a040]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0300] | ld r2, [a040]
addxy r2, r1, r2 | ld r0, [0300]
ph_wrap r1, r3, r2 | ld r2, [c076]
mul r1, r1, #256.0 | st r1, [0302]
ph_clamp r1, r3, -r1
st r1, [a130] | ld r1, [c074]
mul r3, r1, r2
mul r7, r3, r4 | ld r4, [0307]
mul r1, r0, r7 | mad r0, r4, #0.5
shl r2, r1, 9
min r3, r2, #2.0 | ld r2, [a138]
mul r7, r0, #2.0 | addxy r4, r2, #-0.49274435423331175
ph_wrap r1, r3, r7
mov r7, #0.041666666666666664 | mul r0, r4, #0.17583843797623297 | st r1, [0306]
mov r4, r7
ymacd r4, r0, #0.16666666666666666
ymacd r4, r0, #0.5
ymacd r4, r0, #1.0
ymacd r4, r0, #1.0
mul r0, r4, r4
mul r4, r0, r0
mul r0, r4, r4
mul r4, r0, r0 | addxy r0, r2, #-2.7058547537905633
mul r0, r2, r0
addxy r0, r0, #2.7976123302951308
mul r0, r2, r0
addxy r0, r0, #-1.4042515113978813
mul r2, r2, r0 | addxy r0, r1, #1.0
mul r0, r0, r4 | addxy r4, r2, #0.67161111008399732
addxy r2, r1, #-1.0
mul r1, r4, r2
mul r4, r1, #0.36269500851631165
addxy r2, r4, #0.5
min r1, r0, r2
addxy r4, r1, #0.125
mul r0, r4, #4.0
ph_tri r2, r3, -r0
mul r1, r2, r2
addxy r4, r1, #-8.8404402339953698
mul r0, r1, r4
addxy r1, r0, #21.601100021326904 | ld r0, [c072]
mul r4, r1, r2 | ld r1, [0309]
mad r1, r3, #0.5 | ld r3, [a140]
ph_wrap r2, r3, r1
addxy r1, r3, #-0.49274435423331175 | st r2, [0308]
mul r1, r1, #0.17583843797623297
ymacd r7, r1, #0.16666666666666666
ymacd r7, r1, #0.5
ymacd r7, r1, #1.0
ymacd r7, r1, #1.0
mul r1, r7, r7
mul r7, r1, r1
mul r1, r7, r7
mul r7, r1, r1 | addxy r1, r3, #-2.7058547537905633
mul r1, r3, r1
addxy r1, r1, #2.7976123302951308
mul r1, r3, r1
addxy r1, r1, #-1.4042515113978813
mul r3, r3, r1 | addxy r1, r2, #1.0
mul r1, r1, r7 | addxy r7, r3, #0.67161111008399732
addxy r3, r2, #-1.0
mul r2, r7, r3
mul r7, r2, #0.36269500851631165
addxy r3, r7, #0.5
min r2, r1, r3
addxy r7, r2, #0.125
mul r1, r7, #4.0
ph_tri r3, r3, -r1
mul r2, r3, r3
addxy r7, r2, #-8.8404402339953698
mul r1, r2, r7
addxy r2, r1, #21.601100021326904 | ld r1, [c073]
mul r7, r2, r3
mac_p1w r2, r7, r1, r0, r4 | ld r1, [a130] | ld r4, [c070]
mul r3, r2, #-0.036350000649690628 | ld r2, [c056]
st r3, [0304]
ld r0, [0304]
mad r0, r1, r2 | ld r1, [033c] | ld r2, [c057]
mad r0, r1, r2 | ld r2, [a039]
st r0, [a151]
st r2, [a161] | ld r2, [c04b]
ld r0, [a061]
ld r1, [a121]
mul r1, r1, r2 | st r0, [a169] | ld r2, [c05e]
ld r0, [a129]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a031]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [a1b1] | ld r1, [030d]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a041]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [030a] | ld r2, [a041]
addxy r2, r1, r2 | ld r0, [030a]
ph_wrap r1, r3, r2 | ld r2, [c076]
mul r1, r1, #256.0 | st r1, [030c]
ph_clamp r1, r3, -r1
st r1, [a131] | ld r1, [c074]
mul r3, r1, r2
mul r7, r3, r4 | ld r4, [0311]
mul r1, r0, r7 | mad r0, r4, #0.5
shl r2, r1, 9
min r3, r2, #2.0 | ld r2, [a139]
mul r7, r0, #2.0 | addxy r4, r2, #-0.49274435423331175
ph_wrap r1, r3, r7
mov r7, #0.041666666666666664 | mul r0, r4, #0.17583843797623297 | st r1, [0310]
mov r4, r7
ymacd r4, r0, #0.16666666666666666
ymacd r4, r0, #0.5
ymacd r4, r0, #1.0
ymacd r4, r0, #1.0
mul r0, r4, r4
mul r4, r0, r0
mul r0, r4, r4
mul r4, r0, r0 | addxy r0, r2, #-2.7058547537905633
mul r0, r2, r0
addxy r0, r0, #2.7976123302951308
mul r0, r2, r0
addxy r0, r0, #-1.4042515113978813
mul r2, r2, r0 | addxy r0, r1, #1.0
mul r0, r0, r4 | addxy r4, r2, #0.67161111008399732
addxy r2, r1, #-1.0
mul r1, r4, r2
mul r4, r1, #0.36269500851631165
addxy r2, r4, #0.5
min r1, r0, r2
addxy r4, r1, #0.125
mul r0, r4, #4.0
ph_tri r2, r3, -r0
mul r1, r2, r2
addxy r4, r1, #-8.8404402339953698
mul r0, r1, r4
addxy r1, r0, #21.601100021326904 | ld r0, [c072]
mul r4, r1, r2 | ld r1, [0313]
mad r1, r3, #0.5 | ld r3, [a141]
ph_wrap r2, r3, r1
addxy r1, r3, #-0.49274435423331175 | st r2, [0312]
mul r1, r1, #0.17583843797623297
ymacd r7, r1, #0.16666666666666666
ymacd r7, r1, #0.5
ymacd r7, r1, #1.0
ymacd r7, r1, #1.0
mul r1, r7, r7
mul r7, r1, r1
mul r1, r7, r7
mul r7, r1, r1 | addxy r1, r3, #-2.7058547537905633
mul r1, r3, r1
addxy r1, r1, #2.7976123302951308
mul r1, r3, r1
addxy r1, r1, #-1.4042515113978813
mul r3, r3, r1 | addxy r1, r2, #1.0
mul r1, r1, r7 | addxy r7, r3, #0.67161111008399732
addxy r3, r2, #-1.0
mul r2, r7, r3
mul r7, r2, #0.36269500851631165
addxy r3, r7, #0.5
min r2, r1, r3
addxy r7, r2, #0.125
mul r1, r7, #4.0
ph_tri r3, r3, -r1
mul r2, r3, r3
addxy r7, r2, #-8.8404402339953698
mul r1, r2, r7
addxy r2, r1, #21.601100021326904 | ld r1, [c073]
mul r7, r2, r3
mac_p1w r2, r7, r1, r0, r4 | ld r1, [a131] | ld r4, [c070]
mul r3, r2, #-0.036350000649690628 | ld r2, [c056]
st r3, [030e]
ld r0, [030e]
mad r0, r1, r2 | ld r1, [033c] | ld r2, [c057]
mad r0, r1, r2 | ld r2, [a03a]
st r0, [a152]
st r2, [a162] | ld r2, [c04b]
ld r0, [a062]
ld r1, [a122]
mul r1, r1, r2 | st r0, [a16a] | ld r2, [c05e]
ld r0, [a12a]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a032]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [a1b2] | ld r1, [0317]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a042]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0314] | ld r2, [a042]
addxy r2, r1, r2 | ld r0, [0314]
ph_wrap r1, r3, r2 | ld r2, [c076]
mul r1, r1, #256.0 | st r1, [0316]
ph_clamp r1, r3, -r1
st r1, [a132] | ld r1, [c074]
mul r3, r1, r2
mul r7, r3, r4 | ld r4, [031b]
mul r1, r0, r7 | mad r0, r4, #0.5
shl r2, r1, 9
min r3, r2, #2.0 | ld r2, [a13a]
mul r7, r0, #2.0 | addxy r4, r2, #-0.49274435423331175
ph_wrap r1, r3, r7
mov r7, #0.041666666666666664 | mul r0, r4, #0.17583843797623297 | st r1, [031a]
mov r4, r7
ymacd r4, r0, #0.16666666666666666
ymacd r4, r0, #0.5
ymacd r4, r0, #1.0
ymacd r4, r0, #1.0
mul r0, r4, r4
mul r4, r0, r0
mul r0, r4, r4
mul r4, r0, r0 | addxy r0, r2, #-2.7058547537905633
mul r0, r2, r0
addxy r0, r0, #2.7976123302951308
mul r0, r2, r0
addxy r0, r0, #-1.4042515113978813
mul r2, r2, r0 | addxy r0, r1, #1.0
mul r0, r0, r4 | addxy r4, r2, #0.67161111008399732
addxy r2, r1, #-1.0
mul r1, r4, r2
mul r4, r1, #0.36269500851631165
addxy r2, r4, #0.5
min r1, r0, r2
addxy r4, r1, #0.125
mul r0, r4, #4.0
ph_tri r2, r3, -r0
mul r1, r2, r2
addxy r4, r1, #-8.8404402339953698
mul r0, r1, r4
addxy r1, r0, #21.601100021326904 | ld r0, [c072]
mul r4, r1, r2 | ld r1, [031d]
mad r1, r3, #0.5 | ld r3, [a142]
ph_wrap r2, r3, r1
addxy r1, r3, #-0.49274435423331175 | st r2, [031c]
mul r1, r1, #0.17583843797623297
ymacd r7, r1, #0.16666666666666666
ymacd r7, r1, #0.5
ymacd r7, r1, #1.0
ymacd r7, r1, #1.0
mul r1, r7, r7
mul r7, r1, r1
mul r1, r7, r7
mul r7, r1, r1 | addxy r1, r3, #-2.7058547537905633
mul r1, r3, r1
addxy r1, r1, #2.7976123302951308
mul r1, r3, r1
addxy r1, r1, #-1.4042515113978813
mul r3, r3, r1 | addxy r1, r2, #1.0
mul r1, r1, r7 | addxy r7, r3, #0.67161111008399732
addxy r3, r2, #-1.0
mul r2, r7, r3
mul r7, r2, #0.36269500851631165
addxy r3, r7, #0.5
min r2, r1, r3
addxy r7, r2, #0.125
mul r1, r7, #4.0
ph_tri r3, r3, -r1
mul r2, r3, r3
addxy r7, r2, #-8.8404402339953698
mul r1, r2, r7
addxy r2, r1, #21.601100021326904 | ld r1, [c073]
mul r7, r2, r3
mac_p1w r2, r7, r1, r0, r4 | ld r1, [a132] | ld r4, [c070]
mul r3, r2, #-0.036350000649690628 | ld r2, [c056]
st r3, [0318]
ld r0, [0318]
mad r0, r1, r2 | ld r1, [033c] | ld r2, [c057]
mad r0, r1, r2 | ld r2, [a03b]
st r0, [a153]
st r2, [a163] | ld r2, [c04b]
ld r0, [a063]
ld r1, [a123]
mul r1, r1, r2 | st r0, [a16b] | ld r2, [c05e]
ld r0, [a12b]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a033]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [a1b3] | ld r1, [0321]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a043]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [031e] | ld r2, [a043]
addxy r2, r1, r2 | ld r0, [031e]
ph_wrap r1, r3, r2 | ld r2, [c076]
mul r1, r1, #256.0 | st r1, [0320]
ph_clamp r1, r3, -r1
st r1, [a133] | ld r1, [c074]
mul r3, r1, r2
mul r7, r3, r4 | ld r4, [0325]
mul r1, r0, r7 | mad r0, r4, #0.5
shl r2, r1, 9
min r3, r2, #2.0 | ld r2, [a13b]
mul r7, r0, #2.0 | addxy r4, r2, #-0.49274435423331175
ph_wrap r1, r3, r7
mov r7, #0.041666666666666664 | mul r0, r4, #0.17583843797623297 | st r1, [0324]
mov r4, r7
ymacd r4, r0, #0.16666666666666666
ymacd r4, r0, #0.5
ymacd r4, r0, #1.0
ymacd r4, r0, #1.0
mul r0, r4, r4
mul r4, r0, r0
mul r0, r4, r4
mul r4, r0, r0 | addxy r0, r2, #-2.7058547537905633
mul r0, r2, r0
addxy r0, r0, #2.7976123302951308
mul r0, r2, r0
addxy r0, r0, #-1.4042515113978813
mul r2, r2, r0 | addxy r0, r1, #1.0
mul r0, r0, r4 | addxy r4, r2, #0.67161111008399732
addxy r2, r1, #-1.0
mul r1, r4, r2
mul r4, r1, #0.36269500851631165
addxy r2, r4, #0.5
min r1, r0, r2
addxy r4, r1, #0.125
mul r0, r4, #4.0
ph_tri r2, r3, -r0
mul r1, r2, r2
addxy r4, r1, #-8.8404402339953698
mul r0, r1, r4
addxy r1, r0, #21.601100021326904 | ld r0, [c072]
mul r4, r1, r2 | ld r1, [0327]
mad r1, r3, #0.5 | ld r3, [a143]
ph_wrap r2, r3, r1
addxy r1, r3, #-0.49274435423331175 | st r2, [0326]
mul r1, r1, #0.17583843797623297
ymacd r7, r1, #0.16666666666666666
ymacd r7, r1, #0.5
ymacd r7, r1, #1.0
ymacd r7, r1, #1.0
mul r1, r7, r7
mul r7, r1, r1
mul r1, r7, r7
mul r7, r1, r1 | addxy r1, r3, #-2.7058547537905633
mul r1, r3, r1
addxy r1, r1, #2.7976123302951308
mul r1, r3, r1
addxy r1, r1, #-1.4042515113978813
mul r3, r3, r1 | addxy r1, r2, #1.0
mul r1, r1, r7 | addxy r7, r3, #0.67161111008399732
addxy r3, r2, #-1.0
mul r2, r7, r3
mul r7, r2, #0.36269500851631165
addxy r3, r7, #0.5
min r2, r1, r3
addxy r7, r2, #0.125
mul r1, r7, #4.0
ph_tri r3, r3, -r1
mul r2, r3, r3
addxy r7, r2, #-8.8404402339953698
mul r1, r2, r7
addxy r2, r1, #21.601100021326904 | ld r1, [c073]
mul r7, r2, r3
mac_p1w r2, r7, r1, r0, r4 | ld r1, [a133] | ld r4, [c070]
mul r3, r2, #-0.036350000649690628 | ld r2, [c056]
st r3, [0322]
ld r0, [0322]
mad r0, r1, r2 | ld r1, [033c] | ld r2, [c057]
mad r0, r1, r2 | ld r2, [a03c]
st r0, [a154]
st r2, [a164] | ld r2, [c04b]
ld r0, [a064]
ld r1, [a124]
mul r1, r1, r2 | st r0, [a16c] | ld r2, [c05e]
ld r0, [a12c]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a034]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [a1b4] | ld r1, [032b]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a044]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0328] | ld r2, [a044]
addxy r2, r1, r2 | ld r0, [0328]
ph_wrap r1, r3, r2 | ld r2, [c076]
mul r1, r1, #256.0 | st r1, [032a]
ph_clamp r1, r3, -r1
st r1, [a134] | ld r1, [c074]
mul r3, r1, r2
mul r7, r3, r4 | ld r4, [032f]
mul r1, r0, r7 | mad r0, r4, #0.5
shl r2, r1, 9
min r3, r2, #2.0 | ld r2, [a13c]
mul r7, r0, #2.0 | addxy r4, r2, #-0.49274435423331175
ph_wrap r1, r3, r7
mov r7, #0.041666666666666664 | mul r0, r4, #0.17583843797623297 | st r1, [032e]
mov r4, r7
ymacd r4, r0, #0.16666666666666666
ymacd r4, r0, #0.5
ymacd r4, r0, #1.0
ymacd r4, r0, #1.0
mul r0, r4, r4
mul r4, r0, r0
mul r0, r4, r4
mul r4, r0, r0 | addxy r0, r2, #-2.7058547537905633
mul r0, r2, r0
addxy r0, r0, #2.7976123302951308
mul r0, r2, r0
addxy r0, r0, #-1.4042515113978813
mul r2, r2, r0 | addxy r0, r1, #1.0
mul r0, r0, r4 | addxy r4, r2, #0.67161111008399732
addxy r2, r1, #-1.0
mul r1, r4, r2
mul r4, r1, #0.36269500851631165
addxy r2, r4, #0.5
min r1, r0, r2
addxy r4, r1, #0.125
mul r0, r4, #4.0
ph_tri r2, r3, -r0
mul r1, r2, r2
addxy r4, r1, #-8.8404402339953698
mul r0, r1, r4
addxy r1, r0, #21.601100021326904 | ld r0, [c072]
mul r4, r1, r2 | ld r1, [0331]
mad r1, r3, #0.5 | ld r3, [a144]
ph_wrap r2, r3, r1
addxy r1, r3, #-0.49274435423331175 | st r2, [0330]
mul r1, r1, #0.17583843797623297
ymacd r7, r1, #0.16666666666666666
ymacd r7, r1, #0.5
ymacd r7, r1, #1.0
ymacd r7, r1, #1.0
mul r1, r7, r7
mul r7, r1, r1
mul r1, r7, r7
mul r7, r1, r1 | addxy r1, r3, #-2.7058547537905633
mul r1, r3, r1
addxy r1, r1, #2.7976123302951308
mul r1, r3, r1
addxy r1, r1, #-1.4042515113978813
mul r3, r3, r1 | addxy r1, r2, #1.0
mul r1, r1, r7 | addxy r7, r3, #0.67161111008399732
addxy r3, r2, #-1.0
mul r2, r7, r3
mul r7, r2, #0.36269500851631165
addxy r3, r7, #0.5
min r2, r1, r3
addxy r7, r2, #0.125
mul r1, r7, #4.0
ph_tri r3, r3, -r1
mul r2, r3, r3
addxy r7, r2, #-8.8404402339953698
mul r1, r2, r7
addxy r2, r1, #21.601100021326904 | ld r1, [c073]
mul r7, r2, r3
mac_p1w r2, r7, r1, r0, r4 | ld r1, [a134] | ld r4, [c070]
mul r3, r2, #-0.036350000649690628 | ld r2, [c056]
st r3, [032c]
ld r0, [032c]
mad r0, r1, r2 | ld r1, [033c] | ld r2, [c057]
mad r0, r1, r2 | ld r2, [a03d]
st r0, [a155]
st r2, [a165] | ld r2, [c04b]
ld r0, [a065]
ld r1, [a125]
mul r1, r1, r2 | st r0, [a16d] | ld r2, [c05e]
ld r0, [a12d]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a035]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [a1b5] | ld r1, [0335]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a045]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0332] | ld r2, [a045]
addxy r2, r1, r2 | ld r0, [0332]
ph_wrap r1, r3, r2 | ld r2, [c076]
mul r1, r1, #256.0 | st r1, [0334]
ph_clamp r1, r3, -r1
st r1, [a135] | ld r1, [c074]
mul r3, r1, r2
mul r7, r3, r4 | ld r4, [0339]
mul r1, r0, r7 | mad r0, r4, #0.5
shl r2, r1, 9
min r3, r2, #2.0 | ld r2, [a13d]
mul r7, r0, #2.0 | addxy r4, r2, #-0.49274435423331175
ph_wrap r1, r3, r7
mov r7, #0.041666666666666664 | mul r0, r4, #0.17583843797623297 | st r1, [0338]
mov r4, r7
ymacd r4, r0, #0.16666666666666666
ymacd r4, r0, #0.5
ymacd r4, r0, #1.0
ymacd r4, r0, #1.0
mul r0, r4, r4
mul r4, r0, r0
mul r0, r4, r4
mul r4, r0, r0 | addxy r0, r2, #-2.7058547537905633
mul r0, r2, r0
addxy r0, r0, #2.7976123302951308
mul r0, r2, r0
addxy r0, r0, #-1.4042515113978813
mul r2, r2, r0 | addxy r0, r1, #1.0
mul r0, r0, r4 | addxy r4, r2, #0.67161111008399732
addxy r2, r1, #-1.0
mul r1, r4, r2
mul r4, r1, #0.36269500851631165
addxy r2, r4, #0.5
min r1, r0, r2
addxy r4, r1, #0.125
mul r0, r4, #4.0
ph_tri r2, r3, -r0
mul r1, r2, r2
addxy r4, r1, #-8.8404402339953698
mul r0, r1, r4
addxy r1, r0, #21.601100021326904 | ld r0, [c072]
mul r4, r1, r2 | ld r1, [033b]
mad r1, r3, #0.5 | ld r3, [a145]
ph_wrap r2, r3, r1
addxy r1, r3, #-0.49274435423331175 | st r2, [033a]
mul r1, r1, #0.17583843797623297
ymacd r7, r1, #0.16666666666666666
ymacd r7, r1, #0.5
ymacd r7, r1, #1.0
ymacd r7, r1, #1.0
mul r1, r7, r7
mul r7, r1, r1
mul r1, r7, r7
mul r7, r1, r1 | addxy r1, r3, #-2.7058547537905633
mul r1, r3, r1
addxy r1, r1, #2.7976123302951308
mul r1, r3, r1
addxy r1, r1, #-1.4042515113978813
mul r3, r3, r1 | addxy r1, r2, #1.0
mul r1, r1, r7 | addxy r7, r3, #0.67161111008399732
addxy r3, r2, #-1.0
mul r2, r7, r3
mul r7, r2, #0.36269500851631165
addxy r3, r7, #0.5
min r2, r1, r3
addxy r7, r2, #0.125
mul r1, r7, #4.0
ph_tri r3, r3, -r1
mul r2, r3, r3
addxy r7, r2, #-8.8404402339953698
mul r1, r2, r7
addxy r2, r1, #21.601100021326904 | ld r1, [c073]
mul r7, r2, r3
mac_p1w r2, r7, r1, r0, r4 | ld r1, [a135]
mul r3, r2, #-0.036350000649690628 | ld r2, [c056]
st r3, [0336]
ld r0, [0336]
mad r0, r1, r2 | ld r1, [033c] | ld r2, [c057]
mad r0, r1, r2
st r0, [a156]
