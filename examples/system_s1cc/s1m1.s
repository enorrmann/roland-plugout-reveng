mov r7, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r6, [c000]
addxy r7, r7, r0 | ld r0, [0405] | ld r2, [06e1]
addxy r7, r7, r1 | mul r2, r2, #7.31275 | ld r1, [0407]
addxy r7, r7, r0 | ld r0, [0409]
addxy r7, r7, r1 | ld r1, [040b]
addxy r7, r7, r0 | ld r0, [040d]
addxy r7, r7, r1 | ld r1, [060d]
addxy r7, r7, r0 | mov r0, #0.0
mul r7, r7, #0.19047619047619047 | st r0, [6008]
mul r7, r7, r6 | st r0, [6009]
st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [06e3]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [06e2] | ld r0, [a168] | ld r7, [c061]
subxy r1, r1, r0 | st r0, [060c] | ld r0, [060f]
ph_wrap r2, r3, r2
mul r1, r1, r1 | st r2, [06e0] | ld r2, [c048]
mad r0, r0, -r1 | ld r1, [0611] | ld r3, [06e0]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [060e]
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
mad r1, r0, -r2 | ld r2, [a168] | ld r0, [0601]
add r1, #1.0 | st r2, [0604]
st r1, [0616] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [0605]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [0618] | ld r1, [0603] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a160]
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
subxy r3, r3, r2 | st r0, [0600] | st r0, [a170] | ld r0, [0607]
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
mad r1, r2, #1.25 | ld r2, [0616]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [0600]
mul r3, r1, r2 | ld r2, [a1b0] | ld r1, [c062]
min r0, r0, #1.0 | st r3, [061e]
addxy r2, r2, #-4.75 | st r0, [0606] | st r0, [a178]
mul r1, r1, r2 | ld r4, [0606] | ld r0, [a151]
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [061b]
shl r4, r7, 14 | ld r7, [061d]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.03125
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [061a]
mad r3, r0, #0.015625
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [061e]
mul r0, r4, r7 | mul r3, r1, #4.0 | ld r1, [062d] | ld r7, [c061]
mul r2, r0, #-64.0 | ld r0, [a169] | st r3, [061c]
subxy r1, r1, r0 | st r0, [062c] | ld r0, [062f]
mul r1, r1, r1 | st r2, [0400] | ld r2, [c048] | ld r3, [06e0]
mad r0, r0, -r1 | ld r1, [0631]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [062e]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [0633]
subxy r1, r3, r1 | st r1, [0630]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [0632] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0635] | ld r2, [a161]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0634]
mul r1, r3, r1 | st r3, [a121]
mad r1, r0, -r2 | ld r2, [a169] | ld r0, [0621]
add r1, #1.0 | st r2, [0624]
st r1, [0636] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [0625]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [0638] | ld r1, [0623] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a161]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0622]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0621]
mad r1, r2, #1.25 | ld r2, [a169]
subxy r1, r0, r1 | st r2, [062a]
mad r0, r1, r3 | ld r3, [062b] | ld r1, [0629]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0620] | st r0, [a171] | ld r0, [0627]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a161]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0628]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0627]
mad r1, r2, #1.25 | ld r2, [0636]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [0620]
mul r3, r1, r2 | ld r2, [a1b1] | ld r1, [c062]
min r0, r0, #1.0 | st r3, [063e]
addxy r2, r2, #-4.75 | st r0, [0626] | st r0, [a179]
mul r1, r1, r2 | ld r4, [0626] | ld r0, [a152]
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [0638] | ld r7, [c060]
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [063b]
shl r4, r7, 14 | ld r7, [063d]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.03125
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [063a]
mad r3, r0, #0.015625
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [063e]
mul r0, r4, r7 | mul r3, r1, #4.0 | ld r1, [064d] | ld r7, [c061]
mul r2, r0, #-64.0 | ld r0, [a16a] | st r3, [063c]
subxy r1, r1, r0 | st r0, [064c] | ld r0, [064f]
mul r1, r1, r1 | st r2, [0402] | ld r2, [c048] | ld r3, [06e0]
mad r0, r0, -r1 | ld r1, [0651]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [064e]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [0653]
subxy r1, r3, r1 | st r1, [0650]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [0652] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0655] | ld r2, [a162]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0654]
mul r1, r3, r1 | st r3, [a122]
mad r1, r0, -r2 | ld r2, [a16a] | ld r0, [0641]
add r1, #1.0 | st r2, [0644]
st r1, [0656] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [0645]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [0658] | ld r1, [0643] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a162]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0642]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0641]
mad r1, r2, #1.25 | ld r2, [a16a]
subxy r1, r0, r1 | st r2, [064a]
mad r0, r1, r3 | ld r3, [064b] | ld r1, [0649]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0640] | st r0, [a172] | ld r0, [0647]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a162]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0648]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0647]
mad r1, r2, #1.25 | ld r2, [0656]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [0640]
mul r3, r1, r2 | ld r2, [a1b2] | ld r1, [c062]
min r0, r0, #1.0 | st r3, [065e]
addxy r2, r2, #-4.75 | st r0, [0646] | st r0, [a17a]
mul r1, r1, r2 | ld r4, [0646] | ld r0, [a153]
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [0658] | ld r7, [c060]
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [065b]
shl r4, r7, 14 | ld r7, [065d]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.03125
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [065a]
mad r3, r0, #0.015625
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [065e]
mul r0, r4, r7 | mul r3, r1, #4.0 | ld r1, [066d] | ld r7, [c061]
mul r2, r0, #-64.0 | ld r0, [a16b] | st r3, [065c]
subxy r1, r1, r0 | st r0, [066c] | ld r0, [066f]
mul r1, r1, r1 | st r2, [0404] | ld r2, [c048] | ld r3, [06e0]
mad r0, r0, -r1 | ld r1, [0671]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [066e]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [0673]
subxy r1, r3, r1 | st r1, [0670]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [0672] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0675] | ld r2, [a163]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0674]
mul r1, r3, r1 | st r3, [a123]
mad r1, r0, -r2 | ld r2, [a16b] | ld r0, [0661]
add r1, #1.0 | st r2, [0664]
st r1, [0676] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [0665]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [0678] | ld r1, [0663] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a163]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0662]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0661]
mad r1, r2, #1.25 | ld r2, [a16b]
subxy r1, r0, r1 | st r2, [066a]
mad r0, r1, r3 | ld r3, [066b] | ld r1, [0669]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0660] | st r0, [a173] | ld r0, [0667]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a163]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0668]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0667]
mad r1, r2, #1.25 | ld r2, [0676]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [0660]
mul r3, r1, r2 | ld r2, [a1b3] | ld r1, [c062]
min r0, r0, #1.0 | st r3, [067e]
addxy r2, r2, #-4.75 | st r0, [0666] | st r0, [a17b]
mul r1, r1, r2 | ld r4, [0666] | ld r0, [a154]
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [0678] | ld r7, [c060]
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [067b]
shl r4, r7, 14 | ld r7, [067d]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.03125
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [067a]
mad r3, r0, #0.015625
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [067e]
mul r0, r4, r7 | mul r3, r1, #4.0 | ld r1, [068d] | ld r7, [c061]
mul r2, r0, #-64.0 | ld r0, [a16c] | st r3, [067c]
subxy r1, r1, r0 | st r0, [068c] | ld r0, [068f]
mul r1, r1, r1 | st r2, [0406] | ld r2, [c048] | ld r3, [06e0]
mad r0, r0, -r1 | ld r1, [0691]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [068e]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [0693]
subxy r1, r3, r1 | st r1, [0690]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [0692] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0695] | ld r2, [a164]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0694]
mul r1, r3, r1 | st r3, [a124]
mad r1, r0, -r2 | ld r2, [a16c] | ld r0, [0681]
add r1, #1.0 | st r2, [0684]
st r1, [0696] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [0685]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [0698] | ld r1, [0683] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a164]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0682]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0681]
mad r1, r2, #1.25 | ld r2, [a16c]
subxy r1, r0, r1 | st r2, [068a]
mad r0, r1, r3 | ld r3, [068b] | ld r1, [0689]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0680] | st r0, [a174] | ld r0, [0687]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a164]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0688]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0687]
mad r1, r2, #1.25 | ld r2, [0696]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [0680]
mul r3, r1, r2 | ld r2, [a1b4] | ld r1, [c062]
min r0, r0, #1.0 | st r3, [069e]
addxy r2, r2, #-4.75 | st r0, [0686] | st r0, [a17c]
mul r1, r1, r2 | ld r4, [0686] | ld r0, [a155]
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [0698] | ld r7, [c060]
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [069b]
shl r4, r7, 14 | ld r7, [069d]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.03125
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [069a]
mad r3, r0, #0.015625
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [069e]
mul r0, r4, r7 | mul r3, r1, #4.0 | ld r1, [06ad] | ld r7, [c061]
mul r2, r0, #-64.0 | ld r0, [a16d] | st r3, [069c]
subxy r1, r1, r0 | st r0, [06ac] | ld r0, [06af]
mul r1, r1, r1 | st r2, [0408] | ld r2, [c048] | ld r3, [06e0]
mad r0, r0, -r1 | ld r1, [06b1]
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
mad r3, r0, r2 | ld r0, [06b5] | ld r2, [a165]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [06b4]
mul r1, r3, r1 | st r3, [a125]
mad r1, r0, -r2 | ld r2, [a16d] | ld r0, [06a1]
add r1, #1.0 | st r2, [06a4]
st r1, [06b6] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [06a5]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [06b8] | ld r1, [06a3] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a165]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [06a2]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [06a1]
mad r1, r2, #1.25 | ld r2, [a16d]
subxy r1, r0, r1 | st r2, [06aa]
mad r0, r1, r3 | ld r3, [06ab] | ld r1, [06a9]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [06a0] | st r0, [a175] | ld r0, [06a7]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a165]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [06a8]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [06a7]
mad r1, r2, #1.25 | ld r2, [06b6]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [06a0]
mul r3, r1, r2 | ld r2, [a1b5] | ld r1, [c062]
min r0, r0, #1.0 | st r3, [06be]
addxy r2, r2, #-4.75 | st r0, [06a6] | st r0, [a17d]
mul r1, r1, r2 | ld r4, [06a6] | ld r0, [a156]
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [06b8] | ld r7, [c060]
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [06bb]
shl r4, r7, 14 | ld r7, [06bd]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.03125
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [06ba]
mad r3, r0, #0.015625
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [06be]
mul r0, r4, r7 | mul r3, r1, #4.0 | ld r1, [06cd] | ld r7, [c061]
mul r2, r0, #-64.0 | ld r0, [a16e] | st r3, [06bc]
subxy r1, r1, r0 | st r0, [06cc] | ld r0, [06cf]
mul r1, r1, r1 | st r2, [040a] | ld r2, [c048] | ld r3, [06e0]
mad r0, r0, -r1 | ld r1, [06d1]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [06ce]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [06d3]
subxy r1, r3, r1 | st r1, [06d0]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [06d2] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [06d5] | ld r2, [a166]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [06d4]
mul r1, r3, r1 | st r3, [a126]
mad r1, r0, -r2 | ld r2, [a16e] | ld r0, [06c1]
add r1, #1.0 | st r2, [06c4]
st r1, [06d6] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [06c5]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [06d8] | ld r1, [06c3] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a166]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [06c2]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [06c1]
mad r1, r2, #1.25 | ld r2, [a16e]
subxy r1, r0, r1 | st r2, [06ca]
mad r0, r1, r3 | ld r3, [06cb] | ld r1, [06c9]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [06c0] | st r0, [a176] | ld r0, [06c7]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a166]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [06c8]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [06c7]
mad r1, r2, #1.25 | ld r2, [06d6]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [06c0]
mul r3, r1, r2 | ld r2, [a1b6] | ld r1, [c062]
min r0, r0, #1.0 | st r3, [06de]
addxy r2, r2, #-4.75 | st r0, [06c6] | st r0, [a17e]
mul r1, r1, r2 | ld r4, [06c6] | ld r0, [a157]
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [06d8] | ld r7, [c060]
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
min r7, r2, #3.3569336665095761e-05 | ld r2, [06db]
shl r4, r7, 14 | ld r7, [06dd]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.03125
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [06da]
mad r3, r0, #0.015625
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [06de]
mul r0, r4, r7 | mul r3, r1, #4.0
mul r2, r0, #-64.0 | st r3, [06dc]
st r2, [040c]
