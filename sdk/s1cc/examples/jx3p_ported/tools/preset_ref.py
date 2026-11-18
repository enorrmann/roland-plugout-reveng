#!/usr/bin/env python3
"""preset_ref.py: Roland's JX-3P PLUG-OUT presets rendered by Roland's own programs, the reference our JX-3P-S1's
captures are compared with (README.md §3.6).

    python3 sdk/s1cc/examples/jx3p_ported/tools/preset_ref.py "1 Preset.bin:1-8" [--note 60] [--on 1.0] [--secs 1.6]
        -> work/jx3p/preset_ref/<n>.npy (voice 0's output: DSP4 bank-1 cell 2229, as work/jx3p/rec.py; 96 kHz)

The S8 slave chain in the emulator (work/jx3p/ref.py: MAIN0..MAIN5 and the DCO slot programs from the user's
plugins/JX-3P; examples/from_scratch's model writes the DSP words of the patch values), one note on voice 0. The DCO
slot programs follow the preset's waveforms (DCO1 / DCO2 WAVEFORM 1 = the sqr program, 0 / 2 the saw program, DCO2 3
the noise program); VCA MODE is the model's 0x736. Not in it: the master (effects, the noise word: NOISE and RANDOM
are silent here), velocity curves beyond the model's.
"""
import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..', '..', '..'))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(ROOT, 'work', 'jx3p'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'system_1_jx'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import patches as PT          # noqa: E402
import ref                    # noqa: E402
import s1jx                   # noqa: E402
import jxpatch                # noqa: E402
import make_voice as MV       # noqa: E402
sys.path.insert(0, os.path.join(ROOT, 'sdk', 's1cc', 'examples', 'boutique_ported', 'tools'))
from jupiter8_bank import j8_values   # noqa: E402  (the patch struct of every PLUG-OUT: 64 int2x4, name, 48 int8x4)

JX = os.path.join(ROOT, 'plugins', 'JX-3P')

VCA_MODE_ID = 0x736           # examples/from_scratch/engine/model_spec.py


def dco2_words(wave):
    """DCO2's slot program for DSP2 (bank 6 at C000 + 0x200 * 2 + idx, as s1jx.emu_map_s8)"""
    name = {1: 'dco2_sqr', 3: 'dco2_noise'}.get(wave, 'dco2_saw')
    p = MV.R.Prog(MV.roland(name), 'DCO2')
    return p.relocate(lambda r: 0xC000 + 0x400 + (r.addr & 0x1FF) if r.addr >> 12 == 0xC else r.addr)


def render(v, note=60, on=1.0, secs=1.6):
    ids = {n: i for i, n, _d in jxpatch.patch_params()}
    vals = s1jx.default_vals()
    for n, x in v.items():
        if n in ids:
            vals[ids[n]] = x
    vals[VCA_MODE_ID] = v.get('VCA MODE', 1)
    vals[0x7E4] = s1jx.float_bits(vals[ids['VCF CUTOFF FREQ']] / 255)
    vals[0x74D] = s1jx.float_bits(vals[ids['LFO RATE']] / 255)
    sl, wr = ref.make(dco1='sqr' if v['DCO1 WAVEFORM'] == 1 else 'saw', dco2=dco2_words(v['DCO2 WAVEFORM']),
                      vals=vals)
    for k in s1jx.note_writes(0, note, 100):
        wr(*k)
    key = (4, 0x2229)
    a = sl.run(int(on * 96000), (key,))
    for k in s1jx.note_writes(0, note, 0):
        wr(*k)
    b = sl.run(int((secs - on) * 96000), (key,))
    return np.array(a[key] + b[key])


def main():
    argv = sys.argv[1:]

    def take(flag, d):
        if flag in argv:
            i = argv.index(flag)
            x = argv[i + 1]
            del argv[i:i + 2]
            return x
        return d
    note, on, secs = int(take('--note', 60)), float(take('--on', 1.0)), float(take('--secs', 1.6))
    out = os.path.join(ROOT, 'work', 'jx3p', 'preset_ref')
    os.makedirs(out, exist_ok=True)
    for spec in argv:
        path, _, rng = spec.partition(':')
        if not os.path.exists(path):
            path = os.path.join(JX, 'Patch', path)
        allp = j8_values(path, os.path.join(JX, 'Script', 'Script.xml'))
        for i in (PT.parse_ranges(rng, len(allp)) if rng else range(len(allp))):
            name, v = allp[i]
            x = render(v, note, on, secs)
            np.save(os.path.join(out, '%d.npy' % (i + 1)), x)
            print('%d %-18s peak %.4f' % (i + 1, name, np.abs(x).max()))


if __name__ == '__main__':
    main()
