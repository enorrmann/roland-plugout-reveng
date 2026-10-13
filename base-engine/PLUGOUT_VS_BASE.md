# Do the PLUG-OUTs add processing, or only configure the base system?

Short answer: **both, at different layers.** The audio-rate algorithms
(oscillator models, filter models, effects) are not in any PLUG-OUT image; the
base system already carries them and a PLUG-OUT selects and parameterises them.
What a PLUG-OUT does bring is its own **control-rate program**: parameter
semantics, envelopes, LFOs, voice logic and effect mapping, a separate code
base of 13–15 k instructions per instrument.

Method: static disassembly (capstone) and emulation of engine functions
(unicorn) of the decoded images in `up/`, with OS-table calls trapped
(`tools/engine_analysis.py`, `tools/engine_emu.py`). The encrypted OS was not
used. **[H]** = observed, **[I]** = inferred.

## 1. How an engine talks to the sound hardware **[H]**

Every engine reaches the DSP/OS through the same table at `0x0113FC00`.
Trapped calls show the roles of the entries the engines use:

| Offset | Observed behaviour |
|---|---|
| `+0xA4` | set a DSP parameter: `(object, unit = [obj+4], id = r2, float = s0)` |
| `+0xA0` | same, with a 5th argument and `r3 = 3` (ramped / alternative form) |
| `+0xB0` | set a parameter through a pointer id (observed with 2 values per waveform) |
| `+0xBC`, `+0xC0`, `+0xF0` | float setter, object function, start-up |

A parameter is addressed by **(unit, id)**: `[obj+4]` is the unit (2/3 =
oscillators, 4/5 = filter and effects, 1 = LFO/common), ids are small
integers stored in the object by its constructor. Constructors take a *mode*
argument (0, 1, 2, 3…) that offsets the ids, i.e. one object class serves
several identical hardware voices.

## 2. The oscillator selector in the base engine **[H]**

The slot-0 model's waveform list has 24 labels. They are **4 variant classes ×
6 rows**:

| Labels | Variant class | Data in the engine |
|---|---|---|
| 1–6 `SAW SQR TRI SAW2 SQR2 TRI2` | `VcoVari00` | none (DSP-side) |
| 7–12 `NOISESAW LOGIC FM FM SYNC VOWEL COWBELL` | `VcoVari01` | none (DSP-side) |
| 13–18 `FM 1:1 … FM 6:1` | `VcoVari02` | 6 rows × 2 floats |
| 19–24 `FM Type A … F` | `VcoVari03` | 6 rows × 5 + 5 + 2 floats |

(`VcoVari00/01` contain only constructors and stubs.) The row index passed to
the variant's setter (`v13`; `v14` = the ramped form) selects the row and the
setter sends it to the DSP:

### `VcoVari02` — FM ratio rows (unit 2 for oscillator 1, unit 3 for oscillator 2)

| Row | Label | id 370 | id 368 | id 371 | id 369 |
|---|---|---|---|---|---|
| 0 | FM 1:1 | 1.0 | 1.0 | 1.2732 | 0 |
| 1 | FM 1:1.5 | 1.0 | 1.5 | 1.2732 | 0 |
| 2 | FM 1:2 | 1.0 | 2.0 | 1.2732 | 0 |
| 3 | FM 1:3.5 | 1.0 | 3.5 | 1.2732 | 0 |
| 4 | FM 1:15 | 1.0 | 15.0 | 1.2732 | 0 |
| 5 | FM 6:1 | 6.0 | 1.0 | 1.2732 | 0 |

(oscillator 2 uses ids 374/372/375/373; sets `496–499` / `500–503` appear in
the same constructor.) The label text equals the row's carrier:modulator
ratio, which is how the table was identified.

### `VcoVari03` — "FM Type" rows

| Row | Label | ids 376, 374, 370, 372, 368 | ids 377, 375, 371, 373, 369 |
|---|---|---|---|
| 0 | FM Type A | 1, 4, 4, 11, 15 | 4.456, 0.318, 2.865, 0, 0 |
| 1 | FM Type B | 1, 1, 4, 9, 9 | 4.456, 0.318, 2.865, 0, 0 |
| 2 | FM Type C | 1, 2, 4, 9, 9 | 4.456, 0.318, 2.865, 0, 0 |
| 3 | FM Type D | 1, 1, 3.5, 11, 11 | 4.456, 0.318, −2.228, 0, 0 |
| 4 | FM Type E | 1, 3, 3.5, 11, 11 | 4.456, 0.318, −2.228, 0, 0 |
| 5 | FM Type F | 1, 1, 40, 1, 1 | 4.456, 0.318, 0.159, 0, 0 |

plus two sends of `(1.0, −0.16)` through `+0xB0` for every row. The first
group reads as five frequency ratios and the second as five weights/levels
**[I]**.

**Consequence:** the FM, FM-ratio and FM-type "waveforms" are *data rows*
configuring a generic multi-operator FM oscillator that lives in the DSP.
Adding a new FM-type waveform needs no new DSP code, only a new row.

Basic waveforms in every engine (base and PLUG-OUT) are selected differently:
a 6-entry float table `{-3,-2,-1,0,1,2}` indexed by the panel value is sent
as one parameter (JX-3P: ids from `[obj+0x444]` / `[obj+0x454]`).

## 3. Filter models: the factory instruments' filters are in the base **[H]/[I]**

The slot-0 model has the filter-model list
`LPF/HPF, SBF, SYS-1, JP-8, JU-106, FMT 2, FMT 3, HARM` and a 48-entry
slope/response list (8 × 6). The base engine has one class per model:
`VcfVari00, 01, Jp8, Ju106, Sys1, Fbof, Fmt2, Fmt3`. So the JUPITER-8 and
JUNO-106 filter characters are already part of the base system; the
factory PLUG-OUTs reuse them through the shared OS table. **[I]**

## 4. What the PLUG-OUT engines add **[H]**

### 4.1 They are different programs

Instruction-sequence ("shape") similarity between engine images:

| | base `p0s` | JUPITER-8 | JUNO-106 | JX-3P |
|---|---|---|---|---|
| JX-3P shares | 1 % | 18 % | 17 % | – |
| JUPITER-8 shares | 8 % | – | 35 % | 17 % |
| JUNO-106 shares | 9 % | 35 % | – | 16 % |

A PLUG-OUT engine is not a configuration of the base engine: it is a separate
~14 k-instruction program built from a common template (17–35 % shared
shape among PLUG-OUTs).

### 4.2 Real computation exists in them

* `FltVoice` constructor (JX-3P `0x60E08B2C`): builds 256-entry **cubic
  curve tables** (`((i+1)/256)³` and variants) at start-up with
  floating-point loops; the `EnvVoice` constructor (`0x60E087CC`) sets the
  per-voice id blocks (44–51 / 64–71 by mode) and its own state.
* `OscVoice`/`AmpVoice`/`LfoVoice` methods multiply per-voice state by
  tables before sending (e.g. `value × array[index]`), i.e. control-rate
  modulation maths in the CPU.
* `VoiceMix`, `PatchLev`, `EfxCmn` and the 330–382 parameter-translation
  methods (`CPrmDSPJx3p*`, `CPrmDSPJp8*`, `CPrmDSPJu106*`) implement each
  instrument's parameter semantics and value→float curves.

### 4.3 What they do not contain

* No DSP microcode: the 0x12000–0x24000 span of the JX-3P engine that looked
  like a separate blob is more Thumb code (clamp/switch sequences), and the
  rest are float curve tables and vtables.
* No audio-rate loops over sample buffers were seen in the functions
  examined (not an exhaustive proof); the CPU code observed runs on
  parameter changes and drives the DSP through the OS table.

## 5. Interpretation and limits

* The architecture is **ARM control layer + fixed DSP "synth kit"**. The DSP
  exposes oscillators with several variants, filter models, effects; the
  base engine and each PLUG-OUT are two different control layers on that kit.
* Id spaces: JX-3P oscillators use ids 32–75 on units 2/3; the base FM
  variants use ids 368–377 and 496–503 on the same units. The JX-3P engine
  does not use the FM block. Whether every id a PLUG-OUT sends exists in the
  base DSP is **not proven**: the DSP program itself is not visible
  (probably inside the encrypted OS).
* So the practical reading: a custom PLUG-OUT can (a) change the control
  layer freely (curves, envelope behaviour, LFO, voice logic, labels), and
  (b) re-select DSP features the kit already has (the FM rows above, filter
  models, waveform floats). It cannot introduce a new audio algorithm
  without access to the DSP program.

## 6. Reproduce

```bash
python3 tools/sys8fw.py report base-engine/up --out base-engine/report
# emulation helpers: tools/engine_emu.py, tools/engine_analysis.py
```
