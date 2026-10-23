"""Check the code examples of the technical documents: every ```dsp block must assemble.

    python3 tools/doccheck.py docs/WAVE_BUS.md docs/VOICE_ALLOCATION.md system-1/*.md
    python3 tools/doccheck.py --show docs/WAVE_BUS.md      # also print the words of each block

A ```dsp block is DSP assembly in the syntax of asm/ (docs/DSP_ISA.md): one instruction per line, ';' comments.
Blocks are assembled one by one with asm.Assembler; a block that fails is reported with its file and line and the
exit code is 1. ```c blocks are not compiled (they call OS services); keep them to code that exists in the repo.
"""
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
import asm  # noqa: E402

FENCE = re.compile(r'^```dsp\s*$')


def blocks(path):
    lines = open(path, encoding='utf-8').read().splitlines()
    i = 0
    while i < len(lines):
        if FENCE.match(lines[i]):
            start = i + 1
            j = start
            while j < len(lines) and not lines[j].startswith('```'):
                j += 1
            yield start + 1, lines[start:j]
            i = j
        i += 1


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    show = '--show' in sys.argv
    bad = n = 0
    for path in args:
        for line_no, body in blocks(path):
            n += 1
            src = '\n'.join(l.split(';')[0].rstrip() for l in body if l.split(';')[0].strip()) + '\n'
            r = asm.Assembler().assemble(src)
            if not r.ok:
                bad += 1
                print(f'{path}:{line_no}: does not assemble: {r.errors[:2]}')
            elif show:
                print(f'{path}:{line_no}: {len(r.words)} words: ' + ' '.join(f'{w:04x}' for w in r.words))
    print(f'{n} dsp blocks, {bad} failed')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
