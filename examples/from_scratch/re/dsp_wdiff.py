"""Align two DSP microcode word streams and classify the words that differ.

usage: python3 dsp_wdiff.py a.bin b.bin      two programs (e.g. DCO1 vs DCO2 copy)
       python3 dsp_wdiff.py a.bin --halves   a program made of two voice copies

Words that differ between copies of the same code are operands; the output
groups them by high byte and delta, e.g. "c0 +0x16" = parameter ids, voice
stride 0x16. Words that never change are operations.
"""
import sys, os, difflib, collections
sys.path.insert(0, os.path.dirname(__file__))
from dspmc import blob_words

def main():
    a = blob_words(sys.argv[1])
    if sys.argv[2] == '--halves':
        half = max(range(len(a) // 4, len(a) // 2 + 1),
                   key=lambda s: sum(a[i] == a[i + s] for i in range(len(a) - s)))
        print(f'halves split at word {half} (byte {2 * half:#x}, execution order)')
        b, a = a[half:], a[:half]
    else:
        b = blob_words(sys.argv[2])
    sm = difflib.SequenceMatcher(None, a, b, autojunk=False)
    cls, same = collections.Counter(), 0
    for op, i1, i2, j1, j2 in sm.get_opcodes():
        if op == 'equal':
            same += i2 - i1
        elif op == 'replace' and i2 - i1 == j2 - j1:
            for x, y in zip(a[i1:i2], b[j1:j2]):
                cls[(x >> 8, (y - x) & 0xffff)] += 1
    print(f'{same} of {len(a)} words identical')
    for (hb, d), n in cls.most_common(25):
        print(f'  {hb:02x}xx  +{d:#06x}  {n}')

if __name__ == '__main__':
    main()
