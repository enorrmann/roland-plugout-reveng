mov r7, #0.0 | mov r5, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r3, [a168]
addxy r7, r7, r0 | mov r0, #0.0 | st r3, [0604] | ld r6, [c000] | ld r2, [0603]
addxy r7, r7, r1 | st r0, [6008] | ld r1, [0601]
mul r7, r7, #0.6666666666666666 | st r0, [6009] | ld r4, [a160]
mul r7, r7, r6 | mov r6, #1.0 | st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [0605]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r7, [6006]
mul r3, r3, #256.0 | mul r0, r0, r0 | st r7, [6007]
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | mov r5, #0.0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0602]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2
mad r4, r3, #1.25 | ld r3, [a169]
subxy r4, r1, r4 | st r3, [0610]
mad r1, r4, r0 | ld r0, [a151]
min r1, r1, r6 | ld r4, [a161]
mov r6, #1.0 | st r1, [0600]
ld r1, [0600]
mul r2, r1, r0 | ld r0, [0611] | ld r1, [060d]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r2, [0400] | ld r2, [060f]
mul r3, r3, #256.0 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [060e]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0 | ld r0, [a152]
min r1, r1, r6
st r1, [060c]
ld r1, [060c]
mul r2, r1, r0
st r2, [0402]
