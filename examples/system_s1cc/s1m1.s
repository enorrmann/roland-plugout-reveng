mov r7, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r4, [a169]
addxy r7, r7, r0 | ld r0, [0405] | ld r2, [06f1] | ld r6, [c000]
addxy r7, r7, r1 | mul r2, r2, #7.31275 | ld r1, [0407] | ld r5, [0635]
addxy r7, r7, r0 | subxy r5, r5, r4 | ld r0, [0409] | st r4, [0634]
addxy r7, r7, r1 | mul r5, r5, r5 | ld r1, [040b] | ld r4, [0637]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | st r0, [6008] | ld r1, [060d]
mul r7, r7, #0.2222222222222222 | mad r4, r4, -r5 | st r0, [6009]
mul r7, r7, r6 | st r0, [600a] | ld r5, [0639]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [06f3]
add r0, #0.6184303127229214:32 | ld r6, [c048]
ph_wrap r0, r3, r0 | st r7, [6006]
addxy r2, r2, r0 | addxy r6, r4, r6 | st r0, [06f2] | ld r0, [a168]
subxy r1, r1, r0 | st r0, [060c] | ld r0, [060f]
ph_wrap r2, r3, r2 | st r7, [6007]
mul r1, r1, r1 | st r2, [06f0] | ld r2, [c048]
mad r0, r0, -r1 | ld r1, [0611] | ld r3, [06f0]
addxy r2, r0, r2 | subxy r3, r1, r3 | ld r7, [06f0]
ph_wrap r0, r3, r2
ph_wrap r4, r3, r6 | st r0, [060e]
subxy r2, r0, r2 | subxy r6, r4, r6 | st r4, [0636]
subxy r7, r5, r7 | mul r3, r3, r2 | ld r2, [c049]
mul r7, r7, r6 | mad r1, r3, #0.5 | ld r3, [0613] | ld r6, [c049]
mad r5, r7, #0.5 | subxy r1, r3, r1 | ld r7, [063b] | st r1, [0610]
subxy r5, r7, r5 | mad r3, r1, r2 | st r5, [0638] | ld r1, [c04d]
mad r7, r5, r6 | mul r3, r3, r1 | ld r5, [c04d] | st r3, [0612]
mul r7, r7, r5 | st r7, [063a] | ld r1, [c04c]
mul r2, r0, r1 | ph_tri r0, r3, -r0 | ld r5, [c04c]
mul r6, r4, r5 | ld r1, [c04b]
ph_clamp r2, r3, -r2 | ld r5, [c04b]
ph_clamp r6, r3, -r6
ph_tri r4, r3, -r4
addxy r3, r3, r2 | addxy r7, r7, r6
mul r2, r0, r0 | mul r6, r4, r4
mul r2, r2, r1 | mul r6, r6, r5 | ld r1, [c04a]
addxy r2, r2, r1 | ld r5, [c04a]
addxy r6, r6, r5 | mad r3, r0, r2 | ld r0, [0615] | ld r1, [c04e]
mad r7, r4, r6 | addxy r0, r0, r1 | ld r4, [063d] | ld r5, [c04e]
addxy r4, r4, r5 | ld r2, [a160] | ld r1, [c050]
mul r0, r0, r2 | ld r6, [a161] | ld r5, [c050]
mul r4, r4, r6 | ld r2, [c051]
min r0, r0, #1.0 | ld r6, [c051]
min r4, r4, #1.0 | st r0, [0614]
mul r3, r3, r0 | mul r7, r7, r4 | st r4, [063c]
mul r1, r3, r1 | mul r5, r7, r5 | st r3, [a120]
mad r1, r0, -r2 | mad r5, r4, -r6 | st r7, [a121] | ld r0, [0601]
add r1, #1.0 | add r5, #1.0 | ld r2, [a168] | ld r4, [0629]
st r1, [0616] | st r5, [063e]
ld r1, [c04f] | ld r6, [a169] | st r2, [0604]
mul r1, r3, r1 | ld r5, [c04f] | ld r3, [0605] | st r6, [062c]
mul r5, r7, r5 | subxy r3, r3, r2 | ld r7, [062d] | st r1, [0618]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r5, [0640] | ld r1, [0603] | ld r0, [c043]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r5, [062b] | ld r4, [c043]
mul r6, r6, #256.0 | mul r3, r3, r3
mul r7, r7, r7
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r3, #1.0 | subxy r7, r7, #1.0
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r1, r1, r3 | mul r5, r5, r7 | ld r3, [a160]
ph_clamp r2, r3, -r2 | ld r7, [a161]
ph_clamp r6, r3, -r6
max r1, r1, r2
max r5, r5, r6
mul r1, r1, r3 | mul r5, r5, r7
subxy r2, r1, r3 | subxy r6, r5, r7 | st r1, [0602]
subxy r3, r3, #1.0 | subxy r7, r7, #1.0 | st r5, [062a]
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r3, r3, r0 | mul r7, r7, r4 | ld r0, [c041]
ph_clamp r2, r3, -r2 | ld r4, [c041]
ph_clamp r6, r3, -r6
mad r3, r2, r0 | mad r7, r6, r4 | ld r0, [c042]
mad r3, r1, r0 | ld r4, [c042]
mad r7, r5, r4 | ld r0, [c040]
mul r1, r1, r0 | ld r4, [c040] | ld r0, [0601]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [0629] | ld r2, [a168]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [a169] | st r2, [060a]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r3, [060b] | st r6, [0632]
mad r4, r5, r7 | subxy r3, r3, r2 | ld r7, [0633] | ld r1, [0609]
min r0, r0, #1.0 | ld r5, [0631]
min r4, r4, #1.0 | st r0, [0600] | st r0, [a170] | ld r0, [0607]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r4, [0628] | st r4, [a171] | ld r4, [062f]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r0, [c047]
mul r6, r6, #256.0 | mul r3, r3, r3 | ld r4, [c047]
mul r7, r7, r7
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r3, #1.0 | subxy r7, r7, #1.0
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r1, r1, r3 | mul r5, r5, r7 | ld r3, [a160]
ph_clamp r2, r3, -r2 | ld r7, [a161]
ph_clamp r6, r3, -r6
max r1, r1, r2
max r5, r5, r6
mul r1, r1, r3 | mul r5, r5, r7
subxy r2, r1, r3 | subxy r6, r5, r7 | st r1, [0608]
subxy r3, r3, #1.0 | subxy r7, r7, #1.0 | st r5, [0630]
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r3, r3, r0 | mul r7, r7, r4 | ld r0, [c045]
ph_clamp r2, r3, -r2 | ld r4, [c045]
ph_clamp r6, r3, -r6
mad r3, r2, r0 | mad r7, r6, r4 | ld r0, [c046]
mad r3, r1, r0 | ld r4, [c046]
mad r7, r5, r4 | ld r0, [c044]
mul r1, r1, r0 | ld r4, [c044] | ld r0, [0607]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [062f] | ld r2, [0616]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [063e]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r1, [0600]
mad r4, r5, r7 | mul r3, r1, r2 | ld r5, [0628] | ld r2, [c061]
min r0, r0, #1.0 | st r3, [061e] | ld r3, [c062]
min r4, r4, #1.0 | st r0, [0606] | st r0, [a178]
mul r7, r5, r6 | st r4, [062e] | st r4, [a179] | ld r1, [0606]
ld r0, [a151] | st r7, [0646] | ld r7, [c062]
ld r4, [a152] | st r0, [0620] | ld r6, [c061]
st r4, [0648] | ld r0, [a1b0]
addxy r0, r0, #-4.75 | ld r4, [a1b1] | ld r5, [062e]
addxy r4, r4, #-4.75 | mul r3, r3, r0
mul r7, r7, r4 | mul r0, r1, r2 | ld r1, [0618] | ld r2, [c060]
mul r4, r5, r6 | mad r0, r3, #0.125 | ld r5, [0640] | ld r6, [c060]
mad r4, r7, #0.125 | mad r0, r1, #0.125
mad r4, r5, #0.125 | mul r3, r0, #0.17328679513998632
mul r7, r4, #0.17328679513998632 | mov r1, #0.041666666666666664
mov r5, #0.041666666666666664 | ymacd r1, r3, #0.16666666666666666
ymacd r5, r7, #0.16666666666666666 | ymacd r1, r3, #0.5
ymacd r5, r7, #0.5 | ymacd r1, r3, #1.0
ymacd r5, r7, #1.0 | ymacd r1, r3, #1.0
ymacd r5, r7, #1.0 | mul r0, r1, r1
mul r4, r5, r5 | mul r3, r0, r0
mul r7, r4, r4 | mul r1, r3, r3
mul r5, r7, r7 | mul r0, r1, r1
mul r4, r5, r5 | mul r3, r0, r0
mul r7, r4, r4 | mul r1, r3, #6.103515625e-05 | ld r3, [c063]
mul r5, r7, #6.103515625e-05 | mul r0, r2, r1 | ld r7, [c063] | st r3, [0622]
mul r4, r6, r5 | st r7, [064a]
min r2, r0, #3.3569336665095761e-05
min r6, r4, #3.3569336665095761e-05
mul r1, r2, #256.0 | mul r5, r6, #256.0 | ld r2, [061b] | ld r6, [0643]
mul r0, r1, #64.0 | mul r4, r5, #64.0 | ld r1, [061d] | ld r5, [0645]
mul r3, r0, r1 | mul r7, r4, r5 | st r0, [0626]
addxy r3, r3, r2 | addxy r7, r7, r6 | st r4, [064e]
mov r2, r3 | mov r6, r7 | st r3, [0624] | ld r3, [0620]
mad r2, r3, #-0.0625 | st r7, [064c] | ld r7, [0648]
mad r6, r7, #-0.0625 | ld r3, [0622] | ld r7, [064a]
mul r0, r3, r1 | mul r4, r7, r5
mad r0, r2, #0.5 | mad r4, r6, #0.5 | ld r2, [0626] | ld r6, [064e]
mul r0, r2, r0 | mul r4, r6, r4
mad r0, r1, #-0.5 | mad r4, r5, #-0.5
mul r1, r2, r0 | mul r5, r6, r4 | ld r2, [0624] | ld r6, [064c]
mad r1, r2, #-0.5 | mad r5, r6, #-0.5
mul r2, r1, #-2.0 | mul r6, r5, #-2.0 | st r1, [0624]
st r2, [061a] | st r6, [0642]
mov r2, r1 | mov r6, r5 | st r5, [064c] | ld r1, [0620]
mad r2, r1, #0.03125 | mul r1, r3, r0 | ld r5, [0648] | ld r3, [0626]
mad r6, r5, #0.03125 | mul r5, r7, r4 | ld r7, [064e]
mad r1, r2, #0.5 | mad r5, r6, #0.5
mul r2, r3, r1 | mul r6, r7, r5 | ld r1, [061e] | ld r5, [0646]
mad r2, r0, #-0.5 | mad r6, r4, #-0.5 | ld r0, [0624] | ld r4, [064c]
mul r3, r2, #4.0 | mul r7, r6, #4.0
mul r2, r1, r0 | mul r6, r5, r4 | ld r0, [a16a] | ld r1, [065d] | ld r5, [0685]
subxy r1, r1, r0 | ld r4, [a16b] | st r0, [065c]
subxy r5, r5, r4 | mul r1, r1, r1 | st r4, [0684] | ld r0, [065f]
st r3, [061c] | st r7, [0644] | ld r4, [0687]
mul r5, r5, r5 | mul r3, r2, #-32.0 | ld r2, [c048]
mul r7, r6, #-32.0 | mad r0, r0, -r1 | ld r6, [c048] | st r3, [0400] | ld r1, [0661]
mad r4, r4, -r5 | addxy r2, r0, r2 | st r7, [0402] | ld r5, [0689]
addxy r6, r4, r6 | ld r3, [06f0]
ph_wrap r0, r3, r2 | ld r7, [06f0]
ph_wrap r4, r3, r6 | st r0, [065e]
subxy r2, r0, r2 | subxy r6, r4, r6 | st r4, [0686]
subxy r3, r1, r3 | subxy r7, r5, r7
mul r3, r3, r2 | mul r7, r7, r6 | ld r2, [c049]
mad r1, r3, #0.5 | mad r5, r7, #0.5 | ld r3, [0663] | ld r7, [068b] | ld r6, [c049]
st r1, [0660] | st r5, [0688]
subxy r1, r3, r1 | subxy r5, r7, r5
mad r3, r1, r2 | mad r7, r5, r6 | ld r1, [c04d]
mul r3, r3, r1 | ld r5, [c04d] | st r3, [0662]
mul r7, r7, r5 | st r7, [068a] | ld r1, [c04c]
mul r2, r0, r1 | ph_tri r0, r3, -r0 | ld r5, [c04c]
mul r6, r4, r5 | ld r1, [c04b]
ph_clamp r2, r3, -r2 | ld r5, [c04b]
ph_clamp r6, r3, -r6
ph_tri r4, r3, -r4
addxy r3, r3, r2 | addxy r7, r7, r6
mul r2, r0, r0 | mul r6, r4, r4
mul r2, r2, r1 | mul r6, r6, r5 | ld r1, [c04a]
addxy r2, r2, r1 | ld r5, [c04a]
addxy r6, r6, r5 | mad r3, r0, r2 | ld r0, [0665] | ld r1, [c04e]
mad r7, r4, r6 | addxy r0, r0, r1 | ld r4, [068d] | ld r5, [c04e]
addxy r4, r4, r5 | ld r2, [a162] | ld r1, [c050]
mul r0, r0, r2 | ld r6, [a163] | ld r5, [c050]
mul r4, r4, r6 | ld r2, [c051]
min r0, r0, #1.0 | ld r6, [c051]
min r4, r4, #1.0 | st r0, [0664]
mul r3, r3, r0 | mul r7, r7, r4 | st r4, [068c]
mul r1, r3, r1 | mul r5, r7, r5 | st r3, [a122]
mad r1, r0, -r2 | mad r5, r4, -r6 | st r7, [a123] | ld r0, [0651]
add r1, #1.0 | add r5, #1.0 | ld r2, [a16a] | ld r4, [0679]
st r1, [0666] | st r5, [068e]
ld r1, [c04f] | ld r6, [a16b] | st r2, [0654]
mul r1, r3, r1 | ld r5, [c04f] | ld r3, [0655] | st r6, [067c]
mul r5, r7, r5 | subxy r3, r3, r2 | ld r7, [067d] | st r1, [0668]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r5, [0690] | ld r1, [0653] | ld r0, [c043]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r5, [067b] | ld r4, [c043]
mul r6, r6, #256.0 | mul r3, r3, r3
mul r7, r7, r7
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r3, #1.0 | subxy r7, r7, #1.0
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r1, r1, r3 | mul r5, r5, r7 | ld r3, [a162]
ph_clamp r2, r3, -r2 | ld r7, [a163]
ph_clamp r6, r3, -r6
max r1, r1, r2
max r5, r5, r6
mul r1, r1, r3 | mul r5, r5, r7
subxy r2, r1, r3 | subxy r6, r5, r7 | st r1, [0652]
subxy r3, r3, #1.0 | subxy r7, r7, #1.0 | st r5, [067a]
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r3, r3, r0 | mul r7, r7, r4 | ld r0, [c041]
ph_clamp r2, r3, -r2 | ld r4, [c041]
ph_clamp r6, r3, -r6
mad r3, r2, r0 | mad r7, r6, r4 | ld r0, [c042]
mad r3, r1, r0 | ld r4, [c042]
mad r7, r5, r4 | ld r0, [c040]
mul r1, r1, r0 | ld r4, [c040] | ld r0, [0651]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [0679] | ld r2, [a16a]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [a16b] | st r2, [065a]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r3, [065b] | st r6, [0682]
mad r4, r5, r7 | subxy r3, r3, r2 | ld r7, [0683] | ld r1, [0659]
min r0, r0, #1.0 | ld r5, [0681]
min r4, r4, #1.0 | st r0, [0650] | st r0, [a172] | ld r0, [0657]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r4, [0678] | st r4, [a173] | ld r4, [067f]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r0, [c047]
mul r6, r6, #256.0 | mul r3, r3, r3 | ld r4, [c047]
mul r7, r7, r7
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r3, #1.0 | subxy r7, r7, #1.0
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r1, r1, r3 | mul r5, r5, r7 | ld r3, [a162]
ph_clamp r2, r3, -r2 | ld r7, [a163]
ph_clamp r6, r3, -r6
max r1, r1, r2
max r5, r5, r6
mul r1, r1, r3 | mul r5, r5, r7
subxy r2, r1, r3 | subxy r6, r5, r7 | st r1, [0658]
subxy r3, r3, #1.0 | subxy r7, r7, #1.0 | st r5, [0680]
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r3, r3, r0 | mul r7, r7, r4 | ld r0, [c045]
ph_clamp r2, r3, -r2 | ld r4, [c045]
ph_clamp r6, r3, -r6
mad r3, r2, r0 | mad r7, r6, r4 | ld r0, [c046]
mad r3, r1, r0 | ld r4, [c046]
mad r7, r5, r4 | ld r0, [c044]
mul r1, r1, r0 | ld r4, [c044] | ld r0, [0657]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [067f] | ld r2, [0666]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [068e]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r1, [0650]
mad r4, r5, r7 | mul r3, r1, r2 | ld r5, [0678] | ld r2, [c061]
min r0, r0, #1.0 | st r3, [066e] | ld r3, [c062]
min r4, r4, #1.0 | st r0, [0656] | st r0, [a17a]
mul r7, r5, r6 | st r4, [067e] | st r4, [a17b] | ld r1, [0656]
ld r0, [a153] | st r7, [0696] | ld r7, [c062]
ld r4, [a154] | st r0, [0670] | ld r6, [c061]
st r4, [0698] | ld r0, [a1b2]
addxy r0, r0, #-4.75 | ld r4, [a1b3] | ld r5, [067e]
addxy r4, r4, #-4.75 | mul r3, r3, r0
mul r7, r7, r4 | mul r0, r1, r2 | ld r1, [0668] | ld r2, [c060]
mul r4, r5, r6 | mad r0, r3, #0.125 | ld r5, [0690] | ld r6, [c060]
mad r4, r7, #0.125 | mad r0, r1, #0.125
mad r4, r5, #0.125 | mul r3, r0, #0.17328679513998632
mul r7, r4, #0.17328679513998632 | mov r1, #0.041666666666666664
mov r5, #0.041666666666666664 | ymacd r1, r3, #0.16666666666666666
ymacd r5, r7, #0.16666666666666666 | ymacd r1, r3, #0.5
ymacd r5, r7, #0.5 | ymacd r1, r3, #1.0
ymacd r5, r7, #1.0 | ymacd r1, r3, #1.0
ymacd r5, r7, #1.0 | mul r0, r1, r1
mul r4, r5, r5 | mul r3, r0, r0
mul r7, r4, r4 | mul r1, r3, r3
mul r5, r7, r7 | mul r0, r1, r1
mul r4, r5, r5 | mul r3, r0, r0
mul r7, r4, r4 | mul r1, r3, #6.103515625e-05 | ld r3, [c063]
mul r5, r7, #6.103515625e-05 | mul r0, r2, r1 | ld r7, [c063] | st r3, [0672]
mul r4, r6, r5 | st r7, [069a]
min r2, r0, #3.3569336665095761e-05
min r6, r4, #3.3569336665095761e-05
mul r1, r2, #256.0 | mul r5, r6, #256.0 | ld r2, [066b] | ld r6, [0693]
mul r0, r1, #64.0 | mul r4, r5, #64.0 | ld r1, [066d] | ld r5, [0695]
mul r3, r0, r1 | mul r7, r4, r5 | st r0, [0676]
addxy r3, r3, r2 | addxy r7, r7, r6 | st r4, [069e]
mov r2, r3 | mov r6, r7 | st r3, [0674] | ld r3, [0670]
mad r2, r3, #-0.0625 | st r7, [069c] | ld r7, [0698]
mad r6, r7, #-0.0625 | ld r3, [0672] | ld r7, [069a]
mul r0, r3, r1 | mul r4, r7, r5
mad r0, r2, #0.5 | mad r4, r6, #0.5 | ld r2, [0676] | ld r6, [069e]
mul r0, r2, r0 | mul r4, r6, r4
mad r0, r1, #-0.5 | mad r4, r5, #-0.5
mul r1, r2, r0 | mul r5, r6, r4 | ld r2, [0674] | ld r6, [069c]
mad r1, r2, #-0.5 | mad r5, r6, #-0.5
mul r2, r1, #-2.0 | mul r6, r5, #-2.0 | st r1, [0674]
st r2, [066a] | st r6, [0692]
mov r2, r1 | mov r6, r5 | st r5, [069c] | ld r1, [0670]
mad r2, r1, #0.03125 | mul r1, r3, r0 | ld r5, [0698] | ld r3, [0676]
mad r6, r5, #0.03125 | mul r5, r7, r4 | ld r7, [069e]
mad r1, r2, #0.5 | mad r5, r6, #0.5
mul r2, r3, r1 | mul r6, r7, r5 | ld r1, [066e] | ld r5, [0696]
mad r2, r0, #-0.5 | mad r6, r4, #-0.5 | ld r0, [0674] | ld r4, [069c]
mul r3, r2, #4.0 | mul r7, r6, #4.0
mul r2, r1, r0 | mul r6, r5, r4 | ld r0, [a16c] | ld r1, [06ad] | ld r5, [06d5]
subxy r1, r1, r0 | ld r4, [a16d] | st r0, [06ac]
subxy r5, r5, r4 | mul r1, r1, r1 | st r4, [06d4] | ld r0, [06af]
st r3, [066c] | st r7, [0694] | ld r4, [06d7]
mul r5, r5, r5 | mul r3, r2, #-32.0 | ld r2, [c048]
mul r7, r6, #-32.0 | mad r0, r0, -r1 | ld r6, [c048] | st r3, [0404] | ld r1, [06b1]
mad r4, r4, -r5 | addxy r2, r0, r2 | st r7, [0406] | ld r5, [06d9]
addxy r6, r4, r6 | ld r3, [06f0]
ph_wrap r0, r3, r2 | ld r7, [06f0]
ph_wrap r4, r3, r6 | st r0, [06ae]
subxy r2, r0, r2 | subxy r6, r4, r6 | st r4, [06d6]
subxy r3, r1, r3 | subxy r7, r5, r7
mul r3, r3, r2 | mul r7, r7, r6 | ld r2, [c049]
mad r1, r3, #0.5 | mad r5, r7, #0.5 | ld r3, [06b3] | ld r7, [06db] | ld r6, [c049]
st r1, [06b0] | st r5, [06d8]
subxy r1, r3, r1 | subxy r5, r7, r5
mad r3, r1, r2 | mad r7, r5, r6 | ld r1, [c04d]
mul r3, r3, r1 | ld r5, [c04d] | st r3, [06b2]
mul r7, r7, r5 | st r7, [06da] | ld r1, [c04c]
mul r2, r0, r1 | ph_tri r0, r3, -r0 | ld r5, [c04c]
mul r6, r4, r5 | ld r1, [c04b]
ph_clamp r2, r3, -r2 | ld r5, [c04b]
ph_clamp r6, r3, -r6
ph_tri r4, r3, -r4
addxy r3, r3, r2 | addxy r7, r7, r6
mul r2, r0, r0 | mul r6, r4, r4
mul r2, r2, r1 | mul r6, r6, r5 | ld r1, [c04a]
addxy r2, r2, r1 | ld r5, [c04a]
addxy r6, r6, r5 | mad r3, r0, r2 | ld r0, [06b5] | ld r1, [c04e]
mad r7, r4, r6 | addxy r0, r0, r1 | ld r4, [06dd] | ld r5, [c04e]
addxy r4, r4, r5 | ld r2, [a164] | ld r1, [c050]
mul r0, r0, r2 | ld r6, [a165] | ld r5, [c050]
mul r4, r4, r6 | ld r2, [c051]
min r0, r0, #1.0 | ld r6, [c051]
min r4, r4, #1.0 | st r0, [06b4]
mul r3, r3, r0 | mul r7, r7, r4 | st r4, [06dc]
mul r1, r3, r1 | mul r5, r7, r5 | st r3, [a124]
mad r1, r0, -r2 | mad r5, r4, -r6 | st r7, [a125] | ld r0, [06a1]
add r1, #1.0 | add r5, #1.0 | ld r2, [a16c] | ld r4, [06c9]
st r1, [06b6] | st r5, [06de]
ld r1, [c04f] | ld r6, [a16d] | st r2, [06a4]
mul r1, r3, r1 | ld r5, [c04f] | ld r3, [06a5] | st r6, [06cc]
mul r5, r7, r5 | subxy r3, r3, r2 | ld r7, [06cd] | st r1, [06b8]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r5, [06e0] | ld r1, [06a3] | ld r0, [c043]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r5, [06cb] | ld r4, [c043]
mul r6, r6, #256.0 | mul r3, r3, r3
mul r7, r7, r7
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r3, #1.0 | subxy r7, r7, #1.0
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r1, r1, r3 | mul r5, r5, r7 | ld r3, [a164]
ph_clamp r2, r3, -r2 | ld r7, [a165]
ph_clamp r6, r3, -r6
max r1, r1, r2
max r5, r5, r6
mul r1, r1, r3 | mul r5, r5, r7
subxy r2, r1, r3 | subxy r6, r5, r7 | st r1, [06a2]
subxy r3, r3, #1.0 | subxy r7, r7, #1.0 | st r5, [06ca]
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r3, r3, r0 | mul r7, r7, r4 | ld r0, [c041]
ph_clamp r2, r3, -r2 | ld r4, [c041]
ph_clamp r6, r3, -r6
mad r3, r2, r0 | mad r7, r6, r4 | ld r0, [c042]
mad r3, r1, r0 | ld r4, [c042]
mad r7, r5, r4 | ld r0, [c040]
mul r1, r1, r0 | ld r4, [c040] | ld r0, [06a1]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [06c9] | ld r2, [a16c]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [a16d] | st r2, [06aa]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r3, [06ab] | st r6, [06d2]
mad r4, r5, r7 | subxy r3, r3, r2 | ld r7, [06d3] | ld r1, [06a9]
min r0, r0, #1.0 | ld r5, [06d1]
min r4, r4, #1.0 | st r0, [06a0] | st r0, [a174] | ld r0, [06a7]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r4, [06c8] | st r4, [a175] | ld r4, [06cf]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r0, [c047]
mul r6, r6, #256.0 | mul r3, r3, r3 | ld r4, [c047]
mul r7, r7, r7
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r3, #1.0 | subxy r7, r7, #1.0
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r1, r1, r3 | mul r5, r5, r7 | ld r3, [a164]
ph_clamp r2, r3, -r2 | ld r7, [a165]
ph_clamp r6, r3, -r6
max r1, r1, r2
max r5, r5, r6
mul r1, r1, r3 | mul r5, r5, r7
subxy r2, r1, r3 | subxy r6, r5, r7 | st r1, [06a8]
subxy r3, r3, #1.0 | subxy r7, r7, #1.0 | st r5, [06d0]
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r3, r3, r0 | mul r7, r7, r4 | ld r0, [c045]
ph_clamp r2, r3, -r2 | ld r4, [c045]
ph_clamp r6, r3, -r6
mad r3, r2, r0 | mad r7, r6, r4 | ld r0, [c046]
mad r3, r1, r0 | ld r4, [c046]
mad r7, r5, r4 | ld r0, [c044]
mul r1, r1, r0 | ld r4, [c044] | ld r0, [06a7]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [06cf] | ld r2, [06b6]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [06de]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r1, [06a0]
mad r4, r5, r7 | mul r3, r1, r2 | ld r5, [06c8] | ld r2, [c061]
min r0, r0, #1.0 | st r3, [06be] | ld r3, [c062]
min r4, r4, #1.0 | st r0, [06a6] | st r0, [a17c]
mul r7, r5, r6 | st r4, [06ce] | st r4, [a17d] | ld r1, [06a6]
ld r0, [a155] | st r7, [06e6] | ld r7, [c062]
ld r4, [a156] | st r0, [06c0] | ld r6, [c061]
st r4, [06e8] | ld r0, [a1b4]
addxy r0, r0, #-4.75 | ld r4, [a1b5] | ld r5, [06ce]
addxy r4, r4, #-4.75 | mul r3, r3, r0
mul r7, r7, r4 | mul r0, r1, r2 | ld r1, [06b8] | ld r2, [c060]
mul r4, r5, r6 | mad r0, r3, #0.125 | ld r5, [06e0] | ld r6, [c060]
mad r4, r7, #0.125 | mad r0, r1, #0.125
mad r4, r5, #0.125 | mul r3, r0, #0.17328679513998632
mul r7, r4, #0.17328679513998632 | mov r1, #0.041666666666666664
mov r5, #0.041666666666666664 | ymacd r1, r3, #0.16666666666666666
ymacd r5, r7, #0.16666666666666666 | ymacd r1, r3, #0.5
ymacd r5, r7, #0.5 | ymacd r1, r3, #1.0
ymacd r5, r7, #1.0 | ymacd r1, r3, #1.0
ymacd r5, r7, #1.0 | mul r0, r1, r1
mul r4, r5, r5 | mul r3, r0, r0
mul r7, r4, r4 | mul r1, r3, r3
mul r5, r7, r7 | mul r0, r1, r1
mul r4, r5, r5 | mul r3, r0, r0
mul r7, r4, r4 | mul r1, r3, #6.103515625e-05 | ld r3, [c063]
mul r5, r7, #6.103515625e-05 | mul r0, r2, r1 | ld r7, [c063] | st r3, [06c2]
mul r4, r6, r5 | st r7, [06ea]
min r2, r0, #3.3569336665095761e-05
min r6, r4, #3.3569336665095761e-05
mul r1, r2, #256.0 | mul r5, r6, #256.0 | ld r2, [06bb] | ld r6, [06e3]
mul r0, r1, #64.0 | mul r4, r5, #64.0 | ld r1, [06bd] | ld r5, [06e5]
mul r3, r0, r1 | mul r7, r4, r5 | st r0, [06c6]
addxy r3, r3, r2 | addxy r7, r7, r6 | st r4, [06ee]
mov r2, r3 | mov r6, r7 | st r3, [06c4] | ld r3, [06c0]
mad r2, r3, #-0.0625 | st r7, [06ec] | ld r7, [06e8]
mad r6, r7, #-0.0625 | ld r3, [06c2] | ld r7, [06ea]
mul r0, r3, r1 | mul r4, r7, r5
mad r0, r2, #0.5 | mad r4, r6, #0.5 | ld r2, [06c6] | ld r6, [06ee]
mul r0, r2, r0 | mul r4, r6, r4
mad r0, r1, #-0.5 | mad r4, r5, #-0.5
mul r1, r2, r0 | mul r5, r6, r4 | ld r2, [06c4] | ld r6, [06ec]
mad r1, r2, #-0.5 | mad r5, r6, #-0.5
mul r2, r1, #-2.0 | mul r6, r5, #-2.0 | st r1, [06c4]
st r2, [06ba] | st r6, [06e2]
mov r2, r1 | mov r6, r5 | st r5, [06ec] | ld r1, [06c0]
mad r2, r1, #0.03125 | mul r1, r3, r0 | ld r5, [06e8] | ld r3, [06c6]
mad r6, r5, #0.03125 | mul r5, r7, r4 | ld r7, [06ee]
mad r1, r2, #0.5 | mad r5, r6, #0.5
mul r2, r3, r1 | mul r6, r7, r5 | ld r1, [06be] | ld r5, [06e6]
mad r2, r0, #-0.5 | mad r6, r4, #-0.5 | ld r0, [06c4] | ld r4, [06ec]
mul r3, r2, #4.0 | mul r7, r6, #4.0
mul r2, r1, r0 | mul r6, r5, r4 | st r3, [06bc]
mul r3, r2, #-32.0 | mul r7, r6, #-32.0 | st r7, [06e4]
st r3, [0408] | st r7, [040a]
