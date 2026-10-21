; own SAW for the DCO2 slot (voices A and B), written for asm/asm.py
; f = 440 * 2^(p - 4.75) / 1.032045, fs = 96 kHz (calibrated to Roland's SAW: 2026-10-21, +52 cents measured);
; phase lives in the ring (load S+1, store S)
mov r5, #0.0
mov r6, #1.0
; voice A
ld r1, [0671]
ld r0, [2207]
add r0, #-4.75
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
st r1, [0670]
st r1, [2221]
st r1, [221d]
; voice B
ld r1, [0681]
ld r0, [2214]
add r0, #-4.75
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
st r1, [0680]
st r1, [2229]
st r1, [2225]
