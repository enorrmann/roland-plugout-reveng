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
; the ramp measurement: value, time, crossing times
ld r3, [c030]
ld r5, [c031]
ld r6, [c032]
ld r4, [a1e1]
mov r0, r0
st r3, [a1e0]
addxy r4, r4, #1.52587890625e-05
mul r4, r4, r5
st r4, [a1e1]
addxy r2, r3, #-0.01
mul r2, r2, #-256.0
mul r2, r2, r6
min r2, r2, #1.0
max r2, r2, #0.0
mul r2, r2, #1.52587890625e-05
ld r4, [a1e2]
mov r0, r0
addxy r4, r4, r2
mul r4, r4, r5
st r4, [a1e2]
addxy r2, r3, #-0.1
mul r2, r2, #-256.0
mul r2, r2, r6
min r2, r2, #1.0
max r2, r2, #0.0
mul r2, r2, #1.52587890625e-05
ld r4, [a1e3]
mov r0, r0
addxy r4, r4, r2
mul r4, r4, r5
st r4, [a1e3]
addxy r2, r3, #-0.25
mul r2, r2, #-256.0
mul r2, r2, r6
min r2, r2, #1.0
max r2, r2, #0.0
mul r2, r2, #1.52587890625e-05
ld r4, [a1e4]
mov r0, r0
addxy r4, r4, r2
mul r4, r4, r5
st r4, [a1e4]
addxy r2, r3, #-0.5
mul r2, r2, #-256.0
mul r2, r2, r6
min r2, r2, #1.0
max r2, r2, #0.0
mul r2, r2, #1.52587890625e-05
ld r4, [a1e5]
mov r0, r0
addxy r4, r4, r2
mul r4, r4, r5
st r4, [a1e5]
addxy r2, r3, #-0.75
mul r2, r2, #-256.0
mul r2, r2, r6
min r2, r2, #1.0
max r2, r2, #0.0
mul r2, r2, #1.52587890625e-05
ld r4, [a1e6]
mov r0, r0
addxy r4, r4, r2
mul r4, r4, r5
st r4, [a1e6]
addxy r2, r3, #-0.9
mul r2, r2, #-256.0
mul r2, r2, r6
min r2, r2, #1.0
max r2, r2, #0.0
mul r2, r2, #1.52587890625e-05
ld r4, [a1e7]
mov r0, r0
addxy r4, r4, r2
mul r4, r4, r5
st r4, [a1e7]
addxy r2, r3, #-0.99
mul r2, r2, #-256.0
mul r2, r2, r6
min r2, r2, #1.0
max r2, r2, #0.0
mul r2, r2, #1.52587890625e-05
ld r4, [a1e8]
mov r0, r0
addxy r4, r4, r2
mul r4, r4, r5
st r4, [a1e8]
addxy r2, r3, #-0.999
mul r2, r2, #-256.0
mul r2, r2, r6
min r2, r2, #1.0
max r2, r2, #0.0
mul r2, r2, #1.52587890625e-05
ld r4, [a1e9]
mov r0, r0
addxy r4, r4, r2
mul r4, r4, r5
st r4, [a1e9]
mov r2, #0.375
st r2, [a1ea]
