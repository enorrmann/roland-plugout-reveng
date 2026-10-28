mov r7, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r4, [a169]
addxy r7, r7, r0 | ld r0, [0405] | ld r2, [0771] | ld r6, [c000]
addxy r7, r7, r1 | mul r2, r2, #7.31275 | ld r1, [0407] | ld r5, [063b]
addxy r7, r7, r0 | subxy r5, r5, r4 | ld r0, [0409] | st r4, [063a]
addxy r7, r7, r1 | mul r5, r5, r5 | ld r1, [040b] | ld r4, [063d]
addxy r7, r7, r0 | mad r4, r4, -r5 | ld r0, [040d] | st r5, [0648]
addxy r7, r7, r1 | ld r1, [040f] | ld r5, [063f]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | st r0, [6008] | ld r1, [060d]
mul r7, r7, #0.16666666666666666 | st r0, [6009]
mul r7, r7, r6 | st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [0773]
add r0, #0.6184303127229214:32 | ld r6, [c048]
ph_wrap r0, r3, r0 | st r7, [6006]
addxy r2, r2, r0 | addxy r6, r4, r6 | st r0, [0772] | ld r0, [a168]
subxy r1, r1, r0 | st r0, [060c] | ld r0, [060f]
ph_wrap r2, r3, r2 | st r7, [6007]
mul r1, r1, r1 | st r2, [0770] | ld r2, [c048]
mad r0, r0, -r1 | st r1, [061a] | ld r1, [0611]
addxy r2, r0, r2 | ld r3, [0770]
ph_wrap r0, r3, r2 | ld r7, [0770]
ph_wrap r4, r3, r6 | st r0, [060e]
subxy r2, r0, r2 | subxy r6, r4, r6 | st r4, [063c]
subxy r3, r1, r3 | subxy r7, r5, r7
mul r3, r3, r2 | mul r7, r7, r6 | ld r2, [c049]
mad r1, r3, #0.5 | mad r5, r7, #0.5 | ld r3, [0613] | ld r7, [0641] | ld r6, [c049]
st r1, [0610] | st r5, [063e]
subxy r1, r3, r1 | subxy r5, r7, r5
mad r3, r1, r2 | mad r7, r5, r6 | ld r1, [c04d]
mul r3, r3, r1 | ld r5, [c04d] | st r3, [0612]
mul r7, r7, r5 | st r7, [0640] | ld r1, [c04c]
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
mad r7, r4, r6 | addxy r0, r0, r1 | ld r4, [0643] | ld r5, [c04e]
addxy r4, r4, r5 | ld r2, [a160] | ld r1, [c050]
mul r0, r0, r2 | ld r6, [a161] | ld r5, [c050]
mul r4, r4, r6 | ld r2, [c051]
min r0, r0, #1.0 | ld r6, [c051]
min r4, r4, #1.0 | st r0, [0614]
mul r3, r3, r0 | mul r7, r7, r4 | st r4, [0642]
mul r1, r3, r1 | mul r5, r7, r5 | st r3, [a120]
mad r1, r0, -r2 | mad r5, r4, -r6 | st r7, [a121] | ld r0, [0601]
add r1, #1.0 | add r5, #1.0 | ld r2, [a168] | ld r4, [062f]
st r1, [0616] | st r5, [0644]
ld r1, [c04f] | ld r6, [a169] | st r2, [0604]
mul r1, r3, r1 | ld r5, [c04f] | ld r3, [0605] | st r6, [0632]
mul r5, r7, r5 | subxy r3, r3, r2 | ld r7, [0633] | st r1, [0618]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r5, [0646] | ld r1, [0603] | ld r0, [c043]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r5, [0631] | ld r4, [c043]
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
subxy r3, r3, #1.0 | subxy r7, r7, #1.0 | st r5, [0630]
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r3, r3, r0 | mul r7, r7, r4 | ld r0, [c041]
ph_clamp r2, r3, -r2 | ld r4, [c041]
ph_clamp r6, r3, -r6
mad r3, r2, r0 | mad r7, r6, r4 | ld r0, [c042]
mad r3, r1, r0 | ld r4, [c042]
mad r7, r5, r4 | ld r0, [c040]
mul r1, r1, r0 | ld r4, [c040] | ld r0, [0601]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [062f] | ld r2, [a168]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [a169] | st r2, [060a]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r3, [060b] | st r6, [0638]
mad r4, r5, r7 | subxy r3, r3, r2 | ld r7, [0639] | ld r1, [0609]
min r0, r0, #1.0 | ld r5, [0637]
min r4, r4, #1.0 | st r0, [0600] | ld r0, [0607]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r4, [062e] | ld r4, [0635] | ld r0, [c047]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r4, [c047]
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
subxy r2, r1, r3 | subxy r6, r5, r7 | st r1, [0608]
subxy r3, r3, #1.0 | subxy r7, r7, #1.0 | st r5, [0636]
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r3, r3, r0 | mul r7, r7, r4 | ld r0, [c045]
ph_clamp r2, r3, -r2 | ld r4, [c045]
ph_clamp r6, r3, -r6
mad r3, r2, r0 | mad r7, r6, r4 | ld r0, [c046]
mad r3, r1, r0 | ld r4, [c046]
mad r7, r5, r4 | ld r0, [c044]
mul r1, r1, r0 | ld r4, [c044] | ld r0, [0607]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [0635] | ld r2, [c054]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [c054]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r1, [a120]
mad r4, r5, r7 | ld r5, [a121] | ld r3, [c055]
min r0, r0, #1.0 | ld r7, [c055]
min r4, r4, #1.0 | st r0, [0606] | ld r0, [a151]
mad r2, r1, r3 | mad r6, r5, r7 | ld r1, [061c] | ld r5, [064a] | ld r3, [c056]
mad r2, r1, r3 | ld r7, [c056] | st r4, [0634] | ld r1, [0606]
mad r6, r5, r7 | ld r5, [0634] | ld r3, [c057] | st r0, [0626]
mad r2, r1, r3 | ld r7, [c057] | ld r1, [0600]
mad r6, r5, r7 | ld r5, [062e] | ld r3, [c058]
mad r2, r1, r3 | ld r7, [c058] | ld r1, [a130]
mad r6, r5, r7 | ld r5, [a131] | ld r3, [c059]
mad r2, r1, r3 | ld r7, [c059] | ld r1, [a120]
mad r6, r5, r7 | st r2, [a138] | ld r2, [c05a]
st r6, [a139] | ld r6, [c05a]
ld r5, [a121] | ld r3, [c05b]
mad r2, r1, r3 | ld r7, [c05b] | ld r1, [061c]
mad r6, r5, r7 | ld r5, [064a] | ld r3, [c05c]
mad r2, r1, r3 | ld r7, [c05c] | ld r1, [0606]
mad r6, r5, r7 | ld r5, [0634] | ld r3, [c05d]
mad r2, r1, r3 | ld r7, [c05d] | ld r1, [0600]
mad r6, r5, r7 | ld r5, [062e] | ld r3, [c05e]
mad r2, r1, r3 | ld r7, [c05e] | ld r1, [a130]
mad r6, r5, r7 | ld r5, [a131] | ld r3, [c05f]
mad r2, r1, r3 | ld r7, [c05f] | ld r4, [a152]
mad r6, r5, r7 | st r2, [a140] | ld r1, [0600] | st r4, [0654]
st r6, [a141] | ld r5, [062e]
ld r2, [0616] | ld r6, [0644] | ld r0, [a1b0]
mul r3, r1, r2 | mul r7, r5, r6 | ld r4, [a1b1] | ld r1, [0606] | ld r5, [0634]
st r3, [0624] | st r7, [0652]
addxy r0, r0, #-4.75 | addxy r4, r4, #-4.75 | ld r3, [c062]
mul r3, r3, r0 | ld r7, [c062]
mul r7, r7, r4 | ld r2, [c061]
mul r0, r1, r2 | ld r6, [c061] | ld r1, [0618]
mul r4, r5, r6 | mad r0, r3, #0.125 | ld r5, [0646] | ld r2, [c060]
mad r4, r7, #0.125 | mad r0, r1, #0.125 | ld r6, [c060]
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
mul r5, r7, #6.103515625e-05 | mul r0, r2, r1 | ld r7, [c063] | st r3, [0628]
mul r4, r6, r5 | st r7, [0656]
min r2, r0, #3.3569336665095761e-05
min r6, r4, #3.3569336665095761e-05
mul r1, r2, #256.0 | mul r5, r6, #256.0 | ld r2, [0621] | ld r6, [064f]
mul r0, r1, #64.0 | mul r4, r5, #64.0 | ld r1, [0623] | ld r5, [0651]
mul r3, r0, r1 | mul r7, r4, r5 | st r0, [062c]
addxy r3, r3, r2 | addxy r7, r7, r6 | st r4, [065a]
mov r2, r3 | mov r6, r7 | st r3, [062a] | ld r3, [0626]
mad r2, r3, #-0.0625 | st r7, [0658] | ld r7, [0654]
mad r6, r7, #-0.0625 | ld r3, [0628] | ld r7, [0656]
mul r0, r3, r1 | mul r4, r7, r5
mad r0, r2, #0.5 | mad r4, r6, #0.5 | ld r2, [062c] | ld r6, [065a]
mul r0, r2, r0 | mul r4, r6, r4
mad r0, r1, #-0.5 | mad r4, r5, #-0.5
mul r1, r2, r0 | mul r5, r6, r4 | ld r2, [062a] | ld r6, [0658]
mad r1, r2, #-0.5 | mad r5, r6, #-0.5
mul r2, r1, #-2.0 | mul r6, r5, #-2.0 | st r1, [062a]
st r2, [0620] | st r6, [064e]
mov r2, r1 | mov r6, r5 | st r5, [0658] | ld r1, [0626]
mad r2, r1, #0.03125 | mul r1, r3, r0 | ld r5, [0654] | ld r3, [062c]
mad r6, r5, #0.03125 | mul r5, r7, r4 | ld r7, [065a]
mad r1, r2, #0.5 | mad r5, r6, #0.5
mul r2, r3, r1 | mul r6, r7, r5 | ld r1, [0624] | ld r5, [0652]
mad r2, r0, #-0.5 | mad r6, r4, #-0.5 | ld r0, [062a] | ld r4, [0658]
mul r3, r2, #4.0 | mul r7, r6, #4.0
mul r2, r1, r0 | mul r6, r5, r4 | ld r0, [a16a] | ld r1, [0669] | ld r5, [0697]
subxy r1, r1, r0 | ld r4, [a16b] | st r0, [0668]
subxy r5, r5, r4 | mul r1, r1, r1 | st r4, [0696] | ld r0, [066b]
st r3, [0622] | st r7, [0650] | ld r4, [0699]
mul r5, r5, r5 | mul r3, r2, #-32.0 | ld r2, [c048] | st r1, [0676]
mul r7, r6, #-32.0 | mad r0, r0, -r1 | ld r6, [c048] | st r3, [0400] | ld r1, [066d]
mad r4, r4, -r5 | addxy r2, r0, r2 | st r7, [0402] | ld r3, [0770]
addxy r6, r4, r6 | subxy r3, r1, r3 | st r5, [06a4] | ld r5, [069b]
ph_wrap r0, r3, r2 | ld r7, [0770]
ph_wrap r4, r3, r6 | st r0, [066a]
subxy r2, r0, r2 | subxy r6, r4, r6 | st r4, [0698]
subxy r7, r5, r7 | mul r3, r3, r2 | ld r2, [c049]
mul r7, r7, r6 | mad r1, r3, #0.5 | ld r3, [066f] | ld r6, [c049]
mad r5, r7, #0.5 | subxy r1, r3, r1 | ld r7, [069d] | st r1, [066c]
subxy r5, r7, r5 | mad r3, r1, r2 | st r5, [069a] | ld r1, [c04d]
mad r7, r5, r6 | mul r3, r3, r1 | ld r5, [c04d] | st r3, [066e]
mul r7, r7, r5 | st r7, [069c] | ld r1, [c04c]
mul r2, r0, r1 | ph_tri r0, r3, -r0 | ld r5, [c04c]
mul r6, r4, r5 | ld r1, [c04b]
ph_clamp r2, r3, -r2 | ld r5, [c04b]
ph_clamp r6, r3, -r6
ph_tri r4, r3, -r4
addxy r3, r3, r2 | addxy r7, r7, r6
mul r2, r0, r0 | mul r6, r4, r4
mul r2, r2, r1 | mul r6, r6, r5 | ld r1, [c04a]
addxy r2, r2, r1 | ld r5, [c04a]
addxy r6, r6, r5 | mad r3, r0, r2 | ld r0, [0671] | ld r1, [c04e]
mad r7, r4, r6 | addxy r0, r0, r1 | ld r4, [069f] | ld r5, [c04e]
addxy r4, r4, r5 | ld r2, [a162] | ld r1, [c050]
mul r0, r0, r2 | ld r6, [a163] | ld r5, [c050]
mul r4, r4, r6 | ld r2, [c051]
min r0, r0, #1.0 | ld r6, [c051]
min r4, r4, #1.0 | st r0, [0670]
mul r3, r3, r0 | mul r7, r7, r4 | st r4, [069e]
mul r1, r3, r1 | mul r5, r7, r5 | st r3, [a122]
mad r1, r0, -r2 | mad r5, r4, -r6 | st r7, [a123] | ld r0, [065d]
add r1, #1.0 | add r5, #1.0 | ld r2, [a16a] | ld r4, [068b]
st r1, [0672] | st r5, [06a0]
ld r1, [c04f] | ld r6, [a16b] | st r2, [0660]
mul r1, r3, r1 | ld r5, [c04f] | ld r3, [0661] | st r6, [068e]
mul r5, r7, r5 | subxy r3, r3, r2 | ld r7, [068f] | st r1, [0674]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r5, [06a2] | ld r1, [065f] | ld r0, [c043]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r5, [068d] | ld r4, [c043]
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
subxy r2, r1, r3 | subxy r6, r5, r7 | st r1, [065e]
subxy r3, r3, #1.0 | subxy r7, r7, #1.0 | st r5, [068c]
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r3, r3, r0 | mul r7, r7, r4 | ld r0, [c041]
ph_clamp r2, r3, -r2 | ld r4, [c041]
ph_clamp r6, r3, -r6
mad r3, r2, r0 | mad r7, r6, r4 | ld r0, [c042]
mad r3, r1, r0 | ld r4, [c042]
mad r7, r5, r4 | ld r0, [c040]
mul r1, r1, r0 | ld r4, [c040] | ld r0, [065d]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [068b] | ld r2, [a16a]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [a16b] | st r2, [0666]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r3, [0667] | st r6, [0694]
mad r4, r5, r7 | subxy r3, r3, r2 | ld r7, [0695] | ld r1, [0665]
min r0, r0, #1.0 | ld r5, [0693]
min r4, r4, #1.0 | st r0, [065c] | ld r0, [0663]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r4, [068a] | ld r4, [0691] | ld r0, [c047]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r4, [c047]
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
subxy r2, r1, r3 | subxy r6, r5, r7 | st r1, [0664]
subxy r3, r3, #1.0 | subxy r7, r7, #1.0 | st r5, [0692]
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r3, r3, r0 | mul r7, r7, r4 | ld r0, [c045]
ph_clamp r2, r3, -r2 | ld r4, [c045]
ph_clamp r6, r3, -r6
mad r3, r2, r0 | mad r7, r6, r4 | ld r0, [c046]
mad r3, r1, r0 | ld r4, [c046]
mad r7, r5, r4 | ld r0, [c044]
mul r1, r1, r0 | ld r4, [c044] | ld r0, [0663]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [0691] | ld r2, [c054]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [c054]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r1, [a122]
mad r4, r5, r7 | ld r5, [a123] | ld r3, [c055]
min r0, r0, #1.0 | ld r7, [c055]
min r4, r4, #1.0 | st r0, [0662] | ld r0, [a153]
mad r2, r1, r3 | mad r6, r5, r7 | ld r1, [0678] | ld r5, [06a6] | ld r3, [c056]
mad r2, r1, r3 | ld r7, [c056] | st r4, [0690] | ld r1, [0662]
mad r6, r5, r7 | ld r5, [0690] | ld r3, [c057] | st r0, [0682]
mad r2, r1, r3 | ld r7, [c057] | ld r1, [065c]
mad r6, r5, r7 | ld r5, [068a] | ld r3, [c058]
mad r2, r1, r3 | ld r7, [c058] | ld r1, [a132]
mad r6, r5, r7 | ld r5, [a133] | ld r3, [c059]
mad r2, r1, r3 | ld r7, [c059] | ld r1, [a122]
mad r6, r5, r7 | st r2, [a13a] | ld r2, [c05a]
st r6, [a13b] | ld r6, [c05a]
ld r5, [a123] | ld r3, [c05b]
mad r2, r1, r3 | ld r7, [c05b] | ld r1, [0678]
mad r6, r5, r7 | ld r5, [06a6] | ld r3, [c05c]
mad r2, r1, r3 | ld r7, [c05c] | ld r1, [0662]
mad r6, r5, r7 | ld r5, [0690] | ld r3, [c05d]
mad r2, r1, r3 | ld r7, [c05d] | ld r1, [065c]
mad r6, r5, r7 | ld r5, [068a] | ld r3, [c05e]
mad r2, r1, r3 | ld r7, [c05e] | ld r1, [a132]
mad r6, r5, r7 | ld r5, [a133] | ld r3, [c05f]
mad r2, r1, r3 | ld r7, [c05f] | ld r4, [a154]
mad r6, r5, r7 | st r2, [a142] | ld r1, [065c] | st r4, [06b0]
st r6, [a143] | ld r5, [068a]
ld r2, [0672] | ld r6, [06a0] | ld r0, [a1b2]
mul r3, r1, r2 | mul r7, r5, r6 | ld r4, [a1b3] | ld r1, [0662] | ld r5, [0690]
st r3, [0680] | st r7, [06ae]
addxy r0, r0, #-4.75 | addxy r4, r4, #-4.75 | ld r3, [c062]
mul r3, r3, r0 | ld r7, [c062]
mul r7, r7, r4 | ld r2, [c061]
mul r0, r1, r2 | ld r6, [c061] | ld r1, [0674]
mul r4, r5, r6 | mad r0, r3, #0.125 | ld r5, [06a2] | ld r2, [c060]
mad r4, r7, #0.125 | mad r0, r1, #0.125 | ld r6, [c060]
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
mul r5, r7, #6.103515625e-05 | mul r0, r2, r1 | ld r7, [c063] | st r3, [0684]
mul r4, r6, r5 | st r7, [06b2]
min r2, r0, #3.3569336665095761e-05
min r6, r4, #3.3569336665095761e-05
mul r1, r2, #256.0 | mul r5, r6, #256.0 | ld r2, [067d] | ld r6, [06ab]
mul r0, r1, #64.0 | mul r4, r5, #64.0 | ld r1, [067f] | ld r5, [06ad]
mul r3, r0, r1 | mul r7, r4, r5 | st r0, [0688]
addxy r3, r3, r2 | addxy r7, r7, r6 | st r4, [06b6]
mov r2, r3 | mov r6, r7 | st r3, [0686] | ld r3, [0682]
mad r2, r3, #-0.0625 | st r7, [06b4] | ld r7, [06b0]
mad r6, r7, #-0.0625 | ld r3, [0684] | ld r7, [06b2]
mul r0, r3, r1 | mul r4, r7, r5
mad r0, r2, #0.5 | mad r4, r6, #0.5 | ld r2, [0688] | ld r6, [06b6]
mul r0, r2, r0 | mul r4, r6, r4
mad r0, r1, #-0.5 | mad r4, r5, #-0.5
mul r1, r2, r0 | mul r5, r6, r4 | ld r2, [0686] | ld r6, [06b4]
mad r1, r2, #-0.5 | mad r5, r6, #-0.5
mul r2, r1, #-2.0 | mul r6, r5, #-2.0 | st r1, [0686]
st r2, [067c] | st r6, [06aa]
mov r2, r1 | mov r6, r5 | st r5, [06b4] | ld r1, [0682]
mad r2, r1, #0.03125 | mul r1, r3, r0 | ld r5, [06b0] | ld r3, [0688]
mad r6, r5, #0.03125 | mul r5, r7, r4 | ld r7, [06b6]
mad r1, r2, #0.5 | mad r5, r6, #0.5
mul r2, r3, r1 | mul r6, r7, r5 | ld r1, [0680] | ld r5, [06ae]
mad r2, r0, #-0.5 | mad r6, r4, #-0.5 | ld r0, [0686] | ld r4, [06b4]
mul r3, r2, #4.0 | mul r7, r6, #4.0
mul r2, r1, r0 | mul r6, r5, r4 | ld r0, [a16c] | ld r1, [06c5] | ld r5, [06f3]
subxy r1, r1, r0 | ld r4, [a16d] | st r0, [06c4]
subxy r5, r5, r4 | mul r1, r1, r1 | st r4, [06f2] | ld r0, [06c7]
st r3, [067e] | st r7, [06ac] | ld r4, [06f5]
mul r5, r5, r5 | mul r3, r2, #-32.0 | ld r2, [c048] | st r1, [06d2]
mul r7, r6, #-32.0 | mad r0, r0, -r1 | ld r6, [c048] | st r3, [0404] | ld r1, [06c9]
mad r4, r4, -r5 | addxy r2, r0, r2 | st r7, [0406] | ld r3, [0770]
addxy r6, r4, r6 | subxy r3, r1, r3 | st r5, [0700] | ld r5, [06f7]
ph_wrap r0, r3, r2 | ld r7, [0770]
ph_wrap r4, r3, r6 | st r0, [06c6]
subxy r2, r0, r2 | subxy r6, r4, r6 | st r4, [06f4]
subxy r7, r5, r7 | mul r3, r3, r2 | ld r2, [c049]
mul r7, r7, r6 | mad r1, r3, #0.5 | ld r3, [06cb] | ld r6, [c049]
mad r5, r7, #0.5 | subxy r1, r3, r1 | ld r7, [06f9] | st r1, [06c8]
subxy r5, r7, r5 | mad r3, r1, r2 | st r5, [06f6] | ld r1, [c04d]
mad r7, r5, r6 | mul r3, r3, r1 | ld r5, [c04d] | st r3, [06ca]
mul r7, r7, r5 | st r7, [06f8] | ld r1, [c04c]
mul r2, r0, r1 | ph_tri r0, r3, -r0 | ld r5, [c04c]
mul r6, r4, r5 | ld r1, [c04b]
ph_clamp r2, r3, -r2 | ld r5, [c04b]
ph_clamp r6, r3, -r6
ph_tri r4, r3, -r4
addxy r3, r3, r2 | addxy r7, r7, r6
mul r2, r0, r0 | mul r6, r4, r4
mul r2, r2, r1 | mul r6, r6, r5 | ld r1, [c04a]
addxy r2, r2, r1 | ld r5, [c04a]
addxy r6, r6, r5 | mad r3, r0, r2 | ld r0, [06cd] | ld r1, [c04e]
mad r7, r4, r6 | addxy r0, r0, r1 | ld r4, [06fb] | ld r5, [c04e]
addxy r4, r4, r5 | ld r2, [a164] | ld r1, [c050]
mul r0, r0, r2 | ld r6, [a165] | ld r5, [c050]
mul r4, r4, r6 | ld r2, [c051]
min r0, r0, #1.0 | ld r6, [c051]
min r4, r4, #1.0 | st r0, [06cc]
mul r3, r3, r0 | mul r7, r7, r4 | st r4, [06fa]
mul r1, r3, r1 | mul r5, r7, r5 | st r3, [a124]
mad r1, r0, -r2 | mad r5, r4, -r6 | st r7, [a125] | ld r0, [06b9]
add r1, #1.0 | add r5, #1.0 | ld r2, [a16c] | ld r4, [06e7]
st r1, [06ce] | st r5, [06fc]
ld r1, [c04f] | ld r6, [a16d] | st r2, [06bc]
mul r1, r3, r1 | ld r5, [c04f] | ld r3, [06bd] | st r6, [06ea]
mul r5, r7, r5 | subxy r3, r3, r2 | ld r7, [06eb] | st r1, [06d0]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r5, [06fe] | ld r1, [06bb] | ld r0, [c043]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r5, [06e9] | ld r4, [c043]
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
subxy r2, r1, r3 | subxy r6, r5, r7 | st r1, [06ba]
subxy r3, r3, #1.0 | subxy r7, r7, #1.0 | st r5, [06e8]
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r3, r3, r0 | mul r7, r7, r4 | ld r0, [c041]
ph_clamp r2, r3, -r2 | ld r4, [c041]
ph_clamp r6, r3, -r6
mad r3, r2, r0 | mad r7, r6, r4 | ld r0, [c042]
mad r3, r1, r0 | ld r4, [c042]
mad r7, r5, r4 | ld r0, [c040]
mul r1, r1, r0 | ld r4, [c040] | ld r0, [06b9]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [06e7] | ld r2, [a16c]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [a16d] | st r2, [06c2]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r3, [06c3] | st r6, [06f0]
mad r4, r5, r7 | subxy r3, r3, r2 | ld r7, [06f1] | ld r1, [06c1]
min r0, r0, #1.0 | ld r5, [06ef]
min r4, r4, #1.0 | st r0, [06b8] | ld r0, [06bf]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r4, [06e6] | ld r4, [06ed] | ld r0, [c047]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r4, [c047]
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
subxy r2, r1, r3 | subxy r6, r5, r7 | st r1, [06c0]
subxy r3, r3, #1.0 | subxy r7, r7, #1.0 | st r5, [06ee]
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r3, r3, r0 | mul r7, r7, r4 | ld r0, [c045]
ph_clamp r2, r3, -r2 | ld r4, [c045]
ph_clamp r6, r3, -r6
mad r3, r2, r0 | mad r7, r6, r4 | ld r0, [c046]
mad r3, r1, r0 | ld r4, [c046]
mad r7, r5, r4 | ld r0, [c044]
mul r1, r1, r0 | ld r4, [c044] | ld r0, [06bf]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [06ed] | ld r2, [c054]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [c054]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r1, [a124]
mad r4, r5, r7 | ld r5, [a125] | ld r3, [c055]
min r0, r0, #1.0 | ld r7, [c055]
min r4, r4, #1.0 | st r0, [06be] | ld r0, [a155]
mad r2, r1, r3 | mad r6, r5, r7 | ld r1, [06d4] | ld r5, [0702] | ld r3, [c056]
mad r2, r1, r3 | ld r7, [c056] | st r4, [06ec] | ld r1, [06be]
mad r6, r5, r7 | ld r5, [06ec] | ld r3, [c057] | st r0, [06de]
mad r2, r1, r3 | ld r7, [c057] | ld r1, [06b8]
mad r6, r5, r7 | ld r5, [06e6] | ld r3, [c058]
mad r2, r1, r3 | ld r7, [c058] | ld r1, [a134]
mad r6, r5, r7 | ld r5, [a135] | ld r3, [c059]
mad r2, r1, r3 | ld r7, [c059] | ld r1, [a124]
mad r6, r5, r7 | st r2, [a13c] | ld r2, [c05a]
st r6, [a13d] | ld r6, [c05a]
ld r5, [a125] | ld r3, [c05b]
mad r2, r1, r3 | ld r7, [c05b] | ld r1, [06d4]
mad r6, r5, r7 | ld r5, [0702] | ld r3, [c05c]
mad r2, r1, r3 | ld r7, [c05c] | ld r1, [06be]
mad r6, r5, r7 | ld r5, [06ec] | ld r3, [c05d]
mad r2, r1, r3 | ld r7, [c05d] | ld r1, [06b8]
mad r6, r5, r7 | ld r5, [06e6] | ld r3, [c05e]
mad r2, r1, r3 | ld r7, [c05e] | ld r1, [a134]
mad r6, r5, r7 | ld r5, [a135] | ld r3, [c05f]
mad r2, r1, r3 | ld r7, [c05f] | ld r4, [a156]
mad r6, r5, r7 | st r2, [a144] | ld r1, [06b8] | st r4, [070c]
st r6, [a145] | ld r5, [06e6]
ld r2, [06ce] | ld r6, [06fc] | ld r0, [a1b4]
mul r3, r1, r2 | mul r7, r5, r6 | ld r4, [a1b5] | ld r1, [06be] | ld r5, [06ec]
st r3, [06dc] | st r7, [070a]
addxy r0, r0, #-4.75 | addxy r4, r4, #-4.75 | ld r3, [c062]
mul r3, r3, r0 | ld r7, [c062]
mul r7, r7, r4 | ld r2, [c061]
mul r0, r1, r2 | ld r6, [c061] | ld r1, [06d0]
mul r4, r5, r6 | mad r0, r3, #0.125 | ld r5, [06fe] | ld r2, [c060]
mad r4, r7, #0.125 | mad r0, r1, #0.125 | ld r6, [c060]
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
mul r5, r7, #6.103515625e-05 | mul r0, r2, r1 | ld r7, [c063] | st r3, [06e0]
mul r4, r6, r5 | st r7, [070e]
min r2, r0, #3.3569336665095761e-05
min r6, r4, #3.3569336665095761e-05
mul r1, r2, #256.0 | mul r5, r6, #256.0 | ld r2, [06d9] | ld r6, [0707]
mul r0, r1, #64.0 | mul r4, r5, #64.0 | ld r1, [06db] | ld r5, [0709]
mul r3, r0, r1 | mul r7, r4, r5 | st r0, [06e4]
addxy r3, r3, r2 | addxy r7, r7, r6 | st r4, [0712]
mov r2, r3 | mov r6, r7 | st r3, [06e2] | ld r3, [06de]
mad r2, r3, #-0.0625 | st r7, [0710] | ld r7, [070c]
mad r6, r7, #-0.0625 | ld r3, [06e0] | ld r7, [070e]
mul r0, r3, r1 | mul r4, r7, r5
mad r0, r2, #0.5 | mad r4, r6, #0.5 | ld r2, [06e4] | ld r6, [0712]
mul r0, r2, r0 | mul r4, r6, r4
mad r0, r1, #-0.5 | mad r4, r5, #-0.5
mul r1, r2, r0 | mul r5, r6, r4 | ld r2, [06e2] | ld r6, [0710]
mad r1, r2, #-0.5 | mad r5, r6, #-0.5
mul r2, r1, #-2.0 | mul r6, r5, #-2.0 | st r1, [06e2]
st r2, [06d8] | st r6, [0706]
mov r2, r1 | mov r6, r5 | st r5, [0710] | ld r1, [06de]
mad r2, r1, #0.03125 | mul r1, r3, r0 | ld r5, [070c] | ld r3, [06e4]
mad r6, r5, #0.03125 | mul r5, r7, r4 | ld r7, [0712]
mad r1, r2, #0.5 | mad r5, r6, #0.5
mul r2, r3, r1 | mul r6, r7, r5 | ld r1, [06dc] | ld r5, [070a]
mad r2, r0, #-0.5 | mad r6, r4, #-0.5 | ld r0, [06e2] | ld r4, [0710]
mul r3, r2, #4.0 | mul r7, r6, #4.0
mul r2, r1, r0 | mul r6, r5, r4 | st r3, [06da]
mul r3, r2, #-32.0 | mul r7, r6, #-32.0 | st r7, [0708]
st r3, [0408] | st r7, [040a]
