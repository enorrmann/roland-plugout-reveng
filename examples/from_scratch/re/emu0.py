import sys, struct
import unicorn as U
from unicorn import arm_const as A
R0B=0x61800000
TRAP=0x0F000000; STACK=0x20100000
class Emu0:
    def __init__(self, img, base=R0B, getval=None):
        uc=self.uc=U.Uc(U.UC_ARCH_ARM, U.UC_MODE_THUMB|U.UC_MODE_MCLASS)
        uc.mem_map(base, 0x200000); uc.mem_write(base, img)
        uc.mem_map(STACK-0x10000, 0x20000)
        uc.mem_map(TRAP,0x1000)
        for i in range(0,0x1000,2): uc.mem_write(TRAP+i,b"\x70\x47")
        uc.mem_map(0x01000000,0x1000)
        for k in range(4): uc.mem_write(0x01000400+4*k, struct.pack('<I',(TRAP+0x10*k)|1))
        uc.mem_write(TRAP+0x800, b"\x00\xbe")  # bkpt return
        self.log=[]; self.getval=getval or (lambda *a:0)
        uc.hook_add(U.UC_HOOK_CODE, self._hook, begin=TRAP, end=TRAP+0x7ff)
        uc.reg_write(A.UC_ARM_REG_C1_C0_2, 0xF00000); uc.reg_write(A.UC_ARM_REG_FPEXC,0x40000000)
    def _hook(self, uc, addr, size, ud):
        k=(addr-TRAP)//0x10
        if (addr-TRAP)%0x10: return
        r=[uc.reg_read(getattr(A,f'UC_ARM_REG_R{i}')) for i in range(4)]
        s=lambda x: x-(1<<32) if x&0x80000000 else x
        names=['get','set','flush','set2']
        if k==0:
            v=self.getval(s(r[0]),s(r[1]),s(r[2]))
            self.log.append(('get',s(r[0]),s(r[1]),s(r[2]),v))
            uc.mem_write(r[3], struct.pack('<i',v)); uc.reg_write(A.UC_ARM_REG_R0,1)
        else:
            self.log.append((names[k],)+tuple(s(x) for x in r))
    def u32(self,a): return struct.unpack('<I',self.uc.mem_read(a,4))[0]
    def call(self, fn, *args):
        uc=self.uc
        for i,a in enumerate(args): uc.reg_write(getattr(A,f'UC_ARM_REG_R{i}'), a&0xffffffff)
        uc.reg_write(A.UC_ARM_REG_SP, STACK); uc.reg_write(A.UC_ARM_REG_LR, (TRAP+0x800)|1)
        uc.emu_start(fn|1, TRAP+0x800, count=2000000)
        return uc.reg_read(A.UC_ARM_REG_R0)
