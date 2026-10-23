mov r5, #0.0 | mov r6, #1.0
mov r7, #0.0 | ld r2, [0283] | ld r0, [0287]
add r0, #0.6184303127229214:32
ph_wrap r0, r3, r0
mul r2, r2, #7.31275 | st r0, [0286]
addxy r2, r2, r0
ph_wrap r2, r3, r2
st r2, [0282] | ld r1, [0309] | ld r4, [c058]
ld r3, [a060] | ld r2, [030d]
subxy r2, r2, r3 | st r3, [030c] | st r3, [a168]
mul r2, r2, r2
mad r1, r1, -r2
addxy r0, r1, r4
ph_wrap r1, r3, r0
subxy r0, r1, r0 | st r1, [0308] | ld r3, [030b] | ld r2, [0282]
subxy r4, r3, r2
mul r4, r4, r0
mad r3, r4, #0.5
st r3, [030a] | ld r4, [c058] | ld r2, [0241]
mul r4, r4, #4.0 | subxy r0, r2, r3
min r4, r4, r6
mad r2, r0, r4
st r2, [0240] | ld r4, [c065]
mul r7, r3, r4 | ld r4, [c064]
mad r7, r2, r4 | ld r4, [c062]
mad r7, r1, r4 | mul r3, r1, #256.0 | ld r4, [c063]
ph_clamp r3, r3, -r3
mad r7, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c061]
mad r7, r3, r4 | mul r0, r3, r3 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3
mad r7, r0, r4 | ld r2, [0305] | ld r4, [c059] | ld r3, [a038]
addxy r2, r2, r4 | st r3, [a160]
mul r2, r2, r3
min r2, r2, r6
mul r1, r7, r2 | st r2, [0304] | ld r4, [c05c]
mul r0, r1, r4 | ld r3, [c05e]
mad r0, r2, -r3
st r0, [030e] | ld r4, [c05b]
ld r3, [c05d]
mad r3, r1, r4
st r3, [a180] | ld r0, [a030] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a178]
mov r7, #0.0 | add r0, #-4.75 | st r0, [0306] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0301]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0300] | ld r4, [c050]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c051]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [0306] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0303]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0302] | ld r4, [c053]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c054]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1 | ld r4, [030e]
mad r7, r7, r4
st r7, [a151] | ld r1, [0319] | ld r4, [c058]
ld r3, [a061] | ld r2, [031d]
subxy r2, r2, r3 | st r3, [031c] | st r3, [a169]
mul r2, r2, r2
mad r1, r1, -r2
addxy r0, r1, r4
ph_wrap r1, r3, r0
subxy r0, r1, r0 | st r1, [0318] | ld r3, [031b] | ld r2, [0282]
subxy r4, r3, r2
mul r4, r4, r0
mad r3, r4, #0.5
st r3, [031a] | ld r4, [c058] | ld r2, [0243]
mul r4, r4, #4.0 | subxy r0, r2, r3
min r4, r4, r6
mad r2, r0, r4
st r2, [0242] | ld r4, [c065]
mul r7, r3, r4 | ld r4, [c064]
mad r7, r2, r4 | ld r4, [c062]
mad r7, r1, r4 | mul r3, r1, #256.0 | ld r4, [c063]
ph_clamp r3, r3, -r3
mad r7, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c061]
mad r7, r3, r4 | mul r0, r3, r3 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3
mad r7, r0, r4 | ld r2, [0315] | ld r4, [c059] | ld r3, [a039]
addxy r2, r2, r4 | st r3, [a161]
mul r2, r2, r3
min r2, r2, r6
mul r1, r7, r2 | st r2, [0314] | ld r4, [c05c]
mul r0, r1, r4 | ld r3, [c05e]
mad r0, r2, -r3
st r0, [031e] | ld r4, [c05b]
ld r3, [c05d]
mad r3, r1, r4
st r3, [a181] | ld r0, [a031] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a179]
mov r7, #0.0 | add r0, #-4.75 | st r0, [0316] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0311]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0310] | ld r4, [c050]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c051]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [0316] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0313]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0312] | ld r4, [c053]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c054]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1 | ld r4, [031e]
mad r7, r7, r4
st r7, [a152] | ld r1, [0329] | ld r4, [c058]
ld r3, [a062] | ld r2, [032d]
subxy r2, r2, r3 | st r3, [032c] | st r3, [a16a]
mul r2, r2, r2
mad r1, r1, -r2
addxy r0, r1, r4
ph_wrap r1, r3, r0
subxy r0, r1, r0 | st r1, [0328] | ld r3, [032b] | ld r2, [0282]
subxy r4, r3, r2
mul r4, r4, r0
mad r3, r4, #0.5
st r3, [032a] | ld r4, [c058] | ld r2, [0245]
mul r4, r4, #4.0 | subxy r0, r2, r3
min r4, r4, r6
mad r2, r0, r4
st r2, [0244] | ld r4, [c065]
mul r7, r3, r4 | ld r4, [c064]
mad r7, r2, r4 | ld r4, [c062]
mad r7, r1, r4 | mul r3, r1, #256.0 | ld r4, [c063]
ph_clamp r3, r3, -r3
mad r7, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c061]
mad r7, r3, r4 | mul r0, r3, r3 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3
mad r7, r0, r4 | ld r2, [0325] | ld r4, [c059] | ld r3, [a03a]
addxy r2, r2, r4 | st r3, [a162]
mul r2, r2, r3
min r2, r2, r6
mul r1, r7, r2 | st r2, [0324] | ld r4, [c05c]
mul r0, r1, r4 | ld r3, [c05e]
mad r0, r2, -r3
st r0, [032e] | ld r4, [c05b]
ld r3, [c05d]
mad r3, r1, r4
st r3, [a182] | ld r0, [a032] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17a]
mov r7, #0.0 | add r0, #-4.75 | st r0, [0326] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0321]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0320] | ld r4, [c050]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c051]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [0326] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0323]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0322] | ld r4, [c053]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c054]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1 | ld r4, [032e]
mad r7, r7, r4
st r7, [a153] | ld r1, [0339] | ld r4, [c058]
ld r3, [a063] | ld r2, [033d]
subxy r2, r2, r3 | st r3, [033c] | st r3, [a16b]
mul r2, r2, r2
mad r1, r1, -r2
addxy r0, r1, r4
ph_wrap r1, r3, r0
subxy r0, r1, r0 | st r1, [0338] | ld r3, [033b] | ld r2, [0282]
subxy r4, r3, r2
mul r4, r4, r0
mad r3, r4, #0.5
st r3, [033a] | ld r4, [c058] | ld r2, [0247]
mul r4, r4, #4.0 | subxy r0, r2, r3
min r4, r4, r6
mad r2, r0, r4
st r2, [0246] | ld r4, [c065]
mul r7, r3, r4 | ld r4, [c064]
mad r7, r2, r4 | ld r4, [c062]
mad r7, r1, r4 | mul r3, r1, #256.0 | ld r4, [c063]
ph_clamp r3, r3, -r3
mad r7, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c061]
mad r7, r3, r4 | mul r0, r3, r3 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3
mad r7, r0, r4 | ld r2, [0335] | ld r4, [c059] | ld r3, [a03b]
addxy r2, r2, r4 | st r3, [a163]
mul r2, r2, r3
min r2, r2, r6
mul r1, r7, r2 | st r2, [0334] | ld r4, [c05c]
mul r0, r1, r4 | ld r3, [c05e]
mad r0, r2, -r3
st r0, [033e] | ld r4, [c05b]
ld r3, [c05d]
mad r3, r1, r4
st r3, [a183] | ld r0, [a033] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17b]
mov r7, #0.0 | add r0, #-4.75 | st r0, [0336] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0331]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0330] | ld r4, [c050]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c051]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [0336] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0333]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0332] | ld r4, [c053]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c054]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1 | ld r4, [033e]
mad r7, r7, r4
st r7, [a154] | ld r1, [0349] | ld r4, [c058]
ld r3, [a064] | ld r2, [034d]
subxy r2, r2, r3 | st r3, [034c] | st r3, [a16c]
mul r2, r2, r2
mad r1, r1, -r2
addxy r0, r1, r4
ph_wrap r1, r3, r0
subxy r0, r1, r0 | st r1, [0348] | ld r3, [034b] | ld r2, [0282]
subxy r4, r3, r2
mul r4, r4, r0
mad r3, r4, #0.5
st r3, [034a] | ld r4, [c058] | ld r2, [0249]
mul r4, r4, #4.0 | subxy r0, r2, r3
min r4, r4, r6
mad r2, r0, r4
st r2, [0248] | ld r4, [c065]
mul r7, r3, r4 | ld r4, [c064]
mad r7, r2, r4 | ld r4, [c062]
mad r7, r1, r4 | mul r3, r1, #256.0 | ld r4, [c063]
ph_clamp r3, r3, -r3
mad r7, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c061]
mad r7, r3, r4 | mul r0, r3, r3 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3
mad r7, r0, r4 | ld r2, [0345] | ld r4, [c059] | ld r3, [a03c]
addxy r2, r2, r4 | st r3, [a164]
mul r2, r2, r3
min r2, r2, r6
mul r1, r7, r2 | st r2, [0344] | ld r4, [c05c]
mul r0, r1, r4 | ld r3, [c05e]
mad r0, r2, -r3
st r0, [034e] | ld r4, [c05b]
ld r3, [c05d]
mad r3, r1, r4
st r3, [a184] | ld r0, [a034] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17c]
mov r7, #0.0 | add r0, #-4.75 | st r0, [0346] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0341]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0340] | ld r4, [c050]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c051]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [0346] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0343]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0342] | ld r4, [c053]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c054]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1 | ld r4, [034e]
mad r7, r7, r4
st r7, [a155] | ld r1, [0359] | ld r4, [c058]
ld r3, [a065] | ld r2, [035d]
subxy r2, r2, r3 | st r3, [035c] | st r3, [a16d]
mul r2, r2, r2
mad r1, r1, -r2
addxy r0, r1, r4
ph_wrap r1, r3, r0
subxy r0, r1, r0 | st r1, [0358] | ld r3, [035b] | ld r2, [0282]
subxy r4, r3, r2
mul r4, r4, r0
mad r3, r4, #0.5
st r3, [035a] | ld r4, [c058] | ld r2, [024b]
mul r4, r4, #4.0 | subxy r0, r2, r3
min r4, r4, r6
mad r2, r0, r4
st r2, [024a] | ld r4, [c065]
mul r7, r3, r4 | ld r4, [c064]
mad r7, r2, r4 | ld r4, [c062]
mad r7, r1, r4 | mul r3, r1, #256.0 | ld r4, [c063]
ph_clamp r3, r3, -r3
mad r7, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c061]
mad r7, r3, r4 | mul r0, r3, r3 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3
mad r7, r0, r4 | ld r2, [0355] | ld r4, [c059] | ld r3, [a03d]
addxy r2, r2, r4 | st r3, [a165]
mul r2, r2, r3
min r2, r2, r6
mul r1, r7, r2 | st r2, [0354] | ld r4, [c05c]
mul r0, r1, r4 | ld r3, [c05e]
mad r0, r2, -r3
st r0, [035e] | ld r4, [c05b]
ld r3, [c05d]
mad r3, r1, r4
st r3, [a185] | ld r0, [a035] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17d]
mov r7, #0.0 | add r0, #-4.75 | st r0, [0356] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0351]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0350] | ld r4, [c050]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c051]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [0356] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0353]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0352] | ld r4, [c053]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c054]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1 | ld r4, [035e]
mad r7, r7, r4
st r7, [a156] | ld r1, [0369] | ld r4, [c058]
ld r3, [a066] | ld r2, [036d]
subxy r2, r2, r3 | st r3, [036c] | st r3, [a16e]
mul r2, r2, r2
mad r1, r1, -r2
addxy r0, r1, r4
ph_wrap r1, r3, r0
subxy r0, r1, r0 | st r1, [0368] | ld r3, [036b] | ld r2, [0282]
subxy r4, r3, r2
mul r4, r4, r0
mad r3, r4, #0.5
st r3, [036a] | ld r4, [c058] | ld r2, [024d]
mul r4, r4, #4.0 | subxy r0, r2, r3
min r4, r4, r6
mad r2, r0, r4
st r2, [024c] | ld r4, [c065]
mul r7, r3, r4 | ld r4, [c064]
mad r7, r2, r4 | ld r4, [c062]
mad r7, r1, r4 | mul r3, r1, #256.0 | ld r4, [c063]
ph_clamp r3, r3, -r3
mad r7, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c061]
mad r7, r3, r4 | mul r0, r3, r3 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3
mad r7, r0, r4 | ld r2, [0365] | ld r4, [c059] | ld r3, [a03e]
addxy r2, r2, r4 | st r3, [a166]
mul r2, r2, r3
min r2, r2, r6
mul r1, r7, r2 | st r2, [0364] | ld r4, [c05c]
mul r0, r1, r4 | ld r3, [c05e]
mad r0, r2, -r3
st r0, [036e] | ld r4, [c05b]
ld r3, [c05d]
mad r3, r1, r4
st r3, [a186] | ld r0, [a036] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17e]
mov r7, #0.0 | add r0, #-4.75 | st r0, [0366] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0361]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0360] | ld r4, [c050]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c051]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [0366] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0363]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0362] | ld r4, [c053]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c054]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1 | ld r4, [036e]
mad r7, r7, r4
st r7, [a157] | ld r1, [0379] | ld r4, [c058]
ld r3, [a067] | ld r2, [037d]
subxy r2, r2, r3 | st r3, [037c] | st r3, [a16f]
mul r2, r2, r2
mad r1, r1, -r2
addxy r0, r1, r4
ph_wrap r1, r3, r0
subxy r0, r1, r0 | st r1, [0378] | ld r3, [037b] | ld r2, [0282]
subxy r4, r3, r2
mul r4, r4, r0
mad r3, r4, #0.5
st r3, [037a] | ld r4, [c058] | ld r2, [024f]
mul r4, r4, #4.0 | subxy r0, r2, r3
min r4, r4, r6
mad r2, r0, r4
st r2, [024e] | ld r4, [c065]
mul r7, r3, r4 | ld r4, [c064]
mad r7, r2, r4 | ld r4, [c062]
mad r7, r1, r4 | mul r3, r1, #256.0 | ld r4, [c063]
ph_clamp r3, r3, -r3
mad r7, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c061]
mad r7, r3, r4 | mul r0, r3, r3 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3
mad r7, r0, r4 | ld r2, [0375] | ld r4, [c059] | ld r3, [a03f]
addxy r2, r2, r4 | st r3, [a167]
mul r2, r2, r3
min r2, r2, r6
mul r1, r7, r2 | st r2, [0374] | ld r4, [c05c]
mul r0, r1, r4 | ld r3, [c05e]
mad r0, r2, -r3
st r0, [037e] | ld r4, [c05b]
ld r3, [c05d]
mad r3, r1, r4
st r3, [a187] | ld r0, [a037] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17f]
mov r7, #0.0 | add r0, #-4.75 | st r0, [0376] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0371]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0370] | ld r4, [c050]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c051]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [0376] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0373]
mul r2, r0, #0.04332169878499658 | mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r3, r1, #256.0 | st r1, [0372] | ld r4, [c053]
mul r2, r1, r4 | ph_clamp r3, r3, -r3 | ld r4, [c054]
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1 | ld r4, [037e]
mad r7, r7, r4
st r7, [a158]
