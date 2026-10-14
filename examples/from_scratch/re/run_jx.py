import sys
from emu1 import *
from jxpatch import patch_params
IMG='/tmp/claude-1000/-home-emilio-git-roland-plugout-reveng/34b06ee2-b299-4101-8d2a-2477aa666108/scratchpad/jx/region1.bin'
def boot(slave=0):
    vals={}
    e=Emu1(open(IMG,'rb').read(), slave=slave, getval=lambda r0,r1,r2: vals.get(r2,0))
    e.vals=vals
    D=e.call(0x60a0fccc); e.call(e.u32(D+4)); D=e.call(0x60a0fccc)
    e.o=e.u32(D+20); e.vt=e.u32(e.o)
    e.call(e.u32(e.vt),e.o)
    return e
def setp(e,i,v):
    e.vals[i]=v; e.call(e.u32(e.vt+0x30), e.o, i, 0, v)
if __name__=='__main__':
    e=boot()
    e.log.clear()
    for i,n,d in patch_params(): 
        e.log.clear(); setp(e,i,d)
        print(f'{i:#x} {n}={d}:', '; '.join(e.fmt()))
