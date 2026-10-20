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

* The architecture is **ARM control layer + DSP**. The DSP exposes
  oscillators with several variants, filter models, effects; the base engine
  and each PLUG-OUT are different control layers on it. Whether the DSP side is
  one fixed "kit" shared by all PLUG-OUTs is **not established**: §7 shows
  that equivalent functions use different ids in different engines.
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

## 7. Is "SAW" the same waveform in every engine? **[H]** (selection) / **[I]** (identity)

Measured by emulating each engine's oscillator setters with trapped OS calls
(unit 2 = oscillator 1, unit 3 = oscillator 2):

| Engine | How SAW is requested |
|---|---|
| JX-3P | one scalar, 6 positions `SAW, SQR, PULSE×4`; SAW = `-3.0` (ids 38/39) |
| JUPITER-8 | 4 channel parameters (ids 48–51), one-hot with sign: SAW = ch 48 `1.0`; TRI/SIN share ch 50 (+1/−1), SQR/NOISE share ch 51 (+1/−1) |
| JUNO-106 | same 4-channel pattern on ids 47–50 |
| JUNO-60 | same pattern on ids 35–38 |
| SYSTEM-100 | 4-channel pattern: setter at `0x60A092AE` copies a 5-row × 4-float table to the stack, selects the row by the value and sends the 4 floats to 4 ids |
| base SYSTEM-8 | scalar `-3…2`, plus the 4 × 6 variant rows (§2) |

The selection mechanism is common (unit 2/3, ids ≥ 32, the same value
curves), but the **ids differ per engine for the same function** (the same
curve goes to id 37 in JUNO-60 and id 39 in JUNO-106; JUNO-106 skips ids 33
and 34 that JUPITER-8 uses). Nothing in the engine images shows what waveform
the DSP generates for a given request, so "same saw" can be neither proven nor
refuted from the control code. The decisive test is on hardware: the same
(unit, id, value) from two engines.

## 8. SYSTEM-100 and SH-101: flat engines **[H]**

The SYSTEM-100 and SH-101 engines (SYSTEM-1 PLUG-OUTs, also shipped for
SYSTEM-8) are built differently from JUNO-60 / JX-3P / JUPITER-4:

| Engine | Classes (RTTI) | Reachable code | VFP ops | Non-stub methods in the parameter class |
|---|---|---|---|---|
| SYSTEM-100 | 2 (`CPrmDSPSystem100`, `CInterface`) | 14.7 k insns | 962 | 142 of 451 |
| SH-101 | 2 | 12.7 k | 727 | 114 of 422 |
| JUNO-60 | 22 | 17.6 k | 995 | 83 of 358 |
| JX-3P | 21 | 17.6 k | 603 | 86 of 334 |
| JUPITER-4 | 21 | 17.8 k | 1006 | 81 of 354 |

* No voice classes: the parameter class holds 8 static unit descriptors
  (`{unit, …, ids}` at `0x60A3FD34…`) and calls free functions that send
  values through the OS table. It is still a real program (value curves,
  tables, loops), not a bare table.
* The parameter class registers **186 ids** (SH-101: 172); **84** of them have
  a dedicated handler (SH-101: 54). Ids `1829–1844` exist only in SYSTEM-100
  (14 real handlers, 2 no-ops); `1844` is a 4-channel one-hot selector, the
  rest send single curve values.

### What the SYSTEM-100 adds, measured on the plug-in's own `Script.xml`

The patch parameter store is the same for every PLUG-OUT (114 + 522 + 3524
values). What differs is how many slots a PLUG-OUT names and uses:

| Plug-out | Voice-section fields (LFO…EXTEND) | Extra fields (jack/S&H/LFO-2…) | Total |
|---|---|---|---|
| SYSTEM-100 | 61 | 40 | **101** |
| SH-101 | 45 | 25 | 70 |
| JX-3P | 44 | 15 | 59 |
| JUNO-60 | 38 | 16 | 54 |
| JUPITER-4 | 39 | 14 | 53 |

The same slot can mean different things per plug-out (NAME2 slot 7 is
`S&H MODE` in SYSTEM-100, `LFO CV GAIN` in SH-101).

### The patch matrix **[H]** (data) / **[I]** (engine link)

`Script.xml` defines `matrixIn` (15 inputs: LFO-1/2 EXT CV, S&H, GLIDE,
VCO-1 EXT CV, VCO-2 SYNC, RING MOD, MIXER EXT, VCF/VCA ADSR gate and ADSR, VCF
LFO, PHASER LFO/MANUAL) and `matrixOut` (14 sources: KYBD CV, LFO-1, LFO-2,
S&H, S&H CLK, VCO-1/2 and their syncs, RING MOD, NOISE, MIXER, VCF ADSR, VCA
ADSR). Each input is stored as a 0…14 selector (0 = open) in the patch, and
`defaultOut` gives the normalled routing
`0,0,0,0,8,7,6,10,0,2,13,0,14,0,0`. The 15 selectors correspond to the 15
matrix-input fields that SH-101 leaves unused (`(LFO CV IN)` …). That the 14
extra ids of §8 are the engine side of this matrix is a hypothesis: the
handlers were observed to send values, not to resolve a routing graph.


## 9. Reproduce

```bash
python3 tools/sys8fw.py report base-engine/up --out base-engine/report
# emulation helpers: tools/engine_emu.py, tools/engine_analysis.py
# engine region of a plug-in: plugout_image.raw_regions(flash_blocks(plugout.load("plugins/SYSTEM-100/Script/Code8_1.Dat")))[1] -> Region.from_raw(...).plain
# flat engines (SYSTEM-100, SH-101): python3 tools/engine_params.py ENGINE.region1.bin CPrmDSPSystem100
```
