#!/bin/sh
# Build plugins/JX-3P-INIT-PROBEQ: stock JX-3P whose region 0 init patches the producer wrapper of the master MIDI
# input queue to count the wraps (SysEx = type 3, USB-MIDI packets = type 4) in its own PLUG-OUT name.
#
#   [MASK=0x1A20] examples/midi_send/make_init_probe_q.sh [OUT_DIR]
# Capture all ports (run_probe.py --no-install --window ..) DURING the install.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
OUT=${1:-$ROOT/plugins/JX-3P-INIT-PROBEQ}
BASE=0x61800000; CAVE_OFF=0x100; INIT_OFF=0x11823c
B="python3 $ROOT/tools/plugout_build.py"
WORK=$(mktemp -d); trap 'rm -rf "$WORK"' EXIT
cd "$WORK"
arm-none-eabi-as -mcpu=cortex-a9 -mthumb --defsym CAVE_OFF=$CAVE_OFF --defsym INIT_OFF=$INIT_OFF \
    --defsym WRAPP=0x600753ea --defsym ROUTER=0x6000345c --defsym DTORW=0x600753f4 \
    --defsym MASK=${MASK:-0x1A20} --defsym WRAPPER=0x601F4D66 --defsym PRODUCER=0x601F4A6C --defsym TAIL=0x601F24D8 --defsym DEST=0x60445957 "$HERE/init_probe_q.s" -o h.o
arm-none-eabi-ld -Ttext=$(printf 0x%x $((BASE + CAVE_OFF))) --entry=hook h.o -o h.elf
for bad in .rodata .data .bss; do
    if arm-none-eabi-objdump -h h.elf | grep -q "$bad"; then
        echo "ERROR: hook has a $bad section" >&2; exit 1
    fi
done
arm-none-eabi-objcopy -O binary -j .text h.elf "$HERE/init_probe_q.bin"
arm-none-eabi-objdump -d h.elf | tail -n +7
echo "hook: $(wc -c < "$HERE/init_probe_q.bin") bytes"
$B unpack "$ROOT/plugins/JX-3P/Script/Code8_1.Dat" p \
    --relocs-from "$ROOT/plugins/JX-3P/Script/Code8_2.Dat" >/dev/null
$B set-id p --name "PROBE Q" --version 1.31
$B patch p --region 0 --at 0x61800100 --file "$HERE/init_probe_q.bin"
$B patch p --region 0 --at 0x61800024 --hex 01018061 --reloc 0x61800024
$B plugin p --template "$ROOT/plugins/JX-3P-MEM" -o "$OUT" >/dev/null
echo "plugin: $OUT  MASK=${MASK:-0x1A20}"
