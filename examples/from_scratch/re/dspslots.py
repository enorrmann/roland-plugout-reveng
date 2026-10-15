"""Map the word positions of an instruction class (template) on a master DSP: which position acts as
an ALU op, which as a transfer, and which make the instruction illegal.

    python3 dspslots.py 0x0a0 [0x158 ...] [--len 5]      # header = class << 5 | len
    python3 dspslots.py all --top 40

For each body position p (all other words 0000, neutral as op / transfer / immediate):
  probes 3c01 and 5c01: as an A-op they give r3 = .75 / r5 = .75 (r0 + r1); as a transfer they load
  r3 / r5 from memory; as a slot (reference) they change what the filler transfers (0000 = load r0
  from slot 0) read; as an immediate they change a value.
Printed: the registers that changed, for each probe; 'X' = nothing stored (program stopped). Uses dspprobe.probe (r0..r7 = REGS).
"""
import argparse, collections, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value   # noqa: E402
import dspprobe                        # noqa: E402
import dspclass                        # noqa: E402

REGS = dspprobe.REGS


def classify(after, probe):
    v = [raw_value(w) for w in after]
    if all(w == 0 for w in after):
        return 'X'
    ch = [i for i in range(8) if abs(v[i] - REGS[i]) > 1e-9]
    return ','.join(f'r{i}={v[i]:.4g}' for i in ch) or '-'



def run(pk, cls, length):
    hdr = cls << 5 | length
    out = []
    for p in range(1, length):
        res = ''
        for kind, word in (('3', 0x3c01), ('5', 0x5c01)):
            body = [0] * (length - 1)
            body[p - 1] = word
            line = ' '.join(f'{w:04x}' for w in [hdr] + body)
            res += classify(dspprobe.probe(pk, [line]), kind) + ' / '
        out.append(res.strip(' /'))
    return hdr, out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('classes', nargs='+')
    ap.add_argument('--len', type=int)
    ap.add_argument('--top', type=int, default=40)
    a = ap.parse_args()
    c = dspclass.corpus()
    lens = collections.defaultdict(collections.Counter)
    tot = collections.Counter()
    for ins, f in c.items():
        lens[ins[0] >> 5][len(ins)] += f
        tot[ins[0] >> 5] += f
    classes = [k for k, _ in tot.most_common(a.top)] if a.classes == ['all'] else [int(x, 0) for x in a.classes]
    pk = Peek()
    for cls in classes:
        for length in ([a.len] if a.len else [lens[cls].most_common(1)[0][0]]):
            if length < 2:
                continue
            hdr, res = run(pk, cls, length)
            print(f'{cls:#06x} {hdr:04x} len {length}: ' + ' | '.join(f'{p}:{r}' for p, r in enumerate(res, 1)), flush=True)




def ab_main():
    """second pass: tell unit A from unit B at every 'A' position of the measured templates
    (probe 3cc1: unit A r3 = r3 + r1 = -0.125, unit B r3 = |r3| + r1 = 0.625)"""
    tpath = os.path.join(HERE, 'dsp_templates.txt')
    pk = Peek()
    for line in open(tpath):
        w = line.split()
        cls, length, t = int(w[0], 16), int(w[1]), w[3]
        res = []
        for p, k in enumerate(t, 1):
            if k != 'A':
                continue
            body = [0] * (length - 1)
            body[p - 1] = 0x3cc1
            v = raw_value(dspprobe.probe(pk, [' '.join(f'{x:04x}' for x in [cls << 5 | length] + body)])[3])
            res.append(f'{p}:' + ('A' if abs(v + 0.125) < 1e-9 else 'B' if abs(v - 0.625) < 1e-9 else f'?{v:.4g}'))
        print(f'{cls:#06x} {length:2d} {t:14s} ' + ' '.join(res), flush=True)


if __name__ == '__main__':
    ab_main() if sys.argv[1:] == ['ab'] else main()
