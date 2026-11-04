ld r2, [a038] | ld r3, [c088]
ld r6, [a039] | ld r7, [c088] | st r3, [018a]
st r2, [a160] | ld r2, [c04b]
st r6, [a161] | ld r6, [c04b]
ld r0, [a060] | st r7, [01b4] | ld r3, [c081]
ld r4, [a061] | ld r7, [c081] | st r3, [0194]
ld r1, [a120] | st r7, [01be] | ld r3, [c08b]
mul r1, r1, r2 | ld r5, [a121] | ld r2, [c05e] | st r3, [0196]
mul r5, r5, r6 | st r0, [a168] | ld r6, [c05e]
st r4, [a169] | ld r7, [c08b]
ld r0, [a128] | st r7, [01c0] | ld r3, [c084]
mad r1, r0, r2 | ld r4, [a129] | ld r2, [c05f] | st r3, [0198]
mad r5, r4, r6 | addxy r1, r1, r2 | ld r6, [c05f]
addxy r5, r5, r6 | mov r0, #0.041666666666666664 | ld r2, [a030]
addxy r2, r2, r1 | mov r4, #0.041666666666666664 | ld r6, [a031]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [0300] | ld r7, [c084]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [030a] | ld r3, [c08c]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5 | st r7, [01c2] | ld r7, [c08c]
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0 | st r3, [019a] | ld r3, [c085]
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a040] | st r7, [01c4]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a041] | st r3, [019c]
mul r4, r4, r4 | mul r0, r0, r0 | ld r7, [c085]
mul r4, r4, r4 | mul r0, r0, r0 | st r7, [01c6] | ld r3, [c08d]
mul r4, r4, r4 | mul r2, r2, r0 | ld r7, [c08d] | st r3, [019e]
mul r6, r6, r4 | mul r2, r1, #-0.08664339756999316 | st r2, [0302] | ld r1, [0307] | ld r3, [c079]
mul r6, r5, #-0.08664339756999316 | mov r0, #0.16666666666666666 | st r6, [030c] | ld r5, [0311]
mov r4, #0.16666666666666666 | ymacd r0, r2, #0.5 | st r7, [01c8] | ld r7, [c079]
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a048]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a049]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mov r0, #0.0 | st r2, [0304] | ld r2, [a040]
addxy r2, r1, r2 | mov r4, #0.0 | st r6, [030e] | ld r6, [a041]
addxy r6, r5, r6 | st r0, [0308] | ld r0, [0302]
ph_wrap r1, r3, r2 | ld r2, [c080] | st r4, [0312] | ld r4, [030c]
ph_wrap r5, r3, r6 | ld r6, [c080] | st r2, [018c]
st r6, [01b6] | ld r2, [c089]
ld r6, [c089] | st r2, [018e]
st r6, [01b8] | ld r2, [c082]
ld r6, [c082] | st r2, [0190]
st r6, [01ba] | ld r2, [c08a]
ld r6, [c08a] | st r2, [0192]
st r6, [01bc] | ld r2, [c083]
ld r6, [c083] | st r2, [01a0]
st r6, [01ca] | ld r2, [c07b]
mul r3, r3, r2 | mul r1, r1, #256.0 | ld r6, [c07b] | st r1, [0306]
mul r7, r7, r6 | mul r5, r5, #256.0 | ld r2, [c072] | st r5, [0310]
mul r3, r3, r2 | ph_clamp r1, r3, -r1 | ld r6, [c072]
mul r7, r7, r6 | mul r2, r0, r3 | st r1, [a130] | ld r1, [0304]
mul r6, r4, r7 | st r1, [01a8]
ph_clamp r5, r3, -r5
shl r3, r2, 10 | st r5, [a131] | ld r5, [030e]
shl r7, r6, 10 | min r2, r3, #0.25 | ld r3, [c07a] | st r5, [01d2]
min r6, r7, #0.25 | ld r7, [c07a] | st r2, [01a2]
mul r3, r1, r3 | mul r7, r5, r7 | st r6, [01cc] | ld r2, [c07c]
mul r3, r3, r2 | ld r6, [c07c]
mul r7, r7, r6 | ld r2, [c073]
mul r3, r3, r2 | ld r6, [c073]
mul r7, r7, r6 | mul r2, r3, #256.0 | ld r3, [a138]
mul r6, r7, #256.0 | st r2, [01a4] | ld r2, [0181]
mad r0, r2, #0.5 | st r6, [01ce] | ld r6, [01ab]
mad r4, r6, #0.5 | mul r2, r0, #2.0 | ld r7, [a139]
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
mad r1, r3, #0.53299000445677069 | mad r5, r7, #0.53299000445677069 | ld r3, [c086]
mul r3, r3, r0 | ld r7, [c086]
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.017766333481892358, r3 | ld r3, [0198]
mac_p1wc3 r4, r5, r6, #0.017766333481892358, r7 | ld r7, [01c2]
mul r1, r0, #0.27700085155960358 | mul r5, r4, #0.27700085155960358
addxy r2, r1, #0.0092333617186534551 | addxy r6, r5, #0.0092333617186534551
min r0, r3, r2 | ld r3, [0194] | ld r2, [01a0]
min r4, r7, r6 | ld r7, [01be] | ld r6, [01ca]
min r1, r0, #0.0092333617186534534
min r5, r4, #0.0092333617186534534
mul r0, r3, r2 | mul r4, r7, r6 | ld r3, [c087] | ld r2, [01a6] | ld r6, [01d0]
mac_p1wc3 r3, r3, r2, #-0.5, r0 | ld r7, [c087] | ld r2, [018c]
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
addxy r2, r3, #0.5 | addxy r6, r7, #0.5 | ld r3, [a140]
mul r0, r1, r2 | mul r4, r5, r6 | ld r1, [0183] | ld r5, [01ad] | ld r7, [a141]
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
mad r2, r3, #0.53299000445677069 | mad r6, r7, #0.53299000445677069 | ld r3, [c08e]
mul r3, r3, r1 | ld r7, [c08e]
mul r7, r7, r5
mac_p1wc3 r1, r2, r0, #0.017766333481892358, r3 | ld r3, [019a]
mac_p1wc3 r5, r6, r4, #0.017766333481892358, r7 | ld r7, [01c4]
mul r2, r1, #0.27700085155960358 | mul r6, r5, #0.27700085155960358
addxy r0, r2, #0.0092333617186534551 | addxy r4, r6, #0.0092333617186534551
min r1, r3, r0 | ld r3, [018e] | ld r0, [018c]
min r5, r7, r4 | ld r7, [01b8] | ld r4, [01b6]
min r2, r1, #0.0092333617186534534
min r6, r5, #0.0092333617186534534
mul r1, r3, r0 | mul r5, r7, r4 | ld r3, [c08f] | ld r0, [0196] | ld r4, [01c0]
mul r3, r3, r0 | ld r7, [c08f]
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
mul r1, r2, #7.3127498626708984 | mul r5, r6, #7.3127498626708984 | ld r2, [c07f]
ph_wrap r0, r3, r1 | ld r1, [c07e]
ph_wrap r4, r3, r5 | st r0, [0184] | ld r5, [c07e]
st r4, [01ae] | st r0, [018a] | ld r0, [01a2]
mul r1, r1, r0 | st r4, [01b4] | ld r4, [01cc] | ld r6, [c07f]
mul r5, r5, r4 | mad r2, r1, #-0.14540000259876251 | ld r0, [018a] | ld r4, [01b4]
mad r6, r5, #-0.14540000259876251 | mul r1, r3, r2 | ld r2, [c077]
mul r5, r7, r6 | mul r2, r2, r0 | ld r6, [c077]
mul r6, r6, r4 | ld r3, [c078]
addxy r0, r2, r3 | ld r7, [c078]
addxy r4, r6, r7 | mul r2, r1, r0 | ld r3, [c075] | ld r0, [01a2]
mul r6, r5, r4 | ld r7, [c075] | ld r4, [01cc]
ld r1, [c076]
mac_p1w r0, r0, r3, r2, r1 | ld r5, [c076] | ld r3, [0308]
mac_p1w r4, r4, r7, r6, r5 | ld r7, [0312] | ld r1, [a1b8]
mad r3, r0, #-0.072700001299381256 | mad r7, r4, #-0.072700001299381256 | ld r5, [a1b9]
ld r2, [a1d0] | st r3, [018a]
mul r0, r1, r2 | ld r6, [a1d1] | st r7, [01b4] | ld r2, [c071]
mul r4, r5, r6 | ld r3, [0300] | ld r7, [030a] | ld r1, [a1c0]
st r0, [01a2] | st r4, [01cc]
addxy r3, r3, #-4.75 | addxy r7, r7, #-4.75 | ld r0, [c074]
mul r0, r0, r3 | ld r4, [c074] | ld r5, [a1c1]
mul r4, r4, r7 | ld r6, [c071]
mac_p1wc3 r3, r1, r2, #0.125, r0 | ld r1, [a1d8] | ld r2, [c070]
mac_p1wc3 r7, r5, r6, #0.125, r4 | ld r5, [a1d9] | ld r6, [c070]
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
mul r1, r0, #6.103515625e-05 | mul r5, r4, #6.103515625e-05 | ld r0, [c07d]
mul r3, r2, r1 | mul r7, r6, r5 | ld r4, [c07d] | st r0, [0196]
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
st r0, [a151]
st r4, [a152]
