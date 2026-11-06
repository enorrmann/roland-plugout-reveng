mov r7, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r4, [a169]
addxy r7, r7, r0 | ld r0, [0405] | ld r2, [0771] | ld r6, [c000]
addxy r7, r7, r1 | mul r2, r2, #7.31275 | ld r1, [0407] | ld r5, [063b] | ld r3, [c090]
addxy r7, r7, r0 | subxy r5, r5, r4 | ld r0, [0409] | st r4, [063a] | ld r4, [c099]
addxy r7, r7, r1 | mul r5, r5, r5 | ld r1, [040b] | st r3, [a120]
addxy r7, r7, r0 | ld r0, [040d] | st r5, [0648] | ld r5, [c050]
addxy r7, r7, r1 | ld r1, [040f]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | st r0, [6008] | ld r1, [060d]
mul r7, r7, #0.16666666666666666 | st r0, [6009]
mul r7, r7, r6 | st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [0773]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [0772] | ld r0, [a168] | ld r7, [c091]
subxy r1, r1, r0 | mul r5, r7, r5 | st r0, [060c] | ld r0, [c098]
mul r1, r1, r1 | ld r6, [c051] | st r7, [a121]
ph_wrap r2, r3, r2 | st r1, [061a] | ld r1, [c050]
mul r1, r3, r1 | mad r5, r4, -r6 | st r2, [0770] | ld r2, [c051] | ld r6, [064d]
mad r1, r0, -r2 | add r5, #1.0 | ld r2, [061f] | ld r0, [061d]
add r1, #1.0 | st r5, [0644] | ld r5, [c04f] | ld r4, [064b]
mul r5, r7, r5 | st r1, [0616] | ld r1, [c04f]
mul r1, r3, r1 | st r5, [0646] | ld r5, [0648] | ld r3, [c053]
mad r6, r6, -r5 | addxy r5, r4, #-0.999 | st r1, [0618] | ld r1, [061a] | ld r7, [c053]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [061e] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [064c]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7
mul r2, r2, #1.25 | mul r6, r6, #1.25
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r2, [0616] | ld r6, [0644]
min r0, r0, #1.0
min r4, r4, #1.0 | st r0, [061c] | st r0, [a128]
st r4, [064a] | st r4, [a129]
ld r0, [c080]
ld r4, [c081] | st r0, [0600]
st r4, [062e] | ld r0, [c088] | ld r1, [0600]
mul r3, r1, r2 | ld r4, [c089] | st r0, [0606] | ld r0, [a151]
st r4, [0634] | ld r4, [a152] | ld r2, [c061]
ld r5, [062e] | st r0, [0626] | st r4, [0654]
mul r7, r5, r6 | ld r0, [a1b0] | st r3, [0624] | ld r3, [c062]
addxy r0, r0, #-4.75 | ld r4, [a1b1] | st r7, [0652] | ld r7, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | ld r1, [0606] | ld r5, [0634] | ld r6, [c061]
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [0618] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [0646] | ld r6, [c060]
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
mul r1, r3, #6.103515625e-05 | mul r5, r7, #6.103515625e-05 | ld r3, [c063]
mul r0, r2, r1 | mul r4, r6, r5 | ld r7, [c063] | st r3, [0628]
min r2, r0, #3.3569336665095761e-05 | st r7, [0656] | ld r0, [0621]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [0623] | ld r4, [064f]
shl r5, r6, 14 | ld r6, [0651]
mul r3, r1, r2 | mul r7, r5, r6
addxy r3, r3, r0 | addxy r7, r7, r4
mov r0, r3 | mov r4, r7 | st r3, [062a] | ld r3, [0626]
mad r0, r3, #-0.0625 | st r7, [0658] | ld r7, [0654]
mad r4, r7, #-0.0625 | ld r3, [0628] | ld r7, [0656]
mac_p1wc3 r0, r3, r2, #0.5, r0
mac_p1wc3 r4, r7, r6, #0.5, r4
mac_p1wc3 r0, r1, r0, #-0.5, r2 | ld r2, [062a]
mac_p1wc3 r4, r5, r4, #-0.5, r6 | ld r6, [0658] | st r1, [062a]
mac_p1wc3 r2, r1, r0, #-0.5, r2 | st r5, [0658]
mac_p1wc3 r6, r5, r4, #-0.5, r6 | st r2, [062c]
mul r1, r2, #-2.0 | mul r5, r6, #-2.0 | st r6, [065a]
st r1, [0620] | st r5, [064e]
mov r1, r2 | mov r5, r6 | ld r2, [0626] | ld r6, [0654]
mad r1, r2, #0.03125 | mad r5, r6, #0.03125
mac_p1wc3 r2, r3, r0, #0.5, r1 | ld r3, [062a]
mac_p1wc3 r6, r7, r4, #0.5, r5 | ld r7, [0658]
mac_p1wc3 r1, r3, r2, #-0.5, r0 | ld r2, [0624] | ld r0, [062c]
mac_p1wc3 r5, r7, r6, #-0.5, r4 | ld r6, [0652] | ld r4, [065a]
mul r3, r1, #4.0 | mul r7, r5, #4.0
mul r1, r2, r0 | mul r5, r6, r4 | st r3, [0622] | ld r0, [a16a]
mul r3, r1, #-32.0 | mul r7, r5, #-32.0 | st r7, [0650] | ld r4, [a16b]
ld r1, [0669] | ld r5, [0697] | ld r2, [c051]
subxy r1, r1, r0 | subxy r5, r5, r4 | st r3, [0400] | ld r3, [c092]
mul r1, r1, r1 | mul r5, r5, r5 | st r7, [0402] | ld r7, [c093]
st r1, [0676] | st r5, [06a4]
ld r1, [c050] | st r0, [0668]
mul r1, r3, r1 | ld r5, [c050] | st r4, [0696]
mul r5, r7, r5 | ld r0, [c09a] | st r3, [a122]
mad r1, r0, -r2 | ld r4, [c09b] | ld r2, [067b] | ld r0, [0679]
add r1, #1.0 | ld r6, [c051] | st r7, [a123]
mad r5, r4, -r6 | st r1, [0672] | ld r1, [c04f] | ld r6, [06a9]
add r5, #1.0 | ld r4, [06a7]
mul r1, r3, r1 | st r5, [06a0] | ld r5, [c04f]
mul r5, r7, r5 | st r1, [0674] | ld r1, [0676] | ld r3, [c053]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | st r5, [06a2] | ld r5, [06a4] | ld r7, [c053]
mad r6, r6, -r5 | addxy r5, r4, #-0.999
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [067a] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [06a8]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7
mul r2, r2, #1.25 | mul r6, r6, #1.25
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r2, [0672] | ld r6, [06a0]
min r0, r0, #1.0
min r4, r4, #1.0 | st r0, [0678] | st r0, [a12a]
st r4, [06a6] | st r4, [a12b]
ld r0, [c082]
ld r4, [c083] | st r0, [065c]
st r4, [068a] | ld r0, [c08a] | ld r1, [065c]
mul r3, r1, r2 | ld r4, [c08b] | st r0, [0662] | ld r0, [a153]
st r4, [0690] | ld r4, [a154] | ld r2, [c061]
ld r5, [068a] | st r0, [0682] | st r4, [06b0]
mul r7, r5, r6 | ld r0, [a1b2] | st r3, [0680] | ld r3, [c062]
addxy r0, r0, #-4.75 | ld r4, [a1b3] | st r7, [06ae] | ld r7, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | ld r1, [0662] | ld r5, [0690] | ld r6, [c061]
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [0674] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [06a2] | ld r6, [c060]
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
mul r1, r3, #6.103515625e-05 | mul r5, r7, #6.103515625e-05 | ld r3, [c063]
mul r0, r2, r1 | mul r4, r6, r5 | ld r7, [c063] | st r3, [0684]
min r2, r0, #3.3569336665095761e-05 | st r7, [06b2] | ld r0, [067d]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [067f] | ld r4, [06ab]
shl r5, r6, 14 | ld r6, [06ad]
mul r3, r1, r2 | mul r7, r5, r6
addxy r3, r3, r0 | addxy r7, r7, r4
mov r0, r3 | mov r4, r7 | st r3, [0686] | ld r3, [0682]
mad r0, r3, #-0.0625 | st r7, [06b4] | ld r7, [06b0]
mad r4, r7, #-0.0625 | ld r3, [0684] | ld r7, [06b2]
mac_p1wc3 r0, r3, r2, #0.5, r0
mac_p1wc3 r4, r7, r6, #0.5, r4
mac_p1wc3 r0, r1, r0, #-0.5, r2 | ld r2, [0686]
mac_p1wc3 r4, r5, r4, #-0.5, r6 | ld r6, [06b4] | st r1, [0686]
mac_p1wc3 r2, r1, r0, #-0.5, r2 | st r5, [06b4]
mac_p1wc3 r6, r5, r4, #-0.5, r6 | st r2, [0688]
mul r1, r2, #-2.0 | mul r5, r6, #-2.0 | st r6, [06b6]
st r1, [067c] | st r5, [06aa]
mov r1, r2 | mov r5, r6 | ld r2, [0682] | ld r6, [06b0]
mad r1, r2, #0.03125 | mad r5, r6, #0.03125
mac_p1wc3 r2, r3, r0, #0.5, r1 | ld r3, [0686]
mac_p1wc3 r6, r7, r4, #0.5, r5 | ld r7, [06b4]
mac_p1wc3 r1, r3, r2, #-0.5, r0 | ld r2, [0680] | ld r0, [0688]
mac_p1wc3 r5, r7, r6, #-0.5, r4 | ld r6, [06ae] | ld r4, [06b6]
mul r3, r1, #4.0 | mul r7, r5, #4.0
mul r1, r2, r0 | mul r5, r6, r4 | st r3, [067e] | ld r0, [a16c]
mul r3, r1, #-32.0 | mul r7, r5, #-32.0 | st r7, [06ac] | ld r4, [a16d]
ld r1, [06c5] | ld r5, [06f3] | ld r2, [c051]
subxy r1, r1, r0 | subxy r5, r5, r4 | st r3, [0404] | ld r3, [c094]
mul r1, r1, r1 | mul r5, r5, r5 | st r7, [0406] | ld r7, [c095]
st r1, [06d2] | st r5, [0700]
ld r1, [c050] | st r0, [06c4]
mul r1, r3, r1 | ld r5, [c050] | st r4, [06f2]
mul r5, r7, r5 | ld r0, [c09c] | st r3, [a124]
mad r1, r0, -r2 | ld r4, [c09d] | ld r2, [06d7] | ld r0, [06d5]
add r1, #1.0 | ld r6, [c051] | st r7, [a125]
mad r5, r4, -r6 | st r1, [06ce] | ld r1, [c04f] | ld r6, [0705]
add r5, #1.0 | ld r4, [0703]
mul r1, r3, r1 | st r5, [06fc] | ld r5, [c04f]
mul r5, r7, r5 | st r1, [06d0] | ld r1, [06d2] | ld r3, [c053]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | st r5, [06fe] | ld r5, [0700] | ld r7, [c053]
mad r6, r6, -r5 | addxy r5, r4, #-0.999
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [06d6] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [0704]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7
mul r2, r2, #1.25 | mul r6, r6, #1.25
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r2, [06ce] | ld r6, [06fc]
min r0, r0, #1.0
min r4, r4, #1.0 | st r0, [06d4] | st r0, [a12c]
st r4, [0702] | st r4, [a12d]
ld r0, [c084]
ld r4, [c085] | st r0, [06b8]
st r4, [06e6] | ld r0, [c08c] | ld r1, [06b8]
mul r3, r1, r2 | ld r4, [c08d] | st r0, [06be] | ld r0, [a155]
st r4, [06ec] | ld r4, [a156] | ld r2, [c061]
ld r5, [06e6] | st r0, [06de] | st r4, [070c]
mul r7, r5, r6 | ld r0, [a1b4] | st r3, [06dc] | ld r3, [c062]
addxy r0, r0, #-4.75 | ld r4, [a1b5] | st r7, [070a] | ld r7, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | ld r1, [06be] | ld r5, [06ec] | ld r6, [c061]
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [06d0] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [06fe] | ld r6, [c060]
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
mul r1, r3, #6.103515625e-05 | mul r5, r7, #6.103515625e-05 | ld r3, [c063]
mul r0, r2, r1 | mul r4, r6, r5 | ld r7, [c063] | st r3, [06e0]
min r2, r0, #3.3569336665095761e-05 | st r7, [070e] | ld r0, [06d9]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [06db] | ld r4, [0707]
shl r5, r6, 14 | ld r6, [0709]
mul r3, r1, r2 | mul r7, r5, r6
addxy r3, r3, r0 | addxy r7, r7, r4
mov r0, r3 | mov r4, r7 | st r3, [06e2] | ld r3, [06de]
mad r0, r3, #-0.0625 | st r7, [0710] | ld r7, [070c]
mad r4, r7, #-0.0625 | ld r3, [06e0] | ld r7, [070e]
mac_p1wc3 r0, r3, r2, #0.5, r0
mac_p1wc3 r4, r7, r6, #0.5, r4
mac_p1wc3 r0, r1, r0, #-0.5, r2 | ld r2, [06e2]
mac_p1wc3 r4, r5, r4, #-0.5, r6 | ld r6, [0710] | st r1, [06e2]
mac_p1wc3 r2, r1, r0, #-0.5, r2 | st r5, [0710]
mac_p1wc3 r6, r5, r4, #-0.5, r6 | st r2, [06e4]
mul r1, r2, #-2.0 | mul r5, r6, #-2.0 | st r6, [0712]
st r1, [06d8] | st r5, [0706]
mov r1, r2 | mov r5, r6 | ld r2, [06de] | ld r6, [070c]
mad r1, r2, #0.03125 | mad r5, r6, #0.03125
mac_p1wc3 r2, r3, r0, #0.5, r1 | ld r3, [06e2]
mac_p1wc3 r6, r7, r4, #0.5, r5 | ld r7, [0710]
mac_p1wc3 r1, r3, r2, #-0.5, r0 | ld r2, [06dc] | ld r0, [06e4]
mac_p1wc3 r5, r7, r6, #-0.5, r4 | ld r6, [070a] | ld r4, [0712]
mul r3, r1, #4.0 | mul r7, r5, #4.0
mul r1, r2, r0 | mul r5, r6, r4 | st r3, [06da] | ld r0, [a16e]
mul r3, r1, #-32.0 | mul r7, r5, #-32.0 | st r7, [0708] | ld r4, [a16f]
ld r1, [0721] | ld r5, [074f] | ld r2, [c051]
subxy r1, r1, r0 | subxy r5, r5, r4 | st r3, [0408] | ld r3, [c096]
mul r1, r1, r1 | mul r5, r5, r5 | st r7, [040a] | ld r7, [c097]
st r1, [072e] | st r5, [075c]
ld r1, [c050] | st r0, [0720]
mul r1, r3, r1 | ld r5, [c050] | st r4, [074e]
mul r5, r7, r5 | ld r0, [c09e] | st r3, [a126]
mad r1, r0, -r2 | ld r4, [c09f] | ld r2, [0733] | ld r0, [0731]
add r1, #1.0 | ld r6, [c051] | st r7, [a127]
mad r5, r4, -r6 | st r1, [072a] | ld r1, [c04f] | ld r6, [0761]
add r5, #1.0 | ld r4, [075f]
mul r1, r3, r1 | st r5, [0758] | ld r5, [c04f]
mul r5, r7, r5 | st r1, [072c] | ld r1, [072e] | ld r3, [c053]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | st r5, [075a] | ld r5, [075c] | ld r7, [c053]
mad r6, r6, -r5 | addxy r5, r4, #-0.999
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [0732] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [0760]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7
mul r2, r2, #1.25 | mul r6, r6, #1.25
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r2, [072a] | ld r6, [0758]
min r0, r0, #1.0
min r4, r4, #1.0 | st r0, [0730] | st r0, [a12e]
st r4, [075e] | st r4, [a12f]
ld r0, [c086]
ld r4, [c087] | st r0, [0714]
st r4, [0742] | ld r0, [c08e] | ld r1, [0714]
mul r3, r1, r2 | ld r4, [c08f] | st r0, [071a] | ld r0, [a157]
st r4, [0748] | ld r4, [a158] | ld r2, [c061]
ld r5, [0742] | st r0, [073a] | st r4, [0768]
mul r7, r5, r6 | ld r0, [a1b6] | st r3, [0738] | ld r3, [c062]
addxy r0, r0, #-4.75 | ld r4, [a1b7] | st r7, [0766] | ld r7, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | ld r1, [071a] | ld r5, [0748] | ld r6, [c061]
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [072c] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [075a] | ld r6, [c060]
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
mul r1, r3, #6.103515625e-05 | mul r5, r7, #6.103515625e-05 | ld r3, [c063]
mul r0, r2, r1 | mul r4, r6, r5 | ld r7, [c063] | st r3, [073c]
min r2, r0, #3.3569336665095761e-05 | st r7, [076a] | ld r0, [0735]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [0737] | ld r4, [0763]
shl r5, r6, 14 | ld r6, [0765]
mul r3, r1, r2 | mul r7, r5, r6
addxy r3, r3, r0 | addxy r7, r7, r4
mov r0, r3 | mov r4, r7 | st r3, [073e] | ld r3, [073a]
mad r0, r3, #-0.0625 | st r7, [076c] | ld r7, [0768]
mad r4, r7, #-0.0625 | ld r3, [073c] | ld r7, [076a]
mac_p1wc3 r0, r3, r2, #0.5, r0
mac_p1wc3 r4, r7, r6, #0.5, r4
mac_p1wc3 r0, r1, r0, #-0.5, r2 | ld r2, [073e]
mac_p1wc3 r4, r5, r4, #-0.5, r6 | ld r6, [076c] | st r1, [073e]
mac_p1wc3 r2, r1, r0, #-0.5, r2 | st r5, [076c]
mac_p1wc3 r6, r5, r4, #-0.5, r6 | st r2, [0740]
mul r1, r2, #-2.0 | mul r5, r6, #-2.0 | st r6, [076e]
st r1, [0734] | st r5, [0762]
mov r1, r2 | mov r5, r6 | ld r2, [073a] | ld r6, [0768]
mad r1, r2, #0.03125 | mad r5, r6, #0.03125
mac_p1wc3 r2, r3, r0, #0.5, r1 | ld r3, [073e]
mac_p1wc3 r6, r7, r4, #0.5, r5 | ld r7, [076c]
mac_p1wc3 r1, r3, r2, #-0.5, r0 | ld r2, [0738] | ld r0, [0740]
mac_p1wc3 r5, r7, r6, #-0.5, r4 | ld r6, [0766] | ld r4, [076e]
mul r3, r1, #4.0 | mul r7, r5, #4.0
mul r1, r2, r0 | mul r5, r6, r4 | st r3, [0736]
mul r3, r1, #-32.0 | mul r7, r5, #-32.0 | st r7, [0764]
st r3, [040c] | st r7, [040e]
