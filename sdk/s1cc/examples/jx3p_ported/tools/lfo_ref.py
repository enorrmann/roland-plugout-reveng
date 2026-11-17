"""lfo_ref.py: the JX-3P's LFO as Roland's MAIN0 makes it, for the ARM side's check (README.md §3.5).

    python3 sdk/s1cc/examples/jx3p_ported/tools/lfo_ref.py      -> work/jx3p/lfo_ref.npz, lfo_ref.txt

The S8 chain with Roland's programs (work/jx3p/ref.py), one note on voice 0 at sample 0; every LFO word as Roland's
ARM writes it (jx_arm.json: LFO WAVEFORM, RATE, DELAY TIME, KEY TRIG, TEMPO SYNC); MAIN0's LFO words recorded per
sample: MXM 8405 (lfo, the voice's A0A5) and 8406 (A0A6). Cases: (waveform, rate, delay, key trig) knob values.
"""
import json
import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'work', 'jx3p'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'system_1_jx'))
import ref    # noqa: E402
import s1jx   # noqa: E402

CASES = [(0, 175, 0, 0), (0, 100, 0, 0), (0, 230, 0, 0), (1, 175, 0, 0), (2, 175, 0, 0), (0, 175, 128, 0),
         (0, 175, 60, 1), (1, 60, 0, 1)]
N = 96000
PIDS = {'wave': '0x6cd', 'delay': '0x6ce', 'rate': '0x6cf', 'kt': '0x6d3', 'sync': '0x702'}


def words(J, pid, k):
    row = J['params'][pid]['rows'][k]
    return [(key, v) for key, v in row.items() if v is not None and key[0] == '0']


def main():
    J = json.load(open(os.path.join(ROOT, 'work', 'jx3p', 'jx_arm.json')))
    rec = {}
    for wave, rate, delay, kt in CASES:
        v = s1jx.default_vals()
        v.update({0x6CD: wave, 0x6CF: rate, 0x6CE: delay, 0x6D3: kt})
        v[0x7E4] = s1jx.float_bits(v[0x6EA] / 255)
        v[0x74D] = s1jx.float_bits(rate / 255)
        sl, wr = ref.make(vals=v)
        for name, k in (('wave', wave), ('delay', delay), ('rate', rate), ('kt', kt), ('sync', 0)):
            for key, x in words(J, PIDS[name], k):
                d, kind, idx = key.split(':')
                wr('P' if kind == 'C' else 'B', int(d), int(idx, 16), x)
        keys = [(2, 0x8005), (2, 0x8006)]
        for kk in s1jx.note_writes(0, 60, 100):
            wr(*kk)
        r = sl.run(N, keys)
        rec['%d_%d_%d_%d' % (wave, rate, delay, kt)] = np.array([r[keys[0]], r[keys[1]]])
        print('case', wave, rate, delay, kt, 'lfo min %.4f max %.4f' % (min(r[keys[0]]), max(r[keys[0]])))
    path = os.path.join(ROOT, 'work', 'jx3p', 'lfo_ref.npz')
    np.savez(path, **rec)
    with open(os.path.join(ROOT, 'work', 'jx3p', 'lfo_ref.txt'), 'w') as f:     # for tools/lfo_check.c
        for name, x in rec.items():
            f.write('case %s %d\n' % (name.replace('_', ' '), N))
            f.write('\n'.join('%.7g' % v for v in x[0]) + '\n')
    print(path)


if __name__ == '__main__':
    main()
