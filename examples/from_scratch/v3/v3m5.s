; v3m5: voices 2,3 gated mix -> IOMD 60B0 / 60B1
ld r4, [a029]
ld r1, [8201]
mov r4, r4
mul r1, r1, r4
ld r4, [a038]
ld r2, [8202]
mov r4, r4
mul r2, r2, r4
addxy r1, r1, r2
st r1, [60b0]
st r1, [60b1]
