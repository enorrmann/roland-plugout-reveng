#!/usr/bin/env python3
"""dsplift_names.py: step 1 before `dsplift.py --kernel`: writes a whole names file for a program (dsplift.md §6.4).

    dsplift_names.py OUT.txt [the same arguments as dsplift.py --kernel, --names SEEDS.txt ...]

OUT.txt = the seed names files (--names, includes expanded, in order) and then a `cell X NAME` line for every bank-0
cell they leave unnamed, named by dsplift_c.auto_names from the program (the stage the value flows into, the
parameters it reads, its form). The lifter then reads OUT.txt like any names file (`dsplift.py --kernel --names
OUT.txt`) and does no naming of its own: OUT.txt can be read, and corrected by hand, between the two steps. Running
this tool again overwrites OUT.txt: a correction that should stay goes into a seed file.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)


def expand(path, seen=None):
    """a names file's lines with its includes inlined"""
    seen = seen or set()
    path = os.path.abspath(path)
    if path in seen:
        raise SystemExit('%s: included twice' % path)
    seen.add(path)
    out = ['# ---- from %s' % os.path.relpath(path)]
    for ln in open(path):
        ln = ln.rstrip('\n')
        w = ln.split('#', 1)[0].split()
        if w and w[0] == 'include':
            out += expand(os.path.join(os.path.dirname(path), w[1]), seen)
        else:
            out.append(ln)
    return out


def write(out, seeds, made):
    """made: [(program name, dsplift_c.Names after auto_names)]"""
    lines = ['# %s: names for %s, written by sdk/s1cc/tools/dsplift_names.py (step 1; dsplift.py --kernel --names %s is'
             % (os.path.basename(out), ', '.join(n for n, _ in made), os.path.basename(out)),
             '# step 2). The seed files below, then the cells named automatically. Running dsplift_names.py again',
             '# overwrites this file: corrections that should stay go into the seed files.', '']
    for p in seeds:
        lines += expand(p) + ['']
    for name, nm in made:
        auto = nm.dump()
        lines.append('# ---- %s: %d cells named by dsplift_c.auto_names (GROUP_ROLE, dsplift.md §6.4)' % (name, len(auto)))
        lines += auto + ['']
    open(out, 'w').write('\n'.join(lines))
    print('%s: %s' % (out, ', '.join('%s %d auto cells' % (n, len(nm.auto)) for n, nm in made)))


if __name__ == '__main__':
    import dsplift
    if '--kernel' not in sys.argv:
        sys.argv.insert(2, '--kernel')
    dsplift.main(names_mode=True)
