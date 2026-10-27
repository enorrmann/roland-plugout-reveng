mov r6, #1.0 | mov r7, #0.0 | ld r0, [0287] | ld r2, [0283] | ld r4, [c058]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | ld r1, [0309]
ph_wrap r0, r3, r0
addxy r2, r2, r0 | mov r5, #0.0 | st r0, [0286]
ph_wrap r2, r3, r2 | ld r3, [a060]
st r2, [0282] | ld r2, [030d] | st r3, [030c]
subxy r2, r2, r3 | st r3, [a168]
mul r2, r2, r2
mad r1, r1, -r2 | ld r2, [0282]
addxy r0, r1, r4
ph_wrap r1, r3, r0 | ld r3, [030b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [0241] | st r1, [0308]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [030a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [0240]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0305]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a038]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a160]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0304]
mul r0, r1, r4 | ld r4, [c05b]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [030e] | ld r0, [a030] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a178] | ld r4, [c044] | ld r1, [0301]
add r0, #-4.75 | mov r3, #0.041666666666666664:32 | st r0, [0306] | st r3, [a180]
addxy r0, r0, r4 | ld r4, [c050]
mul r2, r0, #0.04332169878499658 | ld r0, [0306]
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c051] | st r1, [0300]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | mov r3, #0.041666666666666664:32 | ld r4, [a040]
mul r1, r2, r4 | ld r4, [c045]
addxy r0, r0, r4 | addxy r7, r7, r1 | ld r4, [c046] | ld r1, [0303]
addxy r0, r0, r4 | ld r4, [c053]
add r0, #-4.75
mul r2, r0, #0.04332169878499658
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c054] | st r1, [0302]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4 | ld r3, [a061] | ld r2, [031d] | ld r4, [030e]
subxy r2, r2, r3 | addxy r7, r7, r1 | ld r1, [0319] | st r3, [031c] | st r3, [a169]
mul r2, r2, r2 | mad r7, r7, r4 | ld r4, [c058]
mad r1, r1, -r2 | ld r2, [0282] | st r7, [a151]
addxy r0, r1, r4
ph_wrap r1, r3, r0 | ld r3, [031b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [0243] | st r1, [0318]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [031a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [0242]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0315]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a039]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a161]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0314]
mul r0, r1, r4 | ld r4, [c05b]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [031e] | ld r0, [a031] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a179] | ld r4, [c044] | ld r1, [0311]
add r0, #-4.75 | mov r3, #0.041666666666666664:32 | st r0, [0316] | st r3, [a181]
addxy r0, r0, r4 | ld r4, [c050]
mul r2, r0, #0.04332169878499658 | ld r0, [0316]
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c051] | st r1, [0310]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | mov r3, #0.041666666666666664:32 | ld r4, [a040]
mul r1, r2, r4 | ld r4, [c045]
addxy r0, r0, r4 | addxy r7, r7, r1 | ld r4, [c046] | ld r1, [0313]
addxy r0, r0, r4 | ld r4, [c053]
add r0, #-4.75
mul r2, r0, #0.04332169878499658
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c054] | st r1, [0312]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4 | ld r3, [a062] | ld r2, [032d] | ld r4, [031e]
subxy r2, r2, r3 | addxy r7, r7, r1 | ld r1, [0329] | st r3, [032c] | st r3, [a16a]
mul r2, r2, r2 | mad r7, r7, r4 | ld r4, [c058]
mad r1, r1, -r2 | ld r2, [0282] | st r7, [a152]
addxy r0, r1, r4
ph_wrap r1, r3, r0 | ld r3, [032b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [0245] | st r1, [0328]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [032a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [0244]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0325]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a03a]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a162]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0324]
mul r0, r1, r4 | ld r4, [c05b]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [032e] | ld r0, [a032] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17a] | ld r4, [c044] | ld r1, [0321]
add r0, #-4.75 | mov r3, #0.041666666666666664:32 | st r0, [0326] | st r3, [a182]
addxy r0, r0, r4 | ld r4, [c050]
mul r2, r0, #0.04332169878499658 | ld r0, [0326]
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c051] | st r1, [0320]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | mov r3, #0.041666666666666664:32 | ld r4, [a040]
mul r1, r2, r4 | ld r4, [c045]
addxy r0, r0, r4 | addxy r7, r7, r1 | ld r4, [c046] | ld r1, [0323]
addxy r0, r0, r4 | ld r4, [c053]
add r0, #-4.75
mul r2, r0, #0.04332169878499658
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c054] | st r1, [0322]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4 | ld r3, [a063] | ld r2, [033d] | ld r4, [032e]
subxy r2, r2, r3 | addxy r7, r7, r1 | ld r1, [0339] | st r3, [033c] | st r3, [a16b]
mul r2, r2, r2 | mad r7, r7, r4 | ld r4, [c058]
mad r1, r1, -r2 | ld r2, [0282] | st r7, [a153]
addxy r0, r1, r4
ph_wrap r1, r3, r0 | ld r3, [033b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [0247] | st r1, [0338]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [033a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [0246]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0335]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a03b]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a163]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0334]
mul r0, r1, r4 | ld r4, [c05b]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [033e] | ld r0, [a033] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17b] | ld r4, [c044] | ld r1, [0331]
add r0, #-4.75 | mov r3, #0.041666666666666664:32 | st r0, [0336] | st r3, [a183]
addxy r0, r0, r4 | ld r4, [c050]
mul r2, r0, #0.04332169878499658 | ld r0, [0336]
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c051] | st r1, [0330]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | mov r3, #0.041666666666666664:32 | ld r4, [a040]
mul r1, r2, r4 | ld r4, [c045]
addxy r0, r0, r4 | addxy r7, r7, r1 | ld r4, [c046] | ld r1, [0333]
addxy r0, r0, r4 | ld r4, [c053]
add r0, #-4.75
mul r2, r0, #0.04332169878499658
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c054] | st r1, [0332]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4 | ld r3, [a064] | ld r2, [034d] | ld r4, [033e]
subxy r2, r2, r3 | addxy r7, r7, r1 | ld r1, [0349] | st r3, [034c] | st r3, [a16c]
mul r2, r2, r2 | mad r7, r7, r4 | ld r4, [c058]
mad r1, r1, -r2 | ld r2, [0282] | st r7, [a154]
addxy r0, r1, r4
ph_wrap r1, r3, r0 | ld r3, [034b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [0249] | st r1, [0348]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [034a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [0248]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0345]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a03c]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a164]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0344]
mul r0, r1, r4 | ld r4, [c05b]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [034e] | ld r0, [a034] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17c] | ld r4, [c044] | ld r1, [0341]
add r0, #-4.75 | mov r3, #0.041666666666666664:32 | st r0, [0346] | st r3, [a184]
addxy r0, r0, r4 | ld r4, [c050]
mul r2, r0, #0.04332169878499658 | ld r0, [0346]
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c051] | st r1, [0340]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | mov r3, #0.041666666666666664:32 | ld r4, [a040]
mul r1, r2, r4 | ld r4, [c045]
addxy r0, r0, r4 | addxy r7, r7, r1 | ld r4, [c046] | ld r1, [0343]
addxy r0, r0, r4 | ld r4, [c053]
add r0, #-4.75
mul r2, r0, #0.04332169878499658
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c054] | st r1, [0342]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4 | ld r3, [a065] | ld r2, [035d] | ld r4, [034e]
subxy r2, r2, r3 | addxy r7, r7, r1 | ld r1, [0359] | st r3, [035c] | st r3, [a16d]
mul r2, r2, r2 | mad r7, r7, r4 | ld r4, [c058]
mad r1, r1, -r2 | ld r2, [0282] | st r7, [a155]
addxy r0, r1, r4
ph_wrap r1, r3, r0 | ld r3, [035b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [024b] | st r1, [0358]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [035a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [024a]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0355]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a03d]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a165]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0354]
mul r0, r1, r4 | ld r4, [c05b]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [035e] | ld r0, [a035] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17d] | ld r4, [c044] | ld r1, [0351]
add r0, #-4.75 | mov r3, #0.041666666666666664:32 | st r0, [0356] | st r3, [a185]
addxy r0, r0, r4 | ld r4, [c050]
mul r2, r0, #0.04332169878499658 | ld r0, [0356]
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c051] | st r1, [0350]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | mov r3, #0.041666666666666664:32 | ld r4, [a040]
mul r1, r2, r4 | ld r4, [c045]
addxy r0, r0, r4 | addxy r7, r7, r1 | ld r4, [c046] | ld r1, [0353]
addxy r0, r0, r4 | ld r4, [c053]
add r0, #-4.75
mul r2, r0, #0.04332169878499658
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c054] | st r1, [0352]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4 | ld r3, [a066] | ld r2, [036d] | ld r4, [035e]
subxy r2, r2, r3 | addxy r7, r7, r1 | ld r1, [0369] | st r3, [036c] | st r3, [a16e]
mul r2, r2, r2 | mad r7, r7, r4 | ld r4, [c058]
mad r1, r1, -r2 | ld r2, [0282] | st r7, [a156]
addxy r0, r1, r4
ph_wrap r1, r3, r0 | ld r3, [036b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [024d] | st r1, [0368]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [036a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [024c]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0365]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a03e]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a166]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0364]
mul r0, r1, r4 | ld r4, [c05b]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [036e] | ld r0, [a036] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17e] | ld r4, [c044] | ld r1, [0361]
add r0, #-4.75 | mov r3, #0.041666666666666664:32 | st r0, [0366] | st r3, [a186]
addxy r0, r0, r4 | ld r4, [c050]
mul r2, r0, #0.04332169878499658 | ld r0, [0366]
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c051] | st r1, [0360]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | mov r3, #0.041666666666666664:32 | ld r4, [a040]
mul r1, r2, r4 | ld r4, [c045]
addxy r0, r0, r4 | addxy r7, r7, r1 | ld r4, [c046] | ld r1, [0363]
addxy r0, r0, r4 | ld r4, [c053]
add r0, #-4.75
mul r2, r0, #0.04332169878499658
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c054] | st r1, [0362]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4 | ld r3, [a067] | ld r2, [037d] | ld r4, [036e]
subxy r2, r2, r3 | addxy r7, r7, r1 | ld r1, [0379] | st r3, [037c] | st r3, [a16f]
mul r2, r2, r2 | mad r7, r7, r4 | ld r4, [c058]
mad r1, r1, -r2 | ld r2, [0282] | st r7, [a157]
addxy r0, r1, r4
ph_wrap r1, r3, r0 | ld r3, [037b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [024f] | st r1, [0378]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [037a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [024e]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0375]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a03f]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a167]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0374]
mul r0, r1, r4 | ld r4, [c05b]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [037e] | ld r0, [a037] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17f] | ld r4, [c044] | ld r1, [0371]
add r0, #-4.75 | mov r3, #0.041666666666666664:32 | st r0, [0376] | st r3, [a187]
addxy r0, r0, r4 | ld r4, [c050]
mul r2, r0, #0.04332169878499658 | ld r0, [0376]
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c051] | st r1, [0370]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c052]
mad r2, r3, r4 | mov r3, #0.041666666666666664:32 | ld r4, [a040]
mul r1, r2, r4 | ld r4, [c045]
addxy r0, r0, r4 | addxy r7, r7, r1 | ld r4, [c046] | ld r1, [0373]
addxy r0, r0, r4 | ld r4, [c053]
add r0, #-4.75
mul r2, r0, #0.04332169878499658
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
mul r3, r1, #256.0 | mul r2, r1, r4 | ld r4, [c054] | st r1, [0372]
ph_clamp r3, r3, -r3
mad r2, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c055]
mad r2, r3, r4 | ld r4, [a041]
mul r1, r2, r4 | ld r4, [037e]
addxy r7, r7, r1
mad r7, r7, r4
st r7, [a158]
