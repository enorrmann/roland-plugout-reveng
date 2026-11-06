ld r4, [a061] | ld r5, [0307] | ld r1, [0301]
subxy r5, r5, r4 | ld r6, [a039] | st r4, [0306] | ld r7, [c049]
mul r5, r5, r5 | st r6, [a161] | ld r6, [030b] | ld r3, [c049]
mad r6, r6, -r5 | st r4, [a169] | ld r4, [0309]
addxy r5, r4, #-0.999 | ld r0, [a060]
mul r5, r5, #256.0 | subxy r1, r1, r0 | ld r2, [a038] | st r0, [0300]
ph_clamp r5, r3, -r5 | st r2, [a160] | ld r2, [0305]
mul r5, r5, #256.0 | mul r1, r1, r1 | st r0, [a168] | ld r0, [0303]
ph_clamp r5, r3, -r5
max r6, r6, r5 | ld r5, [c048]
subxy r6, r6, #1.0 | subxy r7, r5, r7 | st r6, [030a]
mul r6, r6, #256.0 | mad r2, r2, -r1
ph_clamp r6, r3, -r6
mad r5, r6, r7 | mul r6, r6, #1.25 | ld r7, [c049]
subxy r6, r4, r6 | addxy r1, r0, #-0.999
mad r4, r6, r5 | ld r5, [a121] | ld r6, [c04b]
mul r5, r5, r6 | mul r1, r1, #256.0 | ld r6, [c05e]
min r4, r4, #1.0 | ph_clamp r1, r3, -r1
mad r5, r4, r6 | mul r1, r1, #256.0 | ld r6, [c05f] | st r4, [0308]
addxy r5, r5, r6 | ld r6, [a031]
ph_clamp r1, r3, -r1 | ld r4, [a039]
max r2, r2, r1 | ld r4, [a063] | ld r1, [c048]
addxy r6, r6, r5 | mov r5, #0.0 | st r2, [0304] | st r4, [a16b]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r5, [0262] | st r6, [a1b1] | ld r5, [0313]
mul r2, r2, #256.0 | subxy r5, r5, r4 | ld r6, [a03b] | st r4, [0312]
ph_clamp r2, r3, -r2 | st r6, [a163] | ld r6, [0317]
mul r5, r5, r5 | mad r1, r2, r3 | ld r4, [0315] | ld r3, [c049]
mul r2, r2, #1.25 | mad r6, r6, -r5
addxy r5, r4, #-0.999 | subxy r2, r0, r2
mul r5, r5, #256.0 | mad r0, r2, r1 | ld r1, [a120]
ph_clamp r5, r3, -r5 | min r0, r0, #1.0 | ld r2, [c04b]
mul r1, r1, r2 | mul r5, r5, #256.0 | ld r2, [c05e] | st r0, [0302]
ph_clamp r5, r3, -r5
mad r1, r0, r2 | ld r2, [c05f] | ld r0, [a038]
max r6, r6, r5 | ld r5, [c048] | ld r0, [a062]
addxy r1, r1, r2 | subxy r6, r6, #1.0 | st r6, [0316] | ld r2, [a030]
addxy r2, r2, r1 | mul r6, r6, #256.0 | st r0, [a16a] | st r0, [030c]
mov r1, #0.0 | st r2, [a1b0]
ph_clamp r6, r3, -r6 | st r1, [0260] | ld r1, [030d]
subxy r7, r5, r7 | subxy r1, r1, r0 | ld r2, [a03a] | ld r0, [030f]
mad r5, r6, r7 | mul r6, r6, #1.25 | st r2, [a162] | ld r2, [0311]
subxy r6, r4, r6 | mul r1, r1, r1 | ld r7, [c049]
mad r4, r6, r5 | mad r2, r2, -r1 | ld r5, [a123]
addxy r1, r0, #-0.999 | ld r6, [c04b]
mul r5, r5, r6 | mul r1, r1, #256.0 | ld r6, [c05e]
min r4, r4, #1.0 | ph_clamp r1, r3, -r1
mad r5, r4, r6 | mul r1, r1, #256.0 | ld r6, [c05f] | st r4, [0314]
addxy r5, r5, r6 | ld r6, [a033]
ph_clamp r1, r3, -r1 | ld r4, [a03b]
max r2, r2, r1 | ld r4, [a065] | ld r1, [c048]
addxy r6, r6, r5 | mov r5, #0.0 | st r2, [0310] | st r4, [a16d]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r5, [0266] | st r6, [a1b3] | ld r5, [031f]
mul r2, r2, #256.0 | subxy r5, r5, r4 | ld r6, [a03d] | st r4, [031e]
ph_clamp r2, r3, -r2 | st r6, [a165] | ld r6, [0323]
mul r5, r5, r5 | mad r1, r2, r3 | ld r4, [0321] | ld r3, [c049]
mul r2, r2, #1.25 | mad r6, r6, -r5
addxy r5, r4, #-0.999 | subxy r2, r0, r2
mul r5, r5, #256.0 | mad r0, r2, r1 | ld r1, [a122]
ph_clamp r5, r3, -r5 | min r0, r0, #1.0 | ld r2, [c04b]
mul r1, r1, r2 | mul r5, r5, #256.0 | ld r2, [c05e] | st r0, [030e]
ph_clamp r5, r3, -r5
mad r1, r0, r2 | ld r2, [c05f] | ld r0, [a03a]
max r6, r6, r5 | ld r5, [c048] | ld r0, [a064]
addxy r1, r1, r2 | subxy r6, r6, #1.0 | st r6, [0322] | ld r2, [a032]
addxy r2, r2, r1 | mul r6, r6, #256.0 | st r0, [a16c] | st r0, [0318]
mov r1, #0.0 | st r2, [a1b2]
ph_clamp r6, r3, -r6 | st r1, [0264] | ld r1, [0319]
subxy r7, r5, r7 | subxy r1, r1, r0 | ld r2, [a03c] | ld r0, [031b]
mad r5, r6, r7 | mul r6, r6, #1.25 | st r2, [a164] | ld r2, [031d]
subxy r6, r4, r6 | mul r1, r1, r1 | ld r7, [c049]
mad r4, r6, r5 | mad r2, r2, -r1 | ld r5, [a125]
addxy r1, r0, #-0.999 | ld r6, [c04b]
mul r5, r5, r6 | mul r1, r1, #256.0 | ld r6, [c05e]
min r4, r4, #1.0 | ph_clamp r1, r3, -r1
mad r5, r4, r6 | mul r1, r1, #256.0 | ld r6, [c05f] | st r4, [0320]
addxy r5, r5, r6 | ld r6, [a035]
ph_clamp r1, r3, -r1 | ld r4, [a03d]
max r2, r2, r1 | ld r4, [a067] | ld r1, [c048]
addxy r6, r6, r5 | mov r5, #0.0 | st r2, [031c] | st r4, [a16f]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r5, [026a] | st r6, [a1b5] | ld r5, [032b]
mul r2, r2, #256.0 | subxy r5, r5, r4 | ld r6, [a03f] | st r4, [032a]
ph_clamp r2, r3, -r2 | st r6, [a167] | ld r6, [032f]
mul r5, r5, r5 | mad r1, r2, r3 | ld r4, [032d] | ld r3, [c049]
mul r2, r2, #1.25 | mad r6, r6, -r5
addxy r5, r4, #-0.999 | subxy r2, r0, r2
mul r5, r5, #256.0 | mad r0, r2, r1 | ld r1, [a124]
ph_clamp r5, r3, -r5 | min r0, r0, #1.0 | ld r2, [c04b]
mul r1, r1, r2 | mul r5, r5, #256.0 | ld r2, [c05e] | st r0, [031a]
ph_clamp r5, r3, -r5
mad r1, r0, r2 | ld r2, [c05f] | ld r0, [a03c]
max r6, r6, r5 | ld r5, [c048] | ld r0, [a066]
addxy r1, r1, r2 | subxy r6, r6, #1.0 | st r6, [032e] | ld r2, [a034]
addxy r2, r2, r1 | mul r6, r6, #256.0 | st r0, [a16e] | st r0, [0324]
mov r1, #0.0 | st r2, [a1b4]
ph_clamp r6, r3, -r6 | st r1, [0268] | ld r1, [0325]
subxy r7, r5, r7 | subxy r1, r1, r0 | ld r2, [a03e] | ld r0, [0327]
mad r5, r6, r7 | mul r6, r6, #1.25 | st r2, [a166] | ld r2, [0329]
subxy r6, r4, r6 | mul r1, r1, r1
mad r4, r6, r5 | mad r2, r2, -r1 | ld r5, [a127]
addxy r1, r0, #-0.999 | ld r6, [c04b]
mul r5, r5, r6 | mul r1, r1, #256.0 | ld r6, [c05e]
min r4, r4, #1.0 | ph_clamp r1, r3, -r1
mad r5, r4, r6 | mul r1, r1, #256.0 | ld r6, [c05f] | st r4, [032c]
addxy r5, r5, r6 | ld r6, [a037]
ph_clamp r1, r3, -r1 | ld r4, [a03f]
max r2, r2, r1 | ld r4, [a040] | ld r1, [c048]
addxy r6, r6, r5 | mov r5, #0.0 | st r2, [0328]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r5, [026e] | ld r5, [0781] | st r6, [a1b7]
mul r2, r2, #256.0 | addxy r5, r5, r4
mov r6, #1.0
ph_clamp r2, r3, -r2
subxy r6, r6, r5 | mad r1, r2, r3
mul r2, r2, #1.25
sign r6, r6
subxy r2, r0, r2 | addxy r6, r6, #1.0
mad r0, r2, r1 | mul r6, r6, #0.5 | ld r1, [a126]
subxy r5, r6, r5 | ld r2, [c04b]
mul r1, r1, r2 | mul r4, r5, #127.0 | ld r2, [c05e] | st r5, [0780] | ld r5, [0783]
min r0, r0, #1.0
mad r1, r0, r2 | ld r2, [c05f] | st r0, [0326] | ld r0, [a03e]
floor r6, r4
subxy r4, r6, r4 | addxy r1, r1, r2 | ld r2, [a036]
mul r6, r6, #6.103515625e-05 | addxy r2, r2, r1
addxy r6, r6, #0.007843017578125 | st r2, [a1b6]
mov r1, #0.0
hset h6, r6
st r1, [026c]
ldb5 r1, 0
ldb5 r2, 1
subxy r2, r1, r2
mul r2, r2, r4 | mov r6, #1.0 | ld r4, [a041]
addxy r5, r5, r4 | addxy r1, r1, r2
subxy r6, r6, r5 | st r1, [a151]
sign r6, r6
addxy r6, r6, #1.0
mul r6, r6, #0.5
subxy r5, r6, r5
mul r4, r5, #127.0 | st r5, [0782] | ld r5, [0785]
floor r6, r4
subxy r4, r6, r4 | mul r6, r6, #6.103515625e-05
addxy r6, r6, #0.007843017578125
hset h6, r6
mov r0, r0
ldb5 r1, 0
ldb5 r2, 1
subxy r2, r1, r2
mul r2, r2, r4 | mov r6, #1.0 | ld r4, [a042]
addxy r5, r5, r4 | addxy r1, r1, r2
subxy r6, r6, r5 | st r1, [a152]
sign r6, r6
addxy r6, r6, #1.0
mul r6, r6, #0.5
subxy r5, r6, r5
mul r4, r5, #127.0 | st r5, [0784] | ld r5, [0787]
floor r6, r4
subxy r4, r6, r4 | mul r6, r6, #6.103515625e-05
addxy r6, r6, #0.007843017578125
hset h6, r6
mov r0, r0
ldb5 r1, 0
ldb5 r2, 1
subxy r2, r1, r2
mul r2, r2, r4 | mov r6, #1.0 | ld r4, [a043]
addxy r5, r5, r4 | addxy r1, r1, r2
subxy r6, r6, r5 | st r1, [a153]
sign r6, r6
addxy r6, r6, #1.0
mul r6, r6, #0.5
subxy r5, r6, r5
mul r4, r5, #127.0 | st r5, [0786] | ld r5, [0789]
floor r6, r4
subxy r4, r6, r4 | mul r6, r6, #6.103515625e-05
addxy r6, r6, #0.007843017578125
hset h6, r6
mov r0, r0
ldb5 r1, 0
ldb5 r2, 1
subxy r2, r1, r2
mul r2, r2, r4 | mov r6, #1.0 | ld r4, [a044]
addxy r5, r5, r4 | addxy r1, r1, r2
subxy r6, r6, r5 | st r1, [a154]
sign r6, r6
addxy r6, r6, #1.0
mul r6, r6, #0.5
subxy r5, r6, r5
mul r4, r5, #127.0 | st r5, [0788] | ld r5, [078b]
floor r6, r4
subxy r4, r6, r4 | mul r6, r6, #6.103515625e-05
addxy r6, r6, #0.007843017578125
hset h6, r6
mov r0, r0
ldb5 r1, 0
ldb5 r2, 1
subxy r2, r1, r2
mul r2, r2, r4 | mov r6, #1.0 | ld r4, [a045]
addxy r5, r5, r4 | addxy r1, r1, r2
subxy r6, r6, r5 | st r1, [a155]
sign r6, r6
addxy r6, r6, #1.0
mul r6, r6, #0.5
subxy r5, r6, r5
mul r4, r5, #127.0 | st r5, [078a] | ld r5, [078d]
floor r6, r4
subxy r4, r6, r4 | mul r6, r6, #6.103515625e-05
addxy r6, r6, #0.007843017578125
hset h6, r6
mov r0, r0
ldb5 r1, 0
ldb5 r2, 1
subxy r2, r1, r2
mul r2, r2, r4 | mov r6, #1.0 | ld r4, [a046]
addxy r5, r5, r4 | addxy r1, r1, r2
subxy r6, r6, r5 | st r1, [a156]
sign r6, r6
addxy r6, r6, #1.0
mul r6, r6, #0.5
subxy r5, r6, r5
mul r4, r5, #127.0 | st r5, [078c] | ld r5, [078f]
floor r6, r4
subxy r4, r6, r4 | mul r6, r6, #6.103515625e-05
addxy r6, r6, #0.007843017578125
hset h6, r6
mov r0, r0
ldb5 r1, 0
ldb5 r2, 1
subxy r2, r1, r2
mul r2, r2, r4 | mov r6, #1.0 | ld r4, [a047]
addxy r5, r5, r4 | addxy r1, r1, r2
subxy r6, r6, r5 | st r1, [a157]
sign r6, r6
addxy r6, r6, #1.0
mul r6, r6, #0.5
subxy r5, r6, r5
mul r4, r5, #127.0 | st r5, [078e]
floor r6, r4
subxy r4, r6, r4 | mul r6, r6, #6.103515625e-05
addxy r6, r6, #0.007843017578125
hset h6, r6
mov r0, r0
ldb5 r1, 0
ldb5 r2, 1
subxy r2, r1, r2
mul r2, r2, r4
addxy r1, r1, r2
st r1, [a158]
