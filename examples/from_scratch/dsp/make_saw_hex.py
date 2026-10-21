"""Hex words of our DCO2 SAW for engine/programs.py (MC_TEST=dco2_saw=<file>): the slot program with its
start and end words, in execution order. Source: saw_dco2.s (asm/asm.py).

    python3 make_saw_hex.py   -> saw_dco2.hex
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
import asm  # noqa: E402

r = asm.Assembler().assemble(open(os.path.join(HERE, 'saw_dco2.s')).read())
assert r.ok, r.errors
w = [0x0A02, 0xFF70] + r.words + [0x0B02, 0x00F8, 0x0001]
open(os.path.join(HERE, 'saw_dco2.hex'), 'w').write(
    '# DCO2 SAW from saw_dco2.s (make_saw_hex.py)\n' + '\n'.join('%04x' % x for x in w) + '\n')
print('saw_dco2.hex: %d words' % len(w))
