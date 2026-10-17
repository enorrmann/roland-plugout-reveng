"""Record notes (USB audio), print f0 and harmonic magnitudes 1..8 relative to h1.

    python3 dspharm.py NAME [notes...]
"""
import sys,time; import os; sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
import numpy as np, dspaudio as da, dspown
name=sys.argv[1]; ns=[int(x) for x in sys.argv[2:]] or [48,60]
for n in ns:
    x=da.load(da.record(f'{name}_n{n}',n,0,2.0))[40000:150000,0]
    f=dspown.freq(x); w=np.hanning(len(x)); X=np.abs(np.fft.rfft((x-x.mean())*w)); fr=np.fft.rfftfreq(len(x),1/da.RATE)
    h=[X[max(0,int(round(k*f*len(x)/da.RATE))-3):int(round(k*f*len(x)/da.RATE))+4].max() for k in range(1,9)]
    print(f'{name} note {n}: f0 {f:8.3f} rms {x.std():.4f}  h/h1: '+' '.join(f'{v/h[0]:.3f}' for v in h),flush=True)
    time.sleep(0.4)
