# Volume VI — Chapter 7: Mutable Oscillators That Need Memory — Where They Could Live in the SYSTEM-1

> **Scope:** SYSTEM-1.

Study specification (2026-11-03, with the dated additions up to 2026-11-06). For each shape of `~/git/eurorack` that
cannot be written without memory (delay line or data table), which memory of the SYSTEM-1 serves it, how much fits,
what is missing in s1cc and in the host, and in which order it is best done. The shapes without memory are ported in
`sdk/s1cc/examples/mutable_ported` ([sdk/s1cc/examples/mutable_ported/README.md](../../sdk/s1cc/examples/mutable_ported/README.md)).

Tags: **[measured]** (on the device), **[code]** (read in the firmware / the source), **[estimated]** (calculation),
**[open]**.

## 1. What needs memory

| shape | origin | memory | size per voice |
|---|---|---|---|
| PLUCKED, BOWED, BLOWN, FLUTED | Braids (`digital_oscillator.cc`) | variable delay line (waveguide, Karplus) | 1 period of the lowest note: 96000 / f samples |
| SAW_COMB | Braids | variable delay line (comb) | the same |
| *string* engine (Karplus + dispersion) | Plaits `physical_modelling/string.cc` | variable delay line + dispersion filter | the same (Plaits: 1024 at 48 kHz) |
| *string machine* engine (chorus / ensemble) | Plaits `engine2/string_machine_engine.cc` | delay line with modulated taps | ~10–20 ms: 1000–2000 samples, **one per PLUG-OUT** (after the mix) |
| Rings, Elements | `rings/dsp`, `elements/dsp` | several lines (strings, tube, diffuser) | 1–4 k samples per voice |
| Clouds / Beads | `clouds/dsp`, `beads/dsp` | audio buffer of seconds | 100 k+ samples, global |
| WAVETABLES, WAVE_MAP, WAVE_LINE, WAVE_PARAPHONIC | Braids (`wt_waves`, 129 bytes per wave, 256 waves) | table read by phase | 129–256 words per wave; 2 waves per voice for the morph |
| *wavetable* engine | Plaits (`wav_integrated_waves`, 4 banks × 64 waves × 132) | the same | the same |
| *wave terrain* engine, terrains 5–8 | Plaits | tables of 64 × 64 or of waves | 4 k words |
| *speech* engine (LPC, SAM) | Plaits `speech/` | frames of coefficients (ROM data) | **at control rate**: one frame every 10–25 ms |

What does **not** need memory was ported even though the original uses tables: the function tables (sine, folder,
`ws_*`, `lut_svf_cutoff`, bells) are curves, and s1cc turns them into polynomials or knob tables.

## 2. The memories of the SYSTEM-1 that a kernel could use

| memory | size | who writes | read with a computed index | state |
|---|---|---|---|---|
| bank 0 of the slave (ring) | DSP A: **1024 words**, the addresses wrap at `0x0400` (measured, WT-ALIAS, §4.2 rule 6); DSP B: separate, size not measured (`0000`–`0017` of the OS), shared with the whole state of the kernel | the DSP | **yes**: `hset h6` + `ldidx` (relative to the ring) | [measured] a line of 750 samples with a normal load and with `ldidx` (C-S1SY15X, [Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) §7.4); `hset` from a register in MP-CSAW2-8 (COLOR MOD, 2026-10-29) |
| bank 1 of the slave (ring) | 1024 words (`2000`–`23FF`) | the DSP | `ldidx` does not address it [code] | main ↔ sections mailbox ([Vol. III ch. 2](../vol3_voice_allocation_scheduling/system1_voice_allocation.md) §3.2) |
| bank 5 (`A000`–`A3FF`) | 1024 words, shared by DSP A and B of the slave; free today ~`A070`–`A150` and `A200`–`A3FF` [code, SYSTEM-S1CC] | the ARM (`FD6C`) and the DSP | `da` / `db` (h6 / h7) | [measured] on the SYSTEM-8 slaves ([Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §7.4); on the S1 built (MP-CSAW2-LUT, `LFO_LUT`), **untested** |
| delay memory (bank 2, `tapreq`) | megawords (S8: 2 M per BMC) | the DSP (input `4300`) | taps with base `h4` and result `h5`; ≥ 24 taps per sample, fraction 1/64 | on the S1 only the OS's effect on the **master** B uses it ([Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) §16.3); on the S1 slaves **not measured** [open] |
| parameters (bank 6, `C060`–`C0FF`) | 160 words per DSP | the ARM (`FCEC`) | no (the kernel reads fixed ids) | [measured] 1000 writes/s per id, exact ([Vol. I ch. 6](../vol1_system_architecture/s1_arm_clock.md) §2) |
| memory of the ARM (region 1 of the PLUG-OUT) | ≤ 1 MiB compressed per region (~0.94 MB of PCM) | — | the ARM copies it to banks 5 / 6 | [measured] on the S8 ([NOTES.md](../vol7_cleanroom_evidence_archive/NOTES.md), «Muestras largas») |

> Later (§4.1, §4.2 below, measured 2026-11-05/06): on the S1 slave, bank 5 at `A200` or above is **not** reliable; the
> usable range for tables is `A000`–`A1FF` (`asm/s1rules.py` rule R10).

## 3. The two ideas of the request

### 3.1 Streaming from the ARM's memory with the 1 ms clock

The tick of the slave's ARM ([Vol. I ch. 6](../vol1_system_architecture/s1_arm_clock.md)) calls our function every 1.00 ms; from there `FCEC`
writes DSP parameters and `FD6C` bank-5 words.

| use | words per ms | fits | why |
|---|---|---|---|
| audio (samples of a voice) | 96 per voice (768 with 8 voices) | **no** [estimated] | tested: ~33 `FCEC` per ms without disturbing the OS (8 voices × 2 envelopes + LFO). 96 per voice is 3× that for **one** voice. Moreover the DSP would have to read the word of sample n (a counter and an indexed load of bank 5) and the ARM go one block ahead with its clock without drift relative to the audio's: a missed tick is an audible gap |
| feedback (Karplus, strings) | — | **no** | the output sample goes back into the line within the same period: the ARM is at least one tick (96 samples) behind |
| control frames (LPC / SAM of the *speech* engine, formants of VOWEL_FOF) | 10–20 coefficients every 10–25 ms | **yes** [estimated] | like the host's envelopes (`ARM_ENV`): `FCEC` to `C080`… per voice |
| rewriting a bank-5 wavetable when COLOR moves | 256 words each time the wave changes | **yes**, spread over ticks [estimated] | ~30 words per ms → a new wave in ~9 ms; the DSP always reads from a fixed table (double buffer: the ARM fills the other half and changes a base parameter) |

Conclusion: the clock serves for **data that change at control rate** (voice frames, change of wave, grain positions),
not for audio or for delays.

### 3.2 The delay line

**Bank 0 with `ldidx`** (what already works on the slave). The line is a stretch of the ring: the kernel stores the input
at `S + b` every sample and reads `S + b + d` with `hset h6 = (b + d)/16384` and `ldidx` (d computed: the pitch of the
note; two reads and a linear interpolation for the fractional delay). Nothing else of the DSP may store into
`S + b + 1 … S + b + dmax` ([Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) §7.3, rule 1).

Room: slave DSP A's ring has **1024 words** (the addresses wrap at `0x0400`, §4.2 rule 6) [measured]. With `mp_saw`
and 8 voices the state of DSP A reaches `0x0384` (each state word takes two of the ring, `S` and `S + 1`: about 100
words per voice plus about 100 fixed ones; `make_s1cc.py bank0_top`), which leaves ≈ 120 words for lines [estimated].
A line of d samples gives a lowest note of 96 000 / d Hz. Slave DSP B holds a line of at least 750 samples (measured:
[Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) §7.4); its total size is not measured.

For lower notes without more memory: the line at half rate (the Plaits string runs at 48 kHz): write one sample every two
with the condition `X9bY` (untested on the S1) or averaging in pairs, and read with `d/2`. It doubles the reach with a loop
filter at 48 kHz, like the original.

**Delay memory (`tapreq`)** on the slaves: if it exists, it is the only one that gives strings at any pitch, chorus /
ensemble, Rings / Elements and Clouds buffers. On the S1 it was only seen on the master; the OS does not use it on the
slaves and no official PLUG-OUT does either ([Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) §16.2). On the S8 slaves
there are taps in the programs `p0s_*` (window not measured). Missing test, in this order: (1) a kernel of a slave with
the `tapreq` sequence of the master ([Vol. II ch. 4](../vol2_dsp_core_isa/instruction_set_reference.md) §9.1: `[4300] ← x`, the base instruction,
`0b03 4050 0301`, `3a90`) and the tap read into a word of the transport; (2) a USB capture of a fixed comb of 5000
samples (impossible in bank 0). If the tap gives silence, the delay memory stays only for the master: a global chorus /
ensemble (string machine) on master B, yes; per-voice strings, no.

### 3.3 The large delay memory, from the master

What was measured and read [code unless stated]:

* The delay memory (EDM, bank 2 of the DSPs, `tapreq`) only answers **where the ARM connected the DT2 window**
  ([Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §8.4; on the S8 a single slot of the master, DSP4 `f7dc`). On the SYSTEM-1 the OS
  connects it for its delay + reverb effect, program `0x0` of **master B** ([Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) §16.3: base `h4` = 256, interpolation between two samples, write `4001`). No slave program of the S1 uses bank 2, and the
  register map of the S1's BMC (`0x401xxxxx`) does not have the constants of the S8 (`0x41158000`, `0xC0200000`): we do
  not know yet how to configure the window ourselves.
* The slave sends the master 8 IOMD words per sample: `6006`/`6007` (the mix of SYSTEM-S1CC) and `6008`–`600D`, which
  today are 0 ([Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) §7.1). They are **6 free audio channels per sample,
  slave → master** [measured: `6006`/`6007`; the rest, same mechanism].
* The master's ARM (region 0, `model.c`) assigns the voices: it knows the note of each voice and can write parameters of
  master B with `FCEC`.

**Measured 2026-11-03: MP-EDMKS, a Karplus string in the delay memory, without own microcode on the master.**
`sdk/s1cc/examples/edm/edm_pluck.c` (noise burst of 1–16 ms per note on the slave) + `EDM_KS=1`
(`examples/system_s1cc/build.sh`, `model.c`): at each note region 0 writes

| word | value | what it is |
|---|---|---|
| bank 5 `A101`, `A102` (`FD6C(sel 1, idx 1 / 2)`) | (period − 3) / 16384 | times of the OS's delay (left / right), in units of 16384 samples; `A103` = 2 × `A101` |
| master B `C001` / `C002` (`FCEC(1, …)`) | 1 / 1 | delay on / level |
| master B `C004` | 0.985 | feedback (the OS sets 0.6 with DELAY > 0) |

How it was found: with `FD6C(sel 0)` (`A0xx`) and writing to `0x4016A000` the line stayed at 961 samples (echoes every
961 samples, feedback 0.97 per turn: the delay's line with DELAY TIME at 0). A `PARAMLOG` build (`edm_dump`) read
`A101` = 0.05847 = 958 / 16384 in the `A1xx` page of `FD6C`: 3 samples of loop latency. `0x4016A000` reads 0. Captures:
`sdk/s1cc/examples/edm/ks_probe.sh` (note by MIDI channel 5 of the device + `arecord`, autocorrelation).

| note | expected | measured (0.8 s after the note) |
|---|---|---|
| 36 | 65.41 Hz | 65.48 Hz |
| 55 | 196.00 Hz | 195.92 Hz |
| 60 | 261.63 Hz | 261.58 Hz |
| 72 | 523.25 Hz | 521.74 Hz |

Limits of this form: a single string (the OS's line, the last note sets the pitch); the OS's program smooths the time,
so the pitch glides from the previous note for ~0.5 s; DELAY / DELAY TIME of the panel write their values again; the
reverb stays in the chain. Per-voice strings need the own program of master B (below), with the same `tapreq` that we
now know the window accepts on that DSP.

**Measured 2026-11-03: tests of an own `tapreq` on master B** (`examples/system_s1cc/microcode/edm.s`, options `EDM_TAP` /
`EDM_ENTRY` / `EDM_STEP` / `EDM_MAIN` of `make_s1cc.py`; capture with `sdk/s1cc/examples/edm/ks_probe.sh`):

| test | what | result |
|---|---|---|
| EDMTAP 1 | our main `0x1000`: 2 `tapreq` of 0.5 s (with and without a position counter), results at `4040` / `4043` | the dry burst sounds, **without echo** |
| EDMTAP 2 | the same with results at `402E`; the other channel reads the result word of the OS's delay (`4005`) | the channel of `4005` brings the OS's 10 ms echo: **bank-2 loads work from `0x1000`**; our request, nothing |
| EDMTAP 3 | our two requests at the exact time of the OS (`A101`) | nothing on either channel: **`tapreq` is only served in the program at `0x0`** (as on the S8, where only one slot had the line) |
| EDMOWN (own `0x0`, without the entry calls) | replaces the effect | **hung** when selected |
| EDMENT 4 (own `0x0`: header + calls to `0x1000`, `0x5FF2`, `0x6FF2` + `tapreq` + jump to `0x7FF2`) | | **switched off** when selected |
| EDMENT A (the same without `tapreq`) | | does not hang, **total silence** |
| EDMENT B (A + main with the mix directly to `A118` / `A119`) | | silence: the main program does not run |
| EDMENT A + own closings `0x5FF2`, `0x6FF2`, `0x7FF2` (order of the SH-101's set) | | silence |
| control: only the 4-instruction header at `0x0` (15 words, like the 30-byte `0x0` of the stock PLUG-OUTs) | | **silence** |

After the EDMENT tests, MP-EDMKS hummed at full scale (95 Hz, peak 1.0, without playing) and then stayed almost silent:
master B does not go back to its state by itself when another PLUG-OUT is chosen. **Switching off and on** fixed it:
MP-EDMKS silent again without notes and the string at 130.79 Hz (note 48) and 260.87 Hz (note 60). Since then
`model.c` sets the feedback to 0 at load and writes it only with the first note (by default 0.975).

Reading: **it was already measured** ([Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) §15.3, rule 1 of
[examples/system_1_jx/README.md](../../examples/system_1_jx/README.md) §5): a PLUG-OUT **never queues `0x0000`**; it is the
header the DSP is executing. Roland's engines have it in their tables but pass the next entry to `FC30` / `FC34`
(executed in Unicorn). The EDMOWN / EDMENT tests broke that rule: hence the hang, the switch-off and the silences (and the
broken state until power-off). Own code cannot be put at `0x0`.

Consequences (2026-11-03):
* Own `tapreq`: **not** for now. Only the program at `0x0` (the OS's effect) serves it; what enables it is still to be
  known (on the S8: the task / slot the ARM connects, [Vol. III ch. 3](../vol3_voice_allocation_scheduling/slot_program_model.md) §2.2) and whether
  a section of ours can receive it.
* What does work: the times of the OS's two taps (`A101` / `A102`, `FD6C sel 1`), their feedback and level (`C004`,
  `C001`, `C002`), and **reading their results `4004`–`4009` from our main program** (test EDMTAP 2).
* The pitch glide: the effect smooths the time with `r2 = A101 (1 − r7) + previous · r7`, with
  `r7 = min(0.0287 + previous r7, 0.9998875)` (it rises to 0.99989 in ~35 samples and stays) → τ = 92.6 ms, measured
  ~90 ms. It is a constant of the OS's code; it is changed by overwriting the OS's set from `create()` (§3.3.2). Read
  from the transfers of the program (`st r7, [0009]` / `ld r6, [000a]`); the emulator does not run its class-`0x06f0`
  instruction.
* To attack the glide without own code at `0x0`: write `A101` with overshoot from a tick of the master's ARM (the 1 ms
  hook exists on the slave, [Vol. I ch. 6](../vol1_system_architecture/s1_arm_clock.md); on the master not looked for).

#### 3.3.1 Static reading of the master's OS 1.30 (2026-11-03, without the device)

| finding | where | state |
|---|---|---|
| `FC30` / `FC34` / `FC38` / `FC3C` = the same routine (`0x6001CF6C`, a `push` into a vector) on four queues: programs master A, programs master B, **data** master A, **data** master B. `FC40` on the master is `bx lr` (it does nothing) | service table `0x600AB5DC` → `0x6001ED77`… | [code] |
| the data sets are `{address, bytes, pointer}` like those of programs. Roland (SH-2, SH-101, PROMARS) queues on the master: `{0x110, 8 bytes = 0, 0}` and `{0x17F8, 8 bytes = 0x807FFFFF, 0}` | Unicorn: `load` of the SH-2 (region 0) | [measured in Unicorn] |
| `{0x17F8: 0x807FFFFF, 0}` goes with **all** the OS's programs (more than 50 sets on the slave): it is a common initialisation, **not** the key of the line | search in `app_master.bin` / `app_slave.bin` | [code] |
| the OS does queue its master-B set **with `0x0`**: `{0x0 effect 0x552 bytes, 0x5FF2, 0x6FF2, 0x1000 default main 0x38 bytes, 0x7FF2}` + data `{0x118, 0}, {0x17F8, 0}, {0x0, 0x110}…`, from descriptors `{count, programs, data}` at `0x6006025C` | `0x600971F0`, `0x60060278` | [code] |
| the glide of the string: fixed coefficient 0.9998875 in the effect (τ 92.6 ms) | §3.3 | [code + measured] |

Remaining hypotheses (all need the device):
1. **`tapreq` is only executed in the top-level program** (the one at `0x0`, which the DSP walks without a call), not in
   the called ones (`cond 25, 10` → `0x1000`, where our request did nothing). The OS also jumps (`cond 0, 8`, without
   return) to the closing `0x7FF2`, which the PLUG-OUTs do queue (`FC34 {…, 0x1000, 0x7FF2}`). Cheap test within the
   rules: our `0x7FF2` of master B with the frame counter **plus** a `tapreq` of 0.5 s on the line the OS writes, read in
   the next sample by the main program. If it gives an echo, we have an own `tapreq` without touching `0x0`. Risk: the
   closing has 14 bytes up to the end of program memory (`0x8000`); the request fits (3 instructions of 2–3 words) only
   if the closing can grow backwards (start before `0x7FF2`): that is the first thing to measure.
2. If not: `tapreq` depends on the slot / task the ARM connects (as on the S8); look in the master's OS for the write of
   the register equivalent to the S8's `+0x310` ([Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) §2.2) for the effect.
3. Queuing a complete set like the OS's (with `0x0`): **no**: rule 1 forbids it and the EDMENT tests confirmed it.

#### 3.3.2 Overwriting the OS's set from region 0 (measured 2026-11-03)

`EDM_GLIDE=1` (with `EDM_KS`, build MP-EDMKG; `model.c edm_glide`) changes in the OS's RAM the smoothing constant of the
effect: word `0x60097404` (inside the program `0x0`, 0x552 bytes at `0x60097364`, halfwords `2fff 8a00` =
`mov r6, #0.9998875@0:24`) → `0x28000000` (0.5, `asm/asm.py`), only if it has the value of 1.30.

| finding | state |
|---|---|
| The OS queues its master-B set (with `0x0`) **only at boot** from its descriptors in RAM; choosing the PLUG-OUT or going through SYSTEM-1 mode does not reload it (patch without effect until power-off) | [measured] |
| `create()` of the slot's PLUG-OUT runs **at boot, before** that load, even if the device boots in SYSTEM-1 mode: after switching off and on, the DSP has the patched value although that variant only wrote it into the RAM | [measured: read of the DSP] |
| Result: **the string changes pitch in < 10 ms** (before ~140 ms, τ 90 ms): 367 → 734 → 184 samples | [measured, `seq.wav`] |
| Program memory of master B mapped at **`0x40170000` + address in bytes** (at `+0` the effect, `0C030FE0 A0000A03…`; at `+0x1000` our main); master A at `0x40150000`. Same format as the image (two 16-bit words per 32-bit word, the upper first); the first word differs (`00010001` in the DSP, `00000001` in the RAM) | [measured: read] |
| The ARM can write that memory live (`0x401700A0` read the written value); its effect on the audio was not measured (the USB delay, below, confused the test) | [measured: read] / [open] |
| **The USB audio arrives up to ~2.7 s late** relative to the MIDI with the USB connected (always so, according to the user): the probes look for the start of the note instead of using fixed times (`ks_probe.sh`) | [measured] |

Consequence: **an own `0x0` is possible without queuing it**. Region 0, in `create()`, overwrites the effect's program (or
the descriptor `{0x0, 0x552, 0x60097364}` at `0x600971F0`, pointing it to a program of ours in region 0) and the OS loads
it itself at boot, with its sequence. That way an own `0x0` has `tapreq` (only that program receives it, §3.3) and the
rule of not queuing `0x0000` is respected. Requirements: keep what the OS expects from the effect (header, calls
`cond 25/26/27`, jump to `0x7FF2`, counter `A004`); the change only appears **after switching off and on** with the
PLUG-OUT installed, and it holds in all modes until power-off (SYSTEM-1 mode also runs that `0x0`).

From the OS (static reading, agent 2026-11-03, not measured): the ARM has an **indirect port to the EDM** (`0x40100050`
address, `+0x54` control, data at `+0x800`, `+0x7C` bit 8 to write; used by the boot self-test `0x6001D51A`), that is,
region 0 could fill tables in the EDM. No ARM register enables `tapreq` per program: everything is per DSP. The OS marks
the programs of a set by rewriting their first word `…ff71` → `…ff70`.

#### 3.3.3 Own `tapreq`: a call from the OS's effect (measured 2026-11-03)

**It works.** `EDM_CALL=1` (build MP-EDMCALL: `KERNEL=sdk/s1cc/build/edm_pluck NAME=MP-EDMCALL EDM_KS=0 EDM_CALL=1
sh examples/system_s1cc/build.sh`) requests an own tap of the OS's line **from inside the program at `0x0`**, without
queuing `0x0` and without Roland microcode in the PLUG-OUT:

| piece | where | what |
|---|---|---|
| block | `microcode/edm_call.s` `edm_call_block`, written on master B at byte `0x0C00` (free: the effect ends at `0x552`, the main starts at `0x1000`) | the two OS instructions that the call replaces (copied **from the DSP** at the moment) + `mov r2, #age` / `hset h4, #256` / `hset h5, #(402E − 4000)/16384` / `f6 r2, -r4` / `tapreq r2, 0` / `cond 0, 15, 8` (return) |
| site | `edm_call_site`, over the effect's second `tapreq` and the next instruction (5 half-words at byte `0x164`, OS 1.30) | `cond 0, 10, 0` with slot `0x0600` (call to `0x0C00`) + filler `mov r2, r2` |
| region 0 | `model.c edm_call`, in `load` | writes the block and then the site at `0x40170000 + byte`, only if the site has the FNV-1a of 1.30 (if it is already patched or it is another OS, it touches nothing) |
| main | `edm.s master_edm` with `EDM_REQ` 0 | reads `402E`–`4030` (x0, x1, fraction) one sample later and adds `0.7 ×` the tap to the left channel |
| the line | `EDM_TAP` in `model.c` | the OS's delay switched on with level and feedback 0: it only writes the line with our send |

Result: a dry noise burst on both channels and **its echo only on the left at 48 007 samples** (48 000 requested + 7 of
latency send → line → tap → read), gain 0.074 relative to the dry, without repetitions. The `r4` of the site is the
effect's position counter (its second tap uses the same form `f6 r2, -r4`), so the address is the age in units of 16384
samples.

Consequences:
* The OS's `0x0` serves as a **trampoline**: any number of own requests in a block called from there (the calls
  `cond 0, 10, 0` / return `cond 0, 15, 8` work on master B). Per-voice strings, chorus, granular: the requests in the
  block, the logic in our main program (it reads the results one sample later).
* The patch lives until power-off and holds in all modes; with another PLUG-OUT the block keeps being called (it requests
  a tap nobody reads: harmless). At power-off the OS loads its effect again.
* Pending: measure how many requests fit per sample on master B, and whether boot (OS table, §3.3.2) is better than the
  live write (this one writes with the DSP running: three consecutive words with interrupts off; no problem was
  observed).

**Own write of the EDM: not yet** (tests MP-EDMW, `EDM_CALL=write`, `edm_call.s edm_call_block_w`, 2026-11-03; capture
of 14–16 s with the USB delay discounted):

| test | write | read | result |
|---|---|---|---|
| 1 | our send `st → [4034]` with `h4 = 296` | left: `h4 = 296`, 0.5 s; right: the OS's line, `h4 = 256`, 0.25 s | left **nothing**; right echo at 24 006 samples |
| 2 | the same, the OS's delay off (`C001 = 0`) | left: `h4 = 256`, 0.5 s; right: `h4 = 296`, 0.25 s | nothing on either (16 s) |
| 3 | only the OS's (`[4001]`, `C001 = 1`) | left: `h4 = 296`, 0.5 s; right: `h4 = 256`, 0.25 s | right echo (24 364 from the measured start); left **nothing** in 15 s |

Reading: the base `h4` is not a shift over a ring that advances by itself (with 40 units of difference the echo would
have fallen at ~7.3 s), and storing to another even word of bank 2 does not write where we read it. The effect carries its
own position `p` (`r4`, `+[E013]` per sample, wrapping at 35.15625) and requests at `base + age − p`: the write of
`[4001]` falls at `base − p`, but how the hardware receives that address is not seen (candidates: the class-`0x06f0`
instruction, not decoded, before the first tap, or a map of the bank-2 window that the ARM sets: the maps 1–3 per DSP of
the controller, `0x40100100…`). Possible next step: read the EDM from the ARM with the indirect port (`0x40100050` /
`+0x54` / `+0x800`) to find where the OS's line falls.

The address of each write is set by the DSP's map table, not by the program (§3.3.4).

What can already be done without an own write: everything that **reads** the OS's line with own taps (chorus / ensemble,
multi-tap, granular over what enters the delay), with the signal our main program sends to `2000` / `2002`.

Design: **the voices with memory end on master B.**

```
slave A/B (per voice)  excitation / dry signal of voice v  ->  6008 + v  (up to 6 voices)
master B (ours, 0x1000)  per voice: line in the EDM: input = excitation + filtered feedback,
                         variable tap (tapreq) at the length of the note (parameter from the master's ARM),
                         interpolated; sum of voices -> send to the OS's effect (2000 / 2002)
```

| shape | what runs on the slave | what runs on master B | voices |
|---|---|---|---|
| PLUCKED, Plaits string, SAW_COMB | the attack pulse / noise and its envelope | Karplus: line + one-pole loop filter + optional dispersion, ~15 instr. per voice [estimated] | 6, at any pitch (the line can have seconds) |
| BOWED, BLOWN, FLUTED | the bow / breath (non-linear) needs the sample of the string in the same sample: it is **not** split between chips | — | only if the slave has the window |
| chorus / ensemble of the *string machine* | the divide-down saws (`mp_string_synth`) | 2–3 taps modulated by an LFO over the mix, ~20 instr. in total | all: it is a global effect |
| Clouds / Beads (granular over a buffer) | the voice | buffer of seconds and grains read with taps | global |
| Rings / Elements | the excitation | modal resonator (without memory: it can go on the slave, `mp_modal`) or of strings (on the master) | 4–6 |

Limits and risks:
* Master B already runs the OS's effect (107 instr.) and our mix; its budget is not measured (steps 14/14A of C-S1SY hung
  because of untested op forms). Each `tapreq` costs one request and one read; on the S8 ≥ 24 requests per sample fit.
* The own line cannot overwrite that of the OS's delay: which stretch of the EDM it uses has to be known (base 256, length
  according to DELAY TIME).
* **The first thing to measure**: whether a `tapreq` of *our* master-B program (`0x1000`) reads the EDM or whether the
  window is only connected for the OS's program at `0x0` (on the S8 it depended on the slot).

Proposed test (a test PLUG-OUT, USB capture): in the master-B main program of SYSTEM-S1CC, write the mix into the line with
the sequence of the OS's program (same `h4`, a base far from the OS's), read a fixed tap of 0.5 s and add it to the
output. A 0.5 s echo in the capture = the EDM is ours from master B. Silence = the connection of the window in the
master's ARM has to be found (or our code placed in slot `0x0`, replacing the OS's effect).

#### 3.3.4 Own lines: the bank-2 map table (measured 2026-11-03)

**It works.** Which bank-2 word is written to / read from which delay-memory address is not said by the program: it is said
by a **table per DSP**, that of master B at `0x4016E000` (that of A at `0x4014E000`), 8 bytes per entry, 768 entries
(`0x1800` bytes):

| word 0 | word 1 | what |
|---|---|---|
| `0x80000000 \| address` | `word << 16` | write: what the DSP stores at `[4000 + word]` goes to the address |
| `address` | `word << 16 \| length` (in the DSP: length 0) | read: `[4000 + word]` brings the sample of that address |
| `0x807FFFFF` | `0x007FFFFF` | free |

The addresses are relative to a pointer that advances by itself: a read at `write + d` gives the sample written `d`
samples before. The unit is the sample (256 in the units of `h4` = `0x400000`).

| finding | where | state |
|---|---|---|
| The OS loads its table for master B with a data set `{0x0, 0x110 bytes}` (34 entries, origin `0x60097928`) + `{0x110, 8 bytes = 0}` + `{0x17F8, 0x807FFFFF 0}`, from the descriptor `0x60060280` | OS 1.30 | [code] |
| Entries of the OS: `[4001]` → `0x400000` (the delay's write, base 256); `[400a]` → `0x48CA00` (input of the reverb, 576 000 samples later: the maximum length of the delay and the wrap of `p`); the reverb's lines: an even word writes, an odd one reads (`[400e]` → `0x48F340`, `[400f]` ← `0x48FB5C`, …) | `0x60097928` | [code] |
| In the DSP the live table is not equal to the RAM copy: the OS's reads lose the length and three of them are shifted −2 / +1 samples (`0049aa4c 002c23e9` → `0049aa4a 002c0000`); ours, loaded the same way, keep the exact address and length 0 | PARAMLOG `edm_ecf_dump` | [measured] |
| **Writing the table directly from the ARM** (stores to `0x4016E000 + 8k`, with the DSP running, addresses `0x4A0000` / `0x4C0000`) **switched the device off** when the PLUG-OUT was chosen. With the data set through `FC3C` (master B's data queue) and addresses inside the stretch of the OS's delay: no problem. Which of the two things caused the switch-off was not separated | MP-EDMECF | [measured] |
| `[4034]` → `0x440000`, `[4035]` ← `0x44BB80` (48 000) and `[4036]` → `0x460000`, `[4037]` ← `0x465DC0` (24 000), written and read by the **main program at `0x1000`** with ordinary `st` / `ld`: left echo at **48 001** samples and right at **24 001**, gain 0.700 (the one requested) | `ks_probe`-style, `p2.wav` | [measured] |

How it is done (`EDM_ECF=1`, build MP-EDMECF: `KERNEL=sdk/s1cc/build/edm_pluck NAME=MP-EDMECF EDM_KS=0 EDM_ECF=1
sh examples/system_s1cc/build.sh`):

| piece | what |
|---|---|
| `model.c edm_ecf`, in `load` | reads the table (entries 0 and 33 of OS 1.30; 34–38 at zero, free or ours) and queues `{0x110, 40 bytes}` with `FC3C`: two write / read pairs + the zero entry behind. If they are already there, it does not queue |
| `microcode/edm_ecf.s master_ecf` | main program of master B: `st` of the send to `[4034]` and `[4036]`, `ld` of `[4035]` and `[4037]`, × 0.7 to the output |
| duration | until power-off (the OS loads its table again at boot); with another PLUG-OUT the entries remain but nobody uses those words |

Consequences:
* **Each voice can have its line**: one write entry and one or two read entries per voice, in free words of bank 2
  (`4034` onwards). Neither `tapreq` nor touching the program at `0x0` is needed.
* Fixed length per entry. For the pitch of a string: (a) `tapreq` on the own line (with the OS's convention, `h4` = write
  address / 16384, `f6 r2, -r4`; not measured on an own line); (b) rewrite the read entry at each note with another
  `FC3C` data set (the set is applied with the queue: latency not measured); (c) several fixed reads and interpolation.
* The stretch used (`0x440000`–`0x465DC0`) is inside the OS's delay: its taps of more than 262 144 samples (2.7 s) would
  read our lines. How large the S1's EDM is beyond `0x49B0DA` is still to be measured (the test at `0x4A0000` was the one
  that switched the device off, together with the direct write).

#### 3.3.5 One Karplus-Strong string per voice (MP-EDMSTR: **works, 6 voices**, 2026-11-04)

**State (2026-11-04): works on the device, 6 voices, «sounds very nice» (user).** Build:
`sdk/s1cc/s1cc build sdk/s1cc/examples/edm/edm_string.c && KERNEL=sdk/s1cc/build/edm_string sh
examples/system_s1cc/build.sh` (`EDM_STR=1 EDM_STR_ONE=1` come from the kernel's `S1_BUILD`) → `plugins/MP-EDMSTR` (the
build says «8 voices»: those of the slave; the assigner of region 0 uses the 6 strings).

The form that works (`EDM_STR_ONE`, the last of the table of tests below):

| piece | what |
|---|---|
| map table | only the writes, one per voice (`[4040 + 8v]` → `0x440000 + 0x4000 v`), queued with `FC3C` in `load`: the table **does not change afterwards** |
| reads | one `tapreq` per voice in the block of the trampoline (`edm_call.s edm_str_block1`, §3.3.3): `ld r2, [C030 + v]` (the age, which region 0 writes with `FCEC` at each note), `hset h4` = base of the line, `hset h5` → `4080 + 8v`, `f6 r2, -r4`, `tapreq` |
| loop | `edm_str.s master_str1`: `z = x0 + g (x1 − x0)` (`g` at `C020 + v`), `s += b (z − s)` (state in bank 0 `0300 + 2v`), `w = x + fb s` |
| tuning | `make_s1cc.py str1_tune`: `d` and `g` such that `d + 1` + the phase delay of the two filters at the note = the period; tables for 16 steps of FILTER CUTOFF (`b` 0.25 … 1); moving CUTOFF retunes the voices |
| verification without the device | `sdk/s1cc/examples/edm/str1_sim.py`: ±0.5 cents (note 91 with `b` 0.25 dies out before it is measured) |

Tests on the device (2026-11-04), in order:

| build | what changes | result |
|---|---|---|
| MP-EDMSTR | 4 reads by table per voice, retouched through `FC3C` at each note | sounds, **fixed pitch**: the queue is only emptied when the PLUG-OUT is chosen |
| MP-EDMSTRL | the same + PARAMLOG dump of the table | confirms: the entries do not change after `load` |
| MP-EDMSTRW | read address written directly into the table at each note | **hung** at the first note |
| MP-EDMSTRT | reads by `tapreq` (2 per voice), 8 voices: 16 requests | **hung** when chosen |
| MP-EDMSTR1 | the same, 1 voice (2 requests) | works |
| MP-EDMSTR4 | 4 voices (8 requests) | **hung** when chosen |
| MP-EDMSTR1P | 1 voice + the block padded (not executed) to the size of the 4-voice one | works: it is not the size |
| MP-EDMSTR2 | 2 voices (4 requests, bases 272 / 273) | works |
| MP-EDMSTR3 | 3 voices (6 requests) | works |
| MP-EDMSTR6 | `EDM_STR_ONE`: 6 voices, 1 request each | **works** |

The panel's DELAY (the OS's effect, its line at `0x400000`) **keeps working together with the strings** [measured
2026-11-04, user]: our lines (`0x440000`–`0x454000`) and its tap coexist.

**Measured limit: 6 own `tapreq` per sample on master B work, 8 hang** (plus the 2 of the OS's delay). `make_s1cc.py`
rejects more than 6. Below the limit, the options `EDM_STR_TAP`, `EDM_STR_NV` and `EDM_STR_PAD` remain to repeat the
tests.

**SAW_COMB (MP-SAWCOMB, `EDM_STR_COMB=1`): works on the device** (2026-11-04, «sounds good»). Kernel
`sdk/s1cc/examples/edm/edm_sawcomb.c`: the saw of `mp_saw.c` with the filter and the VCA of `mp_dsp_b.h` on the slave; on
master B the comb of Braids (`digital_oscillator.cc RenderComb`) on the same lines: delay = period of the note + OSC1 COLOR
(±64 semitones, Braids' TIMBRE), feedback = OSC2 COLOR (bipolar, curve `0.97 tanh(2x)/tanh 2` instead of
`ws_moderate_overdrive`), no filter in the loop (`b` = 1), output `x + z` (`master_str1` with `.if STR_COMB`). Emulator:
impulse 0.5 → echoes 0.5, 0.4, 0.32 at the period (366.94 samples, note 60). The two COLORs are written by region 0 from
the knob: **they are not modulated** (COLOR MOD does not reach the master).

**Plaits string (MP-PLSTRING, `EDM_STR_PL=1`): works on the device** (2026-11-04, «sounds beautiful»). Kernel
`edm_plstring.c` (5 ms noise burst through a pole: OSC1 COLOR = brightness of the excitation, the TIMBRE of Plaits'
string engine); master `edm_str.s master_pstr`, 6 voices:

| piece | what | why |
|---|---|---|
| damping | two equal poles `b` in series at 24 … 84 semitones above the note (FILTER CUTOFF, 8 steps) | Plaits' SVF with Q 0.5 is that; with 12 semitones (Plaits' minimum) only the burst was heard |
| dispersion | three first-order allpasses `y = a (u − y1) + x1` (OSC2 COLOR, 8 steps) | Plaits uses an allpass on another line: another `tapreq` per voice, more than the 6 per sample. With a fixed `a` the stretch went from nothing (lows) to semitones (highs): `a` per note so that the 8th partial rises `300 amt³` cents (half: +60, ¾: +236, maximum: +420, the same on all notes) |
| output | the string alone, without the burst | like Plaits |
| tuning | `make_s1cc.py pl_loop`: bisection with the phase of each stage added (no 2π wrap) | emulator `pl_sim.py`: ±2.5 cents (resolution) |

Measured on the device (OSC2 COLOR, note 48, `disp.wav`): minimum, partials at +1 cent; maximum of the 50-cent version:
2nd +5, 4th +18, 8th +55 (the computed values) → barely audible; hence the curve `300 amt³`.

**BLOWN (MP-BLOWN, `EDM_STR_BLOWN=1`): works** (2026-11-04, «sounds good»). Braids `RenderBlown` (reed + tube, 6 voices) in
`master_blown`; the breath is made on the slave (`edm_blown.c`). Changes against Braids, measured:

| what | Braids | here | why |
|---|---|---|---|
| pressure | 0.8 fixed | OSC1 MIX 0.35 … 0.9 | |
| breath noise | ±0.68 … 0.29 of the pressure | OSC1 COLOR 0 … 0.25 | Braids on the device: «square with noise», noise at −30 dB between harmonics |
| tube loss | none (the average of two samples) | a pole in the reflection at 18 … 66 semitones above the note (OSC2 COLOR, 8 steps) | without it the reed gives a square at any pressure (H3 −10, H5 −16, H7 −15 dB, measured); with it: H3 −20 / H5 −38 (soft) … −11 / −18 (clarinet) … −10 / −14 (square) |
| body | `lut_flute_body_filter` per note + COLOR | the same table (`30 · 2^(i/12) / 4096`) at the note | |

Tuning: the loop (line + average + pole) = half a period (the reflection inverts), `blown_loop`; emulator `blown_sim.py`:
±4 cents, even harmonics at −45 … −68 dB.

**BOWED (MP-BOWED, `EDM_STR_BOWED=1`): works** (2026-11-04, 3 voices: **two lines per voice**, neck and bridge = 6
`tapreq`, the limit). Braids `RenderBowed` in `master_bowed`: the bow (`edm_bowed.c`: 0.16 × OSC1 MIX × the AMP envelope)
against the velocity of the string; friction `min(1, (|Δv| s + 0.75)^−4)` (`lut_bowing_friction` is STK's table, fitted
exactly) with `1/y` by a quadratic seed + 2 Newton steps (error 7·10⁻⁴); `absadd` (`|x| + Y`) in one op. OSC1 COLOR = bow
pressure (`s = p0/32`), OSC2 COLOR = position (bridge part `(6 … 69)/256`, 8 steps).

| measured on the device | what | fix |
|---|---|---|
| «metallic, like ring mod» | Braids' body (biquad at 493 Hz, `out = t − y1`: band-pass) left the fundamental of C3 15 dB below the 7th harmonic | the same poles as a low-pass of gain 1 (`gin = 1 − p1 − p2`): the fundamental on top |
| period 2 samples long on all notes | one more sample of latency per line than the model's (`a + 1 + g`) | `bowed_loop` subtracts 2: C2 +0.8, C3 0.0, C4 +1.2, C5 +0.6 cents |
| peak 1.0 with gain 2.4 | | 1.2: peak 0.62 |

The very low 6th harmonic with OSC2 COLOR at the centre is that of the real string bowed at 1/6. Pending: measure whether
the one-line forms (MP-EDMSTR, MP-PLSTRING, MP-BLOWN) are also one sample long (the computation with the emulator was
exact; on the device the fine tuning was not measured).

Plan for COLOR MOD on the master (not implemented, 2026-11-04):
* Transport: the slave sends master B 8 tested words, `6006`–`600D`; 6 already carry the audio of the voices.
  `6026`–`602D` are read by **master A** in the stock synths ([Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) Part B), they
  do not serve for B. `600C` / `600D` remain: **multiplex** the 12 colors (6 voices × 2) on them, `600C` = value, `600D` =
  slot number / 16; each color is renewed every 12 samples (8 kHz).
* Slave (DSP B, `host_b.s`): the colors of each voice (`s1_color(0/1)` of the kernel through `s1_export`, or the COLOR MOD
  words of the host) in a block of bank 0; a slot counter in the state; `hset h6` from the register + `ldidx` (the form of
  `color.s colors_idx`, measured on the S1) → `600C`, the counter → `600D`.
* Master B (`edm_str.s`): per slot j, `sel = max(0, 1 − K |tag − j/16|)`, `c_j += sel (value − c_j)` (state in bank 0,
  ~8 instructions × 12); per voice `age = base_v · 2^(−10.67 (c0 − 0.5))` (`base_v` = period / 16384 through `FCEC` at
  each note; the exponential split of the SFU as s1cc's `exp2split` emits it), clipped to `[0, 0.9997]` (the lines are at
  16 384 samples), and the fraction of the `tapreq` itself (`R + 2`, 1/64) instead of `g`; `fb = 0.97 r (1.5 − 0.5 r²)`,
  `r = 2 c1 − 1`. The trampoline block reads the age from bank 0 (written by the main program in the same sample) instead
  of `C030 + v`. About 190 more instructions on master B (the SH-2 has 406 in the same place).

What follows in this section describes the first design (reads by table), kept as a record. Variant with a dump of the
table at each note: `NAME=MP-EDMSTRL PARAMLOG=1` (id `0x3D00` state, `0x3D01 + k` entries 33–44, 73–75;
`examples/system_1/paramlog.py`).

| piece | where | what |
|---|---|---|
| excitation | `edm_string.c` (slave, DSP A of the kernel) | noise burst of 1–16 ms (OSC1 COLOR) × OSC1 MIX at each note, like `edm_pluck.c` |
| transport | `host_b.s head_b`, `.if EDM_STR` | no mix: the output of voice v × `C000` → IOMD `6006 + v` (8 words, the ones the OS's stock mixer reads on the master: [code], §1 of [Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) Part B) |
| lines | `model.c edm_str`, in `load` | one `FC3C` data queue `{0x110, 41 × 8 bytes}`: per voice one write `[4040 + 8v]` → `0x440000 + 0x4000 v` and four reads `[4041/43/45/47 + 8v]` ← that address `+ d … + d + 3`; a zero entry behind (entries 34–74 of the table). Even words write, odd words read (the OS's convention) |
| pitch | `model.c edm_str_note`, at each note | another `FC3C` queue with the voice's four reads (`d` of the note) and `f` at `C020 + v` (`FCEC`) |
| loop | `microcode/edm_str.s master_str` (main program of master B, 220 instructions, one per line like `edm_ecf.s`) | `F0 = a (y0 + y2) + c y1`, `F1 = a (y1 + y3) + c y2` (linear-phase filter `[a, 1 − 2a, a]`, 1 sample), `z = F0 + f (F1 − F0)`, `w = x + fb z` → `[4040 + 8v]`; the sum × 0.5 × VOLUME → send of the effect |
| panel (region 0) | `edm_str_param` | AMP DECAY → `fb` with the key held (`1 − 10^−(1 + 3v/255)`, 0.9 … 0.9999 per turn), AMP RELEASE → `fb` released (`1 − 10^−(0.3 + 3v/255)`), FILTER CUTOFF → `a = 0.25 (1 − v/255)` (`C01D`, `1 − 2a` at `C01E`) |

Period: the reads give `y_k = w[n − d − 1 − k]` (the read arrives one sample after its input: 48 001 for 48 000, §3.3.4),
the filter is centred on `y1` and the interpolation adds `f`: **period = d + 2 + f** (`make_s1cc.py str_period`; note 0:
d = 11 739, each line has 16 384 samples of room).

Verified without the device: `sdk/s1cc/examples/edm/str_sim.py` runs `build/s1mm1.s` in the C emulator with a model of
the table (the reads as above): 8 strings at once (notes 48 … 84), pitches within ±0.9 cents (resolution of the FFT).
Test on the device: `sdk/s1cc/examples/edm/str_probe.sh` (six notes one at a time, each in another voice, and a chord of
three; it prints the pitch of each one).

**Measured 2026-11-04: it does not change pitch.** All the notes sound the same and a chord sounds like one note.
MP-EDMSTRL (dump of the table at each note): the entries loaded in `load` are there (state 1, `[4041]` ← `0x44016C` =
`0x440000` + 364, note 60), but **after each note they stay the same**: the `FC3C` data queue is only emptied when the
PLUG-OUT is chosen. Read in OS 1.30: `0x6001ECA2` empties the four queues (`FC30`–`FC3C`, vectors `{capacity, count, read,
data}` at `0x604C0C70`–`0x604C0CA0`), queues the OS's sets for PLUG-OUT mode, calls `load` (vt+0x08) and then the OS
loads everything; during execution nothing empties the queues [code + measured].

**MP-EDMSTRW** (`EDM_STR_DIRECT=1`): at each note, the address word of the voice's four reads written directly into the
table (one 32-bit `str` per entry, inside the loaded stretch): **the device hung at the first note** (2026-11-04; it had
to be switched off). With the direct write of §3.3.4 (switch-off) these are two failures: **the table is not written from
the ARM with the DSP running**, not even one word. What remains: the pitch by `tapreq` from the trampoline (§3.3.3), with
the fixed table (only the writes of each voice) and two requests per voice (age and age + 2: `y0`…`y3` + `f` as now),
which depends on `tapreq` serving on an own line (MP-EDMIDX, §3.3.6).

What can fail on the device (each point is a question of the test):
1. **The `FC3C` queue in the middle of execution**: §3.3.4 only used it in `load`. If the queues of each note are not
   applied, all voices stay at note 60 (the load) and only `f` changes: it sounds almost the same pitch. MP-EDMSTRL shows
   it (entries 35–38 must change from one note to the next).
2. **How many entries the table serves per sample**: the OS uses 34; with 8 voices there are 75. If the DMA does not
   cope, mute lines or noise will be heard in the high voices (the entries beyond the limit).
3. `6008`–`600D` from our slave to our master: read in the OS, not measured with an own PLUG-OUT.
4. Budget of master B: the OS's effect (107) + 220 of ours; the SH-2 has 406 in the same place.

Limits: the pitch bend does not move the string (only the note); the entries stay until power-off and keep writing
`0x440000`–`0x460000` with another PLUG-OUT (the OS's delay only reads them with more than 2.7 s).

#### 3.3.6 The delay memory as indexed memory? (WAV, wavetable, granular; analysis 2026-11-03)

Question: load a WAV into the EDM and read it advancing the position (wavetable, granular). Model of the memory that
comes out of §3.3.3 / §3.3.4 [measured unless stated]:

* The table's entries are in coordinates of a **position that the hardware moves by itself** (P): the write at `A` falls
  every sample at the absolute address `P(t) + A`, and the read at `A + d` brings what was written `d` samples before.
  That is, **each write entry sweeps the whole EDM once per turn** of P (length of the turn not measured: ≥ 635 098
  samples, `0x400000`–`0x49B0DA`; the whole EDM is 2 097 152 words, below).
* The OS's `tapreq` requests `base + age − p`, with `p` its own counter (`+[E013]` per sample, wrapping at 576 000). If
  the request were relative to P like the entries, `p` would be superfluous: most likely **the request is an absolute
  address** and `p` the software copy of P [inferred; on the S8 `f5c` is the write position and the taps subtract that
  position, [Vol. II ch. 9](../vol2_dsp_core_isa/operation_uses.md) §5]. Test MP-EDMIDX (below).
* `tapreq` is only served in the program at `0x0`: the requests go in the trampoline block (§3.3.3), which can load the
  memory address (`ld r2, [...]` that our main program wrote; at the site only `r2` is free), ~5 instructions per
  request. Result one sample later, two neighbouring samples + fraction (on the S8 1/64, truncated; ≥ 24 requests per
  sample).

| use | what it needs | state |
|---|---|---|
| read at a computed position: granular, *scrub*, variable speed, reading a wavetable | `tapreq` with the address from a register, in the trampoline | **possible**: the request with an immediate age works (§3.3.3); from a register it is the same instruction. On an own line, not measured (MP-EDMIDX) |
| buffer recorded live (Clouds / Beads, looper, granular over what is played) | an own write (§3.3.4) + requests with the age of each grain | **possible already** with the measured pieces: the age grows one sample per sample with P; a grain advancing at speed r requests `age₀ + (1 − r) t` |
| freezing the buffer (*freeze*) | stop writing it: requeue its write entry with another address (`FC3C`) | untested. It lasts until the next writer of a lower address passes: with the line at `0x440000`, that of the OS's delay (`0x400000`) arrives at 262 144 samples (**2.73 s**) |
| **fixed WAV loaded from the PLUG-OUT** | (1) write it: through the ARM's indirect port (`0x40100050` / `+0x54` / `+0x800`, used by the boot self-test; untested on the S1, on the S8 the write was not achieved) or record it through the DSP in real time with data the ARM delivers at 96 words per ms (`FCEC` does not reach that: ~33 per ms); (2) that **no writer overwrites it**: they all sweep the whole EDM per turn, so the writes of the OS's delay and reverb have to be taken out (requeue their entries as free, and put back those of the OS's RAM, `0x60097928`, when leaving) and have no own lines | **open**: the hardest part; moreover the WAV fits in ~0.94 MB per region of the PLUG-OUT (16 bits: ~4.9 s at 96 kHz) |
| small wavetable (≤ 1024 words) | bank 5 + `ldb5` (§4) | simpler than the EDM; the EDM only for large tables (Plaits: 4 × 64 × 132 = 33 792 words) |

Conclusion: **granular and variable reading over a buffer recorded live, yes** (the request on an own line still to be
measured); **a fixed WAV, not yet**: the EDM has to be written from the ARM and the OS's writers switched off. Without the
ARM's port a slow filling by the DSP remains [estimated, untested]: a write entry at `A` and a read entry at `A + V`
(V = length of the turn: the same absolute address one turn before) make a line that recirculates into itself; the main
program writes the WAV sample that the ARM left in bank 5 when that address comes up and otherwise the one it read
(`ld` + `st`). The ARM delivers ~30 words per ms, so each turn fills ~1/3 of the positions: the whole WAV in ~3–4 turns
(≥ 20 s), with the OS's writers off while it lasts. V (the turn of P) has to be measured before trying it.

**The ARM's port to the EDM, read from the boot self-test** (`0x6001D51A`, OS 1.30; called only once from the boot state
machine `0x60019FB0`, a case that also initialises the EDM object `0x604BB40C`) [code, not measured]:

| step | what it does |
|---|---|
| before | ROM service `0x01008BDF(0x20003534, 1, i, 0x17, 0)` for i = 0…23 (24 routes to `0x17`), `0x0100A211(0x20003534, k, 0)` for k = 0…3 (**the 4 maps**: the windows of the DSPs, kept in the object `+8…+0x14`), `0x01009F13(0x20003534, 0x40000000)` (the argument is a float, 2.0: see §3.3.7) |
| write of word n | `[0x40100050] = 0x01000000 + 4n`; `[0x4010007C] \|= 0x100`; wait for `[0x40100008]` bit 30 at 0; `[0x40100800] = data`; `[0x4010007C] &= ~0x100`; wait again |
| between the two passes | mode `0x41200000` |
| read of word n | `[0x40100050] = 0x01000000 + 4n`; `[0x4010007C] \|= 0x100`; wait; `[0x40100054] \|= 0x20000000`; wait for `[0x40100054]` bit 31 at 0; data = `[0x40100800]`; `[0x4010007C] &= ~0x100`; wait |
| data | n = 0 and n = 2^k for k = 0…21, data = n (test of the address and data lines) |
| after | the 4 maps from the object, the 24 routes to `i → i`, mode `0x40000000` |

Consequences: **the S1's EDM has 2^21 = 2 097 152 words** (21.8 s at 96 kHz, like a BMC of the S8); the ARM writes it word
by word with busy waiting, but the OS does it **with the DSPs' maps and the 24 routes disconnected**. Using it with the
audio running (in `load`, to load a WAV) means repeating that frame: an audio cut while it lasts and the risk of
switch-off that the direct write of the table already gave (§3.3.4). Test to do before any write: a read of a few words
with the complete frame in a PARAMLOG build.

**Probe MP-EDMIDX** (built, untested): `KERNEL=sdk/s1cc/build/edm_pluck NAME=MP-EDMIDX EDM_KS=0 EDM_ECF=1 EDM_CALL=idx
sh examples/system_s1cc/build.sh`; capture `sdk/s1cc/examples/edm/idx_probe.sh`.

| piece | what |
|---|---|
| write | `EDM_ECF`: `[4034]` → `0x440000` (units of `h4`: 272) receives the left send (`master_edm` with `EDM_WR`) |
| left request | `edm_call.s edm_call_block_idx`: `h4 = 272`, `r2 = 48000/16384`, `f6 r2, -r4`, `tapreq` → `402E` |
| right request | `h4 = 272`, `r2 = 0`, `f6 r2, -r2` (the base alone: **a fixed address**), `tapreq` → `4031`; `h4` goes back to 256 before the return |
| output | dry + 0.7 × each tap, left / right |

Reading the result: left with an echo at ~48 007 = the OS's convention holds for an own line (age − p). Right − left
before the echo: **slow steps** (they change once per turn) = the request is absolute, and then a fixed position or one
advancing at any speed reads a still buffer (the basis of wavetable / WAV); **a copy of the burst a few samples away** =
the request is relative to the write position.

#### 3.3.7 The ARM's port to the EDM: probe MP-EDMPORT (2026-11-04: the port with the DSPs running **kills the audio**)

Question: can region 0 (the master's ARM) **write a table or a whole sample into the EDM** with the DSPs running, and have
it last (≥ 1 s) to read it later from the DSP? It is the path for samples and large wavetables without reducing them (an
Analog table of 92 frames = 188 416 samples; the EDM has ≥ 2^21 words).

Reading of the self-test `0x6001D51A` disassembled (OS 1.30, 2026-11-04) [code], which corrects the table of §3.3.6:
* `0x01009F13(obj, x)` (veneer `0x6003CD0E`) receives a **float**: 2.0 before writing, 10.0 between the passes, 2.0 at the
  end; elsewhere in the OS 1.0, 3.0 and 40.0. It is not a "controller mode"; probably a time (wait) [inferred, the S1's
  ROM `0x01000000` is not dumped].
* The test goes through n = 0 and n = 2^k for k = 0…21 (`lsl r6=4, i−1` with i < 0x17): **22 address lines**. If the
  test passes at boot, the EDM has **≥ 2^22 = 4 194 304 words (43.7 s)**, not 2^21 [inferred: what the OS does if it
  fails was not seen, `0x60040D5A(obj, 0x40, 0)` only clears a state bit].
* The maps are restored with `0x0100A211(obj, k, [0x604BB40C + 8 + 4k])` (`0x6001D4E4` with 0) and the routes with
  `0x01008BDF(obj, 1, i, i, 0)`.

Probe (`model.c edm_port_probe`, option `EDM_PORT` of `build.sh`, requires `PARAMLOG=1`). Step 1: the port's sequence
**without** disconnecting maps or routes (the DSPs keep sounding); every wait has a limit (it counts a *timeout* instead
of hanging the ARM).

| build | what it does at each note | PARAMLOG ids |
|---|---|---|
| MP-EDMPORTR (`EDM_PORT=r`) | reads 64 words `k · 0x10000` (k = 0…63: 4M words) and 16 words from `EDM_PORT_W0` (by default `0x380000`) | `0x3E00` (flag 0: notes, 1: timeouts), `0x3E40 + k`, `0x3E80 + i` |
| MP-EDMPORTW (`EDM_PORT=w`) | the same; at the **first** note it writes 64 words `0x5A0000 + i` at `EDM_PORT_W0 + i` and reads them back at once | also `0x3EC0 + i` |

Builds: `KERNEL=sdk/s1cc/build/mp_saw NAME=MP-EDMPORTR PARAMLOG=1 EDM_PORT=r sh examples/system_s1cc/build.sh` (the same
with `NAME=MP-EDMPORTW EDM_PORT=w`). Capture: `python3 examples/system_1/paramlog.py -t 20` while notes are played (with
the panel's DELAY up, so that the OS's delay writes the EDM).

How to read the result:

| observation | means |
|---|---|
| timeouts > 0 | the port does not answer with the DSPs running: step 2 = the frame of the self-test (maps and routes disconnected during the access, audio cut) |
| R: non-zero words that change from note to note at some `k` | there the OS's delay / reverb writes: the stretch the writers sweep and the relation between the port's `n` and the addresses of the map table (`0x400000`…) |
| R: the rest at 0 and fixed | memory nobody writes: the place for a persistent table |
| W: `0x3EC0 + i` = `0x5A0000 + i` | **the ARM writes the EDM** |
| W: `0x3E80 + i` stays the same in the following notes (seconds later) | the table **lasts**; if it is erased, an OS writer passes there (R shows which) |

**Measured 2026-11-04** (PARAMLOG capture, notes by MIDI channel 5):

| build | result |
|---|---|
| MP-EDMPORTR (80 reads per note, without restoring registers) | no timeouts, **all reads 0**; after the first note **the device goes silent** (PLUG-OUT and SYSTEM-1 mode) until power-off |
| MP-EDMPORT0 (control: the same build without the probe) | sounds, after switching off and on |
| MP-EDMPORTD (`EDM_PORT=d`: **one** read of word 0, with `0x50` / `0x54` / `0x7C` restored) | read 0, no timeouts; **silent all the same** (the whole device, until power-off) |

Conclusion: **the ARM's access to the EDM through the port with the DSPs' DMA active locks the EDM and the audio**; it is
the reason for the self-test's frame (routes and maps disconnected) and it agrees with the writes that never arrived on
the S8 (NOTES.md «Muestras largas»). W is not attempted in this form.

Registers of the controller `0x40100000` (32 words, before / after the read) [measured]:

| word | value | interpretation |
|---|---|---|
| `+0x00` | `0x45534D39` («ESM9» in ASCII), fixed | chip identifier [inferred] |
| `+0x48` | ~7 000 000 … 7 570 000, **decreases at 96 kHz** (−386 in ~4 ms between before and after; −69 749 between two notes ~0.73 s apart) | down-counter of samples: probably **P**, the position that moves the EDM (§3.3.6); its range (< 2^23) matches the addresses `0x4xxxxx` of the map table [inferred] |
| `+0x4C`, `+0x60`, `+0x78` | change by themselves (123…713, ~1250, 256…135 185) | state of the DMA [open] |
| `+0x50` | `0x01000000` before the read | the port's address register (word 0) |
| `+0x64` | `0x5FF`, fixed | [open] |
| `+0x7C` | `0x11` (sometimes `0x10011`) | control; bit 8 (`0x100`) = the ARM's request |
| the rest | 0 | — |

Step 2 (not built): the complete frame of the self-test in `load` (routes to `0x17`, maps to 0, the value of each map kept
at `0x604BB40C + 8 + 4k`, wait; access; maps and routes restored), first a single word.

Next step if W works: the WAV's table from region 0 (16 bits, ~0.94 MB per region) in the free stretch, and the read from
the DSP with `tapreq` in the trampoline (§3.3.3) at a fixed address (the form of the right request of MP-EDMIDX, §3.3.6),
up to 6 requests per sample (§3.3.5).

#### 3.3.8 A whole wavetable in the EDM: MP-EDMWT (2026-11-05, **works**: table kept and voice at the note's pitch)

Goal: a **complete, unreduced** Serum wavetable (Sys8.wav of *Echo Sound Works Core Tables / Analog*: 92 frames × 2048 =
188 416 samples, 1.96 s), kept in the EDM and read by the DSP at the note's pitch, OSC1 COLOR = position. Build:
`KERNEL=sdk/s1cc/build/mp_saw NAME=MP-EDMWT8 PARAMLOG=1 EDM_WT=1 WT_DIAG=1 sh examples/system_s1cc/build.sh` (`WT_WAV=`
another table, ≤ 262 144 samples; `WT_DIAG=0` only the voice).

| piece | where | what |
|---|---|---|
| the line | `model.c wt_load` (FC3C in `load`, entries 34–36 like EDM_ECF) | write `[4040]` → `0x440000`, read `[4041]` ← `0x440000 + L − 1`: a loop of L samples that rewrites itself (`edm_wt.s master_wt`) and therefore **lasts** (≤ 262 144: the writer of the OS's delay at `0x400000`, §3.3.6) |
| the filling | `model.c wt_fill`, in the first notes (≤ 500 ms of blocking per note) | the ARM follows the EDM's clock (word `0x40100048`, −1 per sample, **only read**: §3.3.7) with the cycle counter (1625 cycles per sample), measured drift ~3 ppm (re-read once per note); each sample k of the table goes into parameter `C034` through `FCEC`, k = samples since the change of `C035` (sync); a lost sample (interrupts) is put in another turn |
| master B | `microcode/edm_wt.s master_wt` | counter c (0 at the sync, wrap at L); if `C034` changed, it writes that sample instead of the recirculated one; phase p (dt per note in `C030`), ages of two requests (frame f and f + 1, `C031`) to bank 0 `0310`/`0311`, `0314`/`0315`; mix by `C032` (COLOR between the frames), level `C033` |
| the requests | `edm_call.s edm_wt_block` in the trampoline (§3.3.3) | two `tapreq` on the line, ages from bank 0 (`+1`: the one of this sample or the previous, whether it runs before or after the main program); results `4080`/`4088` |

Measured on the device (USB capture, 2026-11-05):

| build | what | result |
|---|---|---|
| MP-EDMWT | filling reading the counter without pause | **hang** at the first note (the register read millions of times per second) |
| MP-EDMWTC | time by the cycle counter, counter read 2 times per note | no hang; 173 928 of 188 416 in one turn; drift 5 samples / 1.96 s, 58 / 20 s → MP-EDMWTS re-reads once per note (1–2 samples) |
| MP-EDMWTD | words `40C0`/`40C1`, results `40D0` | silence: **the map table does not reach the words `40C0`+** [inferred]: use only the measured ones (`4034`–`407F` lines, `4080`–`40BF` results) |
| MP-EDMWT2 | the slaves' mix at the start (left) and at the end (right) of the main program | both: the main program runs whole |
| MP-EDMWT5 | `WT_DIAG=3`: the line receives the saw; left = its read | **echo of the note at ~1.96 s**, full level: the line of 188 415 samples works |
| MP-EDMWT7 | `WT_DIAG=4`: the main program reads `A1C0` (written by the ARM with FD6C sel 1) and `A101` (written by the OS, read by its effect) | **both 0**: the main program at `0x1000` of master B reads bank 5 as 0 → ARM → master B only through parameters (`FCEC`) |
| MP-EDMWT8 | parameters `C034`/`C035`, ages through bank 0 | left (the line alone): fundamental 46.9 Hz (= 96 000 / 2048), one recorded turn against Sys8.wav: correlation **0.90**; right (the voice): follows the notes (110, 131, 147, 196, 73 Hz), the centroid moves from 4 to 25 harmonics with OSC1 COLOR |

Pending: switch the voice off on release (it keeps sounding), envelope, more voices (≤ 6 `tapreq` per sample: 3 voices
with morph or 6 without), measure the stored table (why 0.90), filling without blocking the OS, and the self-test's frame
(§3.3.7) to load faster. Working notes: after installing, switch off and on before choosing the PLUG-OUT (choosing it
freshly installed hung twice); the USB audio lag varies: captures of 15–20 s.


## 4. Data tables (wavetables)

### 4.0 Which plug-out to use? (single-cycle oscillator vs. long table)

| Case | Plug-out | Kernel / example | Why |
|------|----------|------------------|---------|
| **Single-cycle oscillator** (one wave, or a few frames; e.g. a saw, a pulse, a Serum frame) | **slave, FIXED frame in bank 5** (`SL_WT=b5`, §4.1) | `sdk/s1cc/examples/wavetable/jp8_saw.c` (concrete example, Jupiter 8 Saw), `wt_slave.c` (template WT-SLAVE5) | **8 voices**, no EDM, no filling at the start of the note. The frame (127 points, 63 harmonics) is written **once** at `A080`–`A0FF` → perfect period. **Measured the most faithful** (2026-11-06): harmonics within 0.5 dB of the ideal saw, inter-harmonic floor ~−161 dB |
| **Long table / multi-frame wavetable with morph** (sweep between many frames, long samples) | **master's EDM** (`EDM_WT=1`, §4.4) | `sdk/s1cc/examples/wavetable/wt_edm_cmod.c` (WT-LINES8) | The whole table lives in the external delay memory (up to the size of region 0, ~0.94 MB), with morph between all the frames and per-voice COLOR MOD. Cost: ~6 voices, it fills in the first ~13 notes, 17–22 dB more noise than the ideal read |

Practical rule: if the wave is **a single one** (or two/three frames that are not swept in real time), it goes to the
**slave** with the frame **fixed in bank 5** (`SL_WT=b5`) — it is the most faithful and clean (measured, §4.1). The
recirculating loop of bank 0 (`SL_WT=b0`, 768 pts / 384 harmonics) gives more brightness but was measured **worse** (the
ARM refills it in blocks, the period is not stable → it deforms harmonics and adds noise; §4.1). If a **long table** or
samples of more than one frame with continuous morph have to be swept, it goes to the **EDM** (`EDM_WT`). The examples
carry the build lines in their header comment.

They are not delays: they are fixed data read by phase. Bank 0 does not serve (the ring would move them, and a fixed
table in bank 0 is only possible if the DSP stores nothing in bank 0 [measured on the S8]). The place is **bank 5**:

* the ARM writes the waves with `FD6C` at load and when the wave changes (§3.1); on the slave only `A000`–`A1FF` is
  reliable and the host leaves the window `A080`–`A0FF` free: one frame of 127 points + guard (§4.1);
* the DSP reads `da` (h6) / `db` (h7) with h = base + phase · N, and a second load for the linear interpolation: ~8
  instructions per voice, 8 voices [estimated];
* per-voice COLOR (COLOR MOD) no: the 8 voices share the two waves the ARM put; the morph between them is per voice.

The indexed load of bank 5 (`ldb5`) runs on the S1 slave: WT-SLAVE5, 8 voices (§4.1).

### 4.1 Wavetable on the slave: WT-SLAVE5, WT-SLAVE768 (2026-11-05, **they work: 8 voices**)

The whole table (Sys8.wav, 92 frames) lives in region 1 (`examples/system_s1cc/build/sl_wt.h`, each frame limited to 63
harmonics, 127 points + guard); the slave's ARM writes into bank 5 `A080`–`A0FF` (`FD6C sel 0`) the frame that OSC1 COLOR
asks for, a mix of the two neighbours, in `load` and each time COLOR changes (`engine.c sl_frame`). Each voice of DSP A
(`microcode/sl_wt.s sl_wt5_voice`): phase p += `s1_dt` (`A040 + v`), x = 127 p, `floor`, `hset h6`, `ldb5` Y 0 and Y 1,
linear interpolation → the voice's output (`A151 + v`); DSP B (filter + VCA of `mp_dsp_b.h`) as for any oscillator.
Kernel `sdk/s1cc/examples/wavetable/wt_slave.c`, option `SL_WT=b5`: `KERNEL=sdk/s1cc/build/wt_slave sh
examples/system_s1cc/build.sh` (with `S1_BUILD("SL_WT=b5")`). Measured: the 8 voices at their pitch (USB, 16 notes).

**Measured quality of the read: bank 5 (fixed) ≫ bank 0 (recirculating)** (2026-11-06, A/B on the device, same WAV
*Jupiter 8 Saw* = 2 frames, same note A3 = 220 Hz, filter open, DELAY/REVERB at 0; example `jp8_saw.c`). Against an ideal
saw (harmonics 1/n): **b5 = 0.5 dB mean error** (H2 −5.9 vs −6.0 ideal; H4 −12.3 vs −12.0; H7 −17.4 vs −16.9) with an
inter-harmonic floor (0.3–10 kHz) of **−161 dB** median; **b0 = 3.7 dB of error** (H2 −12.1, H4 −7.4, H7 −10.5: deformed
harmonics) with a floor of **−91 dB**. `SL_WT=b0` sounds brighter (384 harmonics against 63) but the recirculating loop
has no stable period — the ARM refills it in blocks of 64 points every 12 ms, and those seams modulate the wave — so it
deforms the harmonic balance and adds wide-band noise. `SL_WT=b5` is written only once → perfect period. **For
single-cycle: bank 5.** The only loss of b5 is the highest octave (63 harmonics).

**More points in b5: `A200` is NOT reliable (measured 2026-11-06).** The b5 frame lives by default at `A080`–`A0FF`
(128 words = 127 points + guard). The `ldb5` builds the address with a 10-bit offset (`SL_STG & 0x3FF`), so it reaches the
1024 low words of bank 5 (`A000`–`A3FF`); the host only uses `A000`–`A1FF`, leaving `A200`–`A3FF` (512 words) free in
theory. The options `SL_STG` (default `0xA080`) and `SL_N` (default 127, b5) were added and a frame of 256 points / 128
harmonics was tested at `A200` (JP8-SAW5T, `SL_STG=0xA200 SL_N=256`). Result on the device: sound comes out at the
correct pitch but the wave comes out **corrupt — "metallic"** (error 7.0 dB against 2.7 dB of the 127 one at `A080`;
peaks/dips; floor −128 vs −162 dB). **Isolated 2026-11-06 with a second build `SL_STG=0xA200 SL_N=127`** (the same number
of writes as the `A080` one that works): **also corrupt** (error 6.0 dB, floor −147 dB). Since it fails with 127 the same
as with 256, **it is not the burst of writes: it is the address `A200` itself**. The structure of the corruption is not a
simple wrap (2nd half vs 1st of the cycle: corr 0.03) but the correct shape + single words read/written wrong.
**Conclusion: the slave's `ldb5`/`FD6C` on the real device reliably reaches only `A000`–`A1FF` (512 words), not the 1024
of the emulator (`bank5_own[0x400]`).** Of those 512 the host uses `A000`–`A07F` (per-voice control) and `A100`–`A1FF`
(the DSP's stores), so the only window for the frame is `A080`–`A0FF` → **127 points is the real ceiling of b5.** For more
points: the `b0` loop (768 pts, worse quality, §4.1) or the EDM (§4.4). The options `SL_STG`/`SL_N` remain (validated at
`A080`); `make_s1cc.py` warns if `SL_STG >= 0xA200` (measured unreliable). Whoever wants to retry it: a dedicated
write-vs-read probe (read `A200` back through a path other than `ldb5`).

**`SL_WT=b0` (WT-SLAVE0L, works 2026-11-05):** the frame in a loop of bank 0 of DSP A (`SL_S=0x0040`, `SL_L=512` points,
255 harmonics), which a single `ldb5` per sample fills from the blocks of 64 points the ARM rotates in bank 5
(`A080`–`A0BF`, first place at `A0FF`, one every 12 ms > one turn of the loop); the voices read with `ldidx`. 8 voices,
periodic at their pitch. 1024 points need ~1040 consecutive words free of the stores of both DSPs (rule 6): today the
largest free stretch is `0x0018`–`0x02FF` (744); the voice blocks would have to be moved.

**State (2026-11-05): WT-SLAVE768 is the one that works** (user: 8 voices, COLOR advances from frame to frame).

| | WT-SLAVE768 |
|---|---|
| build | `sdk/s1cc/s1cc build sdk/s1cc/examples/wavetable/wt_slave1k.c` and `KERNEL=sdk/s1cc/build/wt_slave1k sh examples/system_s1cc/build.sh` (the kernel's `S1_BUILD`: `SL_WT=b0 SL_MORPH=0 SL_L=768 SL_S=0x0040 A_BLOCK0=0x0360 SL_KOUT=0x0390 SL_P=0x03A0 SL_C=0x03B0 MIX=0`) |
| table | Sys8.wav, 92 frames, each one limited to 384 harmonics and brought to 768 points (`make_s1cc.py sl_frames`, `build/sl_wt.h`, in region 1) |
| DSP A (slave) | loop of 768 in bank 0 `0x0040`–`0x0350`, one `ldb5` per sample copies the prepared block; each voice: phase, one `hset`, two `ldidx` (points k and k + 1), linear interpolation between points → `A151 + v` (`sl_wt.s sl_wt_loop`, `sl_wt1_voice`); voice blocks and own words at `0x0360`–`0x03B1` |
| DSP B (slave) | filter + VCA + envelopes of `mp_dsp_b.h`, like any oscillator |
| ARM (region 1) | `engine.c sl_tick` every 1 ms: every 12 ms the next block of 64 points of the frame nearest to OSC1 COLOR → bank 5 `A080`–`A0BF` + its place at `A0FF`; a new frame enters whole in 144 ms |
| COLOR | jumps to the nearest frame (no mixing of frames: it is what the user asked for by "morph") |
| limit | bank 0 of DSP A has 1024 words (below, WT-ALIAS): 896 points at most |

**`SL_MORPH`** (2026-11-05): `SL_MORPH=1` (WT-SLAVE0I) keeps two interleaved frames in the loop (256 points each) and the
DSP mixes them by COLOR; on the device it sounded worse than WT-SLAVE0L and when going from one pair of frames to the next
it was slow (one turn of blocks) and the half-written frame was heard. `SL_MORPH=0` (default): **one frame** of `SL_L`
points; COLOR chooses the nearest frame of the whole table (no mixing) and region 1 writes it into the loop block by block
(`engine.c sl_tick`: 64 points every 12 ms → 1024 points in 192 ms, 512 in 96 ms). Linear interpolation only between
neighbouring points of the frame (`sl_wt1_voice`, two `ldidx`).

| build | loop | addresses | state |
|---|---|---|---|
| WT-SLAVE512 (`wt_slave1k.c` + `NAME=WT-SLAVE512 SL_L=512 A_BLOCK0=0x0300 SL_KOUT=0x0330 SL_P=0x0340 SL_C=0x0350 B_OUT0=0x0400`) | `0x0040`, 512 | those of WT-SLAVE0L (all proven) | **works** (user, 2026-11-05): 8 voices, COLOR advances by frame |
| WT-SLAVE1K (`sdk/s1cc/examples/wavetable/wt_slave1k.c`) | `0x0040`–`0x0450`, 1024 | voice blocks of DSP A `0x0460`, `SL_KOUT` `0x04A0`, `SL_P` `0x04B0`, `SL_C` `0x04C0`, outputs of DSP B `0x04D0` (options `A_BLOCK0`, `B_OUT0`): **new** for DSP A (DSP B stored at `0x0400`–`0x05FF` in C-S1SY / MP-CSAW2-8) | **does not work** (capture `work/captures/wt_slave1k_184127.wav`): level ~1/10, no period at the note's pitch (4–5 % harmonic energy), COLOR without effect |
| WT-SLAVE768 (`wt_slave1k.c` + `NAME=WT-SLAVE768 SL_L=768 A_BLOCK0=0x0360 SL_KOUT=0x0390 SL_P=0x03A0 SL_C=0x03B0`) | `0x0040`–`0x0350`, 768 | everything of DSP A below `0x0400`; `0x0380`–`0x03B1` new (promoted) | **works** (user, 2026-11-05) |

**Bank 0 of the slave's DSP A has 1024 words** (measured, WT-ALIAS 2026-11-05): the addresses wrap at `0x0400`. WT-ALIAS
(WT-SLAVE512 + `SL_ALIAS=1`): DSP A stores 1.0 at `0x0770` every sample; voices 0–3 multiply their output by `[0371]`, 4–7
by `[0771]` (control); 8 held notes one after another: the level rises 8 times the same (0.0016 · √n: 0.0016 … 0.0045,
`work/captures/wt_alias_*.wav`), that is `[0371]` = 1.0 = what was stored at `0x0770`. That is why WT-SLAVE1K failed (its
words at `0x0460`–`0x04C0` fell at `0x0060`–`0x00C0`, inside the loop) and WT-SLAVE0 (`0x0360` + 1024) overwrote itself.
The stores of DSP B do not reach that bank: in WT-SLAVE0L DSP B stores at `0x0600`–`0x0742` (= `0x0200`–`0x0342` with the
wrap, inside its loop `0x0041`–`0x0250`) and it sounded fine. Maximum loop: `0x0018`–`0x03FF` (1000 words) minus the words
of DSP A (82 with 8 voices) minus 17 → 896 points (14 blocks of 64). `make_s1cc.py` rejects a loop outside
`0x0018`–`0x03FF` and compares the words of DSP A modulo `0x0400`.

`make_s1cc.py` rejects the loop if it falls on any store of the two DSPs (voice blocks of A and B, outputs of B, phases,
counter). Immediates of `mul` > 256.0 have no encoding: `x = p · 256 · (SL_N / 256)`.

**Input WAV** (`wt_samples`, EDM_WT and SL_WT): only whole frames of 2048 from the start are taken; the partial tail and
the frames that do not fit (EDM: 262 144 samples = 128 frames; SL_WT: `SL_TBYTES`, 640 KiB of frames in region 1 = 319
frames of 1024 points) are discarded with a warning; the normalisation is over what remains.

Per-voice COLOR MOD: not with a single frame in the loop (all voices read the same one); it needs the whole table within
reach of the DSP (the EDM, §3.3.8: ≤ 6 `tapreq` per sample) or several frames in bank 0 (fewer points).

### 4.3 Master B → slave through `600E`–`6015`: WT-M2S (2026-11-05, **works with `MA_MAIN`**)

Goal: the whole wavetable in the EDM read by master B (one `tapreq` per voice) and each voice's sample sent to the slave,
which applies filter and amp to it as today (§3.3.8 + §4.1). First, the transport master B → slave.

Test (`M2S_PROBE`, on WT-SLAVE768): master B stores 1.0 at `600E`/`6010`/`6012`/`6014` and 0.5 at `600F`/`6011`/`6013`/`6015`
every sample; the slave's DSP A multiplies voice v by `[600E + v]`. USB capture: 8 held notes one after another
(48 … 72), amplitude of the fundamental of each new note (`work/captures/wt_m2s*.wav`).

| build | `0x0100` of master A | master B writes | the slave reads | result |
|---|---|---|---|---|
| WT-M2SREF (`M2S_PROBE=2`) | what was left | yes | no | normal: the 8 notes, even fundamentals (420–540) |
| WT-M2S, 1st time | what was left | yes | yes | ~15 times lower, the note alone an octave up: what is read is a signal at the voice's pitch |
| WT-M2S, 2nd and 3rd time (other boots) | what was left | yes | yes | **digital silence** (what is read = 0); then PLG1 silent until WT-SLAVE768 is reinstalled |
| WT-M2SR0 (`M2S_PROBE=3 MA_MAIN=0`; boot, one note in SYSTEM-1 mode, PLG1) | what was left | no | yes | digital silence (0) |
| **WT-M2S + `MA_MAIN=1`** | **ours, empty** | yes | yes | **the 8 notes, alternating fundamentals 453 / 949 / 428 / 918 / 449 / 894 / 475 / 1032: ratio ~2.05 = 1.0 / 0.5** |

Conclusion [measured]: **master B does reach the slave through `600E`–`6015`** (plain ld / st, without enabling anything:
like the PROMARS, whose master B writes `6010` and its slave reads it in the main program of one DSP and in section
`0x1900` of the other, `system-1/s1_progs.py`). What prevented it is **the main program of master A at `0x0100`, which we
never loaded**: in PLUG-OUT mode the OS queues on master A only its output program at `0x82` (126 B, it ends exactly at
`0x100`) and the closings; the DSP continues at `0x0100`, where what the previous mode loaded remains, and that program
writes those words after master B (0 at rest; a signal at the note's pitch the time it sounded). That it is the
modulation program of the stock synth (696 instructions, writes `600E`–`6021` and bank 5 `A100`–`A113`) is [inferred]:
the content was not read. Roland's PLUG-OUTs load their own `0x0100` (`FC30 {0x100, 0x6DF2, 0x7FF2}`).

What is done from now on: **`make_s1cc.py` always loads an own empty main program on master A** (`MA_MAIN=1`, by default;
`s1ma0` = start + Roland's end, 7 half-words; `model.c set_master_a`, `FC30` in `load` before master B; `MA_MAIN=0` = as
before). Besides the bus, this way master A stops running a foreign program (cycles and stores to bank 5 that we did not
know about). Pending in the other S1 tools that do not load it either (`examples/system_1`, `system_1_synth`,
`system_1_ramp`, `mutable_system`; `system_1_jx` does load its own). The previous SYSTEM-S1CC builds were tested without
it; a new build of an already tested kernel must be tested again.

Next: the EDM → slave scheme (master B: one `tapreq` per voice → `600E + v`; slave: filter + amp).

### 4.4 Whole wavetable in the EDM, voices on the slave: WT-EDM (2026-11-05, **works**)

The table of §3.3.8 (all of Sys8.wav, 92 frames × 2048, in an own loop of the EDM) with the voices on the slave, to have its
filter and its amp: `sdk/s1cc/s1cc build sdk/s1cc/examples/wavetable/wt_edm.c` and `KERNEL=sdk/s1cc/build/wt_edm sh
examples/system_s1cc/build.sh` (`S1_BUILD`: `EDM_WT=1 WT_SLAVE=1 SL_WT=m MIX=0`).

```
region 0 (model.c)  assigner of 6 voices (NV = WT_NV); at each note voice v receives its phase step at C030 + v
                    (wt_voice); COLOR → C036 = the first sample of the nearest frame (no mixing); the first notes
                    fill the loop (wt_fill, <= 500 ms each)
master B (edm_wt.s master_wt_s, 353 instr.)  the loop as master_wt; per voice ONE tapreq (age of c - (frame + phase))
                    from the block of the OS's effect (edm_call.s edm_wt_block, 6 requests); the result of the previous
                    sample, interpolated (x0 + (x1 - x0) fr) → 600E + v; then the slave's mix 6006 / 6007 ×
                    VOLUME → sends of the effect, like master.s
slave A (sl_wt.s sl_m2s_voice)  [600E + v] → the voice's output (A151 + v)
slave B             filter + amp + envelopes of mp_dsp_b.h, like any oscillator
master A            our empty main program (MA_MAIN, §4.3): without it nothing arrives from 600E..
```

Measured (USB capture `work/captures/wt_edm_192818.wav`, 8 held notes one after another): note 48 alone at 130.79 Hz
(130.81 expected), periodic (correlation 0.988); each new note appears with its fundamental (the 7th and 8th steal voices:
6); on release, silence (the slave's amp closes the voice: what was missing in MP-EDMWT8).

**Incident:** the first time the freshly installed PLUG-OUT was chosen and notes were played, **the device switched
off**; after switching it on, the first notes were slow to start (the filling blocks up to 500 ms per note) and then it
worked. Cause not determined. A direct store of the ARM into the map table causes such a switch-off (§3.3.4); here the table is loaded with `FC3C` in `load`, as in MP-EDMWT8, which did not switch off. Differences with
MP-EDMWT8: the own main program of master A (`MA_MAIN`), 6 requests per sample (MP-EDMWT8: also 6, 3 voices × 2), the
assigner of region 0 instead of its own. Working rule that already existed (§3.3.8): after installing, switch off and on
before choosing the PLUG-OUT.

**Alternating sound (measured 2026-11-05) and its cause.** With a held note the timbre alternated every ~94 200 samples
between the requested frame and one ~45 frames before (frames 88–89 ↔ 44–45, `wt_edm_hold_*.wav`), and freshly switched on
(table half filled) between note and silence (`wt_edm_boot_*.wav`). Tests:

| build | what | result |
|---|---|---|
| WT-AGE (`WT_AGEFIX=40000,120000`) | voice 0 reads at fixed age 40 000, voice 1 at 120 000; left/right directly from master B | right = left delayed by **80 000** (correlation 0.87): the EDM reads large ages fine |
| WT-AGE2 (`WT_DIAGA=1`) | left voice 0, right the age it requested / 16 | the age jumps −0.3593 × 16 = **5.749 units every 94 176 samples**: `c` wraps at half |

Cause [measured]: the constant of the limit of `c`, `mov r3, #11.49609` (`WT_LEN / 16384 − 2^−15`, encoded `4b7f`), **ran
on master B as 5.748** (half); the assembler and the emulator read it as 11.496 (that is why `wt_sim` / the simulation of
ages were right). With `c` of half a turn, half the time the age pointed half a table off. The same was in MP-EDMWT8
(probable cause of its correlation 0.90). Fix: the limit with one op, `mov r3, #11.5` (`4b80`) + `addxy r3, r3, #−2^−15`
(also exact; `#11.49609` gives 64 samples fewer per turn). **Verified** (`wt_edm_fix_194906.wav`, note held 10 s):
level and timbre even the whole time (centroid 516–564 Hz, no jumps), periodic (164.67 Hz, correlation 0.96). Which 16-bit
immediates the device decodes differently is not measured: `#127.0` (slave) and `#5.75`, `#0.125` (master B) run fine;
`4b7f` (mantissa with its 7 low bits at 1) does not.

**Clicks every 2^16 samples and at each turn (measured 2026-11-05).** With the limit `mov #11.5` + `addxy #−2^−15` the voice
jumped phase every **65 536 exact samples** (`wt_edm_fix_194906.wav`, note 40); with fixed ages (WT-AGE) there are no
jumps: it is the count of `c`, again a constant the device reads differently from the emulator. Fix: **the limit and the
addend of the wrap as parameters** that the ARM writes in exact float (`C038` = (LEN − ½) / 16384, `C039` = LEN / 32768;
`model.c wt_limits`, in `load` and at each note), without immediates. WT-AGE3 (ramp of the age): the whole turn, without
2^16 jumps; a large error (0.44) remained right at the change of turn of the age → **margin**: the ages go from 256 to
LEN + 256 (the copy of one turn before; never an age of a few samples). WT-EDM with both (`wt_edm_v3_200124.wav`): note
alone at 130.79 Hz, correlation 0.992, chord without jumps; small transients remain (residual ~0.002) at the change of
turn, every ~1.97 s, and one of half a frame (1024 table samples) once. The user accepts it. Master B: 379 instructions
(MP-EDMWT8: 340).

Rule this leaves: **on master B, no constant that is not "round" in programs that the emulator cannot validate against
the device**; exact values go through parameters (`FCEC`) or are built with ops from tested round values.

**Per-voice COLOR MOD: WT-EDM-CMOD (2026-11-05, works, 6 voices).** `sdk/s1cc/examples/wavetable/wt_edm_cmod.c`
(`WT_CMOD=1`): the kernel reads `s1_color(0)`, so the host builds the COLOR of each voice with its COLOR MOD source (knob +
LFO / envelopes / sub); the slave's DSP A takes it to a position of the table (`sl_wt.s sl_m2s_voice`: color × (frames −
1), step `WT_CQ`) and sends it to the master through **`6008 + v`** (slave → master, 6 words; DSP B does not write
there); master B uses it instead of `C036` for that voice. `WT_CQ`: 1 = by frame, 2 = half, 4 = quarter, 0 = by sample
(the cycle is the end of frame k and the start of k + 1, the boundary moves with COLOR: continuous, with a small jump at
the joint if the frames differ).

| build | what | result |
|---|---|---|
| WT-EDM-CMOD (Sys8) | COLOR MOD = LFO, 6 notes one after another | 5 voices modulate, **one does not** |
| WT-CMOD-VTAG (`WT_VTAG=1`: voice v at (v + 1) / 8) | which voice it is | the one with the lowest level: **voice 0** |
| WT-CMOD-ROT (`WT_S2MROT=1`: voice v through 6008 + (v + 1) mod 6) | the word or the voice? | it follows voice 0 (voice 5, now through 6008, modulates) |
| cause | `SL_KOUT` (the kernel's own outputs, which nobody reads) at `0x0700 + 2v`: with the wrap of bank 0 of DSP A at `0x0400` (§4.2 rule 6), `0x0708` = `0x0308`, the COLOR word of voice 0 → the kernel overwrote it with ~0 | `SL_KOUT` → `0x0260` (proven, C-S1SY) |
| WT-CMOD-VTAG2 | the fix | **the 6 voices modulate** (change of shape 0.25–0.27), levels 0.12 … 0.75 in order |

Tables: Sys8 is the most stepped of the folder (mean change between frames 0.148); **Circle PWM** (255 frames, 127 fit: it
compresses well) has steps 10 times smaller with a large sweep: WT-CMOD-PWM (by frame), WT-CMOD-PWMQ (quarter),
WT-CMOD-PWMS (continuous). **Chosen: by frame (`WT_CQ=1`, the default).** WT-CMOD-PWMS sounded much worse (user; held note
`wt_pwms_203312.wav`: no period at the note's pitch, correlation 0.28, the harmonic part between 1 % and 62 % with a cycle
of one turn of the table), while by frame the same note is 98.6 % harmonic (`wt_vtag2_203000.wav`). Cause of the
continuous one not measured.

**Quality against an ideal wavetable (measured 2026-11-05, MI-WT = WT-EDM-CMOD with `80's PWM.wav`, 31 frames).** Capture
`work/captures/wt_pwm80_204717.wav` (note 48 held, without modulations; level −52.7 dBFS rms). Reference: the frame in a
loop, band-limited. Results: frame **12** sounds (because of the position of COLOR; 0 does not match); pitch 130.783 Hz
(−0.40 cents); the strong harmonics 1–19 match the frame within ±0.1–1 dB. Deviations: (1) the frame's null harmonics (3,
6, 9, 12, 15 …, −37 dB) come out +7 … +14 dB; (2) **non-harmonic noise −24.5 dB** (the model of linear reading + 1/64
fraction gives −59.4 dB; without interpolation −49; inverted fraction −43; x0/x1 one sample earlier −39 … −42: none
explains it); the waveform **changes slowly and repeats every 50 cycles** (0.382 s; window against window 50 cycles later
−52 dB, 1–13 cycles later −35 … −19 dB); around each harmonic there are sidebands at ±5.77 Hz and a comb of **1.512 Hz =
96000 / 63488 (one turn of the EDM loop)**; the time shift per cycle has a component at 5.7 Hz (standard deviation 0.42
samples). A fixed table, read like the code, does not reproduce it (least-squares fit of the 2048 points: residual
−23 dB): the content or the position read changes with time. Causes [measured with `WT_DIAG=1` in `master_wt_s`: left =
the loop `[4041]`, right = the read of voice 0 at `600E`; WT-DIAG1 `wt_diag1_210942.wav`, WT-DIAG2 `wt_diag2_211255.wav`]:
(a) **the loop measured WT_LEN + 1** (63 489, exact repetition) with the read at `WT_A + WT_LEN − 1`: the loop is the
distance + 2. With `WT_RDIST = WT_LEN − 2` it measures 63 488 exactly → now it is the default. The table moved one sample
per turn relative to `c` (hence the comb of 1.512 Hz and the slow change). (b) **The filling (`model.c wt_fill`) loses ~7 %
of the writes**: isolated holes at 0 every ~9–10 samples, correct neighbours (two `FCEC` in the same DSP sample);
`wt_done` marks them anyway, so they stay forever. The following notes filled 700 holes correctly, 90 with the
neighbour's value and **overwrote 1865 good samples** (writes out of time). (c) One note fills up to frame 23 (≤ 500 ms).
The written samples match the WAV at −33 dB (after the DC block of the output). Pending: a filling that does not depend on
the ARM's timing. Separately, the aliasing of the linear reading of an unfiltered table (computed): note 72 −48 dB, 84
−30 dB, 96 −22 dB; with the table band-limited per octave (mipmaps), ≤ −59 dB in all.

**Memory validations in `make_s1cc.py`** (errors, except where stated; added 2026-11-05 after this case): `bank0_wrap_check`
(two addresses of the slave's DSP A that coincide modulo `0x0400`), `collision_check`: DSP A in `0x0000`–`0x0017` of the OS
(modulo `0x0400`); DSP B the same as a warning (its size is not measured; addresses already proven do not warn); slave
bank 5 written by both DSPs; IOMD with more than one writer or in the wrong direction (slave → master `6006`–`600D`,
`6026`–`602D`; master → slave `600E`–`6021`); master B on the words of the OS's effect (bank 0 `0000`–`0033`, bank 2
`4000`–`4033`). Plus those that already existed: unproven addresses (warning), double bank-5 stores by DSP A (rule 4), the
`SL_WT=b0` loop.

**Indexed filling and "tuc" (2026-11-05, MI-WT).** `WT_FILLIDX=1` (default): the ARM sends pairs (value, index) in
`C03A`/`C03B`, `C03C`/`C03D` (value first); master B writes the loop only if `c` = index (`m = 1 − sign(i − c)²`); 9 turns
of filling (k mod 3, lead 3 / 6). Measured: held notes with a cycle-to-cycle difference of −40 … −50 dB (old filling −12 …
−29). `wt_voice` sends the note's step BEFORE the filling (it arrived 40–350 ms late, behind the filling's parameters).
The "tuc" ~1–2 s into some notes was **`sign(0) = 0`** at the wrap of master B's phase: with `p + dt` exactly at `WT_F`,
`(sign + 1) F/2` subtracted half a frame → the voice was shifted by 1024 table samples (measured `wt_long_235143.wav`,
note 52, 1022 of 2048). Fix: `sign + sign²` (2 / 0 / 0); the same at the wrap of the age (`sign² − sign`). Rule: **in the
DSP `sign(0) = 0`; every decision with `sign` must give a valid value also at 0**. Verified (`wt_sign5_235825.wav`, notes
48/52/55/59 of 2.3 s): no jumps within the note, cycle-to-cycle difference −42 … −58 dB (the best measured). On the way:
`mul r5, r3, r3` (destination ≠ source) and a `mov r5, r3` stuck to `sign r3` (latency) gave −10 / −19 dB; `make_s1cc` now
warns about the form `mul rD, rX, rX`; the latency risks of the hand-written master B are not checked. WT-NOCMOD
(`WT_CMOD=0`) does not follow COLOR (it stays at frame 0–2) and sounds noisier: a separate error, open.

**Aliasing with COLOR near 0 (measured 2026-11-06, MI-WT, `80's PWM.wav`).** Captures `wt_alias_000104.wav` /
`wt_alias14_000244.wav` (frame 0, identical to the decimal: deterministic, it is not the USB) and `wt_sign5_235825.wav`
(frame 14). Total non-harmonic power:

| frame | measured | model (linear, 1/64 fraction, unfiltered table) | model with the band-limited table |
|---|---|---|---|
| 14 | −46 … −48 dB | −60 … −66 dB | −63 … −70 dB |
| 0 (notes 48–84) | −22 … −26 dB | −29 … −48 dB | −62 … −66 dB |

Origin: (1) frame 0 is a thin pulse, with 16 dB more energy above harmonic 100 than the other frames (−39 dB against −55);
the linear reading of the unfiltered table at the note folds back those harmonics; (2) the device stays 17–22 dB worse than
that model in both frames, and the excess grows with the high content of the frame (cause not measured: the interpolation
fraction of `tapreq` or the precision of the EDM; no simulated variant of the reading — without interpolation, delayed or
inverted fraction — reaches what is measured). It is not vibrato (the pitch varies < 0.1 cent). Planned fix (not done,
postponed by the user): tables band-limited per octave.

**Morph with one read per voice (measured 2026-11-06).** The folder's Serum tables are continuous sweeps (joining half a
frame = mixing, error 0 in those of 150–256 frames; 80's PWM 2.2 %), so a FIXED position between frames sounds fine
(WT-CONT `WT_CQ=0`, COLOR still: −35 … −39 dB non-harmonic). But **moving** the position shifts the wave in time: it is
reading faster or slower = a change of pitch (Doppler); with COLOR MOD (LFO, 8 frames in ~0.35 s) the cycle moved by up
to ±36 samples per cycle (`wt_cont5_002159.wav`). With one read per voice, only whole-frame steps keep the phase; a real
modulable morph needs two reads and a mix (3 voices). Also: COLOR + COLOR MOD goes out of 0..1 → position outside the
table → silence with pulses: the slave clips it (`sl_wt.s`, `max`/`min`). `WT_FLATCH=1` (default): master B takes the new
position only at the start of the voice's cycle (state `0x0320 + 4v`); `WT_FLK` = approach per cycle, **1** (capture
only): values < 1 leave fractional positions = Doppler. MI-WT reinstalled with `WT_CQ=1` + capture, not verified.

**Morph by ¼ frame without Doppler: WT-LINES (`WT_LINES=1`, 2026-11-06).** Master B writes every sample 3 more lines with
the table mixed ¼, ½, ¾ towards the next frame at the same phase (`x` = sample `c` of the loop, `xn` = `[4043]`, fixed read
at `RDIST − WT_F`; writes `[4044]`/`[4046]`/`[4048]`, entries 34–39 of the map table). The slave sends at `6008 + v`
`u = k/8 − j·WT_LS/16384 − (LEN + 512)/16384` (k frame, j quarter): master's age never wraps and falls in line j one turn
older (each line keeps 2 turns + 770; the lines are remade the same each turn). The older ages read **higher**
addresses. Measured: (1) with the separation and the frame/line split on the master, 640 instructions → **hangs** (481
ran; master B's budget between 514 and 640); (2) a line offset through the words `6026 + v` always arrived as 0 (no
slave → master connection, probable); (3) with `WT_LS = 0x20000` the quarters appear (`wt_lines3_004021.wav`) but with
loose faults: line 2 fell on the writers of the OS's reverb (`0x48CA00`, `0x48F340` …). Now `WT_LS = 0x60000`
(`0x440000`, `0x4A0000`, `0x500000`, `0x560000`; the EDM has 2 097 152 words and wraps): **works** (user, 2026-11-06;
`wt_lines4_004456.wav`, COLOR MOD with LFO: steps of ¼ frame, pitch ±5 cents, without the absurd positions; non-harmonic
−13 … −28 dB, worse where the morph moves fast). Table ≤ ~65 000 samples.

**Eighths (`WT_LSUB=8`, 2026-11-06, **works**: user; `wt_lines8_004950.wav`, COLOR MOD with LFO: steps of ⅛, pitch ±5
cents, cycle against the average of its neighbours −41 dB median, 7 % of the cycles worse than −20 dB = the fast steps of
the LFO; floor of the still note −31 dB per 25 ms window):** 7 lines, `WT_A + j·0x20000` and `+ 0x20000` more from line 2
(`0x460000`, `0x4A0000` … `0x540000`: outside the reverb), writes `[4044]` … `[4050]`, entries 34–43; slave: `q` in eighths
and the jump of line 2 with `max`/`min`; master B 508 instructions. `WT_LSUB=4` (default) generates the same as the
version that works (496 instructions, same addresses).

**More precision than ⅛ by reusing lines (analysed, discarded 2026-11-06).** Each morph line keeps a fixed *mix ratio*
(`j/WT_LSUB` towards the next frame), all read at the **same phase** (that is why there is no Doppler when changing line).
The resolution is the number of distinct ratios available **at the same instant** = the number of lines. Rewriting a line
"in a circle" (the EDM's ring) adds no ratios: each line is already a ring that keeps 2 turns of *its* ratio so that the
request's age falls on an identical copy; overwriting line 0 with ratio 1/16 would erase ratio 0 — both cannot be had at
once without one more line. That is, the ring gives the time history of *one* ratio, not several ratios. Real paths to
raise the precision: (a) **more lines** (`WT_LSUB=16`, 15 lines) — held back by the EDM space (≈10 free lines between
`0x440000` and `0x600000`), the write words of bank 2 and master B's budget; (b) **interpolate between the two neighbouring
lines when reading** (same phase → no Doppler, continuous resolution with the same number of lines) — it costs a second
`tapreq` + a MAC per voice, which lowers the voices (≈6 → 3, like the continuous 2-read morph above). Today the audible
limit is not the ⅛ step but the noise floor (below), so that is worked on first.

**Noise floor: the table at double scale in the EDM (`WT_SCALE2=1`, built 2026-11-06, not measured on the device).** The
table is stored today at **half scale**: `wt_fill` writes `wt_i2f(wt_table[k], 16)` = peak ±0.5 (the WAV normalised to ±1
→ int16 ±32767 → /65536). The device stays 17–22 dB worse than an ideal read and the floor of the still note is −31 dB per
25 ms window (`WT-LINES8`): if the read of the delay memory has a fixed quantisation floor (not proportional to the
signal), storing at full scale uses one more bit and lowers that floor ~6 dB. `WT_SCALE2=1`: the filling writes at `2^-15`
(peak ±1.0, full scale) and the **slave** divides by 2 again at the voice's input (`sl_wt.s sl_m2s_voice`, before the
filter of DSP B), so level and headroom do not change and the `×0.5` is fused into a VLIW gap of the slave (`s1m0.s`,
`mul r0, r0, #0.5 | … | st r0, [a151]`): **master B stays at 508 instructions**, identical to the one that works. The mix
of lines and the interpolation of `tapreq` stay between the two ends (≤ ±1.0): there is no new clip in the EDM; the
slave's filter sees ±0.5 again. Build: `WT_WAV="…/80's PWM.wav" WT_LINES=1 WT_LSUB=8 WT_SCALE2=1 NAME=WT-LINES8S
KERNEL=sdk/s1cc/build/wt_edm_cmod sh examples/system_s1cc/build.sh`. Measure A/B (same WAV, same note, COLOR still):
capture a held note with `WT-LINES8` and with `WT-LINES8S`, `python3 wt_noise.py before.wav after.wav 48`
(cycle-vs-neighbours and floor per 25 ms; `wt_noise.py`/`wt_capture.sh` of the session's scratchpad, rewrite them if
lost).

**MEASURED 2026-11-06 on the device (note 48 held, COLOR still): `WT_SCALE2` makes it MUCH WORSE — discarded.** `WT-LINES8`
(±0.5): fundamental H1 = 0 dB, normal harmonic series, inter-harmonic noise −76 dB, 25 ms floor −36 dB median / −30.5 dB
worst. `WT-LINES8S` (×2): **fundamental H1 = −79 dB (it disappears)**, only **even** harmonics (H2 = 0, H4 = −8, H6 = −13;
odd ≈ −80 dB), half the wavelength (the period detector gave the octave, 262 Hz), rms at half, inter-harmonic noise
−61 dB, 25 ms floor −15 dB / −10 dB (**+22 dB worse**). It is the signature of **saturation/wrap**: storing at full scale
(peak ±1.0) wraps the samples near the top in the fixed-point format of the delay memory → only even harmonics + noise.
Conclusion: the EDM does **not** have spare bits; half scale (±0.5) gives the headroom the path needs, and the 17–22 dB of
excess noise do **not** come from wasted bits. `WT_SCALE2` stays in the code (default 0) in case it serves with another
table; **do not use it at ×2**. The only thing left to try from this idea: a **moderate** boost (e.g. `2^-15.5` ≈ ×1.4,
peak ±0.7) that does not reach the wrap, in case the floor is fixed and gains ~3 dB; such a marked wrap at ×2 suggests
little margin.

**Tables band-limited per octave: discarded with the morph by lines.** The planned fix for the aliasing (above) asked for
a filtered copy of the table per octave. With WT-LINES each version of the table is an EDM line that keeps 2 turns
(~128 000 samples with 31 frames) and, for the morph, `WT_LSUB − 1` more lines: (7 octaves × 8 lines = 56 lines) against
the free space between `0x440000` and `0x600000` without the OS's reverb (≈ 1.3 M words ≈ 10 lines), the write words of
bank 2 (`4040`–`407F` before the results of `tapreq`) and master B's budget (~3 instructions per line; 514 ran, 640 hung).
Moreover the morph lines are remade in the DSP from the table with two fixed reads, but filtering to the octave is a long
FIR that the DSP cannot do per sample, and filling each octave by the ARM (`FCEC`, ~6 s per table) does not scale. Only a
compromise remains open: ONE filtered table for the high range instead of the morph lines.

**Possible improvement (not done): longer tables in several lines.** Today the table lives in ONE line of the EDM
(`WT_A = 0x440000`, loop of `WT_LEN`); the 6 voices read from that same line with one `tapreq` each (6 per sample = the
measured limit, §3.3.5). A line does not go beyond `WT_GAP` = 262 144 samples (2.73 s: the writer of the OS's delay at
`0x400000`, §3.3.6). For more, split the table into several looped lines (each with its write/read pair in the map table,
like those of `EDM_STR` at `0x440000 + 0x4000 v`, loaded with `FC3C`), one after another: the recirculation goes through
the entries of the map table, not through `tapreq`, so it **costs no voices**; the voice keeps one `tapreq` and chooses the
line by the base of its request. Open: how many map entries and how much EDM remain free, and the size of the table in
region 0 (≈ 0.94 MB per region, from the project's notes on long samples: ≈ 470 000 int16 samples ≈ 4.9 s), which limits
before the EDM.

Pending: the instability (hangs when going from SYSTEM-1 mode to PLUG-OUT, one switch-off; suspect: the live patch of
`edm_call` into master B's program memory during `load`) (the per-voice frame instead of the global `C036`: master B already
has the per-voice age); the filling without blocking the OS; 7th and 8th voice (no: 6 `tapreq` at most).

### 4.2 Rules of the indexed loads and of bank 5 on the S1 slave (measured 2026-11-05)

None of these appears in the emulator (model of the SYSTEM-8); they cost the tests WT-SLAVE5* / IDX-PROBE*:

| # | rule | evidence | where it is applied |
|---|---|---|---|
| 1 | several `hset` per sample work (`h6` and `h7`; `ldb5` and `ldidx`), each with its value | IDX-PROBE (tail: 6 `ldb5`, 4 `ldidx` exact); IDX-PROBE2 (8 pairs `hset` + `ldb5` Y 0 / Y 1 exact) | — |
| 2 | an indexed load uses the register its `hset` named **as it is at load time**: do not write it between the `hset` and the last indexed load (nor in the same instruction) | IDX-PROBE head: two loads read the index of another value of the register; WT-SLAVE5: `mov r6, #1.0 \| ldb5 r1, 0` | `asm/dspsched.py` (`hset_src`, `HSET_HOLD`) |
| 3 | a single indexed load per instruction: `ldidx r1, 0 \| ldidx r2, 1` gives both the word of the second | IDX-PROBE (bank-0 pair: 0.757 / 0.752 for 0.625 / 0.75) | `asm/dspsched.py` (`Ins.fits`) |
| 4 | **DSP A and B of a slave run at the same time**: a bank-5 word that A writes twice per sample (the kernel's 0, then the wave) can be read by B in between. A single store per sample on each shared word | WT-SLAVE5 with the kernel writing `A151 + v`: only the voice that B reads before the 0 sounded (voice 0; with the order reversed, the first two and the last two); with the kernel in bank 0 (`SL_KOUT`): the 8 | `make_s1cc.py addresses` (`SL_WT`) |
| 6 | **bank 0 of DSP A has 1024 words: the addresses wrap at `0x0400`** (measured, WT-ALIAS). Every line / table of bank 0 must stay in `0x0018`–`0x03FF` and outside the stores of DSP A modulo `0x0400`; those of DSP B do not touch it | WT-SLAVE0 (loop `0x0360` + 1024: overwrites itself), WT-SLAVE0L, WT-SLAVE1K, WT-ALIAS (§4.1) | `make_s1cc.py` (`SL_S`, `SL_L`) |
| 5 | the distance `hset` → load 2 is enough (8 changed nothing); the loads and the SFU with distance 1 are fine | WT-SLAVE5B (`HSET_DIST=8`), WT-SLAVE5L (`LAT2=1`): the same as without them | `HSET_DIST`, `LAT2` stay as test options |

Master B (§3.3.8): our main program at `0x1000` reads bank 5 as 0; `sign` only as `sign rX, rX` (`asm/master_b_forms.txt`).

**Proven addresses** (`asm/proven_addresses.txt`): the `ld` / `st` addresses that a build which ran on the device used,
per DSP (`sA` / `sB` slave, `mB` master B), with the build as evidence. `make_s1cc.py` warns on each build about every
address outside the list (`warning: s1m0: 0200..020f never ran on the device`) and about every shared bank-5 word that
the slave's DSP A stores more than once per sample (rule 4). After checking a build on the device: `KERNEL=…
OUTDIR=examples/system_s1cc/build python3 examples/system_s1cc/make_s1cc.py --promote-addresses NAME`. The tables read by
index (`hset`) are not seen there: they must fall in those ranges.

### 4.5 Loading a frame over USB live: `SL_LOAD` (2026-11-06, **UNTESTED on the device**)

Changing the wavetable of the fixed frame (`SL_WT=b5`, §4.1) **live, without reinstalling**: the PC sends the 127 samples
of the frame by MIDI and the DSP changes table when the writing finishes. The live write mechanism of bank 5 is already
proven — it is the same `b5()` / `FD6C(sel 0)` with which `sl_frame()` rewrites the frame when OSC1 COLOR moves (§4.1),
and the same as `re/dsplive.py` (`Link.write_b5`) does on the SYSTEM-8. What is new is the receive handler in region 1
and the PC tool.

**Two stages, on purpose** (`examples/system_s1cc/wt_load.h`, reusable): the transport first fills a buffer in the **ARM's
RAM** (`wtl_buf`, Q15) and a *commit* command dumps it to the destination with `WTL_COMMIT(buf, n)`. Today the
destination is bank 5 (`sl_commit()` in `engine.c`: `b5(SL_STG_IDX + i, sl_bits(buf[i]))`); the same loader serves later
for the master's **EDM** (§4.4) by defining another `WTL_COMMIT` that writes the delay line from the ARM.

**Transport** (the protocol of `re/dsplive.py` ported to the SYSTEM-1, without touching region 0): for each word, a
*pitch bend* carries 14 data bits and a CC1 (modulation) carries the 7-bit command. Region 0 already forwards the bend →
voice parameter `0x130` and CC1 → `0x131` (`model.c` `a_bend` / `a_cc`), and both reach region 1 (`e_param`). The
command is served when it changes value (sequence bit 6 alternates on each command):

```
cmd = seq<<6 | op<<4 | ext                 (7 bits, one CC)
  op 0 ADDR   index = data (0..127); the next sample falls at wtl_buf[index]
  op 1 DATA   wtl_buf[index++] = sext18(data | ext<<14) >> 2   (an 18-bit value * 2^17 -> Q15)
  op 2 CTRL   ext 0 commit (dumps the buffer to bank 5);  ext 1 nop;  ext 2 clear (hands COLOR back)
```

The `data` is read with `os_get(2, 0x130)` when the command arrives (robust if the bend event is lost), the same as
`dsplive` reads the bend from the slave's store. Once a frame is loaded, `sl_loaded` freezes it (COLOR stops overwriting
it) until a *clear*; it survives reselecting the PLUG-OUT (`e_load` dumps it again).

**Limits**: only 127 points (window `A080`–`A0FF`; `A200`+ is not addressable, §4.1 / `s1rules.py` R10); it is a swap of the
resident frame, not sample-by-sample streaming; only one MIDI client at a time (project rule).

```
# build (SL_WT=b5 + SL_LOAD), install, and with the PLUG-OUT sounding:
SL_LOAD=1 WT_WAV=".../Jupiter 8 Saw.wav" KERNEL=sdk/s1cc/build/jp8_saw sh examples/system_s1cc/build.sh
python3 tools/plugout_tool.py install plugins/JP8-SAW5 --slot 1 --force --no-patches -y
python3 tools/wt_load.py --saw            # a band-limited saw (built-in test shape)
python3 tools/wt_load.py wave.wav         # one frame of a WAV (--frame K, --frame-size N)
python3 tools/wt_load.py --revert         # hand the frame back to OSC1 COLOR
```

**UNTESTED on the device (2026-11-06)**: that the voice parameters `0x130` / `0x131` reach `e_param` of region 1 (the
notes `0x110+v` and velocities `0x120+v`, also of group 2, do arrive). If it does not load, repoint the two ids
(`-DWTL_DATA_ID` / `-DWTL_CMD_ID`) to a pair known to reach the region, and adjust the tool.

## 5. What is missing in s1cc

| piece | what it does | where |
|---|---|---|
| `s1_delay_write(line, x)` / `s1_delay_read(line, d)` (`d` float, interpolated) | one line per voice in bank 0: the ring reserver gives it `b`, takes the rest of the state out of `S + b … S + b + dmax`; `hset h6` + `ldidx` | `s1dsp.h`, `graph.cpp` (today E303: computed index into a state array), `kernel.cpp` (reservation), `make_s1cc.py` (that the host does not overwrite the line) |
| `S1_TABLE(name, n)` in bank 5 + `s1_table(name, phase)` | a table the ARM writes (from a function that runs on the PC, like `S1_PARAM`) and the DSP reads with `da` interpolating | `s1dsp.h`, `lower.cpp` (`ldb5`), `engine.c` (copy with `FD6C` in `load` and when the knob moves) |
| `S1_FRAMES(name, panel_id, n)` | control frames that the tick writes with `FCEC` (speech) | `engine.c` (tick), `make_s1cc.py` |
| emulator | the ring of bank 0 is already exact; indexed bank 5 too (`emu/dspcore.h`) | — |

The SDK header declares `s1_ldb5(i)` (a word of the wavetable frame at `A080..`,
[Vol. V ch. 2](../vol5_toolchain_compiler_sdk/s1dsp_api_reference.md) §2.1); `S1_TABLE` / `s1_delay_*` / `S1_FRAMES` are not in it.

## 6. Proposed order

1. **`S1_TABLE` in s1cc** and the wavetables of Braids / Plaits (WAVETABLES, WAVE_MAP, WAVE_LINE, *wavetable* engine),
   8 voices: indexed bank 5 works on the S1 (WT-SLAVE5, §4.1); `MP-CSAW2-LUT` is built, not tested on the device.
2. **`s1_delay_*` in bank 0**: PLUCKED, SAW_COMB and the Plaits string within the room of §3.2, or twice the reach at
   half rate [estimated].
3. **The EDM on master B**: per-voice strings work (MP-EDMSTR, §3.3.5, 6 voices) and so does reading at a computed
   position (WT-EDM, §4.4); the probe MP-EDMIDX (§3.3.6) is built, not run. Next: PLUCKED / strings at any pitch with 6
   voices (excitation through `6008`–`600D`), chorus of the *string machine*, granular buffer. `tapreq` on a slave
   (§3.2) for BOWED / BLOWN / FLUTED is [open].
4. **Frames by the tick**: the Plaits *speech* engine (LPC) with `S1_FRAMES`.
5. Chorus / ensemble of the *string machine* on master B (the delay memory that the OS already uses for its delay): only
   if the OS leaves room in that memory [open].
