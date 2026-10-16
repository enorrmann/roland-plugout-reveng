"""Indexed transfers (mode byte 28) on master DSP3: markers in bank 0 [03D0..03D7], h6 set by `0b02 4080`, then
`0c02 028Y` loads / `8280` stores; results in bank 5 (DPC3 RAW).

    python3 dspidx.py
"""
import sys, time
import os; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspmaster, dspasm as A
dspmaster.CONTEXT='dsp3'
NOP=[1]
O=0xA3C0; RD=0x41122000
def mk(cases):
    P=[[0x0A02,0xFF70]]
    for j in range(8):
        P+=[A.a('move',1,imm=-(1+j/8)),A.st(1,0x03D0+j)]
    for k,c in enumerate(cases):
        P+=c+[NOP,NOP,A.st(0,O+k)]
    P+=[[0x0A02,0xFF70],[0x0B02,0x00F8],[1]]
    return [w for i in P for w in i]
h6=lambda a: [A.a('move',0,imm=a/16384),NOP,[0x0B02,0x4080]]
x9b=lambda port,a: [A.a('move',3,imm=a/16384),NOP,[0x1803,0x0040,0xB9B0|port],NOP,NOP]
C={'h6=3d2; 0280':h6(0x3d2)+[[0x0C02,0x0280]],
   'h6=3d2; 0281':h6(0x3d2)+[[0x0C02,0x0281]],
   'h6=3d2; 0282':h6(0x3d2)+[[0x0C02,0x0282]],
   'h6=3d2; 0283':h6(0x3d2)+[[0x0C02,0x0283]],
   'h6=3d2; b9b1 3d4; 0281':h6(0x3d2)+x9b(1,0x3d4)+[[0x0C02,0x0281]],
   'h6=3d2; b9b1 3d4; 0280':h6(0x3d2)+x9b(1,0x3d4)+[[0x0C02,0x0280]],
   'h6=3d2; b9b2 3d5; 0282':h6(0x3d2)+x9b(2,0x3d5)+[[0x0C02,0x0282]],
   'h6=3d2; b9b3 3d6; 0283':h6(0x3d2)+x9b(3,0x3d6)+[[0x0C02,0x0283]],
   'h6=3d2 frac .5; 0280':h6(0x3d2+0.5)+[[0x0C02,0x0280]],
   'h6=3d2; r0=7; 8280; ld[3d2]':h6(0x3d2)+[A.a('move',0,imm=7.0),[0x0C02,0x8280],NOP,A.ld(0,0x3d2)],
   }
with dspmaster.Session(Peek(),size=200) as ses:
    ses.load(mk(list(C.values()))); time.sleep(0.3)
    r=ses.pk.read(RD+4*(O&0x3FF),len(C))
    for n,x in zip(C,r): print(f'{n:30s} r0 = {raw_value(x):+.6g}')
