mov r7, #0.0 | ld r0, [a151] | ld r6, [c000]
addxy r7, r7, r0 | ld r1, [a152] | ld r4, [0183]
addxy r7, r7, r1 | ld r0, [a153]
addxy r7, r7, r0 | ld r1, [a154] | ld r0, [0409]
addxy r7, r7, r1 | ld r1, [040b]
addxy r7, r7, r0 | ld r0, [040d]
addxy r7, r7, r1 | ld r1, [040f]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | st r0, [6008] | ld r1, [0181]
mul r7, r7, #0.16666666666666666 | st r0, [6009]
mul r7, r7, r6 | st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [a1a4]
subxy r2, r1, r0 | st r7, [6006]
mul r7, r4, #-1.0 | mul r3, r2, r2 | st r7, [6007]
addxy r1, r7, #1.0 | st r0, [0180] | ld r2, [c060]
mad r4, r3, r1 | ld r3, [0185]
addxy r1, r3, #0.1180340051651001 | mul r7, r4, r2
mul r0, r1, #2.0 | st r7, [0182]
ph_wrap r2, r3, r0
addxy r1, r2, #1.0 | mul r7, r2, #0.5
mul r0, r1, r1 | addxy r3, r7, #0.5
shl r7, r0, 9 | st r3, [0184] | ld r0, [c061]
mul r3, r7, #1.99853515625
ph_wrap r2, r3, r3
mul r1, r4, r2 | ld r4, [0189]
mul r7, r1, r0 | ld r0, [a1a5] | ld r1, [0187]
subxy r2, r1, r0 | mul r7, r4, #-1.0 | st r7, [0408]
mul r3, r2, r2 | addxy r1, r7, #1.0 | st r0, [0186] | ld r2, [c060]
mad r4, r3, r1 | ld r3, [018b]
addxy r1, r3, #0.1180340051651001 | mul r7, r4, r2
mul r0, r1, #2.0 | st r7, [0188]
ph_wrap r2, r3, r0
addxy r1, r2, #1.0 | mul r7, r2, #0.5
mul r0, r1, r1 | addxy r3, r7, #0.5
shl r7, r0, 9 | st r3, [018a] | ld r0, [c061]
mul r3, r7, #1.99853515625
ph_wrap r2, r3, r3
mul r1, r4, r2 | ld r4, [018f]
mul r7, r1, r0 | ld r0, [a1a6] | ld r1, [018d]
subxy r2, r1, r0 | mul r7, r4, #-1.0 | st r7, [040a]
mul r3, r2, r2 | addxy r1, r7, #1.0 | st r0, [018c] | ld r2, [c060]
mad r4, r3, r1 | ld r3, [0191]
addxy r1, r3, #0.1180340051651001 | mul r7, r4, r2
mul r0, r1, #2.0 | st r7, [018e]
ph_wrap r2, r3, r0
addxy r1, r2, #1.0 | mul r7, r2, #0.5
mul r0, r1, r1 | addxy r3, r7, #0.5
shl r7, r0, 9 | st r3, [0190] | ld r0, [c061]
mul r3, r7, #1.99853515625
ph_wrap r2, r3, r3
mul r1, r4, r2 | ld r4, [0195]
mul r7, r1, r0 | ld r0, [a1a7] | ld r1, [0193]
subxy r2, r1, r0 | mul r7, r4, #-1.0 | st r7, [040c]
mul r3, r2, r2 | addxy r1, r7, #1.0 | st r0, [0192] | ld r2, [c060]
mad r4, r3, r1 | ld r3, [0197]
addxy r1, r3, #0.1180340051651001 | mul r7, r4, r2
mul r0, r1, #2.0 | st r7, [0194]
ph_wrap r2, r3, r0
addxy r1, r2, #1.0 | mul r7, r2, #0.5
mul r0, r1, r1 | addxy r3, r7, #0.5
shl r7, r0, 9 | st r3, [0196] | ld r0, [c061]
mul r3, r7, #1.99853515625
ph_wrap r2, r3, r3
mul r1, r4, r2
mul r7, r1, r0
st r7, [040e]
