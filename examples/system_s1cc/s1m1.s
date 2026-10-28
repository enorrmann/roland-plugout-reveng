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
subxy r0, r0, r3 | st r1, [0600] | ld r1, [0607]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4 | mov r5, #0.0
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0608]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a0]
min r1, r1, r6 | ld r0, [a151]
addxy r7, r4, #-4.75 | st r1, [0606]
mul r4, r3, r7 | mov r6, #1.0 | ld r1, [0606] | ld r7, [c060] | st r0, [0610]
mul r3, r1, r2 | mov r2, #0.041666666666666664
mad r3, r4, #0.125
mul r1, r3, #0.17328679513998632
ymacd r2, r1, #0.16666666666666666
ymacd r2, r1, #0.5
ymacd r2, r1, #1.0
ymacd r2, r1, #1.0
mul r4, r2, r2
mul r3, r4, r4
mul r1, r3, r3
mul r2, r1, r1
mul r4, r2, r2 | ld r2, [c063]
mul r3, r4, #0.0009765625
mul r1, r7, r3
min r4, r1, #0.00053710938664153218 | ld r1, [060d]
mul r7, r4, #256.0 | ld r4, [060f]
mul r3, r7, #4.0
mul r7, r3, r4
addxy r7, r7, r1
mov r1, r7
mad r1, r0, #-0.125
mul r0, r2, r4
mad r0, r1, #0.5
mul r1, r3, r0
mad r1, r4, #-0.5
mul r0, r3, r1
mad r0, r7, #-0.5
mul r4, r0, #-2.0
mov r7, r0 | st r4, [060c] | ld r4, [0610]
mad r7, r4, #0.0625
mul r4, r2, r1
mad r4, r7, #0.5
mul r2, r3, r4 | ld r3, [0600]
mul r4, r3, r0 | ld r3, [a169] | ld r0, [0617]
mad r2, r1, #-0.5 | st r3, [0616]
mul r1, r4, #-16.0 | subxy r0, r0, r3 | ld r4, [a161]
mul r0, r0, r0 | mul r7, r2, #4.0 | st r1, [0400] | ld r1, [0613]
addxy r3, r1, #-0.999 | ld r2, [0615] | st r7, [060e]
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c043]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0614]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [061b]
mad r4, r3, #1.25 | ld r3, [a169]
subxy r4, r1, r4 | st r3, [061c]
mad r1, r4, r0 | ld r0, [061d] | ld r4, [a161]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [0612] | ld r1, [0619]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4 | mov r5, #0.0
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [061a]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a1]
min r1, r1, r6 | ld r0, [a152]
addxy r7, r4, #-4.75 | st r1, [0618]
mul r4, r3, r7 | mov r6, #1.0 | ld r1, [0618] | ld r7, [c060] | st r0, [0622]
mul r3, r1, r2 | mov r2, #0.041666666666666664
mad r3, r4, #0.125
mul r1, r3, #0.17328679513998632
ymacd r2, r1, #0.16666666666666666
ymacd r2, r1, #0.5
ymacd r2, r1, #1.0
ymacd r2, r1, #1.0
mul r4, r2, r2
mul r3, r4, r4
mul r1, r3, r3
mul r2, r1, r1
mul r4, r2, r2 | ld r2, [c063]
mul r3, r4, #0.0009765625
mul r1, r7, r3
min r4, r1, #0.00053710938664153218 | ld r1, [061f]
mul r7, r4, #256.0 | ld r4, [0621]
mul r3, r7, #4.0
mul r7, r3, r4
addxy r7, r7, r1
mov r1, r7
mad r1, r0, #-0.125
mul r0, r2, r4
mad r0, r1, #0.5
mul r1, r3, r0
mad r1, r4, #-0.5
mul r0, r3, r1
mad r0, r7, #-0.5
mul r4, r0, #-2.0
mov r7, r0 | st r4, [061e] | ld r4, [0622]
mad r7, r4, #0.0625
mul r4, r2, r1
mad r4, r7, #0.5
mul r2, r3, r4 | ld r3, [0612]
mul r4, r3, r0 | ld r3, [a16a] | ld r0, [0629]
mad r2, r1, #-0.5 | st r3, [0628]
mul r1, r4, #-16.0 | subxy r0, r0, r3 | ld r4, [a162]
mul r0, r0, r0 | mul r7, r2, #4.0 | st r1, [0402] | ld r1, [0625]
addxy r3, r1, #-0.999 | ld r2, [0627] | st r7, [0620]
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c043]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0626]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [062d]
mad r4, r3, #1.25 | ld r3, [a16a]
subxy r4, r1, r4 | st r3, [062e]
mad r1, r4, r0 | ld r0, [062f] | ld r4, [a162]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [0624] | ld r1, [062b]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4 | mov r5, #0.0
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [062c]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a2]
min r1, r1, r6 | ld r0, [a153]
addxy r7, r4, #-4.75 | st r1, [062a]
mul r4, r3, r7 | mov r6, #1.0 | ld r1, [062a] | ld r7, [c060] | st r0, [0634]
mul r3, r1, r2 | mov r2, #0.041666666666666664
mad r3, r4, #0.125
mul r1, r3, #0.17328679513998632
ymacd r2, r1, #0.16666666666666666
ymacd r2, r1, #0.5
ymacd r2, r1, #1.0
ymacd r2, r1, #1.0
mul r4, r2, r2
mul r3, r4, r4
mul r1, r3, r3
mul r2, r1, r1
mul r4, r2, r2 | ld r2, [c063]
mul r3, r4, #0.0009765625
mul r1, r7, r3
min r4, r1, #0.00053710938664153218 | ld r1, [0631]
mul r7, r4, #256.0 | ld r4, [0633]
mul r3, r7, #4.0
mul r7, r3, r4
addxy r7, r7, r1
mov r1, r7
mad r1, r0, #-0.125
mul r0, r2, r4
mad r0, r1, #0.5
mul r1, r3, r0
mad r1, r4, #-0.5
mul r0, r3, r1
mad r0, r7, #-0.5
mul r4, r0, #-2.0
mov r7, r0 | st r4, [0630] | ld r4, [0634]
mad r7, r4, #0.0625
mul r4, r2, r1
mad r4, r7, #0.5
mul r2, r3, r4 | ld r3, [0624]
mul r4, r3, r0 | ld r3, [a16b] | ld r0, [063b]
mad r2, r1, #-0.5 | st r3, [063a]
mul r1, r4, #-16.0 | subxy r0, r0, r3 | ld r4, [a163]
mul r0, r0, r0 | mul r7, r2, #4.0 | st r1, [0404] | ld r1, [0637]
addxy r3, r1, #-0.999 | ld r2, [0639] | st r7, [0632]
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c043]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0638]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [063f]
mad r4, r3, #1.25 | ld r3, [a16b]
subxy r4, r1, r4 | st r3, [0640]
mad r1, r4, r0 | ld r0, [0641] | ld r4, [a163]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [0636] | ld r1, [063d]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4 | mov r5, #0.0
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [063e]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a3]
min r1, r1, r6 | ld r0, [a154]
addxy r7, r4, #-4.75 | st r1, [063c]
mul r4, r3, r7 | mov r6, #1.0 | ld r1, [063c] | ld r7, [c060] | st r0, [0646]
mul r3, r1, r2 | mov r2, #0.041666666666666664
mad r3, r4, #0.125
mul r1, r3, #0.17328679513998632
ymacd r2, r1, #0.16666666666666666
ymacd r2, r1, #0.5
ymacd r2, r1, #1.0
ymacd r2, r1, #1.0
mul r4, r2, r2
mul r3, r4, r4
mul r1, r3, r3
mul r2, r1, r1
mul r4, r2, r2 | ld r2, [c063]
mul r3, r4, #0.0009765625
mul r1, r7, r3
min r4, r1, #0.00053710938664153218 | ld r1, [0643]
mul r7, r4, #256.0 | ld r4, [0645]
mul r3, r7, #4.0
mul r7, r3, r4
addxy r7, r7, r1
mov r1, r7
mad r1, r0, #-0.125
mul r0, r2, r4
mad r0, r1, #0.5
mul r1, r3, r0
mad r1, r4, #-0.5
mul r0, r3, r1
mad r0, r7, #-0.5
mul r4, r0, #-2.0
mov r7, r0 | st r4, [0642] | ld r4, [0646]
mad r7, r4, #0.0625
mul r4, r2, r1
mad r4, r7, #0.5
mul r2, r3, r4 | ld r3, [0636]
mul r4, r3, r0 | ld r3, [a16c] | ld r0, [064d]
mad r2, r1, #-0.5 | st r3, [064c]
mul r1, r4, #-16.0 | subxy r0, r0, r3 | ld r4, [a164]
mul r0, r0, r0 | mul r7, r2, #4.0 | st r1, [0406] | ld r1, [0649]
addxy r3, r1, #-0.999 | ld r2, [064b] | st r7, [0644]
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c043]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [064a]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0651]
mad r4, r3, #1.25 | ld r3, [a16c]
subxy r4, r1, r4 | st r3, [0652]
mad r1, r4, r0 | ld r0, [0653] | ld r4, [a164]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [0648] | ld r1, [064f]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4 | mov r5, #0.0
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0650]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a4]
min r1, r1, r6 | ld r0, [a155]
addxy r7, r4, #-4.75 | st r1, [064e]
mul r4, r3, r7 | mov r6, #1.0 | ld r1, [064e] | ld r7, [c060] | st r0, [0658]
mul r3, r1, r2 | mov r2, #0.041666666666666664
mad r3, r4, #0.125
mul r1, r3, #0.17328679513998632
ymacd r2, r1, #0.16666666666666666
ymacd r2, r1, #0.5
ymacd r2, r1, #1.0
ymacd r2, r1, #1.0
mul r4, r2, r2
mul r3, r4, r4
mul r1, r3, r3
mul r2, r1, r1
mul r4, r2, r2 | ld r2, [c063]
mul r3, r4, #0.0009765625
mul r1, r7, r3
min r4, r1, #0.00053710938664153218 | ld r1, [0655]
mul r7, r4, #256.0 | ld r4, [0657]
mul r3, r7, #4.0
mul r7, r3, r4
addxy r7, r7, r1
mov r1, r7
mad r1, r0, #-0.125
mul r0, r2, r4
mad r0, r1, #0.5
mul r1, r3, r0
mad r1, r4, #-0.5
mul r0, r3, r1
mad r0, r7, #-0.5
mul r4, r0, #-2.0
mov r7, r0 | st r4, [0654] | ld r4, [0658]
mad r7, r4, #0.0625
mul r4, r2, r1
mad r4, r7, #0.5
mul r2, r3, r4 | ld r3, [0648]
mul r4, r3, r0 | ld r3, [a16d] | ld r0, [065f]
mad r2, r1, #-0.5 | st r3, [065e]
mul r1, r4, #-16.0 | subxy r0, r0, r3 | ld r4, [a165]
mul r0, r0, r0 | mul r7, r2, #4.0 | st r1, [0408] | ld r1, [065b]
addxy r3, r1, #-0.999 | ld r2, [065d] | st r7, [0656]
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c043]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [065c]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0663]
mad r4, r3, #1.25 | ld r3, [a16d]
subxy r4, r1, r4 | st r3, [0664]
mad r1, r4, r0 | ld r0, [0665] | ld r4, [a165]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [065a] | ld r1, [0661]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4 | mov r5, #0.0
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0662]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a5]
min r1, r1, r6 | ld r0, [a156]
addxy r7, r4, #-4.75 | st r1, [0660]
mul r4, r3, r7 | mov r6, #1.0 | ld r1, [0660] | ld r7, [c060] | st r0, [066a]
mul r3, r1, r2 | mov r2, #0.041666666666666664
mad r3, r4, #0.125
mul r1, r3, #0.17328679513998632
ymacd r2, r1, #0.16666666666666666
ymacd r2, r1, #0.5
ymacd r2, r1, #1.0
ymacd r2, r1, #1.0
mul r4, r2, r2
mul r3, r4, r4
mul r1, r3, r3
mul r2, r1, r1
mul r4, r2, r2 | ld r2, [c063]
mul r3, r4, #0.0009765625
mul r1, r7, r3
min r4, r1, #0.00053710938664153218 | ld r1, [0667]
mul r7, r4, #256.0 | ld r4, [0669]
mul r3, r7, #4.0
mul r7, r3, r4
addxy r7, r7, r1
mov r1, r7
mad r1, r0, #-0.125
mul r0, r2, r4
mad r0, r1, #0.5
mul r1, r3, r0
mad r1, r4, #-0.5
mul r0, r3, r1
mad r0, r7, #-0.5
mul r4, r0, #-2.0
mov r7, r0 | st r4, [0666] | ld r4, [066a]
mad r7, r4, #0.0625
mul r4, r2, r1
mad r4, r7, #0.5
mul r2, r3, r4 | ld r3, [065a]
mul r4, r3, r0 | ld r3, [a16e] | ld r0, [0671]
mad r2, r1, #-0.5 | st r3, [0670]
mul r1, r4, #-16.0 | subxy r0, r0, r3 | ld r4, [a166]
mul r0, r0, r0 | mul r7, r2, #4.0 | st r1, [040a] | ld r1, [066d]
addxy r3, r1, #-0.999 | ld r2, [066f] | st r7, [0668]
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c043]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [066e]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0675]
mad r4, r3, #1.25 | ld r3, [a16e]
subxy r4, r1, r4 | st r3, [0676]
mad r1, r4, r0 | ld r0, [0677] | ld r4, [a166]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [066c] | ld r1, [0673]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4 | mov r5, #0.0
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0674]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a6]
min r1, r1, r6 | ld r0, [a157]
addxy r7, r4, #-4.75 | st r1, [0672]
mul r4, r3, r7 | mov r6, #1.0 | ld r1, [0672] | ld r7, [c060] | st r0, [067c]
mul r3, r1, r2 | mov r2, #0.041666666666666664
mad r3, r4, #0.125
mul r1, r3, #0.17328679513998632
ymacd r2, r1, #0.16666666666666666
ymacd r2, r1, #0.5
ymacd r2, r1, #1.0
ymacd r2, r1, #1.0
mul r4, r2, r2
mul r3, r4, r4
mul r1, r3, r3
mul r2, r1, r1
mul r4, r2, r2 | ld r2, [c063]
mul r3, r4, #0.0009765625
mul r1, r7, r3
min r4, r1, #0.00053710938664153218 | ld r1, [0679]
mul r7, r4, #256.0 | ld r4, [067b]
mul r3, r7, #4.0
mul r7, r3, r4
addxy r7, r7, r1
mov r1, r7
mad r1, r0, #-0.125
mul r0, r2, r4
mad r0, r1, #0.5
mul r1, r3, r0
mad r1, r4, #-0.5
mul r0, r3, r1
mad r0, r7, #-0.5
mul r4, r0, #-2.0
mov r7, r0 | st r4, [0678] | ld r4, [067c]
mad r7, r4, #0.0625
mul r4, r2, r1
mad r4, r7, #0.5
mul r2, r3, r4 | ld r3, [066c]
mul r4, r3, r0 | ld r3, [a16f] | ld r0, [0683]
mad r2, r1, #-0.5 | st r3, [0682]
mul r1, r4, #-16.0 | subxy r0, r0, r3 | ld r4, [a167]
mul r0, r0, r0 | mul r7, r2, #4.0 | st r1, [040c] | ld r1, [067f]
addxy r3, r1, #-0.999 | ld r2, [0681] | st r7, [067a]
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c043]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0680]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0687]
mad r4, r3, #1.25 | ld r3, [a16f]
subxy r4, r1, r4 | st r3, [0688]
mad r1, r4, r0 | ld r0, [0689] | ld r4, [a167]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [067e] | ld r1, [0685]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0686]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [c061]
mad r4, r3, #1.25 | ld r3, [c062]
subxy r4, r1, r4
mad r1, r4, r0 | ld r4, [a1a7]
min r1, r1, r6 | ld r0, [a158]
addxy r7, r4, #-4.75 | st r1, [0684]
mul r4, r3, r7 | ld r1, [0684] | ld r7, [c060] | st r0, [068e]
mul r3, r1, r2 | mov r2, #0.041666666666666664
mad r3, r4, #0.125
mul r1, r3, #0.17328679513998632
ymacd r2, r1, #0.16666666666666666
ymacd r2, r1, #0.5
ymacd r2, r1, #1.0
ymacd r2, r1, #1.0
mul r4, r2, r2
mul r3, r4, r4
mul r1, r3, r3
mul r2, r1, r1
mul r4, r2, r2 | ld r2, [c063]
mul r3, r4, #0.0009765625
mul r1, r7, r3
min r4, r1, #0.00053710938664153218 | ld r1, [068b]
mul r7, r4, #256.0 | ld r4, [068d]
mul r3, r7, #4.0
mul r7, r3, r4
addxy r7, r7, r1
mov r1, r7
mad r1, r0, #-0.125
mul r0, r2, r4
mad r0, r1, #0.5
mul r1, r3, r0
mad r1, r4, #-0.5
mul r0, r3, r1
mad r0, r7, #-0.5
mul r4, r0, #-2.0
mov r7, r0 | st r4, [068a] | ld r4, [068e]
mad r7, r4, #0.0625
mul r4, r2, r1
mad r4, r7, #0.5
mul r2, r3, r4 | ld r3, [067e]
mad r2, r1, #-0.5
mul r4, r3, r0 | mul r7, r2, #4.0
mul r1, r4, #-16.0 | st r7, [068c]
st r1, [040e]
