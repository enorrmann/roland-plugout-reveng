"""Delay line as loop memory (2026-10-17, master DSP4 f7dc): fill it with a counter for 11 s, then write back the
tap of age a (u = 25 -> 409602 samples) every sample; tap(a) and tap(2a) then agree (to a few samples) for 45 s.
The DMA writes [4300] every sample even without a store (repeats the last value), so stored data must be refreshed.

    python3 dspedmloop.py
"""
import sys,time; import os; sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspmaster, dspasm as A
from dsptap import S, NOP
dspmaster.CONTEXT='efdc'
W=0x4300; OUT5=0xA3C0; RAW5=0x41126000; U1,U2=25.0,50.0
def tap(k,u,reg):
    return [A.a('move',3,imm=u),[0x050<<5|2,0xBF6C],NOP,[0x0B03,0x4050,(W+1+3*k)&0x1FFF],[0x060<<5|2,0x3A90],NOP,NOP,
            A.ld(reg,W+1+3*k),NOP,NOP]
def prog(loop):
    P=[[0x0A02,0xFF70],A.ld(1,S+1),NOP,A.a('add',1,1,imm=2**-20)]+A.wrap(1,2)+[A.st(1,S),
       [0x1D06,0x4022,0x7FE0,0xC1FC,(324<<6)&0xFFFF,0]]
    P+=tap(0,U1,4)+tap(1,U2,5)
    P+=[A.st(4 if loop else 1,W),NOP,A.st(4,OUT5),A.st(5,OUT5+1),A.st(1,OUT5+2)]
    P+=[[0x0A02,0xFF70],[0x0B02,0x00F8],[1]]
    return [w for i in P for w in i]
pk=Peek()
with dspmaster.Session(pk,4,0xF7DC,size=200) as ses:
    ses.load(prog(False)); time.sleep(11)
    ses.load(prog(True)); t0=time.time()
    for t in (1,6,15,30,45):
        time.sleep(max(0,t-(time.time()-t0)))
        r=[]
        for _ in range(3):
            x=pk.read(RAW5+4*(OUT5&0x3FF),3); r.append((raw_value(x[0]),raw_value(x[1]),x[0]==x[1]))
        print(f't={t:2d}s  tapA, tapB, equal:',[(round(a,5),round(b,5),e) for a,b,e in r])
