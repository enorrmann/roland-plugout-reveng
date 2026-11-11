ld r2, [a038] | ld r4, [0305]
st r2, [a160] | ld r2, [c04b]
ld r0, [a060]
ld r1, [a120]
mul r1, r1, r2 | st r0, [a168] | ld r2, [c05e]
ld r0, [a128]
mad r1, r0, r2 | ld r2, [c05f] | ld r0, [a038]
addxy r1, r1, r2 | ld r2, [a030] | st r0, [0306]
addxy r2, r2, r1 | ld r1, [0303] | st r0, [0312]
st r2, [0300] | ld r2, [a040]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [0300]
mul r1, r1, #256.0 | mad r4, r2, #-0.0625 | st r1, [0302]
ph_clamp r1, r3, -r1
mul r5, r4, #-16.0 | mov r1, #0.0 | st r1, [a130]
sign r6, r5 | st r1, [a151]
mul r3, r2, #0.0625
max r7, r6, -r6 | st r3, [0304] | ld r3, [0307]
mul r1, r0, r7 | addxy r5, r3, #-1.0 | ld r7, [0309]
mul r4, r1, r3 | mul r6, r0, r5
max r1, r7, r4 | ld r4, [030b]
mul r3, r0, r1 | addxy r5, r6, #1.0 | ld r1, [a140]
mul r7, r3, r5 | subxy r3, r4, r1
mul r5, r6, r3 | st r7, [0308] | ld r3, [a138]
subxy r1, r4, r5 | ld r4, [030d] | ld r5, [030f]
mul r6, r1, #-1.0 | mad r4, r2, #0.0625 | st r3, [a1e0] | st r3, [0344]
st r6, [030a] | ld r6, [0311] | st r1, [0346]
mad r4, r6, #-0.5 | ld r1, [031b]
mad r5, r4, #-0.95148396492004395
mul r4, r5, #-1.0509898609631672 | mad r6, r5, #-0.0009765625
mul r4, r5, #-1.0 | st r4, [030c]
st r4, [030e] | ld r4, [c083]
mad r6, r2, #-0.125 | addxy r5, r4, r2 | st r6, [0310] | ld r2, [0313]
mac_p1wc3 r4, r6, r7, #0.125, r5 | ld r5, [0315]
subxy r6, r2, r0 | ld r2, [0317] | st r4, [a1b8] | st r4, [0342]
max r7, r6, #0.0 | st r5, [0348]
subxy r6, r2, r5
mul r6, r6, r7
addxy r7, r6, r2 | subxy r6, r2, r0
sign r2, r6
max r6, r2, #0.0
max r2, r2, -r2
mul r6, r6, r2 | subxy r2, r0, r7
mul r7, r6, r2 | mov r6, r0
mad r6, r7, #0.98692291975021362 | ld r7, [0319]
mul r2, r6, #0.5499267578125 | mad r7, r6, #-0.5499267578125 | st r6, [0316]
mul r2, r7, #-2.0 | st r2, [0318]
sign r2, r2
max r4, r2, #0.0
max r2, r2, -r2
mul r4, r4, r2 | addxy r2, r6, #-0.90921198668146508
mul r2, r2, #1.099853515625
sign r2, r2
max r3, r2, #0.0
max r2, r2, -r2
mul r3, r3, r2 | addxy r2, r6, #-1.8184239733629302
mul r2, r2, r3 | addxy r3, r3, #-1.0
subxy r2, r6, r2
mov r5, r2
mad r5, r1, #0.90921198668146508
addxy r5, r5, #0.90921198668146508
mul r5, r5, r3 | mov r3, r7
mad r3, r2, #-0.5499267578125
mad r3, r5, #-0.5499267578125
addxy r2, r3, #-0.5
mul r5, r2, r4 | addxy r2, r6, #-0.090921166152192007
subxy r3, r7, r5 | mul r7, r2, #1.099853515625
sign r5, r7
min r7, r5, #0.0
mul r5, r7, r2
addxy r2, r6, r5
mov r5, r2
mad r5, r1, #-0.90921198668146508
addxy r1, r5, #-0.090921166152192007 | addxy r5, r7, #1.0
mul r7, r1, r5 | mov r5, r3
subxy r1, r2, r7 | addxy r7, r4, #-1.0
mad r5, r1, #0.5499267578125
addxy r2, r5, #0.049999982118606567
mul r1, r2, r7 | ld r7, [031d]
addxy r5, r1, r3
addxy r2, r5, #-0.5 | mul r4, r5, #2.0
mac_p1wc3 r1, r2, r6, #0.5, r7 | st r4, [031a]
mul r3, r1, #-2.0
sign r4, r3
mac_p1wc3 r1, r2, r6, #0.4999978244304657, r7
min r5, r4, #0.0
addxy r3, r1, #-2.8440263122320175e-07
mac_p1w r4, r3, r5, r2, r6 | ld r2, [0348]
min r7, r4, #0.0
mul r1, r7, r0
mul r3, r1, #-2.0
mac_p1wc3 r4, r7, r0, #0.49897074699401855, r2 | st r3, [031c]
sign r5, r6
addxy r1, r4, #-6.2497565522789955e-07
absadd r3, r5, #-1.0
mac_p1w r6, r1, r3, r7, r0 | ld r5, [c07a] | ld r3, [0346] | st r0, [0348]
addxy r1, r5, #-0.18653011322021484
mul r7, r1, r3 | ld r3, [0323]
min r2, r6, #0.0
addxy r6, r7, r5 | mul r4, r2, #-2.0 | st r2, [a1d0]
mul r1, r6, #-1.0 | addxy r4, r6, #-3.2987773034089276 | st r4, [0314]
addxy r7, r1, #1.0 | mul r2, r4, r6
sign r5, r7
max r7, r5, #0.0
max r5, r5, -r5
mul r7, r7, r5 | addxy r5, r2, #2.2986957032627005
addxy r2, r7, #-1.0
mul r7, r5, r2
shl r5, r7, 13
sign r7, r1
mac_p1wc3 r2, r4, r6, #0.0001220703125, r5
min r4, r7, #0.0 | ld r7, [031f]
addxy r6, r4, #1.0 | subxy r4, r7, r0
mul r5, r2, r6
sign r6, r4
subxy r1, r2, r5
max r2, r6, #0.0
max r5, r6, -r6
mul r4, r2, r5 | subxy r6, r0, r7
mul r2, r4, r6 | mov r5, r0 | ld r4, [0321]
mad r5, r2, #0.98195701837539673
mad r4, r5, #-0.5499267578125 | mul r7, r5, #0.5499267578125 | st r5, [031e]
mul r6, r4, #-2.0 | mov r0, r3 | st r7, [0320]
sign r2, r6
max r7, r2, #0.0
max r6, r2, -r2
mul r2, r7, r6 | addxy r7, r5, #-0.90921198668146508
mul r6, r7, #1.099853515625
sign r7, r6
max r6, r7, #0.0
max r7, r7, -r7
mul r6, r6, r7 | addxy r7, r5, #-1.909211986681465
mul r7, r6, r7 | addxy r6, r6, #-1.0
subxy r7, r5, r7
mad r0, r7, #0.5499267578125
addxy r0, r0, #0.5
mul r0, r0, r6 | mov r6, r4
mad r6, r7, #-0.5499267578125
subxy r7, r0, r6
addxy r0, r7, #-0.5
mul r6, r0, r2 | addxy r0, r5, #-0.090921166152192007
subxy r7, r4, r6 | mul r4, r0, #1.099853515625
sign r6, r4
min r4, r6, #0.0
mul r6, r4, r0
addxy r0, r5, r6
mad r3, r0, #-0.5499267578125
addxy r6, r3, #0.049999982118606567 | addxy r3, r4, #1.0
mul r4, r6, r3 | mov r6, r7
mad r6, r0, #-0.5499267578125
subxy r3, r4, r6 | addxy r4, r2, #-1.0
addxy r0, r3, #0.049999982118606567
mul r6, r0, r4 | ld r0, [0325]
addxy r3, r6, r7
addxy r2, r3, #-0.5499267578125 | st r3, [0322]
mac_p1wc3 r4, r2, r5, #0.5, r0
mul r6, r4, #-2.0 | mul r4, r1, #6.103515625e-05
sign r7, r6
mul r6, r4, #1.1687164306640625
min r3, r7, #0.0
addxy r7, r6, #0.9998210072517395
mul r1, r0, r7 | ld r0, [0348]
mac_p1wc3 r4, r2, r5, #0.5, r1
mac_p1w r6, r4, r3, r2, r5 | ld r3, [0327]
mul r7, r6, r0
mul r1, r7, #-2.0
sign r4, r5 | st r1, [0324]
mac_p1wc3 r2, r6, r0, #0.49994239211082458, r3
absadd r7, r4, #-1.0
mac_p1w r1, r2, r7, r6, r0 | ld r3, [0346] | ld r2, [c070]
addxy r6, r1, #0.5 | mul r4, r1, r3 | st r2, [a190] | st r2, [0346]
mul r3, r6, #-2.0 | mul r5, r1, #-2.0 | ld r0, [c07b]
sign r3, r3 | st r5, [0326] | ld r7, [c07d]
max r2, r3, #0.0 | st r0, [034a] | st r4, [a1d8]
max r3, r3, -r3 | st r7, [a198]
mul r2, r2, r3
mul r3, r2, r6
sign r6, r5
subxy r2, r1, r3
min r1, r6, #0.0 | ld r6, [c071]
mul r3, r2, r1 | ld r1, [0344]
addxy r6, r4, #-0.2199999988079071 | st r6, [0344]
mul r6, r6, #2.0 | addxy r5, r2, r3
sign r6, r6 | st r5, [0348]
max r5, r6, #0.0
max r6, r6, -r6
mul r5, r5, r6 | addxy r6, r4, #-0.2202799916267395
mac_p1wc3 r6, r6, r6, #0.5, r4
addxy r6, r6, #-0.22014006972312927
mac_p1wc3 r6, r6, r5, #-0.5, r4
mad r6, r4, #0.5 | addxy r5, r5, #-1.0
addxy r0, r6, #0.11013999581336975
mul r0, r0, r5
addxy r5, r6, r0
min r2, r1, #1.5 | ld r1, [c07e]
addxy r6, r5, #0.11013999581336975 | ld r5, [c07c]
max r3, r2, #0.5 | ld r2, [c072]
mac_p1wc3 r0, r6, r7, #-0.5, r4
addxy r6, r3, #-0.5 | st r0, [034e]
mul r3, r5, r6
mul r5, r3, r4 | mul r3, r2, r6
mul r2, r3, r1 | addxy r5, r5, #0.03125 | ld r1, [c073]
mul r3, r2, r4 | mul r2, r1, r6
mul r1, r2, r0 | st r3, [0350] | ld r0, [0348]
min r6, r4, #0.0625 | st r1, [034c] | ld r1, [034a]
max r4, r6, #-0.0625 | ld r6, [c074] | st r2, [0352]
mul r6, r6, r1 | mul r5, r0, r5 | ld r2, [c077]
addxy r3, r6, #-2.4933026163133269 | mul r2, r2, r7
mul r3, r3, r6
addxy r3, r3, #-0.35210319192549572
mul r3, r3, r6 | ld r6, [c075]
mul r6, r6, r1 | ld r1, [c076]
mad r1, r2, #0.5 | addxy r2, r3, #-0.10581673482835371
mul r3, r2, r1 | mul r1, r7, r7
mul r2, r3, r0 | mul r3, r6, r1 | ld r0, [c078]
mul r7, r3, r0 | mov r1, r4 | ld r3, [0344]
mad r1, r7, #0.25 | mul r0, r4, r3
min r5, r5, #0.015625
subxy r7, r0, r1
max r5, r5, #-0.015625
addxy r4, r7, #0.052031248807907104
mul r0, r5, r4
mul r1, r0, #256.0
addxy r7, r1, #0.125
max r0, r7, -r7
mul r5, r7, #8.0
max r1, r0, #0.000244140625
sign r4, r5
max r5, r1, #0.125
mul r7, r5, #4.0 | ld r5, [0352]
recipseed r0, r7
newton r0, r7
newton r0, r7
newton r0, r7
ld r1, [034e] | ld r7, [0350]
mac_p1wc3 r5, r5, r1, #0.25, r7 | ld r1, [034a]
mul r7, r5, r1 | addxy r5, r6, #18.130695456454621
mul r1, r5, r6
mul r5, r1, #0.03125
addxy r1, r5, #1.1001681331638273
mul r5, r1, r6 | ld r1, [c079] | ld r6, [034c]
mad r1, r5, #-0.0066254325211048126 | st r0, [034c] | ld r0, [0346]
mad r1, r2, #-0.17955571413040161
mad r1, r6, #-0.5
mad r1, r7, #0.5
addxy r6, r1, #-1.1000000238418579 | addxy r5, r1, #-2.1964182825956109
mul r7, r6, #4.0 | mul r2, r5, r1
sign r6, r7
max r7, r6, #0.0
max r6, r6, -r6
mul r7, r7, r6 | addxy r6, r2, #1.2060601122385444
mul r2, r7, r6 | ld r6, [c080]
mul r6, r6, r0 | addxy r0, r1, #-1.0144000053405762 | ld r7, [c07f]
mad r7, r6, #0.00390625 | mul r6, r0, #4.0
sign r0, r6
min r6, r0, #0.0
mac_p1wc3 r0, r5, r1, #0.58022848056866572, r1
subxy r5, r2, r0 | addxy r0, r6, #1.0
addxy r2, r5, #0.61045558468169503
mac_p1wc3 r5, r2, r0, #-0.58022848056866572, r1 | ld r2, [c081]
mul r6, r5, r7
addxy r0, r6, #1.0774212709395252
mul r1, r6, r0 | st r0, [0346] | ld r0, [c082]
mul r0, r0, r3
mad r2, r0, #0.125 | ld r0, [034c]
mul r3, r2, r4 | addxy r4, r6, #0.53874214227128459
mul r2, r3, r0 | mul r3, r4, #-6.8938360214233398
sign r0, r3
min r4, r0, #0.0
addxy r3, r6, #-0.010465713971836583
mul r0, r3, r4
mac_p1wc3 r3, r5, r7, #0.97801053524017334, r0
addxy r5, r6, #0.59353022966335212
mul r7, r5, #-6.8938360214233398
sign r0, r7
max r5, r0, #0.0
max r7, r0, -r0
mul r0, r5, r7 | addxy r5, r1, #0.28720393903492503 | ld r1, [0346]
mul r7, r0, r5
mac_p1wc3 r0, r6, r1, #0.010417531936634082, r3
subxy r5, r7, r0 | addxy r1, r4, #1.0
addxy r6, r5, #0.29022591364275391
mac_p1wc3 r7, r6, r1, #-0.010417531936634082, r3
shl r0, r7, 9
addxy r5, r0, #-0.77370223703919905 | mul r3, r0, #1.2924868924081134 | st r0, [a1c0]
mul r4, r5, #1.2924868924081134
max r5, r0, -r0
sign r6, r4
max r4, r5, #0.0015111371817171856
min r1, r6, #0.0
max r6, r4, #0.0060445487268687426
sign r7, r3
mul r3, r6, #10.339895139264907
recipseed r5, r3
newton r5, r3
newton r5, r3
newton r5, r3
mac_p1wc3 r4, r5, r7, #0.16156086155101418, r0
addxy r5, r0, #-1.5474044740783981 | addxy r6, r4, #-0.24999999999999997
mul r7, r5, #1.2924868924081134
sign r4, r7
mac_p1wc3 r3, r6, r1, #0.16156086155101418, r0
min r6, r4, #0.0
addxy r1, r3, #-0.24999999999999997 | addxy r0, r6, #1.0
mul r5, r1, r0
subxy r7, r5, r3
mad r7, r2, #0.5
addxy r4, r7, #-0.24999999999999997 | ld r7, [0342]
mul r6, r4, #8.0
sign r1, r6
addxy r6, r7, #-1.25
min r0, r1, #0.0
mul r1, r6, #8.0 | addxy r5, r0, #1.0
sign r0, r1
mac_p1wc3 r3, r4, r5, #-0.5, r2
max r5, r0, -r0 | st r3, [a1c8]
max r4, r0, #0.0
mul r2, r5, r6 | addxy r0, r7, #0.25
mul r3, r2, r4 | mul r5, r0, #8.0
subxy r1, r7, r3
sign r6, r5
min r2, r6, #0.0
addxy r4, r1, #-0.25
mul r3, r2, r4
addxy r7, r3, r1
addxy r0, r7, #0.59374997850216871
mul r5, r0, #-5.5451774597167969
floor r6, r5
mad r7, r6, #0.18033687961558437
addxy r2, r7, #0.59374997850216871 | addxy r3, r7, #-1.3899606075128503
mul r4, r2, r2 | mov r0, r7
mul r1, r4, r3 | mov r3, r7
mad r0, r1, #0.27953547022611447
addxy r5, r0, #-1.0292819975560181
mul r2, r5, r4 | mov r5, r7
mad r3, r2, #0.42706930106914331
addxy r1, r3, #-0.6686081920331266 | addxy r3, r6, #0.69314718055994529
mul r0, r1, r4 | mul r1, r3, #0.0625
mad r5, r0, #0.73211893606984124 | mov r0, #0.041666666666666664
ymacd r0, r1, #0.16666666666666666
ymacd r0, r1, #0.5
ymacd r0, r1, #1.0
ymacd r0, r1, #1.0
addxy r2, r5, #-0.30793450019262536 | mul r5, r0, r0
mul r6, r5, r5
mac_p1wc3 r5, r2, r4, #0.65042786114249618, r7
mul r3, r6, r6 | addxy r6, r5, #0.034303146377230566
mul r1, r3, r3
mac_p1wc3 r3, r6, r4, #0.1269169056354455, r7
mul r0, r1, #0.015625 | addxy r1, r3, #0.05246911125984572
mul r2, r0, r1
mul r5, r2, #-3.2040359716844296
sign r6, r5
absadd r4, r6, #-1.0
mac_p1w r7, r2, r4, r0, r1 | ld r3, [0329]
mov r5, r7
mad r5, r3, #-0.31210635861690367 | mul r3, r7, #-1.0
mul r6, r5, #-3.2040359716844296
max r5, r3, -r3
ph_wrap r2, r3, r6
shl r4, r7, 9
max r6, r5, #2.3811825456001562e-06
mul r1, r4, #-1.6020179858422148
max r4, r6, #4.7623650912003123e-06
sign r0, r1
max r1, r4, #6.6451018797526793e-06
shl r3, r1, 11
mul r5, r3, #1.6020179858422148
recipseed r3, r5
newton r3, r5
newton r3, r5
newton r3, r5
mul r4, r2, #-0.5 | mul r6, r3, r0
addxy r5, r4, #-2.0 | mul r1, r6, #0.015625
ph_wrap r3, r3, r5
absadd r0, r3, #-0.5
mul r6, r0, r1
max r4, r6, #-7.62939453125e-06
shl r5, r4, 15
shl r3, r5, 2
min r0, r3, #1.0
mul r6, r0, r0
addxy r5, r6, #-29.32148121184904 | mul r4, r6, r0
mul r3, r5, r6
mul r5, r3, #0.03125
addxy r3, r5, #15.600957895328319
mul r5, r3, r6
addxy r3, r5, #-126.45629034055113
mul r6, r4, r3
mul r5, r6, #0.0078125 | mov r6, r7
mad r0, r5, #0.41625187771182148 | mad r6, r2, #-0.31210635861690367
mul r4, r0, r2 | mul r5, r6, #-3.2040359716844296
mul r3, r4, #-1.570796012878418
ph_wrap r4, r3, r5 | st r3, [0332]
mul r3, r4, #-0.5
addxy r6, r3, #-2.0
ph_wrap r5, r3, r6
absadd r3, r5, #-0.5
mul r6, r3, r1 | st r2, [0342]
max r5, r6, #-7.62939453125e-06
shl r3, r5, 15
shl r6, r3, 2
min r5, r6, #1.0
mul r3, r5, r5
addxy r2, r3, #-29.32148121184904 | mul r6, r3, r5
mul r2, r2, r3
mul r2, r2, #0.03125
addxy r2, r2, #15.600957895328319
mul r2, r2, r3
addxy r3, r2, #-126.45629034055113
mul r2, r6, r3
mul r6, r2, #0.0078125
mad r5, r6, #0.41625187771182148
mov r6, r7 | mul r3, r5, r4
mad r6, r4, #-0.31210635861690367 | mul r2, r3, #-1.570796012878418
mul r3, r6, #-3.2040359716844296 | st r2, [0336]
ph_wrap r2, r3, r3
mul r6, r2, #-0.5
addxy r3, r6, #-2.0
ph_wrap r6, r3, r3
absadd r3, r6, #-0.5
mul r6, r3, r1 | mad r7, r2, #-0.31210635861690367 | st r4, [034c]
max r3, r6, #-7.62939453125e-06 | st r0, [0346]
shl r6, r3, 15
shl r3, r6, 2
min r6, r3, #1.0
mul r3, r6, r6
addxy r4, r3, #-29.32148121184904 | mul r0, r3, r6
mul r4, r4, r3
mul r4, r4, #0.03125
addxy r4, r4, #15.600957895328319
mul r4, r4, r3
addxy r3, r4, #-126.45629034055113
mul r4, r0, r3
mul r0, r4, #0.0078125
mad r6, r0, #0.41625187771182148 | mul r0, r7, #-3.2040359716844296
mul r3, r6, r2
mul r4, r3, #-1.570796012878418
ph_wrap r3, r3, r0 | st r4, [033a]
mul r4, r3, #-0.5 | st r3, [0328]
addxy r7, r4, #-2.0
ph_wrap r0, r3, r7
absadd r4, r0, #-0.5
mul r7, r4, r1 | st r3, [0354]
max r0, r7, #-7.62939453125e-06 | st r5, [034a] | ld r5, [0333]
shl r4, r0, 15 | st r2, [034e] | ld r2, [0337]
shl r1, r4, 2 | st r6, [0352] | ld r6, [0339]
min r7, r1, #1.0 | st r2, [033c]
mul r0, r7, r7 | st r6, [032c]
addxy r1, r0, #-29.32148121184904 | mul r4, r0, r7 | st r5, [0340]
mul r1, r1, r0
mul r1, r1, #0.03125
addxy r1, r1, #15.600957895328319
mul r1, r1, r0
addxy r0, r1, #-126.45629034055113
mul r1, r4, r0
mul r4, r1, #0.0078125
mad r7, r4, #0.41625187771182148 | ld r4, [032b]
mul r0, r7, r3 | st r4, [0344] | ld r4, [0331]
mul r1, r0, #-1.570796012878418 | st r4, [0358] | ld r0, [032d]
st r1, [033e] | ld r1, [032f] | st r7, [0356]
ld r4, [0341] | st r1, [0348] | st r0, [0350]
ld r7, [033f] | ld r1, [033b]
ld r3, [033d] | st r4, [0330] | st r7, [0334]
addxy r4, r4, r7 | addxy r7, r3, r1 | ld r0, [0335] | st r1, [0338]
st r3, [032e] | st r0, [032a]
addxy r3, r6, r2 | addxy r1, r0, r5 | ld r6, [0356] | ld r2, [0354]
ld r0, [0358]
mac_p1wc3 r5, r6, r2, #-0.63661989959316345, r0 | ld r6, [0352] | ld r2, [034e]
ld r0, [0348]
mac_p1wc3 r6, r6, r2, #-0.63661989959316345, r0 | st r5, [0348] | ld r2, [034a]
ld r0, [034c] | ld r5, [0350]
mac_p1wc3 r2, r2, r0, #-0.63661989959316345, r5 | st r6, [0350] | ld r0, [0346]
ld r5, [0342] | ld r6, [0344]
mac_p1wc3 r0, r0, r5, #-0.63661989959316345, r6 | ld r5, [0350] | ld r6, [0348]
mad r1, r0, #-0.19878032459288963
mad r3, r1, #0.75637414825001081
mad r3, r2, #-0.230385895564755
mad r7, r3, #0.83873986160372505
mad r4, r7, #0.91845215940572489
mad r4, r5, #-0.35093804238459891
mad r4, r6, #-0.61119927918437622
st r4, [a188]
