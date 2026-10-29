mov r7, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r6, [c000]
addxy r7, r7, r0 | ld r0, [0405] | ld r2, [06bf]
addxy r7, r7, r1 | mul r2, r2, #7.31275 | ld r1, [0407]
addxy r7, r7, r0 | ld r0, [0409]
addxy r7, r7, r1 | ld r1, [060d]
addxy r7, r7, r0 | mov r0, #0.0
mul r7, r7, #0.26666666666666666 | st r0, [6008]
mul r7, r7, r6 | st r0, [6009]
st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [06c1]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [06c0] | ld r0, [a168] | ld r7, [c061]
subxy r1, r1, r0 | st r0, [060c] | ld r0, [060f]
ph_wrap r2, r3, r2
mul r1, r1, r1 | st r2, [06be] | ld r2, [c048]
mad r0, r0, -r1 | st r1, [061a] | ld r1, [0611]
addxy r2, r0, r2 | ld r3, [06be]
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
mul r1, r1, r2
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [0618] | ld r7, [c060]
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
shl r4, r7, 14 | ld r7, [0623]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [0620]
mad r3, r0, #0.03125
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [0624]
mul r0, r4, r7 | mul r3, r1, #4.0 | ld r1, [0633] | ld r7, [c061]
mul r2, r0, #-32.0 | ld r0, [a169] | st r3, [0622]
subxy r1, r1, r0 | st r0, [0632] | ld r0, [0635]
mul r1, r1, r1 | st r2, [0400] | ld r2, [c048] | ld r3, [06be]
mad r0, r0, -r1 | st r1, [0640] | ld r1, [0637]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [0634]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [0639]
subxy r1, r3, r1 | st r1, [0636]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [0638] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [063b] | ld r2, [a161]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [063a]
mul r1, r3, r1 | st r3, [a121]
mad r1, r0, -r2 | ld r2, [0645] | ld r0, [0643]
add r1, #1.0
st r1, [063c] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [063e] | ld r1, [0640]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0644]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [062b]
subxy r2, r0, r2
mad r0, r2, r1 | ld r2, [a169] | ld r1, [0629]
min r0, r0, #1.0 | st r2, [062a]
subxy r3, r3, r2 | st r0, [0642] | st r0, [a129] | ld r0, [0627]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c043]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a161]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0628]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0627]
mad r1, r2, #1.25 | ld r2, [a169]
subxy r1, r0, r1 | st r2, [0630]
mad r0, r1, r3 | ld r3, [0631] | ld r1, [062f]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0626] | ld r0, [062d]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a161]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [062e]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [062d]
mad r1, r2, #1.25 | ld r2, [c05a]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [a121] | ld r3, [c055]
min r0, r0, #1.0
st r0, [062c] | ld r0, [c054]
mad r0, r1, r3 | ld r3, [c05b] | ld r4, [062c]
mad r2, r1, r3 | ld r1, [0642] | ld r3, [c056]
mad r0, r1, r3 | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [062c] | ld r3, [c057]
mad r0, r1, r3 | ld r3, [c05d]
mad r2, r1, r3 | ld r1, [0626] | ld r3, [c058]
mad r0, r1, r3 | ld r3, [c05e]
mad r2, r1, r3 | ld r1, [a131] | ld r3, [c059]
mad r0, r1, r3 | ld r3, [c05f]
mad r2, r1, r3 | ld r1, [0626] | st r0, [a139]
st r2, [a141] | ld r2, [063c]
mul r3, r1, r2 | ld r2, [a1b1] | ld r1, [c062]
addxy r2, r2, #-4.75 | st r3, [064a] | ld r0, [a152]
mul r1, r1, r2
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [063e] | ld r7, [c060]
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [0647]
shl r4, r7, 14 | ld r7, [0649]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [0646]
mad r3, r0, #0.03125
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [064a]
mul r0, r4, r7 | mul r3, r1, #4.0 | ld r1, [0659] | ld r7, [c061]
mul r2, r0, #-32.0 | ld r0, [a16a] | st r3, [0648]
subxy r1, r1, r0 | st r0, [0658] | ld r0, [065b]
mul r1, r1, r1 | st r2, [0402] | ld r2, [c048] | ld r3, [06be]
mad r0, r0, -r1 | st r1, [0666] | ld r1, [065d]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [065a]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [065f]
subxy r1, r3, r1 | st r1, [065c]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [065e] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0661] | ld r2, [a162]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0660]
mul r1, r3, r1 | st r3, [a122]
mad r1, r0, -r2 | ld r2, [066b] | ld r0, [0669]
add r1, #1.0
st r1, [0662] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0664] | ld r1, [0666]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [066a]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [0651]
subxy r2, r0, r2
mad r0, r2, r1 | ld r2, [a16a] | ld r1, [064f]
min r0, r0, #1.0 | st r2, [0650]
subxy r3, r3, r2 | st r0, [0668] | st r0, [a12a] | ld r0, [064d]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c043]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a162]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [064e]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [064d]
mad r1, r2, #1.25 | ld r2, [a16a]
subxy r1, r0, r1 | st r2, [0656]
mad r0, r1, r3 | ld r3, [0657] | ld r1, [0655]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [064c] | ld r0, [0653]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a162]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0654]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0653]
mad r1, r2, #1.25 | ld r2, [c05a]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [a122] | ld r3, [c055]
min r0, r0, #1.0
st r0, [0652] | ld r0, [c054]
mad r0, r1, r3 | ld r3, [c05b] | ld r4, [0652]
mad r2, r1, r3 | ld r1, [0668] | ld r3, [c056]
mad r0, r1, r3 | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [0652] | ld r3, [c057]
mad r0, r1, r3 | ld r3, [c05d]
mad r2, r1, r3 | ld r1, [064c] | ld r3, [c058]
mad r0, r1, r3 | ld r3, [c05e]
mad r2, r1, r3 | ld r1, [a132] | ld r3, [c059]
mad r0, r1, r3 | ld r3, [c05f]
mad r2, r1, r3 | ld r1, [064c] | st r0, [a13a]
st r2, [a142] | ld r2, [0662]
mul r3, r1, r2 | ld r2, [a1b2] | ld r1, [c062]
addxy r2, r2, #-4.75 | st r3, [0670] | ld r0, [a153]
mul r1, r1, r2
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [0664] | ld r7, [c060]
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [066d]
shl r4, r7, 14 | ld r7, [066f]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [066c]
mad r3, r0, #0.03125
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [0670]
mul r0, r4, r7 | mul r3, r1, #4.0 | ld r1, [067f] | ld r7, [c061]
mul r2, r0, #-32.0 | ld r0, [a16b] | st r3, [066e]
subxy r1, r1, r0 | st r0, [067e] | ld r0, [0681]
mul r1, r1, r1 | st r2, [0404] | ld r2, [c048] | ld r3, [06be]
mad r0, r0, -r1 | st r1, [068c] | ld r1, [0683]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [0680]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [0685]
subxy r1, r3, r1 | st r1, [0682]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [0684] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0687] | ld r2, [a163]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0686]
mul r1, r3, r1 | st r3, [a123]
mad r1, r0, -r2 | ld r2, [0691] | ld r0, [068f]
add r1, #1.0
st r1, [0688] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [068a] | ld r1, [068c]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0690]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [0677]
subxy r2, r0, r2
mad r0, r2, r1 | ld r2, [a16b] | ld r1, [0675]
min r0, r0, #1.0 | st r2, [0676]
subxy r3, r3, r2 | st r0, [068e] | st r0, [a12b] | ld r0, [0673]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c043]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a163]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0674]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0673]
mad r1, r2, #1.25 | ld r2, [a16b]
subxy r1, r0, r1 | st r2, [067c]
mad r0, r1, r3 | ld r3, [067d] | ld r1, [067b]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0672] | ld r0, [0679]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a163]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [067a]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0679]
mad r1, r2, #1.25 | ld r2, [c05a]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [a123] | ld r3, [c055]
min r0, r0, #1.0
st r0, [0678] | ld r0, [c054]
mad r0, r1, r3 | ld r3, [c05b] | ld r4, [0678]
mad r2, r1, r3 | ld r1, [068e] | ld r3, [c056]
mad r0, r1, r3 | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [0678] | ld r3, [c057]
mad r0, r1, r3 | ld r3, [c05d]
mad r2, r1, r3 | ld r1, [0672] | ld r3, [c058]
mad r0, r1, r3 | ld r3, [c05e]
mad r2, r1, r3 | ld r1, [a133] | ld r3, [c059]
mad r0, r1, r3 | ld r3, [c05f]
mad r2, r1, r3 | ld r1, [0672] | st r0, [a13b]
st r2, [a143] | ld r2, [0688]
mul r3, r1, r2 | ld r2, [a1b3] | ld r1, [c062]
addxy r2, r2, #-4.75 | st r3, [0696] | ld r0, [a154]
mul r1, r1, r2
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [068a] | ld r7, [c060]
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [0693]
shl r4, r7, 14 | ld r7, [0695]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [0692]
mad r3, r0, #0.03125
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [0696]
mul r0, r4, r7 | mul r3, r1, #4.0 | ld r1, [06a5] | ld r7, [c061]
mul r2, r0, #-32.0 | ld r0, [a16c] | st r3, [0694]
subxy r1, r1, r0 | st r0, [06a4] | ld r0, [06a7]
mul r1, r1, r1 | st r2, [0406] | ld r2, [c048] | ld r3, [06be]
mad r0, r0, -r1 | st r1, [06b2] | ld r1, [06a9]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [06a6]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [06ab]
subxy r1, r3, r1 | st r1, [06a8]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [06aa] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [06ad] | ld r2, [a164]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [06ac]
mul r1, r3, r1 | st r3, [a124]
mad r1, r0, -r2 | ld r2, [06b7] | ld r0, [06b5]
add r1, #1.0
st r1, [06ae] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [06b0] | ld r1, [06b2]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [06b6]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [069d]
subxy r2, r0, r2
mad r0, r2, r1 | ld r2, [a16c] | ld r1, [069b]
min r0, r0, #1.0 | st r2, [069c]
subxy r3, r3, r2 | st r0, [06b4] | st r0, [a12c] | ld r0, [0699]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c043]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a164]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [069a]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0699]
mad r1, r2, #1.25 | ld r2, [a16c]
subxy r1, r0, r1 | st r2, [06a2]
mad r0, r1, r3 | ld r3, [06a3] | ld r1, [06a1]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0698] | ld r0, [069f]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a164]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [06a0]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [069f]
mad r1, r2, #1.25 | ld r2, [c05a]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [a124] | ld r3, [c055]
min r0, r0, #1.0
st r0, [069e] | ld r0, [c054]
mad r0, r1, r3 | ld r3, [c05b] | ld r4, [069e]
mad r2, r1, r3 | ld r1, [06b4] | ld r3, [c056]
mad r0, r1, r3 | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [069e] | ld r3, [c057]
mad r0, r1, r3 | ld r3, [c05d]
mad r2, r1, r3 | ld r1, [0698] | ld r3, [c058]
mad r0, r1, r3 | ld r3, [c05e]
mad r2, r1, r3 | ld r1, [a134] | ld r3, [c059]
mad r0, r1, r3 | ld r3, [c05f]
mad r2, r1, r3 | ld r1, [0698] | st r0, [a13c]
st r2, [a144] | ld r2, [06ae]
mul r3, r1, r2 | ld r2, [a1b4] | ld r1, [c062]
addxy r2, r2, #-4.75 | st r3, [06bc] | ld r0, [a155]
mul r1, r1, r2
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [06b0] | ld r7, [c060]
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [06b9]
shl r4, r7, 14 | ld r7, [06bb]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [06b8]
mad r3, r0, #0.03125
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [06bc]
mul r0, r4, r7 | mul r3, r1, #4.0
mul r2, r0, #-32.0 | st r3, [06ba]
st r2, [0408]
