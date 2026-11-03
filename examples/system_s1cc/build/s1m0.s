mov r0, #0.0 | ld r1, [0181] | ld r4, [0183]
mul r7, r4, #-1.0 | st r0, [0300] | ld r0, [a060]
subxy r2, r1, r0 | addxy r1, r7, #1.0 | st r0, [0180]
mul r3, r2, r2 | ld r2, [c070]
mad r4, r3, r1 | ld r3, [0185]
addxy r1, r3, #0.1180340051651001 | mul r7, r4, r2
mul r0, r1, #2.0 | st r7, [0182]
ph_wrap r2, r3, r0
addxy r1, r2, #1.0 | mul r7, r2, #0.5
mul r0, r1, r1 | addxy r3, r7, #0.5
shl r7, r0, 9 | st r3, [0184] | ld r0, [c071]
mul r3, r7, #1.99853515625
ph_wrap r2, r3, r3
mul r1, r4, r2 | ld r4, [0189]
mul r7, r1, r0 | mov r0, #0.0 | ld r1, [0187]
st r0, [0302] | ld r0, [a061]
subxy r2, r1, r0 | mul r7, r4, #-1.0 | st r7, [a151] | st r0, [0186]
mul r3, r2, r2 | addxy r1, r7, #1.0 | ld r2, [c070]
mad r4, r3, r1 | ld r3, [018b]
addxy r1, r3, #0.1180340051651001 | mul r7, r4, r2
mul r0, r1, #2.0 | st r7, [0188]
ph_wrap r2, r3, r0
addxy r1, r2, #1.0 | mul r7, r2, #0.5
mul r0, r1, r1 | addxy r3, r7, #0.5
shl r7, r0, 9 | st r3, [018a] | ld r0, [c071]
mul r3, r7, #1.99853515625
ph_wrap r2, r3, r3
mul r1, r4, r2 | ld r4, [018f]
mul r7, r1, r0 | mov r0, #0.0 | ld r1, [018d]
st r0, [0304] | ld r0, [a062]
subxy r2, r1, r0 | mul r7, r4, #-1.0 | st r7, [a152] | st r0, [018c]
mul r3, r2, r2 | addxy r1, r7, #1.0 | ld r2, [c070]
mad r4, r3, r1 | ld r3, [0191]
addxy r1, r3, #0.1180340051651001 | mul r7, r4, r2
mul r0, r1, #2.0 | st r7, [018e]
ph_wrap r2, r3, r0
addxy r1, r2, #1.0 | mul r7, r2, #0.5
mul r0, r1, r1 | addxy r3, r7, #0.5
shl r7, r0, 9 | st r3, [0190] | ld r0, [c071]
mul r3, r7, #1.99853515625
ph_wrap r2, r3, r3
mul r1, r4, r2 | ld r4, [0195]
mul r7, r1, r0 | mov r0, #0.0 | ld r1, [0193]
st r0, [0306] | ld r0, [a063]
subxy r2, r1, r0 | mul r7, r4, #-1.0 | st r7, [a153] | st r0, [0192]
mul r3, r2, r2 | addxy r1, r7, #1.0 | ld r2, [c070]
mad r4, r3, r1 | ld r3, [0197]
addxy r1, r3, #0.1180340051651001 | mul r7, r4, r2
mul r0, r1, #2.0 | st r7, [0194]
ph_wrap r2, r3, r0
addxy r1, r2, #1.0 | mul r7, r2, #0.5
mul r0, r1, r1 | addxy r3, r7, #0.5
shl r7, r0, 9 | st r3, [0196] | ld r0, [c071]
mul r3, r7, #1.99853515625
ph_wrap r2, r3, r3
mul r1, r4, r2
mul r7, r1, r0 | mov r0, #0.0
st r0, [0308] | ld r0, [a064]
mov r0, #0.0 | st r0, [a1a4]
st r0, [030a] | ld r0, [a065]
mov r0, #0.0 | st r0, [a1a5]
st r0, [030c] | ld r0, [a066]
mov r0, #0.0 | st r0, [a1a6]
st r0, [030e] | ld r0, [a067]
st r7, [a154]
st r0, [a1a7]
