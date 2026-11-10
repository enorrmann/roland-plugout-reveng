# SYSTEM-1 — scripts

Study scripts for the SYSTEM-1 (the device, not a SYSTEM-8 PLUG-OUT). The documentation of the SYSTEM-1 is in the
volumes of [`docs/`](../docs/index.md); the Spanish documents of this folder are obsolete
([docs/OBSOLETE.md](../docs/OBSOLETE.md)) and kept only as raw data.

## Where the SYSTEM-1 is documented

| Chapter | What it covers |
|---|---|
| [Vol. I ch. 1](../docs/vol1_system_architecture/soc_overview.md) §6 | hardware, capacity of the slave DSPs, floating point (no FPU), what code runs where, flows, DSP windows, tools |
| [Vol. I ch. 3](../docs/vol1_system_architecture/bus_interconnect.md) Part B | communication between the four DSPs, audio path, header/closing rule, panel effects (delay + reverb) in PLUG-OUT mode |
| [Vol. I ch. 6](../docs/vol1_system_architecture/s1_arm_clock.md) | 1 ms tick on the slave ARM and writing DSP parameters from it |
| [Vol. II ch. 6](../docs/vol2_dsp_core_isa/memory_banks_dma.md) §7 | data banks of the DSPs, ranges used by the OS and the official PLUG-OUTs, rules, delay-line measurement |
| [Vol. III ch. 2](../docs/vol3_voice_allocation_scheduling/system1_voice_allocation.md) | voice allocation: ids `0x110+v`/`0x120+v`, stock split per DSP, sections, PLUG-OUTs, C-S1V4/V8 |
| [Vol. III ch. 3](../docs/vol3_voice_allocation_scheduling/slot_program_model.md) | program memory (main, sections, closing) of both devices |
| [Vol. III ch. 8](../docs/vol3_voice_allocation_scheduling/s1_optimizations.md) | optimization catalogue of SYSTEM-S1CC |
| [Vol. III ch. 9](../docs/vol3_voice_allocation_scheduling/system1_polyphony_studies.md) | six voices for the stock synth, a polyphonic SH-101, SYSTEM-8 microcode on the SYSTEM-1 (`dsp_fit.py`, `s1_progs.py`) |
| [Vol. IV ch. 6](../docs/vol4_firmware_container_protocol/base_firmware.md) §9 | firmware images (updater 1.30 not encrypted, plug-in `Code1.Dat` 1.20), extraction, known addresses, MIDI send |
| [Vol. V ch. 6](../docs/vol5_toolchain_compiler_sdk/audio_capture.md) | audio capture and sending notes |
| [Vol. VI ch. 2](../docs/vol6_reference_designs_applications/c_s1v8_system1_reference.md) | C-S1V4 / C-S1V8 |

## Scripts

From the repository root. They need `firmware/sys1_sys_v130.zip` and the plug-in in `plugins/SYSTEM-1/`.

| Script | What it does |
|---|---|
| `python3 system-1/extract_fw.py` | unpacks the 1.30 and 1.20 images into `work/sys1_v130/` and `work/sys1_v120/` |
| `python3 system-1/voice_sets.py [IMAGE] [--entries]` | lists the DSP program sets of the slave and their cost in instructions with 4 and with 6 voices |
| `python3 system-1/fnmatch.py A B 0xADDR:N …` | finds in image B the function of image A (ignores PC-relative offsets) |
| `python3 system-1/dsp_fit.py [--voices 2] [--dco1/--dco2/--fx …]` | measures in the emulator the DSP time and the links of the JX-3P and searches a placement on the SYSTEM-1 ([Vol. III ch. 9](../docs/vol3_voice_allocation_scheduling/system1_polyphony_studies.md) §3); needs `examples/from_scratch/engine/build/programs.h` |
| `python3 system-1/s1_progs.py [DIR…] [--listing]` | DSP programs of a SYSTEM-1 PLUG-OUT (SH-101, SH-2, PROMARS): where they go, executed instructions, IOMD ([Vol. III ch. 9](../docs/vol3_voice_allocation_scheduling/system1_polyphony_studies.md) §3.4) |
| `python3 system-1/vfp_count.py [IMAGE…]` | counts floating-point (VFP) instructions in ARM images ([Vol. I ch. 1](../docs/vol1_system_architecture/soc_overview.md) §6.3) |
| `python3 system-1/asgn_sites.py [IMAGE] [START END]` | lists the instructions of the master's assigner with the "4 voices" constants ([Vol. III ch. 9](../docs/vol3_voice_allocation_scheduling/system1_polyphony_studies.md) §1.4) |
