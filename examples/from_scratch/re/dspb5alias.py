"""Bank-5 aliasing (2026-10-17): indexed stores at 0x3b0, 0x7b0, 0xbb0, 0xfb0, 0x13b0, 0x23b0, read back in the same
sample and later: persistent words = index mod 0x400; bit 10 set = same-sample only.

    python3 dspb5alias.py
"""
import sys,time; import os; sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspmaster, dspasm as A
dspmaster.CONTEXT='dsp3'
NOP=[1]; O=0xA3C0; B5=0x41122000; U=1/16384
pk=Peek()
AD=[0x3b0,0x7b0,0xbb0,0xfb0,0x13b0,0x23b0]
def H(v): return [A.a('move',0,imm=v*U),NOP,[0x0B02,0x4080],NOP]
def prog(write):
    P=[[0x0A02,0xFF70]]
    if write:
        for k,a in enumerate(AD): P+=H(a)+[A.a('move',1,imm=20+k),NOP,[0x0C02,0x9DA0],NOP]
    for k,a in enumerate(AD): P+=H(a)+[A.a('move',0,imm=-7.0),NOP,[0x0C02,0x0DA0],NOP,NOP,A.st(0,O+k)]
    P+=[[0x0A02,0xFF70],[0x0B02,0x00F8],[1]]
    return [w for i in P for w in i]
save=pk.read(B5+4*0x3b0,1)
with dspmaster.Session(pk,size=240) as ses:
    ses.load(prog(True)); time.sleep(0.1)
    print('write+read:',[raw_value(x) for x in pk.read(B5+4*(O&0x3FF),len(AD))])
    ses.load(prog(False)); time.sleep(0.5)
    print('read only :',[raw_value(x) for x in pk.read(B5+4*(O&0x3FF),len(AD))])
    print('ARM b5[0x3b0]:',raw_value(pk.read(B5+4*0x3b0,1)[0]))
pk.write(B5+4*0x3b0,save)
