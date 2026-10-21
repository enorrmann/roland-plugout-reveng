"""Listing of a Roland main program (engine/build/programs.h, from the JX-3P) with its bus references:
IOMD (bank 3, 6nnn), MXM (bank 4, 8nnn) and bank 5 (Annn). Used for docs/WAVE_BUS.md.

    python3 dump_main.py MAIN5 > main5.txt      (needs engine/build, i.e. engine/build.sh run once)
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))   # the asm/ modules import each other by name
import disasm  # noqa: E402
import isa  # noqa: E402

name = sys.argv[1] if len(sys.argv) > 1 else 'MAIN5'
src = open(os.path.join(HERE, '..', 'engine', 'build', 'programs.h')).read()
m = re.search(r'mc_%s\[(\d+)\].*?=\s*\{(.*?)\};' % name, src, re.S)
words = []
for t in re.findall(r'0x([0-9a-f]+)', m.group(2)):
    v = int(t, 16)
    words += [v >> 16, v & 0xFFFF]
n = int(re.search(r'MC_%s_BYTES (0x[0-9a-f]+)' % name, src).group(1), 16) // 2
text, _ = disasm.program(words[:n], isa.Table.load(), name)
print(text)
