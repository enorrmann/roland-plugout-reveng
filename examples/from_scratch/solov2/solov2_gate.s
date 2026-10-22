; solov2 DSP4 main: four gated voices (two from DSP2, two from DSP3), IOMD 6020 / 6021 (make_solov2.py)
mov r7, #0.0:32
ld r4, [a078]
ld r1, [8005]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
ld r4, [a079]
ld r1, [8006]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
ld r4, [a07a]
ld r1, [8204]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
ld r4, [a07b]
ld r1, [8205]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
mul r7, r7, #0.25:32
st r7, [6020]
st r7, [6021]
