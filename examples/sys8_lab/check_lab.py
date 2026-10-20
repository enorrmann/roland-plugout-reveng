"""Preflight for the S8LAB tests: PLG1 must hold S8LAB 0.01, and the patch 0 OSC1 leaf must be readable.

Prints the slot list, then the OSC1 values of patch 0 (WAVE, COLOR, MOD, RANGE, CROSS MOD; no writes).

    python3 check_lab.py
"""
import subprocess, sys, os
import color_test as C
import patches as PT

TOOLS = os.path.join(C.ROOT, 'tools', 'plugout_tool.py')


def main():
    info = subprocess.run([sys.executable, TOOLS, 'info'], capture_output=True, text=True)
    print(info.stdout.rstrip())
    if 'PLUG-OUT slot 1: S8LAB id 0x0020 version 0.01' not in info.stdout:
        sys.exit('PLG1 does not hold S8LAB 0.01: run examples/sys8_lab/build_s8lab.sh install')
    leaf = next(x for x in C.LAYOUT.leaves['patch'] if x.path == 'OSC1')
    addr = PT.patch_address(C.SLOT, C.PATCH, 0) + leaf.offset
    data = C.read_bytes(addr, leaf.size)
    if not data:
        sys.exit('no answer to the OSC1 read')
    pairs = [(data[i] << 4) | data[i + 1] for i in range(0, len(data) - 1, 2)]
    print('patch 0, OSC1 leaf (nibble pairs, WAVE COLOR MOD RANGE CROSS):', pairs)
    print('OK: S8LAB in PLG1 and the patch reads back')


if __name__ == '__main__':
    main()
