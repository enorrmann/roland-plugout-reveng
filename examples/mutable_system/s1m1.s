mov r7, #0.0 | mov r5, #0.0 | ld r0, [040d] | ld r1, [042d] | ld r2, [c060]
addxy r7, r7, r0 | ld r0, [044d] | ld r6, [c000]
addxy r7, r7, r1 | ld r1, [046d]
addxy r7, r7, r0 | ld r0, [048d]
addxy r7, r7, r1 | ld r1, [04ad]
addxy r7, r7, r0 | ld r0, [04cd]
addxy r7, r7, r1 | ld r1, [04ed]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | ld r1, [a188] | st r0, [6008]
mul r7, r7, #0.16666666666666666 | mul r3, r1, r2 | st r0, [6009]
mul r7, r7, r6 | mov r6, #1.0 | st r0, [600a]
min r4, r3, #0.39269908169872414 | st r7, [6006]
max r7, r4, #0.000267552393860559 | st r7, [6007]
mul r1, r7, r7 | st r0, [600b]
mul r2, r1, #-0.3333231608072917 | ld r1, [0419] | st r0, [600c]
addxy r3, r2, #1.99993896484375 | ld r2, [0417] | st r0, [600d] | ld r0, [a151]
mul r4, r3, r7
mad r2, r4, r1
mad r0, r1, #-1.99993896484375
ph_clamp r3, r3, -r2
subxy r7, r3, r0 | st r3, [0416] | ld r0, [0405]
mad r1, r4, r7 | ld r4, [a160]
ph_clamp r2, r3, -r7 | st r1, [0418] | ld r3, [a168] | ld r1, [0401]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [0404] | st r2, [041a] | ld r2, [0403]
mul r3, r3, #256.0 | mul r0, r0, r0 | ld r7, [041a]
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
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0406] | ld r1, [a189]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0408] | ld r2, [c060]
mad r3, r5, r0
subxy r5, r4, r3 | mul r3, r1, r2 | st r3, [040a]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [a152]
min r4, r3, #0.39269908169872414 | st r4, [040c]
max r7, r4, #0.000267552393860559
mul r1, r7, r7
mul r2, r1, #-0.3333231608072917 | ld r1, [0439]
addxy r3, r2, #1.99993896484375 | ld r2, [0437]
mul r4, r3, r7
mad r2, r4, r1
mad r0, r1, #-1.99993896484375
ph_clamp r3, r3, -r2
subxy r7, r3, r0 | st r3, [0436] | ld r0, [0425]
mad r1, r4, r7 | ld r4, [a161]
ph_clamp r2, r3, -r7 | st r1, [0438] | ld r3, [a169] | ld r1, [0421]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [0424] | st r2, [043a] | ld r2, [0423]
mul r3, r3, #256.0 | mul r0, r0, r0 | ld r7, [043a]
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
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0426] | ld r1, [a18a]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0428] | ld r2, [c060]
mad r3, r5, r0
subxy r5, r4, r3 | mul r3, r1, r2 | st r3, [042a]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [a153]
min r4, r3, #0.39269908169872414 | st r4, [042c]
max r7, r4, #0.000267552393860559
mul r1, r7, r7
mul r2, r1, #-0.3333231608072917 | ld r1, [0459]
addxy r3, r2, #1.99993896484375 | ld r2, [0457]
mul r4, r3, r7
mad r2, r4, r1
mad r0, r1, #-1.99993896484375
ph_clamp r3, r3, -r2
subxy r7, r3, r0 | st r3, [0456] | ld r0, [0445]
mad r1, r4, r7 | ld r4, [a162]
ph_clamp r2, r3, -r7 | st r1, [0458] | ld r3, [a16a] | ld r1, [0441]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [0444] | st r2, [045a] | ld r2, [0443]
mul r3, r3, #256.0 | mul r0, r0, r0 | ld r7, [045a]
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
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0446] | ld r1, [a18b]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0448] | ld r2, [c060]
mad r3, r5, r0
subxy r5, r4, r3 | mul r3, r1, r2 | st r3, [044a]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [a154]
min r4, r3, #0.39269908169872414 | st r4, [044c]
max r7, r4, #0.000267552393860559
mul r1, r7, r7
mul r2, r1, #-0.3333231608072917 | ld r1, [0479]
addxy r3, r2, #1.99993896484375 | ld r2, [0477]
mul r4, r3, r7
mad r2, r4, r1
mad r0, r1, #-1.99993896484375
ph_clamp r3, r3, -r2
subxy r7, r3, r0 | st r3, [0476] | ld r0, [0465]
mad r1, r4, r7 | ld r4, [a163]
ph_clamp r2, r3, -r7 | st r1, [0478] | ld r3, [a16b] | ld r1, [0461]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [0464] | st r2, [047a] | ld r2, [0463]
mul r3, r3, #256.0 | mul r0, r0, r0 | ld r7, [047a]
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
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0466] | ld r1, [a18c]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0468] | ld r2, [c060]
mad r3, r5, r0
subxy r5, r4, r3 | mul r3, r1, r2 | st r3, [046a]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [a155]
min r4, r3, #0.39269908169872414 | st r4, [046c]
max r7, r4, #0.000267552393860559
mul r1, r7, r7
mul r2, r1, #-0.3333231608072917 | ld r1, [0499]
addxy r3, r2, #1.99993896484375 | ld r2, [0497]
mul r4, r3, r7
mad r2, r4, r1
mad r0, r1, #-1.99993896484375
ph_clamp r3, r3, -r2
subxy r7, r3, r0 | st r3, [0496] | ld r0, [0485]
mad r1, r4, r7 | ld r4, [a164]
ph_clamp r2, r3, -r7 | st r1, [0498] | ld r3, [a16c] | ld r1, [0481]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [0484] | st r2, [049a] | ld r2, [0483]
mul r3, r3, #256.0 | mul r0, r0, r0 | ld r7, [049a]
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
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0486] | ld r1, [a18d]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0488] | ld r2, [c060]
mad r3, r5, r0
subxy r5, r4, r3 | mul r3, r1, r2 | st r3, [048a]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [a156]
min r4, r3, #0.39269908169872414 | st r4, [048c]
max r7, r4, #0.000267552393860559
mul r1, r7, r7
mul r2, r1, #-0.3333231608072917 | ld r1, [04b9]
addxy r3, r2, #1.99993896484375 | ld r2, [04b7]
mul r4, r3, r7
mad r2, r4, r1
mad r0, r1, #-1.99993896484375
ph_clamp r3, r3, -r2
subxy r7, r3, r0 | st r3, [04b6] | ld r0, [04a5]
mad r1, r4, r7 | ld r4, [a165]
ph_clamp r2, r3, -r7 | st r1, [04b8] | ld r3, [a16d] | ld r1, [04a1]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [04a4] | st r2, [04ba] | ld r2, [04a3]
mul r3, r3, #256.0 | mul r0, r0, r0 | ld r7, [04ba]
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
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [04a6] | ld r1, [a18e]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [04a8] | ld r2, [c060]
mad r3, r5, r0
subxy r5, r4, r3 | mul r3, r1, r2 | st r3, [04aa]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [a157]
min r4, r3, #0.39269908169872414 | st r4, [04ac]
max r7, r4, #0.000267552393860559
mul r1, r7, r7
mul r2, r1, #-0.3333231608072917 | ld r1, [04d9]
addxy r3, r2, #1.99993896484375 | ld r2, [04d7]
mul r4, r3, r7
mad r2, r4, r1
mad r0, r1, #-1.99993896484375
ph_clamp r3, r3, -r2
subxy r7, r3, r0 | st r3, [04d6] | ld r0, [04c5]
mad r1, r4, r7 | ld r4, [a166]
ph_clamp r2, r3, -r7 | st r1, [04d8] | ld r3, [a16e] | ld r1, [04c1]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [04c4] | st r2, [04da] | ld r2, [04c3]
mul r3, r3, #256.0 | mul r0, r0, r0 | ld r7, [04da]
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
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [04c6] | ld r1, [a18f]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [04c8] | ld r2, [c060]
mad r3, r5, r0
subxy r5, r4, r3 | mul r3, r1, r2 | st r3, [04ca]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [a158]
min r4, r3, #0.39269908169872414 | st r4, [04cc]
max r7, r4, #0.000267552393860559
mul r1, r7, r7
mul r2, r1, #-0.3333231608072917 | ld r1, [04f9]
addxy r3, r2, #1.99993896484375 | ld r2, [04f7]
mul r4, r3, r7
mad r2, r4, r1
mad r0, r1, #-1.99993896484375
ph_clamp r3, r3, -r2
subxy r7, r3, r0 | st r3, [04f6] | ld r0, [04e5]
mad r1, r4, r7 | ld r4, [a167]
ph_clamp r2, r3, -r7 | st r1, [04f8] | ld r3, [a16f] | ld r1, [04e1]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [04e4] | st r2, [04fa] | ld r2, [04e3]
mul r3, r3, #256.0 | mul r0, r0, r0 | ld r7, [04fa]
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
