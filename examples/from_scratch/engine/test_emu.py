"""Run the built engine in emu1 next to the JX-3P engine (reference only) and
compare what both send to the slave OS:

* start-up: program registration (vt+8, contents and order) and the final DSP
  state after vt+0xC / vt+0x24(1), for random complete parameter states
* parameter changes (vt+0x30) for every model source, incl. the program swaps
  (+0xC0 / +0x70 with the program contents) of waveform and effect type
* notes on all four voices of each slave

usage: python3 test_emu.py [n]      (after build.sh)
"""
import os, random, struct, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 're'))
import ref, run_jx
import test_model as T
from emu1 import Emu1
from unicorn import arm_const as A

OURS = os.path.join(HERE, 'build', 'engine_0x60A00000.bin')


class Emu1D(Emu1):
    """+0xB0 gets a pointer to the id (our engine: on the stack): read it at call time"""
    def _hook(self, uc, addr, size, ud):
        n = len(self.log)
        super()._hook(uc, addr, size, ud)
        if len(self.log) > n and self.log[-1][0] == 0xB0:
            off, r0, r1, *rest = self.log[-1]
            self.idbuf = getattr(self, 'idbuf', {})
            self.log[-1] = (off, r0, ('id', self.u32(r1)), *rest)

    def u32(self, a):
        if isinstance(a, tuple):
            return a[1]
        return super().u32(a)


class Eng:
    """our engine in emu1, same calling convention as run_jx"""
    def __init__(self, vals, slave):
        self.vals = dict(vals)
        img = open(OURS, 'rb').read()
        self.e = e = Emu1D(img, slave=slave, getval=lambda r0, r1, r2: self.vals.get(r2, 0))
        init = struct.unpack_from('<I', img, 0x24)[0]
        D = e.call(init); e.call(e.u32(D + 4)); D = e.call(init)
        e.o = e.u32(D + 20); e.vt = e.u32(e.o)
        e.call(e.u32(e.vt), e.o)

    def boot(self):
        e = self.e
        e.log.clear()
        e.call(e.u32(e.vt + 8), e.o)
        e.call(e.u32(e.vt + 0xC), e.o)
        e.call(e.u32(e.vt + 0x24), e.o, 1)

    def setp(self, i, v):
        self.vals[i] = v
        self.e.call(self.e.u32(self.e.vt + 0x30), self.e.o, i, 0, v)


def calls(e, log, kinds=(0x70, 0xC0)):
    """program-related calls with their program contents"""
    out = []
    for off, r0, r1, r2, r3, s0, st in log:
        if off == 0x70:
            def blob(p):
                a, n, n2, src = struct.unpack('<4I', e.uc.mem_read(p, 16))
                return (a, n, n2, bytes(e.uc.mem_read(src, (n + 3) & ~3)))
            sp = e.uc.reg_read(A.UC_ARM_REG_SP)
            stack = struct.unpack('<5I', e.uc.mem_read(sp, 20))
            out.append(('swap', r0, blob(r1), blob(r2), stack[1:]))
        elif off == 0xC0:
            out.append(('sync', r0, r3))
    return out


def registration(e, log):
    out = []
    for off, r0, *_ in log:
        if 0x28 <= off <= 0x6C:
            if (off - 0x28) // 4 < 6:
                out.append(('slot', off, r0))
            else:
                a, n, n2, src = struct.unpack('<4I', e.uc.mem_read(r0, 16))
                out.append(('prog', off, a, n, n2, bytes(e.uc.mem_read(src, (n + 3) & ~3))))
    return out


def main():
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 6
    rng = random.Random(7)
    ok = True
    # --- start-up ------------------------------------------------------------------
    bad_state, bad_reg = {}, 0
    for slave in (0, 1):
        for it in range(n):
            vals = T.rand_state(rng)
            j = ref.boot(vals, slave)
            o = Eng(vals, slave); o.boot()
            if registration(j, j.log) != registration(o.e, o.e.log):
                bad_reg += 1
            js, os_ = ref.last_writes(j, j.log), ref.last_writes(o.e, o.e.log)
            for k, ab in T.compare(os_, js).items():
                bad_state.setdefault(k, (slave, ab))
    print(f'start-up: registration differs in {bad_reg} of {2 * n} boots; '
          f'{len(bad_state)} DSP ids differ in the final state')
    for k, (slave, ab) in sorted(bad_state.items())[:20]:
        print(f'  {T.fmt(k)}: slave {slave}: ours {ab[0]} jx {ab[1]}')
    ok &= not bad_reg and not bad_state
    # --- parameter changes -------------------------------------------------------------
    fails, swaps_bad, quirks = {}, 0, 0
    for slave in (0, 1):
        vals = T.rand_state(rng)
        j = ref.boot(vals, slave)
        o = Eng(vals, slave); o.boot()
        for it in range(n):
            for x in T.SOURCES:
                v = T.rand_value(rng, x)
                j.log.clear(); run_jx.setp(j, x, v)
                o.e.log.clear(); o.setp(x, v)
                if calls(j, j.log) != calls(o.e, o.e.log):
                    swaps_bad += 1
                    if swaps_bad <= 3:
                        print('  swap differs', hex(x), v, [c[:2] for c in calls(o.e, o.e.log)],
                              [c[:2] for c in calls(j, j.log)])
                jw, ow = ref.last_writes(j, j.log), ref.last_writes(o.e, o.e.log)
                bad = T.compare(ow, jw)
                if bad:
                    vals[x] = v
                    fresh = ref.state({**o.vals}, slave)
                    for k in list(bad):
                        if k in ow and T.close(ow[k], fresh.get(k)):
                            quirks += 1; del bad[k]
                for k, ab in bad.items():
                    fails.setdefault((x, k), (slave, v, ab))
                j.call(j.u32(j.vt + 0x24), j.o, 1)      # keep the JX-3P's cached flags current
    print(f'changes: {len(fails)} (source, DSP id) differ, {swaps_bad} program swaps differ '
          f'({quirks} writes differ only by the JX-3P\'s cached part flags)')
    for (x, k), (slave, v, ab) in sorted(fails.items())[:20]:
        print(f'  {x:#x}={v} -> {T.fmt(k)}: slave {slave}: ours {ab[0]} jx {ab[1]}')
    ok &= not fails and not swaps_bad
    # --- mute / unmute ----------------------------------------------------------------------
    mbad = 0
    for t in range(6):
        for off in (0x10, 0x18):
            j = ref.boot({0x748: t}, 0); o = Eng({0x748: t}, 0); o.boot()
            j.log.clear(); j.call(j.u32(j.vt + off), j.o, 0)
            o.e.log.clear(); o.e.call(o.e.u32(o.e.vt + off), o.e.o, 0)
            strip = lambda log: [x[:6] for x in log]
            if strip(j.log) != strip(o.e.log):
                mbad += 1
    print(f'mute / unmute: {mbad} of 12 differ')
    ok &= not mbad
    # --- notes ----------------------------------------------------------------------------
    nbad = 0
    for slave in (0, 1):
        vals = T.rand_state(rng)
        j = ref.boot(vals, slave)
        o = Eng(vals, slave); o.boot()
        for voice in range(8):
            for note, vel in ((60, 100), (72, 1), (30, 127), (60, 0)):
                for eng, f in ((j, lambda i, v: run_jx.setp(j, i, v)), (o.e, o.setp)):
                    eng.log.clear()
                    f(0x429 + voice, note); f(0x43A + voice, vel); f(0x45C, voice)
                jw, ow = ref.last_writes(j, j.log), ref.last_writes(o.e, o.e.log)
                bad = T.compare(ow, jw)
                nbad += len(bad)
                if bad and nbad <= 6:
                    print('  note', slave, voice, note, vel, {T.fmt(k): v for k, v in bad.items()})
    print(f'notes: {nbad} DSP writes differ')
    ok &= not nbad
    print('OK' if ok else 'MISMATCH')
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
