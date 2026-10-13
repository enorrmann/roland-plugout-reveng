# PLUG-OUT USB install protocol

Extracted from the code of the VST3 plugins (`JUNO-60.vst3`, `JX-3P.vst3`,
`SH-101.vst3`, x86-64). All three DLLs contain the same routines; the addresses
below are from `JUNO-60.vst3` (image base `0x180000000`).

| Function | Address |
|---|---|
| Full install (PLUG-OUT button) | `0x1803547c0` |
| Device and slot query | `0x180352b50` |
| `Code*.Dat` file selection | `0x180353480` |
| Container load and decryption | `0x180356320` |
| Send + wait for reply (with retries) | `0x180356f00` |
| Send without waiting (checks for an immediate error) | `0x180357100` |
| Reply matching | `0x1802950a0` |
| MIDI output queue (pacing, `midiOutLongMsg`) | `0x1804364c0` |
| Slot menu "PLUG-OUT %d: %s" | `0x180327f00` / `0x180353830` |

## 1. Port and messages

* Port: the synth's second USB MIDI port, **`SYSTEM-8 CTRL`**
  (`Script/Script.xml`, `<midiOut><name>`). On Linux usually `hw:N,0,1`.
* Every message is a Roland SysEx:
  `F0 41 <dev> 00 00 7E <cmd> <a1 a2 a3 a4> <data> <checksum> F7`
  * `dev` = 0x10 by default (device ID 17).
  * `00 00 7E` = PLUG-OUT model ID.
  * `cmd` = `12` (DT1). Queries are also DT1 with one data byte.
  * Roland checksum: `(-(sum of address + data)) & 0x7F`.
* The plugin handles addresses as 28-bit integers: `70 10 00 00` = `0x0E040000`.
* **The device answers on the same address with bit 21 set**
  (`70 ..` → `71 ..`). A reply is accepted when the address and the number of
  data bytes match; the command byte is not checked.

## 2. Device query (`0x180352b50`)

Each query is a DT1 with one data byte (`00` unless stated):

| Sent | Reply | Content |
|---|---|---|
| `70 00 02 00` | `71 00 02 00`, 1 byte | type: 0 SYSTEM-1, 1 SYSTEM-1m, 2 SYSTEM-8 |
| `70 00 01 00` [`00`] | `71 00 01 00`, 48 bytes | bytes 0-3: firmware version (one nibble per byte) |
| `70 00 02 01` | 1 byte | multiplied by the next value: number of patches |
| `70 00 02 02` | 1 byte | |
| `70 00 02 03` | 1 byte | number of slots |
| `70 00 01 00` [`i`] | 48 bytes | slot *i*: bytes 8-11 model ID, 12-15 version (nibbles), 16-47 name (16 characters, two nibbles each) |

Minimum firmware: SYSTEM-1/1m ≥ 1.30, SYSTEM-8 ≥ 1.31 (`0x130`/`0x131`).

On SYSTEM-8 the plugin also reads `00 00 68 07`, `00 00 68 16` and
`00 00 68 13..15` with RQ1; they only feed informational fields, not the transfer.

Slot state relative to the container (`0x1803532b6`):

* same model ID and name → *same version* or *other version*;
* model ID `0xFFFF` → empty; another model ID → occupied by another PLUG-OUT.

The plugin refuses if the same model is already in **another** slot, does not
resend the firmware if the slot already holds the same version, and asks for
confirmation if the slot holds another PLUG-OUT.

Slots: slot 0 is the synth itself; the menu offers `PLUG-OUT 1..N-1`.

## 3. File selection (`0x180353480`)

| Device type | File |
|---|---|
| SYSTEM-1 / SYSTEM-1m | `Script/Code1.Dat` |
| SYSTEM-8 | `Script/Code8_<slot>.Dat` |

`Code8_1/2/3.Dat` are **separate builds for each slot**: use the one for the
target slot. `SH-101/Script/Code.Dat` is not used by the plugin.
`JX-3P/Script/Code1.Dat` carries the model name `SYSTEM-8` (id 0x05), so the
JX-3P plugin would reject it: the JX-3P only installs on a SYSTEM-8.

## 4. `Code*.Dat` container

Cipher: chained XOR over 32-bit LE words,
`plain[i] = cipher[i] ^ cipher[i-1]`, with `cipher[-1] = 0xBACADABE` (`0x1803564a5`).

Decrypted header:

| Offset | Field |
|---|---|
| `0x00` | u32 `0x80` |
| `0x04` | `"PLUGOUT_SYS_INF"` |
| `0x14` | model name (16 bytes) |
| `0x24` | u16 model ID (JUNO-60 `0x0A`, JX-3P `0x08`, SH-101 `0x01`) |
| `0x26` | u16 version (`%X.%02X`) |
| `0x30` | u32 offset, u32 size → **START** message |
| `0x40` | u32 offset, u32 size → **END** message |
| `0x50` | u32 offset, u32 stride, u32 **count** → data records |

Each record is `<u32 length><complete SysEx message>`. Data records are at
`offset + i * stride` (`stride = 0x260`), `i = 0 .. count-1`
(4096 in `Code8_*`, 2048 in `Code1`).

Data record types — **all sent by the host, in order**:

| Address | Size | What |
|---|---|---|
| `70 30 00 09` | 599 bytes | firmware block: 586 bytes 7-in-8 = 512 bytes |
| `70 30 01 09` | 14 bytes | filler record (data `01`) |

There are two regions (two runs of blocks + filler), each with a
`"SYSTEM-8:Plugout…"` header; the filler pads each region to half of the table.

> The earlier attempt treated `70 30 01 09` as device ACKs and skipped them, and
> sent `70 10 00 01` at the start. Both were wrong: the fillers are sent and
> `70 10 00 01` is the final message.

## 5. Install sequence (`0x1803547c0`)

Session parameters: 5 s timeout, 3 attempts (each attempt resends the message).

1. **START**: the container's START message with its data byte replaced by
   `slot | flags` (checksum recomputed). `flags` = `0x40` when the plugin is not
   authorized by Roland Cloud, `0x00` when it is.
   Expected reply `71 10 00 00`, 1 byte, must be `00`.
2. **Data**: for `i = 0 .. count-1`:
   * if `i % 32 == 31`: send `70 30 03 00` [`00`] and wait for `71 30 03 00`
     with 4 bytes (flow control; the content is not checked);
   * send record `i` exactly as stored;
   * if `71 30 00 00` (1 byte) arrives, the device rejected the record → abort.
3. **END**: the container's END message unchanged (4 data bytes, probably a
   checksum of the image). 20 s timeout, 3 attempts.
   Reply `71 10 00 01`, 1 byte, must be `00`.

Pacing: the plugin waits `10 ms + length × 0.32 ms` between messages (setting
`10010` = 10 ms + wire time at 31250 baud). By default the tools add **no**
delays: the SYSTEM-8 sets the pace with its flow-control replies, and the port
write blocks while the USB buffer is full (`--gap-ms 10 --wire-pacing`
reproduces the plugin's pacing).

## 6. Patch bank (`0x1803551a5`, `0x180355af0`)

After END the plugin writes its patch bank into the slot (DT1, no replies):

1. `70 20 01 01` [`n`] with `n` = (`70 00 02 01`) × (`70 00 02 02`) = 64.
2. For each patch `p = 0..n-1` and each struct (`usrX`, `usr2X`, `usr3X` on
   SYSTEM-8; only `usrX` on SYSTEM-1), one DT1 per leaf struct of `Script.xml`,
   in order, at `base(slot, p, struct) + leaf offset`.
3. `70 20 01 02` [`1` if the slot already held the same version, else `0`].

If the slot already holds the same version, the plugin skips the firmware and
sends only the patches (after confirmation). With another version it asks
before sending them.

### Reading patch memory (RQ1, verified 2026-10-13)

The plugin never reads patches, but the device answers the standard read
request: `F0 41 <dev> 00 00 7E 11 <addr:4> <size:4> <sum> F7` (size as 4 x 7-bit
bytes) is answered by a `DT1` (`12`) at the same address with `size` data
bytes (nibble-encoded leaves, e.g. `NAME0`: 2 nibbles per character).
`tools/patch_read.py --slot 3 --patches 0-63` lists the 64 names. Up to 128
bytes per request answered; a 70-byte request returned 64 (not understood).

### Patch memory (`Script/Script.xml`)

| Struct | Type | Leaves | Bytes |
|---|---|---|---|
| `fm.usrX` | `patch` | 14 (LFO, OSC1, …, NAME0, PAT_NAME1-3) | 640 |
| `fm.usr2X` | `patch2` | 17 | 3975 |
| `fm.usr3X` | `patch3` | 66 (sequencer) | 15580 |

Values are `intNxM`: N bytes of M bits each, most significant first
(`int2x4` = 2 nibbles, `int8x4` = 8 nibbles, `int1x7` = 1 byte); `stringNx4` =
2 nibbles per character. A leaf's memory is its values concatenated (or placed
as given by `<address>`/`<size>`).

### Addresses (`0x18036d270`)

28-bit integers (7 bits per byte). For `usrX`, with `b = p / 8`, `i = p % 8`:

* slot 0/1: `((b << 7) + i) << 14` + `0x4000000` (slot 0) / `0x8000000` (slot 1)
* slot 2/3: `((b << 4) + i) << 14` + `0x1C00000` (slot 2) / `0x1E00000` (slot 3)

For `usr2X`/`usr3X` (struct `k` = 1/2): `(p << 18) + T[slot + (k-1)*4]`, with
`T = 0x5000000, 0x6000000, 0x7000000, 0xD000000, 0x9000000, 0xA000000, 0xB000000, 0xC000000`.

### Bank files (`Patch/*.bin`, `KoaBankFile`)

* 16 bytes `KoaBankFile0000N` + tag (`PG-JU60`, `PG-JX3P`, `PG-SH101`) + 64 records.
* Record: name (16) + header (12 bytes in v3, 27 in v2) + leaf memory
  concatenated in `Script.xml` order.
* v3 (JUNO-60, JX-3P) holds `patch`+`patch2`+`patch3` (20 223 bytes per record);
  v2 (SH-101) only `patch` (683 bytes); `patch2`/`patch3` take the `Script.xml`
  defaults.
* Verified: all 9 banks have exactly these sizes, every byte is 7-bit, and
  `patch2`/`patch3` match the defaults 98-100 %.

The plugin sends the bank loaded in its library; the tool defaults to the first
file in `Patch/` (`--bank` to choose another).
