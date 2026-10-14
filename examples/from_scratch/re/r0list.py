"""Dump the region 0 record list (SYNINFO +0x28) of a stock PLUG-OUT.
   python3 re/r0list.py region0.bin [base]"""
import struct, sys
d = open(sys.argv[1], 'rb').read()
base = int(sys.argv[2], 16) if len(sys.argv) > 2 else 0x61800000
p = struct.unpack_from('<I', d, 0x28)[0] - base
def cstr(b): return b.split(b'\0')[0].decode('latin1')
while True:
    t, n = struct.unpack_from('<iI', d, p)
    if t == -1:
        print(f'{p+base:08x} END'); break
    body = d[p+8:p+n]
    w = struct.unpack_from('<%di' % ((n-8)//4), body) if n > 8 else ()
    if t == 0:
        print(f'{p+base:08x} GROUP size={n} id={w[0]} {cstr(body[4:])!r}')
    elif t == 1:
        print(f'{p+base:08x}   PARAM size={n:#x} ' + ' '.join(f'{x:#x}' for x in w[:13]) + f' {cstr(body[0x34:])!r}')
    elif t == 3:
        print(f'{p+base:08x}     LABEL {cstr(body)!r}')
    else:
        print(f'{p+base:08x} T{t} size={n:#x} ' + ' '.join(f'{x:#x}' for x in w))
    p += n
