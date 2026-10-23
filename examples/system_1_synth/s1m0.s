; s1m0 (STEP 5): slave DSP A main @0x0100: 8 voices x (DCO1 + DCO2 saws) -> A151+v
mov r5, #0.0
mov r6, #1.0
mov r7, #0.0
; voice 0: DCO1
ld r0, [a030]
mov r4, r4
add r0, #-4.75
ld r4, [c044]
mov r4, r4
addxy r0, r0, r4
ld r1, [0301]
mov r4, r4
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
st r1, [0300]
ld r4, [a040]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
; voice 0: DCO2
ld r0, [a030]
ld r4, [c045]
mov r4, r4
addxy r0, r0, r4
ld r4, [c046]
mov r4, r4
addxy r0, r0, r4
add r0, #-4.75
ld r1, [0303]
mov r4, r4
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
st r1, [0302]
ld r4, [a041]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
st r7, [a151]
ld r4, [a038]
mov r5, r5
st r4, [a160]
ld r4, [a060]
mov r5, r5
st r4, [a168]
; voice 1: DCO1
mov r7, #0.0
ld r0, [a031]
mov r4, r4
add r0, #-4.75
ld r4, [c044]
mov r4, r4
addxy r0, r0, r4
ld r1, [0311]
mov r4, r4
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
st r1, [0310]
ld r4, [a040]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
; voice 1: DCO2
ld r0, [a031]
ld r4, [c045]
mov r4, r4
addxy r0, r0, r4
ld r4, [c046]
mov r4, r4
addxy r0, r0, r4
add r0, #-4.75
ld r1, [0313]
mov r4, r4
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
st r1, [0312]
ld r4, [a041]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
st r7, [a152]
ld r4, [a039]
mov r5, r5
st r4, [a161]
ld r4, [a061]
mov r5, r5
st r4, [a169]
; voice 2: DCO1
mov r7, #0.0
ld r0, [a032]
mov r4, r4
add r0, #-4.75
ld r4, [c044]
mov r4, r4
addxy r0, r0, r4
ld r1, [0321]
mov r4, r4
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
st r1, [0320]
ld r4, [a040]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
; voice 2: DCO2
ld r0, [a032]
ld r4, [c045]
mov r4, r4
addxy r0, r0, r4
ld r4, [c046]
mov r4, r4
addxy r0, r0, r4
add r0, #-4.75
ld r1, [0323]
mov r4, r4
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
st r1, [0322]
ld r4, [a041]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
st r7, [a153]
ld r4, [a03a]
mov r5, r5
st r4, [a162]
ld r4, [a062]
mov r5, r5
st r4, [a16a]
; voice 3: DCO1
mov r7, #0.0
ld r0, [a033]
mov r4, r4
add r0, #-4.75
ld r4, [c044]
mov r4, r4
addxy r0, r0, r4
ld r1, [0331]
mov r4, r4
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
st r1, [0330]
ld r4, [a040]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
; voice 3: DCO2
ld r0, [a033]
ld r4, [c045]
mov r4, r4
addxy r0, r0, r4
ld r4, [c046]
mov r4, r4
addxy r0, r0, r4
add r0, #-4.75
ld r1, [0333]
mov r4, r4
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
st r1, [0332]
ld r4, [a041]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
st r7, [a154]
ld r4, [a03b]
mov r5, r5
st r4, [a163]
ld r4, [a063]
mov r5, r5
st r4, [a16b]
; voice 4: DCO1
mov r7, #0.0
ld r0, [a034]
mov r4, r4
add r0, #-4.75
ld r4, [c044]
mov r4, r4
addxy r0, r0, r4
ld r1, [0341]
mov r4, r4
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
st r1, [0340]
ld r4, [a040]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
; voice 4: DCO2
ld r0, [a034]
ld r4, [c045]
mov r4, r4
addxy r0, r0, r4
ld r4, [c046]
mov r4, r4
addxy r0, r0, r4
add r0, #-4.75
ld r1, [0343]
mov r4, r4
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
st r1, [0342]
ld r4, [a041]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
st r7, [a155]
ld r4, [a03c]
mov r5, r5
st r4, [a164]
ld r4, [a064]
mov r5, r5
st r4, [a16c]
; voice 5: DCO1
mov r7, #0.0
ld r0, [a035]
mov r4, r4
add r0, #-4.75
ld r4, [c044]
mov r4, r4
addxy r0, r0, r4
ld r1, [0351]
mov r4, r4
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
st r1, [0350]
ld r4, [a040]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
; voice 5: DCO2
ld r0, [a035]
ld r4, [c045]
mov r4, r4
addxy r0, r0, r4
ld r4, [c046]
mov r4, r4
addxy r0, r0, r4
add r0, #-4.75
ld r1, [0353]
mov r4, r4
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
st r1, [0352]
ld r4, [a041]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
st r7, [a156]
ld r4, [a03d]
mov r5, r5
st r4, [a165]
ld r4, [a065]
mov r5, r5
st r4, [a16d]
; voice 6: DCO1
mov r7, #0.0
ld r0, [a036]
mov r4, r4
add r0, #-4.75
ld r4, [c044]
mov r4, r4
addxy r0, r0, r4
ld r1, [0361]
mov r4, r4
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
st r1, [0360]
ld r4, [a040]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
; voice 6: DCO2
ld r0, [a036]
ld r4, [c045]
mov r4, r4
addxy r0, r0, r4
ld r4, [c046]
mov r4, r4
addxy r0, r0, r4
add r0, #-4.75
ld r1, [0363]
mov r4, r4
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
st r1, [0362]
ld r4, [a041]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
st r7, [a157]
ld r4, [a03e]
mov r5, r5
st r4, [a166]
ld r4, [a066]
mov r5, r5
st r4, [a16e]
; voice 7: DCO1
mov r7, #0.0
ld r0, [a037]
mov r4, r4
add r0, #-4.75
ld r4, [c044]
mov r4, r4
addxy r0, r0, r4
ld r1, [0371]
mov r4, r4
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
st r1, [0370]
ld r4, [a040]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
; voice 7: DCO2
ld r0, [a037]
ld r4, [c045]
mov r4, r4
addxy r0, r0, r4
ld r4, [c046]
mov r4, r4
addxy r0, r0, r4
add r0, #-4.75
ld r1, [0373]
mov r4, r4
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
st r1, [0372]
ld r4, [a041]
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1
st r7, [a158]
ld r4, [a03f]
mov r5, r5
st r4, [a167]
ld r4, [a067]
mov r5, r5
st r4, [a16f]
