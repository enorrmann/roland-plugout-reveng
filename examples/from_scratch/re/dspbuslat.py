"""Inter-DSP transfers on the master, measured (2026-10-17): DSP3 (main program) keeps a sample counter (+2^-16 per
sample, bank 5) and publishes it on MXM (Z 85F0) and IOMD (6030); DSP4 (slot 0xF7DC, the global delay replaced for
a moment) copies MXM O 83F0 -> IOMD 6032 and IOMD 6030 -> IOMD 6033; DSP3 reads 6032/6033 at the start and at the
end of its program and stores counter - received (= round-trip latency in samples).

    python3 dspbuslat.py [--dsp4-first]
"""
import argparse, os, sys, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspmaster, dspasm as A

NOP = [1]
INC = 2.0 ** -16
B3, B4, IOMD = 0x41122000, 0x41126000, 0x4113E000


def dsp3_prog():
    P = [[0x0A02, 0xFF70],
         A.ld(5, 0x6032), A.ld(6, 0x6033),                    # received at the start of the program
         A.ld(0, 0xA3F0), NOP, NOP, A.a('add', 0, 0, imm=INC), A.st(0, 0xA3F0),   # counter
         A.st(0, 0x85F0), A.st(0, 0x6030)]                    # publish on MXM Z and IOMD
    P += [NOP] * 20
    P += [A.ld(1, 0x6032), A.ld(2, 0x6033), NOP, NOP,         # received at the end
          A.a('sub', 3, 5, 0), A.st(3, 0xA3F1), A.a('sub', 3, 6, 0), A.st(3, 0xA3F2),
          A.a('sub', 3, 1, 0), A.st(3, 0xA3F3), A.a('sub', 3, 2, 0), A.st(3, 0xA3F4),
          A.st(5, 0xA3F5), A.st(1, 0xA3F6),
          [0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return [w for i in P for w in i]


def dsp4_prog():
    P = [[0x0A02, 0xFF70], A.ld(1, 0x83F0), A.ld(2, 0x6030), NOP, NOP,
         A.st(1, 0x6032), A.st(2, 0x6033), A.st(1, 0xA3C0), A.st(2, 0xA3C1),
         [0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return [w for i in P for w in i]


def main():
    ap = argparse.ArgumentParser()
    ap.parse_args()
    pk = Peek()
    dspmaster.CONTEXT = 'dsp3'
    with dspmaster.Session(pk, size=128) as s3:
        s3.load(dsp3_prog())
        dspmaster.CONTEXT = 'efdc'
        with dspmaster.Session(pk, 4, 0xF7DC, size=200) as s4:
            s4.load(dsp4_prog())
            time.sleep(0.5)
            for _ in range(4):
                d3 = [raw_value(x) for x in pk.read(B3 + 4 * 0x3F0, 7)]
                d4 = [raw_value(x) for x in pk.read(B4 + 4 * 0x3C0, 2)]
                io = [raw_value(x) for x in pk.read(IOMD + 4 * 0x30, 4)]
                L = [v / INC for v in d3[1:5]]
                print(f'counter {d3[0]:.4f} | round trip in samples (counter - received): '
                      f'MXM->IOMD read@start {L[0]:.2f}, IOMD->IOMD @start {L[1]:.2f}, MXM->IOMD @end {L[2]:.2f}, '
                      f'IOMD->IOMD @end {L[3]:.2f} | DSP4 got MXM {d4[0]:.4f} IOMD {d4[1]:.4f} | IOMD 30..33 '
                      + ' '.join(f'{v:.4f}' for v in io), flush=True)
                time.sleep(0.3)
    dspmaster.CONTEXT = 'dsp3'


if __name__ == '__main__':
    main()
