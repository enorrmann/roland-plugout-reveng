mov r7, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r3, [c090]
addxy r7, r7, r0 | ld r0, [0405] | ld r2, [0711] | ld r6, [c000]
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
st r0, [600d] | ld r0, [0713]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [0712] | ld r0, [a168] | ld r7, [c061]
subxy r1, r1, r0 | st r0, [060c] | ld r0, [c098]
mul r1, r1, r1
ph_wrap r2, r3, r2 | ld r1, [c050]
mul r1, r3, r1 | st r2, [0710] | ld r2, [c051]
mad r1, r0, -r2 | ld r0, [c080]
add r1, #1.0 | st r0, [0600] | ld r0, [c088]
st r1, [0616] | ld r1, [c04f]
mul r1, r3, r1 | ld r2, [0616] | st r0, [0606]
st r1, [0618] | ld r1, [0600]
mul r3, r1, r2 | ld r2, [a1b0] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [0606] | ld r0, [a151]
mul r1, r1, r2
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [0618] | ld r7, [c060]
mad r2, r4, #0.125 | mov r4, #0.041666666666666664
mul r1, r2, #0.17328679513998632
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, r1
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, #6.103515625e-05
mul r2, r7, r4 | ld r4, [c063]
min r1, r2, #3.0517578125e-05
shl r7, r1, 15 | ld r1, [061b]
ph_clamp r2, r3, -r7
mac_p1wc3 r7, r4, r1, #-0.125, r0
mul r0, r7, #-4.0
ph_clamp r7, r3, -r0
mul r0, r7, r7
mul r0, r7, r0
mad r7, r0, #-0.3333333432674408 | ld r0, [061d]
subxy r7, r0, r7
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [061f]
subxy r7, r0, r7 | st r7, [061c]
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [0621]
subxy r7, r0, r7 | st r7, [061e]
mul r7, r2, r7
addxy r7, r7, r0
subxy r0, r1, r7 | st r7, [0620]
mul r7, r2, r0 | addxy r0, r4, #0.5
addxy r2, r7, r1
mul r7, r0, r2 | ld r0, [a169] | st r2, [061a] | ld r2, [c051]
mul r1, r3, r7 | ld r3, [c091] | st r0, [062e]
mul r4, r1, #4.0 | ld r1, [062f] | st r3, [a121] | ld r7, [c061]
subxy r1, r1, r0 | ld r0, [c099] | st r4, [0400]
mul r1, r1, r1
ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c081]
add r1, #1.0 | st r0, [0622] | ld r0, [c089]
st r1, [0638] | ld r1, [c04f]
mul r1, r3, r1 | ld r2, [0638] | st r0, [0628]
st r1, [063a] | ld r1, [0622]
mul r3, r1, r2 | ld r2, [a1b1] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [0628] | ld r0, [a152]
mul r1, r1, r2
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [063a] | ld r7, [c060]
mad r2, r4, #0.125 | mov r4, #0.041666666666666664
mul r1, r2, #0.17328679513998632
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, r1
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, #6.103515625e-05
mul r2, r7, r4 | ld r4, [c063]
min r1, r2, #3.0517578125e-05
shl r7, r1, 15 | ld r1, [063d]
ph_clamp r2, r3, -r7
mac_p1wc3 r7, r4, r1, #-0.125, r0
mul r0, r7, #-4.0
ph_clamp r7, r3, -r0
mul r0, r7, r7
mul r0, r7, r0
mad r7, r0, #-0.3333333432674408 | ld r0, [063f]
subxy r7, r0, r7
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [0641]
subxy r7, r0, r7 | st r7, [063e]
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [0643]
subxy r7, r0, r7 | st r7, [0640]
mul r7, r2, r7
addxy r7, r7, r0
subxy r0, r1, r7 | st r7, [0642]
mul r7, r2, r0 | addxy r0, r4, #0.5
addxy r2, r7, r1
mul r7, r0, r2 | ld r0, [a16a] | st r2, [063c] | ld r2, [c051]
mul r1, r3, r7 | ld r3, [c092] | st r0, [0650]
mul r4, r1, #4.0 | ld r1, [0651] | st r3, [a122] | ld r7, [c061]
subxy r1, r1, r0 | ld r0, [c09a] | st r4, [0402]
mul r1, r1, r1
ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c082]
add r1, #1.0 | st r0, [0644] | ld r0, [c08a]
st r1, [065a] | ld r1, [c04f]
mul r1, r3, r1 | ld r2, [065a] | st r0, [064a]
st r1, [065c] | ld r1, [0644]
mul r3, r1, r2 | ld r2, [a1b2] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [064a] | ld r0, [a153]
mul r1, r1, r2
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [065c] | ld r7, [c060]
mad r2, r4, #0.125 | mov r4, #0.041666666666666664
mul r1, r2, #0.17328679513998632
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, r1
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, #6.103515625e-05
mul r2, r7, r4 | ld r4, [c063]
min r1, r2, #3.0517578125e-05
shl r7, r1, 15 | ld r1, [065f]
ph_clamp r2, r3, -r7
mac_p1wc3 r7, r4, r1, #-0.125, r0
mul r0, r7, #-4.0
ph_clamp r7, r3, -r0
mul r0, r7, r7
mul r0, r7, r0
mad r7, r0, #-0.3333333432674408 | ld r0, [0661]
subxy r7, r0, r7
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [0663]
subxy r7, r0, r7 | st r7, [0660]
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [0665]
subxy r7, r0, r7 | st r7, [0662]
mul r7, r2, r7
addxy r7, r7, r0
subxy r0, r1, r7 | st r7, [0664]
mul r7, r2, r0 | addxy r0, r4, #0.5
addxy r2, r7, r1
mul r7, r0, r2 | ld r0, [a16b] | st r2, [065e] | ld r2, [c051]
mul r1, r3, r7 | ld r3, [c093] | st r0, [0672]
mul r4, r1, #4.0 | ld r1, [0673] | st r3, [a123] | ld r7, [c061]
subxy r1, r1, r0 | ld r0, [c09b] | st r4, [0404]
mul r1, r1, r1
ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c083]
add r1, #1.0 | st r0, [0666] | ld r0, [c08b]
st r1, [067c] | ld r1, [c04f]
mul r1, r3, r1 | ld r2, [067c] | st r0, [066c]
st r1, [067e] | ld r1, [0666]
mul r3, r1, r2 | ld r2, [a1b3] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [066c] | ld r0, [a154]
mul r1, r1, r2
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [067e] | ld r7, [c060]
mad r2, r4, #0.125 | mov r4, #0.041666666666666664
mul r1, r2, #0.17328679513998632
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, r1
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, #6.103515625e-05
mul r2, r7, r4 | ld r4, [c063]
min r1, r2, #3.0517578125e-05
shl r7, r1, 15 | ld r1, [0681]
ph_clamp r2, r3, -r7
mac_p1wc3 r7, r4, r1, #-0.125, r0
mul r0, r7, #-4.0
ph_clamp r7, r3, -r0
mul r0, r7, r7
mul r0, r7, r0
mad r7, r0, #-0.3333333432674408 | ld r0, [0683]
subxy r7, r0, r7
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [0685]
subxy r7, r0, r7 | st r7, [0682]
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [0687]
subxy r7, r0, r7 | st r7, [0684]
mul r7, r2, r7
addxy r7, r7, r0
subxy r0, r1, r7 | st r7, [0686]
mul r7, r2, r0 | addxy r0, r4, #0.5
addxy r2, r7, r1
mul r7, r0, r2 | ld r0, [a16c] | st r2, [0680] | ld r2, [c051]
mul r1, r3, r7 | ld r3, [c094] | st r0, [0694]
mul r4, r1, #4.0 | ld r1, [0695] | st r3, [a124] | ld r7, [c061]
subxy r1, r1, r0 | ld r0, [c09c] | st r4, [0406]
mul r1, r1, r1
ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c084]
add r1, #1.0 | st r0, [0688] | ld r0, [c08c]
st r1, [069e] | ld r1, [c04f]
mul r1, r3, r1 | ld r2, [069e] | st r0, [068e]
st r1, [06a0] | ld r1, [0688]
mul r3, r1, r2 | ld r2, [a1b4] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [068e] | ld r0, [a155]
mul r1, r1, r2
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [06a0] | ld r7, [c060]
mad r2, r4, #0.125 | mov r4, #0.041666666666666664
mul r1, r2, #0.17328679513998632
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, r1
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, #6.103515625e-05
mul r2, r7, r4 | ld r4, [c063]
min r1, r2, #3.0517578125e-05
shl r7, r1, 15 | ld r1, [06a3]
ph_clamp r2, r3, -r7
mac_p1wc3 r7, r4, r1, #-0.125, r0
mul r0, r7, #-4.0
ph_clamp r7, r3, -r0
mul r0, r7, r7
mul r0, r7, r0
mad r7, r0, #-0.3333333432674408 | ld r0, [06a5]
subxy r7, r0, r7
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [06a7]
subxy r7, r0, r7 | st r7, [06a4]
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [06a9]
subxy r7, r0, r7 | st r7, [06a6]
mul r7, r2, r7
addxy r7, r7, r0
subxy r0, r1, r7 | st r7, [06a8]
mul r7, r2, r0 | addxy r0, r4, #0.5
addxy r2, r7, r1
mul r7, r0, r2 | ld r0, [a16d] | st r2, [06a2] | ld r2, [c051]
mul r1, r3, r7 | ld r3, [c095] | st r0, [06b6]
mul r4, r1, #4.0 | ld r1, [06b7] | st r3, [a125] | ld r7, [c061]
subxy r1, r1, r0 | ld r0, [c09d] | st r4, [0408]
mul r1, r1, r1
ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c085]
add r1, #1.0 | st r0, [06aa] | ld r0, [c08d]
st r1, [06c0] | ld r1, [c04f]
mul r1, r3, r1 | ld r2, [06c0] | st r0, [06b0]
st r1, [06c2] | ld r1, [06aa]
mul r3, r1, r2 | ld r2, [a1b5] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [06b0] | ld r0, [a156]
mul r1, r1, r2
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [06c2] | ld r7, [c060]
mad r2, r4, #0.125 | mov r4, #0.041666666666666664
mul r1, r2, #0.17328679513998632
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, r1
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, #6.103515625e-05
mul r2, r7, r4 | ld r4, [c063]
min r1, r2, #3.0517578125e-05
shl r7, r1, 15 | ld r1, [06c5]
ph_clamp r2, r3, -r7
mac_p1wc3 r7, r4, r1, #-0.125, r0
mul r0, r7, #-4.0
ph_clamp r7, r3, -r0
mul r0, r7, r7
mul r0, r7, r0
mad r7, r0, #-0.3333333432674408 | ld r0, [06c7]
subxy r7, r0, r7
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [06c9]
subxy r7, r0, r7 | st r7, [06c6]
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [06cb]
subxy r7, r0, r7 | st r7, [06c8]
mul r7, r2, r7
addxy r7, r7, r0
subxy r0, r1, r7 | st r7, [06ca]
mul r7, r2, r0 | addxy r0, r4, #0.5
addxy r2, r7, r1
mul r7, r0, r2 | ld r0, [a16e] | st r2, [06c4] | ld r2, [c051]
mul r1, r3, r7 | ld r3, [c096] | st r0, [06d8]
mul r4, r1, #4.0 | ld r1, [06d9] | st r3, [a126] | ld r7, [c061]
subxy r1, r1, r0 | ld r0, [c09e] | st r4, [040a]
mul r1, r1, r1
ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c086]
add r1, #1.0 | st r0, [06cc] | ld r0, [c08e]
st r1, [06e2] | ld r1, [c04f]
mul r1, r3, r1 | ld r2, [06e2] | st r0, [06d2]
st r1, [06e4] | ld r1, [06cc]
mul r3, r1, r2 | ld r2, [a1b6] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [06d2] | ld r0, [a157]
mul r1, r1, r2
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [06e4] | ld r7, [c060]
mad r2, r4, #0.125 | mov r4, #0.041666666666666664
mul r1, r2, #0.17328679513998632
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, r1
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, #6.103515625e-05
mul r2, r7, r4 | ld r4, [c063]
min r1, r2, #3.0517578125e-05
shl r7, r1, 15 | ld r1, [06e7]
ph_clamp r2, r3, -r7
mac_p1wc3 r7, r4, r1, #-0.125, r0
mul r0, r7, #-4.0
ph_clamp r7, r3, -r0
mul r0, r7, r7
mul r0, r7, r0
mad r7, r0, #-0.3333333432674408 | ld r0, [06e9]
subxy r7, r0, r7
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [06eb]
subxy r7, r0, r7 | st r7, [06e8]
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [06ed]
subxy r7, r0, r7 | st r7, [06ea]
mul r7, r2, r7
addxy r7, r7, r0
subxy r0, r1, r7 | st r7, [06ec]
mul r7, r2, r0 | addxy r0, r4, #0.5
addxy r2, r7, r1
mul r7, r0, r2 | ld r0, [a16f] | st r2, [06e6] | ld r2, [c051]
mul r1, r3, r7 | ld r3, [c097] | st r0, [06fa]
mul r4, r1, #4.0 | ld r1, [06fb] | st r3, [a127] | ld r7, [c061]
subxy r1, r1, r0 | ld r0, [c09f] | st r4, [040c]
mul r1, r1, r1
ld r1, [c050]
mul r1, r3, r1
mad r1, r0, -r2 | ld r0, [c087]
add r1, #1.0 | st r0, [06ee] | ld r0, [c08f]
st r1, [0704] | ld r1, [c04f]
mul r1, r3, r1 | ld r2, [0704] | st r0, [06f4]
st r1, [0706] | ld r1, [06ee]
mul r3, r1, r2 | ld r2, [a1b7] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [06f4] | ld r0, [a158]
mul r1, r1, r2
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [0706] | ld r7, [c060]
mad r2, r4, #0.125 | mov r4, #0.041666666666666664
mul r1, r2, #0.17328679513998632
ymacd r4, r1, #0.16666666666666666
ymacd r4, r1, #0.5
ymacd r4, r1, #1.0
ymacd r4, r1, #1.0
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, r1
mul r2, r4, r4
mul r1, r2, r2
mul r4, r1, #6.103515625e-05
mul r2, r7, r4 | ld r4, [c063]
min r1, r2, #3.0517578125e-05
shl r7, r1, 15 | ld r1, [0709]
ph_clamp r2, r3, -r7
mac_p1wc3 r7, r4, r1, #-0.125, r0
mul r0, r7, #-4.0
ph_clamp r7, r3, -r0
mul r0, r7, r7
mul r0, r7, r0
mad r7, r0, #-0.3333333432674408 | ld r0, [070b]
subxy r7, r0, r7
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [070d]
subxy r7, r0, r7 | st r7, [070a]
mul r7, r2, r7
addxy r7, r7, r0 | ld r0, [070f]
subxy r7, r0, r7 | st r7, [070c]
mul r7, r2, r7
addxy r7, r7, r0
subxy r0, r1, r7 | st r7, [070e]
mul r7, r2, r0 | addxy r0, r4, #0.5
addxy r2, r7, r1
mul r7, r0, r2 | st r2, [0708]
mul r1, r3, r7
mul r4, r1, #4.0
st r4, [040e]
