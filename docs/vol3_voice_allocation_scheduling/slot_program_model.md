# Volume III — Chapter 3: DSP Program Memory — Main Program, Slots and Sections (SYSTEM-8 and SYSTEM-1)

> **Scope:** both: SYSTEM-8 slots and SYSTEM-1 sections, each in its own section.

State: 2026-10-22 (with the dated additions of 2026-10-28). It gathers what is known about how the program memory of
a DSP is divided, who runs which part, how much fits in each and which usage variants are possible. It serves to decide
where to put an own program. Detail of the ISA and of the time: [Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md); loading from the
ARM: [Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md); own PLUG-OUTs: [Vol. VI ch. 1](../vol6_reference_designs_applications/c_v4_system8_reference.md) (SYSTEM-8),
[examples/system_1/README.md](../../examples/system_1/README.md) (SYSTEM-1).

Legend: **[measured]** on the device · **[code]** seen in the binaries · **[inferred]** · **[open]** (untested).

## 1. What both devices have in common

* Program memory is a stream of 16-bit words; each 32-bit word carries two, the upper half first. The slot/section
  addresses are given **in bytes** [code].
* In each sample the DSP runs **several programs**: the **main** program and then the **slot programs** (SYSTEM-8) or
  **sections** (SYSTEM-1) that are enabled, in order. Each one ends with `0a02 ff70 0b02 00f8 0001`; `0b02 00f8` is
  the return to the scheduler [measured on the SYSTEM-8; the same ending on the SYSTEM-1, which works: C-S1V4].
* All programs of a DSP **share r0–r7, flags and hidden registers**; nothing is reset between programs or between
  samples [measured, SYSTEM-8].
* The main program runs **before** the slots: it writes an input into bank 1 and the slot reads it in the same
  sample; the slot's output comes back to the main program one sample later (ring) [measured, C-V4].
* **The real limit is time, not space**: about 925–945 instructions per sample and per DSP on the SYSTEM-8 (on the
  SYSTEM-1, slave DSP A: **between 703 and 826**, 703 ran and 826 hung [measured 2026-10-28]) **adding all its programs**; going over loses
  the end and on the slaves leaves the device unstable until it is switched off [measured]. What a jump skips costs
  nothing [measured]. An instruction takes 2 to 62 bytes (1–31 words; average ≈ 6–8 in Roland's programs: 575 instructions of the
  SH-101 = 5222 bytes; ≈ 8–10 bytes in our programs with 32-bit immediates,
  [Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §1), so the budget of one sample is ≈ 6–8 KiB of executed code.
* Start: SYSTEM-8 `0a02 ff70`, SYSTEM-1 `0a02 ff71`; they are markers without effect [measured], put for fidelity.

## 2. SYSTEM-8

### 2.1 Map [code]

Program window of 64 KiB per DSP (`PRG0` = `0x41460000`, 0x10000 bytes, in the OS's dump table; the slave's ARM writes
at `0x41480000 + n·0x100000` + address). Slots used by Roland's PLUG-OUTs (the same ones `engine_v3.c` registers):

| DSP (slave) | main | slots | space of the main program |
|---|---|---|---|
| DSP0, DSP1 | `0x0100` | `0x8000` (LFO/envelopes) | `0x0100`–`0x7FFF` (≈ 32 KiB) |
| DSP2, DSP3 | `0x0100` | DCO1 `0x6000`, DCO2 `0xB000` | `0x0100`–`0x5FFF` (≈ 24 KiB); each DCO `0x5000` (20 KiB) |
| DSP4 | `0x0100` | `0x6000`, effect `0xB000` | `0x0100`–`0x5FFF` |
| DSP5 | `0x0100` | `0x6000` | `0x0100`–`0x5FFF` |
| master DSP4 | (OS) | effects `0xEFDC`, `0xF7DC` | — |
| master DSP3 | (OS) | — (idle; the main program ends `0b03 0080 7fee`) | — |

### 2.2 Who decides what runs [code, measured]

* The engine (region 1) **registers** in `e_load` the slot addresses of each DSP (`+0x28 + 4·dsp`), the programs
  (`+0x40 + 4·dsp`) and the bank-0 data blocks (`+0x58 + 4·dsp`); the OS copies them.
* A slot **only runs when its task bit is set**: `+0x70` sets it (`swap`: changes the program of a running slot and
  activates its task; DCO1 = task 2, DCO2 = 3; effect of DSP4 = 3). Task register of the DSP: `0x41104318 + n·0x1000`
  (`mask | prio << 28`) [code].
* Descriptors: `{address, bytes, bytes, pointer, 2 OS words}`; **one per DSP**, they are not shared (with a shared one
  every other swap was silenced) [measured].
* Empty programs: DSP0, 1, 2, 3 and 5 can have an empty main program; **an empty main program on DSP4 silences
  everything** [measured, [S8 wave-bus log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md) §4l]. The registered DCO slots get an own null program
  (`gate/dco2_null.hex`) instead of being left without a program [code].

### 2.3 Uses

| use | how | state |
|---|---|---|
| everything in main programs (six programs) | C-V3/C-V4: own main programs, DCO slots with our saw | [measured] C-V4 |
| a slot as a program bank | `+0x70` changes the program of a slot without stopping the rest (the JX-3P does it when changing the waveform or the effect) | [code], used by C-V4 at start-up |
| live tests | `re/dsplive.py` writes single words into the DCO2 slot of DSP2/3 (shadow of 8 KiB) | [measured] |
| a main program longer than its space (running over the next slot) | possible only if that slot has neither a task nor a registered program; the OS does not check it in what was seen | [open] |
| own slot addresses (other than Roland's) | `+0x28` receives them from the engine: they could be chosen | [open] |

## 3. SYSTEM-1

### 3.1 Map [code]

Two DSPs per BMC (A and B). Program memory of 32 KiB (the highest thing written is the closing at `0x7FF2`–`0x7FFF`).

```
0x0000 ─ header (written by the OS: stock sets {0x0000, 0x1E bytes, …}). The PLUG-OUTs do NOT queue it: doing so
         hangs the device [measured, C-S1JX7, 2026-10-22; Vol. I ch. 3 §15.3]
0x0100 ─ MAIN ─────────────────── up to 0x18FF (6 KiB)
0x1900 ─ section 2 ────────────── up to 0x30FF (6 KiB)
0x3100 ─ section 3                up to 0x48FF
0x4900 ─ section 4                up to 0x60FF
0x6100 ─ section 5                up to 0x7FF1
0x7FF2 ─ CLOSING: 7 half-words (0000, ld r0,[e000], st r0,[a0xx]): frame counter to bank 5
```

On the **master** these are also used: DSP A `0x0082` (output program that the OS puts in PLUG-OUT mode,
`0x82`–`0xFF`) and DSP B `0x1000` (program of the PLUG-OUT); and empty section closings at `0x5FF2`, `0x6DF2`,
`0x6FF2` [code].

### 3.2 Who decides what runs [code, measured]

* Programs: **sets** `{address, bytes, pointer}` terminated by `{0, 0, 0}` in the queues `FC30` (DSP A) and `FC34`
  (DSP B); `FC38`/`FC3C` are data queues. The OS empties the queues and then calls `load` of the engine (slave) or of
  the model (master), which queues its own; the master first queues its own PLUG-OUT-mode sets.
* Voice sections [code, measured 2026-10-28]:
  * `FC40(index, program set, data set 1, data set 2, data set 3)` (`0x60003F22`) places a program in a section. The
    four arguments are lists `{address, bytes, pointer}` terminated by `{0, 0, 0}`; the data ones initialise the
    voice's data memory (Roland: `{0x17f8, 8, …}`) and can be empty. It queues on its own queue (`0x600602d8`) with the
    section number and calls the task machinery. The stock synth uses it that way (`0x6004ecb4`: `FC40(i, voice set,
    data set, empty, empty)`).
  * **A program queued with `FC30` / `FC34` at the address of a section is copied but does not run** [measured: the
    voice of section 2 stayed silent]. With `FC40` it runs.
  * `FC44(index, on)` (`0x60003F96`) handles the bit `1 << section` of `[0x60018520 + A/B·4 + 0x68]`, which is an
    **off** bit: `on = 1` clears it (switches the section on), `on = 0` sets it [code]. The OS switches the sections off
    when reloading (`bic 0x3E`).
  * Index → (DSP, section, address): 0 → A/2 `0x1900`, 4 → A/3 `0x3100`, 1 → A/4 `0x4900`, 5 → A/5 `0x6100`, 2 → B/2,
    6 → B/3, 3 → B/4, 7 → B/5 ([Vol. III ch. 9](system1_polyphony_studies.md) §2.2).
  * A section runs **after the main program, in the same sample**, with the same registers and the same memory; its
    instructions add to the same time budget: they give space, not time [measured: SYSTEM-S1CC with voice 6 in section 2
    sounds the same as the others].
* The main program and the closing always run (C-S1V4 sounds with all sections off) [measured].

### 3.3 Uses

| use | who | state |
|---|---|---|
| everything in the main program, sections off | SH-101 (575 instructions), **C-S1V4** (137) | [measured] C-S1V4 |
| main + voice sections (one program per voice) | stock synth (4 per DSP), SH-2 (2 on DSP A) | [code] |
| switching voices on/off without reloading | `FC44(i, on)`: it also saves cycles | [code] |
| changing the program of a voice | `FC40` on a section, **while the sound runs** (from the parameter handler, flag 0 only, no `FC44`): see below | [measured 2026-11-12, JP-08-WTEST4] |
| a main program longer than 6 KiB (running over `0x1900`…) | **hangs the device** [measured 2026-10-28, SYSTEM-S1CC: 6534 bytes with 702 instructions and 6740 bytes with 578 instructions hung; 5854 bytes with 703 instructions ran]. Still to know whether it is the size of the region or the bytes executed per sample (both coincide at ≈ 6 KiB); a program split between the main program and section 2 would tell them apart | [measured] / [open] |
| code that does not run every sample | conditional jumps: alternative programs in the same memory, chosen by a flag; what is skipped costs nothing | [measured on the SYSTEM-8; same ISA] |
| using a section as a "second main program" | place the program with `FC40` (not with `FC30`) and switch it on with `FC44(index, 1)`: it runs after the main program on the same DSP. SYSTEM-S1CC does it only when the main program does not fit its 6 KiB (`examples/system_s1cc/make_s1cc.py SECTIONS`) | [measured 2026-10-28] |

**Changing a section's program while the PLUG-OUT plays** [code: SYSTEM-1 OS slave 1.30; measured 2026-11-12].
The stock synth does it when OSC1 / OSC2 WAVE moves: its parameter handler (`0x6004f14a`) sends `0x167` to
`0x6004ed62` and `0x16C` to `0x6004ecb4`. With a flag ≠ 0 they only keep the value; with flag 0:

```
this->vt[+0x34]()                         ; OSC2: +0x40   (own methods, purpose open)
for i in 0..3:                            ; OSC2: 4..7
    FC40(i, prog_set[sel], data_set[sel], empty, empty)
this->vt[+0x3c]()                         ; OSC2: +0x48
```

No `FC44`. `FC40` (`0x60003F22` → `0x60004a52`) is synchronous: it stops the section, copies program and data,
waits for the DSP (`0x600050a0`) and starts it again. Our version (SYSTEM-S1CC-BOUTIQUE `engine.c`
`k<i>_engine_sections`, `multi.c`, `examples/system_s1cc_boutique/README.md` §12):

```c
void k0_engine_sections(void)                    /* one per kernel: its sections, nothing else */
{
    for (int i = 0; i < NSECT; i++)
        ((place_fn)SVC(0x40))(sect_index[i], sect_set[i], set_none, set_none, set_none);
}
/* multi.c, parameter handler: only with flag 0, as the stock synth */
if ((id == 0x167 || id == 0x16C) && flag == 0)
    k_sections[kmap2[wave1][wave2]]();
```

| sequence | result on the device |
|---|---|
| `FC30` / `FC34` of the main programs while playing | copied, not run: the sound does not change |
| `FC44(i, 0)`, `FC40`, `FC44(i, 1)`, every flag | **hangs the device** [inferred: `FC40` waits for a section already off] |
| `FC40` only, flag 0 only | **works**: the program changes, the note goes on, a slight click |


## 4. Choosing

* **A single program in the main slot** (C-V4/C-S1V4, SH-101): the simplest; all the data are freely chosen. Everything
  that fits in the time of one sample fits (6 KiB on the SYSTEM-1, ≥ 24 KiB on the SYSTEM-8).
* **Slots / sections**: when a part of the sound changes by itself (waveform, effect), when there are per-voice copies
  that are worth switching off, or to change programs while running without touching the main program.
* **Going from the space of one zone into the next**: only with that zone unused and without a task; untested on both
  devices. Before trying it, do a test with a long program that jumps over what does not fit.

> Paths `re/…`, `gate/…` are relative to `examples/from_scratch/`.
