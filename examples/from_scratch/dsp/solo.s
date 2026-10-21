; solo: eight SAW voices with note gate, DSP4 main (our own, make_solo.py)
mov r5, #0.0:32
mov r6, #1.0:32
mov r7, #0.0:32
ld r0, [a070]
ld r4, [a078]
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
addxy r7, r7, r1
ld r0, [a071]
ld r4, [a079]
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
ld r1, [0675]
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
st r1, [0674]
mul r1, r1, r4
addxy r7, r7, r1
ld r0, [a072]
ld r4, [a07a]
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
ld r1, [0679]
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
st r1, [0678]
mul r1, r1, r4
addxy r7, r7, r1
ld r0, [a073]
ld r4, [a07b]
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
ld r1, [067d]
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
st r1, [067c]
mul r1, r1, r4
addxy r7, r7, r1
ld r0, [a074]
ld r4, [a07c]
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
ld r1, [0681]
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
st r1, [0680]
mul r1, r1, r4
addxy r7, r7, r1
ld r0, [a075]
ld r4, [a07d]
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
ld r1, [0685]
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
st r1, [0684]
mul r1, r1, r4
addxy r7, r7, r1
ld r0, [a076]
ld r4, [a07e]
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
ld r1, [0689]
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
st r1, [0688]
mul r1, r1, r4
addxy r7, r7, r1
ld r0, [a077]
ld r4, [a07f]
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
ld r1, [068d]
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
st r1, [068c]
mul r1, r1, r4
addxy r7, r7, r1
mul r7, r7, #0.125:32
st r7, [6020]
st r7, [6021]
