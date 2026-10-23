; s1m1 (STEP 5): slave DSP B main @0x0100: sum of A151+v x amp ADSR -> x C000 -> 6006 / 6007
mov r7, #0.0
mov r5, #0.0
mov r6, #1.0
; voice 0: amp ADSR
ld r1, [0401]
ld r2, [0403]
ld r4, [a160]
ld r3, [a168]
ld r0, [0405]
mov r5, r5
st r3, [0404]
subxy r0, r0, r3
mul r0, r0, r0
subxy r0, r0, #1.0
mul r2, r2, r0
addxy r3, r1, #-0.999
mul r3, r3, #256.0
mul r3, r3, #256.0
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
st r2, [0402]
subxy r3, r2, r4
subxy r4, r4, #1.0
ld r0, [c043]
mov r5, r5
mul r0, r0, r4
ld r4, [c041]
mov r5, r5
mad r0, r3, r4
ld r4, [c042]
mov r5, r5
mad r0, r2, r4
ld r4, [c040]
mov r5, r5
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0400]
ld r0, [a151]
mov r5, r5
mul r0, r0, r1
addxy r7, r7, r0
; voice 1: amp ADSR
ld r1, [0411]
ld r2, [0413]
ld r4, [a161]
ld r3, [a169]
ld r0, [0415]
mov r5, r5
st r3, [0414]
subxy r0, r0, r3
mul r0, r0, r0
subxy r0, r0, #1.0
mul r2, r2, r0
addxy r3, r1, #-0.999
mul r3, r3, #256.0
mul r3, r3, #256.0
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
st r2, [0412]
subxy r3, r2, r4
subxy r4, r4, #1.0
ld r0, [c043]
mov r5, r5
mul r0, r0, r4
ld r4, [c041]
mov r5, r5
mad r0, r3, r4
ld r4, [c042]
mov r5, r5
mad r0, r2, r4
ld r4, [c040]
mov r5, r5
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0410]
ld r0, [a152]
mov r5, r5
mul r0, r0, r1
addxy r7, r7, r0
; voice 2: amp ADSR
ld r1, [0421]
ld r2, [0423]
ld r4, [a162]
ld r3, [a16a]
ld r0, [0425]
mov r5, r5
st r3, [0424]
subxy r0, r0, r3
mul r0, r0, r0
subxy r0, r0, #1.0
mul r2, r2, r0
addxy r3, r1, #-0.999
mul r3, r3, #256.0
mul r3, r3, #256.0
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
st r2, [0422]
subxy r3, r2, r4
subxy r4, r4, #1.0
ld r0, [c043]
mov r5, r5
mul r0, r0, r4
ld r4, [c041]
mov r5, r5
mad r0, r3, r4
ld r4, [c042]
mov r5, r5
mad r0, r2, r4
ld r4, [c040]
mov r5, r5
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0420]
ld r0, [a153]
mov r5, r5
mul r0, r0, r1
addxy r7, r7, r0
; voice 3: amp ADSR
ld r1, [0431]
ld r2, [0433]
ld r4, [a163]
ld r3, [a16b]
ld r0, [0435]
mov r5, r5
st r3, [0434]
subxy r0, r0, r3
mul r0, r0, r0
subxy r0, r0, #1.0
mul r2, r2, r0
addxy r3, r1, #-0.999
mul r3, r3, #256.0
mul r3, r3, #256.0
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
st r2, [0432]
subxy r3, r2, r4
subxy r4, r4, #1.0
ld r0, [c043]
mov r5, r5
mul r0, r0, r4
ld r4, [c041]
mov r5, r5
mad r0, r3, r4
ld r4, [c042]
mov r5, r5
mad r0, r2, r4
ld r4, [c040]
mov r5, r5
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0430]
ld r0, [a154]
mov r5, r5
mul r0, r0, r1
addxy r7, r7, r0
; voice 4: amp ADSR
ld r1, [0441]
ld r2, [0443]
ld r4, [a164]
ld r3, [a16c]
ld r0, [0445]
mov r5, r5
st r3, [0444]
subxy r0, r0, r3
mul r0, r0, r0
subxy r0, r0, #1.0
mul r2, r2, r0
addxy r3, r1, #-0.999
mul r3, r3, #256.0
mul r3, r3, #256.0
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
st r2, [0442]
subxy r3, r2, r4
subxy r4, r4, #1.0
ld r0, [c043]
mov r5, r5
mul r0, r0, r4
ld r4, [c041]
mov r5, r5
mad r0, r3, r4
ld r4, [c042]
mov r5, r5
mad r0, r2, r4
ld r4, [c040]
mov r5, r5
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0440]
ld r0, [a155]
mov r5, r5
mul r0, r0, r1
addxy r7, r7, r0
; voice 5: amp ADSR
ld r1, [0451]
ld r2, [0453]
ld r4, [a165]
ld r3, [a16d]
ld r0, [0455]
mov r5, r5
st r3, [0454]
subxy r0, r0, r3
mul r0, r0, r0
subxy r0, r0, #1.0
mul r2, r2, r0
addxy r3, r1, #-0.999
mul r3, r3, #256.0
mul r3, r3, #256.0
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
st r2, [0452]
subxy r3, r2, r4
subxy r4, r4, #1.0
ld r0, [c043]
mov r5, r5
mul r0, r0, r4
ld r4, [c041]
mov r5, r5
mad r0, r3, r4
ld r4, [c042]
mov r5, r5
mad r0, r2, r4
ld r4, [c040]
mov r5, r5
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0450]
ld r0, [a156]
mov r5, r5
mul r0, r0, r1
addxy r7, r7, r0
; voice 6: amp ADSR
ld r1, [0461]
ld r2, [0463]
ld r4, [a166]
ld r3, [a16e]
ld r0, [0465]
mov r5, r5
st r3, [0464]
subxy r0, r0, r3
mul r0, r0, r0
subxy r0, r0, #1.0
mul r2, r2, r0
addxy r3, r1, #-0.999
mul r3, r3, #256.0
mul r3, r3, #256.0
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
st r2, [0462]
subxy r3, r2, r4
subxy r4, r4, #1.0
ld r0, [c043]
mov r5, r5
mul r0, r0, r4
ld r4, [c041]
mov r5, r5
mad r0, r3, r4
ld r4, [c042]
mov r5, r5
mad r0, r2, r4
ld r4, [c040]
mov r5, r5
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0460]
ld r0, [a157]
mov r5, r5
mul r0, r0, r1
addxy r7, r7, r0
; voice 7: amp ADSR
ld r1, [0471]
ld r2, [0473]
ld r4, [a167]
ld r3, [a16f]
ld r0, [0475]
mov r5, r5
st r3, [0474]
subxy r0, r0, r3
mul r0, r0, r0
subxy r0, r0, #1.0
mul r2, r2, r0
addxy r3, r1, #-0.999
mul r3, r3, #256.0
mul r3, r3, #256.0
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
st r2, [0472]
subxy r3, r2, r4
subxy r4, r4, #1.0
ld r0, [c043]
mov r5, r5
mul r0, r0, r4
ld r4, [c041]
mov r5, r5
mad r0, r3, r4
ld r4, [c042]
mov r5, r5
mad r0, r2, r4
ld r4, [c040]
mov r5, r5
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0470]
ld r0, [a158]
mov r5, r5
mul r0, r0, r1
addxy r7, r7, r0
ld r6, [c000]
mul r7, r7, #0.08333333333333333
mul r7, r7, r6
st r7, [6006]
st r7, [6007]
mov r0, #0.0
st r0, [6008]
st r0, [6009]
st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d]
