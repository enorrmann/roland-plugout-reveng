# Glossary

Terms, abbreviations and names used in the documentation. Each entry says what the term means **in these
documents** and where it is defined; **S8** = SYSTEM-8 only, **S1** = SYSTEM-1 only, **both** = common to both
devices. Many abbreviations are Roland's own names, read in OS strings, debug screens or ARM tables (`DT0`, `DPC`,
`MXM`, `IOMD`, `DSPM`, `UMDW`, …); where no expansion of such a name appears in Roland's code or in the evidence, the
entry says so and gives none.

## A

**A / B (DSP A, DSP B)** — S1. The two DSPs of each SYSTEM-1 BMC; programs are queued for A with `FC30` and for B
with `FC34`. [Vol. I ch. 1](vol1_system_architecture/soc_overview.md) §6.

**A op, B op (unit A, unit B)** — both. The two kinds of arithmetic op of a DSP instruction: unit A (add, multiply,
multiply-add, Newton step…) and unit B (min/max, abs, phase ops, bit ops, SFU…). [Vol. II ch. 1](vol2_dsp_core_isa/core_architecture.md) §5.

**A/A/B, AAB** — both. Instruction template (class 0x160) with two A ops and one B op; all three read before they
write and the instruction costs one cycle. [Vol. II ch. 4](vol2_dsp_core_isa/instruction_set_reference.md) §10.

**`App1_Main`** — S8. The SYSTEM-8 OS image inside the updater (`p1y01_m0c0a_up.bin`), loaded at `0x60000000`;
its payload is 3-key 3DES in ECB. [Vol. IV ch. 1](vol4_firmware_container_protocol/container_formats.md) §1.4.1a.

**assigner** — both. The region-0 object that receives notes and assigns them to voices (`CAssign…` classes on the
S8, SYNINFO `+0x2C` on the S1). [Vol. III ch. 1](vol3_voice_allocation_scheduling/system8_voice_allocation.md), [ch. 2](vol3_voice_allocation_scheduling/system1_voice_allocation.md).

## B

**bank 0 … bank 7** — both. The DSP data banks, selected by bits 15–13 of a reference: 0 `0nnn` (DT0, ring), 1 `2nnn`
(DT1, ring, main ↔ section mailbox), 2 `4nnn` (delay-memory window), 3 `6nnn` (IOMD), 4 `8nnn` (MXM, S8), 5 `Annn`
(DPC, plain memory the ARM writes), 6 `Cnnn` (parameters, read-only for the DSP), 7 `Ennn` (counters / hardware
state). [Vol. II ch. 1](vol2_dsp_core_isa/core_architecture.md) §7.1; S1: [Vol. II ch. 6](vol2_dsp_core_isa/memory_banks_dma.md) §7.

**BMC** — both. Roland's chip that holds an ARM core and the DSPs. The SYSTEM-8 has a master and two slave BMCs, the
SYSTEM-1 a master and one slave. The exact core of its ARM is not known. [Vol. I ch. 1](vol1_system_architecture/soc_overview.md).

**budget** — both. The number of instructions a DSP can execute per sample (one cycle per instruction): ≈ 925–945 on
the S8; on the S1 slave DSP A 703 ran and 826 hung, DSP B 708 ran. [Vol. III ch. 4](vol3_voice_allocation_scheduling/execution_timing_budgets.md).

## C

**C word (condition word)** — both. A word in an instruction that predicates its A/B ops on one of the condition flags.
[Vol. II ch. 3](vol2_dsp_core_isa/instruction_word_encoding.md) §5.4.

**C-… (C-V4, C-S1V8, C-S1SY, C-WTLIVE, …)** — names of our own PLUG-OUT builds (the name shown on the device).
**MP-…** = Mutable ports, **WT-…** = wavetable builds. [Vol. VI](vol6_reference_designs_applications/c_v4_system8_reference.md).

**cable** — both. The 4-bit cable number of a USB-MIDI packet (`{cable<<4 | CIN, …}`); the OS uses it to route
messages. [Vol. I ch. 5](vol1_system_architecture/midi_sysex_routing.md).

**`cchain`, `dspcore`, `dspref`** — the C emulator of the DSP and its Python bindings (`examples/from_scratch/emu/`).
[Vol. V ch. 5](vol5_toolchain_compiler_sdk/emulator.md).

**CEZUtil…** — S8. Class-name prefix of the OS's debug console screens (`CEZUtilDSPDump`, …) on master core 1.
[Vol. IV ch. 6](vol4_firmware_container_protocol/base_firmware.md) §8.

**CIN** — both. Code Index Number of a USB-MIDI event packet (`byte0 & 15`): 4 = SysEx bytes, 5/6/7 = SysEx end,
8/9/B/E = note off/on/CC/bend. [Vol. I ch. 5](vol1_system_architecture/midi_sysex_routing.md).

**closing** — both. The program at `0x7FF2` that ends a DSP's sample and updates the frame counter. S1:
[Vol. I ch. 3](vol1_system_architecture/bus_interconnect.md) §15.3.

**`Code*.Dat`** — both. The PLUG-OUT firmware container of the plugins (`Code8_<slot>.Dat` for the S8, `Code1.Dat`
for the S1), installed over USB-MIDI. [Vol. IV ch. 1](vol4_firmware_container_protocol/container_formats.md), [ch. 4](vol4_firmware_container_protocol/usb_sysex_protocol_spec.md).

## D

**DCO slot** — S8. The program memory area where an oscillator program runs (DCO1 at `0x6000`, DCO2 at `0xB000` of
DSP2/DSP3 of each slave). [Vol. III ch. 3](vol3_voice_allocation_scheduling/slot_program_model.md).

**`divseed`, `recipseed`, `newton`** — both. Reciprocal seed with a numerator, reciprocal seed, and the Newton /
Goldschmidt step used to divide. [Vol. II ch. 5](vol2_dsp_core_isa/sfu_and_accelerators.md).

**`doccheck`** — `tools/doccheck.py`: checks that every ```` ```dsp ```` block of the documents assembles.

**DPC** — both. Roland's name for bank 5 in the ARM bank table (plain persistent memory, `A000`–`A3FF`); no
expansion is known. [Vol. II ch. 6](vol2_dsp_core_isa/memory_banks_dma.md) §1.

**DSP0 … DSP5** — S8. The six DSPs of each SYSTEM-8 BMC; in the voice chain DSP0/1 → DSP2/3 → DSP4/5.
[Vol. III ch. 1](vol3_voice_allocation_scheduling/system8_voice_allocation.md).

**DSPLIVE, PEEK, PARAMLOG, VOICELOG, MC_TEST, MA_MAIN, SL_LOAD** — build options of our PLUG-OUTs: live writing of
the slave DSPs (`dsplive.py`), live access to the master (`dsppeek.py`), sending each received parameter to the PC,
sending each assigned note to the PC, replacing a main program for a test, loading an own empty master-A main program
(S1), loading a wavetable frame over USB (S1, untested). See the chapter of each example.

**DSPM** — S8. A seventh DSP object per BMC in the OS tables (`CDSPMPrm`, base `0x41260000`); not studied.
[Vol. II ch. 1](vol2_dsp_core_isa/core_architecture.md) §11.

**DT0, DT1, DT2** — both. Roland's names for banks 0, 1 and 2 in the ARM bank table. [Vol. II ch. 6](vol2_dsp_core_isa/memory_banks_dma.md).

**DT1, RQ1 (SysEx)** — both. Roland's SysEx commands "data set 1" (`12`) and "data request 1" (`11`); the install
protocol and the patch-memory reads use them. [Vol. IV ch. 4](vol4_firmware_container_protocol/usb_sysex_protocol_spec.md).

## E

**EDM** — both. External delay memory: the large delay/sample memory of a BMC (0x200000 words), read by the DSPs
through tap requests and cleared by the ARM at start-up. **IDM** is the other delay memory channel of the same block
(0x20000 words); no expansion of its name appears in the evidence. [Vol. II ch. 6](vol2_dsp_core_isa/memory_banks_dma.md) §2.1;
use: [Vol. VI ch. 7](vol6_reference_designs_applications/mutable_memory.md).

**engine** — both. The PLUG-OUT code of region 1, on the slaves (vtable with `load`, `param`, …).

**evidence tags** — [M] measured on the device, [L] read in Roland's code, [C] our code, [E] emulator, [I] inferred,
[A] open; some chapters use [exact] / [measured] / [inferred] / [code] / [sim] / [est]. Each chapter gives its legend.

## F

**FCxx, FDxx, FExx services** — S1. Entries of the PLUG-OUT service table at `0x0100FC00`, named by their low
address: `FC30`/`FC34` queue programs for DSP A/B, `FC38`/`FC3C` data queues, `FC40` place a voice section, `FC44`
switch a section on/off, `FCE4` parameter with hardware ramp, `FCEC` immediate parameter, `FD6C` bank-5 word,
`FE3C` end-of-ramp callback. [Vol. IV ch. 3](vol4_firmware_container_protocol/os_service_table.md).

**family A / family B** — S8. The two layouts of Roland's SYSTEM-8 PLUG-OUTs: family A (JX-3P, JUNO-60, JUNO-106,
JUPITER-4, JUPITER-8: voice chains through the six DSPs) and family B (SH-101, SH-2, SYSTEM-100, PROMARS, base of the
SYSTEM-8). [Vol. III ch. 1](vol3_voice_allocation_scheduling/system8_voice_allocation.md) §3, §6.

**flags, condition groups** — both. Condition bits written by `X9bY` (group X−8 ← flags of unit Y): c0 ≠ 0, c1 ≥ 0,
c2 no overflow, c3 ≤ 0. [Vol. II ch. 4](vol2_dsp_core_isa/instruction_set_reference.md) §8.

**`frexp`, `expsplit`, `log2split`, `sqrtsplit`, `floor`, `ceil`** — both. SFU operations of unit B.
[Vol. II ch. 5](vol2_dsp_core_isa/sfu_and_accelerators.md).

## H

**h4 … h7 (hidden registers)** — both. Registers loaded by `40TY` words: h4 delay base, h5 tap-result address, h6 and
h7 indices of the indexed transfers. [Vol. II ch. 1](vol2_dsp_core_isa/core_architecture.md) §5.6.

**header** — both. The OS program at `0x0000` that starts each DSP's sample; a PLUG-OUT never queues it (S1).
[Vol. I ch. 3](vol1_system_architecture/bus_interconnect.md) §15.3.

**`hset`, `ldidx`, `ldb5`** — both. Our mnemonics: set a hidden register; indexed load from bank 0 (ring-relative,
`X28Y`); indexed load from bank 5 (`da`/`db`). [Vol. II ch. 9](vol2_dsp_core_isa/operation_uses.md) §2.

## I

**IEEE, SD31, RAW views** — both. Three ARM windows on the same DSP memory: IEEE-754 float, signed 31-bit fraction,
and the DSP's own RAW words. [Vol. I ch. 2](vol1_system_architecture/memory_map.md).

**IOMD** — both. Roland's name for bank 3 (`6nnn`): input/output words of the BMC, each with an owner, that link the
DSPs with each other and the chips with each other (and carry the output). No expansion is known.
[Vol. I ch. 3](vol1_system_architecture/bus_interconnect.md) §4.7 (S8), §14.4 (S1).

## L

**LZSS** — both. The compression of the regions of the containers. [Vol. IV ch. 1](vol4_firmware_container_protocol/container_formats.md) §3.

## M

**MAC** — both. The multiply-accumulate unit; a MAC takes two words of an instruction and can add two products.
[Vol. II ch. 1](vol2_dsp_core_isa/core_architecture.md) §5.4.

**main program (MAIN0–MAIN5)** — both. The program that runs every sample on a DSP and calls the slots / sections;
MAIN0–MAIN5 are those of Roland's family-A voice microcode on the S8. [Vol. III ch. 5](vol3_voice_allocation_scheduling/dsp_main_programs.md).

**master, slave** — both. The BMC that runs the OS and region 0 (master) and the BMCs that compute the voices and run
region 1 (slaves). [Vol. I ch. 1](vol1_system_architecture/soc_overview.md).

**model** — both. The PLUG-OUT code of region 0 that receives the panel parameters (S1: the object at SYNINFO
`+0x30`). [Vol. I ch. 1](vol1_system_architecture/soc_overview.md) §6.4.

**`mport`** — `sdk/mport`, the toolkit for porting Mutable Instruments code to the DSP. [Vol. V ch. 3](vol5_toolchain_compiler_sdk/mport_mutable_sdk.md).

**MXM** — S8. Roland's name for bank 4 (`8nnn`): the bus between the DSPs of one BMC, in zones M/O/Z/S of 0x200
words, latched per sample. No expansion is known; the S1 programs do not use it. [Vol. I ch. 3](vol1_system_architecture/bus_interconnect.md) §4.3.

## P

**pair mailbox** — S8. Bank 5 `A400`–`A7FF`, shared by the two DSPs of a pair (0/1, 2/3, 4/5) in Roland's main
programs. [Vol. I ch. 3](vol1_system_architecture/bus_interconnect.md) §4.4.

**`ph_wrap`, `ph_tri`, `ph_clamp`** — both. Our names of the unit-B phase operations (wrap into [−1, 1), triangle,
clamp). [Vol. III ch. 6](vol3_voice_allocation_scheduling/microcode_optimization.md).

**PLG1–PLG3, slot** — S8: the three PLUG-OUT slots; S1: one slot. The PLUG-OUT is selected on the panel.

**PLUG-OUT** — both. Roland's name for a synth engine installed into the instrument from a plugin; here also our own
builds. A PLUG-OUT has two regions (region 0, region 1). [Vol. IV ch. 2](vol4_firmware_container_protocol/plugout_virtual_machine.md).

**`PRELOAD_PLUGOUT`** — S8. The wrapper of the factory PLUG-OUTs inside the updater. [Vol. IV ch. 6](vol4_firmware_container_protocol/base_firmware.md) §1.

**proven forms, bad forms** — S1. The op groups that ran on the device (`asm/proven_forms.txt`) and those that hang
it (`asm/bad_forms.txt`); our scheduler only emits proven groups. [Vol. III ch. 6](vol3_voice_allocation_scheduling/microcode_optimization.md).

## R

**R2–R10 (access rules)** — S1. The per-instruction access rules of `asm/s1rules.py` (bank-5, bank-0 and bank-6
limits, …). [Vol. V ch. 4](vol5_toolchain_compiler_sdk/assembler_disassembler_tools.md).

**RAW** — both. The DSP's only number format: 32-bit sign-magnitude with a 2-bit exponent. [Vol. II ch. 2](vol2_dsp_core_isa/data_types_arithmetic.md) §1.

**region 0, region 1** — both. The two code regions of a PLUG-OUT: region 0 runs on the master's ARM (model,
assigner; the only one that can call the OS's MIDI functions), region 1 on the slaves' ARM (engine).
[Vol. IV ch. 2](vol4_firmware_container_protocol/plugout_virtual_machine.md).

**ring** — both. The addressing of banks 0 and 1: a value stored at S is read at S + 1 in the next sample.
[Vol. I ch. 3](vol1_system_architecture/bus_interconnect.md) §4.1.

## S

**`s1cc`, `s1dsp.h`** — the C / C++ → DSP microcode compiler (LLVM 19) and its SDK header. [Vol. V ch. 1](vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md), [ch. 2](vol5_toolchain_compiler_sdk/s1dsp_api_reference.md).

**section** — S1. One of the voice program areas of a DSP (`0x1900`, `0x3100`, `0x4900`, `0x6100`), placed with
`FC40` and switched with `FC44`. [Vol. III ch. 3](vol3_voice_allocation_scheduling/slot_program_model.md) §3.

**SFU** — both. Our name for the special-function ops of unit B (nibble f): reciprocal seed, root split, exp split,
log2 split, sign, floor, ceil. [Vol. II ch. 5](vol2_dsp_core_isa/sfu_and_accelerators.md).

**SYNINFO** — both. The descriptor at the start of each region (identity, version, `create`/`destroy` and getters)
that the OS checks when it loads a PLUG-OUT. [Vol. IV ch. 1](vol4_firmware_container_protocol/container_formats.md).

**SYSTEM-S1CC** — S1. Our PLUG-OUT that is only an s1cc kernel. [examples/system_s1cc/README.md](../examples/system_s1cc/README.md).

## T

**`tapreq`, tap** — both. A delay-memory read request (`Xa9Y`): the read address in a register, the results (two
samples and the fraction) in bank 2 at h5. [Vol. II ch. 4](vol2_dsp_core_isa/instruction_set_reference.md) §9.1.

**template, class** — both. The layout of the units in an instruction word, selected by the class number (e.g. 0x0B0
= A + B, 0x160 = A/A/B). [Vol. II ch. 3](vol2_dsp_core_isa/instruction_word_encoding.md) §4.2.

## U, W, X

**UMDW** — S8. Roland's name of the internal MIDI/parameter stream between the OS objects (`CUMDWRx/Tx`); no
expansion is known. [Vol. I ch. 4](vol1_system_architecture/host_control_plane.md).

**μT-Kernel** — S8. The RTOS of master core 0. [Vol. IV ch. 6](vol4_firmware_container_protocol/base_firmware.md) §8.4.

**wave bus** — both. The connection that carries the voices between the BMCs (pins configured by
`BMCInitWaveBus`), seen by the DSPs as IOMD words. [Vol. II ch. 6](vol2_dsp_core_isa/memory_banks_dma.md) §2.4.

**WVM** — S8. Name of a monitor on the ARM debug page (`WVM_MON`, wave-bus memory fill). [Vol. II ch. 6](vol2_dsp_core_isa/memory_banks_dma.md) §2.1.

**`X9bY`, `Xa9Y`, `X28Y`, `40TY`, `009Y`** — both. Transfer/control words named by their hex pattern: set flags,
delay-tap request, indexed transfer, hidden-register load, jump. [Vol. II ch. 4](vol2_dsp_core_isa/instruction_set_reference.md) §8–§9.
