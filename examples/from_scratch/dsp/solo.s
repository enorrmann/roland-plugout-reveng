; solo: SAW with note gate, DSP4 main program (our own, make_solo.py)
mov r5, #0.0:32
mov r6, #1.0:32
ld r0, [a070]
ld r4, [a071]
add r0, #-4.75:32
mul r2, r0, #0.04332169878499658:32
mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666:32
ymacd r3, r2, #0.5:32
ymacd r3, r2, #1.0:32
ymacd r3, r2, #1.0:32
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
ld r1, [0671]
mov r4, r4
mul r3, r3, #0.009166666666666667:32
addxy r1, r3, r1
addxy r2, r1, #-1.0:32
max r2, r2, r5
mul r2, r2, #256.0:32
min r2, r2, r6
mul r2, r2, #256.0:32
min r2, r2, r6
mul r2, r2, #256.0:32
min r2, r2, r6
mul r2, r2, #256.0:32
min r2, r2, r6
mul r2, r2, #2.0:32
subxy r1, r2, r1
st r1, [0670]
mul r1, r1, r4
st r1, [6020]
st r1, [6021]
