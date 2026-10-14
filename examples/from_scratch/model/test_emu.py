"""Compare our region 0 with the JX-3P's in an emulator: same calls, same
kernel parameter traffic (0x01000400 get/set/flush/touch) expected.

usage: python3 test_emu.py [n_sequences]   (after build.sh and engine/build.sh)
The JX-3P image is only the reference here; nothing of it goes into the build.
"""
import os, random, struct, sys
import unicorn as U
from unicorn import arm_const as A

HERE = os.path.dirname(os.path.abspath(__file__))
BASE = 0x61800000
TRAP = 0x0F000000
STACK = 0x20100000
SVC = 0x01000400
NAMES = ['get', 'set', 'flush', 'touch']
OURS = os.path.join(HERE, 'build', 'region0_0x61800000.bin')
JX = os.path.join(HERE, '..', 'engine', 'build', 'proj', 'region0.bin')


def s32(x):
    return x - (1 << 32) if x & 0x80000000 else x


class Emu0:
    """Region 0 with the kernel parameter services trapped; get returns
    self.vals[id] (default 0) in r0, like the kernel."""
    def __init__(self, img):
        uc = self.uc = U.Uc(U.UC_ARCH_ARM, U.UC_MODE_THUMB | U.UC_MODE_MCLASS)
        uc.mem_map(BASE, 0x200000)
        uc.mem_write(BASE, img)
        uc.mem_map(STACK - 0x10000, 0x20000)
        uc.mem_map(TRAP, 0x1000)
        for i in range(0, 0x1000, 2):
            uc.mem_write(TRAP + i, b'\x70\x47')            # bx lr
        uc.mem_map(SVC & ~0xfff, 0x1000)
        for k in range(4):
            uc.mem_write(SVC + 4 * k, struct.pack('<I', (TRAP + 0x10 * k) | 1))
        uc.mem_map(0x4003C000, 0x1000)                      # timer (stock mode 3 only)
        uc.hook_add(U.UC_HOOK_CODE, self._hook, begin=TRAP, end=TRAP + 0x7ff)
        self.log, self.vals = [], {}
        init = struct.unpack_from('<I', img, 0x24)[0]
        self.init = init
        d = self.call(init)
        self.call(self.u32(d + 4))                          # create
        d = self.call(init)
        self.desc = d
        self.obj = [self.u32(d + 12), self.u32(d + 16)]
        self.log.clear()

    def _hook(self, uc, addr, size, ud):
        if (addr - TRAP) % 0x10:
            return
        k = (addr - TRAP) // 0x10
        r = [s32(uc.reg_read(getattr(A, f'UC_ARM_REG_R{i}'))) for i in range(4)]
        if k == 0:
            v = self.vals.get(r[2], 0)
            uc.reg_write(A.UC_ARM_REG_R0, v & 0xffffffff)
            self.log.append(('get', r[0], r[2]))
        elif k == 1:
            self.log.append(('set', r[0], r[1], r[2], r[3]))
        else:
            self.log.append((NAMES[k], r[0], r[1]) + ((r[2],) if k == 3 else ()))

    def u32(self, a):
        return struct.unpack('<I', self.uc.mem_read(a, 4))[0]

    def call(self, fn, *args):
        uc = self.uc
        for i, a in enumerate(args):
            uc.reg_write(getattr(A, f'UC_ARM_REG_R{i}'), a & 0xffffffff)
        uc.reg_write(A.UC_ARM_REG_SP, STACK)
        uc.reg_write(A.UC_ARM_REG_LR, (TRAP + 0x800) | 1)
        uc.emu_start(fn | 1, TRAP + 0x800, count=200000)
        return uc.reg_read(A.UC_ARM_REG_R0)

    def vt(self, part, n, *args):
        o = self.obj[part]
        return self.call(self.u32(self.u32(o) + 4 * n), o, *args)

    def select(self, part, mode):
        return self.call(self.u32(self.desc + 24), part, mode)


def scenario(rng, n):
    """A random sequence of (kind, args) events."""
    ev = [('vals', {0x6FF: 0, 0x6FD: 0, 0x6FE: 0, 0x22D: 8})]
    for _ in range(n):
        r = rng.random()
        part = rng.randrange(2)
        if r < 0.40:
            ev.append(('vt', part, 3, rng.randrange(58, 66), rng.choice([0, 1, 64, 100, 127])))
        elif r < 0.65:
            ev.append(('vt', part, 2, rng.randrange(58, 66)))
        elif r < 0.72:
            ev.append(('vt', part, 4, 64, rng.choice([0, 127])))
        elif r < 0.76:
            ev.append(('vt', part, 4, rng.choice([1, 7, 11, 65, 120, 121, 122, 123, 127, 5]),
                       rng.randrange(128)))
        elif r < 0.80:
            ev.append(('vt', part, 6, rng.randrange(0x4000)))
        elif r < 0.86:
            ev.append(('vals', {rng.choice([0x6FF, 0x6FD, 0x6FE]): rng.choice([0, 1, 2, 30])}))
            ev.append(('vt', part, 1, rng.choice([4, 4, 1])))
        elif r < 0.90:
            ev.append(('select', part, rng.randrange(5)))
        elif r < 0.92:
            ev.append(('vt', part, 5, rng.randrange(10)))
        elif r < 0.94:
            ev.append(('vt', part, 0))
        elif r < 0.96:
            ev.append(('vt', part, 16))
        else:
            ev.append(('vt', part, rng.choice([14, 15]), rng.choice([4, 8, 6])))
    return ev


def run(emu, events):
    out = []
    for e in events:
        if e[0] == 'vals':
            emu.vals.update(e[1]); continue
        emu.log.clear()
        if e[0] == 'vt':
            r = emu.vt(*e[1:])
        else:
            r = emu.select(*e[1:])
        out.append((e, tuple(emu.log), r if e[0] == 'vt' and e[2] in (10, 16) else None))
    return out


def main():
    if not os.path.exists(JX):
        raise SystemExit(f'{JX} missing: run engine/build.sh first (reference only)')
    jx_img, our_img = open(JX, 'rb').read(), open(OURS, 'rb').read()
    # same data the OS walks: parameter list (one label renamed) and the init
    ours, jx = Emu0(our_img), Emu0(jx_img)
    assert ours.u32(ours.desc + 24) and ours.obj[0] and ours.obj[1], 'descriptor'
    print('descriptor ok: create', hex(ours.u32(ours.desc + 4)), 'parts',
          hex(ours.obj[0]), hex(ours.obj[1]))
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 300
    bad = 0
    for seed in range(n):
        ev = scenario(random.Random(seed), 120)
        a, b = Emu0(our_img), Emu0(jx_img)
        ra, rb = run(a, ev), run(b, ev)
        for (e, la, va), (_, lb, vb) in zip(ra, rb):
            if la != lb or va != vb:
                bad += 1
                print(f'seed {seed}: differs at {e}\n  ours {la} {va}\n  jx   {lb} {vb}')
                break
        if bad >= 5:
            break
    total = sum(len(l) for _, l, _ in ra)
    print(f'{n} sequences x 120 events: {"OK" if not bad else f"{bad} differ"} '
          f'(last sequence: {total} kernel calls)')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
