"""The op groups that may share one DSP instruction on the SYSTEM-1 (asm/proven_forms.txt, rule R8 of asm/s1rules.py):
list them, and add the ones a TRY_FORMS build tested on the device.

    python3 tools/proven_forms.py list
    python3 tools/proven_forms.py promote examples/system_s1cc/build/try_forms.txt --by MP-CSAW-TRY
    python3 tools/proven_forms.py promote 'mad:reg + mul:imm; add:imm + mov:i32' --by MP-CSAW-TRY

Workflow (docs/examples_system_s1cc_README.md §4.1):
  1. KERNEL=dir python3 examples/system_s1cc/make_s1cc.py --forms
       the groups not on the list that would shorten the slave programs, and how many instructions each saves
  2. KERNEL=dir NAME=X-TRY TRY_FORMS='GROUP; GROUP' sh examples/system_s1cc/build.sh   (or TRY_FORMS=all)
       the PLUG-OUT with those groups; build/try_forms.txt lists the ones it used
  3. install it, select it, play (notes, chords, the knobs): no hang, the same sound as the proven build (a USB
     capture against make_s1cc.py --sim is the exact test)
  4. promote: the groups go into asm/proven_forms.txt with the date and the PLUG-OUT that ran them
Only promote after the device ran the build: a group the device does not execute as the emulator does can hang it.
"""
import datetime
import os
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
import dspsched  # noqa: E402

LIST = os.path.join(ROOT, 'asm', 'proven_forms.txt')


def read_forms(arg):
    """signatures from a try_forms.txt file (one group per line, '#' comments) or from 'G; G' text"""
    if os.path.exists(arg):
        text = '; '.join(line.split('#')[0].strip() for line in open(arg))
    else:
        text = arg
    return dspsched.parse_forms(text)


def main(argv):
    if len(argv) < 2 or argv[1] not in ('list', 'promote'):
        print(__doc__)
        return 2
    proven = dspsched.load_proven(LIST)
    if argv[1] == 'list':
        for sig in sorted(proven):
            print(dspsched.form_text(sig))
        print(f'{len(proven)} op groups ({os.path.relpath(LIST, ROOT)})')
        return 0
    args = argv[2:]
    if '--by' not in args or args.index('--by') + 1 >= len(args) or len(args) < 3:
        print('promote FILE|GROUPS --by PLUGOUT: the PLUG-OUT that ran them on the device is required')
        return 2
    by = args[args.index('--by') + 1]
    src = [a for k, a in enumerate(args) if a != '--by' and (k == 0 or args[k - 1] != '--by')]
    forms = set()
    for a in src:
        forms |= read_forms(a)
    new = sorted(forms - proven)
    if not new:
        print('nothing new: every group is already on the list')
        return 0
    with open(LIST, 'a') as f:
        f.write(f'# proven {datetime.date.today().isoformat()} by {by} (TRY_FORMS build run on the device)\n')
        for sig in new:
            f.write(dspsched.form_text(sig) + '\n')
    for sig in new:
        print('added:', dspsched.form_text(sig))
    print(f'{len(new)} op groups added to {os.path.relpath(LIST, ROOT)}')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
