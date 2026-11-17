"""env_ref.py: the JX-3P's envelopes as Roland's MAIN0 makes them, for tools/env_check.c (README.md §3.4).

    python3 sdk/s1cc/examples/jx3p_ported/tools/env_ref.py      -> work/jx3p/env_ref.txt

Runs the S8 JX-3P PLUG-OUT's slave chain in the emulator (examples/system_1_jx s1jx / the work/jx3p ref chain, Roland's
programs from the user's plugins/JX-3P), one note on voice 0, and records MAIN0's ENV1 / ENV2 words (MXM 840A / 840B)
at every tick (192 samples). Format: per case a line `case A D S R OFF NT` (knob values 0..255, note off at tick OFF,
NT ticks), then NT lines `env1 env2`.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'work', 'jx3p'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'system_1_jx'))
import ref    # noqa: E402  (work/jx3p/ref.py: the S8 chain with the JX-3P programs)
import s1jx   # noqa: E402

CASES = [(0, 0, 255, 0), (128, 128, 128, 128), (64, 64, 192, 64), (255, 200, 30, 200), (10, 40, 0, 10),
         (200, 0, 100, 255)]
NT, OFF = 700, 400


def main():
    import json
    J = json.load(open(os.path.join(ROOT, 'work', 'jx3p', 'jx_arm.json')))
    out = []
    for a, d, s, r in CASES:
        v = s1jx.default_vals()
        v.update({0x6EF: a, 0x6F0: d, 0x6F1: s, 0x6F2: r, 0x6F4: a, 0x6F5: d, 0x6F6: s, 0x6F7: r})
        v[0x7E4] = s1jx.float_bits(v[0x6EA] / 255)
        v[0x74D] = s1jx.float_bits(v[0x6CF] / 255)
        sl, wr = ref.make(vals=v)
        # the envelope words as Roland's ARM writes them (jx_arm.json, tools/plugout_dsp_tables.py), over the ref
        # chain's model (examples/from_scratch: an older reconstruction; its ATTACK 128 is 0.0035784, Roland's ARM
        # 0.0035947): the check compares the formula, both sides with the same words
        for pid, i, kv in (('0x6ef', 0x2F, a), ('0x6f0', 0x30, d), ('0x6f1', 0x31, s), ('0x6f2', 0x32, r),
                           ('0x6f4', 0x33, a), ('0x6f5', 0x34, d), ('0x6f6', 0x35, s), ('0x6f7', 0x36, r)):
            wr('P', 0, i, J['params'][pid]['rows'][kv]['0:C:%03x' % i])
        keys = [(2, 0x800A), (2, 0x800B)]
        for k in s1jx.note_writes(0, 60, 100):
            wr(*k)
        x = sl.run(OFF * 192, keys)
        for k in s1jx.note_writes(0, 60, 0):
            wr(*k)
        y = sl.run((NT - OFF) * 192, keys)
        e1, e2 = x[keys[0]] + y[keys[0]], x[keys[1]] + y[keys[1]]
        out.append('case %d %d %d %d %d %d' % (a, d, s, r, OFF, NT))
        out += ['%.9g %.9g' % (e1[t * 192], e2[t * 192]) for t in range(NT)]
        print('case', a, d, s, r)
    path = os.path.join(ROOT, 'work', 'jx3p', 'env_ref.txt')
    open(path, 'w').write('\n'.join(out) + '\n')
    print(path)


if __name__ == '__main__':
    main()
