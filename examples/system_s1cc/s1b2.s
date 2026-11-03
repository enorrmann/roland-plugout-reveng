ld r0, [a16e] | ld r1, [0721] | ld r5, [074f]
subxy r1, r1, r0 | ld r4, [a16f] | st r0, [0720] | ld r2, [c048]
subxy r5, r5, r4 | mul r1, r1, r1 | st r4, [074e] | ld r0, [0723] | ld r6, [c048]
mul r5, r5, r5 | mad r0, r0, -r1 | ld r4, [0751] | st r1, [072e]
mad r4, r4, -r5 | addxy r2, r0, r2 | st r5, [075c] | ld r1, [0725]
addxy r6, r4, r6 | ld r5, [0753] | ld r3, [0770]
ph_wrap r0, r3, r2 | ld r7, [0770]
ph_wrap r4, r3, r6 | st r0, [0722]
subxy r2, r0, r2 | subxy r6, r4, r6 | st r4, [0750]
subxy r3, r1, r3 | subxy r7, r5, r7
mul r3, r3, r2 | mul r7, r7, r6 | ld r2, [c049]
mad r1, r3, #0.5 | mad r5, r7, #0.5 | ld r3, [0727] | ld r7, [0755] | ld r6, [c049]
st r1, [0724] | st r5, [0752]
subxy r1, r3, r1 | subxy r5, r7, r5
mad r3, r1, r2 | mad r7, r5, r6 | ld r1, [c04d]
mul r3, r3, r1 | ld r5, [c04d] | st r3, [0726]
mul r7, r7, r5 | st r7, [0754] | ld r1, [c04c]
mul r2, r0, r1 | ph_tri r0, r3, -r0 | ld r5, [c04c]
mul r6, r4, r5 | ld r1, [c04b]
ph_clamp r2, r3, -r2 | ld r5, [c04b]
ph_clamp r6, r3, -r6
ph_tri r4, r3, -r4
addxy r3, r3, r2 | addxy r7, r7, r6
mul r2, r0, r0 | mul r6, r4, r4
mul r2, r2, r1 | mul r6, r6, r5 | ld r1, [c04a]
addxy r2, r2, r1 | ld r5, [c04a]
addxy r6, r6, r5 | mad r3, r0, r2 | ld r0, [0729] | ld r1, [c04e]
mad r7, r4, r6 | addxy r0, r0, r1 | ld r4, [0757] | ld r5, [c04e]
addxy r4, r4, r5 | ld r2, [a166] | ld r1, [c050]
mul r0, r0, r2 | ld r6, [a167] | ld r5, [c050]
mul r4, r4, r6 | ld r2, [c051]
min r0, r0, #1.0 | ld r6, [c051]
min r4, r4, #1.0 | st r0, [0728]
mul r3, r3, r0 | mul r7, r7, r4 | st r4, [0756]
mul r1, r3, r1 | mul r5, r7, r5 | st r3, [a126] | st r3, [051c]
mad r1, r0, -r2 | mad r5, r4, -r6 | ld r2, [0733] | ld r6, [0761]
add r1, #1.0 | add r5, #1.0 | ld r0, [052d] | ld r4, [052f]
st r1, [072a] | st r5, [0758]
ld r1, [c04f] | st r7, [a127]
mul r1, r3, r1 | ld r5, [c04f] | st r7, [051e]
mul r5, r7, r5 | st r1, [072c] | ld r1, [072e] | ld r3, [c053]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | st r5, [075a] | ld r5, [075c] | ld r7, [c053]
mad r6, r6, -r5 | addxy r5, r4, #-0.999
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5
mul r1, r1, #256.0 | mul r5, r5, #256.0
ph_clamp r1, r3, -r1
ph_clamp r5, r3, -r5 | max r2, r2, r1 | ld r1, [c052]
max r6, r6, r5 | st r2, [0732] | ld r5, [c052]
subxy r2, r2, #1.0 | subxy r6, r6, #1.0 | st r6, [0760]
mul r2, r2, #256.0 | mul r6, r6, #256.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r1, r3 | subxy r7, r5, r7
mad r1, r2, r3 | mad r5, r6, r7 | ld r3, [0719] | ld r7, [0747]
mul r2, r2, #1.25 | mul r6, r6, #1.25
subxy r2, r0, r2 | subxy r6, r4, r6
mad r0, r2, r1 | mad r4, r6, r5 | ld r2, [a16e] | ld r1, [0717] | ld r5, [0745]
min r0, r0, #1.0 | ld r6, [a16f] | st r2, [0718]
min r4, r4, #1.0 | st r0, [052c] | st r0, [a12e] | ld r0, [054d]
subxy r3, r3, r2 | subxy r7, r7, r6 | st r4, [052e] | st r4, [a12f] | ld r4, [054f]
addxy r2, r0, #-0.999 | addxy r6, r4, #-0.999 | st r6, [0746] | ld r0, [c043]
mul r2, r2, #256.0 | mul r6, r6, #256.0 | ld r4, [c043]
mul r3, r3, r3 | mul r7, r7, r7
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r3, #1.0 | subxy r7, r7, #1.0
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r1, r1, r3 | mul r5, r5, r7 | ld r3, [a166]
ph_clamp r2, r3, -r2 | ld r7, [a167]
ph_clamp r6, r3, -r6 | max r1, r1, r2
max r5, r5, r6
mul r1, r1, r3 | mul r5, r5, r7
subxy r2, r1, r3 | subxy r6, r5, r7 | st r1, [0716]
subxy r3, r3, #1.0 | subxy r7, r7, #1.0 | st r5, [0744]
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r3, r3, r0 | mul r7, r7, r4 | ld r0, [c041]
ph_clamp r2, r3, -r2 | ld r4, [c041]
ph_clamp r6, r3, -r6
mad r3, r2, r0 | mad r7, r6, r4 | ld r0, [c042]
mad r3, r1, r0 | ld r4, [c042]
mad r7, r5, r4 | ld r0, [c040]
mul r1, r1, r0 | ld r4, [c040] | ld r0, [054d]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [054f] | ld r2, [a16e]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [a16f] | st r2, [071e]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r3, [071f] | st r6, [074c]
mad r4, r5, r7 | subxy r3, r3, r2 | ld r7, [074d] | ld r1, [071d]
min r0, r0, #1.0 | ld r5, [074b]
min r4, r4, #1.0 | st r0, [054c] | ld r0, [053d]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r4, [054e] | ld r4, [053f] | ld r0, [c047]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r4, [c047]
mul r6, r6, #256.0 | mul r3, r3, r3
mul r7, r7, r7 | subxy r3, r3, #1.0
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r7, r7, #1.0 | mul r2, r2, #256.0
mul r6, r6, #256.0 | mul r1, r1, r3 | ld r3, [a166]
mul r5, r5, r7 | ld r7, [a167]
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6 | max r1, r1, r2
max r5, r5, r6
mul r1, r1, r3 | mul r5, r5, r7
subxy r2, r1, r3 | subxy r6, r5, r7 | st r1, [071c]
subxy r3, r3, #1.0 | subxy r7, r7, #1.0 | st r5, [074a]
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r3, r3, r0 | mul r7, r7, r4 | ld r0, [c045]
ph_clamp r2, r3, -r2 | ld r4, [c045]
ph_clamp r6, r3, -r6
mad r3, r2, r0 | mad r7, r6, r4 | ld r0, [c046]
mad r3, r1, r0 | ld r4, [c046]
mad r7, r5, r4 | ld r0, [c044]
mul r1, r1, r0 | ld r4, [c044] | ld r0, [053d]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [053f] | ld r2, [072a]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [0758]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r1, [a136]
mad r4, r5, r7 | ld r5, [a137] | st r1, [055c]
min r0, r0, #1.0 | st r5, [055e] | ld r1, [054c]
min r4, r4, #1.0 | st r0, [053c] | ld r0, [a157]
mul r3, r1, r2 | st r4, [053e] | ld r4, [a158] | ld r2, [c061]
ld r5, [054e] | st r0, [073a] | st r4, [0768]
mul r7, r5, r6 | ld r0, [a1b6] | st r3, [0738] | ld r3, [c062]
addxy r0, r0, #-4.75 | ld r4, [a1b7] | st r7, [0766] | ld r7, [c062]
addxy r4, r4, #-4.75 | mul r3, r3, r0 | ld r1, [053c] | ld r5, [053e] | ld r6, [c061]
mul r7, r7, r4
mac_p1wc3 r0, r1, r2, #0.125, r3 | ld r1, [072c] | ld r2, [c060]
mac_p1wc3 r4, r5, r6, #0.125, r7 | ld r5, [075a] | ld r6, [c060]
mad r0, r1, #0.125 | mad r4, r5, #0.125
mul r3, r0, #0.17328679513998632 | mul r7, r4, #0.17328679513998632
mov r1, #0.041666666666666664 | mov r5, #0.041666666666666664
ymacd r1, r3, #0.16666666666666666 | ymacd r5, r7, #0.16666666666666666
ymacd r1, r3, #0.5 | ymacd r5, r7, #0.5
ymacd r1, r3, #1.0 | ymacd r5, r7, #1.0
ymacd r1, r3, #1.0 | ymacd r5, r7, #1.0
mul r0, r1, r1 | mul r4, r5, r5
mul r3, r0, r0 | mul r7, r4, r4
mul r1, r3, r3 | mul r5, r7, r7
mul r0, r1, r1 | mul r4, r5, r5
mul r3, r0, r0 | mul r7, r4, r4
mul r1, r3, #6.103515625e-05 | mul r5, r7, #6.103515625e-05 | ld r3, [c063]
mul r0, r2, r1 | mul r4, r6, r5 | ld r7, [c063] | st r3, [073c]
min r2, r0, #3.3569336665095761e-05 | st r7, [076a] | ld r0, [0735]
min r6, r4, #3.3569336665095761e-05 | shl r1, r2, 14 | ld r2, [0737] | ld r4, [0763]
shl r5, r6, 14 | ld r6, [0765]
mul r3, r1, r2 | mul r7, r5, r6
addxy r3, r3, r0 | addxy r7, r7, r4
mov r0, r3 | mov r4, r7 | st r3, [073e] | ld r3, [073a]
mad r0, r3, #-0.0625 | st r7, [076c] | ld r7, [0768]
mad r4, r7, #-0.0625 | ld r3, [073c] | ld r7, [076a]
mac_p1wc3 r0, r3, r2, #0.5, r0
mac_p1wc3 r4, r7, r6, #0.5, r4
mac_p1wc3 r0, r1, r0, #-0.5, r2 | ld r2, [073e]
mac_p1wc3 r4, r5, r4, #-0.5, r6 | ld r6, [076c] | st r1, [073e]
mac_p1wc3 r2, r1, r0, #-0.5, r2 | st r5, [076c]
mac_p1wc3 r6, r5, r4, #-0.5, r6 | st r2, [0740]
mul r1, r2, #-2.0 | mul r5, r6, #-2.0 | st r6, [076e]
st r1, [0734] | st r5, [0762]
mov r1, r2 | mov r5, r6 | ld r2, [073a] | ld r6, [0768]
mad r1, r2, #0.03125 | mad r5, r6, #0.03125
mac_p1wc3 r2, r3, r0, #0.5, r1 | ld r3, [073e]
mac_p1wc3 r6, r7, r4, #0.5, r5 | ld r7, [076c]
mac_p1wc3 r1, r3, r2, #-0.5, r0 | ld r2, [0738] | ld r0, [0740]
mac_p1wc3 r5, r7, r6, #-0.5, r4 | ld r6, [0766] | ld r4, [076e]
mul r3, r1, #4.0 | mul r7, r5, #4.0
mul r1, r2, r0 | mul r5, r6, r4 | st r7, [0764] | ld r0, [c056]
mul r7, r5, #-32.0 | mul r3, r1, #-32.0 | st r3, [0736] | ld r1, [c054]
hset h6, r0
st r7, [040e] | st r3, [040c]
ld r2, [c055] | ldidx r3, 0 | ldidx r4, 2
ymacd r3, r2, r1 | ldidx r5, 4 | ldidx r6, 6 | ld r0, [c05c]
ymacd r4, r2, r1 | st r3, [a138] | ldidx r3, 8
ymacd r5, r2, r1 | st r4, [a139] | ldidx r4, 10
ymacd r6, r2, r1 | st r5, [a13a] | ldidx r5, 12
ymacd r3, r2, r1 | st r6, [a13b] | ldidx r6, 14
ymacd r4, r2, r1 | st r3, [a13c]
ymacd r5, r2, r1 | st r4, [a13d]
ymacd r6, r2, r1 | st r5, [a13e] | ld r1, [c05a]
hset h6, r0
st r6, [a13f] | ld r2, [c05b]
ldidx r3, 0 | ldidx r4, 2
ymacd r3, r2, r1 | ldidx r5, 4 | ldidx r6, 6
ymacd r4, r2, r1 | st r3, [a140] | ldidx r3, 8
ymacd r5, r2, r1 | st r4, [a141] | ldidx r4, 10
ymacd r6, r2, r1 | st r5, [a142] | ldidx r5, 12
ymacd r3, r2, r1 | st r6, [a143] | ldidx r6, 14
ymacd r4, r2, r1 | st r3, [a144]
ymacd r5, r2, r1 | st r4, [a145]
ymacd r6, r2, r1 | st r5, [a146]
st r6, [a147]
