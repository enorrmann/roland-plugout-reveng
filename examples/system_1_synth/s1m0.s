mov r5, #0.0
mov r6, #1.0
mov r7, #0.0 | ld r1, [0281] | ld r4, [c058] | ld r2, [0283]
addxy r7, r1, r4
ph_wrap r1, r3, r7
subxy r7, r1, r7 | st r1, [0280] | ld r0, [0287]
add r0, #0.6184303127229214:32
ph_wrap r0, r3, r0
mul r2, r2, #7.31275 | st r0, [0286]
addxy r2, r2, r0
ph_wrap r2, r3, r2
st r2, [0282] | ld r3, [0285]
subxy r0, r3, r2
mul r0, r0, r7
mad r3, r0, #0.5
st r3, [0284] | ld r4, [c063]
mul r7, r3, r4 | ld r4, [c064]
mad r7, r2, r4 | ld r4, [c061]
mad r7, r1, r4 | ld r4, [c062]
mul r3, r1, #256.0
ph_clamp r3, r3, -r3
mad r7, r3, r4 | ld r4, [c060]
ph_tri r3, r3, -r1
mad r7, r3, r4 | ld r4, [c05a]
mul r0, r7, r4
st r0, [0288] | ld r4, [c05b]
mul r0, r7, r4
add r7, #-1.0 | st r0, [028a] | ld r4, [c05c]
mul r0, r7, r4
mov r7, #0.0 | st r0, [028c] | ld r2, [0305] | ld r4, [c059]
addxy r2, r2, r4 | ld r3, [a038]
mul r2, r2, r3
min r2, r2, r6
st r2, [0304] | ld r0, [a030] | ld r4, [0288]
mad r0, r4, r2 | st r0, [a178]
add r0, #-4.75 | st r0, [0306] | ld r4, [c044]
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
addxy r7, r7, r1 | ld r0, [0306] | ld r4, [c045]
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
addxy r7, r7, r1 | ld r3, [0304] | ld r4, [028c]
mul r4, r3, r4
mad r7, r7, r4 | ld r4, [028a] | ld r0, [c05d]
mad r0, r3, r4
st r0, [a180]
st r7, [a151] | ld r4, [a038]
st r4, [a160]
ld r4, [a060]
mov r7, #0.0 | st r4, [a168] | ld r2, [0315]
ld r4, [c059] | ld r3, [a039]
addxy r2, r2, r4
mul r2, r2, r3
min r2, r2, r6
st r2, [0314] | ld r0, [a031] | ld r4, [0288]
mad r0, r4, r2 | st r0, [a179]
add r0, #-4.75 | st r0, [0316] | ld r4, [c044]
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
addxy r7, r7, r1 | ld r0, [0316] | ld r4, [c045]
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
addxy r7, r7, r1 | ld r3, [0314] | ld r4, [028c]
mul r4, r3, r4
mad r7, r7, r4 | ld r4, [028a] | ld r0, [c05d]
mad r0, r3, r4
st r0, [a181]
st r7, [a152] | ld r4, [a039]
st r4, [a161]
ld r4, [a061]
mov r7, #0.0 | st r4, [a169] | ld r2, [0325]
ld r4, [c059] | ld r3, [a03a]
addxy r2, r2, r4
mul r2, r2, r3
min r2, r2, r6
st r2, [0324] | ld r0, [a032] | ld r4, [0288]
mad r0, r4, r2 | st r0, [a17a]
add r0, #-4.75 | st r0, [0326] | ld r4, [c044]
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
addxy r7, r7, r1 | ld r0, [0326] | ld r4, [c045]
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
addxy r7, r7, r1 | ld r3, [0324] | ld r4, [028c]
mul r4, r3, r4
mad r7, r7, r4 | ld r4, [028a] | ld r0, [c05d]
mad r0, r3, r4
st r0, [a182]
st r7, [a153] | ld r4, [a03a]
st r4, [a162]
ld r4, [a062]
mov r7, #0.0 | st r4, [a16a] | ld r2, [0335]
ld r4, [c059] | ld r3, [a03b]
addxy r2, r2, r4
mul r2, r2, r3
min r2, r2, r6
st r2, [0334] | ld r0, [a033] | ld r4, [0288]
mad r0, r4, r2 | st r0, [a17b]
add r0, #-4.75 | st r0, [0336] | ld r4, [c044]
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
addxy r7, r7, r1 | ld r0, [0336] | ld r4, [c045]
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
addxy r7, r7, r1 | ld r3, [0334] | ld r4, [028c]
mul r4, r3, r4
mad r7, r7, r4 | ld r4, [028a] | ld r0, [c05d]
mad r0, r3, r4
st r0, [a183]
st r7, [a154] | ld r4, [a03b]
st r4, [a163]
ld r4, [a063]
mov r7, #0.0 | st r4, [a16b] | ld r2, [0345]
ld r4, [c059] | ld r3, [a03c]
addxy r2, r2, r4
mul r2, r2, r3
min r2, r2, r6
st r2, [0344] | ld r0, [a034] | ld r4, [0288]
mad r0, r4, r2 | st r0, [a17c]
add r0, #-4.75 | st r0, [0346] | ld r4, [c044]
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
addxy r7, r7, r1 | ld r0, [0346] | ld r4, [c045]
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
addxy r7, r7, r1 | ld r3, [0344] | ld r4, [028c]
mul r4, r3, r4
mad r7, r7, r4 | ld r4, [028a] | ld r0, [c05d]
mad r0, r3, r4
st r0, [a184]
st r7, [a155] | ld r4, [a03c]
st r4, [a164]
ld r4, [a064]
mov r7, #0.0 | st r4, [a16c] | ld r2, [0355]
ld r4, [c059] | ld r3, [a03d]
addxy r2, r2, r4
mul r2, r2, r3
min r2, r2, r6
st r2, [0354] | ld r0, [a035] | ld r4, [0288]
mad r0, r4, r2 | st r0, [a17d]
add r0, #-4.75 | st r0, [0356] | ld r4, [c044]
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
addxy r7, r7, r1 | ld r0, [0356] | ld r4, [c045]
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
addxy r7, r7, r1 | ld r3, [0354] | ld r4, [028c]
mul r4, r3, r4
mad r7, r7, r4 | ld r4, [028a] | ld r0, [c05d]
mad r0, r3, r4
st r0, [a185]
st r7, [a156] | ld r4, [a03d]
st r4, [a165]
ld r4, [a065]
mov r7, #0.0 | st r4, [a16d] | ld r2, [0365]
ld r4, [c059] | ld r3, [a03e]
addxy r2, r2, r4
mul r2, r2, r3
min r2, r2, r6
st r2, [0364] | ld r0, [a036] | ld r4, [0288]
mad r0, r4, r2 | st r0, [a17e]
add r0, #-4.75 | st r0, [0366] | ld r4, [c044]
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
addxy r7, r7, r1 | ld r0, [0366] | ld r4, [c045]
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
addxy r7, r7, r1 | ld r3, [0364] | ld r4, [028c]
mul r4, r3, r4
mad r7, r7, r4 | ld r4, [028a] | ld r0, [c05d]
mad r0, r3, r4
st r0, [a186]
st r7, [a157] | ld r4, [a03e]
st r4, [a166]
ld r4, [a066]
mov r7, #0.0 | st r4, [a16e] | ld r2, [0375]
ld r4, [c059] | ld r3, [a03f]
addxy r2, r2, r4
mul r2, r2, r3
min r2, r2, r6
st r2, [0374] | ld r0, [a037] | ld r4, [0288]
mad r0, r4, r2 | st r0, [a17f]
add r0, #-4.75 | st r0, [0376] | ld r4, [c044]
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
addxy r7, r7, r1 | ld r0, [0376] | ld r4, [c045]
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
addxy r7, r7, r1 | ld r3, [0374] | ld r4, [028c]
mul r4, r3, r4
mad r7, r7, r4 | ld r4, [028a] | ld r0, [c05d]
mad r0, r3, r4
st r0, [a187]
st r7, [a158] | ld r4, [a03f]
st r4, [a167]
ld r4, [a067]
st r4, [a16f]
