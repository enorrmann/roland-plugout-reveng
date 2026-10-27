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
mad r0, r1, r4 | st r0, [a178] | ld r4, [c044]
mov r3, #0.041666666666666664 | st r0, [0306] | st r3, [a180]
ld r0, [0306]
addxy r0, r0, r4
st r0, [0302]
ld r0, [0302]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [0401]
mul r1, r0, r0
mul r2, r1, #0.0078125 | ld r1, [c071]
mul r4, r2, r3 | st r2, [a188]
mad r7, r4, #2.3466666666666667 | ld r4, [0403]
ph_wrap r0, r3, r7
mul r3, r2, r1 | st r0, [0400]
mad r4, r3, #2.3466666666666667
ph_wrap r7, r3, r4 | ld r4, [c072]
addxy r1, r0, r7 | mul r0, r2, r4 | st r7, [0402] | ld r7, [0405] | ld r4, [c073]
mul r3, r1, #4.76837158203125e-07 | mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7 | ld r7, [0407]
mul r0, r2, r4 | mad r3, r1, #4.76837158203125e-07 | st r1, [0404]
mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7
mad r3, r1, #4.76837158203125e-07 | st r1, [0406] | ld r1, [c074]
mul r4, r3, #256.0 | mul r3, r2, r1 | ld r1, [c075]
mul r0, r4, #256.0 | ld r4, [0409]
mad r4, r3, #2.3466666666666667 | mul r7, r0, #8.0
ph_wrap r0, r3, r4 | ld r4, [040b]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [0408] | ld r1, [c076]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4 | ld r4, [040d]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [040a]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4
mad r7, r0, #0.25 | st r0, [040c]
mul r2, r7, #0.5714983344088459
mul r1, r2, r2
mul r3, r1, #-0.43681829687651003
addxy r4, r3, #1.2582678124581252 | ld r3, [a061]
ymacd r4, r1, #-1.6510263081007801 | st r3, [031c] | st r3, [a169]
ymacd r4, r1, #1.8030530594057832 | ld r1, [0319]
mul r0, r4, r2 | ld r4, [a040] | ld r2, [031d]
subxy r2, r2, r3 | st r0, [0300]
mul r2, r2, r2 | ld r0, [0300]
mul r0, r0, r4 | mad r1, r1, -r2 | ld r4, [030e] | ld r2, [0282]
mad r0, r0, r4 | ld r4, [c058]
addxy r0, r1, r4 | st r0, [a151]
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
mad r0, r1, r4 | st r0, [a179] | ld r4, [c044]
mov r3, #0.041666666666666664 | st r0, [0316] | st r3, [a181]
ld r0, [0316]
addxy r0, r0, r4
st r0, [0312]
ld r0, [0312]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [0421]
mul r1, r0, r0
mul r2, r1, #0.0078125 | ld r1, [c071]
mul r4, r2, r3 | st r2, [a189]
mad r7, r4, #2.3466666666666667 | ld r4, [0423]
ph_wrap r0, r3, r7
mul r3, r2, r1 | st r0, [0420]
mad r4, r3, #2.3466666666666667
ph_wrap r7, r3, r4 | ld r4, [c072]
addxy r1, r0, r7 | mul r0, r2, r4 | st r7, [0422] | ld r7, [0425] | ld r4, [c073]
mul r3, r1, #4.76837158203125e-07 | mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7 | ld r7, [0427]
mul r0, r2, r4 | mad r3, r1, #4.76837158203125e-07 | st r1, [0424]
mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7
mad r3, r1, #4.76837158203125e-07 | st r1, [0426] | ld r1, [c074]
mul r4, r3, #256.0 | mul r3, r2, r1 | ld r1, [c075]
mul r0, r4, #256.0 | ld r4, [0429]
mad r4, r3, #2.3466666666666667 | mul r7, r0, #8.0
ph_wrap r0, r3, r4 | ld r4, [042b]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [0428] | ld r1, [c076]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4 | ld r4, [042d]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [042a]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4
mad r7, r0, #0.25 | st r0, [042c]
mul r2, r7, #0.5714983344088459
mul r1, r2, r2
mul r3, r1, #-0.43681829687651003
addxy r4, r3, #1.2582678124581252 | ld r3, [a062]
ymacd r4, r1, #-1.6510263081007801 | st r3, [032c] | st r3, [a16a]
ymacd r4, r1, #1.8030530594057832 | ld r1, [0329]
mul r0, r4, r2 | ld r4, [a040] | ld r2, [032d]
subxy r2, r2, r3 | st r0, [0310]
mul r2, r2, r2 | ld r0, [0310]
mul r0, r0, r4 | mad r1, r1, -r2 | ld r4, [031e] | ld r2, [0282]
mad r0, r0, r4 | ld r4, [c058]
addxy r0, r1, r4 | st r0, [a152]
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
mad r0, r1, r4 | st r0, [a17a] | ld r4, [c044]
mov r3, #0.041666666666666664 | st r0, [0326] | st r3, [a182]
ld r0, [0326]
addxy r0, r0, r4
st r0, [0322]
ld r0, [0322]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [0441]
mul r1, r0, r0
mul r2, r1, #0.0078125 | ld r1, [c071]
mul r4, r2, r3 | st r2, [a18a]
mad r7, r4, #2.3466666666666667 | ld r4, [0443]
ph_wrap r0, r3, r7
mul r3, r2, r1 | st r0, [0440]
mad r4, r3, #2.3466666666666667
ph_wrap r7, r3, r4 | ld r4, [c072]
addxy r1, r0, r7 | mul r0, r2, r4 | st r7, [0442] | ld r7, [0445] | ld r4, [c073]
mul r3, r1, #4.76837158203125e-07 | mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7 | ld r7, [0447]
mul r0, r2, r4 | mad r3, r1, #4.76837158203125e-07 | st r1, [0444]
mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7
mad r3, r1, #4.76837158203125e-07 | st r1, [0446] | ld r1, [c074]
mul r4, r3, #256.0 | mul r3, r2, r1 | ld r1, [c075]
mul r0, r4, #256.0 | ld r4, [0449]
mad r4, r3, #2.3466666666666667 | mul r7, r0, #8.0
ph_wrap r0, r3, r4 | ld r4, [044b]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [0448] | ld r1, [c076]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4 | ld r4, [044d]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [044a]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4
mad r7, r0, #0.25 | st r0, [044c]
mul r2, r7, #0.5714983344088459
mul r1, r2, r2
mul r3, r1, #-0.43681829687651003
addxy r4, r3, #1.2582678124581252 | ld r3, [a063]
ymacd r4, r1, #-1.6510263081007801 | st r3, [033c] | st r3, [a16b]
ymacd r4, r1, #1.8030530594057832 | ld r1, [0339]
mul r0, r4, r2 | ld r4, [a040] | ld r2, [033d]
subxy r2, r2, r3 | st r0, [0320]
mul r2, r2, r2 | ld r0, [0320]
mul r0, r0, r4 | mad r1, r1, -r2 | ld r4, [032e] | ld r2, [0282]
mad r0, r0, r4 | ld r4, [c058]
addxy r0, r1, r4 | st r0, [a153]
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
mad r0, r1, r4 | st r0, [a17b] | ld r4, [c044]
mov r3, #0.041666666666666664 | st r0, [0336] | st r3, [a183]
ld r0, [0336]
addxy r0, r0, r4
st r0, [0332]
ld r0, [0332]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [0461]
mul r1, r0, r0
mul r2, r1, #0.0078125 | ld r1, [c071]
mul r4, r2, r3 | st r2, [a18b]
mad r7, r4, #2.3466666666666667 | ld r4, [0463]
ph_wrap r0, r3, r7
mul r3, r2, r1 | st r0, [0460]
mad r4, r3, #2.3466666666666667
ph_wrap r7, r3, r4 | ld r4, [c072]
addxy r1, r0, r7 | mul r0, r2, r4 | st r7, [0462] | ld r7, [0465] | ld r4, [c073]
mul r3, r1, #4.76837158203125e-07 | mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7 | ld r7, [0467]
mul r0, r2, r4 | mad r3, r1, #4.76837158203125e-07 | st r1, [0464]
mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7
mad r3, r1, #4.76837158203125e-07 | st r1, [0466] | ld r1, [c074]
mul r4, r3, #256.0 | mul r3, r2, r1 | ld r1, [c075]
mul r0, r4, #256.0 | ld r4, [0469]
mad r4, r3, #2.3466666666666667 | mul r7, r0, #8.0
ph_wrap r0, r3, r4 | ld r4, [046b]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [0468] | ld r1, [c076]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4 | ld r4, [046d]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [046a]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4
mad r7, r0, #0.25 | st r0, [046c]
mul r2, r7, #0.5714983344088459
mul r1, r2, r2
mul r3, r1, #-0.43681829687651003
addxy r4, r3, #1.2582678124581252 | ld r3, [a064]
ymacd r4, r1, #-1.6510263081007801 | st r3, [034c] | st r3, [a16c]
ymacd r4, r1, #1.8030530594057832 | ld r1, [0349]
mul r0, r4, r2 | ld r4, [a040] | ld r2, [034d]
subxy r2, r2, r3 | st r0, [0330]
mul r2, r2, r2 | ld r0, [0330]
mul r0, r0, r4 | mad r1, r1, -r2 | ld r4, [033e] | ld r2, [0282]
mad r0, r0, r4 | ld r4, [c058]
addxy r0, r1, r4 | st r0, [a154]
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
mad r0, r1, r4 | st r0, [a17c] | ld r4, [c044]
mov r3, #0.041666666666666664 | st r0, [0346] | st r3, [a184]
ld r0, [0346]
addxy r0, r0, r4
st r0, [0342]
ld r0, [0342]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [0481]
mul r1, r0, r0
mul r2, r1, #0.0078125 | ld r1, [c071]
mul r4, r2, r3 | st r2, [a18c]
mad r7, r4, #2.3466666666666667 | ld r4, [0483]
ph_wrap r0, r3, r7
mul r3, r2, r1 | st r0, [0480]
mad r4, r3, #2.3466666666666667
ph_wrap r7, r3, r4 | ld r4, [c072]
addxy r1, r0, r7 | mul r0, r2, r4 | st r7, [0482] | ld r7, [0485] | ld r4, [c073]
mul r3, r1, #4.76837158203125e-07 | mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7 | ld r7, [0487]
mul r0, r2, r4 | mad r3, r1, #4.76837158203125e-07 | st r1, [0484]
mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7
mad r3, r1, #4.76837158203125e-07 | st r1, [0486] | ld r1, [c074]
mul r4, r3, #256.0 | mul r3, r2, r1 | ld r1, [c075]
mul r0, r4, #256.0 | ld r4, [0489]
mad r4, r3, #2.3466666666666667 | mul r7, r0, #8.0
ph_wrap r0, r3, r4 | ld r4, [048b]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [0488] | ld r1, [c076]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4 | ld r4, [048d]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [048a]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4
mad r7, r0, #0.25 | st r0, [048c]
mul r2, r7, #0.5714983344088459
mul r1, r2, r2
mul r3, r1, #-0.43681829687651003
addxy r4, r3, #1.2582678124581252 | ld r3, [a065]
ymacd r4, r1, #-1.6510263081007801 | st r3, [035c] | st r3, [a16d]
ymacd r4, r1, #1.8030530594057832 | ld r1, [0359]
mul r0, r4, r2 | ld r4, [a040] | ld r2, [035d]
subxy r2, r2, r3 | st r0, [0340]
mul r2, r2, r2 | ld r0, [0340]
mul r0, r0, r4 | mad r1, r1, -r2 | ld r4, [034e] | ld r2, [0282]
mad r0, r0, r4 | ld r4, [c058]
addxy r0, r1, r4 | st r0, [a155]
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
mad r0, r1, r4 | st r0, [a17d] | ld r4, [c044]
mov r3, #0.041666666666666664 | st r0, [0356] | st r3, [a185]
ld r0, [0356]
addxy r0, r0, r4
st r0, [0352]
ld r0, [0352]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [04a1]
mul r1, r0, r0
mul r2, r1, #0.0078125 | ld r1, [c071]
mul r4, r2, r3 | st r2, [a18d]
mad r7, r4, #2.3466666666666667 | ld r4, [04a3]
ph_wrap r0, r3, r7
mul r3, r2, r1 | st r0, [04a0]
mad r4, r3, #2.3466666666666667
ph_wrap r7, r3, r4 | ld r4, [c072]
addxy r1, r0, r7 | mul r0, r2, r4 | st r7, [04a2] | ld r7, [04a5] | ld r4, [c073]
mul r3, r1, #4.76837158203125e-07 | mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7 | ld r7, [04a7]
mul r0, r2, r4 | mad r3, r1, #4.76837158203125e-07 | st r1, [04a4]
mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7
mad r3, r1, #4.76837158203125e-07 | st r1, [04a6] | ld r1, [c074]
mul r4, r3, #256.0 | mul r3, r2, r1 | ld r1, [c075]
mul r0, r4, #256.0 | ld r4, [04a9]
mad r4, r3, #2.3466666666666667 | mul r7, r0, #8.0
ph_wrap r0, r3, r4 | ld r4, [04ab]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [04a8] | ld r1, [c076]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4 | ld r4, [04ad]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [04aa]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4
mad r7, r0, #0.25 | st r0, [04ac]
mul r2, r7, #0.5714983344088459
mul r1, r2, r2
mul r3, r1, #-0.43681829687651003
addxy r4, r3, #1.2582678124581252 | ld r3, [a066]
ymacd r4, r1, #-1.6510263081007801 | st r3, [036c] | st r3, [a16e]
ymacd r4, r1, #1.8030530594057832 | ld r1, [0369]
mul r0, r4, r2 | ld r4, [a040] | ld r2, [036d]
subxy r2, r2, r3 | st r0, [0350]
mul r2, r2, r2 | ld r0, [0350]
mul r0, r0, r4 | mad r1, r1, -r2 | ld r4, [035e] | ld r2, [0282]
mad r0, r0, r4 | ld r4, [c058]
addxy r0, r1, r4 | st r0, [a156]
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
mad r0, r1, r4 | st r0, [a17e] | ld r4, [c044]
mov r3, #0.041666666666666664 | st r0, [0366] | st r3, [a186]
ld r0, [0366]
addxy r0, r0, r4
st r0, [0362]
ld r0, [0362]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [04c1]
mul r1, r0, r0
mul r2, r1, #0.0078125 | ld r1, [c071]
mul r4, r2, r3 | st r2, [a18e]
mad r7, r4, #2.3466666666666667 | ld r4, [04c3]
ph_wrap r0, r3, r7
mul r3, r2, r1 | st r0, [04c0]
mad r4, r3, #2.3466666666666667
ph_wrap r7, r3, r4 | ld r4, [c072]
addxy r1, r0, r7 | mul r0, r2, r4 | st r7, [04c2] | ld r7, [04c5] | ld r4, [c073]
mul r3, r1, #4.76837158203125e-07 | mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7 | ld r7, [04c7]
mul r0, r2, r4 | mad r3, r1, #4.76837158203125e-07 | st r1, [04c4]
mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7
mad r3, r1, #4.76837158203125e-07 | st r1, [04c6] | ld r1, [c074]
mul r4, r3, #256.0 | mul r3, r2, r1 | ld r1, [c075]
mul r0, r4, #256.0 | ld r4, [04c9]
mad r4, r3, #2.3466666666666667 | mul r7, r0, #8.0
ph_wrap r0, r3, r4 | ld r4, [04cb]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [04c8] | ld r1, [c076]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4 | ld r4, [04cd]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [04ca]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4
mad r7, r0, #0.25 | st r0, [04cc]
mul r2, r7, #0.5714983344088459
mul r1, r2, r2
mul r3, r1, #-0.43681829687651003
addxy r4, r3, #1.2582678124581252 | ld r3, [a067]
ymacd r4, r1, #-1.6510263081007801 | st r3, [037c] | st r3, [a16f]
ymacd r4, r1, #1.8030530594057832 | ld r1, [0379]
mul r0, r4, r2 | ld r4, [a040] | ld r2, [037d]
subxy r2, r2, r3 | st r0, [0360]
mul r2, r2, r2 | ld r0, [0360]
mul r0, r0, r4 | mad r1, r1, -r2 | ld r4, [036e] | ld r2, [0282]
mad r0, r0, r4 | ld r4, [c058]
addxy r0, r1, r4 | st r0, [a157]
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
mad r0, r1, r4 | st r0, [a17f] | ld r4, [c044]
mov r3, #0.041666666666666664 | st r0, [0376] | st r3, [a187]
ld r0, [0376]
addxy r0, r0, r4
st r0, [0372]
ld r0, [0372]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [04e1]
mul r1, r0, r0
mul r2, r1, #0.0078125 | ld r1, [c071]
mul r4, r2, r3 | st r2, [a18f]
mad r7, r4, #2.3466666666666667 | ld r4, [04e3]
ph_wrap r0, r3, r7
mul r3, r2, r1 | st r0, [04e0]
mad r4, r3, #2.3466666666666667
ph_wrap r7, r3, r4 | ld r4, [c072]
addxy r1, r0, r7 | mul r0, r2, r4 | st r7, [04e2] | ld r7, [04e5] | ld r4, [c073]
mul r3, r1, #4.76837158203125e-07 | mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7 | ld r7, [04e7]
mul r0, r2, r4 | mad r3, r1, #4.76837158203125e-07 | st r1, [04e4]
mad r7, r0, #2.3466666666666667
ph_wrap r1, r3, r7
mad r3, r1, #4.76837158203125e-07 | st r1, [04e6] | ld r1, [c074]
mul r4, r3, #256.0 | mul r3, r2, r1 | ld r1, [c075]
mul r0, r4, #256.0 | ld r4, [04e9]
mad r4, r3, #2.3466666666666667 | mul r7, r0, #8.0
ph_wrap r0, r3, r4 | ld r4, [04eb]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [04e8] | ld r1, [c076]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4 | ld r4, [04ed]
mul r3, r2, r1 | mad r7, r0, #0.25 | st r0, [04ea]
mad r4, r3, #2.3466666666666667
ph_wrap r0, r3, r4
mad r7, r0, #0.25 | st r0, [04ec]
mul r2, r7, #0.5714983344088459
mul r1, r2, r2
mul r3, r1, #-0.43681829687651003
addxy r4, r3, #1.2582678124581252
ymacd r4, r1, #-1.6510263081007801
ymacd r4, r1, #1.8030530594057832
mul r0, r4, r2 | ld r4, [a040]
st r0, [0370]
ld r0, [0370]
mul r0, r0, r4 | ld r4, [037e]
mad r0, r0, r4
st r0, [a158]
