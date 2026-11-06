mov r7, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r4, [a169]
addxy r7, r7, r0 | ld r0, [0405] | ld r2, [0741] | ld r6, [c000]
addxy r7, r7, r1 | mul r2, r2, #7.31275 | ld r1, [0407] | ld r5, [0635] | ld r3, [c090]
addxy r7, r7, r0 | subxy r5, r5, r4 | ld r0, [0409] | st r4, [0634] | ld r4, [c099]
addxy r7, r7, r1 | mul r5, r5, r5 | ld r1, [040b] | st r3, [a120]
addxy r7, r7, r0 | ld r0, [040d] | ld r5, [c050]
addxy r7, r7, r1 | ld r1, [040f]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | st r0, [6008] | ld r1, [060d]
mul r7, r7, #0.16666666666666666 | st r0, [6009]
mul r7, r7, r6 | st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [0743]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [0742] | ld r0, [a168] | ld r7, [c091]
subxy r1, r1, r0 | mul r5, r7, r5 | st r0, [060c] | ld r0, [c098]
mul r1, r1, r1 | ld r6, [c051] | st r7, [a121]
ph_wrap r2, r3, r2 | ld r1, [c050]
mul r1, r3, r1 | mad r5, r4, -r6 | st r2, [0740] | ld r2, [c051]
mad r1, r0, -r2 | add r5, #1.0 | ld r0, [c080]
add r1, #1.0 | st r5, [063e] | ld r4, [c081]
st r1, [0616] | ld r1, [c04f] | ld r6, [063e]
mul r1, r3, r1 | ld r5, [c04f] | st r0, [0600] | ld r2, [0616]
mul r5, r7, r5 | st r4, [0628] | ld r0, [c088]
ld r4, [c089] | st r1, [0618] | ld r1, [0600]
st r5, [0640] | st r0, [0606] | ld r5, [0628]
mul r3, r1, r2 | mul r7, r5, r6 | st r4, [062e] | ld r0, [a151]
ld r4, [a152] | st r0, [0620] | ld r2, [c061]
st r4, [0648] | ld r0, [a1b0] | ld r6, [c061]
addxy r0, r0, #-4.75 | ld r4, [a1b1] | st r3, [061e] | ld r3, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | st r7, [0646] | ld r7, [c062] | ld r1, [0606]
mul r7, r7, r4 | ld r5, [062e]
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [0618] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [0640] | ld r6, [c060]
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
mul r0, r2, r1 | mul r4, r6, r5 | ld r7, [c063] | st r3, [0622]
min r2, r0, #3.3569336665095761e-05 | st r7, [064a] | ld r0, [061b]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [061d] | ld r4, [0643]
shl r5, r6, 14 | ld r6, [0645] | st r1, [0624]
mul r3, r1, r2 | mul r7, r5, r6 | st r5, [064c]
addxy r3, r3, r0 | addxy r7, r7, r4 | ld r0, [0620] | ld r4, [0648]
subxy r1, r3, r0 | subxy r5, r7, r4 | ld r0, [0622] | ld r4, [064a]
mac_p1wc3 r1, r0, r2, #-0.5, r1 | ld r0, [0624]
mac_p1wc3 r5, r4, r6, #-0.5, r5 | ld r4, [064c]
mac_p1wc3 r1, r0, r1, #-0.5, r2
mac_p1wc3 r5, r4, r5, #-0.5, r6
mac_p1wc3 r2, r0, r1, #-0.5, r3
mac_p1wc3 r6, r4, r5, #-0.5, r7 | st r2, [0626]
mul r3, r2, #-2.0 | mul r7, r6, #-2.0 | st r6, [064e]
st r3, [061a] | st r7, [0642]
mov r3, r2 | mov r7, r6 | ld r2, [0620] | ld r6, [0648]
mad r3, r2, #0.5 | mad r7, r6, #0.5 | ld r2, [0622] | ld r6, [064a]
mac_p1wc3 r2, r2, r1, #0.5, r3
mac_p1wc3 r6, r6, r5, #0.5, r7
mac_p1wc3 r3, r0, r2, #-0.5, r1 | ld r2, [061e] | ld r1, [0626]
mac_p1wc3 r7, r4, r6, #-0.5, r5 | ld r6, [0646] | ld r5, [064e]
mul r0, r3, #4.0 | mul r4, r7, #4.0
mul r3, r2, r1 | mul r7, r6, r5 | st r0, [061c] | ld r1, [065d] | ld r2, [c051]
mul r0, r3, #-2.0 | mul r4, r7, #-2.0 | st r4, [0644] | ld r5, [0685] | ld r3, [c092]
st r0, [0400] | st r4, [0402]
ld r0, [a16a] | ld r7, [c093]
subxy r1, r1, r0 | ld r4, [a16b] | st r0, [065c] | ld r0, [c09a]
subxy r5, r5, r4 | mul r1, r1, r1 | st r4, [0684] | ld r4, [c09b]
mul r5, r5, r5 | ld r1, [c050] | st r3, [a122]
mul r1, r3, r1 | ld r5, [c050] | st r7, [a123]
mul r5, r7, r5 | mad r1, r0, -r2 | ld r6, [c051]
mad r5, r4, -r6 | add r1, #1.0 | ld r0, [c082]
add r5, #1.0 | st r1, [0666] | ld r4, [c083]
st r5, [068e] | ld r1, [c04f] | ld r2, [0666]
mul r1, r3, r1 | ld r5, [c04f] | st r0, [0650] | ld r6, [068e]
mul r5, r7, r5 | st r4, [0678] | ld r0, [c08a]
ld r4, [c08b] | st r1, [0668] | ld r1, [0650]
st r5, [0690] | st r0, [0656] | ld r5, [0678]
mul r3, r1, r2 | mul r7, r5, r6 | st r4, [067e] | ld r0, [a153]
ld r4, [a154] | st r0, [0670] | ld r2, [c061]
st r4, [0698] | ld r0, [a1b2] | ld r6, [c061]
addxy r0, r0, #-4.75 | ld r4, [a1b3] | st r3, [066e] | ld r3, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | st r7, [0696] | ld r7, [c062] | ld r1, [0656]
mul r7, r7, r4 | ld r5, [067e]
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [0668] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [0690] | ld r6, [c060]
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
mul r0, r2, r1 | mul r4, r6, r5 | ld r7, [c063] | st r3, [0672]
min r2, r0, #3.3569336665095761e-05 | st r7, [069a] | ld r0, [066b]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [066d] | ld r4, [0693]
shl r5, r6, 14 | ld r6, [0695] | st r1, [0674]
mul r3, r1, r2 | mul r7, r5, r6 | st r5, [069c]
addxy r3, r3, r0 | addxy r7, r7, r4 | ld r0, [0670] | ld r4, [0698]
subxy r1, r3, r0 | subxy r5, r7, r4 | ld r0, [0672] | ld r4, [069a]
mac_p1wc3 r1, r0, r2, #-0.5, r1 | ld r0, [0674]
mac_p1wc3 r5, r4, r6, #-0.5, r5 | ld r4, [069c]
mac_p1wc3 r1, r0, r1, #-0.5, r2
mac_p1wc3 r5, r4, r5, #-0.5, r6
mac_p1wc3 r2, r0, r1, #-0.5, r3
mac_p1wc3 r6, r4, r5, #-0.5, r7 | st r2, [0676]
mul r3, r2, #-2.0 | mul r7, r6, #-2.0 | st r6, [069e]
st r3, [066a] | st r7, [0692]
mov r3, r2 | mov r7, r6 | ld r2, [0670] | ld r6, [0698]
mad r3, r2, #0.5 | mad r7, r6, #0.5 | ld r2, [0672] | ld r6, [069a]
mac_p1wc3 r2, r2, r1, #0.5, r3
mac_p1wc3 r6, r6, r5, #0.5, r7
mac_p1wc3 r3, r0, r2, #-0.5, r1 | ld r2, [066e] | ld r1, [0676]
mac_p1wc3 r7, r4, r6, #-0.5, r5 | ld r6, [0696] | ld r5, [069e]
mul r0, r3, #4.0 | mul r4, r7, #4.0
mul r3, r2, r1 | mul r7, r6, r5 | st r0, [066c] | ld r1, [06ad] | ld r2, [c051]
mul r0, r3, #-2.0 | mul r4, r7, #-2.0 | st r4, [0694] | ld r5, [06d5] | ld r3, [c094]
st r0, [0404] | st r4, [0406]
ld r0, [a16c] | ld r7, [c095]
subxy r1, r1, r0 | ld r4, [a16d] | st r0, [06ac] | ld r0, [c09c]
subxy r5, r5, r4 | mul r1, r1, r1 | st r4, [06d4] | ld r4, [c09d]
mul r5, r5, r5 | ld r1, [c050] | st r3, [a124]
mul r1, r3, r1 | ld r5, [c050] | st r7, [a125]
mul r5, r7, r5 | mad r1, r0, -r2 | ld r6, [c051]
mad r5, r4, -r6 | add r1, #1.0 | ld r0, [c084]
add r5, #1.0 | st r1, [06b6] | ld r4, [c085]
st r5, [06de] | ld r1, [c04f] | ld r2, [06b6]
mul r1, r3, r1 | ld r5, [c04f] | st r0, [06a0] | ld r6, [06de]
mul r5, r7, r5 | st r4, [06c8] | ld r0, [c08c]
ld r4, [c08d] | st r1, [06b8] | ld r1, [06a0]
st r5, [06e0] | st r0, [06a6] | ld r5, [06c8]
mul r3, r1, r2 | mul r7, r5, r6 | st r4, [06ce] | ld r0, [a155]
ld r4, [a156] | st r0, [06c0] | ld r2, [c061]
st r4, [06e8] | ld r0, [a1b4] | ld r6, [c061]
addxy r0, r0, #-4.75 | ld r4, [a1b5] | st r3, [06be] | ld r3, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | st r7, [06e6] | ld r7, [c062] | ld r1, [06a6]
mul r7, r7, r4 | ld r5, [06ce]
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [06b8] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [06e0] | ld r6, [c060]
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
mul r0, r2, r1 | mul r4, r6, r5 | ld r7, [c063] | st r3, [06c2]
min r2, r0, #3.3569336665095761e-05 | st r7, [06ea] | ld r0, [06bb]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [06bd] | ld r4, [06e3]
shl r5, r6, 14 | ld r6, [06e5] | st r1, [06c4]
mul r3, r1, r2 | mul r7, r5, r6 | st r5, [06ec]
addxy r3, r3, r0 | addxy r7, r7, r4 | ld r0, [06c0] | ld r4, [06e8]
subxy r1, r3, r0 | subxy r5, r7, r4 | ld r0, [06c2] | ld r4, [06ea]
mac_p1wc3 r1, r0, r2, #-0.5, r1 | ld r0, [06c4]
mac_p1wc3 r5, r4, r6, #-0.5, r5 | ld r4, [06ec]
mac_p1wc3 r1, r0, r1, #-0.5, r2
mac_p1wc3 r5, r4, r5, #-0.5, r6
mac_p1wc3 r2, r0, r1, #-0.5, r3
mac_p1wc3 r6, r4, r5, #-0.5, r7 | st r2, [06c6]
mul r3, r2, #-2.0 | mul r7, r6, #-2.0 | st r6, [06ee]
st r3, [06ba] | st r7, [06e2]
mov r3, r2 | mov r7, r6 | ld r2, [06c0] | ld r6, [06e8]
mad r3, r2, #0.5 | mad r7, r6, #0.5 | ld r2, [06c2] | ld r6, [06ea]
mac_p1wc3 r2, r2, r1, #0.5, r3
mac_p1wc3 r6, r6, r5, #0.5, r7
mac_p1wc3 r3, r0, r2, #-0.5, r1 | ld r2, [06be] | ld r1, [06c6]
mac_p1wc3 r7, r4, r6, #-0.5, r5 | ld r6, [06e6] | ld r5, [06ee]
mul r0, r3, #4.0 | mul r4, r7, #4.0
mul r3, r2, r1 | mul r7, r6, r5 | st r0, [06bc] | ld r1, [06fd] | ld r2, [c051]
mul r0, r3, #-2.0 | mul r4, r7, #-2.0 | st r4, [06e4] | ld r5, [0725] | ld r3, [c096]
st r0, [0408] | st r4, [040a]
ld r0, [a16e] | ld r7, [c097]
subxy r1, r1, r0 | ld r4, [a16f] | st r0, [06fc] | ld r0, [c09e]
subxy r5, r5, r4 | mul r1, r1, r1 | st r4, [0724] | ld r4, [c09f]
mul r5, r5, r5 | ld r1, [c050] | st r3, [a126]
mul r1, r3, r1 | ld r5, [c050] | st r7, [a127]
mul r5, r7, r5 | mad r1, r0, -r2 | ld r6, [c051]
mad r5, r4, -r6 | add r1, #1.0 | ld r0, [c086]
add r5, #1.0 | st r1, [0706] | ld r4, [c087]
st r5, [072e] | ld r1, [c04f] | ld r2, [0706]
mul r1, r3, r1 | ld r5, [c04f] | st r0, [06f0] | ld r6, [072e]
mul r5, r7, r5 | st r4, [0718] | ld r0, [c08e]
ld r4, [c08f] | st r1, [0708] | ld r1, [06f0]
st r5, [0730] | st r0, [06f6] | ld r5, [0718]
mul r3, r1, r2 | mul r7, r5, r6 | st r4, [071e] | ld r0, [a157]
ld r4, [a158] | st r0, [0710] | ld r2, [c061]
st r4, [0738] | ld r0, [a1b6] | ld r6, [c061]
addxy r0, r0, #-4.75 | ld r4, [a1b7] | st r3, [070e] | ld r3, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | st r7, [0736] | ld r7, [c062] | ld r1, [06f6]
mul r7, r7, r4 | ld r5, [071e]
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [0708] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [0730] | ld r6, [c060]
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
mul r0, r2, r1 | mul r4, r6, r5 | ld r7, [c063] | st r3, [0712]
min r2, r0, #3.3569336665095761e-05 | st r7, [073a] | ld r0, [070b]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [070d] | ld r4, [0733]
shl r5, r6, 14 | ld r6, [0735] | st r1, [0714]
mul r3, r1, r2 | mul r7, r5, r6 | st r5, [073c]
addxy r3, r3, r0 | addxy r7, r7, r4 | ld r0, [0710] | ld r4, [0738]
subxy r1, r3, r0 | subxy r5, r7, r4 | ld r0, [0712] | ld r4, [073a]
mac_p1wc3 r1, r0, r2, #-0.5, r1 | ld r0, [0714]
mac_p1wc3 r5, r4, r6, #-0.5, r5 | ld r4, [073c]
mac_p1wc3 r1, r0, r1, #-0.5, r2
mac_p1wc3 r5, r4, r5, #-0.5, r6
mac_p1wc3 r2, r0, r1, #-0.5, r3
mac_p1wc3 r6, r4, r5, #-0.5, r7 | st r2, [0716]
mul r3, r2, #-2.0 | mul r7, r6, #-2.0 | st r6, [073e]
st r3, [070a] | st r7, [0732]
mov r3, r2 | mov r7, r6 | ld r2, [0710] | ld r6, [0738]
mad r3, r2, #0.5 | mad r7, r6, #0.5 | ld r2, [0712] | ld r6, [073a]
mac_p1wc3 r2, r2, r1, #0.5, r3
mac_p1wc3 r6, r6, r5, #0.5, r7
mac_p1wc3 r3, r0, r2, #-0.5, r1 | ld r2, [070e] | ld r1, [0716]
mac_p1wc3 r7, r4, r6, #-0.5, r5 | ld r6, [0736] | ld r5, [073e]
mul r0, r3, #4.0 | mul r4, r7, #4.0
mul r3, r2, r1 | mul r7, r6, r5 | st r0, [070c]
mul r0, r3, #-2.0 | mul r4, r7, #-2.0 | st r4, [0734]
st r0, [040c] | st r4, [040e]
