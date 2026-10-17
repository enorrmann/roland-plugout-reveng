"""Bank 5 beyond 0x3FF, h7 (`db`/`df`) and the time-varying modes d8/d9/dc/dd on idle master DSP3 (2026-10-17).

    python3 dspb5map.py
"""
import sys,time,json; import os; sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspmaster, dspasm as A
dspmaster.CONTEXT='dsp3'
NOP=[1]; O=0xA3C0; B5=0x41122000; U=1/16384
pk=Peek()
def H(t,v): return [A.a('move',0,imm=v*U),NOP,[0x0B02,0x4080 if t==6 else 0x40C0],NOP]
def LD(word): return [A.a('move',0,imm=-7.0),NOP,[0x0C02,word],NOP,NOP]
def ST(word,v): return [A.a('move',1,imm=v),NOP,[0x0C02,word],NOP]
cases=[]
# store distinct values via indexed store at h6 = a, read back with plain ref and with da
for a,v in ((0x450,11.25),(0x7f0,12.5),(0x810,13.75),(0xc10,14.0)):
    cases.append((f'st h6={a:#x} {v}; plain ld A{a&0xfff:03x}',H(6,a)+ST(0x9DA0,v)+[A.a('move',0,imm=-7.0),NOP,A.ld(0,0xA000|(a&0xFFF)),NOP,NOP]))
    cases.append((f'   ...; da0 read back',H(6,a)+LD(0x0DA0)))
cases.append(('h7=0x123: db0',H(7,0x123)+LD(0x0DB0)))
cases.append(('h7=0x123: df1',H(7,0x123)+LD(0x0DF1)))
cases.append(('h7=0x450 db0 (after st 0x450)',H(7,0x450)+LD(0x0DB0)))
cases.append(('d8 0',LD(0x0D80))); cases.append(('d8 0 again',LD(0x0D80)))
cases.append(('d9 0',LD(0x0D90))); cases.append(('dc 0',LD(0x0DC0)))
def prog(cs):
    P=[[0x0A02,0xFF70]]
    for k,(n,c) in enumerate(cs): P+=c+[A.st(0,O+k)]
    P+=[[0x0A02,0xFF70],[0x0B02,0x00F8],[1]]
    return [w for i in P for w in i]
save=pk.read(B5,0x400)
mk=[A.raw(i/2) for i in range(0x3C0)]; inv={A.raw(i/2):i for i in range(0x3C0)}
for i in range(0,0x3C0,64): pk.write(B5+4*i,mk[i:i+64])
with dspmaster.Session(pk,size=240) as ses:
    for b in range(0,len(cases),8):
        cs=cases[b:b+8]; ses.load(prog(cs)); time.sleep(0.1)
        for (n,_),x in zip(cs,pk.read(B5+4*(O&0x3FF),len(cs))):
            print(f'{n:40s} {("b5[%#x]"%inv[x]) if x in inv else ("%+.6g"%raw_value(x))}')
    # counter-like d8: sample several times
    ses.load(prog([('d8',LD(0x0D80)),('d9',LD(0x0D90)),('d8 1',LD(0x0D81)),('d8 again',LD(0x0D80))]))
    for t in range(4):
        time.sleep(0.05); r=pk.read(B5+4*(O&0x3FF),4)
        print('d8,d9,d81,d8:',[("b5[%#x]"%inv[x]) if x in inv else ("%+.6g"%raw_value(x)) for x in r])
print('ARM b5 window 0x3ff..:',[hex(x) for x in pk.read(B5+4*0x3f8,8)])
for i in range(0,0x400,64): pk.write(B5+4*i,save[i:i+64])
