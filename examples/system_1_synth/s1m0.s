mov r5, #0.0
mov r6, #1.0
mov r7, #0.0 | ld r0, [a030]
add r0, #-4.75 | st r0, [a178] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0301]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0300] | ld r4, [c050]
mul r2, r1, r4 | ld r4, [c051]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c052]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [a030] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0303]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0302] | ld r4, [c053]
mul r2, r1, r4 | ld r4, [c054]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c055]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1
st r7, [a151] | ld r4, [a038]
st r4, [a160]
ld r4, [a060]
mov r7, #0.0 | st r4, [a168] | ld r0, [a031]
add r0, #-4.75 | st r0, [a179] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0311]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0310] | ld r4, [c050]
mul r2, r1, r4 | ld r4, [c051]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c052]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [a031] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0313]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0312] | ld r4, [c053]
mul r2, r1, r4 | ld r4, [c054]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c055]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1
st r7, [a152] | ld r4, [a039]
st r4, [a161]
ld r4, [a061]
mov r7, #0.0 | st r4, [a169] | ld r0, [a032]
add r0, #-4.75 | st r0, [a17a] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0321]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0320] | ld r4, [c050]
mul r2, r1, r4 | ld r4, [c051]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c052]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [a032] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0323]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0322] | ld r4, [c053]
mul r2, r1, r4 | ld r4, [c054]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c055]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1
st r7, [a153] | ld r4, [a03a]
st r4, [a162]
ld r4, [a062]
mov r7, #0.0 | st r4, [a16a] | ld r0, [a033]
add r0, #-4.75 | st r0, [a17b] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0331]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0330] | ld r4, [c050]
mul r2, r1, r4 | ld r4, [c051]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c052]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [a033] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0333]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0332] | ld r4, [c053]
mul r2, r1, r4 | ld r4, [c054]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c055]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1
st r7, [a154] | ld r4, [a03b]
st r4, [a163]
ld r4, [a063]
mov r7, #0.0 | st r4, [a16b] | ld r0, [a034]
add r0, #-4.75 | st r0, [a17c] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0341]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0340] | ld r4, [c050]
mul r2, r1, r4 | ld r4, [c051]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c052]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [a034] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0343]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0342] | ld r4, [c053]
mul r2, r1, r4 | ld r4, [c054]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c055]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1
st r7, [a155] | ld r4, [a03c]
st r4, [a164]
ld r4, [a064]
mov r7, #0.0 | st r4, [a16c] | ld r0, [a035]
add r0, #-4.75 | st r0, [a17d] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0351]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0350] | ld r4, [c050]
mul r2, r1, r4 | ld r4, [c051]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c052]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [a035] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0353]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0352] | ld r4, [c053]
mul r2, r1, r4 | ld r4, [c054]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c055]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1
st r7, [a156] | ld r4, [a03d]
st r4, [a165]
ld r4, [a065]
mov r7, #0.0 | st r4, [a16d] | ld r0, [a036]
add r0, #-4.75 | st r0, [a17e] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0361]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0360] | ld r4, [c050]
mul r2, r1, r4 | ld r4, [c051]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c052]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [a036] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0363]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0362] | ld r4, [c053]
mul r2, r1, r4 | ld r4, [c054]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c055]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1
st r7, [a157] | ld r4, [a03e]
st r4, [a166]
ld r4, [a066]
mov r7, #0.0 | st r4, [a16e] | ld r0, [a037]
add r0, #-4.75 | st r0, [a17f] | ld r4, [c044]
addxy r0, r0, r4 | ld r1, [0371]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0370] | ld r4, [c050]
mul r2, r1, r4 | ld r4, [c051]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c052]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a040]
mul r1, r2, r4
addxy r7, r7, r1 | ld r0, [a037] | ld r4, [c045]
addxy r0, r0, r4 | ld r4, [c046]
addxy r0, r0, r4
add r0, #-4.75 | ld r1, [0373]
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
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
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
mul r3, r1, #256.0 | st r1, [0372] | ld r4, [c053]
mul r2, r1, r4 | ld r4, [c054]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ld r4, [c055]
ph_tri r3, r3, -r1
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4
addxy r7, r7, r1
st r7, [a158] | ld r4, [a03f]
st r4, [a167]
ld r4, [a067]
st r4, [a16f]
