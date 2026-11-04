ld r2, [a03a] | ld r3, [c088]
ld r6, [a03b] | ld r7, [c088] | st r3, [01de]
st r2, [a162] | ld r2, [c04b]
st r6, [a163] | ld r6, [c04b]
ld r0, [a062] | st r7, [0208] | ld r3, [c081]
ld r4, [a063] | ld r7, [c081] | st r3, [01e8]
ld r1, [a122] | st r7, [0212] | ld r3, [c08b]
mul r1, r1, r2 | ld r5, [a123] | ld r2, [c05e] | st r3, [01ea]
mul r5, r5, r6 | st r0, [a16a] | ld r6, [c05e]
st r4, [a16b] | ld r7, [c08b]
ld r0, [a12a] | st r7, [0214] | ld r3, [c084]
mad r1, r0, r2 | ld r4, [a12b] | ld r2, [c05f] | st r3, [01ec]
mad r5, r4, r6 | addxy r1, r1, r2 | ld r6, [c05f]
addxy r5, r5, r6 | mov r0, #0.041666666666666664 | ld r2, [a032]
addxy r2, r2, r1 | mov r4, #0.041666666666666664 | ld r6, [a033]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [0314] | ld r7, [c084]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [031e] | ld r3, [c08c]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5 | st r7, [0216] | ld r7, [c08c]
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0 | st r3, [01ee] | ld r3, [c085]
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a042] | st r7, [0218]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a043] | st r3, [01f0]
mul r4, r4, r4 | mul r0, r0, r0 | ld r7, [c085]
mul r4, r4, r4 | mul r0, r0, r0 | st r7, [021a] | ld r3, [c08d]
mul r4, r4, r4 | mul r2, r2, r0 | ld r7, [c08d] | st r3, [01f2]
mul r6, r6, r4 | mul r2, r1, #-0.08664339756999316 | st r2, [0316] | ld r1, [031b] | ld r3, [c079]
mul r6, r5, #-0.08664339756999316 | mov r0, #0.16666666666666666 | st r6, [0320] | ld r5, [0325]
mov r4, #0.16666666666666666 | ymacd r0, r2, #0.5 | st r7, [021c] | ld r7, [c079]
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a04a]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a04b]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mov r0, #0.0 | st r2, [0318] | ld r2, [a042]
addxy r2, r1, r2 | mov r4, #0.0 | st r6, [0322] | ld r6, [a043]
addxy r6, r5, r6 | st r0, [031c] | ld r0, [0316]
ph_wrap r1, r3, r2 | ld r2, [c080] | st r4, [0326] | ld r4, [0320]
ph_wrap r5, r3, r6 | ld r6, [c080] | st r2, [01e0]
st r6, [020a] | ld r2, [c089]
ld r6, [c089] | st r2, [01e2]
st r6, [020c] | ld r2, [c082]
ld r6, [c082] | st r2, [01e4]
st r6, [020e] | ld r2, [c08a]
ld r6, [c08a] | st r2, [01e6]
st r6, [0210] | ld r2, [c083]
ld r6, [c083] | st r2, [01f4]
st r6, [021e] | ld r2, [c07b]
mul r3, r3, r2 | mul r1, r1, #256.0 | ld r6, [c07b] | st r1, [031a]
mul r7, r7, r6 | mul r5, r5, #256.0 | ld r2, [c072] | st r5, [0324]
mul r3, r3, r2 | ph_clamp r1, r3, -r1 | ld r6, [c072]
mul r7, r7, r6 | mul r2, r0, r3 | st r1, [a132] | ld r1, [0318]
mul r6, r4, r7 | st r1, [01fc]
ph_clamp r5, r3, -r5
shl r3, r2, 10 | st r5, [a133] | ld r5, [0322]
shl r7, r6, 10 | min r2, r3, #0.25 | ld r3, [c07a] | st r5, [0226]
min r6, r7, #0.25 | ld r7, [c07a] | st r2, [01f6]
mul r3, r1, r3 | mul r7, r5, r7 | st r6, [0220] | ld r2, [c07c]
mul r3, r3, r2 | ld r6, [c07c]
mul r7, r7, r6 | ld r2, [c073]
mul r3, r3, r2 | ld r6, [c073]
mul r7, r7, r6 | mul r2, r3, #256.0 | ld r3, [a13a]
mul r6, r7, #256.0 | st r2, [01f8] | ld r2, [01d5]
mad r0, r2, #0.5 | st r6, [0222] | ld r6, [01ff]
mad r4, r6, #0.5 | mul r2, r0, #2.0 | ld r7, [a13b]
mul r6, r4, #2.0
ph_wrap r0, r3, r2
ph_wrap r4, r3, r6 | st r0, [01d4]
addxy r2, r3, #0.72547136654093203 | addxy r6, r7, #0.72547136654093203 | st r4, [01fe]
st r0, [01fa] | st r4, [0224]
mul r2, r2, r2 | mul r6, r6, r6
shl r0, r1, 11 | ld r1, [01e8]
shl r4, r5, 11 | ld r5, [0212]
mul r2, r2, r2 | mul r6, r6, r6
mul r0, r0, #1.1080034062384143 | mul r4, r4, #1.1080034062384143
min r2, r2, r0 | ld r0, [01f4]
min r6, r6, r4 | ld r4, [021e]
mac_p1wc3 r0, r0, r2, #0.27700085155960358, r1 | ld r2, [01e0]
mac_p1wc3 r4, r4, r6, #0.27700085155960358, r5 | ld r6, [020a] | st r0, [01f4]
mac_p1wc3 r0, r1, r3, #0.066666666666666666, r2 | st r4, [021e] | ld r2, [01fc]
mac_p1wc3 r4, r5, r7, #0.066666666666666666, r6 | ld r6, [0226] | ld r1, [01e4]
addxy r2, r2, #-0.000244140625 | addxy r6, r6, #-0.000244140625 | ld r5, [020e]
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
addxy r2, r0, #0.033333333333333333 | addxy r6, r4, #0.033333333333333333 | ld r0, [01fa] | ld r4, [0224]
addxy r0, r0, #1.0 | addxy r4, r4, #1.0
mac_p1wc3 r2, r2, r0, #-0.17008187206293277, r1
mac_p1wc3 r6, r6, r4, #-0.17008187206293277, r5
mul r1, r2, #15.0 | mul r5, r6, #15.0
floor r1, r1
floor r5, r5 | st r1, [01fa]
mad r2, r1, #-0.066666666666666666 | mad r6, r5, #-0.066666666666666666 | st r5, [0224] | ld r1, [01f4]
addxy r0, r3, #-0.17011031009890953 | mul r1, r1, r2 | ld r5, [021e] | st r0, [01f4]
addxy r4, r7, #-0.17011031009890953 | mul r0, r3, r0 | st r4, [021e]
mul r4, r7, r4 | addxy r0, r0, #0.72448098229473701
addxy r4, r4, #0.72448098229473701 | mul r3, r3, r0
mul r7, r7, r4 | mul r5, r5, r6
addxy r0, r3, #0.93103060483042199 | addxy r4, r7, #0.93103060483042199 | ld r3, [01ec] | ld r7, [0216]
st r1, [01ec] | st r5, [0216] | ld r1, [01f0]
mul r3, r3, r0 | mul r7, r7, r4 | ld r5, [021a]
mad r1, r3, #0.53299000445677069 | mad r5, r7, #0.53299000445677069 | ld r3, [c086]
mul r3, r3, r0 | ld r7, [c086]
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.017766333481892358, r3 | ld r3, [01ec]
mac_p1wc3 r4, r5, r6, #0.017766333481892358, r7 | ld r7, [0216]
mul r1, r0, #0.27700085155960358 | mul r5, r4, #0.27700085155960358
addxy r2, r1, #0.0092333617186534551 | addxy r6, r5, #0.0092333617186534551
min r0, r3, r2 | ld r3, [01e8] | ld r2, [01f4]
min r4, r7, r6 | ld r7, [0212] | ld r6, [021e]
min r1, r0, #0.0092333617186534534
min r5, r4, #0.0092333617186534534
mul r0, r3, r2 | mul r4, r7, r6 | ld r3, [c087] | ld r2, [01fa] | ld r6, [0224]
mac_p1wc3 r3, r3, r2, #-0.5, r0 | ld r7, [c087] | ld r2, [01e0]
mac_p1wc3 r7, r7, r6, #-0.5, r4 | ld r6, [020a]
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
addxy r2, r3, #0.5 | addxy r6, r7, #0.5 | ld r3, [a142]
mul r0, r1, r2 | mul r4, r5, r6 | ld r1, [01d7] | ld r5, [0201] | ld r7, [a143]
ld r2, [01f6] | ld r6, [0220]
mad r2, r1, #0.25 | mad r6, r5, #0.25 | st r0, [01f6]
mul r1, r2, #4.0 | mul r5, r6, #4.0 | st r4, [0220]
ph_wrap r2, r3, r1
ph_wrap r6, r3, r5 | st r2, [01d6]
max r1, r2, -r2 | st r6, [0200]
max r5, r6, -r6 | st r1, [01e0] | ld r1, [01f8]
addxy r0, r3, #0.72547136654093203 | addxy r4, r7, #0.72547136654093203 | st r5, [020a] | ld r5, [0222]
st r2, [01fa] | st r6, [0224]
mul r0, r0, r0 | mul r4, r4, r4
shl r2, r1, 14 | ld r1, [01e2]
shl r6, r5, 14 | ld r5, [020c]
mul r0, r0, r0 | mul r4, r4, r4
mul r2, r2, #1.1080034062384143 | mul r6, r6, #1.1080034062384143
min r0, r0, r2 | ld r2, [01ea]
min r4, r4, r6 | ld r6, [0214]
mac_p1wc3 r2, r2, r0, #0.27700085155960358, r1 | ld r0, [01de]
mac_p1wc3 r6, r6, r4, #0.27700085155960358, r5 | ld r4, [0208] | st r2, [01ea]
mac_p1wc3 r2, r1, r3, #0.066666666666666666, r0 | st r6, [0214] | ld r0, [01f8]
mac_p1wc3 r6, r5, r7, #0.066666666666666666, r4 | ld r4, [0222] | ld r1, [01e6]
addxy r0, r0, #-3.0517578125e-05 | addxy r4, r4, #-3.0517578125e-05 | ld r5, [0210]
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
addxy r0, r2, #0.033333333333333333 | addxy r4, r6, #0.033333333333333333 | ld r2, [01fa] | ld r6, [0224]
addxy r2, r2, #1.0 | addxy r6, r6, #1.0
mac_p1wc3 r0, r0, r2, #-0.17008187206293277, r1
mac_p1wc3 r4, r4, r6, #-0.17008187206293277, r5
mul r2, r0, #15.0 | mul r6, r4, #15.0
floor r1, r2 | ld r2, [01ea]
floor r5, r6 | ld r6, [0214] | st r1, [01ea]
mad r0, r1, #-0.066666666666666666 | mad r4, r5, #-0.066666666666666666 | st r5, [0214]
addxy r1, r3, #-0.17011031009890953 | addxy r5, r7, #-0.17011031009890953
mul r1, r3, r1 | mul r5, r7, r5
addxy r1, r1, #0.72448098229473701 | addxy r5, r5, #0.72448098229473701
mul r3, r3, r1 | mul r7, r7, r5
mul r2, r2, r0 | mul r6, r6, r4
addxy r1, r3, #0.93103060483042199 | addxy r5, r7, #0.93103060483042199 | ld r3, [01ee] | ld r7, [0218]
st r2, [01ee] | st r6, [0218] | ld r2, [01f2]
mul r3, r3, r1 | mul r7, r7, r5 | ld r6, [021c]
mad r2, r3, #0.53299000445677069 | mad r6, r7, #0.53299000445677069 | ld r3, [c08e]
mul r3, r3, r1 | ld r7, [c08e]
mul r7, r7, r5
mac_p1wc3 r1, r2, r0, #0.017766333481892358, r3 | ld r3, [01ee]
mac_p1wc3 r5, r6, r4, #0.017766333481892358, r7 | ld r7, [0218]
mul r2, r1, #0.27700085155960358 | mul r6, r5, #0.27700085155960358
addxy r0, r2, #0.0092333617186534551 | addxy r4, r6, #0.0092333617186534551
min r1, r3, r0 | ld r3, [01e2] | ld r0, [01e0]
min r5, r7, r4 | ld r7, [020c] | ld r4, [020a]
min r2, r1, #0.0092333617186534534
min r6, r5, #0.0092333617186534534
mul r1, r3, r0 | mul r5, r7, r4 | ld r3, [c08f] | ld r0, [01ea] | ld r4, [0214]
mul r3, r3, r0 | ld r7, [c08f]
mul r7, r7, r4 | subxy r0, r1, r3 | ld r1, [01de]
subxy r4, r5, r7 | mul r3, r1, r2 | ld r5, [0208]
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
addxy r1, r0, #0.5 | addxy r5, r4, #0.5 | ld r0, [01d9] | ld r4, [0203]
mul r3, r2, r1 | mul r7, r6, r5
addxy r2, r0, #0.084514582991543796 | addxy r6, r4, #0.084514582991543796
mul r1, r2, #7.3127498626708984 | mul r5, r6, #7.3127498626708984 | ld r2, [c07f]
ph_wrap r0, r3, r1 | ld r1, [c07e]
ph_wrap r4, r3, r5 | st r0, [01d8] | ld r5, [c07e]
st r4, [0202] | st r0, [01de] | ld r0, [01f6]
mul r1, r1, r0 | st r4, [0208] | ld r4, [0220] | ld r6, [c07f]
mul r5, r5, r4 | mad r2, r1, #-0.14540000259876251 | ld r0, [01de] | ld r4, [0208]
mad r6, r5, #-0.14540000259876251 | mul r1, r3, r2 | ld r2, [c077]
mul r5, r7, r6 | mul r2, r2, r0 | ld r6, [c077]
mul r6, r6, r4 | ld r3, [c078]
addxy r0, r2, r3 | ld r7, [c078]
addxy r4, r6, r7 | mul r2, r1, r0 | ld r3, [c075] | ld r0, [01f6]
mul r6, r5, r4 | ld r7, [c075] | ld r4, [0220]
ld r1, [c076]
mac_p1w r0, r0, r3, r2, r1 | ld r5, [c076] | ld r3, [031c]
mac_p1w r4, r4, r7, r6, r5 | ld r7, [0326] | ld r1, [a1ba]
mad r3, r0, #-0.072700001299381256 | mad r7, r4, #-0.072700001299381256 | ld r5, [a1bb]
ld r2, [a1d2] | st r3, [01de]
mul r0, r1, r2 | ld r6, [a1d3] | st r7, [0208] | ld r2, [c071]
mul r4, r5, r6 | ld r3, [0314] | ld r7, [031e] | ld r1, [a1c2]
st r0, [01f6] | st r4, [0220]
addxy r3, r3, #-4.75 | addxy r7, r7, #-4.75 | ld r0, [c074]
mul r0, r0, r3 | ld r4, [c074] | ld r5, [a1c3]
mul r4, r4, r7 | ld r6, [c071]
mac_p1wc3 r3, r1, r2, #0.125, r0 | ld r1, [a1da] | ld r2, [c070]
mac_p1wc3 r7, r5, r6, #0.125, r4 | ld r5, [a1db] | ld r6, [c070]
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
mul r3, r2, r1 | mul r7, r6, r5 | ld r4, [c07d] | st r0, [01ea]
min r2, r3, #3.3569336665095761e-05 | st r4, [0214] | ld r3, [01db]
min r6, r7, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [01dd] | ld r7, [0205]
shl r5, r6, 14 | ld r6, [0207]
mul r0, r1, r2 | mul r4, r5, r6
addxy r0, r0, r3 | addxy r4, r4, r7
mov r3, r0 | mov r7, r4 | st r0, [01e0] | ld r0, [01de]
mad r3, r0, #-0.0625 | st r4, [020a] | ld r4, [0208]
mad r7, r4, #-0.0625 | ld r0, [01ea] | ld r4, [0214]
mac_p1wc3 r3, r0, r2, #0.5, r3
mac_p1wc3 r7, r4, r6, #0.5, r7
mac_p1wc3 r3, r1, r3, #-0.5, r2 | ld r2, [01e0]
mac_p1wc3 r7, r5, r7, #-0.5, r6 | ld r6, [020a] | st r1, [01e0]
mac_p1wc3 r2, r1, r3, #-0.5, r2 | st r5, [020a]
mac_p1wc3 r6, r5, r7, #-0.5, r6 | st r2, [01e2]
mul r1, r2, #-2.0 | mul r5, r6, #-2.0 | st r6, [020c]
st r1, [01da] | st r5, [0204]
mov r1, r2 | mov r5, r6 | ld r2, [01de] | ld r6, [0208]
mad r1, r2, #0.03125 | mad r5, r6, #0.03125
mac_p1wc3 r2, r0, r3, #0.5, r1 | ld r0, [01e0]
mac_p1wc3 r6, r4, r7, #0.5, r5 | ld r4, [020a]
mac_p1wc3 r1, r0, r2, #-0.5, r3 | ld r2, [01f6] | ld r3, [01e2]
mac_p1wc3 r5, r4, r6, #-0.5, r7 | ld r6, [0220] | ld r7, [020c]
mul r0, r1, #4.0 | mul r4, r5, #4.0
mul r1, r2, r3 | mul r5, r6, r7 | st r0, [01dc] | ld r2, [a03c]
mul r0, r1, #-32.0 | mul r4, r5, #-32.0 | st r4, [0206] | ld r6, [a03d]
st r0, [a153]
st r4, [a154]
st r2, [a164] | ld r2, [c04b]
st r6, [a165] | ld r6, [c04b]
ld r0, [a064]
ld r4, [a065]
ld r1, [a124]
mul r1, r1, r2 | ld r5, [a125] | ld r2, [c05e]
mul r5, r5, r6 | st r0, [a16c] | ld r6, [c05e]
st r4, [a16d]
ld r0, [a12c]
mad r1, r0, r2 | ld r4, [a12d] | ld r2, [c05f]
mad r5, r4, r6 | addxy r1, r1, r2 | ld r6, [c05f]
addxy r5, r5, r6 | mov r0, #0.041666666666666664 | ld r2, [a034]
addxy r2, r2, r1 | mov r4, #0.041666666666666664 | ld r6, [a035]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [0328]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [0332]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a044]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a045]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mul r2, r1, #-0.08664339756999316 | st r2, [032a] | ld r1, [032f]
mul r6, r5, #-0.08664339756999316 | mov r0, #0.16666666666666666 | st r6, [0334] | ld r5, [0339]
mov r4, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a04c]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a04d]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mov r0, #0.0 | st r2, [032c] | ld r2, [a044]
addxy r2, r1, r2 | mov r4, #0.0 | st r6, [0336] | ld r6, [a045]
addxy r6, r5, r6 | st r0, [0330] | ld r0, [032a]
ph_wrap r1, r3, r2 | ld r2, [0328] | st r4, [033a] | st r0, [a1b4]
ph_wrap r5, r3, r6 | st r1, [032e] | ld r6, [0332] | st r2, [a1e4]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [0338] | st r6, [a1e5] | ld r4, [0334]
ph_clamp r1, r3, -r1 | ld r2, [a03e] | ld r3, [032c] | ld r7, [0336]
ph_clamp r5, r3, -r5 | st r1, [a134] | ld r1, [0330]
st r5, [a135] | ld r5, [033a]
ld r6, [a03f]
st r4, [a1b5]
st r1, [a1cc]
st r5, [a1cd]
st r2, [a166] | ld r2, [c04b]
st r6, [a167] | ld r6, [c04b]
ld r0, [a066]
ld r4, [a067]
ld r1, [a126]
mul r1, r1, r2 | ld r5, [a127] | ld r2, [c05e]
mul r5, r5, r6 | st r0, [a16e] | ld r6, [c05e]
st r4, [a16f]
ld r0, [a12e]
mad r1, r0, r2 | ld r4, [a12f] | ld r2, [c05f]
mad r5, r4, r6 | addxy r1, r1, r2 | ld r6, [c05f]
addxy r5, r5, r6 | mov r0, #0.041666666666666664 | ld r2, [a036]
addxy r2, r2, r1 | mov r4, #0.041666666666666664 | ld r6, [a037]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [033c] | st r3, [a1ec]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [0346] | st r7, [a1ed]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a046]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a047]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mul r2, r1, #-0.08664339756999316 | st r2, [033e] | ld r1, [0343]
mul r6, r5, #-0.08664339756999316 | mov r0, #0.16666666666666666 | st r6, [0348] | ld r5, [034d]
mov r4, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a04e]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a04f]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mov r0, #0.0 | st r2, [0340] | ld r2, [a046]
addxy r2, r1, r2 | mov r4, #0.0 | st r6, [034a] | ld r6, [a047]
addxy r6, r5, r6 | st r0, [0344] | ld r0, [033e]
ph_wrap r1, r3, r2 | st r4, [034e] | ld r4, [0348] | st r0, [a1b6]
ph_wrap r5, r3, r6 | st r1, [0342] | ld r2, [033c] | st r4, [a1b7]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [034c] | ld r6, [0346] | st r2, [a1e6]
ph_clamp r1, r3, -r1 | ld r3, [0340] | ld r7, [034a]
ph_clamp r5, r3, -r5 | st r1, [a136] | ld r1, [0344]
st r5, [a137] | ld r5, [034e]
st r1, [a1ce]
st r5, [a1cf]
st r6, [a1e7]
st r3, [a1ee]
st r7, [a1ef]
