mov r7, #0.0 | ld r0, [0401] | ld r1, [0403] | ld r6, [c000]
addxy r7, r7, r0 | ld r0, [0405] | ld r2, [06e5]
addxy r7, r7, r1 | mul r2, r2, #7.31275 | ld r1, [0407]
addxy r7, r7, r0 | ld r0, [0409]
addxy r7, r7, r1 | ld r1, [040b]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | st r0, [6008] | ld r1, [060d]
mul r7, r7, #0.2222222222222222 | st r0, [6009]
mul r7, r7, r6 | st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [06e7]
add r0, #0.6184303127229214:32 | st r7, [6006]
ph_wrap r0, r3, r0 | st r7, [6007]
addxy r2, r2, r0 | st r0, [06e6] | ld r0, [a168] | ld r7, [c061]
subxy r1, r1, r0 | st r0, [060c] | ld r0, [060f]
ph_wrap r2, r3, r2
mul r1, r1, r1 | st r2, [06e4] | ld r2, [c048]
mad r0, r0, -r1 | st r1, [061a] | ld r1, [0611]
addxy r2, r0, r2 | ld r3, [06e4]
ph_wrap r0, r3, r2
subxy r2, r0, r2 | subxy r3, r1, r3 | st r0, [060e]
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
mul r1, r3, r1 | st r3, [a120] | st r3, [0510]
mad r1, r0, -r2 | ld r2, [061f] | ld r0, [0521]
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
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [0605]
subxy r2, r0, r2
mad r0, r2, r1 | ld r2, [a168] | ld r1, [0603]
min r0, r0, #1.0 | st r2, [0604]
subxy r3, r3, r2 | st r0, [0520] | st r0, [a128] | ld r0, [0541]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c043]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a160]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0602]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0541]
mad r1, r2, #1.25 | ld r2, [a168]
subxy r1, r0, r1 | st r2, [060a]
mad r0, r1, r3 | ld r3, [060b] | ld r1, [0609]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0540] | ld r0, [0531]
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
mul r1, r1, r0 | ld r0, [0531]
mad r1, r2, #1.25 | ld r2, [0616]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [a130]
min r0, r0, #1.0 | st r1, [0550] | ld r1, [0540]
mul r3, r1, r2 | ld r2, [a1b0] | st r0, [0530] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [0530] | st r3, [0624]
mul r1, r1, r2 | ld r0, [a151]
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
mul r4, r1, #6.103515625e-05 | ld r1, [c063]
mul r2, r7, r4
min r7, r2, #3.3569336665095761e-05 | ld r2, [0621]
shl r4, r7, 14 | ld r7, [0623]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [0620]
mad r3, r0, #0.03125
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [0624]
mul r0, r4, r7 | mul r3, r1, #4.0 | ld r1, [0633] | ld r7, [c061]
mul r2, r0, #-32.0 | ld r0, [a169] | st r3, [0622]
subxy r1, r1, r0 | st r0, [0632] | ld r0, [0635]
mul r1, r1, r1 | st r2, [0400] | ld r2, [c048] | ld r3, [06e4]
mad r0, r0, -r1 | st r1, [0640] | ld r1, [0637]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [0634]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [0639]
subxy r1, r3, r1 | st r1, [0636]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [0638] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [063b] | ld r2, [a161]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [063a]
mul r1, r3, r1 | st r3, [a121] | st r3, [0512]
mad r1, r0, -r2 | ld r2, [0645] | ld r0, [0523]
add r1, #1.0
st r1, [063c] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [063e] | ld r1, [0640]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0644]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [062b]
subxy r2, r0, r2
mad r0, r2, r1 | ld r2, [a169] | ld r1, [0629]
min r0, r0, #1.0 | st r2, [062a]
subxy r3, r3, r2 | st r0, [0522] | st r0, [a129] | ld r0, [0543]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c043]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a161]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0628]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0543]
mad r1, r2, #1.25 | ld r2, [a169]
subxy r1, r0, r1 | st r2, [0630]
mad r0, r1, r3 | ld r3, [0631] | ld r1, [062f]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0542] | ld r0, [0533]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a161]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [062e]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0533]
mad r1, r2, #1.25 | ld r2, [063c]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [a131]
min r0, r0, #1.0 | st r1, [0552] | ld r1, [0542]
mul r3, r1, r2 | ld r2, [a1b1] | st r0, [0532] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [0532] | st r3, [064a]
mul r1, r1, r2 | ld r0, [a152]
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [063e] | ld r7, [c060]
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
mul r4, r1, #6.103515625e-05 | ld r1, [c063]
mul r2, r7, r4
min r7, r2, #3.3569336665095761e-05 | ld r2, [0647]
shl r4, r7, 14 | ld r7, [0649]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [0646]
mad r3, r0, #0.03125
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [064a]
mul r0, r4, r7 | mul r3, r1, #4.0 | ld r1, [0659] | ld r7, [c061]
mul r2, r0, #-32.0 | ld r0, [a16a] | st r3, [0648]
subxy r1, r1, r0 | st r0, [0658] | ld r0, [065b]
mul r1, r1, r1 | st r2, [0402] | ld r2, [c048] | ld r3, [06e4]
mad r0, r0, -r1 | st r1, [0666] | ld r1, [065d]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [065a]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [065f]
subxy r1, r3, r1 | st r1, [065c]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [065e] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0661] | ld r2, [a162]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0660]
mul r1, r3, r1 | st r3, [a122] | st r3, [0514]
mad r1, r0, -r2 | ld r2, [066b] | ld r0, [0525]
add r1, #1.0
st r1, [0662] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [0664] | ld r1, [0666]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [066a]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [0651]
subxy r2, r0, r2
mad r0, r2, r1 | ld r2, [a16a] | ld r1, [064f]
min r0, r0, #1.0 | st r2, [0650]
subxy r3, r3, r2 | st r0, [0524] | st r0, [a12a] | ld r0, [0545]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c043]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a162]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [064e]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0545]
mad r1, r2, #1.25 | ld r2, [a16a]
subxy r1, r0, r1 | st r2, [0656]
mad r0, r1, r3 | ld r3, [0657] | ld r1, [0655]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0544] | ld r0, [0535]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a162]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0654]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0535]
mad r1, r2, #1.25 | ld r2, [0662]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [a132]
min r0, r0, #1.0 | st r1, [0554] | ld r1, [0544]
mul r3, r1, r2 | ld r2, [a1b2] | st r0, [0534] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [0534] | st r3, [0670]
mul r1, r1, r2 | ld r0, [a153]
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [0664] | ld r7, [c060]
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
mul r4, r1, #6.103515625e-05 | ld r1, [c063]
mul r2, r7, r4
min r7, r2, #3.3569336665095761e-05 | ld r2, [066d]
shl r4, r7, 14 | ld r7, [066f]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [066c]
mad r3, r0, #0.03125
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [0670]
mul r0, r4, r7 | mul r3, r1, #4.0 | ld r1, [067f] | ld r7, [c061]
mul r2, r0, #-32.0 | ld r0, [a16b] | st r3, [066e]
subxy r1, r1, r0 | st r0, [067e] | ld r0, [0681]
mul r1, r1, r1 | st r2, [0404] | ld r2, [c048] | ld r3, [06e4]
mad r0, r0, -r1 | st r1, [068c] | ld r1, [0683]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [0680]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [0685]
subxy r1, r3, r1 | st r1, [0682]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [0684] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [0687] | ld r2, [a163]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [0686]
mul r1, r3, r1 | st r3, [a123] | st r3, [0516]
mad r1, r0, -r2 | ld r2, [0691] | ld r0, [0527]
add r1, #1.0
st r1, [0688] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [068a] | ld r1, [068c]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0690]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [0677]
subxy r2, r0, r2
mad r0, r2, r1 | ld r2, [a16b] | ld r1, [0675]
min r0, r0, #1.0 | st r2, [0676]
subxy r3, r3, r2 | st r0, [0526] | st r0, [a12b] | ld r0, [0547]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c043]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a163]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [0674]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0547]
mad r1, r2, #1.25 | ld r2, [a16b]
subxy r1, r0, r1 | st r2, [067c]
mad r0, r1, r3 | ld r3, [067d] | ld r1, [067b]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0546] | ld r0, [0537]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a163]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [067a]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0537]
mad r1, r2, #1.25 | ld r2, [0688]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [a133]
min r0, r0, #1.0 | st r1, [0556] | ld r1, [0546]
mul r3, r1, r2 | ld r2, [a1b3] | st r0, [0536] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [0536] | st r3, [0696]
mul r1, r1, r2 | ld r0, [a154]
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [068a] | ld r7, [c060]
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
mul r4, r1, #6.103515625e-05 | ld r1, [c063]
mul r2, r7, r4
min r7, r2, #3.3569336665095761e-05 | ld r2, [0693]
shl r4, r7, 14 | ld r7, [0695]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [0692]
mad r3, r0, #0.03125
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [0696]
mul r0, r4, r7 | mul r3, r1, #4.0 | ld r1, [06a5] | ld r7, [c061]
mul r2, r0, #-32.0 | ld r0, [a16c] | st r3, [0694]
subxy r1, r1, r0 | st r0, [06a4] | ld r0, [06a7]
mul r1, r1, r1 | st r2, [0406] | ld r2, [c048] | ld r3, [06e4]
mad r0, r0, -r1 | st r1, [06b2] | ld r1, [06a9]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [06a6]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [06ab]
subxy r1, r3, r1 | st r1, [06a8]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [06aa] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [06ad] | ld r2, [a164]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [06ac]
mul r1, r3, r1 | st r3, [a124] | st r3, [0518]
mad r1, r0, -r2 | ld r2, [06b7] | ld r0, [0529]
add r1, #1.0
st r1, [06ae] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [06b0] | ld r1, [06b2]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [06b6]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [069d]
subxy r2, r0, r2
mad r0, r2, r1 | ld r2, [a16c] | ld r1, [069b]
min r0, r0, #1.0 | st r2, [069c]
subxy r3, r3, r2 | st r0, [0528] | st r0, [a12c] | ld r0, [0549]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c043]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a164]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [069a]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [0549]
mad r1, r2, #1.25 | ld r2, [a16c]
subxy r1, r0, r1 | st r2, [06a2]
mad r0, r1, r3 | ld r3, [06a3] | ld r1, [06a1]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [0548] | ld r0, [0539]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a164]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [06a0]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [0539]
mad r1, r2, #1.25 | ld r2, [06ae]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [a134]
min r0, r0, #1.0 | st r1, [0558] | ld r1, [0548]
mul r3, r1, r2 | ld r2, [a1b4] | st r0, [0538] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [0538] | st r3, [06bc]
mul r1, r1, r2 | ld r0, [a155]
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [06b0] | ld r7, [c060]
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
mul r4, r1, #6.103515625e-05 | ld r1, [c063]
mul r2, r7, r4
min r7, r2, #3.3569336665095761e-05 | ld r2, [06b9]
shl r4, r7, 14 | ld r7, [06bb]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [06b8]
mad r3, r0, #0.03125
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [06bc]
mul r0, r4, r7 | mul r3, r1, #4.0 | ld r1, [06cb] | ld r7, [c061]
mul r2, r0, #-32.0 | ld r0, [a16d] | st r3, [06ba]
subxy r1, r1, r0 | st r0, [06ca] | ld r0, [06cd]
mul r1, r1, r1 | st r2, [0408] | ld r2, [c048] | ld r3, [06e4]
mad r0, r0, -r1 | st r1, [06d8] | ld r1, [06cf]
addxy r2, r0, r2 | subxy r3, r1, r3
ph_wrap r0, r3, r2
subxy r2, r0, r2 | st r0, [06cc]
mul r3, r3, r2 | ld r2, [c049]
mad r1, r3, #0.5 | ld r3, [06d1]
subxy r1, r3, r1 | st r1, [06ce]
mad r3, r1, r2 | ld r1, [c04d]
mul r3, r3, r1 | st r3, [06d0] | ld r1, [c04c]
mul r2, r0, r1 | ld r1, [c04b]
ph_clamp r2, r3, -r2
ph_tri r0, r3, -r0
addxy r3, r3, r2 | mul r2, r0, r0
mul r2, r2, r1 | ld r1, [c04a]
addxy r2, r2, r1 | ld r1, [c04e]
mad r3, r0, r2 | ld r0, [06d3] | ld r2, [a165]
addxy r0, r0, r1 | ld r1, [c050]
mul r0, r0, r2 | ld r2, [c051]
min r0, r0, #1.0
mul r3, r3, r0 | st r0, [06d2]
mul r1, r3, r1 | st r3, [a125] | st r3, [051a]
mad r1, r0, -r2 | ld r2, [06dd] | ld r0, [052b]
add r1, #1.0
st r1, [06d4] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [06d6] | ld r1, [06d8]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [06dc]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25 | ld r3, [06c3]
subxy r2, r0, r2
mad r0, r2, r1 | ld r2, [a16d] | ld r1, [06c1]
min r0, r0, #1.0 | st r2, [06c2]
subxy r3, r3, r2 | st r0, [052a] | st r0, [a12d] | ld r0, [054b]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c043]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a165]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [06c0]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c041]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c042]
mad r3, r1, r0 | ld r0, [c040]
mul r1, r1, r0 | ld r0, [054b]
mad r1, r2, #1.25 | ld r2, [a16d]
subxy r1, r0, r1 | st r2, [06c8]
mad r0, r1, r3 | ld r3, [06c9] | ld r1, [06c7]
min r0, r0, #1.0
subxy r3, r3, r2 | st r0, [054a] | ld r0, [053b]
addxy r2, r0, #-0.999 | mul r3, r3, r3 | ld r0, [c047]
mul r2, r2, #256.0 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
mul r2, r2, #256.0 | mul r1, r1, r3 | ld r3, [a165]
ph_clamp r2, r3, -r2
max r1, r1, r2
mul r1, r1, r3
subxy r2, r1, r3 | subxy r3, r3, #1.0 | st r1, [06c6]
mul r2, r2, #256.0 | mul r3, r3, r0 | ld r0, [c045]
ph_clamp r2, r3, -r2
mad r3, r2, r0 | ld r0, [c046]
mad r3, r1, r0 | ld r0, [c044]
mul r1, r1, r0 | ld r0, [053b]
mad r1, r2, #1.25 | ld r2, [06d4]
subxy r1, r0, r1
mad r0, r1, r3 | ld r1, [a135]
min r0, r0, #1.0 | st r1, [055a] | ld r1, [054a]
mul r3, r1, r2 | ld r2, [a1b5] | st r0, [053a] | ld r1, [c062]
addxy r2, r2, #-4.75 | ld r4, [053a] | st r3, [06e2]
mul r1, r1, r2 | ld r0, [a156]
mac_p1wc3 r2, r4, r7, #0.125, r1 | ld r4, [06d6] | ld r7, [c060]
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
mul r4, r1, #6.103515625e-05 | ld r1, [c063]
mul r2, r7, r4
min r7, r2, #3.3569336665095761e-05 | ld r2, [06df]
shl r4, r7, 14 | ld r7, [06e1]
mul r3, r4, r7
addxy r3, r3, r2
mov r2, r3
mad r2, r0, #-0.0625
mac_p1wc3 r2, r1, r7, #0.5, r2
mac_p1wc3 r2, r4, r2, #-0.5, r7
mac_p1wc3 r7, r4, r2, #-0.5, r3
mul r3, r7, #-2.0
mov r3, r7 | st r3, [06de]
mad r3, r0, #0.03125
mac_p1wc3 r0, r1, r2, #0.5, r3
mac_p1wc3 r1, r4, r0, #-0.5, r2 | ld r4, [06e2]
mul r0, r4, r7 | mul r3, r1, #4.0
mul r2, r0, #-32.0 | st r3, [06e0]
st r2, [040a]
