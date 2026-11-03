ld r2, [a03e] | ld r3, [c076]
ld r6, [a03f] | ld r7, [c076]
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
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [a1b6]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [a1b7]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a046]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a047]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | mul r2, r1, #-0.08664339756999316 | st r2, [0390] | ld r1, [0395]
mul r6, r5, #-0.08664339756999316 | mov r0, #0.16666666666666666 | st r6, [03a8] | ld r5, [03ad]
mov r4, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a04e]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a04f]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0 | ld r0, [0390]
mul r6, r6, r4 | st r2, [0392] | ld r2, [a046]
addxy r2, r1, r2 | st r6, [03aa] | ld r6, [a047]
addxy r6, r5, r6 | ld r4, [03a8]
ph_wrap r1, r3, r2 | ld r2, [c074]
ph_wrap r5, r3, r6 | st r1, [0394] | ld r6, [c074]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [03ac]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a136] | ld r1, [0392]
mul r2, r2, r3 | mul r6, r6, r7 | st r5, [a137] | ld r5, [03aa] | st r1, [039c]
ld r3, [c070] | st r5, [03b4]
mul r2, r2, r3 | ld r7, [c070]
mul r6, r6, r7 | ld r3, [c075]
ld r7, [c075]
ld r1, [c077]
mul r3, r3, r1 | ld r5, [c077]
mul r7, r7, r5 | ld r1, [c071]
mul r3, r3, r1 | mul r1, r0, r2 | ld r5, [c071]
mul r7, r7, r5 | mul r5, r4, r6
shl r2, r1, 10
shl r6, r5, 10 | min r1, r2, #0.25 | ld r2, [039c]
min r5, r6, #0.25 | ld r6, [03b4] | st r1, [03a0]
mul r3, r2, r3 | mul r7, r6, r7 | st r5, [03b8]
mul r3, r3, #256.0 | mul r7, r7, #256.0
addxy r1, r0, #-0.004999999888241291 | addxy r5, r4, #-0.004999999888241291 | st r3, [039e] | ld r3, [a13e]
mul r1, r1, #5.479451911627053 | mul r5, r5, #5.479451911627053 | st r7, [03b6] | ld r7, [a13f]
max r3, r3, r1 | ld r1, [0399]
max r7, r7, r5 | st r3, [03a2] | ld r5, [03b1]
mov r3, r0 | mov r7, r4 | st r7, [03ba]
mad r3, r1, #0.5 | mad r7, r5, #0.5
mul r1, r3, #2.0 | mul r5, r7, #2.0
ph_wrap r3, r3, r1
ph_wrap r7, r3, r5 | st r3, [0398]
addxy r1, r3, #1.0 | addxy r5, r7, #1.0 | st r7, [03b0]
st r3, [03a4] | st r7, [03bc]
ph_wrap r1, r3, r1
ph_wrap r5, r3, r5
mul r3, r0, #2.0 | mul r7, r4, #2.0
min r2, r1, r3
min r6, r5, r7
mul r0, r0, #-2.0 | mul r4, r4, #-2.0
max r2, r2, r0 | st r0, [03a6] | ld r0, [039c]
max r6, r6, r4 | st r4, [03be] | ld r4, [03b4]
mul r2, r0, r2 | mul r6, r4, r6
shl r2, r2, 13
shl r6, r6, 13
absadd r0, r2, #-2.0
absadd r4, r6, #-2.0
mul r2, r2, r0 | mul r6, r6, r4
addxy r0, r2, r1 | addxy r4, r6, r5 | ld r2, [03a2] | ld r6, [03ba]
mov r1, r2 | mov r5, r6 | ld r2, [03a4] | ld r6, [03bc]
mad r1, r2, #-0.68493148895338163 | mad r5, r6, #-0.68493148895338163
addxy r2, r1, #-0.65753423000762279 | addxy r6, r5, #-0.65753423000762279
mul r1, r2, #-1.4600000381469727 | mul r5, r6, #-1.4600000381469727
ph_wrap r2, r3, r1
ph_wrap r6, r3, r5 | min r1, r2, r3 | ld r3, [03a6]
min r5, r6, r7 | ld r7, [03be]
max r1, r1, r3 | ld r3, [039c]
max r5, r5, r7 | ld r7, [03b4]
mul r3, r3, r1 | mul r7, r7, r5
shl r1, r3, 13
shl r5, r7, 13
absadd r3, r1, #-2.0
absadd r7, r5, #-2.0
mul r1, r1, r3 | mul r5, r5, r7
addxy r3, r1, r2 | addxy r7, r5, r6 | ld r2, [03a2] | ld r6, [03ba]
subxy r1, r3, r0 | subxy r5, r7, r4
mad r2, r1, #-0.68493148895338163 | mad r6, r5, #-0.68493148895338163
addxy r3, r0, #1.0 | addxy r7, r4, #1.0
addxy r1, r2, #0.027397258945758795 | addxy r5, r6, #0.027397258945758795
mul r2, r3, r1 | mul r6, r7, r5 | ld r3, [c072]
mad r0, r2, #-0.73000001907348633 | mad r4, r6, #-0.73000001907348633 | ld r7, [c072]
mul r1, r3, r0 | mul r5, r7, r4 | ld r3, [03a0] | ld r7, [03b8] | ld r2, [a146]
addxy r0, r3, #-0.0024999999441206455 | addxy r4, r7, #-0.0024999999441206455 | ld r6, [a147] | st r1, [03a2]
mul r0, r0, #10.958903823254106 | mul r4, r4, #10.958903823254106 | st r5, [03ba]
max r2, r2, r0 | ld r0, [039b]
max r6, r6, r4 | ld r4, [03b3] | st r2, [03a6]
mov r1, r3 | mov r5, r7 | st r6, [03be]
mad r1, r0, #0.25 | mad r5, r4, #0.25
mul r0, r1, #4.0 | mul r4, r5, #4.0
ph_wrap r1, r3, r0
ph_wrap r5, r3, r4 | st r1, [039a]
addxy r0, r1, #1.0 | addxy r4, r5, #1.0 | st r5, [03b2]
st r1, [039c] | st r5, [03b4]
ph_wrap r0, r3, r0
ph_wrap r4, r3, r4
mul r1, r3, #4.0 | mul r5, r7, #4.0
min r2, r0, r1
min r6, r4, r5
mul r3, r3, #-4.0 | mul r7, r7, #-4.0
max r2, r2, r3 | st r3, [03a0] | ld r3, [039e]
max r6, r6, r7 | st r7, [03b8] | ld r7, [03b6]
mul r2, r3, r2 | mul r6, r7, r6
shl r2, r2, 15
shl r6, r6, 15
absadd r3, r2, #-1.0
absadd r7, r6, #-1.0
mac_p1wc3 r2, r2, r3, #0.25, r0 | ld r3, [03a6]
mac_p1wc3 r6, r6, r7, #0.25, r4 | ld r7, [03be]
mov r0, r3 | mov r4, r7 | ld r3, [039c] | ld r7, [03b4]
mad r0, r3, #-0.68493148895338163 | mad r4, r7, #-0.68493148895338163
addxy r3, r0, #-0.65753423000762279 | addxy r7, r4, #-0.65753423000762279
mul r0, r3, #-1.4600000381469727 | mul r4, r7, #-1.4600000381469727
ph_wrap r3, r3, r0
ph_wrap r7, r3, r4 | min r0, r3, r1 | ld r1, [03a0]
min r4, r7, r5 | ld r5, [03b8]
max r0, r0, r1 | ld r1, [039e]
max r4, r4, r5 | ld r5, [03b6]
mul r1, r1, r0 | mul r5, r5, r4
shl r0, r1, 15
shl r4, r5, 15
absadd r1, r0, #-1.0
absadd r5, r4, #-1.0
mac_p1wc3 r0, r0, r1, #0.25, r3 | ld r3, [03a6]
mac_p1wc3 r4, r4, r5, #0.25, r7 | ld r7, [03be]
subxy r1, r0, r2 | subxy r5, r4, r6
mad r1, r3, #-0.36500000953674316 | mad r5, r7, #-0.36500000953674316
addxy r0, r2, #0.25 | addxy r4, r6, #0.25
addxy r3, r1, #-0.0099999997764825821 | addxy r7, r5, #-0.0099999997764825821
mac_p1wc3 r1, r0, r3, #0.5, r2 | ld r0, [c073] | ld r3, [03a2]
mac_p1wc3 r5, r4, r7, #0.5, r6 | ld r4, [c073] | ld r7, [03ba]
mac_p1wc3 r2, r1, r0, #0.125, r3
mac_p1wc3 r6, r5, r4, #0.125, r7
mul r1, r2, #4.0 | mul r5, r6, #4.0 | ld r2, [c056]
st r1, [0396] | st r5, [03ae]
ld r0, [0396] | ld r4, [03ae] | ld r1, [a136]
mad r0, r1, r2 | ld r5, [a137] | ld r6, [c056]
mad r4, r5, r6 | ld r1, [03c0] | ld r2, [c057]
mad r0, r1, r2 | ld r5, [03c0] | ld r6, [c057]
mad r4, r5, r6 | st r0, [a157]
st r4, [a158]
