mov r7, #0.0 | mov r5, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r3, [a168]
addxy r7, r7, r0 | ld r0, [0405] | st r3, [0604] | ld r6, [c000]
addxy r7, r7, r1 | ld r1, [0407] | ld r2, [0603] | ld r4, [a160]
addxy r7, r7, r0 | ld r0, [0409]
addxy r7, r7, r1 | ld r1, [040b]
addxy r7, r7, r0 | ld r0, [040d]
addxy r7, r7, r1 | ld r1, [040f]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | st r0, [6008] | ld r1, [0601]
mul r7, r7, #0.16666666666666666 | st r0, [6009]
mul r7, r7, r6 | mov r6, #1.0 | st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [0605]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r7, [6006]
mul r3, r3, #256.0 | mul r0, r0, r0 | st r7, [6007]
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0602]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0609]
mad r4, r3, #1.25 | ld r3, [a168]
subxy r4, r1, r4 | st r3, [060a]
mad r1, r4, r0 | ld r0, [060b] | ld r4, [a160]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [0600] | st r1, [a170] | ld r1, [0607]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0608]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a0]
min r1, r1, r6 | ld r0, [a151]
addxy r5, r4, #-4.75 | st r1, [0606] | st r1, [a178]
mul r6, r3, r5 | mov r5, #0.041666666666666664 | ld r1, [0606] | ld r4, [c060]
mul r7, r1, r2
mad r7, r6, #0.125
mul r3, r7, #0.17328679513998632
ymacd r5, r3, #0.16666666666666666
ymacd r5, r3, #0.5
ymacd r5, r3, #1.0
ymacd r5, r3, #1.0
mul r1, r5, r5
mul r2, r1, r1
mul r6, r2, r2 | ld r2, [c063]
mul r7, r6, r6
mul r3, r7, r7
mul r5, r3, #0.0009765625
mul r1, r4, r5 | ld r5, [060f] | ld r4, [060d]
min r6, r1, #0.00053710938664153218
mul r7, r6, #256.0
mul r3, r7, #4.0
mul r1, r3, r5
addxy r6, r1, r4
mov r7, r6
mad r7, r0, #-0.0625
mul r1, r2, r5
mad r1, r7, #0.5
mul r4, r3, r1
mad r4, r5, #-0.5
mul r7, r3, r4
mad r7, r6, #-0.5
mov r5, r7
mad r5, r0, #0.03125
mul r6, r2, r4 | mul r1, r7, #-2.0 | ld r2, [0600]
mad r6, r5, #0.5 | st r1, [060c]
mul r1, r3, r6 | mul r5, r2, r7 | ld r2, [0613]
mad r1, r4, #-0.5 | ld r4, [a161]
mul r0, r1, #4.0 | ld r1, [0611]
mul r3, r5, #-32.0 | st r0, [060e] | ld r0, [0615]
mov r5, #0.0 | mov r6, #1.0 | st r3, [0400] | ld r3, [a169]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [0614]
mul r3, r3, #256.0 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0612]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0619]
mad r4, r3, #1.25 | ld r3, [a169]
subxy r4, r1, r4 | st r3, [061a]
mad r1, r4, r0 | ld r0, [061b] | ld r4, [a161]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [0610] | st r1, [a171] | ld r1, [0617]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0618]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a1]
min r1, r1, r6 | ld r0, [a152]
addxy r5, r4, #-4.75 | st r1, [0616] | st r1, [a179]
mul r6, r3, r5 | mov r5, #0.041666666666666664 | ld r1, [0616] | ld r4, [c060]
mul r7, r1, r2
mad r7, r6, #0.125
mul r3, r7, #0.17328679513998632
ymacd r5, r3, #0.16666666666666666
ymacd r5, r3, #0.5
ymacd r5, r3, #1.0
ymacd r5, r3, #1.0
mul r1, r5, r5
mul r2, r1, r1
mul r6, r2, r2 | ld r2, [c063]
mul r7, r6, r6
mul r3, r7, r7
mul r5, r3, #0.0009765625
mul r1, r4, r5 | ld r5, [061f] | ld r4, [061d]
min r6, r1, #0.00053710938664153218
mul r7, r6, #256.0
mul r3, r7, #4.0
mul r1, r3, r5
addxy r6, r1, r4
mov r7, r6
mad r7, r0, #-0.0625
mul r1, r2, r5
mad r1, r7, #0.5
mul r4, r3, r1
mad r4, r5, #-0.5
mul r7, r3, r4
mad r7, r6, #-0.5
mov r5, r7
mad r5, r0, #0.03125
mul r6, r2, r4 | mul r1, r7, #-2.0 | ld r2, [0610]
mad r6, r5, #0.5 | st r1, [061c]
mul r1, r3, r6 | mul r5, r2, r7 | ld r2, [0623]
mad r1, r4, #-0.5 | ld r4, [a162]
mul r0, r1, #4.0 | ld r1, [0621]
mul r3, r5, #-32.0 | st r0, [061e] | ld r0, [0625]
mov r5, #0.0 | mov r6, #1.0 | st r3, [0402] | ld r3, [a16a]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [0624]
mul r3, r3, #256.0 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0622]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0629]
mad r4, r3, #1.25 | ld r3, [a16a]
subxy r4, r1, r4 | st r3, [062a]
mad r1, r4, r0 | ld r0, [062b] | ld r4, [a162]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [0620] | st r1, [a172] | ld r1, [0627]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0628]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a2]
min r1, r1, r6 | ld r0, [a153]
addxy r5, r4, #-4.75 | st r1, [0626] | st r1, [a17a]
mul r6, r3, r5 | mov r5, #0.041666666666666664 | ld r1, [0626] | ld r4, [c060]
mul r7, r1, r2
mad r7, r6, #0.125
mul r3, r7, #0.17328679513998632
ymacd r5, r3, #0.16666666666666666
ymacd r5, r3, #0.5
ymacd r5, r3, #1.0
ymacd r5, r3, #1.0
mul r1, r5, r5
mul r2, r1, r1
mul r6, r2, r2 | ld r2, [c063]
mul r7, r6, r6
mul r3, r7, r7
mul r5, r3, #0.0009765625
mul r1, r4, r5 | ld r5, [062f] | ld r4, [062d]
min r6, r1, #0.00053710938664153218
mul r7, r6, #256.0
mul r3, r7, #4.0
mul r1, r3, r5
addxy r6, r1, r4
mov r7, r6
mad r7, r0, #-0.0625
mul r1, r2, r5
mad r1, r7, #0.5
mul r4, r3, r1
mad r4, r5, #-0.5
mul r7, r3, r4
mad r7, r6, #-0.5
mov r5, r7
mad r5, r0, #0.03125
mul r6, r2, r4 | mul r1, r7, #-2.0 | ld r2, [0620]
mad r6, r5, #0.5 | st r1, [062c]
mul r1, r3, r6 | mul r5, r2, r7 | ld r2, [0633]
mad r1, r4, #-0.5 | ld r4, [a163]
mul r0, r1, #4.0 | ld r1, [0631]
mul r3, r5, #-32.0 | st r0, [062e] | ld r0, [0635]
mov r5, #0.0 | mov r6, #1.0 | st r3, [0404] | ld r3, [a16b]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [0634]
mul r3, r3, #256.0 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0632]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0639]
mad r4, r3, #1.25 | ld r3, [a16b]
subxy r4, r1, r4 | st r3, [063a]
mad r1, r4, r0 | ld r0, [063b] | ld r4, [a163]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [0630] | st r1, [a173] | ld r1, [0637]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0638]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a3]
min r1, r1, r6 | ld r0, [a154]
addxy r5, r4, #-4.75 | st r1, [0636] | st r1, [a17b]
mul r6, r3, r5 | mov r5, #0.041666666666666664 | ld r1, [0636] | ld r4, [c060]
mul r7, r1, r2
mad r7, r6, #0.125
mul r3, r7, #0.17328679513998632
ymacd r5, r3, #0.16666666666666666
ymacd r5, r3, #0.5
ymacd r5, r3, #1.0
ymacd r5, r3, #1.0
mul r1, r5, r5
mul r2, r1, r1
mul r6, r2, r2 | ld r2, [c063]
mul r7, r6, r6
mul r3, r7, r7
mul r5, r3, #0.0009765625
mul r1, r4, r5 | ld r5, [063f] | ld r4, [063d]
min r6, r1, #0.00053710938664153218
mul r7, r6, #256.0
mul r3, r7, #4.0
mul r1, r3, r5
addxy r6, r1, r4
mov r7, r6
mad r7, r0, #-0.0625
mul r1, r2, r5
mad r1, r7, #0.5
mul r4, r3, r1
mad r4, r5, #-0.5
mul r7, r3, r4
mad r7, r6, #-0.5
mov r5, r7
mad r5, r0, #0.03125
mul r6, r2, r4 | mul r1, r7, #-2.0 | ld r2, [0630]
mad r6, r5, #0.5 | st r1, [063c]
mul r1, r3, r6 | mul r5, r2, r7 | ld r2, [0643]
mad r1, r4, #-0.5 | ld r4, [a164]
mul r0, r1, #4.0 | ld r1, [0641]
mul r3, r5, #-32.0 | st r0, [063e] | ld r0, [0645]
mov r5, #0.0 | mov r6, #1.0 | st r3, [0406] | ld r3, [a16c]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [0644]
mul r3, r3, #256.0 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0642]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0649]
mad r4, r3, #1.25 | ld r3, [a16c]
subxy r4, r1, r4 | st r3, [064a]
mad r1, r4, r0 | ld r0, [064b] | ld r4, [a164]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [0640] | st r1, [a174] | ld r1, [0647]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0648]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a4]
min r1, r1, r6 | ld r0, [a155]
addxy r5, r4, #-4.75 | st r1, [0646] | st r1, [a17c]
mul r6, r3, r5 | mov r5, #0.041666666666666664 | ld r1, [0646] | ld r4, [c060]
mul r7, r1, r2
mad r7, r6, #0.125
mul r3, r7, #0.17328679513998632
ymacd r5, r3, #0.16666666666666666
ymacd r5, r3, #0.5
ymacd r5, r3, #1.0
ymacd r5, r3, #1.0
mul r1, r5, r5
mul r2, r1, r1
mul r6, r2, r2 | ld r2, [c063]
mul r7, r6, r6
mul r3, r7, r7
mul r5, r3, #0.0009765625
mul r1, r4, r5 | ld r5, [064f] | ld r4, [064d]
min r6, r1, #0.00053710938664153218
mul r7, r6, #256.0
mul r3, r7, #4.0
mul r1, r3, r5
addxy r6, r1, r4
mov r7, r6
mad r7, r0, #-0.0625
mul r1, r2, r5
mad r1, r7, #0.5
mul r4, r3, r1
mad r4, r5, #-0.5
mul r7, r3, r4
mad r7, r6, #-0.5
mov r5, r7
mad r5, r0, #0.03125
mul r6, r2, r4 | mul r1, r7, #-2.0 | ld r2, [0640]
mad r6, r5, #0.5 | st r1, [064c]
mul r1, r3, r6 | mul r5, r2, r7 | ld r2, [0653]
mad r1, r4, #-0.5 | ld r4, [a165]
mul r0, r1, #4.0 | ld r1, [0651]
mul r3, r5, #-32.0 | st r0, [064e] | ld r0, [0655]
mov r5, #0.0 | mov r6, #1.0 | st r3, [0408] | ld r3, [a16d]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [0654]
mul r3, r3, #256.0 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0652]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0659]
mad r4, r3, #1.25 | ld r3, [a16d]
subxy r4, r1, r4 | st r3, [065a]
mad r1, r4, r0 | ld r0, [065b] | ld r4, [a165]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [0650] | st r1, [a175] | ld r1, [0657]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0658]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a5]
min r1, r1, r6 | ld r0, [a156]
addxy r5, r4, #-4.75 | st r1, [0656] | st r1, [a17d]
mul r6, r3, r5 | mov r5, #0.041666666666666664 | ld r1, [0656] | ld r4, [c060]
mul r7, r1, r2
mad r7, r6, #0.125
mul r3, r7, #0.17328679513998632
ymacd r5, r3, #0.16666666666666666
ymacd r5, r3, #0.5
ymacd r5, r3, #1.0
ymacd r5, r3, #1.0
mul r1, r5, r5
mul r2, r1, r1
mul r6, r2, r2 | ld r2, [c063]
mul r7, r6, r6
mul r3, r7, r7
mul r5, r3, #0.0009765625
mul r1, r4, r5 | ld r5, [065f] | ld r4, [065d]
min r6, r1, #0.00053710938664153218
mul r7, r6, #256.0
mul r3, r7, #4.0
mul r1, r3, r5
addxy r6, r1, r4
mov r7, r6
mad r7, r0, #-0.0625
mul r1, r2, r5
mad r1, r7, #0.5
mul r4, r3, r1
mad r4, r5, #-0.5
mul r7, r3, r4
mad r7, r6, #-0.5
mov r5, r7
mad r5, r0, #0.03125
mul r6, r2, r4 | mul r1, r7, #-2.0 | ld r2, [0650]
mad r6, r5, #0.5 | st r1, [065c]
mul r1, r3, r6 | mul r5, r2, r7 | ld r2, [0663]
mad r1, r4, #-0.5 | ld r4, [a166]
mul r0, r1, #4.0 | ld r1, [0661]
mul r3, r5, #-32.0 | st r0, [065e] | ld r0, [0665]
mov r5, #0.0 | mov r6, #1.0 | st r3, [040a] | ld r3, [a16e]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [0664]
mul r3, r3, #256.0 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0662]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0669]
mad r4, r3, #1.25 | ld r3, [a16e]
subxy r4, r1, r4 | st r3, [066a]
mad r1, r4, r0 | ld r0, [066b] | ld r4, [a166]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [0660] | st r1, [a176] | ld r1, [0667]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0668]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a6]
min r1, r1, r6 | ld r0, [a157]
addxy r5, r4, #-4.75 | st r1, [0666] | st r1, [a17e]
mul r6, r3, r5 | mov r5, #0.041666666666666664 | ld r1, [0666] | ld r4, [c060]
mul r7, r1, r2
mad r7, r6, #0.125
mul r3, r7, #0.17328679513998632
ymacd r5, r3, #0.16666666666666666
ymacd r5, r3, #0.5
ymacd r5, r3, #1.0
ymacd r5, r3, #1.0
mul r1, r5, r5
mul r2, r1, r1
mul r6, r2, r2 | ld r2, [c063]
mul r7, r6, r6
mul r3, r7, r7
mul r5, r3, #0.0009765625
mul r1, r4, r5 | ld r5, [066f] | ld r4, [066d]
min r6, r1, #0.00053710938664153218
mul r7, r6, #256.0
mul r3, r7, #4.0
mul r1, r3, r5
addxy r6, r1, r4
mov r7, r6
mad r7, r0, #-0.0625
mul r1, r2, r5
mad r1, r7, #0.5
mul r4, r3, r1
mad r4, r5, #-0.5
mul r7, r3, r4
mad r7, r6, #-0.5
mov r5, r7
mad r5, r0, #0.03125
mul r6, r2, r4 | mul r1, r7, #-2.0 | ld r2, [0660]
mad r6, r5, #0.5 | st r1, [066c]
mul r1, r3, r6 | mul r5, r2, r7 | ld r2, [0673]
mad r1, r4, #-0.5 | ld r4, [a167]
mul r0, r1, #4.0 | ld r1, [0671]
mul r3, r5, #-32.0 | st r0, [066e] | ld r0, [0675]
mov r5, #0.0 | mov r6, #1.0 | st r3, [040c] | ld r3, [a16f]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [0674]
mul r3, r3, #256.0 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0672]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0679]
mad r4, r3, #1.25 | ld r3, [a16f]
subxy r4, r1, r4 | st r3, [067a]
mad r1, r4, r0 | ld r0, [067b] | ld r4, [a167]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [0670] | st r1, [a177] | ld r1, [0677]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0678]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a7]
min r1, r1, r6 | ld r0, [a158]
addxy r5, r4, #-4.75 | st r1, [0676] | st r1, [a17f]
mul r6, r3, r5 | mov r5, #0.041666666666666664 | ld r1, [0676] | ld r4, [c060]
mul r7, r1, r2
mad r7, r6, #0.125
mul r3, r7, #0.17328679513998632
ymacd r5, r3, #0.16666666666666666
ymacd r5, r3, #0.5
ymacd r5, r3, #1.0
ymacd r5, r3, #1.0
mul r1, r5, r5
mul r2, r1, r1
mul r6, r2, r2 | ld r2, [c063]
mul r7, r6, r6
mul r3, r7, r7
mul r5, r3, #0.0009765625
mul r1, r4, r5 | ld r5, [067f] | ld r4, [067d]
min r6, r1, #0.00053710938664153218
mul r7, r6, #256.0
mul r3, r7, #4.0
mul r1, r3, r5
addxy r6, r1, r4
mov r7, r6
mad r7, r0, #-0.0625
mul r1, r2, r5
mad r1, r7, #0.5
mul r4, r3, r1
mad r4, r5, #-0.5
mul r7, r3, r4
mad r7, r6, #-0.5
mov r5, r7
mad r5, r0, #0.03125
mul r6, r2, r4 | mul r1, r7, #-2.0 | ld r2, [0670]
mad r6, r5, #0.5 | st r1, [067c]
mul r1, r3, r6 | mul r5, r2, r7
mad r1, r4, #-0.5
mul r0, r1, #4.0
mul r3, r5, #-32.0 | st r0, [067e]
st r3, [040e]
