ld r0, [03db] | ld r2, [03d9] | ld r4, [a061]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | ld r1, [0301] | ld r5, [0325] | ld r6, [a039]
ph_wrap r0, r3, r0 | st r6, [a161] | st r4, [0324] | ld r6, [0329]
addxy r2, r2, r0 | subxy r5, r5, r4 | st r0, [03da] | ld r0, [a060]
ph_wrap r2, r3, r2 | st r0, [a168] | st r0, [0300]
subxy r1, r1, r0 | mul r5, r5, r5 | st r2, [03d8] | ld r2, [a038]
mul r1, r1, r1 | mad r6, r6, -r5 | st r2, [a160] | ld r2, [0305]
mad r2, r2, -r1 | st r4, [a169] | ld r0, [0303] | ld r3, [c049]
addxy r1, r0, #-0.999 | ld r4, [0327] | ld r7, [c049]
addxy r5, r4, #-0.999 | mul r1, r1, #256.0
mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
max r2, r2, r1 | ld r1, [c048]
max r6, r6, r5 | st r2, [0304] | ld r5, [c048]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [0328]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7 | ld r3, [c051]
mul r2, r2, #1.25 | mul r6, r6, #1.25 | ld r7, [c051]
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r1, [a120]
min r0, r0, #1.0 | ld r5, [a121] | ld r2, [c04b]
mul r1, r1, r2 | ld r6, [c04b] | st r0, [0302]
mul r5, r5, r6 | ld r2, [c05e]
min r4, r4, #1.0 | ld r6, [c05e]
mad r1, r0, r2 | mad r5, r4, r6 | ld r2, [c05f] | st r4, [0326]
addxy r1, r1, r2 | ld r6, [c05f] | ld r2, [a030]
addxy r5, r5, r6 | addxy r2, r2, r1 | ld r6, [a031]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [a1b0]
mul r6, r5, #0.08664339756999316 | mov r0, #0.041666666666666664 | st r6, [a1b1]
mov r4, #0.041666666666666664 | ymacd r0, r2, #0.16666666666666666
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a040]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a041]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mul r2, r1, #-0.08664339756999316 | st r2, [0306] | ld r1, [030b]
mul r6, r5, #-0.08664339756999316 | mov r0, #0.16666666666666666 | st r6, [032a] | ld r5, [032f]
mov r4, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a048]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a049]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0 | ld r0, [0306]
mul r6, r6, r4 | st r2, [0308] | ld r2, [a040]
addxy r2, r1, r2 | st r6, [032c] | ld r6, [a041]
addxy r6, r5, r6 | ld r4, [032a]
ph_wrap r1, r3, r2 | ld r2, [c050]
ph_wrap r5, r3, r6 | st r1, [030a] | ld r6, [c050]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [032e]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [030c] | ld r1, [a120]
mad r2, r1, r3 | st r5, [0330] | ld r5, [a121] | ld r3, [c052]
mad r6, r5, r7 | ld r1, [0302] | ld r5, [0326] | ld r7, [c052]
mad r2, r1, r3 | mad r6, r5, r7 | ld r1, [a178]
ld r5, [a179] | ld r3, [c053]
mad r2, r1, r3 | ld r7, [c053] | ld r1, [a170]
mad r6, r5, r7 | ld r5, [a171] | ld r3, [c054]
mad r2, r1, r3 | ld r7, [c054] | ld r1, [030c]
mad r6, r5, r7 | ld r5, [0330] | ld r3, [c055]
mad r2, r1, r3 | ld r7, [c055] | ld r1, [a120]
mad r6, r5, r7 | st r2, [030e] | ld r2, [c058] | ld r5, [a121]
st r6, [0332] | ld r6, [c058]
ld r3, [c059]
mad r2, r1, r3 | ld r7, [c059] | ld r1, [0302]
mad r6, r5, r7 | ld r5, [0326] | ld r3, [c05a]
mad r2, r1, r3 | ld r7, [c05a] | ld r1, [a178]
mad r6, r5, r7 | ld r5, [a179] | ld r3, [c05b]
mad r2, r1, r3 | ld r7, [c05b] | ld r1, [a170]
mad r6, r5, r7 | ld r5, [a171] | ld r3, [c05c]
mad r2, r1, r3 | ld r7, [c05c] | ld r1, [030c]
mad r6, r5, r7 | ld r5, [0330] | ld r3, [c05d]
mad r2, r1, r3 | ld r7, [c05d] | ld r1, [0308]
mad r6, r5, r7 | st r2, [0310] | ld r2, [c074] | ld r5, [032c]
st r6, [0334] | ld r6, [c074]
ld r3, [c076] | st r1, [0318]
mul r2, r2, r3 | ld r7, [c076] | st r5, [033c]
mul r6, r6, r7 | ld r3, [c070]
mul r2, r2, r3 | ld r7, [c070]
mul r6, r6, r7 | ld r3, [c075]
ld r7, [c075]
ld r1, [c077]
mul r3, r3, r1 | ld r5, [c077]
mul r7, r7, r5 | ld r1, [c071]
mul r3, r3, r1 | mul r1, r0, r2 | ld r5, [c071]
mul r7, r7, r5 | mul r5, r4, r6
min r2, r1, #0.00390625 | ld r1, [0318]
min r6, r5, #0.00390625 | ld r5, [033c] | st r2, [031c]
mul r3, r1, r3 | mul r7, r5, r7 | st r6, [0340]
addxy r2, r0, #-0.004999999888241291 | addxy r6, r4, #-0.004999999888241291 | st r3, [031a] | ld r3, [030e]
mul r2, r2, #5.479451911627053 | mul r6, r6, #5.479451911627053 | st r7, [033e] | ld r7, [0332]
max r3, r3, r2 | ld r2, [0315]
max r7, r7, r6 | st r3, [031e] | ld r6, [0339]
mov r3, r0 | mov r7, r4 | st r7, [0342]
mad r3, r2, #0.5 | mad r7, r6, #0.5
mul r2, r3, #2.0 | mul r6, r7, #2.0
ph_wrap r3, r3, r2
ph_wrap r7, r3, r6 | st r3, [0314]
addxy r2, r3, #1.0 | addxy r6, r7, #1.0 | st r7, [0338]
st r3, [0320] | st r7, [0344]
ph_wrap r2, r3, r2
ph_wrap r6, r3, r6
mul r3, r0, #2.0 | mul r7, r4, #2.0
min r1, r2, r3
min r5, r6, r7
mul r0, r0, #-2.0 | mul r4, r4, #-2.0
max r1, r1, r0 | st r0, [0322] | ld r0, [0318]
max r5, r5, r4 | st r4, [0346] | ld r4, [033c]
mul r1, r0, r1 | mul r5, r4, r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #32.0 | mul r5, r5, #32.0
max r0, r1, -r1
max r4, r5, -r5
addxy r0, r0, #-2.0 | addxy r4, r4, #-2.0
mul r1, r1, r0 | mul r5, r5, r4
addxy r0, r1, r2 | addxy r4, r5, r6 | ld r1, [031e] | ld r5, [0342]
mov r2, r1 | mov r6, r5 | ld r1, [0320] | ld r5, [0344]
mad r2, r1, #-0.68493148895338163 | mad r6, r5, #-0.68493148895338163
addxy r1, r2, #-0.65753423000762279 | addxy r5, r6, #-0.65753423000762279
mul r2, r1, #-1.4600000381469727 | mul r6, r5, #-1.4600000381469727
ph_wrap r1, r3, r2
ph_wrap r5, r3, r6
min r2, r1, r3 | ld r3, [0322]
min r6, r5, r7 | ld r7, [0346]
max r2, r2, r3 | ld r3, [0318]
max r6, r6, r7 | ld r7, [033c]
mul r3, r3, r2 | mul r7, r7, r6
mul r2, r3, #256.0 | mul r6, r7, #256.0
mul r3, r2, #32.0 | mul r7, r6, #32.0
max r2, r3, -r3
max r6, r7, -r7
addxy r2, r2, #-2.0 | addxy r6, r6, #-2.0
mul r3, r3, r2 | mul r7, r7, r6
addxy r2, r3, r1 | addxy r6, r7, r5 | ld r1, [031e] | ld r5, [0342]
subxy r3, r2, r0 | subxy r7, r6, r4
mad r1, r3, #-0.68493148895338163 | mad r5, r7, #-0.68493148895338163
addxy r2, r0, #1.0 | addxy r6, r4, #1.0
addxy r3, r1, #0.027397258945758795 | addxy r7, r5, #0.027397258945758795
mul r1, r2, r3 | mul r5, r6, r7 | ld r2, [c072]
mad r0, r1, #-0.73000001907348633 | mad r4, r5, #-0.73000001907348633 | ld r6, [c072] | ld r1, [0310] | ld r5, [0334]
mul r3, r2, r0 | mul r7, r6, r4 | ld r2, [031c] | ld r6, [0340]
addxy r0, r2, #-3.9062499126885086e-05 | addxy r4, r6, #-3.9062499126885086e-05 | st r3, [031e]
mul r0, r0, #256.0 | mul r4, r4, #256.0 | st r7, [0342]
mul r0, r0, #2.7397259558135265 | mul r4, r4, #2.7397259558135265
max r1, r1, r0 | ld r0, [0317]
max r5, r5, r4 | ld r4, [033b] | st r1, [0322]
mov r3, r2 | mov r7, r6 | st r5, [0346]
mad r3, r0, #0.00390625 | mad r7, r4, #0.00390625
mul r0, r3, #256.0 | mul r4, r7, #256.0
ph_wrap r3, r3, r0
ph_wrap r7, r3, r4 | st r3, [0316]
addxy r0, r3, #1.0 | addxy r4, r7, #1.0 | st r7, [033a]
st r3, [0318] | st r7, [033c]
ph_wrap r0, r3, r0
ph_wrap r4, r3, r4
mul r3, r2, #256.0 | mul r7, r6, #256.0
min r1, r0, r3
min r5, r4, r7
mul r2, r2, #-256.0 | mul r6, r6, #-256.0
max r1, r1, r2 | st r2, [031c] | ld r2, [031a]
max r5, r5, r6 | st r6, [0340] | ld r6, [033e]
mul r1, r2, r1 | mul r5, r6, r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #8.0 | mul r5, r5, #8.0
max r2, r1, -r1
max r6, r5, -r5
addxy r2, r2, #-1.0 | addxy r6, r6, #-1.0
mul r1, r1, r2 | mul r5, r5, r6 | ld r2, [0322] | ld r6, [0346]
mad r1, r0, #0.25 | mad r5, r4, #0.25
mov r0, r2 | mov r4, r6 | ld r2, [0318] | ld r6, [033c]
mad r0, r2, #-0.68493148895338163 | mad r4, r6, #-0.68493148895338163
addxy r2, r0, #-0.65753423000762279 | addxy r6, r4, #-0.65753423000762279
mul r0, r2, #-1.4600000381469727 | mul r4, r6, #-1.4600000381469727
ph_wrap r2, r3, r0
ph_wrap r6, r3, r4
min r0, r2, r3 | ld r3, [031c]
min r4, r6, r7 | ld r7, [0340]
max r0, r0, r3 | ld r3, [031a]
max r4, r4, r7 | ld r7, [033e]
mul r3, r3, r0 | mul r7, r7, r4
mul r0, r3, #256.0 | mul r4, r7, #256.0
mul r3, r0, #256.0 | mul r7, r4, #256.0
mul r0, r3, #8.0 | mul r4, r7, #8.0
max r3, r0, -r0
max r7, r4, -r4
addxy r3, r3, #-1.0 | addxy r7, r7, #-1.0
mul r0, r0, r3 | mul r4, r4, r7
mad r0, r2, #0.25 | mad r4, r6, #0.25 | ld r2, [0322] | ld r6, [0346]
subxy r3, r0, r1 | subxy r7, r4, r5
mad r3, r2, #-0.36500000953674316 | mad r7, r6, #-0.36500000953674316
addxy r0, r1, #0.25 | addxy r4, r5, #0.25
addxy r2, r3, #-0.0099999997764825821 | addxy r6, r7, #-0.0099999997764825821
mul r3, r0, r2 | mul r7, r4, r6 | ld r0, [c073]
mad r3, r1, #0.5 | mad r7, r5, #0.5 | ld r4, [c073] | ld r1, [031e] | ld r5, [0342]
mul r2, r3, r0 | mul r6, r7, r4
mad r2, r1, #0.125 | mad r6, r5, #0.125 | ld r1, [030c] | ld r5, [0330]
mul r3, r2, #4.0 | mul r7, r6, #4.0 | ld r2, [c056]
st r3, [0312] | st r7, [0336]
ld r0, [0312] | ld r4, [0336] | ld r6, [c056]
mad r0, r1, r2 | mad r4, r5, r6 | ld r1, [03d8] | ld r2, [c057]
mad r0, r1, r2 | ld r5, [03d8] | ld r6, [c057] | ld r1, [0349]
mad r4, r5, r6 | st r0, [a151] | ld r5, [036d] | ld r3, [c049]
st r4, [a152] | ld r7, [c049]
ld r0, [a062]
subxy r1, r1, r0 | ld r4, [a063] | st r0, [0348]
subxy r5, r5, r4 | mul r1, r1, r1 | ld r2, [a03a] | st r4, [036c]
mul r5, r5, r5 | ld r6, [a03b]
st r2, [a162] | ld r2, [034d]
mad r2, r2, -r1 | st r6, [a163] | ld r6, [0371]
mad r6, r6, -r5 | st r0, [a16a] | ld r0, [034b]
addxy r1, r0, #-0.999 | st r4, [a16b] | ld r4, [036f]
addxy r5, r4, #-0.999 | mul r1, r1, #256.0
mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
max r2, r2, r1 | ld r1, [c048]
max r6, r6, r5 | st r2, [034c] | ld r5, [c048]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [0370]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7 | ld r3, [c051]
mul r2, r2, #1.25 | mul r6, r6, #1.25 | ld r7, [c051]
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r1, [a122]
min r0, r0, #1.0 | ld r5, [a123] | ld r2, [c04b]
mul r1, r1, r2 | ld r6, [c04b] | st r0, [034a]
mul r5, r5, r6 | ld r2, [c05e]
min r4, r4, #1.0 | ld r6, [c05e]
mad r1, r0, r2 | mad r5, r4, r6 | ld r2, [c05f] | st r4, [036e]
addxy r1, r1, r2 | ld r6, [c05f] | ld r2, [a032]
addxy r5, r5, r6 | addxy r2, r2, r1 | ld r6, [a033]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [a1b2]
mul r6, r5, #0.08664339756999316 | mov r0, #0.041666666666666664 | st r6, [a1b3]
mov r4, #0.041666666666666664 | ymacd r0, r2, #0.16666666666666666
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a042]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a043]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mul r2, r1, #-0.08664339756999316 | st r2, [034e] | ld r1, [0353]
mul r6, r5, #-0.08664339756999316 | mov r0, #0.16666666666666666 | st r6, [0372] | ld r5, [0377]
mov r4, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a04a]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a04b]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0 | ld r0, [034e]
mul r6, r6, r4 | st r2, [0350] | ld r2, [a042]
addxy r2, r1, r2 | st r6, [0374] | ld r6, [a043]
addxy r6, r5, r6 | ld r4, [0372]
ph_wrap r1, r3, r2 | ld r2, [c050]
ph_wrap r5, r3, r6 | st r1, [0352] | ld r6, [c050]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [0376]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [0354] | ld r1, [a122]
mad r2, r1, r3 | st r5, [0378] | ld r5, [a123] | ld r3, [c052]
mad r6, r5, r7 | ld r1, [034a] | ld r5, [036e] | ld r7, [c052]
mad r2, r1, r3 | mad r6, r5, r7 | ld r1, [a17a]
ld r5, [a17b] | ld r3, [c053]
mad r2, r1, r3 | ld r7, [c053] | ld r1, [a172]
mad r6, r5, r7 | ld r5, [a173] | ld r3, [c054]
mad r2, r1, r3 | ld r7, [c054] | ld r1, [0354]
mad r6, r5, r7 | ld r5, [0378] | ld r3, [c055]
mad r2, r1, r3 | ld r7, [c055] | ld r1, [a122]
mad r6, r5, r7 | st r2, [0356] | ld r2, [c058] | ld r5, [a123]
st r6, [037a] | ld r6, [c058]
ld r3, [c059]
mad r2, r1, r3 | ld r7, [c059] | ld r1, [034a]
mad r6, r5, r7 | ld r5, [036e] | ld r3, [c05a]
mad r2, r1, r3 | ld r7, [c05a] | ld r1, [a17a]
mad r6, r5, r7 | ld r5, [a17b] | ld r3, [c05b]
mad r2, r1, r3 | ld r7, [c05b] | ld r1, [a172]
mad r6, r5, r7 | ld r5, [a173] | ld r3, [c05c]
mad r2, r1, r3 | ld r7, [c05c] | ld r1, [0354]
mad r6, r5, r7 | ld r5, [0378] | ld r3, [c05d]
mad r2, r1, r3 | ld r7, [c05d] | ld r1, [0350]
mad r6, r5, r7 | st r2, [0358] | ld r2, [c074] | ld r5, [0374]
st r6, [037c] | ld r6, [c074]
ld r3, [c076] | st r1, [0360]
mul r2, r2, r3 | ld r7, [c076] | st r5, [0384]
mul r6, r6, r7 | ld r3, [c070]
mul r2, r2, r3 | ld r7, [c070]
mul r6, r6, r7 | ld r3, [c075]
ld r7, [c075]
ld r1, [c077]
mul r3, r3, r1 | ld r5, [c077]
mul r7, r7, r5 | ld r1, [c071]
mul r3, r3, r1 | mul r1, r0, r2 | ld r5, [c071]
mul r7, r7, r5 | mul r5, r4, r6
min r2, r1, #0.00390625 | ld r1, [0360]
min r6, r5, #0.00390625 | ld r5, [0384] | st r2, [0364]
mul r3, r1, r3 | mul r7, r5, r7 | st r6, [0388]
addxy r2, r0, #-0.004999999888241291 | addxy r6, r4, #-0.004999999888241291 | st r3, [0362] | ld r3, [0356]
mul r2, r2, #5.479451911627053 | mul r6, r6, #5.479451911627053 | st r7, [0386] | ld r7, [037a]
max r3, r3, r2 | ld r2, [035d]
max r7, r7, r6 | st r3, [0366] | ld r6, [0381]
mov r3, r0 | mov r7, r4 | st r7, [038a]
mad r3, r2, #0.5 | mad r7, r6, #0.5
mul r2, r3, #2.0 | mul r6, r7, #2.0
ph_wrap r3, r3, r2
ph_wrap r7, r3, r6 | st r3, [035c]
addxy r2, r3, #1.0 | addxy r6, r7, #1.0 | st r7, [0380]
st r3, [0368] | st r7, [038c]
ph_wrap r2, r3, r2
ph_wrap r6, r3, r6
mul r3, r0, #2.0 | mul r7, r4, #2.0
min r1, r2, r3
min r5, r6, r7
mul r0, r0, #-2.0 | mul r4, r4, #-2.0
max r1, r1, r0 | st r0, [036a] | ld r0, [0360]
max r5, r5, r4 | st r4, [038e] | ld r4, [0384]
mul r1, r0, r1 | mul r5, r4, r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #32.0 | mul r5, r5, #32.0
max r0, r1, -r1
max r4, r5, -r5
addxy r0, r0, #-2.0 | addxy r4, r4, #-2.0
mul r1, r1, r0 | mul r5, r5, r4
addxy r0, r1, r2 | addxy r4, r5, r6 | ld r1, [0366] | ld r5, [038a]
mov r2, r1 | mov r6, r5 | ld r1, [0368] | ld r5, [038c]
mad r2, r1, #-0.68493148895338163 | mad r6, r5, #-0.68493148895338163
addxy r1, r2, #-0.65753423000762279 | addxy r5, r6, #-0.65753423000762279
mul r2, r1, #-1.4600000381469727 | mul r6, r5, #-1.4600000381469727
ph_wrap r1, r3, r2
ph_wrap r5, r3, r6
min r2, r1, r3 | ld r3, [036a]
min r6, r5, r7 | ld r7, [038e]
max r2, r2, r3 | ld r3, [0360]
max r6, r6, r7 | ld r7, [0384]
mul r3, r3, r2 | mul r7, r7, r6
mul r2, r3, #256.0 | mul r6, r7, #256.0
mul r3, r2, #32.0 | mul r7, r6, #32.0
max r2, r3, -r3
max r6, r7, -r7
addxy r2, r2, #-2.0 | addxy r6, r6, #-2.0
mul r3, r3, r2 | mul r7, r7, r6
addxy r2, r3, r1 | addxy r6, r7, r5 | ld r1, [0366] | ld r5, [038a]
subxy r3, r2, r0 | subxy r7, r6, r4
mad r1, r3, #-0.68493148895338163 | mad r5, r7, #-0.68493148895338163
addxy r2, r0, #1.0 | addxy r6, r4, #1.0
addxy r3, r1, #0.027397258945758795 | addxy r7, r5, #0.027397258945758795
mul r1, r2, r3 | mul r5, r6, r7 | ld r2, [c072]
mad r0, r1, #-0.73000001907348633 | mad r4, r5, #-0.73000001907348633 | ld r6, [c072] | ld r1, [0358] | ld r5, [037c]
mul r3, r2, r0 | mul r7, r6, r4 | ld r2, [0364] | ld r6, [0388]
addxy r0, r2, #-3.9062499126885086e-05 | addxy r4, r6, #-3.9062499126885086e-05 | st r3, [0366]
mul r0, r0, #256.0 | mul r4, r4, #256.0 | st r7, [038a]
mul r0, r0, #2.7397259558135265 | mul r4, r4, #2.7397259558135265
max r1, r1, r0 | ld r0, [035f]
max r5, r5, r4 | ld r4, [0383] | st r1, [036a]
mov r3, r2 | mov r7, r6 | st r5, [038e]
mad r3, r0, #0.00390625 | mad r7, r4, #0.00390625
mul r0, r3, #256.0 | mul r4, r7, #256.0
ph_wrap r3, r3, r0
ph_wrap r7, r3, r4 | st r3, [035e]
addxy r0, r3, #1.0 | addxy r4, r7, #1.0 | st r7, [0382]
st r3, [0360] | st r7, [0384]
ph_wrap r0, r3, r0
ph_wrap r4, r3, r4
mul r3, r2, #256.0 | mul r7, r6, #256.0
min r1, r0, r3
min r5, r4, r7
mul r2, r2, #-256.0 | mul r6, r6, #-256.0
max r1, r1, r2 | st r2, [0364] | ld r2, [0362]
max r5, r5, r6 | st r6, [0388] | ld r6, [0386]
mul r1, r2, r1 | mul r5, r6, r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #8.0 | mul r5, r5, #8.0
max r2, r1, -r1
max r6, r5, -r5
addxy r2, r2, #-1.0 | addxy r6, r6, #-1.0
mul r1, r1, r2 | mul r5, r5, r6 | ld r2, [036a] | ld r6, [038e]
mad r1, r0, #0.25 | mad r5, r4, #0.25
mov r0, r2 | mov r4, r6 | ld r2, [0360] | ld r6, [0384]
mad r0, r2, #-0.68493148895338163 | mad r4, r6, #-0.68493148895338163
addxy r2, r0, #-0.65753423000762279 | addxy r6, r4, #-0.65753423000762279
mul r0, r2, #-1.4600000381469727 | mul r4, r6, #-1.4600000381469727
ph_wrap r2, r3, r0
ph_wrap r6, r3, r4
min r0, r2, r3 | ld r3, [0364]
min r4, r6, r7 | ld r7, [0388]
max r0, r0, r3 | ld r3, [0362]
max r4, r4, r7 | ld r7, [0386]
mul r3, r3, r0 | mul r7, r7, r4
mul r0, r3, #256.0 | mul r4, r7, #256.0
mul r3, r0, #256.0 | mul r7, r4, #256.0
mul r0, r3, #8.0 | mul r4, r7, #8.0
max r3, r0, -r0
max r7, r4, -r4
addxy r3, r3, #-1.0 | addxy r7, r7, #-1.0
mul r0, r0, r3 | mul r4, r4, r7
mad r0, r2, #0.25 | mad r4, r6, #0.25 | ld r2, [036a] | ld r6, [038e]
subxy r3, r0, r1 | subxy r7, r4, r5
mad r3, r2, #-0.36500000953674316 | mad r7, r6, #-0.36500000953674316
addxy r0, r1, #0.25 | addxy r4, r5, #0.25
addxy r2, r3, #-0.0099999997764825821 | addxy r6, r7, #-0.0099999997764825821
mul r3, r0, r2 | mul r7, r4, r6 | ld r0, [c073]
mad r3, r1, #0.5 | mad r7, r5, #0.5 | ld r4, [c073] | ld r1, [0366] | ld r5, [038a]
mul r2, r3, r0 | mul r6, r7, r4
mad r2, r1, #0.125 | mad r6, r5, #0.125 | ld r1, [0354] | ld r5, [0378]
mul r3, r2, #4.0 | mul r7, r6, #4.0 | ld r2, [c056]
st r3, [035a] | st r7, [037e]
ld r0, [035a] | ld r4, [037e] | ld r6, [c056]
mad r0, r1, r2 | mad r4, r5, r6 | ld r1, [03d8] | ld r2, [c057]
mad r0, r1, r2 | ld r5, [03d8] | ld r6, [c057]
mad r4, r5, r6 | st r0, [a153]
st r4, [a154]
