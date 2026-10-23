mov r7, #0.0 | ld r0, [040d]
addxy r7, r7, r0 | ld r1, [042d]
addxy r7, r7, r1 | ld r0, [044d]
addxy r7, r7, r0 | ld r1, [046d]
addxy r7, r7, r1 | ld r0, [048d]
addxy r7, r7, r0 | ld r1, [04ad]
addxy r7, r7, r1 | ld r0, [04cd]
addxy r7, r7, r0 | ld r1, [04ed]
addxy r7, r7, r1 | ld r6, [c000]
mul r7, r7, #0.16666666666666666
mul r7, r7, r6
st r7, [6006]
mov r0, #0.0 | st r7, [6007]
st r0, [6008]
st r0, [6009]
st r0, [600a]
st r0, [600b]
st r0, [600c]
mov r5, #0.0 | st r0, [600d]
mov r6, #1.0 | ld r1, [0401] | ld r2, [0403] | ld r4, [a160]
ld r3, [a168] | ld r0, [0405]
subxy r0, r0, r3 | st r3, [0404]
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
subxy r3, r2, r4 | st r2, [0402]
subxy r4, r4, #1.0 | ld r0, [c043]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0400] | ld r7, [a151]
mul r7, r7, r1 | ld r1, [0411] | ld r2, [0413] | ld r4, [a160]
ld r3, [a168] | ld r0, [0415]
subxy r0, r0, r3 | st r3, [0414]
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
subxy r3, r2, r4 | st r2, [0412]
subxy r4, r4, #1.0 | ld r0, [c053]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0410] | ld r6, [c04a] | ld r0, [a180]
ld r2, [c04c] | ld r3, [a178]
mul r7, r7, r6 | ld r4, [c04b]
mad r0, r2, r1
add r3, #-4.75
mad r0, r4, r3
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3
mul r0, r0, #0.02879793265790644
min r0, r0, #0.7 | ld r1, [0407] | ld r2, [0409]
ld r3, [040b] | ld r4, [040d] | ld r6, [c049]
mad r7, r6, -r4
ph_clamp r7, r3, -r7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0406]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0408]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [040a]
mad r4, r5, r0
mov r5, #0.0 | st r4, [040c]
mov r6, #1.0 | ld r1, [0421] | ld r2, [0423] | ld r4, [a161]
ld r3, [a169] | ld r0, [0425]
subxy r0, r0, r3 | st r3, [0424]
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
subxy r3, r2, r4 | st r2, [0422]
subxy r4, r4, #1.0 | ld r0, [c043]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0420] | ld r7, [a152]
mul r7, r7, r1 | ld r1, [0431] | ld r2, [0433] | ld r4, [a161]
ld r3, [a169] | ld r0, [0435]
subxy r0, r0, r3 | st r3, [0434]
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
subxy r3, r2, r4 | st r2, [0432]
subxy r4, r4, #1.0 | ld r0, [c053]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0430] | ld r6, [c04a] | ld r0, [a181]
ld r2, [c04c] | ld r3, [a179]
mul r7, r7, r6 | ld r4, [c04b]
mad r0, r2, r1
add r3, #-4.75
mad r0, r4, r3
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3
mul r0, r0, #0.02879793265790644
min r0, r0, #0.7 | ld r1, [0427] | ld r2, [0429]
ld r3, [042b] | ld r4, [042d] | ld r6, [c049]
mad r7, r6, -r4
ph_clamp r7, r3, -r7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0426]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0428]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [042a]
mad r4, r5, r0
mov r5, #0.0 | st r4, [042c]
mov r6, #1.0 | ld r1, [0441] | ld r2, [0443] | ld r4, [a162]
ld r3, [a16a] | ld r0, [0445]
subxy r0, r0, r3 | st r3, [0444]
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
subxy r3, r2, r4 | st r2, [0442]
subxy r4, r4, #1.0 | ld r0, [c043]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0440] | ld r7, [a153]
mul r7, r7, r1 | ld r1, [0451] | ld r2, [0453] | ld r4, [a162]
ld r3, [a16a] | ld r0, [0455]
subxy r0, r0, r3 | st r3, [0454]
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
subxy r3, r2, r4 | st r2, [0452]
subxy r4, r4, #1.0 | ld r0, [c053]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0450] | ld r6, [c04a] | ld r0, [a182]
ld r2, [c04c] | ld r3, [a17a]
mul r7, r7, r6 | ld r4, [c04b]
mad r0, r2, r1
add r3, #-4.75
mad r0, r4, r3
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3
mul r0, r0, #0.02879793265790644
min r0, r0, #0.7 | ld r1, [0447] | ld r2, [0449]
ld r3, [044b] | ld r4, [044d] | ld r6, [c049]
mad r7, r6, -r4
ph_clamp r7, r3, -r7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0446]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0448]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [044a]
mad r4, r5, r0
mov r5, #0.0 | st r4, [044c]
mov r6, #1.0 | ld r1, [0461] | ld r2, [0463] | ld r4, [a163]
ld r3, [a16b] | ld r0, [0465]
subxy r0, r0, r3 | st r3, [0464]
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
subxy r3, r2, r4 | st r2, [0462]
subxy r4, r4, #1.0 | ld r0, [c043]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0460] | ld r7, [a154]
mul r7, r7, r1 | ld r1, [0471] | ld r2, [0473] | ld r4, [a163]
ld r3, [a16b] | ld r0, [0475]
subxy r0, r0, r3 | st r3, [0474]
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
subxy r3, r2, r4 | st r2, [0472]
subxy r4, r4, #1.0 | ld r0, [c053]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0470] | ld r6, [c04a] | ld r0, [a183]
ld r2, [c04c] | ld r3, [a17b]
mul r7, r7, r6 | ld r4, [c04b]
mad r0, r2, r1
add r3, #-4.75
mad r0, r4, r3
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3
mul r0, r0, #0.02879793265790644
min r0, r0, #0.7 | ld r1, [0467] | ld r2, [0469]
ld r3, [046b] | ld r4, [046d] | ld r6, [c049]
mad r7, r6, -r4
ph_clamp r7, r3, -r7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0466]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0468]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [046a]
mad r4, r5, r0
mov r5, #0.0 | st r4, [046c]
mov r6, #1.0 | ld r1, [0481] | ld r2, [0483] | ld r4, [a164]
ld r3, [a16c] | ld r0, [0485]
subxy r0, r0, r3 | st r3, [0484]
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
subxy r3, r2, r4 | st r2, [0482]
subxy r4, r4, #1.0 | ld r0, [c043]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0480] | ld r7, [a155]
mul r7, r7, r1 | ld r1, [0491] | ld r2, [0493] | ld r4, [a164]
ld r3, [a16c] | ld r0, [0495]
subxy r0, r0, r3 | st r3, [0494]
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
subxy r3, r2, r4 | st r2, [0492]
subxy r4, r4, #1.0 | ld r0, [c053]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [0490] | ld r6, [c04a] | ld r0, [a184]
ld r2, [c04c] | ld r3, [a17c]
mul r7, r7, r6 | ld r4, [c04b]
mad r0, r2, r1
add r3, #-4.75
mad r0, r4, r3
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3
mul r0, r0, #0.02879793265790644
min r0, r0, #0.7 | ld r1, [0487] | ld r2, [0489]
ld r3, [048b] | ld r4, [048d] | ld r6, [c049]
mad r7, r6, -r4
ph_clamp r7, r3, -r7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0486]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0488]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [048a]
mad r4, r5, r0
mov r5, #0.0 | st r4, [048c]
mov r6, #1.0 | ld r1, [04a1] | ld r2, [04a3] | ld r4, [a165]
ld r3, [a16d] | ld r0, [04a5]
subxy r0, r0, r3 | st r3, [04a4]
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
subxy r3, r2, r4 | st r2, [04a2]
subxy r4, r4, #1.0 | ld r0, [c043]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [04a0] | ld r7, [a156]
mul r7, r7, r1 | ld r1, [04b1] | ld r2, [04b3] | ld r4, [a165]
ld r3, [a16d] | ld r0, [04b5]
subxy r0, r0, r3 | st r3, [04b4]
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
subxy r3, r2, r4 | st r2, [04b2]
subxy r4, r4, #1.0 | ld r0, [c053]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [04b0] | ld r6, [c04a] | ld r0, [a185]
ld r2, [c04c] | ld r3, [a17d]
mul r7, r7, r6 | ld r4, [c04b]
mad r0, r2, r1
add r3, #-4.75
mad r0, r4, r3
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3
mul r0, r0, #0.02879793265790644
min r0, r0, #0.7 | ld r1, [04a7] | ld r2, [04a9]
ld r3, [04ab] | ld r4, [04ad] | ld r6, [c049]
mad r7, r6, -r4
ph_clamp r7, r3, -r7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [04a6]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [04a8]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [04aa]
mad r4, r5, r0
mov r5, #0.0 | st r4, [04ac]
mov r6, #1.0 | ld r1, [04c1] | ld r2, [04c3] | ld r4, [a166]
ld r3, [a16e] | ld r0, [04c5]
subxy r0, r0, r3 | st r3, [04c4]
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
subxy r3, r2, r4 | st r2, [04c2]
subxy r4, r4, #1.0 | ld r0, [c043]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [04c0] | ld r7, [a157]
mul r7, r7, r1 | ld r1, [04d1] | ld r2, [04d3] | ld r4, [a166]
ld r3, [a16e] | ld r0, [04d5]
subxy r0, r0, r3 | st r3, [04d4]
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
subxy r3, r2, r4 | st r2, [04d2]
subxy r4, r4, #1.0 | ld r0, [c053]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [04d0] | ld r6, [c04a] | ld r0, [a186]
ld r2, [c04c] | ld r3, [a17e]
mul r7, r7, r6 | ld r4, [c04b]
mad r0, r2, r1
add r3, #-4.75
mad r0, r4, r3
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3
mul r0, r0, #0.02879793265790644
min r0, r0, #0.7 | ld r1, [04c7] | ld r2, [04c9]
ld r3, [04cb] | ld r4, [04cd] | ld r6, [c049]
mad r7, r6, -r4
ph_clamp r7, r3, -r7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [04c6]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [04c8]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [04ca]
mad r4, r5, r0
mov r5, #0.0 | st r4, [04cc]
mov r6, #1.0 | ld r1, [04e1] | ld r2, [04e3] | ld r4, [a167]
ld r3, [a16f] | ld r0, [04e5]
subxy r0, r0, r3 | st r3, [04e4]
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
subxy r3, r2, r4 | st r2, [04e2]
subxy r4, r4, #1.0 | ld r0, [c043]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [04e0] | ld r7, [a158]
mul r7, r7, r1 | ld r1, [04f1] | ld r2, [04f3] | ld r4, [a167]
ld r3, [a16f] | ld r0, [04f5]
subxy r0, r0, r3 | st r3, [04f4]
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
subxy r3, r2, r4 | st r2, [04f2]
subxy r4, r4, #1.0 | ld r0, [c053]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0
min r1, r1, r6
st r1, [04f0] | ld r6, [c04a] | ld r0, [a187]
ld r2, [c04c] | ld r3, [a17f]
mul r7, r7, r6 | ld r4, [c04b]
mad r0, r2, r1
add r3, #-4.75
mad r0, r4, r3
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3
mul r0, r0, #0.02879793265790644
min r0, r0, #0.7 | ld r1, [04e7] | ld r2, [04e9]
ld r3, [04eb] | ld r4, [04ed] | ld r6, [c049]
mad r7, r6, -r4
ph_clamp r7, r3, -r7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [04e6]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [04e8]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [04ea]
mad r4, r5, r0
mov r5, #0.0 | st r4, [04ec]
mov r6, #1.0
