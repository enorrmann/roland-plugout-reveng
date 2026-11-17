"""make_chorus.py: the S8 JX-3P PLUG-OUT's EFFECT TYPE 2 "JX-3P CHORUS" (DSP4 slot b000), lifted and measured from
Roland's own engine. README.md §8.

    python3 sdk/s1cc/examples/jx3p_ported/tools/make_chorus.py

1. Roland's program `fx_ch` (examples/from_scratch/engine/programs.py, SHA-256 checked) -> work/jx3p/chorus/fx_ch.hex
   (execution-order words), fx_ch.s (asm/disasm.py listing).
2. The lift (sdk/s1cc/tools/dsplift.py) -> jx3p_chorus_lift.c: a study copy. The delay memory is not modelled there
   (dsplift: the taps read 0; f4 / f5 / f6 on h4..h7 are the line's position arithmetic), so it does not run as a
   chorus; jx3p_chorus.h is the model (README.md §8.2).
3. The chorus's parameter words as Roland's ARM engine computes them (unicorn, tools/plugout_dsp_tables.py's Engine):
   the engine's effect object hands every word to its writer (region 1 0x60a0b41e / 0x60a0b43a: object, ., index, .,
   s0 = value) before the OS call, and plugout_dsp_tables.py only logs the OS call, which this path defers: the words
   are taken at the writer's entry. EFFECT TYPE = 2 (id 0x748) writes C1F7, C1F5, C1F4, C1F1, C1F6, C1F2, C1F3;
   EFFECT LEVEL (0x6F9) C1F2 / C1F3; EFFECT TONE (0x749) C1F6.
   -> jx3p_chorus_tables.h (the constants and the 256-value rows) and work/jx3p/chorus/fx_ch_words.json.
"""
import json
import os
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
EX = os.path.abspath(os.path.join(HERE, '..'))
ROOT = os.path.abspath(os.path.join(EX, '..', '..', '..', '..'))
OUT = os.path.join(ROOT, 'work', 'jx3p', 'chorus')
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(ROOT, 'asm'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))

EFFECT_TYPE, EFFECT_LEVEL, EFFECT_TONE = 0x748, 0x6F9, 0x749     # patch3 / patch ids (Script.xml, test_emu.py)
CHORUS = 2                                                      # effectTypeL: OVERDRIVE, DISTORTION, JX-3P CHORUS, ...
WRITERS = (0x60a0b41e, 0x60a0b43a)                              # the effect object's word writers [code]


def program():
    import make_voice as M
    import disasm
    import isa
    w = M.roland('fx_ch')
    os.makedirs(OUT, exist_ok=True)
    hexp = os.path.join(OUT, 'fx_ch.hex')
    open(hexp, 'w').write(' '.join('%04x' % x for x in w) + '\n')
    text, rep = disasm.program(w, isa.Table.load(), 'fx_ch')
    open(os.path.join(OUT, 'fx_ch.s'), 'w').write(text)
    print('fx_ch: %d words, %d instructions (%d kept verbatim)' % (len(w), rep['instructions'], rep['raw']))
    return hexp


def lift(hexp):
    out = os.path.join(EX, 'jx3p_chorus_lift.c')
    subprocess.run([sys.executable, os.path.join(ROOT, 'sdk', 's1cc', 'tools', 'dsplift.py'), out,
                    'jx3p_chorus=' + hexp], check=True)
    print(out)


def words():
    """{'type': {idx: value}, 'level': [{idx: value}] * 256, 'tone': [...]} from the engine's writer"""
    import plugout_dsp_tables as T
    import unicorn as U
    from unicorn import arm_const as A
    img, base = T.unpack(os.path.join(ROOT, 'plugins', 'JX-3P'), 'Code8_1.Dat')
    eng = T.Engine(img, base)
    e = eng.e
    got = []

    def hook(uc, addr, size, ud):
        obj = uc.reg_read(A.UC_ARM_REG_R0)
        if struct.unpack('<I', uc.mem_read(obj + 4, 4))[0] != 4:          # the object's DSP: 4 (the effect slot)
            return
        idx = uc.reg_read(A.UC_ARM_REG_R2)
        v = struct.unpack('<f', struct.pack('<I', uc.reg_read(A.UC_ARM_REG_S0) & 0xFFFFFFFF))[0]
        got.append((idx, v))
    for a in WRITERS:
        e.uc.hook_add(U.UC_HOOK_CODE, hook, begin=a, end=a + 2)

    def setp(pid, v):
        got.clear()
        eng.vals[pid] = v
        e.call(e.u32(eng.vt + T.SETP_SLOT), eng.o, pid, 0, v)
        return {'C%03X' % i: x for i, x in got if i >= 0x1F0}       # C000 + index: C1F0 ..
    r = {'type': setp(EFFECT_TYPE, CHORUS)}
    r['level'] = [setp(EFFECT_LEVEL, v) for v in range(256)]
    r['tone'] = [setp(EFFECT_TONE, v) for v in range(256)]
    json.dump(r, open(os.path.join(OUT, 'fx_ch_words.json'), 'w'), indent=0)
    return r


def tables(r):
    t = r['type']
    lvl = [row['C1F2'] for row in r['level']]
    tone = [row['C1F6'] for row in r['tone']]
    assert all(row.get('C1F3') == 1.0 for row in r['level']), 'C1F3 is not 1 for every EFFECT LEVEL'
    def f(x):
        t = '%.9g' % x
        return (t if ('.' in t or 'e' in t) else t + '.0') + 'f'
    arr = lambda name, v: ('static const float %s[256] = {\n' % name
                           + ''.join('    ' + ', '.join(f(x) for x in v[i:i + 8]) + ',\n' for i in range(0, 256, 8))
                           + '};\n')
    s = ['/* jx3p_chorus_tables.h: the JX-3P CHORUS words as Roland\'s S8 engine computes them (EFFECT TYPE 2).',
         '   Written by sdk/s1cc/examples/jx3p_ported/tools/make_chorus.py; do not edit. README.md §8. */',
         '#ifndef JX3P_CHORUS_TABLES_H', '#define JX3P_CHORUS_TABLES_H',
         '#define JXC_C1F0 %s   /* the input\'s gain (vt[0] of the effect object) */' % f(t.get('C1F0', 1.0)),
         '#define JXC_C1F1 %s   /* the dry path\'s blend (out = C1F1 C1F3 x + (1 - C1F1) x) */' % f(t['C1F1']),
         '#define JXC_C1F3 %s   /* (EFFECT LEVEL writes it 1 for every value) */' % f(t['C1F3']),
         '#define JXC_C1F4 %s   /* LFO depth on the BBD clock */' % f(t['C1F4']),
         '#define JXC_C1F5 %s   /* LFO phase step per sample (the phase wraps over [-1, 1)) */' % f(t['C1F5']),
         '#define JXC_C1F7 %s   /* the output stage\'s coefficient */' % f(t['C1F7']),
         '/* EFFECT LEVEL 0..255 -> C1F2 (the wet gain, x 1.1 in the program); EFFECT TONE 0..255 -> C1F6 */',
         arr('jxc_level', lvl), arr('jxc_tone', tone), '#endif', '']
    path = os.path.join(EX, 'jx3p_chorus_tables.h')
    open(path, 'w').write('\n'.join(s))
    print(path)
    print('type 2:', ' '.join('%s=%.9g' % kv for kv in sorted(t.items())))


def main():
    lift(program())
    tables(words())


if __name__ == '__main__':
    main()
