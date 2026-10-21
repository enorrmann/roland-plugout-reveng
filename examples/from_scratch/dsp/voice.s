; mono voice, DSP5 DCO1 slot. Own asm: see make_voice.py
mov r5, #0.0:32
mov r6, #1.0:32
mov r7, #1.0:32
ld r3, [06a1]
mov r4, r4
addxy r3, r3, #0.00011458333333333333:32
addxy r2, r3, #-1.0:32
max r2, r2, r5
mul r2, r2, #256.0:32
min r2, r2, r6
mul r2, r2, #256.0:32
min r2, r2, r6
mul r2, r2, #256.0:32
min r2, r2, r6
mul r2, r2, #256.0:32
min r2, r2, r6
mul r2, r2, #2.0:32
subxy r3, r2, r3
st r3, [06a0]
ld r0, [a021]
mov r4, r4
mad r0, r3, #0.02:32
add r0, #-4.75:32
mul r2, r0, #0.04332169878499658:32
mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666:32
ymacd r3, r2, #0.5:32
ymacd r3, r2, #1.0:32
ymacd r3, r2, #1.0:32
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
ld r1, [0671]
mov r4, r4
mul r3, r3, #0.009166666666666667:32
addxy r1, r3, r1
addxy r2, r1, #-1.0:32
max r2, r2, r5
mul r2, r2, #256.0:32
min r2, r2, r6
mul r2, r2, #256.0:32
min r2, r2, r6
mul r2, r2, #256.0:32
min r2, r2, r6
mul r2, r2, #256.0:32
min r2, r2, r6
mul r2, r2, #2.0:32
subxy r1, r2, r1
st r1, [0670]
mul r1, r1, r7
st r1, [6020]
st r1, [6021]
st r1, [6000]
st r1, [6001]
st r1, [6002]
st r1, [6003]
st r1, [6004]
st r1, [6005]
st r1, [6006]
st r1, [6007]
st r1, [6008]
st r1, [6009]
st r1, [600a]
st r1, [600b]
st r1, [600c]
st r1, [600d]
st r1, [600e]
st r1, [600f]
st r1, [6010]
st r1, [6011]
st r1, [6012]
st r1, [6013]
st r1, [6014]
st r1, [6015]
st r1, [6016]
st r1, [6017]
st r1, [6018]
st r1, [6019]
st r1, [601a]
st r1, [601b]
st r1, [601c]
st r1, [601d]
st r1, [601e]
st r1, [601f]
