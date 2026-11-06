mov r7, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r4, [a169]
addxy r7, r7, r0 | ld r0, [0405] | ld r2, [0731] | ld r6, [c000]
addxy r7, r7, r1 | mul r2, r2, #7.31275 | ld r1, [0407] | ld r5, [0633] | ld r3, [c090]
addxy r7, r7, r0 | subxy r5, r5, r4 | ld r0, [0409] | st r4, [0632] | ld r4, [c099]
addxy r7, r7, r1 | mul r5, r5, r5 | ld r1, [040b] | st r3, [a120]
addxy r7, r7, r0 | ld r0, [040d] | ld r5, [c050]
addxy r7, r7, r1 | ld r1, [040f]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | ld r0, [0733] | ld r1, [060d]
add r0, #0.6184303127229214:32 | mul r7, r7, #0.16666666666666666
ph_wrap r0, r3, r0
addxy r2, r2, r0 | mul r7, r7, r6 | st r0, [0732] | ld r0, [a168]
subxy r1, r1, r0 | ph_wrap r2, r3, r2 | st r7, [6006]
mul r1, r1, r1 | st r7, [6007]
ld r7, [c091] | st r2, [0730]
mul r5, r7, r5 | ld r1, [c050] | st r0, [060c]
mul r1, r3, r1 | ld r0, [c098] | st r7, [a121]
ld r2, [c051]
mad r1, r0, -r2 | ld r6, [c051]
mad r5, r4, -r6 | add r1, #1.0 | ld r0, [c080]
add r5, #1.0 | st r1, [0616] | ld r4, [c081]
st r5, [063c] | ld r1, [c04f] | ld r2, [0616]
mul r1, r3, r1 | ld r5, [c04f] | st r0, [0600] | ld r6, [063c]
mul r5, r7, r5 | st r4, [0626] | st r0, [a170]
st r4, [a171] | ld r0, [c088]
ld r4, [c089] | st r1, [0618] | ld r1, [0600]
st r5, [063e] | st r0, [0606] | ld r5, [0626]
mul r3, r1, r2 | mul r7, r5, r6 | st r4, [062c] | st r0, [a178] | ld r1, [0606]
st r4, [a179] | st r3, [061e] | ld r5, [062c]
ld r0, [a151] | st r7, [0644] | ld r3, [c062]
ld r4, [a152] | st r0, [0620] | ld r7, [c062]
st r4, [0646] | ld r0, [a1b0] | ld r2, [c061]
addxy r0, r0, #-4.75 | ld r4, [a1b1] | ld r6, [c061]
addxy r4, r4, #-4.75 | mul r3, r3, r0
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [0618] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [063e] | ld r6, [c060]
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
min r2, r0, #3.3569336665095761e-05 | st r7, [0648] | ld r0, [061b]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [061d] | ld r4, [0641]
shl r5, r6, 14 | ld r6, [0643] | st r1, [0624]
mul r3, r1, r2 | mul r7, r5, r6 | st r5, [064a]
addxy r3, r3, r0 | addxy r7, r7, r4 | ld r0, [0620] | ld r4, [0646]
mov r1, r0 | mov r5, r4 | ld r0, [0622] | ld r4, [0648]
mad r1, r3, #-0.0009765625 | mad r5, r7, #-0.0009765625
shl r1, r1, 10
shl r5, r5, 10
mac_p1wc3 r1, r0, r2, #-0.5, r1 | ld r0, [0624]
mac_p1wc3 r5, r4, r6, #-0.5, r5 | ld r4, [064a]
mac_p1wc3 r1, r0, r1, #-0.5, r2
mac_p1wc3 r5, r4, r5, #-0.5, r6
mac_p1wc3 r2, r0, r1, #-0.5, r3
mac_p1wc3 r6, r4, r5, #-0.5, r7
mul r3, r2, #-2.0 | mul r7, r6, #-2.0
st r3, [061a] | st r7, [0640] | ld r3, [0620]
mad r3, r2, #0.001953125 | ld r7, [0646] | st r2, [0620]
mad r7, r6, #0.001953125 | st r6, [0646] | ld r2, [0622]
shl r3, r3, 10 | ld r6, [0648]
shl r7, r7, 10
mac_p1wc3 r2, r2, r1, #0.25, r3
mac_p1wc3 r6, r6, r5, #0.25, r7
mac_p1wc3 r3, r0, r2, #-0.5, r1 | ld r2, [061e] | ld r1, [0620]
mac_p1wc3 r7, r4, r6, #-0.5, r5 | ld r6, [0644] | ld r5, [0646]
mul r0, r3, #4.0 | mul r4, r7, #4.0
mul r3, r2, r1 | mul r7, r6, r5 | st r0, [061c] | ld r1, [0659] | ld r2, [c051]
mul r0, r3, #-0.001953125 | mul r4, r7, #-0.001953125 | st r4, [0642] | ld r5, [067f] | ld r3, [c092]
st r0, [0400] | st r4, [0402]
ld r0, [a16a] | ld r7, [c093]
subxy r1, r1, r0 | ld r4, [a16b] | st r0, [0658] | ld r0, [c09a]
subxy r5, r5, r4 | mul r1, r1, r1 | st r4, [067e] | ld r4, [c09b]
mul r5, r5, r5 | ld r1, [c050] | st r3, [a122]
mul r1, r3, r1 | ld r5, [c050] | st r7, [a123]
mul r5, r7, r5 | mad r1, r0, -r2 | ld r6, [c051]
mad r5, r4, -r6 | add r1, #1.0 | ld r0, [c082]
add r5, #1.0 | st r1, [0662] | ld r4, [c083]
st r5, [0688] | ld r1, [c04f] | ld r2, [0662]
mul r1, r3, r1 | ld r5, [c04f] | st r0, [064c] | ld r6, [0688]
mul r5, r7, r5 | st r4, [0672] | st r0, [a172]
st r4, [a173] | ld r0, [c08a]
ld r4, [c08b] | st r1, [0664] | ld r1, [064c]
st r5, [068a] | st r0, [0652] | ld r5, [0672]
mul r3, r1, r2 | mul r7, r5, r6 | st r4, [0678] | st r0, [a17a] | ld r1, [0652]
st r4, [a17b] | st r3, [066a] | ld r5, [0678]
ld r0, [a153] | st r7, [0690] | ld r3, [c062]
ld r4, [a154] | st r0, [066c] | ld r7, [c062]
st r4, [0692] | ld r0, [a1b2] | ld r2, [c061]
addxy r0, r0, #-4.75 | ld r4, [a1b3] | ld r6, [c061]
addxy r4, r4, #-4.75 | mul r3, r3, r0
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [0664] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [068a] | ld r6, [c060]
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
mul r0, r2, r1 | mul r4, r6, r5 | ld r7, [c063] | st r3, [066e]
min r2, r0, #3.3569336665095761e-05 | st r7, [0694] | ld r0, [0667]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [0669] | ld r4, [068d]
shl r5, r6, 14 | ld r6, [068f] | st r1, [0670]
mul r3, r1, r2 | mul r7, r5, r6 | st r5, [0696]
addxy r3, r3, r0 | addxy r7, r7, r4 | ld r0, [066c] | ld r4, [0692]
mov r1, r0 | mov r5, r4 | ld r0, [066e] | ld r4, [0694]
mad r1, r3, #-0.0009765625 | mad r5, r7, #-0.0009765625
shl r1, r1, 10
shl r5, r5, 10
mac_p1wc3 r1, r0, r2, #-0.5, r1 | ld r0, [0670]
mac_p1wc3 r5, r4, r6, #-0.5, r5 | ld r4, [0696]
mac_p1wc3 r1, r0, r1, #-0.5, r2
mac_p1wc3 r5, r4, r5, #-0.5, r6
mac_p1wc3 r2, r0, r1, #-0.5, r3
mac_p1wc3 r6, r4, r5, #-0.5, r7
mul r3, r2, #-2.0 | mul r7, r6, #-2.0
st r3, [0666] | st r7, [068c] | ld r3, [066c]
mad r3, r2, #0.001953125 | ld r7, [0692] | st r2, [066c]
mad r7, r6, #0.001953125 | st r6, [0692] | ld r2, [066e]
shl r3, r3, 10 | ld r6, [0694]
shl r7, r7, 10
mac_p1wc3 r2, r2, r1, #0.25, r3
mac_p1wc3 r6, r6, r5, #0.25, r7
mac_p1wc3 r3, r0, r2, #-0.5, r1 | ld r2, [066a] | ld r1, [066c]
mac_p1wc3 r7, r4, r6, #-0.5, r5 | ld r6, [0690] | ld r5, [0692]
mul r0, r3, #4.0 | mul r4, r7, #4.0
mul r3, r2, r1 | mul r7, r6, r5 | st r0, [0668] | ld r1, [06a5] | ld r2, [c051]
mul r0, r3, #-0.001953125 | mul r4, r7, #-0.001953125 | st r4, [068e] | ld r5, [06cb] | ld r3, [c094]
st r0, [0404] | st r4, [0406]
ld r0, [a16c] | ld r7, [c095]
subxy r1, r1, r0 | ld r4, [a16d] | st r0, [06a4] | ld r0, [c09c]
subxy r5, r5, r4 | mul r1, r1, r1 | st r4, [06ca] | ld r4, [c09d]
mul r5, r5, r5 | ld r1, [c050] | st r3, [a124]
mul r1, r3, r1 | ld r5, [c050] | st r7, [a125]
mul r5, r7, r5 | mad r1, r0, -r2 | ld r6, [c051]
mad r5, r4, -r6 | add r1, #1.0 | ld r0, [c084]
add r5, #1.0 | st r1, [06ae] | ld r4, [c085]
st r5, [06d4] | ld r1, [c04f] | ld r2, [06ae]
mul r1, r3, r1 | ld r5, [c04f] | st r0, [0698] | ld r6, [06d4]
mul r5, r7, r5 | st r4, [06be] | st r0, [a174]
st r4, [a175] | ld r0, [c08c]
ld r4, [c08d] | st r1, [06b0] | ld r1, [0698]
st r5, [06d6] | st r0, [069e] | ld r5, [06be]
mul r3, r1, r2 | mul r7, r5, r6 | st r4, [06c4] | st r0, [a17c] | ld r1, [069e]
st r4, [a17d] | st r3, [06b6] | ld r5, [06c4]
ld r0, [a155] | st r7, [06dc] | ld r3, [c062]
ld r4, [a156] | st r0, [06b8] | ld r7, [c062]
st r4, [06de] | ld r0, [a1b4] | ld r2, [c061]
addxy r0, r0, #-4.75 | ld r4, [a1b5] | ld r6, [c061]
addxy r4, r4, #-4.75 | mul r3, r3, r0
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [06b0] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [06d6] | ld r6, [c060]
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
mul r0, r2, r1 | mul r4, r6, r5 | ld r7, [c063] | st r3, [06ba]
min r2, r0, #3.3569336665095761e-05 | st r7, [06e0] | ld r0, [06b3]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [06b5] | ld r4, [06d9]
shl r5, r6, 14 | ld r6, [06db] | st r1, [06bc]
mul r3, r1, r2 | mul r7, r5, r6 | st r5, [06e2]
addxy r3, r3, r0 | addxy r7, r7, r4 | ld r0, [06b8] | ld r4, [06de]
mov r1, r0 | mov r5, r4 | ld r0, [06ba] | ld r4, [06e0]
mad r1, r3, #-0.0009765625 | mad r5, r7, #-0.0009765625
shl r1, r1, 10
shl r5, r5, 10
mac_p1wc3 r1, r0, r2, #-0.5, r1 | ld r0, [06bc]
mac_p1wc3 r5, r4, r6, #-0.5, r5 | ld r4, [06e2]
mac_p1wc3 r1, r0, r1, #-0.5, r2
mac_p1wc3 r5, r4, r5, #-0.5, r6
mac_p1wc3 r2, r0, r1, #-0.5, r3
mac_p1wc3 r6, r4, r5, #-0.5, r7
mul r3, r2, #-2.0 | mul r7, r6, #-2.0
st r3, [06b2] | st r7, [06d8] | ld r3, [06b8]
mad r3, r2, #0.001953125 | ld r7, [06de] | st r2, [06b8]
mad r7, r6, #0.001953125 | st r6, [06de] | ld r2, [06ba]
shl r3, r3, 10 | ld r6, [06e0]
shl r7, r7, 10
mac_p1wc3 r2, r2, r1, #0.25, r3
mac_p1wc3 r6, r6, r5, #0.25, r7
mac_p1wc3 r3, r0, r2, #-0.5, r1 | ld r2, [06b6] | ld r1, [06b8]
mac_p1wc3 r7, r4, r6, #-0.5, r5 | ld r6, [06dc] | ld r5, [06de]
mul r0, r3, #4.0 | mul r4, r7, #4.0
mul r3, r2, r1 | mul r7, r6, r5 | st r0, [06b4] | ld r1, [06f1] | ld r2, [c051]
mul r0, r3, #-0.001953125 | mul r4, r7, #-0.001953125 | st r4, [06da] | ld r5, [0717] | ld r3, [c096]
st r0, [0408] | st r4, [040a]
ld r0, [a16e] | ld r7, [c097]
subxy r1, r1, r0 | ld r4, [a16f] | st r0, [06f0] | ld r0, [c09e]
subxy r5, r5, r4 | mul r1, r1, r1 | st r4, [0716] | ld r4, [c09f]
mul r5, r5, r5 | ld r1, [c050] | st r3, [a126]
mul r1, r3, r1 | ld r5, [c050] | st r7, [a127]
mul r5, r7, r5 | mad r1, r0, -r2 | ld r6, [c051]
mad r5, r4, -r6 | add r1, #1.0 | ld r0, [c086]
add r5, #1.0 | st r1, [06fa] | ld r4, [c087]
st r5, [0720] | ld r1, [c04f] | ld r2, [06fa]
mul r1, r3, r1 | ld r5, [c04f] | st r0, [06e4] | ld r6, [0720]
mul r5, r7, r5 | st r4, [070a] | st r0, [a176]
st r4, [a177] | ld r0, [c08e]
ld r4, [c08f] | st r1, [06fc] | ld r1, [06e4]
st r5, [0722] | st r0, [06ea] | ld r5, [070a]
mul r3, r1, r2 | mul r7, r5, r6 | st r4, [0710] | st r0, [a17e] | ld r1, [06ea]
st r4, [a17f] | st r3, [0702] | ld r5, [0710]
ld r0, [a157] | st r7, [0728] | ld r3, [c062]
ld r4, [a158] | st r0, [0704] | ld r7, [c062]
st r4, [072a] | ld r0, [a1b6] | ld r2, [c061]
addxy r0, r0, #-4.75 | ld r4, [a1b7] | ld r6, [c061]
addxy r4, r4, #-4.75 | mul r3, r3, r0
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [06fc] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [0722] | ld r6, [c060]
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
mul r0, r2, r1 | mul r4, r6, r5 | ld r7, [c063] | st r3, [0706]
min r2, r0, #3.3569336665095761e-05 | st r7, [072c] | ld r0, [06ff]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [0701] | ld r4, [0725]
shl r5, r6, 14 | ld r6, [0727] | st r1, [0708]
mul r3, r1, r2 | mul r7, r5, r6 | st r5, [072e]
addxy r3, r3, r0 | addxy r7, r7, r4 | ld r0, [0704] | ld r4, [072a]
mov r1, r0 | mov r5, r4 | ld r0, [0706] | ld r4, [072c]
mad r1, r3, #-0.0009765625 | mad r5, r7, #-0.0009765625
shl r1, r1, 10
shl r5, r5, 10
mac_p1wc3 r1, r0, r2, #-0.5, r1 | ld r0, [0708]
mac_p1wc3 r5, r4, r6, #-0.5, r5 | ld r4, [072e]
mac_p1wc3 r1, r0, r1, #-0.5, r2
mac_p1wc3 r5, r4, r5, #-0.5, r6
mac_p1wc3 r2, r0, r1, #-0.5, r3
mac_p1wc3 r6, r4, r5, #-0.5, r7
mul r3, r2, #-2.0 | mul r7, r6, #-2.0
st r3, [06fe] | st r7, [0724] | ld r3, [0704]
mad r3, r2, #0.001953125 | ld r7, [072a] | st r2, [0704]
mad r7, r6, #0.001953125 | st r6, [072a] | ld r2, [0706]
shl r3, r3, 10 | ld r6, [072c]
shl r7, r7, 10
mac_p1wc3 r2, r2, r1, #0.25, r3
mac_p1wc3 r6, r6, r5, #0.25, r7
mac_p1wc3 r3, r0, r2, #-0.5, r1 | ld r2, [0702] | ld r1, [0704]
mac_p1wc3 r7, r4, r6, #-0.5, r5 | ld r6, [0728] | ld r5, [072a]
mul r0, r3, #4.0 | mul r4, r7, #4.0
mul r3, r2, r1 | mul r7, r6, r5 | st r0, [0700]
mul r0, r3, #-0.001953125 | mul r4, r7, #-0.001953125 | st r4, [0726]
st r0, [040c] | st r4, [040e]
