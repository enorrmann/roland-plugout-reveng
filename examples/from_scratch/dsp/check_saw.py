"""Check saw_dco2.s: assemble it with asm/asm.py and compare its simulated output with re/dspasm.py's SAW.

    python3 examples/from_scratch/dsp/check_saw.py      (from the repository root)
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
import asm    # noqa: E402
import dspasm  # noqa: E402

r = asm.Assembler().assemble(open(os.path.join(HERE, 'saw_dco2.s')).read())
print(r.summary())
if not r.ok:
    sys.exit(r.errors)
# calibrated: the source uses K / 1.032045, i.e. dspasm's SAW at fs = 96000 * 1.032045
ref = [w for p in dspasm.saw('dco2', fs=96000.0 * 1.032045) for w in p]
worst = 0.0
for pitch in (4.75, 6.0, 8.0):
    a = dspasm.simulate(ref, pitch, 300, slot='dco2')
    b = dspasm.simulate(r.words, pitch, 300, slot='dco2')
    worst = max(worst, max(abs(x - y) for x, y in zip(a, b)))
print('max difference against dspasm SAW: %.3g' % worst)
sys.exit(0 if worst < 1e-6 else 1)
