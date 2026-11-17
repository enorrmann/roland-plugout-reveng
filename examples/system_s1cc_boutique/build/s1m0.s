ld r0, [a060] | ld r1, [0181]
mov r0, #0.0 | st r0, [a168]
ld r2, [a038] | st r0, [0300]
ld r0, [a1b8]
subxy r2, r1, r0 | st r2, [a160]
mad r1, r2, #0.0065235896036028862 | ld r5, [a030] | ld r2, [0183]
min r6, r5, #0.25 | ld r3, [a040] | st r1, [0180]
addxy r5, r3, r2 | ld r4, [a048]
ld r7, [a1a0]
mul r0, r4, r7 | addxy r7, r5, #-1.0
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
mad r4, r0, #-0.34634525287237566 | addxy r5, r7, #1.5 | st r6, [018c] | ld r6, [c070]
mul r2, r4, #0.85324448347091675 | mul r0, r6, r5 | ld r5, [c072]
addxy r3, r2, #-0.00016196510841837153 | ld r2, [c071] | st r5, [01b4]
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
mul r1, r5, #-1.3197836875915527 | st r6, [01a6] | ld r6, [a1b0]
mad r0, r6, #0.0079154361452299423 | st r1, [01aa] | ld r1, [01b4]
mul r6, r1, r5 | mul r7, r0, #0.98699551820755005
max r2, r0, #0.0 | st r7, [01ac]
mad r5, r6, #0.19999980926513672 | addxy r1, r4, #0.003475784348252155 | ld r4, [a049]
mul r7, r2, r5 | mul r6, r1, r3 | ld r2, [0300] | ld r1, [01b7] | ld r3, [a041]
max r0, r7, #-0.065972779036556614
mac_p1wc3 r5, r6, r0, #-0.019954273773468967, r2 | ld r0, [a061]
mov r0, #0.0 | mul r7, r5, #-50.114577526223556 | st r0, [a169]
ld r2, [a039] | st r0, [0302]
ld r0, [a1b9]
subxy r2, r1, r0 | st r2, [a161]
mad r1, r2, #0.0065235896036028862 | ld r5, [a031] | ld r2, [01b9]
min r6, r5, #0.25 | st r7, [a151] | st r1, [01b6]
addxy r5, r3, r2 | ld r7, [a1a1]
mul r0, r4, r7 | addxy r7, r5, #-1.0
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
mad r4, r0, #-0.34634525287237566 | addxy r5, r7, #1.5 | st r6, [01c2] | ld r6, [c070]
mul r2, r4, #0.85324448347091675 | mul r0, r6, r5 | ld r5, [c072]
addxy r3, r2, #-0.00016196510841837153 | ld r2, [c071] | st r5, [01ea]
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
mul r1, r5, #-1.3197836875915527 | st r6, [01dc] | ld r6, [a1b1]
mad r0, r6, #0.0079154361452299423 | st r1, [01e0] | ld r1, [01ea]
mul r6, r1, r5 | mul r7, r0, #0.98699551820755005
max r2, r0, #0.0 | st r7, [01e2]
mad r5, r6, #0.19999980926513672 | addxy r1, r4, #0.003475784348252155 | ld r4, [a04a]
mul r7, r2, r5 | mul r6, r1, r3 | ld r2, [0302] | ld r1, [01ed] | ld r3, [a042]
max r0, r7, #-0.065972779036556614
mac_p1wc3 r5, r6, r0, #-0.019954273773468967, r2 | ld r0, [a062]
mov r0, #0.0 | mul r7, r5, #-50.114577526223556 | st r0, [a16a]
ld r2, [a03a] | st r0, [0304]
ld r0, [a1ba]
subxy r2, r1, r0 | st r2, [a162]
mad r1, r2, #0.0065235896036028862 | ld r5, [a032] | ld r2, [01ef]
min r6, r5, #0.25 | st r7, [a152] | st r1, [01ec]
addxy r5, r3, r2 | ld r7, [a1a2]
mul r0, r4, r7 | addxy r7, r5, #-1.0
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
mad r4, r0, #-0.34634525287237566 | addxy r5, r7, #1.5 | st r6, [01f8] | ld r6, [c070]
mul r2, r4, #0.85324448347091675 | mul r0, r6, r5 | ld r5, [c072]
addxy r3, r2, #-0.00016196510841837153 | ld r2, [c071] | st r5, [0220]
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
mul r1, r5, #-1.3197836875915527 | st r6, [0212] | ld r6, [a1b2]
mad r0, r6, #0.0079154361452299423 | st r1, [0216] | ld r1, [0220]
mul r6, r1, r5 | mul r7, r0, #0.98699551820755005
max r2, r0, #0.0 | st r7, [0218]
mad r5, r6, #0.19999980926513672 | addxy r1, r4, #0.003475784348252155 | ld r4, [a04b]
mul r7, r2, r5 | mul r6, r1, r3 | ld r2, [0304] | ld r1, [0223] | ld r3, [a043]
max r0, r7, #-0.065972779036556614
mac_p1wc3 r5, r6, r0, #-0.019954273773468967, r2 | ld r0, [a063]
mov r0, #0.0 | mul r7, r5, #-50.114577526223556 | st r0, [a16b]
ld r2, [a03b] | st r0, [0306]
ld r0, [a1bb]
subxy r2, r1, r0 | st r2, [a163]
mad r1, r2, #0.0065235896036028862 | ld r5, [a033] | ld r2, [0225]
min r6, r5, #0.25 | st r7, [a153] | st r1, [0222]
addxy r5, r3, r2 | ld r7, [a1a3]
mul r0, r4, r7 | addxy r7, r5, #-1.0
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
mad r4, r0, #-0.34634525287237566 | addxy r5, r7, #1.5 | st r6, [022e] | ld r6, [c070]
mul r2, r4, #0.85324448347091675 | mul r0, r6, r5 | ld r5, [c072]
addxy r3, r2, #-0.00016196510841837153 | ld r2, [c071] | st r5, [0256]
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
mul r1, r5, #-1.3197836875915527 | st r6, [0248] | ld r6, [a1b3]
mad r0, r6, #0.0079154361452299423 | st r1, [024c] | ld r1, [0256]
mul r6, r1, r5 | mul r7, r0, #0.98699551820755005
max r2, r0, #0.0 | st r7, [024e]
mad r5, r6, #0.19999980926513672 | addxy r1, r4, #0.003475784348252155
mul r7, r2, r5 | mul r6, r1, r3 | ld r2, [0306] | ld r3, [a04c]
max r0, r7, #-0.065972779036556614 | st r3, [a1d4]
mac_p1wc3 r5, r6, r0, #-0.019954273773468967, r2 | ld r0, [a064]
mov r0, #0.0 | mul r7, r5, #-50.114577526223556 | st r0, [a16c]
st r0, [0308] | ld r0, [a044]
st r0, [a1ac] | ld r1, [0308]
ld r0, [a065]
mov r0, #0.0 | st r0, [a16d]
ld r2, [a03c] | st r0, [030a]
st r2, [a164]
ld r0, [a045]
ld r2, [a034]
st r0, [a1ad]
st r2, [a1cc]
ld r0, [a066]
ld r2, [a03d]
mov r0, #0.0 | st r0, [a16e]
st r2, [a165] | st r0, [030c]
ld r2, [a035]
st r2, [a1cd]
ld r0, [a046]
ld r2, [a03e]
st r0, [a1ae]
st r1, [a1c4] | ld r1, [030a]
st r2, [a166]
ld r0, [a067]
ld r3, [a04d]
ld r2, [a036]
mov r0, #0.0 | st r0, [a16f]
st r1, [a1c5] | ld r1, [030c] | st r0, [030e]
st r3, [a1d5]
st r2, [a1ce]
ld r3, [a04e]
ld r2, [a03f]
st r1, [a1c6] | ld r1, [030e]
st r3, [a1d6]
st r2, [a167]
ld r0, [a047]
ld r2, [a037]
ld r3, [a04f]
st r7, [a154]
st r0, [a1af]
st r1, [a1c7]
st r2, [a1cf]
st r3, [a1d7]
