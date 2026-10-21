; gate, DSP4 DCO1 slot. Saw from DSP2 (MXM 8005) times the note gate, to IOMD 6020 / 6021. Own asm: see make_chain.py
ld r4, [a022]
ld r1, [8005]
mov r4, r4
mul r1, r1, r4
st r1, [6020]
st r1, [6021]
