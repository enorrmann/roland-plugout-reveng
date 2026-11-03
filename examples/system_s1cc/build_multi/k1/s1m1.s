mov r7, #0.0 | ld r0, [a151] | ld r1, [0403] | ld r2, [06d1]
addxy r7, r7, r0 | mul r2, r2, #7.31275 | ld r0, [0405] | ld r3, [c090]
addxy r7, r7, r1 | ld r1, [0407] | st r3, [a120] | ld r6, [c000]
addxy r7, r7, r0 | ld r0, [0409]
addxy r7, r7, r1 | ld r1, [040b]
addxy r7, r7, r0 | ld r0, [040d]
addxy r7, r7, r1 | ld r1, [040f]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | st r0, [6008] | ld r1, [060d]
mul r7, r7, #0.16666666666666666 | st r0, [6009]
mul r7, r7, r6 | st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [06d3]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [06d2] | ld r0, [a168]
subxy r1, r1, r0 | st r0, [060c] | ld r0, [c098]
mul r1, r1, r1
ph_wrap r2, r3, r2 | ld r1, [c050]
mul r1, r3, r1 | st r2, [06d0] | ld r2, [c051]
mad r1, r0, -r2 | ld r0, [c080]
add r1, #1.0 | st r0, [0600] | st r0, [a170]
st r1, [0616] | ld r1, [c04f]
mul r1, r3, r1 | ld r0, [c088] | ld r2, [0616]
st r0, [0606] | st r1, [0618]
st r0, [a178] | ld r0, [0600]
ld r1, [0606] | st r0, [a1b0]
st r1, [a1b8] | ld r1, [0627]
ld r0, [a169] | ld r3, [0618]
subxy r1, r1, r0 | st r3, [a1d0] | ld r3, [c091]
mul r1, r1, r1 | st r2, [a1c8] | st r0, [0626]
ld r1, [c050] | st r3, [a121]
mul r1, r3, r1 | ld r0, [c099]
ld r2, [c051]
mad r1, r0, -r2 | ld r0, [c081] | ld r2, [a1a1]
add r1, #1.0 | st r0, [061a] | st r0, [a171]
ld r0, [c089] | st r1, [0630]
ld r1, [c04f] | st r0, [0620]
mul r1, r3, r1 | st r0, [a179] | ld r3, [0181] | st r2, [0186]
ld r0, [a1a9] | st r1, [0632]
mov r4, r0 | ld r1, [a1e1]
mad r4, r3, #0.5 | st r1, [0188]
mul r7, r4, #2.0
ph_wrap r3, r3, r7
mov r4, r3 | st r3, [0180]
mad r4, r2, #-0.89999997615814209
ph_wrap r7, r3, r4
mul r4, r0, #2.0 | mul r0, r0, #-2.0
min r2, r7, r4
max r2, r2, r0
mul r2, r1, r2
shl r2, r2, 13
absadd r1, r2, #-2.0
mul r2, r2, r1
addxy r1, r2, r7
addxy r2, r3, #1.0
ph_wrap r7, r3, r2
min r3, r7, r4 | ld r4, [0188]
max r2, r3, r0
mul r3, r4, r2
shl r0, r3, 13
absadd r4, r0, #-2.0
mul r2, r0, r4 | ld r4, [0186]
addxy r3, r2, r7 | ld r2, [c063]
subxy r0, r3, r1 | ld r3, [a1c1]
mad r0, r4, #0.44999998807907104
mad r0, r4, #0.44999998807907104 | ld r4, [061a]
mul r7, r2, r0 | ld r2, [0630]
mul r0, r4, r2 | ld r2, [a1d9] | ld r4, [c062]
addxy r2, r2, #-4.75 | st r0, [0186]
addxy r1, r3, r7 | mul r4, r4, r2 | ld r3, [0620] | ld r7, [c061]
mac_p1wc3 r2, r3, r7, #0.125, r4 | ld r3, [0632] | ld r7, [c060]
mad r2, r3, #0.125 | mov r3, #0.041666666666666664
mul r4, r2, #0.17328679513998632
ymacd r3, r4, #0.16666666666666666
ymacd r3, r4, #0.5
ymacd r3, r4, #1.0
ymacd r3, r4, #1.0
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, r4
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, #6.103515625e-05 | ld r4, [c064]
mul r2, r7, r3
min r7, r2, #3.3569336665095761e-05 | ld r2, [0183]
shl r3, r7, 14 | ld r7, [0185]
mul r0, r3, r7
addxy r0, r0, r2
mov r2, r0
mad r2, r1, #-0.0625
mac_p1wc3 r2, r4, r7, #0.5, r2
mac_p1wc3 r2, r3, r2, #-0.5, r7
mac_p1wc3 r7, r3, r2, #-0.5, r0
mul r0, r7, #-2.0
mov r0, r7 | st r0, [0182]
mad r0, r1, #0.03125
mac_p1wc3 r1, r4, r2, #0.5, r0
mac_p1wc3 r4, r3, r1, #-0.5, r2 | ld r3, [0186]
mul r0, r4, #4.0 | mul r1, r3, r7 | ld r3, [c092]
mul r2, r1, #-32.0 | st r0, [0184] | ld r0, [a16a]
ld r1, [0641] | st r2, [0402] | st r0, [0640]
subxy r1, r1, r0 | ld r0, [c09a] | st r3, [a122]
mul r1, r1, r1 | ld r2, [c051]
ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c082] | ld r2, [a1a2]
add r1, #1.0 | st r0, [0634] | st r0, [a172]
ld r0, [c08a] | st r1, [064a]
ld r1, [c04f] | st r0, [063a]
mul r1, r3, r1 | st r0, [a17a] | ld r3, [018b] | st r2, [0190]
ld r0, [a1aa] | st r1, [064c]
mov r4, r0 | ld r1, [a1e2]
mad r4, r3, #0.5 | st r1, [0192]
mul r7, r4, #2.0
ph_wrap r3, r3, r7
mov r4, r3 | st r3, [018a]
mad r4, r2, #-0.89999997615814209
ph_wrap r7, r3, r4
mul r4, r0, #2.0 | mul r0, r0, #-2.0
min r2, r7, r4
max r2, r2, r0
mul r2, r1, r2
shl r2, r2, 13
absadd r1, r2, #-2.0
mul r2, r2, r1
addxy r1, r2, r7
addxy r2, r3, #1.0
ph_wrap r7, r3, r2
min r3, r7, r4 | ld r4, [0192]
max r2, r3, r0
mul r3, r4, r2
shl r0, r3, 13
absadd r4, r0, #-2.0
mul r2, r0, r4 | ld r4, [0190]
addxy r3, r2, r7 | ld r2, [c063]
subxy r0, r3, r1 | ld r3, [a1c2]
mad r0, r4, #0.44999998807907104
mad r0, r4, #0.44999998807907104 | ld r4, [0634]
mul r7, r2, r0 | ld r2, [064a]
mul r0, r4, r2 | ld r2, [a1da] | ld r4, [c062]
addxy r2, r2, #-4.75 | st r0, [0190]
addxy r1, r3, r7 | mul r4, r4, r2 | ld r3, [063a] | ld r7, [c061]
mac_p1wc3 r2, r3, r7, #0.125, r4 | ld r3, [064c] | ld r7, [c060]
mad r2, r3, #0.125 | mov r3, #0.041666666666666664
mul r4, r2, #0.17328679513998632
ymacd r3, r4, #0.16666666666666666
ymacd r3, r4, #0.5
ymacd r3, r4, #1.0
ymacd r3, r4, #1.0
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, r4
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, #6.103515625e-05 | ld r4, [c064]
mul r2, r7, r3
min r7, r2, #3.3569336665095761e-05 | ld r2, [018d]
shl r3, r7, 14 | ld r7, [018f]
mul r0, r3, r7
addxy r0, r0, r2
mov r2, r0
mad r2, r1, #-0.0625
mac_p1wc3 r2, r4, r7, #0.5, r2
mac_p1wc3 r2, r3, r2, #-0.5, r7
mac_p1wc3 r7, r3, r2, #-0.5, r0
mul r0, r7, #-2.0
mov r0, r7 | st r0, [018c]
mad r0, r1, #0.03125
mac_p1wc3 r1, r4, r2, #0.5, r0
mac_p1wc3 r4, r3, r1, #-0.5, r2 | ld r3, [0190]
mul r0, r4, #4.0 | mul r1, r3, r7 | ld r3, [c093]
mul r2, r1, #-32.0 | st r0, [018e] | ld r0, [a16b]
ld r1, [065b] | st r2, [0404] | st r0, [065a]
subxy r1, r1, r0 | ld r0, [c09b] | st r3, [a123]
mul r1, r1, r1 | ld r2, [c051]
ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c083] | ld r2, [a1a3]
add r1, #1.0 | st r0, [064e] | st r0, [a173]
ld r0, [c08b] | st r1, [0664]
ld r1, [c04f] | st r0, [0654]
mul r1, r3, r1 | st r0, [a17b] | ld r3, [0195] | st r2, [019a]
ld r0, [a1ab] | st r1, [0666]
mov r4, r0 | ld r1, [a1e3]
mad r4, r3, #0.5 | st r1, [019c]
mul r7, r4, #2.0
ph_wrap r3, r3, r7
mov r4, r3 | st r3, [0194]
mad r4, r2, #-0.89999997615814209
ph_wrap r7, r3, r4
mul r4, r0, #2.0 | mul r0, r0, #-2.0
min r2, r7, r4
max r2, r2, r0
mul r2, r1, r2
shl r2, r2, 13
absadd r1, r2, #-2.0
mul r2, r2, r1
addxy r1, r2, r7
addxy r2, r3, #1.0
ph_wrap r7, r3, r2
min r3, r7, r4 | ld r4, [019c]
max r2, r3, r0
mul r3, r4, r2
shl r0, r3, 13
absadd r4, r0, #-2.0
mul r2, r0, r4 | ld r4, [019a]
addxy r3, r2, r7 | ld r2, [c063]
subxy r0, r3, r1 | ld r3, [a1c3]
mad r0, r4, #0.44999998807907104
mad r0, r4, #0.44999998807907104 | ld r4, [064e]
mul r7, r2, r0 | ld r2, [0664]
mul r0, r4, r2 | ld r2, [a1db] | ld r4, [c062]
addxy r2, r2, #-4.75 | st r0, [019a]
addxy r1, r3, r7 | mul r4, r4, r2 | ld r3, [0654] | ld r7, [c061]
mac_p1wc3 r2, r3, r7, #0.125, r4 | ld r3, [0666] | ld r7, [c060]
mad r2, r3, #0.125 | mov r3, #0.041666666666666664
mul r4, r2, #0.17328679513998632
ymacd r3, r4, #0.16666666666666666
ymacd r3, r4, #0.5
ymacd r3, r4, #1.0
ymacd r3, r4, #1.0
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, r4
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, #6.103515625e-05 | ld r4, [c064]
mul r2, r7, r3
min r7, r2, #3.3569336665095761e-05 | ld r2, [0197]
shl r3, r7, 14 | ld r7, [0199]
mul r0, r3, r7
addxy r0, r0, r2
mov r2, r0
mad r2, r1, #-0.0625
mac_p1wc3 r2, r4, r7, #0.5, r2
mac_p1wc3 r2, r3, r2, #-0.5, r7
mac_p1wc3 r7, r3, r2, #-0.5, r0
mul r0, r7, #-2.0
mov r0, r7 | st r0, [0196]
mad r0, r1, #0.03125
mac_p1wc3 r1, r4, r2, #0.5, r0
mac_p1wc3 r4, r3, r1, #-0.5, r2 | ld r3, [019a]
mul r0, r4, #4.0 | mul r1, r3, r7 | ld r3, [c094]
mul r2, r1, #-32.0 | st r0, [0198] | ld r0, [a16c]
ld r1, [0675] | st r2, [0406] | st r0, [0674]
subxy r1, r1, r0 | ld r0, [c09c] | st r3, [a124]
mul r1, r1, r1 | ld r2, [c051]
ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c084] | ld r2, [a1a4]
add r1, #1.0 | st r0, [0668] | st r0, [a174]
ld r0, [c08c] | st r1, [067e]
ld r1, [c04f] | st r0, [066e]
mul r1, r3, r1 | st r0, [a17c] | ld r3, [019f] | st r2, [01a4]
ld r0, [a1ac] | st r1, [0680]
mov r4, r0 | ld r1, [a1e4]
mad r4, r3, #0.5 | st r1, [01a6]
mul r7, r4, #2.0
ph_wrap r3, r3, r7
mov r4, r3 | st r3, [019e]
mad r4, r2, #-0.89999997615814209
ph_wrap r7, r3, r4
mul r4, r0, #2.0 | mul r0, r0, #-2.0
min r2, r7, r4
max r2, r2, r0
mul r2, r1, r2
shl r2, r2, 13
absadd r1, r2, #-2.0
mul r2, r2, r1
addxy r1, r2, r7
addxy r2, r3, #1.0
ph_wrap r7, r3, r2
min r3, r7, r4 | ld r4, [01a6]
max r2, r3, r0
mul r3, r4, r2
shl r0, r3, 13
absadd r4, r0, #-2.0
mul r2, r0, r4 | ld r4, [01a4]
addxy r3, r2, r7 | ld r2, [c063]
subxy r0, r3, r1 | ld r3, [a1c4]
mad r0, r4, #0.44999998807907104
mad r0, r4, #0.44999998807907104 | ld r4, [0668]
mul r7, r2, r0 | ld r2, [067e]
mul r0, r4, r2 | ld r2, [a1dc] | ld r4, [c062]
addxy r2, r2, #-4.75 | st r0, [01a4]
addxy r1, r3, r7 | mul r4, r4, r2 | ld r3, [066e] | ld r7, [c061]
mac_p1wc3 r2, r3, r7, #0.125, r4 | ld r3, [0680] | ld r7, [c060]
mad r2, r3, #0.125 | mov r3, #0.041666666666666664
mul r4, r2, #0.17328679513998632
ymacd r3, r4, #0.16666666666666666
ymacd r3, r4, #0.5
ymacd r3, r4, #1.0
ymacd r3, r4, #1.0
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, r4
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, #6.103515625e-05 | ld r4, [c064]
mul r2, r7, r3
min r7, r2, #3.3569336665095761e-05 | ld r2, [01a1]
shl r3, r7, 14 | ld r7, [01a3]
mul r0, r3, r7
addxy r0, r0, r2
mov r2, r0
mad r2, r1, #-0.0625
mac_p1wc3 r2, r4, r7, #0.5, r2
mac_p1wc3 r2, r3, r2, #-0.5, r7
mac_p1wc3 r7, r3, r2, #-0.5, r0
mul r0, r7, #-2.0
mov r0, r7 | st r0, [01a0]
mad r0, r1, #0.03125
mac_p1wc3 r1, r4, r2, #0.5, r0
mac_p1wc3 r4, r3, r1, #-0.5, r2 | ld r3, [01a4]
mul r0, r4, #4.0 | mul r1, r3, r7 | ld r3, [c095]
mul r2, r1, #-32.0 | st r0, [01a2] | ld r0, [a16d]
ld r1, [068f] | st r2, [0408] | st r0, [068e]
subxy r1, r1, r0 | ld r0, [c09d] | st r3, [a125]
mul r1, r1, r1 | ld r2, [c051]
ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c085] | ld r2, [a1a5]
add r1, #1.0 | st r0, [0682] | st r0, [a175]
ld r0, [c08d] | st r1, [0698]
ld r1, [c04f] | st r0, [0688]
mul r1, r3, r1 | st r0, [a17d] | ld r3, [01a9] | st r2, [01ae]
ld r0, [a1ad] | st r1, [069a]
mov r4, r0 | ld r1, [a1e5]
mad r4, r3, #0.5 | st r1, [01b0]
mul r7, r4, #2.0
ph_wrap r3, r3, r7
mov r4, r3 | st r3, [01a8]
mad r4, r2, #-0.89999997615814209
ph_wrap r7, r3, r4
mul r4, r0, #2.0 | mul r0, r0, #-2.0
min r2, r7, r4
max r2, r2, r0
mul r2, r1, r2
shl r2, r2, 13
absadd r1, r2, #-2.0
mul r2, r2, r1
addxy r1, r2, r7
addxy r2, r3, #1.0
ph_wrap r7, r3, r2
min r3, r7, r4 | ld r4, [01b0]
max r2, r3, r0
mul r3, r4, r2
shl r0, r3, 13
absadd r4, r0, #-2.0
mul r2, r0, r4 | ld r4, [01ae]
addxy r3, r2, r7 | ld r2, [c063]
subxy r0, r3, r1 | ld r3, [a1c5]
mad r0, r4, #0.44999998807907104
mad r0, r4, #0.44999998807907104 | ld r4, [0682]
mul r7, r2, r0 | ld r2, [0698]
mul r0, r4, r2 | ld r2, [a1dd] | ld r4, [c062]
addxy r2, r2, #-4.75 | st r0, [01ae]
addxy r1, r3, r7 | mul r4, r4, r2 | ld r3, [0688] | ld r7, [c061]
mac_p1wc3 r2, r3, r7, #0.125, r4 | ld r3, [069a] | ld r7, [c060]
mad r2, r3, #0.125 | mov r3, #0.041666666666666664
mul r4, r2, #0.17328679513998632
ymacd r3, r4, #0.16666666666666666
ymacd r3, r4, #0.5
ymacd r3, r4, #1.0
ymacd r3, r4, #1.0
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, r4
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, #6.103515625e-05 | ld r4, [c064]
mul r2, r7, r3
min r7, r2, #3.3569336665095761e-05 | ld r2, [01ab]
shl r3, r7, 14 | ld r7, [01ad]
mul r0, r3, r7
addxy r0, r0, r2
mov r2, r0
mad r2, r1, #-0.0625
mac_p1wc3 r2, r4, r7, #0.5, r2
mac_p1wc3 r2, r3, r2, #-0.5, r7
mac_p1wc3 r7, r3, r2, #-0.5, r0
mul r0, r7, #-2.0
mov r0, r7 | st r0, [01aa]
mad r0, r1, #0.03125
mac_p1wc3 r1, r4, r2, #0.5, r0
mac_p1wc3 r4, r3, r1, #-0.5, r2 | ld r3, [01ae]
mul r0, r4, #4.0 | mul r1, r3, r7 | ld r3, [c096]
mul r2, r1, #-32.0 | st r0, [01ac] | ld r0, [a16e]
ld r1, [06a9] | st r2, [040a] | st r0, [06a8]
subxy r1, r1, r0 | ld r0, [c09e] | st r3, [a126]
mul r1, r1, r1 | ld r2, [c051]
ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c086] | ld r2, [a1a6]
add r1, #1.0 | st r0, [069c] | st r0, [a176]
ld r0, [c08e] | st r1, [06b2]
ld r1, [c04f] | st r0, [06a2]
mul r1, r3, r1 | st r0, [a17e] | ld r3, [01b3] | st r2, [01b8]
ld r0, [a1ae] | st r1, [06b4]
mov r4, r0 | ld r1, [a1e6]
mad r4, r3, #0.5 | st r1, [01ba]
mul r7, r4, #2.0
ph_wrap r3, r3, r7
mov r4, r3 | st r3, [01b2]
mad r4, r2, #-0.89999997615814209
ph_wrap r7, r3, r4
mul r4, r0, #2.0 | mul r0, r0, #-2.0
min r2, r7, r4
max r2, r2, r0
mul r2, r1, r2
shl r2, r2, 13
absadd r1, r2, #-2.0
mul r2, r2, r1
addxy r1, r2, r7
addxy r2, r3, #1.0
ph_wrap r7, r3, r2
min r3, r7, r4 | ld r4, [01ba]
max r2, r3, r0
mul r3, r4, r2
shl r0, r3, 13
absadd r4, r0, #-2.0
mul r2, r0, r4 | ld r4, [01b8]
addxy r3, r2, r7 | ld r2, [c063]
subxy r0, r3, r1 | ld r3, [a1c6]
mad r0, r4, #0.44999998807907104
mad r0, r4, #0.44999998807907104 | ld r4, [069c]
mul r7, r2, r0 | ld r2, [06b2]
mul r0, r4, r2 | ld r2, [a1de] | ld r4, [c062]
addxy r2, r2, #-4.75 | st r0, [01b8]
addxy r1, r3, r7 | mul r4, r4, r2 | ld r3, [06a2] | ld r7, [c061]
mac_p1wc3 r2, r3, r7, #0.125, r4 | ld r3, [06b4] | ld r7, [c060]
mad r2, r3, #0.125 | mov r3, #0.041666666666666664
mul r4, r2, #0.17328679513998632
ymacd r3, r4, #0.16666666666666666
ymacd r3, r4, #0.5
ymacd r3, r4, #1.0
ymacd r3, r4, #1.0
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, r4
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, #6.103515625e-05 | ld r4, [c064]
mul r2, r7, r3
min r7, r2, #3.3569336665095761e-05 | ld r2, [01b5]
shl r3, r7, 14 | ld r7, [01b7]
mul r0, r3, r7
addxy r0, r0, r2
mov r2, r0
mad r2, r1, #-0.0625
mac_p1wc3 r2, r4, r7, #0.5, r2
mac_p1wc3 r2, r3, r2, #-0.5, r7
mac_p1wc3 r7, r3, r2, #-0.5, r0
mul r0, r7, #-2.0
mov r0, r7 | st r0, [01b4]
mad r0, r1, #0.03125
mac_p1wc3 r1, r4, r2, #0.5, r0
mac_p1wc3 r4, r3, r1, #-0.5, r2 | ld r3, [01b8]
mul r0, r4, #4.0 | mul r1, r3, r7 | ld r3, [c097]
mul r2, r1, #-32.0 | st r0, [01b6] | ld r0, [a16f]
ld r1, [06c3] | st r2, [040c] | st r0, [06c2]
subxy r1, r1, r0 | ld r0, [c09f] | st r3, [a127]
mul r1, r1, r1 | ld r2, [c051]
ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c087] | ld r2, [a1a7]
add r1, #1.0 | st r0, [06b6] | st r0, [a177]
ld r0, [c08f] | st r1, [06cc]
ld r1, [c04f] | st r0, [06bc]
mul r1, r3, r1 | st r0, [a17f] | ld r3, [01bd] | st r2, [01c2]
ld r0, [a1af] | st r1, [06ce]
mov r4, r0 | ld r1, [a1e7]
mad r4, r3, #0.5 | st r1, [01c4]
mul r7, r4, #2.0
ph_wrap r3, r3, r7
mov r4, r3 | st r3, [01bc]
mad r4, r2, #-0.89999997615814209
ph_wrap r7, r3, r4
mul r4, r0, #2.0 | mul r0, r0, #-2.0
min r2, r7, r4
max r2, r2, r0
mul r2, r1, r2
shl r2, r2, 13
absadd r1, r2, #-2.0
mul r2, r2, r1
addxy r1, r2, r7
addxy r2, r3, #1.0
ph_wrap r7, r3, r2
min r3, r7, r4 | ld r4, [01c4]
max r2, r3, r0
mul r3, r4, r2
shl r0, r3, 13
absadd r4, r0, #-2.0
mul r2, r0, r4 | ld r4, [01c2]
addxy r3, r2, r7 | ld r2, [c063]
subxy r0, r3, r1 | ld r3, [a1c7]
mad r0, r4, #0.44999998807907104
mad r0, r4, #0.44999998807907104 | ld r4, [06b6]
mul r7, r2, r0 | ld r2, [06cc]
mul r0, r4, r2 | ld r2, [a1df] | ld r4, [c062]
addxy r2, r2, #-4.75 | st r0, [01c2]
addxy r1, r3, r7 | mul r4, r4, r2 | ld r3, [06bc] | ld r7, [c061]
mac_p1wc3 r2, r3, r7, #0.125, r4 | ld r3, [06ce] | ld r7, [c060]
mad r2, r3, #0.125 | mov r3, #0.041666666666666664
mul r4, r2, #0.17328679513998632
ymacd r3, r4, #0.16666666666666666
ymacd r3, r4, #0.5
ymacd r3, r4, #1.0
ymacd r3, r4, #1.0
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, r4
mul r2, r3, r3
mul r4, r2, r2
mul r3, r4, #6.103515625e-05 | ld r4, [c064]
mul r2, r7, r3
min r7, r2, #3.3569336665095761e-05 | ld r2, [01bf]
shl r3, r7, 14 | ld r7, [01c1]
mul r0, r3, r7
addxy r0, r0, r2
mov r2, r0
mad r2, r1, #-0.0625
mac_p1wc3 r2, r4, r7, #0.5, r2
mac_p1wc3 r2, r3, r2, #-0.5, r7
mac_p1wc3 r7, r3, r2, #-0.5, r0
mul r0, r7, #-2.0
mov r0, r7 | st r0, [01be]
mad r0, r1, #0.03125
mac_p1wc3 r1, r4, r2, #0.5, r0
mac_p1wc3 r4, r3, r1, #-0.5, r2 | ld r3, [01c2]
mul r1, r3, r7 | mul r0, r4, #4.0
mul r2, r1, #-32.0 | st r0, [01c0]
st r2, [040e]
