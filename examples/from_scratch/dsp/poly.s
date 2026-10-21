; four-voice mono-slave voice, DSP5 DCO1 slot (make_poly.py)
mov r5, #0.0:32
mov r6, #1.0:32
mov r4, #0.0:32
ld r3, [06a1]
mov r4, r4
addxy r3, r3, #0.00011458333333333333:32
addxy r2, r3, #-1.0:32
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
subxy r3, r2, r3
st r3, [06a0]
st r3, [06b0]
mov r4, #0.0:32
ld r7, [0691]
ld r0, [a068]
mov r4, r4
subxy r1, r7, r0
mul r2, r1, #256.0:32
mul r2, r2, #256.0:32
max r2, r2, r5
min r2, r2, r6
mul r2, r2, #0.0098:32
add r2, #0.0002:32
mul r1, r1, r2
addxy r7, r7, r1
st r7, [0690]
ld r0, [a060]
ld r3, [06b0]
mad r0, r3, #0.02:32
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
mul r1, r1, r7
addxy r4, r4, r1
ld r7, [0695]
ld r0, [a069]
mov r4, r4
subxy r1, r7, r0
mul r2, r1, #256.0:32
mul r2, r2, #256.0:32
max r2, r2, r5
min r2, r2, r6
mul r2, r2, #0.0098:32
add r2, #0.0002:32
mul r1, r1, r2
addxy r7, r7, r1
st r7, [0694]
ld r0, [a061]
ld r3, [06b0]
mad r0, r3, #0.02:32
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
mul r1, r1, r7
addxy r4, r4, r1
ld r7, [0699]
ld r0, [a06a]
mov r4, r4
subxy r1, r7, r0
mul r2, r1, #256.0:32
mul r2, r2, #256.0:32
max r2, r2, r5
min r2, r2, r6
mul r2, r2, #0.0098:32
add r2, #0.0002:32
mul r1, r1, r2
addxy r7, r7, r1
st r7, [0698]
ld r0, [a062]
ld r3, [06b0]
mad r0, r3, #0.02:32
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
mul r1, r1, r7
addxy r4, r4, r1
ld r7, [069d]
ld r0, [a06b]
mov r4, r4
subxy r1, r7, r0
mul r2, r1, #256.0:32
mul r2, r2, #256.0:32
max r2, r2, r5
min r2, r2, r6
mul r2, r2, #0.0098:32
add r2, #0.0002:32
mul r1, r1, r2
addxy r7, r7, r1
st r7, [069c]
ld r0, [a063]
ld r3, [06b0]
mad r0, r3, #0.02:32
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
mul r1, r1, r7
addxy r4, r4, r1
mul r4, r4, #0.25:32
st r4, [6020]
st r4, [6021]
