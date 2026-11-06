mov r7, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r3, [c090]
addxy r7, r7, r0 | ld r0, [0405] | ld r2, [0701] | ld r6, [c000]
addxy r7, r7, r1 | mul r2, r2, #7.31275 | ld r1, [0407] | st r3, [a120]
addxy r7, r7, r0 | ld r0, [0409]
addxy r7, r7, r1 | ld r1, [040b]
addxy r7, r7, r0 | ld r0, [040d]
addxy r7, r7, r1 | ld r1, [040f]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | st r0, [6008] | ld r1, [060d]
mul r7, r7, #0.16666666666666666 | st r0, [6009]
mul r7, r7, r6 | st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [0703]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [0702] | ld r0, [a168]
subxy r1, r1, r0 | st r0, [060c] | ld r0, [c098]
mul r1, r1, r1
ph_wrap r2, r3, r2 | st r1, [061a] | ld r1, [c050]
mul r1, r3, r1 | st r2, [0700] | ld r2, [c051]
mad r1, r0, -r2 | ld r2, [061f] | ld r0, [061d]
add r1, #1.0
st r1, [0616] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0618] | ld r1, [061a]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [061e]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c091]
subxy r2, r0, r2 | st r3, [a121]
mad r0, r2, r1
min r0, r0, #1.0
st r0, [061c] | st r0, [a128]
ld r0, [c080]
st r0, [0600] | ld r0, [a151]
ld r1, [0600]
mul r2, r1, r0 | ld r0, [a169] | ld r1, [062d]
subxy r1, r1, r0 | st r2, [0400] | ld r2, [c051]
mul r1, r1, r1 | st r0, [062c] | ld r0, [c099]
st r1, [063a] | ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r2, [063f] | ld r0, [063d]
add r1, #1.0
st r1, [0636] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0638] | ld r1, [063a]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [063e]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c092]
subxy r2, r0, r2 | st r3, [a122]
mad r0, r2, r1
min r0, r0, #1.0
st r0, [063c] | st r0, [a129]
ld r0, [c081]
st r0, [0620] | ld r0, [a152]
ld r1, [0620]
mul r2, r1, r0 | ld r0, [a16a] | ld r1, [064d]
subxy r1, r1, r0 | st r2, [0402] | ld r2, [c051]
mul r1, r1, r1 | st r0, [064c] | ld r0, [c09a]
st r1, [065a] | ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r2, [065f] | ld r0, [065d]
add r1, #1.0
st r1, [0656] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0658] | ld r1, [065a]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [065e]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c093]
subxy r2, r0, r2 | st r3, [a123]
mad r0, r2, r1
min r0, r0, #1.0
st r0, [065c] | st r0, [a12a]
ld r0, [c082]
st r0, [0640] | ld r0, [a153]
ld r1, [0640]
mul r2, r1, r0 | ld r0, [a16b] | ld r1, [066d]
subxy r1, r1, r0 | st r2, [0404] | ld r2, [c051]
mul r1, r1, r1 | st r0, [066c] | ld r0, [c09b]
st r1, [067a] | ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r2, [067f] | ld r0, [067d]
add r1, #1.0
st r1, [0676] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0678] | ld r1, [067a]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [067e]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c094]
subxy r2, r0, r2 | st r3, [a124]
mad r0, r2, r1
min r0, r0, #1.0
st r0, [067c] | st r0, [a12b]
ld r0, [c083]
st r0, [0660] | ld r0, [a154]
ld r1, [0660]
mul r2, r1, r0 | ld r0, [a16c] | ld r1, [068d]
subxy r1, r1, r0 | st r2, [0406] | ld r2, [c051]
mul r1, r1, r1 | st r0, [068c] | ld r0, [c09c]
st r1, [069a] | ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r2, [069f] | ld r0, [069d]
add r1, #1.0
st r1, [0696] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0698] | ld r1, [069a]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [069e]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c095]
subxy r2, r0, r2 | st r3, [a125]
mad r0, r2, r1
min r0, r0, #1.0
st r0, [069c] | st r0, [a12c]
ld r0, [c084]
st r0, [0680] | ld r0, [a155]
ld r1, [0680]
mul r2, r1, r0 | ld r0, [a16d] | ld r1, [06ad]
subxy r1, r1, r0 | st r2, [0408] | ld r2, [c051]
mul r1, r1, r1 | st r0, [06ac] | ld r0, [c09d]
st r1, [06ba] | ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r2, [06bf] | ld r0, [06bd]
add r1, #1.0
st r1, [06b6] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [06b8] | ld r1, [06ba]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [06be]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c096]
subxy r2, r0, r2 | st r3, [a126]
mad r0, r2, r1
min r0, r0, #1.0
st r0, [06bc] | st r0, [a12d]
ld r0, [c085]
st r0, [06a0] | ld r0, [a156]
ld r1, [06a0]
mul r2, r1, r0 | ld r0, [a16e] | ld r1, [06cd]
subxy r1, r1, r0 | st r2, [040a] | ld r2, [c051]
mul r1, r1, r1 | st r0, [06cc] | ld r0, [c09e]
st r1, [06da] | ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r2, [06df] | ld r0, [06dd]
add r1, #1.0
st r1, [06d6] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [06d8] | ld r1, [06da]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [06de]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [c097]
subxy r2, r0, r2 | st r3, [a127]
mad r0, r2, r1
min r0, r0, #1.0
st r0, [06dc] | st r0, [a12e]
ld r0, [c086]
st r0, [06c0] | ld r0, [a157]
ld r1, [06c0]
mul r2, r1, r0 | ld r0, [a16f] | ld r1, [06ed]
subxy r1, r1, r0 | st r2, [040c] | ld r2, [c051]
mul r1, r1, r1 | st r0, [06ec] | ld r0, [c09f]
st r1, [06fa] | ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r2, [06ff] | ld r0, [06fd]
add r1, #1.0
st r1, [06f6] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [06f8] | ld r1, [06fa]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [06fe]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1
min r0, r0, #1.0
st r0, [06fc] | st r0, [a12f]
ld r0, [c087]
st r0, [06e0] | ld r0, [a158]
ld r1, [06e0]
mul r2, r1, r0
st r2, [040e]
