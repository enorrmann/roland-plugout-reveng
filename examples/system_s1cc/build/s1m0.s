ld r2, [a038] | ld r6, [0303]
st r2, [a160] | ld r2, [c04b]
ld r0, [a060]
ld r1, [a120]
mul r1, r1, r2 | st r0, [a168] | ld r2, [c05e]
ld r0, [a128]
mad r1, r0, r2 | ld r2, [c05f] | ld r0, [a038]
addxy r1, r1, r2 | ld r2, [a030]
addxy r2, r2, r1 | mov r1, #0.0
min r4, r0, #1.0 | st r2, [0300] | st r1, [a151]
max r5, r0, #1.0 | ld r2, [0300] | st r4, [0304]
mad r6, r2, #-0.0625 | mul r3, r2, #0.0625 | st r5, [034c] | ld r5, [031d]
mul r7, r6, #-16.0 | st r3, [0302] | ld r6, [0305]
sign r1, r7 | st r5, [034e] | ld r5, [031f]
max r3, r1, -r1 | st r5, [0350] | ld r5, [0321]
mul r0, r4, r3 | addxy r1, r6, #-1.0 | st r5, [0352] | ld r5, [0323]
mul r7, r0, r6 | mul r3, r4, r1 | ld r0, [0307] | st r5, [0354]
max r6, r0, r7 | ld r5, [0325] | st r4, [0326]
mul r1, r4, r6 | addxy r0, r3, #1.0 | ld r6, [0313] | ld r3, [0315]
mul r7, r1, r0 | ld r0, [031b] | ld r1, [0317]
mad r6, r1, #-0.5 | st r0, [0358] | ld r0, [0329]
mad r6, r2, #0.0625 | st r0, [035a] | ld r0, [032b]
mad r3, r6, #-0.95148396492004395 | st r0, [035c] | ld r0, [032d]
mul r6, r3, #-1.0509898609631672 | mad r1, r3, #-0.0009765625 | st r0, [035e] | ld r0, [032f]
st r0, [0360] | ld r0, [0331] | st r6, [0312]
mul r6, r3, #-1.0 | st r0, [0362] | ld r0, [0333]
st r0, [0364] | ld r0, [0335] | st r6, [0314]
st r0, [0336] | st r0, [0366] | ld r0, [0337]
st r0, [0338] | st r0, [a188]
st r0, [0368] | ld r0, [0339] | ld r6, [c085]
st r0, [036a] | ld r0, [033b] | st r1, [0316]
st r0, [033c] | st r0, [036c] | ld r0, [033d]
st r0, [033e] | st r0, [036e] | ld r0, [033f]
mad r1, r2, #-0.125 | addxy r3, r6, r2 | st r0, [0370] | ld r0, [0341]
mac_p1wc3 r6, r1, r7, #0.125, r3 | st r5, [0356] | ld r1, [0358]
st r7, [0306] | ld r5, [0327] | st r0, [0342]
subxy r7, r1, r4 | subxy r2, r5, r4 | st r0, [0372] | ld r0, [0343] | st r6, [a1f0]
st r0, [0344] | st r0, [0374] | ld r0, [0345]
sign r5, r7 | st r0, [0376] | ld r0, [0347]
max r3, r2, #0.0 | st r6, [037e] | st r0, [a1d0]
max r2, r5, -r5 | st r0, [0348]
max r7, r5, #0.0 | ld r5, [0356] | st r0, [0378]
mul r6, r5, #-0.5 | ld r0, [0349] | st r4, [0358]
floor r6, r6 | st r0, [034a]
mad r6, r5, #0.5 | st r0, [037a] | ld r0, [034b]
mul r5, r6, #-1.0 | st r0, [037c] | ld r0, [c07c]
st r5, [0324] | ld r5, [0354] | st r0, [0356]
subxy r0, r1, r5
mul r3, r3, r0 | subxy r0, r4, r1
addxy r1, r0, r3
mul r0, r1, r2 | mov r1, r4 | ld r4, [0350]
mul r3, r0, r7
mad r1, r3, #0.98692291975021362
addxy r0, r1, #-0.90921198668146508 | mul r2, r1, #0.5499267578125 | st r1, [031a]
mul r7, r0, #1.099853515625 | st r2, [031c]
sign r3, r7
max r2, r3, -r3
max r0, r3, #0.0
addxy r3, r1, #-0.090921166152192007 | mul r7, r0, r2
mul r0, r3, #1.099853515625
sign r2, r0
min r0, r2, #0.0
addxy r2, r1, #-1.8184239733629302 | mul r3, r0, r3
mul r2, r2, r7 | addxy r7, r7, #-1.0
subxy r2, r1, r2 | addxy r3, r1, r3
mov r5, r2
mad r5, r4, #0.90921198668146508
addxy r5, r5, #0.90921198668146508
mul r5, r5, r7 | ld r7, [034e]
mov r6, r3 | st r6, [034e]
mad r6, r4, #-0.90921198668146508 | mad r7, r1, #-0.5499267578125
addxy r4, r6, #-0.090921166152192007 | addxy r6, r0, #1.0
mul r0, r4, r6 | mul r6, r7, #-2.0
subxy r4, r3, r0
sign r3, r6
max r0, r3, -r3
max r6, r3, #0.0
mul r3, r6, r0 | mov r6, r7
mad r6, r2, #-0.5499267578125
mad r6, r5, #-0.5499267578125
addxy r0, r6, #-0.5
mul r2, r0, r3
subxy r5, r7, r2 | addxy r7, r3, #-1.0
mov r6, r5
mad r6, r4, #0.5499267578125
addxy r0, r6, #0.049999982118606567
mul r2, r0, r7 | ld r0, [0352]
addxy r4, r2, r5
addxy r3, r4, #-0.5 | mul r6, r4, #2.0 | ld r4, [034e]
mac_p1wc3 r7, r3, r1, #0.5, r0 | st r6, [031e]
mul r2, r7, #-2.0 | mul r7, r4, #4.76837158203125e-07
sign r5, r2
mul r2, r7, #1.1928762841206038
min r6, r5, #0.0
addxy r5, r0, r2
mac_p1wc3 r7, r3, r1, #0.4999978244304657, r5
addxy r0, r7, #-2.8440263122320175e-07 | ld r7, [0358]
mac_p1w r2, r0, r6, r3, r1
min r5, r2, #0.0
mul r0, r5, r7 | mul r2, r4, #9.5367431640625e-07
mul r6, r0, #-2.0 | mul r0, r2, #1.3133725305626101
sign r3, r1 | st r6, [0320] | ld r6, [0354]
addxy r1, r6, r0
mac_p1wc3 r4, r5, r7, #0.49897074699401855, r1
addxy r2, r4, #-6.2497565522789955e-07
absadd r6, r3, #-1.0
mac_p1w r0, r2, r6, r5, r7 | ld r3, [0356] | ld r6, [034c]
min r1, r0, #0.0
addxy r2, r3, #-0.18653011322021484 | addxy r5, r6, #-1.0 | st r1, [a1b9]
mul r4, r1, #-2.0 | mul r0, r2, r5
subxy r4, r3, r0 | st r4, [0322]
addxy r2, r4, #1.0 | addxy r1, r4, #3.2987773034089276
sign r3, r2
max r0, r3, #0.0
max r2, r3, -r3
mul r6, r1, r4 | mul r3, r0, r2
addxy r0, r3, #-1.0 | addxy r2, r6, #2.2986957032627005
mul r3, r0, r2
sign r2, r4
shl r6, r3, 13 | min r3, r2, #0.0 | ld r2, [035a]
mac_p1wc3 r0, r1, r4, #0.0001220703125, r6 | st r5, [035a] | ld r5, [035e]
addxy r1, r3, #1.0 | subxy r3, r2, r7
mul r6, r0, r1
sign r1, r3
subxy r4, r0, r6 | subxy r3, r7, r2
max r0, r1, -r1
max r6, r1, #0.0
mul r1, r3, r0 | mov r3, r7
mul r2, r1, r6 | mov r7, r5
mad r3, r2, #0.98195701837539673
addxy r1, r3, #-0.90921198668146508 | mul r0, r3, #0.5499267578125 | st r3, [0328]
mul r6, r1, #1.099853515625 | st r0, [032a]
sign r2, r6
max r0, r2, -r2
max r1, r2, #0.0
addxy r2, r3, #-0.090921166152192007 | mul r6, r1, r0
mul r1, r2, #1.099853515625
sign r0, r1
min r1, r0, #0.0
addxy r0, r3, #-1.909211986681465
mul r0, r0, r6 | addxy r6, r6, #-1.0
subxy r0, r3, r0
mad r7, r0, #0.5499267578125
addxy r7, r7, #0.5
mul r7, r7, r6 | mul r6, r1, r2
addxy r2, r3, r6
mad r5, r2, #-0.5499267578125
addxy r6, r5, #0.049999982118606567 | addxy r5, r1, #1.0
mac_p1wc3 r1, r6, r5, #0.5499267578125, r2 | ld r6, [035c]
mad r6, r3, #-0.5499267578125
mul r5, r6, #-2.0
sign r2, r5
max r5, r2, -r2
max r2, r2, #0.0
mul r2, r2, r5 | mov r5, r6
mad r5, r0, #-0.5499267578125
subxy r0, r7, r5
addxy r7, r0, #-0.5
mul r5, r7, r2
subxy r0, r6, r5 | addxy r5, r2, #-1.0
subxy r7, r0, r1
addxy r6, r7, #-0.049999982118606567
mul r1, r6, r5 | ld r5, [0360]
subxy r7, r0, r1
addxy r6, r7, #0.5499267578125 | mul r2, r7, #-1.0
mac_p1wc3 r0, r6, r3, #-0.5, r5 | st r2, [032c]
mul r1, r0, #2.0 | mul r0, r4, #6.103515625e-05
sign r2, r1
mul r1, r0, #1.1687164306640625
min r7, r2, #0.0
addxy r2, r1, #0.9998210072517395
mul r4, r2, r5 | ld r2, [0358]
mac_p1wc3 r0, r6, r3, #-0.5, r4
mac_p1w r1, r0, r7, r6, r3 | ld r7, [0362]
mul r5, r1, r2
mul r4, r5, #2.0
sign r0, r3 | st r4, [032e]
mac_p1wc3 r6, r1, r2, #-0.49994239211082458, r7
absadd r5, r0, #-1.0
mac_p1w r4, r6, r5, r1, r2 | ld r7, [035a] | ld r1, [c070]
mul r3, r4, #2.0 | mul r0, r4, r7 | st r1, [a1c9]
sign r7, r3 | st r3, [0330] | ld r6, [c072]
addxy r3, r4, #-0.5 | st r6, [a190] | st r6, [035a]
min r1, r7, #0.0 | st r6, [a1f8] | ld r2, [c07d]
mul r7, r3, #2.0 | addxy r2, r0, #-0.2199999988079071 | st r2, [a1d1] | st r2, [0362]
sign r7, r7 | ld r5, [c07f] | st r0, [a1c1]
max r6, r7, -r7 | st r5, [a1e8]
max r7, r7, #0.0 | st r5, [a189]
mul r7, r7, r6 | mul r2, r2, #2.0
mul r6, r7, r3
subxy r7, r4, r6 | ld r6, [c073]
mul r3, r7, r1 | st r6, [0358] | ld r1, [c071]
addxy r4, r7, r3 | ld r7, [c074]
sign r2, r2 | st r4, [0360] | ld r6, [c075]
max r4, r2, -r2 | st r7, [035c] | ld r3, [c080]
max r2, r2, #0.0
mul r2, r2, r4 | addxy r4, r0, #-0.2202799916267395
mac_p1wc3 r4, r4, r4, #0.5, r0
addxy r4, r4, #-0.22014006972312927
mac_p1wc3 r4, r4, r2, #-0.5, r0
mad r4, r0, #0.5 | addxy r2, r2, #-1.0
addxy r7, r4, #0.11013999581336975
mul r7, r7, r2
addxy r2, r4, r7
addxy r4, r2, #0.11013999581336975 | mul r2, r1, r6
mac_p1wc3 r7, r4, r5, #-0.5, r0 | ld r4, [c07e] | st r2, [0356]
mul r6, r4, r1
mul r4, r6, r0 | mul r6, r2, r7 | ld r2, [0362]
addxy r4, r4, #0.0625 | st r6, [035e] | ld r6, [035c]
mul r3, r3, r6 | st r7, [035c] | ld r7, [0360]
mul r6, r3, r1 | mul r4, r7, r4
min r1, r0, #0.0625
mul r3, r6, r0 | ld r0, [c076]
max r6, r1, #-0.0625 | ld r1, [c077] | st r3, [034c]
mul r1, r1, r5
mad r0, r1, #0.5 | ld r1, [c078]
mul r1, r1, r2
addxy r3, r1, #-2.4933026163133269
mul r3, r3, r1
addxy r3, r3, #-0.35210319192549572
mul r3, r3, r1
addxy r1, r3, #-0.10581673482835371
mul r3, r1, r0 | ld r0, [c079]
mul r1, r3, r7 | mul r3, r0, r2
mul r7, r5, r5 | ld r5, [c07a]
mul r0, r3, r7
mul r7, r0, r5 | mov r0, r6 | ld r5, [0358]
mad r0, r7, #0.25 | mul r7, r6, r5
max r4, r4, #-0.03125
subxy r6, r7, r0
min r4, r4, #0.03125
addxy r7, r6, #0.052031248807907104
mul r0, r4, r7
addxy r6, r0, #-0.0009765625
shl r4, r6, 10
mul r7, r4, #-1.0 | mul r4, r0, #-1.0
sign r6, r7
addxy r7, r4, #0.0009765625 | st r6, [0360]
max r0, r7, -r7
max r4, r0, #1.9073486328125e-06
max r7, r4, #0.0009765625
shl r0, r7, 9
recipseed r6, r0
newton r6, r0
newton r6, r0
newton r6, r0
addxy r4, r3, #18.130695456454621 | st r6, [0354] | ld r6, [034c]
mul r7, r4, r3 | ld r4, [0356]
mul r0, r7, #0.03125 | ld r7, [035c]
mac_p1wc3 r4, r4, r7, #0.25, r6
addxy r6, r0, #1.1001681331638273 | mul r7, r4, r2 | ld r2, [c07b] | ld r0, [035e]
mul r4, r6, r3
mad r2, r4, #-0.0066254325211048126
mad r2, r1, #0.17955571413040161
mad r2, r0, #-0.25
mad r2, r7, #0.25
addxy r4, r2, #-1.1000000238418579 | addxy r6, r2, #-2.1964182825956109
mul r1, r4, #4.0 | mul r3, r6, r2
sign r0, r1
max r7, r0, #0.0
max r4, r0, -r0
mul r1, r7, r4 | addxy r0, r3, #1.2060601122385444
mul r7, r1, r0 | ld r0, [c082]
mac_p1wc3 r4, r6, r2, #0.58022848056866572, r2 | ld r6, [035a] | ld r1, [c081]
subxy r3, r7, r4 | mul r7, r0, r6 | ld r6, [c084]
mad r3, r2, #-0.58022848056866572 | mad r1, r7, #0.00390625 | ld r0, [c083]
addxy r4, r3, #0.61045558468169503 | mul r7, r6, r5
mul r2, r4, r1 | mad r0, r7, #0.125
addxy r3, r2, #0.59353022966335212
mul r4, r3, #-6.8938360214233398 | addxy r3, r2, #1.0774212709395252
sign r1, r4
max r6, r1, #0.0
max r5, r1, -r1
mul r7, r6, r5 | mul r4, r2, r3 | ld r5, [0360]
addxy r1, r4, #0.28720393903492503 | mul r4, r5, r0
mul r6, r7, r1 | ld r7, [0354]
mac_p1wc3 r5, r2, r3, #0.010417531936634082, r2
subxy r0, r6, r5 | mul r1, r4, r7
mad r0, r2, #-0.010417531936634082
addxy r4, r0, #0.29022591364345757 | st r0, [a191]
max r5, r4, -r4
shl r7, r4, 9
max r2, r5, #2.9514398080413782e-06
mul r3, r7, #1.2924868924081134 | mul r7, r2, #256.0
sign r6, r3
max r3, r7, #0.096712779629899881
mul r4, r3, #5.1699475696324537 | addxy r7, r0, #0.28871477646103672
recipseed r2, r4
newton r2, r4
newton r2, r4
newton r2, r4
shl r3, r7, 9
addxy r7, r0, #0.28720363927931952 | mul r4, r3, #1.2924868924081134
mul r5, r2, r6
shl r3, r7, 9
sign r2, r4
mul r4, r3, #1.2924868924081134 | mov r3, r0
min r6, r2, #0.0
mad r3, r5, #0.0030222743634343713
sign r2, r4
addxy r4, r3, #0.28720363927931952
min r7, r2, #0.0
mul r2, r4, r6 | addxy r4, r7, #1.0
shl r5, r2, 9
mad r0, r5, #0.001953125
addxy r3, r0, #0.28720363927931952
mul r6, r3, r4
subxy r2, r6, r0
mad r2, r1, #0.0060445487268687426
addxy r5, r2, #0.28720363927931952
shl r7, r5, 9
mul r3, r7, #1.2924868924081134
sign r4, r3 | ld r3, [037e]
min r6, r4, #0.0
addxy r0, r6, #1.0 | addxy r4, r3, #-1.25
mac_p1wc3 r2, r5, r0, #-0.0060445487268687426, r1
mul r6, r4, #8.0 | mul r7, r2, #256.0
sign r5, r6 | st r7, [a199]
addxy r2, r3, #0.25
max r0, r5, -r5
mul r7, r2, #8.0 | mul r2, r0, r4
max r1, r5, #0.0
sign r6, r7
mul r7, r2, r1
min r5, r6, #0.0
subxy r6, r3, r7
addxy r0, r6, #-0.25
mul r4, r5, r0
addxy r2, r4, r6 | mov r4, #0.041666666666666664
addxy r1, r2, #0.59374997850216871
mul r3, r1, #-5.5451774597167969
floor r7, r3
addxy r5, r7, #0.69314718055994529 | mad r2, r7, #0.18033687961558437
mul r0, r5, #0.0625
ymacd r4, r0, #0.16666666666666666
ymacd r4, r0, #0.5
ymacd r4, r0, #1.0
ymacd r4, r0, #1.0
mul r6, r4, r4 | addxy r4, r2, #0.59374997850216871
mul r1, r6, r6 | mul r6, r4, r4
mul r3, r1, r1 | addxy r1, r2, #-1.3899606075128503
mul r5, r3, r3 | mul r3, r6, r1
mul r0, r5, #0.015625 | mov r5, r2
mad r5, r3, #0.27953547022611447
addxy r7, r5, #-1.0292819975560181
mul r4, r7, r6 | mov r1, r2
mad r1, r4, #0.42706930106914331
addxy r3, r1, #-0.6686081920331266
mul r5, r3, r6 | mov r7, r2
mad r7, r5, #0.73211893606984124
addxy r4, r7, #-0.30793450019262536
mac_p1wc3 r1, r4, r6, #0.65042786114249618, r2
addxy r3, r1, #0.034303146377230566
mac_p1wc3 r5, r3, r6, #0.1269169056354455, r2
addxy r7, r5, #0.05246911125984572
mul r4, r7, r0
mul r1, r4, #-3.2040359716844296 | st r4, [a1d9]
sign r3, r1
absadd r6, r3, #-1.0
mac_p1w r2, r4, r6, r7, r0 | ld r1, [0364]
mov r5, r2
mad r5, r1, #-0.31210635861690367 | mul r1, r2, #-1.0
mul r3, r5, #-3.2040359716844296
max r5, r1, -r1
ph_wrap r4, r3, r3
shl r6, r2, 9
max r3, r5, #2.3811825456001562e-06
mul r0, r6, #-1.6020179858422148
max r6, r3, #4.7623650912003123e-06
sign r7, r0
max r0, r6, #6.6451013120345206e-06
shl r1, r0, 11
mul r5, r1, #1.6020179858422148
recipseed r0, r5
newton r0, r5
newton r0, r5
newton r0, r5
mul r1, r4, #-0.5 | mul r3, r0, r7
addxy r5, r1, #-2.0 | mul r6, r3, #0.015625
ph_wrap r0, r3, r5
absadd r7, r0, #-0.5
mul r3, r7, r6
max r1, r3, #-7.62939453125e-06
shl r5, r1, 15
shl r0, r5, 2
min r7, r0, #1.0
mul r3, r7, r7
addxy r5, r3, #-29.32148121184904 | mul r1, r3, r7
mul r0, r5, r3
mul r5, r0, #0.03125
addxy r0, r5, #15.600957895328319
mul r5, r0, r3
addxy r0, r5, #-126.45629034055113
mul r3, r1, r0
mul r5, r3, #0.0078125 | mov r3, r2
mad r7, r5, #0.41625187771182148 | mad r3, r4, #-0.31210635861690367
mul r1, r7, r4 | mul r5, r3, #-3.2040359716844296
mul r0, r1, #-1.570796012878418
ph_wrap r1, r3, r5 | st r0, [0334]
mul r0, r1, #-0.5
addxy r3, r0, #-2.0
ph_wrap r5, r3, r3
absadd r0, r5, #-0.5
mul r3, r0, r6 | st r4, [0364]
max r5, r3, #-7.62939453125e-06
shl r0, r5, 15
shl r3, r0, 2
min r5, r3, #1.0
mul r0, r5, r5
addxy r4, r0, #-29.32148121184904 | mul r3, r0, r5
mul r4, r4, r0
mul r4, r4, #0.03125
addxy r4, r4, #15.600957895328319
mul r4, r4, r0
addxy r0, r4, #-126.45629034055113
mul r4, r3, r0
mul r3, r4, #0.0078125
mad r5, r3, #0.41625187771182148
mov r3, r2 | mul r0, r5, r1
mad r3, r1, #-0.31210635861690367 | mul r4, r0, #-1.570796012878418
mul r5, r3, #-3.2040359716844296 | st r4, [033a]
ph_wrap r4, r3, r5
mul r1, r4, #-0.5
addxy r3, r1, #-2.0
ph_wrap r5, r3, r3
absadd r1, r5, #-0.5
mul r3, r1, r6 | mad r2, r4, #-0.31210635861690367 | st r0, [037e]
max r5, r3, #-7.62939453125e-06
shl r1, r5, 15
shl r3, r1, 2
min r5, r3, #1.0
mul r1, r5, r5
addxy r0, r1, #-29.32148121184904 | mul r3, r1, r5
mul r0, r0, r1
mul r0, r0, #0.03125
addxy r0, r0, #15.600957895328319
mul r0, r0, r1
addxy r1, r0, #-126.45629034055113
mul r0, r3, r1
mul r3, r0, #0.0078125
mad r5, r3, #0.41625187771182148 | mul r3, r2, #-3.2040359716844296
mul r1, r5, r4
mul r0, r1, #-1.570796012878418
ph_wrap r5, r3, r3 | st r0, [0340]
mul r0, r5, #-0.5 | st r5, [0332]
addxy r4, r0, #-2.0
ph_wrap r2, r3, r4
absadd r3, r2, #-0.5
mul r0, r3, r6
max r4, r0, #-7.62939453125e-06
shl r2, r4, 15
shl r3, r2, 2
min r6, r3, #1.0
mul r0, r6, r6
addxy r2, r0, #-29.32148121184904 | mul r4, r0, r6
mul r3, r2, r0 | st r4, [a1e0]
mul r2, r3, #0.03125
addxy r3, r2, #15.600957895328319
mul r2, r3, r0
addxy r3, r2, #-126.45629034055113
mul r0, r3, r4
mul r2, r0, #0.0078125 | ld r0, [0364]
mad r6, r2, #0.41625187771182148 | ld r2, [037c]
mul r3, r6, r5 | ld r5, [0366]
mul r4, r3, #-1.570796012878418
mac_p1wc3 r6, r7, r0, #-0.63661989959316345, r2 | st r4, [0346] | ld r4, [0368]
mad r4, r5, #0.58266710469935634 | ld r7, [036a] | ld r0, [036c]
mad r4, r7, #0.38910162374577151 | ld r2, [036e] | ld r5, [0370]
mad r4, r0, #0.77034243706960015 | ld r7, [0372] | ld r0, [0374]
mad r4, r2, #0.91845215940572489 | ld r2, [0376]
mad r4, r5, #0.22341414130630471 | ld r5, [0378]
mad r4, r7, #0.91845215940572489
mad r4, r0, #0.77034243706960015 | ld r0, [037a]
mad r4, r2, #0.11298477383489047 | ld r2, [037e]
addxy r7, r4, r5
mad r7, r0, #0.58266710469935634
mad r7, r6, #-0.11582275620173728
mad r7, r2, #-0.17747603225581576
mad r7, r1, #-0.35093804238459891
mad r7, r3, #-0.61119927918437622
mul r4, r7, #-0.12282902002334595 | st r7, [a1d8]
sign r5, r4
max r0, r5, -r5
max r2, r5, #0.0
min r6, r5, #0.0
mul r1, r2, r0 | st r6, [a198]
st r6, [a1b8]
st r1, [a1c0]
st r1, [a1c8]
