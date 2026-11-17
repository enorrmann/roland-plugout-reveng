mov r7, #0.0 | ld r0, [a151] | ld r2, [06e1]
addxy r7, r7, r0 | mul r2, r2, #7.31275 | ld r1, [a152]
addxy r7, r7, r1 | ld r0, [a153] | ld r3, [c090]
addxy r7, r7, r0 | ld r1, [a154] | ld r0, [0409]
addxy r7, r7, r1 | ld r1, [040b] | st r3, [a120] | ld r6, [c000]
addxy r7, r7, r0 | ld r0, [040d] | ld r5, [a1cc]
addxy r7, r7, r1 | ld r1, [040f] | ld r4, [a1d4]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | st r0, [6008] | ld r1, [060f]
mul r7, r7, #0.16666666666666666 | st r0, [6009]
mul r7, r7, r6 | st r0, [600a]
min r6, r5, #0.25 | st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [06e3]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [06e2] | ld r0, [a168]
subxy r1, r1, r0 | st r0, [060e] | ld r0, [c098]
mul r1, r1, r1
ph_wrap r2, r3, r2 | ld r1, [c050]
mul r1, r3, r1 | st r2, [06e0] | ld r2, [c051]
mad r1, r0, -r2 | ld r0, [c080]
add r1, #1.0 | st r0, [0600] | ld r0, [c088]
st r1, [0618] | ld r1, [c04f]
mul r1, r3, r1 | st r0, [0606] | ld r0, [c0a0]
st r0, [060c] | st r1, [061a] | ld r1, [0600]
ld r0, [060c] | st r1, [a1b0] | ld r3, [c091]
st r0, [a1a0] | ld r1, [062b]
ld r0, [a169] | ld r2, [0606]
subxy r1, r1, r0 | st r2, [a1b8] | st r0, [062a]
mul r1, r1, r1 | ld r0, [c099] | st r3, [a121]
ld r1, [c050]
mul r1, r3, r1 | ld r2, [c051]
mad r1, r0, -r2 | ld r0, [c081]
add r1, #1.0 | st r0, [061c] | ld r0, [c089]
st r1, [0634] | ld r1, [c04f]
mul r1, r3, r1 | st r0, [0622] | ld r0, [c0a1]
st r0, [0628] | st r1, [0636] | ld r1, [061c]
ld r0, [0628] | st r1, [a1b1] | ld r3, [c092]
st r0, [a1a1] | ld r1, [0647]
ld r0, [a16a] | ld r2, [0622]
subxy r1, r1, r0 | st r2, [a1b9] | st r0, [0646]
mul r1, r1, r1 | ld r0, [c09a] | st r3, [a122]
ld r1, [c050]
mul r1, r3, r1 | ld r2, [c051]
mad r1, r0, -r2 | ld r0, [c082]
add r1, #1.0 | st r0, [0638] | ld r0, [c08a]
st r1, [0650] | ld r1, [c04f]
mul r1, r3, r1 | st r0, [063e] | ld r0, [c0a2]
st r0, [0644] | st r1, [0652] | ld r1, [0638]
ld r0, [0644] | st r1, [a1b2] | ld r3, [c093]
st r0, [a1a2] | ld r1, [0663]
ld r0, [a16b] | ld r2, [063e]
subxy r1, r1, r0 | st r2, [a1ba] | st r0, [0662]
mul r1, r1, r1 | ld r0, [c09b] | st r3, [a123]
ld r1, [c050]
mul r1, r3, r1 | ld r2, [c051]
mad r1, r0, -r2 | ld r0, [c083]
add r1, #1.0 | st r0, [0654] | ld r0, [c08b]
st r1, [066c] | ld r1, [c04f]
mul r1, r3, r1 | st r0, [065a] | ld r0, [c0a3]
st r0, [0660] | st r1, [066e] | ld r1, [0654]
ld r0, [0660] | st r1, [a1b3] | ld r3, [c094]
st r0, [a1a3] | ld r1, [067f]
ld r0, [a16c] | ld r2, [065a]
subxy r1, r1, r0 | st r2, [a1bb] | st r0, [067e]
mul r1, r1, r1 | ld r0, [c09c] | st r3, [a124]
ld r1, [c050]
mul r1, r3, r1 | ld r2, [c051]
mad r1, r0, -r2 | ld r0, [c084]
add r1, #1.0 | st r0, [0670] | ld r0, [c08c]
st r1, [0688] | ld r1, [c04f]
mul r1, r3, r1 | st r0, [0676] | ld r0, [c0a4] | ld r3, [a1ac]
st r1, [068a] | st r0, [067c] | ld r0, [0676]
ld r1, [0181] | ld r7, [067c]
subxy r2, r1, r0 | mul r0, r4, r7
mad r1, r2, #0.0065235896036028862 | ld r2, [0183]
addxy r5, r3, r2 | st r1, [0180]
addxy r7, r5, #-1.0
sign r3, r7
min r2, r3, #0.0
subxy r7, r5, r2
mul r3, r7, #-1.0 | addxy r2, r7, #1.0
addxy r5, r3, #-1.0 | mul r3, r4, r2
max r5, r3, #-6.103515625e-05 | st r5, [0182]
addxy r3, r7, #2.0
shl r2, r5, 14
mul r5, r4, r3
min r4, r5, #6.103515625e-05
shl r3, r4, 14
addxy r5, r2, #1.0 | addxy r2, r3, #-1.0
mul r4, r5, r5 | mul r5, r2, r2
subxy r3, r4, r5 | ld r5, [0187]
mad r7, r3, #0.5 | ld r3, [0185]
addxy r2, r7, #1.5 | mad r7, r5, #-0.96461350614310715 | ld r5, [018b]
mad r7, r3, #0.33408930870924336 | mul r4, r2, #-2.0
mad r5, r6, #0.000244140625 | mul r2, r7, #-0.88454544544219971 | st r4, [0184]
addxy r3, r5, #-0.000244140625 | addxy r4, r2, #-1.3268181681632996
shl r2, r3, 12 | st r4, [0186]
sign r4, r2
min r6, r4, #0.0
mad r5, r6, #-0.000244140625
addxy r3, r5, #-0.000244140625 | addxy r4, r5, #-0.00048828125
mul r2, r0, r3 | mul r6, r0, r4 | st r3, [018a]
mul r3, r2, r2
shl r0, r3, 15
shl r4, r0, 15
shl r2, r4, 15
mul r4, r6, r6
shl r3, r2, 15
shl r2, r4, 15
shl r0, r3, 8
shl r3, r2, 15
shl r6, r3, 15
shl r4, r6, 15
shl r2, r4, 8 | ld r4, [018f]
subxy r3, r0, r2 | ld r0, [018d]
mad r5, r3, #-0.0001220703125
mad r4, r5, #0.51834231722422253 | addxy r6, r5, #-0.0003662109375
mad r4, r0, #-0.34634525287237566 | addxy r5, r7, #1.5 | st r6, [018c] | ld r6, [c060]
mul r2, r4, #0.85324448347091675 | mul r0, r6, r5 | ld r5, [c062]
addxy r3, r2, #-0.00016196510841837153 | ld r2, [c061] | st r5, [01b4]
addxy r3, r4, #-0.00018982262593660493 | mul r4, r1, r1 | st r3, [018e]
mac_p1wc3 r7, r3, r2, #-0.00012654841729106995, r0
mul r3, r4, r4 | mul r6, r7, #1.7064889669418335
mul r2, r3, r5 | st r6, [0192] | ld r6, [0193]
addxy r0, r2, #0.24691356861851216 | ld r2, [0195]
mul r7, r0, #2.0250000953674316 | mul r0, r2, #14.0
recipseed r3, r7
newton r3, r7
newton r3, r7
newton r3, r7
ph_clamp r4, r3, -r0
mul r7, r5, r3
mul r2, r7, r4
mac_p1wc3 r0, r6, r3, #-0.00011771097204206603, r2 | ld r3, [0197]
shl r7, r0, 10
mul r4, r7, #1.1999969482421875 | ld r7, [0199]
ph_clamp r6, r3, -r4
addxy r2, r3, r6 | st r6, [0196]
mul r0, r2, r1 | addxy r4, r1, #-0.5
mac_p1wc3 r3, r7, r4, #-0.5, r0
mul r2, r3, #-2.0 | mov r0, r3
mad r0, r7, #-0.5 | st r2, [0198] | ld r2, [019b]
mac_p1w r7, r0, r1, r2, r4
mul r0, r7, #-2.0
mov r0, r7 | st r0, [019a]
mad r0, r2, #-0.5 | ld r2, [019d]
mac_p1w r0, r0, r1, r2, r4
mul r5, r0, #-2.0
mul r5, r6, r1 | mov r6, r3 | st r5, [019c]
mad r6, r5, #-0.5
mac_p1wc3 r5, r3, r4, #-0.5, r6
mac_p1wc3 r3, r5, r1, #-0.5, r7
mul r6, r7, r4
addxy r5, r3, r6 | mov r3, r0 | ld r6, [019f]
mad r3, r2, #-0.5
mac_p1wc3 r7, r5, r1, #-0.5, r0
mac_p1w r5, r3, r1, r6, r4
mul r3, r0, r4 | mul r2, r5, #-2.0
addxy r6, r7, r3 | mul r7, r5, #-1.3199996948242188 | st r2, [019e] | ld r3, [01a1]
mac_p1w r2, r5, r4, r6, r1 | ld r6, [01a5] | ld r1, [01a3]
mul r0, r2, #4.0 | mul r2, r6, #-1.0 | st r7, [01a8]
addxy r0, r2, #0.93000000715255737 | st r0, [0194]
mov r4, #0.93000000715255737 | addxy r6, r3, r1
sign r7, r0 | st r4, [01a4]
max r4, r7, -r7
addxy r2, r6, #-0.93000000715255737
mul r0, r4, r2 | mov r4, r3
subxy r7, r1, r0 | mul r1, r3, #-1.0
mul r6, r7, #-1.0 | addxy r0, r1, #0.93000000715255737
mad r4, r7, #0.0035899877548217773 | mad r3, r7, #-0.0035899877548217773 | st r6, [01a2]
sign r6, r0
max r2, r4, #0.93000000715255737
min r4, r6, #0.0
min r1, r3, #0.93000000715255737
subxy r0, r2, r1 | addxy r6, r4, #1.0 | ld r1, [01a7]
mul r7, r0, r6
addxy r3, r7, r2 | ld r7, [01a9] | ld r2, [01ab]
max r4, r1, #-0.003475784348252155 | st r3, [01a0]
mad r5, r7, #0.75757595327964178 | mul r0, r4, #0.9965362548828125
mad r5, r2, #-0.75745202755847907 | addxy r6, r0, #0.0034637451171875 | ld r0, [01ad]
mul r1, r5, #-1.3197836875915527 | st r6, [01a6] | ld r6, [0670]
mad r0, r6, #0.0079154361452299423 | st r1, [01aa] | ld r1, [01b4]
mul r6, r1, r5 | mul r7, r0, #0.98699551820755005
max r2, r0, #0.0 | st r7, [01ac]
mad r5, r6, #0.19999980926513672 | addxy r1, r4, #0.003475784348252155 | ld r4, [a1d5]
mul r7, r2, r5 | mul r6, r1, r3 | ld r2, [a1c4] | ld r1, [069b]
max r0, r7, #-0.065972779036556614 | ld r3, [c095]
mac_p1wc3 r5, r6, r0, #-0.019954273773468967, r2 | ld r0, [a16d] | ld r2, [c051]
subxy r1, r1, r0 | mul r7, r5, #-50.114577526223556 | st r0, [069a] | ld r0, [c09d]
mul r1, r1, r1 | st r3, [a125] | st r7, [0408]
ld r1, [c050] | ld r5, [a1cd]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c085]
add r1, #1.0 | st r0, [068c] | ld r0, [c08d]
min r6, r5, #0.25 | st r1, [06a4] | ld r1, [c04f]
mul r1, r3, r1 | st r0, [0692] | ld r0, [c0a5] | ld r3, [a1ad]
st r1, [06a6] | st r0, [0698] | ld r0, [0692]
ld r1, [01b7] | ld r7, [0698]
subxy r2, r1, r0 | mul r0, r4, r7
mad r1, r2, #0.0065235896036028862 | ld r2, [01b9]
addxy r5, r3, r2 | st r1, [01b6]
addxy r7, r5, #-1.0
sign r3, r7
min r2, r3, #0.0
subxy r7, r5, r2
mul r3, r7, #-1.0 | addxy r2, r7, #1.0
addxy r5, r3, #-1.0 | mul r3, r4, r2
max r5, r3, #-6.103515625e-05 | st r5, [01b8]
addxy r3, r7, #2.0
shl r2, r5, 14
mul r5, r4, r3
min r4, r5, #6.103515625e-05
shl r3, r4, 14
addxy r5, r2, #1.0 | addxy r2, r3, #-1.0
mul r4, r5, r5 | mul r5, r2, r2
subxy r3, r4, r5 | ld r5, [01bd]
mad r7, r3, #0.5 | ld r3, [01bb]
addxy r2, r7, #1.5 | mad r7, r5, #-0.96461350614310715 | ld r5, [01c1]
mad r7, r3, #0.33408930870924336 | mul r4, r2, #-2.0
mad r5, r6, #0.000244140625 | mul r2, r7, #-0.88454544544219971 | st r4, [01ba]
addxy r3, r5, #-0.000244140625 | addxy r4, r2, #-1.3268181681632996
shl r2, r3, 12 | st r4, [01bc]
sign r4, r2
min r6, r4, #0.0
mad r5, r6, #-0.000244140625
addxy r3, r5, #-0.000244140625 | addxy r4, r5, #-0.00048828125
mul r2, r0, r3 | mul r6, r0, r4 | st r3, [01c0]
mul r3, r2, r2
shl r0, r3, 15
shl r4, r0, 15
shl r2, r4, 15
mul r4, r6, r6
shl r3, r2, 15
shl r2, r4, 15
shl r0, r3, 8
shl r3, r2, 15
shl r6, r3, 15
shl r4, r6, 15
shl r2, r4, 8 | ld r4, [01c5]
subxy r3, r0, r2 | ld r0, [01c3]
mad r5, r3, #-0.0001220703125
mad r4, r5, #0.51834231722422253 | addxy r6, r5, #-0.0003662109375
mad r4, r0, #-0.34634525287237566 | addxy r5, r7, #1.5 | st r6, [01c2] | ld r6, [c060]
mul r2, r4, #0.85324448347091675 | mul r0, r6, r5 | ld r5, [c062]
addxy r3, r2, #-0.00016196510841837153 | ld r2, [c061] | st r5, [01ea]
addxy r3, r4, #-0.00018982262593660493 | mul r4, r1, r1 | st r3, [01c4]
mac_p1wc3 r7, r3, r2, #-0.00012654841729106995, r0
mul r3, r4, r4 | mul r6, r7, #1.7064889669418335
mul r2, r3, r5 | st r6, [01c8] | ld r6, [01c9]
addxy r0, r2, #0.24691356861851216 | ld r2, [01cb]
mul r7, r0, #2.0250000953674316 | mul r0, r2, #14.0
recipseed r3, r7
newton r3, r7
newton r3, r7
newton r3, r7
ph_clamp r4, r3, -r0
mul r7, r5, r3
mul r2, r7, r4
mac_p1wc3 r0, r6, r3, #-0.00011771097204206603, r2 | ld r3, [01cd]
shl r7, r0, 10
mul r4, r7, #1.1999969482421875 | ld r7, [01cf]
ph_clamp r6, r3, -r4
addxy r2, r3, r6 | st r6, [01cc]
mul r0, r2, r1 | addxy r4, r1, #-0.5
mac_p1wc3 r3, r7, r4, #-0.5, r0
mul r2, r3, #-2.0 | mov r0, r3
mad r0, r7, #-0.5 | st r2, [01ce] | ld r2, [01d1]
mac_p1w r7, r0, r1, r2, r4
mul r0, r7, #-2.0
mov r0, r7 | st r0, [01d0]
mad r0, r2, #-0.5 | ld r2, [01d3]
mac_p1w r0, r0, r1, r2, r4
mul r5, r0, #-2.0
mul r5, r6, r1 | mov r6, r3 | st r5, [01d2]
mad r6, r5, #-0.5
mac_p1wc3 r5, r3, r4, #-0.5, r6
mac_p1wc3 r3, r5, r1, #-0.5, r7
mul r6, r7, r4
addxy r5, r3, r6 | mov r3, r0 | ld r6, [01d5]
mad r3, r2, #-0.5
mac_p1wc3 r7, r5, r1, #-0.5, r0
mac_p1w r5, r3, r1, r6, r4
mul r3, r0, r4 | mul r2, r5, #-2.0
addxy r6, r7, r3 | mul r7, r5, #-1.3199996948242188 | st r2, [01d4] | ld r3, [01d7]
mac_p1w r2, r5, r4, r6, r1 | ld r6, [01db] | ld r1, [01d9]
mul r0, r2, #4.0 | mul r2, r6, #-1.0 | st r7, [01de]
addxy r0, r2, #0.93000000715255737 | st r0, [01ca]
mov r4, #0.93000000715255737 | addxy r6, r3, r1
sign r7, r0 | st r4, [01da]
max r4, r7, -r7
addxy r2, r6, #-0.93000000715255737
mul r0, r4, r2 | mov r4, r3
subxy r7, r1, r0 | mul r1, r3, #-1.0
mul r6, r7, #-1.0 | addxy r0, r1, #0.93000000715255737
mad r4, r7, #0.0035899877548217773 | mad r3, r7, #-0.0035899877548217773 | st r6, [01d8]
sign r6, r0
max r2, r4, #0.93000000715255737
min r4, r6, #0.0
min r1, r3, #0.93000000715255737
subxy r0, r2, r1 | addxy r6, r4, #1.0 | ld r1, [01dd]
mul r7, r0, r6
addxy r3, r7, r2 | ld r7, [01df] | ld r2, [01e1]
max r4, r1, #-0.003475784348252155 | st r3, [01d6]
mad r5, r7, #0.75757595327964178 | mul r0, r4, #0.9965362548828125
mad r5, r2, #-0.75745202755847907 | addxy r6, r0, #0.0034637451171875 | ld r0, [01e3]
mul r1, r5, #-1.3197836875915527 | st r6, [01dc] | ld r6, [068c]
mad r0, r6, #0.0079154361452299423 | st r1, [01e0] | ld r1, [01ea]
mul r6, r1, r5 | mul r7, r0, #0.98699551820755005
max r2, r0, #0.0 | st r7, [01e2]
mad r5, r6, #0.19999980926513672 | addxy r1, r4, #0.003475784348252155 | ld r4, [a1d6]
mul r7, r2, r5 | mul r6, r1, r3 | ld r2, [a1c5] | ld r1, [06b7]
max r0, r7, #-0.065972779036556614 | ld r3, [c096]
mac_p1wc3 r5, r6, r0, #-0.019954273773468967, r2 | ld r0, [a16e] | ld r2, [c051]
subxy r1, r1, r0 | mul r7, r5, #-50.114577526223556 | st r0, [06b6] | ld r0, [c09e]
mul r1, r1, r1 | st r3, [a126] | st r7, [040a]
ld r1, [c050] | ld r5, [a1ce]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c086]
add r1, #1.0 | st r0, [06a8] | ld r0, [c08e]
min r6, r5, #0.25 | st r1, [06c0] | ld r1, [c04f]
mul r1, r3, r1 | st r0, [06ae] | ld r0, [c0a6] | ld r3, [a1ae]
st r1, [06c2] | st r0, [06b4] | ld r0, [06ae]
ld r1, [01ed] | ld r7, [06b4]
subxy r2, r1, r0 | mul r0, r4, r7
mad r1, r2, #0.0065235896036028862 | ld r2, [01ef]
addxy r5, r3, r2 | st r1, [01ec]
addxy r7, r5, #-1.0
sign r3, r7
min r2, r3, #0.0
subxy r7, r5, r2
mul r3, r7, #-1.0 | addxy r2, r7, #1.0
addxy r5, r3, #-1.0 | mul r3, r4, r2
max r5, r3, #-6.103515625e-05 | st r5, [01ee]
addxy r3, r7, #2.0
shl r2, r5, 14
mul r5, r4, r3
min r4, r5, #6.103515625e-05
shl r3, r4, 14
addxy r5, r2, #1.0 | addxy r2, r3, #-1.0
mul r4, r5, r5 | mul r5, r2, r2
subxy r3, r4, r5 | ld r5, [01f3]
mad r7, r3, #0.5 | ld r3, [01f1]
addxy r2, r7, #1.5 | mad r7, r5, #-0.96461350614310715 | ld r5, [01f7]
mad r7, r3, #0.33408930870924336 | mul r4, r2, #-2.0
mad r5, r6, #0.000244140625 | mul r2, r7, #-0.88454544544219971 | st r4, [01f0]
addxy r3, r5, #-0.000244140625 | addxy r4, r2, #-1.3268181681632996
shl r2, r3, 12 | st r4, [01f2]
sign r4, r2
min r6, r4, #0.0
mad r5, r6, #-0.000244140625
addxy r3, r5, #-0.000244140625 | addxy r4, r5, #-0.00048828125
mul r2, r0, r3 | mul r6, r0, r4 | st r3, [01f6]
mul r3, r2, r2
shl r0, r3, 15
shl r4, r0, 15
shl r2, r4, 15
mul r4, r6, r6
shl r3, r2, 15
shl r2, r4, 15
shl r0, r3, 8
shl r3, r2, 15
shl r6, r3, 15
shl r4, r6, 15
shl r2, r4, 8 | ld r4, [01fb]
subxy r3, r0, r2 | ld r0, [01f9]
mad r5, r3, #-0.0001220703125
mad r4, r5, #0.51834231722422253 | addxy r6, r5, #-0.0003662109375
mad r4, r0, #-0.34634525287237566 | addxy r5, r7, #1.5 | st r6, [01f8] | ld r6, [c060]
mul r2, r4, #0.85324448347091675 | mul r0, r6, r5 | ld r5, [c062]
addxy r3, r2, #-0.00016196510841837153 | ld r2, [c061] | st r5, [0220]
addxy r3, r4, #-0.00018982262593660493 | mul r4, r1, r1 | st r3, [01fa]
mac_p1wc3 r7, r3, r2, #-0.00012654841729106995, r0
mul r3, r4, r4 | mul r6, r7, #1.7064889669418335
mul r2, r3, r5 | st r6, [01fe] | ld r6, [01ff]
addxy r0, r2, #0.24691356861851216 | ld r2, [0201]
mul r7, r0, #2.0250000953674316 | mul r0, r2, #14.0
recipseed r3, r7
newton r3, r7
newton r3, r7
newton r3, r7
ph_clamp r4, r3, -r0
mul r7, r5, r3
mul r2, r7, r4
mac_p1wc3 r0, r6, r3, #-0.00011771097204206603, r2 | ld r3, [0203]
shl r7, r0, 10
mul r4, r7, #1.1999969482421875 | ld r7, [0205]
ph_clamp r6, r3, -r4
addxy r2, r3, r6 | st r6, [0202]
mul r0, r2, r1 | addxy r4, r1, #-0.5
mac_p1wc3 r3, r7, r4, #-0.5, r0
mul r2, r3, #-2.0 | mov r0, r3
mad r0, r7, #-0.5 | st r2, [0204] | ld r2, [0207]
mac_p1w r7, r0, r1, r2, r4
mul r0, r7, #-2.0
mov r0, r7 | st r0, [0206]
mad r0, r2, #-0.5 | ld r2, [0209]
mac_p1w r0, r0, r1, r2, r4
mul r5, r0, #-2.0
mul r5, r6, r1 | mov r6, r3 | st r5, [0208]
mad r6, r5, #-0.5
mac_p1wc3 r5, r3, r4, #-0.5, r6
mac_p1wc3 r3, r5, r1, #-0.5, r7
mul r6, r7, r4
addxy r5, r3, r6 | mov r3, r0 | ld r6, [020b]
mad r3, r2, #-0.5
mac_p1wc3 r7, r5, r1, #-0.5, r0
mac_p1w r5, r3, r1, r6, r4
mul r3, r0, r4 | mul r2, r5, #-2.0
addxy r6, r7, r3 | mul r7, r5, #-1.3199996948242188 | st r2, [020a] | ld r3, [020d]
mac_p1w r2, r5, r4, r6, r1 | ld r6, [0211] | ld r1, [020f]
mul r0, r2, #4.0 | mul r2, r6, #-1.0 | st r7, [0214]
addxy r0, r2, #0.93000000715255737 | st r0, [0200]
mov r4, #0.93000000715255737 | addxy r6, r3, r1
sign r7, r0 | st r4, [0210]
max r4, r7, -r7
addxy r2, r6, #-0.93000000715255737
mul r0, r4, r2 | mov r4, r3
subxy r7, r1, r0 | mul r1, r3, #-1.0
mul r6, r7, #-1.0 | addxy r0, r1, #0.93000000715255737
mad r4, r7, #0.0035899877548217773 | mad r3, r7, #-0.0035899877548217773 | st r6, [020e]
sign r6, r0
max r2, r4, #0.93000000715255737
min r4, r6, #0.0
min r1, r3, #0.93000000715255737
subxy r0, r2, r1 | addxy r6, r4, #1.0 | ld r1, [0213]
mul r7, r0, r6
addxy r3, r7, r2 | ld r7, [0215] | ld r2, [0217]
max r4, r1, #-0.003475784348252155 | st r3, [020c]
mad r5, r7, #0.75757595327964178 | mul r0, r4, #0.9965362548828125
mad r5, r2, #-0.75745202755847907 | addxy r6, r0, #0.0034637451171875 | ld r0, [0219]
mul r1, r5, #-1.3197836875915527 | st r6, [0212] | ld r6, [06a8]
mad r0, r6, #0.0079154361452299423 | st r1, [0216] | ld r1, [0220]
mul r6, r1, r5 | mul r7, r0, #0.98699551820755005
max r2, r0, #0.0 | st r7, [0218]
mad r5, r6, #0.19999980926513672 | addxy r1, r4, #0.003475784348252155 | ld r4, [a1d7]
mul r7, r2, r5 | mul r6, r1, r3 | ld r2, [a1c6] | ld r1, [06d3]
max r0, r7, #-0.065972779036556614 | ld r3, [c097]
mac_p1wc3 r5, r6, r0, #-0.019954273773468967, r2 | ld r0, [a16f] | ld r2, [c051]
subxy r1, r1, r0 | mul r7, r5, #-50.114577526223556 | st r0, [06d2] | ld r0, [c09f]
mul r1, r1, r1 | st r3, [a127] | st r7, [040c]
ld r1, [c050] | ld r5, [a1cf]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c087]
add r1, #1.0 | st r0, [06c4] | ld r0, [c08f]
min r6, r5, #0.25 | st r1, [06dc] | ld r1, [c04f]
mul r1, r3, r1 | st r0, [06ca] | ld r0, [c0a7] | ld r3, [a1af]
st r1, [06de] | st r0, [06d0] | ld r0, [06ca]
ld r1, [0223] | ld r7, [06d0]
subxy r2, r1, r0 | mul r0, r4, r7
mad r1, r2, #0.0065235896036028862 | ld r2, [0225]
addxy r5, r3, r2 | st r1, [0222]
addxy r7, r5, #-1.0
sign r3, r7
min r2, r3, #0.0
subxy r7, r5, r2
mul r3, r7, #-1.0 | addxy r2, r7, #1.0
addxy r5, r3, #-1.0 | mul r3, r4, r2
max r5, r3, #-6.103515625e-05 | st r5, [0224]
addxy r3, r7, #2.0
shl r2, r5, 14
mul r5, r4, r3
min r4, r5, #6.103515625e-05
shl r3, r4, 14
addxy r5, r2, #1.0 | addxy r2, r3, #-1.0
mul r4, r5, r5 | mul r5, r2, r2
subxy r3, r4, r5 | ld r5, [0229]
mad r7, r3, #0.5 | ld r3, [0227]
addxy r2, r7, #1.5 | mad r7, r5, #-0.96461350614310715 | ld r5, [022d]
mad r7, r3, #0.33408930870924336 | mul r4, r2, #-2.0
mad r5, r6, #0.000244140625 | mul r2, r7, #-0.88454544544219971 | st r4, [0226]
addxy r3, r5, #-0.000244140625 | addxy r4, r2, #-1.3268181681632996
shl r2, r3, 12 | st r4, [0228]
sign r4, r2
min r6, r4, #0.0
mad r5, r6, #-0.000244140625
addxy r3, r5, #-0.000244140625 | addxy r4, r5, #-0.00048828125
mul r2, r0, r3 | mul r6, r0, r4 | st r3, [022c]
mul r3, r2, r2
shl r0, r3, 15
shl r4, r0, 15
shl r2, r4, 15
mul r4, r6, r6
shl r3, r2, 15
shl r2, r4, 15
shl r0, r3, 8
shl r3, r2, 15
shl r6, r3, 15
shl r4, r6, 15
shl r2, r4, 8 | ld r4, [0231]
subxy r3, r0, r2 | ld r0, [022f]
mad r5, r3, #-0.0001220703125
mad r4, r5, #0.51834231722422253 | addxy r6, r5, #-0.0003662109375
mad r4, r0, #-0.34634525287237566 | addxy r5, r7, #1.5 | st r6, [022e] | ld r6, [c060]
mul r2, r4, #0.85324448347091675 | mul r0, r6, r5 | ld r5, [c062]
addxy r3, r2, #-0.00016196510841837153 | ld r2, [c061] | st r5, [0256]
addxy r3, r4, #-0.00018982262593660493 | mul r4, r1, r1 | st r3, [0230]
mac_p1wc3 r7, r3, r2, #-0.00012654841729106995, r0
mul r3, r4, r4 | mul r6, r7, #1.7064889669418335
mul r2, r3, r5 | st r6, [0234] | ld r6, [0235]
addxy r0, r2, #0.24691356861851216 | ld r2, [0237]
mul r7, r0, #2.0250000953674316 | mul r0, r2, #14.0
recipseed r3, r7
newton r3, r7
newton r3, r7
newton r3, r7
ph_clamp r4, r3, -r0
mul r7, r5, r3
mul r2, r7, r4
mac_p1wc3 r0, r6, r3, #-0.00011771097204206603, r2 | ld r3, [0239]
shl r7, r0, 10
mul r4, r7, #1.1999969482421875 | ld r7, [023b]
ph_clamp r6, r3, -r4
addxy r2, r3, r6 | st r6, [0238]
mul r0, r2, r1 | addxy r4, r1, #-0.5
mac_p1wc3 r3, r7, r4, #-0.5, r0
mul r2, r3, #-2.0 | mov r0, r3
mad r0, r7, #-0.5 | st r2, [023a] | ld r2, [023d]
mac_p1w r7, r0, r1, r2, r4
mul r0, r7, #-2.0
mov r0, r7 | st r0, [023c]
mad r0, r2, #-0.5 | ld r2, [023f]
mac_p1w r0, r0, r1, r2, r4
mul r5, r0, #-2.0
mul r5, r6, r1 | mov r6, r3 | st r5, [023e]
mad r6, r5, #-0.5
mac_p1wc3 r5, r3, r4, #-0.5, r6
mac_p1wc3 r3, r5, r1, #-0.5, r7
mul r6, r7, r4
addxy r5, r3, r6 | mov r3, r0 | ld r6, [0241]
mad r3, r2, #-0.5
mac_p1wc3 r7, r5, r1, #-0.5, r0
mac_p1w r5, r3, r1, r6, r4
mul r3, r0, r4 | mul r2, r5, #-2.0
addxy r6, r7, r3 | mul r7, r5, #-1.3199996948242188 | st r2, [0240] | ld r3, [0243]
mac_p1w r2, r5, r4, r6, r1 | ld r6, [0247] | ld r1, [0245]
mul r0, r2, #4.0 | mul r2, r6, #-1.0 | st r7, [024a]
addxy r0, r2, #0.93000000715255737 | st r0, [0236]
mov r4, #0.93000000715255737 | addxy r6, r3, r1
sign r7, r0 | st r4, [0246]
max r4, r7, -r7
addxy r2, r6, #-0.93000000715255737
mul r0, r4, r2 | mov r4, r3
subxy r7, r1, r0 | mul r1, r3, #-1.0
mul r6, r7, #-1.0 | addxy r0, r1, #0.93000000715255737
mad r4, r7, #0.0035899877548217773 | mad r3, r7, #-0.0035899877548217773 | st r6, [0244]
sign r6, r0
max r2, r4, #0.93000000715255737
min r4, r6, #0.0
min r1, r3, #0.93000000715255737
subxy r0, r2, r1 | addxy r6, r4, #1.0 | ld r1, [0249]
mul r7, r0, r6
addxy r3, r7, r2 | ld r7, [024b] | ld r2, [024d]
max r4, r1, #-0.003475784348252155 | st r3, [0242]
mad r5, r7, #0.75757595327964178 | mul r0, r4, #0.9965362548828125
mad r5, r2, #-0.75745202755847907 | addxy r6, r0, #0.0034637451171875 | ld r0, [024f]
mul r1, r5, #-1.3197836875915527 | st r6, [0248] | ld r6, [06c4]
mad r0, r6, #0.0079154361452299423 | st r1, [024c] | ld r1, [0256]
mul r6, r1, r5 | mul r7, r0, #0.98699551820755005
max r2, r0, #0.0 | st r7, [024e]
mad r5, r6, #0.19999980926513672 | addxy r1, r4, #0.003475784348252155
mul r7, r2, r5 | mul r6, r1, r3 | ld r2, [a1c7]
max r0, r7, #-0.065972779036556614
mac_p1wc3 r5, r6, r0, #-0.019954273773468967, r2
mul r7, r5, #-50.114577526223556
st r7, [040e]
