"""Fixed table in bank 0 of idle master DSP3, one program; the write block is switched off by a jump
(one 32-bit word changes). h6 = T + step*n, n = bank-5 sample counter.

    python3 dspb0fix.py 1      # 2026-10-17: table kept 3 s; one plain bank-0 store wipes it (< 10 ms)
"""
import sys,time; import os; sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspmaster, dspasm as A
dspmaster.CONTEXT='dsp3'
NOP=[1]; CNT=0xA3F0; O=0xA3C0; RD=0x41122000
U=1/16384
def prog(T, step, skip, extra=False):
    P=[[0x0A02,0xFF70],A.ld(1,CNT),NOP,A.a('add',1,1,imm=step*U),A.st(1,CNT),
       A.a('add',0,1,imm=T*U), NOP,[0x0B02,0x4080],NOP]
    W=[]
    for j in range(8):
        W+=[A.a('move',2,imm=1+j/8),NOP,[0x0C02,0xA280|j]]
    Wn=sum(len(i) for i in W)
    P+=[[0x0B03,0x0090,Wn if skip else 0]]+W
    if extra: P+=[A.a('move',2,imm=-3.0),A.st(2,0x123)]
    for j in range(8):
        P+=[[0x0C02,0x0280|j],NOP,A.st(0,O+j)]
    P+=[[0x0A02,0xFF70],[0x0B02,0x00F8],[1]]
    return [w for i in P for w in i]
def rd(pk): return [round(raw_value(x),4) for x in pk.read(RD+4*(O&0x3FF),8)]
step=int(sys.argv[1])
with dspmaster.Session(Peek(),size=200) as ses:
    ses.load(prog(0x500,step,False)); time.sleep(0.3); print('W  ',rd(ses.pk))
    a=pack=dspmaster.pack(dspmaster.adapt(prog(0x500,step,False))); b=dspmaster.pack(dspmaster.adapt(prog(0x500,step,True)))
    print('words changed:',sum(x!=y for x,y in zip(a,b)))
    ses.load(prog(0x500,step,True))
    for t in (0.0,0.05,0.3,1,3): time.sleep(t); print('R',t,rd(ses.pk))
    ses.load(prog(0x500,step,True,True))
    for t in (0.0,0.01,0.05,0.3): time.sleep(t); print('R+st',t,rd(ses.pk))
