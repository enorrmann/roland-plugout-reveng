"""Tests of the C-S1JX ARM code (after build.sh), each one on its own, no hardware: both regions run in Unicorn
(Cortex-M, Thumb) with the OS services of 0x0100FC00 replaced by stubs that record every call.

    python3 test_arm.py            # all
    python3 test_arm.py notes      # only the tests whose name contains 'notes'

* no_vfp: the images have no floating-point instructions (system-1/vfp_count.py: the SYSTEM-1 has no FPU).
* queues: load queues, per DSP, exactly the program sets s1jx.py made (addresses, lengths, words), the two slave
  sections are switched on (FC44 0 / 2) after all eight were switched off.
* state: after load, every bank-5 / bank-6 write equals the Python model (model_full.py through test_model) plus
  engine.c's extra (P4:0x4a), routed as s1jx.Plan says, within 0.1 % (C float32 vs Python); and nothing else is
  written. Master and slave each write only their own chip's programs.
* notes: four notes on the assigner (master) use the four voices (ids 0x110 + v / 0x120 + v) and the master writes the
  pitch and gate of voices 0 and 1 (engine.c's values) to its words; the slave, given the ids 0x112 / 0x122, writes
  voice 2's to the slave words.
"""
import os
import struct
import sys
import traceback

import unicorn as U
from unicorn import arm_const as A

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, HERE)
import s1jx as J  # noqa: E402

PROJ = os.path.join(HERE, 'build', 'proj')
SVC_BASE, STUB, RET, STACK = 0x0100FC00, 0x01100000, 0x01200000, 0x20000000
BASES = (0x60800000, 0x60200000)


class Region:
    def __init__(self, r, get=lambda g, i: 0):
        self.r, self.base, self.get = r, BASES[r], get
        self.img = open(os.path.join(PROJ, f'region{r}.bin'), 'rb').read()
        uc = self.uc = U.Uc(U.UC_ARCH_ARM, U.UC_MODE_THUMB | U.UC_MODE_MCLASS)
        uc.mem_map(self.base, (len(self.img) + 0xFFFF) & ~0xFFFF)
        uc.mem_write(self.base, self.img)
        uc.mem_map(SVC_BASE & ~0xFFF, 0x1000)
        uc.mem_map(STUB, 0x1000)
        uc.mem_map(RET, 0x1000)
        uc.mem_map(STACK, 0x10000)
        uc.mem_write(STUB, b'\x70\x47' * 0x400)                      # bx lr
        uc.mem_write(RET, b'\xfe\xe7' * 0x10)                         # b .
        uc.mem_write(SVC_BASE, b''.join(struct.pack('<I', STUB + 2 * k + 1) for k in range(0x100)))
        self.log = []
        uc.hook_add(U.UC_HOOK_CODE, self._svc, begin=STUB, end=STUB + 0x7FF)

    def u32(self, a):
        return struct.unpack('<I', self.uc.mem_read(a, 4))[0]

    def _svc(self, uc, addr, size, ud):
        off = (addr - STUB) // 2 * 4
        r = [uc.reg_read(x) for x in (A.UC_ARM_REG_R0, A.UC_ARM_REG_R1, A.UC_ARM_REG_R2, A.UC_ARM_REG_R3)]
        if off == 0x00:
            uc.reg_write(A.UC_ARM_REG_R0, self.get(r[0], r[1]) & 0xFFFFFFFF)
        elif off == 0x16C:
            r[1] = self.u32(r[1] + 8)                                 # obj[2] = word index
        elif off in (0x30, 0x34):
            r[0] = self.read_set(r[0])
        self.log.append((off, r))

    def read_set(self, p):
        out = []
        while True:
            a, n, src = struct.unpack('<3I', self.uc.mem_read(p, 12))
            if not (a or n or src):
                return out
            raw = bytes(self.uc.mem_read(src, (n + 3) & ~3))
            w = []
            for i in range(0, len(raw), 4):
                x = struct.unpack_from('<I', raw, i)[0]
                w += [x >> 16, x & 0xFFFF]
            out.append((a, n, w[:n // 2]))
            p += 12

    def call(self, fn, *args):
        uc = self.uc
        for reg, v in zip((A.UC_ARM_REG_R0, A.UC_ARM_REG_R1, A.UC_ARM_REG_R2, A.UC_ARM_REG_R3), args):
            uc.reg_write(reg, v & 0xFFFFFFFF)
        uc.reg_write(A.UC_ARM_REG_SP, STACK + 0xFF00)
        uc.reg_write(A.UC_ARM_REG_LR, RET | 1)
        uc.emu_start(fn | 1, RET, count=50_000_000)
        return uc.reg_read(A.UC_ARM_REG_R0)

    def syn(self, off):
        return struct.unpack_from('<I', self.img, off)[0]

    def obj(self, getter_off):
        o = self.call(self.syn(getter_off))
        return o, self.u32(o)

    def vt(self, obj, k, *args):
        o, vt = obj
        return self.call(self.u32(vt + 4 * k), o, *args)


_PLAN = None


def plan():
    global _PLAN
    if _PLAN is None:
        _PLAN = J.Plan(J.roland_programs())
    return _PLAN


def master():
    m = Region(0)
    m.call(m.syn(0x24))                                               # create
    return m, m.obj(0x2C), m.obj(0x30)


def slave(get=lambda g, i: 0):
    s = Region(1, get)
    s.call(s.syn(0x24))
    return s, s.obj(0x30)


def f32(bits):
    return struct.unpack('<f', struct.pack('<I', bits & 0xFFFFFFFF))[0]


def writes(log):
    """{('B', chip word) | ('P', sel, id): last float written}"""
    out = {}
    for off, r in log:
        if off == 0x16C:
            out[('B', (r[0] << 8) | r[1])] = f32(r[2])
        elif off == 0xEC:
            out[('P', r[0], r[1])] = f32(r[2])
    return out


def expected(chip, notes=()):
    pl = plan()
    w = J.model_writes(J.default_vals())
    on = [0, 0, 0, 0]
    for v, note, vel in [(v, 60, 0) for v in range(4)] + list(notes):    # jx_send_state: every voice at 60, off
        for k, d, i, x in J.note_writes(v, note, vel):
            w[(k, d, i)] = x
        on[v] = vel > 0
    for d in (0, 1):                                                     # engine.c active_flags
        for i in (0x29, 0x3B):
            w[('B', d, i)] = 1.0 if any(on) else 0.0
    out = {}
    for (kind, d, i), v in w.items():
        if d & 1 != chip:
            continue
        if kind == 'B':
            idx = pl.b5_index(d, i)
            if idx is not None:
                out[('B', idx)] = v
        elif i < 0x100:
            out[('P', J.PLACE['MAIN%d' % d][0] & 1, i + pl.b6off['MAIN%d' % d])] = v
    return out


def close(a, b):
    return abs(a - b) <= 1e-3 * max(1.0, abs(b))


# ---------------------------------------------------------------- tests
def test_no_vfp():
    """the compiled code (region*.o; the images also hold the microcode, which is data) has no VFP instruction"""
    import re
    import subprocess
    for r in (0, 1):
        dis = subprocess.run(['arm-none-eabi-objdump', '-d', os.path.join(HERE, 'build', f'region{r}.o')],
                             capture_output=True, text=True, check=True).stdout
        bad = [line for line in dis.splitlines() if re.search(r'\tv[a-z]+(\.[a-z0-9]+)*\s', line)]
        assert not bad, bad[:3]


def test_queues():
    pl = plan()
    progs = {(s1, a): w for s1, a, _u, w in pl.programs()}
    m, a, mo = master()
    m.log.clear()
    m.vt(mo, 2)
    s, e = slave()
    s.log.clear()
    s.vt(e, 2)
    for reg, svc_a, svc_b, dsp_a in ((m, 0x30, 0x34, J.MA), (s, 0x30, 0x34, J.SA)):
        sets = {off: r[0] for off, r in reg.log if off in (svc_a, svc_b)}
        for off, dsp in ((svc_a, dsp_a), (svc_b, dsp_a + 1)):
            got = {addr: w for addr, n, w in sets[off]}
            for (s1, addr), w in progs.items():
                if s1 == dsp:
                    assert got.get(addr) == w, f'{J.S1_NAMES[dsp]} @{addr:#x}: not the s1jx program'
            assert 0 in got and 0x7FF2 in got, (J.S1_NAMES[dsp], sorted(got))
    sec = [r[:2] for off, r in s.log if off == 0x44]
    assert sec[:8] == [[i, 0] for i in range(8)] and sorted(sec[8:]) == [[0, 1], [2, 1]], sec


def test_state():
    for chip in (0, 1):
        if chip == 0:
            reg, a, obj = master()
        else:
            reg, obj = slave()
        reg.log.clear()
        reg.vt(obj, 2)
        got = writes(reg.log)
        if chip == 0:
            got.pop(('P', 1, J.M_VOLUME), None)
        exp = expected(chip)
        missing = [k for k in exp if k not in got]
        extra = [k for k in got if k not in exp]
        wrong = [(k, got[k], exp[k]) for k in exp if k in got and not close(got[k], exp[k])]
        assert not missing and not extra and not wrong, (chip, missing[:5], extra[:5], wrong[:5])
        print(f'    {"master" if chip == 0 else "slave"}: {len(got)} DSP words equal to the model')


def test_notes():
    m, a, mo = master()
    m.vt(mo, 2)
    m.log.clear()
    for note in (64, 67, 71, 74):                                     # four notes: one per voice
        m.vt(a, 3, note, 100)
    sets = [r[:3] for off, r in m.log if off == 0x04]
    voices = {r[1] - 0x110: r[2] for r in sets if 0x110 <= r[1] < 0x114}
    assert sorted(voices.values()) == [64, 67, 71, 74], sets
    assert all([2, 0x120 + v, 100] in sets for v in voices), sets
    got = writes(m.log)
    exp = expected(0, notes=[(v, n, 100) for v, n in voices.items()])
    for k, v in got.items():
        assert k in exp and close(v, exp[k]), (k, v, exp.get(k))
    mine = [v for v in voices if v < 2]
    assert len(mine) == 2 and all(('B', plan().b5_index(0, 0x21 + 0x12 * (v & 1))) in got for v in mine), got
    s, e = slave()
    s.vt(e, 2)
    s.log.clear()
    s.vt(e, 12, 0x112, 0, 67)
    s.vt(e, 12, 0x122, 0, 90)
    got = writes(s.log)
    exp = expected(1, notes=[(2, 67, 90)])
    assert got, 'slave wrote nothing for voice 2'
    for k, v in got.items():
        assert k in exp and close(v, exp[k]), (k, v, exp.get(k))
    print(f'    master: {len(writes(m.log))} words for 4 notes (voices 0, 1 its own), slave: {len(got)} for voice 2')


def main(sel):
    tests = [(k, v) for k, v in globals().items() if k.startswith('test_') and callable(v)]
    if sel:
        tests = [(k, v) for k, v in tests if any(s in k for s in sel)]
    bad = 0
    for k, fn in tests:
        try:
            fn()
            print('PASS', k, flush=True)
        except Exception:
            bad += 1
            print('FAIL', k, flush=True)
            traceback.print_exc()
    print(f'{len(tests) - bad} of {len(tests)} passed')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
