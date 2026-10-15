"""Run a test program on a DSP of the MASTER and read its data bank, live (dsppeek.py transport).

    python3 dspmaster.py prog.txt [--read 0x400 8] [--dsp 4] [--slot 0xEFDC] [--wait 0.05]

The program (16-bit words in execution order, '#' comments, as in re/dspexp/audio) replaces the
global-effect program at <slot> of master DSP <dsp> for a moment: the original words are read,
the test program is written (an empty program first, then the body, then the head, so the DSP
never runs a half-written program), we wait, read bank 0 (RAW view) twice, and put the original
program back (tail first, head last). Values are printed as RAW words and decoded floats.
While it runs, that global effect (reverb/delay of the SYSTEM-8) is replaced.
"""
import argparse, json, os, signal, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value   # noqa: E402

EMPTY = [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001, 0x0000]


def words_of(text):
    return [int(t, 16) for line in text.splitlines() for t in line.split('#')[0].split()]


def pack(hw):
    """halfwords in execution order -> 32-bit memory words (upper half first)"""
    hw = list(hw) + [0] * (len(hw) % 2)
    return [hw[i] << 16 | hw[i + 1] for i in range(0, len(hw), 2)]


SAVED = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'work', 'dspmaster_saved.json')


def restore_saved(pk):
    s = json.load(open(SAVED))
    pa, orig = s['addr'], s['words']
    pk.write(pa, pack(EMPTY))
    pk.write(pa + 12, orig[3:])
    for i in reversed(range(3)):
        pk.write(pa + 4 * i, [orig[i]])
    return pk.read(pa, len(orig)) == orig


def majority(reads):
    out = []
    for col in zip(*reads):
        best = max(set(col), key=col.count)
        if col.count(best) * 2 <= len(col):
            print(f'  no majority: {[f"{x:08x}" for x in col]}', file=sys.stderr)
        out.append(best)
    return out


def prog_addr(dsp, slot):
    return 0x41480000 + dsp * 0x100000 + slot


def bank0(dsp, idx):
    return 0x41404000 + dsp * 0x100000 + 4 * idx       # RAW view


def run(pk, prog, read=(0x400, 8), dsp=None, slot=None, wait=0.05):
    """one test program in the current context (DSPCTX): load, read bank 0 twice (majority), restore"""
    with Session(pk, dsp, slot, wait=wait) as ses:
        ses.load(prog)
        time.sleep(wait)
        first, n = read
        return ses.read(first, n), ses.read(first, n)

# test contexts: (dsp, slot, program used while switching, words saved). 'dsp3' = the main program of master
# DSP3, which is idle (0001 0001 0b03 0080 7fee 0001): no other task shares it. 'efdc' = the global-effect
# control task on DSP4 (shared with other tasks, ~4000 runs/s).
CONTEXTS = {
    'efdc': (4, 0xEFDC, EMPTY, 256),
    'dsp3': (3, 0x0000, [0x0001, 0x0001, 0x0B03, 0x0080, 0x7FEE, 0x0001], 256),
}
CONTEXT = os.environ.get('DSPCTX', 'dsp3')
MAIN_END = [0x0B03, 0x0080, 0x7FEE, 0x0001]


def adapt(prog, ctx=None):
    """our test programs are written as slot programs (0a02 ff70 ... 0a02 ff70 0b02 00f8 0001); the DSP3 main
    program ends with 0b03 0080 7fee 0001 instead"""
    if (ctx or CONTEXT) != 'dsp3':
        return prog
    p = list(prog)
    if p[:2] == [0x0A02, 0xFF70]:
        p = p[2:]
    if p[-5:] == [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001]:
        p = p[:-5]
    return [0x0001, 0x0001] + p + MAIN_END


class Session:
    """keep a test slot open: save the original program once, update only changed words, restore at the end"""
    def __init__(self, pk, dsp=None, slot=None, size=None, wait=0.02):
        cdsp, cslot, cempty, csize = CONTEXTS[CONTEXT]
        dsp = cdsp if dsp is None else dsp
        slot = cslot if slot is None else slot
        size = csize if size is None else size
        self.empty = cempty
        self.pk, self.dsp, self.pa, self.wait = pk, dsp, prog_addr(dsp, slot), wait
        self.orig = pk.read(self.pa, size)
        # keep a copy on disk (a killed run can be repaired with `dspmaster.py --restore`) and restore on SIGTERM
        json.dump({'addr': self.pa, 'words': self.orig}, open(SAVED, 'w'))
        signal.signal(signal.SIGTERM, lambda *a: sys.exit(1))
        self.cur = list(pack(self.empty))
        pk.write(self.pa, self.cur)

    def load(self, prog):
        new = pack(adapt(prog))
        if len(new) > len(self.orig):
            raise ValueError('program too long for the saved area')
        new += [0] * max(0, len(self.cur) - len(new))
        diff = [i for i in range(len(new)) if i >= len(self.cur) or new[i] != self.cur[i]]
        head = [i for i in diff if i < 3]
        for i in [i for i in diff if i >= 3]:                 # body (contiguous runs), head last
            self.pk.write(self.pa + 4 * i, [new[i]])
        for i in reversed(head):
            self.pk.write(self.pa + 4 * i, [new[i]])
        self.cur = new
        time.sleep(self.wait)

    def read(self, first, n, k=7):
        reads = []
        for _ in range(k):
            reads.append(self.pk.read(bank0(self.dsp, first), n))
        return majority(reads)

    def close(self):
        pk, pa, orig = self.pk, self.pa, self.orig
        pk.write(pa, pack(self.empty))
        pk.write(pa + 4 * 3, orig[3:])
        for i in reversed(range(3)):
            pk.write(pa + 4 * i, [orig[i]])
        if pk.read(pa, len(orig)) != orig:
            print('WARNING: original program not restored exactly', file=sys.stderr)

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('prog', nargs='?')
    ap.add_argument('--restore', action='store_true', help='put back the program saved by the last Session')
    ap.add_argument('--read', nargs=2, default=['0x400', '8'])
    ap.add_argument('--dsp', type=int, default=4)
    ap.add_argument('--slot', default='0xEFDC')
    ap.add_argument('--wait', type=float, default=0.05)
    a = ap.parse_args()
    if a.restore:
        print('restored' if restore_saved(Peek()) else 'MISMATCH')
        return
    prog = words_of(open(a.prog).read())
    first, n = int(a.read[0], 0), int(a.read[1], 0)
    x, y = run(Peek(), prog, (first, n), a.dsp, int(a.slot, 0), a.wait)
    for i, (u, v) in enumerate(zip(x, y)):
        print(f'[{first + i:04x}] {u:08x} {raw_value(u):+.9g}' + ('' if u == v else f'   then {v:08x} {raw_value(v):+.9g}'))


if __name__ == '__main__':
    main()
