"""Patch ids of the JX-3P panel parameters, from the plugin's own Script.xml (no hardware needed).

    python3 patch_ids.py                 -> base check and the ENV, PIT and FLT tables
    python3 patch_ids.py "ENV2 ATTACK"   -> id of one name

Method (2026-10-21, docs/log/WAVE_BUS_LOG.md §4l-§4m):
  1. Read every <value> that has <name>, <range> and <default> in document order (system fields without range are skipped).
  2. Find the base b such that id = b + index matches the INIT patch of model/params.py (100 % of its entries).
  3. Check the names against the model (engine/model_spec.py: env_value uses 0x6ef / 0x6f4 for ENV1 / ENV2).
"""
import os, re, sys
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
XML = os.path.join(ROOT, 'plugins', 'JX-3P', 'Script', 'Script.xml')
PARAMS = os.path.join(ROOT, 'examples', 'from_scratch', 'model', 'params.py')


def values():
    x = open(XML, encoding='utf-8', errors='replace').read()
    out = []
    for m in re.finditer(r'<structType>(.*?)</structType>', x, re.S):
        for v in re.finditer(r'<value>\s*<type>[^<]*</type>\s*<name>(.*?)</name>\s*<range>[^<]*</range>\s*<default>(-?\d+)</default>', m.group(1), re.S):
            out.append((v.group(1), int(v.group(2))))
    return out


def init_patch():
    src = open(PARAMS).read()
    blk = src[src.index('INIT = {'):src.index('}', src.index('INIT = {'))]
    return {int(k, 16): int(v) for k, v in re.findall(r'(0x[0-9a-f]+):\s*(\d+)', blk)}


def base(vals, init):
    best = None
    for b in range(0x600, 0x800):
        tot = ok = 0
        for pid, val in init.items():
            i = pid - b
            if 0 <= i < len(vals):
                tot += 1; ok += vals[i][1] == val
        if tot >= 8 and (best is None or ok / tot > best[0]):
            best = (ok / tot, tot, b)
    return best


if __name__ == '__main__':
    vals = values(); init = init_patch()
    ratio, n, b = base(vals, init)
    print('base %#x: %d/%d INIT entries match the XML defaults' % (b, round(ratio * n), n))
    if len(sys.argv) > 1:
        for i, (name, d) in enumerate(vals):
            if name == sys.argv[1]:
                print('%s -> id %#x (default %d)' % (name, b + i, d))
    else:
        for key in ('ENV1 ATTACK', 'ENV1 DECAY', 'ENV1 SUSTAIN', 'ENV1 RELEASE',
                    'ENV2 ATTACK', 'ENV2 DECAY', 'ENV2 SUSTAIN', 'ENV2 RELEASE', '(PITCH ATTACK)', '(PITCH DECAY)'):
            i = [v[0] for v in vals].index(key)
            print('%-14s id %#x  default %3d  INIT %s' % (key, b + i, vals[i][1], init.get(b + i, '-')))
