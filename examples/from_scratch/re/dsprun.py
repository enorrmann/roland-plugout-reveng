"""Run a DSP program on the master's DSP4 (hardware) and read its data bank.

    python3 dsprun.py experiment.py           # experiment file defines PROG, DATA, READ

The program replaces the global-effect program at DSP4 0xEFDC (program memory
0x4188EFDC) for a moment: the region 0 init hook (examples/midi_send,
init_bulk.s) saves the original words, writes DATA (DT40 word index -> float,
through the IEEE view 0x41800000), writes the program (first as an empty task,
then the body, then the head, so the DSP never runs a half-written program),
waits, copies READ = (first word, count) of DT40 (RAW view) twice to RAM
(snapshots DELAY loop turns apart), puts the original program back and dumps
the snapshots. DATA goes through the RAW view too (dspnum.encode).

An experiment file:

    PROG = '''
    0a02 ff70
    1e05 0fe0 1fe2 0400 0401
    0a02 ff70 0b02 00f8 0001
    '''
    DATA = {0x400: 1.0, 0x401: 2.0}
    READ = (0x400, 32)

DATA5 / READ5 instead use bank 5 (DPC4, ids = program refs A000 + id): the
ARM cannot write DT0 (writes to its RAW or IEEE view do not stay; verified),
bank 5 it can.
"""
import os, struct, subprocess, sys, tempfile
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, HERE)
import dspnum

PROG_ADDR = 0x41480000 + 4 * 0x100000 + 0xEFDC     # DSP4 program memory, slot 0xEFDC
DT0 = 0x41400000 + 4 * 0x100000 + 0x4000           # DSP4 bank 0, RAW view (the IEEE view of a
                                                   # data bank neither keeps writes nor reads back)
B5 = 0x41114000 + 4 * 0x4000                       # DSP4 bank 5 (DPC4), IEEE view: ARM writes stay
B5RAW = B5 + 0x2000                                # ... and its RAW view; program refs A000 + id
SNAPDST = 0x61802000                               # region 0 zero area (slot 3) for the snapshots
EMPTY = [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001, 0x0000]


def words(text):
    return [int(x, 16) for x in text.split()]


def pack(hw):
    """halfwords in execution order -> 32-bit memory words (upper half first)"""
    hw = list(hw) + [0] * (len(hw) % 2)
    return [hw[i] << 16 | hw[i + 1] for i in range(0, len(hw), 2)]


def pokes(prog, data, data5=None):
    out = [(DT0 + 4 * i, dspnum.encode(v)) for i, v in sorted(data.items())]
    out += [(B5 + 4 * i, struct.unpack('<I', struct.pack('<f', v))[0]) for i, v in sorted((data5 or {}).items())]
    if prog is None:                                   # data only: the running program stays
        return out, 0
    final = pack(prog)
    empty = pack(EMPTY)
    if len(final) < len(empty):
        final += [0] * (len(empty) - len(final))
    out += [(PROG_ADDR + 4 * i, w) for i, w in enumerate(empty)]           # empty task first
    out += [(PROG_ADDR + 4 * i, w) for i, w in enumerate(final) if i >= len(empty)]
    out += [(PROG_ADDR + 4 * i, final[i]) for i in reversed(range(len(empty)))]
    return out, len(final)


def run(prog, data, read, delay=1000000, out=None, data5=None, bank5=False):
    plist, n = pokes(prog, data, data5)
    tmp = tempfile.mkdtemp()
    with open(os.path.join(tmp, 'poke.bin'), 'wb') as f:
        for a, v in plist:
            f.write(struct.pack('<II', a, v))
    first, count = read
    out = out or os.path.join(ROOT, 'dumps', 'dsprun_last.bin')
    env = dict(os.environ, POKE=os.path.join(tmp, 'poke.bin'), DELAY=str(delay),
               SNAPN=str(count), SNAPSRC=hex((B5RAW if bank5 else DT0) + 4 * first), SNAPDST=hex(SNAPDST))
    if n:
        env.update(SAVEN=str(n), SAVEADDR=hex(PROG_ADDR))
    nbytes = 8 * count                             # two snapshots
    r = subprocess.run([sys.executable, os.path.join(ROOT, 'examples', 'midi_send', 'bulk_dump.py'),
                        '--start', hex(SNAPDST), '--count', str(-(-nbytes // 32)),
                        '-o', out], env=env, capture_output=True, text=True)
    print(r.stdout.rstrip())
    d = open(out, 'rb').read()
    a = struct.unpack_from('<%dI' % count, d, 0)
    b = struct.unpack_from('<%dI' % count, d, 4 * count)
    return [(first + k, a[k], b[k]) for k in range(count)]


def main():
    ns = {}
    exec(open(sys.argv[1]).read(), ns)
    prog = words(ns['PROG']) if ns.get('PROG') else None
    data = ns.get('DATA', {})
    data5 = ns.get('DATA5', {})
    bank5 = 'READ5' in ns
    rows = run(prog, data, ns['READ5'] if bank5 else ns['READ'], ns.get('DELAY', 1000000),
               data5=data5, bank5=bank5)
    if bank5:
        data = data5
    for i, a, b in rows:
        before = data.get(i)
        va = dspnum.decode(a)
        mark = '' if before is None or abs(before - va) < 1e-6 else f'   (written {before:g})'
        later = '' if a == b else f'   then {b:08x} = {dspnum.decode(b):.9g}'
        print(f'{i:#06x}  {a:08x} = {va: .9g}{mark}{later}')


if __name__ == '__main__':
    main()
