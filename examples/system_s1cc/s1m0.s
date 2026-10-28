ld r0, [0367] | ld r2, [0365] | ld r1, [a120]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | ld r3, [c076]
ph_wrap r0, r3, r0 | ld r7, [c070]
addxy r2, r2, r0 | st r0, [0366] | ld r0, [a060]
ph_wrap r2, r3, r2 | st r0, [a168]
st r2, [0364] | ld r2, [a038]
st r2, [a160] | ld r2, [c04b]
mul r1, r1, r2 | ld r0, [a128] | ld r2, [c05e]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a030]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [a1b0]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a040]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [0300] | ld r1, [0305]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a048]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | ld r0, [0300]
st r2, [0302] | ld r2, [a040]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c074]
mul r4, r2, r3 | mul r1, r1, #256.0 | st r1, [0304] | ld r3, [c075]
mul r2, r4, r7 | ph_clamp r1, r3, -r1 | ld r4, [c077]
mul r7, r3, r4 | ld r3, [c071] | st r1, [a130] | ld r1, [0302]
mul r4, r7, r3 | mul r7, r0, r2 | st r1, [0312]
mul r2, r1, r4 | addxy r4, r0, #-0.004999999888241291
min r3, r7, #0.00390625 | ld r7, [a138] | st r2, [030c]
mul r4, r4, #5.479451911627053 | mov r2, r0 | st r3, [030e]
max r7, r7, r4 | ld r4, [0309]
mad r2, r4, #0.5 | mul r3, r0, #2.0
mul r4, r2, #2.0 | mul r0, r0, #-2.0
ph_wrap r2, r3, r4
addxy r4, r2, #1.0 | st r2, [0308]
ph_wrap r4, r3, r4 | st r2, [0310]
min r2, r4, r3
max r2, r2, r0
mul r2, r1, r2
shl r2, r2, -r5
max r1, r2, -r2
addxy r1, r1, #-2.0
mul r2, r2, r1
addxy r1, r2, r4 | mov r2, r7 | ld r4, [0310]
mad r2, r4, #-0.68493148895338163
addxy r4, r2, #-0.65753423000762279
mul r2, r4, #-1.4600000381469727
ph_wrap r4, r3, r2
min r2, r4, r3
max r3, r2, r0 | ld r2, [0312]
mul r0, r2, r3
shl r2, r0, -r5
max r3, r2, -r2
addxy r0, r3, #-2.0
mul r3, r2, r0
addxy r2, r3, r4
subxy r0, r2, r1 | addxy r3, r1, #1.0
mad r7, r0, #-0.68493148895338163 | ld r0, [c072]
addxy r4, r7, #0.027397258945758795
mul r2, r3, r4 | ld r4, [030e] | ld r3, [a140]
mad r1, r2, #-0.73000001907348633 | addxy r2, r4, #-3.9062499126885086e-05
mul r7, r0, r1
shl r0, r2, -r1 | st r7, [0312]
mul r1, r0, #1.3698629779067633 | ld r0, [030b]
max r2, r3, r1
mov r3, r4
mad r3, r0, #0.00390625
mul r1, r3, #256.0
ph_wrap r0, r3, r1
addxy r3, r0, #1.0 | st r0, [030a]
ph_wrap r1, r3, r3
mul r3, r4, #256.0 | mul r4, r4, #-256.0
min r7, r1, r3 | st r4, [030e]
max r7, r7, r4 | ld r4, [030c]
mul r7, r4, r7
shl r7, r7, -r7
shl r7, r7, r4
max r4, r7, -r7
addxy r4, r4, #-1.0
mul r7, r7, r4 | mov r4, r2
mad r4, r0, #-0.68493148895338163 | mad r7, r1, #0.25
addxy r1, r4, #-0.65753423000762279
mul r0, r1, #-1.4600000381469727
ph_wrap r4, r3, r0 | ld r0, [030e]
min r1, r4, r3
max r3, r1, r0 | ld r1, [030c]
mul r0, r1, r3
shl r1, r0, -r7
shl r3, r1, r4
max r0, r3, -r3
addxy r1, r0, #-1.0
mul r0, r3, r1 | addxy r1, r7, #0.25
mad r0, r4, #0.25
subxy r3, r0, r7
mad r3, r2, #-0.36500000953674316 | ld r2, [c073]
addxy r4, r3, #-0.0099999997764825821
mul r0, r1, r4 | ld r1, [0312]
mad r0, r7, #0.5 | ld r7, [c070]
mul r3, r0, r2 | ld r2, [c056]
mad r3, r1, #0.125 | ld r1, [a130]
mul r4, r3, #4.0 | ld r3, [c076]
st r4, [0306]
ld r0, [0306]
mad r0, r1, r2 | ld r1, [0364] | ld r2, [c057]
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
mul r2, r1, #0.08664339756999316 | st r2, [a1b1]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a041]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [0314] | ld r1, [0319]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a049]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | ld r0, [0314]
st r2, [0316] | ld r2, [a041]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c074]
mul r4, r2, r3 | mul r1, r1, #256.0 | st r1, [0318] | ld r3, [c075]
mul r2, r4, r7 | ph_clamp r1, r3, -r1 | ld r4, [c077]
mul r7, r3, r4 | ld r3, [c071] | st r1, [a131] | ld r1, [0316]
mul r4, r7, r3 | mul r7, r0, r2 | st r1, [0326]
mul r2, r1, r4 | addxy r4, r0, #-0.004999999888241291
min r3, r7, #0.00390625 | ld r7, [a139] | st r2, [0320]
mul r4, r4, #5.479451911627053 | mov r2, r0 | st r3, [0322]
max r7, r7, r4 | ld r4, [031d]
mad r2, r4, #0.5 | mul r3, r0, #2.0
mul r4, r2, #2.0 | mul r0, r0, #-2.0
ph_wrap r2, r3, r4
addxy r4, r2, #1.0 | st r2, [031c]
ph_wrap r4, r3, r4 | st r2, [0324]
min r2, r4, r3
max r2, r2, r0
mul r2, r1, r2
shl r2, r2, -r5
max r1, r2, -r2
addxy r1, r1, #-2.0
mul r2, r2, r1
addxy r1, r2, r4 | mov r2, r7 | ld r4, [0324]
mad r2, r4, #-0.68493148895338163
addxy r4, r2, #-0.65753423000762279
mul r2, r4, #-1.4600000381469727
ph_wrap r4, r3, r2
min r2, r4, r3
max r3, r2, r0 | ld r2, [0326]
mul r0, r2, r3
shl r2, r0, -r5
max r3, r2, -r2
addxy r0, r3, #-2.0
mul r3, r2, r0
addxy r2, r3, r4
subxy r0, r2, r1 | addxy r3, r1, #1.0
mad r7, r0, #-0.68493148895338163 | ld r0, [c072]
addxy r4, r7, #0.027397258945758795
mul r2, r3, r4 | ld r4, [0322] | ld r3, [a141]
mad r1, r2, #-0.73000001907348633 | addxy r2, r4, #-3.9062499126885086e-05
mul r7, r0, r1
shl r0, r2, -r1 | st r7, [0326]
mul r1, r0, #1.3698629779067633 | ld r0, [031f]
max r2, r3, r1
mov r3, r4
mad r3, r0, #0.00390625
mul r1, r3, #256.0
ph_wrap r0, r3, r1
addxy r3, r0, #1.0 | st r0, [031e]
ph_wrap r1, r3, r3
mul r3, r4, #256.0 | mul r4, r4, #-256.0
min r7, r1, r3 | st r4, [0322]
max r7, r7, r4 | ld r4, [0320]
mul r7, r4, r7
shl r7, r7, -r7
shl r7, r7, r4
max r4, r7, -r7
addxy r4, r4, #-1.0
mul r7, r7, r4 | mov r4, r2
mad r4, r0, #-0.68493148895338163 | mad r7, r1, #0.25
addxy r1, r4, #-0.65753423000762279
mul r0, r1, #-1.4600000381469727
ph_wrap r4, r3, r0 | ld r0, [0322]
min r1, r4, r3
max r3, r1, r0 | ld r1, [0320]
mul r0, r1, r3
shl r1, r0, -r7
shl r3, r1, r4
max r0, r3, -r3
addxy r1, r0, #-1.0
mul r0, r3, r1 | addxy r1, r7, #0.25
mad r0, r4, #0.25
subxy r3, r0, r7
mad r3, r2, #-0.36500000953674316 | ld r2, [c073]
addxy r4, r3, #-0.0099999997764825821
mul r0, r1, r4 | ld r1, [0326]
mad r0, r7, #0.5 | ld r7, [c070]
mul r3, r0, r2 | ld r2, [c056]
mad r3, r1, #0.125 | ld r1, [a131]
mul r4, r3, #4.0 | ld r3, [c076]
st r4, [031a]
ld r0, [031a]
mad r0, r1, r2 | ld r1, [0364] | ld r2, [c057]
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
mul r2, r1, #0.08664339756999316 | st r2, [a1b2]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a042]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [0328] | ld r1, [032d]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a04a]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | ld r0, [0328]
st r2, [032a] | ld r2, [a042]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c074]
mul r4, r2, r3 | mul r1, r1, #256.0 | st r1, [032c] | ld r3, [c075]
mul r2, r4, r7 | ph_clamp r1, r3, -r1 | ld r4, [c077]
mul r7, r3, r4 | ld r3, [c071] | st r1, [a132] | ld r1, [032a]
mul r4, r7, r3 | mul r7, r0, r2 | st r1, [033a]
mul r2, r1, r4 | addxy r4, r0, #-0.004999999888241291
min r3, r7, #0.00390625 | ld r7, [a13a] | st r2, [0334]
mul r4, r4, #5.479451911627053 | mov r2, r0 | st r3, [0336]
max r7, r7, r4 | ld r4, [0331]
mad r2, r4, #0.5 | mul r3, r0, #2.0
mul r4, r2, #2.0 | mul r0, r0, #-2.0
ph_wrap r2, r3, r4
addxy r4, r2, #1.0 | st r2, [0330]
ph_wrap r4, r3, r4 | st r2, [0338]
min r2, r4, r3
max r2, r2, r0
mul r2, r1, r2
shl r2, r2, -r5
max r1, r2, -r2
addxy r1, r1, #-2.0
mul r2, r2, r1
addxy r1, r2, r4 | mov r2, r7 | ld r4, [0338]
mad r2, r4, #-0.68493148895338163
addxy r4, r2, #-0.65753423000762279
mul r2, r4, #-1.4600000381469727
ph_wrap r4, r3, r2
min r2, r4, r3
max r3, r2, r0 | ld r2, [033a]
mul r0, r2, r3
shl r2, r0, -r5
max r3, r2, -r2
addxy r0, r3, #-2.0
mul r3, r2, r0
addxy r2, r3, r4
subxy r0, r2, r1 | addxy r3, r1, #1.0
mad r7, r0, #-0.68493148895338163 | ld r0, [c072]
addxy r4, r7, #0.027397258945758795
mul r2, r3, r4 | ld r4, [0336] | ld r3, [a142]
mad r1, r2, #-0.73000001907348633 | addxy r2, r4, #-3.9062499126885086e-05
mul r7, r0, r1
shl r0, r2, -r1 | st r7, [033a]
mul r1, r0, #1.3698629779067633 | ld r0, [0333]
max r2, r3, r1
mov r3, r4
mad r3, r0, #0.00390625
mul r1, r3, #256.0
ph_wrap r0, r3, r1
addxy r3, r0, #1.0 | st r0, [0332]
ph_wrap r1, r3, r3
mul r3, r4, #256.0 | mul r4, r4, #-256.0
min r7, r1, r3 | st r4, [0336]
max r7, r7, r4 | ld r4, [0334]
mul r7, r4, r7
shl r7, r7, -r7
shl r7, r7, r4
max r4, r7, -r7
addxy r4, r4, #-1.0
mul r7, r7, r4 | mov r4, r2
mad r4, r0, #-0.68493148895338163 | mad r7, r1, #0.25
addxy r1, r4, #-0.65753423000762279
mul r0, r1, #-1.4600000381469727
ph_wrap r4, r3, r0 | ld r0, [0336]
min r1, r4, r3
max r3, r1, r0 | ld r1, [0334]
mul r0, r1, r3
shl r1, r0, -r7
shl r3, r1, r4
max r0, r3, -r3
addxy r1, r0, #-1.0
mul r0, r3, r1 | addxy r1, r7, #0.25
mad r0, r4, #0.25
subxy r3, r0, r7
mad r3, r2, #-0.36500000953674316 | ld r2, [c073]
addxy r4, r3, #-0.0099999997764825821
mul r0, r1, r4 | ld r1, [033a]
mad r0, r7, #0.5 | ld r7, [c070]
mul r3, r0, r2 | ld r2, [c056]
mad r3, r1, #0.125 | ld r1, [a132]
mul r4, r3, #4.0 | ld r3, [c076]
st r4, [032e]
ld r0, [032e]
mad r0, r1, r2 | ld r1, [0364] | ld r2, [c057]
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
mul r2, r1, #0.08664339756999316 | st r2, [a1b3]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a043]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [033c] | ld r1, [0341]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a04b]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | ld r0, [033c]
st r2, [033e] | ld r2, [a043]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c074]
mul r4, r2, r3 | mul r1, r1, #256.0 | st r1, [0340] | ld r3, [c075]
mul r2, r4, r7 | ph_clamp r1, r3, -r1 | ld r4, [c077]
mul r7, r3, r4 | ld r3, [c071] | st r1, [a133] | ld r1, [033e]
mul r4, r7, r3 | mul r7, r0, r2 | st r1, [034e]
mul r2, r1, r4 | addxy r4, r0, #-0.004999999888241291
min r3, r7, #0.00390625 | ld r7, [a13b] | st r2, [0348]
mul r4, r4, #5.479451911627053 | mov r2, r0 | st r3, [034a]
max r7, r7, r4 | ld r4, [0345]
mad r2, r4, #0.5 | mul r3, r0, #2.0
mul r4, r2, #2.0 | mul r0, r0, #-2.0
ph_wrap r2, r3, r4
addxy r4, r2, #1.0 | st r2, [0344]
ph_wrap r4, r3, r4 | st r2, [034c]
min r2, r4, r3
max r2, r2, r0
mul r2, r1, r2
shl r2, r2, -r5
max r1, r2, -r2
addxy r1, r1, #-2.0
mul r2, r2, r1
addxy r1, r2, r4 | mov r2, r7 | ld r4, [034c]
mad r2, r4, #-0.68493148895338163
addxy r4, r2, #-0.65753423000762279
mul r2, r4, #-1.4600000381469727
ph_wrap r4, r3, r2
min r2, r4, r3
max r3, r2, r0 | ld r2, [034e]
mul r0, r2, r3
shl r2, r0, -r5
max r3, r2, -r2
addxy r0, r3, #-2.0
mul r3, r2, r0
addxy r2, r3, r4
subxy r0, r2, r1 | addxy r3, r1, #1.0
mad r7, r0, #-0.68493148895338163 | ld r0, [c072]
addxy r4, r7, #0.027397258945758795
mul r2, r3, r4 | ld r4, [034a] | ld r3, [a143]
mad r1, r2, #-0.73000001907348633 | addxy r2, r4, #-3.9062499126885086e-05
mul r7, r0, r1
shl r0, r2, -r1 | st r7, [034e]
mul r1, r0, #1.3698629779067633 | ld r0, [0347]
max r2, r3, r1
mov r3, r4
mad r3, r0, #0.00390625
mul r1, r3, #256.0
ph_wrap r0, r3, r1
addxy r3, r0, #1.0 | st r0, [0346]
ph_wrap r1, r3, r3
mul r3, r4, #256.0 | mul r4, r4, #-256.0
min r7, r1, r3 | st r4, [034a]
max r7, r7, r4 | ld r4, [0348]
mul r7, r4, r7
shl r7, r7, -r7
shl r7, r7, r4
max r4, r7, -r7
addxy r4, r4, #-1.0
mul r7, r7, r4 | mov r4, r2
mad r4, r0, #-0.68493148895338163 | mad r7, r1, #0.25
addxy r1, r4, #-0.65753423000762279
mul r0, r1, #-1.4600000381469727
ph_wrap r4, r3, r0 | ld r0, [034a]
min r1, r4, r3
max r3, r1, r0 | ld r1, [0348]
mul r0, r1, r3
shl r1, r0, -r7
shl r3, r1, r4
max r0, r3, -r3
addxy r1, r0, #-1.0
mul r0, r3, r1 | addxy r1, r7, #0.25
mad r0, r4, #0.25
subxy r3, r0, r7
mad r3, r2, #-0.36500000953674316 | ld r2, [c073]
addxy r4, r3, #-0.0099999997764825821
mul r0, r1, r4 | ld r1, [034e]
mad r0, r7, #0.5 | ld r7, [c070]
mul r3, r0, r2 | ld r2, [c056]
mad r3, r1, #0.125 | ld r1, [a133]
mul r4, r3, #4.0 | ld r3, [c076]
st r4, [0342]
ld r0, [0342]
mad r0, r1, r2 | ld r1, [0364] | ld r2, [c057]
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
mul r2, r1, #0.08664339756999316 | st r2, [a1b4]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a044]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [0350] | ld r1, [0355]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a04c]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | ld r0, [0350]
st r2, [0352] | ld r2, [a044]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c074]
mul r4, r2, r3 | mul r1, r1, #256.0 | st r1, [0354] | ld r3, [c075]
mul r2, r4, r7 | ph_clamp r1, r3, -r1 | ld r4, [c077]
mul r7, r3, r4 | ld r3, [c071] | st r1, [a134] | ld r1, [0352]
mul r4, r7, r3 | mul r7, r0, r2 | st r1, [0362]
mul r2, r1, r4 | addxy r4, r0, #-0.004999999888241291
min r3, r7, #0.00390625 | ld r7, [a13c] | st r2, [035c]
mul r4, r4, #5.479451911627053 | mov r2, r0 | st r3, [035e]
max r7, r7, r4 | ld r4, [0359]
mad r2, r4, #0.5 | mul r3, r0, #2.0
mul r4, r2, #2.0 | mul r0, r0, #-2.0
ph_wrap r2, r3, r4
addxy r4, r2, #1.0 | st r2, [0358]
ph_wrap r4, r3, r4 | st r2, [0360]
min r2, r4, r3
max r2, r2, r0
mul r2, r1, r2
shl r2, r2, -r5
max r1, r2, -r2
addxy r1, r1, #-2.0
mul r2, r2, r1
addxy r1, r2, r4 | mov r2, r7 | ld r4, [0360]
mad r2, r4, #-0.68493148895338163
addxy r4, r2, #-0.65753423000762279
mul r2, r4, #-1.4600000381469727
ph_wrap r4, r3, r2
min r2, r4, r3
max r3, r2, r0 | ld r2, [0362]
mul r0, r2, r3
shl r2, r0, -r5
max r3, r2, -r2
addxy r0, r3, #-2.0
mul r3, r2, r0
addxy r2, r3, r4
subxy r0, r2, r1 | addxy r3, r1, #1.0
mad r7, r0, #-0.68493148895338163 | ld r0, [c072]
addxy r4, r7, #0.027397258945758795
mul r2, r3, r4 | ld r4, [035e] | ld r3, [a144]
mad r1, r2, #-0.73000001907348633 | addxy r2, r4, #-3.9062499126885086e-05
mul r7, r0, r1
shl r0, r2, -r1 | st r7, [0362]
mul r1, r0, #1.3698629779067633 | ld r0, [035b]
max r2, r3, r1
mov r3, r4
mad r3, r0, #0.00390625
mul r1, r3, #256.0
ph_wrap r0, r3, r1
addxy r3, r0, #1.0 | st r0, [035a]
ph_wrap r1, r3, r3
mul r3, r4, #256.0 | mul r4, r4, #-256.0
min r7, r1, r3 | st r4, [035e]
max r7, r7, r4 | ld r4, [035c]
mul r7, r4, r7
shl r7, r7, -r7
shl r7, r7, r4
max r4, r7, -r7
addxy r4, r4, #-1.0
mul r7, r7, r4 | mov r4, r2
mad r4, r0, #-0.68493148895338163 | mad r7, r1, #0.25
addxy r1, r4, #-0.65753423000762279
mul r0, r1, #-1.4600000381469727
ph_wrap r4, r3, r0 | ld r0, [035e]
min r1, r4, r3
max r3, r1, r0 | ld r1, [035c]
mul r0, r1, r3
shl r1, r0, -r7
shl r3, r1, r4
max r0, r3, -r3
addxy r1, r0, #-1.0
mul r0, r3, r1 | addxy r1, r7, #0.25
mad r0, r4, #0.25
subxy r3, r0, r7
mad r3, r2, #-0.36500000953674316 | ld r2, [c073]
addxy r4, r3, #-0.0099999997764825821
mul r0, r1, r4 | ld r1, [0362]
mad r0, r7, #0.5
mul r3, r0, r2 | ld r2, [c056]
mad r3, r1, #0.125 | ld r1, [a134]
mul r4, r3, #4.0
st r4, [0356]
ld r0, [0356]
mad r0, r1, r2 | ld r1, [0364] | ld r2, [c057]
mad r0, r1, r2
st r0, [a155]
