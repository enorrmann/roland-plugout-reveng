"""ARCHIVED 2026-10-29 (user decision): the Python DSP reference is no longer used. The reference is the C core
(examples/from_scratch/emu/dspcore.h + dspcomp; docs/EMULATOR.md). The old code is in archive/dspisa.py, kept only
to validate the C port against it; do not import it from tools."""
raise ImportError('dspisa.py is ARCHIVED: do not use it, use the C core (emu/dspcore). Old code: archive/dspisa.py')
