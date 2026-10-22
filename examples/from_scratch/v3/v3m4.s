; v3m4: voices 0,1 gated mix -> IOMD 6020 / 6021
ld r4, [a02a]
ld r1, [8005]
mov r4, r4
mul r1, r1, r4
ld r4, [a039]
ld r2, [8006]
mov r4, r4
mul r2, r2, r4
addxy r1, r1, r2
st r1, [6020]
st r1, [6021]
