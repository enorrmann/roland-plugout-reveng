#!/bin/sh
# POLY-8: the stock C-V4 build of the from-scratch PLUG-OUT, renamed. Same code and the same microcode, and the
# firmware is C-V4 byte for byte -- only the PLUG-OUT name changes (the container header of Code8_<slot>.Dat and the
# SYNINFO the device reports for the slot). Built for slot 1, the slot the web installer targets.
#   examples/from_scratch/v4/build_poly8.sh [src-plugin-dir] [out-plugin-dir]
#   The C-V4 build is left in plugins/C-V4B before the rename (never removes an existing plugins/C-V4).
set -e
V4=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$V4/../../.." && pwd)
NAME=POLY-8
WORK=C-V4B
NAME=$WORK sh "$V4/build_v4.sh" "$@"
python3 - "$ROOT" "$WORK" "$NAME" <<'PY'
import os, subprocess, sys
sys.path.insert(0, os.path.join(sys.argv[1], 'tools'))
import plugout as P
import plugout_build as B

root, work, name = sys.argv[1], sys.argv[2], sys.argv[3]
src = os.path.join(root, 'plugins', work)
dst = os.path.join(root, 'plugins', name)
proj = os.path.join(root, 'examples', 'from_scratch', 'engine', 'build', 'projC')
tool = os.path.join(root, 'tools', 'plugout_build.py')
run = lambda *a: subprocess.run([sys.executable, tool] + list(a), check=True,
                                stdout=subprocess.DEVNULL)
run('set-id', proj, '--name', name, '--version', '0.01')
run('plugin', proj, '-o', dst)

# The firmware is C-V4 byte for byte; only the name in the container header changed.
a = P.load(os.path.join(src, 'Script', 'Code8_1.Dat')).plain
b = P.load(os.path.join(dst, 'Script', 'Code8_1.Dat')).plain
assert len(a) == len(b), (len(a), len(b))
diff = [i for i in range(len(a)) if a[i] != b[i]]
assert diff and diff[0] >= 0x14 and diff[-1] < 0x24, ['unexpected difference', hex(diff[0])]
c = P.load(os.path.join(dst, 'Script', 'Code8_1.Dat'))
print(f'{dst}: {c.model} id {c.model_id:#04x} version {c.version >> 8}.{c.version & 0xff:02x}, '
      f'{c.data_count} records; differs from {work} only in the name field '
      f'({len(diff)} bytes at {diff[0]:#x})')
PY
