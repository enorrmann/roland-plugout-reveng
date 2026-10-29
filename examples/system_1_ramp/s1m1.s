; s1m1: slave DSP B main @0x0100: A150 x C000 -> 6006 / 6007, the other outputs 0
ld r7, [c000]
ld r0, [a150]
mov r4, r4
mul r1, r0, r7
st r1, [6006]
st r1, [6007]
mov r0, #0.0
st r0, [6008]
st r0, [6009]
st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d]
