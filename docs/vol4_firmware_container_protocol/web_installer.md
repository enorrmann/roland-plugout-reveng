# Volume IV — Chapter 5: Web Installer of PLUG-OUTs (`web/`)

Page that installs a `Code*.Dat` into a slot of the SYSTEM-1 / SYSTEM-1m / SYSTEM-8 from the browser, through Web MIDI
with SysEx. It sends exactly the same messages, in the same order, as `tools/plugout_tool.py install … --force
--no-patches` ([Vol. IV ch. 4](usb_sysex_protocol_spec.md) §2 and §5), with a limit of bytes per second that replaces the tool's
blocking write (§5). Nothing has to be installed: it is static HTML + JavaScript, without dependencies.

State: 2026-10-23.
* Tests without hardware: [sim], all pass (§6).
* **Real device: [open]**, nothing has been installed from the page yet.

Tags: **[measured]** = tested on the device, **[sim]** = tests with a simulated synthesizer, **[open]**.

## 1. Use

```sh
python3 -m http.server 8000 --bind 127.0.0.1 --directory web     # from the root of the repository
# open http://localhost:8000/ in Chrome / Edge / Brave (Chromium)
```

The page is a wizard: one step is seen at a time, with a step bar at the top and *Back* / *Next* at the bottom.
*Next* is enabled when the step is complete; next to it, it says what is missing. With MIDI granted it moves to step 2
by itself.

1. **Web MIDI**: the browser asks for permission when the page opens. SysEx must be accepted too.
   * Web MIDI only works on a secure origin (`http://localhost` or `https://`). Opening `index.html` as `file://` or
     from a network IP does not work.
2. **Device**: the page finds the control port (§4) and asks for type, firmware and content of the slots.
   * The user does not choose a port: the box only shows *Find synth* and the detected port, marked "(control port)".
   * The list of ports is folded in *Advanced: another MIDI port*. There the control port carries "(control port — use
     this one)" and the other one of the synthesizer "(not the control port)".
   * If the port in use is not the control one, the box warns and unfolds *Advanced*.
3. **Slot**: it comes before the file because on the SYSTEM-8 the slot decides the file name.
   * SYSTEM-1 / 1m (type 0 / 1): a single PLUG-OUT slot. There is no selector: always slot 1, and the table only shows
     slots 0 and 1.
   * SYSTEM-8: suggests the first empty slot, otherwise 1.
   * The table shows what is in each slot.
4. **File**: button *Choose file…* or drag the file onto the card. Only the exact name (case included) that the
   plug-in would choose is accepted ([Vol. IV ch. 4](usb_sysex_protocol_spec.md) §3):
   * SYSTEM-1 / 1m: `Code1.Dat` (from `plugins/<NAME>/Script/`);
   * SYSTEM-8: `Code8_<slot>.Dat` of the slot chosen in step 3.
   * Another name (`Code1 (1).Dat`, `code1.dat`, `Code8_2.Dat` for slot 1) is rejected without reading the file. The
     browser's picker only filters by extension: no API accepts exact names.
     * With `showOpenFilePicker` (Chrome, Edge): type `.Dat` with `excludeAcceptAllOption`, without "all files".
     * Without it (Brave has it disabled, `brave://flags/#file-system-access-api`): `<input type=file accept=".Dat">`.
       The GTK dialog shows the filter as "custom file", but lets it be changed and any file chosen [seen by the user,
       2026-10-24]; the name is checked afterwards.
   * If the slot of a SYSTEM-8 is changed after choosing the file, the file is discarded.
   * The file does not leave the browser. **There is no default file**: it is always chosen.
5. **Confirm and install** (a single step, the last one; there is no *Next*):
   * shows what is in the slot and what is lost;
   * *Install* is only enabled with the box ticked and without blocks (§3);
   * the installation: START, the data records with the flow query every 32, and END. The box stays locked while it
     lasts; after a failure it is unticked and has to be ticked again.
   * With all the records sent the bar is full and the page shows *All data sent. Waiting for the synth to confirm…*
     while it waits for the answer to END (up to 20 s).
   * At the end it says which PLG to choose on the panel. *Install another* asks the device again (the slot table
     changed) and continues at step 2.

On the command line it is equivalent to `python3 tools/plugout_tool.py install plugins/<NAME> --slot N --force
--no-patches -y`.

## 2. Files

| file | what it is |
|---|---|
| `web/index.html` | the page: step bar, 5 cards (one visible), *Back* / *Next*, styles, no external scripts |
| `web/plugout-protocol.js` | the protocol, without DOM: container, messages, `Port` (queries, installation), Web MIDI. Plain script (no modules, works with `file://` for reading it) that exposes `globalThis.Plugout`. It also works in Node (`module.exports`) |
| `web/app.js` | the logic of the page: steps, blocks, warnings, progress |
| `web/test/test-protocol.js` | test of the library in Node against `tools/plugout.py` (§6.1) |
| `web/test/page-test.html`, `page-test.js`, `run-page-test.sh` | test of the whole page in headless Chrome with simulated Web MIDI (§6.2) |

`plugout-protocol.js` replicates these Python functions:

| JavaScript (`Plugout.*`) | Python | what it does |
|---|---|---|
| `decodeContainer`, `Container` | `plugout.load`, `Container` | decryption (chained XOR, IV `0xBACADABE`), header, START / END / records |
| `Container.regions`, `.target` | `Container.image_regions()` | regions of the firmware and target device (§3) |
| `unpack7in8` | `unpack_7in8` | 7-in-8 data → bytes |
| `buildMessage`, `withData`, `withDeviceId`, `checksum` | `build`, `with_data`, `with_device_id` | Roland DT1 messages |
| `Port.request`, `Port.write` | `Session.request`, `Session.write` | send and wait for an answer at `address \| bit 21`. Up to 3 attempts of 5 s (END 20 s); drops old answers before each query |
| `Port.deviceInfo` | `Session.device_info` | type, firmware, patches per slot, number of slots, each slot |
| `Port.install` | `Session.install` | START (`slot \| flags`), records with `70 30 03 00` before each record `i % 32 == 31`, `71 30 00 00` = rejection → abort, END |
| `planInstall`, `slotState` | `SlotInfo.state_for` + checks of `cmd_install` | state of the target slot and blocks |
| `findDevicePorts` | `midi_io.pick_port` | choice of the port (§4) |

## 3. Checks

| case | the page | `plugout_tool.py` | Roland's plug-in |
|---|---|---|---|
| device firmware < 1.30 (SYSTEM-1/1m) / 1.31 (SYSTEM-8) | **blocks** | exits with an error | rejects |
| the file is for another device (§3.1) | **blocks** | does not check it | chooses the file by type ([Vol. IV ch. 4](usb_sysex_protocol_spec.md) §3) |
| file name ≠ `Code1.Dat` (SYSTEM-1) / `Code8_<slot>.Dat` (SYSTEM-8) | **rejects the file** (step 4) | does not check it | chooses `Code1` / `Code8_<slot>` |
| regions without a `SYSTEM-x:Plugout` header | warns | — | — |
| the same PLUG-OUT in another slot | says nothing (installs) | refuses without `--force` | refuses |
| same version in the target slot | resends it without a warning | skips it without `--force` | skips it |
| another PLUG-OUT in the target slot | warns: it is overwritten | warns | asks for confirmation |
| patch bank | **not sent** | sent unless `--no-patches` | sent |

For our builds it is better to always resend: a changed build keeps the version number. That is why the page behaves
like `--force`.

The patch bank stays out: it needs `Script.xml` and the plug-in's `Patch/*.bin` ([Vol. IV ch. 4](usb_sysex_protocol_spec.md) §6), and
our own PLUG-OUTs do not use it. For Roland's: `plugout_tool.py install <plug-in>` with patches.

### 3.1 Target device of a `Code*.Dat`

Each region of the firmware (each stretch of `70 30 00 09` records) starts with 16 ASCII bytes `SYSTEM-1:Plugout` or
`SYSTEM-8:Plugout`, followed by the date and the version of Roland's tool ([Vol. IV ch. 4](usb_sysex_protocol_spec.md) §4). The page
reads the first record of each region (`unpack7in8`, first 16 bytes):
* if all the regions name the same device, that is `Container.target`;
* otherwise `null`.

A SYSTEM-1m (type 1) accepts `SYSTEM-1` images. Seen in:

| file | model (header) | regions |
|---|---|---|
| `plugins/C-S1SY13F/Script/Code1.Dat` (ours) | `C-S1SY13F` | 2 × `SYSTEM-1:Plugout` |
| `plugins/JX-3P/Script/Code8_1.Dat` | `JX-3P` | 2 × `SYSTEM-8:Plugout` |
| `plugins/JX-3P/Script/Code1.Dat` | `SYSTEM-8` (sic) | 2 × `SYSTEM-1:Plugout` |
| `plugins/C-BMAP/Script/Code8_1.Dat` (ours) | `C-BMAP` | `SYSTEM-8:Plugout` |

The model name of the header does not tell the device: it is the name of the PLUG-OUT, and in `JX-3P/Code1.Dat` it says
`SYSTEM-8` although the regions are SYSTEM-1 ones.

## 4. MIDI port

* **Choice** (like `midi_io.pick_port`, but by name and not by order):
  * among the ports whose name contains `SYSTEM-8` / `SYSTEM-1`, the one with `CTRL` wins;
  * if none, the one with a control name: `MIDI 2` (Linux, = `hw:N,0,1`), `MIDIIN2` / `MIDIOUT2` (Windows);
  * if none, the second in alphabetical order;
  * if there is only one, that one.
  * The input is the one with the same name.
  * Another port can be chosen in the list; a port that does not name the synthesizer is only queried if it is chosen
    by hand.
* **Automatic queries**: only on a port that names the synthesizer, when the page opens or when a port connects. Up
  to 5 attempts, one every 2 s; after that *Find synth* has to be used.
* **Synthesizer connected with the page already open** (user's mistake, 2026-10-24): without the device, the only port
  was `Midi Through Port-0` and it stayed chosen; when the SYSTEM appeared, the list was rebuilt keeping that port, so
  the synthesizer was never asked (and *Find synth* stayed at "Asking Midi Through Port-0…"). Current rule when
  rebuilding the list:
  * the port chosen by hand in *Advanced*, or the port where the device already answered, is kept;
  * otherwise the best candidate (the synthesizer's control port) is taken again.
  * Only one round of queries at a time. If the ports change during a round, at the end of that round the device is
    asked again with the attempts reset.
* **`statechange`**: Chrome also fires it when a port is only opened or closed (`connection` changes), and that
  happens in every round of queries. The page compares the list of ports (`id` + `state`) and only reacts if it
  changed: a new port, or one that went `disconnected`. If the device's port disappears, step 2 goes to
  *Disconnected*.
* **Release**: after each round of queries and after the installation, `Port.close()` closes the input and the output
  (`MIDIPort.close()`), so that another program can open the device.
  * [measured, 2026-10-23] With the previous version of the page open in Brave, `plugout_tool.py install` failed with
    `cannot open hw:3,0,1: Device or resource busy`: the ALSA clients `Chrome (input)` / `Chrome (output)` were
    subscribed to the two ports of the SYSTEM-1 (`/proc/asound/seq/clients`).
  * [open] Whether `close()` is enough for Chrome to release the ALSA port. Until that is checked: **close the tab
    before using the command-line tools** (rule of a single MIDI client).
* **Listening on all the inputs of the synthesizer**: `Port(output, [inputs])` sends through the chosen output and
  listens to the input of the same name and also to all the other inputs whose name names the synthesizer. The
  answers are recognised by address and size, so it does not matter through which port they come back.
* **[measured by the user, 2026-10-23] port 1**: installing through `SYSTEM-1 MIDI 1` from Brave (Linux):
  * the queries (§2 of [Vol. IV ch. 4](usb_sysex_protocol_spec.md)) and START get an answer, START duplicated;
  * the flow query `70 30 03 00` only gets an answer after the third send (2 resends of 5 s, ≈ 10 s every 32
    records).
  * The page had chosen port 1 because Brave lists `MIDI 2` before `MIDI 1` and the rule took "the second" of the
    list. Now it chooses by name (`isControlName`: `CTRL`, `MIDI 2`, `MIDIIN2` / `MIDIOUT2`), and warns in step 2 if
    the chosen port is not the control one.
  * [open] Whether the device answers through the other port (out through 1, back through 2) or whether through port
    1 it does not answer in time. With listening on all inputs, both cases are covered.
* **`Port` receives the `MIDIPort`s**: `listOutputs` / `listInputs` return descriptors `{id, name, manufacturer,
  state, port}`, where `port` is the browser's `MIDIPort`.

## 5. Sending pace

**Same sequence as `plugout_tool.py`**: START, one flow query `70 30 03 00` before each record `i % 32 == 31` (5 s, 3
attempts), END. The only difference is the **pace**: the page limits the bytes per second.

* **Why it is needed**:
  * `plugout_tool.py` sends without pauses because its write to `rawmidi` is blocking. While the USB buffer is full,
    the kernel holds it, and so the sending goes at the device's pace.
  * `MIDIOutput.send` never blocks and Web MIDI does not report the state of the buffer. Without a limit, the records
    enter the browser / ALSA queue all at once.
* **Limit**: `WEB_BYTES_PER_SECOND` = 32 000 B/s by default; it is changed with `?rate=<bytes per second>` in the
  page's URL.
  * `Port({bytesPerSecond})` sends right away while it is less than `RATE_SLACK_MS` (20 ms) ahead of that pace, and
    waits afterwards. The average holds even if the browser's timers are coarse.
  * Approximate duration: C-S1SY13F (60 KB on the wire) ≈ 2 s; SH-101 (≈ 355 KB) ≈ 11 s.
* **Pace of the tool** [measured, log of the installation of C-S1SY13F of 2026-10-23, 1 s resolution]:
  * 2048 records (60 KB) in ≈ 3 s;
  * the 40 blocks of region 2 (24 KB) went in in less than 1 s, that is ≥ 25–40 KB/s.
  * The device's ceiling is not measured [open].
* **Waiting for answers**: by event (`_wait`): the promise resolves with the MIDI message itself. The first version
  polled every 5 ms with `setTimeout`, which the browser throttles (≥ 1 s in a background tab).
* **Diagnostic log** (*Show the MIDI traffic*), at record 32, every 512 and at the end:
  * accumulated time;
  * mean and maximum wait of the flow answers;
  * resends;
  * pace used.
* `new Port(out, in, {gap: 10, wirePacing: true})` reproduces the plug-in's pace (10 ms + line time at 31 250 baud,
  [Vol. IV ch. 4](usb_sysex_protocol_spec.md) §5). That is ≈ 7 min for the SH-101; the page does not use it.

### 5.1 Attempts (2026-10-23, Brave on Linux, SYSTEM-1, SH-101 firmware, measured by the user)

| version | result |
|---|---|
| no limit, waiting by polling, port `MIDI 1` | each flow query answered only at the 3rd send (2 × 5 s): ≈ 10 s every 32 records |
| no limit, waiting by event, port `MIDI 2` | the 1st flow query without an answer and the resend answered in 6 ms: 5 s every 32 records |
| no limit, resend of the flow query at 250 ms | transfer in 9.9 s, 20 resends; **END answered 3**: data was lost. Therefore the queue did not only lose the query |
| flow query every 8 records | written, never installed: replaced by the decision to keep the tool's sequence |
| **tool's sequence + 32 KB/s** | current; **untested** |

## 6. Tests

```sh
node web/test/test-protocol.js [Code*.Dat ...]        # library, ≈ 3 s
sh web/test/run-page-test.sh [/path/from/the/root/Code1.Dat]   # whole page, ≈ 2 s (headless Chrome)
```

### 6.1 `test-protocol.js` (Node)

It uses a simulated synthesizer that answers like the device: type, 48-byte slots, 1 byte, 4-byte flow, START / END,
record rejection. It checks:
* messages, checksum, `withDeviceId`, `unpack7in8`, decoding of slots, versions;
* choice of the port: Linux names, `CTRL`, no foreign port without `allowAny`; the lists carry the `MIDIPort`;
* query engine:
  * an old answer is discarded;
  * foreign SysEx does not cause resends;
  * without an answer: 3 attempts and a `timeout` error;
  * complete `deviceInfo`;
  * `close()` closes both ports;
* installation:
  * a rejection of START sends nothing else;
  * a rejected record cuts there, without END;
  * the device id reaches START, each record and END;
* `planInstall`: overwriting, file for another device, SYSTEM-1m, same model in another slot, old firmware;
* for each `Code*.Dat` (by default, a `C-S1SY*/Code1.Dat` and a `Code8_1.Dat` from `plugins/`):
  * header, the 2048 / 4096 records, START / END and regions identical to `tools/plugout.py --json`;
  * the complete sequence of the installation, message by message.

`tools/plugout.py --json FILE` prints what the Python side reads: `model`, `modelId`, `version`, `dataCount`,
`start`, `end`, `records` (list of hex) and `regions` (16 header bytes of each region).

### 6.2 `run-page-test.sh` (headless Chrome)

`run-page-test.sh` does this:
1. Starts `python3 -m http.server` on `127.0.0.1:8765` (variable `PORT`) over the root of the repository.
2. Opens `web/test/page-test.html` in headless Chrome with a temporary profile. The browser is chosen with `CHROME=`;
   by default `google-chrome` or `chromium`.
3. Reads the verdict from the DOM. Exit code 0 = `PASS`.

The test page loads `index.html` in an iframe, with `navigator.requestMIDIAccess` replaced by a simulated
`MIDIAccess`. **It opens no real MIDI port.** Scenarios:

1. **SYSTEM-1** (`SYSTEM-1 MIDI 1` / `MIDI 2`, slot 1 with `C-S1SY13`):
   * finds the device by itself on `MIDI 2` and closes the ports after asking;
   * only step 2 is visible; *Next* leads to the slot (step 3) before the file;
   * there is no slot selector: slot 1;
   * step 4 asks for `Code1.Dat`; without a file *Next* is blocked; `Code1 (1).Dat` is rejected;
   * with `Code1.Dat` it shows the model and the target device;
   * step 5 is the last one (no *Next*), warns about the overwrite, and *Install* is only enabled with the box;
   * the installation finishes, and what is sent matches message by message START (slot 1) + records + flow + END;
   * closes the ports and says "PLG1".
2. **SYSTEM-8, slot 1, with a SYSTEM-1 build called `Code8_1.Dat`**: step 4 asks for `Code8_1.Dat`; step 5 stays
   blocked (target device), *Install* disabled, and no START is sent.
   Scenarios 1 and 2 rewritten for the wizard on 2026-10-24, **not run yet**.
3. **Device that does not answer** (the simulated one fires `statechange` when ports open and close, like Chrome): at
   most 5 rounds × 3 attempts, and then the page waits for the button. Scenario written on 2026-10-23, **not run
   yet**.

The waits are measured in Chrome's virtual time (`--virtual-time-budget`); that is why the limits are long although
the test takes 1–2 real seconds.

## 7. Open

* First real installation from the page (SYSTEM-1, `C-S1SY13F`), and then on the SYSTEM-8.
* Whether `MIDIPort.close()` releases the ALSA port while the tab is still open (§4).
* In the browser, not yet tested: the reaction only to changes of the port list (§4) and the pace limit of 32 KB/s
  (§5, §5.1).
* `web/Code1.Dat` (image of Roland's SH-101) is not used by the page; it must not be distributed with it.
