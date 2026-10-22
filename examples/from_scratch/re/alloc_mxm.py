"""Bus references of every main program of a Roland region 1 image: MXM (bank 4, 8nnn), IOMD (6nnn) and bank 5
(annn), read and written, from the disassembly (asm/disasm.py). Used for docs/VOICE_ALLOCATION.md.

    python3 alloc_mxm.py region1.bin BASE_HEX OFFSET:SIZE [OFFSET:SIZE ...]
    e.g. python3 alloc_mxm.py region1.bin 60A00000 015270:5066 016918:5018
The program words are read as dump_main.py does (uint32 little-endian, upper half first)."""
import os, re, struct, sys
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
import disasm  # noqa: E402
import isa  # noqa: E402

FIELD = re.compile(r'\[([0-9a-fA-F]{4})\]')


def words_of(b, off, nbytes):
    w = []
    for i in range(0, nbytes // 4 * 4, 4):
        v = struct.unpack_from('<I', b, off + i)[0]
        w += [v >> 16, v & 0xFFFF]
    return w


def main():
    path, base = sys.argv[1], int(sys.argv[2], 16)
    b = open(path, 'rb').read()
    table = isa.Table.load()
    for spec in sys.argv[3:]:
        off, size = spec.split(':')
        off, size = int(off, 16), int(size)
        words = words_of(b, off, size)
        text, _ = disasm.program(words[:size // 2], table, spec)
        mx = {'r': set(), 'w': set()}
        bank = {'r': set(), 'w': set()}
        io = {'r': set(), 'w': set()}
        for line in text.splitlines():
            for m in re.finditer(r'(ld|st)\s+r\d,\s*\[([0-9a-fA-F]{4})\]', line):
                op, a = m.group(1), int(m.group(2), 16)
                k = 'r' if op == 'ld' else 'w'
                if a >> 12 == 0x8:
                    mx[k].add(a)
                elif a >> 12 == 0xa:
                    bank[k].add(a)
                elif a >> 12 == 0x6:
                    io[k].add(a)
        f = lambda s: ' '.join('%04x' % x for x in sorted(s))
        print('== prog %06x size %d (0x%x)' % (off, size, size))
        print('   MXM read : %s' % f(mx['r']))
        print('   MXM write: %s' % f(mx['w']))
        print('   bank5 rd : %s' % f(bank['r']))
        print('   bank5 wr : %s' % f(bank['w']))
        print('   IOMD rd  : %s' % f(io['r']))
        print('   IOMD wr  : %s' % f(io['w']))


if __name__ == '__main__':
    main()
