;; EDM_ECF (docs/MUTABLE_MEMORY.md §3.3.4): our own lines in the delay memory, written and read by plain bank-2
;; transfers from our master B main (0x1000). Which bank-2 word reaches which delay-memory address is not in the
;; program: it is the DSP's map table (master B 0x4016E000, 8 bytes per entry, OS 1.30 puts 34 entries there for its
;; delay and reverb). Region 0 (model.c edm_ecf) adds one write entry and one read entry per line:
;;   line 1: [ECF_W1] -> address ECF_A1, [ECF_W1 + 1] <- ECF_A1 + ECF_D1 (that many samples older)   -> left
;;   line 2: [ECF_W2] -> address ECF_A2, [ECF_W2 + 1] <- ECF_A2 + ECF_D2                              -> right
;; Both lines take this sample's send (the dry mix) and give back the line's output, ECF_GAIN x, added to the OS
;; effect's return; no feedback (one echo per line).

.fragment master_ecf ECF_W1 ECF_W2 ECF_GAIN
ld r0, [{OUT_L:04x}]
ld r1, [{OUT_R:04x}]
ld r7, [c{M_VOLUME:03x}]
mov r4, r4
mul r0, r0, r7
mul r1, r1, r7
mov r4, r4
st r0, [{FX_SEND[0]:04x}]
st r1, [{FX_SEND[1]:04x}]
ld r2, [{FX_RET[0]:04x}]
ld r3, [{FX_RET[1]:04x}]
st r0, [{ECF_W1:04x}]
st r0, [{ECF_W2:04x}]
ld r4, [{ECF_W1 + 1:04x}]
ld r5, [{ECF_W2 + 1:04x}]
mov r7, r7
mul r4, r4, #{ECF_GAIN}
mov r7, r7
mul r5, r5, #{ECF_GAIN}
mov r7, r7
addxy r2, r2, r4
mov r7, r7
addxy r3, r3, r5
mov r7, r7
st r2, [{M_OUT[0]:04x}]
st r3, [{M_OUT[1]:04x}]
