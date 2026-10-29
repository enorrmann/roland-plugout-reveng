mov r4, #0.041666666666666664 | ld r0, [03e3] | ld r2, [03e1] | ld r3, [c049]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | ld r1, [0301]
ph_wrap r0, r3, r0
addxy r2, r2, r0 | mov r7, r4 | st r0, [03e2] | ld r0, [a060]
ph_wrap r2, r3, r2 | st r0, [a168] | st r0, [0300]
subxy r1, r1, r0 | st r2, [03e0] | ld r2, [a038]
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
mul r2, r1, #0.08664339756999316 | st r2, [a1b0] | ld r1, [0309]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a040]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0306] | ld r2, [a040]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [0308]
ph_clamp r1, r3, -r1
st r1, [a130]
ld r1, [a120]
mad r2, r1, r3 | ld r1, [0302] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a178] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a170] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a130] | ld r3, [c055]
mad r2, r1, r3
st r2, [030a]
ld r0, [030a]
addxy r1, r0, #0.029999999329447746
mul r2, r1, r1
mul r3, r2, #0.0043321699430540197
ymacd r7, r3, #0.16666666666666666
ymacd r7, r3, #0.5
ymacd r7, r3, #1.0
ymacd r7, r3, #1.0
mul r0, r7, r7
mul r1, r0, r0 | mul r0, r2, #-0.0043321699430540197
ymacd r4, r0, #0.16666666666666666 | mul r3, r1, r1
ymacd r4, r0, #0.5 | mul r7, r3, r3
ymacd r4, r0, #1.0
ymacd r4, r0, #1.0
mul r1, r4, r4 | mul r4, r7, r7
mul r3, r1, r1 | st r4, [031e]
mul r2, r3, r3
mul r0, r2, r2 | mul r2, r7, r4 | ld r4, [030f]
mul r1, r0, r0 | st r2, [031c] | ld r2, [0306]
mul r3, r0, r1
mac_p1wc3 r3, r2, r3, #0.5, r4
mul r4, r3, #2.0
ph_wrap r3, r3, r4 | ld r4, [0311]
mac_p1wc3 r1, r2, r1, #0.5, r4 | st r3, [030e]
mul r4, r1, #2.0
ph_wrap r1, r3, r4 | ld r4, [0313]
mac_p1wc3 r0, r0, r2, #0.5, r4 | st r1, [0310]
mul r4, r0, #2.0
ph_wrap r0, r3, r4
sum8 r4, r3, r1, r0 | ld r3, [0315] | st r0, [0312]
mov r1, r2
mad r1, r3, #0.5
mul r0, r1, #2.0 | ld r1, [0317]
ph_wrap r3, r3, r0
mac_p1wc3 r0, r7, r2, #0.5, r1 | st r3, [0314]
mul r7, r0, #2.0
ph_wrap r1, r3, r7 | ld r7, [0319]
sum8 r0, r4, r3, r1 | ld r4, [031e] | st r1, [0316]
mac_p1wc3 r3, r4, r2, #0.5, r7 | ld r7, [031b]
mul r1, r3, #2.0 | ld r3, [031c]
ph_wrap r4, r3, r1
mac_p1wc3 r1, r2, r3, #0.5, r7 | st r4, [0318]
mul r2, r1, #2.0 | ld r1, [c070]
ph_wrap r3, r3, r2
sum8 r7, r0, r4, r3 | st r3, [031a] | ld r3, [c049]
mul r2, r1, r7 | mov r4, #0.041666666666666664 | ld r1, [a130]
mul r0, r2, #0.25 | mov r7, r4 | ld r2, [c056]
st r0, [030c]
ld r0, [030c]
mad r0, r1, r2 | ld r1, [03e0] | ld r2, [c057]
mad r0, r1, r2 | ld r1, [0321] | ld r2, [a039]
st r0, [a151]
ld r0, [a061]
subxy r1, r1, r0 | st r2, [a161] | st r0, [0320] | ld r2, [0325]
mul r1, r1, r1 | st r0, [a169] | ld r0, [0323]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0324]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a121] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [0322]
addxy r1, r1, r2 | ld r2, [a031]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [a1b1] | ld r1, [0329]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a041]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0326] | ld r2, [a041]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [0328]
ph_clamp r1, r3, -r1
st r1, [a131]
ld r1, [a121]
mad r2, r1, r3 | ld r1, [0322] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a179] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a171] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a131] | ld r3, [c055]
mad r2, r1, r3
st r2, [032a]
ld r0, [032a]
addxy r1, r0, #0.029999999329447746
mul r2, r1, r1
mul r3, r2, #0.0043321699430540197
ymacd r7, r3, #0.16666666666666666
ymacd r7, r3, #0.5
ymacd r7, r3, #1.0
ymacd r7, r3, #1.0
mul r0, r7, r7
mul r1, r0, r0 | mul r0, r2, #-0.0043321699430540197
ymacd r4, r0, #0.16666666666666666 | mul r3, r1, r1
ymacd r4, r0, #0.5 | mul r7, r3, r3
ymacd r4, r0, #1.0
ymacd r4, r0, #1.0
mul r1, r4, r4 | mul r4, r7, r7
mul r3, r1, r1 | st r4, [033e]
mul r2, r3, r3
mul r0, r2, r2 | mul r2, r7, r4 | ld r4, [032f]
mul r1, r0, r0 | st r2, [033c] | ld r2, [0326]
mul r3, r0, r1
mac_p1wc3 r3, r2, r3, #0.5, r4
mul r4, r3, #2.0
ph_wrap r3, r3, r4 | ld r4, [0331]
mac_p1wc3 r1, r2, r1, #0.5, r4 | st r3, [032e]
mul r4, r1, #2.0
ph_wrap r1, r3, r4 | ld r4, [0333]
mac_p1wc3 r0, r0, r2, #0.5, r4 | st r1, [0330]
mul r4, r0, #2.0
ph_wrap r0, r3, r4
sum8 r4, r3, r1, r0 | ld r3, [0335] | st r0, [0332]
mov r1, r2
mad r1, r3, #0.5
mul r0, r1, #2.0 | ld r1, [0337]
ph_wrap r3, r3, r0
mac_p1wc3 r0, r7, r2, #0.5, r1 | st r3, [0334]
mul r7, r0, #2.0
ph_wrap r1, r3, r7 | ld r7, [0339]
sum8 r0, r4, r3, r1 | ld r4, [033e] | st r1, [0336]
mac_p1wc3 r3, r4, r2, #0.5, r7 | ld r7, [033b]
mul r1, r3, #2.0 | ld r3, [033c]
ph_wrap r4, r3, r1
mac_p1wc3 r1, r2, r3, #0.5, r7 | st r4, [0338]
mul r2, r1, #2.0 | ld r1, [c070]
ph_wrap r3, r3, r2
sum8 r7, r0, r4, r3 | st r3, [033a] | ld r3, [c049]
mul r2, r1, r7 | mov r4, #0.041666666666666664 | ld r1, [a131]
mul r0, r2, #0.25 | mov r7, r4 | ld r2, [c056]
st r0, [032c]
ld r0, [032c]
mad r0, r1, r2 | ld r1, [03e0] | ld r2, [c057]
mad r0, r1, r2 | ld r1, [0341] | ld r2, [a03a]
st r0, [a152]
ld r0, [a062]
subxy r1, r1, r0 | st r2, [a162] | st r0, [0340] | ld r2, [0345]
mul r1, r1, r1 | st r0, [a16a] | ld r0, [0343]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0344]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a122] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [0342]
addxy r1, r1, r2 | ld r2, [a032]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [a1b2] | ld r1, [0349]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a042]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0346] | ld r2, [a042]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [0348]
ph_clamp r1, r3, -r1
st r1, [a132]
ld r1, [a122]
mad r2, r1, r3 | ld r1, [0342] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a17a] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a172] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a132] | ld r3, [c055]
mad r2, r1, r3
st r2, [034a]
ld r0, [034a]
addxy r1, r0, #0.029999999329447746
mul r2, r1, r1
mul r3, r2, #0.0043321699430540197
ymacd r7, r3, #0.16666666666666666
ymacd r7, r3, #0.5
ymacd r7, r3, #1.0
ymacd r7, r3, #1.0
mul r0, r7, r7
mul r1, r0, r0 | mul r0, r2, #-0.0043321699430540197
ymacd r4, r0, #0.16666666666666666 | mul r3, r1, r1
ymacd r4, r0, #0.5 | mul r7, r3, r3
ymacd r4, r0, #1.0
ymacd r4, r0, #1.0
mul r1, r4, r4 | mul r4, r7, r7
mul r3, r1, r1 | st r4, [035e]
mul r2, r3, r3
mul r0, r2, r2 | mul r2, r7, r4 | ld r4, [034f]
mul r1, r0, r0 | st r2, [035c] | ld r2, [0346]
mul r3, r0, r1
mac_p1wc3 r3, r2, r3, #0.5, r4
mul r4, r3, #2.0
ph_wrap r3, r3, r4 | ld r4, [0351]
mac_p1wc3 r1, r2, r1, #0.5, r4 | st r3, [034e]
mul r4, r1, #2.0
ph_wrap r1, r3, r4 | ld r4, [0353]
mac_p1wc3 r0, r0, r2, #0.5, r4 | st r1, [0350]
mul r4, r0, #2.0
ph_wrap r0, r3, r4
sum8 r4, r3, r1, r0 | ld r3, [0355] | st r0, [0352]
mov r1, r2
mad r1, r3, #0.5
mul r0, r1, #2.0 | ld r1, [0357]
ph_wrap r3, r3, r0
mac_p1wc3 r0, r7, r2, #0.5, r1 | st r3, [0354]
mul r7, r0, #2.0
ph_wrap r1, r3, r7 | ld r7, [0359]
sum8 r0, r4, r3, r1 | ld r4, [035e] | st r1, [0356]
mac_p1wc3 r3, r4, r2, #0.5, r7 | ld r7, [035b]
mul r1, r3, #2.0 | ld r3, [035c]
ph_wrap r4, r3, r1
mac_p1wc3 r1, r2, r3, #0.5, r7 | st r4, [0358]
mul r2, r1, #2.0 | ld r1, [c070]
ph_wrap r3, r3, r2
sum8 r7, r0, r4, r3 | st r3, [035a] | ld r3, [c049]
mul r2, r1, r7 | mov r4, #0.041666666666666664 | ld r1, [a132]
mul r0, r2, #0.25 | mov r7, r4 | ld r2, [c056]
st r0, [034c]
ld r0, [034c]
mad r0, r1, r2 | ld r1, [03e0] | ld r2, [c057]
mad r0, r1, r2 | ld r1, [0361] | ld r2, [a03b]
st r0, [a153]
ld r0, [a063]
subxy r1, r1, r0 | st r2, [a163] | st r0, [0360] | ld r2, [0365]
mul r1, r1, r1 | st r0, [a16b] | ld r0, [0363]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0364]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a123] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [0362]
addxy r1, r1, r2 | ld r2, [a033]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [a1b3] | ld r1, [0369]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a043]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0366] | ld r2, [a043]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [0368]
ph_clamp r1, r3, -r1
st r1, [a133]
ld r1, [a123]
mad r2, r1, r3 | ld r1, [0362] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a17b] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a173] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a133] | ld r3, [c055]
mad r2, r1, r3
st r2, [036a]
ld r0, [036a]
addxy r1, r0, #0.029999999329447746
mul r2, r1, r1
mul r3, r2, #0.0043321699430540197
ymacd r7, r3, #0.16666666666666666
ymacd r7, r3, #0.5
ymacd r7, r3, #1.0
ymacd r7, r3, #1.0
mul r0, r7, r7
mul r1, r0, r0 | mul r0, r2, #-0.0043321699430540197
ymacd r4, r0, #0.16666666666666666 | mul r3, r1, r1
ymacd r4, r0, #0.5 | mul r7, r3, r3
ymacd r4, r0, #1.0
ymacd r4, r0, #1.0
mul r1, r4, r4 | mul r4, r7, r7
mul r3, r1, r1 | st r4, [037e]
mul r2, r3, r3
mul r0, r2, r2 | mul r2, r7, r4 | ld r4, [036f]
mul r1, r0, r0 | st r2, [037c] | ld r2, [0366]
mul r3, r0, r1
mac_p1wc3 r3, r2, r3, #0.5, r4
mul r4, r3, #2.0
ph_wrap r3, r3, r4 | ld r4, [0371]
mac_p1wc3 r1, r2, r1, #0.5, r4 | st r3, [036e]
mul r4, r1, #2.0
ph_wrap r1, r3, r4 | ld r4, [0373]
mac_p1wc3 r0, r0, r2, #0.5, r4 | st r1, [0370]
mul r4, r0, #2.0
ph_wrap r0, r3, r4
sum8 r4, r3, r1, r0 | ld r3, [0375] | st r0, [0372]
mov r1, r2
mad r1, r3, #0.5
mul r0, r1, #2.0 | ld r1, [0377]
ph_wrap r3, r3, r0
mac_p1wc3 r0, r7, r2, #0.5, r1 | st r3, [0374]
mul r7, r0, #2.0
ph_wrap r1, r3, r7 | ld r7, [0379]
sum8 r0, r4, r3, r1 | ld r4, [037e] | st r1, [0376]
mac_p1wc3 r3, r4, r2, #0.5, r7 | ld r7, [037b]
mul r1, r3, #2.0 | ld r3, [037c]
ph_wrap r4, r3, r1
mac_p1wc3 r1, r2, r3, #0.5, r7 | st r4, [0378]
mul r2, r1, #2.0 | ld r1, [c070]
ph_wrap r3, r3, r2
sum8 r7, r0, r4, r3 | st r3, [037a] | ld r3, [c049]
mul r2, r1, r7 | mov r4, #0.041666666666666664 | ld r1, [a133]
mul r0, r2, #0.25 | mov r7, r4 | ld r2, [c056]
st r0, [036c]
ld r0, [036c]
mad r0, r1, r2 | ld r1, [03e0] | ld r2, [c057]
mad r0, r1, r2 | ld r1, [0381] | ld r2, [a03c]
st r0, [a154]
ld r0, [a064]
subxy r1, r1, r0 | st r2, [a164] | st r0, [0380] | ld r2, [0385]
mul r1, r1, r1 | st r0, [a16c] | ld r0, [0383]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0384]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a124] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [0382]
addxy r1, r1, r2 | ld r2, [a034]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [a1b4] | ld r1, [0389]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a044]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [0386] | ld r2, [a044]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [0388]
ph_clamp r1, r3, -r1
st r1, [a134]
ld r1, [a124]
mad r2, r1, r3 | ld r1, [0382] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a17c] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a174] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a134] | ld r3, [c055]
mad r2, r1, r3
st r2, [038a]
ld r0, [038a]
addxy r1, r0, #0.029999999329447746
mul r2, r1, r1
mul r3, r2, #0.0043321699430540197
ymacd r7, r3, #0.16666666666666666
ymacd r7, r3, #0.5
ymacd r7, r3, #1.0
ymacd r7, r3, #1.0
mul r0, r7, r7
mul r1, r0, r0 | mul r0, r2, #-0.0043321699430540197
ymacd r4, r0, #0.16666666666666666 | mul r3, r1, r1
ymacd r4, r0, #0.5 | mul r7, r3, r3
ymacd r4, r0, #1.0
ymacd r4, r0, #1.0
mul r1, r4, r4 | mul r4, r7, r7
mul r3, r1, r1 | st r4, [039e]
mul r2, r3, r3
mul r0, r2, r2 | mul r2, r7, r4 | ld r4, [038f]
mul r1, r0, r0 | st r2, [039c] | ld r2, [0386]
mul r3, r0, r1
mac_p1wc3 r3, r2, r3, #0.5, r4
mul r4, r3, #2.0
ph_wrap r3, r3, r4 | ld r4, [0391]
mac_p1wc3 r1, r2, r1, #0.5, r4 | st r3, [038e]
mul r4, r1, #2.0
ph_wrap r1, r3, r4 | ld r4, [0393]
mac_p1wc3 r0, r0, r2, #0.5, r4 | st r1, [0390]
mul r4, r0, #2.0
ph_wrap r0, r3, r4
sum8 r4, r3, r1, r0 | ld r3, [0395] | st r0, [0392]
mov r1, r2
mad r1, r3, #0.5
mul r0, r1, #2.0 | ld r1, [0397]
ph_wrap r3, r3, r0
mac_p1wc3 r0, r7, r2, #0.5, r1 | st r3, [0394]
mul r7, r0, #2.0
ph_wrap r1, r3, r7 | ld r7, [0399]
sum8 r0, r4, r3, r1 | ld r4, [039e] | st r1, [0396]
mac_p1wc3 r3, r4, r2, #0.5, r7 | ld r7, [039b]
mul r1, r3, #2.0 | ld r3, [039c]
ph_wrap r4, r3, r1
mac_p1wc3 r1, r2, r3, #0.5, r7 | st r4, [0398]
mul r2, r1, #2.0 | ld r1, [c070]
ph_wrap r3, r3, r2
sum8 r7, r0, r4, r3 | st r3, [039a] | ld r3, [c049]
mul r2, r1, r7 | mov r4, #0.041666666666666664 | ld r1, [a134]
mul r0, r2, #0.25 | mov r7, r4 | ld r2, [c056]
st r0, [038c]
ld r0, [038c]
mad r0, r1, r2 | ld r1, [03e0] | ld r2, [c057]
mad r0, r1, r2 | ld r1, [03a1] | ld r2, [a03d]
st r0, [a155]
ld r0, [a065]
subxy r1, r1, r0 | st r2, [a165] | st r0, [03a0] | ld r2, [03a5]
mul r1, r1, r1 | st r0, [a16d] | ld r0, [03a3]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [03a4]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a125] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [03a2]
addxy r1, r1, r2 | ld r2, [a035]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [a1b5] | ld r1, [03a9]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a045]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [03a6] | ld r2, [a045]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [03a8]
ph_clamp r1, r3, -r1
st r1, [a135]
ld r1, [a125]
mad r2, r1, r3 | ld r1, [03a2] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a17d] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a175] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a135] | ld r3, [c055]
mad r2, r1, r3
st r2, [03aa]
ld r0, [03aa]
addxy r1, r0, #0.029999999329447746
mul r2, r1, r1
mul r3, r2, #0.0043321699430540197
ymacd r7, r3, #0.16666666666666666
ymacd r7, r3, #0.5
ymacd r7, r3, #1.0
ymacd r7, r3, #1.0
mul r0, r7, r7
mul r1, r0, r0 | mul r0, r2, #-0.0043321699430540197
ymacd r4, r0, #0.16666666666666666 | mul r3, r1, r1
ymacd r4, r0, #0.5 | mul r7, r3, r3
ymacd r4, r0, #1.0
ymacd r4, r0, #1.0
mul r1, r4, r4 | mul r4, r7, r7
mul r3, r1, r1 | st r4, [03be]
mul r2, r3, r3
mul r0, r2, r2 | mul r2, r7, r4 | ld r4, [03af]
mul r1, r0, r0 | st r2, [03bc] | ld r2, [03a6]
mul r3, r0, r1
mac_p1wc3 r3, r2, r3, #0.5, r4
mul r4, r3, #2.0
ph_wrap r3, r3, r4 | ld r4, [03b1]
mac_p1wc3 r1, r2, r1, #0.5, r4 | st r3, [03ae]
mul r4, r1, #2.0
ph_wrap r1, r3, r4 | ld r4, [03b3]
mac_p1wc3 r0, r0, r2, #0.5, r4 | st r1, [03b0]
mul r4, r0, #2.0
ph_wrap r0, r3, r4
sum8 r4, r3, r1, r0 | ld r3, [03b5] | st r0, [03b2]
mov r1, r2
mad r1, r3, #0.5
mul r0, r1, #2.0 | ld r1, [03b7]
ph_wrap r3, r3, r0
mac_p1wc3 r0, r7, r2, #0.5, r1 | st r3, [03b4]
mul r7, r0, #2.0
ph_wrap r1, r3, r7 | ld r7, [03b9]
sum8 r0, r4, r3, r1 | ld r4, [03be] | st r1, [03b6]
mac_p1wc3 r3, r4, r2, #0.5, r7 | ld r7, [03bb]
mul r1, r3, #2.0 | ld r3, [03bc]
ph_wrap r4, r3, r1
mac_p1wc3 r1, r2, r3, #0.5, r7 | st r4, [03b8]
mul r2, r1, #2.0 | ld r1, [c070]
ph_wrap r3, r3, r2
sum8 r7, r0, r4, r3 | st r3, [03ba] | ld r3, [c049]
mul r2, r1, r7 | mov r4, #0.041666666666666664 | ld r1, [a135]
mul r0, r2, #0.25 | mov r7, r4 | ld r2, [c056]
st r0, [03ac]
ld r0, [03ac]
mad r0, r1, r2 | ld r1, [03e0] | ld r2, [c057]
mad r0, r1, r2 | ld r1, [03c1] | ld r2, [a03e]
st r0, [a156]
ld r0, [a066]
subxy r1, r1, r0 | st r2, [a166] | st r0, [03c0] | ld r2, [03c5]
mul r1, r1, r1 | st r0, [a16e] | ld r0, [03c3]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [03c4]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a126] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [03c2]
addxy r1, r1, r2 | ld r2, [a036]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [a1b6] | ld r1, [03c9]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a046]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
st r2, [03c6] | ld r2, [a046]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [03c8]
ph_clamp r1, r3, -r1
st r1, [a136]
ld r1, [a126]
mad r2, r1, r3 | ld r1, [03c2] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a17e] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a176] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a136] | ld r3, [c055]
mad r2, r1, r3
st r2, [03ca]
ld r0, [03ca]
addxy r1, r0, #0.029999999329447746
mul r2, r1, r1
mul r3, r2, #0.0043321699430540197
ymacd r7, r3, #0.16666666666666666
ymacd r7, r3, #0.5
ymacd r7, r3, #1.0
ymacd r7, r3, #1.0
mul r0, r7, r7
mul r1, r0, r0 | mul r0, r2, #-0.0043321699430540197
ymacd r4, r0, #0.16666666666666666 | mul r3, r1, r1
ymacd r4, r0, #0.5 | mul r7, r3, r3
ymacd r4, r0, #1.0
ymacd r4, r0, #1.0
mul r1, r4, r4 | mul r4, r7, r7
mul r3, r1, r1 | st r4, [03de]
mul r2, r3, r3
mul r0, r2, r2 | mul r2, r7, r4 | ld r4, [03cf]
mul r1, r0, r0 | st r2, [03dc] | ld r2, [03c6]
mul r3, r0, r1
mac_p1wc3 r3, r2, r3, #0.5, r4
mul r4, r3, #2.0
ph_wrap r3, r3, r4 | ld r4, [03d1]
mac_p1wc3 r1, r2, r1, #0.5, r4 | st r3, [03ce]
mul r4, r1, #2.0
ph_wrap r1, r3, r4 | ld r4, [03d3]
mac_p1wc3 r0, r0, r2, #0.5, r4 | st r1, [03d0]
mul r4, r0, #2.0
ph_wrap r0, r3, r4
sum8 r4, r3, r1, r0 | ld r3, [03d5] | st r0, [03d2]
mov r1, r2
mad r1, r3, #0.5
mul r0, r1, #2.0 | ld r1, [03d7]
ph_wrap r3, r3, r0
mac_p1wc3 r0, r7, r2, #0.5, r1 | st r3, [03d4]
mul r7, r0, #2.0
ph_wrap r1, r3, r7 | ld r7, [03d9]
sum8 r0, r4, r3, r1 | ld r4, [03de] | st r1, [03d6]
mac_p1wc3 r3, r4, r2, #0.5, r7 | ld r7, [03db]
mul r1, r3, #2.0 | ld r3, [03dc]
ph_wrap r4, r3, r1
mac_p1wc3 r1, r2, r3, #0.5, r7 | st r4, [03d8]
mul r2, r1, #2.0 | ld r1, [c070]
ph_wrap r3, r3, r2
sum8 r7, r0, r4, r3 | st r3, [03da]
mul r2, r1, r7 | ld r1, [a136]
mul r0, r2, #0.25 | ld r2, [c056]
st r0, [03cc]
ld r0, [03cc]
mad r0, r1, r2 | ld r1, [03e0] | ld r2, [c057]
mad r0, r1, r2
st r0, [a157]
