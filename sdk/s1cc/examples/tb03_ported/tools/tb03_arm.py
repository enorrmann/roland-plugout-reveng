"""tb03_arm.py: run pieces of the TB-03's ARM code (BQ2:Appli 1.07) in an emulator and write tb03_tables.h.

    python3 sdk/s1cc/examples/tb03_ported/tools/tb03_arm.py [WORKDIR] [-o tb03_tables.h]   (WORKDIR: work/tb03)

Needs the decoded image (tools/tb03_extract.py) and the unicorn module (pip install unicorn). The image is mapped at
0x60000000, its ITCM copy (0x60163b4c, 0xd710 bytes) at 0x01000300 and its SRAM data (0x601712a8, 0x560 bytes) at
0x20000000, as its own start code copies them; the peripherals are plain memory and every write to the two
parameter windows (0x4014c000, 0x4016c000: 16 bytes per parameter, the value at +4) is recorded. Unmapped pages
are mapped on first touch. What is called (addresses in BQ2:Appli; README.md §3):
    0x6001da18  the TB-03 voice object's constructor (parameter ids of the oscillator mixer)
    0x6001e138  its init: every voice parameter's start value
    knob handlers (the TB-03 knob dispatcher 0x6001d520, param 3..9): TUNE 0x6001e108, CUTOFF 0x6001defe,
    RESONANCE 0x6001ded0, ENV MOD 0x6001de20, DECAY 0x6001de86, ACCENT 0x6001ddf2; each called with every knob value.
"""
import os
import struct
import sys

from unicorn import Uc, UcError, UC_ARCH_ARM, UC_MODE_THUMB, UC_MODE_MCLASS, UC_HOOK_MEM_WRITE, \
    UC_HOOK_MEM_READ_UNMAPPED, UC_HOOK_MEM_WRITE_UNMAPPED, UC_HOOK_MEM_FETCH_UNMAPPED
from unicorn.arm_const import UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3, UC_ARM_REG_SP, \
    UC_ARM_REG_LR

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..', '..'))
B = 0x60000000
VOICE = 0x4016c000                       # the voice DSP's parameter window (the TB-03 voice object's chip 1)
KNOBS = [('tune', 0x6001e108, 0x10, 256), ('cutoff', 0x6001defe, 0x19, 256), ('resonance', 0x6001ded0, 0x1a, 256),
         ('envmod', 0x6001de20, 0x1b, 256), ('decay', 0x6001de86, 0x0d, 128), ('accent', 0x6001ddf2, 0x1c, 256)]


def f32(u):
    return struct.unpack('<f', struct.pack('<I', u))[0]


class Emu:
    def __init__(self, img):
        mu = Uc(UC_ARCH_ARM, UC_MODE_THUMB | UC_MODE_MCLASS)
        mu.mem_map(B, 0x01000000)
        mu.mem_write(B, img)
        mu.mem_map(0x01000000, 0x30000)
        mu.mem_write(0x01000300, img[0x60163B4C - B:0x60163B4C - B + 0xD710])
        mu.mem_write(0x01010000, img[0x6017125C - B:0x6017125C - B + 0x4C])
        mu.mem_map(0x20000000, 0x20000)
        mu.mem_write(0x20000000, img[0x601712A8 - B:0x601712A8 - B + 0x560])
        mu.mem_map(0x40000000, 0x200000)
        mu.mem_map(0x70000000, 0x100000)                 # stack, return trap, the objects
        mu.hook_add(UC_HOOK_MEM_WRITE, self._w, begin=0x40000000, end=0x401FFFFF)
        mu.hook_add(UC_HOOK_MEM_READ_UNMAPPED | UC_HOOK_MEM_WRITE_UNMAPPED | UC_HOOK_MEM_FETCH_UNMAPPED, self._unmapped)
        self.mu, self.log = mu, []

    def _w(self, mu, access, addr, size, value, user):
        self.log.append((addr, value & 0xFFFFFFFF))

    @staticmethod
    def _unmapped(mu, access, addr, size, value, user):
        try:
            mu.mem_map(addr & ~0xFFF, 0x1000)
        except UcError:
            return False
        return True

    def call(self, fn, r0=0, r1=0):
        mu = self.mu
        for reg, v in ((UC_ARM_REG_R0, r0), (UC_ARM_REG_R1, r1), (UC_ARM_REG_R2, 0), (UC_ARM_REG_R3, 0),
                       (UC_ARM_REG_SP, 0x70080000), (UC_ARM_REG_LR, 0x70000001)):
            mu.reg_write(reg, v)
        self.log = []
        mu.emu_start(fn | 1, 0x70000000, count=2000000)

    def voice_params(self):
        """{index: value} written to the voice parameter window by the last call"""
        return {(a - VOICE) >> 4: f32(v) for a, v in self.log if VOICE <= a < VOICE + 0x20000 and a & 0xF == 4}


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('-')]
    work = args[0] if args else os.path.join(ROOT, 'work', 'tb03')
    out = sys.argv[sys.argv.index('-o') + 1] if '-o' in sys.argv else os.path.join(
        os.path.dirname(os.path.abspath(__file__)), '..', 'tb03_tables.h')
    img = open(os.path.join(work, 'BQ2_Appli.bin'), 'rb').read()
    e, obj = Emu(img), 0x70010000
    e.call(0x6001da18, obj)
    e.call(0x6001e138, obj)
    init = e.voice_params()
    tabs = []
    for name, fn, idx, n in KNOBS:
        t = []
        for k in range(n):
            e.call(fn, obj, k)
            p = e.voice_params()
            if list(p) != [idx]:
                raise SystemExit('%s: knob %d wrote %s, expected only C0%02X' % (name, k, p, idx))
            t.append(p[idx])
        tabs.append((name, idx, t))
    L = ['/* tb03_tables.h: the TB-03 (BQ2_UPD.BIN 1.07) ARM side, as data. Written by tools/tb03_arm.py: the voice',
         ' * object\'s constructor and init and its knob handlers run in an ARM emulator (unicorn), every value they write',
         ' * to the voice DSP\'s parameter window recorded. Do not edit: README.md §3. */',
         '#ifndef TB03_TABLES_H', '#define TB03_TABLES_H', '',
         '/* the voice DSP parameters (C000 + index) after the TB-03 init (0x6001e138); the ones it does not write stay 0 */',
         'static const float tb03_init[0x40] = {']
    vals = ['%sf' % repr(float(init.get(i, 0.0))) for i in range(0x40)]
    for i in range(0, 0x40, 4):
        L.append('    ' + ', '.join(vals[i:i + 4]) + ',  /* %02x */' % i)
    L.append('};')
    for name, idx, t in tabs:
        L += ['', '/* %s: knob 0..%d -> C0%02X */' % (name.upper(), len(t) - 1, idx),
              'static const float tb03_%s[%d] = {' % (name, len(t))]
        for i in range(0, len(t), 6):
            L.append('    ' + ', '.join('%sf' % repr(float(x)) for x in t[i:i + 6]) + ',')
        L.append('};')
    L.append('#endif')
    open(out, 'w').write('\n'.join(L) + '\n')
    print('%s: init %d parameters, %d knob tables' % (os.path.normpath(out), len(init), len(tabs)))


if __name__ == '__main__':
    main()
