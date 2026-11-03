mov r7, #0.0 | ld r0, [a151] | ld r2, [0701]
addxy r7, r7, r0 | mul r2, r2, #7.31275 | ld r1, [a152]
addxy r7, r7, r1 | ld r0, [a153] | ld r3, [c090]
addxy r7, r7, r0 | ld r1, [a154] | ld r0, [0409]
addxy r7, r7, r1 | ld r1, [040b] | st r3, [a120] | st r3, [0510]
addxy r7, r7, r0 | ld r0, [040d] | ld r6, [c000]
addxy r7, r7, r1 | ld r1, [040f] | ld r4, [c062]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | st r0, [6008] | ld r1, [060d]
mul r7, r7, #0.16666666666666666 | st r0, [6009]
mul r7, r7, r6 | st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [0703]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [0702] | ld r0, [a168]
subxy r1, r1, r0 | st r0, [060c] | ld r0, [c098]
mul r1, r1, r1
ph_wrap r2, r3, r2 | st r1, [061a] | ld r1, [c050]
mul r1, r3, r1 | st r2, [0700] | ld r2, [c051]
mad r1, r0, -r2 | ld r2, [061f] | ld r0, [0521]
add r1, #1.0
st r1, [0616] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0618] | ld r1, [061a]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [061e]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [0618]
subxy r2, r0, r2 | st r3, [a1d8] | ld r3, [c091]
mad r0, r2, r1 | ld r1, [a130] | ld r2, [0616]
min r0, r0, #1.0 | st r1, [0550] | st r2, [a1d0]
st r0, [0520] | st r0, [a128]
ld r0, [c080] | st r3, [a121]
st r0, [0540] | ld r0, [c088]
st r0, [0530] | ld r0, [0540] | ld r2, [c051]
ld r1, [0530] | st r0, [a1b8] | st r3, [0512]
st r1, [a1c0] | ld r1, [062d]
ld r0, [a169]
subxy r1, r1, r0 | st r0, [062c] | ld r0, [c099]
mul r1, r1, r1
st r1, [063a] | ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r2, [063f] | ld r0, [0523]
add r1, #1.0
st r1, [0636] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0638] | ld r1, [063a]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [063e]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [0638]
subxy r2, r0, r2 | st r3, [a1d9] | ld r3, [c092]
mad r0, r2, r1 | ld r1, [a131] | ld r2, [0636]
min r0, r0, #1.0 | st r1, [0552] | st r2, [a1d1]
st r0, [0522] | st r0, [a129]
ld r0, [c081] | st r3, [a122]
st r0, [0542] | ld r0, [c089]
st r0, [0532] | ld r0, [0542] | ld r2, [c051]
ld r1, [0532] | st r0, [a1b9] | st r3, [0514]
st r1, [a1c1] | ld r1, [064d]
ld r0, [a16a]
subxy r1, r1, r0 | st r0, [064c] | ld r0, [c09a]
mul r1, r1, r1
st r1, [065a] | ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r2, [065f] | ld r0, [0525]
add r1, #1.0
st r1, [0656] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0658] | ld r1, [065a]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [065e]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [0658]
subxy r2, r0, r2 | st r3, [a1da] | ld r3, [c093]
mad r0, r2, r1 | ld r1, [a132] | ld r2, [0656]
min r0, r0, #1.0 | st r1, [0554] | st r2, [a1d2]
st r0, [0524] | st r0, [a12a]
ld r0, [c082] | st r3, [a123]
st r0, [0544] | ld r0, [c08a]
st r0, [0534] | ld r0, [0544] | ld r2, [c051]
ld r1, [0534] | st r0, [a1ba] | st r3, [0516]
st r1, [a1c2] | ld r1, [066d]
ld r0, [a16b]
subxy r1, r1, r0 | st r0, [066c] | ld r0, [c09b]
mul r1, r1, r1
st r1, [067a] | ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r2, [067f] | ld r0, [0527]
add r1, #1.0
st r1, [0676] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0678] | ld r1, [067a]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [067e]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [0678]
subxy r2, r0, r2 | st r3, [a1db] | ld r3, [c094]
mad r0, r2, r1 | ld r1, [a133] | ld r2, [0676]
min r0, r0, #1.0 | st r1, [0556] | st r2, [a1d3]
st r0, [0526] | st r0, [a12b]
ld r0, [c083] | st r3, [a124]
st r0, [0546] | ld r0, [c08b]
st r0, [0536] | ld r0, [0546] | ld r2, [c051]
ld r1, [0536] | st r0, [a1bb] | st r3, [0518]
st r1, [a1c3] | ld r1, [068d]
ld r0, [a16c]
subxy r1, r1, r0 | st r0, [068c] | ld r0, [c09c]
mul r1, r1, r1
st r1, [069a] | ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r2, [069f] | ld r0, [0529]
add r1, #1.0
st r1, [0696] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0698] | ld r1, [069a]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [069e]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a134] | ld r2, [c069]
min r0, r0, #1.0 | st r1, [0558] | ld r1, [c067]
mul r3, r1, r2 | st r0, [0528] | st r0, [a12c]
mul r7, r3, r4 | ld r0, [c084] | ld r3, [a13c]
st r0, [0548] | ld r0, [c08c]
st r0, [0538] | ld r0, [a1b4] | ld r4, [c065]
mul r1, r0, r7 | ld r7, [0181] | st r4, [0188]
mad r0, r7, #0.5
min r2, r1, #0.00390625
mul r1, r0, #2.0 | addxy r0, r3, #-0.49274435423331175
ph_wrap r7, r3, r1
mul r1, r0, #0.17583843797623297 | mov r0, #0.041666666666666664 | st r7, [0180]
mov r4, r0
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r1, r4, r4
mul r4, r1, r1
mul r1, r4, r4
mul r4, r1, r1 | addxy r1, r3, #-2.7058547537905633
mul r1, r3, r1
addxy r1, r1, #2.7976123302951308
mul r1, r3, r1
addxy r1, r1, #-1.4042515113978813
mul r3, r3, r1 | addxy r1, r7, #1.0
mul r4, r4, r1 | addxy r1, r3, #0.67161111008399732
addxy r3, r7, #-1.0
mul r7, r1, r3
mul r1, r7, #0.36269500851631165
addxy r3, r1, #0.5
min r7, r4, r3
addxy r1, r7, #0.125
mul r4, r1, #4.0
ph_tri r3, r3, -r4
mul r7, r3, r3
addxy r1, r7, #-8.8404402339953698
mul r4, r7, r1
addxy r7, r4, #21.601100021326904 | ld r4, [a144]
mul r1, r7, r3 | ld r3, [0183] | ld r7, [c066]
mad r2, r3, #0.00390625 | st r1, [018a]
mul r3, r2, #256.0 | mov r1, r0
ph_wrap r2, r3, r3
addxy r3, r4, #-0.49274435423331175 | st r2, [0182]
mul r3, r3, #0.17583843797623297
ymacd r1, r3, #0.16666666666666666
ymacd r1, r3, #0.5
ymacd r1, r3, #1.0
ymacd r1, r3, #1.0
mul r3, r1, r1
mul r1, r3, r3
mul r3, r1, r1
mul r1, r3, r3 | addxy r3, r4, #-2.7058547537905633
mul r3, r4, r3
addxy r3, r3, #2.7976123302951308
mul r3, r4, r3
addxy r3, r3, #-1.4042515113978813
mul r4, r4, r3 | addxy r3, r2, #1.0
mul r1, r1, r3 | addxy r3, r4, #0.67161111008399732
addxy r4, r2, #-1.0
mul r2, r3, r4
mul r3, r2, #0.36269500851631165
addxy r4, r3, #0.5
min r2, r1, r4
addxy r3, r2, #0.125
mul r1, r3, #4.0
ph_tri r4, r3, -r1
mul r2, r4, r4
addxy r3, r2, #-8.8404402339953698
mul r1, r2, r3
addxy r2, r1, #21.601100021326904 | ld r1, [0188]
mul r3, r2, r4 | ld r2, [018a]
mac_p1w r4, r1, r2, r7, r3 | ld r3, [0548] | ld r7, [0696] | ld r2, [a1cc]
mul r1, r4, #0.03125 | mul r4, r3, r7 | ld r7, [c064]
mad r1, r2, #-0.85969737115440648 | st r4, [018a] | ld r4, [a1e4] | ld r3, [c061]
addxy r4, r4, #-4.75 | ld r2, [0538]
mul r7, r7, r4
mac_p1wc3 r4, r2, r3, #0.125, r7 | ld r2, [0698] | ld r3, [c060]
mad r4, r2, #0.125
mul r7, r4, #0.17328679513998632
ymacd r0, r7, #0.16666666666666666
ymacd r0, r7, #0.5
ymacd r0, r7, #1.0
ymacd r0, r7, #1.0
mul r2, r0, r0
mul r4, r2, r2
mul r7, r4, r4
mul r0, r7, r7
mul r2, r0, r0 | ld r0, [c06b]
mul r4, r2, #6.103515625e-05
mul r7, r3, r4 | ld r4, [0185]
min r2, r7, #3.3569336665095761e-05 | ld r7, [0187]
shl r3, r2, 14
mul r2, r3, r7
addxy r2, r2, r4
mov r4, r2
mad r4, r1, #0.072700001299381256
mac_p1wc3 r4, r0, r7, #0.5, r4
mac_p1wc3 r4, r3, r4, #-0.5, r7
mac_p1wc3 r7, r3, r4, #-0.5, r2
mul r2, r7, #-2.0
mov r2, r7 | st r2, [0184]
mad r2, r1, #-0.036350000649690628
mac_p1wc3 r1, r0, r4, #0.5, r2
mac_p1wc3 r0, r3, r1, #-0.5, r4 | ld r3, [018a]
mul r1, r3, r7 | mul r2, r0, #4.0 | ld r0, [a16d]
mul r4, r1, #-32.0 | ld r1, [06ad] | ld r3, [c095] | st r2, [0186]
subxy r1, r1, r0 | st r0, [06ac] | ld r0, [c09d]
mul r1, r1, r1 | ld r2, [c051] | st r3, [a125]
st r1, [06ba] | ld r1, [c050]
mul r1, r3, r1 | st r3, [051a]
mad r1, r0, -r2 | ld r2, [06bf] | ld r0, [052b]
add r1, #1.0 | st r4, [0408] | ld r4, [c062]
st r1, [06b6] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [06b8] | ld r1, [06ba]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [06be]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a135] | ld r2, [c069]
min r0, r0, #1.0 | st r1, [055a] | ld r1, [c067]
mul r3, r1, r2 | st r0, [052a] | st r0, [a12d]
mul r7, r3, r4 | ld r0, [c085] | ld r3, [a13d]
st r0, [054a] | ld r0, [c08d]
st r0, [053a] | ld r0, [a1b5] | ld r4, [c065]
mul r1, r0, r7 | ld r7, [018d] | st r4, [0194]
mad r0, r7, #0.5
min r2, r1, #0.00390625
mul r1, r0, #2.0 | addxy r0, r3, #-0.49274435423331175
ph_wrap r7, r3, r1
mul r1, r0, #0.17583843797623297 | mov r0, #0.041666666666666664 | st r7, [018c]
mov r4, r0
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r1, r4, r4
mul r4, r1, r1
mul r1, r4, r4
mul r4, r1, r1 | addxy r1, r3, #-2.7058547537905633
mul r1, r3, r1
addxy r1, r1, #2.7976123302951308
mul r1, r3, r1
addxy r1, r1, #-1.4042515113978813
mul r3, r3, r1 | addxy r1, r7, #1.0
mul r4, r4, r1 | addxy r1, r3, #0.67161111008399732
addxy r3, r7, #-1.0
mul r7, r1, r3
mul r1, r7, #0.36269500851631165
addxy r3, r1, #0.5
min r7, r4, r3
addxy r1, r7, #0.125
mul r4, r1, #4.0
ph_tri r3, r3, -r4
mul r7, r3, r3
addxy r1, r7, #-8.8404402339953698
mul r4, r7, r1
addxy r7, r4, #21.601100021326904 | ld r4, [a145]
mul r1, r7, r3 | ld r3, [018f] | ld r7, [c066]
mad r2, r3, #0.00390625 | st r1, [0196]
mul r3, r2, #256.0 | mov r1, r0
ph_wrap r2, r3, r3
addxy r3, r4, #-0.49274435423331175 | st r2, [018e]
mul r3, r3, #0.17583843797623297
ymacd r1, r3, #0.16666666666666666
ymacd r1, r3, #0.5
ymacd r1, r3, #1.0
ymacd r1, r3, #1.0
mul r3, r1, r1
mul r1, r3, r3
mul r3, r1, r1
mul r1, r3, r3 | addxy r3, r4, #-2.7058547537905633
mul r3, r4, r3
addxy r3, r3, #2.7976123302951308
mul r3, r4, r3
addxy r3, r3, #-1.4042515113978813
mul r4, r4, r3 | addxy r3, r2, #1.0
mul r1, r1, r3 | addxy r3, r4, #0.67161111008399732
addxy r4, r2, #-1.0
mul r2, r3, r4
mul r3, r2, #0.36269500851631165
addxy r4, r3, #0.5
min r2, r1, r4
addxy r3, r2, #0.125
mul r1, r3, #4.0
ph_tri r4, r3, -r1
mul r2, r4, r4
addxy r3, r2, #-8.8404402339953698
mul r1, r2, r3
addxy r2, r1, #21.601100021326904 | ld r1, [0194]
mul r3, r2, r4 | ld r2, [0196]
mac_p1w r4, r1, r2, r7, r3 | ld r3, [054a] | ld r7, [06b6] | ld r2, [a1cd]
mul r1, r4, #0.03125 | mul r4, r3, r7 | ld r7, [c064]
mad r1, r2, #-0.85969737115440648 | st r4, [0196] | ld r4, [a1e5] | ld r3, [c061]
addxy r4, r4, #-4.75 | ld r2, [053a]
mul r7, r7, r4
mac_p1wc3 r4, r2, r3, #0.125, r7 | ld r2, [06b8] | ld r3, [c060]
mad r4, r2, #0.125
mul r7, r4, #0.17328679513998632
ymacd r0, r7, #0.16666666666666666
ymacd r0, r7, #0.5
ymacd r0, r7, #1.0
ymacd r0, r7, #1.0
mul r2, r0, r0
mul r4, r2, r2
mul r7, r4, r4
mul r0, r7, r7
mul r2, r0, r0 | ld r0, [c06b]
mul r4, r2, #6.103515625e-05
mul r7, r3, r4 | ld r4, [0191]
min r2, r7, #3.3569336665095761e-05 | ld r7, [0193]
shl r3, r2, 14
mul r2, r3, r7
addxy r2, r2, r4
mov r4, r2
mad r4, r1, #0.072700001299381256
mac_p1wc3 r4, r0, r7, #0.5, r4
mac_p1wc3 r4, r3, r4, #-0.5, r7
mac_p1wc3 r7, r3, r4, #-0.5, r2
mul r2, r7, #-2.0
mov r2, r7 | st r2, [0190]
mad r2, r1, #-0.036350000649690628
mac_p1wc3 r1, r0, r4, #0.5, r2
mac_p1wc3 r0, r3, r1, #-0.5, r4 | ld r3, [0196]
mul r1, r3, r7 | mul r2, r0, #4.0 | ld r0, [a16e]
mul r4, r1, #-32.0 | ld r1, [06cd] | ld r3, [c096] | st r2, [0192]
subxy r1, r1, r0 | st r0, [06cc] | ld r0, [c09e]
mul r1, r1, r1 | ld r2, [c051] | st r3, [a126]
st r1, [06da] | ld r1, [c050]
mul r1, r3, r1 | st r3, [051c]
mad r1, r0, -r2 | ld r2, [06df] | ld r0, [052d]
add r1, #1.0 | st r4, [040a] | ld r4, [c062]
st r1, [06d6] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [06d8] | ld r1, [06da]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [06de]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a136] | ld r2, [c069]
min r0, r0, #1.0 | st r1, [055c] | ld r1, [c067]
mul r3, r1, r2 | st r0, [052c] | st r0, [a12e]
mul r7, r3, r4 | ld r0, [c086] | ld r3, [a13e]
st r0, [054c] | ld r0, [c08e]
st r0, [053c] | ld r0, [a1b6] | ld r4, [c065]
mul r1, r0, r7 | ld r7, [0199] | st r4, [01a0]
mad r0, r7, #0.5
min r2, r1, #0.00390625
mul r1, r0, #2.0 | addxy r0, r3, #-0.49274435423331175
ph_wrap r7, r3, r1
mul r1, r0, #0.17583843797623297 | mov r0, #0.041666666666666664 | st r7, [0198]
mov r4, r0
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r1, r4, r4
mul r4, r1, r1
mul r1, r4, r4
mul r4, r1, r1 | addxy r1, r3, #-2.7058547537905633
mul r1, r3, r1
addxy r1, r1, #2.7976123302951308
mul r1, r3, r1
addxy r1, r1, #-1.4042515113978813
mul r3, r3, r1 | addxy r1, r7, #1.0
mul r4, r4, r1 | addxy r1, r3, #0.67161111008399732
addxy r3, r7, #-1.0
mul r7, r1, r3
mul r1, r7, #0.36269500851631165
addxy r3, r1, #0.5
min r7, r4, r3
addxy r1, r7, #0.125
mul r4, r1, #4.0
ph_tri r3, r3, -r4
mul r7, r3, r3
addxy r1, r7, #-8.8404402339953698
mul r4, r7, r1
addxy r7, r4, #21.601100021326904 | ld r4, [a146]
mul r1, r7, r3 | ld r3, [019b] | ld r7, [c066]
mad r2, r3, #0.00390625 | st r1, [01a2]
mul r3, r2, #256.0 | mov r1, r0
ph_wrap r2, r3, r3
addxy r3, r4, #-0.49274435423331175 | st r2, [019a]
mul r3, r3, #0.17583843797623297
ymacd r1, r3, #0.16666666666666666
ymacd r1, r3, #0.5
ymacd r1, r3, #1.0
ymacd r1, r3, #1.0
mul r3, r1, r1
mul r1, r3, r3
mul r3, r1, r1
mul r1, r3, r3 | addxy r3, r4, #-2.7058547537905633
mul r3, r4, r3
addxy r3, r3, #2.7976123302951308
mul r3, r4, r3
addxy r3, r3, #-1.4042515113978813
mul r4, r4, r3 | addxy r3, r2, #1.0
mul r1, r1, r3 | addxy r3, r4, #0.67161111008399732
addxy r4, r2, #-1.0
mul r2, r3, r4
mul r3, r2, #0.36269500851631165
addxy r4, r3, #0.5
min r2, r1, r4
addxy r3, r2, #0.125
mul r1, r3, #4.0
ph_tri r4, r3, -r1
mul r2, r4, r4
addxy r3, r2, #-8.8404402339953698
mul r1, r2, r3
addxy r2, r1, #21.601100021326904 | ld r1, [01a0]
mul r3, r2, r4 | ld r2, [01a2]
mac_p1w r4, r1, r2, r7, r3 | ld r3, [054c] | ld r7, [06d6] | ld r2, [a1ce]
mul r1, r4, #0.03125 | mul r4, r3, r7 | ld r7, [c064]
mad r1, r2, #-0.85969737115440648 | st r4, [01a2] | ld r4, [a1e6] | ld r3, [c061]
addxy r4, r4, #-4.75 | ld r2, [053c]
mul r7, r7, r4
mac_p1wc3 r4, r2, r3, #0.125, r7 | ld r2, [06d8] | ld r3, [c060]
mad r4, r2, #0.125
mul r7, r4, #0.17328679513998632
ymacd r0, r7, #0.16666666666666666
ymacd r0, r7, #0.5
ymacd r0, r7, #1.0
ymacd r0, r7, #1.0
mul r2, r0, r0
mul r4, r2, r2
mul r7, r4, r4
mul r0, r7, r7
mul r2, r0, r0 | ld r0, [c06b]
mul r4, r2, #6.103515625e-05
mul r7, r3, r4 | ld r4, [019d]
min r2, r7, #3.3569336665095761e-05 | ld r7, [019f]
shl r3, r2, 14
mul r2, r3, r7
addxy r2, r2, r4
mov r4, r2
mad r4, r1, #0.072700001299381256
mac_p1wc3 r4, r0, r7, #0.5, r4
mac_p1wc3 r4, r3, r4, #-0.5, r7
mac_p1wc3 r7, r3, r4, #-0.5, r2
mul r2, r7, #-2.0
mov r2, r7 | st r2, [019c]
mad r2, r1, #-0.036350000649690628
mac_p1wc3 r1, r0, r4, #0.5, r2
mac_p1wc3 r0, r3, r1, #-0.5, r4 | ld r3, [01a2]
mul r1, r3, r7 | mul r2, r0, #4.0 | ld r0, [a16f]
mul r4, r1, #-32.0 | ld r1, [06ed] | ld r3, [c097] | st r2, [019e]
subxy r1, r1, r0 | st r0, [06ec] | ld r0, [c09f]
mul r1, r1, r1 | ld r2, [c051] | st r3, [a127]
st r1, [06fa] | ld r1, [c050]
mul r1, r3, r1 | st r3, [051e]
mad r1, r0, -r2 | ld r2, [06ff] | ld r0, [052f]
add r1, #1.0 | st r4, [040c] | ld r4, [c062]
st r1, [06f6] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [06f8] | ld r1, [06fa]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [06fe]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a137] | ld r2, [c069]
min r0, r0, #1.0 | st r1, [055e] | ld r1, [c067]
mul r3, r1, r2 | st r0, [052e] | st r0, [a12f]
mul r7, r3, r4 | ld r0, [c087] | ld r3, [a13f]
st r0, [054e] | ld r0, [c08f]
st r0, [053e] | ld r0, [a1b7] | ld r4, [c065]
mul r1, r0, r7 | ld r7, [01a5] | st r4, [01ac]
mad r0, r7, #0.5
min r2, r1, #0.00390625
mul r1, r0, #2.0 | addxy r0, r3, #-0.49274435423331175
ph_wrap r7, r3, r1
mul r1, r0, #0.17583843797623297 | mov r0, #0.041666666666666664 | st r7, [01a4]
mov r4, r0
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r1, r4, r4
mul r4, r1, r1
mul r1, r4, r4
mul r4, r1, r1 | addxy r1, r3, #-2.7058547537905633
mul r1, r3, r1
addxy r1, r1, #2.7976123302951308
mul r1, r3, r1
addxy r1, r1, #-1.4042515113978813
mul r3, r3, r1 | addxy r1, r7, #1.0
mul r4, r4, r1 | addxy r1, r3, #0.67161111008399732
addxy r3, r7, #-1.0
mul r7, r1, r3
mul r1, r7, #0.36269500851631165
addxy r3, r1, #0.5
min r7, r4, r3
addxy r1, r7, #0.125
mul r4, r1, #4.0
ph_tri r3, r3, -r4
mul r7, r3, r3
addxy r1, r7, #-8.8404402339953698
mul r4, r7, r1
addxy r7, r4, #21.601100021326904 | ld r4, [a147]
mul r1, r7, r3 | ld r3, [01a7] | ld r7, [c066]
mad r2, r3, #0.00390625 | st r1, [01ae]
mul r3, r2, #256.0 | mov r1, r0
ph_wrap r2, r3, r3
addxy r3, r4, #-0.49274435423331175 | st r2, [01a6]
mul r3, r3, #0.17583843797623297
ymacd r1, r3, #0.16666666666666666
ymacd r1, r3, #0.5
ymacd r1, r3, #1.0
ymacd r1, r3, #1.0
mul r3, r1, r1
mul r1, r3, r3
mul r3, r1, r1
mul r1, r3, r3 | addxy r3, r4, #-2.7058547537905633
mul r3, r4, r3
addxy r3, r3, #2.7976123302951308
mul r3, r4, r3
addxy r3, r3, #-1.4042515113978813
mul r4, r4, r3 | addxy r3, r2, #1.0
mul r1, r1, r3 | addxy r3, r4, #0.67161111008399732
addxy r4, r2, #-1.0
mul r2, r3, r4
mul r3, r2, #0.36269500851631165
addxy r4, r3, #0.5
min r2, r1, r4
addxy r3, r2, #0.125
mul r1, r3, #4.0
ph_tri r4, r3, -r1
mul r2, r4, r4
addxy r3, r2, #-8.8404402339953698
mul r1, r2, r3
addxy r2, r1, #21.601100021326904 | ld r1, [01ac]
mul r3, r2, r4 | ld r2, [01ae]
mac_p1w r4, r1, r2, r7, r3 | ld r3, [054e] | ld r7, [06f6] | ld r2, [a1cf]
mul r1, r4, #0.03125 | mul r4, r3, r7 | ld r7, [c064]
mad r1, r2, #-0.85969737115440648 | st r4, [01ae] | ld r4, [a1e7] | ld r3, [c061]
addxy r4, r4, #-4.75 | ld r2, [053e]
mul r7, r7, r4
mac_p1wc3 r4, r2, r3, #0.125, r7 | ld r2, [06f8] | ld r3, [c060]
mad r4, r2, #0.125
mul r7, r4, #0.17328679513998632
ymacd r0, r7, #0.16666666666666666
ymacd r0, r7, #0.5
ymacd r0, r7, #1.0
ymacd r0, r7, #1.0
mul r2, r0, r0
mul r4, r2, r2
mul r7, r4, r4
mul r0, r7, r7
mul r2, r0, r0 | ld r0, [c06b]
mul r4, r2, #6.103515625e-05
mul r7, r3, r4 | ld r4, [01a9]
min r2, r7, #3.3569336665095761e-05 | ld r7, [01ab]
shl r3, r2, 14
mul r2, r3, r7
addxy r2, r2, r4
mov r4, r2
mad r4, r1, #0.072700001299381256
mac_p1wc3 r4, r0, r7, #0.5, r4
mac_p1wc3 r4, r3, r4, #-0.5, r7
mac_p1wc3 r7, r3, r4, #-0.5, r2
mul r2, r7, #-2.0
mov r2, r7 | st r2, [01a8]
mad r2, r1, #-0.036350000649690628
mac_p1wc3 r1, r0, r4, #0.5, r2
mac_p1wc3 r0, r3, r1, #-0.5, r4 | ld r3, [01ae]
mul r2, r0, #4.0 | mul r1, r3, r7 | ld r0, [c056]
mul r4, r1, #-32.0 | st r2, [01aa] | ld r1, [c054]
hset h6, r0
st r4, [040e] | ld r2, [c055]
ldidx r3, 0 | ldidx r4, 2 | ld r0, [c05c]
ymacd r3, r2, r1 | ldidx r5, 4 | ldidx r6, 6
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
