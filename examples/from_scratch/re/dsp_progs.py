"""List / extract the DSP microcode that the JX-3P engine (region 1, slot 1) hands to the slave.

usage: python3 dsp_progs.py region1.bin [outdir]

Runs the main object's vt+8 (DSP load) in emu1 for every DCO1/DCO2 waveform and
effect type, logs the OS table calls +0x28..+0x6C and prints the descriptors
{DSP address, bytes, bytes, src}. With outdir, writes each program blob there.
"""
import sys, os, struct, hashlib
sys.path.insert(0, os.path.dirname(__file__))
import run_jx
KIND = ('slot', 'prog', 'data')                 # +0x28 / +0x40 / +0x58, + 4*dsp
BASE = 0x60a00000
CASES = [({}, 'default')]
CASES += [({0x6d4: v}, f'DCO1={v}') for v in range(3)]   # 0 SAW, 1 SQR, 2 PULSE
CASES += [({0x6d9: v}, f'DCO2={v}') for v in range(4)]   # 0 SAW, 1 SQR, 2 PULSE, 3 NOISE
CASES += [({0x748: v}, f'FX={v}') for v in range(6)]

def load(vals):
    e = run_jx.boot(); e.vals.update(vals); e.log.clear()
    e.call(e.u32(e.vt + 8), e.o)
    for off, r0, *_ in e.log:
        if 0x28 <= off <= 0x6c:
            kind, dsp = divmod((off - 0x28) // 4, 6)
            yield dsp, KIND[kind], (struct.unpack('<4I', e.uc.mem_read(r0, 16)) if kind else (r0, 0, 0, 0))

def main():
    run_jx.IMG = sys.argv[1]
    outdir = sys.argv[2] if len(sys.argv) > 2 else None
    img = open(sys.argv[1], 'rb').read()
    seen = {}
    for vals, label in CASES:
        for dsp, kind, (addr, n, _, src) in load(vals):
            if kind == 'slot':
                if label == 'default': print(f'DSP{dsp} slot {addr:#06x}')
                continue
            seen.setdefault((dsp, kind, addr, src, n), []).append(label)
    print()
    for (dsp, kind, addr, src, n), labels in sorted(seen.items()):
        blob = img[src - BASE: src - BASE + n]
        print(f'DSP{dsp} {kind} @{addr:#06x} {n:#6x} B src {src:#x} '
              f'md5 {hashlib.md5(blob).hexdigest()[:8]}  [{", ".join(labels)}]')
        if outdir and kind == 'prog':
            os.makedirs(outdir, exist_ok=True)
            open(os.path.join(outdir, f'dsp{dsp}_{addr:04x}_{src:08x}.bin'), 'wb').write(blob)

if __name__ == '__main__':
    main()
