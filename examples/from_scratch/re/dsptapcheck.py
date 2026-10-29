"""The delay line of the C reference (emu/dspcore.h tap / EDM, DSP_HARDWARE.md §8.4) against the device: dsptap.py's
counter program in master DSP4 slot 0xF7DC (the slot whose bank-2 window the ARM wired to the delay memory) for
several delays, whole and fractional; the reference runs the same program on dspref.Machine with set_edm() for as many
samples as the delay needs. Compared: counter - tap1, counter - tap2 (in samples) and the fraction tap3.

    python3 dsptapcheck.py run [--save FILE]     # SYSTEM-8, lab PEEK in slot 3, PLG3 selected
    python3 dsptapcheck.py check FILE            # no device
"""
import argparse, json, os, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'emu'))
os.environ.setdefault('TAPINC', '-20')          # the counter wraps after 2^21 samples: delays up to 1 unit are seen
import dsptap as T                               # noqa: E402
import dspdis                                    # noqa: E402
from dsppeek import raw_value                    # noqa: E402

U = 16384
# delays in units of 16384 samples: whole samples, fractions (1/64 steps truncated), up to one unit
CASES = [3 / U, 7.8 / U, (5 + 0.25) / U, (100 + 0.4) / U, (1000 + 0.9) / U, 0.25, 0.5, 1.0]


def program(D):
    return T.program(D, 'both', 0x4300, base=324)


def simulate(D):
    """the reference: the program sample by sample (bank 0's ring moved by hand: S -> S + 1), the outputs after
    enough samples for the tap to read written samples"""
    import dspref
    ins = [i for _, i in dspdis.instructions(program(D))]
    m = dspref.Machine(context='master_dsp4_efdc')
    m.set_edm()
    pid = m.add(ins)
    n = int(D * U) + 64
    for _ in range(n):
        m.mem[T.S + 1] = m.mem.get(T.S)
        m.run_program(pid)
        m.end_sample()
    return [m.mem.get(T.OUT5 + k) for k in range(6)]


def summary(v):
    return {'c-tap1': T.d(v[4]), 'c-tap2': T.d(v[5]), 'frac': v[2]}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cmd', choices=['run', 'check'])
    ap.add_argument('file', nargs='?')
    ap.add_argument('--save')
    a = ap.parse_args()
    if a.cmd == 'run':
        import dspmaster
        from dsppeek import Peek
        dspmaster.CONTEXT = 'efdc'
        out = open(a.save, 'w') if a.save else None
        with dspmaster.Session(Peek(), 4, 0xF7DC, size=160) as ses:
            for D in CASES:
                ses.load(program(D))
                time.sleep(max(0.3, D * U / 96000 + 0.2))
                reads = [list(ses.pk.read(T.DPC_RAW[ses.dsp] + 4 * (T.OUT5 & 0x3FF), 6)) for _ in range(3)]
                rec = {'D': D, 'reads': reads}
                if out:
                    out.write(json.dumps(rec) + '\n')
                    out.flush()
                print(D * U, [summary([raw_value(x) for x in r]) for r in reads], flush=True)
        return 0
    bad = 0
    for line in open(a.file):
        rec = json.loads(line)
        sim = summary(simulate(rec['D']))
        hw = [summary([raw_value(x) for x in r]) for r in rec['reads']]
        ok = all(abs(h['c-tap1'] - sim['c-tap1']) < 1e-6 and abs(h['c-tap2'] - sim['c-tap2']) < 1e-6 and
                 h['frac'] == sim['frac'] for h in hw)
        bad += not ok
        print(f'delay {rec["D"] * U:10.3f} samples: hw {hw[0]}  reference {sim}  {"OK" if ok else "DIFFERENT"}')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
