ld r2, [a038] | ld r4, [0305]
st r2, [a160] | ld r2, [c04b]
ld r0, [a060]
ld r1, [a120]
mul r1, r1, r2 | st r0, [a168] | ld r2, [c05e]
ld r0, [a128]
mad r1, r0, r2 | ld r2, [c05f] | ld r0, [a038]
addxy r1, r1, r2 | ld r2, [a030] | st r0, [0306]
addxy r2, r2, r1 | ld r1, [0303] | st r0, [032a]
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
mul r5, r6, r3 | ld r3, [a138] | st r7, [0308]
subxy r1, r4, r5 | st r3, [a1e9] | st r3, [0350] | ld r3, [031f]
mul r6, r1, #-1.0 | st r3, [035e] | ld r3, [032d]
st r3, [0360] | ld r3, [032f] | st r6, [030a]
st r3, [0362] | ld r3, [0331] | st r1, [0352]
st r3, [0364] | ld r3, [0333]
st r3, [0366] | ld r3, [0335]
st r3, [0368] | ld r3, [0337]
st r3, [036a] | ld r4, [0317]
ld r6, [031b] | ld r3, [0339]
st r3, [033a] | st r3, [036c] | ld r3, [033b]
mad r4, r6, #-0.5 | ld r5, [0319] | ld r1, [0321]
mad r4, r2, #0.0625 | st r3, [033c] | st r3, [a188]
mad r5, r4, #-0.95148396492004395 | st r3, [036e] | ld r3, [033d]
st r1, [0354] | ld r1, [0323] | st r3, [0370]
mul r4, r5, #-1.0509898609631672 | mad r6, r5, #-0.0009765625 | ld r3, [033f] | st r1, [0356]
st r4, [0316] | ld r1, [0325] | st r3, [0340]
mul r4, r5, #-1.0 | st r3, [0372] | ld r3, [0341]
st r1, [0358] | st r4, [0318] | ld r1, [0327]
st r3, [0342] | st r3, [0374] | ld r3, [0343]
mad r6, r2, #-0.125 | ld r4, [c083] | st r6, [031a]
addxy r5, r4, r2 | st r1, [035a] | ld r1, [0329]
mac_p1wc3 r4, r6, r7, #0.125, r5 | st r3, [0376] | ld r3, [0345]
st r1, [035c] | ld r6, [035e] | st r3, [0346]
subxy r7, r6, r0 | ld r1, [032b] | st r3, [0378] | st r4, [a199]
subxy r2, r1, r0 | ld r3, [0347] | st r4, [0384]
st r3, [0348] | st r3, [037a] | ld r3, [0349]
sign r1, r7 | st r3, [037c] | ld r3, [034b]
max r5, r2, #0.0 | st r3, [034c] | st r3, [a1f0]
max r2, r1, -r1 | st r3, [037e] | ld r3, [034d]
max r7, r1, #0.0 | ld r1, [035c] | st r3, [034e]
mul r4, r1, #-0.5 | st r3, [0380] | ld r3, [034f]
floor r4, r4 | st r3, [0382] | ld r3, [c07a]
mad r4, r1, #0.5 | st r3, [035c]
mul r1, r4, #-1.0 | st r0, [035e]
st r1, [0328] | ld r1, [035a]
subxy r3, r6, r1
mul r5, r5, r3 | subxy r3, r0, r6
addxy r6, r3, r5
mul r3, r6, r2 | mov r6, r0 | ld r0, [0356]
mul r5, r3, r7
mad r6, r5, #0.98692291975021362
addxy r3, r6, #-0.90921198668146508 | mul r2, r6, #0.5499267578125 | st r6, [031e]
mul r7, r3, #1.099853515625 | st r2, [0320]
sign r5, r7
max r2, r5, -r5
max r3, r5, #0.0
addxy r5, r6, #-0.090921166152192007 | mul r7, r3, r2
mul r3, r5, #1.099853515625
sign r2, r3
min r3, r2, #0.0
addxy r2, r6, #-1.8184239733629302 | mul r5, r3, r5
mul r2, r2, r7 | addxy r7, r7, #-1.0
subxy r2, r6, r2 | addxy r5, r6, r5
mov r1, r2
mad r1, r0, #0.90921198668146508
addxy r1, r1, #0.90921198668146508
mul r1, r1, r7 | ld r7, [0354]
mov r4, r5 | st r4, [0354]
mad r4, r0, #-0.90921198668146508 | mad r7, r6, #-0.5499267578125
addxy r0, r4, #-0.090921166152192007 | addxy r4, r3, #1.0
mul r3, r0, r4 | mul r4, r7, #-2.0
subxy r0, r5, r3
sign r5, r4
max r3, r5, -r5
max r4, r5, #0.0
mul r5, r4, r3 | mov r4, r7
mad r4, r2, #-0.5499267578125
mad r4, r1, #-0.5499267578125
addxy r3, r4, #-0.5
mul r2, r3, r5
subxy r1, r7, r2 | addxy r7, r5, #-1.0
mov r4, r1
mad r4, r0, #0.5499267578125
addxy r3, r4, #0.049999982118606567
mul r2, r3, r7 | ld r3, [0358]
addxy r0, r2, r1
addxy r5, r0, #-0.5 | mul r4, r0, #2.0 | ld r0, [0354]
mac_p1wc3 r7, r5, r6, #0.5, r3 | st r4, [0322]
mul r2, r7, #-2.0 | mul r7, r0, #4.76837158203125e-07
sign r1, r2
mul r2, r7, #1.1928762841206038
min r4, r1, #0.0
addxy r1, r3, r2
mac_p1wc3 r7, r5, r6, #0.4999978244304657, r1
addxy r3, r7, #-2.8440263122320175e-07 | ld r7, [035e]
mac_p1w r2, r3, r4, r5, r6
min r1, r2, #0.0
mul r3, r1, r7 | mul r2, r0, #9.5367431640625e-07
mul r4, r3, #-2.0 | mul r3, r2, #1.3133725305626101
sign r5, r6 | st r4, [0324] | ld r4, [035a]
addxy r6, r4, r3
mac_p1wc3 r0, r1, r7, #0.49897074699401855, r6
addxy r2, r0, #-6.2497565522789955e-07
absadd r4, r5, #-1.0
mac_p1w r3, r2, r4, r1, r7 | ld r5, [035c] | ld r4, [0352]
addxy r2, r5, #-0.18653011322021484
mul r1, r2, r4 | ld r4, [0364]
min r6, r3, #0.0
addxy r3, r1, r5 | mul r0, r6, #-2.0 | st r6, [a1d9]
mul r2, r3, #-1.0 | addxy r0, r3, #-3.2987773034089276 | st r0, [0326]
addxy r1, r2, #1.0 | mul r6, r0, r3
sign r5, r1
max r1, r5, #0.0
max r5, r5, -r5
mul r1, r1, r5
addxy r5, r1, #-1.0 | addxy r1, r6, #2.2986957032627005
mul r6, r5, r1
shl r5, r6, 13
sign r6, r2
mac_p1wc3 r1, r0, r3, #0.0001220703125, r5
min r0, r6, #0.0 | ld r6, [0360]
addxy r3, r0, #1.0 | subxy r0, r6, r7
mul r5, r1, r3
sign r3, r0
subxy r2, r1, r5 | subxy r0, r7, r6
max r1, r3, -r3
max r5, r3, #0.0
mul r3, r0, r1 | mov r0, r7
mul r6, r3, r5 | mov r7, r4
mad r0, r6, #0.98195701837539673
addxy r3, r0, #-0.90921198668146508 | mul r1, r0, #0.5499267578125 | st r0, [032c]
mul r5, r3, #1.099853515625 | st r1, [032e]
sign r6, r5
max r1, r6, -r6
max r3, r6, #0.0
addxy r6, r0, #-0.090921166152192007 | mul r5, r3, r1
mul r3, r6, #1.099853515625
sign r1, r3
min r3, r1, #0.0
addxy r1, r0, #-1.909211986681465
mul r1, r1, r5 | addxy r5, r5, #-1.0
subxy r1, r0, r1
mad r7, r1, #0.5499267578125
addxy r7, r7, #0.5
mul r7, r7, r5 | mul r5, r3, r6
addxy r6, r0, r5
mad r4, r6, #-0.5499267578125
addxy r5, r4, #0.049999982118606567 | addxy r4, r3, #1.0
mac_p1wc3 r3, r5, r4, #0.5499267578125, r6 | ld r5, [0362]
mad r5, r0, #-0.5499267578125
mul r4, r5, #-2.0
sign r6, r4
max r4, r6, -r6
max r6, r6, #0.0
mul r6, r6, r4 | mov r4, r5
mad r4, r1, #-0.5499267578125
subxy r1, r7, r4
addxy r7, r1, #-0.5
mul r4, r7, r6
subxy r1, r5, r4 | addxy r4, r6, #-1.0
subxy r7, r1, r3
addxy r5, r7, #-0.049999982118606567
mul r3, r5, r4 | ld r4, [0366]
subxy r7, r1, r3
addxy r5, r7, #0.5499267578125 | mul r6, r7, #-1.0
mac_p1wc3 r1, r5, r0, #-0.5, r4 | st r6, [0330]
mul r3, r1, #2.0 | mul r1, r2, #6.103515625e-05
sign r6, r3
mul r3, r1, #1.1687164306640625
min r7, r6, #0.0
addxy r6, r3, #0.9998210072517395
mul r2, r6, r4 | ld r6, [035e]
mac_p1wc3 r1, r5, r0, #-0.5, r2
mac_p1w r3, r1, r7, r5, r0 | ld r7, [0368]
mul r4, r3, r6
mul r2, r4, #2.0
sign r1, r0 | st r2, [0332]
mac_p1wc3 r5, r3, r6, #-0.49994239211082458, r7
absadd r4, r1, #-1.0
mac_p1w r2, r5, r4, r3, r6 | ld r7, [0352] | ld r5, [c070]
mul r0, r2, #2.0 | mul r1, r2, r7 | st r5, [a190] | st r5, [0352]
sign r3, r0 | st r0, [0334] | st r5, [a1b9]
addxy r0, r2, #-0.5 | ld r6, [c07b] | st r1, [a1e1]
min r7, r3, #0.0 | st r6, [a1f1] | st r6, [035e]
mul r3, r0, #2.0 | addxy r6, r1, #0.2199999988079071 | ld r4, [c07d]
sign r3, r3 | st r4, [a191]
max r5, r3, -r3 | st r4, [a1c1]
max r3, r3, #0.0
mul r3, r3, r5 | mul r6, r6, #-2.0
mul r5, r3, r0
subxy r3, r2, r5 | ld r5, [c071]
mul r0, r3, r7 | ld r7, [0350]
addxy r2, r3, r0 | st r5, [0350] | ld r5, [c073]
sign r6, r6 | st r2, [0368]
max r2, r6, -r6
max r6, r6, #0.0
mul r6, r6, r2 | addxy r2, r1, #0.2202799916267395
mac_p1wc3 r2, r2, r2, #-0.5, r1
min r3, r7, #1.5 | ld r7, [c072]
addxy r2, r2, #-0.22014006972312927
max r0, r3, #0.5 | ld r3, [c07e]
mac_p1wc3 r2, r2, r6, #0.5, r1 | st r3, [0366]
mad r2, r1, #-0.5 | addxy r6, r6, #-1.0
addxy r3, r2, #0.11013999581336975
mul r3, r3, r6
addxy r6, r2, r3
addxy r2, r6, #0.11013999581336975 | addxy r6, r0, #-0.5 | ld r0, [c07c]
mac_p1wc3 r3, r2, r4, #0.5, r1
mul r2, r5, r6 | mul r5, r0, r6 | st r3, [0362]
mul r7, r7, r6 | mul r0, r5, r1 | ld r6, [0366] | st r2, [0364]
mul r7, r7, r6 | mul r5, r2, r3 | ld r2, [035e] | ld r3, [0368]
mul r6, r7, r1 | addxy r0, r0, #-0.03125 | st r5, [0366] | ld r5, [c075]
max r7, r1, #-0.0625 | st r6, [0360]
min r1, r7, #0.0625 | ld r7, [c074]
mul r5, r5, r4 | mul r0, r3, r0
mad r7, r5, #0.5 | ld r5, [c076]
mul r5, r5, r2
addxy r6, r5, #-2.4933026163133269
mul r6, r6, r5
addxy r6, r6, #-0.35210319192549572
mul r6, r6, r5
addxy r5, r6, #-0.10581673482835371
mul r6, r5, r7 | ld r7, [c077]
mul r5, r6, r3 | mul r6, r7, r2
mul r3, r4, r4 | ld r4, [c078]
mul r7, r6, r3
mul r3, r7, r4 | mov r7, r1 | ld r4, [0350]
mad r7, r3, #-0.25 | mul r3, r1, r4
min r0, r0, #0.015625
subxy r1, r3, r7
max r0, r0, #-0.015625
addxy r3, r1, #-0.052031248807907104
mul r7, r0, r3
mul r1, r7, #256.0
addxy r0, r1, #-0.125
mul r3, r0, #-8.0 | mul r0, r7, #-256.0
sign r1, r3
addxy r3, r0, #0.125 | st r1, [0368]
max r7, r3, -r3
max r0, r7, #0.000244140625
max r3, r0, #0.125
mul r7, r3, #4.0 | addxy r3, r6, #18.130695456454621
recipseed r0, r7
newton r0, r7
newton r0, r7
newton r0, r7
mul r1, r3, r6 | st r0, [035c] | ld r3, [0364]
mul r7, r1, #0.03125 | ld r1, [0362] | ld r0, [0360]
mac_p1wc3 r3, r3, r1, #-0.25, r0
addxy r0, r7, #1.1001681331638273 | mul r1, r3, r2 | ld r2, [c079] | ld r7, [0366]
mul r3, r0, r6
mad r2, r3, #-0.0066254325211048126
mad r2, r5, #0.17955571413040161
mad r2, r7, #-0.5
mad r2, r1, #0.5
addxy r3, r2, #-1.1000000238418579 | addxy r0, r2, #-2.1964182825956109
mul r5, r3, #4.0 | mul r6, r0, r2
sign r7, r5
max r1, r7, #0.0
max r3, r7, -r7
mul r5, r1, r3 | addxy r3, r2, #-1.0144000053405762
addxy r7, r6, #1.2060601122385444 | mul r6, r3, #4.0
mul r1, r5, r7
sign r5, r6
mac_p1wc3 r3, r0, r2, #0.58022848056866572, r2
min r7, r5, #0.0
subxy r6, r1, r3 | addxy r0, r7, #1.0 | ld r7, [0352] | ld r3, [c07f]
addxy r5, r6, #0.61045558468169503 | ld r6, [c080]
mac_p1wc3 r1, r5, r0, #-0.58022848056866572, r2 | ld r2, [c081]
mul r5, r6, r7 | ld r6, [c082]
mad r3, r5, #0.00390625 | mul r7, r6, r4
mul r0, r1, r3 | mad r2, r7, #0.125
addxy r5, r0, #0.59353022966335212
mul r6, r5, #-6.8938360214233398
sign r4, r6
max r7, r4, #0.0
max r5, r4, -r4
addxy r4, r0, #1.0774212709395252 | mul r6, r7, r5
mul r7, r0, r4
addxy r5, r7, #0.28720393903492503
mul r7, r6, r5 | addxy r6, r0, #0.53874214227128459
mul r5, r6, #-6.8938360214233398
sign r6, r5
min r5, r6, #0.0 | ld r6, [0368]
mul r2, r2, r6 | ld r6, [035c]
mul r2, r2, r6 | addxy r6, r0, #-0.010465713971836583
mul r6, r6, r5
mac_p1wc3 r1, r1, r3, #0.97801053524017334, r6
mac_p1wc3 r3, r0, r4, #0.010417531936634082, r1
subxy r6, r7, r3 | addxy r4, r5, #1.0
addxy r0, r6, #0.29022591364275391
mac_p1wc3 r7, r0, r4, #-0.010417531936634082, r1
shl r3, r7, 9
max r0, r3, -r3 | st r3, [a1c9]
max r4, r0, #0.0015111371817171856
max r1, r4, #0.0060445487268687426
mul r7, r1, #10.339895139264907 | mul r6, r3, #1.2924868924081134
recipseed r0, r7
newton r0, r7
sign r5, r6
newton r0, r7
addxy r6, r3, #-0.77370223703919905
newton r0, r7
mul r4, r6, #1.2924868924081134 | addxy r6, r3, #-1.5474044740783981
sign r1, r4
mul r4, r6, #1.2924868924081134
min r7, r1, #0.0
sign r1, r4
mac_p1wc3 r4, r0, r5, #0.16156086155101418, r3
min r6, r1, #0.0
addxy r1, r4, #-0.24999999999999997 | addxy r4, r6, #1.0
mac_p1wc3 r0, r1, r7, #0.16156086155101418, r3
addxy r5, r0, #-0.24999999999999997
mul r1, r5, r4
subxy r7, r1, r0
mad r7, r2, #0.5
addxy r3, r7, #-0.24999999999999997 | ld r7, [0384]
mul r6, r3, #8.0
sign r5, r6
min r4, r5, #0.0
addxy r6, r7, #-1.25 | addxy r1, r4, #1.0
mul r5, r6, #8.0
mac_p1wc3 r0, r3, r1, #-0.5, r2
sign r4, r5 | st r0, [a1d1]
addxy r2, r7, #0.25
max r3, r4, -r4
mul r0, r2, #8.0 | mul r2, r3, r6
max r1, r4, #0.0
sign r5, r0
mul r0, r2, r1
min r4, r5, #0.0
subxy r5, r7, r0
addxy r3, r5, #-0.25
mul r6, r4, r3
addxy r2, r6, r5 | mov r6, #0.041666666666666664
addxy r1, r2, #0.59374997850216871
mul r7, r1, #-5.5451774597167969
floor r0, r7
addxy r4, r0, #0.69314718055994529 | mad r2, r0, #0.18033687961558437
mul r3, r4, #0.0625
ymacd r6, r3, #0.16666666666666666
ymacd r6, r3, #0.5
ymacd r6, r3, #1.0
ymacd r6, r3, #1.0
mul r5, r6, r6 | addxy r6, r2, #0.59374997850216871
mul r1, r5, r5 | mul r5, r6, r6
mul r7, r1, r1 | addxy r1, r2, #-1.3899606075128503
mul r4, r7, r7 | mul r7, r5, r1
mul r3, r4, #0.015625 | mov r4, r2
mad r4, r7, #0.27953547022611447
addxy r0, r4, #-1.0292819975560181
mul r6, r0, r5 | mov r1, r2
mad r1, r6, #0.42706930106914331
addxy r7, r1, #-0.6686081920331266
mul r4, r7, r5 | mov r0, r2
mad r0, r4, #0.73211893606984124
addxy r6, r0, #-0.30793450019262536
mac_p1wc3 r1, r6, r5, #0.65042786114249618, r2
addxy r7, r1, #0.034303146377230566
mac_p1wc3 r4, r7, r5, #0.1269169056354455, r2
addxy r0, r4, #0.05246911125984572
mul r6, r0, r3
mul r1, r6, #-3.2040359716844296 | st r6, [a1f9]
sign r7, r1
absadd r5, r7, #-1.0
mac_p1w r2, r6, r5, r0, r3 | ld r1, [036a]
mov r4, r2 | st r2, [036a]
mad r4, r1, #-0.31210635861690367 | mul r1, r2, #-1.0
mul r7, r4, #-3.2040359716844296
max r4, r1, -r1
ph_wrap r6, r3, r7
shl r5, r2, 9
max r7, r4, #2.3811825456001562e-06
mul r3, r5, #-1.6020179858422148
max r5, r7, #4.7623650912003123e-06
sign r0, r3
max r3, r5, #6.6451013120345206e-06
shl r1, r3, 11
mul r4, r1, #1.6020179858422148 | mul r3, r6, #-0.5
recipseed r1, r4
newton r1, r4
newton r1, r4
newton r1, r4
mul r7, r1, r0 | addxy r2, r3, #-2.0
mul r5, r7, #0.015625
ph_wrap r4, r3, r2
absadd r1, r4, #-0.5
mul r0, r1, r5
max r7, r0, #-7.62939453125e-06
shl r3, r7, 15
shl r2, r3, 2
min r4, r2, #1.0
mul r1, r4, r4
addxy r7, r1, #-29.32148121184904 | mul r0, r1, r4
mul r3, r7, r1
mul r2, r3, #0.03125
addxy r7, r2, #15.600957895328319
mul r3, r7, r1
addxy r2, r3, #-126.45629034055113
mul r7, r0, r2 | ld r2, [036a]
mul r1, r7, #0.0078125 | mov r7, r2
mad r4, r1, #0.41625187771182148 | mad r7, r6, #-0.31210635861690367
mul r3, r4, r6 | mul r1, r7, #-3.2040359716844296
mul r0, r3, #-1.570796012878418
ph_wrap r3, r3, r1 | st r0, [0338]
mul r0, r3, #-0.5
addxy r7, r0, #-2.0
ph_wrap r1, r3, r7
absadd r0, r1, #-0.5
mul r7, r0, r5 | st r6, [0384]
max r1, r7, #-7.62939453125e-06
shl r0, r1, 15
shl r7, r0, 2
min r1, r7, #1.0
mul r0, r1, r1
addxy r6, r0, #-29.32148121184904 | mul r7, r0, r1
mul r6, r6, r0
mul r6, r6, #0.03125
addxy r6, r6, #15.600957895328319
mul r6, r6, r0
addxy r0, r6, #-126.45629034055113
mul r6, r7, r0
mul r7, r6, #0.0078125
mad r1, r7, #0.41625187771182148
mov r7, r2 | mul r0, r1, r3
mad r7, r3, #-0.31210635861690367 | mul r6, r0, #-1.570796012878418
mul r1, r7, #-3.2040359716844296 | st r6, [033e]
ph_wrap r6, r3, r1
mul r3, r6, #-0.5
addxy r7, r3, #-2.0
ph_wrap r1, r3, r7
absadd r3, r1, #-0.5
mul r7, r3, r5 | mad r2, r6, #-0.31210635861690367 | st r0, [035c]
max r1, r7, #-7.62939453125e-06
shl r3, r1, 15
shl r7, r3, 2
min r1, r7, #1.0
mul r3, r1, r1
addxy r0, r3, #-29.32148121184904 | mul r7, r3, r1
mul r0, r0, r3
mul r0, r0, #0.03125
addxy r0, r0, #15.600957895328319
mul r0, r0, r3
addxy r3, r0, #-126.45629034055113
mul r0, r7, r3
mul r7, r0, #0.0078125
mad r1, r7, #0.41625187771182148 | mul r7, r2, #-3.2040359716844296
mul r3, r1, r6
mul r0, r3, #-1.570796012878418
ph_wrap r1, r3, r7 | st r0, [0344]
mul r0, r1, #-0.5 | st r1, [0336]
addxy r6, r0, #-2.0
ph_wrap r2, r3, r6
absadd r7, r2, #-0.5
mul r0, r7, r5
max r6, r0, #-7.62939453125e-06
shl r2, r6, 15
shl r7, r2, 2
min r5, r7, #1.0
mul r0, r5, r5
addxy r2, r0, #-29.32148121184904 | mul r6, r0, r5
mul r7, r2, r0 | st r6, [a189]
mul r2, r7, #0.03125
addxy r7, r2, #15.600957895328319
mul r2, r7, r0
addxy r7, r2, #-126.45629034055113
mul r0, r7, r6
mul r2, r0, #0.0078125
mad r5, r2, #0.41625187771182148
mul r7, r5, r1
mul r6, r7, #-1.570796012878418
sign r0, r1 | st r6, [034a]
max r5, r0, #0.0
max r6, r0, -r0
min r2, r0, #0.0 | ld r0, [0382]
mul r1, r5, r6 | st r2, [a1d0] | ld r6, [036c]
st r2, [a1d8] | ld r2, [0384]
mac_p1wc3 r5, r4, r2, #-0.63661989959316345, r0 | st r1, [a1e0] | ld r4, [0370]
st r1, [a1e8] | ld r1, [036e]
mad r1, r6, #0.58266710469935634 | ld r2, [0372] | ld r0, [0374]
mad r1, r4, #0.38910162374577151 | ld r6, [0376] | ld r4, [0378]
mad r1, r2, #0.77034243706960015 | ld r2, [037a]
mad r1, r0, #0.91845215940572489 | ld r0, [037c]
mad r1, r6, #0.22341414130630471 | ld r6, [037e]
mad r1, r4, #0.91845215940572489
mad r1, r2, #0.77034243706960015 | ld r2, [0380]
mad r1, r0, #0.11298477383489047 | ld r0, [035c]
addxy r4, r1, r6
mad r4, r2, #0.58266710469935634
mad r4, r5, #-0.11582275620173728
mad r4, r0, #-0.17747603225581576
mad r4, r3, #-0.35093804238459891
mad r4, r7, #-0.61119927918437622
mul r1, r4, #-0.12282902002334595 | st r4, [a1f8]
sign r6, r1
max r2, r6, -r6
max r0, r6, #0.0
min r5, r6, #0.0
mul r3, r0, r2 | st r5, [a198]
st r5, [a1b8]
st r3, [a1c0]
st r3, [a1c8]
