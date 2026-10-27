mov r3, #0.041666666666666664 | ld r0, [a030]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r7, r4, r4 | ld r4, [c071]
mul r0, r7, r7
mul r1, r0, r0
mul r2, r1, #0.03125
mul r3, r1, #0.018333333333333333 | mul r7, r2, r4 | ld r1, [a038]
mul r0, r7, #1.1733333333333333 | st r3, [0300] | ld r3, [0305]
subxy r4, r3, r1 | st r0, [0302]
mul r0, r4, #256.0 | st r7, [030a] | ld r7, [c073]
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0 | ld r0, [c070]
mul r4, r4, #0.5 | subxy r0, r7, r0
addxy r4, r4, #0.5
mul r4, r4, r0
addxy r0, r7, r4 | subxy r7, r3, r1 | ld r1, [0307]
mad r1, r2, #0.14666666666666667
mul r4, r7, r0 | addxy r7, r1, #-0.5
mad r3, r4, #0.015625
mul r0, r7, #2.0 | ld r7, [0309] | st r3, [0304]
ph_wrap r4, r3, r0 | ld r0, [030a]
mul r2, r4, #0.5
mad r7, r0, #0.29333333333333333
addxy r1, r2, #0.5
addxy r2, r7, #-0.5 | st r1, [0306]
mul r1, r2, #2.0
ph_wrap r0, r3, r1
mul r7, r0, #0.5
addxy r2, r7, #0.5 | ld r7, [c072]
addxy r1, r4, r0 | st r2, [0308]
mul r2, r7, r1
mul r4, r3, r2 | mov r3, #0.041666666666666664
mul r0, r4, #0.5
st r0, [a151]
ld r0, [a031]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r7, r4, r4 | ld r4, [c071]
mul r0, r7, r7
mul r1, r0, r0
mul r2, r1, #0.03125
mul r3, r1, #0.018333333333333333 | mul r7, r2, r4 | ld r1, [a039]
mul r0, r7, #1.1733333333333333 | st r3, [030c] | ld r3, [0311]
subxy r4, r3, r1 | st r0, [030e]
mul r0, r4, #256.0 | st r7, [0316] | ld r7, [c073]
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0 | ld r0, [c070]
mul r4, r4, #0.5 | subxy r0, r7, r0
addxy r4, r4, #0.5
mul r4, r4, r0
addxy r0, r7, r4 | subxy r7, r3, r1 | ld r1, [0313]
mad r1, r2, #0.14666666666666667
mul r4, r7, r0 | addxy r7, r1, #-0.5
mad r3, r4, #0.015625
mul r0, r7, #2.0 | ld r7, [0315] | st r3, [0310]
ph_wrap r4, r3, r0 | ld r0, [0316]
mul r2, r4, #0.5
mad r7, r0, #0.29333333333333333
addxy r1, r2, #0.5
addxy r2, r7, #-0.5 | st r1, [0312]
mul r1, r2, #2.0
ph_wrap r0, r3, r1
mul r7, r0, #0.5
addxy r2, r7, #0.5 | ld r7, [c072]
addxy r1, r4, r0 | st r2, [0314]
mul r2, r7, r1
mul r4, r3, r2 | mov r3, #0.041666666666666664
mul r0, r4, #0.5
st r0, [a152]
ld r0, [a032]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r7, r4, r4 | ld r4, [c071]
mul r0, r7, r7
mul r1, r0, r0
mul r2, r1, #0.03125
mul r3, r1, #0.018333333333333333 | mul r7, r2, r4 | ld r1, [a03a]
mul r0, r7, #1.1733333333333333 | st r3, [0318] | ld r3, [031d]
subxy r4, r3, r1 | st r0, [031a]
mul r0, r4, #256.0 | st r7, [0322] | ld r7, [c073]
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0 | ld r0, [c070]
mul r4, r4, #0.5 | subxy r0, r7, r0
addxy r4, r4, #0.5
mul r4, r4, r0
addxy r0, r7, r4 | subxy r7, r3, r1 | ld r1, [031f]
mad r1, r2, #0.14666666666666667
mul r4, r7, r0 | addxy r7, r1, #-0.5
mad r3, r4, #0.015625
mul r0, r7, #2.0 | ld r7, [0321] | st r3, [031c]
ph_wrap r4, r3, r0 | ld r0, [0322]
mul r2, r4, #0.5
mad r7, r0, #0.29333333333333333
addxy r1, r2, #0.5
addxy r2, r7, #-0.5 | st r1, [031e]
mul r1, r2, #2.0
ph_wrap r0, r3, r1
mul r7, r0, #0.5
addxy r2, r7, #0.5 | ld r7, [c072]
addxy r1, r4, r0 | st r2, [0320]
mul r2, r7, r1
mul r4, r3, r2 | mov r3, #0.041666666666666664
mul r0, r4, #0.5
st r0, [a153]
ld r0, [a033]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r7, r4, r4 | ld r4, [c071]
mul r0, r7, r7
mul r1, r0, r0
mul r2, r1, #0.03125
mul r3, r1, #0.018333333333333333 | mul r7, r2, r4 | ld r1, [a03b]
mul r0, r7, #1.1733333333333333 | st r3, [0324] | ld r3, [0329]
subxy r4, r3, r1 | st r0, [0326]
mul r0, r4, #256.0 | st r7, [032e] | ld r7, [c073]
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0 | ld r0, [c070]
mul r4, r4, #0.5 | subxy r0, r7, r0
addxy r4, r4, #0.5
mul r4, r4, r0
addxy r0, r7, r4 | subxy r7, r3, r1 | ld r1, [032b]
mad r1, r2, #0.14666666666666667
mul r4, r7, r0 | addxy r7, r1, #-0.5
mad r3, r4, #0.015625
mul r0, r7, #2.0 | ld r7, [032d] | st r3, [0328]
ph_wrap r4, r3, r0 | ld r0, [032e]
mul r2, r4, #0.5
mad r7, r0, #0.29333333333333333
addxy r1, r2, #0.5
addxy r2, r7, #-0.5 | st r1, [032a]
mul r1, r2, #2.0
ph_wrap r0, r3, r1
mul r7, r0, #0.5
addxy r2, r7, #0.5 | ld r7, [c072]
addxy r1, r4, r0 | st r2, [032c]
mul r2, r7, r1
mul r4, r3, r2 | mov r3, #0.041666666666666664
mul r0, r4, #0.5
st r0, [a154]
ld r0, [a034]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r7, r4, r4 | ld r4, [c071]
mul r0, r7, r7
mul r1, r0, r0
mul r2, r1, #0.03125
mul r3, r1, #0.018333333333333333 | mul r7, r2, r4 | ld r1, [a03c]
mul r0, r7, #1.1733333333333333 | st r3, [0330] | ld r3, [0335]
subxy r4, r3, r1 | st r0, [0332]
mul r0, r4, #256.0 | st r7, [033a] | ld r7, [c073]
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0 | ld r0, [c070]
mul r4, r4, #0.5 | subxy r0, r7, r0
addxy r4, r4, #0.5
mul r4, r4, r0
addxy r0, r7, r4 | subxy r7, r3, r1 | ld r1, [0337]
mad r1, r2, #0.14666666666666667
mul r4, r7, r0 | addxy r7, r1, #-0.5
mad r3, r4, #0.015625
mul r0, r7, #2.0 | ld r7, [0339] | st r3, [0334]
ph_wrap r4, r3, r0 | ld r0, [033a]
mul r2, r4, #0.5
mad r7, r0, #0.29333333333333333
addxy r1, r2, #0.5
addxy r2, r7, #-0.5 | st r1, [0336]
mul r1, r2, #2.0
ph_wrap r0, r3, r1
mul r7, r0, #0.5
addxy r2, r7, #0.5 | ld r7, [c072]
addxy r1, r4, r0 | st r2, [0338]
mul r2, r7, r1
mul r4, r3, r2 | mov r3, #0.041666666666666664
mul r0, r4, #0.5
st r0, [a155]
ld r0, [a035]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r7, r4, r4 | ld r4, [c071]
mul r0, r7, r7
mul r1, r0, r0
mul r2, r1, #0.03125
mul r3, r1, #0.018333333333333333 | mul r7, r2, r4 | ld r1, [a03d]
mul r0, r7, #1.1733333333333333 | st r3, [033c] | ld r3, [0341]
subxy r4, r3, r1 | st r0, [033e]
mul r0, r4, #256.0 | st r7, [0346] | ld r7, [c073]
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0 | ld r0, [c070]
mul r4, r4, #0.5 | subxy r0, r7, r0
addxy r4, r4, #0.5
mul r4, r4, r0
addxy r0, r7, r4 | subxy r7, r3, r1 | ld r1, [0343]
mad r1, r2, #0.14666666666666667
mul r4, r7, r0 | addxy r7, r1, #-0.5
mad r3, r4, #0.015625
mul r0, r7, #2.0 | ld r7, [0345] | st r3, [0340]
ph_wrap r4, r3, r0 | ld r0, [0346]
mul r2, r4, #0.5
mad r7, r0, #0.29333333333333333
addxy r1, r2, #0.5
addxy r2, r7, #-0.5 | st r1, [0342]
mul r1, r2, #2.0
ph_wrap r0, r3, r1
mul r7, r0, #0.5
addxy r2, r7, #0.5 | ld r7, [c072]
addxy r1, r4, r0 | st r2, [0344]
mul r2, r7, r1
mul r4, r3, r2 | mov r3, #0.041666666666666664
mul r0, r4, #0.5
st r0, [a156]
ld r0, [a036]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r7, r4, r4 | ld r4, [c071]
mul r0, r7, r7
mul r1, r0, r0
mul r2, r1, #0.03125
mul r3, r1, #0.018333333333333333 | mul r7, r2, r4 | ld r1, [a03e]
mul r0, r7, #1.1733333333333333 | st r3, [0348] | ld r3, [034d]
subxy r4, r3, r1 | st r0, [034a]
mul r0, r4, #256.0 | st r7, [0352] | ld r7, [c073]
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0 | ld r0, [c070]
mul r4, r4, #0.5 | subxy r0, r7, r0
addxy r4, r4, #0.5
mul r4, r4, r0
addxy r0, r7, r4 | subxy r7, r3, r1 | ld r1, [034f]
mad r1, r2, #0.14666666666666667
mul r4, r7, r0 | addxy r7, r1, #-0.5
mad r3, r4, #0.015625
mul r0, r7, #2.0 | ld r7, [0351] | st r3, [034c]
ph_wrap r4, r3, r0 | ld r0, [0352]
mul r2, r4, #0.5
mad r7, r0, #0.29333333333333333
addxy r1, r2, #0.5
addxy r2, r7, #-0.5 | st r1, [034e]
mul r1, r2, #2.0
ph_wrap r0, r3, r1
mul r7, r0, #0.5
addxy r2, r7, #0.5 | ld r7, [c072]
addxy r1, r4, r0 | st r2, [0350]
mul r2, r7, r1
mul r4, r3, r2 | mov r3, #0.041666666666666664
mul r0, r4, #0.5
st r0, [a157]
ld r0, [a037]
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r7, r4, r4 | ld r4, [c071]
mul r0, r7, r7
mul r1, r0, r0
mul r2, r1, #0.03125
mul r3, r1, #0.018333333333333333 | mul r7, r2, r4 | ld r1, [a03f]
mul r0, r7, #1.1733333333333333 | st r3, [0354] | ld r3, [0359]
subxy r4, r3, r1 | st r0, [0356]
mul r0, r4, #256.0 | st r7, [035e] | ld r7, [c073]
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0
mul r0, r4, #256.0
ph_clamp r4, r3, -r0 | ld r0, [c070]
mul r4, r4, #0.5 | subxy r0, r7, r0
addxy r4, r4, #0.5
mul r4, r4, r0
addxy r0, r7, r4 | subxy r7, r3, r1 | ld r1, [035b]
mad r1, r2, #0.14666666666666667
mul r4, r7, r0 | addxy r7, r1, #-0.5
mad r3, r4, #0.015625
mul r0, r7, #2.0 | ld r7, [035d] | st r3, [0358]
ph_wrap r4, r3, r0 | ld r0, [035e]
mul r2, r4, #0.5
mad r7, r0, #0.29333333333333333
addxy r1, r2, #0.5
addxy r2, r7, #-0.5 | st r1, [035a]
mul r1, r2, #2.0
ph_wrap r0, r3, r1
mul r7, r0, #0.5
addxy r2, r7, #0.5 | ld r7, [c072]
addxy r1, r4, r0 | st r2, [035c]
mul r2, r7, r1
mul r4, r3, r2
mul r0, r4, #0.5
st r0, [a158]
