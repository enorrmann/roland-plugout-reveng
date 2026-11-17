#!/usr/bin/env python3
"""jx3p_bank.py: patches of Roland's JX-3P PLUG-OUT bank (KoaBankFile, plugins/JX-3P/Patch/*.bin) -> SYSTEM-1 patches
of our JX-3P-S1 (README.md §3.6).

    python3 sdk/s1cc/examples/jx3p_ported/tools/jx3p_bank.py [--init N] [BANK.bin[:RANGES] ...] --bank OUT.bin [--prm DIR]
            [--show]

--init N: N patches "INIT SAW" first (INIT below: DCO1 saw alone at the level of README.md §5.1, 8', the filter open,
no modulation, ENV2 an organ gate), then the converted ones.

The same instrument and the same parameter rows (the port reads Roland's rows, tools/make_arm.py), so the values map
one to one onto the SYSTEM-1 controls the port reads (README.md §3.2); the bank / .PRM files are written by
tools/s1_bank.py (the SYSTEM-1's patch layout). Mapping:

    VCF CUTOFF FREQ, RESONANCE, ENV1 MOD, LFO MOD; DCO LFO MOD, ENV1 MOD; DCO1 / DCO2 LEVEL, RANGE, WAVEFORM; ENV1 /
    ENV2 A D S R; LFO RATE, DELAY TIME, KEY TRIG; HPF CUTOFF                    -> their SYSTEM-1 controls, as is
    VCF PITCH FOLLOW (bipolar, 128 = 0)  -> KEY TRACK 2 (v - 128), limited to 0..255 (the SYSTEM-1 knob is unipolar)
    DCO1 / DCO2 FREQ MOD 0 off, 1 LFO, 2 ENV, 3 both -> OSC1 / OSC2 COLOR MOD 5, 1, 2, 0 (make_arm.py FMOD inverse)
    LFO WAVEFORM 0 SINE, 1 SQUARE, 2 RANDOM -> LFO SHAPE 0, 3, 5
    DCO2 TUNE (min(v / 127, 1) octave: examples/from_scratch/engine/model_spec.py 0x6e0) + FINE TUNE -> OSC2 COARSE
                         round(12 tune) semitones + OSC2 FINE: the row nearest to the rest
    VCA MODE (0 ENV-1, 1 ENV-2, 2 GATE, 3 = 0) -> PLUGOUT_EXTEND01; VCA LEVEL -> PLUGOUT_EXTEND02 (value - default)
    BEND RANGE, BEND GAIN, BEND SENS DCO, BEND SENS VCF, MOD SENS DCO, MOD SENS VCF -> PLUGOUT_EXTEND03..08
                         (value - default; README.md §3.7)
    BRILLIANCE -> AMP_TONE (both 0..255, 128 = 0; README.md §3.2)
    DCO2 CROSS MOD 0 off, 1 SYNC, 2 METAL -> OSC2_SYNC / OSC2_RING (the buttons' words, README.md §4.3)
    SUB OSC TYPE / NOISE TYPE (no JX-3P control) -> 0, their LEDs off (NO_LED)
    not taken: effects (the chorus: README.md §8), portamento (README.md §7), ASSIGN MODE, arpeggio
"""
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
sys.path.insert(0, os.path.join(ROOT, 'sdk', 's1cc', 'examples', 'boutique_ported', 'tools'))
import patches as PT              # noqa: E402
import s1_bank as SB              # noqa: E402
from jupiter8_bank import j8_values  # noqa: E402  (the SYSTEM-8 patch struct: 64 int2x4, name, 48 int8x4)

JX = os.path.join(ROOT, 'plugins', 'JX-3P')
TAG = b'PG-JX3PS1'
FMOD_INV = {0: 5, 1: 1, 2: 2, 3: 0}            # FREQ MOD -> COLOR MOD position (make_arm.py FMOD = [3, 1, 2, 2, 2, 0])
LFO_SHAPE = {0: 0, 1: 3, 2: 5}                 # LFO WAVEFORM -> the SYSTEM-1 LFO SHAPE that maps back to it
DIRECT = {'VCF CUTOFF FREQ': 'FLT_LPF', 'VCF RESONANCE': 'FLT_RESO', 'VCF ENV1 MOD': 'FLT_ENV',
          'VCF LFO MOD': 'LFO_FLT', 'DCO LFO MOD': 'LFO_PITCH', 'DCO ENV1 MOD': 'PIT_ENV', 'DCO1 LEVEL': 'MIX_OSC1',
          'DCO2 LEVEL': 'MIX_OSC2', 'DCO1 RANGE': 'OSC1_RANGE', 'DCO2 RANGE': 'OSC2_RANGE',
          'DCO1 WAVEFORM': 'OSC1_WAVE', 'DCO2 WAVEFORM': 'OSC2_WAVE', 'ENV1 ATTACK': 'FLT_ATK', 'ENV1 DECAY': 'FLT_DCY',
          'ENV1 SUSTAIN': 'FLT_SUS', 'ENV1 RELEASE': 'FLT_REL', 'ENV2 ATTACK': 'AMP_ATK', 'ENV2 DECAY': 'AMP_DCY',
          'ENV2 SUSTAIN': 'AMP_SUS', 'ENV2 RELEASE': 'AMP_REL', 'LFO RATE': 'LFO_RATE',
          'LFO DELAY TIME': 'LFO_FADE', 'LFO KEY TRIG': 'LFO_TRIG', 'HPF CUTOFF': 'FLT_HPF'}


# the INIT patch: the SYSTEM-1 defaults (s1_bank.py S1_DEF) with every source of the JX-3P-S1 voice but DCO1 off.
# Values as the port reads them (README.md §3.2): RANGE positions 0..5 = 64' .. 2' (3 = 8'); COLOR MOD 5 (SUB OSC) =
# FREQ MOD off; the bipolar knobs (LFO TO PITCH / FILTER, PITCH / FILTER ENV AMOUNT) 128 = 0; KEY TRACK 0 = row 128 =
# no follow; MIX_OSC1 128 = the level calibrated in §5.1; CUTOFF 255, RESONANCE 0, HPF 0; ENV2 A 0, D 0, S 255, R 0;
# PLUGOUT_EXTEND01 0 = VCA MODE ENV-2, PLUGOUT_EXTEND02 0 = VCA LEVEL default.
INIT = {'LFO_WAVE': 0, 'LFO_FADE': 0, 'LFO_RATE': 0, 'LFO_PITCH': 128, 'LFO_FLT': 128, 'LFO_AMP': 128, 'LFO_TRIG': 0,
        'OSC1_WAVE': 0, 'OSC1_MOD': 5, 'OSC1_RANGE': 3, 'OSC1_COLOR': 0,
        'OSC2_WAVE': 0, 'OSC2_MOD': 5, 'OSC2_RANGE': 3, 'OSC2_TUNE': 128, 'OSC2_COARSE': 0, 'OSC2_COLOR': 0,
        'OSC2_SYNC': 0, 'OSC2_RING': 0,
        'MIX_OSC1': 128, 'MIX_OSC2': 0, 'MIX_SUB_OSC': 0, 'MIX_NOISE': 0,
        'PIT_ENV': 128, 'PIT_ATK': 0, 'PIT_DCY': 0,
        'FLT_LPF': 255, 'FLT_RESO': 0, 'FLT_HPF': 0, 'FLT_ENV': 128, 'FLT_KEY': 0,
        'FLT_ATK': 0, 'FLT_DCY': 0, 'FLT_SUS': 0, 'FLT_REL': 0,
        'AMP_ATK': 0, 'AMP_DCY': 0, 'AMP_SUS': 255, 'AMP_REL': 0,
        'CRUSHER': 0, 'REVERB': 0, 'DELAY': 0, 'DELAY_TM': 0, 'PORTAMENTO': 0, 'LEGATO': 0, 'MONO': 0,
        'PLUGOUT_EXTEND01': 0, 'PLUGOUT_EXTEND02': 0, 'AMP_TONE': 128}
# SUB OSC TYPE (0x178) / NOISE TYPE (0x179): no JX-3P control; their LEDs show the patch's word. 0 = the panel's own
# value in MANUAL (PARAMLOG 2026-11-17: 0x178 = 0, 0x179 = 0); the SYSTEM-1's default 1 lit them in every patch
NO_LED = {'MIX_OSC_TYPE': 0, 'MIX_NOISE_TYPE': 0}
# PLUGOUT_EXTEND03..08 (README.md §3.7): (JX-3P parameter, its default: Script.xml)
EXT_BM = [('BEND RANGE', 2), ('BEND GAIN', 0), ('BEND SENS DCO', 255), ('BEND SENS VCF', 255), ('MOD SENS DCO', 128),
          ('MOD SENS VCF', 128)]
INIT.update(NO_LED)


def init_patch():
    s1 = dict(SB.S1_DEF)
    s1.update(INIT)
    return s1


def fine_rows():
    """DCO2 FINE TUNE row of each value (octaves), from Roland's ARM (work/jx3p/jx_arm.json)"""
    J = json.load(open(os.path.join(ROOT, 'work', 'jx3p', 'jx_arm.json')))
    return [r['2:C:021'] for r in J['params']['0x6dd']['rows']]


def to_s1(v, fine):
    s1 = dict(SB.S1_DEF)
    s1.update({'LFO_TRIG': 0, 'CRUSHER': 0, 'REVERB': 0, 'DELAY': 0, 'DELAY_TM': 0})
    for jx, s in DIRECT.items():
        s1[s] = v[jx]
    s1['FLT_KEY'] = max(0, min(255, 2 * (v['VCF PITCH FOLLOW'] - 128)))
    s1['OSC1_MOD'] = FMOD_INV[v['DCO1 FREQ MOD'] & 3]
    s1['OSC2_MOD'] = FMOD_INV[v['DCO2 FREQ MOD'] & 3]
    s1['LFO_WAVE'] = LFO_SHAPE.get(v['LFO WAVEFORM'], 0)
    oct_ = min(v['DCO2 TUNE'] / 127.0, 1.0) + fine[v['DCO2 FINE TUNE']]
    semi = max(0, min(12, int(round(12 * min(v['DCO2 TUNE'] / 127.0, 1.0)))))
    rest = oct_ - semi / 12.0
    s1['OSC2_COARSE'] = semi
    s1['OSC2_TUNE'] = min(range(256), key=lambda f: abs(fine[f] - rest))
    mode = v['VCA MODE'] if v['VCA MODE'] in (0, 1, 2) else 0
    s1['PLUGOUT_EXTEND01'] = (mode - 1) & 0xFF
    s1['PLUGOUT_EXTEND02'] = (v['VCA LEVEL'] - 128) & 0xFF
    s1['OSC2_SYNC'] = 1 if v['DCO2 CROSS MOD'] == 1 else 0     # the OSC2 SYNC / RING buttons (README.md §4.3)
    s1['OSC2_RING'] = 1 if v['DCO2 CROSS MOD'] == 2 else 0
    s1['AMP_TONE'] = v['BRILLIANCE']
    for n, (jx, d) in enumerate(EXT_BM):
        s1['PLUGOUT_EXTEND%02d' % (3 + n)] = (v[jx] - d) & 0xFF
    s1.update(NO_LED)
    return s1


def main():
    argv = sys.argv[1:]

    def take(flag):
        if flag in argv:
            i = argv.index(flag)
            x = argv[i + 1]
            del argv[i:i + 2]
            return x
        return None
    bank, prm, ninit = take('--bank'), take('--prm'), int(take('--init') or 0)
    show = '--show' in argv
    specs = [a for a in argv if a != '--show']
    if not specs and not ninit:
        raise SystemExit(__doc__)
    fine = fine_rows()
    pats = [(init_patch(), 'INIT SAW')] * ninit
    if ninit:
        print('INIT SAW x %d' % ninit)
    for spec in specs:
        path, _, rng = spec.partition(':')
        if not os.path.exists(path):
            path = os.path.join(JX, 'Patch', path)
        allp = j8_values(path, os.path.join(JX, 'Script', 'Script.xml'))
        for i in (PT.parse_ranges(rng, len(allp)) if rng else range(len(allp))):
            name, v = allp[i]
            s1 = to_s1(v, fine)
            short = name.split(' [')[0][:16]
            pats.append((s1, short))
            print('%s %d: "%s"' % (os.path.basename(path), i + 1, short))
            if show:
                print('  SYSTEM-1: ' + ', '.join('%s %d' % (n, s1[n]) for n in SB.S1_NAMES if s1[n] != SB.S1_DEF[n]))
    if prm:
        os.makedirs(prm, exist_ok=True)
        for i, (s1, name) in enumerate(pats):
            SB.write_prm(os.path.join(prm, 'PLUGOUT_PATCH%d.PRM' % (i + 1)), s1, name)
    if bank:
        empty = dict(SB.S1_DEF)
        SB.write_bank(bank, pats, TAG, empty)
        print('%s: %d patches (+ %d empty)' % (bank, min(len(pats), 64), 64 - min(len(pats), 64)))


if __name__ == '__main__':
    main()
