ld r0, [6006]
ld r1, [6007]
ld r7, [c01c]
mov r4, r4
mul r0, r0, r7
mul r1, r1, r7
mov r4, r4
st r0, [2000]
st r1, [2002]
ld r2, [2001]
ld r3, [2003]
st r0, [4034]
st r0, [4036]
ld r4, [4035]
ld r5, [4037]
mov r7, r7
mul r4, r4, #0.7
mov r7, r7
mul r5, r5, #0.7
mov r7, r7
addxy r2, r2, r4
mov r7, r7
addxy r3, r3, r5
mov r7, r7
st r2, [a118]
st r3, [a119]
