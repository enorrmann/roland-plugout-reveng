"""Validate the C reference (emu/dspref.py) and dsp/sim_main.py on the master's DSP4, live (dspmaster.py / dsppeek.py transport).

The test program (dspexp/master/m02_sim_check.txt) is stateless: every value it stores is a function of constants,
so the stored words do not depend on how many samples have run. It replaces the global-effect slot 0xEFDC of DSP4
while it runs (dspmaster restores it), reads bank 0 words 0x700-0x703 and compares them with the simulator.

    python3 dspvalid4.py            (needs the lab plugout installed and PLG3 selected)

Bank 0 is a ring, so a word read at an absolute address only holds this program's value in the sample where it was
stored. The DSP reads are therefore compared as a set: each device word must equal one of the simulator's stored
values, and the simulator's values must be the exact RAW encodings of the device's arithmetic.
"""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'dsp'))
sys.path.insert(0, os.path.join(HERE, '..', '..', '..', 'asm'))
os.environ['DSPCTX'] = 'efdc'                # global-effect slot 0xEFDC of DSP4 (dspmaster.CONTEXTS)
import dspasm as A                            # noqa: E402
import sim_main                                # noqa: E402
import dspmaster                               # noqa: E402
from dsppeek import Peek, raw_value           # noqa: E402

BASE = 0x700
ADDRS = [BASE + i for i in range(4)]
CONTEXT = 'master_dsp4_efdc'                   # the measured condition context of DSP4 (dspref.CONTEXT_BITS)


def program():
    body = []
    body.append(A.a('move', 0, imm=0.5))        # r0 = 0.5
    body.append(A.a('move', 1, imm=0.3))        # r1 = 0.3 (imm RAW)
    body.append(A.a('mul', 2, 1, 0))            # r2 = r1 * r0
    body.append(A.a('add', 3, 2, imm=0.25))     # r3 = r2 + 0.25
    body.append(A.st(2, BASE))                  # [0700] = r2
    body.append(A.st(3, BASE + 1))              # [0701] = r3
    body.append(A.ld(4, BASE))                  # r4 = [0700]  (visible one instruction later)
    body.append([0x0001])                       # load latency
    body.append(A.a('add', 5, 4, 1))            # r5 = r4 + r1
    body.append(A.st(5, BASE + 2))              # [0702] = r5
    body.append(A.st(4, BASE + 3))              # [0703] = r4
    words = [0x0A02, 0xFF70]
    for ins in body:
        words += ins
    words += [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001]
    return words


def text(words):
    return '# validation program for dspvalid4.py (stateless, see its docstring)\n' + \
        '\n'.join(' '.join(f'{w:04x}' for w in words[i:i + 8]) for i in range(0, len(words), 8)) + '\n'


def raw_of(v):
    return A.raw(v)


if __name__ == '__main__':
    words = program()
    path = os.path.join(HERE, 'dspexp', 'master', 'm02_sim_check.txt')
    open(path, 'w').write(text(words))

    # simulator: one sample, from zero state (the program reads nothing it did not write first)
    sim, _ = sim_main.simulate(words, {}, [0.0] * 8, {}, CONTEXT, 2, ADDRS)
    sim_vals = [sim[a][-1] for a in ADDRS]
    print('simulator, end of sample 2 (r2, r3, r4, r5 stored at 0700..0703 in that order):')
    for a, v in zip(ADDRS, sim_vals):
        print(f'  [{a:04x}] {v:+.9g}  RAW {raw_of(v):08x}')

    # hardware
    x, y = dspmaster.run(Peek(), words, (BASE, 4), 4, 0xEFDC, 0.05)
    print('hardware, bank 0 of DSP4 (two reads):')
    for i, (u, v) in enumerate(zip(x, y)):
        print(f'  [{BASE + i:04x}] {u:08x} {raw_value(u):+.9g}' + ('' if u == v else f'   then {v:08x}'))

    ok = True
    for i, u in enumerate(x):
        if not any(u == raw_of(v) for v in sim_vals):
            ok = False
            print(f'  MISMATCH [{BASE + i:04x}] {u:08x}: no simulated value has this RAW encoding')
    print('RESULT:', 'every hardware word is a simulated value, bit for bit' if ok else 'FAILED')
