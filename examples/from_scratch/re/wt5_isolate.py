"""Isolate the two halves of `wt5.py send` on the hardware: program writes (DATA, cmd 1) vs bank-5 writes
(DATA with ext bit 2). Only ADDR (cmd 0) carries the bank-5 bit, so the interesting difference between a program
write that works (dspown) and a bank-5 write that may not is the ADDR that preceded it — or the DPC access itself.

Prerequisite: lab in slot 3 (DSPLIVE with bank-5 commands), PLG3 on the panel, no other MIDI tool running.
A silent result means DSP2/DSP3 are not producing output any more (the note still sounds if the rest of the
engine is alive).

  T1  bank-5 write of a few words, then a program that returns a constant (no bank-5 reads)
  T2  bank-5 write of the whole region 0x048..0x1FD (438 words, as wt5.py sends), then the same program
  T3  no bank-5 access from the ARM at all: `wt5.py send WAV --code` (table written by the DSP microcode)

usage: python3 re/wt5_isolate.py [--only T1,T2,T3] [--note 60]
"""
import argparse, os, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dspown, dsplive, dspaudio as da            # noqa: E402

OUT = os.path.join(da.ROOT, 'work', 'wt5_isolate')
WAV = '/home/emilio/PRODUCCION/samples/THE_REST/wavetables/Echo Sound Works Core Tables/Basics/PWM Saw 01.wav'
CONST = '0a02 ff70\n0904 0070 0000 0000\n0c03 8fe0 a048\n0a02 ff70\n0b02 00f8\n0001\n'   # r0 = 1/8 -> A048, return


def rms(tag, note=60):
    path = da.record(tag, note, 0, 2.0)
    x = da.load(path)[40000:150000, 0]
    print(f'  {tag}: rms {x.std():.5f} ' + ('SILENT (DSP dead?)' if x.std() < 1e-4 else 'sounding'), flush=True)
    return x.std()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--only', default='T1,T2,T3')
    ap.add_argument('--note', type=int, default=60)
    a = ap.parse_args()
    want = [t.strip() for t in a.only.split(',')]
    os.makedirs(OUT, exist_ok=True)
    prog = os.path.join(OUT, 'const.txt')
    open(prog, 'w').write(CONST)
    print('baseline (lab as installed):', flush=True)
    rms('base', a.note)
    if 'T1' in want:
        print('T1: bank-5 write of 8 words, then a constant program', flush=True)
        dsplive.Link().write_b5(0x48, [0.0] * 8)
        dspown.load(prog)
        rms('T1', a.note)
    if 'T2' in want:
        print('T2: bank-5 write of 0x048..0x1FD (438 words), then the same program', flush=True)
        dsplive.Link().write_b5(0x48, [0.05] * 438)
        dspown.load(prog)
        rms('T2', a.note)
    if 'T3' in want:
        print('T3: wt5.py send --code (no bank-5 access from the ARM)', flush=True)
        subprocess.run([sys.executable, os.path.join(HERE, 'wt5.py'), 'send', WAV, '--code'], cwd=HERE)
        rms('T3', a.note)
    print('back to the lab program: python3 re/dspown.py restore', flush=True)


if __name__ == '__main__':
    main()
