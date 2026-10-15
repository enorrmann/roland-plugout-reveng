"""The native DSP model (CJx3pSim) of the JX-3P VST3: named parameters and the
functions that use them. Static analysis only (the DLL is never loaded or run).

    python3 vst_sim.py params plugins/JX-3P/JX-3P.vst3       # index, offset, name
    python3 vst_sim.py fields plugins/JX-3P/JX-3P.vst3 0x1803b2530:0x829c ...

CJx3pSim registers its parameters in one function (0x180382bf0 in 1.0.7): for
each, a 40-byte entry {name, 0, 0, 0, 1, &field} is appended to the table at
sim+0x38; the control classes (CDSPJx3p*, the same as on the PLUG-OUT) write
entry i via 0x18040ba10 (table[i].field = value). Voice v uses entries
0x6e*v + (0..0x6d), fields at +0x3f80*v.
"""
import collections, sys
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
from capstone.x86 import X86_OP_MEM, X86_REG_RIP, X86_REG_RBP, X86_REG_RCX, X86_REG_RDI, X86_REG_RSI, X86_REG_RBX
from vst_rtti import Dll

REGISTER = (0x180382bf0, 0x180382cb2 + 58468)    # CJx3pSim parameter registration (1.0.7)


def params(d):
    md = Cs(CS_ARCH_X86, CS_MODE_64); md.detail = True
    va, end = REGISTER
    name, out = None, []
    for i in md.disasm(d.img[va - d.base:end - d.base], va):
        if i.mnemonic != 'lea':
            continue
        m = i.operands[1].mem
        if m.base == X86_REG_RIP:
            t = i.address + i.size + m.disp
            name = d.img[t - d.base:t - d.base + 80].split(b'\0')[0].decode('latin1')
        elif name is not None and m.base != X86_REG_RBP:
            out.append((m.disp, name)); name = None
    return out


def fields(d, names, va, size):
    md = Cs(CS_ARCH_X86, CS_MODE_64); md.detail = True
    used = collections.Counter()
    for i in md.disasm(d.img[va - d.base:va - d.base + size], va):
        for op in i.operands:
            if op.type == X86_OP_MEM and op.mem.disp in names and \
                    op.mem.base in (X86_REG_RCX, X86_REG_RDI, X86_REG_RSI, X86_REG_RBX):
                used[op.mem.disp] += 1
    return used


def main():
    cmd, dll = sys.argv[1], Dll(sys.argv[2])
    p = params(dll)
    if cmd == 'params':
        for k, (off, n) in enumerate(p):
            print(f'{k:#05x} {off:#08x} {n}')
    else:
        names = dict(p)
        for a in sys.argv[3:]:
            va, size = (int(x, 16) for x in a.split(':'))
            u = fields(dll, names, va, size)
            print(f'{va:#x}: {len(u)} named fields')
            print('   ' + ', '.join(f'{names[k].strip()}@{k:#x}' for k in sorted(u)))


if __name__ == '__main__':
    main()
