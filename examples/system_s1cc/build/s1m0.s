ld r2, [a038] | ld r5, [0305]
st r2, [a160] | ld r2, [c04b]
ld r0, [a060] | ld r4, [c080]
ld r1, [a120]
mul r1, r1, r2 | st r0, [a168] | ld r2, [c05e]
ld r0, [a128]
mad r1, r0, r2 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a030] | ld r0, [c07d]
addxy r2, r2, r1 | ld r1, [0303]
min r2, r1, #1.0 | st r2, [0300]
max r3, r2, #-1.0 | ld r2, [0307]
subxy r6, r3, r5 | st r3, [0304]
sign r7, r6
min r1, r7, #0.0
mul r3, r1, r2 | mov r7, r2
shl r6, r3, 10 | ld r3, [0309]
mad r7, r6, #0.0009765625 | mad r2, r4, #0.0009765625 | ld r6, [c07e]
mac_p1wc3 r3, r3, r0, #0.015625, r6
min r0, r3, #1.0
max r6, r0, #-1.0
subxy r3, r7, r6 | addxy r0, r1, #1.0
mul r6, r3, r0
addxy r3, r6, r7 | mov r6, #0.0
max r7, r3, #0.80000000000000004 | st r6, [a151] | st r3, [0308]
addxy r6, r7, #-0.80000000000000004 | addxy r7, r5, #-1.0
mul r3, r6, r6 | mul r6, r1, r7
addxy r4, r6, r5
addxy r1, r4, #-1.0
mul r7, r1, r0
subxy r6, r4, r7
mul r5, r6, r2
mul r1, r5, #-1.0
ph_wrap r0, r3, r1
ph_wrap r4, r3, r0 | st r0, [0306]
max r7, r4, -r4
mul r6, r7, r7
absadd r2, r4, #-1.5024135451971183
mac_p1wc3 r5, r6, r2, #0.020000123795061404, r7 | ld r7, [a038] | ld r2, [0300]
mul r1, r3, r3 | addxy r4, r5, #0.24230911718532214 | st r2, [a1c8] | ld r3, [030b]
shl r0, r1, 9
mul r6, r0, r4
sign r1, r7 | st r6, [a198]
max r5, r1, -r1
absadd r1, r1, #-1.0
addxy r6, r3, #1.0007476806640625 | st r6, [0360]
mul r1, r1, r6 | st r2, [0362] | ld r2, [0313]
addxy r6, r1, r3 | st r7, [0364]
min r1, r3, #-0.00074767856858670712 | st r0, [0366] | ld r0, [c075]
subxy r3, r6, r1 | addxy r2, r2, #-1.0 | st r4, [0368] | ld r4, [c070]
mul r1, r3, r5
shl r3, r1, 10
mad r6, r3, #0.0009765625
addxy r5, r6, #0.00074767856858670712 | ld r6, [030d]
sign r1, r5 | st r5, [030a] | ld r5, [030f]
addxy r3, r1, #1.0 | st r6, [030e]
st r3, [0302] | st r3, [0310]
subxy r3, r5, r6 | st r3, [0314]
sign r5, r3
max r3, r5, #0.0
max r5, r5, -r5
mul r3, r3, r5
addxy r5, r3, #-1.0
mul r3, r5, r6
addxy r5, r6, r3
addxy r3, r5, #-0.89249998331069946
sign r5, r3
min r3, r5, #0.0 | ld r5, [0311]
subxy r5, r5, r1 | addxy r3, r3, #1.0
sign r5, r5
min r5, r5, #0.0
mul r5, r5, r2
addxy r2, r5, #-1.0
mul r2, r2, r3
subxy r3, r2, r5
mul r2, r3, #-1.0
addxy r5, r2, #1.0
sign r2, r1 | st r5, [0312]
min r5, r2, #0.0
mul r2, r5, r1 | addxy r5, r5, #1.0
addxy r2, r1, r2
addxy r7, r2, #1.0
mul r7, r7, r5
subxy r5, r7, r2
mul r7, r5, #-1.0 | st r5, [036a]
sign r2, r7
max r7, r2, #0.0
max r2, r2, -r2
mul r7, r7, r2 | ld r2, [c071]
mad r0, r2, #0.03125 | addxy r2, r3, #-1.0
mul r3, r5, r2 | addxy r5, r6, #-1.0499999523162842
sign r2, r3
max r3, r2, #0.0
max r2, r2, -r2
mul r2, r2, r5
max r4, r4, #0.000507354736328125
mul r5, r2, r3
mul r2, r5, r4 | ld r5, [c072]
mad r6, r2, #-0.015625
subxy r3, r0, r6
mul r4, r3, r5 | ld r3, [c073]
addxy r2, r4, r0 | ld r4, [c074]
max r6, r2, #0.0
mov r5, r6
mad r5, r3, #-0.03125
mac_p1wc3 r0, r5, r4, #0.03125, r3
max r2, r0, #0.0
subxy r5, r6, r2 | addxy r4, r7, #-1.0
mul r3, r5, r4 | ld r5, [0315]
subxy r0, r6, r3 | subxy r6, r5, r1 | ld r3, [0317]
mul r2, r0, #-1.0 | mul r7, r0, r0 | st r3, [0318]
st r2, [030c] | ld r2, [0319] | st r7, [a1b8]
subxy r0, r2, r3
sign r5, r0
max r1, r5, #0.0
max r2, r5, -r5
mul r0, r1, r2
addxy r5, r0, #-1.0
mul r1, r5, r3
addxy r2, r3, r1
addxy r0, r2, #-0.89249998331069946
sign r2, r6 | ld r6, [036a]
sign r5, r0
min r0, r2, #0.0
min r1, r5, #0.0
addxy r5, r0, #1.0 | addxy r2, r6, #1.0
mul r5, r5, r2
subxy r2, r6, r5 | ld r5, [031b]
subxy r5, r2, r5
mul r5, r5, r0
subxy r0, r2, r5 | addxy r2, r1, #1.0
mul r5, r0, r2
subxy r1, r5, r0 | ld r5, [c077]
mul r2, r1, #-1.0 | ld r0, [c07b]
mul r2, r1, r6 | mad r0, r5, #0.03125 | st r2, [031a]
sign r5, r2 | ld r2, [c076]
max r1, r5, #0.0
max r6, r5, -r5
max r5, r2, #0.0002536773681640625
addxy r2, r3, #-1.0499999523162842
mul r6, r6, r2
mul r2, r6, r1
mul r6, r2, r5 | ld r2, [c078]
mad r3, r6, #-0.03125
subxy r1, r0, r3
mul r5, r1, r2 | ld r1, [c079]
addxy r6, r5, r0 | ld r5, [c07a]
max r3, r6, #0.0
mov r2, r3
mad r2, r1, #-0.03125
mac_p1wc3 r0, r2, r5, #0.03125, r1
max r6, r0, #0.0
subxy r2, r3, r6
mul r5, r2, r4 | ld r2, [0366] | ld r4, [0368]
subxy r1, r3, r5
mul r0, r1, #-1.0 | mul r6, r1, r1
mac_p1wc3 r3, r2, r4, #-0.019691418342943499, r7 | st r0, [0316] | ld r0, [0364]
addxy r5, r3, #-0.19692434819607671 | st r6, [a1c0] | ld r6, [031d] | ld r2, [c084]
mac_p1wc3 r1, r5, r0, #-0.19692434819607671, r0 | ld r0, [0362] | ld r4, [c07f]
mac_p1wc3 r2, r2, r6, #0.125, r0 | ld r3, [c07c] | st r6, [a190]
st r2, [a188] | ld r2, [0360] | ld r6, [c083]
mac_p1wc3 r4, r4, r2, #-0.19692434819607671, r0 | ld r5, [c082]
mul r2, r3, r7
mad r4, r2, #-0.078765673371773995
mad r4, r6, #-0.012307771762254794
addxy r0, r4, #0.93539062006379303
mul r3, r0, #-3.5198653128176716
floor r7, r3
mad r4, r7, #0.28410177979210643
addxy r2, r4, #0.93539062006379303 | addxy r0, r4, #-2.1897366932215268
mul r6, r2, r2 | mov r2, r4
mul r3, r6, r0
mad r2, r3, #0.11263104563206487
addxy r0, r2, #-1.6215254918293969
mul r3, r0, r6 | mov r2, r4
mad r2, r3, #0.17207570079698273
addxy r0, r2, #-1.053321859317446
mul r3, r0, r6 | mov r2, r4
mad r2, r3, #0.29498696973904698
addxy r0, r2, #-0.48511840590013933 | addxy r2, r7, #0.69314718055994529
mul r3, r0, r6 | mul r0, r2, #0.0625
mov r7, #0.041666666666666664
ymacd r7, r0, #0.16666666666666666
ymacd r7, r0, #0.5
ymacd r7, r0, #1.0
ymacd r7, r0, #1.0
mul r2, r7, r7
mul r0, r2, r2
mul r7, r0, r0
mul r2, r7, r7 | mov r7, r4
mad r7, r3, #0.61947253563368831 | mul r0, r2, #0.015625
addxy r2, r7, #0.083085306088151345
mac_p1wc3 r3, r2, r6, #0.48428291325349976, r4
addxy r7, r3, #0.31540805693626561
mul r2, r0, r7
mul r4, r2, #-0.26650067457349186
max r6, r2, #-0.9343315861092738
sign r3, r4
absadd r0, r3, #-1.0
mul r7, r6, r0 | st r1, [0362]
addxy r2, r7, r6 | st r5, [0364]
mul r0, r2, #-1.0 | mul r4, r2, #-68.224172690813916 | st r2, [0366]
max r7, r0, -r0
max r6, r7, #2.8628049604227441e-05
sign r3, r4
max r4, r6, #5.7256099208454882e-05
shl r0, r4, 8
mul r7, r0, #1.0660026982939674
recipseed r0, r7
newton r0, r7
newton r0, r7
newton r0, r7
mul r6, r3, r0 | mul r1, r2, #-0.26650067457349186 | ld r3, [031f]
mad r3, r2, #-0.26650067457349186 | mul r4, r6, #0.015625
ph_wrap r0, r3, r3
ph_wrap r7, r3, r1
sign r6, r0 | st r7, [0360]
min r1, r6, #0.0
subxy r3, r0, r7
mul r6, r1, r3 | ld r3, [0362]
subxy r1, r0, r6
mac_p1wc3 r0, r3, r5, #-0.10940246203529515, r1
addxy r6, r0, #-0.0032820707310293085
mul r0, r6, #9.1405621171247731
sign r0, r0
min r7, r0, #0.0 | st r0, [0368]
mac_p1wc3 r7, r3, r5, #0.2188049240705903, r7
addxy r7, r7, #0.10612039130426584
mul r5, r7, #9.1405621171247731
max r7, r7, -r7
max r7, r7, #0.00021367668366268584
max r7, r7, #0.027350615508823787
mul r7, r7, #9.1405621171247731
recipseed r2, r7
newton r2, r7
newton r2, r7
newton r2, r7
max r0, r2, #0.30521833896636963
sign r5, r5
min r3, r0, #16.0
mul r7, r3, r5
min r2, r7, #16.0
max r0, r2, #-0.97087383270263672
mul r3, r0, r6
addxy r5, r3, #0.16410369305294273
mul r7, r5, #-9.1405621171247731
ph_wrap r2, r3, r7
absadd r0, r2, #-0.5
mul r6, r0, r4 | ld r2, [0366]
max r3, r6, #-3.8147009036072765e-05
shl r5, r3, 14 | ld r3, [0360]
mad r1, r2, #0.26650067457349186
min r7, r5, #0.62500059604701619
mul r0, r1, #-1.0 | mov r5, r3
ph_wrap r6, r3, r0 | ld r0, [0368]
mad r5, r2, #-0.26650067457349186
ph_wrap r1, r3, r5
mul r5, r7, r0
sign r7, r6
absadd r0, r7, #-1.0
mul r7, r0, r6 | st r1, [0368]
addxy r0, r6, r7 | subxy r6, r3, r1 | st r4, [0360] | ld r4, [0327]
sign r7, r6 | st r5, [036e]
min r3, r7, #0.0
subxy r6, r0, r1
mul r7, r3, r6 | ld r6, [0321]
mov r1, r6 | st r6, [0374]
subxy r3, r0, r7 | mad r1, r5, #0.019771721478734437 | ld r0, [0323] | ld r7, [0325]
mad r1, r0, #0.02471467541816923 | st r0, [036c]
mad r1, r7, #0.012357337709084615 | st r3, [0372]
mad r1, r4, #-0.4735108174809789 | ld r4, [0362]
mul r7, r1, #1.5991098880767822 | st r1, [036a]
st r7, [0332] | ld r7, [0364]
mac_p1wc3 r1, r4, r7, #-0.10940246203529515, r3
addxy r1, r1, #-0.0032820707310293085 | addxy r3, r6, #-0.5
mul r0, r1, #9.1405621171247731
sign r0, r0
min r5, r0, #0.0 | st r0, [0370]
addxy r0, r5, #1.0
mul r0, r0, r3
subxy r3, r6, r0
addxy r0, r3, #-0.5
mul r0, r0, r5
mac_p1wc3 r5, r4, r7, #-0.2188049240705903, r3
mad r5, r0, #-0.2188049240705903
addxy r3, r5, #-0.0032820707310293085
mul r0, r3, #9.1405621171247731
sign r5, r0
max r0, r3, -r3
max r3, r0, #0.00021367668366268584
max r0, r3, #0.027350615508823787
mul r3, r0, #9.1405621171247731
recipseed r6, r3
newton r6, r3
newton r6, r3
newton r6, r3
max r0, r6, #0.30521830916404724
min r4, r0, #16.0
mul r7, r4, r5
min r3, r7, #16.0
max r6, r3, #-0.97087395191192627 | ld r3, [0372]
mul r0, r1, r6 | mad r3, r2, #0.26650067457349186
addxy r4, r0, #0.16410369305294273 | mul r1, r3, #-1.0 | ld r0, [0368]
mul r5, r4, #-9.1405621171247731 | mov r4, r0
mad r4, r2, #-0.26650067457349186
ph_wrap r7, r3, r5
ph_wrap r6, r3, r1
ph_wrap r5, r3, r4
absadd r3, r7, #-0.5
ld r1, [0360]
mul r4, r3, r1
max r7, r4, #-3.8147009036072765e-05
shl r3, r7, 14 | ld r7, [0370]
min r4, r3, #0.62500059604701619
mul r3, r4, r7
sign r4, r6
absadd r7, r4, #-1.0
mul r4, r7, r6 | st r5, [0368]
addxy r7, r6, r4 | subxy r6, r0, r5
sign r4, r6
min r0, r4, #0.0
subxy r6, r7, r5
mul r4, r0, r6 | ld r6, [036e]
subxy r0, r7, r4 | ld r7, [0374] | ld r4, [036c]
mad r7, r6, #-0.083511171229065637 | st r3, [0374]
mad r7, r4, #-0.026097265897375224 | ld r4, [036a] | st r0, [0372]
mov r6, r4
mad r6, r7, #-0.29610899226597925
mad r6, r3, #0.012364204377795132
mul r7, r6, #2.557152434144939 | st r6, [036c] | ld r6, [0364]
st r7, [0330] | ld r7, [0362]
mac_p1wc3 r3, r7, r6, #-0.10940246203529515, r0
addxy r3, r3, #-0.0032820707310293085 | addxy r0, r4, #-0.31267394675505389
mul r1, r3, #9.1405621171247731
sign r1, r1
min r5, r1, #0.0 | st r1, [0370]
addxy r1, r5, #1.0
mul r1, r1, r0
subxy r0, r4, r1
addxy r1, r0, #-0.31267394675505389
mul r1, r1, r5
mac_p1wc3 r5, r7, r6, #-0.34989311764117048, r0
mad r5, r1, #-0.34989311764117048
addxy r0, r5, #-0.0032820707310293085
mul r1, r0, #9.1405621171247731
sign r5, r1
max r1, r0, -r0
max r0, r1, #0.00021367668366268584
max r1, r0, #0.027350615508823787
mul r0, r1, #9.1405621171247731
recipseed r6, r0
newton r6, r0
newton r6, r0
newton r6, r0
max r1, r6, #0.30521830916404724
min r7, r1, #16.0
mul r0, r7, r5
min r6, r0, #16.0
max r1, r6, #-0.97087395191192627
mul r7, r1, r3 | ld r1, [0372]
addxy r5, r7, #0.16410369305294273 | mad r1, r2, #0.26650067457349186
mul r0, r5, #-9.1405621171247731 | mul r3, r1, #-1.0 | ld r5, [0368]
ph_wrap r6, r3, r0
mov r0, r5
mad r0, r2, #-0.26650067457349186
ph_wrap r7, r3, r3
ph_wrap r1, r3, r0
absadd r3, r6, #-0.5
ld r2, [0360]
mul r0, r3, r2
max r6, r0, #-3.8147009036072765e-05
shl r3, r6, 14 | ld r6, [0370]
min r0, r3, #0.62500059604701619
mul r3, r0, r6
mul r0, r3, #1.5999984741210938
sign r6, r7 | st r0, [0324]
absadd r0, r6, #-1.0
mul r6, r0, r7 | st r3, [0368]
addxy r0, r7, r6 | subxy r7, r5, r1
sign r6, r7
min r5, r6, #0.0
subxy r7, r0, r1
mul r6, r5, r7 | mov r7, r4 | ld r4, [0364]
subxy r1, r0, r6 | ld r0, [036e] | ld r6, [0374]
mul r5, r1, #-1.0 | mad r7, r0, #-0.026111767506329056
mad r7, r6, #-0.052223535012658112 | st r5, [031e] | ld r5, [036c]
mov r0, r5
mad r0, r7, #-0.29610899226597931
mad r0, r3, #0.0077319291625826403
mul r7, r0, #4.0891677427607842 | st r0, [036e]
st r7, [0326] | st r7, [0338] | ld r7, [0362]
mac_p1wc3 r1, r7, r4, #-0.10940246203529515, r1
addxy r1, r1, #-0.0032820707310293085
mul r0, r1, #9.1405621171247731
sign r0, r0
min r3, r0, #0.0 | st r0, [0370]
addxy r6, r3, #1.0 | addxy r0, r5, #-0.19552999395876455
mul r6, r6, r0
subxy r0, r5, r6
addxy r6, r0, #-0.19552999395876455
mul r6, r6, r3
mac_p1wc3 r3, r7, r4, #-0.55951754419000854, r0
mad r3, r6, #-0.55951754419000854
addxy r7, r3, #-0.0032820707310293085
max r6, r7, -r7
mul r4, r7, #9.1405621171247731
max r3, r6, #0.00021367668366268584
sign r0, r4
max r4, r3, #0.027350615508823787
mul r7, r4, #9.1405621171247731
recipseed r3, r7
newton r3, r7
newton r3, r7
newton r3, r7
max r6, r3, #0.30521830916404724
min r4, r6, #16.0
mul r7, r4, r0
min r3, r7, #16.0
max r6, r3, #-0.97087395191192627
mul r4, r6, r1
addxy r0, r4, #0.16410369305294273
mul r7, r0, #-9.1405621171247731
ph_wrap r3, r3, r7
absadd r6, r3, #-0.5
mul r1, r6, r2 | ld r3, [0370]
max r4, r1, #-3.8147009036072765e-05
shl r0, r4, 14 | ld r4, [0374]
min r7, r0, #0.62500059604701619 | ld r0, [0368]
mov r1, r5 | mul r6, r7, r3 | ld r7, [036e] | ld r5, [0337]
mad r1, r4, #-0.016328938805908556 | mul r2, r6, #1.5999984741210938 | ld r4, [0329] | st r5, [032a]
mad r1, r0, #-0.032657877611817113 | st r2, [0322] | ld r0, [032b]
mov r3, r7 | ld r7, [033b] | st r4, [0368]
mad r3, r1, #-0.29610899226597925 | ld r1, [032d] | st r7, [032c]
mad r3, r6, #0.0048351456145904253 | st r1, [0370] | ld r1, [0339]
addxy r7, r7, r1 | mul r2, r3, #6.539028571453386 | st r1, [034a] | ld r1, [033d]
st r7, [0360] | st r1, [0348] | ld r7, [033f]
st r1, [0364] | ld r1, [0341] | st r7, [032e]
st r7, [0362] | st r1, [0350] | ld r7, [0343]
st r1, [0366] | ld r1, [0345] | st r7, [0334]
st r7, [0372] | ld r6, [032f] | st r1, [034e]
ld r7, [0347] | st r1, [0376] | st r6, [0380]
ld r1, [0349] | ld r4, [0333]
st r7, [0336] | st r7, [0378] | ld r7, [034b]
st r2, [0320] | st r2, [033c] | ld r6, [0353]
st r1, [0356] | st r1, [038a] | ld r2, [0331]
st r4, [0340] | st r4, [0386] | ld r1, [035d]
st r0, [0374] | st r7, [0358] | ld r4, [0359]
st r7, [037a] | st r6, [033e] | ld r0, [0335]
st r5, [0384] | st r6, [038c] | ld r7, [034d]
st r2, [0344] | st r3, [037e] | ld r5, [0357]
st r2, [0382] | st r1, [034c] | ld r6, [035f]
addxy r1, r1, r4 | ld r3, [0351] | ld r2, [0355]
st r4, [035a] | st r0, [0328]
st r7, [033a] | st r7, [037c] | ld r7, [034f]
st r0, [0388] | st r6, [0352] | ld r0, [035b]
addxy r4, r6, r5 | addxy r6, r2, r3 | st r3, [035e] | ld r3, [038a]
st r2, [0342] | st r5, [0354] | ld r2, [038c]
addxy r5, r0, r7 | addxy r0, r2, r3 | st r0, [0346] | ld r2, [0386]
st r7, [035c] | ld r7, [0388]
addxy r3, r7, r2 | ld r7, [0384] | ld r2, [0382]
addxy r7, r7, r2 | st r4, [0382] | ld r2, [0380]
ld r4, [037e]
mad r4, r2, #0.15292791415005802 | st r1, [037e] | ld r2, [0370]
ld r1, [036e]
mad r1, r2, #0.24454853968031506 | st r5, [036e] | ld r2, [0374]
ld r5, [036c]
mad r5, r2, #0.39105998791752911 | st r6, [036c] | ld r2, [037c]
ld r6, [037a]
addxy r2, r2, r6 | st r0, [037a] | ld r6, [0378]
ld r0, [0376]
addxy r6, r6, r0 | st r2, [0376] | ld r0, [0372]
ld r2, [0366]
addxy r0, r0, r2 | st r6, [0366] | ld r2, [0362]
ld r6, [0364]
addxy r2, r2, r6 | st r0, [0364] | ld r6, [0368]
ld r0, [036a]
mad r0, r6, #0.62534789351010778 | ld r6, [0360]
mad r0, r5, #-0.44547637083248542 | ld r5, [0364]
mad r1, r0, #-0.24024137872917167 | ld r0, [0366]
mad r1, r3, #0.36585156073553943 | ld r3, [0376]
mad r1, r7, #0.19608913637937525 | ld r7, [037a]
mad r4, r1, #0.40805870896358126 | ld r1, [036c]
mad r4, r6, #-0.038966080491531024 | ld r6, [036e]
mad r4, r2, #-0.16864885699789461 | ld r2, [037e]
mad r4, r5, #-0.25699960104432401 | ld r5, [0382]
mad r4, r0, #-0.25475974407309993
mad r4, r3, #-0.13212077996259911
mad r4, r7, #0.10915186066095336
mad r4, r1, #0.43256014388661762
mad r4, r6, #0.77417538816768594
mad r2, r4, #0.94446211016804105
mad r5, r2, #0.86761109293929106
mul r0, r5, #-0.20322699844837189
st r0, [031c]
