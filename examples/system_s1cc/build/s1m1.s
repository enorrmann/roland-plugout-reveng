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
mad r1, r2, r3 | mad r5, r6, r7 | ld r3, [c078]
mul r2, r2, #1.25 | mul r6, r6, #1.25 | ld r7, [c078] | st r3, [018a]
subxy r2, r0, r2 | subxy r6, r4, r6 | st r7, [01b4] | ld r3, [c071]
mad r0, r2, r1 | mad r4, r6, r5 | ld r7, [c071] | st r3, [0194]
min r0, r0, #1.0 | st r7, [01be] | ld r2, [c070] | ld r1, [a134]
min r4, r4, #1.0 | ld r6, [c070] | st r2, [018c] | ld r5, [a135]
ld r3, [c07b] | st r6, [01b6]
ld r7, [c07b] | st r3, [0196]
st r7, [01c0] | ld r2, [c079]
ld r6, [c079] | st r2, [018e]
ld r3, [c074] | st r6, [01b8]
ld r7, [c074] | st r3, [0198]
st r7, [01c2] | ld r2, [c072]
ld r6, [c072] | st r2, [0190]
ld r3, [c07c] | st r6, [01ba]
ld r7, [c07c] | st r3, [019a]
st r7, [01c4] | ld r2, [c07a]
ld r6, [c07a] | st r2, [0192]
ld r3, [c075] | st r6, [01bc]
ld r7, [c075] | st r3, [019c]
st r7, [01c6] | ld r2, [c073]
ld r6, [c073] | st r0, [0528]
ld r3, [c07d] | st r4, [052a]
ld r7, [c07d] | st r0, [a12c]
st r4, [a12d] | ld r0, [c084]
ld r4, [c085] | st r3, [019e]
st r7, [01c8] | st r2, [01a0]
st r6, [01ca] | ld r3, [c069]
ld r7, [c069] | st r0, [0548]
ld r2, [c06b] | st r4, [054a]
mul r3, r3, r2 | ld r6, [c06b] | st r1, [0558] | ld r1, [a1ec]
mul r7, r7, r6 | ld r0, [c08c] | st r5, [055a] | ld r5, [a1ed]
ld r4, [c08d] | st r0, [0538] | ld r0, [a1b4]
ld r2, [c062] | st r4, [053a] | ld r4, [a1b5]
mul r3, r3, r2 | ld r6, [c062] | st r1, [01a8]
mul r7, r7, r6 | mul r2, r0, r3 | st r5, [01d2]
mul r6, r4, r7
shl r3, r2, 10
shl r7, r6, 10 | min r2, r3, #0.25 | ld r3, [c06a]
min r6, r7, #0.25 | ld r7, [c06a] | st r2, [01a2]
mul r3, r1, r3 | mul r7, r5, r7 | st r6, [01cc] | ld r2, [c06c]
mul r3, r3, r2 | ld r6, [c06c]
mul r7, r7, r6 | ld r2, [c063]
mul r3, r3, r2 | ld r6, [c063]
mul r7, r7, r6 | mul r2, r3, #256.0 | ld r3, [a13c]
mul r6, r7, #256.0 | st r2, [01a4] | ld r2, [0181]
mad r0, r2, #0.5 | st r6, [01ce] | ld r6, [01ab]
mad r4, r6, #0.5 | mul r2, r0, #2.0 | ld r7, [a13d]
mul r6, r4, #2.0
ph_wrap r0, r3, r2
ph_wrap r4, r3, r6 | st r0, [0180]
addxy r2, r3, #0.72547136654093203 | addxy r6, r7, #0.72547136654093203 | st r4, [01aa]
st r0, [01a6] | st r4, [01d0]
mul r2, r2, r2 | mul r6, r6, r6
shl r0, r1, 11 | ld r1, [0194]
shl r4, r5, 11 | ld r5, [01be]
mul r2, r2, r2 | mul r6, r6, r6
mul r0, r0, #1.1080034062384143 | mul r4, r4, #1.1080034062384143
min r2, r2, r0 | ld r0, [01a0]
min r6, r6, r4 | ld r4, [01ca]
mac_p1wc3 r0, r0, r2, #0.27700085155960358, r1 | ld r2, [018c]
mac_p1wc3 r4, r4, r6, #0.27700085155960358, r5 | ld r6, [01b6] | st r0, [01a0]
mac_p1wc3 r0, r1, r3, #0.066666666666666666, r2 | st r4, [01ca] | ld r2, [01a8]
mac_p1wc3 r4, r5, r7, #0.066666666666666666, r6 | ld r6, [01d2] | ld r1, [0190]
addxy r2, r2, #-0.000244140625 | addxy r6, r6, #-0.000244140625 | ld r5, [01ba]
mul r2, r2, #136.53333333333333 | mul r6, r6, #136.53333333333333
min r0, r0, r2
min r4, r4, r6
addxy r2, r3, #-2.5549090593309853 | addxy r6, r7, #-2.5549090593309853
mul r2, r3, r2 | mul r6, r7, r6
addxy r2, r2, #2.8640197605209372 | addxy r6, r6, #2.8640197605209372
mul r3, r3, r2 | mul r7, r7, r6
addxy r2, r3, #-2.2379875792421422 | addxy r6, r7, #-2.2379875792421422
mul r3, r2, r2 | mul r7, r6, r6
mul r2, r3, r3 | mul r6, r7, r7
mul r3, r2, #0.03125 | mul r7, r6, #0.03125
addxy r2, r3, #-0.78393618153496136 | addxy r6, r7, #-0.78393618153496136
mul r1, r1, r2 | mul r5, r5, r6
addxy r2, r0, #0.033333333333333333 | addxy r6, r4, #0.033333333333333333 | ld r0, [01a6] | ld r4, [01d0]
addxy r0, r0, #1.0 | addxy r4, r4, #1.0
mac_p1wc3 r2, r2, r0, #-0.17008187206293277, r1
mac_p1wc3 r6, r6, r4, #-0.17008187206293277, r5
mul r1, r2, #15.0 | mul r5, r6, #15.0
floor r1, r1
floor r5, r5 | st r1, [01a6]
mad r2, r1, #-0.066666666666666666 | mad r6, r5, #-0.066666666666666666 | st r5, [01d0] | ld r1, [01a0]
addxy r0, r3, #-0.17011031009890953 | mul r1, r1, r2 | ld r5, [01ca] | st r0, [01a0]
addxy r4, r7, #-0.17011031009890953 | mul r0, r3, r0 | st r4, [01ca]
mul r4, r7, r4 | addxy r0, r0, #0.72448098229473701
addxy r4, r4, #0.72448098229473701 | mul r3, r3, r0
mul r7, r7, r4 | mul r5, r5, r6
addxy r0, r3, #0.93103060483042199 | addxy r4, r7, #0.93103060483042199 | ld r3, [0198] | ld r7, [01c2]
st r1, [0198] | st r5, [01c2] | ld r1, [019c]
mul r3, r3, r0 | mul r7, r7, r4 | ld r5, [01c6]
mad r1, r3, #0.53299000445677069 | mad r5, r7, #0.53299000445677069 | ld r3, [c076]
mul r3, r3, r0 | ld r7, [c076]
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.017766333481892358, r3 | ld r3, [0198]
mac_p1wc3 r4, r5, r6, #0.017766333481892358, r7 | ld r7, [01c2]
mul r1, r0, #0.27700085155960358 | mul r5, r4, #0.27700085155960358
addxy r2, r1, #0.0092333617186534551 | addxy r6, r5, #0.0092333617186534551
min r0, r3, r2 | ld r3, [0194] | ld r2, [01a0]
min r4, r7, r6 | ld r7, [01be] | ld r6, [01ca]
min r1, r0, #0.0092333617186534534
min r5, r4, #0.0092333617186534534
mul r0, r3, r2 | mul r4, r7, r6 | ld r3, [c077] | ld r2, [01a6] | ld r6, [01d0]
mac_p1wc3 r3, r3, r2, #-0.5, r0 | ld r7, [c077] | ld r2, [018c]
mac_p1wc3 r7, r7, r6, #-0.5, r4 | ld r6, [01b6]
mul r0, r2, r1 | mul r4, r6, r5
subxy r2, r1, r0 | subxy r6, r5, r4
addxy r1, r2, #-0.0023083404296633634 | addxy r5, r6, #-0.0023083404296633634
mul r0, r1, #-216.60583230044517 | mul r4, r5, #-216.60583230044517
ph_tri r2, r3, -r0
ph_tri r6, r3, -r4
mul r1, r2, r2 | mul r5, r6, r6
addxy r0, r1, #-8.8404402339953698 | addxy r4, r5, #-8.8404402339953698
mul r1, r1, r0 | mul r5, r5, r4
addxy r0, r1, #21.601100021326904 | addxy r4, r5, #21.601100021326904
mul r1, r2, r0 | mul r5, r6, r4
addxy r2, r3, #0.5 | addxy r6, r7, #0.5 | ld r3, [a144]
mul r0, r1, r2 | mul r4, r5, r6 | ld r1, [0183] | ld r5, [01ad] | ld r7, [a145]
ld r2, [01a2] | ld r6, [01cc]
mad r2, r1, #0.25 | mad r6, r5, #0.25 | st r0, [01a2]
mul r1, r2, #4.0 | mul r5, r6, #4.0 | st r4, [01cc]
ph_wrap r2, r3, r1
ph_wrap r6, r3, r5 | st r2, [0182]
max r1, r2, -r2 | st r6, [01ac]
max r5, r6, -r6 | st r1, [018c] | ld r1, [01a4]
addxy r0, r3, #0.72547136654093203 | addxy r4, r7, #0.72547136654093203 | st r5, [01b6] | ld r5, [01ce]
st r2, [01a6] | st r6, [01d0]
mul r0, r0, r0 | mul r4, r4, r4
shl r2, r1, 14 | ld r1, [018e]
shl r6, r5, 14 | ld r5, [01b8]
mul r0, r0, r0 | mul r4, r4, r4
mul r2, r2, #1.1080034062384143 | mul r6, r6, #1.1080034062384143
min r0, r0, r2 | ld r2, [0196]
min r4, r4, r6 | ld r6, [01c0]
mac_p1wc3 r2, r2, r0, #0.27700085155960358, r1 | ld r0, [018a]
mac_p1wc3 r6, r6, r4, #0.27700085155960358, r5 | ld r4, [01b4] | st r2, [0196]
mac_p1wc3 r2, r1, r3, #0.066666666666666666, r0 | st r6, [01c0] | ld r0, [01a4]
mac_p1wc3 r6, r5, r7, #0.066666666666666666, r4 | ld r4, [01ce] | ld r1, [0192]
addxy r0, r0, #-3.0517578125e-05 | addxy r4, r4, #-3.0517578125e-05 | ld r5, [01bc]
shl r0, r0, 10
shl r4, r4, 10
mul r0, r0, #1.0666666666666667 | mul r4, r4, #1.0666666666666667
min r2, r2, r0
min r6, r6, r4
addxy r0, r3, #-2.5549090593309853 | addxy r4, r7, #-2.5549090593309853
mul r0, r3, r0 | mul r4, r7, r4
addxy r0, r0, #2.8640197605209372 | addxy r4, r4, #2.8640197605209372
mul r3, r3, r0 | mul r7, r7, r4
addxy r0, r3, #-2.2379875792421422 | addxy r4, r7, #-2.2379875792421422
mul r3, r0, r0 | mul r7, r4, r4
mul r0, r3, r3 | mul r4, r7, r7
mul r3, r0, #0.03125 | mul r7, r4, #0.03125
addxy r0, r3, #-0.78393618153496136 | addxy r4, r7, #-0.78393618153496136
mul r1, r1, r0 | mul r5, r5, r4
addxy r0, r2, #0.033333333333333333 | addxy r4, r6, #0.033333333333333333 | ld r2, [01a6] | ld r6, [01d0]
addxy r2, r2, #1.0 | addxy r6, r6, #1.0
mac_p1wc3 r0, r0, r2, #-0.17008187206293277, r1
mac_p1wc3 r4, r4, r6, #-0.17008187206293277, r5
mul r2, r0, #15.0 | mul r6, r4, #15.0
floor r1, r2 | ld r2, [0196]
floor r5, r6 | ld r6, [01c0] | st r1, [0196]
mad r0, r1, #-0.066666666666666666 | mad r4, r5, #-0.066666666666666666 | st r5, [01c0]
addxy r1, r3, #-0.17011031009890953 | addxy r5, r7, #-0.17011031009890953
mul r1, r3, r1 | mul r5, r7, r5
addxy r1, r1, #0.72448098229473701 | addxy r5, r5, #0.72448098229473701
mul r3, r3, r1 | mul r7, r7, r5
mul r2, r2, r0 | mul r6, r6, r4
addxy r1, r3, #0.93103060483042199 | addxy r5, r7, #0.93103060483042199 | ld r3, [019a] | ld r7, [01c4]
st r2, [019a] | st r6, [01c4] | ld r2, [019e]
mul r3, r3, r1 | mul r7, r7, r5 | ld r6, [01c8]
mad r2, r3, #0.53299000445677069 | mad r6, r7, #0.53299000445677069 | ld r3, [c07e]
mul r3, r3, r1 | ld r7, [c07e]
mul r7, r7, r5
mac_p1wc3 r1, r2, r0, #0.017766333481892358, r3 | ld r3, [019a]
mac_p1wc3 r5, r6, r4, #0.017766333481892358, r7 | ld r7, [01c4]
mul r2, r1, #0.27700085155960358 | mul r6, r5, #0.27700085155960358
addxy r0, r2, #0.0092333617186534551 | addxy r4, r6, #0.0092333617186534551
min r1, r3, r0 | ld r3, [018e] | ld r0, [018c]
min r5, r7, r4 | ld r7, [01b8] | ld r4, [01b6]
min r2, r1, #0.0092333617186534534
min r6, r5, #0.0092333617186534534
mul r1, r3, r0 | mul r5, r7, r4 | ld r3, [c07f] | ld r0, [0196] | ld r4, [01c0]
mul r3, r3, r0 | ld r7, [c07f]
mul r7, r7, r4 | subxy r0, r1, r3 | ld r1, [018a]
subxy r4, r5, r7 | mul r3, r1, r2 | ld r5, [01b4]
mul r7, r5, r6 | subxy r1, r2, r3
subxy r5, r6, r7 | addxy r2, r1, #-0.0023083404296633634
addxy r6, r5, #-0.0023083404296633634 | mul r3, r2, #-216.60583230044517
mul r7, r6, #-216.60583230044517
ph_tri r1, r3, -r3
ph_tri r5, r3, -r7
mul r2, r1, r1 | mul r6, r5, r5
addxy r3, r2, #-8.8404402339953698 | addxy r7, r6, #-8.8404402339953698
mul r2, r2, r3 | mul r6, r6, r7
addxy r3, r2, #21.601100021326904 | addxy r7, r6, #21.601100021326904
mul r2, r1, r3 | mul r6, r5, r7
addxy r1, r0, #0.5 | addxy r5, r4, #0.5 | ld r0, [0185] | ld r4, [01af]
mul r3, r2, r1 | mul r7, r6, r5
addxy r2, r0, #0.084514582991543796 | addxy r6, r4, #0.084514582991543796
mul r1, r2, #7.3127498626708984 | mul r5, r6, #7.3127498626708984 | ld r2, [c06f]
ph_wrap r0, r3, r1 | ld r1, [c06e]
ph_wrap r4, r3, r5 | st r0, [0184] | ld r5, [c06e]
st r4, [01ae] | st r0, [018a] | ld r0, [01a2]
mul r1, r1, r0 | st r4, [01b4] | ld r4, [01cc] | ld r6, [c06f]
mul r5, r5, r4 | mad r2, r1, #-0.14540000259876251 | ld r0, [018a] | ld r4, [01b4]
mad r6, r5, #-0.14540000259876251 | mul r1, r3, r2 | ld r2, [c067]
mul r5, r7, r6 | mul r2, r2, r0 | ld r6, [c067]
mul r6, r6, r4 | ld r3, [c068]
addxy r0, r2, r3 | ld r7, [c068]
addxy r4, r6, r7 | mul r2, r1, r0 | ld r3, [c065] | ld r0, [01a2]
mul r6, r5, r4 | ld r7, [c065] | ld r4, [01cc]
ld r1, [c066]
mac_p1w r0, r0, r3, r2, r1 | ld r5, [c066] | ld r3, [a1cc]
mac_p1w r4, r4, r7, r6, r5 | ld r7, [a1cd] | ld r1, [0548] | ld r5, [054a]
mad r3, r0, #-0.072700001299381256 | mad r7, r4, #-0.072700001299381256 | ld r2, [0696] | ld r6, [06b6]
mul r0, r1, r2 | mul r4, r5, r6 | st r3, [018a] | ld r3, [a1e4]
addxy r3, r3, #-4.75 | st r7, [01b4] | ld r7, [a1e5] | ld r2, [c061]
st r0, [01a2] | st r4, [01cc] | ld r1, [0538]
addxy r7, r7, #-4.75 | ld r0, [c064] | ld r5, [053a]
mul r0, r0, r3 | ld r4, [c064]
mul r4, r4, r7 | ld r6, [c061]
mac_p1wc3 r3, r1, r2, #0.125, r0 | ld r1, [0698] | ld r2, [c060]
mac_p1wc3 r7, r5, r6, #0.125, r4 | ld r5, [06b8] | ld r6, [c060]
mad r3, r1, #0.125 | mad r7, r5, #0.125
mul r0, r3, #0.17328679513998632 | mul r4, r7, #0.17328679513998632
mov r1, #0.041666666666666664 | mov r5, #0.041666666666666664
ymacd r1, r0, #0.16666666666666666 | ymacd r5, r4, #0.16666666666666666
ymacd r1, r0, #0.5 | ymacd r5, r4, #0.5
ymacd r1, r0, #1.0 | ymacd r5, r4, #1.0
ymacd r1, r0, #1.0 | ymacd r5, r4, #1.0
mul r3, r1, r1 | mul r7, r5, r5
mul r0, r3, r3 | mul r4, r7, r7
mul r1, r0, r0 | mul r5, r4, r4
mul r3, r1, r1 | mul r7, r5, r5
mul r0, r3, r3 | mul r4, r7, r7
mul r1, r0, #6.103515625e-05 | mul r5, r4, #6.103515625e-05 | ld r0, [c06d]
mul r3, r2, r1 | mul r7, r6, r5 | ld r4, [c06d] | st r0, [0196]
min r2, r3, #3.3569336665095761e-05 | st r4, [01c0] | ld r3, [0187]
min r6, r7, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [0189] | ld r7, [01b1]
shl r5, r6, 14 | ld r6, [01b3]
mul r0, r1, r2 | mul r4, r5, r6
addxy r0, r0, r3 | addxy r4, r4, r7
mov r3, r0 | mov r7, r4 | st r0, [018c] | ld r0, [018a]
mad r3, r0, #-0.0625 | st r4, [01b6] | ld r4, [01b4]
mad r7, r4, #-0.0625 | ld r0, [0196] | ld r4, [01c0]
mac_p1wc3 r3, r0, r2, #0.5, r3
mac_p1wc3 r7, r4, r6, #0.5, r7
mac_p1wc3 r3, r1, r3, #-0.5, r2 | ld r2, [018c]
mac_p1wc3 r7, r5, r7, #-0.5, r6 | ld r6, [01b6] | st r1, [018c]
mac_p1wc3 r2, r1, r3, #-0.5, r2 | st r5, [01b6]
mac_p1wc3 r6, r5, r7, #-0.5, r6 | st r2, [018e]
mul r1, r2, #-2.0 | mul r5, r6, #-2.0 | st r6, [01b8]
st r1, [0186] | st r5, [01b0]
mov r1, r2 | mov r5, r6 | ld r2, [018a] | ld r6, [01b4]
mad r1, r2, #0.03125 | mad r5, r6, #0.03125
mac_p1wc3 r2, r0, r3, #0.5, r1 | ld r0, [018c]
mac_p1wc3 r6, r4, r7, #0.5, r5 | ld r4, [01b6]
mac_p1wc3 r1, r0, r2, #-0.5, r3 | ld r2, [01a2] | ld r3, [018e]
mac_p1wc3 r5, r4, r6, #-0.5, r7 | ld r6, [01cc] | ld r7, [01b8]
mul r0, r1, #4.0 | mul r4, r5, #4.0
mul r1, r2, r3 | mul r5, r6, r7 | st r0, [0188]
mul r0, r1, #-32.0 | mul r4, r5, #-32.0 | st r4, [01b2]
st r0, [0408] | st r4, [040a]
