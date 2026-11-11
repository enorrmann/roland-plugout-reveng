mov r7, #0.0 | ld r0, [a151] | ld r1, [0403] | ld r2, [064f]
addxy r7, r7, r0 | mul r2, r2, #7.31275 | ld r0, [0405] | ld r3, [c090] | ld r5, [0181]
addxy r7, r7, r1 | ld r1, [060d] | st r3, [a120] | ld r6, [c000]
addxy r7, r7, r0 | mov r0, #0.0 | st r5, [01d6]
mul r7, r7, #0.4444444444444444 | st r0, [6008]
mul r7, r7, r6 | st r0, [6009] | ld r6, [a1a1]
st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [0651]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [0650] | ld r0, [a168] | ld r7, [c063]
subxy r1, r1, r0 | st r0, [060c] | ld r0, [c098]
mul r1, r1, r1 | st r6, [01d4] | ld r6, [c062]
ph_wrap r2, r3, r2 | ld r1, [c050]
mul r1, r3, r1 | st r2, [064e] | ld r2, [c051]
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
mad r1, r0, -r2 | ld r0, [c081]
add r1, #1.0 | st r0, [061a] | st r0, [a171]
st r1, [0630] | ld r1, [c04f]
mul r1, r3, r1 | ld r0, [c089] | ld r3, [061a]
st r1, [0632] | st r0, [0620]
mul r4, r3, r3 | st r0, [a179] | ld r1, [c067] | ld r2, [0620]
mul r2, r2, r2 | ld r0, [a1d9] | ld r3, [c064] | st r4, [01d8]
mad r1, r0, #-0.0625 | ld r0, [0183] | ld r4, [0185]
mac_p1w r3, r3, r5, r0, r7 | ld r0, [c061] | ld r5, [0632]
mad r3, r4, #-0.125 | addxy r1, r1, #0.25 | ld r7, [c06a]
mad r4, r3, #0.00059102423256263137 | st r3, [01da] | ld r3, [c060]
st r4, [0184] | ld r4, [c06b]
mac_p1wc3 r2, r2, r0, #0.015625806566982686, r4 | st r7, [01dc]
mad r2, r5, #0.12500645253586148
mac_p1wc3 r0, r6, r1, #-0.24998709559440613, r2
mad r0, r3, #-0.33203125
addxy r4, r0, #-0.0052062496542930603 | addxy r6, r0, #1.0357659163623476
mul r1, r6, r4 | mul r5, r4, r4
addxy r2, r1, #0.35801112009064834 | addxy r6, r0, #0.33854375034570694
mac_p1wc3 r3, r5, r2, #0.039918037988250651, r0
mul r4, r6, #-32.0 | mov r2, r3
sign r1, r4
mad r2, r0, #0.0032823169182762389
min r5, r1, #0.0
addxy r6, r2, #-0.00022491183279744315
mul r4, r6, r5
addxy r2, r4, r3
mad r3, r2, #-0.25 | addxy r4, r5, #1.0
addxy r6, r3, #-0.00015586745360729042
mac_p1wc3 r3, r6, r4, #0.25, r2
mul r2, r0, #4.76837158203125e-07
max r5, r1, #0.0
max r6, r1, -r1
mul r1, r2, #1.7208572168100142 | mul r4, r5, r6
addxy r5, r3, r1
addxy r6, r5, #-0.00033375212964021711
mul r0, r4, r6
subxy r2, r3, r0
shl r1, r2, 11
min r5, r1, #0.051145697081349195
addxy r4, r5, #0.30334452120476779
mov r3, #0.041666666666666664 | mul r6, r4, #-0.82490699372328891
mov r0, r3
ymacd r0, r6, #0.16666666666666666
ymacd r0, r6, #0.5
ymacd r0, r6, #1.0
ymacd r0, r6, #1.0
mul r2, r0, r0
mul r1, r2, r2
mul r5, r1, r1
mul r4, r5, r5
mul r6, r4, #0.00390625
mul r0, r6, r6
mul r2, r0, r0 | mul r1, r0, r6
mac_p1wc3 r4, r6, r6, #-0.0029344578016347305, r2
mul r5, r1, r0 | addxy r4, r4, #-197.87984592782817
mul r5, r5, r4 | mov r4, r6
mad r4, r1, #-0.035383472137614391 | mul r1, r5, #9.5367431640625e-07
mul r5, r1, #-1.9903222525650495
addxy r1, r4, r5 | mul r4, r0, #0.0016586065835326735
addxy r5, r4, #-0.8125
mul r0, r5, r2
mac_p1wc3 r4, r6, r6, #0.0046649689308783868, r0
addxy r5, r4, #-141.3124173314047
mul r0, r5, r2 | mov r2, r1
mul r4, r0, #0.03125
mac_p1wc3 r5, r6, r6, #0.0040062804431907906, r4
mad r2, r5, #-0.23038101196289062
mul r0, r2, #0.46076202392578125
addxy r6, r0, #1.0
recipseed r4, r6
newton r4, r6
newton r4, r6
newton r4, r6
mul r2, r1, r4
mul r0, r2, r2
mul r5, r0, r0
mul r6, r5, r7 | ld r5, [01da]
addxy r1, r6, #0.25679096095835036 | st r3, [01da]
mul r4, r1, #3.8942180685331551
recipseed r0, r4
newton r0, r4
newton r0, r4
newton r0, r4
mul r6, r5, r0 | mul r1, r0, r7 | ld r4, [0187] | ld r5, [0189]
mad r4, r5, #0.10000000149011612 | st r4, [0196]
mul r0, r4, #0.15000000596046448
ph_clamp r5, r3, -r0
mac_p1wc3 r4, r1, r5, #-0.022222221486362097, r6
mul r0, r4, #-5.3999977111816406
ph_clamp r1, r3, -r0 | ld r0, [018b]
mul r5, r1, r1
mul r6, r5, r1
mad r1, r6, #-0.33333301544189453
mul r4, r1, #2.0 | mov r5, r1
mad r5, r0, #0.5 | addxy r0, r2, #-0.54257943801433939 | st r4, [018a] | ld r4, [018d]
mul r6, r5, r2 | mul r5, r0, r4
subxy r5, r5, r6
mul r6, r5, #1.843048095703125
mov r6, r5 | st r6, [018c]
mad r6, r4, #0.54257943801433939
mul r4, r6, r2 | ld r6, [018f]
mac_p1wc3 r4, r0, r6, #-0.9215240478515625, r4
mul r3, r4, #-1.843048095703125
mov r3, r4 | st r3, [018e]
mad r3, r6, #-0.54257943801433939 | ld r6, [0191]
mul r7, r0, r6
mac_p1wc3 r3, r3, r2, #0.54257943801433939, r7
mul r7, r3, #-3.3968262830749154
mul r7, r1, r2 | st r7, [0190]
addxy r1, r5, r7
mac_p1wc3 r7, r5, r0, #-0.54257943801433939, r1
mac_p1wc3 r5, r7, r2, #0.58878489311191273, r4
mac_p1wc3 r1, r4, r0, #-0.9215240478515625, r5
mac_p1wc3 r7, r1, r2, #-0.54257943801433939, r3 | ld r1, [0193]
mul r4, r3, r0 | mad r3, r6, #-0.29439244655595637
addxy r5, r7, r4 | mul r7, r1, r0
mac_p1wc3 r4, r3, r2, #0.58878489311191273, r7 | ld r7, [0195]
mul r6, r4, #-3.1302571062277735
mac_p1w r1, r4, r0, r5, r2 | st r6, [0192] | ld r2, [01d8]
mul r6, r7, #0.9924468994140625 | mul r5, r2, r2
mul r3, r1, #5.769214398694273 | addxy r0, r6, #0.0075531005859375
addxy r1, r5, #0.41912713505585858 | st r3, [0186]
mul r3, r1, r2 | st r0, [0194] | ld r0, [01dc]
max r6, r3, #0.011256057508677425
mul r5, r4, r0 | addxy r1, r6, #-0.011256057508677425 | ld r6, [a1a9]
mad r4, r5, #0.5 | addxy r3, r7, #0.0076105840931104994 | ld r5, [0197] | ld r7, [01d6]
mul r2, r4, r1 | ld r1, [c06e] | st r5, [0188] | ld r5, [01da]
mul r0, r3, r2 | mul r3, r7, r1 | ld r4, [a1e1] | st r6, [01dc]
mul r2, r3, #0.34657359027997264 | mov r7, r5 | st r0, [01da] | ld r0, [019b]
ymacd r7, r2, #0.16666666666666666 | st r4, [01d6]
ymacd r7, r2, #0.5
ymacd r7, r2, #1.0
ymacd r7, r2, #1.0
mul r1, r7, r7
mul r2, r1, r1
mul r7, r2, r2
mul r1, r7, r7
mul r2, r1, #0.03125 | mul r1, r3, #-0.34657359027997264
ymacd r5, r1, #0.16666666666666666 | mul r7, r6, r2
ymacd r5, r1, #0.5
ymacd r5, r1, #1.0
ymacd r5, r1, #1.0
mul r2, r5, r5
mul r3, r2, r2
mul r1, r3, r3
mul r5, r1, r1
mul r2, r5, #0.03125 | ld r5, [0199]
min r1, r7, #0.0077812499366700649
mul r3, r4, r2 | mov r2, r1
mov r4, r1
mad r2, r5, #0.03125 | mad r4, r0, #0.03125
mul r7, r2, #32.0 | mul r0, r4, #32.0
ph_wrap r2, r3, r7
ph_wrap r4, r3, r0
subxy r0, r5, r2
sign r5, r0
mov r7, r1
min r0, r5, #0.0
subxy r5, r4, r2 | mad r7, r2, #0.03125
mul r2, r0, r5 | mul r7, r7, #32.0
subxy r0, r4, r2
mad r1, r0, #-0.03125
mul r5, r1, #32.0
ph_wrap r4, r3, r5
ph_wrap r7, r3, r7
sign r2, r4 | st r7, [0198]
min r1, r2, #0.0
subxy r5, r4, r7
mul r2, r1, r5 | mul r5, r0, #-1.0
subxy r7, r4, r2
ph_wrap r4, r3, r5
mul r5, r0, #-0.5 | subxy r2, r0, r4
addxy r4, r5, #0.5 | mul r1, r7, #-1.0
ph_tri r0, r3, -r4 | st r1, [019a]
mul r5, r3, r0
shl r4, r5, 15
shl r0, r4, 2
ph_clamp r5, r3, -r0
mul r4, r5, r5
mul r0, r5, r4
mul r6, r4, r0 | mul r4, r4, r4
mac_p1wc3 r4, r5, r5, #-0.034104689076754152, r4
addxy r4, r4, #-17.026106186230564
mul r6, r6, r4
mul r4, r6, #0.03125
mad r5, r4, #-0.095352512591594205
mad r5, r0, #-0.41123178361504448
mul r6, r2, r5
mul r4, r6, #0.78539800643920898
ph_wrap r0, r3, r1 | st r4, [01a2]
mul r1, r7, #-0.5 | subxy r4, r0, r7
addxy r0, r1, #0.5
ph_tri r7, r3, -r0
mul r1, r3, r7
shl r0, r1, 15
shl r3, r0, 2
ph_clamp r7, r3, -r3
mul r1, r7, r7
mul r0, r7, r1
mul r3, r1, r0 | mul r1, r1, r1
mac_p1wc3 r1, r7, r7, #-0.034104689076754152, r1
addxy r1, r1, #-17.026106186230564
mul r3, r3, r1
mul r1, r3, #0.03125
mad r7, r1, #-0.095352512591594205
mad r7, r0, #-0.41123178361504448 | ld r0, [019d]
mul r3, r4, r7 | mov r7, r0 | ld r4, [019f]
mul r1, r3, #-0.78539800643920898 | mad r7, r4, #-0.50126103797649757
st r1, [01a0] | ld r1, [01a1]
mov r4, r1
mad r4, r6, #0.39269900321960449 | ld r6, [01a3]
mad r4, r6, #0.5
mad r7, r4, #0.034666201629152259
mul r6, r7, #1.5688642263412476 | mov r4, r7
st r6, [019e] | st r6, [01ac]
mac_p1wc3 r6, r2, r5, #-0.5, r3 | ld r2, [01a5] | ld r5, [01a7]
mad r4, r0, #-0.31950568415055886 | mad r6, r1, #0.63661989959316345 | ld r3, [01a9] | ld r1, [01ab]
mad r4, r6, #0.017354443547897992 | mad r7, r5, #0.63740378753622462 | st r3, [01b0]
mul r0, r4, #2.4613349606933212 | mad r4, r2, #0.40628358836552458 | ld r2, [01af] | st r1, [01a4]
st r0, [019c] | st r0, [01a8] | ld r0, [01ad]
mad r7, r4, #0.75290865113054539 | addxy r6, r3, r1 | ld r4, [01b1] | ld r3, [01b3]
mad r7, r6, #-0.10523075837658179 | addxy r5, r0, r2 | ld r6, [01b5] | st r0, [01b4]
mad r7, r5, #-0.59394325412832727 | addxy r1, r4, r3 | st r6, [01ae] | ld r5, [c065]
mad r1, r7, #0.69499853757836605 | st r3, [01aa] | ld r3, [c069]
mad r6, r1, #0.45157973770018534 | st r2, [01a6] | ld r7, [c066]
mul r0, r6, #0.48420281195382131 | mul r1, r3, r7 | ld r2, [c068] | st r4, [01b2] | ld r7, [01d6]
mul r4, r2, r5 | st r0, [0182] | ld r6, [c06c] | ld r0, [01dc]
mul r2, r0, r6 | ld r3, [c06d]
mul r0, r7, r3 | mul r5, r4, r2 | ld r7, [01d4]
mul r3, r7, #0.5400000071525568 | mul r6, r1, r0
min r4, r5, #0.00048632812104187906 | st r6, [01d6] | ld r6, [01b7]
addxy r1, r3, #0.51499999966472387 | addxy r0, r7, #-0.89814813687263062
shl r2, r4, 11
recipseed r4, r1
newton r4, r1
newton r4, r1
newton r4, r1
mul r1, r7, #-1.0 | mul r3, r0, #-1.0800000143051136 | st r4, [01dc]
addxy r0, r1, #0.89814813687263062 | mad r6, r2, #0.25
sign r5, r3
max r3, r0, -r0 | st r5, [01de]
max r1, r3, #0.0018084490501203063
mul r0, r1, #34.560000457763635
recipseed r3, r0
newton r3, r0
newton r3, r0
newton r3, r0
ph_wrap r4, r3, r6
mov r0, r4 | mul r1, r5, r3
mad r0, r2, #0.25 | st r1, [01d8]
ph_wrap r6, r3, r0 | ld r0, [01b9]
mad r0, r2, #0.25 | st r6, [01b6]
ph_wrap r0, r3, r0
sign r1, r0
min r1, r1, #0.0
subxy r5, r0, r4
mul r1, r1, r5
subxy r5, r0, r1
mov r0, r5
mad r0, r2, #-0.25
mul r1, r0, #-1.0
ph_wrap r2, r3, r1
sign r0, r2
absadd r1, r0, #-1.0
mul r0, r1, r2
addxy r1, r0, r2 | subxy r0, r4, r6
sign r2, r0
min r4, r2, #0.0
subxy r0, r1, r6
mul r2, r4, r0 | mov r0, r7
mad r0, r5, #0.92592591366159682 | subxy r6, r1, r2
addxy r1, r0, #0.027777776788966198 | mul r4, r6, #-1.0 | ld r0, [01dc]
mad r7, r6, #0.92592591366159682 | mul r2, r1, #-1.0800000143051136 | st r4, [01b8] | ld r6, [01de]
sign r4, r2
mac_p1wc3 r2, r6, r3, #0.015625, r0 | ld r3, [01d8]
min r5, r4, #0.0
mul r6, r2, #0.03125
mac_p1wc3 r0, r5, r6, #0.03125, r3
mul r2, r1, r0
shl r5, r2, 10 | ld r2, [01d6]
mul r1, r5, #-1.0800000143051136
ph_tri r0, r3, -r1
mul r5, r2, r0
shl r1, r5, 15
shl r0, r1, 5
mul r5, r0, #1.6000000238418579 | addxy r0, r7, #0.027777776788966198
ph_clamp r1, r3, -r5
mul r5, r0, #-1.0800000143051136
sign r7, r5
min r5, r7, #0.0
mac_p1wc3 r5, r5, r6, #0.03125, r3
mul r6, r0, r5
shl r3, r6, 10
mul r0, r3, #-1.0800000143051136
ph_tri r5, r3, -r0
mul r6, r2, r5
shl r3, r6, 15
shl r0, r3, 5
mul r2, r0, #1.6000000238418579 | mul r6, r1, r4
ph_clamp r5, r3, -r2
mul r3, r6, #-1.0 | mul r0, r5, r7 | ld r5, [01bd]
mul r2, r0, #-1.0 | st r3, [01c0] | ld r3, [01bb]
mov r7, r3 | st r2, [01be] | ld r2, [01bf]
mad r7, r5, #-0.4735108174809789
mov r5, r2
mad r5, r6, #-0.5 | ld r6, [01c1]
mad r5, r6, #0.5
mad r7, r5, #0.02471467541816923
mul r6, r7, #1.5991098880767822 | mov r5, r7
st r6, [01bc] | st r6, [01ca]
mac_p1wc3 r6, r1, r4, #0.5, r0 | ld r1, [01c3] | ld r4, [01c5]
mad r5, r3, #-0.29610899226597925 | mad r6, r2, #-0.5 | ld r0, [01c7] | ld r2, [01c9]
mad r5, r6, #-0.015455270211538169 | mad r7, r4, #0.62534789351010778 | st r0, [01ce]
mul r3, r5, #2.557152434144939 | mad r5, r1, #0.39105998791752911 | ld r1, [01cd] | st r2, [01c2]
st r3, [01ba] | st r3, [01c6] | ld r3, [01cb]
mad r7, r5, #0.76742375065127288 | addxy r6, r0, r2 | ld r5, [01cf] | ld r0, [01d1]
mad r7, r6, #-0.10324041740892025 | addxy r4, r3, r1 | st r1, [01c4] | ld r1, [0630]
mad r7, r4, #-0.58270937527584066 | addxy r2, r5, r0 | st r5, [01d0] | ld r4, [a1c1]
mad r2, r7, #0.70839720543078366 | ld r5, [01da] | st r0, [01c8]
mac_p1wc3 r0, r5, r1, #-0.20129203364025311, r4 | ld r6, [01d3] | ld r1, [0641]
mul r7, r0, #-4.9679064884762845 | mad r6, r2, #0.45157973770018534 | st r6, [01cc] | ld r0, [a16a]
subxy r1, r1, r0 | mul r3, r6, #0.50948474756359019 | st r3, [01d2] | ld r2, [c051] | ld r5, [01e1]
mul r1, r1, r1 | st r3, [0180] | ld r3, [c092] | ld r6, [a1a2]
ld r1, [c050] | st r0, [0640]
mul r1, r3, r1 | ld r0, [c09a] | st r3, [a122]
mad r1, r0, -r2 | ld r0, [c082] | st r7, [0402]
add r1, #1.0 | st r0, [0634] | st r0, [a172]
st r1, [064a] | ld r1, [c04f]
mul r1, r3, r1 | ld r0, [c08a] | ld r3, [0634] | st r5, [0236]
st r1, [064c] | st r0, [063a]
mul r4, r3, r3 | st r0, [a17a] | ld r1, [c067] | ld r2, [063a]
mul r2, r2, r2 | ld r0, [a1da] | ld r7, [c063] | st r4, [0238]
mad r1, r0, #-0.0625 | ld r0, [01e3] | ld r3, [c064] | ld r4, [01e5]
mac_p1w r3, r3, r5, r0, r7 | ld r0, [c061] | ld r5, [064c] | st r6, [0234]
mad r3, r4, #-0.125 | addxy r1, r1, #0.25 | ld r6, [c062]
mad r4, r3, #0.00059102423256263137 | st r3, [023a] | ld r3, [c060]
st r4, [01e4] | ld r4, [c06b]
mac_p1wc3 r2, r2, r0, #0.015625806566982686, r4 | ld r7, [c06a]
mad r2, r5, #0.12500645253586148 | st r7, [023c]
mac_p1wc3 r0, r6, r1, #-0.24998709559440613, r2
mad r0, r3, #-0.33203125
addxy r4, r0, #-0.0052062496542930603 | addxy r6, r0, #1.0357659163623476
mul r1, r6, r4 | mul r5, r4, r4
addxy r2, r1, #0.35801112009064834 | addxy r6, r0, #0.33854375034570694
mac_p1wc3 r3, r5, r2, #0.039918037988250651, r0
mul r4, r6, #-32.0 | mov r2, r3
sign r1, r4
mad r2, r0, #0.0032823169182762389
min r5, r1, #0.0
addxy r6, r2, #-0.00022491183279744315
mul r4, r6, r5
addxy r2, r4, r3
mad r3, r2, #-0.25 | addxy r4, r5, #1.0
addxy r6, r3, #-0.00015586745360729042
mac_p1wc3 r3, r6, r4, #0.25, r2
mul r2, r0, #4.76837158203125e-07
max r5, r1, #0.0
max r6, r1, -r1
mul r1, r2, #1.7208572168100142 | mul r4, r5, r6
addxy r5, r3, r1
addxy r6, r5, #-0.00033375212964021711
mul r0, r4, r6
subxy r2, r3, r0
shl r1, r2, 11
min r5, r1, #0.051145697081349195
addxy r4, r5, #0.30334452120476779
mov r3, #0.041666666666666664 | mul r6, r4, #-0.82490699372328891
mov r0, r3
ymacd r0, r6, #0.16666666666666666
ymacd r0, r6, #0.5
ymacd r0, r6, #1.0
ymacd r0, r6, #1.0
mul r2, r0, r0
mul r1, r2, r2
mul r5, r1, r1
mul r4, r5, r5
mul r6, r4, #0.00390625
mul r0, r6, r6
mul r2, r0, r0 | mul r1, r0, r6
mac_p1wc3 r4, r6, r6, #-0.0029344578016347305, r2
mul r5, r1, r0 | addxy r4, r4, #-197.87984592782817
mul r5, r5, r4 | mov r4, r6
mad r4, r1, #-0.035383472137614391 | mul r1, r5, #9.5367431640625e-07
mul r5, r1, #-1.9903222525650495
addxy r1, r4, r5 | mul r4, r0, #0.0016586065835326735
addxy r5, r4, #-0.8125
mul r0, r5, r2
mac_p1wc3 r4, r6, r6, #0.0046649689308783868, r0
addxy r5, r4, #-141.3124173314047
mul r0, r5, r2 | mov r2, r1
mul r4, r0, #0.03125
mac_p1wc3 r5, r6, r6, #0.0040062804431907906, r4
mad r2, r5, #-0.23038101196289062
mul r0, r2, #0.46076202392578125
addxy r6, r0, #1.0
recipseed r4, r6
newton r4, r6
newton r4, r6
newton r4, r6
mul r2, r1, r4
mul r0, r2, r2
mul r5, r0, r0
mul r6, r5, r7 | ld r5, [023a]
addxy r1, r6, #0.25679096095835036 | st r3, [023a]
mul r4, r1, #3.8942180685331551
recipseed r0, r4
newton r0, r4
newton r0, r4
newton r0, r4
mul r6, r5, r0 | mul r1, r0, r7 | ld r4, [01e7] | ld r5, [01e9]
mad r4, r5, #0.10000000149011612 | st r4, [01f6]
mul r0, r4, #0.15000000596046448
ph_clamp r5, r3, -r0
mac_p1wc3 r4, r1, r5, #-0.022222221486362097, r6
mul r0, r4, #-5.3999977111816406
ph_clamp r1, r3, -r0 | ld r0, [01eb]
mul r5, r1, r1
mul r6, r5, r1
mad r1, r6, #-0.33333301544189453
mul r4, r1, #2.0 | mov r5, r1
mad r5, r0, #0.5 | addxy r0, r2, #-0.54257943801433939 | st r4, [01ea] | ld r4, [01ed]
mul r6, r5, r2 | mul r5, r0, r4
subxy r5, r5, r6
mul r6, r5, #1.843048095703125
mov r6, r5 | st r6, [01ec]
mad r6, r4, #0.54257943801433939
mul r4, r6, r2 | ld r6, [01ef]
mac_p1wc3 r4, r0, r6, #-0.9215240478515625, r4
mul r3, r4, #-1.843048095703125
mov r3, r4 | st r3, [01ee]
mad r3, r6, #-0.54257943801433939 | ld r6, [01f1]
mul r7, r0, r6
mac_p1wc3 r3, r3, r2, #0.54257943801433939, r7
mul r7, r3, #-3.3968262830749154
mul r7, r1, r2 | st r7, [01f0]
addxy r1, r5, r7
mac_p1wc3 r7, r5, r0, #-0.54257943801433939, r1
mac_p1wc3 r5, r7, r2, #0.58878489311191273, r4
mac_p1wc3 r1, r4, r0, #-0.9215240478515625, r5
mac_p1wc3 r7, r1, r2, #-0.54257943801433939, r3 | ld r1, [01f3]
mul r4, r3, r0 | mad r3, r6, #-0.29439244655595637
addxy r5, r7, r4 | mul r7, r1, r0
mac_p1wc3 r4, r3, r2, #0.58878489311191273, r7 | ld r7, [01f5]
mul r6, r4, #-3.1302571062277735
mac_p1w r1, r4, r0, r5, r2 | st r6, [01f2] | ld r2, [0238]
mul r6, r7, #0.9924468994140625 | mul r5, r2, r2
mul r3, r1, #5.769214398694273 | addxy r0, r6, #0.0075531005859375
addxy r1, r5, #0.41912713505585858 | st r3, [01e6]
mul r3, r1, r2 | st r0, [01f4] | ld r0, [023c]
max r6, r3, #0.011256057508677425
mul r5, r4, r0 | addxy r1, r6, #-0.011256057508677425 | ld r6, [a1aa]
mad r4, r5, #0.5 | addxy r3, r7, #0.0076105840931104994 | ld r5, [01f7] | ld r7, [0236]
mul r2, r4, r1 | ld r1, [c06e] | st r5, [01e8] | ld r5, [023a]
mul r0, r3, r2 | mul r3, r7, r1 | ld r4, [a1e2] | st r6, [023c]
mul r2, r3, #0.34657359027997264 | mov r7, r5 | st r0, [023a] | ld r0, [01fb]
ymacd r7, r2, #0.16666666666666666 | st r4, [0236]
ymacd r7, r2, #0.5
ymacd r7, r2, #1.0
ymacd r7, r2, #1.0
mul r1, r7, r7
mul r2, r1, r1
mul r7, r2, r2
mul r1, r7, r7
mul r2, r1, #0.03125 | mul r1, r3, #-0.34657359027997264
ymacd r5, r1, #0.16666666666666666 | mul r7, r6, r2
ymacd r5, r1, #0.5
ymacd r5, r1, #1.0
ymacd r5, r1, #1.0
mul r2, r5, r5
mul r3, r2, r2
mul r1, r3, r3
mul r5, r1, r1
mul r2, r5, #0.03125 | ld r5, [01f9]
min r1, r7, #0.0077812499366700649
mul r3, r4, r2 | mov r2, r1
mov r4, r1
mad r2, r5, #0.03125 | mad r4, r0, #0.03125
mul r7, r2, #32.0 | mul r0, r4, #32.0
ph_wrap r2, r3, r7
ph_wrap r4, r3, r0
subxy r0, r5, r2
sign r5, r0
mov r7, r1
min r0, r5, #0.0
subxy r5, r4, r2 | mad r7, r2, #0.03125
mul r2, r0, r5 | mul r7, r7, #32.0
subxy r0, r4, r2
mad r1, r0, #-0.03125
mul r5, r1, #32.0
ph_wrap r4, r3, r5
ph_wrap r7, r3, r7
sign r2, r4 | st r7, [01f8]
min r1, r2, #0.0
subxy r5, r4, r7
mul r2, r1, r5 | mul r5, r0, #-1.0
subxy r7, r4, r2
ph_wrap r4, r3, r5
mul r5, r0, #-0.5 | subxy r2, r0, r4
addxy r4, r5, #0.5 | mul r1, r7, #-1.0
ph_tri r0, r3, -r4 | st r1, [01fa]
mul r5, r3, r0
shl r4, r5, 15
shl r0, r4, 2
ph_clamp r5, r3, -r0
mul r4, r5, r5
mul r0, r5, r4
mul r6, r4, r0 | mul r4, r4, r4
mac_p1wc3 r4, r5, r5, #-0.034104689076754152, r4
addxy r4, r4, #-17.026106186230564
mul r6, r6, r4
mul r4, r6, #0.03125
mad r5, r4, #-0.095352512591594205
mad r5, r0, #-0.41123178361504448
mul r6, r2, r5
mul r4, r6, #0.78539800643920898
ph_wrap r0, r3, r1 | st r4, [0202]
mul r1, r7, #-0.5 | subxy r4, r0, r7
addxy r0, r1, #0.5
ph_tri r7, r3, -r0
mul r1, r3, r7
shl r0, r1, 15
shl r3, r0, 2
ph_clamp r7, r3, -r3
mul r1, r7, r7
mul r0, r7, r1
mul r3, r1, r0 | mul r1, r1, r1
mac_p1wc3 r1, r7, r7, #-0.034104689076754152, r1
addxy r1, r1, #-17.026106186230564
mul r3, r3, r1
mul r1, r3, #0.03125
mad r7, r1, #-0.095352512591594205
mad r7, r0, #-0.41123178361504448 | ld r0, [01fd]
mul r3, r4, r7 | mov r7, r0 | ld r4, [01ff]
mul r1, r3, #-0.78539800643920898 | mad r7, r4, #-0.50126103797649757
st r1, [0200] | ld r1, [0201]
mov r4, r1
mad r4, r6, #0.39269900321960449 | ld r6, [0203]
mad r4, r6, #0.5
mad r7, r4, #0.034666201629152259
mul r6, r7, #1.5688642263412476 | mov r4, r7
st r6, [01fe] | st r6, [020c]
mac_p1wc3 r6, r2, r5, #-0.5, r3 | ld r2, [0205] | ld r5, [0207]
mad r4, r0, #-0.31950568415055886 | mad r6, r1, #0.63661989959316345 | ld r3, [0209] | ld r1, [020b]
mad r4, r6, #0.017354443547897992 | mad r7, r5, #0.63740378753622462 | st r3, [0210]
mul r0, r4, #2.4613349606933212 | mad r4, r2, #0.40628358836552458 | ld r2, [020f] | st r1, [0204]
st r0, [01fc] | st r0, [0208] | ld r0, [020d]
mad r7, r4, #0.75290865113054539 | addxy r6, r3, r1 | ld r4, [0211] | ld r3, [0213]
mad r7, r6, #-0.10523075837658179 | addxy r5, r0, r2 | ld r6, [0215] | st r0, [0214]
mad r7, r5, #-0.59394325412832727 | addxy r1, r4, r3 | st r6, [020e] | ld r5, [c065]
mad r1, r7, #0.69499853757836605 | st r3, [020a] | ld r3, [c069]
mad r6, r1, #0.45157973770018534 | st r2, [0206] | ld r7, [c066]
mul r0, r6, #0.48420281195382131 | mul r1, r3, r7 | ld r2, [c068] | st r4, [0212] | ld r7, [0236]
mul r4, r2, r5 | st r0, [01e2] | ld r6, [c06c] | ld r0, [023c]
mul r2, r0, r6 | ld r3, [c06d]
mul r0, r7, r3 | mul r5, r4, r2 | ld r7, [0234]
mul r3, r7, #0.5400000071525568 | mul r6, r1, r0
min r4, r5, #0.00048632812104187906 | st r6, [0236] | ld r6, [0217]
addxy r1, r3, #0.51499999966472387 | addxy r0, r7, #-0.89814813687263062
shl r2, r4, 11
recipseed r4, r1
newton r4, r1
newton r4, r1
newton r4, r1
mul r1, r7, #-1.0 | mul r3, r0, #-1.0800000143051136 | st r4, [023c]
addxy r0, r1, #0.89814813687263062 | mad r6, r2, #0.25
sign r5, r3
max r3, r0, -r0 | st r5, [023e]
max r1, r3, #0.0018084490501203063
mul r0, r1, #34.560000457763635
recipseed r3, r0
newton r3, r0
newton r3, r0
newton r3, r0
ph_wrap r4, r3, r6
mov r0, r4 | mul r1, r5, r3
mad r0, r2, #0.25 | st r1, [0238]
ph_wrap r6, r3, r0 | ld r0, [0219]
mad r0, r2, #0.25 | st r6, [0216]
ph_wrap r0, r3, r0
sign r1, r0
min r1, r1, #0.0
subxy r5, r0, r4
mul r1, r1, r5
subxy r5, r0, r1
mov r0, r5
mad r0, r2, #-0.25
mul r1, r0, #-1.0
ph_wrap r2, r3, r1
sign r0, r2
absadd r1, r0, #-1.0
mul r0, r1, r2
addxy r1, r0, r2 | subxy r0, r4, r6
sign r2, r0
min r4, r2, #0.0
subxy r0, r1, r6
mul r2, r4, r0 | mov r0, r7
mad r0, r5, #0.92592591366159682 | subxy r6, r1, r2
addxy r1, r0, #0.027777776788966198 | mul r4, r6, #-1.0 | ld r0, [023c]
mad r7, r6, #0.92592591366159682 | mul r2, r1, #-1.0800000143051136 | st r4, [0218] | ld r6, [023e]
sign r4, r2
mac_p1wc3 r2, r6, r3, #0.015625, r0 | ld r3, [0238]
min r5, r4, #0.0
mul r6, r2, #0.03125
mac_p1wc3 r0, r5, r6, #0.03125, r3
mul r2, r1, r0
shl r5, r2, 10 | ld r2, [0236]
mul r1, r5, #-1.0800000143051136
ph_tri r0, r3, -r1
mul r5, r2, r0
shl r1, r5, 15
shl r0, r1, 5
mul r5, r0, #1.6000000238418579 | addxy r0, r7, #0.027777776788966198
ph_clamp r1, r3, -r5
mul r5, r0, #-1.0800000143051136
sign r7, r5
min r5, r7, #0.0
mac_p1wc3 r5, r5, r6, #0.03125, r3
mul r6, r0, r5
shl r3, r6, 10
mul r0, r3, #-1.0800000143051136
ph_tri r5, r3, -r0
mul r6, r2, r5
shl r3, r6, 15
shl r0, r3, 5
mul r2, r0, #1.6000000238418579 | mul r6, r1, r4
ph_clamp r5, r3, -r2
mul r3, r6, #-1.0 | mul r0, r5, r7 | ld r5, [021d]
mul r2, r0, #-1.0 | st r3, [0220] | ld r3, [021b]
mov r7, r3 | st r2, [021e] | ld r2, [021f]
mad r7, r5, #-0.4735108174809789
mov r5, r2
mad r5, r6, #-0.5 | ld r6, [0221]
mad r5, r6, #0.5
mad r7, r5, #0.02471467541816923
mul r6, r7, #1.5991098880767822 | mov r5, r7
st r6, [021c] | st r6, [022a]
mac_p1wc3 r6, r1, r4, #0.5, r0 | ld r1, [0223] | ld r4, [0225]
mad r5, r3, #-0.29610899226597925 | mad r6, r2, #-0.5 | ld r0, [0227] | ld r2, [0229]
mad r5, r6, #-0.015455270211538169 | mad r7, r4, #0.62534789351010778 | st r0, [022e]
mul r3, r5, #2.557152434144939 | mad r5, r1, #0.39105998791752911 | ld r1, [022d] | st r2, [0222]
st r3, [021a] | st r3, [0226] | ld r3, [022b]
mad r7, r5, #0.76742375065127288 | addxy r6, r0, r2 | ld r5, [022f] | ld r0, [0231]
mad r7, r6, #-0.10324041740892025 | addxy r4, r3, r1 | st r1, [0224] | ld r6, [0233]
mad r7, r4, #-0.58270937527584066 | addxy r2, r5, r0 | st r5, [0230] | ld r1, [064a]
mad r2, r7, #0.70839720543078366 | ld r4, [a1c2] | ld r5, [023a]
st r0, [0228] | st r6, [022c]
mad r6, r2, #0.45157973770018534 | st r3, [0232]
mac_p1wc3 r0, r5, r1, #-0.20129203364025311, r4
mul r3, r6, #0.50948474756359019 | mul r7, r0, #-4.9679064884762845
st r3, [01e0] | st r7, [0404]
