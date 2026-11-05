mov r1, #0.031219482421875 | mov r3, #0.003875732421875 | ld r0, [0351] | ld r4, [a061] | ld r5, [0307]
addxy r0, r0, #6.103515625e-05 | subxy r5, r5, r4 | ld r6, [a039] | st r4, [0306]
subxy r1, r0, r1 | mul r5, r5, r5 | st r6, [a161] | ld r6, [030b]
sign r1, r1 | st r4, [a169] | ld r4, [0309] | ld r7, [c049]
addxy r1, r1, #1.0 | mad r6, r6, -r5
mul r1, r1, #0.5 | addxy r5, r4, #-0.999
mul r0, r0, r1 | mul r5, r5, #256.0 | ld r1, [a0ff]
subxy r1, r1, r0 | st r0, [0350] | ld r0, [a060]
mov r2, r1 | st r0, [a168] | st r0, [0300]
subxy r3, r1, r3 | addxy r2, r2, #3.0517578125e-05
max r1, r1, #0.0
sign r2, r2
sign r3, r3
min r1, r1, #0.00384521484375 | ph_clamp r5, r3, -r5
addxy r2, r2, #1.0 | addxy r3, r3, #1.0
addxy r1, r1, #0.007843017578125 | mul r2, r2, #0.5
mul r3, r3, #0.5 | mul r5, r5, #256.0
hset h6, r1
mul r2, r2, r3
ph_clamp r5, r3, -r5 | ldb5 r3, 0
max r6, r6, r5 | ld r1, [0240] | ld r5, [c048]
subxy r3, r1, r3 | subxy r6, r6, #1.0 | st r6, [030a]
mul r3, r3, r2 | mul r6, r6, #256.0 | ld r2, [a038]
addxy r3, r3, r1 | subxy r7, r5, r7 | ld r1, [0301] | st r2, [a160]
subxy r1, r1, r0 | ld r2, [0305] | ld r0, [0303]
ph_clamp r6, r3, -r6 | st r3, [0040] | ld r3, [c049]
mul r1, r1, r1 | mad r5, r6, r7 | ld r7, [c049]
mad r2, r2, -r1 | mul r6, r6, #1.25
addxy r1, r0, #-0.999 | subxy r6, r4, r6
mul r1, r1, #256.0 | mad r4, r6, r5 | ld r5, [a121]
ph_clamp r1, r3, -r1 | min r4, r4, #1.0 | ld r6, [c04b]
mul r1, r1, #256.0 | mul r5, r5, r6 | ld r6, [c05e] | st r4, [0308]
ph_clamp r1, r3, -r1
mad r5, r4, r6 | ld r6, [c05f] | ld r4, [a039]
max r2, r2, r1 | ld r1, [c048] | ld r4, [a063]
addxy r5, r5, r6 | subxy r2, r2, #1.0 | st r2, [0304] | ld r6, [a031]
mul r2, r2, #256.0 | addxy r6, r6, r5 | st r4, [a16b] | st r4, [0312]
mov r5, #0.0 | st r6, [a1b1]
ph_clamp r2, r3, -r2 | st r5, [0332] | ld r5, [0313]
subxy r3, r1, r3 | subxy r5, r5, r4 | ld r6, [a03b] | ld r4, [0315]
mad r1, r2, r3 | mul r2, r2, #1.25 | st r6, [a163] | ld r6, [0317]
subxy r2, r0, r2 | mul r5, r5, r5 | ld r3, [c049]
mad r0, r2, r1 | ld r1, [a120] | ld r2, [c04b]
mad r6, r6, -r5 | addxy r5, r4, #-0.999
mul r1, r1, r2 | mul r5, r5, #256.0 | ld r2, [c05e]
min r0, r0, #1.0 | ph_clamp r5, r3, -r5
mad r1, r0, r2 | mul r5, r5, #256.0 | ld r2, [c05f] | st r0, [0302]
addxy r1, r1, r2 | ld r2, [a030]
ph_clamp r5, r3, -r5 | ld r0, [a038]
addxy r2, r2, r1 | mov r1, #0.0 | ld r0, [a062]
max r6, r6, r5 | st r1, [0330] | st r2, [a1b0] | ld r1, [030d]
subxy r6, r6, #1.0 | st r6, [0316] | ld r2, [a03a] | ld r5, [c048]
mul r6, r6, #256.0 | subxy r1, r1, r0 | st r2, [a162] | st r0, [030c] | ld r2, [0311]
ph_clamp r6, r3, -r6 | st r0, [a16a] | ld r0, [030f]
subxy r7, r5, r7 | mul r1, r1, r1
mad r5, r6, r7 | mad r2, r2, -r1 | ld r7, [c049]
mul r6, r6, #1.25 | addxy r1, r0, #-0.999
subxy r6, r4, r6 | mul r1, r1, #256.0
mad r4, r6, r5 | ld r5, [a123] | ld r6, [c04b]
ph_clamp r1, r3, -r1 | min r4, r4, #1.0
mul r1, r1, #256.0 | mul r5, r5, r6 | ld r6, [c05e] | st r4, [0314]
ph_clamp r1, r3, -r1
mad r5, r4, r6 | ld r6, [c05f] | ld r4, [a03b]
max r2, r2, r1 | ld r1, [c048] | ld r4, [a065]
addxy r5, r5, r6 | subxy r2, r2, #1.0 | st r2, [0310] | ld r6, [a033]
mul r2, r2, #256.0 | addxy r6, r6, r5 | st r4, [a16d] | st r4, [031e]
mov r5, #0.0 | st r6, [a1b3]
ph_clamp r2, r3, -r2 | st r5, [0336] | ld r5, [031f]
subxy r3, r1, r3 | subxy r5, r5, r4 | ld r6, [a03d] | ld r4, [0321]
mad r1, r2, r3 | mul r2, r2, #1.25 | st r6, [a165] | ld r6, [0323]
subxy r2, r0, r2 | mul r5, r5, r5 | ld r3, [c049]
mad r0, r2, r1 | ld r1, [a122] | ld r2, [c04b]
mad r6, r6, -r5 | addxy r5, r4, #-0.999
mul r1, r1, r2 | mul r5, r5, #256.0 | ld r2, [c05e]
min r0, r0, #1.0 | ph_clamp r5, r3, -r5
mad r1, r0, r2 | mul r5, r5, #256.0 | ld r2, [c05f] | st r0, [030e]
addxy r1, r1, r2 | ld r2, [a032]
ph_clamp r5, r3, -r5 | ld r0, [a03a]
addxy r2, r2, r1 | mov r1, #0.0 | ld r0, [a064]
max r6, r6, r5 | st r1, [0334] | st r2, [a1b2] | ld r1, [0319]
subxy r6, r6, #1.0 | st r6, [0322] | ld r2, [a03c] | ld r5, [c048]
mul r6, r6, #256.0 | subxy r1, r1, r0 | st r2, [a164] | st r0, [0318] | ld r2, [031d]
ph_clamp r6, r3, -r6 | st r0, [a16c] | ld r0, [031b]
subxy r7, r5, r7 | mul r1, r1, r1
mad r5, r6, r7 | mad r2, r2, -r1 | ld r7, [c049]
mul r6, r6, #1.25 | addxy r1, r0, #-0.999
subxy r6, r4, r6 | mul r1, r1, #256.0
mad r4, r6, r5 | ld r5, [a125] | ld r6, [c04b]
ph_clamp r1, r3, -r1 | min r4, r4, #1.0
mul r1, r1, #256.0 | mul r5, r5, r6 | ld r6, [c05e] | st r4, [0320]
ph_clamp r1, r3, -r1
mad r5, r4, r6 | ld r6, [c05f] | ld r4, [a03d]
max r2, r2, r1 | ld r1, [c048] | ld r4, [a067]
addxy r5, r5, r6 | subxy r2, r2, #1.0 | st r2, [031c] | ld r6, [a035]
mul r2, r2, #256.0 | addxy r6, r6, r5 | st r4, [a16f] | st r4, [032a]
mov r5, #0.0 | st r6, [a1b5]
ph_clamp r2, r3, -r2 | st r5, [033a] | ld r5, [032b]
subxy r3, r1, r3 | subxy r5, r5, r4 | ld r6, [a03f] | ld r4, [032d]
mad r1, r2, r3 | mul r2, r2, #1.25 | st r6, [a167] | ld r6, [032f]
subxy r2, r0, r2 | mul r5, r5, r5 | ld r3, [c049]
mad r0, r2, r1 | ld r1, [a124] | ld r2, [c04b]
mad r6, r6, -r5 | addxy r5, r4, #-0.999
mul r1, r1, r2 | mul r5, r5, #256.0 | ld r2, [c05e]
min r0, r0, #1.0 | ph_clamp r5, r3, -r5
mad r1, r0, r2 | mul r5, r5, #256.0 | ld r2, [c05f] | st r0, [031a]
addxy r1, r1, r2 | ld r2, [a034]
ph_clamp r5, r3, -r5 | ld r0, [a03c]
addxy r2, r2, r1 | mov r1, #0.0 | ld r0, [a066]
max r6, r6, r5 | st r1, [0338] | st r2, [a1b4] | ld r1, [0325]
subxy r6, r6, #1.0 | st r6, [032e] | ld r2, [a03e] | ld r5, [c048]
mul r6, r6, #256.0 | subxy r1, r1, r0 | st r2, [a166] | st r0, [0324] | ld r2, [0329]
ph_clamp r6, r3, -r6 | st r0, [a16e] | ld r0, [0327]
subxy r7, r5, r7 | mul r1, r1, r1
mad r5, r6, r7 | mad r2, r2, -r1
mul r6, r6, #1.25 | addxy r1, r0, #-0.999
subxy r6, r4, r6 | mul r1, r1, #256.0
mad r4, r6, r5 | ld r5, [a127] | ld r6, [c04b]
ph_clamp r1, r3, -r1 | min r4, r4, #1.0
mul r1, r1, #256.0 | mul r5, r5, r6 | ld r6, [c05e] | st r4, [032c]
ph_clamp r1, r3, -r1
mad r5, r4, r6 | ld r6, [c05f] | ld r4, [a03f]
max r2, r2, r1 | ld r1, [c048] | ld r4, [a040]
addxy r5, r5, r6 | subxy r2, r2, #1.0 | st r2, [0328] | ld r6, [a037]
mul r2, r2, #256.0 | addxy r6, r6, r5
mov r5, #0.0 | mov r6, #1.0 | st r6, [a1b7]
ph_clamp r2, r3, -r2 | st r5, [033e] | ld r5, [0341]
subxy r3, r1, r3 | addxy r5, r5, r4
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2 | subxy r6, r6, r5
mad r0, r2, r1 | ld r1, [a126] | ld r2, [c04b]
sign r6, r6
mul r1, r1, r2 | addxy r6, r6, #1.0 | ld r2, [c05e]
min r0, r0, #1.0
mul r6, r6, #0.5 | mad r1, r0, r2 | ld r2, [c05f] | st r0, [0326]
subxy r5, r6, r5 | addxy r1, r1, r2 | ld r2, [a036]
mul r4, r5, #256.0 | addxy r2, r2, r1 | ld r0, [a03e] | st r5, [0340]
floor r6, r4 | ld r0, [0351] | st r2, [a1b6]
mov r1, #0.0 | ld r5, [0343]
subxy r4, r6, r4 | mul r6, r6, #0.0001220703125 | st r1, [033c]
addxy r6, r6, r0 | mov r1, #0.031219482421875
subxy r1, r1, r6
sign r1, r1
addxy r1, r1, #1.0
mul r1, r1, #0.015625
subxy r6, r1, r6
addxy r6, r6, #0.003997802734375
hset h6, r6
mov r0, r0
ldidx r0, 0
ldidx r1, 1
ldidx r2, 2
subxy r2, r0, r2 | ldidx r3, 3
subxy r3, r1, r3 | mul r2, r2, r4
mul r3, r3, r4 | mov r6, #1.0 | ld r4, [a041]
addxy r5, r5, r4 | addxy r0, r0, r2 | ld r2, [a0fe]
subxy r6, r6, r5 | addxy r1, r1, r3
sign r6, r6
addxy r6, r6, #1.0 | subxy r1, r0, r1
mul r6, r6, #0.5 | mul r1, r1, r2
subxy r5, r6, r5 | addxy r0, r0, r1
mul r4, r5, #256.0 | mov r1, #0.031219482421875 | st r0, [a151] | ld r0, [0351] | st r5, [0342]
floor r6, r4 | ld r5, [0345]
subxy r4, r6, r4 | mul r6, r6, #0.0001220703125
addxy r6, r6, r0
subxy r1, r1, r6
sign r1, r1
addxy r1, r1, #1.0
mul r1, r1, #0.015625
subxy r6, r1, r6
addxy r6, r6, #0.003997802734375
hset h6, r6
mov r0, r0
ldidx r0, 0
ldidx r1, 1
ldidx r2, 2
subxy r2, r0, r2 | ldidx r3, 3
subxy r3, r1, r3 | mul r2, r2, r4
mul r3, r3, r4 | mov r6, #1.0 | ld r4, [a042]
addxy r5, r5, r4 | addxy r0, r0, r2 | ld r2, [a0fe]
subxy r6, r6, r5 | addxy r1, r1, r3
sign r6, r6
addxy r6, r6, #1.0 | subxy r1, r0, r1
mul r6, r6, #0.5 | mul r1, r1, r2
subxy r5, r6, r5 | addxy r0, r0, r1
mul r4, r5, #256.0 | mov r1, #0.031219482421875 | st r0, [a152] | ld r0, [0351] | st r5, [0344]
floor r6, r4 | ld r5, [0347]
subxy r4, r6, r4 | mul r6, r6, #0.0001220703125
addxy r6, r6, r0
subxy r1, r1, r6
sign r1, r1
addxy r1, r1, #1.0
mul r1, r1, #0.015625
subxy r6, r1, r6
addxy r6, r6, #0.003997802734375
hset h6, r6
mov r0, r0
ldidx r0, 0
ldidx r1, 1
ldidx r2, 2
subxy r2, r0, r2 | ldidx r3, 3
subxy r3, r1, r3 | mul r2, r2, r4
mul r3, r3, r4 | mov r6, #1.0 | ld r4, [a043]
addxy r5, r5, r4 | addxy r0, r0, r2 | ld r2, [a0fe]
subxy r6, r6, r5 | addxy r1, r1, r3
sign r6, r6
addxy r6, r6, #1.0 | subxy r1, r0, r1
mul r6, r6, #0.5 | mul r1, r1, r2
subxy r5, r6, r5 | addxy r0, r0, r1
mul r4, r5, #256.0 | mov r1, #0.031219482421875 | st r0, [a153] | ld r0, [0351] | st r5, [0346]
floor r6, r4 | ld r5, [0349]
subxy r4, r6, r4 | mul r6, r6, #0.0001220703125
addxy r6, r6, r0
subxy r1, r1, r6
sign r1, r1
addxy r1, r1, #1.0
mul r1, r1, #0.015625
subxy r6, r1, r6
addxy r6, r6, #0.003997802734375
hset h6, r6
mov r0, r0
ldidx r0, 0
ldidx r1, 1
ldidx r2, 2
subxy r2, r0, r2 | ldidx r3, 3
subxy r3, r1, r3 | mul r2, r2, r4
mul r3, r3, r4 | mov r6, #1.0 | ld r4, [a044]
addxy r5, r5, r4 | addxy r0, r0, r2 | ld r2, [a0fe]
subxy r6, r6, r5 | addxy r1, r1, r3
sign r6, r6
addxy r6, r6, #1.0 | subxy r1, r0, r1
mul r6, r6, #0.5 | mul r1, r1, r2
subxy r5, r6, r5 | addxy r0, r0, r1
mul r4, r5, #256.0 | mov r1, #0.031219482421875 | st r0, [a154] | ld r0, [0351] | st r5, [0348]
floor r6, r4 | ld r5, [034b]
subxy r4, r6, r4 | mul r6, r6, #0.0001220703125
addxy r6, r6, r0
subxy r1, r1, r6
sign r1, r1
addxy r1, r1, #1.0
mul r1, r1, #0.015625
subxy r6, r1, r6
addxy r6, r6, #0.003997802734375
hset h6, r6
mov r0, r0
ldidx r0, 0
ldidx r1, 1
ldidx r2, 2
subxy r2, r0, r2 | ldidx r3, 3
subxy r3, r1, r3 | mul r2, r2, r4
mul r3, r3, r4 | mov r6, #1.0 | ld r4, [a045]
addxy r5, r5, r4 | addxy r0, r0, r2 | ld r2, [a0fe]
subxy r6, r6, r5 | addxy r1, r1, r3
sign r6, r6
addxy r6, r6, #1.0 | subxy r1, r0, r1
mul r6, r6, #0.5 | mul r1, r1, r2
subxy r5, r6, r5 | addxy r0, r0, r1
mul r4, r5, #256.0 | mov r1, #0.031219482421875 | st r0, [a155] | ld r0, [0351] | st r5, [034a]
floor r6, r4 | ld r5, [034d]
subxy r4, r6, r4 | mul r6, r6, #0.0001220703125
addxy r6, r6, r0
subxy r1, r1, r6
sign r1, r1
addxy r1, r1, #1.0
mul r1, r1, #0.015625
subxy r6, r1, r6
addxy r6, r6, #0.003997802734375
hset h6, r6
mov r0, r0
ldidx r0, 0
ldidx r1, 1
ldidx r2, 2
subxy r2, r0, r2 | ldidx r3, 3
subxy r3, r1, r3 | mul r2, r2, r4
mul r3, r3, r4 | mov r6, #1.0 | ld r4, [a046]
addxy r5, r5, r4 | addxy r0, r0, r2 | ld r2, [a0fe]
subxy r6, r6, r5 | addxy r1, r1, r3
sign r6, r6
addxy r6, r6, #1.0 | subxy r1, r0, r1
mul r6, r6, #0.5 | mul r1, r1, r2
subxy r5, r6, r5 | addxy r0, r0, r1
mul r4, r5, #256.0 | mov r1, #0.031219482421875 | st r0, [a156] | ld r0, [0351] | st r5, [034c]
floor r6, r4 | ld r5, [034f]
subxy r4, r6, r4 | mul r6, r6, #0.0001220703125
addxy r6, r6, r0
subxy r1, r1, r6
sign r1, r1
addxy r1, r1, #1.0
mul r1, r1, #0.015625
subxy r6, r1, r6
addxy r6, r6, #0.003997802734375
hset h6, r6
mov r0, r0
ldidx r0, 0
ldidx r1, 1
ldidx r2, 2
subxy r2, r0, r2 | ldidx r3, 3
subxy r3, r1, r3 | mul r2, r2, r4
mul r3, r3, r4 | mov r6, #1.0 | ld r4, [a047]
addxy r5, r5, r4 | addxy r0, r0, r2 | ld r2, [a0fe]
subxy r6, r6, r5 | addxy r1, r1, r3
sign r6, r6
addxy r6, r6, #1.0 | subxy r1, r0, r1
mul r6, r6, #0.5 | mul r1, r1, r2
subxy r5, r6, r5 | addxy r0, r0, r1
mul r4, r5, #256.0 | mov r1, #0.031219482421875 | st r0, [a157] | ld r0, [0351] | st r5, [034e]
floor r6, r4
subxy r4, r6, r4 | mul r6, r6, #0.0001220703125
addxy r6, r6, r0
subxy r1, r1, r6
sign r1, r1
addxy r1, r1, #1.0
mul r1, r1, #0.015625
subxy r6, r1, r6
addxy r6, r6, #0.003997802734375
hset h6, r6
mov r0, r0
ldidx r0, 0
ldidx r1, 1
ldidx r2, 2
subxy r2, r0, r2 | ldidx r3, 3
subxy r3, r1, r3 | mul r2, r2, r4
mul r3, r3, r4 | addxy r0, r0, r2 | ld r2, [a0fe]
addxy r1, r1, r3
subxy r1, r0, r1
mul r1, r1, r2
addxy r0, r0, r1
st r0, [a158]
