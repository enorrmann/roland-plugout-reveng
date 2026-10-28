mov r7, #0.0 | mov r5, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r3, [a168]
addxy r7, r7, r0 | ld r0, [0405] | st r3, [0604] | ld r6, [c000]
addxy r7, r7, r1 | ld r1, [0407] | ld r2, [0603] | ld r4, [a160]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | st r0, [6008] | ld r1, [0601]
mul r7, r7, #0.3333333333333333 | st r0, [6009]
mul r7, r7, r6 | mov r6, #1.0 | st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [0605]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r7, [6006]
mul r3, r3, #256.0 | mul r0, r0, r0 | st r7, [6007]
mul r3, r3, #256.0 | subxy r0, r0, #1.0 | ld r7, [c061]
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
mul r2, r2, r4 | mov r5, #0.0
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0608]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [a1a0]
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0 | ld r0, [a151]
min r1, r1, r6 | st r0, [0612]
mov r6, #1.0 | st r1, [0606] | st r1, [a178] | ld r1, [0600]
mul r3, r1, r2 | ld r2, [a1b0] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [0606] | st r3, [0610]
mul r1, r1, r2 | mul r2, r4, r7 | ld r4, [a1a8]
mad r2, r1, #0.125 | ld r7, [c060]
mad r2, r4, #0.125 | mov r4, #0.041666666666666664
mul r1, r2, #0.17328679513998632
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, r1
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, #6.103515625e-05 | ld r1, [c063]
mul r2, r7, r4
min r7, r2, #3.3569336665095761e-05
mul r4, r7, #256.0 | ld r7, [060d]
mul r2, r4, #64.0 | ld r4, [060f]
mul r3, r2, r4
addxy r3, r3, r7
mov r7, r3
mad r7, r0, #-0.0625 | mul r0, r1, r4
mad r0, r7, #0.5
mul r7, r2, r0
mad r7, r4, #-0.5
mul r0, r2, r7
mad r0, r3, #-0.5
mul r4, r0, #-2.0 | mov r3, r0
st r4, [060c] | ld r4, [0612]
mad r3, r4, #0.03125 | mul r4, r1, r7
mad r4, r3, #0.5
mul r1, r2, r4 | ld r2, [0610]
mad r1, r7, #-0.5 | mul r4, r2, r0 | ld r0, [0619] | ld r2, [0617]
mul r3, r1, #4.0 | ld r1, [0615]
mul r7, r4, #-32.0 | st r3, [060e] | ld r3, [a169]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [0618] | ld r4, [a161]
mul r3, r3, #256.0 | mul r0, r0, r0 | st r7, [0400] | ld r7, [c061]
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0616]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [061d]
mad r4, r3, #1.25 | ld r3, [a169]
subxy r4, r1, r4 | st r3, [061e]
mad r1, r4, r0 | ld r0, [061f] | ld r4, [a161]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [0614] | st r1, [a171] | ld r1, [061b]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4 | mov r5, #0.0
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [061c]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [a1a1]
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0 | ld r0, [a152]
min r1, r1, r6 | st r0, [0626]
mov r6, #1.0 | st r1, [061a] | st r1, [a179] | ld r1, [0614]
mul r3, r1, r2 | ld r2, [a1b1] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [061a] | st r3, [0624]
mul r1, r1, r2 | mul r2, r4, r7 | ld r4, [a1a9]
mad r2, r1, #0.125 | ld r7, [c060]
mad r2, r4, #0.125 | mov r4, #0.041666666666666664
mul r1, r2, #0.17328679513998632
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, r1
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, #6.103515625e-05 | ld r1, [c063]
mul r2, r7, r4
min r7, r2, #3.3569336665095761e-05
mul r4, r7, #256.0 | ld r7, [0621]
mul r2, r4, #64.0 | ld r4, [0623]
mul r3, r2, r4
addxy r3, r3, r7
mov r7, r3
mad r7, r0, #-0.0625 | mul r0, r1, r4
mad r0, r7, #0.5
mul r7, r2, r0
mad r7, r4, #-0.5
mul r0, r2, r7
mad r0, r3, #-0.5
mul r4, r0, #-2.0 | mov r3, r0
st r4, [0620] | ld r4, [0626]
mad r3, r4, #0.03125 | mul r4, r1, r7
mad r4, r3, #0.5
mul r1, r2, r4 | ld r2, [0624]
mad r1, r7, #-0.5 | mul r4, r2, r0 | ld r0, [062d] | ld r2, [062b]
mul r3, r1, #4.0 | ld r1, [0629]
mul r7, r4, #-32.0 | st r3, [0622] | ld r3, [a16a]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [062c] | ld r4, [a162]
mul r3, r3, #256.0 | mul r0, r0, r0 | st r7, [0402] | ld r7, [c061]
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [062a]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0631]
mad r4, r3, #1.25 | ld r3, [a16a]
subxy r4, r1, r4 | st r3, [0632]
mad r1, r4, r0 | ld r0, [0633] | ld r4, [a162]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [0628] | st r1, [a172] | ld r1, [062f]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4 | mov r5, #0.0
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0630]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [a1a2]
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0 | ld r0, [a153]
min r1, r1, r6 | st r0, [063a]
mov r6, #1.0 | st r1, [062e] | st r1, [a17a] | ld r1, [0628]
mul r3, r1, r2 | ld r2, [a1b2] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [062e] | st r3, [0638]
mul r1, r1, r2 | mul r2, r4, r7 | ld r4, [a1aa]
mad r2, r1, #0.125 | ld r7, [c060]
mad r2, r4, #0.125 | mov r4, #0.041666666666666664
mul r1, r2, #0.17328679513998632
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, r1
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, #6.103515625e-05 | ld r1, [c063]
mul r2, r7, r4
min r7, r2, #3.3569336665095761e-05
mul r4, r7, #256.0 | ld r7, [0635]
mul r2, r4, #64.0 | ld r4, [0637]
mul r3, r2, r4
addxy r3, r3, r7
mov r7, r3
mad r7, r0, #-0.0625 | mul r0, r1, r4
mad r0, r7, #0.5
mul r7, r2, r0
mad r7, r4, #-0.5
mul r0, r2, r7
mad r0, r3, #-0.5
mul r4, r0, #-2.0 | mov r3, r0
st r4, [0634] | ld r4, [063a]
mad r3, r4, #0.03125 | mul r4, r1, r7
mad r4, r3, #0.5
mul r1, r2, r4 | ld r2, [0638]
mad r1, r7, #-0.5 | mul r4, r2, r0 | ld r0, [0641] | ld r2, [063f]
mul r3, r1, #4.0 | ld r1, [063d]
mul r7, r4, #-32.0 | st r3, [0636] | ld r3, [a16b]
subxy r0, r0, r3 | addxy r3, r1, #-0.999 | st r3, [0640] | ld r4, [a163]
mul r3, r3, #256.0 | mul r0, r0, r0 | st r7, [0404] | ld r7, [c061]
mul r3, r3, #256.0 | subxy r0, r0, #1.0
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [063e]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0645]
mad r4, r3, #1.25 | ld r3, [a16b]
subxy r4, r1, r4 | st r3, [0646]
mad r1, r4, r0 | ld r0, [0647] | ld r4, [a163]
min r1, r1, r6
subxy r0, r0, r3 | st r1, [063c] | st r1, [a173] | ld r1, [0643]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c047]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0644]
mul r0, r0, r4 | ld r4, [c045]
mad r0, r3, r4 | ld r4, [c046]
mad r0, r2, r4 | ld r4, [c044]
mul r4, r4, r2 | ld r2, [a1a3]
mad r4, r3, #1.25
subxy r4, r1, r4
mad r1, r4, r0 | ld r0, [a154]
min r1, r1, r6 | st r0, [064e]
st r1, [0642] | st r1, [a17b] | ld r1, [063c]
mul r3, r1, r2 | ld r2, [a1b3] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [0642] | st r3, [064c]
mul r1, r1, r2 | mul r2, r4, r7 | ld r4, [a1ab]
mad r2, r1, #0.125 | ld r7, [c060]
mad r2, r4, #0.125 | mov r4, #0.041666666666666664
mul r1, r2, #0.17328679513998632
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, r1
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, #6.103515625e-05 | ld r1, [c063]
mul r2, r7, r4
min r7, r2, #3.3569336665095761e-05
mul r4, r7, #256.0 | ld r7, [0649]
mul r2, r4, #64.0 | ld r4, [064b]
mul r3, r2, r4
addxy r3, r3, r7
mov r7, r3
mad r7, r0, #-0.0625 | mul r0, r1, r4
mad r0, r7, #0.5
mul r7, r2, r0
mad r7, r4, #-0.5
mul r0, r2, r7
mad r0, r3, #-0.5
mul r4, r0, #-2.0 | mov r3, r0
st r4, [0648] | ld r4, [064e]
mad r3, r4, #0.03125 | mul r4, r1, r7
mad r4, r3, #0.5
mul r1, r2, r4 | ld r2, [064c]
mad r1, r7, #-0.5 | mul r4, r2, r0
mul r3, r1, #4.0
mul r7, r4, #-32.0 | st r3, [064a]
st r7, [0406]
