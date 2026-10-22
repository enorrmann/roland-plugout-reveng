; v4m4: voices 0,1 gated + A400/A401 (DSP5) + IOMD 6000-6003 -> 6020/6021; voices -> 6060-6063
ld r4, [a02a]
ld r1, [8004]
mov r4, r4
mul r1, r1, r4
st r1, [6060]
ld r4, [a039]
ld r2, [8005]
mov r4, r4
mul r2, r2, r4
st r2, [6061]
addxy r1, r1, r2
ld r2, [a400]
mov r4, r4
st r2, [6062]
addxy r1, r1, r2
ld r2, [a401]
mov r4, r4
st r2, [6063]
addxy r1, r1, r2
ld r2, [6000]
mov r4, r4
addxy r1, r1, r2
ld r2, [6001]
mov r4, r4
addxy r1, r1, r2
ld r2, [6002]
mov r4, r4
addxy r1, r1, r2
ld r2, [6003]
mov r4, r4
addxy r1, r1, r2
st r1, [6020]
st r1, [6021]
