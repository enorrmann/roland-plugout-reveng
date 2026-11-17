#!/usr/bin/env python3
"""plugout_dsp_tables.py -- what a PLUG-OUT's engine sends to the DSPs for every value of every patch parameter.

    python3 tools/plugout_dsp_tables.py PLUGIN_DIR -o OUT.json [--dsps 0,2,4] [--code Code8_1.Dat] [--id-base 0x6CD]

For porting a SYSTEM-8 / SYSTEM-1 PLUG-OUT synth (first used for the JX-3P, sdk/s1cc/examples/jx3p_ported): the knob
-> DSP word tables come from Roland's own engine, run in unicorn, instead of being re-derived by hand per synth.

1. Unpacks region 1 (the engine) of PLUGIN_DIR/Script/CODE (tools/plugout_build.py unpack) to work/<plugin>/.
2. Boots it the way the OS does [code, JX-3P 2026-10]: init = SYNINFO +0x24 -> descriptor D; call D+4; init again;
   object = [D + 20], vtable = [object]; call vtable[0](object).
3. Every patch parameter of Script.xml's "patch" struct (tools/patches.py; id = ID_BASE + nibble offset / 2) is set to
   its default, then to every value 0..255 with the engine's setter (vtable +0x30)(object, id, 0, value), and the
   OS-table calls are recorded:
     +a0(dsp, idx, type, _, s0)            DSP parameter word C000 + idx = s0
     +b0(dsp, obj, 0x0F0002C1, _, s0)      per-voice word: bank 5 A000 + [obj] = s0 (the engine's descriptor; its
                                           first word is the bank-5 index)
     +70(dsp, prog, data, ...)             a program swapped into a slot (kept as "slot:<dsp>:<prog address>")
4. Writes OUT.json: {"init": {word: value} (what boot + the defaults wrote), "params": {"0x6cd": {"name", "default", "words", "diff", "rows"}}}
   word = "D:C:idx" (parameter) or "D:A:idx" (bank 5) in hex; rows[v] = {word: value} for v = 0..255 (every word
   that parameter writes for some value; None where that value does not write it); diff = the words that change.

Needs unicorn (plain python3, not -I). The engine's other services return 0 (examples/from_scratch/re/emu1.py).
"""
from __future__ import annotations

import argparse
import json
import os
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
sys.path.insert(0, HERE)
SETP_SLOT = 0x30          # the engine's parameter setter: vtable + 0x30 (object, id, 0, value)
B5_MSG = 0x0F0002C1       # +b0's third argument for a per-voice bank-5 word [code, JX-3P]


def unpack(plugin, code):
    out = os.path.join(ROOT, 'work', os.path.basename(os.path.normpath(plugin)).lower() + '_engine')
    if not os.path.exists(os.path.join(out, 'region1.bin')):
        subprocess.run([sys.executable, os.path.join(HERE, 'plugout_build.py'), 'unpack',
                        os.path.join(plugin, 'Script', code), out], check=True, stdout=subprocess.DEVNULL)
    img = open(os.path.join(out, 'region1.bin'), 'rb').read()
    # the engine runs where its slot puts it (slot 1: 0x60A00000), not at the container's load address: the base is
    # the 1 MiB block of its own SYNINFO init pointer
    return img, struct.unpack_from('<I', img, 0x24)[0] & 0xFFF00000


class Engine:
    def __init__(self, img, base):
        from emu1 import Emu1
        self.e = e = Emu1(img, base=base, getval=lambda r0, r1, r2: self.vals.get(r2, 0))
        e.PREC = None
        self.vals = {}
        init = struct.unpack_from('<I', img, 0x24)[0] & ~1
        d = e.call(init)
        e.call(e.u32(d + 4))
        d = e.call(init)
        self.o = e.u32(d + 20)
        self.vt = e.u32(self.o)
        e.call(e.u32(self.vt), self.o)

    def setp(self, pid, v):
        self.vals[pid] = v
        self.e.log.clear()
        self.e.call(self.e.u32(self.vt + SETP_SLOT), self.o, pid, 0, v)
        return self.words(self.e.log)

    def words(self, log, dsps=None):
        w = {}
        for off, r0, r1, r2, r3, s0, st in log:
            if dsps is not None and r0 not in dsps:
                continue
            if off == 0xA0:
                w['%d:C:%03x' % (r0, r1)] = s0
            elif off == 0xB0 and r2 == B5_MSG:
                w['%d:A:%03x' % (r0, self.e.u32(r1))] = s0
            elif off == 0x70:
                w['slot:%d:%x' % (r0, r2)] = r1
        return w


def patch_params(script, id_base):
    """[(id, name, default)] of the patch struct (examples/from_scratch/re/jxpatch.py, generalized)"""
    import xml.etree.ElementTree as ET
    import patches as P
    lay = P.ScriptLayout(script)
    root = ET.parse(script).getroot()
    types = {e.findtext('type'): e for e in root.findall('structType')}
    out = []
    for leaf in lay.leaves['patch']:
        t = leaf.path
        e = types.get(t) or types.get({'NAME0': 'PAT_NAME'}.get(t, t))
        if e is None or t.startswith('NAME') or t.startswith('PAT_NAME'):
            continue
        off = leaf.offset
        for v in e.findall('value'):
            n = int(v.findtext('type')[3])
            out.append((id_base + off // 2, v.findtext('name'), int(v.findtext('default') or 0)))
            off += n
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0], formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('plugin', help='PLUG-OUT folder (plugins/NAME: Script/Script.xml and the code file)')
    ap.add_argument('-o', '--out', required=True)
    ap.add_argument('--code', default='Code8_1.Dat', help='the code file in Script/ (default Code8_1.Dat)')
    ap.add_argument('--dsps', default='0,1,2,3,4,5', help='DSPs to keep (default all six)')
    ap.add_argument('--id-base', default='0x6CD', help='parameter id of the patch struct\'s first nibble (JX-3P 0x6CD)')
    a = ap.parse_args()
    dsps = {int(x) for x in a.dsps.split(',')}
    img, base = unpack(a.plugin, a.code)
    eng = Engine(img, base)
    pp = patch_params(os.path.join(a.plugin, 'Script', 'Script.xml'), int(a.id_base, 0))
    boot = list(eng.e.log)                 # what the engine wrote while it booted (its constructor's words)
    eng.e.log.clear()
    for i, n, d in pp:
        eng.e.call(eng.e.u32(eng.vt + SETP_SLOT), eng.o, i, 0, d)
        eng.vals[i] = d
    init = eng.words(boot + eng.e.log, dsps)
    out = {'plugin': os.path.basename(os.path.normpath(a.plugin)), 'init': init, 'params': {}}
    for i, n, d in pp:
        rows = []
        for v in range(256):
            eng.setp(i, v)
            rows.append(eng.words(eng.e.log, dsps))
        eng.setp(i, d)
        init.update(eng.words(eng.e.log, dsps))
        keys = sorted(set(k for r in rows for k in r))
        diff = [k for k in keys if len(set(r.get(k) for r in rows)) > 1]
        out['params']['0x%03x' % i] = {'name': n, 'default': d, 'words': keys, 'diff': diff,
                                       'rows': [{k: r.get(k) for k in keys} for r in rows]}
        print('%#x %-24s %2d words, %2d change' % (i, n, len(keys), len(diff)))
    os.makedirs(os.path.dirname(os.path.abspath(a.out)), exist_ok=True)
    json.dump(out, open(a.out, 'w'))
    print(a.out)


if __name__ == '__main__':
    main()
