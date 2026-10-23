mov r7, #0.0 | mov r5, #0.0 | ld r0, [040d] | ld r1, [042d] | ld r3, [a168]
addxy r7, r7, r0 | ld r0, [044d] | st r3, [0404] | ld r6, [c000]
addxy r7, r7, r1 | ld r1, [046d] | ld r2, [0403] | ld r4, [a160]
addxy r7, r7, r0 | ld r0, [048d]
addxy r7, r7, r1 | ld r1, [04ad]
addxy r7, r7, r0 | ld r0, [04cd]
addxy r7, r7, r1 | ld r1, [04ed]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | st r0, [6008] | ld r1, [0401]
mul r7, r7, #0.16666666666666666 | st r0, [6009]
mul r7, r7, r6 | mov r6, #1.0 | st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [0405]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r7, [6006]
mul r3, r3, #256.0 | mul r0, r0, r0 | st r7, [6007] | ld r7, [a151]
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0402]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0413]
mad r4, r3, #1.25 | ld r3, [a168]
subxy r4, r1, r4 | st r3, [0414]
mad r1, r4, r0 | ld r0, [0415] | ld r4, [a160]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [0400] | ld r1, [0411]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0412]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a178]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a180] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [0410] | ld r1, [0407]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [040d]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [0409]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [040b]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7 | ld r7, [a152]
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0406] | ld r1, [0421]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0408] | ld r2, [0423]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [040a] | ld r3, [a169]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [0425] | st r3, [0424]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r4, [040c] | ld r4, [a161]
mul r3, r3, #256.0 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0422]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0433]
mad r4, r3, #1.25 | ld r3, [a169]
subxy r4, r1, r4 | st r3, [0434]
mad r1, r4, r0 | ld r0, [0435] | ld r4, [a161]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [0420] | ld r1, [0431]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0432]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a179]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a181] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [0430] | ld r1, [0427]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [042d]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [0429]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [042b]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7 | ld r7, [a153]
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0426] | ld r1, [0441]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0428] | ld r2, [0443]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [042a] | ld r3, [a16a]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [0445] | st r3, [0444]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r4, [042c] | ld r4, [a162]
mul r3, r3, #256.0 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0442]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0453]
mad r4, r3, #1.25 | ld r3, [a16a]
subxy r4, r1, r4 | st r3, [0454]
mad r1, r4, r0 | ld r0, [0455] | ld r4, [a162]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [0440] | ld r1, [0451]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0452]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a17a]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a182] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [0450] | ld r1, [0447]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [044d]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [0449]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [044b]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7 | ld r7, [a154]
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0446] | ld r1, [0461]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0448] | ld r2, [0463]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [044a] | ld r3, [a16b]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [0465] | st r3, [0464]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r4, [044c] | ld r4, [a163]
mul r3, r3, #256.0 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0462]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0473]
mad r4, r3, #1.25 | ld r3, [a16b]
subxy r4, r1, r4 | st r3, [0474]
mad r1, r4, r0 | ld r0, [0475] | ld r4, [a163]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [0460] | ld r1, [0471]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0472]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a17b]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a183] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [0470] | ld r1, [0467]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [046d]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [0469]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [046b]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7 | ld r7, [a155]
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0466] | ld r1, [0481]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0468] | ld r2, [0483]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [046a] | ld r3, [a16c]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [0485] | st r3, [0484]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r4, [046c] | ld r4, [a164]
mul r3, r3, #256.0 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0482]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0493]
mad r4, r3, #1.25 | ld r3, [a16c]
subxy r4, r1, r4 | st r3, [0494]
mad r1, r4, r0 | ld r0, [0495] | ld r4, [a164]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [0480] | ld r1, [0491]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0492]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a17c]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a184] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [0490] | ld r1, [0487]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [048d]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [0489]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [048b]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7 | ld r7, [a156]
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0486] | ld r1, [04a1]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0488] | ld r2, [04a3]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [048a] | ld r3, [a16d]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [04a5] | st r3, [04a4]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r4, [048c] | ld r4, [a165]
mul r3, r3, #256.0 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [04a2]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [04b3]
mad r4, r3, #1.25 | ld r3, [a16d]
subxy r4, r1, r4 | st r3, [04b4]
mad r1, r4, r0 | ld r0, [04b5] | ld r4, [a165]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [04a0] | ld r1, [04b1]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [04b2]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a17d]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a185] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [04b0] | ld r1, [04a7]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [04ad]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [04a9]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [04ab]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7 | ld r7, [a157]
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [04a6] | ld r1, [04c1]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [04a8] | ld r2, [04c3]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [04aa] | ld r3, [a16e]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [04c5] | st r3, [04c4]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r4, [04ac] | ld r4, [a166]
mul r3, r3, #256.0 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [04c2]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [04d3]
mad r4, r3, #1.25 | ld r3, [a16e]
subxy r4, r1, r4 | st r3, [04d4]
mad r1, r4, r0 | ld r0, [04d5] | ld r4, [a166]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [04c0] | ld r1, [04d1]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [04d2]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a17e]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a186] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [04d0] | ld r1, [04c7]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [04cd]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [04c9]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [04cb]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7 | ld r7, [a158]
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [04c6] | ld r1, [04e1]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [04c8] | ld r2, [04e3]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [04ca] | ld r3, [a16f]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [04e5] | st r3, [04e4]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r4, [04cc] | ld r4, [a167]
mul r3, r3, #256.0 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [04e2]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [04f3]
mad r4, r3, #1.25 | ld r3, [a16f]
subxy r4, r1, r4 | st r3, [04f4]
mad r1, r4, r0 | ld r0, [04f5] | ld r4, [a167]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [04e0] | ld r1, [04f1]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [04f2]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a17f]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a187] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [04f0] | ld r1, [04e7]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [04ed]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [04e9]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [04eb]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [04e6]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [04e8]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [04ea]
mad r4, r5, r0 | mov r5, #0.0
st r4, [04ec]
