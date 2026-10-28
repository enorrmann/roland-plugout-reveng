ld r0, [a040] | ld r3, [0301]
mov r4, r0 | ld r1, [a048] | ld r2, [c071]
mad r4, r3, #0.5 | st r1, [0306]
mul r7, r4, #2.0
ph_wrap r3, r3, r7
addxy r4, r3, #1.0 | st r3, [0300]
ph_wrap r7, r3, r4 | st r4, [0302]
mul r4, r0, #2.0 | st r3, [0304]
min r3, r7, r4
mul r0, r0, #-2.0
max r3, r3, r0
mul r3, r1, r3
mul r3, r3, #256.0
mul r3, r3, #32.0
max r1, r3, -r3
addxy r1, r1, #-2.0
mul r3, r3, r1
addxy r1, r3, r7 | ld r7, [0304]
mov r3, r2
mad r3, r7, #-0.5
addxy r7, r3, #-0.5
mul r3, r7, #-2.0
ph_wrap r7, r3, r3
min r3, r7, r4
max r4, r3, r0 | ld r3, [0306]
mul r0, r3, r4
mul r3, r0, #256.0
mul r4, r3, #32.0
max r0, r4, -r4
addxy r3, r0, #-2.0
mul r0, r4, r3
addxy r4, r0, r7
subxy r3, r1, r4 | ld r4, [0302]
addxy r0, r2, r3
addxy r7, r2, r0 | ld r0, [c073]
ph_tri r3, r3, -r4 | ld r2, [c072]
addxy r4, r7, #-1.0
mul r7, r0, r4 | mul r0, r1, r2 | ld r1, [c074]
addxy r4, r0, r7 | ld r7, [c070]
mul r2, r3, r1 | ld r1, [a030]
addxy r0, r2, r4 | st r1, [a1a0] | ld r2, [c071]
mul r3, r7, r0 | ld r0, [a038]
st r0, [a160]
ld r0, [a041]
mov r4, r0 | st r3, [a151] | ld r3, [0309]
mad r4, r3, #0.5 | ld r1, [a060]
mul r7, r4, #2.0 | st r1, [a168]
ph_wrap r3, r3, r7 | ld r1, [a049]
addxy r4, r3, #1.0 | st r3, [0308]
ph_wrap r7, r3, r4 | st r4, [030a]
mul r4, r0, #2.0 | st r3, [030c]
min r3, r7, r4 | st r1, [030e]
mul r0, r0, #-2.0
max r3, r3, r0
mul r3, r1, r3
mul r3, r3, #256.0
mul r3, r3, #32.0
max r1, r3, -r3
addxy r1, r1, #-2.0
mul r3, r3, r1
addxy r1, r3, r7 | ld r7, [030c]
mov r3, r2
mad r3, r7, #-0.5
addxy r7, r3, #-0.5
mul r3, r7, #-2.0
ph_wrap r7, r3, r3
min r3, r7, r4
max r4, r3, r0 | ld r3, [030e]
mul r0, r3, r4
mul r3, r0, #256.0
mul r4, r3, #32.0
max r0, r4, -r4
addxy r3, r0, #-2.0
mul r0, r4, r3
addxy r4, r0, r7
subxy r3, r1, r4 | ld r4, [030a]
addxy r0, r2, r3
addxy r7, r2, r0 | ld r0, [c073]
ph_tri r3, r3, -r4 | ld r2, [c072]
addxy r4, r7, #-1.0
mul r7, r0, r4 | mul r0, r1, r2 | ld r1, [c074]
addxy r4, r0, r7 | ld r7, [c070]
mul r2, r3, r1 | ld r1, [a031]
addxy r0, r2, r4 | st r1, [a1a1] | ld r2, [c071]
mul r3, r7, r0 | ld r0, [a039]
st r0, [a161]
ld r0, [a042]
mov r4, r0 | st r3, [a152] | ld r3, [0311]
mad r4, r3, #0.5 | ld r1, [a061]
mul r7, r4, #2.0 | st r1, [a169]
ph_wrap r3, r3, r7 | ld r1, [a04a]
addxy r4, r3, #1.0 | st r3, [0310]
ph_wrap r7, r3, r4 | st r4, [0312]
mul r4, r0, #2.0 | st r3, [0314]
min r3, r7, r4 | st r1, [0316]
mul r0, r0, #-2.0
max r3, r3, r0
mul r3, r1, r3
mul r3, r3, #256.0
mul r3, r3, #32.0
max r1, r3, -r3
addxy r1, r1, #-2.0
mul r3, r3, r1
addxy r1, r3, r7 | ld r7, [0314]
mov r3, r2
mad r3, r7, #-0.5
addxy r7, r3, #-0.5
mul r3, r7, #-2.0
ph_wrap r7, r3, r3
min r3, r7, r4
max r4, r3, r0 | ld r3, [0316]
mul r0, r3, r4
mul r3, r0, #256.0
mul r4, r3, #32.0
max r0, r4, -r4
addxy r3, r0, #-2.0
mul r0, r4, r3
addxy r4, r0, r7
subxy r3, r1, r4 | ld r4, [0312]
addxy r0, r2, r3
addxy r7, r2, r0 | ld r0, [c073]
ph_tri r3, r3, -r4 | ld r2, [c072]
addxy r4, r7, #-1.0
mul r7, r0, r4 | mul r0, r1, r2 | ld r1, [c074]
addxy r4, r0, r7 | ld r7, [c070]
mul r2, r3, r1 | ld r1, [a032]
addxy r0, r2, r4 | st r1, [a1a2] | ld r2, [c071]
mul r3, r7, r0 | ld r0, [a03a]
st r0, [a162]
ld r0, [a043]
mov r4, r0 | st r3, [a153] | ld r3, [0319]
mad r4, r3, #0.5 | ld r1, [a062]
mul r7, r4, #2.0 | st r1, [a16a]
ph_wrap r3, r3, r7 | ld r1, [a04b]
addxy r4, r3, #1.0 | st r3, [0318]
ph_wrap r7, r3, r4 | st r4, [031a]
mul r4, r0, #2.0 | st r3, [031c]
min r3, r7, r4 | st r1, [031e]
mul r0, r0, #-2.0
max r3, r3, r0
mul r3, r1, r3
mul r3, r3, #256.0
mul r3, r3, #32.0
max r1, r3, -r3
addxy r1, r1, #-2.0
mul r3, r3, r1
addxy r1, r3, r7 | ld r7, [031c]
mov r3, r2
mad r3, r7, #-0.5
addxy r7, r3, #-0.5
mul r3, r7, #-2.0
ph_wrap r7, r3, r3
min r3, r7, r4
max r4, r3, r0 | ld r3, [031e]
mul r0, r3, r4
mul r3, r0, #256.0
mul r4, r3, #32.0
max r0, r4, -r4
addxy r3, r0, #-2.0
mul r0, r4, r3
addxy r4, r0, r7
subxy r3, r1, r4 | ld r4, [031a]
addxy r0, r2, r3
addxy r7, r2, r0 | ld r0, [c073]
ph_tri r3, r3, -r4 | ld r2, [c072]
addxy r4, r7, #-1.0
mul r7, r0, r4 | mul r0, r1, r2 | ld r1, [c074]
addxy r4, r0, r7 | ld r7, [c070]
mul r2, r3, r1 | ld r1, [a033]
addxy r0, r2, r4 | st r1, [a1a3] | ld r2, [c071]
mul r3, r7, r0 | ld r0, [a03b]
st r0, [a163]
ld r0, [a044]
mov r4, r0 | st r3, [a154] | ld r3, [0321]
mad r4, r3, #0.5 | ld r1, [a063]
mul r7, r4, #2.0 | st r1, [a16b]
ph_wrap r3, r3, r7 | ld r1, [a04c]
addxy r4, r3, #1.0 | st r3, [0320]
ph_wrap r7, r3, r4 | st r4, [0322]
mul r4, r0, #2.0 | st r3, [0324]
min r3, r7, r4 | st r1, [0326]
mul r0, r0, #-2.0
max r3, r3, r0
mul r3, r1, r3
mul r3, r3, #256.0
mul r3, r3, #32.0
max r1, r3, -r3
addxy r1, r1, #-2.0
mul r3, r3, r1
addxy r1, r3, r7 | ld r7, [0324]
mov r3, r2
mad r3, r7, #-0.5
addxy r7, r3, #-0.5
mul r3, r7, #-2.0
ph_wrap r7, r3, r3
min r3, r7, r4
max r4, r3, r0 | ld r3, [0326]
mul r0, r3, r4
mul r3, r0, #256.0
mul r4, r3, #32.0
max r0, r4, -r4
addxy r3, r0, #-2.0
mul r0, r4, r3
addxy r4, r0, r7
subxy r3, r1, r4 | ld r4, [0322]
addxy r0, r2, r3
addxy r7, r2, r0 | ld r0, [c073]
ph_tri r3, r3, -r4 | ld r2, [c072]
addxy r4, r7, #-1.0
mul r7, r0, r4 | mul r0, r1, r2 | ld r1, [c074]
addxy r4, r0, r7 | ld r7, [c070]
mul r2, r3, r1 | ld r1, [a034]
addxy r0, r2, r4 | st r1, [a1a4] | ld r2, [c071]
mul r3, r7, r0 | ld r0, [a03c]
st r0, [a164]
ld r0, [a045]
mov r4, r0 | st r3, [a155] | ld r3, [0329]
mad r4, r3, #0.5 | ld r1, [a064]
mul r7, r4, #2.0 | st r1, [a16c]
ph_wrap r3, r3, r7 | ld r1, [a04d]
addxy r4, r3, #1.0 | st r3, [0328]
ph_wrap r7, r3, r4 | st r4, [032a]
mul r4, r0, #2.0 | st r3, [032c]
min r3, r7, r4 | st r1, [032e]
mul r0, r0, #-2.0
max r3, r3, r0
mul r3, r1, r3
mul r3, r3, #256.0
mul r3, r3, #32.0
max r1, r3, -r3
addxy r1, r1, #-2.0
mul r3, r3, r1
addxy r1, r3, r7 | ld r7, [032c]
mov r3, r2
mad r3, r7, #-0.5
addxy r7, r3, #-0.5
mul r3, r7, #-2.0
ph_wrap r7, r3, r3
min r3, r7, r4
max r4, r3, r0 | ld r3, [032e]
mul r0, r3, r4
mul r3, r0, #256.0
mul r4, r3, #32.0
max r0, r4, -r4
addxy r3, r0, #-2.0
mul r0, r4, r3
addxy r4, r0, r7
subxy r3, r1, r4 | ld r4, [032a]
addxy r0, r2, r3
addxy r7, r2, r0 | ld r0, [c073]
ph_tri r3, r3, -r4 | ld r2, [c072]
addxy r4, r7, #-1.0
mul r7, r0, r4 | mul r0, r1, r2 | ld r1, [c074]
addxy r4, r0, r7 | ld r7, [c070]
mul r2, r3, r1 | ld r1, [a035]
addxy r0, r2, r4 | st r1, [a1a5] | ld r2, [c071]
mul r3, r7, r0 | ld r0, [a03d]
st r0, [a165]
ld r0, [a046]
mov r4, r0 | st r3, [a156] | ld r3, [0331]
mad r4, r3, #0.5 | ld r1, [a065]
mul r7, r4, #2.0 | st r1, [a16d]
ph_wrap r3, r3, r7 | ld r1, [a04e]
addxy r4, r3, #1.0 | st r3, [0330]
ph_wrap r7, r3, r4 | st r4, [0332]
mul r4, r0, #2.0 | st r3, [0334]
min r3, r7, r4 | st r1, [0336]
mul r0, r0, #-2.0
max r3, r3, r0
mul r3, r1, r3
mul r3, r3, #256.0
mul r3, r3, #32.0
max r1, r3, -r3
addxy r1, r1, #-2.0
mul r3, r3, r1
addxy r1, r3, r7 | ld r7, [0334]
mov r3, r2
mad r3, r7, #-0.5
addxy r7, r3, #-0.5
mul r3, r7, #-2.0
ph_wrap r7, r3, r3
min r3, r7, r4
max r4, r3, r0 | ld r3, [0336]
mul r0, r3, r4
mul r3, r0, #256.0
mul r4, r3, #32.0
max r0, r4, -r4
addxy r3, r0, #-2.0
mul r0, r4, r3
addxy r4, r0, r7
subxy r3, r1, r4 | ld r4, [0332]
addxy r0, r2, r3
addxy r7, r2, r0 | ld r0, [c073]
ph_tri r3, r3, -r4 | ld r2, [c072]
addxy r4, r7, #-1.0
mul r7, r0, r4 | mul r0, r1, r2 | ld r1, [c074]
addxy r4, r0, r7 | ld r7, [c070]
mul r2, r3, r1 | ld r1, [a036]
addxy r0, r2, r4 | st r1, [a1a6] | ld r2, [c071]
mul r3, r7, r0 | ld r0, [a03e]
st r0, [a166]
ld r0, [a047]
mov r4, r0 | st r3, [a157] | ld r3, [0339]
mad r4, r3, #0.5 | ld r1, [a066]
mul r7, r4, #2.0 | st r1, [a16e]
ph_wrap r3, r3, r7 | ld r1, [a04f]
addxy r4, r3, #1.0 | st r3, [0338]
ph_wrap r7, r3, r4 | st r4, [033a]
mul r4, r0, #2.0 | st r3, [033c]
min r3, r7, r4 | st r1, [033e]
mul r0, r0, #-2.0
max r3, r3, r0
mul r3, r1, r3
mul r3, r3, #256.0
mul r3, r3, #32.0
max r1, r3, -r3
addxy r1, r1, #-2.0
mul r3, r3, r1
addxy r1, r3, r7 | ld r7, [033c]
mov r3, r2
mad r3, r7, #-0.5
addxy r7, r3, #-0.5
mul r3, r7, #-2.0
ph_wrap r7, r3, r3
min r3, r7, r4
max r4, r3, r0 | ld r3, [033e]
mul r0, r3, r4
mul r3, r0, #256.0
mul r4, r3, #32.0
max r0, r4, -r4
addxy r3, r0, #-2.0
mul r0, r4, r3
addxy r4, r0, r7
subxy r3, r1, r4 | ld r4, [033a]
addxy r0, r2, r3
addxy r7, r2, r0 | ld r0, [c073]
ph_tri r3, r3, -r4 | ld r2, [c072]
addxy r4, r7, #-1.0
mul r7, r0, r4 | mul r0, r1, r2 | ld r1, [c074]
addxy r4, r0, r7 | ld r7, [c070]
mul r2, r3, r1 | ld r1, [a037]
addxy r0, r2, r4 | st r1, [a1a7]
mul r3, r7, r0 | ld r0, [a03f]
ld r1, [a067]
st r3, [a158]
st r0, [a167]
st r1, [a16f]
