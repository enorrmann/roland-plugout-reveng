"""Count ARM VFP (floating-point unit) instructions in binary images: does the code use an FPU?

    python3 system-1/vfp_count.py IMAGE...
    python3 system-1/vfp_count.py            # the default set: SYSTEM-8 OS and engines vs SYSTEM-1 OS and PLUG-OUTs

Scans every halfword pair as Thumb-2: coprocessor instructions (first halfword ECxx/EDxx/EExx, the forms a
compiler emits; FFxx/EFxx match mostly data) whose coprocessor
field is 10 or 11 (VFP single / double). Data words also match by chance, so a count of about 1 per 10 KiB is the
noise floor; real VFP code gives tens per 10 KiB. Result and interpretation: system-1/ARCHITECTURE.md §1.1.
"""
import os
import struct
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
DEFAULT = [
    ('SYSTEM-8 OS (master + slaves)', 'dumps/os_full.bin'),
    ('SYSTEM-8 JX-3P region 0 (master)', 'examples/from_scratch/engine/build/jx/region0.bin'),
    ('SYSTEM-8 JX-3P region 1 (slaves)', 'examples/from_scratch/engine/build/jx/region1.bin'),
    ('SYSTEM-8 SH-2 region 1', 'work/unpack/SH-2/region1.bin'),
    ('SYSTEM-1 OS 1.30 master', 'work/sys1_v130/app_master.bin'),
    ('SYSTEM-1 OS 1.30 slave', 'work/sys1_v130/app_slave.bin'),
    ('SYSTEM-1 SH-2 region 0', 'work/unpack/SH-2_S1_Code1/region0.bin'),
    ('SYSTEM-1 SH-2 region 1', 'work/unpack/SH-2_S1_Code1/region1.bin'),
    ('SYSTEM-1 PROMARS region 0', 'work/unpack/PROMARS_S1_Code1/region0.bin'),
    ('SYSTEM-1 PROMARS region 1', 'work/unpack/PROMARS_S1_Code1/region1.bin'),
]


def vfp(data):
    n = 0
    for o in range(0, len(data) - 3, 2):
        h1, h2 = struct.unpack_from('<HH', data, o)
        if h1 >> 8 in (0xEC, 0xED, 0xEE) and (h2 >> 9) & 7 == 5:   # coprocessor instruction, cp10 / cp11
            n += 1
    return n


def main():
    items = [(p, p) for p in sys.argv[1:]] or DEFAULT
    print(f'{"image":<36} {"bytes":>9} {"VFP":>6} {"per 10 KiB":>10}')
    for label, path in items:
        full = path if os.path.isabs(path) else os.path.join(ROOT, path)
        if not os.path.exists(full):
            print(f'{label:<36} missing ({path})')
            continue
        d = open(full, 'rb').read()
        n = vfp(d)
        print(f'{label:<36} {len(d):9d} {n:6d} {n / len(d) * 10240:10.1f}')


if __name__ == '__main__':
    main()
