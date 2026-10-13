"""
engine_emu.py -- run engine functions under unicorn and log every call that
goes through the OS service table at 0x0113FC00.

    from engine_emu import Emu
    e = Emu(open("p0s.bin","rb").read(), 0x60800000)
    obj = e.alloc(0x2000)
    e.call(ctor, obj, mode)               # constructor / init
    calls = e.call(setter, obj, 0, 3)     # -> [(os_offset, r0, r1, r2, r3, s0, s1, stack0), ...]
"""
from __future__ import annotations

import struct

import unicorn as U
from unicorn import arm_const as A

OS_TABLE = 0x0113FC00
OS_SIZE = 0x400
TRAP = 0x0F000000
STACK = 0x20000000
HEAP = 0x30000000


class Emu:
    def __init__(self, image: bytes, base: int, extra: int = 0x200000):
        self.uc = U.Uc(U.UC_ARCH_ARM, U.UC_MODE_THUMB | U.UC_MODE_MCLASS)
        self.base, self.size = base, len(image)
        span = (len(image) + extra + 0xFFF) & ~0xFFF
        self.uc.mem_map(base, span)
        self.uc.mem_write(base, image)
        self.uc.mem_map(STACK, 0x20000)
        self.uc.mem_map(HEAP, 0x400000)
        self.uc.mem_map(TRAP, 0x1000)
        for i in range(0, 0x1000, 2):
            self.uc.mem_write(TRAP + i, b"\x70\x47")          # bx lr
        self.uc.mem_map(OS_TABLE & ~0xFFF, 0x2000)
        for off in range(0, OS_SIZE, 4):
            self.uc.mem_write(OS_TABLE + off, struct.pack("<I", (TRAP + (off // 4) * 4) | 1))
        self.calls: list = []
        self.heap_ptr = HEAP
        self.uc.hook_add(U.UC_HOOK_CODE, self._hook, begin=TRAP, end=TRAP + 0xFFF)
        # enable VFP / FPU
        self.uc.reg_write(A.UC_ARM_REG_C1_C0_2, 0xF00000)
        self.uc.reg_write(A.UC_ARM_REG_FPEXC, 0x40000000)

    def alloc(self, n: int) -> int:
        a = self.heap_ptr
        self.heap_ptr += (n + 15) & ~15
        return a

    def _hook(self, uc, addr, size, ud):
        if (addr - TRAP) % 4 == 0:
            off = (addr - TRAP) // 4 * 4
            sp = uc.reg_read(A.UC_ARM_REG_SP)
            s0 = struct.unpack("<f", uc.reg_read(A.UC_ARM_REG_S0).to_bytes(4, "little"))[0] \
                if False else self._sreg(0)
            s1 = self._sreg(1)
            self.calls.append((off, uc.reg_read(A.UC_ARM_REG_R0), uc.reg_read(A.UC_ARM_REG_R1),
                               uc.reg_read(A.UC_ARM_REG_R2), uc.reg_read(A.UC_ARM_REG_R3), s0, s1,
                               struct.unpack("<I", bytes(uc.mem_read(sp, 4)))[0]))

    def _sreg(self, n: int) -> float:
        v = self.uc.reg_read(A.UC_ARM_REG_S0 + n)
        return struct.unpack("<f", struct.pack("<I", v & 0xFFFFFFFF))[0]

    def set_s(self, n: int, f: float):
        self.uc.reg_write(A.UC_ARM_REG_S0 + n, struct.unpack("<I", struct.pack("<f", f))[0])

    def mem(self, a, n):
        return bytes(self.uc.mem_read(a, n))

    def u32(self, a):
        return struct.unpack("<I", self.mem(a, 4))[0]

    def w32(self, a, v):
        self.uc.mem_write(a, struct.pack("<I", v & 0xFFFFFFFF))

    def call(self, fn: int, *args, s0=None, s1=None, maxinsn=200000):
        uc = self.uc
        self.calls = []
        for i, v in enumerate(args[:4]):
            uc.reg_write(A.UC_ARM_REG_R0 + i, v & 0xFFFFFFFF)
        sp = STACK + 0x1F000
        extra = args[4:]
        sp -= 4 * len(extra)
        for i, v in enumerate(extra):
            uc.mem_write(sp + 4 * i, struct.pack("<I", v & 0xFFFFFFFF))
        uc.reg_write(A.UC_ARM_REG_SP, sp)
        uc.reg_write(A.UC_ARM_REG_LR, (TRAP + 0xFFC) | 1)
        if s0 is not None:
            self.set_s(0, s0)
        if s1 is not None:
            self.set_s(1, s1)
        try:
            uc.emu_start(fn | 1, TRAP + 0xFFC, count=maxinsn)
        except U.UcError as e:
            self.error = str(e)
            raise
        return list(self.calls)
