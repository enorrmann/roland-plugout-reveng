"""MSVC x64 RTTI of a VST3 DLL: class name -> vtables (with their methods).

    python3 vst_rtti.py plugins/JX-3P/JX-3P.vst3 [name-filter]

TypeDescriptor ".?AV<name>@@" <- CompleteObjectLocator (signature 1, RVA of the
type descriptor at +12, RVA of itself at +20) <- the qword before each vtable.
Static analysis only (the DLL is never loaded or run).
"""
import struct, sys
import pefile


class Dll:
    def __init__(self, path):
        self.pe = pe = pefile.PE(path, fast_load=True)
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.img = pe.get_memory_mapped_image()
        self.secs = {s.Name.rstrip(b'\0').decode(): (s.VirtualAddress, s.Misc_VirtualSize) for s in pe.sections}

    def q(self, rva): return struct.unpack_from('<Q', self.img, rva)[0]
    def d(self, rva): return struct.unpack_from('<I', self.img, rva)[0]

    def in_text(self, va):
        a, n = self.secs['.text']
        return a <= va - self.base < a + n

    def classes(self):
        """{name: [vtable rva, ...]}"""
        img = self.img
        tds = {}
        i = img.find(b'.?AV')
        while i >= 0:
            j = img.find(b'\0', i)
            tds[i - 16] = img[i + 4:j - 2].decode('latin1')   # descriptor = vftable ptr, spare, name
            i = img.find(b'.?AV', j)
        cols = {}
        a, n = self.secs['.rdata']
        for off in range(a, a + n - 24, 4):
            if self.d(off) == 1 and self.d(off + 20) == off and self.d(off + 12) in tds:
                cols[off] = tds[self.d(off + 12)]
        out = {}
        for off in range(a, a + n - 8, 8):
            v = self.q(off) - self.base
            if v in cols:
                out.setdefault(cols[v], []).append(off + 8)
        return out

    def vtable(self, rva):
        m = []
        while self.in_text(self.q(rva)):
            m.append(self.q(rva)); rva += 8
        return m


def main():
    d = Dll(sys.argv[1])
    flt = sys.argv[2] if len(sys.argv) > 2 else ''
    for name, vts in sorted(d.classes().items()):
        if flt in name:
            for vt in vts:
                ms = d.vtable(vt)
                print(f'{name}  vtable {d.base + vt:#x}  {len(ms)} methods')
                print('   ', ' '.join(f'{x:#x}' for x in ms))


if __name__ == '__main__':
    main()
