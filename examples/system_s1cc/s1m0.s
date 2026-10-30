ld r0, [040d] | ld r2, [040b] | ld r3, [c049]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | ld r1, [0301] | ld r4, [a061] | ld r5, [0327]
ph_wrap r0, r3, r0 | ld r6, [a039] | st r4, [0326] | ld r7, [c049]
addxy r2, r2, r0 | subxy r5, r5, r4 | st r0, [040c] | ld r0, [a060]
ph_wrap r2, r3, r2 | st r0, [a168] | st r0, [0300]
subxy r1, r1, r0 | mul r5, r5, r5 | st r2, [040a] | ld r2, [a038]
mul r1, r1, r1 | st r2, [a160] | ld r2, [0305]
mad r2, r2, -r1 | ld r0, [0303] | st r6, [a161]
addxy r1, r0, #-0.999 | st r4, [a169] | ld r6, [032b]
mul r1, r1, #256.0 | mad r6, r6, -r5 | ld r4, [0329]
ph_clamp r1, r3, -r1
mul r1, r1, #256.0 | addxy r5, r4, #-0.999
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0304]
mul r2, r2, #256.0 | mul r5, r5, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a120] | ld r2, [c04b]
mul r1, r1, r2 | ph_clamp r5, r3, -r5 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [0302]
addxy r1, r1, r2 | mul r5, r5, #256.0 | ld r2, [a030]
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
ph_clamp r5, r3, -r5 | st r2, [0306] | ld r2, [a040]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [0308]
ph_clamp r1, r3, -r1 | max r6, r6, r5 | ld r5, [c048]
subxy r6, r6, #1.0 | subxy r7, r5, r7 | st r1, [a130] | st r6, [032a]
mul r6, r6, #256.0 | ld r1, [a120]
mad r2, r1, r3 | ph_clamp r6, r3, -r6 | ld r1, [0302] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a178] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a170] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a130] | ld r3, [c055]
mad r2, r1, r3 | mad r5, r6, r7 | ld r7, [c051]
mul r6, r6, #1.25 | st r2, [030a]
subxy r6, r4, r6 | ld r0, [030a]
addxy r1, r0, #0.029999999329447746 | mad r4, r6, r5 | ld r5, [a121]
mul r2, r1, r1 | mov r0, #0.041666666666666664 | ld r6, [c04b]
mul r3, r2, #0.0043321699430540197 | mov r1, r0
ymacd r1, r3, #0.16666666666666666 | mul r5, r5, r6 | ld r6, [c05e]
ymacd r1, r3, #0.5
ymacd r1, r3, #1.0
ymacd r1, r3, #1.0
mul r3, r1, r1
mul r1, r3, r3
mul r3, r1, r1
mul r1, r3, r3 | mul r3, r2, #-0.0043321699430540197
ymacd r0, r3, #0.16666666666666666 | st r1, [031c]
ymacd r0, r3, #0.5
ymacd r0, r3, #1.0
ymacd r0, r3, #1.0
mul r2, r0, r0
mul r3, r2, r2
mul r0, r3, r3 | mul r3, r1, r1
mul r2, r0, r0 | st r3, [0324]
mul r0, r2, r2 | st r2, [031e]
mul r1, r2, r0 | ld r2, [031c] | st r0, [0320]
mul r0, r2, r3 | ld r3, [030f] | ld r2, [0320]
min r4, r4, #1.0 | st r0, [0322] | ld r0, [0306]
mac_p1wc3 r1, r0, r1, #0.5, r3 | st r4, [0328]
mul r3, r1, #2.0 | mad r5, r4, r6 | ld r6, [c05f]
ph_wrap r1, r3, r3 | ld r3, [0311]
mac_p1wc3 r2, r0, r2, #0.5, r3 | st r1, [030e]
mul r3, r2, #2.0 | addxy r5, r5, r6 | st r1, [0320] | ld r1, [031e]
ph_wrap r2, r3, r3 | ld r3, [0313] | ld r6, [a031]
mac_p1wc3 r1, r1, r0, #0.5, r3 | st r2, [0310]
mul r3, r1, #2.0 | addxy r6, r6, r5
ph_wrap r1, r3, r3 | ld r3, [0320] | st r6, [a1b1]
sum8 r3, r3, r2, r1 | st r1, [0312] | ld r2, [0315]
mov r1, r0 | mul r6, r5, #0.08664339756999316 | st r3, [0320] | ld r3, [031c]
mad r1, r2, #0.5 | mov r4, #0.041666666666666664 | ld r5, [032f]
mul r2, r1, #2.0 | ymacd r4, r6, #0.16666666666666666
ph_wrap r1, r3, r2 | ld r2, [0317]
mac_p1wc3 r3, r3, r0, #0.5, r2 | st r1, [0314]
mul r2, r3, #2.0 | ymacd r4, r6, #0.5
ph_wrap r3, r3, r2 | ld r2, [0320]
sum8 r2, r2, r1, r3 | st r3, [0316] | ld r1, [0319]
ymacd r4, r6, #1.0 | ld r3, [0324]
mac_p1wc3 r3, r3, r0, #0.5, r1 | st r2, [0324] | ld r2, [0322]
mul r1, r3, #2.0 | ymacd r4, r6, #1.0 | ld r6, [a041]
ph_wrap r3, r3, r1 | ld r1, [031b]
mac_p1wc3 r0, r0, r2, #0.5, r1 | st r3, [0318]
mul r2, r0, #2.0 | mul r4, r4, r4 | ld r0, [0324]
ph_wrap r1, r3, r2
sum8 r2, r0, r3, r1 | ld r0, [c070] | st r1, [031a]
mul r3, r0, r2 | mul r4, r4, r4 | ld r2, [c056]
mul r1, r3, #0.25 | mul r4, r4, r4 | ld r3, [c049]
mul r6, r6, r4 | st r1, [030c] | ld r1, [a130]
ld r0, [030c] | st r6, [032c]
mad r0, r1, r2 | ld r1, [040a] | ld r2, [c057]
mad r0, r1, r2 | ld r1, [034d] | ld r2, [a03a]
st r0, [a151]
ld r0, [a062]
subxy r1, r1, r0 | st r2, [a162] | st r0, [034c] | ld r2, [0351]
mul r1, r1, r1 | st r0, [a16a] | ld r0, [034f]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | ld r6, [a041]
mul r1, r1, #256.0 | addxy r6, r5, r6
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0350]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a122] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [034e]
addxy r1, r1, r2 | ld r2, [a032]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [a1b2] | ld r1, [0355]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a042]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0
ph_wrap r5, r3, r6 | st r2, [0352] | ld r2, [a042] | ld r6, [c050]
addxy r2, r1, r2 | mul r5, r5, #256.0 | st r5, [032e]
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [0354]
ph_clamp r5, r3, -r5
ph_clamp r1, r3, -r1 | st r5, [a131]
st r1, [a132]
ld r5, [a121]
mad r6, r5, r7 | ld r1, [a122] | ld r5, [0328]
mad r2, r1, r3 | ld r7, [c052] | ld r1, [034e]
mad r6, r5, r7 | ld r3, [c052] | ld r5, [a179]
mad r2, r1, r3 | ld r7, [c053] | ld r1, [a17a]
mad r6, r5, r7 | ld r3, [c053] | ld r5, [a171]
mad r2, r1, r3 | ld r7, [c054] | ld r1, [a172]
mad r6, r5, r7 | ld r3, [c054] | ld r5, [a131]
mad r2, r1, r3 | ld r7, [c055] | ld r1, [a132]
mad r6, r5, r7 | ld r3, [c055]
mad r2, r1, r3 | st r6, [0330]
st r2, [0356] | ld r4, [0330]
addxy r5, r4, #0.029999999329447746 | ld r0, [0356]
addxy r1, r0, #0.029999999329447746 | mul r6, r5, r5
mov r4, #0.041666666666666664 | mul r2, r1, r1
mov r0, #0.041666666666666664 | mul r7, r6, #0.0043321699430540197
mov r5, r4 | mul r3, r2, #0.0043321699430540197
mov r1, r0
ymacd r5, r7, #0.16666666666666666 | ymacd r1, r3, #0.16666666666666666
ymacd r5, r7, #0.5 | ymacd r1, r3, #0.5
ymacd r5, r7, #1.0 | ymacd r1, r3, #1.0
ymacd r5, r7, #1.0 | ymacd r1, r3, #1.0
mul r7, r5, r5 | mul r3, r1, r1
mul r5, r7, r7 | mul r1, r3, r3
mul r7, r5, r5 | mul r3, r1, r1
mul r5, r7, r7 | mul r1, r3, r3
mul r7, r6, #-0.0043321699430540197 | mul r3, r2, #-0.0043321699430540197 | st r5, [0342]
ymacd r4, r7, #0.16666666666666666 | ymacd r0, r3, #0.16666666666666666 | st r1, [0368]
ymacd r4, r7, #0.5 | ymacd r0, r3, #0.5
ymacd r4, r7, #1.0 | ymacd r0, r3, #1.0
ymacd r4, r7, #1.0 | ymacd r0, r3, #1.0
mul r6, r4, r4 | mul r2, r0, r0
mul r7, r6, r6 | mul r3, r2, r2
mul r4, r7, r7 | mul r0, r3, r3
mul r6, r4, r4 | mul r2, r0, r0
mul r4, r6, r6 | mul r0, r2, r2 | st r6, [0344]
mul r7, r5, r5 | mul r3, r1, r1 | st r2, [036a]
mul r5, r6, r4 | mul r1, r2, r0 | ld r6, [0342] | ld r2, [0368]
st r4, [0346] | st r0, [036c]
mul r4, r6, r7 | mul r0, r2, r3 | st r7, [034a] | ld r7, [0335]
st r4, [0348] | st r0, [036e] | ld r4, [032c]
mac_p1wc3 r5, r4, r5, #0.5, r7 | st r3, [0370] | ld r0, [0352]
mul r7, r5, #2.0 | ld r3, [035b] | ld r6, [0346]
mac_p1wc3 r1, r0, r1, #0.5, r3 | ld r2, [036c]
mul r3, r1, #2.0
ph_wrap r5, r3, r7 | ld r7, [0337]
ph_wrap r1, r3, r3 | ld r3, [035d] | st r5, [0334]
mac_p1wc3 r6, r4, r6, #0.5, r7 | st r5, [0346] | ld r5, [0344]
mac_p1wc3 r2, r0, r2, #0.5, r3 | st r1, [035a]
mul r7, r6, #2.0 | mul r3, r2, #2.0 | st r1, [036c] | ld r1, [036a]
ph_wrap r6, r3, r7 | ld r7, [0339]
ph_wrap r2, r3, r3 | ld r3, [035f] | st r6, [0336]
mac_p1wc3 r5, r5, r4, #0.5, r7 | st r2, [035c]
mac_p1wc3 r1, r1, r0, #0.5, r3
mul r7, r5, #2.0 | mul r3, r1, #2.0
ph_wrap r5, r3, r7 | ld r7, [0346]
ph_wrap r1, r3, r3 | ld r3, [036c] | st r5, [0338]
sum8 r7, r7, r6, r5 | st r1, [035e] | ld r6, [033b]
sum8 r3, r3, r2, r1 | ld r2, [0361] | st r7, [0346]
mov r5, r4 | mov r1, r0 | st r3, [036c] | ld r7, [0342]
mad r5, r6, #0.5 | mad r1, r2, #0.5 | ld r3, [0368]
mul r6, r5, #2.0 | mul r2, r1, #2.0
ph_wrap r5, r3, r6 | ld r6, [033d]
ph_wrap r1, r3, r2 | ld r2, [0363] | st r5, [033a]
mac_p1wc3 r7, r7, r4, #0.5, r6 | st r1, [0360]
mac_p1wc3 r3, r3, r0, #0.5, r2
mul r6, r7, #2.0 | mul r2, r3, #2.0
ph_wrap r7, r3, r6 | ld r6, [0346]
ph_wrap r3, r3, r2 | ld r2, [036c] | st r7, [033c]
sum8 r6, r6, r5, r7 | st r3, [0362] | ld r5, [033f]
sum8 r2, r2, r1, r3 | ld r7, [034a] | ld r1, [0365]
mac_p1wc3 r7, r7, r4, #0.5, r5 | ld r3, [0370] | st r6, [034a]
mac_p1wc3 r3, r3, r0, #0.5, r1 | st r2, [0370] | ld r6, [0348]
mul r5, r7, #2.0 | mul r1, r3, #2.0 | ld r2, [036e]
ph_wrap r7, r3, r5 | ld r5, [0341]
ph_wrap r3, r3, r1 | ld r1, [0367] | st r7, [033e]
mac_p1wc3 r4, r4, r6, #0.5, r5 | st r3, [0364]
mac_p1wc3 r0, r0, r2, #0.5, r1
mul r6, r4, #2.0 | mul r2, r0, #2.0 | ld r4, [034a] | ld r0, [0370]
ph_wrap r5, r3, r6
ph_wrap r1, r3, r2 | st r5, [0340]
sum8 r6, r4, r7, r5 | ld r4, [c070] | st r1, [0366]
sum8 r2, r0, r3, r1 | ld r0, [c070]
mul r7, r4, r6 | mul r3, r0, r2 | ld r6, [c056]
mul r5, r7, #0.25 | mul r1, r3, #0.25 | ld r2, [c056]
st r5, [0332] | st r1, [0358]
ld r4, [0332] | ld r5, [a131] | ld r0, [0358]
mad r4, r5, r6 | ld r1, [a132] | ld r5, [040a]
mad r0, r1, r2 | ld r6, [c057] | ld r1, [040a]
mad r4, r5, r6 | ld r2, [c057] | ld r5, [0373]
mad r0, r1, r2 | st r4, [a152] | ld r1, [0399] | ld r7, [c049]
st r0, [a153] | ld r3, [c049]
ld r4, [a063]
subxy r5, r5, r4 | ld r0, [a064] | st r4, [0372]
subxy r1, r1, r0 | mul r5, r5, r5 | ld r6, [a03b] | st r0, [0398]
mul r1, r1, r1 | ld r2, [a03c]
st r6, [a163] | ld r6, [0377]
mad r6, r6, -r5 | st r2, [a164] | ld r2, [039d]
mad r2, r2, -r1 | st r4, [a16b] | ld r4, [0375]
addxy r5, r4, #-0.999 | st r0, [a16c] | ld r0, [039b]
addxy r1, r0, #-0.999 | mul r5, r5, #256.0
mul r1, r1, #256.0
ph_clamp r5, r3, -r5
ph_clamp r1, r3, -r1
mul r5, r5, #256.0 | mul r1, r1, #256.0
ph_clamp r5, r3, -r5
ph_clamp r1, r3, -r1 | max r6, r6, r5 | ld r5, [c048]
max r2, r2, r1 | st r6, [0376] | ld r1, [c048]
subxy r6, r6, #1.0 | subxy r2, r2, #1.0 | st r2, [039c]
mul r6, r6, #256.0 | mul r2, r2, #256.0
ph_clamp r6, r3, -r6
subxy r7, r5, r7 | subxy r3, r1, r3
ph_clamp r2, r3, -r2
mad r5, r6, r7 | mad r1, r2, r3 | ld r7, [c051]
mul r6, r6, #1.25 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r6, r4, r6 | subxy r2, r0, r2
mad r4, r6, r5 | mad r0, r2, r1 | ld r5, [a123]
min r4, r4, #1.0 | ld r6, [c04b] | ld r1, [a124]
mul r5, r5, r6 | ld r2, [c04b] | st r4, [0374]
mul r1, r1, r2 | ld r6, [c05e]
min r0, r0, #1.0 | ld r2, [c05e]
mad r5, r4, r6 | mad r1, r0, r2 | ld r6, [c05f] | st r0, [039a]
addxy r5, r5, r6 | ld r2, [c05f] | ld r6, [a033]
addxy r1, r1, r2 | addxy r6, r6, r5 | ld r2, [a034]
addxy r2, r2, r1 | mul r6, r5, #0.08664339756999316 | st r6, [a1b3] | ld r5, [037b]
mov r4, #0.041666666666666664 | mul r2, r1, #0.08664339756999316 | st r2, [a1b4] | ld r1, [03a1]
mov r0, #0.041666666666666664 | ymacd r4, r6, #0.16666666666666666
ymacd r0, r2, #0.16666666666666666 | ymacd r4, r6, #0.5
ymacd r0, r2, #0.5 | ymacd r4, r6, #1.0
ymacd r0, r2, #1.0 | ymacd r4, r6, #1.0 | ld r6, [a043]
ymacd r0, r2, #1.0 | mul r4, r4, r4 | ld r2, [a044]
mul r0, r0, r0 | mul r4, r4, r4
mul r0, r0, r0 | mul r4, r4, r4
mul r0, r0, r0 | mul r6, r6, r4
mul r2, r2, r0 | st r6, [0378] | ld r6, [a043]
addxy r6, r5, r6 | st r2, [039e] | ld r2, [a044]
addxy r2, r1, r2
ph_wrap r5, r3, r6 | ld r6, [c050]
ph_wrap r1, r3, r2 | st r5, [037a] | ld r2, [c050]
mul r5, r5, #256.0 | mul r1, r1, #256.0 | st r1, [03a0]
ph_clamp r5, r3, -r5
ph_clamp r1, r3, -r1 | st r5, [a133]
st r1, [a134]
ld r5, [a123]
mad r6, r5, r7 | ld r1, [a124] | ld r5, [0374]
mad r2, r1, r3 | ld r7, [c052] | ld r1, [039a]
mad r6, r5, r7 | ld r3, [c052] | ld r5, [a17b]
mad r2, r1, r3 | ld r7, [c053] | ld r1, [a17c]
mad r6, r5, r7 | ld r3, [c053] | ld r5, [a173]
mad r2, r1, r3 | ld r7, [c054] | ld r1, [a174]
mad r6, r5, r7 | ld r3, [c054] | ld r5, [a133]
mad r2, r1, r3 | ld r7, [c055] | ld r1, [a134]
mad r6, r5, r7 | ld r3, [c055]
mad r2, r1, r3 | st r6, [037c]
st r2, [03a2] | ld r4, [037c]
addxy r5, r4, #0.029999999329447746 | ld r0, [03a2]
addxy r1, r0, #0.029999999329447746 | mul r6, r5, r5
mov r4, #0.041666666666666664 | mul r2, r1, r1
mov r0, #0.041666666666666664 | mul r7, r6, #0.0043321699430540197
mov r5, r4 | mul r3, r2, #0.0043321699430540197
mov r1, r0
ymacd r5, r7, #0.16666666666666666 | ymacd r1, r3, #0.16666666666666666
ymacd r5, r7, #0.5 | ymacd r1, r3, #0.5
ymacd r5, r7, #1.0 | ymacd r1, r3, #1.0
ymacd r5, r7, #1.0 | ymacd r1, r3, #1.0
mul r7, r5, r5 | mul r3, r1, r1
mul r5, r7, r7 | mul r1, r3, r3
mul r7, r5, r5 | mul r3, r1, r1
mul r5, r7, r7 | mul r1, r3, r3
mul r7, r6, #-0.0043321699430540197 | mul r3, r2, #-0.0043321699430540197 | st r5, [038e]
ymacd r4, r7, #0.16666666666666666 | ymacd r0, r3, #0.16666666666666666 | st r1, [03b4]
ymacd r4, r7, #0.5 | ymacd r0, r3, #0.5
ymacd r4, r7, #1.0 | ymacd r0, r3, #1.0
ymacd r4, r7, #1.0 | ymacd r0, r3, #1.0
mul r6, r4, r4 | mul r2, r0, r0
mul r7, r6, r6 | mul r3, r2, r2
mul r4, r7, r7 | mul r0, r3, r3
mul r6, r4, r4 | mul r2, r0, r0
mul r4, r6, r6 | mul r0, r2, r2 | st r6, [0390]
mul r7, r5, r5 | mul r3, r1, r1 | st r2, [03b6]
mul r5, r6, r4 | mul r1, r2, r0 | ld r6, [038e] | ld r2, [03b4]
st r4, [0392] | st r0, [03b8]
mul r4, r6, r7 | mul r0, r2, r3 | st r7, [0396] | ld r7, [0381]
st r4, [0394] | st r0, [03ba] | ld r4, [0378]
mac_p1wc3 r5, r4, r5, #0.5, r7 | st r3, [03bc] | ld r0, [039e]
mul r7, r5, #2.0 | ld r3, [03a7] | ld r6, [0392]
mac_p1wc3 r1, r0, r1, #0.5, r3 | ld r2, [03b8]
mul r3, r1, #2.0
ph_wrap r5, r3, r7 | ld r7, [0383]
ph_wrap r1, r3, r3 | ld r3, [03a9] | st r5, [0380]
mac_p1wc3 r6, r4, r6, #0.5, r7 | st r5, [0392] | ld r5, [0390]
mac_p1wc3 r2, r0, r2, #0.5, r3 | st r1, [03a6]
mul r7, r6, #2.0 | mul r3, r2, #2.0 | st r1, [03b8] | ld r1, [03b6]
ph_wrap r6, r3, r7 | ld r7, [0385]
ph_wrap r2, r3, r3 | ld r3, [03ab] | st r6, [0382]
mac_p1wc3 r5, r5, r4, #0.5, r7 | st r2, [03a8]
mac_p1wc3 r1, r1, r0, #0.5, r3
mul r7, r5, #2.0 | mul r3, r1, #2.0
ph_wrap r5, r3, r7 | ld r7, [0392]
ph_wrap r1, r3, r3 | ld r3, [03b8] | st r5, [0384]
sum8 r7, r7, r6, r5 | st r1, [03aa] | ld r6, [0387]
sum8 r3, r3, r2, r1 | ld r2, [03ad] | st r7, [0392]
mov r5, r4 | mov r1, r0 | st r3, [03b8] | ld r7, [038e]
mad r5, r6, #0.5 | mad r1, r2, #0.5 | ld r3, [03b4]
mul r6, r5, #2.0 | mul r2, r1, #2.0
ph_wrap r5, r3, r6 | ld r6, [0389]
ph_wrap r1, r3, r2 | ld r2, [03af] | st r5, [0386]
mac_p1wc3 r7, r7, r4, #0.5, r6 | st r1, [03ac]
mac_p1wc3 r3, r3, r0, #0.5, r2
mul r6, r7, #2.0 | mul r2, r3, #2.0
ph_wrap r7, r3, r6 | ld r6, [0392]
ph_wrap r3, r3, r2 | ld r2, [03b8] | st r7, [0388]
sum8 r6, r6, r5, r7 | st r3, [03ae] | ld r5, [038b]
sum8 r2, r2, r1, r3 | ld r7, [0396] | ld r1, [03b1]
mac_p1wc3 r7, r7, r4, #0.5, r5 | ld r3, [03bc] | st r6, [0396]
mac_p1wc3 r3, r3, r0, #0.5, r1 | st r2, [03bc] | ld r6, [0394]
mul r5, r7, #2.0 | mul r1, r3, #2.0 | ld r2, [03ba]
ph_wrap r7, r3, r5 | ld r5, [038d]
ph_wrap r3, r3, r1 | ld r1, [03b3] | st r7, [038a]
mac_p1wc3 r4, r4, r6, #0.5, r5 | st r3, [03b0]
mac_p1wc3 r0, r0, r2, #0.5, r1
mul r6, r4, #2.0 | mul r2, r0, #2.0 | ld r4, [0396] | ld r0, [03bc]
ph_wrap r5, r3, r6
ph_wrap r1, r3, r2 | st r5, [038c]
sum8 r6, r4, r7, r5 | ld r4, [c070] | st r1, [03b2]
sum8 r2, r0, r3, r1 | ld r0, [c070]
mul r7, r4, r6 | mul r3, r0, r2 | ld r6, [c056]
mul r5, r7, #0.25 | mul r1, r3, #0.25 | ld r2, [c056]
st r5, [037e] | st r1, [03a4]
ld r4, [037e] | ld r5, [a133] | ld r0, [03a4]
mad r4, r5, r6 | ld r1, [a134] | ld r5, [040a]
mad r0, r1, r2 | ld r6, [c057] | ld r1, [040a]
mad r4, r5, r6 | ld r2, [c057] | ld r5, [03bf]
mad r0, r1, r2 | st r4, [a154] | ld r1, [03e5] | ld r7, [c049]
st r0, [a155] | ld r3, [c049]
ld r4, [a065]
subxy r5, r5, r4 | ld r0, [a066] | st r4, [03be]
subxy r1, r1, r0 | mul r5, r5, r5 | ld r6, [a03d] | st r0, [03e4]
mul r1, r1, r1 | ld r2, [a03e]
st r6, [a165] | ld r6, [03c3]
mad r6, r6, -r5 | st r2, [a166] | ld r2, [03e9]
mad r2, r2, -r1 | st r4, [a16d] | ld r4, [03c1]
addxy r5, r4, #-0.999 | st r0, [a16e] | ld r0, [03e7]
addxy r1, r0, #-0.999 | mul r5, r5, #256.0
mul r1, r1, #256.0
ph_clamp r5, r3, -r5
ph_clamp r1, r3, -r1
mul r5, r5, #256.0 | mul r1, r1, #256.0
ph_clamp r5, r3, -r5
ph_clamp r1, r3, -r1 | max r6, r6, r5 | ld r5, [c048]
max r2, r2, r1 | st r6, [03c2] | ld r1, [c048]
subxy r6, r6, #1.0 | subxy r2, r2, #1.0 | st r2, [03e8]
mul r6, r6, #256.0 | mul r2, r2, #256.0
ph_clamp r6, r3, -r6
subxy r7, r5, r7 | subxy r3, r1, r3
ph_clamp r2, r3, -r2
mad r5, r6, r7 | mad r1, r2, r3 | ld r7, [c051]
mul r6, r6, #1.25 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r6, r4, r6 | subxy r2, r0, r2
mad r4, r6, r5 | mad r0, r2, r1 | ld r5, [a125]
min r4, r4, #1.0 | ld r6, [c04b] | ld r1, [a126]
mul r5, r5, r6 | ld r2, [c04b] | st r4, [03c0]
mul r1, r1, r2 | ld r6, [c05e]
min r0, r0, #1.0 | ld r2, [c05e]
mad r5, r4, r6 | mad r1, r0, r2 | ld r6, [c05f] | st r0, [03e6]
addxy r5, r5, r6 | ld r2, [c05f] | ld r6, [a035]
addxy r1, r1, r2 | addxy r6, r6, r5 | ld r2, [a036]
addxy r2, r2, r1 | mul r6, r5, #0.08664339756999316 | st r6, [a1b5] | ld r5, [03c7]
mov r4, #0.041666666666666664 | mul r2, r1, #0.08664339756999316 | st r2, [a1b6] | ld r1, [03ed]
mov r0, #0.041666666666666664 | ymacd r4, r6, #0.16666666666666666
ymacd r0, r2, #0.16666666666666666 | ymacd r4, r6, #0.5
ymacd r0, r2, #0.5 | ymacd r4, r6, #1.0
ymacd r0, r2, #1.0 | ymacd r4, r6, #1.0 | ld r6, [a045]
ymacd r0, r2, #1.0 | mul r4, r4, r4 | ld r2, [a046]
mul r0, r0, r0 | mul r4, r4, r4
mul r0, r0, r0 | mul r4, r4, r4
mul r0, r0, r0 | mul r6, r6, r4
mul r2, r2, r0 | st r6, [03c4] | ld r6, [a045]
addxy r6, r5, r6 | st r2, [03ea] | ld r2, [a046]
addxy r2, r1, r2
ph_wrap r5, r3, r6 | ld r6, [c050]
ph_wrap r1, r3, r2 | st r5, [03c6] | ld r2, [c050]
mul r5, r5, #256.0 | mul r1, r1, #256.0 | st r1, [03ec]
ph_clamp r5, r3, -r5
ph_clamp r1, r3, -r1 | st r5, [a135]
st r1, [a136]
ld r5, [a125]
mad r6, r5, r7 | ld r1, [a126] | ld r5, [03c0]
mad r2, r1, r3 | ld r7, [c052] | ld r1, [03e6]
mad r6, r5, r7 | ld r3, [c052] | ld r5, [a17d]
mad r2, r1, r3 | ld r7, [c053] | ld r1, [a17e]
mad r6, r5, r7 | ld r3, [c053] | ld r5, [a175]
mad r2, r1, r3 | ld r7, [c054] | ld r1, [a176]
mad r6, r5, r7 | ld r3, [c054] | ld r5, [a135]
mad r2, r1, r3 | ld r7, [c055] | ld r1, [a136]
mad r6, r5, r7 | ld r3, [c055]
mad r2, r1, r3 | st r6, [03c8]
st r2, [03ee] | ld r4, [03c8]
addxy r5, r4, #0.029999999329447746 | ld r0, [03ee]
addxy r1, r0, #0.029999999329447746 | mul r6, r5, r5
mov r4, #0.041666666666666664 | mul r2, r1, r1
mov r0, #0.041666666666666664 | mul r7, r6, #0.0043321699430540197
mov r5, r4 | mul r3, r2, #0.0043321699430540197
mov r1, r0
ymacd r5, r7, #0.16666666666666666 | ymacd r1, r3, #0.16666666666666666
ymacd r5, r7, #0.5 | ymacd r1, r3, #0.5
ymacd r5, r7, #1.0 | ymacd r1, r3, #1.0
ymacd r5, r7, #1.0 | ymacd r1, r3, #1.0
mul r7, r5, r5 | mul r3, r1, r1
mul r5, r7, r7 | mul r1, r3, r3
mul r7, r5, r5 | mul r3, r1, r1
mul r5, r7, r7 | mul r1, r3, r3
mul r7, r6, #-0.0043321699430540197 | mul r3, r2, #-0.0043321699430540197 | st r5, [03da]
ymacd r4, r7, #0.16666666666666666 | ymacd r0, r3, #0.16666666666666666 | st r1, [0400]
ymacd r4, r7, #0.5 | ymacd r0, r3, #0.5
ymacd r4, r7, #1.0 | ymacd r0, r3, #1.0
ymacd r4, r7, #1.0 | ymacd r0, r3, #1.0
mul r6, r4, r4 | mul r2, r0, r0
mul r7, r6, r6 | mul r3, r2, r2
mul r4, r7, r7 | mul r0, r3, r3
mul r6, r4, r4 | mul r2, r0, r0
mul r4, r6, r6 | mul r0, r2, r2 | st r6, [03dc]
mul r7, r5, r5 | mul r3, r1, r1 | st r2, [0402]
mul r5, r6, r4 | mul r1, r2, r0 | ld r6, [03da] | ld r2, [0400]
st r4, [03de] | st r0, [0404]
mul r4, r6, r7 | mul r0, r2, r3 | st r7, [03e2] | ld r7, [03cd]
st r4, [03e0] | st r0, [0406] | ld r4, [03c4]
mac_p1wc3 r5, r4, r5, #0.5, r7 | st r3, [0408] | ld r0, [03ea]
mul r7, r5, #2.0 | ld r3, [03f3] | ld r6, [03de]
mac_p1wc3 r1, r0, r1, #0.5, r3 | ld r2, [0404]
mul r3, r1, #2.0
ph_wrap r5, r3, r7 | ld r7, [03cf]
ph_wrap r1, r3, r3 | ld r3, [03f5] | st r5, [03cc]
mac_p1wc3 r6, r4, r6, #0.5, r7 | st r5, [03de] | ld r5, [03dc]
mac_p1wc3 r2, r0, r2, #0.5, r3 | st r1, [03f2]
mul r7, r6, #2.0 | mul r3, r2, #2.0 | st r1, [0404] | ld r1, [0402]
ph_wrap r6, r3, r7 | ld r7, [03d1]
ph_wrap r2, r3, r3 | ld r3, [03f7] | st r6, [03ce]
mac_p1wc3 r5, r5, r4, #0.5, r7 | st r2, [03f4]
mac_p1wc3 r1, r1, r0, #0.5, r3
mul r7, r5, #2.0 | mul r3, r1, #2.0
ph_wrap r5, r3, r7 | ld r7, [03de]
ph_wrap r1, r3, r3 | ld r3, [0404] | st r5, [03d0]
sum8 r7, r7, r6, r5 | st r1, [03f6] | ld r6, [03d3]
sum8 r3, r3, r2, r1 | ld r2, [03f9] | st r7, [03de]
mov r5, r4 | mov r1, r0 | st r3, [0404] | ld r7, [03da]
mad r5, r6, #0.5 | mad r1, r2, #0.5 | ld r3, [0400]
mul r6, r5, #2.0 | mul r2, r1, #2.0
ph_wrap r5, r3, r6 | ld r6, [03d5]
ph_wrap r1, r3, r2 | ld r2, [03fb] | st r5, [03d2]
mac_p1wc3 r7, r7, r4, #0.5, r6 | st r1, [03f8]
mac_p1wc3 r3, r3, r0, #0.5, r2
mul r6, r7, #2.0 | mul r2, r3, #2.0
ph_wrap r7, r3, r6 | ld r6, [03de]
ph_wrap r3, r3, r2 | ld r2, [0404] | st r7, [03d4]
sum8 r6, r6, r5, r7 | st r3, [03fa] | ld r5, [03d7]
sum8 r2, r2, r1, r3 | ld r7, [03e2] | ld r1, [03fd]
mac_p1wc3 r7, r7, r4, #0.5, r5 | ld r3, [0408] | st r6, [03e2]
mac_p1wc3 r3, r3, r0, #0.5, r1 | st r2, [0408] | ld r6, [03e0]
mul r5, r7, #2.0 | mul r1, r3, #2.0 | ld r2, [0406]
ph_wrap r7, r3, r5 | ld r5, [03d9]
ph_wrap r3, r3, r1 | ld r1, [03ff] | st r7, [03d6]
mac_p1wc3 r4, r4, r6, #0.5, r5 | st r3, [03fc]
mac_p1wc3 r0, r0, r2, #0.5, r1
mul r6, r4, #2.0 | mul r2, r0, #2.0 | ld r4, [03e2] | ld r0, [0408]
ph_wrap r5, r3, r6
ph_wrap r1, r3, r2 | st r5, [03d8]
sum8 r6, r4, r7, r5 | ld r4, [c070] | st r1, [03fe]
sum8 r2, r0, r3, r1 | ld r0, [c070]
mul r7, r4, r6 | mul r3, r0, r2 | ld r6, [c056]
mul r5, r7, #0.25 | mul r1, r3, #0.25 | ld r2, [c056]
st r5, [03ca] | st r1, [03f0]
ld r4, [03ca] | ld r5, [a135] | ld r0, [03f0]
mad r4, r5, r6 | ld r1, [a136] | ld r5, [040a]
mad r0, r1, r2 | ld r6, [c057] | ld r1, [040a]
mad r4, r5, r6 | ld r2, [c057]
mad r0, r1, r2 | st r4, [a156]
st r0, [a157]
