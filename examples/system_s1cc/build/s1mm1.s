ld r0, [0301]
ld r1, [c03f]
ld r2, [0305]
addxy r0, r0, #6.103515625e-05
ld r3, [c038]
mov r7, r7
st r1, [0304]
subxy r2, r1, r2
subxy r3, r0, r3
mov r7, r7
sign r2, r2
sign r3, r3
mul r2, r2, r2
addxy r3, r3, #1.0
mov r7, r7
mul r3, r3, #0.5
mov r7, r7
mul r0, r0, r3
mov r3, #1.0
mov r7, r7
subxy r3, r2, r3
mov r7, r7
mul r0, r0, r3
mov r7, r7
st r0, [0300]
ld r1, [c03a]
ld r2, [c03c]
ld r3, [4041]
mov r7, r7
subxy r1, r0, r1
subxy r2, r0, r2
mov r7, r7
sign r1, r1
sign r2, r2
mov r7, r7
mul r1, r1, r1
mul r2, r2, r2
ld r4, [c03b]
ld r5, [c03d]
mul r1, r1, #-1.0
mul r2, r2, #-1.0
subxy r4, r3, r4
subxy r5, r3, r5
addxy r1, r1, #1.0
addxy r2, r2, #1.0
mov r7, r7
mul r4, r4, r1
mul r5, r5, r2
mov r7, r7
addxy r3, r3, r4
mov r7, r7
addxy r3, r3, r5
mov r7, r7
st r3, [4040]
ld r4, [4043]
mov r7, r7
subxy r4, r3, r4
mov r7, r7
mul r4, r4, #0.125
mov r7, r7
addxy r5, r3, r4
mov r7, r7
st r5, [4044]
addxy r5, r5, r4
mov r7, r7
st r5, [4046]
addxy r5, r5, r4
mov r7, r7
st r5, [4048]
addxy r5, r5, r4
mov r7, r7
st r5, [404a]
addxy r5, r5, r4
mov r7, r7
st r5, [404c]
addxy r5, r5, r4
mov r7, r7
st r5, [404e]
addxy r5, r5, r4
mov r7, r7
st r5, [4050]
ld r2, [c039]
; @voice 0
ld r4, [0309]
ld r5, [c030]
mov r3, #0.125
mov r6, r6
addxy r4, r4, r5
mov r6, r6
subxy r3, r3, r4
mov r6, r6
sign r3, r3
mov r6, r6
mov r5, r3
mov r6, r6
mul r5, r5, r5
mov r6, r6
addxy r3, r3, r5
mov r6, r6
mul r3, r3, #0.0625
mov r6, r6
subxy r4, r3, r4
ld r5, [6008]
ld r6, [0321]
mul r3, r3, #8.0
st r4, [0308]
subxy r5, r6, r5
mov r7, r7
mul r5, r5, r3
mov r7, r7
addxy r6, r6, r5
mov r7, r7
st r6, [0320]
addxy r4, r4, r6
mov r6, r6
subxy r6, r4, r0
mov r5, r5
addxy r6, r6, #-0.0001220703125
mov r5, r5
addxy r6, r6, #-0.015625
mov r5, r5
mov r3, r6
mov r5, r5
sign r3, r3
mov r7, r7
mov r5, r3
mov r7, r7
mul r5, r5, r5
mul r3, r3, #-1.0
mov r7, r7
addxy r3, r3, r5
mov r7, r7
mul r3, r3, r2
mov r5, r5
addxy r6, r6, r3
mov r5, r5
addxy r6, r6, #0.015625
mov r5, r5
st r6, [0340]
st r6, [0341]
ld r4, [4080]
ld r5, [4081]
ld r6, [4082]
mov r3, r3
subxy r5, r4, r5
mov r3, r3
mul r5, r5, r6
mov r3, r3
addxy r1, r4, r5
mov r3, r3
st r1, [600e]
; @voice 1
ld r4, [030d]
ld r5, [c031]
mov r3, #0.125
mov r6, r6
addxy r4, r4, r5
mov r6, r6
subxy r3, r3, r4
mov r6, r6
sign r3, r3
mov r6, r6
mov r5, r3
mov r6, r6
mul r5, r5, r5
mov r6, r6
addxy r3, r3, r5
mov r6, r6
mul r3, r3, #0.0625
mov r6, r6
subxy r4, r3, r4
ld r5, [6009]
ld r6, [0325]
mul r3, r3, #8.0
st r4, [030c]
subxy r5, r6, r5
mov r7, r7
mul r5, r5, r3
mov r7, r7
addxy r6, r6, r5
mov r7, r7
st r6, [0324]
addxy r4, r4, r6
mov r6, r6
subxy r6, r4, r0
mov r5, r5
addxy r6, r6, #-0.0001220703125
mov r5, r5
addxy r6, r6, #-0.015625
mov r5, r5
mov r3, r6
mov r5, r5
sign r3, r3
mov r7, r7
mov r5, r3
mov r7, r7
mul r5, r5, r5
mul r3, r3, #-1.0
mov r7, r7
addxy r3, r3, r5
mov r7, r7
mul r3, r3, r2
mov r5, r5
addxy r6, r6, r3
mov r5, r5
addxy r6, r6, #0.015625
mov r5, r5
st r6, [0344]
st r6, [0345]
ld r4, [4088]
ld r5, [4089]
ld r6, [408a]
mov r3, r3
subxy r5, r4, r5
mov r3, r3
mul r5, r5, r6
mov r3, r3
addxy r1, r4, r5
mov r3, r3
st r1, [600f]
; @voice 2
ld r4, [0311]
ld r5, [c032]
mov r3, #0.125
mov r6, r6
addxy r4, r4, r5
mov r6, r6
subxy r3, r3, r4
mov r6, r6
sign r3, r3
mov r6, r6
mov r5, r3
mov r6, r6
mul r5, r5, r5
mov r6, r6
addxy r3, r3, r5
mov r6, r6
mul r3, r3, #0.0625
mov r6, r6
subxy r4, r3, r4
ld r5, [600a]
ld r6, [0329]
mul r3, r3, #8.0
st r4, [0310]
subxy r5, r6, r5
mov r7, r7
mul r5, r5, r3
mov r7, r7
addxy r6, r6, r5
mov r7, r7
st r6, [0328]
addxy r4, r4, r6
mov r6, r6
subxy r6, r4, r0
mov r5, r5
addxy r6, r6, #-0.0001220703125
mov r5, r5
addxy r6, r6, #-0.015625
mov r5, r5
mov r3, r6
mov r5, r5
sign r3, r3
mov r7, r7
mov r5, r3
mov r7, r7
mul r5, r5, r5
mul r3, r3, #-1.0
mov r7, r7
addxy r3, r3, r5
mov r7, r7
mul r3, r3, r2
mov r5, r5
addxy r6, r6, r3
mov r5, r5
addxy r6, r6, #0.015625
mov r5, r5
st r6, [0348]
st r6, [0349]
ld r4, [4090]
ld r5, [4091]
ld r6, [4092]
mov r3, r3
subxy r5, r4, r5
mov r3, r3
mul r5, r5, r6
mov r3, r3
addxy r1, r4, r5
mov r3, r3
st r1, [6010]
; @voice 3
ld r4, [0315]
ld r5, [c033]
mov r3, #0.125
mov r6, r6
addxy r4, r4, r5
mov r6, r6
subxy r3, r3, r4
mov r6, r6
sign r3, r3
mov r6, r6
mov r5, r3
mov r6, r6
mul r5, r5, r5
mov r6, r6
addxy r3, r3, r5
mov r6, r6
mul r3, r3, #0.0625
mov r6, r6
subxy r4, r3, r4
ld r5, [600b]
ld r6, [032d]
mul r3, r3, #8.0
st r4, [0314]
subxy r5, r6, r5
mov r7, r7
mul r5, r5, r3
mov r7, r7
addxy r6, r6, r5
mov r7, r7
st r6, [032c]
addxy r4, r4, r6
mov r6, r6
subxy r6, r4, r0
mov r5, r5
addxy r6, r6, #-0.0001220703125
mov r5, r5
addxy r6, r6, #-0.015625
mov r5, r5
mov r3, r6
mov r5, r5
sign r3, r3
mov r7, r7
mov r5, r3
mov r7, r7
mul r5, r5, r5
mul r3, r3, #-1.0
mov r7, r7
addxy r3, r3, r5
mov r7, r7
mul r3, r3, r2
mov r5, r5
addxy r6, r6, r3
mov r5, r5
addxy r6, r6, #0.015625
mov r5, r5
st r6, [034c]
st r6, [034d]
ld r4, [4098]
ld r5, [4099]
ld r6, [409a]
mov r3, r3
subxy r5, r4, r5
mov r3, r3
mul r5, r5, r6
mov r3, r3
addxy r1, r4, r5
mov r3, r3
st r1, [6011]
; @voice 4
ld r4, [0319]
ld r5, [c034]
mov r3, #0.125
mov r6, r6
addxy r4, r4, r5
mov r6, r6
subxy r3, r3, r4
mov r6, r6
sign r3, r3
mov r6, r6
mov r5, r3
mov r6, r6
mul r5, r5, r5
mov r6, r6
addxy r3, r3, r5
mov r6, r6
mul r3, r3, #0.0625
mov r6, r6
subxy r4, r3, r4
ld r5, [600c]
ld r6, [0331]
mul r3, r3, #8.0
st r4, [0318]
subxy r5, r6, r5
mov r7, r7
mul r5, r5, r3
mov r7, r7
addxy r6, r6, r5
mov r7, r7
st r6, [0330]
addxy r4, r4, r6
mov r6, r6
subxy r6, r4, r0
mov r5, r5
addxy r6, r6, #-0.0001220703125
mov r5, r5
addxy r6, r6, #-0.015625
mov r5, r5
mov r3, r6
mov r5, r5
sign r3, r3
mov r7, r7
mov r5, r3
mov r7, r7
mul r5, r5, r5
mul r3, r3, #-1.0
mov r7, r7
addxy r3, r3, r5
mov r7, r7
mul r3, r3, r2
mov r5, r5
addxy r6, r6, r3
mov r5, r5
addxy r6, r6, #0.015625
mov r5, r5
st r6, [0350]
st r6, [0351]
ld r4, [40a0]
ld r5, [40a1]
ld r6, [40a2]
mov r3, r3
subxy r5, r4, r5
mov r3, r3
mul r5, r5, r6
mov r3, r3
addxy r1, r4, r5
mov r3, r3
st r1, [6012]
; @voice 5
ld r4, [031d]
ld r5, [c035]
mov r3, #0.125
mov r6, r6
addxy r4, r4, r5
mov r6, r6
subxy r3, r3, r4
mov r6, r6
sign r3, r3
mov r6, r6
mov r5, r3
mov r6, r6
mul r5, r5, r5
mov r6, r6
addxy r3, r3, r5
mov r6, r6
mul r3, r3, #0.0625
mov r6, r6
subxy r4, r3, r4
ld r5, [600d]
ld r6, [0335]
mul r3, r3, #8.0
st r4, [031c]
subxy r5, r6, r5
mov r7, r7
mul r5, r5, r3
mov r7, r7
addxy r6, r6, r5
mov r7, r7
st r6, [0334]
addxy r4, r4, r6
mov r6, r6
subxy r6, r4, r0
mov r5, r5
addxy r6, r6, #-0.0001220703125
mov r5, r5
addxy r6, r6, #-0.015625
mov r5, r5
mov r3, r6
mov r5, r5
sign r3, r3
mov r7, r7
mov r5, r3
mov r7, r7
mul r5, r5, r5
mul r3, r3, #-1.0
mov r7, r7
addxy r3, r3, r5
mov r7, r7
mul r3, r3, r2
mov r5, r5
addxy r6, r6, r3
mov r5, r5
addxy r6, r6, #0.015625
mov r5, r5
st r6, [0354]
st r6, [0355]
ld r4, [40a8]
ld r5, [40a9]
ld r6, [40aa]
mov r3, r3
subxy r5, r4, r5
mov r3, r3
mul r5, r5, r6
mov r3, r3
addxy r1, r4, r5
mov r3, r3
st r1, [6013]
; @voices end
ld r0, [6006]
ld r1, [6007]
ld r7, [c01c]
mov r4, r4
mul r0, r0, r7
mul r1, r1, r7
st r0, [2000]
st r1, [2002]
ld r2, [2001]
ld r3, [2003]
mov r4, r4
st r2, [a118]
st r3, [a119]
mov r2, #0.0
st r2, [a11a]
st r2, [a11b]
st r2, [a11c]
st r2, [a11d]
