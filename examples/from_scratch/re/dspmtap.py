"""N delay-tap requests per sample on master DSP4 f7dc, each to its own result triplet (h5 = 0x301 + 3k);
input = counter c (+INC per sample). Prints age of each tap (samples; includes the ARM read delay, compare differences) and its fraction.

    python3 dspmtap.py "[0.1*k for k in range(1,9)]" "[0.25+0.125*k for k in range(24)]"   # 2026-10-17: 24 taps OK, frac 1/64
"""
import sys,time,os; import os; sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspmaster, dspasm as A
from dsptap import S, NOP
dspmaster.CONTEXT='efdc'
INC=2**-20; W=0x4300; OUT5=0xA3C0; RAW5=0x41126000
def program(Ds):
    P=[A.ld(1,S+1),NOP,A.a('add',1,1,imm=INC)]+A.wrap(1,2)+[A.st(1,S),A.st(1,W),NOP,
       [0x1D06,0x4022,0x7FE0,0xC1FC,(324<<6)&0xFFFF,0]]
    for k,D in enumerate(Ds):
        P+=[A.a('move',3,imm=D),[0x050<<5|2,0xBF6C],NOP,[0x0B03,0x4050,(W+1+3*k)&0x1FFF],[0x060<<5|2,0x3A90],NOP]
    P+=[NOP,NOP]
    for k in range(len(Ds)):
        P+=[A.ld(4,W+1+3*k),A.ld(6,W+3+3*k),NOP,NOP,A.st(4,OUT5+2*k),A.st(6,OUT5+2*k+1)]
    P+=[A.st(1,OUT5+0x30)]
    P=[[0x0A02,0xFF70]]+P+[[0x0A02,0xFF70],[0x0B02,0x00F8],[1]]
    return [w for i in P for w in i]
def age(c,t):
    n=2/INC; return ((c-t)/INC+n/2)%n-n/2
sets=[eval(s) for s in sys.argv[1:]]
with dspmaster.Session(Peek(),4,0xF7DC,size=700) as ses:
    for Ds in sets:
        pr=program(Ds); print(len(pr),'words')
        ses.load(pr); time.sleep(0.6)
        r=ses.pk.read(RAW5+4*(OUT5&0x3FF),2*len(Ds)); c=raw_value(ses.pk.read(RAW5+4*((OUT5+0x30)&0x3FF),1)[0])
        for k,D in enumerate(Ds):
            print(f'  tap{k} D={D:<12.8g} age {age(c,raw_value(r[2*k])):+10.2f}  frac {raw_value(r[2*k+1]):.8f} ({r[2*k+1]:08x})')
