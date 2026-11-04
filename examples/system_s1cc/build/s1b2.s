ld r0, [a16e] | ld r1, [06cd] | ld r5, [06ed]
subxy r1, r1, r0 | ld r4, [a16f] | ld r3, [c096] | st r0, [06cc]
mul r1, r1, r1 | subxy r5, r5, r4 | ld r0, [c09e] | st r4, [06ec]
mul r5, r5, r5 | st r1, [06da] | ld r1, [c050]
mul r1, r3, r1 | ld r2, [c051] | st r5, [06fa]
mad r1, r0, -r2 | ld r7, [c097] | ld r2, [06df] | ld r0, [052d]
add r1, #1.0 | ld r5, [c050] | st r3, [a126]
mul r5, r7, r5 | ld r4, [c09f] | st r1, [06d6]
ld r6, [c051] | st r3, [051c]
mad r5, r4, -r6 | ld r1, [c04f] | ld r6, [06ff] | ld r4, [052f]
add r5, #1.0 | st r7, [a127] | st r7, [051e]
mul r1, r3, r1 | st r5, [06f6] | ld r5, [c04f]
mul r5, r7, r5 | st r1, [06d8] | ld r1, [06da] | ld r3, [c053]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | st r5, [06f8] | ld r5, [06fa] | ld r7, [c053]
mul r1, r1, #256.0 | mad r6, r6, -r5
addxy r5, r4, #-0.999
ph_clamp r1, r3, -r1
mul r5, r5, #256.0 | mul r1, r1, #256.0
ph_clamp r5, r3, -r5
ph_clamp r1, r3, -r1
mul r5, r5, #256.0
max r2, r2, r1 | ph_clamp r5, r3, -r5 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [06de]
max r6, r6, r5 | ld r5, [c052]
mul r2, r2, #256.0 | subxy r6, r6, #1.0 | st r6, [06fe]
ph_clamp r2, r3, -r2
mul r6, r6, #256.0 | mad r1, r2, r3 | ld r3, [c078]
ph_clamp r6, r3, -r6 | st r3, [01de] | ld r3, [c071]
subxy r7, r5, r7 | mul r2, r2, #1.25 | st r3, [01e8] | ld r3, [c07b]
mad r5, r6, r7 | subxy r2, r0, r2 | ld r7, [c078] | st r3, [01ea]
mul r6, r6, #1.25 | mad r0, r2, r1 | ld r2, [c070] | st r7, [0208]
subxy r6, r4, r6 | ld r7, [c071] | st r2, [01e0] | ld r1, [a136]
mad r4, r6, r5 | ld r2, [c079] | st r7, [0212] | ld r5, [a137]
min r0, r0, #1.0 | ld r3, [c074] | st r2, [01e2]
min r4, r4, #1.0 | ld r6, [c070] | st r3, [01ec]
ld r7, [c07b] | st r6, [020a]
ld r2, [c072] | st r7, [0214]
ld r3, [c07c] | st r2, [01e4]
ld r6, [c079] | st r3, [01ee]
ld r7, [c074] | st r6, [020c]
ld r2, [c07a] | st r7, [0216]
ld r3, [c075] | st r2, [01e6]
ld r6, [c072] | st r3, [01f0]
ld r7, [c07c] | st r6, [020e]
ld r2, [c073] | st r7, [0218]
ld r3, [c07d] | st r0, [052c]
st r0, [a12e] | ld r6, [c07a]
ld r7, [c075] | st r3, [01f2]
ld r0, [c086] | st r2, [01f4]
st r6, [0210] | st r7, [021a]
ld r3, [c069] | st r0, [054c]
ld r2, [c06b] | st r4, [052e]
mul r3, r3, r2 | ld r6, [c073] | st r4, [a12f]
ld r7, [c07d] | st r6, [021e]
ld r0, [c08e] | st r7, [021c]
ld r4, [c087] | st r0, [053c] | ld r0, [a1b6]
ld r2, [c062] | st r4, [054e]
mul r3, r3, r2 | ld r7, [c069] | st r1, [055c] | ld r1, [a1ee]
mul r2, r0, r3 | ld r6, [c06b] | st r5, [055e] | ld r5, [a1ef]
mul r7, r7, r6 | ld r4, [c08f] | st r1, [01fc]
shl r3, r2, 10 | ld r6, [c062] | st r4, [053e] | ld r4, [a1b7]
mul r7, r7, r6 | st r5, [0226]
mul r6, r4, r7
min r2, r3, #0.25 | shl r7, r6, 10 | ld r3, [c06a]
mul r3, r1, r3 | st r2, [01f6] | ld r2, [c06c]
min r6, r7, #0.25 | ld r7, [c06a]
mul r3, r3, r2 | mul r7, r5, r7 | st r6, [0220] | ld r2, [c063]
mul r3, r3, r2 | ld r6, [c06c]
mul r7, r7, r6 | mul r2, r3, #256.0 | ld r6, [c063]
mul r7, r7, r6 | st r2, [01f8] | ld r2, [01d5]
mul r6, r7, #256.0 | mad r0, r2, #0.5 | ld r3, [a13e]
mul r2, r0, #2.0 | st r6, [0222] | ld r6, [01ff]
mad r4, r6, #0.5 | ld r7, [a13f]
ph_wrap r0, r3, r2
addxy r2, r3, #0.72547136654093203 | mul r6, r4, #2.0 | st r0, [01d4]
mul r2, r2, r2 | st r0, [01fa]
shl r0, r1, 11 | ld r1, [01e8]
ph_wrap r4, r3, r6
addxy r6, r7, #0.72547136654093203 | mul r2, r2, r2 | st r4, [01fe]
mul r0, r0, #1.1080034062384143 | mul r6, r6, r6 | st r4, [0224]
shl r4, r5, 11 | ld r5, [0212]
min r2, r2, r0 | ld r0, [01f4]
mul r6, r6, r6 | mul r4, r4, #1.1080034062384143
mac_p1wc3 r0, r0, r2, #0.27700085155960358, r1 | ld r2, [01e0]
min r6, r6, r4 | ld r4, [021e] | st r0, [01f4]
mac_p1wc3 r0, r1, r3, #0.066666666666666666, r2 | ld r2, [01fc] | ld r1, [01e4]
mac_p1wc3 r4, r4, r6, #0.27700085155960358, r5 | ld r6, [020a]
addxy r2, r2, #-0.000244140625 | st r4, [021e]
mac_p1wc3 r4, r5, r7, #0.066666666666666666, r6 | ld r6, [0226] | ld r5, [020e]
mul r2, r2, #136.53333333333333 | addxy r6, r6, #-0.000244140625
min r0, r0, r2
addxy r2, r3, #-2.5549090593309853 | mul r6, r6, #136.53333333333333
mul r2, r3, r2
min r4, r4, r6
addxy r6, r7, #-2.5549090593309853 | addxy r2, r2, #2.8640197605209372
mul r6, r7, r6 | mul r3, r3, r2
addxy r6, r6, #2.8640197605209372 | addxy r2, r3, #-2.2379875792421422
mul r7, r7, r6 | mul r3, r2, r2
addxy r6, r7, #-2.2379875792421422 | mul r2, r3, r3
mul r7, r6, r6 | mul r3, r2, #0.03125
mul r6, r7, r7 | addxy r2, r3, #-0.78393618153496136
mul r7, r6, #0.03125 | mul r1, r1, r2
addxy r2, r0, #0.033333333333333333 | addxy r6, r7, #-0.78393618153496136 | ld r0, [01fa]
mul r5, r5, r6 | addxy r0, r0, #1.0
addxy r6, r4, #0.033333333333333333 | ld r4, [0224]
mac_p1wc3 r2, r2, r0, #-0.17008187206293277, r1
addxy r4, r4, #1.0 | mul r1, r2, #15.0
mac_p1wc3 r6, r6, r4, #-0.17008187206293277, r5
floor r1, r1
mul r5, r6, #15.0 | mad r2, r1, #-0.066666666666666666 | st r1, [01fa] | ld r1, [01f4]
floor r5, r5 | st r0, [01f4]
addxy r0, r3, #-0.17011031009890953 | mad r6, r5, #-0.066666666666666666 | st r5, [0224] | ld r5, [021e]
mul r0, r3, r0 | addxy r4, r7, #-0.17011031009890953 | st r4, [021e]
addxy r0, r0, #0.72448098229473701 | mul r4, r7, r4
mul r3, r3, r0 | mul r1, r1, r2
addxy r4, r4, #0.72448098229473701 | addxy r0, r3, #0.93103060483042199 | ld r3, [01ec]
mul r7, r7, r4 | mul r3, r3, r0 | st r1, [01ec] | ld r1, [01f0]
mul r5, r5, r6 | addxy r4, r7, #0.93103060483042199 | ld r7, [0216]
mad r1, r3, #0.53299000445677069 | mul r7, r7, r4 | st r5, [0216] | ld r3, [c076] | ld r5, [021a]
mul r3, r3, r0 | mad r5, r7, #0.53299000445677069 | ld r7, [c076]
mac_p1wc3 r0, r1, r2, #0.017766333481892358, r3 | ld r3, [01ec]
mul r7, r7, r4 | mul r1, r0, #0.27700085155960358
mac_p1wc3 r4, r5, r6, #0.017766333481892358, r7 | ld r7, [0216]
addxy r2, r1, #0.0092333617186534551 | mul r5, r4, #0.27700085155960358
min r0, r3, r2 | ld r3, [01e8] | ld r2, [01f4]
addxy r6, r5, #0.0092333617186534551
min r1, r0, #0.0092333617186534534
mul r0, r3, r2 | ld r3, [c077] | ld r2, [01fa]
min r4, r7, r6 | ld r7, [0212] | ld r6, [021e]
min r5, r4, #0.0092333617186534534
mac_p1wc3 r3, r3, r2, #-0.5, r0 | ld r2, [01e0]
mul r4, r7, r6 | mul r0, r2, r1 | ld r7, [c077] | ld r6, [0224]
mac_p1wc3 r7, r7, r6, #-0.5, r4 | ld r6, [020a]
subxy r2, r1, r0 | mul r4, r6, r5
addxy r1, r2, #-0.0023083404296633634 | subxy r6, r5, r4
mul r0, r1, #-216.60583230044517 | addxy r5, r6, #-0.0023083404296633634
ph_tri r2, r3, -r0
mul r4, r5, #-216.60583230044517 | mul r1, r2, r2
ph_tri r6, r3, -r4
addxy r0, r1, #-8.8404402339953698 | mul r5, r6, r6
mul r1, r1, r0 | addxy r4, r5, #-8.8404402339953698
addxy r0, r1, #21.601100021326904 | mul r5, r5, r4
mul r1, r2, r0 | addxy r2, r3, #0.5 | ld r3, [a146]
addxy r4, r5, #21.601100021326904 | mul r0, r1, r2 | ld r1, [01d7] | ld r2, [01f6]
mul r5, r6, r4 | addxy r6, r7, #0.5 | st r0, [01f6] | ld r7, [a147]
mad r2, r1, #0.25 | mul r4, r5, r6 | ld r5, [0201] | ld r6, [0220]
mul r1, r2, #4.0 | mad r6, r5, #0.25 | st r4, [0220]
ph_wrap r2, r3, r1
mul r5, r6, #4.0 | addxy r0, r3, #0.72547136654093203 | st r2, [01d6]
max r1, r2, -r2 | st r2, [01fa]
ph_wrap r6, r3, r5 | st r1, [01e0] | ld r1, [01f8]
max r5, r6, -r6 | st r6, [0200]
mul r0, r0, r0 | addxy r4, r7, #0.72547136654093203 | st r5, [020a] | ld r5, [0222]
shl r2, r1, 14 | st r6, [0224] | ld r1, [01e2]
mul r0, r0, r0 | mul r2, r2, #1.1080034062384143
mul r4, r4, r4
shl r6, r5, 14 | ld r5, [020c]
min r0, r0, r2 | ld r2, [01ea]
mul r4, r4, r4 | mul r6, r6, #1.1080034062384143
mac_p1wc3 r2, r2, r0, #0.27700085155960358, r1 | ld r0, [01de]
min r4, r4, r6 | ld r6, [0214] | st r2, [01ea]
mac_p1wc3 r2, r1, r3, #0.066666666666666666, r0 | ld r0, [01f8] | ld r1, [01e6]
mac_p1wc3 r6, r6, r4, #0.27700085155960358, r5 | ld r4, [0208]
addxy r0, r0, #-3.0517578125e-05 | st r6, [0214]
mac_p1wc3 r6, r5, r7, #0.066666666666666666, r4 | ld r4, [0222] | ld r5, [0210]
shl r0, r0, 10
addxy r4, r4, #-3.0517578125e-05 | mul r0, r0, #1.0666666666666667
shl r4, r4, 10
min r2, r2, r0
addxy r0, r3, #-2.5549090593309853 | mul r4, r4, #1.0666666666666667
mul r0, r3, r0
min r6, r6, r4
addxy r4, r7, #-2.5549090593309853 | addxy r0, r0, #2.8640197605209372
mul r4, r7, r4 | mul r3, r3, r0
addxy r4, r4, #2.8640197605209372 | addxy r0, r3, #-2.2379875792421422
mul r7, r7, r4 | mul r3, r0, r0
addxy r4, r7, #-2.2379875792421422 | mul r0, r3, r3
mul r7, r4, r4 | mul r3, r0, #0.03125
mul r4, r7, r7 | addxy r0, r3, #-0.78393618153496136
mul r7, r4, #0.03125 | mul r1, r1, r0
addxy r0, r2, #0.033333333333333333 | addxy r4, r7, #-0.78393618153496136 | ld r2, [01fa]
mul r5, r5, r4 | addxy r2, r2, #1.0
addxy r4, r6, #0.033333333333333333 | ld r6, [0224]
mac_p1wc3 r0, r0, r2, #-0.17008187206293277, r1
addxy r6, r6, #1.0 | mul r2, r0, #15.0
mac_p1wc3 r4, r4, r6, #-0.17008187206293277, r5
floor r1, r2 | ld r2, [01ea]
mul r6, r4, #15.0 | mad r0, r1, #-0.066666666666666666 | st r1, [01ea]
addxy r1, r3, #-0.17011031009890953 | mul r2, r2, r0
floor r5, r6 | ld r6, [0214]
mul r1, r3, r1 | mad r4, r5, #-0.066666666666666666 | st r5, [0214]
addxy r5, r7, #-0.17011031009890953 | addxy r1, r1, #0.72448098229473701
mul r5, r7, r5 | mul r3, r3, r1
addxy r5, r5, #0.72448098229473701 | addxy r1, r3, #0.93103060483042199 | ld r3, [01ee]
mul r7, r7, r5 | mul r3, r3, r1 | st r2, [01ee] | ld r2, [01f2]
mul r6, r6, r4 | addxy r5, r7, #0.93103060483042199 | ld r7, [0218]
mad r2, r3, #0.53299000445677069 | mul r7, r7, r5 | st r6, [0218] | ld r3, [c07e] | ld r6, [021c]
mul r3, r3, r1 | mad r6, r7, #0.53299000445677069 | ld r7, [c07e]
mac_p1wc3 r1, r2, r0, #0.017766333481892358, r3 | ld r3, [01ee]
mul r7, r7, r5 | mul r2, r1, #0.27700085155960358
mac_p1wc3 r5, r6, r4, #0.017766333481892358, r7 | ld r7, [0218]
addxy r0, r2, #0.0092333617186534551 | mul r6, r5, #0.27700085155960358
min r1, r3, r0 | ld r3, [01e2] | ld r0, [01e0]
addxy r4, r6, #0.0092333617186534551
min r2, r1, #0.0092333617186534534
mul r1, r3, r0 | ld r3, [c07f] | ld r0, [01ea]
min r5, r7, r4 | ld r7, [020c] | ld r4, [020a]
min r6, r5, #0.0092333617186534534
mul r3, r3, r0 | mul r5, r7, r4 | ld r7, [c07f] | ld r4, [0214]
subxy r0, r1, r3 | mul r7, r7, r4 | ld r1, [01de]
mul r3, r1, r2 | subxy r4, r5, r7 | ld r5, [0208]
subxy r1, r2, r3 | mul r7, r5, r6
addxy r2, r1, #-0.0023083404296633634 | subxy r5, r6, r7
mul r3, r2, #-216.60583230044517 | addxy r6, r5, #-0.0023083404296633634
ph_tri r1, r3, -r3
mul r7, r6, #-216.60583230044517 | mul r2, r1, r1
ph_tri r5, r3, -r7
addxy r3, r2, #-8.8404402339953698 | mul r6, r5, r5
mul r2, r2, r3 | addxy r7, r6, #-8.8404402339953698
addxy r3, r2, #21.601100021326904 | mul r6, r6, r7
mul r2, r1, r3 | addxy r1, r0, #0.5 | ld r0, [01d9]
addxy r7, r6, #21.601100021326904 | mul r3, r2, r1
mul r6, r5, r7 | addxy r2, r0, #0.084514582991543796
addxy r5, r4, #0.5 | mul r1, r2, #7.3127498626708984 | ld r4, [0203] | ld r2, [c06f]
mul r7, r6, r5 | addxy r6, r4, #0.084514582991543796
ph_wrap r0, r3, r1 | ld r1, [c06e]
mul r5, r6, #7.3127498626708984 | st r0, [01d8] | ld r6, [c06f]
ph_wrap r4, r3, r5 | st r0, [01de] | ld r0, [01f6] | ld r5, [c06e]
mul r1, r1, r0 | st r4, [0202] | ld r0, [01de]
mad r2, r1, #-0.14540000259876251 | st r4, [0208] | ld r4, [0220]
mul r5, r5, r4 | mul r1, r3, r2 | ld r2, [c067] | ld r4, [0208]
mad r6, r5, #-0.14540000259876251 | mul r2, r2, r0 | ld r3, [c068]
mul r5, r7, r6 | addxy r0, r2, r3 | ld r6, [c067]
mul r6, r6, r4 | mul r2, r1, r0 | ld r7, [c068] | ld r0, [01f6]
addxy r4, r6, r7 | ld r3, [c065]
mul r6, r5, r4 | ld r1, [c066] | ld r4, [0220]
mac_p1w r0, r0, r3, r2, r1 | ld r3, [a1ce] | ld r7, [c065]
mad r3, r0, #-0.072700001299381256 | ld r5, [c066] | ld r1, [054c] | ld r2, [06d6]
mac_p1w r4, r4, r7, r6, r5 | ld r7, [a1cf] | st r3, [01de]
mul r0, r1, r2 | mad r7, r4, #-0.072700001299381256 | ld r3, [a1e6] | ld r5, [054e] | ld r6, [06f6]
addxy r3, r3, #-4.75 | mul r4, r5, r6 | st r0, [01f6] | ld r0, [c064] | ld r1, [053c]
mul r0, r0, r3 | st r7, [0208] | ld r2, [c061] | ld r7, [a1e7]
addxy r7, r7, #-4.75 | st r4, [0220] | ld r4, [c064] | ld r5, [053e]
mac_p1wc3 r3, r1, r2, #0.125, r0 | ld r1, [06d8] | ld r6, [c061]
mul r4, r4, r7 | mad r3, r1, #0.125 | ld r2, [c060]
mac_p1wc3 r7, r5, r6, #0.125, r4 | ld r5, [06f8] | ld r6, [c060]
mul r0, r3, #0.17328679513998632 | mov r1, #0.041666666666666664
mad r7, r5, #0.125 | mov r5, #0.041666666666666664
ymacd r1, r0, #0.16666666666666666 | mul r4, r7, #0.17328679513998632
ymacd r1, r0, #0.5 | ymacd r5, r4, #0.16666666666666666
ymacd r1, r0, #1.0 | ymacd r5, r4, #0.5
ymacd r1, r0, #1.0 | ymacd r5, r4, #1.0
mul r3, r1, r1 | ymacd r5, r4, #1.0
mul r0, r3, r3 | mul r7, r5, r5
mul r1, r0, r0 | mul r4, r7, r7
mul r3, r1, r1 | mul r5, r4, r4
mul r0, r3, r3 | mul r7, r5, r5
mul r1, r0, #6.103515625e-05 | mul r4, r7, r7 | ld r0, [c06d]
mul r3, r2, r1 | mul r5, r4, #6.103515625e-05 | st r0, [01ea] | ld r4, [c06d]
min r2, r3, #3.3569336665095761e-05 | ld r3, [01db] | st r4, [0214]
mul r7, r6, r5
shl r1, r2, 14 | min r6, r7, #3.3569336665095761e-05 | ld r2, [01dd] | ld r7, [0205]
mul r0, r1, r2
shl r5, r6, 14 | ld r6, [0207]
addxy r0, r0, r3 | mul r4, r5, r6
mov r3, r0 | addxy r4, r4, r7 | st r0, [01e0] | ld r0, [01de]
mad r3, r0, #-0.0625 | st r4, [020a] | ld r0, [01ea]
mov r7, r4 | ld r4, [0208]
mac_p1wc3 r3, r0, r2, #0.5, r3
mad r7, r4, #-0.0625 | ld r4, [0214]
mac_p1wc3 r3, r1, r3, #-0.5, r2 | ld r2, [01e0]
mac_p1wc3 r7, r4, r6, #0.5, r7 | st r1, [01e0]
mac_p1wc3 r2, r1, r3, #-0.5, r2
mac_p1wc3 r7, r5, r7, #-0.5, r6 | ld r6, [020a] | st r2, [01e2]
mul r1, r2, #-2.0 | st r5, [020a]
mac_p1wc3 r6, r5, r7, #-0.5, r6 | st r1, [01da]
mov r1, r2 | mul r5, r6, #-2.0 | ld r2, [01de] | st r6, [020c]
mad r1, r2, #0.03125 | st r5, [0204]
mov r5, r6 | ld r6, [0208]
mac_p1wc3 r2, r0, r3, #0.5, r1 | ld r0, [01e0]
mad r5, r6, #0.03125
mac_p1wc3 r1, r0, r2, #-0.5, r3 | ld r2, [01f6] | ld r3, [01e2]
mac_p1wc3 r6, r4, r7, #0.5, r5 | ld r4, [020a]
mul r0, r1, #4.0 | mul r1, r2, r3 | ld r2, [c055]
mac_p1wc3 r5, r4, r6, #-0.5, r7 | st r0, [01dc] | ld r6, [0220]
mul r0, r1, #-32.0 | mul r4, r5, #4.0 | ld r7, [020c] | ld r1, [c054]
mul r5, r6, r7 | st r0, [040c] | ld r0, [c056]
mul r4, r5, #-32.0 | st r4, [0206]
hset h6, r0
st r4, [040e] | ld r0, [c05c]
ldidx r3, 0 | ldidx r4, 2
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
