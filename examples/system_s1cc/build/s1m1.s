mov r7, #0.0 | ld r0, [0401] | ld r2, [069d] | ld r3, [c090]
addxy r7, r7, r0 | mov r0, #0.0 | ld r1, [0601] | st r3, [a120]
mul r2, r2, #7.31275 | mul r7, r7, #1.3333333333333333 | st r0, [6008]
st r0, [6009]
st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d] | ld r0, [069f]
add r0, #0.6184303127229214:32 | ld r6, [c000]
ph_wrap r0, r3, r0
addxy r2, r2, r0 | mul r7, r7, r6 | st r0, [069e] | ld r0, [a168]
subxy r1, r1, r0 | st r0, [0600] | ld r0, [c098]
mul r1, r1, r1 | st r7, [6006]
ph_wrap r2, r3, r2 | st r1, [060e] | ld r1, [c050]
mul r1, r3, r1 | st r2, [069c] | ld r2, [c051]
mad r1, r0, -r2 | ld r2, [0613] | ld r0, [0611]
add r1, #1.0 | st r7, [6007]
st r1, [060a] | ld r1, [c04f]
mul r1, r3, r1 | ld r3, [c053]
st r1, [060c] | ld r1, [060e]
mad r2, r2, -r1 | addxy r1, r0, #-0.999
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
mul r1, r1, #256.0
ph_clamp r1, r3, -r1
max r2, r2, r1 | ld r1, [c052]
subxy r2, r2, #1.0 | subxy r3, r1, r3 | st r2, [0612]
mul r2, r2, #256.0
ph_clamp r2, r3, -r2
mad r1, r2, r3 | mul r2, r2, #1.25
subxy r2, r0, r2
mad r0, r2, r1
min r0, r0, #1.0
st r0, [0610] | st r0, [a128]
ld r0, [a188]
addxy r1, r0, #-0.59374997850216871
mul r2, r1, #5.5451774597167969
floor r3, r2
mad r0, r3, #-0.18033687961558437
addxy r4, r0, #-0.59374997850216871 | addxy r6, r0, #1.3899606075128503
mul r5, r4, r4 | mov r1, r0
mul r7, r5, r6 | mov r6, r0
mad r1, r7, #0.27953547022611447
addxy r2, r1, #1.0292819975560181
mul r4, r2, r5 | mov r2, r0
mad r6, r4, #0.42706930106914331
addxy r7, r6, #0.6686081920331266 | addxy r6, r3, #-0.69314718055994529
mul r1, r7, r5
mad r2, r1, #0.73211893606984124 | mov r1, #0.0022679399982700001
expsplit r7, r6
ymacd r1, r6, #0.0070657335609299998
ymacd r1, r6, #0.042538763698700001
ymacd r1, r6, #0.16636075598899999
ymacd r1, r6, #0.50005197393800005
ymacd r1, r6, #0.999996450303
addxy r4, r2, #0.30793450019262536
ymacd r1, r6, #1.00000005659
mac_p1wc3 r6, r4, r5, #0.65042786114249618, r0
mul r2, r1, r7 | addxy r1, r6, #-0.034303146377230566
mac_p1wc3 r7, r1, r5, #0.1269169056354455, r0
mul r3, r2, #0.00390625 | addxy r2, r7, #-0.05246911125984572
mul r4, r3, r2
mul r1, r4, #25.632287773475436
min r6, r4, #0.0097143103328884255
sign r5, r1
absadd r0, r5, #-1.0
mul r7, r6, r0 | ld r1, [0615]
addxy r3, r7, r6
mov r5, r3 | mul r2, r3, #25.632287773475436
mad r5, r1, #0.039013294827112958
ph_wrap r4, r3, r2
mul r0, r5, #25.632287773475436
ph_wrap r7, r3, r0
sign r6, r4
min r2, r6, #0.0 | shl r6, r3, 12
subxy r1, r7, r4
mul r5, r2, r1 | mul r2, r6, #1.6020179858422148
subxy r0, r7, r5
max r7, r3, -r3
max r5, r7, #2.9764781820001952e-07
max r6, r5, #2.3716577961153382e-06
sign r1, r2
shl r2, r6, 12
mul r7, r2, #1.6020179858422148 | mul r2, r0, #0.5
recipseed r5, r7
newton r5, r7
newton r5, r7
newton r5, r7
addxy r6, r2, #-2.0 | mul r2, r1, r5
ph_wrap r7, r3, r6
mul r6, r2, #0.001953125
absadd r1, r7, #-0.5
mul r5, r1, r6 | st r6, [066e]
max r2, r5, #-1.52587890625e-05 | st r3, [066c]
shl r7, r2, 15
shl r1, r7, 1
min r5, r1, #1.0
mul r2, r5, r5
addxy r1, r2, #-29.32148121184904 | mul r7, r2, r5
mul r1, r1, r2
mul r1, r1, #0.03125
addxy r1, r1, #15.600957895328319
mul r1, r1, r2
addxy r2, r1, #-126.45629034055113
mul r1, r7, r2 | mul r2, r0, #-1.0
mul r7, r1, #0.0078125
ph_wrap r1, r3, r2
mad r5, r7, #0.41625187771182148 | subxy r7, r0, r1
mov r1, r3 | mul r2, r7, r5
mad r1, r4, #0.039013294827112958
mov r4, r3 | mul r7, r1, #25.632287773475436 | ld r3, [061d]
mad r4, r0, #-0.039013294827112958 | ld r0, [0617]
mul r1, r4, #25.632287773475436 | mov r6, r0 | ld r4, [0619]
ph_wrap r5, r3, r7
ph_wrap r7, r3, r1 | ld r1, [061b]
mad r6, r4, #0.017333100814576129
mad r6, r1, #0.034666201629152259
mad r6, r3, #-0.50126103797649757
mad r6, r2, #-0.013613382825177921
mul r4, r6, #1.5688642263412476
sign r3, r7 | st r4, [0626]
min r4, r3, #0.0
subxy r3, r7, r5
mul r4, r4, r3
subxy r3, r7, r4
mul r7, r3, #0.5
addxy r4, r7, #-2.0
ph_wrap r7, r3, r4
absadd r4, r7, #-0.5
ld r7, [066e] | st r2, [0672] | st r6, [0670]
mul r4, r4, r7 | mad r0, r1, #-0.034578990788006983
max r4, r4, #-1.52587890625e-05
shl r4, r4, 15
shl r4, r4, 1
min r4, r4, #1.0
mul r7, r4, r4
addxy r2, r7, #-29.32148121184904 | mul r6, r7, r4
mul r2, r2, r7
mul r2, r2, #0.03125
addxy r2, r2, #15.600957895328319
mul r2, r2, r7
addxy r7, r2, #-126.45629034055113
mul r2, r6, r7 | mul r7, r3, #-1.0
mul r6, r2, #0.0078125
ph_wrap r2, r3, r7
mad r4, r6, #0.41625187771182148 | subxy r6, r2, r3 | ld r2, [066c]
mul r7, r6, r4 | mov r6, r2
mad r6, r5, #0.039013294827112958
mul r4, r6, #25.632287773475436 | mov r6, r2
mad r6, r3, #-0.039013294827112958
ph_wrap r5, r3, r4
mul r4, r6, #25.632287773475436 | ld r6, [0672]
ph_wrap r3, r3, r4 | ld r4, [0670]
mad r0, r6, #0.054316540859160913
mov r1, r4
mad r1, r0, #-0.31950568415055886
mad r1, r7, #0.0086772217739489958
mul r0, r1, #2.4613349606933212 | subxy r1, r3, r5 | st r1, [0674]
sign r0, r3 | st r0, [062a]
min r0, r0, #0.0
mul r0, r0, r1
subxy r1, r3, r0
mul r3, r1, #0.5
addxy r0, r3, #-2.0
ph_wrap r3, r3, r0
absadd r0, r3, #-0.5
ld r3, [066e] | st r7, [0676]
mul r0, r0, r3
max r0, r0, #-1.52587890625e-05
shl r0, r0, 15
shl r0, r0, 1
min r0, r0, #1.0
mul r3, r0, r0
addxy r6, r3, #-29.32148121184904 | mul r7, r3, r0
mul r6, r6, r3
mul r6, r6, #0.03125
addxy r6, r6, #15.600957895328319
mul r6, r6, r3
addxy r3, r6, #-126.45629034055113
mul r6, r7, r3 | mul r3, r1, #-1.0
mul r7, r6, #0.0078125
ph_wrap r6, r3, r3
mad r0, r7, #0.41625187771182148 | subxy r7, r6, r1
mul r3, r7, r0 | mov r7, r2
mad r7, r5, #0.039013294827112958 | mul r6, r3, #0.78539800643920898
mul r0, r7, #25.632287773475436 | mad r2, r1, #-0.039013294827112958 | st r6, [0618] | ld r1, [0672]
ph_wrap r6, r3, r0
mul r5, r2, #25.632287773475436 | mov r0, r4 | ld r2, [0676]
ph_wrap r7, r3, r5 | ld r5, [0674]
mad r0, r1, #0.017310784434747633 | subxy r6, r7, r6
mad r0, r2, #-0.034621568869495266
mov r1, r5
mad r1, r0, #-0.31950568415055886
mad r1, r3, #0.0055308940240068882
mul r0, r1, #3.8615003688747924
st r0, [061c] | st r0, [062e]
sign r0, r7
min r0, r0, #0.0
mul r0, r0, r6
subxy r6, r7, r0
mul r0, r6, #0.5 | mul r7, r6, #-1.0
addxy r0, r0, #-2.0 | st r7, [0614]
ph_wrap r0, r3, r0
absadd r0, r0, #-0.5
ld r4, [066e] | st r3, [0672]
mul r0, r0, r4 | st r1, [066e]
max r4, r0, #-1.52587890625e-05
shl r0, r4, 15
shl r4, r0, 1
min r0, r4, #1.0
mul r4, r0, r0
addxy r3, r4, #-29.32148121184904 | mul r1, r4, r0
mul r3, r3, r4
mul r3, r3, #0.03125
addxy r3, r3, #15.600957895328319
mul r3, r3, r4
addxy r4, r3, #-126.45629034055113
mul r3, r1, r4
mul r1, r3, #0.0078125
ph_wrap r4, r3, r7
mad r0, r1, #0.41625187771182148 | subxy r3, r4, r6 | ld r6, [0672]
mov r4, r5 | mul r1, r3, r0 | ld r3, [066e] | ld r5, [062d]
mad r4, r2, #-0.011033959563931266 | mul r7, r1, #0.78539800643920898 | ld r2, [061f] | st r5, [0620]
mad r4, r6, #-0.022067919127862531 | st r7, [061a] | ld r6, [0621]
mov r0, r3 | st r6, [0676] | ld r3, [0631]
mad r0, r4, #-0.31950568415055886 | ld r4, [0623] | ld r6, [062b]
mad r0, r1, #0.0035254127993634612 | ld r1, [0625] | st r4, [066c]
mul r7, r0, #6.0581697887311936 | st r1, [067a] | ld r1, [0635]
st r0, [0678] | ld r0, [0633] | st r1, [0624]
ld r4, [062f] | st r1, [068a] | st r0, [0642]
st r0, [0688] | ld r1, [0645] | st r7, [0616]
st r7, [0632] | st r2, [0672] | ld r0, [0643]
st r4, [063e] | st r3, [0622] | ld r7, [0627]
st r4, [0684] | st r3, [0686] | ld r2, [0629]
st r6, [063a] | st r6, [0680] | ld r4, [063f]
st r5, [0682] | ld r3, [0641] | st r1, [0634]
st r1, [069a] | ld r6, [063b] | st r0, [0652]
ld r5, [063d] | st r0, [0698] | st r7, [0636]
ld r1, [0655] | st r2, [061e] | st r7, [067c]
st r2, [067e] | st r4, [064e] | ld r0, [0653]
st r3, [0630] | st r4, [0694] | ld r7, [0637]
st r3, [0696] | ld r2, [0639] | st r6, [064a]
st r5, [062c] | st r6, [0690] | ld r4, [064f]
st r5, [0692] | ld r3, [0651] | st r1, [0644]
addxy r1, r1, r0 | ld r6, [064b] | ld r5, [064d]
st r0, [0648] | st r7, [0646]
st r2, [0628] | st r7, [068c] | ld r7, [0647]
st r2, [068e] | st r3, [0640] | ld r2, [0649]
addxy r0, r3, r4 | addxy r3, r5, r6 | st r6, [0650] | ld r6, [0698]
st r5, [063c] | st r4, [064c] | ld r5, [069a]
addxy r4, r2, r7 | addxy r2, r5, r6 | st r2, [0638] | ld r5, [0694]
st r7, [0654] | ld r7, [0696]
addxy r6, r7, r5 | ld r7, [0692] | ld r5, [0690]
addxy r7, r7, r5 | st r1, [0690] | ld r5, [068e]
ld r1, [068c]
addxy r5, r5, r1 | st r0, [068c] | ld r1, [068a]
ld r0, [0688]
addxy r1, r1, r0 | st r3, [0688] | ld r0, [0686]
ld r3, [0684]
addxy r0, r0, r3 | st r4, [0684] | ld r3, [0682]
ld r4, [0680]
addxy r3, r3, r4 | st r2, [0680] | ld r4, [067e]
ld r2, [067c]
addxy r4, r4, r2 | st r6, [067c] | ld r2, [067a]
ld r6, [0678]
mad r6, r2, #0.16506635417516702 | ld r2, [066c]
st r6, [0678] | ld r6, [066e]
mad r6, r2, #0.25896669803799377 | st r7, [066e] | ld r2, [0676]
ld r7, [0674]
mad r7, r2, #0.40628358836552458 | st r5, [0674] | ld r2, [0672]
ld r5, [0670]
mad r5, r2, #0.63740378753622462 | ld r2, [0674]
mad r7, r5, #0.22097782018591347 | ld r5, [066e]
mad r7, r4, #-0.52936917562544605 | ld r4, [0678]
mad r3, r7, #-0.53718654841981373 | ld r7, [067c]
mad r0, r3, #0.70592774070452735 | ld r3, [0680]
mad r0, r6, #-0.83812802180719204 | ld r6, [0684]
mad r0, r1, #0.57264643845884444 | ld r1, [0688]
mad r0, r2, #-0.77726307498973901 | ld r2, [068c]
mad r5, r0, #-0.39701488105087301 | ld r0, [0690]
mad r5, r4, #0.32347846074374453
mad r7, r5, #0.74669904684126054 | ld r5, [a190]
mad r7, r3, #0.56011136001431161 | ld r3, [c064]
mad r7, r6, #-0.77927501268892685 | ld r6, [0657]
mad r1, r7, #-0.35824365072440822 | ld r7, [c065]
mad r2, r1, #0.57750262536394703
mad r0, r2, #0.78962470663225681 | ld r2, [c066]
mac_p1w r1, r7, r5, r6, r3 | st r2, [068c] | ld r2, [0659] | ld r3, [c061]
mul r4, r0, #0.24270382523536682 | mad r1, r2, #-0.125 | ld r5, [a1c8]
mad r2, r1, #0.00059102423256263137 | st r4, [0656] | ld r4, [a1b8] | ld r6, [c062]
addxy r5, r5, #-4.0 | st r1, [0688] | ld r1, [c067] | ld r7, [a1c0]
mac_p1wc3 r3, r3, r4, #0.31251613133965372, r1 | st r7, [0690] | ld r0, [a198]
mac_p1wc3 r4, r6, r5, #0.3999793529510498, r3 | ld r7, [c063] | st r2, [0658]
mac_p1wc3 r1, r7, r0, #-0.098462174098038355, r4 | ld r2, [c060]
mad r1, r2, #-0.52308029989582872
addxy r5, r1, #1.6399421222646811 | mul r6, r1, r1
mul r3, r5, r1 | addxy r4, r1, #0.54154195753921097
addxy r7, r3, #0.88853660408730595 | mul r2, r4, #-20.312368869781494
mac_p1wc3 r0, r6, r7, #0.156076457093833, r1
sign r5, r2
mov r6, r0
min r3, r5, #0.0
mad r6, r1, #0.01283360659695975 | addxy r2, r3, #1.0
mul r7, r6, r3
addxy r4, r7, r0
mad r0, r4, #-0.25
mac_p1wc3 r6, r0, r2, #0.25, r4
mul r2, r1, #1.9073486328125e-06
mul r4, r2, #1.6821048271655761
max r7, r5, #0.0
max r3, r5, -r5
addxy r5, r6, r4 | mul r0, r7, r3
addxy r7, r5, #-0.0017357466479611457
mul r3, r0, r7
subxy r1, r6, r3
shl r2, r1, 9
min r4, r2, #0.24261596295467222
addxy r5, r4, #0.38414192743631892
mul r0, r5, #-8.570918096524009
floor r7, r0
mad r4, r7, #0.11667361521113548
addxy r6, r4, #0.38414192743631892 | addxy r1, r4, #-0.89927101669544385
mul r3, r6, r6 | mov r5, r4
mul r2, r3, r1
mad r5, r2, #0.66782198922234748
addxy r0, r5, #-0.66592064797056505
mac_p1wc3 r6, r0, r3, #0.98011684684848221, r4
addxy r1, r6, #-0.42397246468377003 | addxy r0, r7, #0.69314718055994529
mac_p1wc3 r2, r1, r3, #0.56036683344886928, r4
mul r6, r0, #0.0625 | mov r1, #0.041666666666666664
ymacd r1, r6, #0.16666666666666666
ymacd r1, r6, #0.5
ymacd r1, r6, #1.0
ymacd r1, r6, #1.0
addxy r5, r2, #-0.11163975526919394 | mul r2, r1, r1
mul r7, r2, r2
mac_p1wc3 r2, r5, r3, #0.1525624896003642, r4
mul r0, r7, r7 | addxy r7, r2, #0.0052055987791555227
mul r6, r0, r0 | mul r0, r7, r3
mul r1, r6, #0.0078125
shl r6, r0, 9
mad r4, r6, #0.15674212287947156
addxy r5, r4, #0.26746831222518341
mul r2, r1, r5
mul r7, r2, r2
mul r3, r7, r7 | mul r6, r7, r2
shl r0, r3, 11
mac_p1wc3 r3, r2, r2, #-2.6314348072331516e-05, r0
mul r4, r6, #256.0 | addxy r6, r3, #-10.774741628290871
mul r3, r6, r7
addxy r6, r3, #55.269758217331066
mul r3, r4, r6
mul r4, r3, #0.015625 | addxy r3, r7, #-26.67382429333388
mac_p1wc3 r6, r1, r5, #-0.0029393248755641135, r4
mul r1, r3, r0
mul r5, r1, #0.03125
mac_p1wc3 r4, r2, r2, #4.0775713015514925e-05, r5
addxy r7, r4, #-7.6945925365761161
mul r3, r7, r0
mac_p1wc3 r1, r2, r2, #2.0618203122164245e-05, r3
mul r0, r1, #-1.0 | addxy r5, r1, #-0.51295815948718992
addxy r2, r0, #0.51295815948718992 | mul r4, r5, #-1.9494767389989687
max r3, r2, -r2
max r5, r3, #0.0010018714052484178
sign r7, r4
max r4, r5, #0.25647907974359496
mul r1, r4, #1.9494767389989687
recipseed r2, r1
newton r2, r1
newton r2, r1
newton r2, r1
mul r0, r2, r7
mul r5, r0, r6
mul r2, r5, #-1.0 | addxy r4, r5, #-0.50643763657887331
addxy r7, r2, #0.50643763657887331 | mul r3, r4, #-1.9745767845282536
max r0, r7, -r7
max r6, r0, #0.00098913600894311193
max r4, r6, #0.50643763657887331
sign r1, r3
mul r3, r4, #1.9745767845282536
recipseed r6, r3
newton r6, r3
newton r6, r3
newton r6, r3
mul r2, r6, r1 | ld r6, [068c]
mul r0, r2, r5
mul r4, r0, r0
mul r7, r4, r4
shl r3, r7, 12
mul r1, r3, r6
addxy r7, r1, #3.118532561918824 | mul r2, r1, #0.32066363911387313
max r3, r7, -r7
addxy r5, r2, #1.0
max r2, r3, #0.0060908839099977031
sign r4, r5
max r5, r2, #3.118532561918824
mul r1, r5, #0.32066363911387313 | ld r5, [0688]
recipseed r2, r1
newton r2, r1
newton r2, r1
newton r2, r1
mul r7, r2, r4 | ld r2, [065b] | ld r4, [065d]
mad r2, r4, #0.10000000149011612 | mul r3, r5, r7 | st r2, [066a]
min r5, r2, #13.33333280351429
mul r1, r7, r6
max r7, r5, #-13.33333280351429
mac_p1wc3 r4, r1, r7, #-0.29629627471107184, r3
max r2, r4, #-2.4691367509110771
min r5, r2, #2.4691367509110771
mul r1, r5, r5
mul r7, r1, r5 | ld r1, [065f]
min r3, r7, #0.044101884047646804
max r4, r3, #-0.044101884047646804
mad r5, r4, #-0.054674905854509848 | ld r4, [0661]
mov r7, r5 | mul r2, r5, #-1.6199993777274813
mad r7, r1, #-0.61728418772776927 | addxy r2, r0, #0.12660940914471833 | st r2, [065e]
mul r3, r0, r7
mac_p1wc3 r1, r2, r4, #0.80999968886374063, r3
mul r7, r1, #7.8983071381130143 | mov r3, r1
mad r3, r4, #0.12660940914471833 | st r7, [0660] | ld r7, [0663]
mul r4, r2, r7
mac_p1wc3 r3, r3, r0, #-0.12660940914471833, r4
mul r4, r3, #-62.383255647966998
mov r4, r3 | st r4, [0662]
mad r4, r7, #-0.016029942483974685
mul r7, r4, r0
mul r4, r7, #256.0 | ld r7, [0665]
mac_p1wc3 r4, r2, r7, #0.12184229618743554, r4
mul r6, r4, #7.8983071381130143
mul r6, r0, r5 | mov r5, r1 | st r6, [0664]
mad r5, r6, #0.80999968886374063
mac_p1wc3 r6, r1, r2, #0.12660940914471833, r5
mul r1, r6, r0 | mov r6, r3
shl r5, r1, 9
mul r1, r3, r2 | mad r6, r5, #0.015426381129126981
shl r5, r1, 9
mad r6, r5, #0.015426381129126981
mul r3, r6, r0 | mov r5, r4
mul r1, r3, #256.0 | mov r6, r4 | ld r3, [0667]
mad r5, r1, #0.12184229618743554 | mad r6, r7, #0.12660940914471833
mul r1, r2, r3
mac_p1wc3 r7, r6, r0, #-0.25321881828943665, r1
mac_p1wc3 r6, r4, r2, #0.12660940914471833, r5
mul r3, r7, #-31.191627823983499 | mul r1, r6, r0
mul r4, r7, r2 | mul r3, r1, #256.0 | st r3, [0666] | ld r1, [0669]
mul r5, r4, #256.0
addxy r6, r5, r3 | ld r3, [0690]
mul r0, r6, #-0.96234787770130226 | mul r6, r3, r3
max r2, r1, #-0.0076105840931104994 | st r0, [065a]
addxy r0, r6, #41.912717390729171 | mul r4, r2, #0.9924468994140625 | ld r6, [068c]
mul r1, r0, r3 | addxy r5, r4, #0.0075531005859375
mul r4, r1, #0.015625 | mul r0, r7, r6 | st r5, [0668]
max r5, r4, #0.17587591425014287
mad r7, r0, #0.5 | addxy r3, r5, #-0.17587591425014287 | ld r5, [066b]
mul r1, r7, r3 | addxy r6, r2, #0.0076105840931104994 | st r5, [065c]
max r4, r1, #-49.619343008612816
mul r0, r6, r4
mul r7, r0, #-79.204778997710079
st r7, [0400]
