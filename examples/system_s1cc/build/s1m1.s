mov r7, #0.0 | ld r0, [0401] | ld r2, [0683] | ld r3, [c090]
addxy r7, r7, r0 | mov r0, #0.0 | ld r1, [060d] | st r3, [a120] | st r3, [0510]
mul r2, r2, #7.31275 | mul r7, r7, #1.3333333333333333 | st r0, [6008]
st r0, [6009] | ld r5, [0621]
st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [0685]
add r0, #0.6184303127229214:32 | ld r6, [c000]
ph_wrap r0, r3, r0
addxy r2, r2, r0 | mul r7, r7, r6 | st r0, [0684] | ld r0, [a168]
subxy r1, r1, r0 | st r0, [060c] | ld r0, [c098]
mul r1, r1, r1 | st r7, [6006]
ph_wrap r2, r3, r2 | st r1, [061a] | ld r1, [c050]
mul r1, r3, r1 | st r2, [0682] | ld r2, [c051]
mad r1, r0, -r2 | ld r2, [061f] | ld r0, [0521]
add r1, #1.0 | st r7, [6007]
mov r7, r5 | st r1, [0616] | ld r1, [c04f]
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
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1 | ld r1, [a130]
min r0, r0, #1.0 | st r1, [0550] | ld r1, [a190]
mul r3, r1, r1 | mov r6, r1 | st r0, [0520] | st r0, [a128]
mad r6, r3, #-0.22929172626490923 | ld r0, [c080] | ld r2, [a198] | st r3, [065e]
mul r4, r2, r2 | st r0, [0540] | ld r0, [c088]
st r0, [0530] | ld r0, [a188]
mad r7, r0, #0.12282902002334595 | addxy r0, r6, #0.58141152171439203 | st r2, [065c]
mul r6, r0, r7 | mov r0, r2 | st r1, [0660]
mad r5, r6, #-0.0020813931703287381 | mad r0, r4, #-0.52307693774883568
mul r6, r4, r2 | st r5, [0620]
mac_p1wc3 r4, r2, r2, #-0.85196686811061939, r2 | st r6, [0654]
mad r4, r6, #-0.39337473276942653
mul r4, r3, r1 | st r4, [0652]
mac_p1wc3 r6, r1, r1, #-0.98354070045811071, r1 | st r4, [065a]
mac_p1wc3 r5, r5, r0, #-0.30769230769230771, r7 | ld r7, [a1b8]
mad r6, r4, #-0.38800633351507763 | addxy r5, r7, #-1.25 | st r5, [0656]
mul r6, r5, #8.0 | st r6, [0658]
sign r6, r6
max r4, r6, #0.0
max r6, r6, -r6
mul r6, r6, r5
mul r5, r6, r4 | addxy r4, r7, #0.25
subxy r6, r7, r5 | mul r5, r4, #8.0
sign r7, r5
min r4, r7, #0.0
addxy r5, r6, #-0.25
mul r7, r4, r5
addxy r4, r7, r6
addxy r5, r4, #0.59374997850216871
mul r7, r5, #-5.5451774597167969 | mov r5, r4
floor r6, r7
mad r5, r6, #0.18033687961558437
addxy r7, r5, #0.59374997850216871 | addxy r2, r5, #-1.3899606075128503
mul r7, r7, r7 | mov r3, r5
mul r2, r7, r2
mad r3, r2, #0.27953547022611447
addxy r2, r3, #-1.0292819975560181
mul r3, r2, r7 | mov r2, r5
mad r2, r3, #0.42706930106914331
addxy r3, r2, #-0.6686081920331266
mul r2, r3, r7 | mov r3, r5
mad r3, r2, #0.73211893606984124
addxy r2, r3, #-0.30793450019262536 | addxy r3, r6, #0.69314718055994529
mul r6, r3, #0.0625 | mov r3, #0.041666666666666664
ymacd r3, r6, #0.16666666666666666
ymacd r3, r6, #0.5
ymacd r3, r6, #1.0
ymacd r3, r6, #1.0
mul r6, r3, r3
mul r3, r6, r6
mul r6, r3, r3
mul r3, r6, r6
mul r6, r3, #0.015625
mac_p1wc3 r3, r2, r7, #0.65042786114249618, r5
addxy r2, r3, #0.034303146377230566
mac_p1wc3 r3, r2, r7, #0.1269169056354455, r5
addxy r5, r4, #0.45833373069763184 | mul r2, r4, r4
mul r5, r5, #8.0
sign r5, r5
max r1, r5, #0.0
max r5, r5, -r5
mac_p1wc3 r7, r4, r4, #0.91847825778863046, r4
mul r1, r1, r5 | addxy r5, r7, #0.2108967413509085
addxy r1, r1, #-1.0
mul r5, r5, r1
addxy r1, r5, r7 | mul r5, r4, #8.0
sign r7, r5
min r4, r7, #0.0
addxy r5, r4, #1.0
mul r7, r1, r5 | addxy r5, r3, #0.05246911125984572
subxy r4, r1, r7 | mul r1, r6, r5
mul r7, r1, #-3.2040359716844296
sign r3, r7
absadd r7, r3, #-1.0
mac_p1w r3, r1, r7, r6, r5 | ld r1, [0623] | st r4, [0664]
mov r7, r3 | st r2, [0666]
mad r7, r1, #-0.31210635861690367 | addxy r2, r0, #0.30769230769230771 | st r0, [0668]
mul r5, r7, #-3.2040359716844296
ph_wrap r6, r3, r5
mov r5, r4 | st r6, [0674]
min r1, r4, #0.55758226361485363
mad r5, r6, #-0.33967391238327793
max r7, r1, #-1.0065597769943722
addxy r1, r5, #-0.16711955938562187 | st r7, [0670]
mul r5, r1, #-2.9440000057220459 | st r1, [0662]
sign r5, r5
min r5, r5, #0.0
addxy r4, r5, #1.0 | addxy r1, r5, #1.340482588292577 | st r5, [066c]
mul r2, r4, r2 | mul r1, r1, r5 | st r4, [066e]
subxy r2, r0, r2 | addxy r1, r5, r1
addxy r0, r2, #0.30769230769230771
mul r0, r0, r5
addxy r0, r0, r2 | ld r2, [0625]
mad r2, r0, #-0.001808950473858579 | ld r0, [0627]
mad r0, r1, #-0.093249998986721039 | st r2, [066a]
addxy r2, r0, #-0.125
mul r5, r2, #-8.0
sign r5, r5
max r4, r5, #0.0
max r5, r5, -r5
mul r4, r4, r5
mac_p1wc3 r5, r3, r2, #0.0048016362864139028, r0
addxy r2, r5, #-0.00060020453580173785
mul r0, r4, r2 | addxy r4, r7, #-0.16654675346254721
mul r5, r0, #256.0 | mul r2, r4, #-2.5573125049704686
mad r5, r1, #0.11462466018373105 | st r2, [0672]
min r0, r6, r2
mov r1, r7
shl r4, r3, 9
mad r1, r0, #0.39103551015230648 | mul r0, r4, #-1.6020179858422148
sign r4, r0
mul r0, r3, #-1.0
max r0, r0, -r0
max r0, r0, #2.3811825456001562e-06
max r0, r0, #4.7623650912003123e-06
max r0, r0, #6.6451018797526793e-06
shl r0, r0, 11
mul r0, r0, #1.6020179858422148
recipseed r6, r0
newton r6, r0
newton r6, r0
newton r6, r0
mul r2, r6, r4 | ld r0, [0666] | ld r4, [066e]
addxy r6, r0, #-0.015625 | mul r7, r2, #0.015625
mul r2, r4, r6
subxy r4, r0, r2 | ld r0, [066c]
addxy r6, r4, #-0.015625 | st r3, [066c]
mul r2, r6, r0 | ld r0, [0664]
addxy r6, r2, r4
mad r6, r0, #0.046000000089406967
addxy r2, r6, #-0.0076874997466802588
mul r4, r2, #-64.0
sign r2, r4
mul r4, r6, #-1.0
addxy r6, r4, #0.0076874997466802588
max r4, r6, -r6
max r6, r4, #3.0517578125e-05
max r4, r6, #0.0078125
mul r6, r4, #64.0
recipseed r3, r6
newton r3, r6
newton r3, r6
newton r3, r6
mul r0, r3, r2 | ld r4, [0662]
mul r6, r4, r0
addxy r3, r6, #-0.5095108685749169
mul r2, r3, #2.9440000057220459
ph_wrap r4, r3, r2
absadd r0, r4, #-0.5
mul r6, r0, r7 | addxy r0, r1, #0.094143586638990417
max r3, r6, #-0.0001220703125 | ld r6, [066a]
shl r2, r3, 13
addxy r3, r6, #3.2034623017027206
min r4, r2, #1.0
mul r2, r0, r3 | addxy r0, r5, #0.15365236116524489
mul r1, r2, r4 | ld r4, [066c] | st r0, [0662]
mul r3, r1, r0
mul r2, r3, #5.9384168655410319 | mov r3, r4
st r2, [0630] | ld r2, [0674]
mad r3, r2, #-0.31210635861690367 | st r1, [0674]
mul r2, r3, #-3.2040359716844296
ph_wrap r3, r3, r2 | ld r2, [0664]
mov r1, r2 | st r3, [066e]
mad r1, r3, #-0.33967391238327793 | addxy r2, r7, #-0.00390625
addxy r1, r1, #-0.16711955938562187 | st r2, [0676]
mul r0, r1, #-2.9440000057220459 | st r1, [0666]
sign r0, r0
min r0, r0, #0.0
addxy r1, r0, #1.0
mul r3, r1, r2
subxy r3, r7, r3
mul r2, r3, r0
addxy r3, r3, r2 | mul r2, r1, r0
addxy r1, r0, r2 | mov r2, r3
mad r2, r5, #0.025422648701109364 | mad r6, r1, #-0.0011138206293276069
addxy r1, r2, #0.00390625 | st r6, [066a]
mul r5, r1, #-256.0
sign r5, r5
max r6, r5, #0.0
max r5, r5, -r5
mul r6, r6, r5 | mul r5, r1, r4
shl r1, r5, 10
mad r2, r1, #0.20338118960887491
addxy r5, r2, #0.00390625
mul r1, r6, r5 | ld r6, [066e] | ld r5, [0672]
subxy r2, r3, r1 | ld r1, [0670]
min r3, r6, r5
mov r5, r1
mad r5, r3, #0.39103551015230648 | ld r3, [0664]
mov r1, r3
mad r1, r0, #-0.67934782476655586
addxy r0, r1, #-0.50679347176889977 | mul r1, r1, #-1.0
addxy r1, r1, #0.50679347176889977 | mul r0, r0, #-2.9440000057220459
max r1, r1, -r1
max r1, r1, #0.00066342561012358971
max r1, r1, #0.16983695619163897
mul r1, r1, #2.9440000057220459
recipseed r3, r1
newton r3, r1
newton r3, r1
sign r0, r0
newton r3, r1
mul r6, r3, r0 | ld r4, [0666]
mul r1, r4, r6
addxy r3, r1, #-0.5095108685749169
mul r0, r3, #2.9440000057220459
ph_wrap r4, r3, r0
absadd r6, r4, #-0.5
mul r1, r6, r7 | ld r6, [066a] | st r7, [067c]
max r3, r1, #-0.0001220703125
shl r0, r3, 13
addxy r1, r6, #3.2046905404962507 | addxy r3, r5, #0.094143586638990417
min r4, r0, #1.0
mul r0, r1, r3
mul r5, r0, r2 | ld r0, [066c]
mul r1, r5, r4 | st r5, [0666]
mul r3, r1, #233.45755821837284 | mov r1, r0
st r3, [0634] | ld r3, [066e]
mad r1, r3, #-0.31210635861690367 | st r4, [066e]
mul r3, r1, #-3.2040359716844296
ph_wrap r1, r3, r3 | ld r3, [0664]
mov r4, r3 | ld r3, [0676] | st r1, [067a]
mad r4, r1, #-0.33967391238327793
addxy r4, r4, #-0.16711955938562187
mul r5, r4, #-2.9440000057220459 | st r4, [0678]
sign r5, r5
min r5, r5, #0.0
addxy r4, r5, #1.0
mul r1, r4, r3 | mul r4, r4, r5
subxy r1, r7, r1 | addxy r4, r5, r4
mul r3, r1, r5 | mad r6, r4, #-0.001114441312736953
sum8 r4, r1, r3, r2
addxy r7, r1, r3 | mul r1, r4, #-256.0
sign r3, r1
max r2, r3, #0.0
max r1, r3, -r3
mul r3, r2, r1 | mul r2, r4, r0
shl r1, r2, 10
mad r4, r1, #0.20338118960887491
mul r2, r3, r4 | ld r3, [067a] | ld r4, [0672]
subxy r1, r7, r2 | ld r2, [0670]
min r7, r3, r4 | st r1, [066a] | ld r3, [0678]
mov r4, r2
mad r4, r7, #0.39103551015230648 | ld r7, [0664]
mov r2, r7
mad r2, r5, #-0.67934782476655586
addxy r5, r2, #-0.50679347176889977 | mul r2, r2, #-1.0
addxy r2, r2, #0.50679347176889977 | mul r5, r5, #-2.9440000057220459
max r2, r2, -r2
max r2, r2, #0.00066342561012358971
max r2, r2, #0.16983695619163897
mul r2, r2, #2.9440000057220459
recipseed r0, r2
newton r0, r2
newton r0, r2
sign r5, r5
newton r0, r2
mul r7, r0, r5
mul r1, r3, r7
addxy r2, r1, #-0.5095108685749169
mul r0, r2, #2.9440000057220459
ph_wrap r5, r3, r0
absadd r3, r5, #-0.5
ld r7, [067c]
mul r1, r3, r7 | addxy r3, r6, #3.2059194637334172
max r2, r1, #-0.0001220703125
shl r0, r2, 13
addxy r1, r4, #0.094143586638990417
min r5, r0, #1.0 | ld r0, [066a]
mul r2, r3, r1
mul r4, r2, r0 | ld r2, [066c]
mul r3, r4, r5 | st r4, [0678]
mul r1, r3, #233.32753501166249 | mov r3, r2
st r1, [0638] | ld r1, [067a]
mad r3, r1, #-0.31210635861690367 | st r5, [067a]
mul r1, r3, #-3.2040359716844296
ph_wrap r3, r3, r1 | ld r1, [0664]
mov r5, r1 | ld r1, [0676] | st r3, [0622]
mad r5, r3, #-0.33967391238327793 | st r3, [0676]
addxy r5, r5, #-0.16711955938562187
mul r4, r5, #-2.9440000057220459 | st r5, [067e]
sign r4, r4
min r4, r4, #0.0
addxy r5, r4, #1.0
mul r1, r5, r1 | mul r5, r5, r4
addxy r5, r4, r5 | subxy r1, r7, r1
mad r6, r5, #-0.0011150623420259536 | mul r7, r1, r4
mul r5, r6, #0.99777407723768829 | addxy r3, r1, r7
addxy r5, r5, #-0.0016679411512323834
sum8 r5, r1, r7, r0 | st r5, [0624]
mul r1, r5, #-256.0
sign r7, r1
max r0, r7, #0.0
max r1, r7, -r7
mul r7, r0, r1 | mul r0, r5, r2
shl r1, r0, 10
mad r5, r1, #0.20338118960887491
mul r2, r7, r5 | ld r7, [0676] | ld r5, [0672]
subxy r0, r3, r2 | ld r2, [0670]
mul r1, r0, #32.0
min r3, r7, r5 | st r1, [0626] | ld r1, [0664]
mad r1, r4, #-0.67934782476655586 | mad r2, r3, #0.39103551015230648
addxy r7, r1, #-0.50679347176889977 | mul r4, r1, #-1.0
mul r5, r7, #-2.9440000057220459 | addxy r7, r4, #0.50679347176889977
sign r3, r5
max r5, r7, -r7
max r1, r5, #0.00066342561012358971
max r4, r1, #0.16983695619163897
mul r7, r4, #2.9440000057220459
recipseed r4, r7
newton r4, r7
newton r4, r7
newton r4, r7
mul r1, r4, r3 | ld r5, [067e]
mul r7, r5, r1
addxy r4, r7, #-0.5095108685749169
mul r3, r4, #2.9440000057220459
ph_wrap r5, r3, r3
absadd r1, r5, #-0.5
ld r7, [067c]
mul r4, r1, r7 | addxy r7, r6, #3.2071490717956306
max r3, r4, #-0.0001220703125
addxy r4, r2, #0.094143586638990417
shl r5, r3, 13
mul r3, r7, r4 | ld r7, [0629] | ld r4, [062b]
min r1, r5, #1.0 | st r7, [067c] | ld r7, [0635]
mul r5, r3, r0 | ld r0, [062f] | ld r3, [062d]
mul r6, r5, r1 | st r0, [0676] | ld r0, [063f]
mul r2, r6, #233.19758422083112 | ld r6, [0631] | st r5, [0672]
st r6, [063e] | st r6, [066c] | ld r6, [0641]
st r2, [063c] | st r3, [0664] | ld r5, [063d]
st r1, [0670] | ld r2, [0633] | st r4, [067e]
ld r3, [0639] | ld r1, [063b]
st r0, [062e] | st r6, [066a] | ld r4, [0637]
addxy r0, r0, r5 | addxy r5, r1, r3 | st r5, [0632] | ld r6, [0660]
st r2, [0628] | st r1, [062c]
st r2, [0680] | st r4, [062a]
mov r2, r6 | addxy r1, r4, r7 | st r3, [0636] | ld r6, [065e]
mad r2, r6, #-0.39487129125824577 | ld r3, [0680] | ld r4, [066c]
addxy r7, r3, r4 | st r7, [063a] | ld r3, [0672]
ld r4, [0670] | ld r6, [0676]
mac_p1wc3 r3, r3, r4, #0.0042882090881911968, r6 | st r2, [0676] | ld r4, [0678]
ld r6, [067a] | ld r2, [0664]
mac_p1wc3 r4, r4, r6, #0.0042858207881466565, r2 | st r0, [0664] | ld r6, [0666]
ld r2, [066e] | ld r0, [067e]
mac_p1wc3 r6, r6, r2, #0.0042834338182557979, r0 | st r5, [067e] | ld r2, [0674]
ld r0, [0662] | ld r5, [067c]
mac_p1wc3 r2, r2, r0, #0.16839504915909823, r5 | ld r0, [067e] | ld r5, [066a]
mad r6, r2, #0.016600337846586922 | ld r2, [0664]
mad r4, r6, #0.5060008047626724 | ld r6, [0676]
mad r4, r7, #0.011177478629099007 | addxy r7, r6, #0.49510531013321074 | ld r6, [065e]
mad r3, r4, #0.57449937643883475
mad r3, r1, #0.0084897857990424621 | ld r1, [065c]
mad r3, r0, #0.01012207263263897
mad r3, r5, #0.089724680981883714
mad r3, r2, #0.011020794636912121 | ld r2, [0654]
mac_p1wc3 r0, r1, r1, #-0.77351390236857831, r1
mul r4, r7, r3 | mad r0, r2, #-0.42889389207119188 | ld r7, [0660] | ld r2, [0668]
mad r5, r4, #-0.028508363739871666 | mad r7, r6, #-0.45717497885030173 | ld r4, [065a]
mul r6, r5, r2 | mad r7, r4, #0.11104427371485703 | st r5, [0640] | ld r5, [0652] | ld r4, [c060]
mad r3, r6, #-0.29160521319112204 | addxy r2, r5, #0.26397516033834412 | ld r6, [0658]
addxy r5, r6, #-3.3439236656300895
mul r6, r2, r5 | ld r2, [0656]
mul r5, r6, r2 | ld r2, [c061]
mul r6, r5, r4 | addxy r5, r0, #0.21820918763667807
addxy r4, r7, #2.2522163013075054
mul r0, r5, r4 | ld r4, [a1c8]
mul r7, r0, r2 | ld r0, [0643] | ld r2, [0645] | ld r5, [a1c0]
mac_p1wc3 r7, r7, r3, #0.11052250680982215, r6
mac_p1wc3 r3, r0, r4, #-0.125, r2
mad r7, r3, #-0.62783472121947903
mac_p1wc3 r6, r7, r5, #-0.060719641037299113, r0 | ld r0, [0647]
mac_p1wc3 r3, r6, r5, #-0.046978922102775482, r2 | ld r2, [0649]
mul r4, r6, #-16.469135569917416 | mul r7, r3, #-21.286141853410484
mad r3, r0, #0.046978922102775482 | st r4, [0642]
mac_p1wc3 r4, r3, r5, #-0.036347697124607663, r0 | st r7, [0644] | ld r7, [a1d0]
mul r6, r4, #-27.512059335472795 | mad r4, r2, #0.0022717310702879789 | ld r0, [064b] | st r7, [0656]
mad r2, r4, #-0.13252583666178999 | st r6, [0646] | ld r6, [064d]
addxy r2, r0, r6 | mov r7, r4 | st r2, [0648] | ld r3, [a1d8]
mad r7, r2, #0.018173848562303831 | ld r5, [a1e0]
mac_p1wc3 r2, r1, r4, #0.55257456844603692, r7 | ld r7, [064f]
mul r1, r5, r3 | mad r0, r2, #-0.46322009463608405
max r4, r1, #0.031863559037446976 | st r0, [064a]
mad r4, r7, #-0.48980201941689522 | mad r6, r0, #0.0046518445014953613
addxy r5, r4, #-0.031863559037446976 | mad r2, r0, #-0.0014572176733639532 | st r6, [064c] | ld r0, [0651]
mul r3, r5, #4.083282470703125 | mad r2, r6, #0.0054960866489644732 | ld r6, [0656]
sign r1, r3
mov r3, r4
min r5, r1, #0.0
mad r3, r7, #-0.49600245812738197 | addxy r1, r5, #1.0
addxy r3, r3, #-0.031863559037446976 | mad r7, r4, #0.0051044632946286583
mul r1, r1, r3 | mov r3, r4
mad r3, r1, #-0.9874991774559021 | mad r6, r0, #0.5
mov r1, r3
mad r1, r7, #-0.48980201941689522 | mad r0, r6, #-0.79960823059082031
addxy r4, r1, #-0.03178389451817365
mul r7, r4, r5
addxy r1, r7, r3 | mul r7, r6, #-2.0
addxy r4, r1, #-0.031863559037446976
mul r5, r4, #2.0416412353515625
sign r3, r7 | st r5, [064e]
min r4, r3, #0.0
mov r5, r6
mad r5, r0, #0.5 | addxy r7, r4, #1.0
mul r3, r7, r5
subxy r7, r6, r3
mov r5, r7
mad r5, r0, #-0.5
mul r6, r5, r4
addxy r3, r6, r7
mad r1, r3, #0.48980201941689522 | mul r0, r3, #2.0
addxy r5, r1, #-0.031863559037446976 | st r0, [0650] | ld r0, [c056]
mul r4, r2, r5 | ld r1, [c054]
mul r6, r4, #240.49570763245896 | ld r2, [c055]
hset h6, r0
st r6, [0400]
ldidx r3, 0
ymacd r3, r2, r1 | ld r0, [c05c]
hset h6, r0
st r3, [a138] | ld r1, [c05a]
ld r2, [c05b] | ldidx r3, 0
ymacd r3, r2, r1
st r3, [a140]
