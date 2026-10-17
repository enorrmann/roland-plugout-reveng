"""Emulated check of the DSPLIVE engine (re/dsplive.py protocol): after DSPLIVE=1 dspaudio.py build <batch>.

Boots the engine in emu1 with the DSP program memory mapped, replays what dsplive.py poke sends
(parameter 0x465 = bend - 0x2000, then 0x467 = command) and checks the DCO2 slots of DSP2/DSP3
hold the new programs, then that restore brings back the built ones.

usage: python3 test_live.py <batch> <path>=<file> ...     (e.g. l00.batch d2a=d00_zero.txt)
       python3 test_live.py --modes        # 2026-10-17: both-DSP program writes and bank-5 data writes
"""
import os, struct, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 're'))
import dsplive as L
import dspaudio as da
from test_emu import Eng

SLOT = 0xB000


def prog_mem(e, d, n):
    return list(struct.unpack('<%dI' % n, e.uc.mem_read(0x41480000 + d * 0x100000 + SLOT, 4 * n)))


def packed(words):
    w = words + [0] * (len(words) & 1)
    return [w[i] << 16 | w[i + 1] for i in range(0, len(w), 2)]


class FakeLink(L.Link):
    def __init__(self, eng):
        self.eng, self.seq = eng, 0
        self.cmd(2, 1)
        self.cmd(2, 1)

    def cmd(self, cmd, ext, data=0x2000):
        self.seq ^= 1
        self.eng.setp(0x465, data - 0x2000)          # region 0: bend - 0x2000
        self.eng.setp(0x467, self.seq << 6 | cmd << 4 | ext)


def modes():
    """one command writes DSP2 and DSP3; bank-5 values (18-bit fixed point) land as RAW in both DPC RAW views"""
    import dspasm as A
    eng = Eng({}, 0)
    for d in (2, 3):
        eng.e.uc.mem_map(0x41480000 + d * 0x100000, 0x10000)
    for d in (2, 3):
        eng.e.uc.mem_map(0x41114000 + d * 0x4000 + 0x2000, 0x1000)
    eng.boot()
    link = FakeLink(eng)
    words = [0x0A02, 0xFF70, 0x1234, 0x5678, 0x0B02, 0x00F8, 0x0001, 0x0000]
    link.write(2, 0, words, both=True)
    for d in (2, 3):
        assert prog_mem(eng.e, d, 4)[:4] == packed(words)[:4], f'd{d}: both-DSP write'
    vals = [0.0, 0.5, -0.5, 0.1, -0.1, 0.9, -0.9, 0.12, 1 / 131072, -0.999]
    link.write_b5(0x48, vals)
    for d in (2, 3):
        got = struct.unpack('<%dI' % len(vals), eng.e.uc.mem_read(0x41114000 + d * 0x4000 + 0x2000 + 4 * 0x48, 4 * len(vals)))
        want = [A.raw(round(v * 131072) / 131072) for v in vals]
        assert list(got) == want, (d, [hex(x) for x in got], [hex(x) for x in want])
    print('both-DSP program write OK, bank-5 data write OK (DSP2 and DSP3)')


def main():
    if sys.argv[1] == '--modes':
        return modes()
    batch = sys.argv[1]
    if not os.path.exists(batch):
        batch = os.path.join(HERE, '..', 're', 'dspexp', 'audio', batch)
    st = L.state_from_batch(batch)
    eng = Eng({}, 0)
    for d in (2, 3):
        eng.e.uc.mem_map(0x41480000 + d * 0x100000, 0x10000)
    eng.boot()
    for d in (2, 3):
        w = packed(st['words'][str(d)])
        assert prog_mem(eng.e, d, len(w)) == w, f'd{d}: boot program differs'
    link = FakeLink(eng)
    for t in sys.argv[2:]:
        k, f = t.split('=')
        st['paths'][k] = f if os.path.exists(f) else os.path.join(HERE, '..', 're', 'dspexp', 'audio', f)
    installed = dict(st['words'])
    for d in (2, 3):
        new = L.program({v: st['paths'][f'd{d}{v}'] for v in 'ab'}, d)
        rs = L.runs(st['words'][str(d)], new)
        for start, w in reversed(rs):
            link.write(d, start, w)
        got = prog_mem(eng.e, d, len(packed(new)))
        print(f'd{d}: {sum(len(w) for _, w in rs)} words poked, program memory', 'OK' if got == packed(new) else 'DIFFERS')
        assert got == packed(new)
    link.cmd(2, 0)                                    # restore
    for d in (2, 3):
        w = packed(installed[str(d)])
        assert prog_mem(eng.e, d, len(w)) == w, f'd{d}: restore differs'
    # bend / modulation must not reach the DSP parameters
    eng.e.log.clear()
    link.cmd(2, 1)
    assert not [x for x in eng.e.log if x[0] in (0xA0, 0xA4, 0xB0)], eng.e.log
    print('restore OK, no parameter writes from pokes')


if __name__ == '__main__':
    main()
