; s1mm1: master DSP B @0x1000: slave 6006 / 6007 x volume (C01C) -> A118 / A119 (the OS output program reads them)
ld r0, [6006]
ld r1, [6007]
ld r7, [c01c]
mov r4, r4
mul r0, r0, r7
mul r1, r1, r7
st r0, [a118]
st r1, [a119]
mov r2, #0.0
st r2, [a11a]
st r2, [a11b]
st r2, [a11c]
st r2, [a11d]
