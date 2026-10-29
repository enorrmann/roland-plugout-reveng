mov r7, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r6, [c000]
addxy r7, r7, r0 | ld r0, [0405] | ld r2, [06f1]
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
st r0, [600d] | ld r0, [06f3]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [06f2] | ld r0, [a168]
subxy r1, r1, r0 | st r0, [060c] | ld r0, [060f]
ph_wrap r2, r3, r2
mul r1, r1, r1 | st r2, [06f0] | ld r2, [c048]
mad r0, r0, -r1 | ld r1, [0611] | ld r3, [06f0]
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
mad r1, r2, #1.25 | ld r2, [a168]
subxy r1, r0, r1 | st r2, [060a]
mad r0, r1, r3 | ld r3, [060b] | ld r1, [0609]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0600] | ld r0, [0607]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a160]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0608]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0607]
mad r1, r2, #1.25
subxy r1, r0, r1
mad r0, r1, r3 | ld r3, [c061]
min r0, r0, #1.0 | ld r1, [c060]
st r0, [0606] | ld r0, [a151]
ld r2, [0606]
mul r4, r2, r3 | mov r2, #0.041666666666666664
mul r7, r4, #0.34657359027997264
ymacd r2, r7, #0.16666666666666666
ymacd r2, r7, #0.5
ymacd r2, r7, #1.0
ymacd r2, r7, #1.0
mul r3, r2, r2
mul r4, r3, r3
mul r7, r4, r4
mul r2, r7, r7
mul r3, r2, #0.00390625
mul r4, r1, r3 | ld r1, [061b]
min r7, r4, #0.0035156249068677425
mul r2, r7, #256.0 | subxy r3, r1, r0 | ld r0, [061d]
mul r4, r3, r2
addxy r7, r4, r1
subxy r3, r0, r7 | st r7, [061a] | ld r7, [0600]
mul r4, r3, r2 | ld r2, [c048]
addxy r1, r4, r0 | ld r0, [a169]
mul r3, r1, r7 | st r1, [061c] | ld r1, [062b]
subxy r1, r1, r0 | st r0, [062a] | ld r0, [062d]
mul r1, r1, r1 | st r3, [0400] | ld r3, [06f0]
mad r0, r0, -r1 | ld r1, [062f]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [062c]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [0631]
subxy r1, r3, r1 | st r1, [062e]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [0630] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0633] | ld r2, [a161]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0632]
mul r1, r3, r1 | st r3, [a121]
mad r1, r0, -r2 | ld r2, [a169] | ld r0, [061f]
add r1, #1.0 | st r2, [0622]
st r1, [0634] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [0623]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [0636] | ld r1, [0621] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a161]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0620]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [061f]
mad r1, r2, #1.25 | ld r2, [a169]
subxy r1, r0, r1 | st r2, [0628]
mad r0, r1, r3 | ld r3, [0629] | ld r1, [0627]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [061e] | ld r0, [0625]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a161]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0626]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0625]
mad r1, r2, #1.25
subxy r1, r0, r1
mad r0, r1, r3 | ld r3, [c061]
min r0, r0, #1.0 | ld r1, [c060]
st r0, [0624] | ld r0, [a152]
ld r2, [0624]
mul r4, r2, r3 | mov r2, #0.041666666666666664
mul r7, r4, #0.34657359027997264
ymacd r2, r7, #0.16666666666666666
ymacd r2, r7, #0.5
ymacd r2, r7, #1.0
ymacd r2, r7, #1.0
mul r3, r2, r2
mul r4, r3, r3
mul r7, r4, r4
mul r2, r7, r7
mul r3, r2, #0.00390625
mul r4, r1, r3 | ld r1, [0639]
min r7, r4, #0.0035156249068677425
mul r2, r7, #256.0 | subxy r3, r1, r0 | ld r0, [063b]
mul r4, r3, r2
addxy r7, r4, r1
subxy r3, r0, r7 | st r7, [0638] | ld r7, [061e]
mul r4, r3, r2 | ld r2, [c048]
addxy r1, r4, r0 | ld r0, [a16a]
mul r3, r1, r7 | st r1, [063a] | ld r1, [0649]
subxy r1, r1, r0 | st r0, [0648] | ld r0, [064b]
mul r1, r1, r1 | st r3, [0402] | ld r3, [06f0]
mad r0, r0, -r1 | ld r1, [064d]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [064a]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [064f]
subxy r1, r3, r1 | st r1, [064c]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [064e] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0651] | ld r2, [a162]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0650]
mul r1, r3, r1 | st r3, [a122]
mad r1, r0, -r2 | ld r2, [a16a] | ld r0, [063d]
add r1, #1.0 | st r2, [0640]
st r1, [0652] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [0641]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [0654] | ld r1, [063f] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a162]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [063e]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [063d]
mad r1, r2, #1.25 | ld r2, [a16a]
subxy r1, r0, r1 | st r2, [0646]
mad r0, r1, r3 | ld r3, [0647] | ld r1, [0645]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [063c] | ld r0, [0643]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a162]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0644]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0643]
mad r1, r2, #1.25
subxy r1, r0, r1
mad r0, r1, r3 | ld r3, [c061]
min r0, r0, #1.0 | ld r1, [c060]
st r0, [0642] | ld r0, [a153]
ld r2, [0642]
mul r4, r2, r3 | mov r2, #0.041666666666666664
mul r7, r4, #0.34657359027997264
ymacd r2, r7, #0.16666666666666666
ymacd r2, r7, #0.5
ymacd r2, r7, #1.0
ymacd r2, r7, #1.0
mul r3, r2, r2
mul r4, r3, r3
mul r7, r4, r4
mul r2, r7, r7
mul r3, r2, #0.00390625
mul r4, r1, r3 | ld r1, [0657]
min r7, r4, #0.0035156249068677425
mul r2, r7, #256.0 | subxy r3, r1, r0 | ld r0, [0659]
mul r4, r3, r2
addxy r7, r4, r1
subxy r3, r0, r7 | st r7, [0656] | ld r7, [063c]
mul r4, r3, r2 | ld r2, [c048]
addxy r1, r4, r0 | ld r0, [a16b]
mul r3, r1, r7 | st r1, [0658] | ld r1, [0667]
subxy r1, r1, r0 | st r0, [0666] | ld r0, [0669]
mul r1, r1, r1 | st r3, [0404] | ld r3, [06f0]
mad r0, r0, -r1 | ld r1, [066b]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [0668]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [066d]
subxy r1, r3, r1 | st r1, [066a]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [066c] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [066f] | ld r2, [a163]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [066e]
mul r1, r3, r1 | st r3, [a123]
mad r1, r0, -r2 | ld r2, [a16b] | ld r0, [065b]
add r1, #1.0 | st r2, [065e]
st r1, [0670] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [065f]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [0672] | ld r1, [065d] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a163]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [065c]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [065b]
mad r1, r2, #1.25 | ld r2, [a16b]
subxy r1, r0, r1 | st r2, [0664]
mad r0, r1, r3 | ld r3, [0665] | ld r1, [0663]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [065a] | ld r0, [0661]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a163]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0662]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0661]
mad r1, r2, #1.25
subxy r1, r0, r1
mad r0, r1, r3 | ld r3, [c061]
min r0, r0, #1.0 | ld r1, [c060]
st r0, [0660] | ld r0, [a154]
ld r2, [0660]
mul r4, r2, r3 | mov r2, #0.041666666666666664
mul r7, r4, #0.34657359027997264
ymacd r2, r7, #0.16666666666666666
ymacd r2, r7, #0.5
ymacd r2, r7, #1.0
ymacd r2, r7, #1.0
mul r3, r2, r2
mul r4, r3, r3
mul r7, r4, r4
mul r2, r7, r7
mul r3, r2, #0.00390625
mul r4, r1, r3 | ld r1, [0675]
min r7, r4, #0.0035156249068677425
mul r2, r7, #256.0 | subxy r3, r1, r0 | ld r0, [0677]
mul r4, r3, r2
addxy r7, r4, r1
subxy r3, r0, r7 | st r7, [0674] | ld r7, [065a]
mul r4, r3, r2 | ld r2, [c048]
addxy r1, r4, r0 | ld r0, [a16c]
mul r3, r1, r7 | st r1, [0676] | ld r1, [0685]
subxy r1, r1, r0 | st r0, [0684] | ld r0, [0687]
mul r1, r1, r1 | st r3, [0406] | ld r3, [06f0]
mad r0, r0, -r1 | ld r1, [0689]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [0686]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [068b]
subxy r1, r3, r1 | st r1, [0688]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [068a] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [068d] | ld r2, [a164]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [068c]
mul r1, r3, r1 | st r3, [a124]
mad r1, r0, -r2 | ld r2, [a16c] | ld r0, [0679]
add r1, #1.0 | st r2, [067c]
st r1, [068e] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [067d]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [0690] | ld r1, [067b] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a164]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [067a]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0679]
mad r1, r2, #1.25 | ld r2, [a16c]
subxy r1, r0, r1 | st r2, [0682]
mad r0, r1, r3 | ld r3, [0683] | ld r1, [0681]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0678] | ld r0, [067f]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a164]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0680]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [067f]
mad r1, r2, #1.25
subxy r1, r0, r1
mad r0, r1, r3 | ld r3, [c061]
min r0, r0, #1.0 | ld r1, [c060]
st r0, [067e] | ld r0, [a155]
ld r2, [067e]
mul r4, r2, r3 | mov r2, #0.041666666666666664
mul r7, r4, #0.34657359027997264
ymacd r2, r7, #0.16666666666666666
ymacd r2, r7, #0.5
ymacd r2, r7, #1.0
ymacd r2, r7, #1.0
mul r3, r2, r2
mul r4, r3, r3
mul r7, r4, r4
mul r2, r7, r7
mul r3, r2, #0.00390625
mul r4, r1, r3 | ld r1, [0693]
min r7, r4, #0.0035156249068677425
mul r2, r7, #256.0 | subxy r3, r1, r0 | ld r0, [0695]
mul r4, r3, r2
addxy r7, r4, r1
subxy r3, r0, r7 | st r7, [0692] | ld r7, [0678]
mul r4, r3, r2 | ld r2, [c048]
addxy r1, r4, r0 | ld r0, [a16d]
mul r3, r1, r7 | st r1, [0694] | ld r1, [06a3]
subxy r1, r1, r0 | st r0, [06a2] | ld r0, [06a5]
mul r1, r1, r1 | st r3, [0408] | ld r3, [06f0]
mad r0, r0, -r1 | ld r1, [06a7]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [06a4]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [06a9]
subxy r1, r3, r1 | st r1, [06a6]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [06a8] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [06ab] | ld r2, [a165]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [06aa]
mul r1, r3, r1 | st r3, [a125]
mad r1, r0, -r2 | ld r2, [a16d] | ld r0, [0697]
add r1, #1.0 | st r2, [069a]
st r1, [06ac] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [069b]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [06ae] | ld r1, [0699] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a165]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0698]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0697]
mad r1, r2, #1.25 | ld r2, [a16d]
subxy r1, r0, r1 | st r2, [06a0]
mad r0, r1, r3 | ld r3, [06a1] | ld r1, [069f]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0696] | ld r0, [069d]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a165]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [069e]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [069d]
mad r1, r2, #1.25
subxy r1, r0, r1
mad r0, r1, r3 | ld r3, [c061]
min r0, r0, #1.0 | ld r1, [c060]
st r0, [069c] | ld r0, [a156]
ld r2, [069c]
mul r4, r2, r3 | mov r2, #0.041666666666666664
mul r7, r4, #0.34657359027997264
ymacd r2, r7, #0.16666666666666666
ymacd r2, r7, #0.5
ymacd r2, r7, #1.0
ymacd r2, r7, #1.0
mul r3, r2, r2
mul r4, r3, r3
mul r7, r4, r4
mul r2, r7, r7
mul r3, r2, #0.00390625
mul r4, r1, r3 | ld r1, [06b1]
min r7, r4, #0.0035156249068677425
mul r2, r7, #256.0 | subxy r3, r1, r0 | ld r0, [06b3]
mul r4, r3, r2
addxy r7, r4, r1
subxy r3, r0, r7 | st r7, [06b0] | ld r7, [0696]
mul r4, r3, r2 | ld r2, [c048]
addxy r1, r4, r0 | ld r0, [a16e]
mul r3, r1, r7 | st r1, [06b2] | ld r1, [06c1]
subxy r1, r1, r0 | st r0, [06c0] | ld r0, [06c3]
mul r1, r1, r1 | st r3, [040a] | ld r3, [06f0]
mad r0, r0, -r1 | ld r1, [06c5]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [06c2]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [06c7]
subxy r1, r3, r1 | st r1, [06c4]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [06c6] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [06c9] | ld r2, [a166]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [06c8]
mul r1, r3, r1 | st r3, [a126]
mad r1, r0, -r2 | ld r2, [a16e] | ld r0, [06b5]
add r1, #1.0 | st r2, [06b8]
st r1, [06ca] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [06b9]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [06cc] | ld r1, [06b7] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a166]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [06b6]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [06b5]
mad r1, r2, #1.25 | ld r2, [a16e]
subxy r1, r0, r1 | st r2, [06be]
mad r0, r1, r3 | ld r3, [06bf] | ld r1, [06bd]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [06b4] | ld r0, [06bb]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a166]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [06bc]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [06bb]
mad r1, r2, #1.25
subxy r1, r0, r1
mad r0, r1, r3 | ld r3, [c061]
min r0, r0, #1.0 | ld r1, [c060]
st r0, [06ba] | ld r0, [a157]
ld r2, [06ba]
mul r4, r2, r3 | mov r2, #0.041666666666666664
mul r7, r4, #0.34657359027997264
ymacd r2, r7, #0.16666666666666666
ymacd r2, r7, #0.5
ymacd r2, r7, #1.0
ymacd r2, r7, #1.0
mul r3, r2, r2
mul r4, r3, r3
mul r7, r4, r4
mul r2, r7, r7
mul r3, r2, #0.00390625
mul r4, r1, r3 | ld r1, [06cf]
min r7, r4, #0.0035156249068677425
mul r2, r7, #256.0 | subxy r3, r1, r0 | ld r0, [06d1]
mul r4, r3, r2
addxy r7, r4, r1
subxy r3, r0, r7 | st r7, [06ce] | ld r7, [06b4]
mul r4, r3, r2 | ld r2, [c048]
addxy r1, r4, r0 | ld r0, [a16f]
mul r3, r1, r7 | st r1, [06d0] | ld r1, [06df]
subxy r1, r1, r0 | st r0, [06de] | ld r0, [06e1]
mul r1, r1, r1 | st r3, [040c] | ld r3, [06f0]
mad r0, r0, -r1 | ld r1, [06e3]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [06e0]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [06e5]
subxy r1, r3, r1 | st r1, [06e2]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [06e4] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [06e7] | ld r2, [a167]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [06e6]
mul r1, r3, r1 | st r3, [a127]
mad r1, r0, -r2 | ld r2, [a16f] | ld r0, [06d3]
add r1, #1.0 | st r2, [06d6]
st r1, [06e8] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [06d7]
subxy r3, r3, r2 | addxy r2, r0, #-0.999 | st r1, [06ea] | ld r1, [06d5] | ld r0, [c043]
mul r2, r2, #256.0 | mul r3, r3, r3
ph_clamp r2, r3, -r2
subxy r3, r3, #1.0 | mul r2, r2, #256.0
mul r1, r1, r3 | ld r3, [a167]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [06d4]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [06d3]
mad r1, r2, #1.25 | ld r2, [a16f]
subxy r1, r0, r1 | st r2, [06dc]
mad r0, r1, r3 | ld r3, [06dd] | ld r1, [06db]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [06d2] | ld r0, [06d9]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a167]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [06da]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [06d9]
mad r1, r2, #1.25
subxy r1, r0, r1
mad r0, r1, r3 | ld r3, [c061]
min r0, r0, #1.0 | ld r1, [c060]
st r0, [06d8] | ld r0, [a158]
ld r2, [06d8]
mul r4, r2, r3 | mov r2, #0.041666666666666664
mul r7, r4, #0.34657359027997264
ymacd r2, r7, #0.16666666666666666
ymacd r2, r7, #0.5
ymacd r2, r7, #1.0
ymacd r2, r7, #1.0
mul r3, r2, r2
mul r4, r3, r3
mul r7, r4, r4
mul r2, r7, r7
mul r3, r2, #0.00390625
mul r4, r1, r3 | ld r1, [06ed]
min r7, r4, #0.0035156249068677425
mul r2, r7, #256.0 | subxy r3, r1, r0 | ld r0, [06ef]
mul r4, r3, r2
addxy r7, r4, r1
subxy r3, r0, r7 | st r7, [06ec] | ld r7, [06d2]
mul r4, r3, r2
addxy r1, r4, r0
mul r3, r1, r7 | st r1, [06ee]
st r3, [040e]
