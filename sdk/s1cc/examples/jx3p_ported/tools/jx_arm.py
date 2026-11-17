"""jx_arm.py: the ARM side of the user's SYSTEM-8 JX-3P PLUG-OUT (plugins/JX-3P 1.01) as data. README.md §3.

    python3 sdk/s1cc/examples/jx3p_ported/tools/jx_arm.py

tools/plugout_dsp_tables.py on plugins/JX-3P, DSPs 0, 2, 4 (one slave's voice pair A / B: MAIN0, MAIN2 + the DCO slots,
MAIN4) -> work/jx3p/jx_arm.json (format: that tool's --help).
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..', '..', '..'))

if __name__ == '__main__':
    subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'plugout_dsp_tables.py'), os.path.join(ROOT, 'plugins', 'JX-3P'),
                    '-o', os.path.join(ROOT, 'work', 'jx3p', 'jx_arm.json'), '--dsps', '0,2,4', '--id-base', '0x6CD'],
                   check=True)
