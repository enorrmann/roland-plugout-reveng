import sys, os, re, struct, capstone as C
d=open(sys.argv[1],'rb').read(); base=int(os.environ.get("BASE","0x60a00000"),16)
md=C.Cs(C.CS_ARCH_ARM, C.CS_MODE_THUMB); md.skipdata=True
def u32(a):
    o=a-base
    return struct.unpack_from('<I',d,o)[0] if 0<=o<=len(d)-4 else None
for a in sys.argv[2:]:
    s,e=[int(x,16) for x in a.split(':')]
    for i in md.disasm(d[s-base:e-base], s):
        x=''
        m=re.match(r'.*\[pc, #(0x[0-9a-f]+|\d+)\]', i.op_str)
        if m and i.mnemonic.startswith('ldr') and not i.mnemonic.startswith('ldrd'):
            la=((i.address+4)&~3)+int(m.group(1),0); v=u32(la)
            x=f'   ; [{la:08x}]={v:#010x}' if v is not None else ''
        elif i.mnemonic.startswith('adr') or (i.mnemonic in('add','addw','subw') and 'pc' in i.op_str):
            pass
        print(f"{i.address:08x}: {i.bytes.hex():10s} {i.mnemonic} {i.op_str}{x}")
    print('--')
