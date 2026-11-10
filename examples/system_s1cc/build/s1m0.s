mov r0, #0.0 | ld r1, [0181] | ld r4, [0183]
mul r7, r4, #-1.0 | st r0, [0300] | ld r0, [a060]
subxy r2, r1, r0 | addxy r1, r7, #1.0 | st r0, [0180]
mul r3, r2, r2 | ld r2, [0185]
addxy r7, r2, #1.2360680103302002 | mad r4, r3, r1
ph_wrap r3, r3, r7
mul r0, r4, #0.99790000915527344 | addxy r1, r3, #1.0 | st r3, [0184]
mul r0, r1, r1 | st r0, [0182]
shl r2, r0, 9 | ld r0, [0187]
mul r7, r2, #1.99853515625 | ld r2, [c070]
ph_wrap r3, r3, r7
mul r1, r4, r3
subxy r7, r0, r1 | ld r1, [c071]
mul r4, r2, r7
addxy r3, r4, r0 | mov r0, #0.0 | ld r4, [018b]
mul r2, r3, r1 | mul r7, r4, #-1.0 | st r0, [0302] | ld r0, [a061]
ld r1, [0189] | st r2, [a151] | st r3, [0186]
subxy r2, r1, r0 | addxy r1, r7, #1.0 | st r0, [0188]
mul r3, r2, r2 | ld r2, [018d]
addxy r7, r2, #1.2360680103302002 | mad r4, r3, r1
ph_wrap r3, r3, r7
mul r0, r4, #0.99790000915527344 | addxy r1, r3, #1.0 | st r3, [018c]
mul r0, r1, r1 | st r0, [018a]
shl r2, r0, 9 | ld r0, [018f]
mul r7, r2, #1.99853515625 | ld r2, [c070]
ph_wrap r3, r3, r7
mul r1, r4, r3
subxy r7, r0, r1 | ld r1, [c071]
mul r4, r2, r7
addxy r3, r4, r0 | mov r0, #0.0 | ld r4, [0193]
mul r2, r3, r1 | mul r7, r4, #-1.0 | st r0, [0304] | ld r0, [a062]
ld r1, [0191] | st r2, [a152] | st r3, [018e]
subxy r2, r1, r0 | addxy r1, r7, #1.0 | st r0, [0190]
mul r3, r2, r2 | ld r2, [0195]
addxy r7, r2, #1.2360680103302002 | mad r4, r3, r1
ph_wrap r3, r3, r7
mul r0, r4, #0.99790000915527344 | addxy r1, r3, #1.0 | st r3, [0194]
mul r0, r1, r1 | st r0, [0192]
shl r2, r0, 9 | ld r0, [0197]
mul r7, r2, #1.99853515625 | ld r2, [c070]
ph_wrap r3, r3, r7
mul r1, r4, r3
subxy r7, r0, r1 | ld r1, [c071]
mul r4, r2, r7
addxy r3, r4, r0 | mov r0, #0.0 | ld r4, [019b]
mul r2, r3, r1 | mul r7, r4, #-1.0 | st r0, [0306] | ld r0, [a063]
ld r1, [0199] | st r2, [a153] | st r3, [0196]
subxy r2, r1, r0 | addxy r1, r7, #1.0 | st r0, [0198]
mul r3, r2, r2 | ld r2, [019d]
addxy r7, r2, #1.2360680103302002 | mad r4, r3, r1
ph_wrap r3, r3, r7
mul r0, r4, #0.99790000915527344 | addxy r1, r3, #1.0 | st r3, [019c]
mul r0, r1, r1 | st r0, [019a]
shl r2, r0, 9 | ld r0, [019f]
mul r7, r2, #1.99853515625 | ld r2, [c070]
ph_wrap r3, r3, r7
mul r1, r4, r3
subxy r7, r0, r1 | ld r1, [c071]
mul r4, r2, r7
addxy r3, r4, r0 | mov r0, #0.0
mul r2, r3, r1 | st r0, [0308] | ld r0, [a064]
mov r0, #0.0 | st r0, [a1a4] | st r3, [019e]
st r0, [030a] | ld r0, [a065]
mov r0, #0.0 | st r0, [a1a5]
st r0, [030c] | ld r0, [a066]
mov r0, #0.0 | st r0, [a1a6]
st r0, [030e] | ld r0, [a067]
st r2, [a154]
st r0, [a1a7]
