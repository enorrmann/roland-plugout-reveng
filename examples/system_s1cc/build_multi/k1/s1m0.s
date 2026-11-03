ld r0, [0393] | ld r2, [0391] | ld r3, [c049]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | ld r1, [0301]
ph_wrap r0, r3, r0
addxy r2, r2, r0 | st r0, [0392] | ld r0, [a060]
ph_wrap r2, r3, r2 | st r0, [a168] | st r0, [0300]
subxy r1, r1, r0 | st r2, [0390] | ld r2, [a038]
mul r1, r1, r1 | st r2, [a160] | ld r2, [0305]
mad r2, r2, -r1 | ld r0, [0303]
addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0304]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a120] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [0302]
addxy r1, r1, r2 | ld r2, [a030]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [0306]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a040]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [0308] | ld r1, [030d]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a048]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [030a] | ld r2, [a040]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [030c]
ph_clamp r1, r3, -r1
st r1, [a130]
ld r1, [a120]
mad r2, r1, r3 | ld r1, [0302] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a178] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a170] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a130] | ld r3, [c055]
mad r2, r1, r3 | ld r0, [a130] | ld r1, [0390]
st r2, [030e] | ld r2, [c056]
mul r0, r0, r2 | ld r3, [c057] | ld r2, [030e]
mad r0, r1, r3 | ld r3, [0181] | st r2, [0186]
st r0, [0310] | ld r0, [0308]
mov r4, r0 | ld r1, [030a]
mad r4, r3, #0.5 | st r1, [0188]
mul r7, r4, #2.0
ph_wrap r3, r3, r7
mov r4, r3 | st r3, [0180]
mad r4, r2, #-0.89999997615814209
ph_wrap r7, r3, r4
mul r4, r0, #2.0 | mul r0, r0, #-2.0
min r2, r7, r4
max r2, r2, r0
mul r2, r1, r2
shl r2, r2, 13
absadd r1, r2, #-2.0
mul r2, r2, r1
addxy r1, r2, r7
addxy r2, r3, #1.0
ph_wrap r7, r3, r2
min r3, r7, r4 | ld r4, [0188]
max r2, r3, r0
mul r3, r4, r2
shl r0, r3, 13
absadd r4, r0, #-2.0
mul r2, r0, r4 | ld r4, [0186]
addxy r3, r2, r7 | ld r2, [c073]
subxy r0, r3, r1 | ld r3, [0310]
mad r0, r4, #0.44999998807907104
mad r0, r4, #0.44999998807907104 | ld r4, [a1b0]
mul r7, r2, r0 | ld r2, [a1c8]
mul r0, r4, r2 | addxy r1, r3, r7 | ld r2, [0306] | ld r4, [c072]
addxy r2, r2, #-4.75 | ld r3, [a1b8] | ld r7, [c071] | st r0, [0186]
mul r4, r4, r2
mac_p1wc3 r2, r3, r7, #0.125, r4 | ld r3, [a1d0] | ld r7, [c070]
mad r2, r3, #0.125 | mov r3, #0.041666666666666664
mul r4, r2, #0.17328679513998632
ymacd r3, r4, #0.16666666666666666
ymacd r3, r4, #0.5
ymacd r3, r4, #1.0
ymacd r3, r4, #1.0
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, r4
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, #6.103515625e-05 | ld r4, [c074]
mul r2, r7, r3
min r7, r2, #3.3569336665095761e-05 | ld r2, [0183]
shl r3, r7, 14 | ld r7, [0185]
mul r0, r3, r7
addxy r0, r0, r2
mov r2, r0
mad r2, r1, #-0.0625
mac_p1wc3 r2, r4, r7, #0.5, r2
mac_p1wc3 r2, r3, r2, #-0.5, r7
mac_p1wc3 r7, r3, r2, #-0.5, r0
mul r0, r7, #-2.0
mov r0, r7 | st r0, [0182]
mad r0, r1, #0.03125
mac_p1wc3 r1, r4, r2, #0.5, r0
mac_p1wc3 r4, r3, r1, #-0.5, r2 | ld r3, [0186]
mul r1, r3, r7 | mul r0, r4, #4.0 | ld r3, [c049]
mul r2, r1, #-32.0 | st r0, [0184] | ld r0, [a061]
st r2, [a151] | ld r1, [0313] | st r0, [0312]
subxy r1, r1, r0 | ld r2, [a039]
mul r1, r1, r1 | st r2, [a161] | ld r2, [0317]
mad r2, r2, -r1 | st r0, [a169] | ld r0, [0315]
addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0316]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a121] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [0314]
addxy r1, r1, r2 | ld r2, [a031]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [0318]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a041]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [031a] | ld r1, [031f]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a049]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [031c] | ld r2, [a041]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [031e]
ph_clamp r1, r3, -r1
st r1, [a131]
ld r1, [a121]
mad r2, r1, r3 | ld r1, [0314] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a179] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a171] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a131] | ld r3, [c055]
mad r2, r1, r3 | ld r0, [a131] | ld r1, [0390]
st r2, [0320] | ld r2, [c056]
mul r0, r0, r2 | ld r3, [c057]
mad r0, r1, r3 | ld r1, [031a] | ld r3, [0318]
st r0, [0322] | ld r0, [0320] | st r1, [a1a9]
st r0, [a1a1] | ld r0, [031c]
ld r2, [0322] | st r0, [a1e1]
st r2, [a1c1] | ld r1, [0325]
ld r0, [a062]
subxy r1, r1, r0 | ld r2, [a03a] | st r0, [0324]
mul r1, r1, r1 | st r2, [a162] | ld r2, [0329]
mad r2, r2, -r1 | st r0, [a16a] | ld r0, [0327]
addxy r1, r0, #-0.999 | st r3, [a1d9] | ld r3, [c049]
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0328]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a122] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [0326]
addxy r1, r1, r2 | ld r2, [a032]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [032a]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a042]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [032c] | ld r1, [0331]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a04a]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [032e] | ld r2, [a042]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [0330]
ph_clamp r1, r3, -r1
st r1, [a132]
ld r1, [a122]
mad r2, r1, r3 | ld r1, [0326] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a17a] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a172] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a132] | ld r3, [c055]
mad r2, r1, r3 | ld r0, [a132] | ld r1, [0390]
st r2, [0332] | ld r2, [c056]
mul r0, r0, r2 | ld r3, [c057]
mad r0, r1, r3 | ld r1, [032c] | ld r3, [032a]
st r0, [0334] | ld r0, [0332] | st r1, [a1aa]
st r0, [a1a2] | ld r0, [032e]
ld r2, [0334] | st r0, [a1e2]
st r2, [a1c2] | ld r1, [0337]
ld r0, [a063]
subxy r1, r1, r0 | ld r2, [a03b] | st r0, [0336]
mul r1, r1, r1 | st r2, [a163] | ld r2, [033b]
mad r2, r2, -r1 | st r0, [a16b] | ld r0, [0339]
addxy r1, r0, #-0.999 | st r3, [a1da] | ld r3, [c049]
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [033a]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a123] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [0338]
addxy r1, r1, r2 | ld r2, [a033]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [033c]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a043]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [033e] | ld r1, [0343]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a04b]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0340] | ld r2, [a043]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [0342]
ph_clamp r1, r3, -r1
st r1, [a133]
ld r1, [a123]
mad r2, r1, r3 | ld r1, [0338] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a17b] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a173] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a133] | ld r3, [c055]
mad r2, r1, r3 | ld r0, [a133] | ld r1, [0390]
st r2, [0344] | ld r2, [c056]
mul r0, r0, r2 | ld r3, [c057]
mad r0, r1, r3 | ld r1, [033e] | ld r3, [033c]
st r0, [0346] | ld r0, [0344] | st r1, [a1ab]
st r0, [a1a3] | ld r0, [0340]
ld r2, [0346] | st r0, [a1e3]
st r2, [a1c3] | ld r1, [0349]
ld r0, [a064]
subxy r1, r1, r0 | ld r2, [a03c] | st r0, [0348]
mul r1, r1, r1 | st r2, [a164] | ld r2, [034d]
mad r2, r2, -r1 | st r0, [a16c] | ld r0, [034b]
addxy r1, r0, #-0.999 | st r3, [a1db] | ld r3, [c049]
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [034c]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a124] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [034a]
addxy r1, r1, r2 | ld r2, [a034]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [034e]
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
mul r2, r2, r0
st r2, [0352] | ld r2, [a044]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [0354]
ph_clamp r1, r3, -r1
st r1, [a134]
ld r1, [a124]
mad r2, r1, r3 | ld r1, [034a] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a17c] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a174] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a134] | ld r3, [c055]
mad r2, r1, r3 | ld r0, [a134] | ld r1, [0390]
st r2, [0356] | ld r2, [c056]
mul r0, r0, r2 | ld r3, [c057]
mad r0, r1, r3 | ld r1, [0350] | ld r3, [034e]
st r0, [0358] | ld r0, [0356] | st r1, [a1ac]
st r0, [a1a4] | ld r0, [0352]
ld r2, [0358] | st r0, [a1e4]
st r2, [a1c4] | ld r1, [035b]
ld r0, [a065]
subxy r1, r1, r0 | ld r2, [a03d] | st r0, [035a]
mul r1, r1, r1 | st r2, [a165] | ld r2, [035f]
mad r2, r2, -r1 | st r0, [a16d] | ld r0, [035d]
addxy r1, r0, #-0.999 | st r3, [a1dc] | ld r3, [c049]
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [035e]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a125] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [035c]
addxy r1, r1, r2 | ld r2, [a035]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [0360]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a045]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [0362] | ld r1, [0367]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a04d]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0364] | ld r2, [a045]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [0366]
ph_clamp r1, r3, -r1
st r1, [a135]
ld r1, [a125]
mad r2, r1, r3 | ld r1, [035c] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a17d] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a175] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a135] | ld r3, [c055]
mad r2, r1, r3 | ld r0, [a135] | ld r1, [0390]
st r2, [0368] | ld r2, [c056]
mul r0, r0, r2 | ld r3, [c057]
mad r0, r1, r3 | ld r1, [0362] | ld r3, [0360]
st r0, [036a] | ld r0, [0368] | st r1, [a1ad]
st r0, [a1a5] | ld r0, [0364]
ld r2, [036a] | st r0, [a1e5]
st r2, [a1c5] | ld r1, [036d]
ld r0, [a066]
subxy r1, r1, r0 | ld r2, [a03e] | st r0, [036c]
mul r1, r1, r1 | st r2, [a166] | ld r2, [0371]
mad r2, r2, -r1 | st r0, [a16e] | ld r0, [036f]
addxy r1, r0, #-0.999 | st r3, [a1dd] | ld r3, [c049]
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0370]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a126] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [036e]
addxy r1, r1, r2 | ld r2, [a036]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [0372]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a046]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [0374] | ld r1, [0379]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a04e]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0376] | ld r2, [a046]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [0378]
ph_clamp r1, r3, -r1
st r1, [a136]
ld r1, [a126]
mad r2, r1, r3 | ld r1, [036e] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a17e] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a176] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a136] | ld r3, [c055]
mad r2, r1, r3 | ld r0, [a136] | ld r1, [0390]
st r2, [037a] | ld r2, [c056]
mul r0, r0, r2 | ld r3, [c057]
mad r0, r1, r3 | ld r1, [0374] | ld r3, [0372]
st r0, [037c] | ld r0, [037a] | st r1, [a1ae]
st r0, [a1a6] | ld r0, [0376]
ld r2, [037c] | st r0, [a1e6]
st r2, [a1c6] | ld r1, [037f]
ld r0, [a067]
subxy r1, r1, r0 | ld r2, [a03f] | st r0, [037e]
mul r1, r1, r1 | st r2, [a167] | ld r2, [0383]
mad r2, r2, -r1 | st r0, [a16f] | ld r0, [0381]
addxy r1, r0, #-0.999 | st r3, [a1de] | ld r3, [c049]
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0382]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a127] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [0380]
addxy r1, r1, r2 | ld r2, [a037]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [0384]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a047]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [0386] | ld r1, [038b]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a04f]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0388] | ld r2, [a047]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [038a]
ph_clamp r1, r3, -r1
st r1, [a137]
ld r1, [a127]
mad r2, r1, r3 | ld r1, [0380] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a17f] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a177] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a137] | ld r3, [c055]
mad r2, r1, r3 | ld r0, [a137] | ld r1, [0390]
st r2, [038c] | ld r2, [c056]
mul r0, r0, r2 | ld r3, [c057]
mad r0, r1, r3 | ld r1, [0386] | ld r3, [0384]
st r0, [038e] | ld r0, [038c] | st r1, [a1af]
st r0, [a1a7] | ld r2, [038e]
ld r0, [0388] | st r2, [a1c7]
st r3, [a1df]
st r0, [a1e7]
