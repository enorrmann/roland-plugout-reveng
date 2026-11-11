mov r7, #0.0 | ld r0, [0401] | ld r6, [c000] | ld r2, [06a9]
addxy r7, r7, r0 | mov r0, #0.0 | ld r1, [0601] | ld r3, [c090]
mul r7, r7, #1.3333333333333333 | mul r2, r2, #7.31275 | st r0, [6008] | ld r4, [a1d9]
mul r7, r7, r6 | st r0, [6009] | ld r5, [a1e1]
st r7, [6006] | ld r6, [a1b9]
st r7, [6007] | ld r7, [a1c1]
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
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0612] | ld r4, [a1e9]
mul r2, r2, #256.0 | st r7, [067c] | ld r7, [0645]
ph_clamp r2, r3, -r2 | st r7, [067e] | ld r7, [0647]
mad r1, r2, r3 | mul r2, r2, #1.25 | st r7, [0680] | ld r7, [0649]
subxy r2, r0, r2 | st r4, [0650] | ld r4, [a1d1]
st r5, [064e] | st r6, [066e] | ld r6, [0631]
mad r0, r2, r1 | st r7, [0682] | ld r5, [a1c9]
min r0, r0, #1.0 | ld r7, [064b] | ld r1, [a190]
st r4, [0654] | st r5, [0652] | ld r4, [0617]
st r7, [0684] | st r6, [0686] | ld r5, [0615]
mul r7, r1, r1 | mov r6, r1 | st r0, [0610] | st r0, [a128]
mad r6, r7, #-0.22929172626490923 | st r4, [0688] | ld r0, [a1f8]
mov r4, r5 | ld r2, [a191]
mad r4, r0, #0.12282902002334595 | addxy r6, r6, #0.58141152171439203 | ld r3, [a199]
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
min r0, r3, #0.0
addxy r4, r0, #1.0
mul r3, r2, r4 | addxy r4, r1, #0.05246911125984572
subxy r0, r2, r3 | mul r2, r4, r5
mul r3, r2, #-3.2040359716844296
sign r1, r3
absadd r3, r1, #-1.0
mac_p1w r1, r3, r2, r4, r5 | ld r2, [0688]
mov r3, r1 | st r1, [0696]
mad r3, r2, #-0.31210635861690367
mul r5, r3, #-3.2040359716844296
ph_wrap r4, r3, r5
mov r5, r0 | st r4, [0690]
min r2, r0, #0.55758226361485363
mad r5, r4, #-0.33967391238327793
max r3, r2, #-1.0065597769943722
addxy r2, r5, #-0.16711955938562187 | addxy r3, r6, #0.30769230769230771 | st r3, [0692]
mul r5, r2, #-2.9440000057220459 | st r2, [0688]
sign r5, r5
min r5, r5, #0.0
addxy r4, r5, #1.0 | addxy r2, r5, #1.340482588292577
mul r3, r4, r3 | mul r2, r2, r5
subxy r6, r6, r3 | addxy r2, r5, r2
addxy r3, r6, #0.30769230769230771
mul r3, r3, r5
addxy r3, r3, r6
addxy r6, r7, #-0.015625
mul r4, r4, r6
subxy r6, r7, r4
addxy r7, r6, #-0.015625
mul r4, r7, r5 | ld r5, [0686]
addxy r7, r4, r6 | mad r5, r3, #-0.001808950473858579
mad r7, r0, #0.046000000089406967 | st r5, [0686]
addxy r4, r7, #-0.0076874997466802588
mul r6, r4, #-64.0 | ld r4, [0672]
mad r4, r2, #-0.093249998986721039 | st r0, [0672]
sign r3, r6
addxy r6, r4, #-0.125
mul r0, r6, #-8.0
sign r0, r0
max r5, r0, #0.0
max r0, r0, -r0
mul r5, r5, r0
mac_p1wc3 r0, r6, r1, #0.0048016362864139028, r4
addxy r6, r0, #-0.00060020453580173785
mul r4, r5, r6 | ld r5, [0692]
mul r0, r4, #256.0 | addxy r6, r5, #-0.16654675346254721
mad r0, r2, #0.11462466018373105 | mul r4, r6, #-2.5573125049704686 | ld r2, [0690]
min r6, r2, r4 | st r4, [0694]
mov r4, r5
mad r4, r6, #0.39103551015230648 | mul r6, r7, #-1.0
addxy r7, r6, #0.0076874997466802588
max r6, r7, -r7
max r7, r6, #3.0517578125e-05
max r6, r7, #0.0078125
mul r7, r6, #64.0
recipseed r5, r7
newton r5, r7
newton r5, r7
newton r5, r7
shl r6, r1, 9
mul r7, r6, #-1.6020179858422148
sign r6, r7
mul r7, r1, #-1.0
max r7, r7, -r7
max r7, r7, #2.3811825456001562e-06
max r7, r7, #4.7623650912003123e-06
max r7, r7, #6.6451013120345206e-06
shl r7, r7, 11
mul r7, r7, #1.6020179858422148
recipseed r1, r7
newton r1, r7
newton r1, r7
newton r1, r7
mul r2, r1, r6 | mul r1, r5, r3 | ld r6, [0688]
mul r7, r2, #0.015625 | mul r2, r1, r6
addxy r5, r2, #-0.5095108685749169
mul r3, r5, #2.9440000057220459
ph_wrap r1, r3, r3
absadd r6, r1, #-0.5
mul r2, r6, r7 | addxy r6, r0, #0.15365236116524489
max r5, r2, #-0.0001220703125 | ld r2, [0686]
shl r3, r5, 13
addxy r5, r2, #3.2034623017027206
min r1, r3, #1.0
mul r3, r6, r5 | addxy r6, r4, #0.094143586638990417
mul r5, r3, r6 | ld r6, [0696]
mul r4, r5, r1 | st r5, [0688]
mul r3, r4, #5.9384168655410319 | mov r4, r6
st r3, [0618] | ld r3, [0690]
mad r4, r3, #-0.31210635861690367 | st r1, [0690]
mul r3, r4, #-3.2040359716844296
ph_wrap r4, r3, r3 | ld r3, [0672]
mov r1, r3 | st r4, [069a]
mad r1, r4, #-0.33967391238327793 | addxy r4, r7, #-0.00390625
addxy r1, r1, #-0.16711955938562187 | st r4, [069c]
mul r5, r1, #-2.9440000057220459 | st r1, [0698]
sign r5, r5
min r5, r5, #0.0
addxy r1, r5, #1.0
mul r6, r1, r4
subxy r6, r7, r6
mul r4, r6, r5
addxy r6, r6, r4 | mul r4, r1, r5
addxy r1, r5, r4 | mov r4, r3
mad r4, r5, #-0.67934782476655586 | mad r2, r1, #-0.0011138206293276069
addxy r1, r4, #-0.50679347176889977 | st r2, [0686]
mul r5, r1, #-2.9440000057220459
sign r1, r5
mov r5, r6
mad r5, r0, #0.025422648701109364
addxy r0, r5, #0.00390625
mul r3, r0, #-256.0
sign r3, r3
max r2, r3, #0.0
max r3, r3, -r3
mul r2, r2, r3 | ld r3, [0696]
mul r0, r0, r3
shl r0, r0, 10
mad r5, r0, #0.20338118960887491
addxy r0, r5, #0.00390625
mul r5, r2, r0 | ld r0, [069a]
subxy r2, r6, r5 | ld r6, [0694]
min r5, r0, r6 | ld r6, [0692]
mov r0, r6
mad r0, r5, #0.39103551015230648 | mul r5, r4, #-1.0
addxy r4, r5, #0.50679347176889977
max r5, r4, -r4
max r4, r5, #0.00066342561012358971
max r5, r4, #0.16983695619163897
mul r4, r5, #2.9440000057220459
recipseed r5, r4
newton r5, r4
newton r5, r4
newton r5, r4
mul r6, r5, r1 | ld r4, [0698]
mul r5, r4, r6
addxy r1, r5, #-0.5095108685749169
mul r4, r1, #2.9440000057220459
ph_wrap r6, r3, r4
absadd r5, r6, #-0.5
mul r1, r5, r7 | st r7, [06a2]
max r4, r1, #-0.0001220703125 | ld r1, [0686]
shl r6, r4, 13
min r5, r6, #1.0
addxy r4, r1, #3.2046905404962507 | addxy r6, r0, #0.094143586638990417
mul r0, r4, r6
mul r4, r0, r2
mul r6, r4, r5 | st r4, [0698]
mul r0, r6, #233.45755821837284 | mov r6, r3
st r0, [061e] | ld r0, [069a]
mad r6, r0, #-0.31210635861690367 | st r5, [069a]
mul r0, r6, #-3.2040359716844296
ph_wrap r6, r3, r0 | ld r0, [0672]
mov r5, r0 | st r6, [06a0]
mad r5, r6, #-0.33967391238327793 | ld r6, [069c]
addxy r5, r5, #-0.16711955938562187
mul r4, r5, #-2.9440000057220459 | st r5, [069e]
sign r4, r4
min r4, r4, #0.0
addxy r5, r4, #1.0
mul r3, r5, r6 | mul r5, r5, r4
subxy r3, r7, r3 | addxy r5, r4, r5
mul r6, r3, r4 | mad r1, r5, #-0.001114441312736953
addxy r7, r3, r6 | mov r5, r0
sum8 r3, r3, r6, r2
mul r6, r3, #-256.0 | mad r5, r4, #-0.67934782476655586
sign r2, r6
max r6, r2, #0.0
max r2, r2, -r2
mul r6, r6, r2 | addxy r4, r5, #-0.50679347176889977 | ld r2, [0696]
mul r0, r3, r2 | mul r4, r4, #-2.9440000057220459 | ld r2, [069e]
shl r0, r0, 10
mad r3, r0, #0.20338118960887491
mul r0, r6, r3 | ld r3, [06a0]
subxy r6, r7, r0 | ld r7, [0694]
min r0, r3, r7 | ld r7, [0692] | st r6, [0686]
mov r3, r7
mad r3, r0, #0.39103551015230648 | mul r0, r5, #-1.0
addxy r5, r0, #0.50679347176889977
max r0, r5, -r5
max r5, r0, #0.00066342561012358971
max r0, r5, #0.16983695619163897
mul r5, r0, #2.9440000057220459
recipseed r6, r5
newton r6, r5
newton r6, r5
sign r4, r4
newton r6, r5
mul r0, r6, r4
mul r7, r2, r0
addxy r5, r7, #-0.5095108685749169
mul r6, r5, #2.9440000057220459
ph_wrap r4, r3, r6
absadd r2, r4, #-0.5
ld r0, [06a2]
mul r7, r2, r0 | addxy r2, r1, #3.2059194637334172
max r5, r7, #-0.0001220703125
shl r6, r5, 13
addxy r7, r3, #0.094143586638990417
min r4, r6, #1.0 | ld r6, [0686]
mul r5, r2, r7
mul r3, r5, r6 | ld r5, [0696]
mul r2, r3, r4 | st r3, [069e]
mul r7, r2, #233.32753501166249 | mov r2, r5
st r7, [0624] | ld r7, [06a0]
mad r2, r7, #-0.31210635861690367 | st r4, [06a0]
mul r7, r2, #-3.2040359716844296
ph_wrap r2, r3, r7 | ld r7, [0672]
mov r4, r7 | st r2, [0616]
mad r4, r2, #-0.33967391238327793 | st r2, [06a6] | ld r2, [069c]
addxy r4, r4, #-0.16711955938562187
mul r3, r4, #-2.9440000057220459 | st r4, [06a4]
sign r3, r3
min r3, r3, #0.0
addxy r4, r3, #1.0 | mad r7, r3, #-0.67934782476655586
mul r2, r4, r2 | mul r4, r4, r3
addxy r4, r3, r4 | subxy r2, r0, r2
mad r1, r4, #-0.0011150623420259536 | mul r0, r2, r3
mul r4, r1, #0.99777407723768829 | addxy r5, r2, r0
addxy r4, r4, #-0.0016679411512323834
addxy r4, r7, #-0.50679347176889977 | st r4, [0630]
mul r3, r4, #-2.9440000057220459
sign r4, r3
sum8 r3, r2, r0, r6
mul r2, r3, #-256.0
sign r0, r2
max r6, r0, #0.0
max r2, r0, -r0
mul r0, r6, r2 | ld r6, [0696]
mul r2, r3, r6
shl r6, r2, 10
mad r3, r6, #0.20338118960887491
mul r2, r0, r3 | ld r3, [06a6]
subxy r6, r5, r2 | ld r5, [0694]
mul r0, r6, #32.0
min r2, r3, r5 | st r0, [0632] | ld r0, [0692]
mul r3, r7, #-1.0 | mad r0, r2, #0.39103551015230648
addxy r5, r3, #0.50679347176889977
max r2, r5, -r5
max r7, r2, #0.00066342561012358971
max r3, r7, #0.16983695619163897
mul r5, r3, #2.9440000057220459
recipseed r2, r5
newton r2, r5
newton r2, r5
newton r2, r5
mul r7, r2, r4 | ld r3, [06a4]
mul r5, r3, r7
addxy r2, r5, #-0.5095108685749169
mul r4, r2, #2.9440000057220459
ph_wrap r3, r3, r4
absadd r7, r3, #-0.5
ld r5, [06a2]
mul r2, r7, r5 | addxy r5, r1, #3.2071490717956306
max r4, r2, #-0.0001220703125
addxy r2, r0, #0.094143586638990417
shl r3, r4, 13
mul r4, r5, r2 | ld r5, [0688] | ld r2, [0690]
min r7, r3, #1.0
mul r3, r4, r6 | ld r4, [0670]
mac_p1wc3 r6, r5, r2, #0.16839504915909823, r4 | ld r2, [0658] | ld r4, [065a]
mad r4, r2, #0.58266710469935634 | mul r1, r3, r7 | ld r2, [065c]
mad r4, r2, #0.38910162374577151 | mul r0, r1, #233.19758422083112 | ld r2, [065e] | ld r1, [066e]
mad r4, r2, #0.77034243706960015 | mul r0, r1, r1 | ld r2, [0660] | st r0, [062a]
mad r4, r2, #0.91845215940572489 | ld r2, [0662]
mad r4, r2, #0.22341414130630471 | ld r2, [0664]
mad r4, r2, #0.91845215940572489 | ld r2, [0666]
mad r4, r2, #0.77034243706960015 | ld r2, [0668]
mad r4, r2, #0.11298477383489047 | ld r2, [066a]
addxy r4, r4, r2 | mov r5, r1 | ld r2, [066c]
mad r4, r2, #0.58266710469935634 | mad r5, r0, #-0.39487129125824577 | ld r2, [0674] | st r0, [066c]
mov r0, r2
mad r0, r4, #0.12282902002334595 | ld r4, [0698]
mad r0, r6, #0.053783092849566262 | ld r6, [069a]
mac_p1wc3 r4, r4, r6, #0.3086534627716338, r0 | ld r6, [069e] | ld r0, [06a0]
mac_p1wc3 r6, r6, r0, #0.5060008047626724, r4 | ld r4, [0656]
mac_p1wc3 r0, r3, r7, #0.57449937643883475, r6
addxy r7, r5, #0.49510531013321074 | mul r3, r4, r4
mul r6, r7, r0 | mov r5, r4
mad r2, r6, #-0.028508363739871666 | mad r5, r3, #-0.52307693774883568
mul r7, r2, r5 | addxy r6, r4, #-2.3315790186958654 | st r2, [0634]
mad r0, r7, #-0.29160521319112204 | mul r2, r6, r3 | ld r6, [066c]
addxy r7, r1, #-4.1170513665950033
mul r3, r7, r6 | mov r5, r4 | ld r6, [068e] | ld r7, [c061]
mad r1, r3, #0.11104427371485703 | addxy r3, r6, #-0.30984204928503106 | ld r6, [068c]
addxy r6, r6, #3.3998833643311013 | mad r5, r2, #0.55447470402002497 | ld r2, [c060]
mul r3, r3, r6 | ld r6, [068a]
mul r3, r3, r6
mul r6, r3, r2 | addxy r3, r5, #-0.28210118391990024
addxy r2, r1, #2.2522163013075054
mul r5, r3, r2 | ld r3, [0654] | ld r2, [0676]
mul r1, r5, r7 | ld r5, [0678]
mac_p1wc3 r7, r3, r2, #-0.125, r5
mad r7, r6, #-0.14750965228341961 | ld r6, [0652]
mac_p1wc3 r3, r1, r0, #0.81166572352091582, r7
mac_p1wc3 r1, r3, r6, #0.039249224126010791, r2 | ld r2, [067a]
mac_p1wc3 r7, r1, r6, #0.015183606254173724, r5 | ld r5, [067c]
mul r0, r1, #25.478210646647959 | mul r3, r7, #65.860506605610666
mad r7, r2, #-0.015183606254173724 | st r0, [063c]
mac_p1wc3 r0, r7, r6, #0.0058737950625882924, r2 | st r3, [063e] | ld r3, [0650]
mul r1, r0, #170.24768303021952 | mad r0, r5, #-0.00036711219141176828 | ld r7, [064e]
mad r5, r0, #0.82008461664737586 | mul r6, r3, r7 | st r1, [0640] | ld r1, [067e]
mov r7, r0 | st r5, [0642] | ld r5, [0680]
addxy r3, r1, r5
mad r7, r3, #-0.0029368975312941462
min r2, r6, #-0.031863559037446976
mac_p1wc3 r6, r4, r0, #0.55257456844603692, r7 | ld r0, [0684] | ld r7, [0682]
mov r3, r6
mad r3, r1, #0.3488626684325794 | mad r2, r0, #0.48980201941689522 | ld r1, [064c]
mul r4, r3, #2.8664574644599963 | mad r6, r3, #0.00067501250856102457
addxy r4, r2, #0.031863559037446976 | mad r5, r3, #0.013334314394818568 | st r4, [0644]
mul r3, r4, #-4.083282470703125 | mad r6, r5, #-0.00088816869227154418 | st r5, [0646]
mad r1, r7, #0.5 | st r6, [064c]
sign r5, r3
mul r3, r1, #-2.0 | mad r7, r1, #-0.79960823059082031
min r4, r5, #0.0
sign r5, r3
min r3, r5, #0.0
mov r5, r2
mad r5, r0, #0.49600245812738197 | addxy r6, r4, #1.0
addxy r5, r5, #0.031863559037446976 | mad r0, r2, #-0.0051044632946286583
mul r6, r6, r5 | mov r5, r2
mad r5, r6, #-0.9874991774559021
mov r6, r5
mad r6, r0, #0.48980201941689522
addxy r2, r6, #0.03178389451817365
mul r0, r2, r4
addxy r6, r0, r5 | mov r0, r1
addxy r2, r6, #0.031863559037446976 | mad r0, r7, #0.5
addxy r5, r3, #1.0 | mul r4, r2, #-2.0416412353515625
mul r2, r5, r0 | st r4, [064a]
subxy r4, r1, r2
mov r5, r4
mad r5, r7, #-0.5
mul r0, r5, r3 | ld r5, [064c]
addxy r1, r0, r4
mad r6, r1, #-0.48980201941689522 | mul r2, r1, #2.0
addxy r7, r6, #0.031863559037446976 | st r2, [0648]
mul r3, r7, r5
mul r0, r3, #256.0
mul r4, r0, #5.8133366660523764
st r4, [0400]
