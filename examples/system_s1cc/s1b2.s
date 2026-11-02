ld r0, [c056]
hset h6, r0
ld r1, [c054]
ld r2, [c055] | ldidx r3, 0 | ldidx r4, 2
ymacd r3, r2, r1 | ldidx r5, 4 | ldidx r6, 6 | ld r0, [c05c]
ymacd r4, r2, r1 | st r3, [a138] | ldidx r3, 8
ymacd r3, r2, r1 | st r4, [a139] | ldidx r4, 10
ymacd r4, r2, r1 | st r3, [a13c]
hset h6, r0
ymacd r5, r2, r1 | st r4, [a13d]
ymacd r6, r2, r1 | ld r1, [c05a] | ldidx r3, 0 | ldidx r4, 2
ld r2, [c05b] | st r5, [a13a] | ldidx r5, 4
ymacd r3, r2, r1 | st r6, [a13b] | ldidx r6, 6
ymacd r4, r2, r1 | st r3, [a140] | ldidx r3, 8
ymacd r5, r2, r1 | st r4, [a141] | ldidx r4, 10
ymacd r6, r2, r1 | st r5, [a142]
ymacd r3, r2, r1 | st r6, [a143]
ymacd r4, r2, r1 | st r3, [a144]
st r4, [a145]
