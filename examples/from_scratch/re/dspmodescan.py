"""Scan transfer mode bytes (loads, X=0) on idle master DSP3 (2026-10-17): bank 5 filled with markers i/2 (ARM
writes, restored afterwards), one reference word, h6 / h7 set (in words). r0 preset -7 = no load.

    python3 dspmodescan.py [REF Y H6 H7 [MODES]]     # hex; default A005 0 10 20, all modes but 9b
    python3 dspmodescan.py A005 3 15 27 d0,d1,da,db  # -> da = bank5[h6+Y], db = bank5[h7+Y], dN = bank5[N*16+Y]
"""
import sys,time; import os; sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspmaster, dspasm as A
dspmaster.CONTEXT='dsp3'
NOP=[1]; O=0xA3C0; B5=0x41122000; U=1/16384
pk=Peek()
save=pk.read(B5,0x400)
import json; json.dump(save,open(''+os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..','..','work','dsp3_bank5_saved.json')+'','w'))
mk=[A.raw(i/2) for i in range(0x3C0)]
for i in range(0,0x3C0,64): pk.write(B5+4*i,mk[i:i+64])
def prog(modes,ref,Y):
    P=[[0x0A02,0xFF70],A.a('move',0,imm=H6*U),NOP,[0x0B02,0x4080],A.a('move',0,imm=H7*U),NOP,[0x0B02,0x40C0],NOP]
    for k,m in enumerate(modes):
        P+=[A.a('move',0,imm=-7.0),NOP,[0x0C03,m<<4|Y,ref],NOP,NOP,A.st(0,O+k)]
    P+=[[0x0A02,0xFF70],[0x0B02,0x00F8],[1]]
    return [w for i in P for w in i]
inv={A.raw(i/2):i for i in range(0x3C0)}
a=sys.argv[1:5] if len(sys.argv)>4 else ['A005','0','10','20']
ref=int(a[0],16); H6=int(a[2],16); H7=int(a[3],16)
Y=int(a[1],16)
modes=[int(x,16) for x in sys.argv[5].split(',')] if len(sys.argv)>5 else [m for m in range(256) if m!=0x9b]
res={}
with dspmaster.Session(pk,size=200) as ses:
    for b in range(0,len(modes),8):
        bt=modes[b:b+8]; ses.load(prog(bt,ref,Y)); time.sleep(0.08)
        r=ses.read_b5 if 0 else pk.read(B5+4*(O&0x3FF),len(bt))
        r2=pk.read(B5+4*(O&0x3FF),len(bt))
        for m,x,y in zip(bt,r,r2):
            v=raw_value(x)
            s='none' if v==-7.0 else (f'b5[{inv[x]:#x}]' if x in inv else f'{v:+.6g}')
            if x!=y: s+=' (varies)'
            res.setdefault(s,[]).append(f'{m:02x}')
for i in range(0,0x400,64): pk.write(B5+4*i,save[i:i+64])
for s,ms in sorted(res.items(),key=lambda t:-len(t[1])): print(f'{s:24s} {len(ms):3d}: {" ".join(ms)}')
