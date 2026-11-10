ld r6, [c000] | ld r0, [a151]
mul r2, r0, r6 | ld r1, [a152] | ld r4, [0183]
mul r3, r1, r6 | mul r7, r4, #-1.0 | ld r0, [a153] | st r2, [6006]
mul r2, r0, r6 | ld r1, [a154] | st r3, [6007]
mul r3, r1, r6 | ld r0, [0409] | st r2, [6008]
mul r2, r0, r6 | ld r1, [040b] | st r3, [6009]
mul r3, r1, r6 | ld r0, [040d] | st r2, [600a]
mul r2, r0, r6 | ld r1, [040f] | st r3, [600b]
mov r0, #0.0 | mul r3, r1, r6 | ld r1, [0181]
ld r0, [a1a4] | st r2, [600c]
subxy r2, r1, r0 | addxy r1, r7, #1.0 | st r3, [600d]
mul r3, r2, r2 | ld r2, [0185] | st r0, [0180]
addxy r7, r2, #1.2360680103302002 | mad r4, r3, r1
ph_wrap r3, r3, r7
mul r0, r4, #0.99790000915527344 | addxy r1, r3, #1.0 | st r3, [0184]
mul r0, r1, r1 | st r0, [0182]
shl r2, r0, 9 | ld r0, [0187]
mul r7, r2, #1.99853515625 | ld r2, [c060]
ph_wrap r3, r3, r7
mul r1, r4, r3
subxy r7, r0, r1 | ld r1, [c061]
mul r4, r2, r7
addxy r3, r4, r0 | ld r0, [a1a5] | ld r4, [018b]
mul r2, r3, r1 | mul r7, r4, #-1.0 | ld r1, [0189] | st r3, [0186]
subxy r2, r1, r0 | addxy r1, r7, #1.0 | st r2, [0408]
mul r3, r2, r2 | ld r2, [018d] | st r0, [0188]
addxy r7, r2, #1.2360680103302002 | mad r4, r3, r1
ph_wrap r3, r3, r7
mul r0, r4, #0.99790000915527344 | addxy r1, r3, #1.0 | st r3, [018c]
mul r0, r1, r1 | st r0, [018a]
shl r2, r0, 9 | ld r0, [018f]
mul r7, r2, #1.99853515625 | ld r2, [c060]
ph_wrap r3, r3, r7
mul r1, r4, r3
subxy r7, r0, r1 | ld r1, [c061]
mul r4, r2, r7
addxy r3, r4, r0 | ld r0, [a1a6] | ld r4, [0193]
mul r2, r3, r1 | mul r7, r4, #-1.0 | ld r1, [0191] | st r3, [018e]
subxy r2, r1, r0 | addxy r1, r7, #1.0 | st r2, [040a]
mul r3, r2, r2 | ld r2, [0195] | st r0, [0190]
addxy r7, r2, #1.2360680103302002 | mad r4, r3, r1
ph_wrap r3, r3, r7
mul r0, r4, #0.99790000915527344 | addxy r1, r3, #1.0 | st r3, [0194]
mul r0, r1, r1 | st r0, [0192]
shl r2, r0, 9 | ld r0, [0197]
mul r7, r2, #1.99853515625 | ld r2, [c060]
ph_wrap r3, r3, r7
mul r1, r4, r3
subxy r7, r0, r1 | ld r1, [c061]
mul r4, r2, r7
addxy r3, r4, r0 | ld r0, [a1a7] | ld r4, [019b]
mul r2, r3, r1 | mul r7, r4, #-1.0 | ld r1, [0199] | st r3, [0196]
subxy r2, r1, r0 | addxy r1, r7, #1.0 | st r2, [040c]
mul r3, r2, r2 | ld r2, [019d] | st r0, [0198]
addxy r7, r2, #1.2360680103302002 | mad r4, r3, r1
ph_wrap r3, r3, r7
mul r0, r4, #0.99790000915527344 | addxy r1, r3, #1.0 | st r3, [019c]
mul r0, r1, r1 | st r0, [019a]
shl r2, r0, 9 | ld r0, [019f]
mul r7, r2, #1.99853515625 | ld r2, [c060]
ph_wrap r3, r3, r7
mul r1, r4, r3
subxy r7, r0, r1 | ld r1, [c061]
mul r4, r2, r7
addxy r3, r4, r0
mul r2, r3, r1 | st r3, [019e]
st r2, [040e]
