ld r0, [0383] | ld r2, [0381] | ld r3, [c049]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | ld r1, [0301]
ph_wrap r0, r3, r0
addxy r2, r2, r0 | st r0, [0382] | ld r0, [a060]
ph_wrap r2, r3, r2 | st r0, [a168] | st r0, [0300]
subxy r1, r1, r0 | st r2, [0380] | ld r2, [a038]
mul r1, r1, r1 | st r2, [a160] | ld r2, [0305]
mad r2, r2, -r1 | ld r0, [0303]
addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0304]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a120] | ld r2, [c04b]
mul r1, r1, r2 | mov r3, #0.041666666666666664 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | ld r2, [c05f] | st r0, [0302]
addxy r1, r1, r2 | ld r2, [a030]
addxy r2, r2, r1 | ld r1, [0309]
st r2, [0306] | ld r2, [a040]
addxy r2, r1, r2 | ld r0, [0306]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [0308]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a130]
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [030d]
mul r1, r0, r0
mul r2, r1, #0.03125 | ld r1, [c071]
mul r4, r2, r3 | mad r7, r2, #0.29333333333333333 | ld r3, [030f]
mad r3, r4, #0.58666666666666667
ph_wrap r0, r3, r7 | ld r7, [c072]
ph_wrap r2, r3, r3 | st r0, [030c]
mac_p1w r4, r0, r1, r7, r2 | st r2, [030e] | ld r1, [a130] | ld r2, [c056]
mul r3, r4, #0.5
st r3, [030a] | ld r3, [c049]
ld r0, [030a]
mad r0, r1, r2 | ld r1, [0380] | ld r2, [c057]
mad r0, r1, r2 | ld r1, [0311] | ld r2, [a039]
st r0, [a151]
ld r0, [a061]
subxy r1, r1, r0 | st r2, [a161] | st r0, [0310] | ld r2, [0315]
mul r1, r1, r1 | st r0, [a169] | ld r0, [0313]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0314]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a121] | ld r2, [c04b]
mul r1, r1, r2 | mov r3, #0.041666666666666664 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | ld r2, [c05f] | st r0, [0312]
addxy r1, r1, r2 | ld r2, [a031]
addxy r2, r2, r1 | ld r1, [0319]
st r2, [0316] | ld r2, [a041]
addxy r2, r1, r2 | ld r0, [0316]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [0318]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a131]
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [031d]
mul r1, r0, r0
mul r2, r1, #0.03125 | ld r1, [c071]
mul r4, r2, r3 | mad r7, r2, #0.29333333333333333 | ld r3, [031f]
mad r3, r4, #0.58666666666666667
ph_wrap r0, r3, r7 | ld r7, [c072]
ph_wrap r2, r3, r3 | st r0, [031c]
mac_p1w r4, r0, r1, r7, r2 | st r2, [031e] | ld r1, [a131] | ld r2, [c056]
mul r3, r4, #0.5
st r3, [031a] | ld r3, [c049]
ld r0, [031a]
mad r0, r1, r2 | ld r1, [0380] | ld r2, [c057]
mad r0, r1, r2 | ld r1, [0321] | ld r2, [a03a]
st r0, [a152]
ld r0, [a062]
subxy r1, r1, r0 | st r2, [a162] | st r0, [0320] | ld r2, [0325]
mul r1, r1, r1 | st r0, [a16a] | ld r0, [0323]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0324]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a122] | ld r2, [c04b]
mul r1, r1, r2 | mov r3, #0.041666666666666664 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | ld r2, [c05f] | st r0, [0322]
addxy r1, r1, r2 | ld r2, [a032]
addxy r2, r2, r1 | ld r1, [0329]
st r2, [0326] | ld r2, [a042]
addxy r2, r1, r2 | ld r0, [0326]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [0328]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a132]
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [032d]
mul r1, r0, r0
mul r2, r1, #0.03125 | ld r1, [c071]
mul r4, r2, r3 | mad r7, r2, #0.29333333333333333 | ld r3, [032f]
mad r3, r4, #0.58666666666666667
ph_wrap r0, r3, r7 | ld r7, [c072]
ph_wrap r2, r3, r3 | st r0, [032c]
mac_p1w r4, r0, r1, r7, r2 | st r2, [032e] | ld r1, [a132] | ld r2, [c056]
mul r3, r4, #0.5
st r3, [032a] | ld r3, [c049]
ld r0, [032a]
mad r0, r1, r2 | ld r1, [0380] | ld r2, [c057]
mad r0, r1, r2 | ld r1, [0331] | ld r2, [a03b]
st r0, [a153]
ld r0, [a063]
subxy r1, r1, r0 | st r2, [a163] | st r0, [0330] | ld r2, [0335]
mul r1, r1, r1 | st r0, [a16b] | ld r0, [0333]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0334]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a123] | ld r2, [c04b]
mul r1, r1, r2 | mov r3, #0.041666666666666664 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | ld r2, [c05f] | st r0, [0332]
addxy r1, r1, r2 | ld r2, [a033]
addxy r2, r2, r1 | ld r1, [0339]
st r2, [0336] | ld r2, [a043]
addxy r2, r1, r2 | ld r0, [0336]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [0338]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a133]
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [033d]
mul r1, r0, r0
mul r2, r1, #0.03125 | ld r1, [c071]
mul r4, r2, r3 | mad r7, r2, #0.29333333333333333 | ld r3, [033f]
mad r3, r4, #0.58666666666666667
ph_wrap r0, r3, r7 | ld r7, [c072]
ph_wrap r2, r3, r3 | st r0, [033c]
mac_p1w r4, r0, r1, r7, r2 | st r2, [033e] | ld r1, [a133] | ld r2, [c056]
mul r3, r4, #0.5
st r3, [033a] | ld r3, [c049]
ld r0, [033a]
mad r0, r1, r2 | ld r1, [0380] | ld r2, [c057]
mad r0, r1, r2 | ld r1, [0341] | ld r2, [a03c]
st r0, [a154]
ld r0, [a064]
subxy r1, r1, r0 | st r2, [a164] | st r0, [0340] | ld r2, [0345]
mul r1, r1, r1 | st r0, [a16c] | ld r0, [0343]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0344]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a124] | ld r2, [c04b]
mul r1, r1, r2 | mov r3, #0.041666666666666664 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | ld r2, [c05f] | st r0, [0342]
addxy r1, r1, r2 | ld r2, [a034]
addxy r2, r2, r1 | ld r1, [0349]
st r2, [0346] | ld r2, [a044]
addxy r2, r1, r2 | ld r0, [0346]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [0348]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a134]
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [034d]
mul r1, r0, r0
mul r2, r1, #0.03125 | ld r1, [c071]
mul r4, r2, r3 | mad r7, r2, #0.29333333333333333 | ld r3, [034f]
mad r3, r4, #0.58666666666666667
ph_wrap r0, r3, r7 | ld r7, [c072]
ph_wrap r2, r3, r3 | st r0, [034c]
mac_p1w r4, r0, r1, r7, r2 | st r2, [034e] | ld r1, [a134] | ld r2, [c056]
mul r3, r4, #0.5
st r3, [034a] | ld r3, [c049]
ld r0, [034a]
mad r0, r1, r2 | ld r1, [0380] | ld r2, [c057]
mad r0, r1, r2 | ld r1, [0351] | ld r2, [a03d]
st r0, [a155]
ld r0, [a065]
subxy r1, r1, r0 | st r2, [a165] | st r0, [0350] | ld r2, [0355]
mul r1, r1, r1 | st r0, [a16d] | ld r0, [0353]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0354]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a125] | ld r2, [c04b]
mul r1, r1, r2 | mov r3, #0.041666666666666664 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | ld r2, [c05f] | st r0, [0352]
addxy r1, r1, r2 | ld r2, [a035]
addxy r2, r2, r1 | ld r1, [0359]
st r2, [0356] | ld r2, [a045]
addxy r2, r1, r2 | ld r0, [0356]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [0358]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a135]
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [035d]
mul r1, r0, r0
mul r2, r1, #0.03125 | ld r1, [c071]
mul r4, r2, r3 | mad r7, r2, #0.29333333333333333 | ld r3, [035f]
mad r3, r4, #0.58666666666666667
ph_wrap r0, r3, r7 | ld r7, [c072]
ph_wrap r2, r3, r3 | st r0, [035c]
mac_p1w r4, r0, r1, r7, r2 | st r2, [035e] | ld r1, [a135] | ld r2, [c056]
mul r3, r4, #0.5
st r3, [035a] | ld r3, [c049]
ld r0, [035a]
mad r0, r1, r2 | ld r1, [0380] | ld r2, [c057]
mad r0, r1, r2 | ld r1, [0361] | ld r2, [a03e]
st r0, [a156]
ld r0, [a066]
subxy r1, r1, r0 | st r2, [a166] | st r0, [0360] | ld r2, [0365]
mul r1, r1, r1 | st r0, [a16e] | ld r0, [0363]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0364]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a126] | ld r2, [c04b]
mul r1, r1, r2 | mov r3, #0.041666666666666664 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | ld r2, [c05f] | st r0, [0362]
addxy r1, r1, r2 | ld r2, [a036]
addxy r2, r2, r1 | ld r1, [0369]
st r2, [0366] | ld r2, [a046]
addxy r2, r1, r2 | ld r0, [0366]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [0368]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a136]
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [036d]
mul r1, r0, r0
mul r2, r1, #0.03125 | ld r1, [c071]
mul r4, r2, r3 | mad r7, r2, #0.29333333333333333 | ld r3, [036f]
mad r3, r4, #0.58666666666666667
ph_wrap r0, r3, r7 | ld r7, [c072]
ph_wrap r2, r3, r3 | st r0, [036c]
mac_p1w r4, r0, r1, r7, r2 | st r2, [036e] | ld r1, [a136] | ld r2, [c056]
mul r3, r4, #0.5
st r3, [036a] | ld r3, [c049]
ld r0, [036a]
mad r0, r1, r2 | ld r1, [0380] | ld r2, [c057]
mad r0, r1, r2 | ld r1, [0371] | ld r2, [a03f]
st r0, [a157]
ld r0, [a067]
subxy r1, r1, r0 | st r2, [a167] | st r0, [0370] | ld r2, [0375]
mul r1, r1, r1 | st r0, [a16f] | ld r0, [0373]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0374]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a127] | ld r2, [c04b]
mul r1, r1, r2 | mov r3, #0.041666666666666664 | ld r2, [c05e]
min r0, r0, #1.0
mad r1, r0, r2 | ld r2, [c05f] | st r0, [0372]
addxy r1, r1, r2 | ld r2, [a037]
addxy r2, r2, r1 | ld r1, [0379]
st r2, [0376] | ld r2, [a047]
addxy r2, r1, r2 | ld r0, [0376]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [0378]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a137]
mul r2, r1, #0.04332169878499658
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3 | ld r3, [c070]
mul r7, r4, r4
mul r0, r7, r7 | ld r7, [037d]
mul r1, r0, r0
mul r2, r1, #0.03125 | ld r1, [c071]
mul r4, r2, r3 | mad r7, r2, #0.29333333333333333 | ld r3, [037f]
mad r3, r4, #0.58666666666666667
ph_wrap r0, r3, r7 | ld r7, [c072]
ph_wrap r2, r3, r3 | st r0, [037c]
mac_p1w r4, r0, r1, r7, r2 | st r2, [037e] | ld r1, [a137] | ld r2, [c056]
mul r3, r4, #0.5
st r3, [037a]
ld r0, [037a]
mad r0, r1, r2 | ld r1, [0380] | ld r2, [c057]
mad r0, r1, r2
st r0, [a158]
