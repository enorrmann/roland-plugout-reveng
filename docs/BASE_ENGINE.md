# What the decoded base engine contains

Everything below was produced by running `tools/sys8fw.py` on
`firmware/SYSTEM8_UP.BIN` (SYSTEM-8 OS 1.32, build 2022/04/28). The files are
kept in this directory. Method names are positional (`<Class>__v<N>`); class
names are the compiler's RTTI type names; sizes are upper bounds (distance to
the next known function). Certainty: **[H]** read directly from the data,
**[I]** inferred.

## 1. Files obtained

| Path | Content | Size |
|---|---|---|
| `info.txt` | `sys8fw info`: members, identities, OS header/trailer | 4 KB |
| `up/sys8fw.json` | manifest: tar headers, region header fields, offsets | 10 KB |
| `up/p1y01_m0c0a_up.enc` | `App1_Main` exactly as shipped (still encrypted) | 2 939 664 B |
| `up/p1y01_p0m.region0.bin` | slot-0 **model** region, decompressed | 1 166 152 B |
| `up/p1y01_p0s.region0.bin` | slot-0 **engine** region (the base synth engine) | 536 308 B |
| `up/p1y01_pl1m.region0.bin` / `.region1.bin` | JUPITER-8 model / engine (slot 1) | 1 160 460 / 342 424 B |
| `up/p1y01_pl1s.region0.bin` / `.region1.bin` | JUNO-106 model / engine (slot 2) | 1 157 780 / 262 192 B |
| `up/p1y01_pl2m.region0.bin` / `.region1.bin` | JX-3P model / engine (slot 3) | 1 159 484 / 275 508 B |
| `elf-asm/*.elf` | one ELF per image (load as ARM LE Thumb-2) with class/vtable/method symbols | 0.3–1.2 MB each |
| `elf-asm/*.S` | `objdump -d` listing of each ELF (Thumb, all bytes treated as code) | 1.7–10 MB each |
| `report/index.md` | table of the 9 members (8 images + the encrypted OS) | |
| `report/<image>.md` | classes, bases, virtual methods with address/size, presets, label lists | 8 files |
| `report/p1y01_m0c0a_up.md` | the encrypted OS: clear header/trailer, payload statistics, cipher conclusions (marked *not decoded*) | |
| `report/symbols.json` | machine-readable class → vtable → methods | 108 KB |

Reproduce:

```bash
T="python3 tools/sys8fw.py"
$T info    firmware/SYSTEM8_UP.BIN > base-engine/info.txt
$T unpack  firmware/SYSTEM8_UP.BIN base-engine/up
$T disasm  base-engine/up --out base-engine/elf-asm   # also (re)writes the ELFs
$T report  base-engine/up --out base-engine/report
```

Note: the `.S` listings of *model* images disassemble large data tables as if
they were code; only the last ~19 KB of each model region (from `0x118000`)
is code.

## 2. Image identities and memory map **[H]**

| Image | Identity (SYNINFO) | Base | `init` | Classes | Virtual methods |
|---|---|---|---|---|---|
| `p0m.region0` | SYSTEM-8, id 0x0005, v1.20 | `0x61600000` | `0x6171827F` | 1 | 17 |
| `p0s.region0` | SYSTEM-8, id 0x000A, v0.01 | `0x60800000` | `0x60816D41` | 97 | 2159 |
| `pl1m.region0` | JUPITER-8, id 0x0006, v1.06 | `0x61800000` | `0x6191823D` | 1 | 17 |
| `pl1m.region1` | SYSTEM-8, id 0x000A, v0.01 | `0x60A00000` | – | 21 | 986 |
| `pl1s.region0` | JUNO-106, id 0x0007, v1.06 | `0x61A00000` | `0x61B1823D` | 1 | 17 |
| `pl1s.region1` | SYSTEM-8, id 0x000A, v0.01 | `0x60C00000` | – | 22 | 915 |
| `pl2m.region0` | JX-3P, id 0x0008, v1.00 | `0x61C00000` | `0x61D1823D` | 1 | 17 |
| `pl2m.region1` | SYSTEM-8, id 0x000A, v0.01 | `0x60E00000` | – | 21 | 837 |

(Engines' `init` is in their own SYNINFO; see `report/*.md`.)

Layout of the slot-0 engine `p0s` (base `0x60800000`): SYNINFO at +0x0;
virtual-method code spans `0x60808014…0x6081EBE2` (≈ 93 KB, functions
reachable through vtables); vtables `0x60880194…0x60882628`; the rest is data
tables and `.bss` addresses beyond the image. Factory engines have the same
shape (code `0x60a080be…0x60a13f3e`, vtables from `0x60a528d4`, etc.).

## 3. Class structure of the base engine (`p0s`) **[H]**

97 classes, 2159 virtual-method slots, **1322 distinct functions** (the
rest are slots shared between classes by inheritance), 40 root classes and
57 derived classes.

### 3.1 Root classes (declared interface)

| Group | Classes (virtual methods) |
|---|---|
| Entry | `CInterface` (1) |
| Synth blocks | `CDSPSystem8Vco` (25), `…Vcf` (25), `…Vca` (10), `…Env` (13), `…Lfo` (32), `…Efx` (12), `…Out` (8), `…Vol` (8), `…Mst` (9), `…CvOut` (16) |
| Effects | `CDSPSystem8EfxCr` (11), `…EfxDs` (8), `…EfxFz` (8), `…EfxMt` (8), `…EfxOd` (8), `…EfxPh` (14) |
| Per-voice | `…VcoVoice` (29), `…VcfVoice` (8), `…VcaVoice` (10), `…EnvVoice` (35), `…LfoVoice` (12) |
| Jack outputs | `…JkOutMst` (11), `…JkOutSlv` (10) |
| Variants | `…VcoVari00..03` (15 each), `…LfoVari00..02` (10 each), `…VcfVari00` (8), `…VcfVari01` (7) |
| Filter models | `…VcfVariJp8` (8), `…VcfVariJu106` (10), `…VcfVariSys1` (8), `…VcfVariFbof` (7), `…VcfVariFmt2` (8), `…VcfVariFmt3` (8) |
| Parameters | `CPrmDSPSystem8` (480) |

### 3.2 Derivations (RTTI base pointers) **[H]**

* Every block class `CDSPSystem8X` has a derived `sCDSPSystem8X` with the
  **same** number of virtual methods. The base's first slot is a bare
  `bx lr` stub (`70 47`) and its remaining slots mostly alias the derived
  class's functions: the base is the abstract/placeholder interface and the
  `s…` class holds the real implementation (its first slot differs). **[I]**
  for the exact role of `s`.
* The `_`-prefixed classes (`_CDSPSystem8Mst`, `_…Voice`, `_…Vari…`) also
  have a base class; the first slots of `_CDSPSystem8Mst` are all `bx lr`
  stubs. Role not established. **[I]**
* `CPrmDSPPlugoutSlave` (481 methods) derives from `CPrmDSPSystem8` (480):
  the plugin/"slave" parameter set is the base parameter set plus one
  method.
* No function is shared by more than three classes in `p0s`
  (`0x6080A60F`, `0x6080A611`, `0x6080A79F` are): small common helpers.

### 3.3 What the filter/oscillator variants tell **[I]**

The base engine implements, as selectable models, the filter responses of
the factory instruments: `VcfVariJp8` (JUPITER-8), `VcfVariJu106`
(JUNO-106), `VcfVariSys1` (SYSTEM-1), `VcfVariFbof`, `VcfVariFmt2/3`
(formant), plus four oscillator variants (`VcoVari00..03`) and three LFO
variants. The slot-0 model's own label list (id 7) names them:
`LPF/HPF, SBF, SYS-1, JP-8, JU-106, FMT 2, FMT 3, HARM`.

## 4. Interface classes — what the OS calls **[H]/[I]**

* **Model region** (`CAssignSystem8` in slot 0, `CAssignPlugout` in
  slots 1–3): one class, **17 virtual methods**, 17 distinct functions, sizes
  2–212 B (upper bounds). Methods 9–12 (observed in `pl2m`) call a common
  helper and then forward 16-bit (sign-extended) arguments through
  function pointers stored in a table next to the code (offsets 0, −4, +4,
  +8); one of them returns `1` after writing a result. Helper routines next
  to them call virtual slots `0x24/0x28/0x30` of an object (get/set value
  pattern). **[I]**: this is the OS-facing assignment interface (get/set of
  a value by index).
* **Engine region**: `CInterface`, **1 virtual method** (first bytes
  `70 47` = `bx lr`): the entry the OS reaches through the SYNINFO `init`
  descriptor. The engine is otherwise driven through `CPrmDSP*`. **[I]**
* The real work is behind the **parameter classes**:

| Image | Parameter class | Methods |
|---|---|---|
| slot-0 engine | `CPrmDSPSystem8` / `CPrmDSPPlugoutSlave` | 480 / 481 |
| JUPITER-8 | `CPrmDSPJp8` (+ `CPrmDSPJp8Sys8`) | 382 |
| JUNO-106 | `CPrmDSPJu106` (+ `…Sys8`) | 352 |
| JX-3P | `CPrmDSPJx3p` (+ `…Sys8`) | 330 |

  The three factory engines share the same group names (`…Noise`,
  `…EfxCmn`, `…AmpVoice`, `…EnvVoice`, `…FltVoice`, `…LfoVoice`,
  `…OscVoice`, `…PatchLev`, `…VoiceCmn`, `…VoiceMix`, `…EfxDsType`, and
  effect classes `Ch/Cr/Ds/Fz/Mt/Od/Ph` — Ch absent in JUPITER-8, Mt/Ph per
  model), i.e. one code template specialised per instrument **[I]**.

## 5. Model-region data **[H]**

### 5.1 Factory presets
64 records of 0x200 B at `0x8000`, name at +0x100 with each 4-byte group
reversed. All four models have 64 named presets:
`report/p1y01_p0m.region0.md` (SYSTEM-8: `SY System-8 Saws`,
`SQ Star Spiral`, `PL Synth Edge`, …) and the factory models'
(`report/p1y01_pl*.region0.md`). Parameter words are 32-bit values
(mostly 0–255, some signed); their mapping to parameters is not decoded.

### 5.2 Value-label lists
Header of 5 words `<2><0x14><list id><count><0>` followed by `count`
entries of 7 words `<3><0x1C><char[20]>`; some lists repeat the last label
to pad to the declared count. Counts: SYSTEM-8 9 lists, JUPITER-8 9,
JUNO-106 7, JX-3P 11. Slot-0 model (`p1y01_p0m.region0.md`):

| id | Entries | Content |
|---|---|---|
| 1 | 18 | LFO waveforms: `SIN TRI SAW SQR S&H RND SINx2 … RNDx2 TYPE1..6` |
| 2 | 24 | oscillator waveforms: `SAW SQR TRI SAW2 … NOISESAW LOGIC FM … VOWEL COWBELL FM 1:1 … FM Type B` |
| 3 | 6 | footage: `64FEET … 2FEET` |
| 4 | 6 | modulation source: `MAN LFO P.ENV F.ENV A.ENV OSC3` |
| 5 | 6 | sub-oscillator: `SIN -2OCT … TRI -2OCT` |
| 6 | 4 | `1 2 3 4` |
| 7 | 8 | filter model: `LPF/HPF SBF SYS-1 JP-8 JU-106 FMT 2 FMT 3 HARM` |
| 8 | 48 | filter response/slope: `LPF -24dB … HPF -24dB, SBF1..6, …` |
| 9 | 14 | modulation destination: `(OPEN) CV GATE LFO LFO A LFO B P.ENV F.ENV A.ENV OSC1 OSC1SYNC OSC2 OSC3 MIXER` |

The JX-3P list 11 (`OVERDRIVE DISTORTION JX-3P CH FUZZ CRUSHER PHASER`) is
the effect-type list whose label was edited on hardware (docs/CUSTOM_PLUGOUTS.md);
the JUNO-106 equivalent reads `…JUNO CH1 JUNO CH2…`.

### 5.3 Code at `0x118000`
~19 KB (`0x118000…0x11CB47` in slot 0) holds the model region's code: the
`CAssign*` class and its helpers.

## 6. What this does and does not give you

* Gives: a complete, patchable, reproducible set of the engine and model
  images in clear; class/vtable/method symbols to drive a disassembler;
  tables (presets, labels) readable and editable.
* Does not give: the OS (`App1_Main`, encrypted — `docs/BASE_FIRMWARE.md` §2),
  meaningful method names, the DSP programs, or the meaning of parameter
  ids. Method roles beyond the structural facts above are not established.
