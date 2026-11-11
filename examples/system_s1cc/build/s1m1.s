mov r7, #0.0 | ld r0, [0401] | ld r6, [c000] | ld r2, [06a9]
addxy r7, r7, r0 | mov r0, #0.0 | ld r1, [0601] | ld r3, [c090]
mul r7, r7, #1.3333333333333333 | mul r2, r2, #7.31275 | st r0, [6008] | ld r4, [a1b9]
mul r7, r7, r6 | st r0, [6009] | ld r5, [a1c1]
st r7, [6006] | ld r6, [a1f8]
st r7, [6007] | ld r7, [a189]
st r7, [0656] | ld r7, [0619] | st r3, [a120]
st r7, [061a] | st r7, [0658] | ld r7, [061b]
st r7, [061c] | st r7, [065a] | ld r7, [061d]
st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [06ab]
add r0, #0.6184303127229214:32 | st r7, [065c] | ld r7, [061f]
st r7, [0620] | st r7, [065e] | ld r7, [0621]
ph_wrap r0, r3, r0 | st r7, [0622]
addxy r2, r2, r0 | st r0, [06aa] | ld r0, [a168]
subxy r1, r1, r0 | ph_wrap r2, r3, r2 | st r7, [0660] | ld r7, [0623]
mul r1, r1, r1 | st r7, [0662] | ld r7, [0625]
st r7, [0626] | st r7, [0664] | ld r7, [0627]
st r1, [060e] | ld r1, [c050]
st r2, [06a8] | st r0, [0600]
st r7, [0628] | st r7, [0666] | ld r7, [0629]
mul r1, r3, r1 | ld r0, [c098] | st r7, [0668] | ld r7, [062b]
ld r2, [c051] | st r7, [062c]
mad r1, r0, -r2 | st r7, [066a] | ld r7, [062d]
add r1, #1.0 | st r7, [062e] | ld r2, [0613]
st r1, [060a] | ld r1, [c04f] | ld r0, [0611]
mul r1, r3, r1 | st r7, [066c] | ld r7, [062f] | ld r3, [c053]
st r1, [060c] | st r7, [0670] | ld r1, [060e]
mad r2, r2, -r1 | addxy r1, r0, #-0.999 | ld r7, [0633] | st r4, [064c]
mul r1, r1, #256.0 | st r7, [0672] | ld r7, [0635]
ph_clamp r1, r3, -r1 | st r7, [0674] | ld r7, [063d]
mul r1, r1, #256.0 | st r7, [0676] | ld r7, [063f]
ph_clamp r1, r3, -r1 | st r7, [0678] | ld r7, [0641]
max r2, r2, r1 | st r7, [067a] | ld r7, [0643] | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0612] | ld r4, [a1c9]
mul r2, r2, #256.0 | st r7, [067c] | ld r7, [0645]
ph_clamp r2, r3, -r2 | st r7, [067e] | ld r7, [0647]
mad r1, r2, r3 | mul r2, r2, #1.25 | st r7, [0680] | ld r7, [0649]
subxy r2, r0, r2 | st r4, [0650] | ld r4, [a199]
st r5, [064e] | st r6, [066e] | ld r6, [0631]
mad r0, r2, r1 | st r7, [0682] | ld r5, [a191]
min r0, r0, #1.0 | ld r7, [064b] | ld r1, [a190]
st r4, [0654] | st r5, [0652] | ld r4, [0617]
st r7, [0684] | st r6, [0686] | ld r5, [0615]
mul r7, r1, r1 | mov r6, r1 | st r0, [0610] | st r0, [a128]
mad r6, r7, #-0.22929172626490923 | st r4, [0688] | ld r0, [a1d8]
mov r4, r5 | ld r2, [a1e8]
mad r4, r0, #0.12282902002334595 | addxy r6, r6, #0.58141152171439203 | ld r3, [a1f0]
mul r6, r6, r4 | mul r0, r2, r2
mad r5, r6, #-0.0020813931703287381
mov r6, r2 | st r5, [0614]
mad r6, r0, #-0.52307693774883568
mac_p1wc3 r5, r5, r6, #-0.30769230769230771, r4
addxy r4, r2, #-2.5421053176441357 | addxy r5, r3, #0.25 | st r5, [068a]
mul r4, r4, r0 | addxy r0, r1, #-2.5772775174587217
mad r2, r4, #0.4617253880327547 | mul r4, r0, r7
addxy r0, r3, #-1.25 | mad r1, r4, #0.39449951927190524 | st r2, [068e]
mul r7, r0, #8.0 | mul r5, r5, #8.0 | st r1, [068c]
sign r4, r7
max r7, r4, -r4
max r4, r4, #0.0
mul r7, r7, r0
mul r0, r7, r4
sign r5, r5
subxy r7, r3, r0
min r5, r5, #0.0
addxy r4, r7, #-0.25
mul r3, r5, r4
addxy r0, r3, r7
addxy r5, r0, #0.59374997850216871
mul r4, r5, #-5.5451774597167969
floor r3, r4
addxy r7, r3, #0.69314718055994529
mul r5, r7, #0.0625 | mov r4, #0.041666666666666664
ymacd r4, r5, #0.16666666666666666
ymacd r4, r5, #0.5
ymacd r4, r5, #1.0
ymacd r4, r5, #1.0
mul r7, r4, r4
mul r5, r7, r7
mul r4, r5, r5
mul r7, r4, r4 | mov r4, r0
mad r4, r3, #0.18033687961558437 | mul r5, r7, #0.015625
addxy r7, r4, #0.59374997850216871
mul r3, r7, r7 | addxy r7, r4, #-1.3899606075128503
mul r7, r3, r7 | mov r1, r4
mad r1, r7, #0.27953547022611447
addxy r7, r1, #-1.0292819975560181
mul r1, r7, r3 | mov r7, r4
mad r7, r1, #0.42706930106914331
addxy r1, r7, #-0.6686081920331266
mul r7, r1, r3 | mov r1, r4
mad r1, r7, #0.73211893606984124
addxy r7, r1, #-0.30793450019262536
mac_p1wc3 r1, r7, r3, #0.65042786114249618, r4
addxy r7, r1, #0.034303146377230566
mac_p1wc3 r1, r7, r3, #0.1269169056354455, r4
addxy r4, r0, #0.45833373069763184 | mul r7, r0, r0
mul r4, r4, #8.0
sign r4, r4
max r2, r4, #0.0
max r4, r4, -r4
mac_p1wc3 r3, r0, r0, #0.91847825778863046, r0
mul r2, r2, r4 | addxy r4, r3, #0.2108967413509085
addxy r2, r2, #-1.0
mul r4, r4, r2
addxy r2, r4, r3 | mul r4, r0, #8.0
sign r3, r4
min r4, r3, #0.0
addxy r3, r4, #1.0
mul r4, r2, r3
subxy r3, r2, r4 | addxy r2, r1, #0.05246911125984572
mul r4, r2, r5
mul r1, r4, #-3.2040359716844296
sign r1, r1
absadd r1, r1, #-1.0
mac_p1w r1, r1, r4, r2, r5 | ld r5, [0688]
mov r4, r1 | st r1, [0694]
mad r4, r5, #-0.31210635861690367 | addxy r1, r6, #0.30769230769230771
mul r2, r4, #-3.2040359716844296
min r4, r3, #0.55758226361485363
ph_wrap r5, r3, r2
max r2, r4, #-1.0065597769943722 | st r5, [0688]
mov r4, r3 | st r2, [0692]
mad r4, r5, #-0.33967391238327793
addxy r4, r4, #-0.16711955938562187
mul r5, r4, #-2.9440000057220459 | st r4, [0690]
sign r5, r5
min r5, r5, #0.0
addxy r2, r5, #1.0 | addxy r4, r5, #1.340482588292577
mul r1, r2, r1 | mul r4, r4, r5
subxy r6, r6, r1 | addxy r4, r5, r4
addxy r1, r6, #0.30769230769230771
mul r1, r1, r5
addxy r1, r1, r6 | mul r6, r5, r2
subxy r2, r5, r6 | addxy r6, r7, #-0.015625
addxy r5, r2, #-1.0
mac_p1w r7, r5, r6, r0, r0 | ld r0, [0686]
addxy r5, r7, #0.015625 | mad r7, r3, #-0.046000000089406967
mul r6, r5, r2 | mad r0, r1, #-0.001808950473858579
subxy r5, r6, r7 | ld r7, [0672] | st r0, [0686]
addxy r2, r5, #0.0076874997466802588 | mad r7, r4, #-0.093249998986721039 | st r3, [0672]
mul r1, r2, #64.0 | addxy r5, r7, #-0.125
sign r6, r1
mul r1, r5, #-8.0
sign r1, r1
max r3, r1, #0.0
max r1, r1, -r1
mul r3, r3, r1 | ld r1, [0694]
mac_p1wc3 r5, r5, r1, #0.0048016362864139028, r7
addxy r7, r5, #-0.00060020453580173785
mul r5, r3, r7 | ld r7, [0692]
mul r3, r5, #256.0 | addxy r5, r7, #-0.16654675346254721
mad r3, r4, #0.11462466018373105 | mul r4, r5, #-2.5573125049704686 | ld r5, [0688]
min r0, r5, r4 | st r4, [0696]
mov r4, r7
mad r4, r0, #0.39103551015230648
max r0, r2, -r2 | st r4, [0698]
max r2, r0, #3.0517578125e-05
max r0, r2, #0.0078125
mul r2, r0, #64.0
recipseed r4, r2
newton r4, r2
newton r4, r2
newton r4, r2
shl r0, r1, 9
mul r2, r1, #-1.0 | mul r7, r0, #-1.6020179858422148
max r0, r2, -r2
sign r5, r7
max r7, r0, #2.3811825456001562e-06
max r2, r7, #4.7623650912003123e-06
max r0, r2, #6.6451013120345206e-06
shl r7, r0, 11
mul r2, r7, #1.6020179858422148
recipseed r0, r2
newton r0, r2
newton r0, r2
newton r0, r2
mul r7, r0, r5 | mul r2, r4, r6 | ld r0, [0690]
mul r5, r2, r0 | mul r1, r7, #0.015625
addxy r7, r5, #-0.5095108685749169
mul r4, r7, #2.9440000057220459
ph_wrap r6, r3, r4
absadd r2, r6, #-0.5
mul r0, r2, r1 | addxy r6, r3, #0.15365236116524489 | ld r2, [0686]
max r5, r0, #-0.0001220703125
shl r7, r5, 13
min r4, r7, #1.0 | ld r7, [0698]
addxy r0, r2, #3.2034623017027206
mul r5, r6, r0 | addxy r6, r7, #0.094143586638990417
mul r0, r5, r6 | ld r6, [0694]
mul r7, r0, r4 | st r0, [0698]
mul r5, r7, #5.9384168655410319 | mov r7, r6
st r5, [0618] | ld r5, [0688]
mad r7, r5, #-0.31210635861690367 | st r4, [0688]
mul r5, r7, #-3.2040359716844296
ph_wrap r7, r3, r5 | ld r5, [0672]
mov r4, r5 | st r7, [069a]
mad r4, r7, #-0.33967391238327793 | addxy r7, r1, #-0.00390625
addxy r4, r4, #-0.16711955938562187 | st r7, [069c]
mul r0, r4, #-2.9440000057220459 | st r4, [0690]
sign r0, r0
min r0, r0, #0.0
addxy r4, r0, #1.0
mul r6, r4, r7
subxy r6, r1, r6
mul r7, r6, r0
addxy r6, r6, r7 | mul r7, r4, r0
mul r4, r0, r4 | addxy r7, r0, r7
subxy r0, r0, r4 | mad r2, r7, #-0.0011138206293276069
mov r4, r5 | st r2, [0686]
mad r4, r0, #0.67934782476655586
addxy r7, r4, #-0.50679347176889977
mul r0, r7, #-2.9440000057220459
sign r7, r0
mov r0, r6
mad r0, r3, #0.025422648701109364
addxy r3, r0, #0.00390625
mul r5, r3, #-256.0
sign r5, r5
max r2, r5, #0.0
max r5, r5, -r5
mul r2, r2, r5 | ld r5, [0694]
mul r3, r3, r5
shl r3, r3, 10
mad r0, r3, #0.20338118960887491
addxy r3, r0, #0.00390625
mul r0, r2, r3 | ld r3, [069a]
subxy r2, r6, r0 | ld r6, [0696]
min r0, r3, r6 | ld r6, [0692]
mov r3, r6
mad r3, r0, #0.39103551015230648 | mul r0, r4, #-1.0
addxy r4, r0, #0.50679347176889977
max r0, r4, -r4
max r4, r0, #0.00066342561012358971
max r0, r4, #0.16983695619163897
mul r4, r0, #2.9440000057220459
recipseed r5, r4
newton r5, r4
newton r5, r4
newton r5, r4
mul r0, r5, r7 | ld r6, [0690]
mul r4, r6, r0
addxy r5, r4, #-0.5095108685749169
mul r7, r5, #2.9440000057220459
ph_wrap r6, r3, r7
absadd r0, r6, #-0.5
mul r4, r0, r1 | ld r0, [0686] | st r1, [06a2]
max r5, r4, #-0.0001220703125
shl r7, r5, 13
addxy r4, r0, #3.2046905404962507 | addxy r5, r3, #0.094143586638990417
min r6, r7, #1.0
mul r7, r4, r5
mul r3, r7, r2 | ld r7, [0694]
mul r4, r3, r6 | st r3, [0690]
mul r5, r4, #233.45755821837284 | mov r4, r7
st r5, [061e] | ld r5, [069a]
mad r4, r5, #-0.31210635861690367 | st r6, [069a]
mul r5, r4, #-3.2040359716844296
ph_wrap r4, r3, r5 | ld r5, [0672]
mov r6, r5 | st r4, [06a0]
mad r6, r4, #-0.33967391238327793 | ld r4, [069c]
addxy r6, r6, #-0.16711955938562187
mul r3, r6, #-2.9440000057220459 | st r6, [069e]
sign r3, r3
min r3, r3, #0.0
addxy r6, r3, #1.0
mul r7, r6, r4
subxy r7, r1, r7
mul r4, r7, r3
addxy r1, r7, r4
mul r1, r6, r3 | mul r6, r3, r6 | st r1, [06a4]
addxy r1, r3, r1 | subxy r3, r3, r6
mov r6, r5
mad r6, r3, #0.67934782476655586 | mad r0, r1, #-0.001114441312736953
addxy r1, r6, #-0.50679347176889977
mul r3, r1, #-2.9440000057220459
sign r1, r3
sum8 r3, r7, r4, r2
mul r7, r3, #-256.0
sign r4, r7
max r2, r4, #0.0
max r7, r4, -r4
mul r4, r2, r7 | ld r2, [0694]
mul r7, r3, r2
shl r7, r7, 10
mad r3, r7, #0.20338118960887491
mul r7, r4, r3 | ld r4, [06a4]
subxy r3, r4, r7 | ld r4, [06a0] | ld r7, [0696]
min r5, r4, r7 | ld r7, [0692] | st r3, [06a4]
mov r4, r7 | ld r7, [069e]
mad r4, r5, #0.39103551015230648 | mul r5, r6, #-1.0
addxy r6, r5, #0.50679347176889977
max r5, r6, -r6
max r6, r5, #0.00066342561012358971
max r5, r6, #0.16983695619163897
mul r6, r5, #2.9440000057220459
recipseed r2, r6
newton r2, r6
newton r2, r6
newton r2, r6
mul r5, r2, r1
mul r3, r7, r5
addxy r6, r3, #-0.5095108685749169
mul r2, r6, #2.9440000057220459
ph_wrap r1, r3, r2
absadd r7, r1, #-0.5
ld r5, [06a2]
mul r3, r7, r5 | addxy r7, r0, #3.2059194637334172
max r6, r3, #-0.0001220703125
shl r2, r6, 13
addxy r3, r4, #0.094143586638990417
min r1, r2, #1.0 | ld r2, [06a4]
mul r6, r7, r3
mul r4, r6, r2 | ld r6, [0694]
mul r7, r4, r1 | st r4, [069e]
mul r3, r7, #233.32753501166249 | mov r7, r6
st r3, [0624] | ld r3, [06a0]
mad r7, r3, #-0.31210635861690367 | st r1, [06a0]
mul r3, r7, #-3.2040359716844296
ph_wrap r7, r3, r3 | ld r3, [0672]
mov r1, r3 | st r7, [0616]
mad r1, r7, #-0.33967391238327793 | st r7, [06a6] | ld r7, [069c]
addxy r1, r1, #-0.16711955938562187
mul r4, r1, #-2.9440000057220459 | st r1, [0686]
sign r4, r4
min r4, r4, #0.0
addxy r1, r4, #1.0
mul r7, r1, r7
subxy r7, r5, r7
mul r5, r7, r4
addxy r6, r7, r5
mul r6, r1, r4 | mul r1, r4, r1 | st r6, [069c]
addxy r6, r4, r6 | subxy r4, r4, r1
mad r0, r6, #-0.0011150623420259536 | mad r3, r4, #0.67934782476655586
mul r1, r0, #0.99777407723768829
addxy r6, r1, #-0.0016679411512323834 | addxy r1, r3, #-0.50679347176889977
mul r6, r1, #-2.9440000057220459 | st r6, [0630]
sum8 r1, r7, r5, r2
sign r4, r6
mul r6, r1, #-256.0
sign r7, r6
max r5, r7, #0.0
max r2, r7, -r7 | ld r7, [0694]
mul r6, r5, r2 | mul r5, r1, r7
shl r2, r5, 10 | ld r5, [069c]
mad r1, r2, #0.20338118960887491
mul r7, r6, r1 | ld r1, [06a6]
subxy r2, r5, r7 | ld r5, [0696]
mul r6, r2, #32.0
min r7, r1, r5 | st r6, [0632] | ld r6, [0692]
mul r1, r3, #-1.0 | mad r6, r7, #0.39103551015230648 | st r2, [0692]
addxy r5, r1, #0.50679347176889977
max r7, r5, -r5
max r3, r7, #0.00066342561012358971
max r1, r3, #0.16983695619163897 | ld r3, [0686]
mul r5, r1, #2.9440000057220459
recipseed r1, r5
newton r1, r5
newton r1, r5
newton r1, r5
mul r7, r1, r4
mul r2, r3, r7
addxy r5, r2, #-0.5095108685749169
mul r1, r5, #2.9440000057220459
ph_wrap r4, r3, r1
absadd r3, r4, #-0.5
ld r7, [06a2]
mul r2, r3, r7 | addxy r3, r0, #3.2071490717956306
max r5, r2, #-0.0001220703125
addxy r7, r6, #0.094143586638990417
shl r1, r5, 13 | ld r5, [0692]
mul r2, r3, r7 | ld r3, [0698] | ld r7, [0688]
min r4, r1, #1.0
mul r1, r2, r5 | ld r2, [0670]
mac_p1wc3 r5, r3, r7, #0.16839504915909823, r2 | ld r7, [0658] | ld r2, [065a]
mad r2, r7, #0.58266710469935634 | mul r0, r1, r4 | ld r7, [065c]
mad r2, r7, #0.38910162374577151 | mul r6, r0, #233.19758422083112 | ld r7, [065e] | ld r0, [066e]
mad r2, r7, #0.77034243706960015 | mul r6, r0, r0 | ld r7, [0660] | st r6, [062a]
mad r2, r7, #0.91845215940572489 | ld r7, [0662]
mad r2, r7, #0.22341414130630471 | ld r7, [0664]
mad r2, r7, #0.91845215940572489 | ld r7, [0666]
mad r2, r7, #0.77034243706960015 | ld r7, [0668]
mad r2, r7, #0.11298477383489047 | ld r7, [066a]
addxy r2, r2, r7 | mov r3, r0 | ld r7, [066c]
mad r2, r7, #0.58266710469935634 | mad r3, r6, #-0.39487129125824577 | ld r7, [0674] | st r6, [066c]
mov r6, r7
mad r6, r2, #0.12282902002334595 | ld r2, [0690]
mad r6, r5, #0.053783092849566262 | ld r5, [069a]
mac_p1wc3 r2, r2, r5, #0.3086534627716338, r6 | ld r5, [069e] | ld r6, [06a0]
mac_p1wc3 r5, r5, r6, #0.5060008047626724, r2 | ld r2, [0656]
mac_p1wc3 r6, r1, r4, #0.57449937643883475, r5
addxy r4, r3, #0.49510531013321074 | mul r1, r2, r2
mul r5, r4, r6 | mov r3, r2
mad r7, r5, #-0.028508363739871666 | mad r3, r1, #-0.52307693774883568
mul r4, r7, r3 | addxy r5, r2, #-2.3315790186958654 | st r7, [0634]
mad r6, r4, #-0.29160521319112204 | mul r7, r5, r1 | ld r5, [066c]
addxy r4, r0, #-4.1170513665950033
mul r1, r4, r5 | mov r3, r2 | ld r5, [068e] | ld r4, [c061]
mad r0, r1, #0.11104427371485703 | addxy r1, r5, #-0.30984204928503106 | ld r5, [068c]
addxy r5, r5, #3.3998833643311013 | mad r3, r7, #0.55447470402002497 | ld r7, [c060]
mul r1, r1, r5 | ld r5, [068a]
mul r1, r1, r5
mul r5, r1, r7 | addxy r1, r3, #-0.28210118391990024
addxy r7, r0, #2.2522163013075054
mul r3, r1, r7 | ld r1, [0654] | ld r7, [0676]
mul r0, r3, r4 | ld r3, [0678]
mac_p1wc3 r4, r1, r7, #-0.38685111851959952, r3
mad r4, r5, #-0.9130283836524492 | ld r5, [0652]
mac_p1wc3 r1, r0, r6, #0.13113341358346642, r4
addxy r0, r5, #0.29022591364275391
mul r6, r1, r0
shl r4, r6, 14
mad r7, r4, #0.79619408270774872 | ld r4, [067a]
mul r5, r7, r0 | st r7, [063c]
shl r1, r5, 9
mad r1, r3, #0.77370223703919905 | ld r3, [067c]
mul r6, r1, #1.2924868924081134 | mad r1, r4, #-0.77370223703919905
mul r7, r1, r0 | st r6, [063e] | ld r6, [0650]
shl r5, r7, 10 | ld r1, [064e]
mad r4, r5, #0.83526118352339107 | mul r0, r6, r1 | ld r5, [067e]
mad r4, r3, #-0.125 | st r4, [0640]
mad r3, r4, #0.0024085044860839844
max r7, r0, #0.031863559037446976 | st r3, [0642] | ld r3, [0680]
addxy r6, r5, r3
subxy r1, r6, r4 | ld r6, [0684]
mac_p1wc3 r0, r2, r4, #0.55257456844603681, r1 | ld r2, [0682] | ld r4, [064c]
mad r7, r6, #-0.48980201941689522 | mad r5, r0, #0.0084184918509322415
addxy r1, r7, #-0.031863559037446976 | mad r0, r5, #0.080182118188577386 | st r5, [0644]
mad r3, r5, #0.0046518445014953613 | mul r5, r1, #4.083282470703125
mad r0, r3, #-0.30241732399842086 | mad r4, r2, #0.5 | st r3, [0646]
sign r3, r5 | st r0, [064c]
mul r5, r4, #-2.0 | mad r2, r4, #-0.79960823059082031
min r1, r3, #0.0
sign r3, r5
min r5, r3, #0.0
mov r3, r7
mad r3, r6, #-0.49600245812738197 | addxy r0, r1, #1.0
addxy r3, r3, #-0.031863559037446976 | mad r6, r7, #0.0051044632946286583
mul r0, r0, r3 | mov r3, r7
mad r3, r0, #-0.9874991774559021
mov r0, r3
mad r0, r6, #-0.48980201941689522
addxy r7, r0, #-0.03178389451817365
mul r6, r7, r1
addxy r0, r6, r3 | mov r6, r4
addxy r7, r0, #-0.031863559037446976 | mad r6, r2, #0.5
addxy r3, r5, #1.0 | mul r1, r7, #2.0416412353515625
mul r7, r3, r6 | st r1, [064a]
subxy r1, r4, r7
mov r3, r1
mad r3, r2, #-0.5
mul r6, r3, r5 | ld r3, [064c]
addxy r4, r6, r1
mad r0, r4, #0.48980201941689522 | mul r7, r4, #2.0
addxy r2, r0, #-0.031863559037446976 | st r7, [0648]
mul r5, r2, r3
mul r6, r5, #-4.3707325703964077
st r6, [0400]
