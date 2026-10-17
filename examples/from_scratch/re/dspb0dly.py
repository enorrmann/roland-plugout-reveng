"""Slave test of the indexed bank-0 delay (DSP_USOS §2): out = (saw + saw delayed k samples)/2.

    python3 dspb0dly.py 92 out.txt; python3 dspown.py load out.txt; python3 dspharm.py dly 72   # 2026-10-17: f0 x 2
"""
import math,sys; import os; sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
import dspasm as A
from dspflagsaw import cmp_imm, jump_unless_ge
NOP=[1]; U=1/16384
k=int(sys.argv[1]); out=sys.argv[2]
SV={'A':0x700,'B':0x780}
P=[[0x0A02,0xFF70]]
for voice in 'AB':
    pin,outs,state=A.SLOTS['dco2'][voice]
    P+=[A.ld(1,state+1),A.ld(0,pin),A.a('add',0,0,imm=-4.75),A.a('mul',2,0,imm=math.log(2)/16),
        A.a('move',3,imm=1/24),A.a('ymacd',3,2,imm=1/6),A.a('ymacd',3,2,imm=0.5),
        A.a('ymacd',3,2,imm=1.0),A.a('ymacd',3,2,imm=1.0)]
    P+=[A.a('mul',3,3,3)]*4+[A.a('mul',3,3,imm=2*440/96000)]
    P+=[A.a('add',1,3,1),cmp_imm(1,-1.0),NOP]
    sub=A.a('add',1,1,imm=-2.0,bits=16)
    P+=[jump_unless_ge(len(sub)),sub,A.st(1,state),A.st(1,SV[voice])]
    P+=[A.a('move',0,imm=(SV[voice]+k)*U),NOP,[0x0B02,0x4080],NOP,[0x0C02,0x2280],NOP,NOP,
        A.a('add',2,2,1),NOP,A.a('mul',2,2,imm=0.5),NOP,A.st(2,outs[0]),A.st(2,outs[1])]
P+=[[0x0A02,0xFF70],[0x0B02,0x00F8],[1]]
open(out,'w').write(A.text(P,f'indexed bank-0 delay test k={k}'))
