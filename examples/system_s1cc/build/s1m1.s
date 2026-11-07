mov r7, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r4, [a169]
addxy r7, r7, r0 | ld r0, [0405] | ld r2, [0761] | ld r6, [c000]
addxy r7, r7, r1 | mul r2, r2, #7.31275 | ld r1, [0407] | ld r5, [0639] | ld r3, [c090]
addxy r7, r7, r0 | subxy r5, r5, r4 | ld r0, [0409] | st r4, [0638] | ld r4, [c099]
addxy r7, r7, r1 | mul r5, r5, r5 | ld r1, [040b] | st r3, [a120]
addxy r7, r7, r0 | ld r0, [040d] | st r5, [0646] | ld r5, [c050]
addxy r7, r7, r1 | ld r1, [040f]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | st r0, [6008] | ld r1, [060d]
mul r7, r7, #0.16666666666666666 | st r0, [6009]
mul r7, r7, r6 | st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [0763]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [0762] | ld r0, [a168] | ld r7, [c091]
subxy r1, r1, r0 | mul r5, r7, r5 | st r0, [060c] | ld r0, [c098]
mul r1, r1, r1 | ld r6, [c051] | st r7, [a121]
ph_wrap r2, r3, r2 | st r1, [061a] | ld r1, [c050]
mul r1, r3, r1 | mad r5, r4, -r6 | st r2, [0760] | ld r2, [c051] | ld r6, [064b]
mad r1, r0, -r2 | add r5, #1.0 | ld r2, [061f] | ld r0, [061d]
add r1, #1.0 | st r5, [0642] | ld r5, [c04f] | ld r4, [0649]
mul r5, r7, r5 | st r1, [0616] | ld r1, [c04f]
mul r1, r3, r1 | st r5, [0644] | ld r5, [0646] | ld r3, [c053]
mad r6, r6, -r5 | addxy r5, r4, #-0.999 | st r1, [0618] | ld r1, [061a] | ld r7, [c053]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [061e] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [064a]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7
mul r2, r2, #1.25 | mul r6, r6, #1.25
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r2, [0616] | ld r6, [0642]
min r0, r0, #1.0
min r4, r4, #1.0 | st r0, [061c] | st r0, [a128]
st r4, [0648] | st r4, [a129]
ld r0, [c080]
ld r4, [c081] | st r0, [0600]
st r4, [062c] | ld r0, [c088] | ld r1, [0600]
mul r3, r1, r2 | ld r4, [c089] | st r0, [0606] | ld r0, [a151]
st r4, [0632] | ld r4, [a152] | ld r2, [c061]
ld r5, [062c] | st r0, [062a] | st r4, [0656]
mul r7, r5, r6 | ld r0, [a1b0] | st r3, [0628] | ld r3, [c062]
addxy r0, r0, #-4.75 | ld r4, [a1b1] | st r7, [0654] | ld r7, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | ld r1, [0606] | ld r5, [0632] | ld r6, [c061]
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [0618] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [0644] | ld r6, [c060]
mad r0, r1, #0.125 | mad r4, r5, #0.125
mul r3, r0, #0.17328679513998632 | mul r7, r4, #0.17328679513998632
mov r1, #0.041666666666666664 | mov r5, #0.041666666666666664
ymacd r1, r3, #0.16666666666666666 | ymacd r5, r7, #0.16666666666666666
ymacd r1, r3, #0.5 | ymacd r5, r7, #0.5
ymacd r1, r3, #1.0 | ymacd r5, r7, #1.0
ymacd r1, r3, #1.0 | ymacd r5, r7, #1.0
mul r0, r1, r1 | mul r4, r5, r5
mul r3, r0, r0 | mul r7, r4, r4
mul r1, r3, r3 | mul r5, r7, r7
mul r0, r1, r1 | mul r4, r5, r5
mul r3, r0, r0 | mul r7, r4, r4
mul r1, r3, #6.103515625e-05 | mul r5, r7, #6.103515625e-05
mul r0, r2, r1 | mul r4, r6, r5 | ld r1, [c063]
min r3, r0, #3.0517578125e-05 | ld r5, [c063]
min r7, r4, #3.0517578125e-05 | shl r2, r3, 15 | ld r3, [0621]
shl r6, r7, 15 | ld r7, [064d]
ph_clamp r0, r3, -r2 | ld r2, [062a]
ph_clamp r4, r3, -r6 | ld r6, [0656] | st r1, [062a]
mac_p1wc3 r2, r1, r3, #-0.125, r2 | st r5, [0656]
mac_p1wc3 r6, r5, r7, #-0.125, r6
mul r2, r2, #-4.0 | mul r6, r6, #-4.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
mul r1, r2, r2 | mul r5, r6, r6
mul r1, r2, r1 | mul r5, r6, r5
mad r2, r1, #-0.3333333432674408 | mad r6, r5, #-0.3333333432674408 | ld r1, [0623] | ld r5, [064f]
subxy r2, r1, r2 | subxy r6, r5, r6
mul r2, r0, r2 | mul r6, r4, r6
addxy r2, r2, r1 | addxy r6, r6, r5 | ld r1, [0625] | ld r5, [0651]
st r2, [0622] | st r6, [064e]
subxy r2, r1, r2 | subxy r6, r5, r6
mul r2, r0, r2 | mul r6, r4, r6
addxy r2, r2, r1 | addxy r6, r6, r5 | ld r1, [0627] | ld r5, [0653]
st r2, [0624] | st r6, [0650]
subxy r2, r1, r2 | subxy r6, r5, r6
mul r2, r0, r2 | mul r6, r4, r6
addxy r2, r2, r1 | addxy r6, r6, r5
subxy r1, r3, r2 | subxy r5, r7, r6 | st r2, [0626]
mul r2, r0, r1 | mul r6, r4, r5 | st r6, [0652] | ld r1, [062a]
addxy r0, r2, r3 | addxy r4, r6, r7 | ld r5, [0656]
addxy r2, r1, #0.5 | addxy r6, r5, #0.5 | ld r1, [0628] | ld r5, [0654]
mul r3, r2, r0 | mul r7, r6, r4 | st r0, [0620]
mul r2, r1, r3 | mul r6, r5, r7 | st r4, [064c] | ld r1, [0665] | ld r3, [c092]
mul r0, r2, #4.0 | mul r4, r6, #4.0 | ld r5, [0691] | ld r7, [c093]
st r0, [0400] | st r4, [0402]
ld r0, [a16a] | ld r2, [c051]
subxy r1, r1, r0 | ld r4, [a16b] | st r0, [0664] | ld r0, [c09a]
subxy r5, r5, r4 | mul r1, r1, r1 | st r4, [0690] | ld r4, [c09b]
mul r5, r5, r5 | st r1, [0672] | ld r1, [c050]
mul r1, r3, r1 | st r5, [069e] | ld r5, [c050]
mul r5, r7, r5 | mad r1, r0, -r2 | ld r6, [c051] | ld r2, [0677] | ld r0, [0675]
mad r5, r4, -r6 | add r1, #1.0 | ld r6, [06a3] | ld r4, [06a1]
add r5, #1.0 | st r1, [066e] | ld r1, [c04f]
mul r1, r3, r1 | st r5, [069a] | ld r5, [c04f]
mul r5, r7, r5 | st r1, [0670] | ld r1, [0672] | st r3, [a122]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | st r5, [069c] | ld r5, [069e] | st r7, [a123]
mad r6, r6, -r5 | addxy r5, r4, #-0.999 | ld r3, [c053]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | ld r7, [c053]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [0676] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [06a2]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7
mul r2, r2, #1.25 | mul r6, r6, #1.25
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r2, [066e] | ld r6, [069a]
min r0, r0, #1.0
min r4, r4, #1.0 | st r0, [0674] | st r0, [a12a]
st r4, [06a0] | st r4, [a12b]
ld r0, [c082]
ld r4, [c083] | st r0, [0658]
st r4, [0684] | ld r0, [c08a] | ld r1, [0658]
mul r3, r1, r2 | ld r4, [c08b] | st r0, [065e] | ld r0, [a153]
st r4, [068a] | ld r4, [a154] | ld r2, [c061]
ld r5, [0684] | st r0, [0682] | st r4, [06ae]
mul r7, r5, r6 | ld r0, [a1b2] | st r3, [0680] | ld r3, [c062]
addxy r0, r0, #-4.75 | ld r4, [a1b3] | st r7, [06ac] | ld r7, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | ld r1, [065e] | ld r5, [068a] | ld r6, [c061]
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [0670] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [069c] | ld r6, [c060]
mad r0, r1, #0.125 | mad r4, r5, #0.125
mul r3, r0, #0.17328679513998632 | mul r7, r4, #0.17328679513998632
mov r1, #0.041666666666666664 | mov r5, #0.041666666666666664
ymacd r1, r3, #0.16666666666666666 | ymacd r5, r7, #0.16666666666666666
ymacd r1, r3, #0.5 | ymacd r5, r7, #0.5
ymacd r1, r3, #1.0 | ymacd r5, r7, #1.0
ymacd r1, r3, #1.0 | ymacd r5, r7, #1.0
mul r0, r1, r1 | mul r4, r5, r5
mul r3, r0, r0 | mul r7, r4, r4
mul r1, r3, r3 | mul r5, r7, r7
mul r0, r1, r1 | mul r4, r5, r5
mul r3, r0, r0 | mul r7, r4, r4
mul r1, r3, #6.103515625e-05 | mul r5, r7, #6.103515625e-05
mul r0, r2, r1 | mul r4, r6, r5 | ld r1, [c063]
min r3, r0, #3.0517578125e-05 | ld r5, [c063]
min r7, r4, #3.0517578125e-05 | shl r2, r3, 15 | ld r3, [0679]
shl r6, r7, 15 | ld r7, [06a5]
ph_clamp r0, r3, -r2 | ld r2, [0682]
ph_clamp r4, r3, -r6 | ld r6, [06ae] | st r1, [0682]
mac_p1wc3 r2, r1, r3, #-0.125, r2 | st r5, [06ae]
mac_p1wc3 r6, r5, r7, #-0.125, r6
mul r2, r2, #-4.0 | mul r6, r6, #-4.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
mul r1, r2, r2 | mul r5, r6, r6
mul r1, r2, r1 | mul r5, r6, r5
mad r2, r1, #-0.3333333432674408 | mad r6, r5, #-0.3333333432674408 | ld r1, [067b] | ld r5, [06a7]
subxy r2, r1, r2 | subxy r6, r5, r6
mul r2, r0, r2 | mul r6, r4, r6
addxy r2, r2, r1 | addxy r6, r6, r5 | ld r1, [067d] | ld r5, [06a9]
st r2, [067a] | st r6, [06a6]
subxy r2, r1, r2 | subxy r6, r5, r6
mul r2, r0, r2 | mul r6, r4, r6
addxy r2, r2, r1 | addxy r6, r6, r5 | ld r1, [067f] | ld r5, [06ab]
st r2, [067c] | st r6, [06a8]
subxy r2, r1, r2 | subxy r6, r5, r6
mul r2, r0, r2 | mul r6, r4, r6
addxy r2, r2, r1 | addxy r6, r6, r5
subxy r1, r3, r2 | subxy r5, r7, r6 | st r2, [067e]
mul r2, r0, r1 | mul r6, r4, r5 | st r6, [06aa] | ld r1, [0682]
addxy r0, r2, r3 | addxy r4, r6, r7 | ld r5, [06ae]
addxy r2, r1, #0.5 | addxy r6, r5, #0.5 | ld r1, [0680] | ld r5, [06ac]
mul r3, r2, r0 | mul r7, r6, r4 | st r0, [0678]
mul r2, r1, r3 | mul r6, r5, r7 | st r4, [06a4] | ld r1, [06bd] | ld r3, [c094]
mul r0, r2, #4.0 | mul r4, r6, #4.0 | ld r5, [06e9] | ld r7, [c095]
st r0, [0404] | st r4, [0406]
ld r0, [a16c] | ld r2, [c051]
subxy r1, r1, r0 | ld r4, [a16d] | st r0, [06bc] | ld r0, [c09c]
subxy r5, r5, r4 | mul r1, r1, r1 | st r4, [06e8] | ld r4, [c09d]
mul r5, r5, r5 | st r1, [06ca] | ld r1, [c050]
mul r1, r3, r1 | st r5, [06f6] | ld r5, [c050]
mul r5, r7, r5 | mad r1, r0, -r2 | ld r6, [c051] | ld r2, [06cf] | ld r0, [06cd]
mad r5, r4, -r6 | add r1, #1.0 | ld r6, [06fb] | ld r4, [06f9]
add r5, #1.0 | st r1, [06c6] | ld r1, [c04f]
mul r1, r3, r1 | st r5, [06f2] | ld r5, [c04f]
mul r5, r7, r5 | st r1, [06c8] | ld r1, [06ca] | st r3, [a124]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | st r5, [06f4] | ld r5, [06f6] | st r7, [a125]
mad r6, r6, -r5 | addxy r5, r4, #-0.999 | ld r3, [c053]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | ld r7, [c053]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [06ce] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [06fa]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7
mul r2, r2, #1.25 | mul r6, r6, #1.25
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r2, [06c6] | ld r6, [06f2]
min r0, r0, #1.0
min r4, r4, #1.0 | st r0, [06cc] | st r0, [a12c]
st r4, [06f8] | st r4, [a12d]
ld r0, [c084]
ld r4, [c085] | st r0, [06b0]
st r4, [06dc] | ld r0, [c08c] | ld r1, [06b0]
mul r3, r1, r2 | ld r4, [c08d] | st r0, [06b6] | ld r0, [a155]
st r4, [06e2] | ld r4, [a156] | ld r2, [c061]
ld r5, [06dc] | st r0, [06da] | st r4, [0706]
mul r7, r5, r6 | ld r0, [a1b4] | st r3, [06d8] | ld r3, [c062]
addxy r0, r0, #-4.75 | ld r4, [a1b5] | st r7, [0704] | ld r7, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | ld r1, [06b6] | ld r5, [06e2] | ld r6, [c061]
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [06c8] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [06f4] | ld r6, [c060]
mad r0, r1, #0.125 | mad r4, r5, #0.125
mul r3, r0, #0.17328679513998632 | mul r7, r4, #0.17328679513998632
mov r1, #0.041666666666666664 | mov r5, #0.041666666666666664
ymacd r1, r3, #0.16666666666666666 | ymacd r5, r7, #0.16666666666666666
ymacd r1, r3, #0.5 | ymacd r5, r7, #0.5
ymacd r1, r3, #1.0 | ymacd r5, r7, #1.0
ymacd r1, r3, #1.0 | ymacd r5, r7, #1.0
mul r0, r1, r1 | mul r4, r5, r5
mul r3, r0, r0 | mul r7, r4, r4
mul r1, r3, r3 | mul r5, r7, r7
mul r0, r1, r1 | mul r4, r5, r5
mul r3, r0, r0 | mul r7, r4, r4
mul r1, r3, #6.103515625e-05 | mul r5, r7, #6.103515625e-05
mul r0, r2, r1 | mul r4, r6, r5 | ld r1, [c063]
min r3, r0, #3.0517578125e-05 | ld r5, [c063]
min r7, r4, #3.0517578125e-05 | shl r2, r3, 15 | ld r3, [06d1]
shl r6, r7, 15 | ld r7, [06fd]
ph_clamp r0, r3, -r2 | ld r2, [06da]
ph_clamp r4, r3, -r6 | ld r6, [0706] | st r1, [06da]
mac_p1wc3 r2, r1, r3, #-0.125, r2 | st r5, [0706]
mac_p1wc3 r6, r5, r7, #-0.125, r6
mul r2, r2, #-4.0 | mul r6, r6, #-4.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
mul r1, r2, r2 | mul r5, r6, r6
mul r1, r2, r1 | mul r5, r6, r5
mad r2, r1, #-0.3333333432674408 | mad r6, r5, #-0.3333333432674408 | ld r1, [06d3] | ld r5, [06ff]
subxy r2, r1, r2 | subxy r6, r5, r6
mul r2, r0, r2 | mul r6, r4, r6
addxy r2, r2, r1 | addxy r6, r6, r5 | ld r1, [06d5] | ld r5, [0701]
st r2, [06d2] | st r6, [06fe]
subxy r2, r1, r2 | subxy r6, r5, r6
mul r2, r0, r2 | mul r6, r4, r6
addxy r2, r2, r1 | addxy r6, r6, r5 | ld r1, [06d7] | ld r5, [0703]
st r2, [06d4] | st r6, [0700]
subxy r2, r1, r2 | subxy r6, r5, r6
mul r2, r0, r2 | mul r6, r4, r6
addxy r2, r2, r1 | addxy r6, r6, r5
subxy r1, r3, r2 | subxy r5, r7, r6 | st r2, [06d6]
mul r2, r0, r1 | mul r6, r4, r5 | st r6, [0702] | ld r1, [06da]
addxy r0, r2, r3 | addxy r4, r6, r7 | ld r5, [0706]
addxy r2, r1, #0.5 | addxy r6, r5, #0.5 | ld r1, [06d8] | ld r5, [0704]
mul r3, r2, r0 | mul r7, r6, r4 | st r0, [06d0]
mul r2, r1, r3 | mul r6, r5, r7 | st r4, [06fc] | ld r1, [0715] | ld r3, [c096]
mul r0, r2, #4.0 | mul r4, r6, #4.0 | ld r5, [0741] | ld r7, [c097]
st r0, [0408] | st r4, [040a]
ld r0, [a16e] | ld r2, [c051]
subxy r1, r1, r0 | ld r4, [a16f] | st r0, [0714] | ld r0, [c09e]
subxy r5, r5, r4 | mul r1, r1, r1 | st r4, [0740] | ld r4, [c09f]
mul r5, r5, r5 | st r1, [0722] | ld r1, [c050]
mul r1, r3, r1 | st r5, [074e] | ld r5, [c050]
mul r5, r7, r5 | mad r1, r0, -r2 | ld r6, [c051] | ld r2, [0727] | ld r0, [0725]
mad r5, r4, -r6 | add r1, #1.0 | ld r6, [0753] | ld r4, [0751]
add r5, #1.0 | st r1, [071e] | ld r1, [c04f]
mul r1, r3, r1 | st r5, [074a] | ld r5, [c04f]
mul r5, r7, r5 | st r1, [0720] | ld r1, [0722] | st r3, [a126]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | st r5, [074c] | ld r5, [074e] | st r7, [a127]
mad r6, r6, -r5 | addxy r5, r4, #-0.999 | ld r3, [c053]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | ld r7, [c053]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [0726] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [0752]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7
mul r2, r2, #1.25 | mul r6, r6, #1.25
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r2, [071e] | ld r6, [074a]
min r0, r0, #1.0
min r4, r4, #1.0 | st r0, [0724] | st r0, [a12e]
st r4, [0750] | st r4, [a12f]
ld r0, [c086]
ld r4, [c087] | st r0, [0708]
st r4, [0734] | ld r0, [c08e] | ld r1, [0708]
mul r3, r1, r2 | ld r4, [c08f] | st r0, [070e] | ld r0, [a157]
st r4, [073a] | ld r4, [a158] | ld r2, [c061]
ld r5, [0734] | st r0, [0732] | st r4, [075e]
mul r7, r5, r6 | ld r0, [a1b6] | st r3, [0730] | ld r3, [c062]
addxy r0, r0, #-4.75 | ld r4, [a1b7] | st r7, [075c] | ld r7, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | ld r1, [070e] | ld r5, [073a] | ld r6, [c061]
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [0720] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [074c] | ld r6, [c060]
mad r0, r1, #0.125 | mad r4, r5, #0.125
mul r3, r0, #0.17328679513998632 | mul r7, r4, #0.17328679513998632
mov r1, #0.041666666666666664 | mov r5, #0.041666666666666664
ymacd r1, r3, #0.16666666666666666 | ymacd r5, r7, #0.16666666666666666
ymacd r1, r3, #0.5 | ymacd r5, r7, #0.5
ymacd r1, r3, #1.0 | ymacd r5, r7, #1.0
ymacd r1, r3, #1.0 | ymacd r5, r7, #1.0
mul r0, r1, r1 | mul r4, r5, r5
mul r3, r0, r0 | mul r7, r4, r4
mul r1, r3, r3 | mul r5, r7, r7
mul r0, r1, r1 | mul r4, r5, r5
mul r3, r0, r0 | mul r7, r4, r4
mul r1, r3, #6.103515625e-05 | mul r5, r7, #6.103515625e-05
mul r0, r2, r1 | mul r4, r6, r5 | ld r1, [c063]
min r3, r0, #3.0517578125e-05 | ld r5, [c063]
min r7, r4, #3.0517578125e-05 | shl r2, r3, 15 | ld r3, [0729]
shl r6, r7, 15 | ld r7, [0755]
ph_clamp r0, r3, -r2 | ld r2, [0732]
ph_clamp r4, r3, -r6 | ld r6, [075e] | st r1, [0732]
mac_p1wc3 r2, r1, r3, #-0.125, r2 | st r5, [075e]
mac_p1wc3 r6, r5, r7, #-0.125, r6
mul r2, r2, #-4.0 | mul r6, r6, #-4.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
mul r1, r2, r2 | mul r5, r6, r6
mul r1, r2, r1 | mul r5, r6, r5
mad r2, r1, #-0.3333333432674408 | mad r6, r5, #-0.3333333432674408 | ld r1, [072b] | ld r5, [0757]
subxy r2, r1, r2 | subxy r6, r5, r6
mul r2, r0, r2 | mul r6, r4, r6
addxy r2, r2, r1 | addxy r6, r6, r5 | ld r1, [072d] | ld r5, [0759]
st r2, [072a] | st r6, [0756]
subxy r2, r1, r2 | subxy r6, r5, r6
mul r2, r0, r2 | mul r6, r4, r6
addxy r2, r2, r1 | addxy r6, r6, r5 | ld r1, [072f] | ld r5, [075b]
st r2, [072c] | st r6, [0758]
subxy r2, r1, r2 | subxy r6, r5, r6
mul r2, r0, r2 | mul r6, r4, r6
addxy r2, r2, r1 | addxy r6, r6, r5
subxy r1, r3, r2 | subxy r5, r7, r6 | st r2, [072e]
mul r2, r0, r1 | mul r6, r4, r5 | st r6, [075a] | ld r1, [0732]
addxy r0, r2, r3 | addxy r4, r6, r7 | ld r5, [075e]
addxy r2, r1, #0.5 | addxy r6, r5, #0.5 | ld r1, [0730] | ld r5, [075c]
mul r3, r2, r0 | mul r7, r6, r4 | st r0, [0728]
mul r2, r1, r3 | mul r6, r5, r7 | st r4, [0754]
mul r0, r2, #4.0 | mul r4, r6, #4.0
st r0, [040c] | st r4, [040e]
