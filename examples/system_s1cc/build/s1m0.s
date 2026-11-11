ld r0, [a060] | ld r1, [0301] | ld r5, [0181]
subxy r1, r1, r0 | ld r2, [a038] | st r0, [0300] | ld r3, [c049]
mul r1, r1, r1 | st r2, [a160] | ld r2, [0305] | ld r7, [c073]
mad r2, r2, -r1 | st r0, [a168] | ld r0, [0303] | st r5, [01d6]
addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0304]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c059]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a120] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [0302]
addxy r1, r1, r2 | ld r2, [a030]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [0306]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a040]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [0308] | ld r1, [030d]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a048]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.0
st r2, [030a] | ld r2, [a040]
addxy r2, r1, r2 | st r0, [0310] | ld r0, [0306]
ph_wrap r1, r3, r2 | ld r2, [c058]
mul r1, r1, #256.0 | st r1, [030c]
ph_clamp r1, r3, -r1
st r1, [a130]
ld r1, [a120]
mad r2, r1, r3 | ld r1, [0302] | ld r3, [c05a]
mad r2, r1, r3 | ld r1, [a178] | ld r3, [c05b]
mad r2, r1, r3 | ld r1, [a170] | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [a130] | ld r3, [c05d]
mad r2, r1, r3 | ld r1, [c077] | ld r3, [a1b0]
mad r1, r0, #-0.0625 | mul r4, r3, r3 | ld r0, [0183] | ld r3, [c074] | st r2, [030e]
mac_p1w r3, r3, r5, r0, r7 | st r4, [01d8] | ld r4, [0185] | ld r0, [c071]
mad r3, r4, #-0.125 | addxy r1, r1, #0.25 | ld r2, [a1b8] | ld r6, [030e]
mad r4, r3, #0.00059102423256263137 | mul r2, r2, r2 | ld r5, [a1d0] | st r6, [01d4]
st r4, [0184] | ld r4, [c07b]
mac_p1wc3 r2, r2, r0, #0.015625806566982686, r4 | ld r6, [c072] | st r3, [01da]
mad r2, r5, #0.12500645253586148 | ld r3, [c070]
mac_p1wc3 r0, r6, r1, #-0.24998709559440613, r2 | ld r7, [c07a]
mad r0, r3, #-0.33203125 | st r7, [01dc]
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
mul r5, r4, r0 | addxy r1, r6, #-0.011256057508677425 | ld r6, [0308]
mad r4, r5, #0.5 | addxy r3, r7, #0.0076105840931104994 | ld r5, [0197] | ld r7, [01d6]
mul r2, r4, r1 | ld r1, [c07e] | st r5, [0188] | ld r5, [01da]
mul r0, r3, r2 | mul r3, r7, r1 | ld r4, [030a] | st r6, [01dc]
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
mad r7, r5, #-0.59394325412832727 | addxy r1, r4, r3 | st r6, [01ae] | ld r5, [c075]
mad r1, r7, #0.69499853757836605 | st r3, [01aa] | ld r3, [c079]
mad r6, r1, #0.45157973770018534 | st r2, [01a6] | ld r7, [c076]
mul r0, r6, #0.48420281195382131 | mul r1, r3, r7 | ld r2, [c078] | st r4, [01b2] | ld r7, [01d6]
mul r4, r2, r5 | st r0, [0182] | ld r6, [c07c] | ld r0, [01dc]
mul r2, r0, r6 | ld r3, [c07d]
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
mad r7, r6, #-0.10324041740892025 | addxy r4, r3, r1 | st r1, [01c4] | ld r1, [a1c8]
mad r7, r4, #-0.58270937527584066 | addxy r2, r5, r0 | st r5, [01d0] | ld r4, [0310]
mad r2, r7, #0.70839720543078366 | ld r5, [01da] | st r0, [01c8]
mac_p1wc3 r0, r5, r1, #-0.20129203364025311, r4 | ld r6, [01d3] | ld r1, [0313]
mul r7, r0, #-4.9679064884762845 | mad r6, r2, #0.45157973770018534 | st r6, [01cc] | ld r0, [a061]
subxy r1, r1, r0 | ld r2, [a039] | st r0, [0312]
mul r1, r1, r1 | mul r3, r6, #0.50948474756359019 | st r2, [a161] | ld r2, [0317] | st r3, [01d2]
mad r2, r2, -r1 | st r0, [a169] | ld r0, [0315] | st r3, [0180]
addxy r1, r0, #-0.999 | ld r3, [c049] | st r7, [a151]
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0316]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c059]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a121] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [0314]
addxy r1, r1, r2 | ld r2, [a031]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [0318]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a041]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [031a] | ld r1, [031f]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a049]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.0
st r2, [031c] | ld r2, [a041]
addxy r2, r1, r2 | st r0, [0322]
ph_wrap r1, r3, r2 | ld r2, [c058]
mul r1, r1, #256.0 | st r1, [031e]
ph_clamp r1, r3, -r1
st r1, [a131]
ld r1, [a121]
mad r2, r1, r3 | ld r1, [0314] | ld r3, [c05a]
mad r2, r1, r3 | ld r1, [a179] | ld r3, [c05b]
mad r2, r1, r3 | ld r1, [a171] | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [a131] | ld r3, [c05d]
mad r2, r1, r3 | ld r1, [031a] | ld r3, [0318]
st r2, [0320] | ld r2, [0322] | st r1, [a1a9]
ld r0, [0320] | st r2, [a1c1]
st r0, [a1a1] | ld r0, [031c]
st r0, [a1e1] | ld r1, [0325]
ld r0, [a062]
subxy r1, r1, r0 | ld r2, [a03a] | st r0, [0324]
mul r1, r1, r1 | st r2, [a162] | ld r2, [0329]
mad r2, r2, -r1 | st r0, [a16a] | ld r0, [0327]
addxy r1, r0, #-0.999 | st r3, [a1d9] | ld r3, [c049]
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0328]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c059]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a122] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f] | st r0, [0326]
addxy r1, r1, r2 | ld r2, [a032]
addxy r2, r2, r1
mul r2, r1, #0.08664339756999316 | st r2, [032a]
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a042]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [032c] | ld r1, [0331]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a04a]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.0
st r2, [032e] | ld r2, [a042]
addxy r2, r1, r2 | st r0, [0334]
ph_wrap r1, r3, r2 | ld r2, [c058]
mul r1, r1, #256.0 | st r1, [0330]
ph_clamp r1, r3, -r1
st r1, [a132]
ld r1, [a122]
mad r2, r1, r3 | ld r1, [0326] | ld r3, [c05a]
mad r2, r1, r3 | ld r1, [a17a] | ld r3, [c05b]
mad r2, r1, r3 | ld r1, [a172] | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [a132] | ld r3, [c05d]
mad r2, r1, r3 | ld r1, [032c] | ld r3, [032a]
st r2, [0332] | ld r2, [0334] | st r1, [a1aa]
ld r0, [0332] | st r2, [a1c2]
st r0, [a1a2] | ld r0, [032e]
st r3, [a1da]
st r0, [a1e2]
