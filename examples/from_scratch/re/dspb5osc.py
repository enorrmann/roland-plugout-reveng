"""Slave test: oscillator reading a table from bank 5 with the indexed mode `0daY` (h6 = base + phase*N).
F = fill table (plain bank-5 stores, every sample) + play; P = play only (table must persist).
    python3 dspb5osc.py F|P [-N 16] [--base 0x100] [--wave tri|sq] -o file
    python3 dspown.py load file; python3 dspharm.py NAME 48 60     # measured 2026-10-17: tri / sq harmonics exact"""
import argparse, math, os, sys
import os; sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
import dspasm as A
from dspflagsaw import cmp_imm, jump_unless_ge
NOP=[1]; U=1/16384
def tab(wave,N):
    if wave=='tri': return [1-2*abs(2*k/N-1) for k in range(N)]
    return [0.8 if k<N//2 else -0.8 for k in range(N)]
def build(mode,N,base,wave,slot='dco2',fs=96000.0):
    K=2*440.0/fs
    P=[[0x0A02,0xFF70]]
    if mode=='F':
        for k,v in enumerate(tab(wave,N)):
            P+=[A.a('move',2,imm=v),A.st(2,0xA000+base+k)]
    for voice in 'AB':
        pin,outs,state=A.SLOTS[slot][voice]
        P+=[A.ld(1,state+1),A.ld(0,pin),A.a('add',0,0,imm=-4.75),A.a('mul',2,0,imm=math.log(2)/16),
            A.a('move',3,imm=1/24),A.a('ymacd',3,2,imm=1/6),A.a('ymacd',3,2,imm=0.5),
            A.a('ymacd',3,2,imm=1.0),A.a('ymacd',3,2,imm=1.0)]
        P+=[A.a('mul',3,3,3)]*4+[A.a('mul',3,3,imm=K)]
        P+=[A.a('add',1,3,1),cmp_imm(1,-1.0),NOP]
        sub=A.a('add',1,1,imm=-2.0,bits=16)
        P+=[jump_unless_ge(len(sub)),sub,A.st(1,state)]
        # h6 = (base + (r1+1)*N/2)/16384
        P+=[A.a('mul',0,1,imm=N/2*U),NOP,A.a('add',0,0,imm=(base+N/2)*U),NOP,[0x0B02,0x4080],NOP,
            [0x0C02,0x2DA0],NOP,NOP,A.st(2,outs[0]),A.st(2,outs[1])]
    P+=[[0x0A02,0xFF70],[0x0B02,0x00F8],[1]]
    return P
ap=argparse.ArgumentParser(); ap.add_argument('mode'); ap.add_argument('-N',type=int,default=16)
ap.add_argument('--base',type=lambda s:int(s,0),default=0x100); ap.add_argument('--wave',default='tri'); ap.add_argument('-o')
a=ap.parse_args()
open(a.o,'w').write(A.text(build(a.mode,a.N,a.base,a.wave),f'bank-5 table osc {a.mode} N={a.N} base={a.base:#x} {a.wave}'))
