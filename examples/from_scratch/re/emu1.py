import struct
import unicorn as U
from unicorn import arm_const as A
TRAP=0x0F000000; STACK=0x20100000; OST=0x0113FC00
class Emu1:
    """Engine (region 1) emulator: OS table 0x0113FC00 entries trapped."""
    PREC = 6          # decimals kept of s0 in the log (None = full float32)
    def __init__(self, img, base=0x60a00000, slave=0, getval=None, bss=0x40000):
        uc=self.uc=U.Uc(U.UC_ARCH_ARM, U.UC_MODE_THUMB|U.UC_MODE_MCLASS)
        span=(len(img)+bss+0xfff)&~0xfff
        uc.mem_map(base, span); uc.mem_write(base, img)
        uc.mem_map(STACK-0x20000, 0x40000)
        uc.mem_map(TRAP,0x2000)
        for i in range(0,0x2000,2): uc.mem_write(TRAP+i,b"\x70\x47")
        uc.mem_map(OST&~0xfff,0x1000)
        for k in range(0,0x100,4): uc.mem_write(OST+k, struct.pack('<I',(TRAP+k*4)|1))
        self.slave=slave; self.getval=getval or (lambda *a:0)
        self.log=[]
        uc.hook_add(U.UC_HOOK_CODE, self._hook, begin=TRAP, end=TRAP+0x3ff)
        uc.reg_write(A.UC_ARM_REG_C1_C0_2, 0xF00000); uc.reg_write(A.UC_ARM_REG_FPEXC,0x40000000)
    def r(self,n): return self.uc.reg_read(getattr(A,f'UC_ARM_REG_R{n}'))
    def s(self,n):
        return struct.unpack('<f',struct.pack('<I',self.uc.reg_read(A.UC_ARM_REG_S0+n)&0xffffffff))[0]
    def _hook(self, uc, addr, size, ud):
        if (addr-TRAP)%16: return
        off=(addr-TRAP)//4
        sp=uc.reg_read(A.UC_ARM_REG_SP)
        st=struct.unpack('<I',uc.mem_read(sp,4))[0]
        s0=self.s(0)
        e=(off,self.r(0),self.r(1),self.r(2),self.r(3),s0 if self.PREC is None else round(s0,self.PREC),st)
        self.log.append(e)
        if off==0xf0: uc.reg_write(A.UC_ARM_REG_R0,self.slave)
        elif off==0x00: uc.reg_write(A.UC_ARM_REG_R0,self.getval(self.r(0),self.r(1),self.r(2)) & 0xffffffff)
    def u32(self,a): return struct.unpack('<I',self.uc.mem_read(a,4))[0]
    def w32(self,a,v): self.uc.mem_write(a,struct.pack('<I',v&0xffffffff))
    def call(self, fn, *args, s0=None, count=5000000):
        uc=self.uc
        for i,a in enumerate(args[:4]): uc.reg_write(getattr(A,f'UC_ARM_REG_R{i}'), a&0xffffffff)
        sp=STACK-4*len(args[4:])
        for i,a in enumerate(args[4:]): uc.mem_write(sp+4*i, struct.pack('<I',a&0xffffffff))
        uc.reg_write(A.UC_ARM_REG_SP, sp); uc.reg_write(A.UC_ARM_REG_LR,(TRAP+0x1ffc)|1)
        if s0 is not None: uc.reg_write(A.UC_ARM_REG_S0, struct.unpack('<I',struct.pack('<f',s0))[0])
        uc.emu_start(fn|1, TRAP+0x1ffc, count=count)
        return self.r(0)
    def fmt(self, log=None):
        out=[]
        for off,r0,r1,r2,r3,s0,st in (self.log if log is None else log):
            out.append(f'+{off:02x}({r0:#x},{r1:#x},{r2:#x},{r3:#x}, s0={s0}, sp0={st:#x})')
        return out
