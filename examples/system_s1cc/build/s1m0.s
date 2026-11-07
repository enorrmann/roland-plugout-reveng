ld r0, [0393] | ld r2, [0391] | ld r6, [a039]
add r0, #0.6184303127229214:32 | mul r2, r2, #7.31275 | st r6, [a161]
ph_wrap r0, r3, r0 | ld r4, [a061] | ld r6, [c04b]
addxy r2, r2, r0 | st r0, [0392] | ld r0, [a060]
ph_wrap r2, r3, r2 | ld r1, [a120]
st r2, [0390] | ld r2, [a038]
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
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [a1b1] | ld r5, [0315]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a040]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a041]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | st r2, [0300] | ld r2, [a040]
addxy r2, r1, r2 | st r6, [0312] | ld r6, [a041]
addxy r6, r5, r6 | ld r0, [0300] | ld r4, [0312]
ph_wrap r1, r3, r2
ph_wrap r5, r3, r6 | st r1, [0302]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [0314]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a130] | ld r1, [0307]
mad r0, r1, #0.5 | st r5, [a131] | ld r5, [0319]
mad r4, r5, #0.5 | mul r2, r0, #2.0
mul r6, r4, #2.0
ph_wrap r3, r3, r2
ph_wrap r7, r3, r6 | st r3, [0306]
addxy r1, r3, #0.99212598425196852 | addxy r5, r7, #0.99212598425196852 | st r7, [0318]
mul r0, r1, #127.0 | mul r4, r5, #127.0
ph_wrap r2, r3, r0
ph_wrap r6, r3, r4
mov r1, r3 | mov r5, r7
mad r1, r2, #-0.007874015748031496 | mad r5, r6, #-0.007874015748031496
addxy r2, r1, #0.97637795275590555 | addxy r6, r5, #0.97637795275590555
subxy r0, r1, r3 | subxy r4, r5, r7
mul r3, r2, #63.5 | mul r7, r6, #63.5 | st r0, [0308]
addxy r2, r3, #128.5 | addxy r6, r7, #128.5 | st r4, [031a]
mul r3, r2, #6.103515625e-05 | mul r7, r6, #6.103515625e-05
hset h6, r3
hset h7, r7
addxy r0, r1, #1.0078740157480315 | addxy r4, r5, #1.0078740157480315 | ldb5 r2, 0
addxy r3, r1, #0.99212598425196852 | mul r0, r0, #63.5 | ldb5h7 r6, 0
addxy r7, r5, #0.99212598425196852 | mul r3, r3, #63.5
mul r7, r7, #63.5 | addxy r3, r3, #128.5
addxy r7, r7, #128.5 | mul r3, r3, #6.103515625e-05
mul r7, r7, #6.103515625e-05 | mul r4, r4, #63.5
hset h6, r3
hset h7, r7
addxy r0, r0, #128.5 | addxy r4, r4, #128.5 | ldb5 r3, 0
addxy r1, r1, #1.0236220472440944 | addxy r5, r5, #1.0236220472440944 | ldb5h7 r7, 0 | st r3, [0310]
mul r0, r0, #6.103515625e-05 | mul r4, r4, #6.103515625e-05 | st r7, [0322]
mul r1, r1, #63.5 | mul r5, r5, #63.5
hset h6, r0
hset h7, r4
addxy r1, r1, #128.5 | addxy r5, r5, #128.5 | ldb5 r0, 0
mul r1, r1, #6.103515625e-05 | mul r5, r5, #6.103515625e-05 | ldb5h7 r4, 0 | st r0, [030c]
hset h6, r1
hset h7, r5
ldb5 r1, 0 | st r4, [031e]
mov r1, r3 | ldb5h7 r5, 0 | st r1, [030a]
mov r5, r7 | st r5, [031c]
mad r1, r2, #-0.33333333333333331 | mad r5, r6, #-0.33333333333333331
subxy r1, r1, r0 | subxy r5, r5, r4 | ld r0, [030a] | ld r4, [031c]
mad r1, r0, #-0.33333333333333331 | mad r5, r4, #-0.33333333333333331
st r1, [030e] | st r5, [0320]
mov r1, r3 | mov r5, r7 | ld r3, [030c] | ld r7, [031e]
mad r1, r2, #-0.40000000000000002 | mad r5, r6, #-0.40000000000000002
mad r1, r3, #-0.80000000000000004 | mad r5, r7, #-0.80000000000000004
mad r1, r0, #0.20000000000000001 | mad r5, r4, #0.20000000000000001
subxy r0, r3, r2 | subxy r4, r7, r6 | ld r3, [0308] | ld r7, [031a]
addxy r2, r3, #0.0078740157480314821 | addxy r6, r7, #0.0078740157480314821 | ld r3, [030e] | ld r7, [0320]
mac_p1wc3 r3, r3, r2, #0.026246719160104987, r1
mac_p1wc3 r7, r7, r6, #0.026246719160104987, r5
mul r1, r3, r2 | mul r5, r7, r6
shl r3, r1, 11 | ld r1, [0310]
shl r7, r5, 11 | ld r5, [0322]
mad r3, r0, #0.16930167193667719 | mad r7, r4, #0.16930167193667719
mac_p1wc3 r0, r3, r2, #-0.0053323361239898332, r1 | ld r3, [c070]
mac_p1wc3 r4, r7, r6, #-0.0053323361239898332, r5 | ld r7, [c070]
mul r2, r3, r0 | mul r6, r7, r4
mul r1, r2, #-187.53506469726562 | mul r5, r6, #-187.53506469726562 | ld r2, [c056]
st r1, [0304] | st r5, [0316]
ld r0, [0304] | ld r4, [0316] | ld r1, [a130]
mad r0, r1, r2 | ld r5, [a131] | ld r6, [c056]
mad r4, r5, r6 | ld r1, [0390] | ld r2, [c057]
mad r0, r1, r2 | ld r5, [0390] | ld r6, [c057]
mad r4, r5, r6 | ld r2, [a03a]
ld r6, [a03b]
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
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [a1b2] | ld r1, [0327]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [a1b3] | ld r5, [0339]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a042]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a043]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | st r2, [0324] | ld r2, [a042]
addxy r2, r1, r2 | st r6, [0336] | ld r6, [a043]
addxy r6, r5, r6 | ld r0, [0324] | ld r4, [0336]
ph_wrap r1, r3, r2
ph_wrap r5, r3, r6 | st r1, [0326]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [0338]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a132] | ld r1, [032b]
mad r0, r1, #0.5 | st r5, [a133] | ld r5, [033d]
mad r4, r5, #0.5 | mul r2, r0, #2.0
mul r6, r4, #2.0
ph_wrap r3, r3, r2
ph_wrap r7, r3, r6 | st r3, [032a]
addxy r1, r3, #0.99212598425196852 | addxy r5, r7, #0.99212598425196852 | st r7, [033c]
mul r0, r1, #127.0 | mul r4, r5, #127.0
ph_wrap r2, r3, r0
ph_wrap r6, r3, r4
mov r1, r3 | mov r5, r7
mad r1, r2, #-0.007874015748031496 | mad r5, r6, #-0.007874015748031496
addxy r2, r1, #0.97637795275590555 | addxy r6, r5, #0.97637795275590555
subxy r0, r1, r3 | subxy r4, r5, r7
mul r3, r2, #63.5 | mul r7, r6, #63.5 | st r0, [032c]
addxy r2, r3, #128.5 | addxy r6, r7, #128.5 | st r4, [033e]
mul r3, r2, #6.103515625e-05 | mul r7, r6, #6.103515625e-05
hset h6, r3
hset h7, r7
addxy r0, r1, #1.0078740157480315 | addxy r4, r5, #1.0078740157480315 | ldb5 r2, 0
addxy r3, r1, #0.99212598425196852 | mul r0, r0, #63.5 | ldb5h7 r6, 0
addxy r7, r5, #0.99212598425196852 | mul r3, r3, #63.5
mul r7, r7, #63.5 | addxy r3, r3, #128.5
addxy r7, r7, #128.5 | mul r3, r3, #6.103515625e-05
mul r7, r7, #6.103515625e-05 | mul r4, r4, #63.5
hset h6, r3
hset h7, r7
addxy r0, r0, #128.5 | addxy r4, r4, #128.5 | ldb5 r3, 0
addxy r1, r1, #1.0236220472440944 | addxy r5, r5, #1.0236220472440944 | ldb5h7 r7, 0 | st r3, [0334]
mul r0, r0, #6.103515625e-05 | mul r4, r4, #6.103515625e-05 | st r7, [0346]
mul r1, r1, #63.5 | mul r5, r5, #63.5
hset h6, r0
hset h7, r4
addxy r1, r1, #128.5 | addxy r5, r5, #128.5 | ldb5 r0, 0
mul r1, r1, #6.103515625e-05 | mul r5, r5, #6.103515625e-05 | ldb5h7 r4, 0 | st r0, [0330]
hset h6, r1
hset h7, r5
ldb5 r1, 0 | st r4, [0342]
mov r1, r3 | ldb5h7 r5, 0 | st r1, [032e]
mov r5, r7 | st r5, [0340]
mad r1, r2, #-0.33333333333333331 | mad r5, r6, #-0.33333333333333331
subxy r1, r1, r0 | subxy r5, r5, r4 | ld r0, [032e] | ld r4, [0340]
mad r1, r0, #-0.33333333333333331 | mad r5, r4, #-0.33333333333333331
st r1, [0332] | st r5, [0344]
mov r1, r3 | mov r5, r7 | ld r3, [0330] | ld r7, [0342]
mad r1, r2, #-0.40000000000000002 | mad r5, r6, #-0.40000000000000002
mad r1, r3, #-0.80000000000000004 | mad r5, r7, #-0.80000000000000004
mad r1, r0, #0.20000000000000001 | mad r5, r4, #0.20000000000000001
subxy r0, r3, r2 | subxy r4, r7, r6 | ld r3, [032c] | ld r7, [033e]
addxy r2, r3, #0.0078740157480314821 | addxy r6, r7, #0.0078740157480314821 | ld r3, [0332] | ld r7, [0344]
mac_p1wc3 r3, r3, r2, #0.026246719160104987, r1
mac_p1wc3 r7, r7, r6, #0.026246719160104987, r5
mul r1, r3, r2 | mul r5, r7, r6
shl r3, r1, 11 | ld r1, [0334]
shl r7, r5, 11 | ld r5, [0346]
mad r3, r0, #0.16930167193667719 | mad r7, r4, #0.16930167193667719
mac_p1wc3 r0, r3, r2, #-0.0053323361239898332, r1 | ld r3, [c070]
mac_p1wc3 r4, r7, r6, #-0.0053323361239898332, r5 | ld r7, [c070]
mul r2, r3, r0 | mul r6, r7, r4
mul r1, r2, #-187.53506469726562 | mul r5, r6, #-187.53506469726562 | ld r2, [c056]
st r1, [0328] | st r5, [033a]
ld r0, [0328] | ld r4, [033a] | ld r1, [a132]
mad r0, r1, r2 | ld r5, [a133] | ld r6, [c056]
mad r4, r5, r6 | ld r1, [0390] | ld r2, [c057]
mad r0, r1, r2 | ld r5, [0390] | ld r6, [c057]
mad r4, r5, r6 | ld r2, [a03c]
ld r6, [a03d]
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
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [a1b4] | ld r1, [034b]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [a1b5] | ld r5, [035d]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a044]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a045]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | st r2, [0348] | ld r2, [a044]
addxy r2, r1, r2 | st r6, [035a] | ld r6, [a045]
addxy r6, r5, r6 | ld r0, [0348] | ld r4, [035a]
ph_wrap r1, r3, r2
ph_wrap r5, r3, r6 | st r1, [034a]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [035c]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a134] | ld r1, [034f]
mad r0, r1, #0.5 | st r5, [a135] | ld r5, [0361]
mad r4, r5, #0.5 | mul r2, r0, #2.0
mul r6, r4, #2.0
ph_wrap r3, r3, r2
ph_wrap r7, r3, r6 | st r3, [034e]
addxy r1, r3, #0.99212598425196852 | addxy r5, r7, #0.99212598425196852 | st r7, [0360]
mul r0, r1, #127.0 | mul r4, r5, #127.0
ph_wrap r2, r3, r0
ph_wrap r6, r3, r4
mov r1, r3 | mov r5, r7
mad r1, r2, #-0.007874015748031496 | mad r5, r6, #-0.007874015748031496
addxy r2, r1, #0.97637795275590555 | addxy r6, r5, #0.97637795275590555
subxy r0, r1, r3 | subxy r4, r5, r7
mul r3, r2, #63.5 | mul r7, r6, #63.5 | st r0, [0350]
addxy r2, r3, #128.5 | addxy r6, r7, #128.5 | st r4, [0362]
mul r3, r2, #6.103515625e-05 | mul r7, r6, #6.103515625e-05
hset h6, r3
hset h7, r7
addxy r0, r1, #1.0078740157480315 | addxy r4, r5, #1.0078740157480315 | ldb5 r2, 0
addxy r3, r1, #0.99212598425196852 | mul r0, r0, #63.5 | ldb5h7 r6, 0
addxy r7, r5, #0.99212598425196852 | mul r3, r3, #63.5
mul r7, r7, #63.5 | addxy r3, r3, #128.5
addxy r7, r7, #128.5 | mul r3, r3, #6.103515625e-05
mul r7, r7, #6.103515625e-05 | mul r4, r4, #63.5
hset h6, r3
hset h7, r7
addxy r0, r0, #128.5 | addxy r4, r4, #128.5 | ldb5 r3, 0
addxy r1, r1, #1.0236220472440944 | addxy r5, r5, #1.0236220472440944 | ldb5h7 r7, 0 | st r3, [0358]
mul r0, r0, #6.103515625e-05 | mul r4, r4, #6.103515625e-05 | st r7, [036a]
mul r1, r1, #63.5 | mul r5, r5, #63.5
hset h6, r0
hset h7, r4
addxy r1, r1, #128.5 | addxy r5, r5, #128.5 | ldb5 r0, 0
mul r1, r1, #6.103515625e-05 | mul r5, r5, #6.103515625e-05 | ldb5h7 r4, 0 | st r0, [0354]
hset h6, r1
hset h7, r5
ldb5 r1, 0 | st r4, [0366]
mov r1, r3 | ldb5h7 r5, 0 | st r1, [0352]
mov r5, r7 | st r5, [0364]
mad r1, r2, #-0.33333333333333331 | mad r5, r6, #-0.33333333333333331
subxy r1, r1, r0 | subxy r5, r5, r4 | ld r0, [0352] | ld r4, [0364]
mad r1, r0, #-0.33333333333333331 | mad r5, r4, #-0.33333333333333331
st r1, [0356] | st r5, [0368]
mov r1, r3 | mov r5, r7 | ld r3, [0354] | ld r7, [0366]
mad r1, r2, #-0.40000000000000002 | mad r5, r6, #-0.40000000000000002
mad r1, r3, #-0.80000000000000004 | mad r5, r7, #-0.80000000000000004
mad r1, r0, #0.20000000000000001 | mad r5, r4, #0.20000000000000001
subxy r0, r3, r2 | subxy r4, r7, r6 | ld r3, [0350] | ld r7, [0362]
addxy r2, r3, #0.0078740157480314821 | addxy r6, r7, #0.0078740157480314821 | ld r3, [0356] | ld r7, [0368]
mac_p1wc3 r3, r3, r2, #0.026246719160104987, r1
mac_p1wc3 r7, r7, r6, #0.026246719160104987, r5
mul r1, r3, r2 | mul r5, r7, r6
shl r3, r1, 11 | ld r1, [0358]
shl r7, r5, 11 | ld r5, [036a]
mad r3, r0, #0.16930167193667719 | mad r7, r4, #0.16930167193667719
mac_p1wc3 r0, r3, r2, #-0.0053323361239898332, r1 | ld r3, [c070]
mac_p1wc3 r4, r7, r6, #-0.0053323361239898332, r5 | ld r7, [c070]
mul r2, r3, r0 | mul r6, r7, r4
mul r1, r2, #-187.53506469726562 | mul r5, r6, #-187.53506469726562 | ld r2, [c056]
st r1, [034c] | st r5, [035e]
ld r0, [034c] | ld r4, [035e] | ld r1, [a134]
mad r0, r1, r2 | ld r5, [a135] | ld r6, [c056]
mad r4, r5, r6 | ld r1, [0390] | ld r2, [c057]
mad r0, r1, r2 | ld r5, [0390] | ld r6, [c057]
mad r4, r5, r6 | ld r2, [a03e]
ld r6, [a03f]
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
addxy r6, r6, r5 | mul r2, r1, #0.08664339756999316 | st r2, [a1b6] | ld r1, [036f]
mul r6, r5, #0.08664339756999316 | ymacd r0, r2, #0.16666666666666666 | st r6, [a1b7] | ld r5, [0381]
ymacd r4, r6, #0.16666666666666666 | ymacd r0, r2, #0.5
ymacd r4, r6, #0.5 | ymacd r0, r2, #1.0
ymacd r4, r6, #1.0 | ymacd r0, r2, #1.0 | ld r2, [a046]
ymacd r4, r6, #1.0 | mul r0, r0, r0 | ld r6, [a047]
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r0, r0, r0
mul r4, r4, r4 | mul r2, r2, r0
mul r6, r6, r4 | st r2, [036c] | ld r2, [a046]
addxy r2, r1, r2 | st r6, [037e] | ld r6, [a047]
addxy r6, r5, r6 | ld r0, [036c] | ld r4, [037e]
ph_wrap r1, r3, r2
ph_wrap r5, r3, r6 | st r1, [036e]
mul r1, r1, #256.0 | mul r5, r5, #256.0 | st r5, [0380]
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | st r1, [a136] | ld r1, [0373]
mad r0, r1, #0.5 | st r5, [a137] | ld r5, [0385]
mad r4, r5, #0.5 | mul r2, r0, #2.0
mul r6, r4, #2.0
ph_wrap r3, r3, r2
ph_wrap r7, r3, r6 | st r3, [0372]
addxy r1, r3, #0.99212598425196852 | addxy r5, r7, #0.99212598425196852 | st r7, [0384]
mul r0, r1, #127.0 | mul r4, r5, #127.0
ph_wrap r2, r3, r0
ph_wrap r6, r3, r4
mov r1, r3 | mov r5, r7
mad r1, r2, #-0.007874015748031496 | mad r5, r6, #-0.007874015748031496
addxy r2, r1, #0.97637795275590555 | addxy r6, r5, #0.97637795275590555
subxy r0, r1, r3 | subxy r4, r5, r7
mul r3, r2, #63.5 | mul r7, r6, #63.5 | st r0, [0374]
addxy r2, r3, #128.5 | addxy r6, r7, #128.5 | st r4, [0386]
mul r3, r2, #6.103515625e-05 | mul r7, r6, #6.103515625e-05
hset h6, r3
hset h7, r7
addxy r0, r1, #1.0078740157480315 | addxy r4, r5, #1.0078740157480315 | ldb5 r2, 0
addxy r3, r1, #0.99212598425196852 | mul r0, r0, #63.5 | ldb5h7 r6, 0
addxy r7, r5, #0.99212598425196852 | mul r3, r3, #63.5
mul r7, r7, #63.5 | addxy r3, r3, #128.5
addxy r7, r7, #128.5 | mul r3, r3, #6.103515625e-05
mul r7, r7, #6.103515625e-05 | mul r4, r4, #63.5
hset h6, r3
hset h7, r7
addxy r0, r0, #128.5 | addxy r4, r4, #128.5 | ldb5 r3, 0
addxy r1, r1, #1.0236220472440944 | addxy r5, r5, #1.0236220472440944 | ldb5h7 r7, 0 | st r3, [037c]
mul r0, r0, #6.103515625e-05 | mul r4, r4, #6.103515625e-05 | st r7, [038e]
mul r1, r1, #63.5 | mul r5, r5, #63.5
hset h6, r0
hset h7, r4
addxy r1, r1, #128.5 | addxy r5, r5, #128.5 | ldb5 r0, 0
mul r1, r1, #6.103515625e-05 | mul r5, r5, #6.103515625e-05 | ldb5h7 r4, 0 | st r0, [0378]
hset h6, r1
hset h7, r5
ldb5 r1, 0 | st r4, [038a]
mov r1, r3 | ldb5h7 r5, 0 | st r1, [0376]
mov r5, r7 | st r5, [0388]
mad r1, r2, #-0.33333333333333331 | mad r5, r6, #-0.33333333333333331
subxy r1, r1, r0 | subxy r5, r5, r4 | ld r0, [0376] | ld r4, [0388]
mad r1, r0, #-0.33333333333333331 | mad r5, r4, #-0.33333333333333331
st r1, [037a] | st r5, [038c]
mov r1, r3 | mov r5, r7 | ld r3, [0378] | ld r7, [038a]
mad r1, r2, #-0.40000000000000002 | mad r5, r6, #-0.40000000000000002
mad r1, r3, #-0.80000000000000004 | mad r5, r7, #-0.80000000000000004
mad r1, r0, #0.20000000000000001 | mad r5, r4, #0.20000000000000001
subxy r0, r3, r2 | subxy r4, r7, r6 | ld r3, [0374] | ld r7, [0386]
addxy r2, r3, #0.0078740157480314821 | addxy r6, r7, #0.0078740157480314821 | ld r3, [037a] | ld r7, [038c]
mac_p1wc3 r3, r3, r2, #0.026246719160104987, r1
mac_p1wc3 r7, r7, r6, #0.026246719160104987, r5
mul r1, r3, r2 | mul r5, r7, r6
shl r3, r1, 11 | ld r1, [037c]
shl r7, r5, 11 | ld r5, [038e]
mad r3, r0, #0.16930167193667719 | mad r7, r4, #0.16930167193667719
mac_p1wc3 r0, r3, r2, #-0.0053323361239898332, r1 | ld r3, [c070]
mac_p1wc3 r4, r7, r6, #-0.0053323361239898332, r5 | ld r7, [c070]
mul r2, r3, r0 | mul r6, r7, r4
mul r1, r2, #-187.53506469726562 | mul r5, r6, #-187.53506469726562 | ld r2, [c056]
st r1, [0370] | st r5, [0382]
ld r0, [0370] | ld r4, [0382] | ld r1, [a136]
mad r0, r1, r2 | ld r5, [a137] | ld r6, [c056]
mad r4, r5, r6 | ld r1, [0390] | ld r2, [c057]
mad r0, r1, r2 | ld r5, [0390] | ld r6, [c057]
mad r4, r5, r6 | st r0, [a157]
st r4, [a158]
