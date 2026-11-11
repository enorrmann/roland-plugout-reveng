# Volume V — Chapter 7: SYSTEM-1 USB MIDI on a Linux Host

> **Scope:** SYSTEM-1 (USB id `0582:017d`), Linux with the in-kernel `snd-usb-audio` driver.

On Linux the SYSTEM-1 plays notes late and some notes get stuck. This happens with notes sent over USB MIDI and with
notes played on the device's own keyboard. The same device works correctly on Windows with Roland's driver (SYSTEM-1
driver 1.0.1, `RDWM1146.SYS`). The cause is that the host does not read the device's USB MIDI input. The fix is to keep
that input open at all times. This chapter gives the measured cause, the fix as a udev rule plus a systemd unit, how to
install it, and how it affects the project tools.

## 1. Cause

**The SYSTEM-1 stalls when the host does not drain its USB MIDI IN endpoint.** The device echoes everything it plays to
the host: its keyboard notes and, for notes received over USB, a copy of the received data. In the measurement of §3,
969 bytes came back for 24 sent notes. Linux reads a USB MIDI IN endpoint only while some client has the rawmidi input
substream open. That client can be an `amidi` dump, an application, or an ALSA sequencer subscription to the device's
output port. With no such client, nothing is read and the device's transmit queue fills. The device firmware then
waits for that queue to drain. That wait delays its note handling: note-ons start up to ~190 ms late, and note-offs
are applied late, which is heard as stuck or overlong notes. Roland's Windows driver always polls the IN endpoint, so
the stall never happens there.

The USB transfer mode does not cause the problem. The device has two altsettings on its MIDI interface (interface 3):

| Altsetting | Kernel name (`MIDI Input Mode` mixer control) | Endpoints |
|---|---|---|
| 0 | High Load | EP3 OUT bulk, EP4 IN bulk, 512 B |
| 1 | Light Load (**kernel default**) | EP3 OUT interrupt, EP5 IN interrupt, 512 B, 1 ms interval |

Changing the mode does not fix the problem (§3). Keep the default, Light Load.

## 2. Fix: keep the input subscribed

Connect both SYSTEM-1 sequencer ports to `Midi Through` (module `snd-seq-dummy`). The subscription makes the kernel
open the rawmidi input and read the IN endpoint continuously:

```sh
aconnect SYSTEM-1:0 "Midi Through:0"
aconnect SYSTEM-1:1 "Midi Through:0"
```

The subscription disappears when the device is unplugged or the host reboots. The two files below recreate it each time
the device appears. A udev rule cannot run `aconnect` itself, for two reasons: the sequencer port is created a moment
after the sound card, and udev kills long-running `RUN` programs. So the rule only asks systemd to start a one-shot unit,
and the unit retries for up to 10 s.

`/etc/udev/rules.d/99-system1-midi.rules`:

```
# Roland SYSTEM-1 (0582:017d): keep its USB MIDI input always read, otherwise the device
# stalls (late notes, stuck notes) when the host does not drain it.
ACTION=="add", SUBSYSTEM=="sound", KERNEL=="controlC*", ATTRS{idVendor}=="0582", ATTRS{idProduct}=="017d", TAG+="systemd", ENV{SYSTEMD_WANTS}+="system1-midi-drain.service"
```

`/etc/systemd/system/system1-midi-drain.service`:

```ini
[Unit]
Description=Keep Roland SYSTEM-1 USB MIDI input open (subscribe to Midi Through)

[Service]
Type=oneshot
ExecStart=/bin/sh -c 'for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do aconnect SYSTEM-1:0 "Midi Through:0" 2>/dev/null && { aconnect SYSTEM-1:1 "Midi Through:0" 2>/dev/null; exit 0; }; sleep 0.5; done; exit 1'
```

The loop lists its counter values instead of using `$(seq 20)` because systemd expands `$` in `ExecStart`.

### 2.1 Install

```sh
sudo install -m 644 99-system1-midi.rules /etc/udev/rules.d/
sudo install -m 644 system1-midi-drain.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo udevadm control --reload
```

Requirements: `alsa-utils` (`aconnect`), and the `snd-seq-dummy` module loaded, which provides `Midi Through`. Most
distributions load it by default. To load it at every boot, run
`echo snd-seq-dummy | sudo tee /etc/modules-load.d/snd-seq-dummy.conf`.

### 2.2 Check

Unplug and replug the SYSTEM-1, then run `aconnect -l`. Both ports must show `Connecting To: 14:0` (the client number
of `Midi Through` can differ):

```
client 28: 'SYSTEM-1' [type=kernel,card=3]
    0 'SYSTEM-1 MIDI 1 '
	Connecting To: 14:0
    1 'SYSTEM-1 MIDI 2 '
	Connecting To: 14:0
```

If the ports are not connected, check the unit with `journalctl -u system1-midi-drain`.

### 2.3 Side effects

- Everything the SYSTEM-1 sends reaches `Midi Through`, and with it every client that listens there.
- **A tool that opens the SYSTEM-1 rawmidi *input* fails with `Device or resource busy`**, because the sequencer already
  holds the input. Examples are `amidi -p hw:X,0,0 -d` and `tools/midi_io.py` (`snd_rawmidi_open` for input and
  output), which `tools/plugout.py`, `tools/plugout_tool.py` and `tools/test_plugout.py` use. Output-only access
  still works: `amidi -S`, `tools/s1_capture.py`, and `tools/wt_load.py`, which writes `/dev/snd/midiCxDy`.
  **`tools/midi_io.py` handles it by itself** (`AlsaPort`, 2026-11-10, verified with `plugout_tool.py info` on the
  SYSTEM-1): when `snd_rawmidi_open` returns `EBUSY` it reads `aconnect -l`, disconnects every sequencer
  subscription of the card's kernel client (both directions), opens the port again and reconnects the same pairs in
  `close()`, with an `atexit` handler as a fallback. Both steps print a `midi_io:` line to stderr. A `kill -9`
  skips the restore. For other tools, such as `amidi -d`, remove the subscription by hand with
  `aconnect -d SYSTEM-1:0 "Midi Through:0"` and `aconnect -d SYSTEM-1:1 "Midi Through:0"`. While that tool runs it
  reads the input itself, so the device does not stall. Afterwards, restore the subscription with `aconnect` (§2) or
  by replugging the device.
- The rule matches only the SYSTEM-1's USB id. Other Roland devices, including the SYSTEM-8, are not affected.

## 3. Measurement

Method:

- **Device and patch:** SYSTEM-1 with the stock engine and the INIT patch, receiving on MIDI channel 5.
- **Notes sent:** 24 notes written directly to `/dev/snd/midiC3D0`, with no sequencer or PipeWire in the path. One
  note every 300 ms; each note is held for 120 ms.
- **Recording:** the device's USB audio, recorded with `arecord -D hw:3,0 -f S32_LE -r 96000 -c 2`.
- **Analysis:** the audio was reduced to an envelope of 1 ms blocks, and each note's sound was located by a threshold
  at 0.3 of full scale. "Late" means the time from a MIDI write to the matching start or end of sound, minus the time
  measured for the first note.
- **"Input read":** a thread kept reading the same rawmidi file descriptor (opened `O_RDWR`).

| Mode | Input read | Note-on late, max | Notes > 10 ms late (on / off) |
|---|---|---|---|
| Light Load | no | 127 ms; 174 ms on a repeat run | 3 / 4; 4 / 5 on a repeat run |
| Light Load | **yes** | **2 ms** (3 runs) | **0 / 0** (3 runs) |
| High Load | no | 139 ms | 3 / 4 |
| High Load | yes | 187 ms | 4 / 5 |

Light Load with the input read is on time within the 1 ms measurement resolution in every run. High Load stays wrong
even when the input is read. With the subscription of §2 in place, the user confirmed that playing the SYSTEM-1's own
keyboard no longer gives late or stuck notes (2026-11-10).

Not verified:

- **Why High Load still stalls when the input is read.** The likely reason is that the device sends its echo on the
  interrupt endpoint EP5, while High Load reads the bulk endpoint EP4. A capture of USB traffic would decide this.
- **The persistent setup after a replug.** The udev rule and the unit of §2 were written and the unit was checked with
  `systemd-analyze verify`. They had not been tested with a real replug when this chapter was written.
