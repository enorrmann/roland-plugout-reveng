ld r0, [a060] | ld r1, [0301] | ld r4, [0183]
subxy r1, r1, r0 | ld r2, [a038] | st r0, [0300] | ld r3, [c049]
mul r1, r1, r1 | st r2, [a160] | ld r2, [0305]
mad r2, r2, -r1 | st r0, [a168] | ld r0, [0303]
addxy r1, r0, #-0.999 | ld r6, [0185]
mul r1, r1, #256.0 | st r6, [01da]
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0304]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c059]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [0307] | ld r2, [a040]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c058]
mul r1, r1, #256.0 | st r1, [0306]
ph_clamp r1, r3, -r1 | min r0, r0, #1.0
mov r0, #0.0 | st r1, [a130] | st r0, [0302]
ld r1, [a120] | st r0, [030a]
mad r2, r1, r3 | ld r1, [0302] | ld r3, [c05a]
mad r2, r1, r3 | ld r1, [a178] | ld r3, [c05b]
mad r2, r1, r3 | ld r1, [a170] | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [a130] | ld r3, [c05d]
mad r2, r1, r3 | ld r0, [a1b8] | ld r1, [0181]
subxy r2, r1, r0 | st r2, [0308] | ld r3, [a1b0] | ld r0, [c071]
mad r1, r2, #0.010400000028312206 | subxy r5, r4, r3 | ld r2, [01a5] | ld r3, [c072] | ld r7, [0308]
mad r4, r5, #0.010400000028312206 | st r7, [01d8]
mac_p1w r5, r3, r6, r2, r0 | ld r0, [01c5] | ld r3, [c073] | st r4, [0182]
mov r2, r0 | st r4, [01dc]
mad r2, r5, #-0.5 | ld r5, [c070] | st r3, [01de]
mul r5, r5, r2 | st r1, [0180]
mad r0, r5, #-0.25 | mul r5, r1, r1
mul r0, r5, r5 | st r0, [01c4]
mul r5, r0, r3
addxy r0, r5, #0.18518526367693175
mul r5, r0, #2.6999988555908203
recipseed r6, r5
newton r6, r5
newton r6, r5
newton r6, r5
mul r0, r2, r6 | mul r7, r6, r3 | ld r5, [01c7] | ld r2, [01c9]
mad r5, r2, #0.10000000149011612 | st r5, [01d6]
mul r6, r5, #0.075000002980232239
ph_clamp r2, r3, -r6
mac_p1wc3 r5, r7, r2, #0.044444442972724194, r0
mul r6, r5, #-2.6999988555908203
ph_clamp r7, r3, -r6
mul r2, r7, r7
mul r0, r2, r7
max r5, r0, #-0.023437522351763107 | ld r0, [01cb]
min r6, r5, #0.011718761175881554
mad r7, r6, #-0.33333301544189453
mul r2, r7, #2.0 | mov r5, r7
mad r5, r0, #0.5 | addxy r0, r1, #-0.5 | st r2, [01ca] | ld r2, [01cd]
mul r6, r5, r1 | mul r5, r2, r0
subxy r5, r5, r6
mul r6, r5, #2.0
mov r6, r5 | st r6, [01cc]
mad r6, r2, #0.5 | ld r2, [01cf]
mul r4, r2, r0
mac_p1wc3 r6, r6, r1, #-0.5, r4
mul r4, r6, #4.0
mov r4, r6 | st r4, [01ce]
mad r4, r2, #0.25 | ld r2, [01d1]
mul r3, r2, r0
mac_p1wc3 r4, r4, r1, #-0.5, r3
mul r3, r4, #4.0
mul r3, r7, r1 | st r3, [01d0]
addxy r7, r5, r3
mac_p1wc3 r3, r5, r0, #-0.5, r7
mac_p1wc3 r5, r3, r1, #-0.5, r6
mul r7, r6, r0
addxy r3, r5, r7 | mul r5, r4, r0
mac_p1wc3 r6, r3, r1, #-0.5, r4 | ld r3, [01d3]
addxy r7, r6, r5 | mad r4, r2, #0.25
mul r6, r3, r0
mac_p1wc3 r5, r4, r1, #-0.5, r6 | ld r6, [01d5]
mac_p1w r3, r5, r0, r7, r1 | ld r7, [01de]
mul r2, r5, #4.0 | mul r4, r3, #-16.0
mul r3, r7, r5 | st r2, [01d2] | ld r7, [a040]
max r2, r6, #-0.0076105840931104994 | st r4, [01c6] | ld r4, [01dc]
mad r5, r3, #0.5 | mul r0, r2, #0.9924468994140625 | ld r3, [a048]
mul r6, r5, r4 | addxy r1, r0, #0.0075531005859375 | ld r5, [c076] | ld r4, [01da]
addxy r0, r2, #0.0076105840931104994 | mul r2, r4, r5 | st r6, [01da]
mov r5, #0.041666666666666664 | mul r4, r2, #0.34657359027997264 | st r1, [01d4] | ld r1, [01d7]
mov r6, r5 | st r1, [01c8] | ld r1, [a030]
ymacd r6, r4, #0.16666666666666666 | st r0, [01dc]
ymacd r6, r4, #0.5
ymacd r6, r4, #1.0
ymacd r6, r4, #1.0
mul r4, r6, r6 | mov r0, r5
mul r6, r4, r4
mul r4, r6, r6
mul r6, r4, r4
mul r4, r6, #0.03125
mul r6, r7, r4 | mul r4, r2, #-0.34657359027997264
mov r2, r5
ymacd r2, r4, #0.16666666666666666
ymacd r2, r4, #0.5
ymacd r2, r4, #1.0
ymacd r2, r4, #1.0
mul r4, r2, r2
mul r2, r4, r4
mul r4, r2, r2
mul r2, r4, r4
mul r4, r2, #0.03125
mul r2, r3, r4
max r4, r2, #3.6765432014362887e-06
min r2, r4, #0.0625 | ld r4, [c074]
mul r7, r7, r4 | addxy r4, r1, #-4.75
mul r4, r4, #0.04332169878499658
ymacd r0, r4, #0.16666666666666666
ymacd r0, r4, #0.5
ymacd r0, r4, #1.0
ymacd r0, r4, #1.0
mul r4, r0, r0
mul r0, r4, r4
mul r4, r0, r0
mul r0, r4, r4
mul r4, r0, #0.03125
mul r0, r7, r4 | ld r7, [c075]
mul r4, r3, r7 | mul r3, r1, #-1.0 | st r0, [01e0]
addxy r7, r3, #4.75
mul r1, r7, #0.04332169878499658
ymacd r5, r1, #0.16666666666666666
ymacd r5, r1, #0.5
ymacd r5, r1, #1.0
ymacd r5, r1, #1.0
mul r3, r5, r5
mul r7, r3, r3
mul r1, r7, r7
mul r5, r1, r1
mul r3, r5, #0.015625
mul r7, r4, r3 | ld r3, [0187]
min r4, r6, #0.0077812499366700649 | st r7, [01de]
max r1, r2, #3.6765432014362887e-06
mov r2, r4 | mov r7, r4
mad r2, r3, #0.03125
min r5, r1, #0.0625
mul r1, r2, #32.0 | mov r2, r4
ph_wrap r6, r3, r1
mad r2, r6, #0.03125
mul r1, r2, #32.0
ph_wrap r2, r3, r1 | ld r1, [0189]
mad r7, r1, #0.03125 | st r2, [0186]
mul r1, r7, #32.0
ph_wrap r7, r3, r1
subxy r1, r3, r6
sign r3, r1
min r1, r3, #0.0
subxy r3, r7, r6
mul r6, r1, r3
subxy r1, r7, r6
mad r4, r1, #-0.03125
mul r3, r4, #32.0
ph_wrap r7, r3, r3
sign r6, r7
min r4, r6, #0.0
subxy r3, r7, r2
mul r6, r4, r3 | mul r3, r1, #-1.0
subxy r2, r7, r6
ph_wrap r7, r3, r3
mul r3, r1, #-0.5 | subxy r6, r1, r7
addxy r7, r3, #0.5 | mul r4, r2, #-1.0
ph_tri r1, r3, -r7 | st r4, [0188]
mul r3, r5, r1
shl r7, r3, 15
shl r1, r7, 2
ph_clamp r3, r3, -r1
mul r7, r3, r3
mul r1, r3, r7
mul r0, r7, r1 | mul r7, r7, r7
mac_p1wc3 r7, r3, r3, #-0.034104689076754152, r7
addxy r7, r7, #-17.026106186230564
mul r0, r0, r7
mul r7, r0, #0.03125
mad r3, r7, #-0.095352512591594205
mad r3, r1, #-0.41123178361504448
mul r0, r6, r3
mul r7, r0, #0.78539800643920898
ph_wrap r1, r3, r4 | st r7, [018c]
mul r4, r2, #-0.5 | subxy r7, r1, r2
addxy r1, r4, #0.5
ph_tri r2, r3, -r1
mul r4, r5, r2
shl r1, r4, 15
shl r5, r1, 2
ph_clamp r2, r3, -r5
mul r4, r2, r2
mul r1, r2, r4
mul r5, r4, r1 | mul r4, r4, r4
mac_p1wc3 r4, r2, r2, #-0.034104689076754152, r4
addxy r4, r4, #-17.026106186230564
mul r5, r5, r4
mul r4, r5, #0.03125
mad r2, r4, #-0.095352512591594205
mad r2, r1, #-0.41123178361504448 | ld r1, [018f]
mul r5, r7, r2 | mov r2, r1 | ld r7, [0191]
mul r4, r5, #-0.78539800643920898 | mad r2, r7, #-0.50126103797649757
st r4, [018a] | ld r4, [018b]
mov r7, r4
mad r7, r0, #0.39269900321960449 | ld r0, [018d]
mad r7, r0, #0.5
mad r2, r7, #0.034666201629152259
mul r0, r2, #1.5688642263412476 | mov r7, r2
st r0, [0190] | st r0, [019c]
mac_p1wc3 r0, r6, r3, #-0.5, r5 | ld r6, [019b] | ld r3, [01a3]
mad r7, r1, #-0.31950568415055886 | mad r0, r4, #0.63661989959316345 | ld r5, [0193] | ld r4, [0199]
mad r7, r0, #0.017354443547897992 | mad r2, r3, #0.63740378753622462 | st r5, [0194]
mul r1, r7, #2.4613349606933212 | mad r7, r6, #0.40628358836552458 | ld r6, [01a1] | st r4, [019a]
st r1, [018e] | st r1, [0192] | ld r1, [019d]
mad r2, r7, #0.75290865113054539 | addxy r0, r5, r4 | ld r7, [0195] | ld r5, [0197]
mad r2, r0, #-0.10523075837658179 | addxy r3, r1, r6 | ld r0, [019f] | st r5, [0198]
mad r2, r3, #-0.59394325412832727 | addxy r4, r7, r5 | st r0, [01a0] | ld r5, [01d8]
mad r4, r2, #0.69499853757836605 | mul r2, r5, #0.5400000071525568 | st r1, [019e]
mad r0, r4, #0.45157973770018534 | addxy r4, r2, #0.51499999966472387 | st r6, [01a2] | ld r6, [01e0]
mul r1, r0, #0.48420281195382131 | st r7, [0196]
recipseed r0, r4 | st r1, [01a4]
newton r0, r4
newton r0, r4
newton r0, r4
min r3, r6, #0.0038906249683350325 | st r0, [01e2]
mul r1, r5, #-1.0 | addxy r6, r5, #-0.89814813687263062
mul r7, r3, #256.0 | addxy r4, r1, #0.89814813687263062
mul r3, r6, #-1.0800000143051136
max r6, r4, -r4
sign r2, r3
max r3, r6, #0.0018084490501203063 | st r2, [01e4]
mul r1, r3, #34.560000457763635
recipseed r3, r1
newton r3, r1
newton r3, r1
newton r3, r1
mul r6, r2, r3 | ld r4, [01a7]
mad r4, r7, #0.25 | st r6, [01e0] | ld r6, [01a9]
mad r6, r7, #0.25
ph_wrap r6, r3, r6
ph_wrap r1, r3, r4
sign r0, r6
min r0, r0, #0.0
subxy r2, r6, r1
mul r0, r0, r2 | mov r4, r1
subxy r2, r6, r0 | mad r4, r7, #0.25
mov r6, r2
mad r6, r7, #-0.25
mul r0, r6, #-1.0
ph_wrap r4, r3, r4
ph_wrap r7, r3, r0 | st r4, [01a6]
sign r6, r7
absadd r0, r6, #-1.0
mul r6, r0, r7
addxy r0, r6, r7 | subxy r6, r1, r4
sign r7, r6
min r1, r7, #0.0
subxy r6, r0, r4
mul r7, r1, r6 | mov r6, r5
mad r6, r2, #0.92592591366159682 | subxy r4, r0, r7
addxy r0, r6, #0.027777776788966198 | mul r1, r4, #-1.0 | ld r6, [01e2]
mad r5, r4, #0.92592591366159682 | mul r7, r0, #-1.0800000143051136 | st r1, [01a8] | ld r4, [01e4]
sign r1, r7
mac_p1wc3 r7, r4, r3, #0.015625, r6 | ld r3, [01e0]
min r2, r1, #0.0
mul r4, r7, #0.03125
mac_p1wc3 r6, r2, r4, #0.03125, r3
max r7, r6, #-0.00094811886083334684
min r2, r7, #0.0625
mul r6, r0, r2
shl r7, r6, 10 | ld r6, [01de]
mul r0, r7, #-1.0800000143051136
ph_tri r2, r3, -r0
mul r7, r6, r2
shl r0, r7, 15
shl r2, r0, 3
mul r7, r2, #1.6000000238418579 | addxy r2, r5, #0.027777776788966198
ph_clamp r0, r3, -r7
mul r7, r2, #-1.0800000143051136
sign r5, r7
min r7, r5, #0.0
mac_p1wc3 r7, r7, r4, #0.03125, r3
mul r4, r2, r7
shl r3, r4, 10
mul r2, r3, #-1.0800000143051136
ph_tri r7, r3, -r2
mul r4, r6, r7
shl r3, r4, 15
shl r2, r3, 3
mul r6, r2, #1.6000000238418579 | mul r4, r0, r1
ph_clamp r7, r3, -r6
mul r3, r4, #-1.0 | mul r2, r7, r5 | ld r7, [01b1]
mul r6, r2, #-1.0 | st r3, [01ac] | ld r3, [01af]
mov r5, r3 | st r6, [01aa] | ld r6, [01ab]
mad r5, r7, #-0.4735108174809789
mov r7, r6
mad r7, r4, #-0.5 | ld r4, [01ad]
mad r7, r4, #0.5
mad r5, r7, #0.02471467541816923
mul r4, r5, #1.5991098880767822 | mov r7, r5
st r4, [01b0] | st r4, [01bc]
mac_p1wc3 r4, r0, r1, #0.5, r2 | ld r0, [01bb] | ld r1, [01c3]
mad r7, r3, #-0.29610899226597925 | mad r4, r6, #-0.5 | ld r2, [01b3] | ld r6, [01b9]
mad r7, r4, #-0.015455270211538169 | mad r5, r1, #0.62534789351010778 | st r2, [01b4]
mul r3, r7, #2.557152434144939 | mad r7, r0, #0.39105998791752911 | ld r0, [01c1] | st r6, [01ba]
st r3, [01ae] | st r3, [01b2] | ld r3, [01bd]
mad r5, r7, #0.76742375065127288 | addxy r4, r2, r6 | ld r7, [01b5] | ld r2, [01b7]
mad r5, r4, #-0.10324041740892025 | addxy r1, r3, r0 | st r0, [01c2] | ld r0, [030a]
mad r5, r1, #-0.58270937527584066 | addxy r6, r7, r2 | st r7, [01b6] | ld r1, [01dc]
mad r6, r5, #0.70839720543078366 | ld r7, [01da] | st r2, [01b8]
mac_p1wc3 r2, r1, r7, #0.069972968350592024, r0 | ld r0, [a061] | ld r1, [030d]
mul r5, r2, #14.291233080031816 | subxy r1, r1, r0 | ld r2, [a039] | st r0, [030c]
mul r1, r1, r1 | st r2, [a161] | ld r2, [0311] | st r3, [01be]
mad r2, r2, -r1 | st r0, [a169] | ld r0, [030f]
addxy r1, r0, #-0.999 | ld r4, [01bf] | st r5, [a151]
mul r1, r1, #256.0 | mad r4, r6, #0.45157973770018534 | st r4, [01c0] | ld r6, [01eb]
ph_clamp r1, r3, -r1 | st r6, [0240]
mul r1, r1, #256.0 | mul r3, r4, #0.50948474756359019 | ld r4, [01e9]
ph_clamp r1, r3, -r1 | st r3, [0184] | ld r3, [c049]
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0310]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c059]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [0313] | ld r2, [a041]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c058]
mul r1, r1, #256.0 | st r1, [0312]
ph_clamp r1, r3, -r1 | min r0, r0, #1.0
mov r0, #0.0 | st r1, [a131] | st r0, [030e]
ld r1, [a121] | st r0, [0316]
mad r2, r1, r3 | ld r1, [030e] | ld r3, [c05a]
mad r2, r1, r3 | ld r1, [a179] | ld r3, [c05b]
mad r2, r1, r3 | ld r1, [a171] | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [a131] | ld r3, [c05d]
mad r2, r1, r3 | ld r0, [a1b9] | ld r1, [01e7]
subxy r2, r1, r0 | st r2, [0314] | ld r3, [a1b1] | ld r0, [c071]
mad r1, r2, #0.010400000028312206 | subxy r5, r4, r3 | ld r2, [020b] | ld r3, [c072] | ld r7, [0314]
mad r4, r5, #0.010400000028312206 | st r7, [023e]
mac_p1w r5, r3, r6, r2, r0 | ld r0, [022b] | ld r3, [c073] | st r4, [01e8]
mov r2, r0 | st r4, [0242]
mad r2, r5, #-0.5 | ld r5, [c070] | st r3, [0244]
mul r5, r5, r2 | st r1, [01e6]
mad r0, r5, #-0.25 | mul r5, r1, r1
mul r0, r5, r5 | st r0, [022a]
mul r5, r0, r3
addxy r0, r5, #0.18518526367693175
mul r5, r0, #2.6999988555908203
recipseed r6, r5
newton r6, r5
newton r6, r5
newton r6, r5
mul r0, r2, r6 | mul r7, r6, r3 | ld r5, [022d] | ld r2, [022f]
mad r5, r2, #0.10000000149011612 | st r5, [023c]
mul r6, r5, #0.075000002980232239
ph_clamp r2, r3, -r6
mac_p1wc3 r5, r7, r2, #0.044444442972724194, r0
mul r6, r5, #-2.6999988555908203
ph_clamp r7, r3, -r6
mul r2, r7, r7
mul r0, r2, r7
max r5, r0, #-0.023437522351763107 | ld r0, [0231]
min r6, r5, #0.011718761175881554
mad r7, r6, #-0.33333301544189453
mul r2, r7, #2.0 | mov r5, r7
mad r5, r0, #0.5 | addxy r0, r1, #-0.5 | st r2, [0230] | ld r2, [0233]
mul r6, r5, r1 | mul r5, r2, r0
subxy r5, r5, r6
mul r6, r5, #2.0
mov r6, r5 | st r6, [0232]
mad r6, r2, #0.5 | ld r2, [0235]
mul r4, r2, r0
mac_p1wc3 r6, r6, r1, #-0.5, r4
mul r4, r6, #4.0
mov r4, r6 | st r4, [0234]
mad r4, r2, #0.25 | ld r2, [0237]
mul r3, r2, r0
mac_p1wc3 r4, r4, r1, #-0.5, r3
mul r3, r4, #4.0
mul r3, r7, r1 | st r3, [0236]
addxy r7, r5, r3
mac_p1wc3 r3, r5, r0, #-0.5, r7
mac_p1wc3 r5, r3, r1, #-0.5, r6
mul r7, r6, r0
addxy r3, r5, r7 | mul r5, r4, r0
mac_p1wc3 r6, r3, r1, #-0.5, r4 | ld r3, [0239]
addxy r7, r6, r5 | mad r4, r2, #0.25
mul r6, r3, r0
mac_p1wc3 r5, r4, r1, #-0.5, r6 | ld r6, [023b]
mac_p1w r3, r5, r0, r7, r1 | ld r7, [0244]
mul r2, r5, #4.0 | mul r4, r3, #-16.0
mul r3, r7, r5 | st r2, [0238] | ld r7, [a041]
max r2, r6, #-0.0076105840931104994 | st r4, [022c] | ld r4, [0242]
mad r5, r3, #0.5 | mul r0, r2, #0.9924468994140625 | ld r3, [a049]
mul r6, r5, r4 | addxy r1, r0, #0.0075531005859375 | ld r5, [c076] | ld r4, [0240]
addxy r0, r2, #0.0076105840931104994 | mul r2, r4, r5 | st r6, [0240]
mov r5, #0.041666666666666664 | mul r4, r2, #0.34657359027997264 | st r1, [023a] | ld r1, [023d]
mov r6, r5 | st r1, [022e] | ld r1, [a031]
ymacd r6, r4, #0.16666666666666666 | st r0, [0242]
ymacd r6, r4, #0.5
ymacd r6, r4, #1.0
ymacd r6, r4, #1.0
mul r4, r6, r6 | mov r0, r5
mul r6, r4, r4
mul r4, r6, r6
mul r6, r4, r4
mul r4, r6, #0.03125
mul r6, r7, r4 | mul r4, r2, #-0.34657359027997264
mov r2, r5
ymacd r2, r4, #0.16666666666666666
ymacd r2, r4, #0.5
ymacd r2, r4, #1.0
ymacd r2, r4, #1.0
mul r4, r2, r2
mul r2, r4, r4
mul r4, r2, r2
mul r2, r4, r4
mul r4, r2, #0.03125
mul r2, r3, r4
max r4, r2, #3.6765432014362887e-06
min r2, r4, #0.0625 | ld r4, [c074]
mul r7, r7, r4 | addxy r4, r1, #-4.75
mul r4, r4, #0.04332169878499658
ymacd r0, r4, #0.16666666666666666
ymacd r0, r4, #0.5
ymacd r0, r4, #1.0
ymacd r0, r4, #1.0
mul r4, r0, r0
mul r0, r4, r4
mul r4, r0, r0
mul r0, r4, r4
mul r4, r0, #0.03125
mul r0, r7, r4 | ld r7, [c075]
mul r4, r3, r7 | mul r3, r1, #-1.0 | st r0, [0246]
addxy r7, r3, #4.75
mul r1, r7, #0.04332169878499658
ymacd r5, r1, #0.16666666666666666
ymacd r5, r1, #0.5
ymacd r5, r1, #1.0
ymacd r5, r1, #1.0
mul r3, r5, r5
mul r7, r3, r3
mul r1, r7, r7
mul r5, r1, r1
mul r3, r5, #0.015625
mul r7, r4, r3 | ld r3, [01ed]
min r4, r6, #0.0077812499366700649 | st r7, [0244]
max r1, r2, #3.6765432014362887e-06
mov r2, r4 | mov r7, r4
mad r2, r3, #0.03125
min r5, r1, #0.0625
mul r1, r2, #32.0 | mov r2, r4
ph_wrap r6, r3, r1
mad r2, r6, #0.03125
mul r1, r2, #32.0
ph_wrap r2, r3, r1 | ld r1, [01ef]
mad r7, r1, #0.03125 | st r2, [01ec]
mul r1, r7, #32.0
ph_wrap r7, r3, r1
subxy r1, r3, r6
sign r3, r1
min r1, r3, #0.0
subxy r3, r7, r6
mul r6, r1, r3
subxy r1, r7, r6
mad r4, r1, #-0.03125
mul r3, r4, #32.0
ph_wrap r7, r3, r3
sign r6, r7
min r4, r6, #0.0
subxy r3, r7, r2
mul r6, r4, r3 | mul r3, r1, #-1.0
subxy r2, r7, r6
ph_wrap r7, r3, r3
mul r3, r1, #-0.5 | subxy r6, r1, r7
addxy r7, r3, #0.5 | mul r4, r2, #-1.0
ph_tri r1, r3, -r7 | st r4, [01ee]
mul r3, r5, r1
shl r7, r3, 15
shl r1, r7, 2
ph_clamp r3, r3, -r1
mul r7, r3, r3
mul r1, r3, r7
mul r0, r7, r1 | mul r7, r7, r7
mac_p1wc3 r7, r3, r3, #-0.034104689076754152, r7
addxy r7, r7, #-17.026106186230564
mul r0, r0, r7
mul r7, r0, #0.03125
mad r3, r7, #-0.095352512591594205
mad r3, r1, #-0.41123178361504448
mul r0, r6, r3
mul r7, r0, #0.78539800643920898
ph_wrap r1, r3, r4 | st r7, [01f2]
mul r4, r2, #-0.5 | subxy r7, r1, r2
addxy r1, r4, #0.5
ph_tri r2, r3, -r1
mul r4, r5, r2
shl r1, r4, 15
shl r5, r1, 2
ph_clamp r2, r3, -r5
mul r4, r2, r2
mul r1, r2, r4
mul r5, r4, r1 | mul r4, r4, r4
mac_p1wc3 r4, r2, r2, #-0.034104689076754152, r4
addxy r4, r4, #-17.026106186230564
mul r5, r5, r4
mul r4, r5, #0.03125
mad r2, r4, #-0.095352512591594205
mad r2, r1, #-0.41123178361504448 | ld r1, [01f5]
mul r5, r7, r2 | mov r2, r1 | ld r7, [01f7]
mul r4, r5, #-0.78539800643920898 | mad r2, r7, #-0.50126103797649757
st r4, [01f0] | ld r4, [01f1]
mov r7, r4
mad r7, r0, #0.39269900321960449 | ld r0, [01f3]
mad r7, r0, #0.5
mad r2, r7, #0.034666201629152259
mul r0, r2, #1.5688642263412476 | mov r7, r2
st r0, [01f6] | st r0, [0202]
mac_p1wc3 r0, r6, r3, #-0.5, r5 | ld r6, [0201] | ld r3, [0209]
mad r7, r1, #-0.31950568415055886 | mad r0, r4, #0.63661989959316345 | ld r5, [01f9] | ld r4, [01ff]
mad r7, r0, #0.017354443547897992 | mad r2, r3, #0.63740378753622462 | st r5, [01fa]
mul r1, r7, #2.4613349606933212 | mad r7, r6, #0.40628358836552458 | ld r6, [0207] | st r4, [0200]
st r1, [01f4] | st r1, [01f8] | ld r1, [0203]
mad r2, r7, #0.75290865113054539 | addxy r0, r5, r4 | ld r7, [01fb] | ld r5, [01fd]
mad r2, r0, #-0.10523075837658179 | addxy r3, r1, r6 | ld r0, [0205] | st r5, [01fe]
mad r2, r3, #-0.59394325412832727 | addxy r4, r7, r5 | st r0, [0206] | ld r5, [023e]
mad r4, r2, #0.69499853757836605 | mul r2, r5, #0.5400000071525568 | st r1, [0204]
mad r0, r4, #0.45157973770018534 | addxy r4, r2, #0.51499999966472387 | st r6, [0208] | ld r6, [0246]
mul r1, r0, #0.48420281195382131 | st r7, [01fc]
recipseed r0, r4 | st r1, [020a]
newton r0, r4
newton r0, r4
newton r0, r4
min r3, r6, #0.0038906249683350325 | st r0, [0248]
mul r1, r5, #-1.0 | addxy r6, r5, #-0.89814813687263062
mul r7, r3, #256.0 | addxy r4, r1, #0.89814813687263062
mul r3, r6, #-1.0800000143051136
max r6, r4, -r4
sign r2, r3
max r3, r6, #0.0018084490501203063 | st r2, [024a]
mul r1, r3, #34.560000457763635
recipseed r3, r1
newton r3, r1
newton r3, r1
newton r3, r1
mul r6, r2, r3 | ld r4, [020d]
mad r4, r7, #0.25 | st r6, [0246] | ld r6, [020f]
mad r6, r7, #0.25
ph_wrap r6, r3, r6
ph_wrap r1, r3, r4
sign r0, r6
min r0, r0, #0.0
subxy r2, r6, r1
mul r0, r0, r2 | mov r4, r1
subxy r2, r6, r0 | mad r4, r7, #0.25
mov r6, r2
mad r6, r7, #-0.25
mul r0, r6, #-1.0
ph_wrap r4, r3, r4
ph_wrap r7, r3, r0 | st r4, [020c]
sign r6, r7
absadd r0, r6, #-1.0
mul r6, r0, r7
addxy r0, r6, r7 | subxy r6, r1, r4
sign r7, r6
min r1, r7, #0.0
subxy r6, r0, r4
mul r7, r1, r6 | mov r6, r5
mad r6, r2, #0.92592591366159682 | subxy r4, r0, r7
addxy r0, r6, #0.027777776788966198 | mul r1, r4, #-1.0 | ld r6, [0248]
mad r5, r4, #0.92592591366159682 | mul r7, r0, #-1.0800000143051136 | st r1, [020e] | ld r4, [024a]
sign r1, r7
mac_p1wc3 r7, r4, r3, #0.015625, r6 | ld r3, [0246]
min r2, r1, #0.0
mul r4, r7, #0.03125
mac_p1wc3 r6, r2, r4, #0.03125, r3
max r7, r6, #-0.00094811886083334684
min r2, r7, #0.0625
mul r6, r0, r2
shl r7, r6, 10 | ld r6, [0244]
mul r0, r7, #-1.0800000143051136
ph_tri r2, r3, -r0
mul r7, r6, r2
shl r0, r7, 15
shl r2, r0, 3
mul r7, r2, #1.6000000238418579 | addxy r2, r5, #0.027777776788966198
ph_clamp r0, r3, -r7
mul r7, r2, #-1.0800000143051136
sign r5, r7
min r7, r5, #0.0
mac_p1wc3 r7, r7, r4, #0.03125, r3
mul r4, r2, r7
shl r3, r4, 10
mul r2, r3, #-1.0800000143051136
ph_tri r7, r3, -r2
mul r4, r6, r7
shl r3, r4, 15
shl r2, r3, 3
mul r6, r2, #1.6000000238418579 | mul r4, r0, r1
ph_clamp r7, r3, -r6
mul r3, r4, #-1.0 | mul r2, r7, r5 | ld r7, [0217]
mul r6, r2, #-1.0 | st r3, [0212] | ld r3, [0215]
mov r5, r3 | st r6, [0210] | ld r6, [0211]
mad r5, r7, #-0.4735108174809789
mov r7, r6
mad r7, r4, #-0.5 | ld r4, [0213]
mad r7, r4, #0.5
mad r5, r7, #0.02471467541816923
mul r4, r5, #1.5991098880767822 | mov r7, r5
st r4, [0216] | st r4, [0222]
mac_p1wc3 r4, r0, r1, #0.5, r2 | ld r0, [0221] | ld r1, [0229]
mad r7, r3, #-0.29610899226597925 | mad r4, r6, #-0.5 | ld r2, [0219] | ld r6, [021f]
mad r7, r4, #-0.015455270211538169 | mad r5, r1, #0.62534789351010778 | st r2, [021a]
mul r3, r7, #2.557152434144939 | mad r7, r0, #0.39105998791752911 | ld r0, [0227] | st r6, [0220]
st r3, [0214] | st r3, [0218] | ld r3, [0223]
mad r5, r7, #0.76742375065127288 | addxy r4, r2, r6 | ld r7, [021b] | ld r2, [021d]
mad r5, r4, #-0.10324041740892025 | addxy r1, r3, r0 | st r0, [0228] | ld r0, [0316]
mad r5, r1, #-0.58270937527584066 | addxy r6, r7, r2 | st r7, [021c] | ld r1, [0242]
mad r6, r5, #0.70839720543078366 | ld r7, [0240] | st r2, [021e]
mac_p1wc3 r2, r1, r7, #0.069972968350592024, r0 | ld r0, [a062] | ld r1, [0319]
mul r5, r2, #14.291233080031816 | subxy r1, r1, r0 | ld r2, [a03a] | st r0, [0318]
mul r1, r1, r1 | st r2, [a162] | ld r2, [031d] | st r3, [0224]
mad r2, r2, -r1 | st r0, [a16a] | ld r0, [031b]
addxy r1, r0, #-0.999 | ld r4, [0225] | st r5, [a152]
mul r1, r1, #256.0 | mad r4, r6, #0.45157973770018534 | st r4, [0226]
ph_clamp r1, r3, -r1
mul r1, r1, #256.0 | mul r3, r4, #0.50948474756359019
ph_clamp r1, r3, -r1 | st r3, [01ea] | ld r3, [c049]
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [031c]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c059]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [031f] | ld r2, [a042]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c058]
mul r1, r1, #256.0 | st r1, [031e]
ph_clamp r1, r3, -r1 | min r0, r0, #1.0
mov r0, #0.0 | st r1, [a132] | st r0, [031a]
ld r1, [a122] | st r0, [0322]
mad r2, r1, r3 | ld r1, [031a] | ld r3, [c05a]
mad r2, r1, r3 | ld r1, [a17a] | ld r3, [c05b]
mad r2, r1, r3 | ld r1, [a172] | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [a132] | ld r3, [c05d]
mad r2, r1, r3 | ld r1, [a042]
st r2, [0320] | ld r2, [0322] | st r1, [a1aa]
ld r0, [0320] | st r2, [a1c2]
st r0, [a1a2] | ld r1, [0325]
ld r0, [a04a]
st r0, [a1d2]
ld r0, [a063]
subxy r1, r1, r0 | ld r2, [a03b] | st r0, [0324]
mul r1, r1, r1 | st r2, [a163] | ld r2, [0329]
mad r2, r2, -r1 | st r0, [a16b] | ld r0, [0327]
addxy r1, r0, #-0.999 | ld r3, [a032]
mul r1, r1, #256.0 | st r3, [a1ca] | ld r3, [c049]
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0328]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c059]
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [032b] | ld r2, [a043]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c058]
mul r1, r1, #256.0 | st r1, [032a]
ph_clamp r1, r3, -r1 | min r0, r0, #1.0
mov r0, #0.0 | st r1, [a133] | st r0, [0326]
ld r1, [a123] | st r0, [032e]
mad r2, r1, r3 | ld r1, [0326] | ld r3, [c05a]
mad r2, r1, r3 | ld r1, [a17b] | ld r3, [c05b]
mad r2, r1, r3 | ld r1, [a173] | ld r3, [c05c]
mad r2, r1, r3 | ld r1, [a133] | ld r3, [c05d]
mad r2, r1, r3 | ld r1, [a043]
st r2, [032c] | ld r2, [032e] | st r1, [a1ab]
ld r0, [032c] | ld r3, [a033]
st r0, [a1a3]
ld r0, [a04b]
st r2, [a1c3]
st r3, [a1cb]
st r0, [a1d3]
