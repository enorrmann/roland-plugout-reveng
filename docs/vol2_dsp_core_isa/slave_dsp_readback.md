# Volume II — Chapter 11: Reading the Memory of the Slave DSPs (investigation, 2026-10-21)

> **Scope:** SYSTEM-8. On the SYSTEM-1 there is no read path from the PC either ([Vol. I ch. 1](../vol1_system_architecture/soc_overview.md) §6.7).

Goal: to be able to read, from the host, the memory of a DSP of a slave BMC (program, bank 0, bank 5, bank 6) to
compare it with a simulator. Without this MAIN0, which runs on the voices (slaves), cannot be validated.

**Short conclusion**: today there is **no verified read path from a slave**. The slave engine's code does access that
memory (it writes and could read), and the only output channel measured from a slave to the host is **USB audio**. A
return data channel is missing, and the resolution of that audio has to be measured.

## 1. What is verified

| fact | source |
|---|---|
| Region 1 of the plugout runs on the ARM of the slave BMCs; region 0 runs on the master. | [Vol. IV ch. 6](../vol4_firmware_container_protocol/base_firmware.md) §8.13 («so the engine code is not present in the master's address space») and §7.10 |
| `dsppeek` only reads the master's memory: «the slave's (the voices) is not reachable from region 0». | [Vol. IV ch. 2](../vol4_firmware_container_protocol/plugout_virtual_machine.md) (live dump of the master), `dsppeek.py` header |
| Memory map of a slave DSP `n` (0–5): base `0x41400000 + n·0x100000`; bank 0 (float) at `+0x00000`; banks 1, 2, 4 at `+0x10000`, `+0x20000`, `+0x30000`; program at `+0x80000` (= `0x41480000 + n·0x100000`, the same address the master uses); parameter block at `+0xA0000`. | [Vol. IV ch. 2](../vol4_firmware_container_protocol/plugout_virtual_machine.md), «Memory map of a slave DSP» (from `s_boot`) |
| Bank 5 of the slave: `0x41114000 + n·0x4000` (0x400 floats). | [Vol. IV ch. 2](../vol4_firmware_container_protocol/plugout_virtual_machine.md), same section |
| The slave engine writes programs and banks into DSP memory with `volatile` pointers (`live_prog`, `live_bank5`) and receives commands by CC1 forwarded by region 0. | `engine/engine.c`, block `#ifdef DSPLIVE` (lines ~352–360, ~506–560) |
| The `dsplive` commands (`ADDR`, `DATA`, `CTRL`) reach the slaves (`cmd 0 ADDR`, `cmd 1 DATA`, `cmd 2 CTRL`, bank 5 with `ext & 4`). | `re/dsplive.py` header |
| The master is the USB host of the slaves (`BMCBulk%d Attached`, `Detached`); the function that sends to the slave is `0x010125C1` → `0x0100D548`. | [Vol. IV ch. 6](../vol4_firmware_container_protocol/base_firmware.md) §8.13 (`USB host bulk to the BMCBulk slaves`) and §8.11 (USB host driver of the master's kernel) |
| The audio output is captured over USB at 96 kHz, in stereo float32. | `re/dspaudio.py` header |

## 2. What is not verified (blockers)

1. **Return channel slave → master.** The only thing documented is the master as a USB host that sends. No code has
   been found in the master that receives data from the slave (neither bulk IN, nor mailbox, nor MIDI). It has to be
   measured.
2. **Resolution of the audio.** The capture is float32, but the output chain of the device (DAC, master mix, USB) may
   have 16 or 24 bits. Without that measurement, a 32-bit RAW value cannot be transmitted exactly over audio.
3. **Read offset of bank 0 and bank 5 on the slave.** The map of [Vol. IV ch. 2](../vol4_firmware_container_protocol/plugout_virtual_machine.md) gives the float view at
   `+0x00000` and says the u32 aliases are at `+0x2000` and `+0x4000`. `engine.c` uses `+0x2000` for bank 5 («as the
   master's»), while [Vol. IV ch. 2](../vol4_firmware_container_protocol/plugout_virtual_machine.md) gives `0x41114000 + n·0x4000` without an offset. Which is the correct RAW
   view must be measured before reading anything.
4. **State of DSP0 on the slave.** MAIN0 runs on a slave; which one has to be confirmed.

## 2b. Measurements of this round (without touching the device)

- **Audio, noise floor**: the existing recordings of programs with zero or constant output
  (`work/dspaudio/a01_zero.raw`, `a02_one_2203.raw`, `d00_zero.raw`, `pc_0.raw`) have a standard deviation of
  0.014–0.023 and are not DC. That floor is equivalent to about 6 bits over full scale. While the floor is not
  removed, the audio cannot carry 32-bit RAW words. It is not a measurement of the DAC's capacity but of the current
  mix of the device; it would have to be measured with the other voices silenced.
- **Link of the slaves (`work/os/s_boot/s_boot.bin`)**: the strings `RXDT`, `TXDT`, `RXMIDI`, `TXMIDI`,
  `USB Access (DSP %c%c%c%c%c%c) %s` and `BMCInitWaveBus` appear. `s_data` has `RXDT Dmp` and `ASGN Dmp`, but there
  are no code references to those strings (search for literal pointers in `s_data.bin` without result): they are
  labels of the debug screen, not commands. The format of the `RXDT` packet and who sends it remain unidentified.
- **Debug screen**: `CEZUtilDSPDump` ([Vol. II ch. 6](memory_banks_dma.md) §1) lists the memory of the DSPs with pages DSP0..5 also on
  the slave. It is the most likely candidate to use a read protocol, but it is not decoded.

## 2c. SysEx protocol of the slave (disassembly of `s_boot`, 2026-10-21)

Disassembled with `arm-none-eabi-objdump -d work/os/s_boot/s_boot.elf` (section `.os`, base `0x01100000`). The strings
are referenced with `adr` (PC-relative), not with literal pointers; `objdump` annotates the target in the comment of
each `adr`.

| address | function | what it does |
|---|---|---|
| `0x01102bd8` | main loop of the slave | calls on every iteration RX MIDI (`0x01107786`), `0x011078e2` (debug `TXMIDI`), RXDT (`0x01107cee`) and TXDT (`0x01107e9e`) |
| `0x01107d76` | USB-MIDI SysEx reassembly | reads the CIN (`byte0 & 15`): 4 = SysEx bytes, 5/6/7 = end with 1/2/3 bytes; accumulates into the context from `+0x000` and stores the length at `+0x200` |
| `0x01107d6c` | dispatcher | passes the context as the buffer and `[ctx+0x200]` as the length to `0x011078fc`: **forwards the whole message** (echo) |
| `0x011078fc` | SysEx sender | packs the buffer into 4-byte USB-MIDI packets (CIN 4, and 5/6/7 at the end) and puts them in the transmit queue |
| `0x011079e6` | transmit queue | inserts a 32-bit word into a circular queue of 1024 entries (`cpsid`/`cpsie` around it) |

Provisional conclusion: the slave has a **bidirectional SysEx channel** over the USB link, with echo. What is not
resolved is whether the master forwards SysEx to the slaves and whether it returns to the host what it receives from
them.

**User's information (2026-10-21)**: arbitrary SysEx do not pass from the master to the slaves, but some known CCs
do. That is consistent with what is documented: the channel that works is the one of the CCs forwarded by region 0
(`dsplive.py`) and that of the OS parameters ([Vol. I ch. 1](../vol1_system_architecture/soc_overview.md) line 68: `set(part, group, id, value)` of the kernel
→ `vt+0x30(id, value)` of the engine). The slave's SysEx echo is only triggered by a SysEx that reaches the slave, and
with generic SysEx none arrives. Therefore **the echo cannot be triggered from the host today**. According to
[Vol. I ch. 1](../vol1_system_architecture/soc_overview.md) line 72, the slaves have no output to the PC: the echo only helps if the master picks it up from the
USB link, and that is not measured.

**Test on the device**: a SysEx `F0 7D 77 01 02 03 04 F7` sent to the port `SYSTEM-8 MIDI 1` (`hw:3,0,0`) with the
DSPLIVE lab installed produced no answer in 5 s. It is **inconclusive**: the lab is the DSPLIVE one and we do not know
whether the master routes generic SysEx to the slaves.

## 2d. Return test with the slave engine (2026-10-21, hardware)

`engine/engine.c` (`-DDSPLIVE`): the control CC `cmd 2, ext 2/3/4` calls `live_sysex_test(cable)`, which puts into
the slave's transmit queue (`0x011079E6`) the SysEx `F0 7D 77 01 02 03 F7` as USB-MIDI packets of cable 0, 8 and 9
(the OS sender always uses cable 0). The host listens on `SYSTEM-8 MIDI 1` with `amidi -d`.

| test | result |
|---|---|
| control CC `cmd 2 ext 2`, `3`, `4` (cable 0, 8, 9) sent with `dsplive.Link` | no test SysEx reaches the host |
| host capture with a SysEx of the master (`F0 7D 03 01 02 03 04 F7`, with `dsppeek ping`) | the capture **does** receive the master's messages |

What remains **not verified**: (a) that the engine receives the control CC (there is no observable effect of `cmd 2`
with `ext 2–4`); (b) that the slave's queue drains towards the master; (c) that the master routes to the host the
packets it receives from the slave. Without (a) it cannot be separated from (b) and (c).

## 3. Proposed route (in order)

1. **Measure the resolution of the audio** with a test program of known values (16- and 24-bit bit patterns) on a
   slave DSP, with `dspaudio.py` and `dspexp/audio`. It is a hardware measurement that does not change the engine
   code.
2. **Confirm the RAW view of the slave's bank 5** with a known write (`dsplive poke` of bank 5) and a read by audio.
3. **READ command in the slave engine** (`engine.c`, new branch of `live_cmd`): copies a word from the requested
   address to a bank-5 word of a test DSP. It requires building the lab and reinstalling it with
   `dsplive.py install`, and selecting PLG3 on the panel.
4. **Output program in the test DSP**: reads that bank-5 word and emits it over audio in two 16-bit fragments. The
   host rebuilds the RAW word from the recording.
5. **Only if step 1 gives enough resolution**: dump bank 0 of DSP0 of the slave (ring of MAIN0) and compare with the
   simulator.

What step 3 does not solve: synchrony with the sample. Reading the ring still depends on the moment (see
`dspmaster.py`: repeated reads and majority).

## 4. State

- No new code on the device in this investigation. DSPLIVE remains installed in PLG3 from the previous session.
- Next step that requires the device: step 1 (measurement of the audio resolution), with `dspaudio.py install`.

> Paths `engine/…`, `re/…` are relative to `examples/from_scratch/`.
