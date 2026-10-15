"""Name the CJx3pSim fields in Ghidra output (Decompile.java).

    python3 annotate.py plugins/JX-3P/JX-3P.vst3 in.c out.c [voice]

*(T *)(param_1 + 0xOFF) -> P_<name> for registered parameters (vst_sim.py),
st_<off> for the rest (internal state); voice v's fields (+0x3f80*v) get the
voice-0 names.
"""
import os, re, sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from vst_rtti import Dll
import vst_sim

HELPERS = {
    'FUN_1803b23f0': 'wrap1',          # fold into [-1, 1) (fmodf)
    'FUN_1803b2480': 'tri',            # wrap1, then triangle
    'FUN_1803b2270': 'poly_table',     # piecewise polynomial (0xd0-byte segments, double)
    'FUN_1803ba7d0': 'lfsr24',         # 24-bit noise shift register (value in [-1, 1))
    'FUN_1803aa520': 'shl',            # x * 2^n, n clamped to +-32 (the DSP's shifter)
    'FUN_1807445f0': 'expf',
}


def ident(s):
    return re.sub(r'\W+', '_', s.strip()).strip('_')


def main():
    dll, src, dst = sys.argv[1:4]
    voice = int(sys.argv[4]) if len(sys.argv) > 4 else 0
    names = {}
    seen = {}
    for k, (off, n) in enumerate(vst_sim.params(Dll(dll))):
        base = ident(n)
        if k < 0x6e * 8:                   # per-voice block
            v, i = divmod(k, 0x6e)
            if v != voice:
                continue
            # ENV1 / ENV2 share names: tell them apart by index
            if 0x20 <= i <= 0x23: base = 'ENV1_' + base
            if 0x25 <= i <= 0x28: base = 'ENV2_' + base
            if i == 0x1f: base = 'ENV1_' + base
            if i == 0x24: base = 'ENV2_' + base
        c = seen.get(base, 0); seen[base] = c + 1
        names[off] = 'P_' + base + (f'_{c}' if c else '')
    text = open(src).read()

    def sub(m):
        off = int(m.group(2), 16)
        return names.get(off, f'st_{off:x}')
    text = re.sub(r'\*\((float|int|uint|undefined4|double|undefined8) \*\)\(param_1 \+ (0x[0-9a-f]+)\)', sub, text)
    for a, b in HELPERS.items():
        text = text.replace(a, b)
    open(dst, 'w').write(text)


if __name__ == '__main__':
    main()
