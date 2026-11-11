ld r2, [a038] | ld r4, [0183] | ld r6, [0185]
st r2, [a160] | ld r2, [c04b]
ld r0, [a060] | st r6, [01da]
ld r1, [a120]
mul r1, r1, r2 | st r0, [a168] | ld r2, [c05e]
ld r0, [a128]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r3, [a1b0]
mul r2, r1, #0.08664339756999316 | subxy r5, r4, r3 | ld r3, [c071]
ymacd r0, r2, #0.16666666666666666 | ld r7, [a140]
ymacd r0, r2, #0.5 | st r7, [01d8]
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a040]
mul r0, r0, r0 | mad r4, r5, #0.010400000028312206
mul r0, r0, r0 | st r4, [0182]
mul r0, r0, r0 | st r4, [01dc]
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [0300] | ld r1, [0305]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a048]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.0
st r2, [0302] | ld r2, [a040]
addxy r2, r1, r2 | st r0, [0306] | ld r0, [a1b8]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [0304]
ph_clamp r1, r3, -r1
st r1, [a130] | ld r1, [0181]
subxy r2, r1, r0 | ld r0, [c070]
mad r1, r2, #0.010400000028312206 | ld r2, [0187]
mac_p1w r5, r3, r6, r2, r0 | ld r0, [0189] | ld r3, [c077] | st r1, [0180]
mad r5, r0, #-0.125 | mul r2, r1, r1 | st r3, [01de]
mad r0, r5, #0.00059102423256263137
mul r0, r2, r2 | st r0, [0188]
mul r2, r0, r3
addxy r0, r2, #0.18518526367693175
mul r2, r0, #2.6999988555908203
recipseed r6, r2
newton r6, r2
newton r6, r2
newton r6, r2
mul r0, r5, r6 | mul r7, r6, r3 | ld r2, [018b] | ld r5, [018d]
mad r2, r5, #0.10000000149011612 | st r2, [019a]
mul r6, r2, #0.075000002980232239
ph_clamp r5, r3, -r6
mac_p1wc3 r2, r7, r5, #-0.022222221486362097, r0
mul r6, r2, #-2.6999988555908203
ph_clamp r7, r3, -r6 | ld r6, [018f]
mul r5, r7, r7
mul r0, r5, r7
mad r7, r0, #-0.33333301544189453
mul r2, r7, #2.0 | mov r5, r7
mad r5, r6, #0.5 | addxy r6, r1, #-0.5 | st r2, [018e] | ld r2, [0191]
mul r0, r5, r1 | mul r5, r2, r6
subxy r5, r5, r0
mul r0, r5, #2.0
mov r0, r5 | st r0, [0190]
mad r0, r2, #0.5 | ld r2, [0193]
mul r4, r2, r6
mac_p1wc3 r0, r0, r1, #-0.5, r4
mul r4, r0, #4.0
mov r4, r0 | st r4, [0192]
mad r4, r2, #0.25 | ld r2, [0195]
mul r3, r2, r6
mac_p1wc3 r4, r4, r1, #-0.5, r3
mul r3, r4, #4.0
mul r3, r7, r1 | st r3, [0194]
addxy r7, r5, r3
mac_p1wc3 r3, r5, r6, #-0.5, r7
mac_p1wc3 r5, r3, r1, #-0.5, r0
mul r7, r0, r6
addxy r3, r5, r7 | mul r5, r4, r6
mac_p1wc3 r0, r3, r1, #-0.5, r4 | ld r3, [0197]
addxy r7, r0, r5 | mad r4, r2, #0.25
mul r0, r3, r6
mac_p1wc3 r5, r4, r1, #-0.5, r0 | ld r0, [0199]
mul r2, r5, #4.0
mac_p1w r3, r5, r6, r7, r1 | st r2, [0196] | ld r7, [01de]
mul r4, r3, #-16.0 | mul r3, r7, r5 | ld r7, [019b]
max r2, r0, #-0.0076105840931104994 | st r4, [018a] | ld r4, [01dc]
mul r6, r2, #0.9924468994140625 | mad r5, r3, #0.5 | st r7, [018c] | ld r3, [0300]
addxy r1, r6, #0.0075531005859375 | mul r0, r5, r4 | ld r4, [c07a] | ld r5, [0302] | st r3, [01de]
addxy r6, r2, #0.0076105840931104994 | st r1, [0198] | ld r2, [01da]
mul r1, r6, r0 | mul r6, r2, r4 | st r5, [01dc]
mov r7, #0.041666666666666664 | mul r0, r6, #0.34657359027997264 | st r1, [01da] | ld r1, [019f]
mov r2, r7
ymacd r2, r0, #0.16666666666666666
ymacd r2, r0, #0.5
ymacd r2, r0, #1.0
ymacd r2, r0, #1.0
mul r4, r2, r2
mul r0, r4, r4
mul r2, r0, r0
mul r4, r2, r2
mul r0, r4, #0.03125 | mul r4, r6, #-0.34657359027997264
ymacd r7, r4, #0.16666666666666666 | mul r2, r3, r0
ymacd r7, r4, #0.5
ymacd r7, r4, #1.0
ymacd r7, r4, #1.0
mul r0, r7, r7
mul r6, r0, r0
mul r4, r6, r6
mul r7, r4, r4
mul r0, r7, #0.03125 | ld r7, [019d]
min r4, r2, #0.0077812499366700649
mul r6, r5, r0 | mov r0, r4
mov r5, r4
mad r0, r7, #0.03125 | mad r5, r1, #0.03125
mul r2, r0, #32.0 | mul r1, r5, #32.0
ph_wrap r0, r3, r2
ph_wrap r5, r3, r1
subxy r1, r7, r0
sign r7, r1
mov r2, r4
min r1, r7, #0.0
subxy r7, r5, r0 | mad r2, r0, #0.03125
mul r0, r1, r7 | mul r2, r2, #32.0
subxy r1, r5, r0
mad r4, r1, #-0.03125
mul r7, r4, #32.0
ph_wrap r5, r3, r7
ph_wrap r2, r3, r2
sign r0, r5 | st r2, [019c]
min r4, r0, #0.0
subxy r7, r5, r2
mul r0, r4, r7 | mul r7, r1, #-1.0
subxy r2, r5, r0
ph_wrap r5, r3, r7
mul r7, r1, #-0.5 | subxy r0, r1, r5
addxy r5, r7, #0.5 | mul r4, r2, #-1.0
ph_tri r1, r3, -r5 | st r4, [019e]
mul r7, r6, r1
shl r5, r7, 15
shl r1, r5, 2
ph_clamp r7, r3, -r1
mul r5, r7, r7
mul r1, r7, r5
mul r3, r5, r1 | mul r5, r5, r5
mac_p1wc3 r5, r7, r7, #-0.034104689076754152, r5
addxy r5, r5, #-17.026106186230564
mul r3, r3, r5
mul r5, r3, #0.03125
mad r7, r5, #-0.095352512591594205
mad r7, r1, #-0.41123178361504448
mul r3, r0, r7
mul r5, r3, #0.78539800643920898
ph_wrap r1, r3, r4 | st r5, [01a6]
mul r4, r2, #-0.5 | subxy r5, r1, r2
addxy r1, r4, #0.5
ph_tri r2, r3, -r1
mul r4, r6, r2
shl r1, r4, 15
shl r6, r1, 2
ph_clamp r2, r3, -r6
mul r4, r2, r2
mul r1, r2, r4
mul r6, r4, r1 | mul r4, r4, r4
mac_p1wc3 r4, r2, r2, #-0.034104689076754152, r4
addxy r4, r4, #-17.026106186230564
mul r6, r6, r4
mul r4, r6, #0.03125
mad r2, r4, #-0.095352512591594205
mad r2, r1, #-0.41123178361504448 | ld r1, [01a1]
mul r6, r5, r2 | mov r2, r1 | ld r5, [01a3]
mul r4, r6, #-0.78539800643920898 | mad r2, r5, #-0.50126103797649757
st r4, [01a4] | ld r4, [01a5]
mov r5, r4
mad r5, r3, #0.39269900321960449 | ld r3, [01a7]
mad r5, r3, #0.5
mad r2, r5, #0.034666201629152259
mul r3, r2, #1.5688642263412476 | mov r5, r2
st r3, [01a2] | st r3, [01b0]
mac_p1wc3 r3, r0, r7, #-0.5, r6 | ld r0, [01a9] | ld r7, [01ab]
mad r5, r1, #-0.31950568415055886 | mad r3, r4, #0.63661989959316345 | ld r6, [01ad] | ld r4, [01af]
mad r5, r3, #0.017354443547897992 | mad r2, r7, #0.63740378753622462 | st r6, [01b4]
mul r1, r5, #2.4613349606933212 | mad r5, r0, #0.40628358836552458 | ld r0, [01b3] | st r4, [01a8]
st r1, [01a0] | st r1, [01ac] | ld r1, [01b1]
mad r2, r5, #0.75290865113054539 | addxy r3, r6, r4 | ld r5, [01b5] | ld r6, [01b7]
mad r2, r3, #-0.10523075837658179 | addxy r7, r1, r0 | ld r3, [01b9] | st r1, [01b8]
mad r2, r7, #-0.59394325412832727 | addxy r4, r5, r6 | st r3, [01b2] | ld r7, [c072]
mad r4, r2, #0.69499853757836605 | st r6, [01ae] | ld r6, [c076]
mad r3, r4, #0.45157973770018534 | st r0, [01aa] | ld r2, [c073]
mul r1, r3, #0.48420281195382131 | mul r4, r6, r2 | ld r0, [c075] | st r5, [01b6] | ld r2, [01dc]
mul r5, r0, r7 | st r1, [0186] | ld r3, [c078] | ld r1, [01de]
mul r0, r1, r3 | ld r6, [c079]
mul r1, r2, r6 | mul r7, r5, r0 | ld r2, [01d8]
mul r6, r2, #0.5400000071525568 | mul r3, r4, r1
min r5, r7, #0.00048632812104187906 | st r3, [01dc]
addxy r4, r6, #0.51499999966472387 | addxy r1, r2, #-0.89814813687263062
shl r0, r5, 11
recipseed r5, r4
newton r5, r4
newton r5, r4
newton r5, r4
mul r4, r2, #-1.0 | mul r7, r1, #-1.0800000143051136 | st r5, [01e0]
addxy r1, r4, #0.89814813687263062
sign r6, r7
max r7, r1, -r1 | st r6, [01e2]
max r4, r7, #0.0018084490501203063
mul r1, r4, #34.560000457763635
recipseed r3, r1
newton r3, r1
newton r3, r1
newton r3, r1
mul r7, r6, r3 | ld r4, [01bb]
mad r4, r0, #0.25 | st r7, [01de] | ld r7, [01bd]
mad r7, r0, #0.25
ph_wrap r7, r3, r7
ph_wrap r1, r3, r4
sign r5, r7
min r5, r5, #0.0
subxy r6, r7, r1
mul r5, r5, r6 | mov r4, r1
subxy r6, r7, r5 | mad r4, r0, #0.25
mov r7, r6
mad r7, r0, #-0.25
mul r5, r7, #-1.0
ph_wrap r4, r3, r4
ph_wrap r0, r3, r5 | st r4, [01ba]
sign r7, r0
absadd r5, r7, #-1.0
mul r7, r5, r0
addxy r5, r7, r0 | subxy r7, r1, r4
sign r0, r7
min r1, r0, #0.0
subxy r7, r5, r4
mul r0, r1, r7 | mov r7, r2
mad r7, r6, #0.92592591366159682 | subxy r4, r5, r0
addxy r5, r7, #0.027777776788966198 | mul r1, r4, #-1.0 | ld r7, [01e0]
mad r2, r4, #0.92592591366159682 | mul r0, r5, #-1.0800000143051136 | st r1, [01bc] | ld r4, [01e2]
sign r1, r0
mac_p1wc3 r0, r4, r3, #0.015625, r7 | ld r3, [01de]
min r6, r1, #0.0
mul r4, r0, #0.03125
mac_p1wc3 r7, r6, r4, #0.03125, r3
mul r0, r5, r7
shl r6, r0, 10 | ld r0, [01dc]
mul r5, r6, #-1.0800000143051136
ph_tri r7, r3, -r5
mul r6, r0, r7
shl r5, r6, 15
shl r7, r5, 5
mul r6, r7, #1.6000000238418579 | addxy r7, r2, #0.027777776788966198
ph_clamp r5, r3, -r6
mul r6, r7, #-1.0800000143051136
sign r2, r6
min r6, r2, #0.0
mac_p1wc3 r6, r6, r4, #0.03125, r3
mul r4, r7, r6
shl r3, r4, 10
mul r7, r3, #-1.0800000143051136
ph_tri r6, r3, -r7
mul r4, r0, r6
shl r3, r4, 15
shl r7, r3, 5
mul r0, r7, #1.6000000238418579 | mul r4, r5, r1
ph_clamp r6, r3, -r0
mul r3, r4, #-1.0 | mul r7, r6, r2 | ld r6, [01c1]
mul r0, r7, #-1.0 | st r3, [01c4] | ld r3, [01bf]
mov r2, r3 | st r0, [01c2] | ld r0, [01c3]
mad r2, r6, #-0.4735108174809789
mov r6, r0
mad r6, r4, #-0.5 | ld r4, [01c5]
mad r6, r4, #0.5
mad r2, r6, #0.02471467541816923
mul r4, r2, #1.5991098880767822 | mov r6, r2
st r4, [01c0] | st r4, [01ce]
mac_p1wc3 r4, r5, r1, #0.5, r7 | ld r5, [01c7] | ld r1, [01c9]
mad r6, r3, #-0.29610899226597925 | mad r4, r0, #-0.5 | ld r7, [01cb] | ld r0, [01cd]
mad r6, r4, #-0.015455270211538169 | mad r2, r1, #0.62534789351010778 | st r7, [01d2]
mul r3, r6, #2.557152434144939 | mad r6, r5, #0.39105998791752911 | ld r5, [01d1] | st r0, [01c6]
st r3, [01be] | st r3, [01ca] | ld r3, [01cf]
mad r2, r6, #0.76742375065127288 | addxy r4, r7, r0 | ld r6, [01d3] | ld r7, [01d5]
mad r2, r4, #-0.10324041740892025 | addxy r1, r3, r5 | st r5, [01c8] | ld r5, [a1c8]
mad r2, r1, #-0.58270937527584066 | addxy r0, r6, r7 | st r6, [01d4] | ld r1, [0306]
mad r0, r2, #0.70839720543078366 | ld r6, [01da] | st r7, [01cc]
mac_p1wc3 r7, r6, r5, #0.069972968350592024, r1 | ld r4, [01d7] | ld r1, [a121]
mul r2, r7, #14.291233080031816 | mad r4, r0, #0.45157973770018534 | st r4, [01d0] | ld r0, [a061]
mul r3, r4, #0.50948474756359019 | st r2, [a151] | st r3, [01d6] | ld r4, [01e7]
ld r2, [a039] | st r3, [0184]
st r2, [a161] | ld r2, [c04b] | ld r6, [01e9]
mul r1, r1, r2 | st r0, [a169] | ld r2, [c05e]
ld r0, [a129] | st r6, [023e]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2 | ld r3, [a1b1]
mul r2, r1, #0.08664339756999316 | subxy r5, r4, r3 | ld r3, [c071]
ymacd r0, r2, #0.16666666666666666 | ld r7, [a141]
ymacd r0, r2, #0.5 | st r7, [023c]
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a041]
mul r0, r0, r0 | mad r4, r5, #0.010400000028312206
mul r0, r0, r0 | st r4, [01e6]
mul r0, r0, r0 | st r4, [0240]
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [0308] | ld r1, [030d]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a049]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.0
st r2, [030a] | ld r2, [a041]
addxy r2, r1, r2 | st r0, [030e] | ld r0, [a1b9]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [030c]
ph_clamp r1, r3, -r1
st r1, [a131] | ld r1, [01e5]
subxy r2, r1, r0 | ld r0, [c070]
mad r1, r2, #0.010400000028312206 | ld r2, [01eb]
mac_p1w r5, r3, r6, r2, r0 | ld r0, [01ed] | ld r3, [c077] | st r1, [01e4]
mad r5, r0, #-0.125 | mul r2, r1, r1 | st r3, [0242]
mad r0, r5, #0.00059102423256263137
mul r0, r2, r2 | st r0, [01ec]
mul r2, r0, r3
addxy r0, r2, #0.18518526367693175
mul r2, r0, #2.6999988555908203
recipseed r6, r2
newton r6, r2
newton r6, r2
newton r6, r2
mul r0, r5, r6 | mul r7, r6, r3 | ld r2, [01ef] | ld r5, [01f1]
mad r2, r5, #0.10000000149011612 | st r2, [01fe]
mul r6, r2, #0.075000002980232239
ph_clamp r5, r3, -r6
mac_p1wc3 r2, r7, r5, #-0.022222221486362097, r0
mul r6, r2, #-2.6999988555908203
ph_clamp r7, r3, -r6 | ld r6, [01f3]
mul r5, r7, r7
mul r0, r5, r7
mad r7, r0, #-0.33333301544189453
mul r2, r7, #2.0 | mov r5, r7
mad r5, r6, #0.5 | addxy r6, r1, #-0.5 | st r2, [01f2] | ld r2, [01f5]
mul r0, r5, r1 | mul r5, r2, r6
subxy r5, r5, r0
mul r0, r5, #2.0
mov r0, r5 | st r0, [01f4]
mad r0, r2, #0.5 | ld r2, [01f7]
mul r4, r2, r6
mac_p1wc3 r0, r0, r1, #-0.5, r4
mul r4, r0, #4.0
mov r4, r0 | st r4, [01f6]
mad r4, r2, #0.25 | ld r2, [01f9]
mul r3, r2, r6
mac_p1wc3 r4, r4, r1, #-0.5, r3
mul r3, r4, #4.0
mul r3, r7, r1 | st r3, [01f8]
addxy r7, r5, r3
mac_p1wc3 r3, r5, r6, #-0.5, r7
mac_p1wc3 r5, r3, r1, #-0.5, r0
mul r7, r0, r6
addxy r3, r5, r7 | mul r5, r4, r6
mac_p1wc3 r0, r3, r1, #-0.5, r4 | ld r3, [01fb]
addxy r7, r0, r5 | mad r4, r2, #0.25
mul r0, r3, r6
mac_p1wc3 r5, r4, r1, #-0.5, r0 | ld r0, [01fd]
mul r2, r5, #4.0
mac_p1w r3, r5, r6, r7, r1 | st r2, [01fa] | ld r7, [0242]
mul r4, r3, #-16.0 | mul r3, r7, r5 | ld r7, [01ff]
max r2, r0, #-0.0076105840931104994 | st r4, [01ee] | ld r4, [0240]
mul r6, r2, #0.9924468994140625 | mad r5, r3, #0.5 | st r7, [01f0] | ld r3, [0308]
addxy r1, r6, #0.0075531005859375 | mul r0, r5, r4 | ld r4, [c07a] | ld r5, [030a] | st r3, [0242]
addxy r6, r2, #0.0076105840931104994 | st r1, [01fc] | ld r2, [023e]
mul r1, r6, r0 | mul r6, r2, r4 | st r5, [0240]
mov r7, #0.041666666666666664 | mul r0, r6, #0.34657359027997264 | st r1, [023e] | ld r1, [0203]
mov r2, r7
ymacd r2, r0, #0.16666666666666666
ymacd r2, r0, #0.5
ymacd r2, r0, #1.0
ymacd r2, r0, #1.0
mul r4, r2, r2
mul r0, r4, r4
mul r2, r0, r0
mul r4, r2, r2
mul r0, r4, #0.03125 | mul r4, r6, #-0.34657359027997264
ymacd r7, r4, #0.16666666666666666 | mul r2, r3, r0
ymacd r7, r4, #0.5
ymacd r7, r4, #1.0
ymacd r7, r4, #1.0
mul r0, r7, r7
mul r6, r0, r0
mul r4, r6, r6
mul r7, r4, r4
mul r0, r7, #0.03125 | ld r7, [0201]
min r4, r2, #0.0077812499366700649
mul r6, r5, r0 | mov r0, r4
mov r5, r4
mad r0, r7, #0.03125 | mad r5, r1, #0.03125
mul r2, r0, #32.0 | mul r1, r5, #32.0
ph_wrap r0, r3, r2
ph_wrap r5, r3, r1
subxy r1, r7, r0
sign r7, r1
mov r2, r4
min r1, r7, #0.0
subxy r7, r5, r0 | mad r2, r0, #0.03125
mul r0, r1, r7 | mul r2, r2, #32.0
subxy r1, r5, r0
mad r4, r1, #-0.03125
mul r7, r4, #32.0
ph_wrap r5, r3, r7
ph_wrap r2, r3, r2
sign r0, r5 | st r2, [0200]
min r4, r0, #0.0
subxy r7, r5, r2
mul r0, r4, r7 | mul r7, r1, #-1.0
subxy r2, r5, r0
ph_wrap r5, r3, r7
mul r7, r1, #-0.5 | subxy r0, r1, r5
addxy r5, r7, #0.5 | mul r4, r2, #-1.0
ph_tri r1, r3, -r5 | st r4, [0202]
mul r7, r6, r1
shl r5, r7, 15
shl r1, r5, 2
ph_clamp r7, r3, -r1
mul r5, r7, r7
mul r1, r7, r5
mul r3, r5, r1 | mul r5, r5, r5
mac_p1wc3 r5, r7, r7, #-0.034104689076754152, r5
addxy r5, r5, #-17.026106186230564
mul r3, r3, r5
mul r5, r3, #0.03125
mad r7, r5, #-0.095352512591594205
mad r7, r1, #-0.41123178361504448
mul r3, r0, r7
mul r5, r3, #0.78539800643920898
ph_wrap r1, r3, r4 | st r5, [020a]
mul r4, r2, #-0.5 | subxy r5, r1, r2
addxy r1, r4, #0.5
ph_tri r2, r3, -r1
mul r4, r6, r2
shl r1, r4, 15
shl r6, r1, 2
ph_clamp r2, r3, -r6
mul r4, r2, r2
mul r1, r2, r4
mul r6, r4, r1 | mul r4, r4, r4
mac_p1wc3 r4, r2, r2, #-0.034104689076754152, r4
addxy r4, r4, #-17.026106186230564
mul r6, r6, r4
mul r4, r6, #0.03125
mad r2, r4, #-0.095352512591594205
mad r2, r1, #-0.41123178361504448 | ld r1, [0205]
mul r6, r5, r2 | mov r2, r1 | ld r5, [0207]
mul r4, r6, #-0.78539800643920898 | mad r2, r5, #-0.50126103797649757
st r4, [0208] | ld r4, [0209]
mov r5, r4
mad r5, r3, #0.39269900321960449 | ld r3, [020b]
mad r5, r3, #0.5
mad r2, r5, #0.034666201629152259
mul r3, r2, #1.5688642263412476 | mov r5, r2
st r3, [0206] | st r3, [0214]
mac_p1wc3 r3, r0, r7, #-0.5, r6 | ld r0, [020d] | ld r7, [020f]
mad r5, r1, #-0.31950568415055886 | mad r3, r4, #0.63661989959316345 | ld r6, [0211] | ld r4, [0213]
mad r5, r3, #0.017354443547897992 | mad r2, r7, #0.63740378753622462 | st r6, [0218]
mul r1, r5, #2.4613349606933212 | mad r5, r0, #0.40628358836552458 | ld r0, [0217] | st r4, [020c]
st r1, [0204] | st r1, [0210] | ld r1, [0215]
mad r2, r5, #0.75290865113054539 | addxy r3, r6, r4 | ld r5, [0219] | ld r6, [021b]
mad r2, r3, #-0.10523075837658179 | addxy r7, r1, r0 | ld r3, [021d] | st r1, [021c]
mad r2, r7, #-0.59394325412832727 | addxy r4, r5, r6 | st r3, [0216] | ld r7, [c072]
mad r4, r2, #0.69499853757836605 | st r6, [0212] | ld r6, [c076]
mad r3, r4, #0.45157973770018534 | st r0, [020e] | ld r2, [c073]
mul r1, r3, #0.48420281195382131 | mul r4, r6, r2 | ld r0, [c075] | st r5, [021a] | ld r2, [0240]
mul r5, r0, r7 | st r1, [01ea] | ld r3, [c078] | ld r1, [0242]
mul r0, r1, r3 | ld r6, [c079]
mul r1, r2, r6 | mul r7, r5, r0 | ld r2, [023c]
mul r6, r2, #0.5400000071525568 | mul r3, r4, r1
min r5, r7, #0.00048632812104187906 | st r3, [0240]
addxy r4, r6, #0.51499999966472387 | addxy r1, r2, #-0.89814813687263062
shl r0, r5, 11
recipseed r5, r4
newton r5, r4
newton r5, r4
newton r5, r4
mul r4, r2, #-1.0 | mul r7, r1, #-1.0800000143051136 | st r5, [0244]
addxy r1, r4, #0.89814813687263062
sign r6, r7
max r7, r1, -r1 | st r6, [0246]
max r4, r7, #0.0018084490501203063
mul r1, r4, #34.560000457763635
recipseed r3, r1
newton r3, r1
newton r3, r1
newton r3, r1
mul r7, r6, r3 | ld r4, [021f]
mad r4, r0, #0.25 | st r7, [0242] | ld r7, [0221]
mad r7, r0, #0.25
ph_wrap r7, r3, r7
ph_wrap r1, r3, r4
sign r5, r7
min r5, r5, #0.0
subxy r6, r7, r1
mul r5, r5, r6 | mov r4, r1
subxy r6, r7, r5 | mad r4, r0, #0.25
mov r7, r6
mad r7, r0, #-0.25
mul r5, r7, #-1.0
ph_wrap r4, r3, r4
ph_wrap r0, r3, r5 | st r4, [021e]
sign r7, r0
absadd r5, r7, #-1.0
mul r7, r5, r0
addxy r5, r7, r0 | subxy r7, r1, r4
sign r0, r7
min r1, r0, #0.0
subxy r7, r5, r4
mul r0, r1, r7 | mov r7, r2
mad r7, r6, #0.92592591366159682 | subxy r4, r5, r0
addxy r5, r7, #0.027777776788966198 | mul r1, r4, #-1.0 | ld r7, [0244]
mad r2, r4, #0.92592591366159682 | mul r0, r5, #-1.0800000143051136 | st r1, [0220] | ld r4, [0246]
sign r1, r0
mac_p1wc3 r0, r4, r3, #0.015625, r7 | ld r3, [0242]
min r6, r1, #0.0
mul r4, r0, #0.03125
mac_p1wc3 r7, r6, r4, #0.03125, r3
mul r0, r5, r7
shl r6, r0, 10 | ld r0, [0240]
mul r5, r6, #-1.0800000143051136
ph_tri r7, r3, -r5
mul r6, r0, r7
shl r5, r6, 15
shl r7, r5, 5
mul r6, r7, #1.6000000238418579 | addxy r7, r2, #0.027777776788966198
ph_clamp r5, r3, -r6
mul r6, r7, #-1.0800000143051136
sign r2, r6
min r6, r2, #0.0
mac_p1wc3 r6, r6, r4, #0.03125, r3
mul r4, r7, r6
shl r3, r4, 10
mul r7, r3, #-1.0800000143051136
ph_tri r6, r3, -r7
mul r4, r0, r6
shl r3, r4, 15
shl r7, r3, 5
mul r0, r7, #1.6000000238418579 | mul r4, r5, r1
ph_clamp r6, r3, -r0
mul r3, r4, #-1.0 | mul r7, r6, r2 | ld r6, [0225]
mul r0, r7, #-1.0 | st r3, [0228] | ld r3, [0223]
mov r2, r3 | st r0, [0226] | ld r0, [0227]
mad r2, r6, #-0.4735108174809789
mov r6, r0
mad r6, r4, #-0.5 | ld r4, [0229]
mad r6, r4, #0.5
mad r2, r6, #0.02471467541816923
mul r4, r2, #1.5991098880767822 | mov r6, r2
st r4, [0224] | st r4, [0232]
mac_p1wc3 r4, r5, r1, #0.5, r7 | ld r5, [022b] | ld r1, [022d]
mad r6, r3, #-0.29610899226597925 | mad r4, r0, #-0.5 | ld r7, [022f] | ld r0, [0231]
mad r6, r4, #-0.015455270211538169 | mad r2, r1, #0.62534789351010778 | st r7, [0236]
mul r3, r6, #2.557152434144939 | mad r6, r5, #0.39105998791752911 | ld r5, [0235] | st r0, [022a]
st r3, [0222] | st r3, [022e] | ld r3, [0233]
mad r2, r6, #0.76742375065127288 | addxy r4, r7, r0 | ld r6, [0237] | ld r7, [0239]
mad r2, r4, #-0.10324041740892025 | addxy r1, r3, r5 | st r5, [022c] | ld r5, [a1c9]
mad r2, r1, #-0.58270937527584066 | addxy r0, r6, r7 | st r6, [0238] | ld r1, [030e]
mad r0, r2, #0.70839720543078366 | ld r6, [023e] | st r7, [0230]
mac_p1wc3 r7, r6, r5, #0.069972968350592024, r1 | ld r4, [023b] | ld r1, [a122]
mul r2, r7, #14.291233080031816 | mad r4, r0, #0.45157973770018534 | st r4, [0234] | ld r0, [a062]
mul r3, r4, #0.50948474756359019 | st r2, [a152] | st r3, [023a]
ld r2, [a03a] | st r3, [01e8]
st r2, [a162] | ld r2, [c04b]
mul r1, r1, r2 | st r0, [a16a] | ld r2, [c05e]
ld r0, [a12a]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2
mul r2, r1, #0.08664339756999316
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a042]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [0310] | ld r1, [0315]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a04a]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.0
st r2, [0312] | ld r2, [a042]
addxy r2, r1, r2 | st r0, [0316] | ld r0, [0310]
ph_wrap r1, r3, r2 | ld r2, [0312] | st r0, [a1aa]
mul r1, r1, #256.0 | st r1, [0314] | st r2, [a1d2]
ph_clamp r1, r3, -r1 | ld r2, [a03b]
st r1, [a132] | ld r1, [0316]
st r1, [a1c2]
st r2, [a163] | ld r2, [c04b]
ld r0, [a063]
ld r1, [a123]
mul r1, r1, r2 | st r0, [a16b] | ld r2, [c05e]
ld r0, [a12b]
mad r1, r0, r2 | mov r0, #0.041666666666666664 | ld r2, [c05f]
addxy r1, r1, r2
mul r2, r1, #0.08664339756999316
ymacd r0, r2, #0.16666666666666666
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a043]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.16666666666666666
mul r2, r1, #-0.08664339756999316 | st r2, [0318] | ld r1, [031d]
ymacd r0, r2, #0.5
ymacd r0, r2, #1.0
ymacd r0, r2, #1.0 | ld r2, [a04b]
mul r0, r0, r0
mul r0, r0, r0
mul r0, r0, r0
mul r2, r2, r0 | mov r0, #0.0
st r2, [031a] | ld r2, [a043]
addxy r2, r1, r2 | st r0, [031e] | ld r0, [0318]
ph_wrap r1, r3, r2 | ld r2, [031a] | st r0, [a1ab]
mul r1, r1, #256.0 | st r1, [031c] | st r2, [a1d3]
ph_clamp r1, r3, -r1
st r1, [a133] | ld r1, [031e]
st r1, [a1c3]
