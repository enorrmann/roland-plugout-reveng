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
mul r1, r3, r1 | mul r5, r7, r5 | st r3, [a126]
mad r1, r0, -r2 | mad r5, r4, -r6 | st r7, [a127] | ld r0, [0715]
add r1, #1.0 | add r5, #1.0 | ld r2, [a16e] | ld r4, [0743]
st r1, [072a] | st r5, [0758]
ld r1, [c04f] | ld r6, [a16f] | st r2, [0718]
mul r1, r3, r1 | ld r5, [c04f] | ld r3, [0719] | st r6, [0746]
mul r5, r7, r5 | subxy r3, r3, r2 | ld r7, [0747] | st r1, [072c]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r5, [075a] | ld r1, [0717] | ld r0, [c043]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r5, [0745] | ld r4, [c043]
mul r6, r6, #256.0 | mul r3, r3, r3
mul r7, r7, r7
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r3, #1.0 | subxy r7, r7, #1.0
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r1, r1, r3 | mul r5, r5, r7 | ld r3, [a166]
ph_clamp r2, r3, -r2 | ld r7, [a167]
ph_clamp r6, r3, -r6
max r1, r1, r2
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
mul r1, r1, r0 | ld r4, [c040] | ld r0, [0715]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [0743] | ld r2, [a16e]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [a16f] | st r2, [071e]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r3, [071f] | st r6, [074c]
mad r4, r5, r7 | subxy r3, r3, r2 | ld r7, [074d] | ld r1, [071d]
min r0, r0, #1.0 | ld r5, [074b]
min r4, r4, #1.0 | st r0, [0714] | ld r0, [071b]
subxy r7, r7, r6 | addxy r2, r0, #-0.999 | st r4, [0742] | ld r4, [0749] | ld r0, [c047]
addxy r6, r4, #-0.999 | mul r2, r2, #256.0 | ld r4, [c047]
mul r6, r6, #256.0 | mul r3, r3, r3
mul r7, r7, r7
ph_clamp r2, r3, -r2
ph_clamp r6, r3, -r6
subxy r3, r3, #1.0 | subxy r7, r7, #1.0
mul r2, r2, #256.0 | mul r6, r6, #256.0
mul r1, r1, r3 | mul r5, r5, r7 | ld r3, [a166]
ph_clamp r2, r3, -r2 | ld r7, [a167]
ph_clamp r6, r3, -r6
max r1, r1, r2
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
mul r1, r1, r0 | ld r4, [c044] | ld r0, [071b]
mul r5, r5, r4 | mad r1, r2, #1.25 | ld r4, [0749] | ld r2, [c054]
mad r5, r6, #1.25 | subxy r1, r0, r1 | ld r6, [c054]
subxy r5, r4, r5 | mad r0, r1, r3 | ld r1, [a126]
mad r4, r5, r7 | ld r5, [a127] | ld r3, [c055]
min r0, r0, #1.0 | ld r7, [c055]
min r4, r4, #1.0 | st r0, [071a] | ld r0, [a157]
mad r2, r1, r3 | mad r6, r5, r7 | ld r1, [0730] | ld r5, [075e] | ld r3, [c056]
mad r2, r1, r3 | ld r7, [c056] | st r4, [0748] | ld r1, [071a]
mad r6, r5, r7 | ld r5, [0748] | ld r3, [c057] | st r0, [073a]
mad r2, r1, r3 | ld r7, [c057] | ld r1, [0714]
mad r6, r5, r7 | ld r5, [0742] | ld r3, [c058]
mad r2, r1, r3 | ld r7, [c058] | ld r1, [a136]
mad r6, r5, r7 | ld r5, [a137] | ld r3, [c059]
mad r2, r1, r3 | ld r7, [c059] | ld r1, [a126]
mad r6, r5, r7 | st r2, [a13e] | ld r2, [c05a]
st r6, [a13f] | ld r6, [c05a]
ld r5, [a127] | ld r3, [c05b]
mad r2, r1, r3 | ld r7, [c05b] | ld r1, [0730]
mad r6, r5, r7 | ld r5, [075e] | ld r3, [c05c]
mad r2, r1, r3 | ld r7, [c05c] | ld r1, [071a]
mad r6, r5, r7 | ld r5, [0748] | ld r3, [c05d]
mad r2, r1, r3 | ld r7, [c05d] | ld r1, [0714]
mad r6, r5, r7 | ld r5, [0742] | ld r3, [c05e]
mad r2, r1, r3 | ld r7, [c05e] | ld r1, [a136]
mad r6, r5, r7 | ld r5, [a137] | ld r3, [c05f]
mad r2, r1, r3 | ld r7, [c05f] | ld r4, [a158]
mad r6, r5, r7 | st r2, [a146] | ld r1, [0714] | st r4, [0768]
st r6, [a147] | ld r5, [0742]
ld r2, [072a] | ld r6, [0758] | ld r0, [a1b6]
mul r3, r1, r2 | mul r7, r5, r6 | ld r4, [a1b7] | ld r1, [071a] | ld r5, [0748]
st r3, [0738] | st r7, [0766]
addxy r0, r0, #-4.75 | addxy r4, r4, #-4.75 | ld r3, [c062]
mul r3, r3, r0 | ld r7, [c062]
mul r7, r7, r4 | ld r2, [c061]
mul r0, r1, r2 | ld r6, [c061] | ld r1, [072c]
mul r4, r5, r6 | mad r0, r3, #0.125 | ld r5, [075a] | ld r2, [c060]
mad r4, r7, #0.125 | mad r0, r1, #0.125 | ld r6, [c060]
mad r4, r5, #0.125 | mul r3, r0, #0.17328679513998632
mul r7, r4, #0.17328679513998632 | mov r1, #0.041666666666666664
mov r5, #0.041666666666666664 | ymacd r1, r3, #0.16666666666666666
ymacd r5, r7, #0.16666666666666666 | ymacd r1, r3, #0.5
ymacd r5, r7, #0.5 | ymacd r1, r3, #1.0
ymacd r5, r7, #1.0 | ymacd r1, r3, #1.0
ymacd r5, r7, #1.0 | mul r0, r1, r1
mul r4, r5, r5 | mul r3, r0, r0
mul r7, r4, r4 | mul r1, r3, r3
mul r5, r7, r7 | mul r0, r1, r1
mul r4, r5, r5 | mul r3, r0, r0
mul r7, r4, r4 | mul r1, r3, #6.103515625e-05 | ld r3, [c063]
mul r5, r7, #6.103515625e-05 | mul r0, r2, r1 | ld r7, [c063] | st r3, [073c]
mul r4, r6, r5 | st r7, [076a]
min r2, r0, #3.3569336665095761e-05
min r6, r4, #3.3569336665095761e-05
mul r1, r2, #256.0 | mul r5, r6, #256.0 | ld r2, [0735] | ld r6, [0763]
mul r0, r1, #64.0 | mul r4, r5, #64.0 | ld r1, [0737] | ld r5, [0765]
mul r3, r0, r1 | mul r7, r4, r5 | st r0, [0740]
addxy r3, r3, r2 | addxy r7, r7, r6 | st r4, [076e]
mov r2, r3 | mov r6, r7 | st r3, [073e] | ld r3, [073a]
mad r2, r3, #-0.0625 | st r7, [076c] | ld r7, [0768]
mad r6, r7, #-0.0625 | ld r3, [073c] | ld r7, [076a]
mul r0, r3, r1 | mul r4, r7, r5
mad r0, r2, #0.5 | mad r4, r6, #0.5 | ld r2, [0740] | ld r6, [076e]
mul r0, r2, r0 | mul r4, r6, r4
mad r0, r1, #-0.5 | mad r4, r5, #-0.5
mul r1, r2, r0 | mul r5, r6, r4 | ld r2, [073e] | ld r6, [076c]
mad r1, r2, #-0.5 | mad r5, r6, #-0.5
mul r2, r1, #-2.0 | mul r6, r5, #-2.0 | st r1, [073e]
st r2, [0734] | st r6, [0762]
mov r2, r1 | mov r6, r5 | st r5, [076c] | ld r1, [073a]
mad r2, r1, #0.03125 | mul r1, r3, r0 | ld r5, [0768] | ld r3, [0740]
mad r6, r5, #0.03125 | mul r5, r7, r4 | ld r7, [076e]
mad r1, r2, #0.5 | mad r5, r6, #0.5
mul r2, r3, r1 | mul r6, r7, r5 | ld r1, [0738] | ld r5, [0766]
mad r2, r0, #-0.5 | mad r6, r4, #-0.5 | ld r0, [073e] | ld r4, [076c]
mul r3, r2, #4.0 | mul r7, r6, #4.0
mul r2, r1, r0 | mul r6, r5, r4 | st r3, [0736]
mul r3, r2, #-32.0 | mul r7, r6, #-32.0 | st r7, [0764]
st r3, [040c] | st r7, [040e]
