ld r0, [0343] | ld r2, [0341] | ld r6, [a039]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | st r6, [a161]
ph_wrap r0, r3, r0 | ld r4, [a061] | ld r6, [c04b]
addxy r2, r2, r0 | st r0, [0342] | ld r0, [a060]
ph_wrap r2, r3, r2 | ld r1, [a120]
st r2, [0340] | ld r2, [a038]
st r2, [a160] | ld r2, [c04b]
mul r1, r1, r2 | ld r5, [a121] | ld r2, [c05e]
mul r5, r5, r6 | st r0, [a168] | ld r6, [c05e]
st r4, [a169]
ld r0, [a128]
mad r1, r0, r2 | ld r4, [a129] | ld r2, [c05f]
mad r5, r4, r6 | addxy r1, r1, r2 | ld r6, [c05f]
addxy r5, r5, r6 | mov r0, #0.041666666666666664 | ld r2, [a030]
addxy r2, r2, r1 | mov r4, #0.041666666666666664 | ld r6, [a031]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [a1b0] | ld r1, [0303]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [a1b1] | ld r5, [030b]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a040]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a041]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | st r2, [0300] | ld r2, [a040]
addxy r2, r1, r2 | st r6, [0308] | ld r6, [a041]
addxy r6, r5, r6 | ld r0, [0300] | ld r4, [0308]
ph_wrap r1, r3, r2
ph_wrap r5, r3, r6 | st r1, [0302]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [030a]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a130] | ld r1, [0307]
mad r0, r1, #0.5 | st r5, [a131] | ld r5, [030f]
mad r4, r5, #0.5 | mul r2, r0, #2.0
mul r6, r4, #2.0
ph_wrap r3, r3, r2
ph_wrap r7, r3, r6 | st r3, [0306]
addxy r1, r3, #0.99212598425196852 | addxy r5, r7, #0.99212598425196852 | st r7, [030e]
mul r0, r1, #127.0 | mul r4, r5, #127.0
ph_wrap r2, r3, r0
ph_wrap r6, r3, r4
mov r1, r3 | mov r5, r7
mad r1, r2, #-0.007874015748031496 | mad r5, r6, #-0.007874015748031496
addxy r2, r1, #0.99212598425196852 | addxy r6, r5, #0.99212598425196852
subxy r0, r1, r3 | subxy r4, r5, r7
mul r3, r2, #63.5 | mul r7, r6, #63.5
addxy r2, r3, #128.5 | addxy r6, r7, #128.5
mul r3, r2, #6.103515625e-05 | mul r7, r6, #6.103515625e-05
hset h6, r3
hset h7, r7
ldb5 r2, 0
addxy r3, r1, #1.0078740157480315 | ldb5h7 r6, 0
addxy r7, r5, #1.0078740157480315 | mul r1, r3, #63.5
mul r5, r7, #63.5 | addxy r3, r1, #128.5
addxy r7, r5, #128.5 | mul r1, r3, #6.103515625e-05
mul r5, r7, #6.103515625e-05
hset h6, r1
hset h7, r5
ldb5 r3, 0
subxy r1, r2, r3 | addxy r3, r0, #0.0078740157480314821 | ldb5h7 r7, 0
subxy r5, r6, r7 | addxy r7, r4, #0.0078740157480314821
mac_p1wc3 r0, r1, r3, #0.015748031496062992, r2 | ld r1, [c070]
mac_p1wc3 r4, r5, r7, #0.015748031496062992, r6 | ld r5, [c070]
mul r3, r1, r0 | mul r7, r5, r4 | ld r1, [a130]
mul r2, r3, #63.5 | mul r6, r7, #63.5 | ld r5, [a131]
st r2, [0304] | st r6, [030c]
ld r0, [0304] | ld r4, [030c] | ld r2, [c056]
mad r0, r1, r2 | ld r6, [c056] | ld r1, [0340]
mad r4, r5, r6 | ld r5, [0340] | ld r2, [c057]
mad r0, r1, r2 | ld r6, [c057] | ld r2, [a03a]
mad r4, r5, r6 | ld r6, [a03b]
st r0, [a151]
st r4, [a152]
st r2, [a162] | ld r2, [c04b]
st r6, [a163] | ld r6, [c04b]
ld r0, [a062]
ld r4, [a063]
ld r1, [a122]
mul r1, r1, r2 | ld r5, [a123] | ld r2, [c05e]
mul r5, r5, r6 | st r0, [a16a] | ld r6, [c05e]
st r4, [a16b]
ld r0, [a12a]
mad r1, r0, r2 | ld r4, [a12b] | ld r2, [c05f]
mad r5, r4, r6 | addxy r1, r1, r2 | ld r6, [c05f]
addxy r5, r5, r6 | mov r0, #0.041666666666666664 | ld r2, [a032]
addxy r2, r2, r1 | mov r4, #0.041666666666666664 | ld r6, [a033]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [a1b2] | ld r1, [0313]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [a1b3] | ld r5, [031b]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a042]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a043]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | st r2, [0310] | ld r2, [a042]
addxy r2, r1, r2 | st r6, [0318] | ld r6, [a043]
addxy r6, r5, r6 | ld r0, [0310] | ld r4, [0318]
ph_wrap r1, r3, r2
ph_wrap r5, r3, r6 | st r1, [0312]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [031a]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a132] | ld r1, [0317]
mad r0, r1, #0.5 | st r5, [a133] | ld r5, [031f]
mad r4, r5, #0.5 | mul r2, r0, #2.0
mul r6, r4, #2.0
ph_wrap r3, r3, r2
ph_wrap r7, r3, r6 | st r3, [0316]
addxy r1, r3, #0.99212598425196852 | addxy r5, r7, #0.99212598425196852 | st r7, [031e]
mul r0, r1, #127.0 | mul r4, r5, #127.0
ph_wrap r2, r3, r0
ph_wrap r6, r3, r4
mov r1, r3 | mov r5, r7
mad r1, r2, #-0.007874015748031496 | mad r5, r6, #-0.007874015748031496
addxy r2, r1, #0.99212598425196852 | addxy r6, r5, #0.99212598425196852
subxy r0, r1, r3 | subxy r4, r5, r7
mul r3, r2, #63.5 | mul r7, r6, #63.5
addxy r2, r3, #128.5 | addxy r6, r7, #128.5
mul r3, r2, #6.103515625e-05 | mul r7, r6, #6.103515625e-05
hset h6, r3
hset h7, r7
ldb5 r2, 0
addxy r3, r1, #1.0078740157480315 | ldb5h7 r6, 0
addxy r7, r5, #1.0078740157480315 | mul r1, r3, #63.5
mul r5, r7, #63.5 | addxy r3, r1, #128.5
addxy r7, r5, #128.5 | mul r1, r3, #6.103515625e-05
mul r5, r7, #6.103515625e-05
hset h6, r1
hset h7, r5
ldb5 r3, 0
subxy r1, r2, r3 | addxy r3, r0, #0.0078740157480314821 | ldb5h7 r7, 0
subxy r5, r6, r7 | addxy r7, r4, #0.0078740157480314821
mac_p1wc3 r0, r1, r3, #0.015748031496062992, r2 | ld r1, [c070]
mac_p1wc3 r4, r5, r7, #0.015748031496062992, r6 | ld r5, [c070]
mul r3, r1, r0 | mul r7, r5, r4 | ld r1, [a132]
mul r2, r3, #63.5 | mul r6, r7, #63.5 | ld r5, [a133]
st r2, [0314] | st r6, [031c]
ld r0, [0314] | ld r4, [031c] | ld r2, [c056]
mad r0, r1, r2 | ld r6, [c056] | ld r1, [0340]
mad r4, r5, r6 | ld r5, [0340] | ld r2, [c057]
mad r0, r1, r2 | ld r6, [c057] | ld r2, [a03c]
mad r4, r5, r6 | ld r6, [a03d]
st r0, [a153]
st r4, [a154]
st r2, [a164] | ld r2, [c04b]
st r6, [a165] | ld r6, [c04b]
ld r0, [a064]
ld r4, [a065]
ld r1, [a124]
mul r1, r1, r2 | ld r5, [a125] | ld r2, [c05e]
mul r5, r5, r6 | st r0, [a16c] | ld r6, [c05e]
st r4, [a16d]
ld r0, [a12c]
mad r1, r0, r2 | ld r4, [a12d] | ld r2, [c05f]
mad r5, r4, r6 | addxy r1, r1, r2 | ld r6, [c05f]
addxy r5, r5, r6 | mov r0, #0.041666666666666664 | ld r2, [a034]
addxy r2, r2, r1 | mov r4, #0.041666666666666664 | ld r6, [a035]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [a1b4] | ld r1, [0323]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [a1b5] | ld r5, [032b]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a044]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a045]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | st r2, [0320] | ld r2, [a044]
addxy r2, r1, r2 | st r6, [0328] | ld r6, [a045]
addxy r6, r5, r6 | ld r0, [0320] | ld r4, [0328]
ph_wrap r1, r3, r2
ph_wrap r5, r3, r6 | st r1, [0322]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [032a]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a134] | ld r1, [0327]
mad r0, r1, #0.5 | st r5, [a135] | ld r5, [032f]
mad r4, r5, #0.5 | mul r2, r0, #2.0
mul r6, r4, #2.0
ph_wrap r3, r3, r2
ph_wrap r7, r3, r6 | st r3, [0326]
addxy r1, r3, #0.99212598425196852 | addxy r5, r7, #0.99212598425196852 | st r7, [032e]
mul r0, r1, #127.0 | mul r4, r5, #127.0
ph_wrap r2, r3, r0
ph_wrap r6, r3, r4
mov r1, r3 | mov r5, r7
mad r1, r2, #-0.007874015748031496 | mad r5, r6, #-0.007874015748031496
addxy r2, r1, #0.99212598425196852 | addxy r6, r5, #0.99212598425196852
subxy r0, r1, r3 | subxy r4, r5, r7
mul r3, r2, #63.5 | mul r7, r6, #63.5
addxy r2, r3, #128.5 | addxy r6, r7, #128.5
mul r3, r2, #6.103515625e-05 | mul r7, r6, #6.103515625e-05
hset h6, r3
hset h7, r7
ldb5 r2, 0
addxy r3, r1, #1.0078740157480315 | ldb5h7 r6, 0
addxy r7, r5, #1.0078740157480315 | mul r1, r3, #63.5
mul r5, r7, #63.5 | addxy r3, r1, #128.5
addxy r7, r5, #128.5 | mul r1, r3, #6.103515625e-05
mul r5, r7, #6.103515625e-05
hset h6, r1
hset h7, r5
ldb5 r3, 0
subxy r1, r2, r3 | addxy r3, r0, #0.0078740157480314821 | ldb5h7 r7, 0
subxy r5, r6, r7 | addxy r7, r4, #0.0078740157480314821
mac_p1wc3 r0, r1, r3, #0.015748031496062992, r2 | ld r1, [c070]
mac_p1wc3 r4, r5, r7, #0.015748031496062992, r6 | ld r5, [c070]
mul r3, r1, r0 | mul r7, r5, r4 | ld r1, [a134]
mul r2, r3, #63.5 | mul r6, r7, #63.5 | ld r5, [a135]
st r2, [0324] | st r6, [032c]
ld r0, [0324] | ld r4, [032c] | ld r2, [c056]
mad r0, r1, r2 | ld r6, [c056] | ld r1, [0340]
mad r4, r5, r6 | ld r5, [0340] | ld r2, [c057]
mad r0, r1, r2 | ld r6, [c057] | ld r2, [a03e]
mad r4, r5, r6 | ld r6, [a03f]
st r0, [a155]
st r4, [a156]
st r2, [a166] | ld r2, [c04b]
st r6, [a167] | ld r6, [c04b]
ld r0, [a066]
ld r4, [a067]
ld r1, [a126]
mul r1, r1, r2 | ld r5, [a127] | ld r2, [c05e]
mul r5, r5, r6 | st r0, [a16e] | ld r6, [c05e]
st r4, [a16f]
ld r0, [a12e]
mad r1, r0, r2 | ld r4, [a12f] | ld r2, [c05f]
mad r5, r4, r6 | addxy r1, r1, r2 | ld r6, [c05f]
addxy r5, r5, r6 | mov r0, #0.041666666666666664 | ld r2, [a036]
addxy r2, r2, r1 | mov r4, #0.041666666666666664 | ld r6, [a037]
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [a1b6] | ld r1, [0333]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [a1b7] | ld r5, [033b]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a046]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a047]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | st r2, [0330] | ld r2, [a046]
addxy r2, r1, r2 | st r6, [0338] | ld r6, [a047]
addxy r6, r5, r6 | ld r0, [0330] | ld r4, [0338]
ph_wrap r1, r3, r2
ph_wrap r5, r3, r6 | st r1, [0332]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [033a]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a136] | ld r1, [0337]
mad r0, r1, #0.5 | st r5, [a137] | ld r5, [033f]
mad r4, r5, #0.5 | mul r2, r0, #2.0
mul r6, r4, #2.0
ph_wrap r3, r3, r2
ph_wrap r7, r3, r6 | st r3, [0336]
addxy r1, r3, #0.99212598425196852 | addxy r5, r7, #0.99212598425196852 | st r7, [033e]
mul r0, r1, #127.0 | mul r4, r5, #127.0
ph_wrap r2, r3, r0
ph_wrap r6, r3, r4
mov r1, r3 | mov r5, r7
mad r1, r2, #-0.007874015748031496 | mad r5, r6, #-0.007874015748031496
addxy r2, r1, #0.99212598425196852 | addxy r6, r5, #0.99212598425196852
subxy r0, r1, r3 | subxy r4, r5, r7
mul r3, r2, #63.5 | mul r7, r6, #63.5
addxy r2, r3, #128.5 | addxy r6, r7, #128.5
mul r3, r2, #6.103515625e-05 | mul r7, r6, #6.103515625e-05
hset h6, r3
hset h7, r7
ldb5 r2, 0
addxy r3, r1, #1.0078740157480315 | ldb5h7 r6, 0
addxy r7, r5, #1.0078740157480315 | mul r1, r3, #63.5
mul r5, r7, #63.5 | addxy r3, r1, #128.5
addxy r7, r5, #128.5 | mul r1, r3, #6.103515625e-05
mul r5, r7, #6.103515625e-05
hset h6, r1
hset h7, r5
ldb5 r3, 0
subxy r1, r2, r3 | addxy r3, r0, #0.0078740157480314821 | ldb5h7 r7, 0
subxy r5, r6, r7 | addxy r7, r4, #0.0078740157480314821
mac_p1wc3 r0, r1, r3, #0.015748031496062992, r2 | ld r1, [c070]
mac_p1wc3 r4, r5, r7, #0.015748031496062992, r6 | ld r5, [c070]
mul r3, r1, r0 | mul r7, r5, r4 | ld r1, [a136]
mul r2, r3, #63.5 | mul r6, r7, #63.5 | ld r5, [a137]
st r2, [0334] | st r6, [033c]
ld r0, [0334] | ld r4, [033c] | ld r2, [c056]
mad r0, r1, r2 | ld r6, [c056] | ld r1, [0340]
mad r4, r5, r6 | ld r5, [0340] | ld r2, [c057]
mad r0, r1, r2 | ld r6, [c057]
mad r4, r5, r6 | st r0, [a157]
st r4, [a158]
