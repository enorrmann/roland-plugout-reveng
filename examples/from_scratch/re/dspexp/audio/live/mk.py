"""write a single-path test file: python3 mk.py name 'comment' outreg 'line' 'line' ...
r0 = x ([2200] noise) is always the first line; output = register outreg to the four outputs."""
import sys
name, comment, reg, *body = sys.argv[1:]
L = [f'# {comment}', '0c03 0fe0 2200'] + body + [f'7809 {reg}fe0 {reg}fe2 {reg}fe4 {reg}fe6 2221 221d 2229 2225']
open(name + '.txt', 'w').write('\n'.join(L) + '\n')
