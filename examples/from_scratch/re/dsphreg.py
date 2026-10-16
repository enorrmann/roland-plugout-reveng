"""Hidden-register writes `40TY` (C position) on master DSP4 slot 0xF7DC: for each low byte TY, reset h4/h5, apply
the word (immediate 5100 [0000]) and read h4/h5 with A op 0 sub 4/5; results in bank 5. Batches of 8 (time budget).

    python3 dsphreg.py [TY ...]        # hex low bytes, default a grid over T, mode, Y
"""
import sys, time
import os; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspmaster, dspasm as A
dspmaster.CONTEXT='efdc'
NOP=[1]
def meas(k):
    return [A.a('move',1,imm=0.0,bits=16),A.a('move',2,imm=0.0,bits=16),NOP,[0x0902,0x1102],NOP,A.st(1,0xA200+2*k),
            A.a('move',1,imm=0.0,bits=16),A.a('move',2,imm=0.0,bits=16),NOP,[0x0902,0x2142],NOP,A.st(2,0xA201+2*k)]
RESET=[[0x0B04,0x4020,0x4800,0x0000],[0x0B03,0x4050,0x0105]]          # h4 = 288, h5 = 0x105<<18 (defaults)
def word(lo):
    m=(lo>>4)&3
    imm={0:[],1:[0x5100],2:[0x5100,0x0000],3:[0x5100,0x0000]}[m]
    return [0x0B02+len(imm),0x4000|lo]+imm
los=[t<<6|m<<4|y for t in range(4) for m in range(4) for y in (0,2,3,5)]
def prog(batch):
    P=[[0x0A02,0xFF70],A.a('move',3,imm=14.6483),A.a('move',5,imm=-0.75)]
    for k,lo in enumerate(batch): P+=RESET+[word(lo),NOP]+meas(k)
    P+=RESET+[[0x0A02,0xFF70],[0x0B02,0x00F8],[1]]
    return [w for i in P for w in i]
los=[int(x,16) for x in sys.argv[1:]] or los
with dspmaster.Session(Peek(),4,0xF7DC,size=200) as ses:
    for b in range(0,len(los),8):
        batch=los[b:b+8]; words=prog(batch)
        assert sum(1 for _ in range(1)) and len(words) < 380
        ses.load(words); time.sleep(0.25)
        r=ses.pk.read(0x41126000+4*0x200,2*len(batch))
        for k,lo in enumerate(batch):
            h4,h5=r[2*k],r[2*k+1]
            tag=('' if h4==0x72000000 else f' h4={raw_value(h4):+.6g}({h4:08x})')+('' if h5==0x04140000 else f' h5={raw_value(h5):+.6g}({h5:08x})')
            print(f'40{lo:02x} t{lo>>6} m{(lo>>4)&3} y{lo&15}:{tag or " -"}', flush=True)
