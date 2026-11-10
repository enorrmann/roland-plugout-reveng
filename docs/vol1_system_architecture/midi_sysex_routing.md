# Volume I — Chapter 5: MIDI and SysEx Routing in the SYSTEM-8 (investigation, 2026-10-21)

> **Scope:** SYSTEM-8. The SYSTEM-1 MIDI send from region 0: [Vol. IV ch. 6](../vol4_firmware_container_protocol/base_firmware.md) §9.3.

Working document on how MIDI and SysEx messages travel through the system: from the host (PC) to the master, from the
master to the slaves and back. Goal: a read channel for the memory of the slave DSPs ([Vol. II ch. 11](../vol2_dsp_core_isa/slave_dsp_readback.md)).
State: **partial**. What remains open is at the end.

Certainty: **[measured]** = seen on the device; **[code]** = read in the disassembly of the image, not measured;
**[inferred]** = deduced, not confirmed.

---

## 1. Topology [code + measured]

| node | code | what it runs | MIDI |
|---|---|---|---|
| PC (host) | — | `amidi`, `dsppeek.py`, `dsplive.py` | USB-MIDI with the SYSTEM-8 |
| Master, core 0 | `m0_core0` (`0x60000000`) | OS (μT-Kernel), MIDI router `0x6000345C`, region 0 (plugout hooks) | receives from the host and sends to the host |
| Master, core 1 | `m1_tcm`, `m1_sdram` | DSP control and global effects | — |
| Slaves (2) | `s_boot`, `s_data` (`0x01100000`, `0x60000000`) | slave OS, region 1 (plugout engine) | receive from the master over **USB bulk** (`BMCBulk`), send to the master over the same link |

Master↔slave link: internal USB. The master is the host (`BMCBulk%d Attached/Detached`; `0x010125C1` → `0x0100D548` sends binary images to the slaves, §3).
The slaves have no output to the PC ([Vol. I ch. 1](soc_overview.md), «the slaves have no output»).

## 2. Messages that arrive from the host [measured]

| message | does it reach the slave? | measured with |
|---|---|---|
| Control CC (`dsplive`: bend 0x465 + CC1 0x467) | **yes** (the engine processes them) | `dsplive install/poke/recall` (live pokes of DSP2/3) |
| Panel parameters (`set(part, group, id, value)`) | yes, through the kernel | [Vol. I ch. 1](soc_overview.md) |
| Generic SysEx `F0 7D 77 01 02 03 F7` | **no** | user's test (2026-10-21) |
| Install SysEx (`F0 41 10 00 00 ...`) | yes, to the master (installation) | [Vol. IV ch. 4](../vol4_firmware_container_protocol/usb_sysex_protocol_spec.md) |

## 3. SysEx reception on the master [code]

Disassembly of `dumps/os_full.bin` (base `0x60000000`, Thumb), with `arm-none-eabi-objdump -D -b binary -m arm
-M force-thumb --adjust-vma=0x60000000`.

| address | function | what it does |
|---|---|---|
| `0x60134692`–`0x601346cc` | packet handler (wraps) | `ldrb [sp+64] >> 4`: if it is **3 (SysEx)** it jumps to `0x601346cc` → `0x6013459a`; the other types go to the dispatcher `0x6013a9dc` if `[r4] != 0` |
| `0x6013459a` | **USB-MIDI SysEx reassembler** | CIN = `byte0 & 15`: 4 = SysEx bytes; 5/6/7 = end with 1/2/3 bytes. Accumulates into the object `r4`: data from `+0x008`, length at `+0x208` (`uint16`), limit 508 bytes (`0x1FC`) |
| `0x60134640`–`0x6013464c` | **complete a message** | **virtual** call: `r0 = [r4]` (vtable), `r6 = [vtable+4]`, `blx r6` with `r1 = obj+8` (buffer) |
| `0x6013a9dc` | MIDI dispatcher of system-common messages | `switch` over `0xF1`, `0xF2`, `0xF3` (`tbb` table); does **not** handle `0xF0` |

**Decision point**: the `vtable+4` method of the reassembler object. That is where it is decided what to do with a
complete SysEx.

**Live dump (2026-10-21, `examples/midi_send/bulk_dump.py`, hardware)**:

| datum | address | value | file |
|---|---|---|---|
| reassembler object (master RAM) | `0x20082008` | `[0]` = `0x0113EC30` (vtable), `[+0x0C]` = `0x10` | `work/ram/m0_ram_20082000.bin` |
| virtual table | `0x0113EC30` | `[0]` = `0x011061D5`, **`[+4]` = `0x01103907`** | `work/ram/m0_vt_0113ec00.bin` |
| method `vtable+4` | `0x01103906` (Thumb) | **`bx lr`: does nothing** | `work/os/m1_tcm/m1_tcm.elf` |

The object and the table live in RAM / in the `m1_tcm` area (`0x0113xxxx`), not in `os_full.bin`, which is why they
had to be dumped. **Result**: the complete SysEx that this object receives **is discarded**: the reassembler stores it
and the final call goes to an empty function. That explains why a generic SysEx from the host does not reach the
slaves.

**[inferred]**: that this object is the one of MIDI 1 (`SYSTEM-8 MIDI 1`, the one the host uses in our tests) and not
the installation one. The installer uses MIDI 2 (`bulk_dump.py`), which may have another receiver; not checked. The
literal of `0x60134740` points to `0x20082008` only in the type-3 branch of the handler of `0x60134692`.

**Address verification (2026-10-21, afternoon)**: every address cited was checked against its source.

| address | source | result |
|---|---|---|
| `0x20082008` contains `0x0113EC30` | live dump `work/ram/m0_ram_20082000.bin` | confirmed |
| `0x2008200C` contains `0x10` | live dump | confirmed |
| `0x0113EC30` = `0x011061D5`; `0x0113EC34` = `0x01103907` | live dump `work/ram/m0_vt_0113ec00.bin` | confirmed |
| `0x01103906` = `bx lr` (`4770`) | **live dump** `work/ram/m1_code_01103800.bin` (and the static image `m1_tcm.elf`) | confirmed live |
| literal `0x60134740` = `0x20082008` | image `dumps/os_full.bin` | confirmed |
| `0x6000345C` = router (`push` of 14 registers) | `dumps/os_full.bin` | confirmed |
| `0x010125C1` → `0x0101257A` → `0x0100D548` | `dumps/kernel_01000000.bin` | confirmed (the chain of jumps) |

`0x010125C1` sends **binary images** (header `BUDB`, load address, type `DATA`/`BOOT`) to the USB link
of the slaves ([Vol. IV ch. 6](../vol4_firmware_container_protocol/base_firmware.md) §7). It cannot forward SysEx: the slave's receiver would treat the data as an image
to write at the load address. The safe path to forward MIDI to a slave is the router `0x6000345C` with USB-MIDI
packets of cable 8 (the same one used by the CC of `examples/midi_send/cc.s`).

The debug strings `SysEx Body %02X %02X %02X`, `SysEx End1/2/3` are at `0x6013adb4` and `0x601fcfa8` (referenced with
`adr` at `0x6013ab38` and `0x601fcd2c`, inside the dispatcher). They are traces, not the decision logic.

## 4. Routing by port and cable [code]

Documented in [Vol. IV ch. 6](../vol4_firmware_container_protocol/base_firmware.md) (router `0x6000345C`):
* Type of the wrap = `w[7] >> 1`: **3 = SysEx** (`Msg`, port in `Msg[6]`), **4 = USB-MIDI packet**
  (`{cable<<4 | CIN, ...}`).
* The router treats each bit of the mask as a destination: bit 0 → `0x60007794`, bit 1 → `0x6003E32A`, bit 2 →
  `0x60057CB8`, bit 3 → `0x6003F37C`, bit 4 → `0x6003F526`, bit 5 → `0x60052B8A`, bit 6 → `0x6000EFC8`, bit 7/8 → queue
  of `queue2_post`.
* Cables 8 and 9 are converted into the logical labels `0x800` and `0x1000`, and the router calls itself with the base
  mask `0x00081603` (bits 0, 1, 9, 10, 12, 19). **[inferred]**: the messages of the slaves come in with cable 8/9.

## 5. Slave side [code]

Disassembly of `work/os/s_boot/s_boot.elf` (`arm-none-eabi-objdump -d`, section `.os` base `0x01100000`). The strings
are referenced with `adr`; `objdump` annotates the target.

| address | function | what it does |
|---|---|---|
| `0x01102bd8` | main loop | calls on every iteration RX MIDI (`0x01107786`), debug `TXMIDI` (`0x011078e2`), RXDT (`0x01107cee`), TXDT (`0x01107e9e`) |
| `0x01107d76` | SysEx reassembler | same as the master's (CIN 4–7, buffer at `+0x000`, length at `+0x200`) |
| `0x01107d6c` | echo dispatcher | passes the complete message to `0x011078fc` (**echo** of the received SysEx) |
| `0x011078fc` | SysEx sender | packs into 4-byte USB-MIDI packets; **cable always 0** (the packet carries CIN 4 in the low nibble) |
| `0x011079e6` | transmit queue | inserts a 32-bit word into a circular queue of 1024 entries (`cpsid`/`cpsie`) |

Conclusion: the slave has a bidirectional SysEx channel with echo. The echo is only triggered by a SysEx that reaches
it, and generic SysEx from the host do not reach it (section 2).

## 6. Tests on the device

| test | result | note |
|---|---|---|
| SysEx `F0 7D 77 01 02 03 F7` to the port `SYSTEM-8 MIDI 1` (`hw:3,0,0`) | does not reach the slave (no echo) | section 2 |
| host capture (`amidi -d`) with a SysEx of the master (`dsppeek ping`) | **does** receive messages from the master (`F0 7D 03 01 02 03 04 F7`) | listening works |
| engine with test command `cmd 2, ext 2/3/4` → SysEx on cable 0/8/9 (`engine/engine.c`, `live_sysex_test`) | **no SysEx reaches the host** | it is not known whether the engine receives the CC, whether the queue drains, or whether the master forwards to the host |

The test command is in `engine/engine.c` (block `#ifdef DSPLIVE`, functions `live_tx_packet` and `live_sysex_test`).
It is installed in PLG3 since that test.

## 7. What is not closed

1. **Vtable of the master's reassembler** (section 3): read the master's live RAM or follow the object's
   initialisation. With that one knows which function decides the forwarding.
2. **SysEx filter of the master**: if the `vtable+4` function discards the SysEx of an unknown manufacturer, the patch
   must replace or wrap that function.
3. **Whether the engine receives the control CC**: there is no observable effect of `cmd 2`. An audible test or a
   signal in the slave's queue is needed before blaming the master.
4. **Whether the master accepts the packets of the slaves** (cable 8/9): the test of section 6 does not separate it
   from causes 3 and 1.

## 8. Risks of patching the master

* A patch in the master's region 0 can leave the device unusable until the firmware is reinstalled.
* The patch in the slave (engine) has already been used several times without damaging the device; the master patch
  has not.
* Any change requires reinstalling the lab and selecting PLG3 on the panel.

## 9. Tools and paths

| what | where |
|---|---|
| Complete OS image | `dumps/os_full.bin` (2939504 bytes, base `0x60000000`) |
| Master image (core 0) | `work/os/m0_core0/m0_core0.elf` |
| Slave image | `work/os/s_boot/s_boot.elf`, `work/os/s_data/s_data.elf` |
| Full disassembly of the OS | `arm-none-eabi-objdump -D -b binary -m arm -M force-thumb --adjust-vma=0x60000000 dumps/os_full.bin` |
| SysEx echo in the engine | `examples/from_scratch/engine/engine.c` (`live_sysex_test`) |
| Sending control CCs | `examples/from_scratch/re/dsplive.py` (`Link.cmd`) |
| Host capture | `amidi -p hw:3,0,0 -d` (SYSTEM-8 MIDI 1) |

## 10. History

* 2026-10-21: map of SysEx reception of the master (section 3) and of the slave (section 5); echo tests (section 6).
* 2026-10-21 (afternoon): dump of the master's RAM. The reassembler object (`0x20082008`) has `vtable[1]` = `bx lr`:
  the complete SysEx is discarded. Verified in the dump; the port it corresponds to, **[inferred]**.

## 11. SysEx forwarding patch (2026-10-21, afternoon): applied, does not trigger

Hook: `examples/midi_send/init_sxfwd.s`, built with `make_init_sxfwd.sh`, installed in slot 3 as `JX-3P-INIT-SXFWD`
(name `SX FWD OK`, version 1.21).

What the hook does when installed (region 0, core 0):
1. Copies the 16 entries of the virtual table `0x0113EC30` into a new table inside the hook's cave, with `[1]` = `fwd`.
2. Points the object `0x20082008` to the new table.
3. Writes the mark `SX FWD OK` into its name.

**Verified**:
* The mark is read with `tools/plugout_tool.py info`: the hook runs on install.
* Dump of `0x20082000` (`work/ram/m0_obj_after_patch.bin`): `[0x20082008] = 0x61C001E0`, which is the new table (the
  cave is at `0x61C00000` for slot 3; the hook is built for slot 1 and the tool relocates it).

**Not verified / negative result**: with `fwd` (which first sends a marker CC `4B BF 66 01` through the router and then
the USB-MIDI packets of the SysEx on cable 8) no message appears at the host, neither on port 1 nor on port 2, with
`F0 7D 77 01 02 03 04 F7` sent by `amidi`. Neither the marker CC nor the echo. That indicates that **the SysEx sent from
the host does not reach `fwd`**: the patched object is not the one that processes that MIDI input. The SysEx could
also not get past the reassembler; that is not separated.

**Recorded risk**: the object `0x20082008` points to a table that lives in the hook's cave. If another plugout is
installed, the cave is rewritten and the table points to foreign code. A SysEx reaching that object afterwards could
hang the OS. So far no hang has happened with the later dumps.

**Next step**: find which object processes the host's MIDI input. Follow backwards from the handler at `0x60134692`
(the one that calls `0x6013459a` with the object): who creates the object, which port feeds it and why the
`vtable+4` call does not happen for `amidi`.

### 11b. Search for the receiver of the host input (2026-10-21, night)

* The constructor `0x60134708` creates **two** SysEx receivers with the same code: `A` = `0x20081DFC` (table
  `0x0113E49C`) and `B` = `0x20082008` (table `0x0113EC30`). The function's literals show it (`0x60134728` =
  `0x20081DFC`, `0x60134740` = `0x20082008`).
* **Both have `vtable[1]` = `bx lr`**: the complete SysEx is discarded in both. Verified in the `m1_tcm` image
  (`0x0111E6B2` and `0x01103906`).
* The hook now patches both objects (version 1.22). With a SysEx `F0 7D 77 01 02 03 04 F7` sent by `amidi` to MIDI 1
  and to MIDI 2, **neither the marker CC nor the echo appears**. Neither of the two receivers is the one that
  processes the host input, or the SysEx does not reach the completion phase.
* The table pointer of object `B` remains pointing to the hook's cave (risk of section 11).
* Untested candidates: (1) the system's MIDI Rx/Tx handling (parameters `0x22C`, `0x22`, `0x1E`, `0x2A`, `0x2B`,
  `0x0C`, read at `0x600753A4`: [Vol. IV ch. 6](../vol4_firmware_container_protocol/base_firmware.md), router), which could discard SysEx before the reassembler; (2)
  another receiver constructor not found with the literals.

### 11c. Second input queue and third receiver (2026-10-21, night)

* The producer of the input queue is `0x601F4A6C` (queue at `0x20080370`), with a single caller (`0x601F4D6A`, wrapper
  `0x601F4D66`).
* There is a **second copy of the consumer loop** at `0x601F4CFC`, with its own type-3 SysEx receiver: object `C` =
  `0x20081D88` (literal `0x601F4DB8`), completed by `0x601F4C12`. Objects `A` and `B` of the loop `0x60134684` as in
  11b.
* Dump of `C` (`work/ram/m0_objC_20081d00.bin`): `[C] = 0x01F75516` (virtual table); `C+0x208` = 0 at rest. **The
  virtual table of `C` has not been read**: `0x01F75516` is neither in the dumps nor in the OS image.
* **Incident**: reading `0x01F75500` with `bulk_dump.py` returned no data (0 messages) and the following `info` did not
  answer: the device was left without USB until it was restarted. Probable cause: unmapped address in region 0. **Do
  not read addresses again without confirming that they exist.**
* After the restart the RAM patch is lost (the RAM is reset), and slot 3 was left with `BULK` (the last dump).
* Completion of `C`: at `0x601F4CBC`-`0x601F4CC4`, `[C+0]` → `[vt+4]` → `blx` (virtual, like in `B`).
* **But** `[C+0] = 0x01F75516` is not a valid virtual-table pointer: it is not 4-byte aligned and it is not Thumb code.
  Moreover, `C` at rest does not have the shape of a reassembler (`+0x208` = 0, the rest of the fields meaningless).
  Provisional conclusion: `C` is not an active receiver, or the pointer read belongs to another object. **What its
  completion does cannot be said.**
* Proposed next step: check whether the host's SysEx reaches the input queue, comparing with a CC (which does reach the
  slave) sent through the same port; the difference will tell whether the filter is before or after the queue.

### 11d. Probe of the input queue (prepared, not installed, 2026-10-21)

Hook: `examples/midi_send/init_probe_q.s`, built with `make_init_probe_q.sh` (`plugins/JX-3P-INIT-PROBEQ`, name
`PROBE Q`, version 1.30).

* Patches the wrapper that produces the input queue, `0x601F4D66` (10 bytes: `nop`, `ldr.w pc,[pc,#0]`, hook address).
  The wrapper is the only producer of the queue `0x20080370`.
* Each incoming message is counted by type (`wrap[7] >> 1`: 3 = SysEx, 4 = USB-MIDI packet) and the result is written
  into the plugout's name: `SX<n> P<m>` (n and m, modulo 10).
* Test: install, then send through `SYSTEM-8 MIDI 1` a CC and a SysEx, and read the name with
  `tools/plugout_tool.py info` after each send.
  * If only `P` goes up with the CC and `SX` does not go up with the SysEx: the SysEx is discarded **before** the
    queue.
  * If `SX` goes up with the SysEx: the SysEx enters the queue, and the discard is in the consumer (`B` or `C`).
  * If nothing goes up: the port does not enter this queue, or the patch was not applied (code-cache coherence on the
    other core, not checked).
* Risk: it patches code in SDRAM from core 0. If the wrapper is in use by the other core, there can be cache coherence
  issues. It needs a restart to remove it.

* **Results of the probe (2026-10-21, with PLG3 selected)**. Version 1.30 (witness in the name): the name does not
  change with CC, SysEx or with the `info` queries; the name is only set at install time, so it does not work as a live
  witness. Version 1.31 (witness in the NAME0 area of `read_diag.py`): after installing and after a SysEx, the decoded
  bytes are still the preset's name (`String 1 [Str1]`); neither the `DEADBEEF` mark nor the counters appear. No
  result: it is not known whether the hook runs or whether the write does not reach the area that RQ1 reads.
