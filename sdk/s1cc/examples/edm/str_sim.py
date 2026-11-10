"""EDM_STR check without the device: build/s1mm1.s (master B, microcode/edm_str.s) on the C emulator, the bank-2
words through a model of the map table (a read entry at + d gives the word stored d + 1 samples earlier, measured
§3.3.4); 8 notes, one burst each; prints each string's pitch against the note. docs/vol6_reference_designs_applications/mutable_memory.md §3.3.5
    KERNEL=sdk/s1cc/build/edm_string sh examples/system_s1cc/build.sh && python3 sdk/s1cc/examples/edm/str_sim.py"""
import sys, struct, numpy as np
import os
R = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..'))
sys.path[:0]=[R+'/asm', R+'/examples/from_scratch/emu']
import asm, dspref
A=asm.Assembler()
prog=[]
for l in open(R+'/examples/system_s1cc/build/s1mm1.s'):
    l=l.strip()
    if not l or l.startswith(';'): continue
    r=asm.Assembler(A.table).assemble(l+'\n'); assert r.ok, (l, r.errors); prog.append(r.words)
m=dspref.Machine(context='master_dsp4_efdc')
NV=8; N=40000
notes=[48,55,60,64,67,72,79,84]
def per(n):
    p=max(96000/(440*2**((n-69)/12))-2,0); return int(p), p-int(p)
w=np.zeros((NV,N)); out=np.zeros(N)
a=0.1; fb=0.999
mem=m.mem
mem[0xC01D]=a; mem[0xC01E]=1-2*a; mem[0xC01C]=1.0
for v in range(NV):
    d,f=per(notes[v]); mem[0xC020+v]=f; mem[0xC028+v]=fb
n=[0]
rng=np.random.default_rng(2)
def acc(op,addr,val):
    if 0x4080<=addr<0x40C0 and op=='r':    # STR_TAP: tapreq results x0, x1 at d and at d + 2 (measured: as the table reads)
        v,k=divmod(addr-0x4080,8); j={0:0,1:1,3:2,4:3}[k]; d,_=per(notes[v]); i=n[0]-d-1-j
        return w[v,i] if i>=0 else 0.0
    if 0x4040<=addr<0x4080:
        v,k=divmod(addr-0x4040,8)
        if op=='w':
            assert k==0; w[v,n[0]]=val; return val
        j=(k-1)//2; d,_=per(notes[v]); i=n[0]-d-1-j
        return w[v,i] if i>=0 else 0.0
    if op=='r': return mem.get(addr) if addr not in X else X[addr]
    mem[addr]=val; return val
X={}
m.set_access(None)
pid=m.add(prog)
m.set_access(acc)
for s in range(N):
    n[0]=s
    for v in range(NV): X[0x6006+v]=rng.uniform(-0.5,0.5) if s<200 and v==0 or (v>0 and 2000*v<=s<2000*v+200) else 0.0
    m.run_program(pid)
    out[s]=mem.get(0x2000)
# per voice: compare stored w with the float model
for v in range(NV):
    d,f=per(notes[v]); seg=w[v,20000:]
    X_=np.abs(np.fft.rfft(seg*np.hanning(len(seg)),16*len(seg))); fr=np.fft.rfftfreq(16*len(seg),1/96000)
    f0=440*2**((notes[v]-69)/12); mk=(fr>f0*0.8)&(fr<f0*1.2); p=fr[np.argmax(X_*mk)]
    print(v, notes[v], round(f0,2), round(p,2), '%.2f cents'%(1200*np.log2(p/f0)), 'rms %.3f'%np.sqrt(np.mean(seg**2)))
print('send rms', np.sqrt(np.mean(out[20000:]**2)))
