mov r7, #0.0 | ld r0, [a151] | ld r2, [0701] | ld r5, [062d]
addxy r7, r7, r0 | mul r2, r2, #7.31275 | ld r1, [a152]
addxy r7, r7, r1 | ld r0, [a153] | ld r6, [c000]
addxy r7, r7, r0 | ld r1, [a154] | ld r0, [0409]
addxy r7, r7, r1 | ld r1, [040b] | ld r4, [a169]
addxy r7, r7, r0 | subxy r5, r5, r4 | ld r0, [040d] | ld r3, [c090] | st r4, [062c]
addxy r7, r7, r1 | mul r5, r5, r5 | ld r1, [040f] | ld r4, [c099] | st r3, [0510]
addxy r7, r7, r0 | mov r0, #0.0 | st r5, [063a] | ld r5, [c050]
addxy r7, r7, r1 | st r0, [6008] | ld r1, [060d]
mul r7, r7, #0.16666666666666666 | st r0, [6009]
mul r7, r7, r6 | st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [0703]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [0702] | ld r0, [a168] | ld r7, [c091]
subxy r1, r1, r0 | mul r5, r7, r5 | st r0, [060c] | ld r0, [c098]
mul r1, r1, r1 | ld r6, [c051] | st r3, [a120]
ph_wrap r2, r3, r2 | st r1, [061a] | ld r1, [c050]
mul r1, r3, r1 | mad r5, r4, -r6 | st r2, [0700] | ld r2, [c051] | ld r6, [063f]
mad r1, r0, -r2 | add r5, #1.0 | ld r2, [061f] | ld r0, [0521]
add r1, #1.0 | st r5, [0636] | ld r5, [c04f] | ld r4, [0523]
mul r5, r7, r5 | st r1, [0616] | ld r1, [c04f]
mul r1, r3, r1 | st r5, [0638] | ld r5, [063a] | st r7, [a121]
mad r6, r6, -r5 | addxy r5, r4, #-0.999 | st r1, [0618] | ld r1, [061a] | ld r3, [c053]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | st r7, [0512] | ld r7, [c053]
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [061e] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [063e]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7 | ld r3, [0618] | ld r7, [0638]
mul r2, r2, #1.25 | mul r6, r6, #1.25 | st r3, [a1d8]
subxy r2, r0, r2 | subxy r6, r4, r6 | st r7, [a1d9]
mad r0, r2, r1 | mad r4, r6, r5 | ld r1, [a130] | ld r2, [0616] | ld r6, [0636]
min r0, r0, #1.0 | ld r5, [a131] | st r1, [0550] | ld r3, [c092]
min r4, r4, #1.0 | st r0, [0520] | st r0, [a128]
st r4, [0522] | st r4, [a129]
ld r0, [c080] | st r5, [0552]
ld r4, [c081] | st r0, [0540]
st r4, [0542] | ld r0, [c088]
ld r4, [c089] | st r0, [0530] | ld r0, [0540]
st r4, [0532] | ld r4, [0542] | st r0, [a1b8]
ld r1, [0530] | ld r5, [0532] | ld r0, [a16a]
st r4, [a1b9] | ld r7, [c093]
st r1, [a1c0] | ld r1, [064d] | st r0, [064c]
subxy r1, r1, r0 | st r5, [a1c1] | ld r5, [066d] | ld r0, [c09a]
mul r1, r1, r1 | ld r4, [a16b] | st r3, [0514]
subxy r5, r5, r4 | st r1, [065a] | ld r1, [c050]
mul r5, r5, r5 | mul r1, r3, r1 | st r2, [a1d0] | st r4, [066c]
st r5, [067a] | ld r5, [c050]
mul r5, r7, r5 | st r6, [a1d1] | ld r4, [c09b]
ld r2, [c051] | st r3, [a122]
mad r1, r0, -r2 | ld r6, [c051] | ld r2, [065f] | ld r0, [0525]
mad r5, r4, -r6 | add r1, #1.0 | ld r6, [067f] | ld r4, [0527]
add r5, #1.0 | st r1, [0656] | ld r1, [c04f]
mul r1, r3, r1 | st r5, [0676] | ld r5, [c04f]
mul r5, r7, r5 | st r1, [0658] | ld r1, [065a] | st r7, [a123]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | st r5, [0678] | ld r5, [067a] | ld r3, [c053]
mad r6, r6, -r5 | addxy r5, r4, #-0.999 | st r7, [0516] | ld r7, [c053]
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [065e] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [067e]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7 | ld r3, [0658] | ld r7, [0678]
mul r2, r2, #1.25 | mul r6, r6, #1.25 | st r3, [a1da]
subxy r2, r0, r2 | subxy r6, r4, r6 | st r7, [a1db]
mad r0, r2, r1 | mad r4, r6, r5 | ld r1, [a132] | ld r2, [0656] | ld r6, [0676]
min r0, r0, #1.0 | ld r5, [a133] | st r1, [0554] | ld r3, [c094]
min r4, r4, #1.0 | st r0, [0524] | st r0, [a12a]
st r4, [0526] | st r4, [a12b]
ld r0, [c082] | st r5, [0556]
ld r4, [c083] | st r0, [0544]
st r4, [0546] | ld r0, [c08a]
ld r4, [c08b] | st r0, [0534] | ld r0, [0544]
st r4, [0536] | ld r4, [0546] | st r0, [a1ba]
ld r1, [0534] | ld r5, [0536] | ld r0, [a16c]
st r4, [a1bb] | ld r7, [c095]
st r1, [a1c2] | ld r1, [068d] | st r0, [068c]
subxy r1, r1, r0 | st r5, [a1c3] | ld r5, [06ad] | ld r0, [c09c]
mul r1, r1, r1 | ld r4, [a16d] | st r3, [0518]
subxy r5, r5, r4 | st r1, [069a] | ld r1, [c050]
mul r5, r5, r5 | mul r1, r3, r1 | st r2, [a1d2] | st r4, [06ac]
st r5, [06ba] | ld r5, [c050]
mul r5, r7, r5 | st r6, [a1d3] | ld r4, [c09d]
ld r2, [c051] | st r3, [a124]
mad r1, r0, -r2 | ld r6, [c051] | ld r2, [069f] | ld r0, [0529]
mad r5, r4, -r6 | add r1, #1.0 | ld r6, [06bf] | ld r4, [052b]
add r5, #1.0 | st r1, [0696] | ld r1, [c04f]
mul r1, r3, r1 | st r5, [06b6] | ld r5, [c04f]
mul r5, r7, r5 | st r1, [0698] | ld r1, [069a] | st r7, [a125]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | st r5, [06b8] | ld r5, [06ba] | ld r3, [c053]
mad r6, r6, -r5 | addxy r5, r4, #-0.999 | st r7, [051a] | ld r7, [c053]
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [069e] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [06be]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7 | ld r3, [c069]
mul r2, r2, #1.25 | mul r6, r6, #1.25 | ld r7, [c069]
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r1, [a134]
min r0, r0, #1.0 | ld r5, [a135] | ld r2, [c067] | st r1, [0558]
min r4, r4, #1.0 | ld r6, [c067] | st r5, [055a] | ld r1, [a1ec]
mul r2, r2, r3 | mul r6, r6, r7 | st r0, [0528] | st r0, [a12c]
st r4, [052a] | st r4, [a12d]
ld r5, [a1ed] | ld r3, [c062] | st r1, [0188]
mul r2, r2, r3 | ld r7, [c062] | st r5, [019c]
mul r6, r6, r7 | ld r0, [c084]
ld r4, [c085] | st r0, [0548]
st r4, [054a] | ld r3, [c068]
ld r7, [c068]
ld r1, [c06a]
mul r3, r3, r1 | ld r5, [c06a]
mul r7, r7, r5 | ld r0, [c08c]
ld r4, [c08d] | st r0, [0538] | ld r0, [a1b4]
st r4, [053a] | ld r1, [c063] | ld r4, [a1b5]
mul r3, r3, r1 | mul r1, r0, r2 | ld r5, [c063]
mul r7, r7, r5 | mul r5, r4, r6
shl r2, r1, 10
shl r6, r5, 10 | min r1, r2, #0.25 | ld r2, [0188]
min r5, r6, #0.25 | ld r6, [019c] | st r1, [018c]
mul r3, r2, r3 | mul r7, r6, r7 | st r5, [01a0]
mul r3, r3, #256.0 | mul r7, r7, #256.0
addxy r1, r0, #-0.004999999888241291 | addxy r5, r4, #-0.004999999888241291 | st r3, [018a] | ld r3, [a13c]
mul r1, r1, #5.479451911627053 | mul r5, r5, #5.479451911627053 | st r7, [019e] | ld r7, [a13d]
max r3, r3, r1 | ld r1, [0181]
max r7, r7, r5 | st r3, [018e] | ld r5, [0195]
mov r3, r0 | mov r7, r4 | st r7, [01a2]
mad r3, r1, #0.5 | mad r7, r5, #0.5
mul r1, r3, #2.0 | mul r5, r7, #2.0
ph_wrap r3, r3, r1
ph_wrap r7, r3, r5 | st r3, [0180]
addxy r1, r3, #1.0 | addxy r5, r7, #1.0 | st r7, [0194]
st r3, [0190] | st r7, [01a4]
ph_wrap r1, r3, r1
ph_wrap r5, r3, r5
mul r3, r0, #2.0 | mul r7, r4, #2.0
min r2, r1, r3
min r6, r5, r7
mul r0, r0, #-2.0 | mul r4, r4, #-2.0
max r2, r2, r0 | st r0, [0192] | ld r0, [0188]
max r6, r6, r4 | st r4, [01a6] | ld r4, [019c]
mul r2, r0, r2 | mul r6, r4, r6
shl r2, r2, 13
shl r6, r6, 13
absadd r0, r2, #-2.0
absadd r4, r6, #-2.0
mul r2, r2, r0 | mul r6, r6, r4
addxy r0, r2, r1 | addxy r4, r6, r5 | ld r2, [018e] | ld r6, [01a2]
mov r1, r2 | mov r5, r6 | ld r2, [0190] | ld r6, [01a4]
mad r1, r2, #-0.68493148895338163 | mad r5, r6, #-0.68493148895338163
addxy r2, r1, #-0.65753423000762279 | addxy r6, r5, #-0.65753423000762279
mul r1, r2, #-1.4600000381469727 | mul r5, r6, #-1.4600000381469727
ph_wrap r2, r3, r1
ph_wrap r6, r3, r5 | min r1, r2, r3 | ld r3, [0192]
min r5, r6, r7 | ld r7, [01a6]
max r1, r1, r3 | ld r3, [0188]
max r5, r5, r7 | ld r7, [019c]
mul r3, r3, r1 | mul r7, r7, r5
shl r1, r3, 13
shl r5, r7, 13
absadd r3, r1, #-2.0
absadd r7, r5, #-2.0
mul r1, r1, r3 | mul r5, r5, r7
addxy r3, r1, r2 | addxy r7, r5, r6 | ld r2, [018e] | ld r6, [01a2]
subxy r1, r3, r0 | subxy r5, r7, r4
mad r2, r1, #-0.68493148895338163 | mad r6, r5, #-0.68493148895338163
addxy r3, r0, #1.0 | addxy r7, r4, #1.0
addxy r1, r2, #0.027397258945758795 | addxy r5, r6, #0.027397258945758795
mul r2, r3, r1 | mul r6, r7, r5 | ld r3, [c065]
mad r0, r2, #-0.73000001907348633 | mad r4, r6, #-0.73000001907348633 | ld r7, [c065]
mul r1, r3, r0 | mul r5, r7, r4 | ld r3, [018c] | ld r7, [01a0] | ld r2, [a144]
addxy r0, r3, #-0.0024999999441206455 | addxy r4, r7, #-0.0024999999441206455 | ld r6, [a145] | st r1, [018e]
mul r0, r0, #10.958903823254106 | mul r4, r4, #10.958903823254106 | st r5, [01a2]
max r2, r2, r0 | ld r0, [0183]
max r6, r6, r4 | ld r4, [0197] | st r2, [0192]
mov r1, r3 | mov r5, r7 | st r6, [01a6]
mad r1, r0, #0.25 | mad r5, r4, #0.25
mul r0, r1, #4.0 | mul r4, r5, #4.0
ph_wrap r1, r3, r0
ph_wrap r5, r3, r4 | st r1, [0182]
addxy r0, r1, #1.0 | addxy r4, r5, #1.0 | st r5, [0196]
st r1, [0188] | st r5, [019c]
ph_wrap r0, r3, r0
ph_wrap r4, r3, r4
mul r1, r3, #4.0 | mul r5, r7, #4.0
min r2, r0, r1
min r6, r4, r5
mul r3, r3, #-4.0 | mul r7, r7, #-4.0
max r2, r2, r3 | st r3, [018c] | ld r3, [018a]
max r6, r6, r7 | st r7, [01a0] | ld r7, [019e]
mul r2, r3, r2 | mul r6, r7, r6
shl r2, r2, 15
shl r6, r6, 15
absadd r3, r2, #-1.0
absadd r7, r6, #-1.0
mac_p1wc3 r2, r2, r3, #0.25, r0 | ld r3, [0192]
mac_p1wc3 r6, r6, r7, #0.25, r4 | ld r7, [01a6]
mov r0, r3 | mov r4, r7 | ld r3, [0188] | ld r7, [019c]
mad r0, r3, #-0.68493148895338163 | mad r4, r7, #-0.68493148895338163
addxy r3, r0, #-0.65753423000762279 | addxy r7, r4, #-0.65753423000762279
mul r0, r3, #-1.4600000381469727 | mul r4, r7, #-1.4600000381469727
ph_wrap r3, r3, r0
ph_wrap r7, r3, r4 | min r0, r3, r1 | ld r1, [018c]
min r4, r7, r5 | ld r5, [01a0]
max r0, r0, r1 | ld r1, [018a]
max r4, r4, r5 | ld r5, [019e]
mul r1, r1, r0 | mul r5, r5, r4
shl r0, r1, 15
shl r4, r5, 15
absadd r1, r0, #-1.0
absadd r5, r4, #-1.0
mac_p1wc3 r0, r0, r1, #0.25, r3 | ld r3, [0192]
mac_p1wc3 r4, r4, r5, #0.25, r7 | ld r7, [01a6]
subxy r1, r0, r2 | subxy r5, r4, r6
mad r1, r3, #-0.36500000953674316 | mad r5, r7, #-0.36500000953674316
addxy r0, r2, #0.25 | addxy r4, r6, #0.25
addxy r3, r1, #-0.0099999997764825821 | addxy r7, r5, #-0.0099999997764825821
mac_p1wc3 r1, r0, r3, #0.5, r2 | ld r0, [c066] | ld r3, [018e]
mac_p1wc3 r5, r4, r7, #0.5, r6 | ld r4, [c066] | ld r7, [01a2]
mac_p1wc3 r2, r1, r0, #0.125, r3 | ld r1, [a1cc] | ld r0, [0548]
mac_p1wc3 r6, r5, r4, #0.125, r7 | ld r5, [a1cd] | ld r4, [054a]
mad r2, r1, #0.25 | mad r6, r5, #0.25 | ld r3, [0696] | ld r7, [06b6]
mul r1, r0, r3 | mul r5, r4, r7 | st r2, [0192] | ld r2, [a1e4]
addxy r2, r2, #-4.75 | st r6, [01a6] | ld r6, [a1e5] | ld r3, [c061]
st r1, [018e] | st r5, [01a2] | ld r0, [0538]
addxy r6, r6, #-4.75 | ld r1, [c064] | ld r4, [053a]
mul r1, r1, r2 | ld r5, [c064]
mul r5, r5, r6 | ld r7, [c061]
mac_p1wc3 r2, r0, r3, #0.125, r1 | ld r0, [0698] | ld r3, [c060]
mac_p1wc3 r6, r4, r7, #0.125, r5 | ld r4, [06b8] | ld r7, [c060]
mad r2, r0, #0.125 | mad r6, r4, #0.125
mul r1, r2, #0.17328679513998632 | mul r5, r6, #0.17328679513998632
mov r0, #0.041666666666666664 | mov r4, #0.041666666666666664
ymacd r0, r1, #0.16666666666666666 | ymacd r4, r5, #0.16666666666666666
ymacd r0, r1, #0.5 | ymacd r4, r5, #0.5
ymacd r0, r1, #1.0 | ymacd r4, r5, #1.0
ymacd r0, r1, #1.0 | ymacd r4, r5, #1.0
mul r2, r0, r0 | mul r6, r4, r4
mul r1, r2, r2 | mul r5, r6, r6
mul r0, r1, r1 | mul r4, r5, r5
mul r2, r0, r0 | mul r6, r4, r4
mul r1, r2, r2 | mul r5, r6, r6
mul r0, r1, #6.103515625e-05 | mul r4, r5, #6.103515625e-05 | ld r1, [c06b]
mul r2, r3, r0 | mul r6, r7, r4 | ld r5, [c06b] | st r1, [018a]
min r3, r2, #3.3569336665095761e-05 | st r5, [019e] | ld r2, [0185]
min r7, r6, #3.3569336665095761e-05 | shl r0, r3, 14 | ld r3, [0187] | ld r6, [0199]
shl r4, r7, 14 | ld r7, [019b]
mul r1, r0, r3 | mul r5, r4, r7
addxy r1, r1, r2 | addxy r5, r5, r6
mov r2, r1 | mov r6, r5 | st r1, [018c] | ld r1, [0192]
mad r2, r1, #-0.25 | st r5, [01a0] | ld r5, [01a6]
mad r6, r5, #-0.25 | ld r1, [018a] | ld r5, [019e]
mac_p1wc3 r2, r1, r3, #0.5, r2
mac_p1wc3 r6, r5, r7, #0.5, r6
mac_p1wc3 r2, r0, r2, #-0.5, r3 | ld r3, [018c]
mac_p1wc3 r6, r4, r6, #-0.5, r7 | ld r7, [01a0] | st r0, [018c]
mac_p1wc3 r3, r0, r2, #-0.5, r3 | st r4, [01a0]
mac_p1wc3 r7, r4, r6, #-0.5, r7 | st r3, [0188]
mul r0, r3, #-2.0 | mul r4, r7, #-2.0 | st r7, [019c]
st r0, [0184] | st r4, [0198]
mov r0, r3 | mov r4, r7 | ld r3, [0192] | ld r7, [01a6]
mad r0, r3, #0.125 | mad r4, r7, #0.125
mac_p1wc3 r3, r1, r2, #0.5, r0 | ld r1, [018c]
mac_p1wc3 r7, r5, r6, #0.5, r4 | ld r5, [01a0]
mac_p1wc3 r0, r1, r3, #-0.5, r2 | ld r3, [018e] | ld r2, [0188]
mac_p1wc3 r4, r5, r7, #-0.5, r6 | ld r7, [01a2] | ld r6, [019c]
mul r1, r0, #4.0 | mul r5, r4, #4.0
mul r0, r3, r2 | mul r4, r7, r6 | st r1, [0186] | ld r3, [c096]
mul r1, r0, #-32.0 | mul r5, r4, #-32.0 | st r5, [019a] | ld r0, [a16e]
st r1, [0408] | st r5, [040a] | ld r1, [06cd]
subxy r1, r1, r0 | ld r4, [a16f] | ld r5, [06ed]
subxy r5, r5, r4 | mul r1, r1, r1 | ld r7, [c097] | st r0, [06cc]
mul r5, r5, r5 | st r1, [06da] | ld r1, [c050]
mul r1, r3, r1 | st r5, [06fa] | ld r5, [c050]
mul r5, r7, r5 | st r4, [06ec] | ld r0, [c09e]
ld r4, [c09f] | st r3, [a126]
ld r2, [c051] | st r7, [a127]
mad r1, r0, -r2 | ld r6, [c051] | ld r2, [06df] | ld r0, [052d]
mad r5, r4, -r6 | add r1, #1.0 | ld r6, [06ff] | ld r4, [052f]
add r5, #1.0 | st r1, [06d6] | ld r1, [c04f]
mul r1, r3, r1 | st r5, [06f6] | ld r5, [c04f]
mul r5, r7, r5 | st r1, [06d8] | ld r1, [06da]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | st r5, [06f8] | ld r5, [06fa]
mad r6, r6, -r5 | addxy r5, r4, #-0.999 | st r3, [051c] | ld r3, [c053]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r7, [051e] | ld r7, [c053]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [06de] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [06fe]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7 | ld r3, [c069]
mul r2, r2, #1.25 | mul r6, r6, #1.25 | ld r7, [c069]
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r1, [a136]
min r0, r0, #1.0 | ld r5, [a137] | ld r2, [c067] | st r1, [055c]
min r4, r4, #1.0 | ld r6, [c067] | st r5, [055e] | ld r1, [a1ee]
mul r2, r2, r3 | mul r6, r6, r7 | st r0, [052c] | st r0, [a12e]
st r4, [052e] | st r4, [a12f]
ld r5, [a1ef] | ld r3, [c062] | st r1, [01b0]
mul r2, r2, r3 | ld r7, [c062] | st r5, [01c4]
mul r6, r6, r7 | ld r0, [c086]
ld r4, [c087] | st r0, [054c]
st r4, [054e] | ld r3, [c068]
ld r7, [c068]
ld r1, [c06a]
mul r3, r3, r1 | ld r5, [c06a]
mul r7, r7, r5 | ld r0, [c08e]
ld r4, [c08f] | st r0, [053c] | ld r0, [a1b6]
st r4, [053e] | ld r1, [c063] | ld r4, [a1b7]
mul r3, r3, r1 | mul r1, r0, r2 | ld r5, [c063]
mul r7, r7, r5 | mul r5, r4, r6
shl r2, r1, 10
shl r6, r5, 10 | min r1, r2, #0.25 | ld r2, [01b0]
min r5, r6, #0.25 | ld r6, [01c4] | st r1, [01b4]
mul r3, r2, r3 | mul r7, r6, r7 | st r5, [01c8]
mul r3, r3, #256.0 | mul r7, r7, #256.0
addxy r1, r0, #-0.004999999888241291 | addxy r5, r4, #-0.004999999888241291 | st r3, [01b2] | ld r3, [a13e]
mul r1, r1, #5.479451911627053 | mul r5, r5, #5.479451911627053 | st r7, [01c6] | ld r7, [a13f]
max r3, r3, r1 | ld r1, [01a9]
max r7, r7, r5 | st r3, [01b6] | ld r5, [01bd]
mov r3, r0 | mov r7, r4 | st r7, [01ca]
mad r3, r1, #0.5 | mad r7, r5, #0.5
mul r1, r3, #2.0 | mul r5, r7, #2.0
ph_wrap r3, r3, r1
ph_wrap r7, r3, r5 | st r3, [01a8]
addxy r1, r3, #1.0 | addxy r5, r7, #1.0 | st r7, [01bc]
st r3, [01b8] | st r7, [01cc]
ph_wrap r1, r3, r1
ph_wrap r5, r3, r5
mul r3, r0, #2.0 | mul r7, r4, #2.0
min r2, r1, r3
min r6, r5, r7
mul r0, r0, #-2.0 | mul r4, r4, #-2.0
max r2, r2, r0 | st r0, [01ba] | ld r0, [01b0]
max r6, r6, r4 | st r4, [01ce] | ld r4, [01c4]
mul r2, r0, r2 | mul r6, r4, r6
shl r2, r2, 13
shl r6, r6, 13
absadd r0, r2, #-2.0
absadd r4, r6, #-2.0
mul r2, r2, r0 | mul r6, r6, r4
addxy r0, r2, r1 | addxy r4, r6, r5 | ld r2, [01b6] | ld r6, [01ca]
mov r1, r2 | mov r5, r6 | ld r2, [01b8] | ld r6, [01cc]
mad r1, r2, #-0.68493148895338163 | mad r5, r6, #-0.68493148895338163
addxy r2, r1, #-0.65753423000762279 | addxy r6, r5, #-0.65753423000762279
mul r1, r2, #-1.4600000381469727 | mul r5, r6, #-1.4600000381469727
ph_wrap r2, r3, r1
ph_wrap r6, r3, r5 | min r1, r2, r3 | ld r3, [01ba]
min r5, r6, r7 | ld r7, [01ce]
max r1, r1, r3 | ld r3, [01b0]
max r5, r5, r7 | ld r7, [01c4]
mul r3, r3, r1 | mul r7, r7, r5
shl r1, r3, 13
shl r5, r7, 13
absadd r3, r1, #-2.0
absadd r7, r5, #-2.0
mul r1, r1, r3 | mul r5, r5, r7
addxy r3, r1, r2 | addxy r7, r5, r6 | ld r2, [01b6] | ld r6, [01ca]
subxy r1, r3, r0 | subxy r5, r7, r4
mad r2, r1, #-0.68493148895338163 | mad r6, r5, #-0.68493148895338163
addxy r3, r0, #1.0 | addxy r7, r4, #1.0
addxy r1, r2, #0.027397258945758795 | addxy r5, r6, #0.027397258945758795
mul r2, r3, r1 | mul r6, r7, r5 | ld r3, [c065]
mad r0, r2, #-0.73000001907348633 | mad r4, r6, #-0.73000001907348633 | ld r7, [c065]
mul r1, r3, r0 | mul r5, r7, r4 | ld r3, [01b4] | ld r7, [01c8] | ld r2, [a146]
addxy r0, r3, #-0.0024999999441206455 | addxy r4, r7, #-0.0024999999441206455 | ld r6, [a147] | st r1, [01b6]
mul r0, r0, #10.958903823254106 | mul r4, r4, #10.958903823254106 | st r5, [01ca]
max r2, r2, r0 | ld r0, [01ab]
max r6, r6, r4 | ld r4, [01bf] | st r2, [01ba]
mov r1, r3 | mov r5, r7 | st r6, [01ce]
mad r1, r0, #0.25 | mad r5, r4, #0.25
mul r0, r1, #4.0 | mul r4, r5, #4.0
ph_wrap r1, r3, r0
ph_wrap r5, r3, r4 | st r1, [01aa]
addxy r0, r1, #1.0 | addxy r4, r5, #1.0 | st r5, [01be]
st r1, [01b0] | st r5, [01c4]
ph_wrap r0, r3, r0
ph_wrap r4, r3, r4
mul r1, r3, #4.0 | mul r5, r7, #4.0
min r2, r0, r1
min r6, r4, r5
mul r3, r3, #-4.0 | mul r7, r7, #-4.0
max r2, r2, r3 | st r3, [01b4] | ld r3, [01b2]
max r6, r6, r7 | st r7, [01c8] | ld r7, [01c6]
mul r2, r3, r2 | mul r6, r7, r6
shl r2, r2, 15
shl r6, r6, 15
absadd r3, r2, #-1.0
absadd r7, r6, #-1.0
mac_p1wc3 r2, r2, r3, #0.25, r0 | ld r3, [01ba]
mac_p1wc3 r6, r6, r7, #0.25, r4 | ld r7, [01ce]
mov r0, r3 | mov r4, r7 | ld r3, [01b0] | ld r7, [01c4]
mad r0, r3, #-0.68493148895338163 | mad r4, r7, #-0.68493148895338163
addxy r3, r0, #-0.65753423000762279 | addxy r7, r4, #-0.65753423000762279
mul r0, r3, #-1.4600000381469727 | mul r4, r7, #-1.4600000381469727
ph_wrap r3, r3, r0
ph_wrap r7, r3, r4 | min r0, r3, r1 | ld r1, [01b4]
min r4, r7, r5 | ld r5, [01c8]
max r0, r0, r1 | ld r1, [01b2]
max r4, r4, r5 | ld r5, [01c6]
mul r1, r1, r0 | mul r5, r5, r4
shl r0, r1, 15
shl r4, r5, 15
absadd r1, r0, #-1.0
absadd r5, r4, #-1.0
mac_p1wc3 r0, r0, r1, #0.25, r3 | ld r3, [01ba]
mac_p1wc3 r4, r4, r5, #0.25, r7 | ld r7, [01ce]
subxy r1, r0, r2 | subxy r5, r4, r6
mad r1, r3, #-0.36500000953674316 | mad r5, r7, #-0.36500000953674316
addxy r0, r2, #0.25 | addxy r3, r1, #-0.0099999997764825821
addxy r4, r6, #0.25 | addxy r7, r5, #-0.0099999997764825821
mac_p1wc3 r1, r0, r3, #0.5, r2 | ld r0, [c066] | ld r3, [01b6]
mac_p1wc3 r5, r4, r7, #0.5, r6 | ld r4, [c066] | ld r7, [01ca]
mac_p1wc3 r2, r1, r0, #0.125, r3 | ld r1, [a1ce] | ld r0, [054c]
mac_p1wc3 r6, r5, r4, #0.125, r7 | ld r5, [a1cf] | ld r3, [06d6]
mad r2, r1, #0.25 | mad r6, r5, #0.25 | ld r4, [054e] | ld r7, [06f6]
mul r1, r0, r3 | mul r5, r4, r7 | st r2, [01ba] | ld r2, [a1e6]
st r1, [01b6] | st r6, [01ce] | ld r0, [053c]
addxy r2, r2, #-4.75 | ld r1, [c064] | ld r6, [a1e7] | st r5, [01ca]
addxy r6, r6, #-4.75 | mul r1, r1, r2 | ld r3, [c061] | ld r4, [053e]
mac_p1wc3 r2, r0, r3, #0.125, r1 | ld r5, [c064] | ld r0, [06d8]
mul r5, r5, r6 | mad r2, r0, #0.125 | ld r7, [c061]
mac_p1wc3 r6, r4, r7, #0.125, r5 | ld r4, [06f8] | ld r3, [c060]
mad r6, r4, #0.125 | mul r1, r2, #0.17328679513998632 | ld r7, [c060]
mov r0, #0.041666666666666664 | mul r5, r6, #0.17328679513998632
mov r4, #0.041666666666666664 | ymacd r0, r1, #0.16666666666666666
ymacd r4, r5, #0.16666666666666666 | ymacd r0, r1, #0.5
ymacd r4, r5, #0.5 | ymacd r0, r1, #1.0
ymacd r4, r5, #1.0 | ymacd r0, r1, #1.0
ymacd r4, r5, #1.0 | mul r2, r0, r0
mul r6, r4, r4 | mul r1, r2, r2
mul r5, r6, r6 | mul r0, r1, r1
mul r4, r5, r5 | mul r2, r0, r0
mul r6, r4, r4 | mul r1, r2, r2
mul r5, r6, r6 | mul r0, r1, #6.103515625e-05 | ld r1, [c06b]
mul r4, r5, #6.103515625e-05 | mul r2, r3, r0 | ld r5, [c06b] | st r1, [01b2]
mul r6, r7, r4 | st r5, [01c6]
min r3, r2, #3.3569336665095761e-05 | ld r2, [01ad]
min r7, r6, #3.3569336665095761e-05 | shl r0, r3, 14 | ld r3, [01af] | ld r6, [01c1]
shl r4, r7, 14 | ld r7, [01c3]
mul r1, r0, r3 | mul r5, r4, r7
addxy r1, r1, r2 | addxy r5, r5, r6
mov r2, r1 | mov r6, r5 | st r1, [01b4] | ld r1, [01ba]
mad r2, r1, #-0.25 | st r5, [01c8] | ld r5, [01ce]
mad r6, r5, #-0.25 | ld r1, [01b2] | ld r5, [01c6]
mac_p1wc3 r2, r1, r3, #0.5, r2
mac_p1wc3 r6, r5, r7, #0.5, r6
mac_p1wc3 r2, r0, r2, #-0.5, r3 | ld r3, [01b4]
mac_p1wc3 r6, r4, r6, #-0.5, r7 | ld r7, [01c8] | st r0, [01b4]
mac_p1wc3 r3, r0, r2, #-0.5, r3 | st r4, [01c8]
mac_p1wc3 r7, r4, r6, #-0.5, r7 | st r3, [01b0]
mul r0, r3, #-2.0 | mul r4, r7, #-2.0 | st r7, [01c4]
mov r0, r3 | st r0, [01ac] | ld r3, [01ba]
mov r4, r7 | st r4, [01c0] | ld r7, [01ce]
mad r0, r3, #0.125 | mad r4, r7, #0.125
mac_p1wc3 r3, r1, r2, #0.5, r0 | ld r1, [01b4]
mac_p1wc3 r7, r5, r6, #0.5, r4 | ld r5, [01c8]
mac_p1wc3 r0, r1, r3, #-0.5, r2 | ld r3, [01b6] | ld r2, [01b0]
mac_p1wc3 r4, r5, r7, #-0.5, r6 | ld r7, [01ca] | ld r6, [01c4]
mul r1, r0, #4.0 | mul r0, r3, r2 | ld r2, [c055]
mul r5, r4, #4.0 | mul r4, r7, r6 | st r1, [01ae]
mul r1, r0, #-32.0 | mul r5, r4, #-32.0 | st r5, [01c2] | ld r0, [c056]
hset h6, r0
st r5, [040e] | st r1, [040c]
ld r1, [c054] | ldidx r3, 0 | ldidx r4, 2
ymacd r3, r2, r1 | ldidx r5, 4 | ldidx r6, 6 | ld r0, [c05c]
ymacd r4, r2, r1 | st r3, [a138] | ldidx r3, 8
ymacd r5, r2, r1 | st r4, [a139] | ldidx r4, 10
ymacd r6, r2, r1 | st r5, [a13a] | ldidx r5, 12
ymacd r3, r2, r1 | st r6, [a13b] | ldidx r6, 14
ymacd r4, r2, r1 | st r3, [a13c]
ymacd r5, r2, r1 | st r4, [a13d]
ymacd r6, r2, r1 | st r5, [a13e] | ld r1, [c05a]
hset h6, r0
st r6, [a13f] | ld r2, [c05b]
ldidx r3, 0 | ldidx r4, 2
ymacd r3, r2, r1 | ldidx r5, 4 | ldidx r6, 6
ymacd r4, r2, r1 | st r3, [a140] | ldidx r3, 8
ymacd r5, r2, r1 | st r4, [a141] | ldidx r4, 10
ymacd r6, r2, r1 | st r5, [a142] | ldidx r5, 12
ymacd r3, r2, r1 | st r6, [a143] | ldidx r6, 14
ymacd r4, r2, r1 | st r3, [a144]
ymacd r5, r2, r1 | st r4, [a145]
ymacd r6, r2, r1 | st r5, [a146]
st r6, [a147]
