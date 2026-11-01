mov r7, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r6, [c000]
addxy r7, r7, r0 | ld r0, [0405] | ld r2, [06d1]
addxy r7, r7, r1 | mul r2, r2, #7.31275 | ld r1, [0407]
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
st r0, [600d] | ld r0, [06d3]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [06d2] | ld r0, [a168]
subxy r1, r1, r0 | st r0, [060c] | ld r0, [060f]
ph_wrap r2, r3, r2
mul r1, r1, r1 | st r2, [06d0] | ld r2, [c048]
mad r0, r0, -r1 | ld r1, [0611] | ld r3, [06d0]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [060e]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [0613]
subxy r1, r3, r1 | st r1, [0610]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [0612] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0615] | ld r2, [a160]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0614]
mul r1, r3, r1 | st r3, [a120]
mad r1, r0, -r2 | ld r2, [a168] | ld r0, [0601]
add r1, #1.0 | st r2, [0604]
st r1, [0616] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [0605]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [0618] | ld r1, [0603] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a160]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0602]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0601]
mad r1, r2, #1.25 | ld r2, [0616]
subxy r1, r0, r1
mad r0, r1, r3
min r0, r0, #1.0
st r0, [0600] | ld r0, [a151]
ld r1, [0600]
mul r3, r1, r2 | ld r1, [0627] | ld r2, [c048]
mul r4, r3, r0 | ld r0, [a169] | ld r3, [06d0]
subxy r1, r1, r0 | st r0, [0626] | ld r0, [0629]
mul r1, r1, r1 | st r4, [0400]
mad r0, r0, -r1 | ld r1, [062b]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [0628]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [062d]
subxy r1, r3, r1 | st r1, [062a]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [062c] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [062f] | ld r2, [a161]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [062e]
mul r1, r3, r1 | st r3, [a121]
mad r1, r0, -r2 | ld r2, [a169] | ld r0, [061b]
add r1, #1.0 | st r2, [061e]
st r1, [0630] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [061f]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [0632] | ld r1, [061d] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a161]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [061c]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [061b]
mad r1, r2, #1.25 | ld r2, [0630]
subxy r1, r0, r1
mad r0, r1, r3
min r0, r0, #1.0
st r0, [061a] | ld r0, [a152]
ld r1, [061a]
mul r3, r1, r2 | ld r1, [0641] | ld r2, [c048]
mul r4, r3, r0 | ld r0, [a16a] | ld r3, [06d0]
subxy r1, r1, r0 | st r0, [0640] | ld r0, [0643]
mul r1, r1, r1 | st r4, [0402]
mad r0, r0, -r1 | ld r1, [0645]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [0642]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [0647]
subxy r1, r3, r1 | st r1, [0644]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [0646] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0649] | ld r2, [a162]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0648]
mul r1, r3, r1 | st r3, [a122]
mad r1, r0, -r2 | ld r2, [a16a] | ld r0, [0635]
add r1, #1.0 | st r2, [0638]
st r1, [064a] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [0639]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [064c] | ld r1, [0637] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a162]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0636]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0635]
mad r1, r2, #1.25 | ld r2, [064a]
subxy r1, r0, r1
mad r0, r1, r3
min r0, r0, #1.0
st r0, [0634] | ld r0, [a153]
ld r1, [0634]
mul r3, r1, r2 | ld r1, [065b] | ld r2, [c048]
mul r4, r3, r0 | ld r0, [a16b] | ld r3, [06d0]
subxy r1, r1, r0 | st r0, [065a] | ld r0, [065d]
mul r1, r1, r1 | st r4, [0404]
mad r0, r0, -r1 | ld r1, [065f]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [065c]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [0661]
subxy r1, r3, r1 | st r1, [065e]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [0660] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0663] | ld r2, [a163]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0662]
mul r1, r3, r1 | st r3, [a123]
mad r1, r0, -r2 | ld r2, [a16b] | ld r0, [064f]
add r1, #1.0 | st r2, [0652]
st r1, [0664] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [0653]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [0666] | ld r1, [0651] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a163]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0650]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [064f]
mad r1, r2, #1.25 | ld r2, [0664]
subxy r1, r0, r1
mad r0, r1, r3
min r0, r0, #1.0
st r0, [064e] | ld r0, [a154]
ld r1, [064e]
mul r3, r1, r2 | ld r1, [0675] | ld r2, [c048]
mul r4, r3, r0 | ld r0, [a16c] | ld r3, [06d0]
subxy r1, r1, r0 | st r0, [0674] | ld r0, [0677]
mul r1, r1, r1 | st r4, [0406]
mad r0, r0, -r1 | ld r1, [0679]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [0676]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [067b]
subxy r1, r3, r1 | st r1, [0678]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [067a] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [067d] | ld r2, [a164]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [067c]
mul r1, r3, r1 | st r3, [a124]
mad r1, r0, -r2 | ld r2, [a16c] | ld r0, [0669]
add r1, #1.0 | st r2, [066c]
st r1, [067e] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [066d]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [0680] | ld r1, [066b] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a164]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [066a]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0669]
mad r1, r2, #1.25 | ld r2, [067e]
subxy r1, r0, r1
mad r0, r1, r3
min r0, r0, #1.0
st r0, [0668] | ld r0, [a155]
ld r1, [0668]
mul r3, r1, r2 | ld r1, [068f] | ld r2, [c048]
mul r4, r3, r0 | ld r0, [a16d] | ld r3, [06d0]
subxy r1, r1, r0 | st r0, [068e] | ld r0, [0691]
mul r1, r1, r1 | st r4, [0408]
mad r0, r0, -r1 | ld r1, [0693]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [0690]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [0695]
subxy r1, r3, r1 | st r1, [0692]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [0694] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0697] | ld r2, [a165]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0696]
mul r1, r3, r1 | st r3, [a125]
mad r1, r0, -r2 | ld r2, [a16d] | ld r0, [0683]
add r1, #1.0 | st r2, [0686]
st r1, [0698] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [0687]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [069a] | ld r1, [0685] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a165]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0684]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0683]
mad r1, r2, #1.25 | ld r2, [0698]
subxy r1, r0, r1
mad r0, r1, r3
min r0, r0, #1.0
st r0, [0682] | ld r0, [a156]
ld r1, [0682]
mul r3, r1, r2 | ld r1, [06a9] | ld r2, [c048]
mul r4, r3, r0 | ld r0, [a16e] | ld r3, [06d0]
subxy r1, r1, r0 | st r0, [06a8] | ld r0, [06ab]
mul r1, r1, r1 | st r4, [040a]
mad r0, r0, -r1 | ld r1, [06ad]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [06aa]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [06af]
subxy r1, r3, r1 | st r1, [06ac]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [06ae] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [06b1] | ld r2, [a166]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [06b0]
mul r1, r3, r1 | st r3, [a126]
mad r1, r0, -r2 | ld r2, [a16e] | ld r0, [069d]
add r1, #1.0 | st r2, [06a0]
st r1, [06b2] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [06a1]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [06b4] | ld r1, [069f] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a166]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [069e]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [069d]
mad r1, r2, #1.25 | ld r2, [06b2]
subxy r1, r0, r1
mad r0, r1, r3
min r0, r0, #1.0
st r0, [069c] | ld r0, [a157]
ld r1, [069c]
mul r3, r1, r2 | ld r1, [06c3] | ld r2, [c048]
mul r4, r3, r0 | ld r0, [a16f] | ld r3, [06d0]
subxy r1, r1, r0 | st r0, [06c2] | ld r0, [06c5]
mul r1, r1, r1 | st r4, [040c]
mad r0, r0, -r1 | ld r1, [06c7]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [06c4]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [06c9]
subxy r1, r3, r1 | st r1, [06c6]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [06c8] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [06cb] | ld r2, [a167]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [06ca]
mul r1, r3, r1 | st r3, [a127]
mad r1, r0, -r2 | ld r2, [a16f] | ld r0, [06b7]
add r1, #1.0 | st r2, [06ba]
st r1, [06cc] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [06bb]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [06ce] | ld r1, [06b9] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a167]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [06b8]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [06b7]
mad r1, r2, #1.25 | ld r2, [06cc]
subxy r1, r0, r1
mad r0, r1, r3
min r0, r0, #1.0
st r0, [06b6] | ld r0, [a158]
ld r1, [06b6]
mul r3, r1, r2
mul r4, r3, r0
st r4, [040e]
