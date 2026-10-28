mov r7, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r6, [c000]
addxy r7, r7, r0 | ld r0, [0405] | ld r2, [06c9]
addxy r7, r7, r1 | mul r2, r2, #7.31275 | ld r1, [0407]
addxy r7, r7, r0 | ld r0, [0409]
addxy r7, r7, r1 | ld r1, [060d]
addxy r7, r7, r0 | mov r0, #0.0
mul r7, r7, #0.26666666666666666 | st r0, [6008]
mul r7, r7, r6 | st r0, [6009]
st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [06cb]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [06ca] | ld r0, [a168] | ld r7, [c061]
subxy r1, r1, r0 | st r0, [060c] | ld r0, [060f]
ph_wrap r2, r3, r2
mul r1, r1, r1 | st r2, [06c8] | ld r2, [c048]
mad r0, r0, -r1 | st r1, [061a] | ld r1, [0611]
addxy r2, r0, r2 | ld r3, [06c8]
ph_wrap r0, r3, r2
subxy r2, r0, r2 | subxy r3, r1, r3 | st r0, [060e]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [0613]
subxy r1, r3, r1 | st r1, [0610]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [0612] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0615] | ld r2, [a160]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0614]
mul r1, r3, r1 | st r3, [a120]
mad r1, r0, -r2 | ld r2, [061f] | ld r0, [061d]
add r1, #1.0
st r1, [0616] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0618] | ld r1, [061a]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [061e]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [0605]
subxy r2, r0, r2
mad r0, r2, r1 | ld r2, [a168] | ld r1, [0603]
min r0, r0, #1.0 | st r2, [0604]
subxy r3, r3, r2 | st r0, [061c] | st r0, [a128] | ld r0, [0601]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c043]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a160]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0602]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0601]
mad r1, r2, #1.25 | ld r2, [a168]
subxy r1, r0, r1 | st r2, [060a]
mad r0, r1, r3 | ld r3, [060b] | ld r1, [0609]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0600] | ld r0, [0607]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a160]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0608]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0607]
mad r1, r2, #1.25 | ld r2, [c05a]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [a120] | ld r3, [c055]
min r0, r0, #1.0
st r0, [0606] | ld r0, [c054]
mad r0, r1, r3 | ld r3, [c05b] | ld r4, [0606]
mad r2, r1, r3 | ld r1, [061c] | ld r3, [c056]
mad r0, r1, r3 | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [0606] | ld r3, [c057]
mad r0, r1, r3 | ld r3, [c05d]
mad r2, r1, r3 | ld r1, [0600] | ld r3, [c058]
mad r0, r1, r3 | ld r3, [c05e]
mad r2, r1, r3 | ld r1, [a130] | ld r3, [c059]
mad r0, r1, r3 | ld r3, [c05f]
mad r2, r1, r3 | ld r1, [0600] | st r0, [a138]
st r2, [a140] | ld r2, [0616]
mul r3, r1, r2 | ld r2, [a1b0] | ld r1, [c062]
addxy r2, r2, #-4.75 | st r3, [0624] | ld r0, [a151]
mul r1, r1, r2 | mul r2, r4, r7 | ld r4, [0618] | ld r7, [c060] | st r0, [0626]
mad r2, r1, #0.125
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [0621]
shl r4, r7, -r6 | ld r7, [0623]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625 | mul r0, r1, r7
mad r0, r2, #0.5
mul r2, r4, r0
mad r2, r7, #-0.5
mul r0, r4, r2
mad r0, r3, #-0.5
mul r7, r0, #-2.0 | mov r3, r0
st r7, [0620] | ld r7, [0626]
mad r3, r7, #0.03125 | mul r7, r1, r2
mad r7, r3, #0.5
mul r1, r4, r7 | ld r4, [0624]
mad r1, r2, #-0.5 | mul r7, r4, r0 | ld r0, [a169]
mul r3, r1, #4.0 | mul r2, r7, #-32.0 | ld r1, [0635] | st r0, [0634] | ld r7, [c061]
subxy r1, r1, r0 | ld r0, [0637] | st r2, [0400] | ld r2, [c048]
mul r1, r1, r1 | st r3, [0622] | ld r3, [06c8]
mad r0, r0, -r1 | st r1, [0642] | ld r1, [0639]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [0636]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [063b]
subxy r1, r3, r1 | st r1, [0638]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [063a] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [063d] | ld r2, [a161]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [063c]
mul r1, r3, r1 | st r3, [a121]
mad r1, r0, -r2 | ld r2, [0647] | ld r0, [0645]
add r1, #1.0
st r1, [063e] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0640] | ld r1, [0642]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0646]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [062d]
subxy r2, r0, r2
mad r0, r2, r1 | ld r2, [a169] | ld r1, [062b]
min r0, r0, #1.0 | st r2, [062c]
subxy r3, r3, r2 | st r0, [0644] | st r0, [a129] | ld r0, [0629]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c043]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a161]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [062a]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0629]
mad r1, r2, #1.25 | ld r2, [a169]
subxy r1, r0, r1 | st r2, [0632]
mad r0, r1, r3 | ld r3, [0633] | ld r1, [0631]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0628] | ld r0, [062f]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a161]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0630]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [062f]
mad r1, r2, #1.25 | ld r2, [c05a]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [a121] | ld r3, [c055]
min r0, r0, #1.0
st r0, [062e] | ld r0, [c054]
mad r0, r1, r3 | ld r3, [c05b] | ld r4, [062e]
mad r2, r1, r3 | ld r1, [0644] | ld r3, [c056]
mad r0, r1, r3 | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [062e] | ld r3, [c057]
mad r0, r1, r3 | ld r3, [c05d]
mad r2, r1, r3 | ld r1, [0628] | ld r3, [c058]
mad r0, r1, r3 | ld r3, [c05e]
mad r2, r1, r3 | ld r1, [a131] | ld r3, [c059]
mad r0, r1, r3 | ld r3, [c05f]
mad r2, r1, r3 | ld r1, [0628] | st r0, [a139]
st r2, [a141] | ld r2, [063e]
mul r3, r1, r2 | ld r2, [a1b1] | ld r1, [c062]
addxy r2, r2, #-4.75 | st r3, [064c] | ld r0, [a152]
mul r1, r1, r2 | mul r2, r4, r7 | ld r4, [0640] | ld r7, [c060] | st r0, [064e]
mad r2, r1, #0.125
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [0649]
shl r4, r7, -r6 | ld r7, [064b]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625 | mul r0, r1, r7
mad r0, r2, #0.5
mul r2, r4, r0
mad r2, r7, #-0.5
mul r0, r4, r2
mad r0, r3, #-0.5
mul r7, r0, #-2.0 | mov r3, r0
st r7, [0648] | ld r7, [064e]
mad r3, r7, #0.03125 | mul r7, r1, r2
mad r7, r3, #0.5
mul r1, r4, r7 | ld r4, [064c]
mad r1, r2, #-0.5 | mul r7, r4, r0 | ld r0, [a16a]
mul r3, r1, #4.0 | mul r2, r7, #-32.0 | ld r1, [065d] | st r0, [065c] | ld r7, [c061]
subxy r1, r1, r0 | ld r0, [065f] | st r2, [0402] | ld r2, [c048]
mul r1, r1, r1 | st r3, [064a] | ld r3, [06c8]
mad r0, r0, -r1 | st r1, [066a] | ld r1, [0661]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [065e]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [0663]
subxy r1, r3, r1 | st r1, [0660]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [0662] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0665] | ld r2, [a162]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0664]
mul r1, r3, r1 | st r3, [a122]
mad r1, r0, -r2 | ld r2, [066f] | ld r0, [066d]
add r1, #1.0
st r1, [0666] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0668] | ld r1, [066a]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [066e]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [0655]
subxy r2, r0, r2
mad r0, r2, r1 | ld r2, [a16a] | ld r1, [0653]
min r0, r0, #1.0 | st r2, [0654]
subxy r3, r3, r2 | st r0, [066c] | st r0, [a12a] | ld r0, [0651]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c043]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a162]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0652]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0651]
mad r1, r2, #1.25 | ld r2, [a16a]
subxy r1, r0, r1 | st r2, [065a]
mad r0, r1, r3 | ld r3, [065b] | ld r1, [0659]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0650] | ld r0, [0657]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a162]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0658]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0657]
mad r1, r2, #1.25 | ld r2, [c05a]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [a122] | ld r3, [c055]
min r0, r0, #1.0
st r0, [0656] | ld r0, [c054]
mad r0, r1, r3 | ld r3, [c05b] | ld r4, [0656]
mad r2, r1, r3 | ld r1, [066c] | ld r3, [c056]
mad r0, r1, r3 | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [0656] | ld r3, [c057]
mad r0, r1, r3 | ld r3, [c05d]
mad r2, r1, r3 | ld r1, [0650] | ld r3, [c058]
mad r0, r1, r3 | ld r3, [c05e]
mad r2, r1, r3 | ld r1, [a132] | ld r3, [c059]
mad r0, r1, r3 | ld r3, [c05f]
mad r2, r1, r3 | ld r1, [0650] | st r0, [a13a]
st r2, [a142] | ld r2, [0666]
mul r3, r1, r2 | ld r2, [a1b2] | ld r1, [c062]
addxy r2, r2, #-4.75 | st r3, [0674] | ld r0, [a153]
mul r1, r1, r2 | mul r2, r4, r7 | ld r4, [0668] | ld r7, [c060] | st r0, [0676]
mad r2, r1, #0.125
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [0671]
shl r4, r7, -r6 | ld r7, [0673]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625 | mul r0, r1, r7
mad r0, r2, #0.5
mul r2, r4, r0
mad r2, r7, #-0.5
mul r0, r4, r2
mad r0, r3, #-0.5
mul r7, r0, #-2.0 | mov r3, r0
st r7, [0670] | ld r7, [0676]
mad r3, r7, #0.03125 | mul r7, r1, r2
mad r7, r3, #0.5
mul r1, r4, r7 | ld r4, [0674]
mad r1, r2, #-0.5 | mul r7, r4, r0 | ld r0, [a16b]
mul r3, r1, #4.0 | mul r2, r7, #-32.0 | ld r1, [0685] | st r0, [0684] | ld r7, [c061]
subxy r1, r1, r0 | ld r0, [0687] | st r2, [0404] | ld r2, [c048]
mul r1, r1, r1 | st r3, [0672] | ld r3, [06c8]
mad r0, r0, -r1 | st r1, [0692] | ld r1, [0689]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [0686]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [068b]
subxy r1, r3, r1 | st r1, [0688]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [068a] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [068d] | ld r2, [a163]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [068c]
mul r1, r3, r1 | st r3, [a123]
mad r1, r0, -r2 | ld r2, [0697] | ld r0, [0695]
add r1, #1.0
st r1, [068e] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0690] | ld r1, [0692]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0696]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [067d]
subxy r2, r0, r2
mad r0, r2, r1 | ld r2, [a16b] | ld r1, [067b]
min r0, r0, #1.0 | st r2, [067c]
subxy r3, r3, r2 | st r0, [0694] | st r0, [a12b] | ld r0, [0679]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c043]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a163]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [067a]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0679]
mad r1, r2, #1.25 | ld r2, [a16b]
subxy r1, r0, r1 | st r2, [0682]
mad r0, r1, r3 | ld r3, [0683] | ld r1, [0681]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0678] | ld r0, [067f]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a163]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0680]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [067f]
mad r1, r2, #1.25 | ld r2, [c05a]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [a123] | ld r3, [c055]
min r0, r0, #1.0
st r0, [067e] | ld r0, [c054]
mad r0, r1, r3 | ld r3, [c05b] | ld r4, [067e]
mad r2, r1, r3 | ld r1, [0694] | ld r3, [c056]
mad r0, r1, r3 | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [067e] | ld r3, [c057]
mad r0, r1, r3 | ld r3, [c05d]
mad r2, r1, r3 | ld r1, [0678] | ld r3, [c058]
mad r0, r1, r3 | ld r3, [c05e]
mad r2, r1, r3 | ld r1, [a133] | ld r3, [c059]
mad r0, r1, r3 | ld r3, [c05f]
mad r2, r1, r3 | ld r1, [0678] | st r0, [a13b]
st r2, [a143] | ld r2, [068e]
mul r3, r1, r2 | ld r2, [a1b3] | ld r1, [c062]
addxy r2, r2, #-4.75 | st r3, [069c] | ld r0, [a154]
mul r1, r1, r2 | mul r2, r4, r7 | ld r4, [0690] | ld r7, [c060] | st r0, [069e]
mad r2, r1, #0.125
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [0699]
shl r4, r7, -r6 | ld r7, [069b]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625 | mul r0, r1, r7
mad r0, r2, #0.5
mul r2, r4, r0
mad r2, r7, #-0.5
mul r0, r4, r2
mad r0, r3, #-0.5
mul r7, r0, #-2.0 | mov r3, r0
st r7, [0698] | ld r7, [069e]
mad r3, r7, #0.03125 | mul r7, r1, r2
mad r7, r3, #0.5
mul r1, r4, r7 | ld r4, [069c]
mad r1, r2, #-0.5 | mul r7, r4, r0 | ld r0, [a16c]
mul r3, r1, #4.0 | mul r2, r7, #-32.0 | ld r1, [06ad] | st r0, [06ac] | ld r7, [c061]
subxy r1, r1, r0 | ld r0, [06af] | st r2, [0406] | ld r2, [c048]
mul r1, r1, r1 | st r3, [069a] | ld r3, [06c8]
mad r0, r0, -r1 | st r1, [06ba] | ld r1, [06b1]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [06ae]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [06b3]
subxy r1, r3, r1 | st r1, [06b0]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [06b2] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [06b5] | ld r2, [a164]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [06b4]
mul r1, r3, r1 | st r3, [a124]
mad r1, r0, -r2 | ld r2, [06bf] | ld r0, [06bd]
add r1, #1.0
st r1, [06b6] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [06b8] | ld r1, [06ba]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [06be]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [06a5]
subxy r2, r0, r2
mad r0, r2, r1 | ld r2, [a16c] | ld r1, [06a3]
min r0, r0, #1.0 | st r2, [06a4]
subxy r3, r3, r2 | st r0, [06bc] | st r0, [a12c] | ld r0, [06a1]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c043]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a164]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [06a2]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [06a1]
mad r1, r2, #1.25 | ld r2, [a16c]
subxy r1, r0, r1 | st r2, [06aa]
mad r0, r1, r3 | ld r3, [06ab] | ld r1, [06a9]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [06a0] | ld r0, [06a7]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a164]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [06a8]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [06a7]
mad r1, r2, #1.25 | ld r2, [c05a]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [a124] | ld r3, [c055]
min r0, r0, #1.0
st r0, [06a6] | ld r0, [c054]
mad r0, r1, r3 | ld r3, [c05b] | ld r4, [06a6]
mad r2, r1, r3 | ld r1, [06bc] | ld r3, [c056]
mad r0, r1, r3 | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [06a6] | ld r3, [c057]
mad r0, r1, r3 | ld r3, [c05d]
mad r2, r1, r3 | ld r1, [06a0] | ld r3, [c058]
mad r0, r1, r3 | ld r3, [c05e]
mad r2, r1, r3 | ld r1, [a134] | ld r3, [c059]
mad r0, r1, r3 | ld r3, [c05f]
mad r2, r1, r3 | ld r1, [06a0] | st r0, [a13c]
st r2, [a144] | ld r2, [06b6]
mul r3, r1, r2 | ld r2, [a1b4] | ld r1, [c062]
addxy r2, r2, #-4.75 | st r3, [06c4] | ld r0, [a155]
mul r1, r1, r2 | mul r2, r4, r7 | ld r4, [06b8] | ld r7, [c060] | st r0, [06c6]
mad r2, r1, #0.125
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [06c1]
shl r4, r7, -r6 | ld r7, [06c3]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625 | mul r0, r1, r7
mad r0, r2, #0.5
mul r2, r4, r0
mad r2, r7, #-0.5
mul r0, r4, r2
mad r0, r3, #-0.5
mul r7, r0, #-2.0 | mov r3, r0
st r7, [06c0] | ld r7, [06c6]
mad r3, r7, #0.03125 | mul r7, r1, r2
mad r7, r3, #0.5
mul r1, r4, r7 | ld r4, [06c4]
mad r1, r2, #-0.5 | mul r7, r4, r0
mul r3, r1, #4.0 | mul r2, r7, #-32.0
st r3, [06c2] | st r2, [0408]
