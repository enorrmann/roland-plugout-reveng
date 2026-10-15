"""3-operand probe: python3 probe3.py name d x y 'op'  (op = one 0902 word, or full lines separated by ;)
r3 = x(noise); r0 = d (old destination), r1 = X, r2 = Y; op (dest r0, X field r1, Y r2: 0B42);
then r1 = 1.0, r0 += r1 (sign), r0 *= r3, out r0: gain - 1 = result."""
import sys
sys.argv = sys.argv  # noqa
from probe import hi16
name, d, x, y, op = sys.argv[1:]
d, x, y = float(d), float(x), float(y)
L = [f'# probe3: r0 = {d} (dest), r1 = {x} (X), r2 = {y} (Y); {op}; +1; *noise; out r0',
     '0c03 0fe0 2200', '0902 3d00', f'0903 0050 {hi16(d):04x}', f'0903 1050 {hi16(x):04x}',
     f'0903 2050 {hi16(y):04x}', *(op.split(';') if ' ' in op else [f'0902 {op}']), '0903 1050 3000', '0902 0c01', '0902 0403',
     '7809 8fe0 8fe2 8fe4 8fe6 2221 221d 2229 2225']
open(name + '.txt', 'w').write('\n'.join(L) + '\n')
