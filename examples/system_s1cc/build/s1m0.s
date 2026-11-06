mov r3, #0.041666666666666664 | ld r0, [0343] | ld r2, [0341] | ld r1, [a120]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275
ph_wrap r0, r3, r0
addxy r2, r2, r0 | st r0, [0342] | ld r0, [a060]
ph_wrap r2, r3, r2 | st r0, [a168]
st r2, [0340] | ld r2, [a038]
st r2, [a160] | ld r2, [c04b]
mul r1, r1, r2 | ld r0, [a128] | ld r2, [c05e]
mad r1, r0, r2 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a030]
addxy r2, r2, r1 | ld r1, [0303]
st r2, [0300] | ld r2, [a040]
addxy r2, r1, r2 | ld r0, [0300]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [0302]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a130]
mul r2, r1, #0.04332169878499658 | ld r1, [0307]
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r5, r4, r4
mul r6, r5, r5
mul r7, r6, r6
mul r0, r7, #0.03125
mad r1, r0, #0.29333333019167185
ph_wrap r2, r3, r1
addxy r3, r2, #0.99212598425196852 | st r2, [0306]
mul r4, r3, #127.0 | mov r6, r2
ph_wrap r5, r3, r4
mad r6, r5, #-0.007874015748031496
addxy r0, r6, #0.99212598425196852 | subxy r7, r6, r2
mul r1, r0, #63.5 | addxy r2, r6, #1.0078740157480315
addxy r3, r1, #128.5 | mul r0, r2, #63.5
mul r4, r3, #6.103515625e-05 | addxy r1, r0, #128.5
hset h6, r4
mul r3, r1, #6.103515625e-05 | addxy r2, r7, #0.0078740157480314821
ldb5 r5, 0
hset h6, r3
mov r0, r0
ldb5 r4, 0
subxy r6, r5, r4
mac_p1wc3 r0, r6, r2, #0.015748031496062992, r5 | ld r2, [c056]
mul r1, r0, #63.5 | mov r3, #0.041666666666666664
st r1, [0304] | ld r1, [a130]
ld r0, [0304]
mad r0, r1, r2 | ld r1, [0340] | ld r2, [c057]
mad r0, r1, r2 | ld r2, [a039]
st r0, [a151]
st r2, [a161] | ld r2, [c04b]
ld r0, [a061]
ld r1, [a121]
mul r1, r1, r2 | st r0, [a169] | ld r2, [c05e]
ld r0, [a129]
mad r1, r0, r2 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a031]
addxy r2, r2, r1 | ld r1, [030b]
st r2, [0308] | ld r2, [a041]
addxy r2, r1, r2 | ld r0, [0308]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [030a]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a131]
mul r2, r1, #0.04332169878499658 | ld r1, [030f]
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r5, r4, r4
mul r6, r5, r5
mul r7, r6, r6
mul r0, r7, #0.03125
mad r1, r0, #0.29333333019167185
ph_wrap r2, r3, r1
addxy r3, r2, #0.99212598425196852 | st r2, [030e]
mul r4, r3, #127.0 | mov r6, r2
ph_wrap r5, r3, r4
mad r6, r5, #-0.007874015748031496
addxy r0, r6, #0.99212598425196852 | subxy r7, r6, r2
mul r1, r0, #63.5 | addxy r2, r6, #1.0078740157480315
addxy r3, r1, #128.5 | mul r0, r2, #63.5
mul r4, r3, #6.103515625e-05 | addxy r1, r0, #128.5
hset h7, r4
mul r3, r1, #6.103515625e-05 | addxy r2, r7, #0.0078740157480314821
ldb5h7 r5, 0
hset h7, r3
mov r0, r0
ldb5h7 r4, 0
subxy r6, r5, r4
mac_p1wc3 r0, r6, r2, #0.015748031496062992, r5 | ld r2, [c056]
mul r1, r0, #63.5 | mov r3, #0.041666666666666664
st r1, [030c] | ld r1, [a131]
ld r0, [030c]
mad r0, r1, r2 | ld r1, [0340] | ld r2, [c057]
mad r0, r1, r2 | ld r2, [a03a]
st r0, [a152]
st r2, [a162] | ld r2, [c04b]
ld r0, [a062]
ld r1, [a122]
mul r1, r1, r2 | st r0, [a16a] | ld r2, [c05e]
ld r0, [a12a]
mad r1, r0, r2 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a032]
addxy r2, r2, r1 | ld r1, [0313]
st r2, [0310] | ld r2, [a042]
addxy r2, r1, r2 | ld r0, [0310]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [0312]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a132]
mul r2, r1, #0.04332169878499658 | ld r1, [0317]
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r5, r4, r4
mul r6, r5, r5
mul r7, r6, r6
mul r0, r7, #0.03125
mad r1, r0, #0.29333333019167185
ph_wrap r2, r3, r1
addxy r3, r2, #0.99212598425196852 | st r2, [0316]
mul r4, r3, #127.0 | mov r6, r2
ph_wrap r5, r3, r4
mad r6, r5, #-0.007874015748031496
addxy r0, r6, #0.99212598425196852 | subxy r7, r6, r2
mul r1, r0, #63.5 | addxy r2, r6, #1.0078740157480315
addxy r3, r1, #128.5 | mul r0, r2, #63.5
mul r4, r3, #6.103515625e-05 | addxy r1, r0, #128.5
hset h6, r4
mul r3, r1, #6.103515625e-05 | addxy r2, r7, #0.0078740157480314821
ldb5 r5, 0
hset h6, r3
mov r0, r0
ldb5 r4, 0
subxy r6, r5, r4
mac_p1wc3 r0, r6, r2, #0.015748031496062992, r5 | ld r2, [c056]
mul r1, r0, #63.5 | mov r3, #0.041666666666666664
st r1, [0314] | ld r1, [a132]
ld r0, [0314]
mad r0, r1, r2 | ld r1, [0340] | ld r2, [c057]
mad r0, r1, r2 | ld r2, [a03b]
st r0, [a153]
st r2, [a163] | ld r2, [c04b]
ld r0, [a063]
ld r1, [a123]
mul r1, r1, r2 | st r0, [a16b] | ld r2, [c05e]
ld r0, [a12b]
mad r1, r0, r2 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a033]
addxy r2, r2, r1 | ld r1, [031b]
st r2, [0318] | ld r2, [a043]
addxy r2, r1, r2 | ld r0, [0318]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [031a]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a133]
mul r2, r1, #0.04332169878499658 | ld r1, [031f]
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r5, r4, r4
mul r6, r5, r5
mul r7, r6, r6
mul r0, r7, #0.03125
mad r1, r0, #0.29333333019167185
ph_wrap r2, r3, r1
addxy r3, r2, #0.99212598425196852 | st r2, [031e]
mul r4, r3, #127.0 | mov r6, r2
ph_wrap r5, r3, r4
mad r6, r5, #-0.007874015748031496
addxy r0, r6, #0.99212598425196852 | subxy r7, r6, r2
mul r1, r0, #63.5 | addxy r2, r6, #1.0078740157480315
addxy r3, r1, #128.5 | mul r0, r2, #63.5
mul r4, r3, #6.103515625e-05 | addxy r1, r0, #128.5
hset h7, r4
mul r3, r1, #6.103515625e-05 | addxy r2, r7, #0.0078740157480314821
ldb5h7 r5, 0
hset h7, r3
mov r0, r0
ldb5h7 r4, 0
subxy r6, r5, r4
mac_p1wc3 r0, r6, r2, #0.015748031496062992, r5 | ld r2, [c056]
mul r1, r0, #63.5 | mov r3, #0.041666666666666664
st r1, [031c] | ld r1, [a133]
ld r0, [031c]
mad r0, r1, r2 | ld r1, [0340] | ld r2, [c057]
mad r0, r1, r2 | ld r2, [a03c]
st r0, [a154]
st r2, [a164] | ld r2, [c04b]
ld r0, [a064]
ld r1, [a124]
mul r1, r1, r2 | st r0, [a16c] | ld r2, [c05e]
ld r0, [a12c]
mad r1, r0, r2 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a034]
addxy r2, r2, r1 | ld r1, [0323]
st r2, [0320] | ld r2, [a044]
addxy r2, r1, r2 | ld r0, [0320]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [0322]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a134]
mul r2, r1, #0.04332169878499658 | ld r1, [0327]
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r5, r4, r4
mul r6, r5, r5
mul r7, r6, r6
mul r0, r7, #0.03125
mad r1, r0, #0.29333333019167185
ph_wrap r2, r3, r1
addxy r3, r2, #0.99212598425196852 | st r2, [0326]
mul r4, r3, #127.0 | mov r6, r2
ph_wrap r5, r3, r4
mad r6, r5, #-0.007874015748031496
addxy r0, r6, #0.99212598425196852 | subxy r7, r6, r2
mul r1, r0, #63.5 | addxy r2, r6, #1.0078740157480315
addxy r3, r1, #128.5 | mul r0, r2, #63.5
mul r4, r3, #6.103515625e-05 | addxy r1, r0, #128.5
hset h6, r4
mul r3, r1, #6.103515625e-05 | addxy r2, r7, #0.0078740157480314821
ldb5 r5, 0
hset h6, r3
mov r0, r0
ldb5 r4, 0
subxy r6, r5, r4
mac_p1wc3 r0, r6, r2, #0.015748031496062992, r5 | ld r2, [c056]
mul r1, r0, #63.5 | mov r3, #0.041666666666666664
st r1, [0324] | ld r1, [a134]
ld r0, [0324]
mad r0, r1, r2 | ld r1, [0340] | ld r2, [c057]
mad r0, r1, r2 | ld r2, [a03d]
st r0, [a155]
st r2, [a165] | ld r2, [c04b]
ld r0, [a065]
ld r1, [a125]
mul r1, r1, r2 | st r0, [a16d] | ld r2, [c05e]
ld r0, [a12d]
mad r1, r0, r2 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a035]
addxy r2, r2, r1 | ld r1, [032b]
st r2, [0328] | ld r2, [a045]
addxy r2, r1, r2 | ld r0, [0328]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [032a]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a135]
mul r2, r1, #0.04332169878499658 | ld r1, [032f]
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r5, r4, r4
mul r6, r5, r5
mul r7, r6, r6
mul r0, r7, #0.03125
mad r1, r0, #0.29333333019167185
ph_wrap r2, r3, r1
addxy r3, r2, #0.99212598425196852 | st r2, [032e]
mul r4, r3, #127.0 | mov r6, r2
ph_wrap r5, r3, r4
mad r6, r5, #-0.007874015748031496
addxy r0, r6, #0.99212598425196852 | subxy r7, r6, r2
mul r1, r0, #63.5 | addxy r2, r6, #1.0078740157480315
addxy r3, r1, #128.5 | mul r0, r2, #63.5
mul r4, r3, #6.103515625e-05 | addxy r1, r0, #128.5
hset h7, r4
mul r3, r1, #6.103515625e-05 | addxy r2, r7, #0.0078740157480314821
ldb5h7 r5, 0
hset h7, r3
mov r0, r0
ldb5h7 r4, 0
subxy r6, r5, r4
mac_p1wc3 r0, r6, r2, #0.015748031496062992, r5 | ld r2, [c056]
mul r1, r0, #63.5 | mov r3, #0.041666666666666664
st r1, [032c] | ld r1, [a135]
ld r0, [032c]
mad r0, r1, r2 | ld r1, [0340] | ld r2, [c057]
mad r0, r1, r2 | ld r2, [a03e]
st r0, [a156]
st r2, [a166] | ld r2, [c04b]
ld r0, [a066]
ld r1, [a126]
mul r1, r1, r2 | st r0, [a16e] | ld r2, [c05e]
ld r0, [a12e]
mad r1, r0, r2 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a036]
addxy r2, r2, r1 | ld r1, [0333]
st r2, [0330] | ld r2, [a046]
addxy r2, r1, r2 | ld r0, [0330]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [0332]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a136]
mul r2, r1, #0.04332169878499658 | ld r1, [0337]
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r5, r4, r4
mul r6, r5, r5
mul r7, r6, r6
mul r0, r7, #0.03125
mad r1, r0, #0.29333333019167185
ph_wrap r2, r3, r1
addxy r3, r2, #0.99212598425196852 | st r2, [0336]
mul r4, r3, #127.0 | mov r6, r2
ph_wrap r5, r3, r4
mad r6, r5, #-0.007874015748031496
addxy r0, r6, #0.99212598425196852 | subxy r7, r6, r2
mul r1, r0, #63.5 | addxy r2, r6, #1.0078740157480315
addxy r3, r1, #128.5 | mul r0, r2, #63.5
mul r4, r3, #6.103515625e-05 | addxy r1, r0, #128.5
hset h6, r4
mul r3, r1, #6.103515625e-05 | addxy r2, r7, #0.0078740157480314821
ldb5 r5, 0
hset h6, r3
mov r0, r0
ldb5 r4, 0
subxy r6, r5, r4
mac_p1wc3 r0, r6, r2, #0.015748031496062992, r5 | ld r2, [c056]
mul r1, r0, #63.5 | mov r3, #0.041666666666666664
st r1, [0334] | ld r1, [a136]
ld r0, [0334]
mad r0, r1, r2 | ld r1, [0340] | ld r2, [c057]
mad r0, r1, r2 | ld r2, [a03f]
st r0, [a157]
st r2, [a167] | ld r2, [c04b]
ld r0, [a067]
ld r1, [a127]
mul r1, r1, r2 | st r0, [a16f] | ld r2, [c05e]
ld r0, [a12f]
mad r1, r0, r2 | ld r2, [c05f]
addxy r1, r1, r2 | ld r2, [a037]
addxy r2, r2, r1 | ld r1, [033b]
st r2, [0338] | ld r2, [a047]
addxy r2, r1, r2 | ld r0, [0338]
ph_wrap r1, r3, r2
mul r1, r1, #256.0 | st r1, [033a]
ph_clamp r1, r3, -r1
addxy r1, r0, #-4.75 | st r1, [a137]
mul r2, r1, #0.04332169878499658 | ld r1, [033f]
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r5, r4, r4
mul r6, r5, r5
mul r7, r6, r6
mul r0, r7, #0.03125
mad r1, r0, #0.29333333019167185
ph_wrap r2, r3, r1
addxy r3, r2, #0.99212598425196852 | st r2, [033e]
mul r4, r3, #127.0 | mov r6, r2
ph_wrap r5, r3, r4
mad r6, r5, #-0.007874015748031496
addxy r0, r6, #0.99212598425196852 | subxy r7, r6, r2
mul r1, r0, #63.5 | addxy r2, r6, #1.0078740157480315
addxy r3, r1, #128.5 | mul r0, r2, #63.5
mul r4, r3, #6.103515625e-05 | addxy r1, r0, #128.5
hset h7, r4
mul r3, r1, #6.103515625e-05 | addxy r2, r7, #0.0078740157480314821
ldb5h7 r5, 0
hset h7, r3
mov r0, r0
ldb5h7 r4, 0
subxy r6, r5, r4
mac_p1wc3 r0, r6, r2, #0.015748031496062992, r5 | ld r2, [c056]
mul r1, r0, #63.5
st r1, [033c] | ld r1, [a137]
ld r0, [033c]
mad r0, r1, r2 | ld r1, [0340] | ld r2, [c057]
mad r0, r1, r2
st r0, [a158]
