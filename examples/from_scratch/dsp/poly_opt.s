; poly with ph_wrap, scheduled (make_poly.py --opt)
mov r5, #0.0:32 | mov r6, #1.0:32 | ld r7, [0691] | ld r0, [a068] | ld r3, [06a1]
subxy r1, r7, r0 | addxy r3, r3, #0.00011458333333333333:32 | ld r0, [a060]
mul r2, r1, #256.0:32 | mov r4, #0.0:32
mul r2, r2, #256.0:32 | mov r4, #0.0:32
ph_wrap r3, r3, r3 | max r2, r2, r5
min r2, r2, r6 | st r3, [06b0] | st r3, [06a0]
mul r2, r2, #0.0098:32 | ld r3, [06b0]
mad r0, r3, #0.02:32 | add r2, #0.0002:32
add r0, #-4.75:32 | mul r1, r1, r2
mul r2, r0, #0.04332169878499658:32 | mov r3, #0.041666666666666664:32 | ld r0, [a069]
ymacd r3, r2, #0.16666666666666666:32 | addxy r7, r7, r1 | ld r1, [0671]
ymacd r3, r2, #0.5:32 | st r7, [0690]
ymacd r3, r2, #1.0:32
ymacd r3, r2, #1.0:32
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667:32
addxy r1, r3, r1
ph_wrap r1, r3, r1 | ld r3, [06b0]
mul r1, r1, r7 | st r1, [0670] | ld r7, [0695]
addxy r4, r4, r1 | subxy r1, r7, r0 | ld r0, [a061]
mul r2, r1, #256.0:32 | mad r0, r3, #0.02:32
mul r2, r2, #256.0:32 | add r0, #-4.75:32
max r2, r2, r5
min r2, r2, r6
mul r2, r2, #0.0098:32 | mov r3, #0.041666666666666664:32
add r2, #0.0002:32
mul r1, r1, r2 | mul r2, r0, #0.04332169878499658:32 | ld r0, [a06a]
ymacd r3, r2, #0.16666666666666666:32 | addxy r7, r7, r1 | ld r1, [0675]
ymacd r3, r2, #0.5:32 | st r7, [0694]
ymacd r3, r2, #1.0:32
ymacd r3, r2, #1.0:32
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667:32
addxy r1, r3, r1
ph_wrap r1, r3, r1 | ld r3, [06b0]
mul r1, r1, r7 | st r1, [0674] | ld r7, [0699]
addxy r4, r4, r1 | subxy r1, r7, r0 | ld r0, [a062]
mul r2, r1, #256.0:32 | mad r0, r3, #0.02:32
mul r2, r2, #256.0:32 | add r0, #-4.75:32
max r2, r2, r5
min r2, r2, r6
mul r2, r2, #0.0098:32 | mov r3, #0.041666666666666664:32
add r2, #0.0002:32
mul r1, r1, r2 | mul r2, r0, #0.04332169878499658:32 | ld r0, [a06b]
ymacd r3, r2, #0.16666666666666666:32 | addxy r7, r7, r1 | ld r1, [0679]
ymacd r3, r2, #0.5:32 | st r7, [0698]
ymacd r3, r2, #1.0:32
ymacd r3, r2, #1.0:32
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667:32
addxy r1, r3, r1
ph_wrap r1, r3, r1 | ld r3, [06b0]
mul r1, r1, r7 | st r1, [0678] | ld r7, [069d]
addxy r4, r4, r1 | subxy r1, r7, r0 | ld r0, [a063]
mul r2, r1, #256.0:32 | mad r0, r3, #0.02:32
mul r2, r2, #256.0:32 | add r0, #-4.75:32
max r2, r2, r5
min r2, r2, r6
mul r2, r2, #0.0098:32 | mov r3, #0.041666666666666664:32
add r2, #0.0002:32
mul r1, r1, r2 | mul r2, r0, #0.04332169878499658:32
ymacd r3, r2, #0.16666666666666666:32 | addxy r7, r7, r1 | ld r1, [067d]
ymacd r3, r2, #0.5:32 | st r7, [069c]
ymacd r3, r2, #1.0:32
ymacd r3, r2, #1.0:32
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667:32
addxy r1, r3, r1
ph_wrap r1, r3, r1
mul r1, r1, r7 | st r1, [067c]
addxy r4, r4, r1
mul r4, r4, #0.25:32
st r4, [6020]
st r4, [6021]
