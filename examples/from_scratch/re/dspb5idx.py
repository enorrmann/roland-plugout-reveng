"""Bank-5 indexed mode `da` on idle master DSP3 (2026-10-17): wrap / truncation / latency of h6, and the indexed
store `9daY` (read back by a plain load and by the ARM). Bank 5 saved and restored.

    python3 dspb5idx.py
"""
import sys,time,json; import os; sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspmaster, dspasm as A
dspmaster.CONTEXT='dsp3'
NOP=[1]; O=0xA3C0; B5=0x41122000; U=1/16384
pk=Peek(); save=pk.read(B5,0x400)
mk=[A.raw(i/2) for i in range(0x3C0)]
inv={A.raw(i/2):i for i in range(0x3C0)}
def fill():
    for i in range(0,0x3C0,64): pk.write(B5+4*i,mk[i:i+64])
fill()
def H6(v): return [A.a('move',0,imm=v*U),NOP,[0x0B02,0x4080],NOP]
def LD(word,lat=2): return [A.a('move',0,imm=-7.0),NOP,[0x0C02,word]]+[NOP]*lat
C={
 'h6=0x3ff, da2 (wrap?)':      H6(0x3ff)+LD(0x0DA2),
 'h6=0x400+5, da0':            H6(0x405)+LD(0x0DA0),
 'h6=0x10.75, da0 (trunc)':    H6(0x10+0.75)+LD(0x0DA0),
 'h6=-1, da0':                 H6(-1)+LD(0x0DA0),
 'h6=0x10 daF (Y=15)':         H6(0x10)+LD(0x0DAF),
 'h6=0x20 da0 lat0':           H6(0x20)+LD(0x0DA0,0),
 'h6=0x20 da0 lat1':           H6(0x20)+LD(0x0DA0,1),
 'h6 set, 0 nop, da0':         [A.a('move',0,imm=0x30*U),NOP,[0x0B02,0x4080],A.a('move',0,imm=-7.0),NOP,[0x0C02,0x0DA0],NOP,NOP],
}
STO=[A.a('move',1,imm=9.5),NOP]+H6(0x300)+[[0x0C02,0x9DA4],NOP,A.a('move',0,imm=-7.0),NOP,A.ld(0,0xA304),NOP,NOP]
def prog(cases):
    P=[[0x0A02,0xFF70]]
    for k,c in enumerate(cases): P+=c+[A.st(0,O+k)]
    P+=[[0x0A02,0xFF70],[0x0B02,0x00F8],[1]]
    return [w for i in P for w in i]
def show(names,r):
    for n,x in zip(names,r):
        print(f'{n:28s} {("b5[%#x]"%inv[x]) if x in inv else ("%+.6g"%raw_value(x))}')
with dspmaster.Session(pk,size=220) as ses:
    ses.load(prog(list(C.values()))); time.sleep(0.1)
    show(C,pk.read(B5+4*(O&0x3FF),len(C)))
    ses.load(prog([STO])); time.sleep(0.1)
    show(['store 9daY h6=0x300 Y=4 ->ld A304'],pk.read(B5+4*(O&0x3FF),1))
    print('ARM b5[0x304] =',raw_value(pk.read(B5+4*0x304,1)[0]))
for i in range(0,0x400,64): pk.write(B5+4*i,save[i:i+64])
