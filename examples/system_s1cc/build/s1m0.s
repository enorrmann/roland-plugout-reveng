ld r2, [a038] | ld r4, [0303]
st r2, [a160] | ld r2, [c04b]
ld r0, [a060]
ld r1, [a120]
mul r1, r1, r2 | st r0, [a168] | ld r2, [c05e]
ld r0, [a128]
mad r1, r0, r2 | ld r2, [c05f] | ld r0, [a038]
addxy r1, r1, r2 | ld r2, [a030] | st r0, [0304]
addxy r2, r2, r1 | mov r1, #0.0 | st r0, [0328]
st r2, [0300] | st r1, [a151]
ld r2, [0300]
mad r4, r2, #-0.0625 | mul r3, r2, #0.0625
mul r5, r4, #-16.0 | st r3, [0302] | ld r3, [0305]
sign r6, r5
max r7, r6, -r6
mul r1, r0, r7 | addxy r5, r3, #-1.0 | ld r7, [0307]
mul r4, r1, r3 | mul r6, r0, r5
max r1, r7, r4 | ld r4, [0309]
mul r3, r0, r1 | addxy r5, r6, #1.0 | ld r1, [c07f]
mul r7, r3, r5 | subxy r3, r4, r1
mul r5, r6, r3 | ld r3, [0315] | st r7, [0306]
subxy r1, r4, r5 | ld r5, [0319] | ld r4, [0317]
mul r6, r1, #-1.0 | mad r3, r5, #-0.5 | st r1, [034e] | ld r1, [031f]
mad r3, r2, #0.0625 | st r6, [0308] | ld r6, [031d]
mad r4, r3, #-0.95148396492004395 | st r6, [035a] | ld r6, [032b]
st r6, [035c] | ld r6, [032d] | st r1, [0350]
mul r3, r4, #-1.0509898609631672 | mad r5, r4, #-0.0009765625 | st r6, [035e] | ld r6, [032f]
st r6, [0360] | ld r6, [0331] | st r3, [0314]
mul r3, r4, #-1.0 | st r6, [0362] | ld r6, [0333]
st r6, [0364] | ld r6, [0335] | st r3, [0316]
st r6, [0366] | ld r6, [0337] | ld r3, [c086]
st r6, [0338] | st r6, [0368] | ld r6, [0339]
addxy r4, r3, r2 | st r6, [033a] | st r6, [a188] | ld r1, [0321]
st r6, [036a] | ld r6, [033b] | st r1, [0352]
st r6, [036c] | ld r6, [033d] | st r5, [0318]
ld r1, [0323] | st r6, [033e] | st r6, [036e]
mad r5, r2, #-0.125 | ld r6, [033f] | st r1, [0354]
ld r1, [0325] | st r6, [0340] | st r6, [0370]
mac_p1wc3 r3, r5, r7, #0.125, r4 | ld r6, [0341] | st r1, [0356]
ld r1, [0327] | st r6, [0372] | st r3, [a199]
ld r6, [0343] | st r1, [0358] | st r3, [0380]
ld r5, [035a] | ld r1, [0329]
st r6, [0344] | st r6, [0374] | ld r6, [0345]
subxy r7, r5, r0 | subxy r2, r1, r0 | st r6, [0346]
sign r1, r7 | st r6, [0376] | ld r6, [0347]
max r4, r2, #0.0 | st r6, [0378] | ld r6, [0349]
max r2, r1, -r1 | st r6, [034a] | st r6, [a1f0]
max r7, r1, #0.0 | ld r1, [0358] | st r6, [037a]
mul r3, r1, #-0.5 | ld r6, [034b] | st r0, [035a]
floor r3, r3 | st r6, [034c]
mad r3, r1, #0.5 | st r6, [037c] | ld r6, [034d]
mul r1, r3, #-1.0 | st r6, [037e] | ld r6, [c07c]
st r1, [0326] | ld r1, [0356] | st r6, [0358]
subxy r6, r5, r1
mul r4, r4, r6 | subxy r6, r0, r5
addxy r5, r6, r4
mul r6, r5, r2 | mov r5, r0 | ld r0, [0352]
mul r4, r6, r7
mad r5, r4, #0.98692291975021362
addxy r6, r5, #-0.90921198668146508 | mul r2, r5, #0.5499267578125 | st r5, [031c]
mul r7, r6, #1.099853515625 | st r2, [031e]
sign r4, r7
max r2, r4, -r4
max r6, r4, #0.0
addxy r4, r5, #-0.090921166152192007 | mul r7, r6, r2
mul r6, r4, #1.099853515625
sign r2, r6
min r6, r2, #0.0
addxy r2, r5, #-1.8184239733629302 | mul r4, r6, r4
mul r2, r2, r7 | addxy r7, r7, #-1.0
subxy r2, r5, r2 | addxy r4, r5, r4
mov r1, r2
mad r1, r0, #0.90921198668146508
addxy r1, r1, #0.90921198668146508
mul r1, r1, r7 | ld r7, [0350]
mov r3, r4 | st r3, [0350]
mad r3, r0, #-0.90921198668146508 | mad r7, r5, #-0.5499267578125
addxy r0, r3, #-0.090921166152192007 | addxy r3, r6, #1.0
mul r6, r0, r3 | mul r3, r7, #-2.0
subxy r0, r4, r6
sign r4, r3
max r6, r4, -r4
max r3, r4, #0.0
mul r4, r3, r6 | mov r3, r7
mad r3, r2, #-0.5499267578125
mad r3, r1, #-0.5499267578125
addxy r6, r3, #-0.5
mul r2, r6, r4
subxy r1, r7, r2 | addxy r7, r4, #-1.0
mov r3, r1
mad r3, r0, #0.5499267578125
addxy r6, r3, #0.049999982118606567
mul r2, r6, r7 | ld r6, [0354]
addxy r0, r2, r1
addxy r4, r0, #-0.5 | mul r3, r0, #2.0 | ld r0, [0350]
mac_p1wc3 r7, r4, r5, #0.5, r6 | st r3, [0320]
mul r2, r7, #-2.0 | mul r7, r0, #4.76837158203125e-07
sign r1, r2
mul r2, r7, #1.1928762841206038
min r3, r1, #0.0
addxy r1, r6, r2
mac_p1wc3 r7, r4, r5, #0.4999978244304657, r1
addxy r6, r7, #-2.8440263122320175e-07 | ld r7, [035a]
mac_p1w r2, r6, r3, r4, r5
min r1, r2, #0.0
mul r6, r1, r7 | mul r2, r0, #9.5367431640625e-07
mul r3, r6, #-2.0 | mul r6, r2, #1.3133725305626101
sign r4, r5 | st r3, [0322] | ld r3, [0356]
addxy r5, r3, r6
mac_p1wc3 r0, r1, r7, #0.49897074699401855, r5
addxy r2, r0, #-6.2497565522789955e-07
absadd r3, r4, #-1.0
mac_p1w r6, r2, r3, r1, r7 | ld r4, [0358] | ld r3, [034e]
addxy r2, r4, #-0.18653011322021484
mul r1, r2, r3 | ld r3, [0360]
min r5, r6, #0.0
addxy r6, r1, r4 | mul r0, r5, #-2.0 | st r5, [a1d9]
mul r2, r6, #-1.0 | addxy r0, r6, #-3.2987773034089276 | st r0, [0324]
addxy r1, r2, #1.0 | mul r5, r0, r6
sign r4, r1
max r1, r4, #0.0
max r4, r4, -r4
mul r1, r1, r4
addxy r4, r1, #-1.0 | addxy r1, r5, #2.2986957032627005
mac_p1w r5, r0, r6, r4, r1
sign r0, r2
min r6, r0, #0.0 | ld r0, [035c]
addxy r1, r6, #1.0 | subxy r6, r0, r7
mul r4, r5, r1
sign r1, r6
subxy r2, r5, r4 | subxy r6, r7, r0
max r5, r1, -r1
max r4, r1, #0.0
mul r1, r6, r5 | mov r6, r7
mul r0, r1, r4 | mov r7, r3
mad r6, r0, #0.98195701837539673
addxy r1, r6, #-0.90921198668146508 | mul r5, r6, #0.5499267578125 | st r6, [032a]
mul r4, r1, #1.099853515625 | st r5, [032c]
sign r0, r4
max r5, r0, -r0
max r1, r0, #0.0
addxy r0, r6, #-0.090921166152192007 | mul r4, r1, r5
mul r1, r0, #1.099853515625
sign r5, r1
min r1, r5, #0.0
addxy r5, r6, #-1.909211986681465
mul r5, r5, r4 | addxy r4, r4, #-1.0
subxy r5, r6, r5
mad r7, r5, #0.5499267578125
addxy r7, r7, #0.5
mul r7, r7, r4 | mul r4, r1, r0
addxy r0, r6, r4
mad r3, r0, #-0.5499267578125
addxy r4, r3, #0.049999982118606567 | addxy r3, r1, #1.0
mac_p1wc3 r1, r4, r3, #0.5499267578125, r0 | ld r4, [035e]
mad r4, r6, #-0.5499267578125
mul r3, r4, #-2.0
sign r0, r3
max r3, r0, -r0
max r0, r0, #0.0
mul r0, r0, r3 | mov r3, r4
mad r3, r5, #-0.5499267578125
subxy r5, r7, r3
addxy r7, r5, #-0.5
mul r3, r7, r0
subxy r5, r4, r3 | addxy r3, r0, #-1.0
subxy r7, r5, r1
addxy r4, r7, #-0.049999982118606567
mul r1, r4, r3 | ld r3, [0362]
subxy r7, r5, r1
addxy r4, r7, #0.5499267578125 | mul r0, r7, #-1.0
mac_p1wc3 r5, r4, r6, #-0.5, r3 | st r0, [032e]
mul r1, r5, #2.0 | mul r5, r2, #6.103515625e-05
sign r0, r1
mul r1, r5, #1.1687164306640625
min r7, r0, #0.0
addxy r0, r1, #0.9998210072517395
mul r2, r0, r3 | ld r0, [035a]
mac_p1wc3 r5, r4, r6, #-0.5, r2
mac_p1w r1, r5, r7, r4, r6 | ld r7, [0364]
mul r3, r1, r0
mul r2, r3, #2.0
sign r5, r6 | st r2, [0330]
mac_p1wc3 r4, r1, r0, #-0.49994239211082458, r7
absadd r3, r5, #-1.0
mac_p1w r2, r4, r3, r1, r0 | ld r7, [034e] | ld r1, [c070]
mul r6, r2, #2.0 | mul r5, r2, r7 | st r1, [a1e9]
sign r7, r6 | st r6, [0332] | ld r4, [c072]
addxy r6, r2, #-0.5 | st r4, [a190] | st r4, [034e]
min r1, r7, #0.0 | st r4, [a1b9] | ld r0, [c07d]
mul r7, r6, #2.0 | addxy r0, r5, #0.2199999988079071 | st r0, [a1f1] | st r0, [0364]
sign r7, r7 | ld r3, [c080] | st r5, [a1e1]
max r4, r7, -r7 | st r3, [a191]
max r7, r7, #0.0 | st r3, [a1c1]
mul r7, r7, r4 | mul r0, r0, #-2.0
mul r4, r7, r6
subxy r7, r2, r4 | ld r4, [c073]
mul r6, r7, r1 | st r4, [035a] | ld r1, [c071]
addxy r2, r7, r6 | ld r7, [c074]
sign r0, r0 | st r2, [0362] | ld r4, [c075]
max r2, r0, -r0 | st r7, [035e] | ld r6, [c081]
max r0, r0, #0.0
mul r0, r0, r2 | addxy r2, r5, #0.2202799916267395
mac_p1wc3 r2, r2, r2, #-0.5, r5
addxy r2, r2, #-0.22014006972312927
mac_p1wc3 r2, r2, r0, #0.5, r5
mad r2, r5, #-0.5 | addxy r0, r0, #-1.0
addxy r7, r2, #0.11013999581336975
mul r7, r7, r0
addxy r0, r2, r7
addxy r2, r0, #0.11013999581336975 | mul r0, r1, r4
mac_p1wc3 r7, r2, r3, #0.5, r5 | ld r2, [c07e] | st r0, [035c]
mul r4, r2, r1
mul r2, r4, r5 | mul r4, r0, r7 | ld r0, [0364]
addxy r2, r2, #-0.0625 | st r4, [0360] | ld r4, [035e]
mul r6, r6, r4 | st r7, [035e] | ld r7, [0362]
mul r4, r6, r1 | mul r2, r7, r2
max r1, r5, #-0.0625
mul r6, r4, r5 | ld r5, [c076]
min r4, r1, #0.0625 | ld r1, [c077] | st r6, [0358]
mul r1, r1, r3
mad r5, r1, #0.5 | ld r1, [c078]
mul r1, r1, r0
addxy r6, r1, #-2.4933026163133269
mul r6, r6, r1
addxy r6, r6, #-0.35210319192549572
mul r6, r6, r1
addxy r1, r6, #-0.10581673482835371
mul r6, r1, r5 | ld r5, [c079]
mul r1, r6, r7 | mul r6, r5, r0
mul r7, r3, r3 | ld r3, [c07a]
mul r5, r6, r7
mul r7, r5, r3 | mov r5, r4 | ld r3, [035a]
mad r5, r7, #-0.25 | mul r7, r4, r3
min r2, r2, #0.03125
subxy r4, r7, r5
max r2, r2, #-0.03125
addxy r7, r4, #-0.052031248807907104
mul r5, r2, r7
addxy r4, r5, #-0.0009765625
shl r2, r4, 10
mul r7, r2, #-1.0 | mul r2, r5, #-1.0
sign r4, r7
addxy r7, r2, #0.0009765625 | st r4, [0362]
max r5, r7, -r7
max r2, r5, #1.9073486328125e-06
max r7, r2, #0.0009765625
mul r5, r7, #256.0 | addxy r7, r6, #18.130695456454621
recipseed r2, r5
newton r2, r5
newton r2, r5
newton r2, r5
mul r4, r7, r6 | ld r5, [035c] | ld r7, [035e]
mul r3, r4, #0.03125 | ld r4, [0358]
mac_p1wc3 r5, r5, r7, #-0.25, r4
addxy r4, r3, #1.1001681331638273 | mul r7, r5, r0 | ld r0, [c07b] | ld r3, [0360]
mul r5, r4, r6
mad r0, r5, #-0.0066254325211048126
mad r0, r1, #0.17955571413040161
mad r0, r3, #-0.25
mad r0, r7, #0.25
addxy r5, r0, #-1.1000000238418579 | addxy r4, r0, #-2.1964182825956109
mul r1, r5, #4.0 | mul r6, r4, r0
sign r3, r1
max r7, r3, #0.0
max r5, r3, -r3
mul r1, r7, r5 | addxy r5, r0, #-1.0144000053405762
addxy r3, r6, #1.2060601122385444 | mul r6, r5, #4.0
mul r7, r1, r3
sign r1, r6
mac_p1wc3 r5, r4, r0, #0.58022848056866572, r0
min r3, r1, #0.0
subxy r6, r7, r5 | addxy r4, r3, #1.0 | ld r3, [034e] | ld r5, [c082]
addxy r1, r6, #0.61045558468169503 | ld r6, [c083]
mac_p1wc3 r7, r1, r4, #-0.58022848056866572, r0 | ld r0, [c084]
mul r1, r6, r3 | ld r6, [c085] | ld r3, [035a]
mad r5, r1, #0.00390625 | mul r1, r6, r3
mul r4, r7, r5 | mad r0, r1, #0.125
addxy r6, r4, #0.59353022966335212
mul r3, r6, #-6.8938360214233398
sign r1, r3
max r6, r1, #0.0
max r3, r1, -r1
mul r1, r6, r3 | addxy r6, r4, #1.0774212709395252
mul r3, r4, r6
addxy r3, r3, #0.28720393903492503
mul r1, r1, r3 | addxy r3, r4, #0.53874214227128459
mul r3, r3, #-6.8938360214233398 | st r1, [035a] | ld r1, [0362]
mul r1, r1, r0
sign r3, r3
mul r0, r1, r2 | addxy r1, r4, #-0.010465713971836583
min r3, r3, #0.0
mul r2, r1, r3
mac_p1wc3 r1, r7, r5, #0.97801053524017334, r2 | ld r5, [035a]
mac_p1wc3 r7, r4, r6, #0.010417531936634082, r1
subxy r2, r5, r7 | addxy r6, r3, #1.0
addxy r4, r2, #0.29022591364275391
mac_p1wc3 r5, r4, r6, #-0.010417531936634082, r1
mul r7, r5, #256.0
max r4, r7, -r7 | st r7, [a1c9]
max r6, r4, #0.00075556859085859282
max r1, r6, #0.0030222743634343713
mul r2, r7, #2.5849737848162269 | mul r5, r1, #20.679790278529815
sign r3, r2
recipseed r1, r5
newton r1, r5
newton r1, r5
addxy r4, r7, #-0.38685111851959952
newton r1, r5
mul r6, r4, #2.5849737848162269 | addxy r4, r7, #-0.77370223703919905
sign r2, r6
mul r6, r4, #2.5849737848162269
min r5, r2, #0.0
sign r2, r6
mac_p1wc3 r6, r1, r3, #0.32312172310202836, r7
min r4, r2, #0.0
addxy r2, r6, #-0.24999999999999997 | addxy r6, r4, #1.0
mac_p1wc3 r1, r2, r5, #0.32312172310202836, r7
addxy r3, r1, #-0.24999999999999997
mul r2, r3, r6
subxy r5, r2, r1
mad r5, r0, #0.25
addxy r7, r5, #-0.24999999999999997 | ld r5, [0380]
mul r4, r7, #8.0
sign r3, r4
min r6, r3, #0.0
addxy r4, r5, #-1.25 | addxy r2, r6, #1.0
mul r3, r4, #8.0
mac_p1wc3 r1, r7, r2, #-0.25, r0
sign r6, r3 | st r1, [a1d1]
addxy r0, r5, #0.25
max r7, r6, -r6
mul r1, r0, #8.0 | mul r0, r7, r4
max r2, r6, #0.0
sign r3, r1
mul r1, r0, r2
min r6, r3, #0.0
subxy r3, r5, r1
addxy r7, r3, #-0.25
mul r4, r6, r7
addxy r0, r4, r3 | mov r4, #0.041666666666666664
addxy r2, r0, #0.59374997850216871
mul r5, r2, #-5.5451774597167969
floor r1, r5
addxy r6, r1, #0.69314718055994529 | mad r0, r1, #0.18033687961558437
mul r7, r6, #0.0625
ymacd r4, r7, #0.16666666666666666
ymacd r4, r7, #0.5
ymacd r4, r7, #1.0
ymacd r4, r7, #1.0
mul r3, r4, r4 | addxy r4, r0, #0.59374997850216871
mul r2, r3, r3 | mul r3, r4, r4
mul r5, r2, r2 | addxy r2, r0, #-1.3899606075128503
mul r6, r5, r5 | mul r5, r3, r2
mul r7, r6, #0.015625 | mov r6, r0
mad r6, r5, #0.27953547022611447
addxy r1, r6, #-1.0292819975560181
mul r4, r1, r3 | mov r2, r0
mad r2, r4, #0.42706930106914331
addxy r5, r2, #-0.6686081920331266
mul r6, r5, r3 | mov r1, r0
mad r1, r6, #0.73211893606984124
addxy r4, r1, #-0.30793450019262536
mac_p1wc3 r2, r4, r3, #0.65042786114249618, r0
addxy r5, r2, #0.034303146377230566
mac_p1wc3 r6, r5, r3, #0.1269169056354455, r0
addxy r1, r6, #0.05246911125984572
mul r4, r1, r7
mul r2, r4, #-3.2040359716844296 | st r4, [a1f9]
sign r5, r2
absadd r3, r5, #-1.0
mac_p1w r0, r4, r3, r1, r7 | ld r2, [0366]
mov r6, r0
mad r6, r2, #-0.31210635861690367 | mul r2, r0, #-1.0
mul r5, r6, #-3.2040359716844296
max r6, r2, -r2
ph_wrap r4, r3, r5
shl r3, r0, 9
max r5, r6, #2.3811825456001562e-06
mul r7, r3, #-1.6020179858422148
max r3, r5, #4.7623650912003123e-06
sign r1, r7
max r7, r3, #6.6451013120345206e-06
shl r2, r7, 11
mul r6, r2, #1.6020179858422148
recipseed r2, r6
newton r2, r6
newton r2, r6
newton r2, r6
mul r3, r4, #-0.5 | mul r5, r2, r1
addxy r6, r3, #-2.0 | mul r7, r5, #0.015625
ph_wrap r2, r3, r6
absadd r1, r2, #-0.5
mul r5, r1, r7
max r3, r5, #-7.62939453125e-06
shl r6, r3, 15
shl r2, r6, 2
min r1, r2, #1.0
mul r5, r1, r1
addxy r6, r5, #-29.32148121184904 | mul r3, r5, r1
mul r2, r6, r5
mul r6, r2, #0.03125
addxy r2, r6, #15.600957895328319
mul r6, r2, r5
addxy r2, r6, #-126.45629034055113
mul r5, r3, r2
mul r6, r5, #0.0078125 | mov r5, r0
mad r1, r6, #0.41625187771182148 | mad r5, r4, #-0.31210635861690367
mul r3, r1, r4 | mul r6, r5, #-3.2040359716844296
mul r2, r3, #-1.570796012878418
ph_wrap r3, r3, r6 | st r2, [0336]
mul r2, r3, #-0.5
addxy r5, r2, #-2.0
ph_wrap r6, r3, r5
absadd r2, r6, #-0.5
mul r5, r2, r7 | st r4, [0366]
max r6, r5, #-7.62939453125e-06
shl r2, r6, 15
shl r5, r2, 2
min r6, r5, #1.0
mul r2, r6, r6
addxy r4, r2, #-29.32148121184904 | mul r5, r2, r6
mul r4, r4, r2
mul r4, r4, #0.03125
addxy r4, r4, #15.600957895328319
mul r4, r4, r2
addxy r2, r4, #-126.45629034055113
mul r4, r5, r2
mul r5, r4, #0.0078125
mad r6, r5, #0.41625187771182148
mov r5, r0 | mul r2, r6, r3
mad r5, r3, #-0.31210635861690367 | mul r4, r2, #-1.570796012878418
mul r6, r5, #-3.2040359716844296 | st r4, [033c]
ph_wrap r4, r3, r6
mul r3, r4, #-0.5
addxy r5, r3, #-2.0
ph_wrap r6, r3, r5
absadd r3, r6, #-0.5
mul r5, r3, r7 | mad r0, r4, #-0.31210635861690367 | st r2, [0380]
max r6, r5, #-7.62939453125e-06
shl r3, r6, 15
shl r5, r3, 2
min r6, r5, #1.0
mul r3, r6, r6
addxy r2, r3, #-29.32148121184904 | mul r5, r3, r6
mul r2, r2, r3
mul r2, r2, #0.03125
addxy r2, r2, #15.600957895328319
mul r2, r2, r3
addxy r3, r2, #-126.45629034055113
mul r2, r5, r3
mul r5, r2, #0.0078125
mad r6, r5, #0.41625187771182148 | mul r5, r0, #-3.2040359716844296
mul r3, r6, r4
mul r2, r3, #-1.570796012878418
ph_wrap r6, r3, r5 | st r2, [0342]
mul r2, r6, #-0.5 | st r6, [0334]
addxy r4, r2, #-2.0
ph_wrap r0, r3, r4
absadd r5, r0, #-0.5
mul r2, r5, r7
max r4, r2, #-7.62939453125e-06
shl r0, r4, 15
shl r5, r0, 2
min r7, r5, #1.0
mul r2, r7, r7
addxy r0, r2, #-29.32148121184904 | mul r4, r2, r7
mul r5, r0, r2 | st r4, [a189]
mul r0, r5, #0.03125
addxy r5, r0, #15.600957895328319
mul r0, r5, r2
addxy r5, r0, #-126.45629034055113
mul r2, r5, r4
mul r0, r2, #0.0078125
mad r7, r0, #0.41625187771182148
mul r5, r7, r6
mul r4, r5, #-1.570796012878418
sign r2, r6 | st r4, [0348]
max r7, r2, #0.0
max r4, r2, -r2
min r0, r2, #0.0 | ld r2, [037e]
mul r6, r7, r4 | st r0, [a1d0] | ld r4, [0368]
st r0, [a1d8] | ld r0, [0366]
mac_p1wc3 r7, r1, r0, #-0.63661989959316345, r2 | st r6, [a1e0] | ld r1, [036c]
st r6, [a1e8] | ld r6, [036a]
mad r6, r4, #0.58266710469935634 | ld r0, [036e] | ld r2, [0370]
mad r6, r1, #0.38910162374577151 | ld r4, [0372] | ld r1, [0374]
mad r6, r0, #0.77034243706960015 | ld r0, [0376]
mad r6, r2, #0.91845215940572489 | ld r2, [0378]
mad r6, r4, #0.22341414130630471 | ld r4, [037a]
mad r6, r1, #0.91845215940572489
mad r6, r0, #0.77034243706960015 | ld r0, [037c]
mad r6, r2, #0.11298477383489047 | ld r2, [0380]
addxy r1, r6, r4
mad r1, r0, #0.58266710469935634
mad r1, r7, #-0.11582275620173728
mad r1, r2, #-0.17747603225581576
mad r1, r3, #-0.35093804238459891
mad r1, r5, #-0.61119927918437622
mul r6, r1, #-0.12282902002334595 | st r1, [a1f8]
sign r4, r6
max r0, r4, -r4
max r2, r4, #0.0
min r7, r4, #0.0
mul r3, r2, r0 | st r7, [a198]
st r7, [a1b8]
st r3, [a1c0]
st r3, [a1c8]
