"""Compare the C engine with the JX-3P engine in emu1 (same OS calls).

usage: python3 test_emu.py   (after build.sh)
"""
import os, sys, struct
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 're'))
from emu1 import Emu1
from unicorn import arm_const as A
from jxpatch import patch_params

B = os.path.join(HERE, 'build')
ON = {0x4A1: 1, 0x4A2: 1, 0x4A3: 1, 0x73E: 1}
# plausible device values for ids outside the patch (part level, switches, ...)
REAL = {**ON, 0x14: 64, 0x2DB: 100, 0x2DC: 50, 0x2DD: 100, 0x2DF: 3, 0x2E0: 64, 0x3EF: 120,
        0x739: 12, 0x73A: 12, 0x73B: 0, 0x73C: 0, 0x744: 40, 0x7E3: 100, 0x801: 64, 0x737: 2}


class Emu1D(Emu1):
    """+0xB0 gets a pointer to the id; read it at call time (it may be on the stack)."""
    def _hook(self, uc, addr, size, ud):
        n = len(self.log)
        super()._hook(uc, addr, size, ud)
        if len(self.log) > n and self.log[-1][0] == 0xB0:
            off, r0, r1, *rest = self.log[-1]
            self.log[-1] = (off, r0, ('id', self.u32(r1)), *rest)
        elif len(self.log) > n and self.log[-1][0] == 0x70:
            # program and data contents, and the stack arguments after ctx
            off, r0, r1, r2, r3, s0, st = self.log[-1]
            sp = uc.reg_read(A.UC_ARM_REG_SP)
            stack = struct.unpack('<5I', uc.mem_read(sp, 20))
            def blob(p):
                a, n, n2, src = struct.unpack('<4I', uc.mem_read(p, 16))
                return (a, n, n2, bytes(uc.mem_read(src, (n + 3) & ~3)))
            self.log[-1] = (off, r0, blob(r1), blob(r2), stack[1:], s0, st)


def boot(img, slave, defaults):
    vals = dict(defaults)
    e = Emu1D(img, slave=slave, getval=lambda r0, r1, r2: vals.get(r2, 0))
    init = struct.unpack_from('<I', img, 0x24)[0]
    D = e.call(init); e.call(e.u32(D + 4)); D = e.call(init)
    e.o = e.u32(D + 20); e.vt = e.u32(e.o)
    e.call(e.u32(e.vt), e.o)
    return e


def norm(e, log):
    out = []
    for off, r0, r1, r2, r3, s0, st in log:
        if 0x28 <= off <= 0x6C:
            if (off - 0x28) // 4 < 6:
                out.append(('slot', off, r0))
            else:
                a, n, n2, src = struct.unpack('<4I', e.uc.mem_read(r0, 16))
                out.append(('prog', off, a, n, n2, bytes(e.uc.mem_read(src, (n + 3) & ~3))))
        elif off == 0xB0:
            out.append(('B0', r0, r1[1], s0))
        elif off == 0xA4:
            out.append(('A4', r0, r1, s0))
        elif off == 0xA0:
            out.append(('A0', r0, r1, r2, r3, s0))
        elif off == 0x70:
            out.append(('swap', r0, r1, r2, tuple(r3)))
        elif off == 0xC0:
            out.append(('sync',))
        elif off not in (0x00, 0xF0):
            out.append((hex(off),))
    return out


def state(calls):
    A, Bk = {}, {}
    for c in calls:
        if c[0] in ('A4', 'A0'):
            A[(c[1], c[2])] = c[-1]
        elif c[0] == 'B0':
            Bk[(c[1], c[2])] = c[3]
    return A, Bk


def run(img, slave, jx, extra):
    d = {i: v for i, n, v in patch_params()}
    e = boot(img, slave, {**d, **extra})
    e.log.clear(); e.call(e.u32(e.vt + 8), e.o); reg = norm(e, e.log)
    e.log.clear(); e.call(e.u32(e.vt + 0xC), e.o)
    if jx:
        for i, v in list(d.items()) + [(0x748, 0)]:      # 0x748 effect type: sent by the OS
            e.call(e.u32(e.vt + 0x30), e.o, i, 0, v)
    for i, v in extra.items():                           # the OS also sends these
        e.call(e.u32(e.vt + 0x30), e.o, i, 0, v)
    e.call(e.u32(e.vt + 0x24), e.o, 1)
    calls = norm(e, e.log)
    st = state(calls)
    swaps = sorted(c for c in calls if c[0] == 'swap')
    notes = []
    for v in range(4 * slave, 4 * slave + 4):
        for note, vel in ((60, 100), (73, 127), (48, 1)):
            e.log.clear()
            for i, val in ((0x429 + v, note), (0x43A + v, vel), (0x45C, v), (0x43A + v, 0)):
                e.call(e.u32(e.vt + 0x30), e.o, i, 0, val)
            notes.append(norm(e, e.log))
    return reg, st, notes, swaps


def main():
    jx = open(os.path.join(B, 'proj', 'region1.bin'), 'rb').read()
    c = open(os.path.join(B, 'engine_0x60A00000.bin'), 'rb').read()
    ok = True
    # the emulator's OS returns 0 for ids outside the patch; also try the on/off
    # ids switched on, as on the device
    for slave, extra in ((0, {}), (1, {}), (0, ON), (1, ON), (0, REAL), (1, REAL)):
        print('get values outside the patch:', extra or 'all 0')
        rj, sj, nj, wj = run(jx, slave, True, extra)
        rc, sc, nc, wc = run(c, slave, False, extra)
        for name, x, y in (('registration', rj, rc), ('state', sj, sc), ('notes', nj, nc),
                           ('slot program start (+0x70)', wj, wc)):
            same = x == y
            ok &= same
            print(f'slave {slave} {name}: {"same" if same else "DIFFERENT"}')
            if not same and name == 'state':
                for k in (0, 1):
                    dk = [(key, x[k].get(key), y[k].get(key)) for key in set(x[k]) | set(y[k])
                          if x[k].get(key) != y[k].get(key)]
                    print('   ', 'param' if k == 0 else 'bank5', len(dk), dk[:5])
            if not same and name != 'state':
                for i, (p, q) in enumerate(zip(x, y)):
                    if p != q:
                        print('    first diff', i, str(p)[:200], '|', str(q)[:200]); break
                print('    lengths', len(x), len(y))
    print('OK' if ok else 'MISMATCH')
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
