"""constant probe: python3 probe.py name a b 'op words' ['post words']
r3 = x (noise); r0 = a; r1 = b (RAW hi16 immediates, dspnum: value = +-M/2^(32-4s));
then the op lines, then r0 = r0 * r3 (0902 0403), out r0: the measured gain is |r0| after the op."""
import struct, sys
def hi16(v):
    if v == 0: return 0
    s = 1 if v < 0 else 0; v = abs(v)
    for e in range(4):
        m = v * 2 ** (32 - 4 * e)
        if m < 2 ** 29:
            w = (s << 31) | (e << 29) | int(round(m))
            break
    else: raise SystemExit('out of range')
    if w & 0xFFFF: raise SystemExit(f'{v}: not exact in 16 bits ({w:08x})')
    return w >> 16
if __name__ == "__main__":
    name, a, b, *ops = sys.argv[1:]
    a, b = float(a), float(b)
    L = [f'# probe: r3 = x, r0 = {a}, r1 = {b}; ' + ' / '.join(ops) + '; r0 *= r3; out r0',
    '0c03 0fe0 2200', '0902 3d00', f'0903 0050 {hi16(a):04x}', f'0903 1050 {hi16(b):04x}'] + \
    [o for o in ops if o] + ['0902 0403', '7809 8fe0 8fe2 8fe4 8fe6 2221 221d 2229 2225']
    open(name + '.txt', 'w').write('\n'.join(L) + '\n')
