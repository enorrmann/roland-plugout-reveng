mov r7, #0.0 | mov r5, #0.0 | ld r0, [040d] | ld r1, [042d] | ld r2, [c060]
addxy r7, r7, r0 | ld r0, [044d] | ld r6, [c000]
addxy r7, r7, r1 | ld r1, [046d]
addxy r7, r7, r0 | ld r0, [048d]
addxy r7, r7, r1 | ld r1, [04ad]
addxy r7, r7, r0 | ld r0, [04cd]
addxy r7, r7, r1 | ld r1, [04ed]
addxy r7, r7, r0 | mov r0, #0.0
addxy r7, r7, r1 | ld r1, [a188] | st r0, [6008]
mul r7, r7, #0.16666666666666666 | mul r3, r1, r2 | st r0, [6009]
mul r7, r7, r6 | mov r6, #1.0 | st r0, [600a]
max r4, r3, #3.5555752281817892e-06 | st r7, [6006]
min r7, r4, #0.0052294920103370468 | st r7, [6007]
addxy r1, r7, #-0.0026165237927826142 | st r0, [600b]
mul r2, r1, #256.0 | mul r4, r1, #-0.9431559201400499 | st r0, [600c]
mul r3, r2, #1.4949473834993656 | addxy r7, r4, #-0.0074955453913659572 | ld r2, [0601] | ld r4, [0603] | st r0, [600d]
ymacd r7, r3, #0.38488110519443103 | ld r0, [a151]
ymacd r7, r3, #0.39044132971623097
mul r1, r2, r7
mad r1, r4, #0.1250038148136845
max r3, r1, #-0.125
min r4, r3, #0.125
mov r3, r2 | mul r1, r4, #7.999755859375
mad r3, r4, #0.5 | st r1, [0602]
mad r3, r0, #-0.06250190740684225
mul r1, r3, r7
min r0, r3, #0.0625 | ld r3, [a168]
mad r1, r2, #-0.50003051897516571 | st r3, [0404]
max r7, r0, #-0.0625 | ld r0, [0405]
mul r4, r1, #-1.9998779315501451 | subxy r0, r0, r3 | ld r1, [0401]
addxy r3, r1, #-0.999 | mul r2, r7, #-15.99951171875 | st r4, [0600] | ld r4, [a160]
mul r3, r3, #256.0 | mul r0, r0, r0 | st r2, [041a] | ld r2, [0403]
mul r3, r3, #256.0 | subxy r0, r0, #1.0 | ld r7, [041a]
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0402]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0413]
mad r4, r3, #1.25 | ld r3, [a168]
subxy r4, r1, r4 | st r3, [0414]
mad r1, r4, r0 | ld r0, [0415] | ld r4, [a160]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [0400] | ld r1, [0411]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0412]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a178]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a180] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [0410] | ld r1, [0407]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [040d]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [0409]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [040b]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0406] | ld r1, [a189]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0408] | ld r2, [c060]
mad r3, r5, r0
subxy r5, r4, r3 | mul r3, r1, r2 | st r3, [040a]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [a152]
max r4, r3, #3.5555752281817892e-06 | st r4, [040c]
min r7, r4, #0.0052294920103370468
addxy r1, r7, #-0.0026165237927826142
mul r2, r1, #256.0 | mul r4, r1, #-0.9431559201400499
mul r3, r2, #1.4949473834993656 | addxy r7, r4, #-0.0074955453913659572 | ld r2, [0621] | ld r4, [0623]
ymacd r7, r3, #0.38488110519443103
ymacd r7, r3, #0.39044132971623097
mul r1, r2, r7
mad r1, r4, #0.1250038148136845
max r3, r1, #-0.125
min r4, r3, #0.125
mov r3, r2 | mul r1, r4, #7.999755859375
mad r3, r4, #0.5 | st r1, [0622]
mad r3, r0, #-0.06250190740684225
mul r1, r3, r7
min r0, r3, #0.0625 | ld r3, [a169]
mad r1, r2, #-0.50003051897516571 | st r3, [0424]
max r7, r0, #-0.0625 | ld r0, [0425]
mul r4, r1, #-1.9998779315501451 | subxy r0, r0, r3 | ld r1, [0421]
addxy r3, r1, #-0.999 | mul r2, r7, #-15.99951171875 | st r4, [0620] | ld r4, [a161]
mul r3, r3, #256.0 | mul r0, r0, r0 | st r2, [043a] | ld r2, [0423]
mul r3, r3, #256.0 | subxy r0, r0, #1.0 | ld r7, [043a]
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0422]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0433]
mad r4, r3, #1.25 | ld r3, [a169]
subxy r4, r1, r4 | st r3, [0434]
mad r1, r4, r0 | ld r0, [0435] | ld r4, [a161]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [0420] | ld r1, [0431]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0432]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a179]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a181] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [0430] | ld r1, [0427]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [042d]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [0429]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [042b]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0426] | ld r1, [a18a]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0428] | ld r2, [c060]
mad r3, r5, r0
subxy r5, r4, r3 | mul r3, r1, r2 | st r3, [042a]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [a153]
max r4, r3, #3.5555752281817892e-06 | st r4, [042c]
min r7, r4, #0.0052294920103370468
addxy r1, r7, #-0.0026165237927826142
mul r2, r1, #256.0 | mul r4, r1, #-0.9431559201400499
mul r3, r2, #1.4949473834993656 | addxy r7, r4, #-0.0074955453913659572 | ld r2, [0641] | ld r4, [0643]
ymacd r7, r3, #0.38488110519443103
ymacd r7, r3, #0.39044132971623097
mul r1, r2, r7
mad r1, r4, #0.1250038148136845
max r3, r1, #-0.125
min r4, r3, #0.125
mov r3, r2 | mul r1, r4, #7.999755859375
mad r3, r4, #0.5 | st r1, [0642]
mad r3, r0, #-0.06250190740684225
mul r1, r3, r7
min r0, r3, #0.0625 | ld r3, [a16a]
mad r1, r2, #-0.50003051897516571 | st r3, [0444]
max r7, r0, #-0.0625 | ld r0, [0445]
mul r4, r1, #-1.9998779315501451 | subxy r0, r0, r3 | ld r1, [0441]
addxy r3, r1, #-0.999 | mul r2, r7, #-15.99951171875 | st r4, [0640] | ld r4, [a162]
mul r3, r3, #256.0 | mul r0, r0, r0 | st r2, [045a] | ld r2, [0443]
mul r3, r3, #256.0 | subxy r0, r0, #1.0 | ld r7, [045a]
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0442]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0453]
mad r4, r3, #1.25 | ld r3, [a16a]
subxy r4, r1, r4 | st r3, [0454]
mad r1, r4, r0 | ld r0, [0455] | ld r4, [a162]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [0440] | ld r1, [0451]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0452]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a17a]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a182] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [0450] | ld r1, [0447]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [044d]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [0449]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [044b]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0446] | ld r1, [a18b]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0448] | ld r2, [c060]
mad r3, r5, r0
subxy r5, r4, r3 | mul r3, r1, r2 | st r3, [044a]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [a154]
max r4, r3, #3.5555752281817892e-06 | st r4, [044c]
min r7, r4, #0.0052294920103370468
addxy r1, r7, #-0.0026165237927826142
mul r2, r1, #256.0 | mul r4, r1, #-0.9431559201400499
mul r3, r2, #1.4949473834993656 | addxy r7, r4, #-0.0074955453913659572 | ld r2, [0661] | ld r4, [0663]
ymacd r7, r3, #0.38488110519443103
ymacd r7, r3, #0.39044132971623097
mul r1, r2, r7
mad r1, r4, #0.1250038148136845
max r3, r1, #-0.125
min r4, r3, #0.125
mov r3, r2 | mul r1, r4, #7.999755859375
mad r3, r4, #0.5 | st r1, [0662]
mad r3, r0, #-0.06250190740684225
mul r1, r3, r7
min r0, r3, #0.0625 | ld r3, [a16b]
mad r1, r2, #-0.50003051897516571 | st r3, [0464]
max r7, r0, #-0.0625 | ld r0, [0465]
mul r4, r1, #-1.9998779315501451 | subxy r0, r0, r3 | ld r1, [0461]
addxy r3, r1, #-0.999 | mul r2, r7, #-15.99951171875 | st r4, [0660] | ld r4, [a163]
mul r3, r3, #256.0 | mul r0, r0, r0 | st r2, [047a] | ld r2, [0463]
mul r3, r3, #256.0 | subxy r0, r0, #1.0 | ld r7, [047a]
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0462]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0473]
mad r4, r3, #1.25 | ld r3, [a16b]
subxy r4, r1, r4 | st r3, [0474]
mad r1, r4, r0 | ld r0, [0475] | ld r4, [a163]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [0460] | ld r1, [0471]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0472]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a17b]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a183] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [0470] | ld r1, [0467]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [046d]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [0469]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [046b]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0466] | ld r1, [a18c]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0468] | ld r2, [c060]
mad r3, r5, r0
subxy r5, r4, r3 | mul r3, r1, r2 | st r3, [046a]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [a155]
max r4, r3, #3.5555752281817892e-06 | st r4, [046c]
min r7, r4, #0.0052294920103370468
addxy r1, r7, #-0.0026165237927826142
mul r2, r1, #256.0 | mul r4, r1, #-0.9431559201400499
mul r3, r2, #1.4949473834993656 | addxy r7, r4, #-0.0074955453913659572 | ld r2, [0681] | ld r4, [0683]
ymacd r7, r3, #0.38488110519443103
ymacd r7, r3, #0.39044132971623097
mul r1, r2, r7
mad r1, r4, #0.1250038148136845
max r3, r1, #-0.125
min r4, r3, #0.125
mov r3, r2 | mul r1, r4, #7.999755859375
mad r3, r4, #0.5 | st r1, [0682]
mad r3, r0, #-0.06250190740684225
mul r1, r3, r7
min r0, r3, #0.0625 | ld r3, [a16c]
mad r1, r2, #-0.50003051897516571 | st r3, [0484]
max r7, r0, #-0.0625 | ld r0, [0485]
mul r4, r1, #-1.9998779315501451 | subxy r0, r0, r3 | ld r1, [0481]
addxy r3, r1, #-0.999 | mul r2, r7, #-15.99951171875 | st r4, [0680] | ld r4, [a164]
mul r3, r3, #256.0 | mul r0, r0, r0 | st r2, [049a] | ld r2, [0483]
mul r3, r3, #256.0 | subxy r0, r0, #1.0 | ld r7, [049a]
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0482]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [0493]
mad r4, r3, #1.25 | ld r3, [a16c]
subxy r4, r1, r4 | st r3, [0494]
mad r1, r4, r0 | ld r0, [0495] | ld r4, [a164]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [0480] | ld r1, [0491]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [0492]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a17c]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a184] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [0490] | ld r1, [0487]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [048d]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [0489]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [048b]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [0486] | ld r1, [a18d]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [0488] | ld r2, [c060]
mad r3, r5, r0
subxy r5, r4, r3 | mul r3, r1, r2 | st r3, [048a]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [a156]
max r4, r3, #3.5555752281817892e-06 | st r4, [048c]
min r7, r4, #0.0052294920103370468
addxy r1, r7, #-0.0026165237927826142
mul r2, r1, #256.0 | mul r4, r1, #-0.9431559201400499
mul r3, r2, #1.4949473834993656 | addxy r7, r4, #-0.0074955453913659572 | ld r2, [06a1] | ld r4, [06a3]
ymacd r7, r3, #0.38488110519443103
ymacd r7, r3, #0.39044132971623097
mul r1, r2, r7
mad r1, r4, #0.1250038148136845
max r3, r1, #-0.125
min r4, r3, #0.125
mov r3, r2 | mul r1, r4, #7.999755859375
mad r3, r4, #0.5 | st r1, [06a2]
mad r3, r0, #-0.06250190740684225
mul r1, r3, r7
min r0, r3, #0.0625 | ld r3, [a16d]
mad r1, r2, #-0.50003051897516571 | st r3, [04a4]
max r7, r0, #-0.0625 | ld r0, [04a5]
mul r4, r1, #-1.9998779315501451 | subxy r0, r0, r3 | ld r1, [04a1]
addxy r3, r1, #-0.999 | mul r2, r7, #-15.99951171875 | st r4, [06a0] | ld r4, [a165]
mul r3, r3, #256.0 | mul r0, r0, r0 | st r2, [04ba] | ld r2, [04a3]
mul r3, r3, #256.0 | subxy r0, r0, #1.0 | ld r7, [04ba]
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [04a2]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [04b3]
mad r4, r3, #1.25 | ld r3, [a16d]
subxy r4, r1, r4 | st r3, [04b4]
mad r1, r4, r0 | ld r0, [04b5] | ld r4, [a165]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [04a0] | ld r1, [04b1]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [04b2]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a17d]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a185] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [04b0] | ld r1, [04a7]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [04ad]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [04a9]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [04ab]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [04a6] | ld r1, [a18e]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [04a8] | ld r2, [c060]
mad r3, r5, r0
subxy r5, r4, r3 | mul r3, r1, r2 | st r3, [04aa]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [a157]
max r4, r3, #3.5555752281817892e-06 | st r4, [04ac]
min r7, r4, #0.0052294920103370468
addxy r1, r7, #-0.0026165237927826142
mul r2, r1, #256.0 | mul r4, r1, #-0.9431559201400499
mul r3, r2, #1.4949473834993656 | addxy r7, r4, #-0.0074955453913659572 | ld r2, [06c1] | ld r4, [06c3]
ymacd r7, r3, #0.38488110519443103
ymacd r7, r3, #0.39044132971623097
mul r1, r2, r7
mad r1, r4, #0.1250038148136845
max r3, r1, #-0.125
min r4, r3, #0.125
mov r3, r2 | mul r1, r4, #7.999755859375
mad r3, r4, #0.5 | st r1, [06c2]
mad r3, r0, #-0.06250190740684225
mul r1, r3, r7
min r0, r3, #0.0625 | ld r3, [a16e]
mad r1, r2, #-0.50003051897516571 | st r3, [04c4]
max r7, r0, #-0.0625 | ld r0, [04c5]
mul r4, r1, #-1.9998779315501451 | subxy r0, r0, r3 | ld r1, [04c1]
addxy r3, r1, #-0.999 | mul r2, r7, #-15.99951171875 | st r4, [06c0] | ld r4, [a166]
mul r3, r3, #256.0 | mul r0, r0, r0 | st r2, [04da] | ld r2, [04c3]
mul r3, r3, #256.0 | subxy r0, r0, #1.0 | ld r7, [04da]
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [04c2]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [04d3]
mad r4, r3, #1.25 | ld r3, [a16e]
subxy r4, r1, r4 | st r3, [04d4]
mad r1, r4, r0 | ld r0, [04d5] | ld r4, [a166]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [04c0] | ld r1, [04d1]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [04d2]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a17e]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a186] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [04d0] | ld r1, [04c7]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [04cd]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [04c9]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [04cb]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [04c6] | ld r1, [a18f]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [04c8] | ld r2, [c060]
mad r3, r5, r0
subxy r5, r4, r3 | mul r3, r1, r2 | st r3, [04ca]
mad r4, r5, r0 | mov r5, #0.0 | ld r0, [a158]
max r4, r3, #3.5555752281817892e-06 | st r4, [04cc]
min r7, r4, #0.0052294920103370468
addxy r1, r7, #-0.0026165237927826142
mul r2, r1, #256.0 | mul r4, r1, #-0.9431559201400499
mul r3, r2, #1.4949473834993656 | addxy r7, r4, #-0.0074955453913659572 | ld r2, [06e1] | ld r4, [06e3]
ymacd r7, r3, #0.38488110519443103
ymacd r7, r3, #0.39044132971623097
mul r1, r2, r7
mad r1, r4, #0.1250038148136845
max r3, r1, #-0.125
min r4, r3, #0.125
mov r3, r2 | mul r1, r4, #7.999755859375
mad r3, r4, #0.5 | st r1, [06e2]
mad r3, r0, #-0.06250190740684225
mul r1, r3, r7
min r0, r3, #0.0625 | ld r3, [a16f]
mad r1, r2, #-0.50003051897516571 | st r3, [04e4]
max r7, r0, #-0.0625 | ld r0, [04e5]
mul r4, r1, #-1.9998779315501451 | subxy r0, r0, r3 | ld r1, [04e1]
addxy r3, r1, #-0.999 | mul r2, r7, #-15.99951171875 | st r4, [06e0] | ld r4, [a167]
mul r3, r3, #256.0 | mul r0, r0, r0 | st r2, [04fa] | ld r2, [04e3]
mul r3, r3, #256.0 | subxy r0, r0, #1.0 | ld r7, [04fa]
max r3, r3, r5
mul r2, r2, r0 | ld r0, [c043]
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [04e2]
mul r0, r0, r4 | ld r4, [c041]
mad r0, r3, r4 | ld r4, [c042]
mad r0, r2, r4 | ld r4, [c040]
mul r4, r4, r2 | ld r2, [04f3]
mad r4, r3, #1.25 | ld r3, [a16f]
subxy r4, r1, r4 | st r3, [04f4]
mad r1, r4, r0 | ld r0, [04f5] | ld r4, [a167]
min r1, r1, r6
mul r7, r7, r1 | subxy r0, r0, r3 | st r1, [04e0] | ld r1, [04f1]
addxy r3, r1, #-0.999 | mul r0, r0, r0
mul r3, r3, #256.0 | subxy r0, r0, #1.0
mul r3, r3, #256.0 | mul r2, r2, r0 | ld r0, [c053]
max r3, r3, r5
min r3, r3, r6
max r2, r2, r3
mul r2, r2, r4
subxy r3, r2, r4 | subxy r4, r4, #1.0 | st r2, [04f2]
mul r0, r0, r4 | ld r4, [c051]
mad r0, r3, r4 | ld r4, [c052]
mad r0, r2, r4 | ld r4, [c050]
mul r4, r4, r2 | ld r2, [c04c]
mad r4, r3, #1.25 | ld r3, [a17f]
subxy r4, r1, r4 | add r3, #-4.75
mad r1, r4, r0 | ld r0, [a187] | ld r4, [c04b]
min r1, r1, r6 | ld r6, [c04a]
mad r0, r2, r1 | mul r7, r7, r6 | ld r6, [c049] | st r1, [04f0] | ld r1, [04e7]
mad r0, r4, r3 | mov r3, #0.041666666666666664:32 | ld r4, [04ed]
min r0, r0, #6.5
max r0, r0, #-8.0
mul r2, r0, #0.04332169878499658 | mad r7, r6, -r4
ymacd r3, r2, #0.16666666666666666 | mov r6, #1.0
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0 | ld r2, [04e9]
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r0, r3, r3 | ld r3, [04eb]
mul r0, r0, #0.02879793265790644
ph_clamp r7, r3, -r7 | min r0, r0, #0.7
subxy r5, r1, r7
mad r1, r5, r0
subxy r5, r2, r1 | st r1, [04e6]
mad r2, r5, r0
subxy r5, r3, r2 | st r2, [04e8]
mad r3, r5, r0
subxy r5, r4, r3 | st r3, [04ea]
mad r4, r5, r0 | mov r5, #0.0
st r4, [04ec]
