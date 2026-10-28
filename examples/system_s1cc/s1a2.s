ld r0, [a064] | ld r1, [0391] | ld r5, [03b5]
subxy r1, r1, r0 | ld r4, [a065] | st r0, [0390] | ld r3, [c049]
subxy r5, r5, r4 | mul r1, r1, r1 | ld r2, [a03c] | st r4, [03b4]
mul r5, r5, r5 | ld r6, [a03d] | ld r7, [c049]
st r2, [a164] | ld r2, [0395]
mad r2, r2, -r1 | st r6, [a165] | ld r6, [03b9]
mad r6, r6, -r5 | st r0, [a16c] | ld r0, [0393]
addxy r1, r0, #-0.999 | st r4, [a16d] | ld r4, [03b7]
addxy r5, r4, #-0.999 | mul r1, r1, #256.0
mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
max r2, r2, r1 | ld r1, [c048]
max r6, r6, r5 | st r2, [0394] | ld r5, [c048]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [03b8]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7 | ld r3, [c051]
mul r2, r2, #1.25 | mul r6, r6, #1.25 | ld r7, [c051]
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r1, [a124]
min r0, r0, #1.0 | ld r5, [a125] | ld r2, [c04b]
mul r1, r1, r2 | ld r6, [c04b] | st r0, [0392]
mul r5, r5, r6 | ld r2, [c05e]
min r4, r4, #1.0 | ld r6, [c05e]
mad r1, r0, r2 | mad r5, r4, r6 | ld r2, [c05f] | st r4, [03b6]
addxy r1, r1, r2 | ld r6, [c05f] | ld r2, [a034]
addxy r5, r5, r6 | addxy r2, r2, r1 | ld r6, [a035]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [a1b4]
mul r6, r5, #0.08664339756999316 | mov r0, #0.041666666666666664 | st r6, [a1b5]
mov r4, #0.041666666666666664 | ymacd r0, r2, #0.16666666666666666
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a044]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a045]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mul r2, r1, #-0.08664339756999316 | st r2, [0396] | ld r1, [039b]
mul r6, r5, #-0.08664339756999316 | mov r0, #0.16666666666666666 | st r6, [03ba] | ld r5, [03bf]
mov r4, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a04c]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a04d]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0 | ld r0, [0396]
mul r6, r6, r4 | st r2, [0398] | ld r2, [a044]
addxy r2, r1, r2 | st r6, [03bc] | ld r6, [a045]
addxy r6, r5, r6 | ld r4, [03ba]
ph_wrap r1, r3, r2 | ld r2, [c050]
ph_wrap r5, r3, r6 | st r1, [039a] | ld r6, [c050]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [03be]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [039c] | ld r1, [a124]
mad r2, r1, r3 | st r5, [03c0] | ld r5, [a125] | ld r3, [c052]
mad r6, r5, r7 | ld r1, [0392] | ld r5, [03b6] | ld r7, [c052]
mad r2, r1, r3 | mad r6, r5, r7 | ld r1, [a17c]
ld r5, [a17d] | ld r3, [c053]
mad r2, r1, r3 | ld r7, [c053] | ld r1, [a174]
mad r6, r5, r7 | ld r5, [a175] | ld r3, [c054]
mad r2, r1, r3 | ld r7, [c054] | ld r1, [039c]
mad r6, r5, r7 | ld r5, [03c0] | ld r3, [c055]
mad r2, r1, r3 | ld r7, [c055] | ld r1, [a124]
mad r6, r5, r7 | st r2, [039e] | ld r2, [c058] | ld r5, [a125]
st r6, [03c2] | ld r6, [c058]
ld r3, [c059]
mad r2, r1, r3 | ld r7, [c059] | ld r1, [0392]
mad r6, r5, r7 | ld r5, [03b6] | ld r3, [c05a]
mad r2, r1, r3 | ld r7, [c05a] | ld r1, [a17c]
mad r6, r5, r7 | ld r5, [a17d] | ld r3, [c05b]
mad r2, r1, r3 | ld r7, [c05b] | ld r1, [a174]
mad r6, r5, r7 | ld r5, [a175] | ld r3, [c05c]
mad r2, r1, r3 | ld r7, [c05c] | ld r1, [039c]
mad r6, r5, r7 | ld r5, [03c0] | ld r3, [c05d]
mad r2, r1, r3 | ld r7, [c05d] | ld r1, [0398]
mad r6, r5, r7 | st r2, [03a0] | ld r2, [c074] | ld r5, [03bc]
st r6, [03c4] | ld r6, [c074]
ld r3, [c076] | st r1, [03a8]
mul r2, r2, r3 | ld r7, [c076] | st r5, [03cc]
mul r6, r6, r7 | ld r3, [c070]
mul r2, r2, r3 | ld r7, [c070]
mul r6, r6, r7 | ld r3, [c075]
ld r7, [c075]
ld r1, [c077]
mul r3, r3, r1 | ld r5, [c077]
mul r7, r7, r5 | ld r1, [c071]
mul r3, r3, r1 | mul r1, r0, r2 | ld r5, [c071]
mul r7, r7, r5 | mul r5, r4, r6
min r2, r1, #0.00390625 | ld r1, [03a8]
min r6, r5, #0.00390625 | ld r5, [03cc] | st r2, [03ac]
mul r3, r1, r3 | mul r7, r5, r7 | st r6, [03d0]
addxy r2, r0, #-0.004999999888241291 | addxy r6, r4, #-0.004999999888241291 | st r3, [03aa] | ld r3, [039e]
mul r2, r2, #5.479451911627053 | mul r6, r6, #5.479451911627053 | st r7, [03ce] | ld r7, [03c2]
max r3, r3, r2 | ld r2, [03a5]
max r7, r7, r6 | st r3, [03ae] | ld r6, [03c9]
mov r3, r0 | mov r7, r4 | st r7, [03d2]
mad r3, r2, #0.5 | mad r7, r6, #0.5
mul r2, r3, #2.0 | mul r6, r7, #2.0
ph_wrap r3, r3, r2
ph_wrap r7, r3, r6 | st r3, [03a4]
addxy r2, r3, #1.0 | addxy r6, r7, #1.0 | st r7, [03c8]
st r3, [03b0] | st r7, [03d4]
ph_wrap r2, r3, r2
ph_wrap r6, r3, r6
mul r3, r0, #2.0 | mul r7, r4, #2.0
min r1, r2, r3
min r5, r6, r7
mul r0, r0, #-2.0 | mul r4, r4, #-2.0
max r1, r1, r0 | st r0, [03b2] | ld r0, [03a8]
max r5, r5, r4 | st r4, [03d6] | ld r4, [03cc]
mul r1, r0, r1 | mul r5, r4, r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #32.0 | mul r5, r5, #32.0
max r0, r1, -r1
max r4, r5, -r5
addxy r0, r0, #-2.0 | addxy r4, r4, #-2.0
mul r1, r1, r0 | mul r5, r5, r4
addxy r0, r1, r2 | addxy r4, r5, r6 | ld r1, [03ae] | ld r5, [03d2]
mov r2, r1 | mov r6, r5 | ld r1, [03b0] | ld r5, [03d4]
mad r2, r1, #-0.68493148895338163 | mad r6, r5, #-0.68493148895338163
addxy r1, r2, #-0.65753423000762279 | addxy r5, r6, #-0.65753423000762279
mul r2, r1, #-1.4600000381469727 | mul r6, r5, #-1.4600000381469727
ph_wrap r1, r3, r2
ph_wrap r5, r3, r6
min r2, r1, r3 | ld r3, [03b2]
min r6, r5, r7 | ld r7, [03d6]
max r2, r2, r3 | ld r3, [03a8]
max r6, r6, r7 | ld r7, [03cc]
mul r3, r3, r2 | mul r7, r7, r6
mul r2, r3, #256.0 | mul r6, r7, #256.0
mul r3, r2, #32.0 | mul r7, r6, #32.0
max r2, r3, -r3
max r6, r7, -r7
addxy r2, r2, #-2.0 | addxy r6, r6, #-2.0
mul r3, r3, r2 | mul r7, r7, r6
addxy r2, r3, r1 | addxy r6, r7, r5 | ld r1, [03ae] | ld r5, [03d2]
subxy r3, r2, r0 | subxy r7, r6, r4
mad r1, r3, #-0.68493148895338163 | mad r5, r7, #-0.68493148895338163
addxy r2, r0, #1.0 | addxy r6, r4, #1.0
addxy r3, r1, #0.027397258945758795 | addxy r7, r5, #0.027397258945758795
mul r1, r2, r3 | mul r5, r6, r7 | ld r2, [c072]
mad r0, r1, #-0.73000001907348633 | mad r4, r5, #-0.73000001907348633 | ld r6, [c072] | ld r1, [03a0] | ld r5, [03c4]
mul r3, r2, r0 | mul r7, r6, r4 | ld r2, [03ac] | ld r6, [03d0]
addxy r0, r2, #-3.9062499126885086e-05 | addxy r4, r6, #-3.9062499126885086e-05 | st r3, [03ae]
mul r0, r0, #256.0 | mul r4, r4, #256.0 | st r7, [03d2]
mul r0, r0, #2.7397259558135265 | mul r4, r4, #2.7397259558135265
max r1, r1, r0 | ld r0, [03a7]
max r5, r5, r4 | ld r4, [03cb] | st r1, [03b2]
mov r3, r2 | mov r7, r6 | st r5, [03d6]
mad r3, r0, #0.00390625 | mad r7, r4, #0.00390625
mul r0, r3, #256.0 | mul r4, r7, #256.0
ph_wrap r3, r3, r0
ph_wrap r7, r3, r4 | st r3, [03a6]
addxy r0, r3, #1.0 | addxy r4, r7, #1.0 | st r7, [03ca]
st r3, [03a8] | st r7, [03cc]
ph_wrap r0, r3, r0
ph_wrap r4, r3, r4
mul r3, r2, #256.0 | mul r7, r6, #256.0
min r1, r0, r3
min r5, r4, r7
mul r2, r2, #-256.0 | mul r6, r6, #-256.0
max r1, r1, r2 | st r2, [03ac] | ld r2, [03aa]
max r5, r5, r6 | st r6, [03d0] | ld r6, [03ce]
mul r1, r2, r1 | mul r5, r6, r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #256.0 | mul r5, r5, #256.0
mul r1, r1, #8.0 | mul r5, r5, #8.0
max r2, r1, -r1
max r6, r5, -r5
addxy r2, r2, #-1.0 | addxy r6, r6, #-1.0
mul r1, r1, r2 | mul r5, r5, r6 | ld r2, [03b2] | ld r6, [03d6]
mad r1, r0, #0.25 | mad r5, r4, #0.25
mov r0, r2 | mov r4, r6 | ld r2, [03a8] | ld r6, [03cc]
mad r0, r2, #-0.68493148895338163 | mad r4, r6, #-0.68493148895338163
addxy r2, r0, #-0.65753423000762279 | addxy r6, r4, #-0.65753423000762279
mul r0, r2, #-1.4600000381469727 | mul r4, r6, #-1.4600000381469727
ph_wrap r2, r3, r0
ph_wrap r6, r3, r4
min r0, r2, r3 | ld r3, [03ac]
min r4, r6, r7 | ld r7, [03d0]
max r0, r0, r3 | ld r3, [03aa]
max r4, r4, r7 | ld r7, [03ce]
mul r3, r3, r0 | mul r7, r7, r4
mul r0, r3, #256.0 | mul r4, r7, #256.0
mul r3, r0, #256.0 | mul r7, r4, #256.0
mul r0, r3, #8.0 | mul r4, r7, #8.0
max r3, r0, -r0
max r7, r4, -r4
addxy r3, r3, #-1.0 | addxy r7, r7, #-1.0
mul r0, r0, r3 | mul r4, r4, r7
mad r0, r2, #0.25 | mad r4, r6, #0.25 | ld r2, [03b2] | ld r6, [03d6]
subxy r3, r0, r1 | subxy r7, r4, r5
mad r3, r2, #-0.36500000953674316 | mad r7, r6, #-0.36500000953674316
addxy r0, r1, #0.25 | addxy r4, r5, #0.25
addxy r2, r3, #-0.0099999997764825821 | addxy r6, r7, #-0.0099999997764825821
mul r3, r0, r2 | mul r7, r4, r6 | ld r0, [c073]
mad r3, r1, #0.5 | mad r7, r5, #0.5 | ld r4, [c073] | ld r1, [03ae] | ld r5, [03d2]
mul r2, r3, r0 | mul r6, r7, r4
mad r2, r1, #0.125 | mad r6, r5, #0.125 | ld r1, [039c] | ld r5, [03c0]
mul r3, r2, #4.0 | mul r7, r6, #4.0 | ld r2, [c056]
st r3, [03a2] | st r7, [03c6]
ld r0, [03a2] | ld r4, [03c6] | ld r6, [c056]
mad r0, r1, r2 | mad r4, r5, r6 | ld r1, [03d8] | ld r2, [c057]
mad r0, r1, r2 | ld r5, [03d8] | ld r6, [c057]
mad r4, r5, r6 | st r0, [a155]
st r4, [a156]
