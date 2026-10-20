"""DSP bank-5 queue -> delay ring copier, verified with the tap of age 409602 (2026-10-20, master DSP4 slot 0xF7DC).

Each sample: load bank5[SRC] -> [4300] (ring input), rotate the 16-word queue at SRC, tap u = 25 -> bank 5 OUT.
The ARM then reads OUT (bank 5 is coherent) and checks that every value is one of the 16 queued words.

    python3 dspcopy.py
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
import time
from dsppeek import raw_value
RAW5_=0x41126000; SRC=0xA004; N=16
V=[0x0A000000+k*0x00110000 for k in range(16)]
pk=Peek()
def copier():
    P=[[0x0A02,0xFF70],[0x1D06,0x4022,0x7FE0,0xC1FC,(324<<6)&0xFFFF,0]]
    P+=[A.ld(4,SRC),NOP,NOP,A.st(4,W),NOP]          # r4 = src[0] -> delay input
    for k in range(N-1):                             # shift the bank-5 queue down: src[k] = src[k+1]
        P+=[A.ld(5,SRC+k+1),NOP,NOP,A.st(5,SRC+k),NOP]
    P+=[A.st(4,SRC+N-1),NOP]                         # rotate: the old src[0] goes to the tail
    P+=tap(0,U1,6)                                   # tap at age 409602 (verified)
    P+=[A.st(6,OUT5),NOP]
    P+=[[0x0A02,0xFF70],[0x0B02,0x00F8],[1]]
    return [w for i in P for w in i]
with dspmaster.Session(pk,4,0xF7DC,size=200) as ses:
    for k,v in enumerate(V): pk.write(RAW5_+4*(4+k),[v])
    print('bank5 src written:', [hex(x) for x in pk.read(RAW5_,N)][:4], '...')
    ses.load(copier()); time.sleep(6)
    seen={}
    t_end=time.time()+4
    while time.time()<t_end:
        w=pk.read(RAW5_+4*(OUT5&0x3FF),1)[0]
        seen[w]=seen.get(w,0)+1
    inV=sum(c for w,c in seen.items() if w in V)
    print(f'reads {sum(seen.values())}: distinct {len(seen)}, in V {inV}, other {sum(seen.values())-inV}')
    print('values seen not in V:', [hex(w) for w in seen if w not in V][:8])
    print('V seen:', sorted(hex(w) for w in seen if w in V))
