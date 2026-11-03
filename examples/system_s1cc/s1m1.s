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
subxy r1, r1, r0 | ph_wrap r4, r3, r6 | st r0, [060c] | ld r0, [060f]
ph_wrap r2, r3, r2 | st r7, [6007]
mul r1, r1, r1 | subxy r6, r4, r6 | st r2, [0770] | ld r2, [c048]
mad r0, r0, -r1 | st r1, [061a] | ld r1, [0611]
addxy r2, r0, r2 | ld r3, [0770] | st r4, [063c]
ph_wrap r0, r3, r2 | ld r7, [0770]
subxy r2, r0, r2 | subxy r3, r1, r3 | st r0, [060e]
subxy r7, r5, r7 | mul r3, r3, r2 | ld r2, [c049]
mul r7, r7, r6 | mad r1, r3, #0.5 | ld r3, [0613] | ld r6, [c049]
mad r5, r7, #0.5 | subxy r1, r3, r1 | ld r7, [0641] | st r1, [0610]
subxy r5, r7, r5 | mad r3, r1, r2 | st r5, [063e] | ld r1, [c04d]
mad r7, r5, r6 | mul r3, r3, r1 | ld r5, [c04d] | st r3, [0612]
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
mul r1, r3, r1 | mul r5, r7, r5 | st r3, [a120] | st r3, [0510]
mad r1, r0, -r2 | mad r5, r4, -r6 | ld r2, [061f] | ld r6, [064d]
add r1, #1.0 | add r5, #1.0 | ld r0, [0521] | ld r4, [0523]
st r1, [0616] | st r5, [0644]
ld r1, [c04f] | st r7, [a121]
mul r1, r3, r1 | ld r5, [c04f] | st r7, [0512]
mul r5, r7, r5 | st r1, [0618] | ld r1, [061a] | ld r3, [c053]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | st r5, [0646] | ld r5, [0648] | ld r7, [c053]
mad r6, r6, -r5 | addxy r5, r4, #-0.999
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [061e] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [064c]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7 | ld r3, [0605] | ld r7, [0633]
mul r2, r2, #1.25 | mul r6, r6, #1.25
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r2, [a168] | ld r1, [0603] | ld r5, [0631]
min r0, r0, #1.0 | ld r6, [a169] | st r2, [0604]
min r4, r4, #1.0 | st r0, [0520] | st r0, [a128] | ld r0, [0541]
subxy r3, r3, r2 | subxy r7, r7, r6 | st r4, [0522] | st r4, [a129] | ld r4, [0543]
addxy r2, r0, #-0.999 | addxy r6, r4, #-0.999 | st r6, [0632] | ld r0, [c043]
mul r2, r2, #256.0 | mul r6, r6, #256.0 | ld r4, [c043]
mul r3, r3, r3 | mul r7, r7, r7
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r3, #1.0 | subxy r7, r7, #1.0
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r1, r1, r3 | mul r5, r5, r7 | ld r3, [a160]
ph_clamp r2, r3, -r2 | ld r7, [a161]
ph_clamp r6, r3, -r6 | max r1, r1, r2
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
mul r1, r1, r0 | ld r4, [c040] | ld r0, [0541]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [0543] | ld r2, [a168]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [a169] | st r2, [060a]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r3, [060b] | st r6, [0638]
mad r4, r5, r7 | subxy r3, r3, r2 | ld r7, [0639] | ld r1, [0609]
min r0, r0, #1.0 | ld r5, [0637]
min r4, r4, #1.0 | st r0, [0540] | ld r0, [0531]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r4, [0542] | ld r4, [0533] | ld r0, [c047]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r4, [c047]
mul r6, r6, #256.0 | mul r3, r3, r3
mul r7, r7, r7 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r7, r7, #1.0 | mul r2, r2, #256.0
mul r6, r6, #256.0 | mul r1, r1, r3 | ld r3, [a160]
mul r5, r5, r7 | ld r7, [a161]
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6 | max r1, r1, r2
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
mul r1, r1, r0 | ld r4, [c044] | ld r0, [0531]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [0533] | ld r2, [0616]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [0644]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r1, [a130]
mad r4, r5, r7 | ld r5, [a131] | st r1, [0550]
min r0, r0, #1.0 | st r5, [0552] | ld r1, [0540]
min r4, r4, #1.0 | st r0, [0530] | ld r0, [a151]
mul r3, r1, r2 | st r4, [0532] | ld r4, [a152] | ld r2, [c061]
ld r5, [0542] | st r0, [0626] | st r4, [0654]
mul r7, r5, r6 | ld r0, [a1b0] | st r3, [0624] | ld r3, [c062]
addxy r0, r0, #-4.75 | ld r4, [a1b1] | st r7, [0652] | ld r7, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | ld r1, [0530] | ld r5, [0532] | ld r6, [c061]
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [0618] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [0646] | ld r6, [c060]
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
mul r0, r2, r1 | mul r4, r6, r5 | ld r7, [c063] | st r3, [0628]
min r2, r0, #3.3569336665095761e-05 | st r7, [0656] | ld r0, [0621]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [0623] | ld r4, [064f]
shl r5, r6, 14 | ld r6, [0651]
mul r3, r1, r2 | mul r7, r5, r6
addxy r3, r3, r0 | addxy r7, r7, r4
mov r0, r3 | mov r4, r7 | st r3, [062a] | ld r3, [0626]
mad r0, r3, #-0.0625 | st r7, [0658] | ld r7, [0654]
mad r4, r7, #-0.0625 | ld r3, [0628] | ld r7, [0656]
mac_p1wc3 r0, r3, r2, #0.5, r0
mac_p1wc3 r4, r7, r6, #0.5, r4
mac_p1wc3 r0, r1, r0, #-0.5, r2 | ld r2, [062a]
mac_p1wc3 r4, r5, r4, #-0.5, r6 | ld r6, [0658] | st r1, [062a]
mac_p1wc3 r2, r1, r0, #-0.5, r2 | st r5, [0658]
mac_p1wc3 r6, r5, r4, #-0.5, r6 | st r2, [062c]
mul r1, r2, #-2.0 | mul r5, r6, #-2.0 | st r6, [065a]
st r1, [0620] | st r5, [064e]
mov r1, r2 | mov r5, r6 | ld r2, [0626] | ld r6, [0654]
mad r1, r2, #0.03125 | mad r5, r6, #0.03125
mac_p1wc3 r2, r3, r0, #0.5, r1 | ld r3, [062a]
mac_p1wc3 r6, r7, r4, #0.5, r5 | ld r7, [0658]
mac_p1wc3 r1, r3, r2, #-0.5, r0 | ld r2, [0624] | ld r0, [062c]
mac_p1wc3 r5, r7, r6, #-0.5, r4 | ld r6, [0652] | ld r4, [065a]
mul r3, r1, #4.0 | mul r7, r5, #4.0
mul r1, r2, r0 | mul r5, r6, r4 | st r3, [0622] | ld r0, [a16a]
mul r3, r1, #-32.0 | mul r7, r5, #-32.0 | st r7, [0650] | ld r4, [a16b]
ld r1, [0669] | ld r5, [0697] | ld r2, [c048]
subxy r1, r1, r0 | subxy r5, r5, r4 | st r0, [0668] | ld r0, [066b] | ld r6, [c048]
mul r1, r1, r1 | mul r5, r5, r5 | st r4, [0696] | ld r4, [0699]
mad r0, r0, -r1 | mad r4, r4, -r5 | st r3, [0400] | ld r3, [0770]
addxy r2, r0, r2 | addxy r6, r4, r6 | st r7, [0402] | ld r7, [0770]
st r1, [0676] | st r5, [06a4] | ld r1, [066d]
ph_wrap r0, r3, r2 | ld r5, [069b]
ph_wrap r4, r3, r6 | st r0, [066a]
subxy r2, r0, r2 | subxy r6, r4, r6 | st r4, [0698]
subxy r3, r1, r3 | subxy r7, r5, r7
mul r3, r3, r2 | mul r7, r7, r6 | ld r2, [c049]
mad r1, r3, #0.5 | mad r5, r7, #0.5 | ld r3, [066f] | ld r7, [069d] | ld r6, [c049]
st r1, [066c] | st r5, [069a]
subxy r1, r3, r1 | subxy r5, r7, r5
mad r3, r1, r2 | mad r7, r5, r6 | ld r1, [c04d]
mul r3, r3, r1 | ld r5, [c04d] | st r3, [066e]
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
mul r1, r3, r1 | mul r5, r7, r5 | st r3, [a122] | st r3, [0514]
mad r1, r0, -r2 | mad r5, r4, -r6 | ld r2, [067b] | ld r6, [06a9]
add r1, #1.0 | add r5, #1.0 | ld r0, [0525] | ld r4, [0527]
st r1, [0672] | st r5, [06a0]
ld r1, [c04f] | st r7, [a123]
mul r1, r3, r1 | ld r5, [c04f] | st r7, [0516]
mul r5, r7, r5 | st r1, [0674] | ld r1, [0676] | ld r3, [c053]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | st r5, [06a2] | ld r5, [06a4] | ld r7, [c053]
mad r6, r6, -r5 | addxy r5, r4, #-0.999
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [067a] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [06a8]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7 | ld r3, [0661] | ld r7, [068f]
mul r2, r2, #1.25 | mul r6, r6, #1.25
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r2, [a16a] | ld r1, [065f] | ld r5, [068d]
min r0, r0, #1.0 | ld r6, [a16b] | st r2, [0660]
min r4, r4, #1.0 | st r0, [0524] | st r0, [a12a] | ld r0, [0545]
subxy r3, r3, r2 | subxy r7, r7, r6 | st r4, [0526] | st r4, [a12b] | ld r4, [0547]
addxy r2, r0, #-0.999 | addxy r6, r4, #-0.999 | st r6, [068e] | ld r0, [c043]
mul r2, r2, #256.0 | mul r6, r6, #256.0 | ld r4, [c043]
mul r3, r3, r3 | mul r7, r7, r7
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r3, #1.0 | subxy r7, r7, #1.0
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r1, r1, r3 | mul r5, r5, r7 | ld r3, [a162]
ph_clamp r2, r3, -r2 | ld r7, [a163]
ph_clamp r6, r3, -r6 | max r1, r1, r2
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
mul r1, r1, r0 | ld r4, [c040] | ld r0, [0545]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [0547] | ld r2, [a16a]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [a16b] | st r2, [0666]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r3, [0667] | st r6, [0694]
mad r4, r5, r7 | subxy r3, r3, r2 | ld r7, [0695] | ld r1, [0665]
min r0, r0, #1.0 | ld r5, [0693]
min r4, r4, #1.0 | st r0, [0544] | ld r0, [0535]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r4, [0546] | ld r4, [0537] | ld r0, [c047]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r4, [c047]
mul r6, r6, #256.0 | mul r3, r3, r3
mul r7, r7, r7 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r7, r7, #1.0 | mul r2, r2, #256.0
mul r6, r6, #256.0 | mul r1, r1, r3 | ld r3, [a162]
mul r5, r5, r7 | ld r7, [a163]
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6 | max r1, r1, r2
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
mul r1, r1, r0 | ld r4, [c044] | ld r0, [0535]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [0537] | ld r2, [0672]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [06a0]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r1, [a132]
mad r4, r5, r7 | ld r5, [a133] | st r1, [0554]
min r0, r0, #1.0 | st r5, [0556] | ld r1, [0544]
min r4, r4, #1.0 | st r0, [0534] | ld r0, [a153]
mul r3, r1, r2 | st r4, [0536] | ld r4, [a154] | ld r2, [c061]
ld r5, [0546] | st r0, [0682] | st r4, [06b0]
mul r7, r5, r6 | ld r0, [a1b2] | st r3, [0680] | ld r3, [c062]
addxy r0, r0, #-4.75 | ld r4, [a1b3] | st r7, [06ae] | ld r7, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | ld r1, [0534] | ld r5, [0536] | ld r6, [c061]
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [0674] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [06a2] | ld r6, [c060]
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
mul r0, r2, r1 | mul r4, r6, r5 | ld r7, [c063] | st r3, [0684]
min r2, r0, #3.3569336665095761e-05 | st r7, [06b2] | ld r0, [067d]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [067f] | ld r4, [06ab]
shl r5, r6, 14 | ld r6, [06ad]
mul r3, r1, r2 | mul r7, r5, r6
addxy r3, r3, r0 | addxy r7, r7, r4
mov r0, r3 | mov r4, r7 | st r3, [0686] | ld r3, [0682]
mad r0, r3, #-0.0625 | st r7, [06b4] | ld r7, [06b0]
mad r4, r7, #-0.0625 | ld r3, [0684] | ld r7, [06b2]
mac_p1wc3 r0, r3, r2, #0.5, r0
mac_p1wc3 r4, r7, r6, #0.5, r4
mac_p1wc3 r0, r1, r0, #-0.5, r2 | ld r2, [0686]
mac_p1wc3 r4, r5, r4, #-0.5, r6 | ld r6, [06b4] | st r1, [0686]
mac_p1wc3 r2, r1, r0, #-0.5, r2 | st r5, [06b4]
mac_p1wc3 r6, r5, r4, #-0.5, r6 | st r2, [0688]
mul r1, r2, #-2.0 | mul r5, r6, #-2.0 | st r6, [06b6]
st r1, [067c] | st r5, [06aa]
mov r1, r2 | mov r5, r6 | ld r2, [0682] | ld r6, [06b0]
mad r1, r2, #0.03125 | mad r5, r6, #0.03125
mac_p1wc3 r2, r3, r0, #0.5, r1 | ld r3, [0686]
mac_p1wc3 r6, r7, r4, #0.5, r5 | ld r7, [06b4]
mac_p1wc3 r1, r3, r2, #-0.5, r0 | ld r2, [0680] | ld r0, [0688]
mac_p1wc3 r5, r7, r6, #-0.5, r4 | ld r6, [06ae] | ld r4, [06b6]
mul r3, r1, #4.0 | mul r7, r5, #4.0
mul r1, r2, r0 | mul r5, r6, r4 | st r3, [067e] | ld r0, [a16c]
mul r3, r1, #-32.0 | mul r7, r5, #-32.0 | st r7, [06ac] | ld r4, [a16d]
ld r1, [06c5] | ld r5, [06f3] | ld r2, [c048]
subxy r1, r1, r0 | subxy r5, r5, r4 | st r0, [06c4] | ld r0, [06c7] | ld r6, [c048]
mul r1, r1, r1 | mul r5, r5, r5 | st r4, [06f2] | ld r4, [06f5]
mad r0, r0, -r1 | mad r4, r4, -r5 | st r3, [0404] | ld r3, [0770]
addxy r2, r0, r2 | addxy r6, r4, r6 | st r7, [0406] | ld r7, [0770]
st r1, [06d2] | st r5, [0700] | ld r1, [06c9]
ph_wrap r0, r3, r2 | ld r5, [06f7]
ph_wrap r4, r3, r6 | st r0, [06c6]
subxy r2, r0, r2 | subxy r6, r4, r6 | st r4, [06f4]
subxy r3, r1, r3 | subxy r7, r5, r7
mul r3, r3, r2 | mul r7, r7, r6 | ld r2, [c049]
mad r1, r3, #0.5 | mad r5, r7, #0.5 | ld r3, [06cb] | ld r7, [06f9] | ld r6, [c049]
st r1, [06c8] | st r5, [06f6]
subxy r1, r3, r1 | subxy r5, r7, r5
mad r3, r1, r2 | mad r7, r5, r6 | ld r1, [c04d]
mul r3, r3, r1 | ld r5, [c04d] | st r3, [06ca]
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
mul r1, r3, r1 | mul r5, r7, r5 | st r3, [a124] | st r3, [0518]
mad r1, r0, -r2 | mad r5, r4, -r6 | ld r2, [06d7] | ld r6, [0705]
add r1, #1.0 | add r5, #1.0 | ld r0, [0529] | ld r4, [052b]
st r1, [06ce] | st r5, [06fc]
ld r1, [c04f] | st r7, [a125]
mul r1, r3, r1 | ld r5, [c04f] | st r7, [051a]
mul r5, r7, r5 | st r1, [06d0] | ld r1, [06d2] | ld r3, [c053]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | st r5, [06fe] | ld r5, [0700] | ld r7, [c053]
mad r6, r6, -r5 | addxy r5, r4, #-0.999
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [06d6] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [0704]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7 | ld r3, [06bd] | ld r7, [06eb]
mul r2, r2, #1.25 | mul r6, r6, #1.25
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r2, [a16c] | ld r1, [06bb] | ld r5, [06e9]
min r0, r0, #1.0 | ld r6, [a16d] | st r2, [06bc]
min r4, r4, #1.0 | st r0, [0528] | st r0, [a12c] | ld r0, [0549]
subxy r3, r3, r2 | subxy r7, r7, r6 | st r4, [052a] | st r4, [a12d] | ld r4, [054b]
addxy r2, r0, #-0.999 | addxy r6, r4, #-0.999 | st r6, [06ea] | ld r0, [c043]
mul r2, r2, #256.0 | mul r6, r6, #256.0 | ld r4, [c043]
mul r3, r3, r3 | mul r7, r7, r7
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r3, #1.0 | subxy r7, r7, #1.0
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r1, r1, r3 | mul r5, r5, r7 | ld r3, [a164]
ph_clamp r2, r3, -r2 | ld r7, [a165]
ph_clamp r6, r3, -r6 | max r1, r1, r2
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
mul r1, r1, r0 | ld r4, [c040] | ld r0, [0549]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [054b] | ld r2, [a16c]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [a16d] | st r2, [06c2]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r3, [06c3] | st r6, [06f0]
mad r4, r5, r7 | subxy r3, r3, r2 | ld r7, [06f1] | ld r1, [06c1]
min r0, r0, #1.0 | ld r5, [06ef]
min r4, r4, #1.0 | st r0, [0548] | ld r0, [0539]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r4, [054a] | ld r4, [053b] | ld r0, [c047]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r4, [c047]
mul r6, r6, #256.0 | mul r3, r3, r3
mul r7, r7, r7 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r7, r7, #1.0 | mul r2, r2, #256.0
mul r6, r6, #256.0 | mul r1, r1, r3 | ld r3, [a164]
mul r5, r5, r7 | ld r7, [a165]
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6 | max r1, r1, r2
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
mul r1, r1, r0 | ld r4, [c044] | ld r0, [0539]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [053b] | ld r2, [06ce]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [06fc]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r1, [a134]
mad r4, r5, r7 | ld r5, [a135] | st r1, [0558]
min r0, r0, #1.0 | st r5, [055a] | ld r1, [0548]
min r4, r4, #1.0 | st r0, [0538] | ld r0, [a155]
mul r3, r1, r2 | st r4, [053a] | ld r4, [a156] | ld r2, [c061]
ld r5, [054a] | st r0, [06de] | st r4, [070c]
mul r7, r5, r6 | ld r0, [a1b4] | st r3, [06dc] | ld r3, [c062]
addxy r0, r0, #-4.75 | ld r4, [a1b5] | st r7, [070a] | ld r7, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | ld r1, [0538] | ld r5, [053a] | ld r6, [c061]
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [06d0] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [06fe] | ld r6, [c060]
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
mul r0, r2, r1 | mul r4, r6, r5 | ld r7, [c063] | st r3, [06e0]
min r2, r0, #3.3569336665095761e-05 | st r7, [070e] | ld r0, [06d9]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [06db] | ld r4, [0707]
shl r5, r6, 14 | ld r6, [0709]
mul r3, r1, r2 | mul r7, r5, r6
addxy r3, r3, r0 | addxy r7, r7, r4
mov r0, r3 | mov r4, r7 | st r3, [06e2] | ld r3, [06de]
mad r0, r3, #-0.0625 | st r7, [0710] | ld r7, [070c]
mad r4, r7, #-0.0625 | ld r3, [06e0] | ld r7, [070e]
mac_p1wc3 r0, r3, r2, #0.5, r0
mac_p1wc3 r4, r7, r6, #0.5, r4
mac_p1wc3 r0, r1, r0, #-0.5, r2 | ld r2, [06e2]
mac_p1wc3 r4, r5, r4, #-0.5, r6 | ld r6, [0710] | st r1, [06e2]
mac_p1wc3 r2, r1, r0, #-0.5, r2 | st r5, [0710]
mac_p1wc3 r6, r5, r4, #-0.5, r6 | st r2, [06e4]
mul r1, r2, #-2.0 | mul r5, r6, #-2.0 | st r6, [0712]
st r1, [06d8] | st r5, [0706]
mov r1, r2 | mov r5, r6 | ld r2, [06de] | ld r6, [070c]
mad r1, r2, #0.03125 | mad r5, r6, #0.03125
mac_p1wc3 r2, r3, r0, #0.5, r1 | ld r3, [06e2]
mac_p1wc3 r6, r7, r4, #0.5, r5 | ld r7, [0710]
mac_p1wc3 r1, r3, r2, #-0.5, r0 | ld r2, [06dc] | ld r0, [06e4]
mac_p1wc3 r5, r7, r6, #-0.5, r4 | ld r6, [070a] | ld r4, [0712]
mul r3, r1, #4.0 | mul r7, r5, #4.0
mul r1, r2, r0 | mul r5, r6, r4 | st r3, [06da]
mul r3, r1, #-32.0 | mul r7, r5, #-32.0 | st r7, [0708]
st r3, [0408] | st r7, [040a]
