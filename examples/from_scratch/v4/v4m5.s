; v4m5: voices 2,3 gated -> bank 5 pair window A400/A401 (DSP4)
ld r4, [a029]
ld r1, [8200]
mov r4, r4
mul r1, r1, r4
st r1, [a400]
ld r4, [a038]
ld r2, [8201]
mov r4, r4
mul r2, r2, r4
st r2, [a401]
