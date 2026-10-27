mov r6, #1.0 | mov r7, #0.0 | ld r0, [0287] | ld r2, [0283] | ld r4, [c058]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | ld r1, [0309]
ph_wrap r0, r3, r0
addxy r2, r2, r0 | mov r5, #0.0 | st r0, [0286]
ph_wrap r2, r3, r2 | ld r3, [a060]
st r2, [0282] | ld r2, [030d] | st r3, [030c]
subxy r2, r2, r3 | st r3, [a168]
mul r2, r2, r2
mad r1, r1, -r2 | ld r2, [0282]
addxy r0, r1, r4
ph_wrap r1, r3, r0 | ld r3, [030b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [0241] | st r1, [0308]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [030a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [0240]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0305]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a038]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a160]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0304]
mul r0, r1, r4 | ld r4, [c05b] | ld r7, [0407]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [030e] | ld r0, [a030] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a178] | ld r4, [c044]
st r0, [0306] | st r3, [a180] | ld r3, [c071]
ld r0, [0306] | st r3, [0402] | st r3, [0408]
addxy r0, r0, r4 | ld r4, [c070]
st r0, [0302] | st r4, [0404]
ld r0, [0302]
mul r1, r0, #0.09375
addxy r2, r1, #0.09375 | ld r1, [040f]
mov r2, #1.0 | st r2, [0400]
subxy r1, r1, r2 | mov r2, #1.0
mul r2, r4, #0.5 | mad r7, r1, #-1.0 | st r2, [040e]
addxy r1, r2, #1.75
ph_wrap r4, r3, r1
mad r4, r0, #0.046875
addxy r2, r4, #-0.202667236328125
mul r0, r2, #1.9189400796122893 | mul r1, r2, #2.001832732604313
addxy r4, r0, #3.8713085459379326 | ld r0, [0409]
ymacd r4, r1, #6.6972039319949506
ymacd r4, r1, #6.8779431091386956
ymacd r4, r1, #5.1805125814717332
ymacd r4, r1, #3.3284960692113827
ymacd r4, r1, #1.8123061674294689
ymacd r4, r1, #0.74554034658491986
ymacd r4, r1, #0.20201272965749378
ymacd r4, r1, #0.027279623358226566 | subxy r1, r0, r3 | ld r3, [040d]
min r2, r4, #0.55349061079323292 | ld r4, [040b]
addxy r3, r3, r4 | mad r7, r2, #0.25 | st r4, [040a]
ph_wrap r4, r3, r7 | st r3, [040c]
addxy r2, r4, #4.5355409384067089e-06 | mad r0, r1, #0.999969482421875 | st r4, [0406]
mul r1, r2, #-2.288726083924792 | mul r7, r2, #1.0000066347860417
addxy r4, r1, #-2.4503958974381903e-09 | mul r1, r3, #1.7763568394002505e-15
ymacd r4, r7, #3.3696628551494898
ymacd r4, r7, #1.8592902910211674e-09
ymacd r4, r7, #-0.16690143041583722 | mul r7, r1, #1.0
mul r2, r0, r4
addxy r0, r2, r7
mul r4, r0, #256.0
mul r3, r4, #256.0 | ld r4, [a040]
mul r2, r3, #3.8014329398411659 | mul r1, r3, #4.0000199974582964 | ld r3, [a061]
addxy r7, r2, #5.4728886258300858e-09 | ld r2, [031d] | st r3, [031c] | st r3, [a169]
ymacd r7, r1, #-3.8340151140824563 | subxy r2, r2, r3
ymacd r7, r1, #2.0041271441752748e-07 | mul r2, r2, r2
ymacd r7, r1, #4.878100807652002
ymacd r7, r1, #-2.2255175827751988e-07
ymacd r7, r1, #-0.99114349708082206 | ld r1, [0319]
mad r1, r1, -r2 | st r7, [0300] | ld r2, [0282]
ld r0, [0300]
mul r0, r0, r4 | ld r4, [030e]
mad r0, r0, r4 | ld r4, [c058]
addxy r0, r1, r4 | st r0, [a151]
ph_wrap r1, r3, r0 | ld r3, [031b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [0243] | st r1, [0318]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [031a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [0242]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0315]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a039]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a161]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0314]
mul r0, r1, r4 | ld r4, [c05b] | ld r7, [0427]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [031e] | ld r0, [a031] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a179] | ld r4, [c044]
st r0, [0316] | st r3, [a181] | ld r3, [c071]
ld r0, [0316] | st r3, [0422] | st r3, [0428]
addxy r0, r0, r4 | ld r4, [c070]
st r0, [0312] | st r4, [0424]
ld r0, [0312]
mul r1, r0, #0.09375
addxy r2, r1, #0.09375 | ld r1, [042f]
mov r2, #1.0 | st r2, [0420]
subxy r1, r1, r2 | mov r2, #1.0
mul r2, r4, #0.5 | mad r7, r1, #-1.0 | st r2, [042e]
addxy r1, r2, #1.75
ph_wrap r4, r3, r1
mad r4, r0, #0.046875
addxy r2, r4, #-0.202667236328125
mul r0, r2, #1.9189400796122893 | mul r1, r2, #2.001832732604313
addxy r4, r0, #3.8713085459379326 | ld r0, [0429]
ymacd r4, r1, #6.6972039319949506
ymacd r4, r1, #6.8779431091386956
ymacd r4, r1, #5.1805125814717332
ymacd r4, r1, #3.3284960692113827
ymacd r4, r1, #1.8123061674294689
ymacd r4, r1, #0.74554034658491986
ymacd r4, r1, #0.20201272965749378
ymacd r4, r1, #0.027279623358226566 | subxy r1, r0, r3 | ld r3, [042d]
min r2, r4, #0.55349061079323292 | ld r4, [042b]
addxy r3, r3, r4 | mad r7, r2, #0.25 | st r4, [042a]
ph_wrap r4, r3, r7 | st r3, [042c]
addxy r2, r4, #4.5355409384067089e-06 | mad r0, r1, #0.999969482421875 | st r4, [0426]
mul r1, r2, #-2.288726083924792 | mul r7, r2, #1.0000066347860417
addxy r4, r1, #-2.4503958974381903e-09 | mul r1, r3, #1.7763568394002505e-15
ymacd r4, r7, #3.3696628551494898
ymacd r4, r7, #1.8592902910211674e-09
ymacd r4, r7, #-0.16690143041583722 | mul r7, r1, #1.0
mul r2, r0, r4
addxy r0, r2, r7
mul r4, r0, #256.0
mul r3, r4, #256.0 | ld r4, [a040]
mul r2, r3, #3.8014329398411659 | mul r1, r3, #4.0000199974582964 | ld r3, [a062]
addxy r7, r2, #5.4728886258300858e-09 | ld r2, [032d] | st r3, [032c] | st r3, [a16a]
ymacd r7, r1, #-3.8340151140824563 | subxy r2, r2, r3
ymacd r7, r1, #2.0041271441752748e-07 | mul r2, r2, r2
ymacd r7, r1, #4.878100807652002
ymacd r7, r1, #-2.2255175827751988e-07
ymacd r7, r1, #-0.99114349708082206 | ld r1, [0329]
mad r1, r1, -r2 | st r7, [0310] | ld r2, [0282]
ld r0, [0310]
mul r0, r0, r4 | ld r4, [031e]
mad r0, r0, r4 | ld r4, [c058]
addxy r0, r1, r4 | st r0, [a152]
ph_wrap r1, r3, r0 | ld r3, [032b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [0245] | st r1, [0328]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [032a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [0244]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0325]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a03a]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a162]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0324]
mul r0, r1, r4 | ld r4, [c05b] | ld r7, [0447]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [032e] | ld r0, [a032] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17a] | ld r4, [c044]
st r0, [0326] | st r3, [a182] | ld r3, [c071]
ld r0, [0326] | st r3, [0442] | st r3, [0448]
addxy r0, r0, r4 | ld r4, [c070]
st r0, [0322] | st r4, [0444]
ld r0, [0322]
mul r1, r0, #0.09375
addxy r2, r1, #0.09375 | ld r1, [044f]
mov r2, #1.0 | st r2, [0440]
subxy r1, r1, r2 | mov r2, #1.0
mul r2, r4, #0.5 | mad r7, r1, #-1.0 | st r2, [044e]
addxy r1, r2, #1.75
ph_wrap r4, r3, r1
mad r4, r0, #0.046875
addxy r2, r4, #-0.202667236328125
mul r0, r2, #1.9189400796122893 | mul r1, r2, #2.001832732604313
addxy r4, r0, #3.8713085459379326 | ld r0, [0449]
ymacd r4, r1, #6.6972039319949506
ymacd r4, r1, #6.8779431091386956
ymacd r4, r1, #5.1805125814717332
ymacd r4, r1, #3.3284960692113827
ymacd r4, r1, #1.8123061674294689
ymacd r4, r1, #0.74554034658491986
ymacd r4, r1, #0.20201272965749378
ymacd r4, r1, #0.027279623358226566 | subxy r1, r0, r3 | ld r3, [044d]
min r2, r4, #0.55349061079323292 | ld r4, [044b]
addxy r3, r3, r4 | mad r7, r2, #0.25 | st r4, [044a]
ph_wrap r4, r3, r7 | st r3, [044c]
addxy r2, r4, #4.5355409384067089e-06 | mad r0, r1, #0.999969482421875 | st r4, [0446]
mul r1, r2, #-2.288726083924792 | mul r7, r2, #1.0000066347860417
addxy r4, r1, #-2.4503958974381903e-09 | mul r1, r3, #1.7763568394002505e-15
ymacd r4, r7, #3.3696628551494898
ymacd r4, r7, #1.8592902910211674e-09
ymacd r4, r7, #-0.16690143041583722 | mul r7, r1, #1.0
mul r2, r0, r4
addxy r0, r2, r7
mul r4, r0, #256.0
mul r3, r4, #256.0 | ld r4, [a040]
mul r2, r3, #3.8014329398411659 | mul r1, r3, #4.0000199974582964 | ld r3, [a063]
addxy r7, r2, #5.4728886258300858e-09 | ld r2, [033d] | st r3, [033c] | st r3, [a16b]
ymacd r7, r1, #-3.8340151140824563 | subxy r2, r2, r3
ymacd r7, r1, #2.0041271441752748e-07 | mul r2, r2, r2
ymacd r7, r1, #4.878100807652002
ymacd r7, r1, #-2.2255175827751988e-07
ymacd r7, r1, #-0.99114349708082206 | ld r1, [0339]
mad r1, r1, -r2 | st r7, [0320] | ld r2, [0282]
ld r0, [0320]
mul r0, r0, r4 | ld r4, [032e]
mad r0, r0, r4 | ld r4, [c058]
addxy r0, r1, r4 | st r0, [a153]
ph_wrap r1, r3, r0 | ld r3, [033b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [0247] | st r1, [0338]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [033a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [0246]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0335]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a03b]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a163]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0334]
mul r0, r1, r4 | ld r4, [c05b] | ld r7, [0467]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [033e] | ld r0, [a033] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17b] | ld r4, [c044]
st r0, [0336] | st r3, [a183] | ld r3, [c071]
ld r0, [0336] | st r3, [0462] | st r3, [0468]
addxy r0, r0, r4 | ld r4, [c070]
st r0, [0332] | st r4, [0464]
ld r0, [0332]
mul r1, r0, #0.09375
addxy r2, r1, #0.09375 | ld r1, [046f]
mov r2, #1.0 | st r2, [0460]
subxy r1, r1, r2 | mov r2, #1.0
mul r2, r4, #0.5 | mad r7, r1, #-1.0 | st r2, [046e]
addxy r1, r2, #1.75
ph_wrap r4, r3, r1
mad r4, r0, #0.046875
addxy r2, r4, #-0.202667236328125
mul r0, r2, #1.9189400796122893 | mul r1, r2, #2.001832732604313
addxy r4, r0, #3.8713085459379326 | ld r0, [0469]
ymacd r4, r1, #6.6972039319949506
ymacd r4, r1, #6.8779431091386956
ymacd r4, r1, #5.1805125814717332
ymacd r4, r1, #3.3284960692113827
ymacd r4, r1, #1.8123061674294689
ymacd r4, r1, #0.74554034658491986
ymacd r4, r1, #0.20201272965749378
ymacd r4, r1, #0.027279623358226566 | subxy r1, r0, r3 | ld r3, [046d]
min r2, r4, #0.55349061079323292 | ld r4, [046b]
addxy r3, r3, r4 | mad r7, r2, #0.25 | st r4, [046a]
ph_wrap r4, r3, r7 | st r3, [046c]
addxy r2, r4, #4.5355409384067089e-06 | mad r0, r1, #0.999969482421875 | st r4, [0466]
mul r1, r2, #-2.288726083924792 | mul r7, r2, #1.0000066347860417
addxy r4, r1, #-2.4503958974381903e-09 | mul r1, r3, #1.7763568394002505e-15
ymacd r4, r7, #3.3696628551494898
ymacd r4, r7, #1.8592902910211674e-09
ymacd r4, r7, #-0.16690143041583722 | mul r7, r1, #1.0
mul r2, r0, r4
addxy r0, r2, r7
mul r4, r0, #256.0
mul r3, r4, #256.0 | ld r4, [a040]
mul r2, r3, #3.8014329398411659 | mul r1, r3, #4.0000199974582964 | ld r3, [a064]
addxy r7, r2, #5.4728886258300858e-09 | ld r2, [034d] | st r3, [034c] | st r3, [a16c]
ymacd r7, r1, #-3.8340151140824563 | subxy r2, r2, r3
ymacd r7, r1, #2.0041271441752748e-07 | mul r2, r2, r2
ymacd r7, r1, #4.878100807652002
ymacd r7, r1, #-2.2255175827751988e-07
ymacd r7, r1, #-0.99114349708082206 | ld r1, [0349]
mad r1, r1, -r2 | st r7, [0330] | ld r2, [0282]
ld r0, [0330]
mul r0, r0, r4 | ld r4, [033e]
mad r0, r0, r4 | ld r4, [c058]
addxy r0, r1, r4 | st r0, [a154]
ph_wrap r1, r3, r0 | ld r3, [034b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [0249] | st r1, [0348]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [034a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [0248]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0345]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a03c]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a164]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0344]
mul r0, r1, r4 | ld r4, [c05b] | ld r7, [0487]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [034e] | ld r0, [a034] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17c] | ld r4, [c044]
st r0, [0346] | st r3, [a184] | ld r3, [c071]
ld r0, [0346] | st r3, [0482] | st r3, [0488]
addxy r0, r0, r4 | ld r4, [c070]
st r0, [0342] | st r4, [0484]
ld r0, [0342]
mul r1, r0, #0.09375
addxy r2, r1, #0.09375 | ld r1, [048f]
mov r2, #1.0 | st r2, [0480]
subxy r1, r1, r2 | mov r2, #1.0
mul r2, r4, #0.5 | mad r7, r1, #-1.0 | st r2, [048e]
addxy r1, r2, #1.75
ph_wrap r4, r3, r1
mad r4, r0, #0.046875
addxy r2, r4, #-0.202667236328125
mul r0, r2, #1.9189400796122893 | mul r1, r2, #2.001832732604313
addxy r4, r0, #3.8713085459379326 | ld r0, [0489]
ymacd r4, r1, #6.6972039319949506
ymacd r4, r1, #6.8779431091386956
ymacd r4, r1, #5.1805125814717332
ymacd r4, r1, #3.3284960692113827
ymacd r4, r1, #1.8123061674294689
ymacd r4, r1, #0.74554034658491986
ymacd r4, r1, #0.20201272965749378
ymacd r4, r1, #0.027279623358226566 | subxy r1, r0, r3 | ld r3, [048d]
min r2, r4, #0.55349061079323292 | ld r4, [048b]
addxy r3, r3, r4 | mad r7, r2, #0.25 | st r4, [048a]
ph_wrap r4, r3, r7 | st r3, [048c]
addxy r2, r4, #4.5355409384067089e-06 | mad r0, r1, #0.999969482421875 | st r4, [0486]
mul r1, r2, #-2.288726083924792 | mul r7, r2, #1.0000066347860417
addxy r4, r1, #-2.4503958974381903e-09 | mul r1, r3, #1.7763568394002505e-15
ymacd r4, r7, #3.3696628551494898
ymacd r4, r7, #1.8592902910211674e-09
ymacd r4, r7, #-0.16690143041583722 | mul r7, r1, #1.0
mul r2, r0, r4
addxy r0, r2, r7
mul r4, r0, #256.0
mul r3, r4, #256.0 | ld r4, [a040]
mul r2, r3, #3.8014329398411659 | mul r1, r3, #4.0000199974582964 | ld r3, [a065]
addxy r7, r2, #5.4728886258300858e-09 | ld r2, [035d] | st r3, [035c] | st r3, [a16d]
ymacd r7, r1, #-3.8340151140824563 | subxy r2, r2, r3
ymacd r7, r1, #2.0041271441752748e-07 | mul r2, r2, r2
ymacd r7, r1, #4.878100807652002
ymacd r7, r1, #-2.2255175827751988e-07
ymacd r7, r1, #-0.99114349708082206 | ld r1, [0359]
mad r1, r1, -r2 | st r7, [0340] | ld r2, [0282]
ld r0, [0340]
mul r0, r0, r4 | ld r4, [034e]
mad r0, r0, r4 | ld r4, [c058]
addxy r0, r1, r4 | st r0, [a155]
ph_wrap r1, r3, r0 | ld r3, [035b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [024b] | st r1, [0358]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [035a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [024a]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0355]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a03d]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a165]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0354]
mul r0, r1, r4 | ld r4, [c05b] | ld r7, [04a7]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [035e] | ld r0, [a035] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17d] | ld r4, [c044]
st r0, [0356] | st r3, [a185] | ld r3, [c071]
ld r0, [0356] | st r3, [04a2] | st r3, [04a8]
addxy r0, r0, r4 | ld r4, [c070]
st r0, [0352] | st r4, [04a4]
ld r0, [0352]
mul r1, r0, #0.09375
addxy r2, r1, #0.09375 | ld r1, [04af]
mov r2, #1.0 | st r2, [04a0]
subxy r1, r1, r2 | mov r2, #1.0
mul r2, r4, #0.5 | mad r7, r1, #-1.0 | st r2, [04ae]
addxy r1, r2, #1.75
ph_wrap r4, r3, r1
mad r4, r0, #0.046875
addxy r2, r4, #-0.202667236328125
mul r0, r2, #1.9189400796122893 | mul r1, r2, #2.001832732604313
addxy r4, r0, #3.8713085459379326 | ld r0, [04a9]
ymacd r4, r1, #6.6972039319949506
ymacd r4, r1, #6.8779431091386956
ymacd r4, r1, #5.1805125814717332
ymacd r4, r1, #3.3284960692113827
ymacd r4, r1, #1.8123061674294689
ymacd r4, r1, #0.74554034658491986
ymacd r4, r1, #0.20201272965749378
ymacd r4, r1, #0.027279623358226566 | subxy r1, r0, r3 | ld r3, [04ad]
min r2, r4, #0.55349061079323292 | ld r4, [04ab]
addxy r3, r3, r4 | mad r7, r2, #0.25 | st r4, [04aa]
ph_wrap r4, r3, r7 | st r3, [04ac]
addxy r2, r4, #4.5355409384067089e-06 | mad r0, r1, #0.999969482421875 | st r4, [04a6]
mul r1, r2, #-2.288726083924792 | mul r7, r2, #1.0000066347860417
addxy r4, r1, #-2.4503958974381903e-09 | mul r1, r3, #1.7763568394002505e-15
ymacd r4, r7, #3.3696628551494898
ymacd r4, r7, #1.8592902910211674e-09
ymacd r4, r7, #-0.16690143041583722 | mul r7, r1, #1.0
mul r2, r0, r4
addxy r0, r2, r7
mul r4, r0, #256.0
mul r3, r4, #256.0 | ld r4, [a040]
mul r2, r3, #3.8014329398411659 | mul r1, r3, #4.0000199974582964 | ld r3, [a066]
addxy r7, r2, #5.4728886258300858e-09 | ld r2, [036d] | st r3, [036c] | st r3, [a16e]
ymacd r7, r1, #-3.8340151140824563 | subxy r2, r2, r3
ymacd r7, r1, #2.0041271441752748e-07 | mul r2, r2, r2
ymacd r7, r1, #4.878100807652002
ymacd r7, r1, #-2.2255175827751988e-07
ymacd r7, r1, #-0.99114349708082206 | ld r1, [0369]
mad r1, r1, -r2 | st r7, [0350] | ld r2, [0282]
ld r0, [0350]
mul r0, r0, r4 | ld r4, [035e]
mad r0, r0, r4 | ld r4, [c058]
addxy r0, r1, r4 | st r0, [a156]
ph_wrap r1, r3, r0 | ld r3, [036b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [024d] | st r1, [0368]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [036a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [024c]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0365]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a03e]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a166]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0364]
mul r0, r1, r4 | ld r4, [c05b] | ld r7, [04c7]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [036e] | ld r0, [a036] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17e] | ld r4, [c044]
st r0, [0366] | st r3, [a186] | ld r3, [c071]
ld r0, [0366] | st r3, [04c2] | st r3, [04c8]
addxy r0, r0, r4 | ld r4, [c070]
st r0, [0362] | st r4, [04c4]
ld r0, [0362]
mul r1, r0, #0.09375
addxy r2, r1, #0.09375 | ld r1, [04cf]
mov r2, #1.0 | st r2, [04c0]
subxy r1, r1, r2 | mov r2, #1.0
mul r2, r4, #0.5 | mad r7, r1, #-1.0 | st r2, [04ce]
addxy r1, r2, #1.75
ph_wrap r4, r3, r1
mad r4, r0, #0.046875
addxy r2, r4, #-0.202667236328125
mul r0, r2, #1.9189400796122893 | mul r1, r2, #2.001832732604313
addxy r4, r0, #3.8713085459379326 | ld r0, [04c9]
ymacd r4, r1, #6.6972039319949506
ymacd r4, r1, #6.8779431091386956
ymacd r4, r1, #5.1805125814717332
ymacd r4, r1, #3.3284960692113827
ymacd r4, r1, #1.8123061674294689
ymacd r4, r1, #0.74554034658491986
ymacd r4, r1, #0.20201272965749378
ymacd r4, r1, #0.027279623358226566 | subxy r1, r0, r3 | ld r3, [04cd]
min r2, r4, #0.55349061079323292 | ld r4, [04cb]
addxy r3, r3, r4 | mad r7, r2, #0.25 | st r4, [04ca]
ph_wrap r4, r3, r7 | st r3, [04cc]
addxy r2, r4, #4.5355409384067089e-06 | mad r0, r1, #0.999969482421875 | st r4, [04c6]
mul r1, r2, #-2.288726083924792 | mul r7, r2, #1.0000066347860417
addxy r4, r1, #-2.4503958974381903e-09 | mul r1, r3, #1.7763568394002505e-15
ymacd r4, r7, #3.3696628551494898
ymacd r4, r7, #1.8592902910211674e-09
ymacd r4, r7, #-0.16690143041583722 | mul r7, r1, #1.0
mul r2, r0, r4
addxy r0, r2, r7
mul r4, r0, #256.0
mul r3, r4, #256.0 | ld r4, [a040]
mul r2, r3, #3.8014329398411659 | mul r1, r3, #4.0000199974582964 | ld r3, [a067]
addxy r7, r2, #5.4728886258300858e-09 | ld r2, [037d] | st r3, [037c] | st r3, [a16f]
ymacd r7, r1, #-3.8340151140824563 | subxy r2, r2, r3
ymacd r7, r1, #2.0041271441752748e-07 | mul r2, r2, r2
ymacd r7, r1, #4.878100807652002
ymacd r7, r1, #-2.2255175827751988e-07
ymacd r7, r1, #-0.99114349708082206 | ld r1, [0379]
mad r1, r1, -r2 | st r7, [0360] | ld r2, [0282]
ld r0, [0360]
mul r0, r0, r4 | ld r4, [036e]
mad r0, r0, r4 | ld r4, [c058]
addxy r0, r1, r4 | st r0, [a157]
ph_wrap r1, r3, r0 | ld r3, [037b]
subxy r0, r1, r0 | subxy r4, r3, r2 | ld r2, [024f] | st r1, [0378]
mul r4, r4, r0
mad r3, r4, #0.5 | ld r4, [c058]
mul r4, r4, #4.0 | subxy r0, r2, r3 | st r3, [037a]
min r4, r4, r6
mad r2, r0, r4 | ld r4, [c065]
mul r7, r3, r4 | mul r3, r1, #256.0 | ld r4, [c064] | st r2, [024e]
mad r7, r2, r4 | ph_clamp r3, r3, -r3 | ld r4, [c062] | ld r2, [0375]
mad r7, r1, r4 | ld r4, [c063]
mad r7, r3, r4 | ld r4, [c061]
ph_tri r3, r3, -r1
mul r0, r3, r3 | mad r7, r3, r4 | ld r4, [c060]
mul r0, r0, #-0.5
add r0, #1.5
mul r0, r0, r3 | ld r3, [a03f]
mad r7, r0, r4 | ld r4, [c059] | st r3, [a167]
addxy r2, r2, r4 | ld r4, [c05c]
mul r2, r2, r3 | ld r3, [c05e]
min r2, r2, r6
mul r1, r7, r2 | mov r7, #0.0 | st r2, [0374]
mul r0, r1, r4 | ld r4, [c05b] | ld r7, [04e7]
mad r0, r2, -r3 | ld r3, [c05d]
mad r3, r1, r4 | st r0, [037e] | ld r0, [a037] | ld r4, [c05a]
mad r0, r1, r4 | st r0, [a17f] | ld r4, [c044]
st r0, [0376] | st r3, [a187] | ld r3, [c071]
ld r0, [0376] | st r3, [04e2] | st r3, [04e8]
addxy r0, r0, r4 | ld r4, [c070]
st r0, [0372] | st r4, [04e4]
ld r0, [0372]
mul r1, r0, #0.09375
addxy r2, r1, #0.09375 | ld r1, [04ef]
mov r2, #1.0 | st r2, [04e0]
subxy r1, r1, r2 | mov r2, #1.0
mul r2, r4, #0.5 | mad r7, r1, #-1.0 | st r2, [04ee]
addxy r1, r2, #1.75
ph_wrap r4, r3, r1
mad r4, r0, #0.046875
addxy r2, r4, #-0.202667236328125
mul r0, r2, #1.9189400796122893 | mul r1, r2, #2.001832732604313
addxy r4, r0, #3.8713085459379326 | ld r0, [04e9]
ymacd r4, r1, #6.6972039319949506
ymacd r4, r1, #6.8779431091386956
ymacd r4, r1, #5.1805125814717332
ymacd r4, r1, #3.3284960692113827
ymacd r4, r1, #1.8123061674294689
ymacd r4, r1, #0.74554034658491986
ymacd r4, r1, #0.20201272965749378
ymacd r4, r1, #0.027279623358226566 | subxy r1, r0, r3 | ld r3, [04ed]
min r2, r4, #0.55349061079323292 | ld r4, [04eb]
addxy r3, r3, r4 | mad r7, r2, #0.25 | st r4, [04ea]
ph_wrap r4, r3, r7 | st r3, [04ec]
addxy r2, r4, #4.5355409384067089e-06 | mad r0, r1, #0.999969482421875 | st r4, [04e6]
mul r1, r2, #-2.288726083924792 | mul r7, r2, #1.0000066347860417
addxy r4, r1, #-2.4503958974381903e-09 | mul r1, r3, #1.7763568394002505e-15
ymacd r4, r7, #3.3696628551494898
ymacd r4, r7, #1.8592902910211674e-09
ymacd r4, r7, #-0.16690143041583722 | mul r7, r1, #1.0
mul r2, r0, r4
addxy r0, r2, r7
mul r4, r0, #256.0
mul r3, r4, #256.0 | ld r4, [a040]
mul r2, r3, #3.8014329398411659 | mul r1, r3, #4.0000199974582964
addxy r7, r2, #5.4728886258300858e-09
ymacd r7, r1, #-3.8340151140824563
ymacd r7, r1, #2.0041271441752748e-07
ymacd r7, r1, #4.878100807652002
ymacd r7, r1, #-2.2255175827751988e-07
ymacd r7, r1, #-0.99114349708082206
st r7, [0370]
ld r0, [0370]
mul r0, r0, r4 | ld r4, [037e]
mad r0, r0, r4
st r0, [a158]
