ld r0, [a060] | ld r1, [0301] | ld r5, [030b]
subxy r1, r1, r0 | ld r2, [a038] | st r0, [0300] | ld r3, [c049]
mul r1, r1, r1 | st r2, [a160] | ld r2, [0305] | ld r7, [c049]
mad r2, r2, -r1 | st r0, [a168] | ld r0, [0303]
addxy r1, r0, #-0.999 | ld r4, [a061]
mul r1, r1, #256.0 | subxy r5, r5, r4 | ld r6, [a039] | st r4, [030a]
ph_clamp r1, r3, -r1 | st r6, [a161] | ld r6, [030f]
mul r1, r1, #256.0 | mul r5, r5, r5 | st r4, [a169] | ld r4, [030d]
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0304]
mul r2, r2, #256.0 | mad r6, r6, -r5
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2 | addxy r5, r4, #-0.999
mad r0, r2, r1 | ld r1, [a120] | ld r2, [c04b]
mul r1, r1, r2 | mul r5, r5, #256.0 | ld r2, [c05e]
min r0, r0, #1.0 | ph_clamp r5, r3, -r5
mad r1, r0, r2 | mul r5, r5, #256.0 | ld r2, [c05f] | st r0, [0302]
addxy r1, r1, r2 | ld r2, [a030]
addxy r2, r2, r1 | ld r1, [0307]
ph_clamp r5, r3, -r5 | st r2, [a1b0]
max r6, r6, r5 | ld r2, [a040] | ld r5, [c048]
addxy r2, r1, r2 | subxy r6, r6, #1.0 | st r6, [030e]
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | mul r6, r6, #256.0 | st r1, [0306]
ph_clamp r1, r3, -r1
ph_clamp r6, r3, -r6 | st r1, [a130]
subxy r7, r5, r7 | ld r1, [a120]
mad r2, r1, r3 | mad r5, r6, r7 | ld r1, [0302] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a178] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a170] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a130] | ld r3, [c055]
mad r2, r1, r3 | mul r6, r6, #1.25 | ld r1, [a038]
subxy r6, r4, r6 | st r2, [0308] | ld r7, [c051]
mad r4, r6, r5 | ld r0, [0308] | ld r5, [a121]
mul r2, r0, r1 | ld r0, [a062] | ld r1, [0315]
mul r3, r2, #0.0010000000474974513 | subxy r1, r1, r0 | ld r2, [a03a] | st r0, [0314]
mul r1, r1, r1 | st r2, [a162] | ld r2, [0319] | st r3, [0260]
mad r2, r2, -r1 | st r0, [a16a] | ld r0, [0317] | ld r3, [c049]
addxy r1, r0, #-0.999 | ld r6, [c04b]
mul r1, r1, #256.0 | mul r5, r5, r6 | ld r6, [c05e]
ph_clamp r1, r3, -r1 | min r4, r4, #1.0
mul r1, r1, #256.0 | mad r5, r4, r6 | ld r6, [c05f] | st r4, [030c]
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0318]
mul r2, r2, #256.0 | addxy r5, r5, r6 | ld r6, [a031]
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051]
subxy r2, r0, r2 | addxy r6, r6, r5 | ld r5, [0311]
mad r0, r2, r1 | ld r1, [a122] | ld r2, [c04b]
mul r1, r1, r2 | ld r2, [c05e] | st r6, [a1b1]
min r0, r0, #1.0 | ld r6, [a041]
mad r1, r0, r2 | addxy r6, r5, r6 | ld r2, [c05f] | st r0, [0316]
addxy r1, r1, r2 | ld r2, [a032]
addxy r2, r2, r1 | ld r1, [031b]
ph_wrap r5, r3, r6 | st r2, [a1b2] | ld r6, [c050]
mul r5, r5, #256.0 | ld r2, [a042] | st r5, [0310]
addxy r2, r1, r2
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | st r1, [031a]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a132]
ld r1, [a122]
mad r2, r1, r3 | ld r1, [0316] | ld r3, [c052] | st r5, [a131]
mad r2, r1, r3 | ld r1, [a17a] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a172] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a132] | ld r3, [c055]
mad r2, r1, r3 | ld r1, [a03a]
st r2, [031c] | ld r5, [a121]
mad r6, r5, r7 | ld r0, [031c] | ld r5, [030c] | ld r7, [c052]
mul r2, r0, r1 | mad r6, r5, r7 | ld r0, [a064] | ld r1, [0329]
mul r3, r2, #0.0010000000474974513 | subxy r1, r1, r0 | ld r2, [a03c] | st r0, [0328]
mul r1, r1, r1 | st r2, [a164] | ld r2, [032d] | st r3, [0264]
mad r2, r2, -r1 | st r0, [a16c] | ld r0, [032b] | ld r3, [c049]
addxy r1, r0, #-0.999 | ld r5, [a179] | ld r7, [c053]
mul r1, r1, #256.0 | mad r6, r5, r7 | ld r5, [a171]
ph_clamp r1, r3, -r1 | ld r7, [c054]
mul r1, r1, #256.0 | mad r6, r5, r7 | ld r5, [a131]
ph_clamp r1, r3, -r1 | ld r7, [c055]
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [032c]
mul r2, r2, #256.0 | mad r6, r5, r7 | ld r5, [a039]
ph_clamp r2, r3, -r2 | st r6, [0312]
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051] | ld r4, [0312]
subxy r2, r0, r2 | mul r6, r4, r5 | ld r4, [a063] | ld r5, [031f]
mad r0, r2, r1 | ld r1, [a124] | ld r2, [c04b] | st r4, [031e]
mul r1, r1, r2 | mul r7, r6, #0.0010000000474974513 | ld r2, [c05e]
min r0, r0, #1.0 | ld r6, [a03b] | st r7, [0262] | ld r7, [c049]
mad r1, r0, r2 | subxy r5, r5, r4 | ld r2, [c05f] | st r0, [032a]
addxy r1, r1, r2 | mul r5, r5, r5 | ld r2, [a034]
addxy r2, r2, r1 | ld r1, [032f] | st r6, [a163]
st r2, [a1b4] | ld r6, [0323]
mad r6, r6, -r5 | ld r2, [a044]
addxy r2, r1, r2 | st r4, [a16b] | ld r4, [0321]
ph_wrap r1, r3, r2 | ld r2, [c050]
mul r1, r1, #256.0 | addxy r5, r4, #-0.999 | st r1, [032e]
ph_clamp r1, r3, -r1
mul r5, r5, #256.0 | st r1, [a134]
ph_clamp r5, r3, -r5 | ld r1, [a124]
mad r2, r1, r3 | mul r5, r5, #256.0 | ld r1, [032a] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a17c] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a174] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a134] | ld r3, [c055]
mad r2, r1, r3 | ph_clamp r5, r3, -r5 | ld r1, [a03c]
max r6, r6, r5 | st r2, [0330] | ld r5, [c048]
subxy r6, r6, #1.0 | subxy r7, r5, r7 | ld r0, [0330] | st r6, [0322]
mul r2, r0, r1 | mul r6, r6, #256.0 | ld r0, [a066] | ld r1, [033d]
mul r3, r2, #0.0010000000474974513 | subxy r1, r1, r0 | ld r2, [a03e] | st r0, [033c]
mul r1, r1, r1 | ph_clamp r6, r3, -r6 | st r2, [a166] | ld r2, [0341] | st r3, [0268]
mad r2, r2, -r1 | st r0, [a16e] | ld r0, [033f] | ld r3, [c049]
addxy r1, r0, #-0.999 | mad r5, r6, r7 | ld r7, [c051]
mul r1, r1, #256.0 | mul r6, r6, #1.25
ph_clamp r1, r3, -r1
mul r1, r1, #256.0 | subxy r6, r4, r6
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c048]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0340]
mul r2, r2, #256.0 | mad r4, r6, r5 | ld r5, [a123]
ph_clamp r2, r3, -r2 | min r4, r4, #1.0 | ld r6, [c04b]
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c051] | st r4, [0320]
subxy r2, r0, r2 | mul r5, r5, r6 | ld r6, [c05e]
mad r0, r2, r1 | ld r1, [a126] | ld r2, [c04b]
mul r1, r1, r2 | mad r5, r4, r6 | ld r2, [c05e]
min r0, r0, #1.0 | ld r6, [c05f]
mad r1, r0, r2 | addxy r5, r5, r6 | ld r2, [c05f] | st r0, [033e]
addxy r1, r1, r2 | ld r2, [a036]
addxy r2, r2, r1 | ld r1, [0343] | ld r6, [a033]
addxy r6, r6, r5 | st r2, [a1b6] | ld r5, [0325]
ld r2, [a046]
addxy r2, r1, r2 | st r6, [a1b3]
ph_wrap r1, r3, r2 | ld r2, [c050] | ld r6, [a043]
mul r1, r1, #256.0 | addxy r6, r5, r6 | st r1, [0342]
ph_clamp r1, r3, -r1
ph_wrap r5, r3, r6 | st r1, [a136] | ld r6, [c050]
mul r5, r5, #256.0 | ld r1, [a126] | st r5, [0324]
mad r2, r1, r3 | ph_clamp r5, r3, -r5 | ld r1, [033e] | ld r3, [c052]
mad r2, r1, r3 | ld r1, [a17e] | ld r3, [c053]
mad r2, r1, r3 | ld r1, [a176] | ld r3, [c054]
mad r2, r1, r3 | ld r1, [a136] | ld r3, [c055]
mad r2, r1, r3 | ld r1, [a03e]
st r2, [0344] | st r5, [a133]
ld r0, [0344] | ld r5, [a123]
mul r2, r0, r1 | mad r6, r5, r7 | ld r1, [0308] | ld r5, [0320] | ld r7, [c052]
max r1, r1, #0.0 | ld r0, [600e]
min r1, r1, #1.0 | st r0, [a151]
mul r1, r1, #240.0 | mad r6, r5, r7 | ld r5, [a17b]
addxy r1, r1, #0.5 | mul r3, r2, #0.0010000000474974513 | ld r7, [c053]
floor r1, r1 | st r3, [026c]
mul r0, r1, #0.125 | mad r6, r5, r7 | ld r5, [a173]
floor r0, r0 | ld r7, [c054]
mul r0, r0, #-8.0 | mad r6, r5, r7 | ld r5, [a133]
addxy r1, r1, r0 | mul r0, r0, #-0.015625 | ld r7, [c055]
addxy r0, r0, #-3.90625 | mul r1, r1, #-8.0
addxy r0, r0, r1 | mul r1, r1, #-0.125
addxy r1, r1, #-1.0 | mad r6, r5, r7 | ld r5, [a03b]
max r1, r1, #0.0 | st r6, [0326]
min r1, r1, #1.0 | ld r4, [0326]
mul r1, r1, #-8.0 | mul r6, r4, r5 | ld r4, [a065] | ld r5, [0333]
addxy r1, r0, r1 | mul r7, r6, #0.0010000000474974513 | ld r0, [600f] | st r4, [a16d]
subxy r5, r5, r4 | st r1, [6008] | ld r1, [0312]
max r1, r1, #0.0 | st r0, [a152] | st r4, [0332] | ld r4, [0335]
min r1, r1, #1.0 | ld r6, [a03d] | st r7, [0266] | ld r7, [c049]
mul r1, r1, #240.0 | mul r5, r5, r5 | st r6, [a165] | ld r6, [0337]
addxy r1, r1, #0.5 | mad r6, r6, -r5
floor r1, r1
mul r0, r1, #0.125 | addxy r5, r4, #-0.999
floor r0, r0
mul r0, r0, #-8.0 | mul r5, r5, #256.0
addxy r1, r1, r0 | mul r0, r0, #-0.015625
addxy r0, r0, #-3.90625 | mul r1, r1, #-8.0
addxy r0, r0, r1 | mul r1, r1, #-0.125
addxy r1, r1, #-1.0
max r1, r1, #0.0
min r1, r1, #1.0 | ph_clamp r5, r3, -r5
mul r1, r1, #-8.0 | mul r5, r5, #256.0
addxy r1, r0, r1 | ld r0, [6010]
ph_clamp r5, r3, -r5 | st r1, [6009] | ld r1, [031c]
max r1, r1, #0.0 | st r0, [a153]
min r1, r1, #1.0
max r6, r6, r5 | ld r5, [c048]
mul r1, r1, #240.0 | subxy r6, r6, #1.0 | st r6, [0336]
addxy r1, r1, #0.5 | mul r6, r6, #256.0
floor r1, r1
mul r0, r1, #0.125 | subxy r7, r5, r7
ph_clamp r6, r3, -r6
floor r0, r0
mad r5, r6, r7 | mul r6, r6, #1.25 | ld r7, [c051]
mul r0, r0, #-8.0 | subxy r6, r4, r6
addxy r1, r1, r0 | mul r0, r0, #-0.015625
mad r4, r6, r5 | ld r5, [a125] | ld r6, [c04b]
addxy r0, r0, #-3.90625 | mul r1, r1, #-8.0
mul r5, r5, r6 | addxy r0, r0, r1 | ld r6, [c05e]
min r4, r4, #1.0
mul r1, r1, #-0.125 | mad r5, r4, r6 | ld r6, [c05f] | st r4, [0334]
addxy r1, r1, #-1.0
max r1, r1, #0.0
addxy r5, r5, r6 | ld r6, [a035]
min r1, r1, #1.0
addxy r6, r6, r5 | mul r1, r1, #-8.0 | ld r5, [0339]
addxy r1, r0, r1 | st r6, [a1b5] | ld r0, [6011]
ld r6, [a045] | st r1, [600a]
addxy r6, r5, r6 | ld r1, [0326] | st r0, [a154]
ph_wrap r5, r3, r6 | ld r6, [c050]
max r1, r1, #0.0 | st r5, [0338]
min r1, r1, #1.0
mul r5, r5, #256.0 | mul r1, r1, #240.0
ph_clamp r5, r3, -r5
addxy r1, r1, #0.5 | st r5, [a135]
floor r1, r1 | ld r5, [a125]
mad r6, r5, r7 | mul r0, r1, #0.125 | ld r5, [0334] | ld r7, [c052]
floor r0, r0
mad r6, r5, r7 | ld r5, [a17d] | ld r7, [c053]
mul r0, r0, #-8.0 | mad r6, r5, r7 | ld r5, [a175]
addxy r1, r1, r0 | mul r0, r0, #-0.015625 | ld r7, [c054]
mad r6, r5, r7 | addxy r0, r0, #-3.90625 | ld r5, [a135]
mul r1, r1, #-8.0 | ld r7, [c055]
addxy r0, r0, r1 | mad r6, r5, r7 | ld r5, [a03d]
mul r1, r1, #-0.125 | st r6, [033a]
addxy r1, r1, #-1.0 | ld r4, [033a]
max r1, r1, #0.0
mul r6, r4, r5 | ld r4, [a067] | ld r5, [0347]
min r1, r1, #1.0 | st r4, [a16f] | st r4, [0346]
mul r7, r6, #0.0010000000474974513 | mul r1, r1, #-8.0 | ld r6, [a03f]
subxy r5, r5, r4 | addxy r1, r0, r1 | st r6, [a167] | ld r6, [034b] | st r7, [026a]
mul r5, r5, r5 | st r1, [600b] | ld r4, [0349]
mad r6, r6, -r5 | addxy r5, r4, #-0.999 | ld r1, [0330] | ld r7, [c049]
max r1, r1, #0.0 | ld r0, [6012]
mul r5, r5, #256.0 | st r0, [a155]
min r1, r1, #1.0 | ph_clamp r5, r3, -r5
mul r1, r1, #240.0 | mul r5, r5, #256.0
addxy r1, r1, #0.5
ph_clamp r5, r3, -r5
floor r1, r1
max r6, r6, r5 | ld r5, [c048]
mul r0, r1, #0.125 | subxy r6, r6, #1.0 | st r6, [034a]
floor r0, r0
mul r0, r0, #-8.0 | mul r6, r6, #256.0
addxy r1, r1, r0 | subxy r7, r5, r7
ph_clamp r6, r3, -r6
mul r0, r0, #-0.015625 | mad r5, r6, r7 | ld r7, [c051]
addxy r0, r0, #-3.90625 | mul r1, r1, #-8.0
mul r6, r6, #1.25 | addxy r0, r0, r1
subxy r6, r4, r6 | mul r1, r1, #-0.125
mad r4, r6, r5 | addxy r1, r1, #-1.0 | ld r5, [a127]
max r1, r1, #0.0 | ld r6, [c04b]
mul r5, r5, r6 | ld r6, [c05e]
min r4, r4, #1.0
min r1, r1, #1.0 | st r4, [0348]
mad r5, r4, r6 | mul r1, r1, #-8.0 | ld r6, [c05f]
addxy r1, r0, r1 | addxy r5, r5, r6 | ld r6, [a037]
addxy r6, r6, r5 | st r1, [600c] | ld r1, [033a]
max r1, r1, #0.0 | st r6, [a1b7] | ld r5, [034d]
min r1, r1, #1.0 | ld r6, [a047]
addxy r6, r5, r6 | mul r1, r1, #240.0 | ld r0, [6013]
ph_wrap r5, r3, r6 | st r0, [a156] | ld r6, [c050]
addxy r1, r1, #0.5 | mul r5, r5, #256.0 | st r5, [034c]
floor r1, r1
mul r0, r1, #0.125
ph_clamp r5, r3, -r5
floor r0, r0 | st r5, [a137]
mul r0, r0, #-8.0 | ld r5, [a127]
mad r6, r5, r7 | addxy r1, r1, r0 | ld r5, [0348] | ld r7, [c052]
mul r0, r0, #-0.015625 | mad r6, r5, r7 | ld r5, [a17f]
addxy r0, r0, #-3.90625 | mul r1, r1, #-8.0 | ld r7, [c053]
addxy r0, r0, r1 | mad r6, r5, r7 | ld r5, [a177]
mul r1, r1, #-0.125 | ld r7, [c054]
addxy r1, r1, #-1.0 | mad r6, r5, r7 | ld r5, [a137]
max r1, r1, #0.0 | ld r7, [c055]
mad r6, r5, r7 | ld r5, [a03f]
min r1, r1, #1.0 | st r6, [034e]
mul r1, r1, #-8.0 | ld r4, [034e]
addxy r1, r0, r1 | mul r6, r4, r5 | ld r0, [6014]
mul r7, r6, #0.0010000000474974513 | st r0, [a157] | ld r0, [6015]
st r7, [026e] | st r0, [a158]
st r1, [600d]
