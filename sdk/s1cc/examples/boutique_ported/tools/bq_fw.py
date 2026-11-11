"""bq_fw.py: run pieces of the Boutique 1.21 ARM code (PX-11:Appli) in an emulator (unicorn), README.md §3.

The image is mapped at 0x60000000; the start code's copies are made by hand (ITCM 0x6016725c -> 0x01000300, 0xe97c
bytes; 0x60175bd8 -> 0x01010000, 0x68; SRAM data 0x60175c40 -> 0x20000000, 0x474; .bss cleared); the boot ROM's
function table (0x02003000) returns 0; peripherals are plain memory, unmapped pages are mapped on first touch;
operator new (0x6005660c) is a bump allocator (the firmware's heap needs its RTOS, which does not run here).
Recorded: every write to the two DSP parameter windows (0x4014c000 chip 0, 0x4016c000 chip 1: 16 bytes per
parameter word C000 + i, the value at +4) and to the indirect program port (0x40100900 command = 0x40000000 |
type << 28 | (0x50000 chip 0 / 0x70000 chip 1) + byte address, 0x40100904 value): the immediate patches the ARM
makes in the DSP programs (type 0 / 1 / 2 = 16 / 24 / 32-bit immediate at that byte of program memory).
The synth object is the global 0x601c32c0 (constructor 0x60054714); setup(model) runs its init (vtable 0
0x6002472a), the model's set-up (vtable 3 0x60024e1e, vtable 4 0x60024c1c); setp(id, v) calls its parameter
dispatcher (vtable 13 0x6002b3e2) with the patch parameter id + 0x141.
"""
import os
import struct,sys,collections
from unicorn import *
from unicorn.arm_const import *
IMAGE=os.environ.get('BQ_APPLI', os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..','..','..','..','work','bq1','PX11_Appli.bin'))
img=open(IMAGE,'rb').read();B=0x60000000
mu=Uc(UC_ARCH_ARM,UC_MODE_THUMB|UC_MODE_MCLASS)
mu.mem_map(B,0x00400000); mu.mem_write(B,img)
mu.mem_map(0x01000000,0x40000); mu.mem_map(0x20000000,0x40000)
mu.mem_map(0x40000000,0x01000000); mu.mem_map(0xE0000000,0x00100000)
mu.mem_map(0x02000000,0x10000)
mu.mem_write(0x02000000,bytes([0x00,0x20,0x70,0x47]))
mu.mem_write(0x02003000,struct.pack('<1024I',*([0x02000001]*1024)))
mu.mem_map(0x70000000,0x100000)
def cp(dst,n,src): mu.mem_write(dst,img[src-B:src-B+n])
cp(0x01000300,0xe97c,0x6016725c); cp(0x01010000,0x68,0x60175bd8); cp(0x20000000,0x474,0x60175c40)
mu.mem_write(0x6016725c,b'\0'*0x188cc4)
log=[]
def w(mu,acc,addr,size,val,u): log.append((addr,val&0xffffffff))
mu.hook_add(UC_HOOK_MEM_WRITE,w,begin=0x40140000,end=0x4017ffff)
def unm(mu,acc,addr,size,val,u):
    try: mu.mem_map(addr&~0xfff,0x1000); return True
    except UcError: return False
mu.hook_add(UC_HOOK_MEM_READ_UNMAPPED|UC_HOOK_MEM_WRITE_UNMAPPED|UC_HOOK_MEM_FETCH_UNMAPPED,unm)
def call(fn,*args,count=20000000):
    regs=[UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_R3]
    for r in regs: mu.reg_write(r,0)
    for r,v in zip(regs,args): mu.reg_write(r,v&0xffffffff)
    mu.reg_write(UC_ARM_REG_SP,0x70080000); mu.reg_write(UC_ARM_REG_LR,0x70000001)
    del log[:]
    try:
        mu.emu_start(fn|1,0x70000000,count=count)
    except UcError as e:
        return 'ERR %s pc %x'%(e,mu.reg_read(UC_ARM_REG_PC))
    pc=mu.reg_read(UC_ARM_REG_PC)
    return 'ok' if pc==0x70000000 else 'stopped pc %x'%pc
def u32(a): return struct.unpack('<I',mu.mem_read(a,4))[0]
def f32(u): return struct.unpack('<f',struct.pack('<I',u))[0]
def params(lg=None):
    out={}
    for a,v in (lg or log):
        if (0x4014c000<=a<0x40150000 or 0x4016c000<=a<0x40170000) and a&0xf==4:
            out[(1 if a>=0x40160000 else 0, (a&0x3fff)>>4)]=f32(v)
    return out
# operator new (0x6005660c): bump allocator in 0x70100000.. (the firmware's heap needs the RTOS)
HEAP=[0x70200000]
mu.mem_map(0x70200000,0x200000)
def _new(mu,addr,size,u):
    n=mu.reg_read(UC_ARM_REG_R0); p=HEAP[0]; HEAP[0]=(p+n+15)&~7
    mu.mem_write(p,b'\0'*n)
    mu.reg_write(UC_ARM_REG_R0,p)
    mu.reg_write(UC_ARM_REG_PC,mu.reg_read(UC_ARM_REG_LR))
mu.hook_add(UC_HOOK_CODE,_new,begin=0x6005660c,end=0x6005660c)
O=0x601c32c0
def setup(mode):
    """construct the synth object, model `mode` (0 JP-08, 1 JU-06, 2 JX-03), init, set model, first update"""
    call(0x60054714,O); mu.mem_write(0x6016721c,b'\x01'); mu.mem_write(0x602eebc8+0x50,mode.to_bytes(4,'little'))
    call(0x6002472a,O); call(0x60024e1e,O); call(0x60024c1c,O)
    return params()
def setp(pid,v,layer=0x141):
    r=call(0x6002b3e2,O,pid+layer,0,v)
    assert r=='ok',r
    return params()
port=[]
_last=[0]
def _pw(mu,acc,a,s,v,u):
    v&=0xffffffff
    if a==0x40100900: _last[0]=v
    else: port.append((_last[0],v))
mu.hook_add(UC_HOOK_MEM_WRITE,_pw,begin=0x40100900,end=0x40100907)
def portw(lst=None):
    """{(chip, dsp address): float} from the indirect port writes (cmd = 0x4 | type<<28 | 0x50000/0x70000 + addr)"""
    out={}
    for c,v in (lst if lst is not None else port):
        chip=1 if (c&0xf0000)==0x70000 else 0
        out[(chip,(c>>28)&3,c&0xffff)]=f32(v)
    return out
mu.mem_write(0x40002004,b'\xff\xff\xff\xff')       # a status bit the program loader polls (bit 5)
