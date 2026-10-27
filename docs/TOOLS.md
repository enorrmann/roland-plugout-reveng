# Herramientas: uso y parámetros

Esta es la parte escrita a mano de [TOOLS.md](TOOLS.md). Contiene los flujos de trabajo y las herramientas de uso
diario con todos sus parámetros. Debajo, `tools/tools_usage.py` genera la referencia de **todas** las herramientas
del repositorio (docstring y opciones de cada script). Tras añadir o cambiar una herramienta:

```sh
python3 tools/tools_usage.py            # regenera docs/TOOLS.md (esta guía + la referencia)
python3 tools/tools_usage.py --check    # código 1 si docs/TOOLS.md está desactualizado
```

Estado: 2026-10-23.

## 1. Flujo de un PLUG-OUT propio (SYSTEM-1, C-S1SY)

```sh
STEP=11 python3 examples/system_1_synth/make_synth.py --sim          # 1. probar en el emulador (≈ 1,4 s)
STEP=11 python3 examples/system_1_synth/make_synth.py --pack-check   #    + optimización bit a bit (≈ 3 s)
STEP=11 NAME=C-S1SY11 sh examples/system_1_synth/build.sh           # 2. build (+ check_optimized)
amidi -l                                                             # 3. ¿está el equipo? (SYSTEM-1 MIDI 1/2)
python3 tools/plugout_tool.py install plugins/C-S1SY11 --slot 1 --force --no-patches -y   # 4. instalar
```

`--sim-check` (≈ 25 s, con la referencia Python) **no** forma parte del flujo diario. Se corre sólo cuando
cambia algo que la referencia valida y `--pack-check` no:
* el núcleo del emulador (`examples/from_scratch/emu/`: `dspcore.*`, `dspjit.py`, `dspcomp.py`, `cchain.py`,
  `chain.py`);
* `simulate()` / `simulate_py()` o el modelo de memoria de la simulación;
* una forma de instrucción que nuestros programas no usaban (op o modo nuevo).

Para probar ondas, filtros y modulaciones se usan `simulate()` y medidas propias sobre el núcleo C, que corre
≈ 3,5× más rápido que el tiempo real.

Después de instalar hay que seleccionar el PLUG-OUT en el panel. Herramientas que hablan MIDI con el equipo:
nunca dos a la vez.

## 2. `examples/system_1_synth/make_synth.py`: programas, simulación y validación

```sh
python3 examples/system_1_synth/make_synth.py                # s1*.s / s1*.hex, build/s1_programs.h
python3 examples/system_1_synth/make_synth.py --sim          # además: tres pruebas de sonido en el emulador
python3 examples/system_1_synth/make_synth.py --pack-check   # además: empaquetado / fusión bit a bit, sólo C (≈ 3 s)
python3 examples/system_1_synth/make_synth.py --sim-check    # además: núcleo C contra la referencia Python (§2.2, ≈ 25 s)
```

### 2.1 Variables de entorno

| variable | valor por defecto | qué hace |
|---|---|---|
| `STEP` | 8 | paso incremental: 1–11 (lista en `sh examples/system_1_synth/build.sh --help`); 0 = el primer diseño |
| `STAGE` | 4 | sólo con `STEP=0` |
| `VOICES` | 8 | voces |
| `PACK` | 1 | empaqueta `ld`/`st` en las instrucciones de las ops (`pack()`); 0 = una unidad por instrucción |
| `SCHED` | 1 desde `STEP=12`, si no 0 | planificador `asm/dspsched.py` en lugar de `pack()` |
| `FUSE` | 1 desde `STEP=11`, si no 0 | fusiona ops independientes en una instrucción (`op1 \| op2`) |
| `RESCOMP` | 0 | resonancia compensada por el corte (autooscilación) |
| `RES_MAX` | 3,6 (4,4 con `RESCOMP=1`) | realimentación del filtro con la resonancia al máximo |
| `GMAX` | 0,7 | tope del coeficiente del filtro |
| `PAD` | 0 | prueba de presupuesto: n rellenos al final del DSP A |
| `SIM` | `c` | `py`: `--sim` con la referencia Python (`re/dspisa.py`, minutos) en vez del núcleo C (1 s de audio ≈ 0,4 s) |
| `SIM_JOBS` | núcleos de la máquina | simulaciones independientes de `--sim` / `--sim-check` a la vez, una por proceso; 1 = en serie |
| `DSP_CC` | `clang` si está, si no `cc` | compilador del núcleo C y del código nativo (`gcc`, `clang` o una ruta) |
| `SIM_N` | 9600 | muestras de la prueba de La 440 de `--sim` |
| `SIM_CHECK_N` | 600 | muestras de la comparación núcleo C / Python de `--sim-check` |
| `SIM_PACK_N` | 19200 | muestras de la comparación empaquetado / sin empaquetar de `--sim-check` |

### 2.2 `--sim-check` y `--pack-check`

`--pack-check` hace sólo la comparación 2 (sin la referencia Python). `--sim-check` hace las dos. Dos comparaciones, muestra a muestra. Cada una dice `identical` o el número de muestras distintas, la primera
muestra distinta y los dos valores:

1. **`sim check`: núcleo C del emulador contra la referencia Python.** Corre `simulate()` (núcleo C,
   `examples/from_scratch/emu/cchain.py`) y `simulate_py()` (`re/dspisa.py`) con los mismos programas y eventos:
   La 440 en la voz 0 en t = 0, Mi en la voz 1 en t = 50, la voz 0 soltada en la mitad. Dura `SIM_CHECK_N`
   muestras y casi todo el tiempo se va en la referencia Python.
2. **`pack check`: programas empaquetados / fusionados contra los mismos sin empaquetar** (`sources(True)` contra
   `sources(False)`), en el núcleo C. 8 notas, la mitad soltadas en la mitad, `SIM_PACK_N` muestras. Valida
   `pack()` y `FUSE`: la optimización no puede cambiar ni un bit.

Paneles probados en las dos comparaciones:

| panel | qué activa |
|---|---|
| `defaults` | sierra + sierra/2, filtro abierto |
| `filter + envelopes` | cuadrada + triángulo, corte 120, resonancia 200, las dos ADSR, key track |
| `LFO sine` / `LFO S&G` / `LFO S&H` (paso ≥ 11) | el anterior + LFO con todas las profundidades; en el paso 9: `LFO triangle` / `LFO S&H` |

Una diferencia en `sim check` es un error del núcleo C, o de una semántica que difiere entre los dos simuladores.
Una diferencia en `pack check` es un error de `pack()`/`FUSE`, o un parámetro fuera de la rejilla de los registros
(`on_grid()`, [MICROCODE_OPTIMIZATION.md](MICROCODE_OPTIMIZATION.md) regla 6).

### 2.3 `--sim`

| línea | qué mide |
|---|---|
| `A 440 …: peak, f` | una nota durante `SIM_N` muestras: pico de la salida del maestro (`A118`) y frecuencia por cruces por cero |
| `8 notes: peak` | 8 notas a la vez |
| `no notes: peak` | 2000 muestras sin notas (tiene que ser 0) |

## 3. `examples/system_1_synth/build.sh`

```sh
sh examples/system_1_synth/build.sh --help       # todas las opciones
```

Toma las variables de §2.1 más `NAME` (nombre del PLUG-OUT y carpeta `plugins/<NAME>`, como mucho 16
caracteres), `MODEL_ID` y `PARAMLOG=1` (la región 0 manda cada parámetro al PC). Al final corre
`tools/check_optimized.py` sobre los programas.

## 4. `tools/check_optimized.py`

```sh
python3 tools/check_optimized.py FILE...            # .s, .hex o .txt; avisos por archivo
python3 tools/check_optimized.py -q FILE...         # una línea por archivo
python3 tools/check_optimized.py --strict FILE...   # código de salida 1 si hay avisos
```

Avisos: `manual-wrap`, `filler`, `transfer-only` y `fusable`. Están explicados en
[MICROCODE_OPTIMIZATION.md](MICROCODE_OPTIMIZATION.md) §2.

## 5. Equipo

| herramienta | uso |
|---|---|
| `tools/plugout_tool.py ports` / `info` | puerto MIDI y estado de los slots |
| `tools/plugout_tool.py install <plugin> --slot N [--force] [--no-patches] [-y]` | instala `Code*.Dat` (+ patches); `--force` sin comprobar la versión, `--no-patches` sólo el firmware |
| `tools/plugout_tool.py install --code1 FILE …` | instala ese archivo como `Script/Code1.Dat` (SYSTEM-1); `--dat` es el `Code*.Dat` elegido a mano |
| `python3 tools/plugout.py [--json] FILE.Dat` | resumen de un contenedor; `--json`: cabecera, START / END, registros y regiones (lo usa `web/test/test-protocol.js`) |
| `python3 -m http.server 8000 --bind 127.0.0.1 --directory web` | instalador web en `http://localhost:8000/` (Chrome / Edge); [WEB_INSTALLER.md](WEB_INSTALLER.md) |
| `node web/test/test-protocol.js [FILE.Dat...]` / `sh web/test/run-page-test.sh` | pruebas del instalador web sin hardware: biblioteca contra `plugout.py`, página completa en Chrome headless |
| `examples/system_1/paramlog.py -t SEG [--raw] [--file LOG...]` | escucha una build `PARAMLOG=1`: ids de los controles del panel (con el PLUG-OUT seleccionado) |

## 6. Emulador (`examples/from_scratch/emu/`)

| herramienta | uso |
|---|---|
| `cchain.Slave(programs, model, native=True, threads=True)` | un BMC: hasta 6 DSP; `step(n)`, `run(n, words)`, `d[n].bank5`, `params`, `iomd`; ver el docstring de `cchain.py` |
| `python3 examples/from_scratch/emu/test_core.py [nombre...]` | pruebas del núcleo C, bit a bit contra `dspisa` / `chain.py` (todas o las que contienen el nombre) |
| `python3 examples/from_scratch/emu/test_chain.py` | pruebas del modelo de la cadena del SYSTEM-8 |

Velocidad y estado: [EMULATOR.md](EMULATOR.md) §3.6.

## 7. Documentación

| herramienta | uso |
|---|---|
| `python3 tools/doccheck.py FILE.md...` | todos los bloques ```` ```dsp ```` de los documentos tienen que ensamblar |
| `python3 tools/tools_usage.py [--check]` | regenera / comprueba este documento |

## 8. `sdk/mport`: portar componentes de Mutable Instruments

Especificación: [MUTABLE_SDK.md](MUTABLE_SDK.md). Ejemplo que lo usa: `examples/mutable_system`
([examples_mutable_system_README.md](examples_mutable_system_README.md)).

```sh
python3 sdk/mport/cli.py list                                     # componentes portados
python3 sdk/mport/cli.py check saw_swarm                          # port vs C++ original (≈ 1,4 s; compila la referencia la 1.ª vez)
python3 sdk/mport/cli.py lines saw_swarm [--plain]                # microcódigo del banco
python3 sdk/mport/cli.py lut braids ws_moderate_overdrive --odd 9 --xmax 0.875   # tabla -> polinomio, error en LSB
python3 -m unittest discover -s sdk/mport/tests                   # pruebas (≈ 1,2 s)
python3 examples/mutable_system/make_mutable.py --sim --pack-check
sh examples/mutable_system/build.sh                               # -> plugins/MUTABLE-SYSTEM
```

`EURORACK` = ruta del checkout de Mutable (por defecto `~/git/eurorack`); la referencia necesita g++.

## 9. `sdk/s1cc`: compilar una voz en C / C++ al microcódigo

Especificación: [S1CC.md](S1CC.md).

```sh
make -C sdk/s1cc                                                   # el compilador (LLVM 19)
sdk/s1cc/s1cc build VOZ.cc [MAS.cc ...] [-I DIR] [-o SALIDA] [--tol X] [--tollog X] [--maxdeg N] [--regs LISTA]
                                                                   # MAS.cc: solo las que no son vecinas de un header incluido (--no-auto)
sdk/s1cc/s1cc build VOZ.cc ... --debug                             # qué funciones C no compila y por qué (S1CC.md §9.1)
sdk/s1cc/s1cc build VOZ.cc ... --check                             # build + check (instrucciones, SNR)
sdk/s1cc/s1cc check SALIDA [--note N] [--n MUESTRAS] [--all-forms] [nombre=perilla ...]
KERNEL=SALIDA NAME=MUTABLE-S1CC sh examples/mutable_system/build.sh
KERNEL=SALIDA [VOICES=1..8] sh examples/system_s1cc/build.sh           # SYSTEM-S1CC: solo el kernel (examples_system_s1cc_README.md)
python3 -m unittest discover -s sdk/s1cc/tests                     # ≈ 8 s
```


# Reference (generated by `tools/tools_usage.py`, do not edit by hand)

## Install, containers, firmware, checks (`tools/`)

### `tools/check_optimized.py`

```
Check that our own DSP microcode uses the optimized forms (docs/MICROCODE_OPTIMIZATION.md); warn where it does not.

    python3 tools/check_optimized.py FILE...          .s (our assembly) or .hex / .txt (program words, '#' comments)
    python3 tools/check_optimized.py --strict FILE... exit code 1 when there is any warning
    python3 tools/check_optimized.py -q FILE...       one summary line per file

Same DSP ISA on the SYSTEM-1 and the SYSTEM-8, so the same checks for both. Every warning is an instruction that
costs a cycle of the per-sample budget for nothing:

  manual-wrap   a phase wrap / step built from `mul rX, rX, #256` + `min rX, rX, ..` pairs: one ph_wrap (or
                ph_clamp / ph_tri) does it (Roland: `ph_wrap rD, r3, rY`, SH-2 and the SYSTEM-8 oscillators)
  filler        `mov rN, rN` alone in an instruction: a load delay that packing did not absorb
  transfer-only an instruction with loads / stores and no op: the transfers fit into an op's instruction (pack())
  fusable       two adjacent single-op instructions, independent (the second does not read or write the first's
                result) and encodable as one instruction `op1 | op2` (A+B, A+A, as Roland). An estimate: the
                assembler decides the encoding; the cost of each fused class is in docs/MICROCODE_OPTIMIZATION.md

Build scripts run it on the programs they generate (examples/system_1_synth/build.sh, ...).
```

### `tools/crypto_scan.py`

```
crypto_scan.py -- look for block-cipher / hash constants in a raw memory dump.

    python3 tools/crypto_scan.py rom_dump.bin --base 0x01000000
    python3 tools/crypto_scan.py rom_dump.bin --base 0x01000000 --sboxes --xrefs

What it reports (offset and, with --base, the address):
  * 32-bit magic constants (TEA/XTEA/RC5/Blowfish/MD5/SHA/CRC/CAST/...) in
    little- and big-endian
  * byte tables (AES S-box, DES permutation / S1, Skipjack F, ...)
  * --sboxes: any 256-byte window that is a permutation of 0..255
    (S-box candidate, algorithm-agnostic) and runs of 4 x 256 x u32 tables
    (Blowfish/CAST style) by entropy of 1 KB windows
  * --xrefs: for every hit, Thumb `ldr rX,[pc,#imm]` / ADR sites that load
    the hit address (needs --base), i.e. the code using the constant

Pure Python, no dependencies.
```

| option | help | default |
|---|---|---|
| `file` |  |  |
| `--base` | address of byte 0 of the file | `0` |
| `--sboxes` | also find 256-byte permutations |  |
| `--xrefs` | find Thumb ldr [pc,#] users of each hit |  |

### `tools/doccheck.py`

```
Check the code examples of the technical documents: every ```dsp block must assemble.

    python3 tools/doccheck.py docs/WAVE_BUS.md docs/VOICE_ALLOCATION.md system-1/*.md
    python3 tools/doccheck.py --show docs/WAVE_BUS.md      # also print the words of each block

A ```dsp block is DSP assembly in the syntax of asm/ (docs/DSP_ISA.md): one instruction per line, ';' comments.
Blocks are assembled one by one with asm.Assembler; a block that fails is reported with its file and line and the
exit code is 1. ```c blocks are not compiled (they call OS services); keep them to code that exists in the repo.
```

### `tools/engine_analysis.py`

```
engine_analysis.py -- static analysis of SYSTEM-8 engine images (Thumb-2).

Used by base-engine/ReadMe + PLUGOUT_VS_BASE.md. Recovers functions from the
vtable methods (sys8fw symbols), follows direct calls, and classifies what
each function does: parameter translation (reads a value, maps it, sends it
to the DSP through the OS table) versus real computation (loops, float math,
persistent state).

    python3 tools/engine_analysis.py base-engine/up base-engine/report/symbols.json
```

### `tools/engine_emu.py`

```
engine_emu.py -- run engine functions under unicorn and log every call that
goes through the OS service table at 0x0113FC00.

    from engine_emu import Emu
    e = Emu(open("p0s.bin","rb").read(), 0x60800000)
    obj = e.alloc(0x2000)
    e.call(ctor, obj, mode)               # constructor / init
    calls = e.call(setter, obj, 0, 3)     # -> [(os_offset, r0, r1, r2, r3, s0, s1, stack0), ...]
```

### `tools/engine_params.py`

```
engine_params.py -- list the parameter ids a "flat" PLUG-OUT engine registers
(SYSTEM-100, SH-101: one CPrmDSP* class, no voice classes) and which of them
have a real handler.

    python3 tools/engine_params.py ENGINE.region1.bin CPrmDSPSystem100 [--base 0x60A00000]

The engine's parameter class has a "register all ids" method (the largest
function with no loops, ~5000 instructions) that calls, for every id, a getter
(vtable +0x58/+0x5c) and a setter (+0x30).  The setter dispatch method (the one
with a binary decision tree of `cmp r1, #id`) stores the value at an object
offset and calls one virtual slot per id; a slot that is only `bx lr` is a
no-op in this class.  Both are driven under unicorn with a fake vtable whose
entries are trapped.  Needs plain `python3` (capstone/unicorn), not `-I`.
```

| option | help | default |
|---|---|---|
| `image` |  |  |
| `cls` |  |  |
| `--base` |  | `1621098496` |

### `tools/lzss.py`

```
lzss.py -- the LZSS codec used inside PLUG-OUT firmware regions.

It is Haruhiko Okumura's classic LZSS (lzss.c, 1989) with the ring buffer
initialised to 0x00 instead of spaces:

    N = 4096 ring buffer, F = 18 max match, THRESHOLD = 2
    flag byte, LSB first: 1 = literal byte, 0 = reference
    reference = 2 bytes: pos = b0 | (b1 & 0xF0) << 4, len = (b1 & 0x0F) + 3
    the first byte is written at ring position N - F

encode() is a port of Okumura's binary-tree encoder; it reproduces Roland's
compressed streams byte for byte (checked on every region of the three plugins
and of SYSTEM8_UP.BIN).

Pure stdlib.
```

### `tools/midi_io.py`

```
midi_io.py -- minimal SysEx-capable MIDI ports.

Backends:
  alsa    Linux raw MIDI through libasound (ctypes, no Python packages needed).
          Port names are ALSA raw MIDI names: "hw:3,0,1".
  rtmidi  python-rtmidi (pip install python-rtmidi): Linux, Windows, macOS.
          Port names are the rtmidi port names (substring match accepted).

The SYSTEM-8 shows two USB MIDI ports. The plugins talk to the second one,
"SYSTEM-8 CTRL" (Script/Script.xml <midiOut><name>); on Linux it usually
appears as subdevice 1 ("hw:N,0,1", sometimes named "SYSTEM-8 MIDI 2").
```

### `tools/os_analyze.py`

```
os_analyze.py -- static analysis of the decrypted SYSTEM-8 OS image
(dumps/os_full.bin, run address 0x60000000, docs/BASE_FIRMWARE.md §7.14/§8).

Linear Thumb-2 sweep with capstone, then:
  * functions  = targets of BL (and of the scatter-load / reset entry),
                 each function ends where the next one starts;
  * per function: callees, callers, constants loaded from literal pools
    (LDR pc-relative) or built with MOVW/MOVT, split into
    strings (pointer to a NUL-terminated string in the image),
    MMIO (0x40000000-0x5FFFFFFF, 0xE0000000+), RAM and code pointers.

    python3 tools/os_analyze.py dumps/os_full.bin -o work/os
        -> work/os/functions.json, work/os/report.txt, work/os/os_full.elf
    python3 tools/os_analyze.py dumps/os_full.bin -o work/os --parts
        -> also splits the image into the per-CPU programs (PARTS, §8.1) and
           analyses each one at its own link address: work/os/<part>/...

The ELF (one section at 0x60000000, function + string symbols) loads in
Ghidra/IDA as ARM Cortex-M little-endian Thumb.
```

| option | help | default |
|---|---|---|
| `image` |  |  |
| `--base` |  |  |
| `-o, --out` |  |  |
| `--parts` | also analyse the per-CPU programs |  |

### `tools/patch_read.py`

```
patch_read.py -- try to read patch names from a SYSTEM-8 slot with RQ1.

    python3 tools/patch_read.py --slot 3 --patches 0-7 [--plugin plugins/JX-3P]

The plugin protocol only documents DT1 writes of the patch bank (docs/PROTOCOL.md
sec. 6); no read has been seen. This sends the standard Roland read request
(RQ1 = address + 4-byte size) for the NAME0 leaf of `fm.usrX` of each patch and
prints whatever comes back (decoded as 2 nibbles per character). Only RQ1 is
used: nothing is written to the device.
```

| option | help | default |
|---|---|---|
| `--slot` |  | `3` |
| `--patches` | 0-based range, e.g. 0-7 or 5 | `0-3` |
| `--plugin` |  |  |
| `--leaf` |  | `NAME0` |
| `--timeout` |  | `1.5` |
| `--backend` |  |  |
| `--port` |  |  |
| `--device-id` |  |  |
| `--gap-ms` |  | `0.0` |
| `--wire-pacing` |  |  |
| `-v, --verbose` |  |  |

### `tools/patches.py`

```
patches.py -- the patch bank the plugins write into a PLUG-OUT slot after the
firmware (JUNO-60.vst3 @ 0x1803551a5..0x1803552d0 and 0x180355af0).

* Memory layout of a patch: Script/Script.xml, struct types "patch",
  "patch2", "patch3" (fm.usrX, fm.usr2X, fm.usr3X). Every leaf struct is written
  as one DT1 message.
* Patch data: Patch/*.bin ("KoaBankFile0000N"). Each record holds the leaf
  memory concatenated in Script.xml order.
* Device addresses: computed per slot / patch / struct (@ 0x18036d270).

See docs/PROTOCOL.md section 6.
```

### `tools/plugout.py`

```
plugout.py -- Roland PLUG-OUT firmware containers (Script/Code*.Dat) and the
USB-MIDI install protocol used by the JUNO-60 / JX-3P / SH-101 plugins.

Everything here was taken from the plugins' own code (JUNO-60.vst3 addresses are
given; JX-3P and SH-101 contain the same routines). See docs/PROTOCOL.md.

Pure stdlib. The MIDI transport lives in midi_io.py.
```

### `tools/plugout_build.py`

```
plugout_build.py -- unpack, modify and rebuild PLUG-OUT firmware (Code*.Dat).

    python3 tools/plugout_build.py unpack plugins/JUNO-60/Script/Code8_1.Dat work/juno \
            --relocs-from plugins/JUNO-60/Script/Code8_2.Dat
    python3 tools/plugout_build.py show work/juno
    python3 tools/plugout_build.py set-id work/juno --name "JUNO-60 MOD" --model-id 0x40 --version 1.00
    python3 tools/plugout_build.py patch work/juno --region 1 --at 0x60a10800 --hex "00bf00bf"
    python3 tools/plugout_build.py pack work/juno -o out/Code8_2.Dat --slot 2
    python3 tools/plugout_build.py plugin work/juno --template plugins/JUNO-60 -o plugins/JUNO-MOD
    python3 tools/plugout_build.py elf work/juno -o work/juno.elf
    python3 tools/plugout_build.py verify plugins/*/Script/Code*.Dat

A project directory holds plugout.json, region<n>.bin (decompressed images)
and relocs.json (pointer offsets, needed to build for another slot). The
format is described in docs/CUSTOM_PLUGOUTS.md.
```

| option | help | default |
|---|---|---|
| `dat` |  |  |
| `dir` |  |  |
| `--relocs-from` | the same PLUG-OUT built for another slot (derives relocs.json) |  |
| `dir` |  |  |
| `other` |  |  |
| `target` |  |  |
| `dir` |  |  |
| `--name` | up to 15 ASCII characters |  |
| `--model-id` |  |  |
| `--version` | e.g. 1.02 |  |
| `dir` |  |  |
| `--region` |  |  |
| `--at` | address in the project's slot |  |
| `--hex` |  |  |
| `--file` |  |  |
| `--reloc` | address of a new absolute pointer word (repeatable) |  |
| `dir` |  |  |
| `-o, --output` |  |  |
| `--slot` | build for this slot (default: the project's) |  |
| `dir` |  |  |
| `-o, --output` |  |  |
| `--template` | plugin folder to take Script.xml and Patch/ from |  |
| `--slots` |  | `[1, 2, 3]` |
| `dir` |  |  |
| `-o, --output` |  |  |
| `dat` |  |  |

### `tools/plugout_image.py`

```
plugout_image.py -- the firmware carried inside a Code*.Dat, and how to rebuild it.

Layers (see docs/CUSTOM_PLUGOUTS.md):

    Code*.Dat (XOR container, plugout.py)
      -> 4096 (SYSTEM-8) / 2048 (SYSTEM-1) records of 512 flash bytes
           first half  = region 0 ("model": SYNINFO name/id/version, panel data)
           second half = region 1 ("engine": the sound engine code)
           unused blocks are sent as filler records and count as 0xFF
      -> region = 0x40-byte "SYSTEM-8:Plugout" header + LZSS stream (lzss.py),
                  padded with 0xFF to 512 bytes
      -> decompressed image, linked at a fixed address per slot

END message data = 16-bit sum of the whole flash image (all records, fillers
as 512 x 0xFF), sent as 4 nibbles. Verified on every shipped container.

Pure stdlib.
```

### `tools/plugout_tool.py`

```
plugout_tool.py -- install a Roland PLUG-OUT (JUNO-60, JX-3P, SH-101) into a
SYSTEM-8 / SYSTEM-1 over USB MIDI, without the VST plugin.

    python3 tools/plugout_tool.py ports
    python3 tools/plugout_tool.py info
    python3 tools/plugout_tool.py install JUNO-60 --slot 1
    python3 tools/plugout_tool.py inspect JUNO-60/Script/Code8_1.Dat

Replicates the plugin's install routine (JUNO-60.vst3 @ 0x1803547c0); see
docs/PROTOCOL.md for the reverse-engineered details.
```

| option | help | default |
|---|---|---|
| `--backend` | default: alsa on Linux, rtmidi elsewhere |  |
| `--port` | MIDI port (default: auto-detect the SYSTEM-8 CTRL port) |  |
| `--device-id` | SysEx device id (default 0x10 = 17) |  |
| `--gap-ms` | extra pause between messages (default 0; the plugin uses 10) | `0.0` |
| `--wire-pacing` | also wait the 31250-baud wire time of each message, as the plugin |  |
| `-v, --verbose` | print every message |  |
| `plugin` | plugin folder: JUNO-60, JX-3P or SH-101 |  |
| `--slot` | PLUG-OUT slot (default 1) | `1` |
| `--dat` | use this Code*.Dat instead of picking one from the plugin |  |
| `--code1` | use this file as Script/Code1.Dat, the SYSTEM-1 image (--dat would be read as Code8_<slot>.Dat on a SYSTEM-8 path) |  |
| `--flags` | bits 4-6 of the start byte (the plugin sends 0x40 when not authorized) | `0` |
| `--device-type` | with --skip-check/--dry-run: 0 SYSTEM-1, 1 SYSTEM-1m, 2 SYSTEM-8 (default) |  |
| `--skip-check` | do not query the device first |  |
| `--force` | install even if the plugin would refuse |  |
| `-y, --yes` | do not ask for confirmation |  |
| `--dry-run` | show what would be sent, no MIDI |  |
| `--bank` | patches to send, e.g. '2 Preset.bin:1-32' (1-based); repeat to combine banks, filled in order (default: the first file in <plugin>/Patch) |  |
| `--no-patches` | send only the firmware |  |
| `--patches-only` | send only the patch bank |  |
| `plugin` | plugin folder |  |
| `--bank` |  |  |
| `dat` |  |  |
| `--image` | write firmware regions to PREFIX<n>.bin |  |
| `--syx` | write the host message stream to a .syx file |  |
| `--slot` | slot written into the start message | `1` |
| `--flags` |  | `0` |
| `--device-id` |  |  |

### `tools/read_param.py`

```
read_param.py -- read the current value of a patch parameter by name with RQ1.

    python3 tools/read_param.py --slot 3 --patch 0                  # EFFECT TONE
    python3 tools/read_param.py --slot 3 --param "EFFECT TYPE" --watch
    python3 tools/read_param.py --list "TONE"                       # find names
    python3 tools/read_param.py --edit --watch                      # the live edit buffer
    python3 tools/read_param.py --edit --leaf NAME0 --watch         # current preset name
    python3 tools/read_param.py --edit --set 200                    # like a knob move
    python3 tools/read_param.py --write "VCF CUTOFF FREQ=100" --write "VCF RESONANCE=40"
    python3 tools/read_param.py --edit --param "VCF CUTOFF FREQ H"       # 32-bit (float bits)
    python3 tools/read_param.py --write "VCF CUTOFF FREQ H=0x3F000000"   # = 0.5f
    python3 tools/read_param.py --write "VCF CUTOFF FREQ H=0.5f"         # same, as a float

The parameter's address comes from the JX-3P Script.xml (the patch struct the
value lives in + its offset). Values are `int8x4`: 8 nibbles, one per byte on
the wire, most significant first. Only RQ1 is sent: nothing is written.

--watch polls and prints on every change (turn the knob on the panel: this
shows whether RQ1 follows the live edit buffer or only the stored patch).
```

| option | help | default |
|---|---|---|
| `--slot` |  | `3` |
| `--patch` | 0-based preset number | `0` |
| `--param` | parameter name (default "EFFECT TONE") |  |
| `--group` | disambiguate names that exist in several groups (default: first match) |  |
| `--write` | write one or more parameters into the EDIT BUFFER, e.g. --write "VCF CUTOFF FREQ=100" --write "VCF RESONANCE=200"; values outside the declared range are refused unless --force | `[]` |
| `--force` | allow values outside the declared range |  |
| `--list` | list parameters whose name contains TEXT |  |
| `--edit` | read the current (edit) patch buffer `fm.PATCH` (0x03000000) instead of the stored patch `fm.usrX`; --slot/--patch are ignored |  |
| `--leaf` | read a whole leaf (e.g. NAME0) raw instead of one value; printed as nibbles and decoded as 2 nibbles per character |  |
| `--set` | WRITE the value into the EDIT BUFFER (needs --edit): the panel parameter changes like a knob move (temporary, not stored) |  |
| `--watch` | poll and print on every change |  |
| `--poll` |  | `0.2` |
| `--plugin` |  |  |
| `--timeout` |  | `1.5` |
| `--backend` |  |  |
| `--port` |  |  |
| `--device-id` |  |  |
| `--gap-ms` |  | `0.0` |
| `--wire-pacing` |  |  |
| `-v, --verbose` |  |  |

### `tools/svc_table.py`

```
svc_table.py -- decode a dump of the OS service table (0x0113FC00) and list the
functions it points to.

    python3 tools/svc_table.py os_svc.bin [--base 0x0113FC00]

Every aligned u32 is classified:
  * odd value inside the kernel image (0x01000400..0x01019A08): Thumb function
    (the code at 0x01000400 is a RAM copy of image 0x602B9774..., see
    docs/BASE_FIRMWARE.md section 6)
  * value inside the OS image (0x60000000..0x602CDA70): pointer into the image
  * value in RAM (0x01xxxxxx / 0x20xxxxxx): data pointer
  * small values: counters / flags
It also prints the dump_loop.py commands that read the first 0x20 bytes of every
distinct function, to be analysed with arm-none-eabi-objdump.
```

| option | help | default |
|---|---|---|
| `file` |  |  |
| `--base` |  | `18086912` |
| `--slot` |  | `3` |

### `tools/sys8fw.py`

```
sys8fw.py -- unpack, inspect, patch and rebuild the SYSTEM-8 updater (SYSTEM8_UP.BIN).

    python3 tools/sys8fw.py info    firmware/SYSTEM8_UP.BIN
    python3 tools/sys8fw.py unpack  firmware/SYSTEM8_UP.BIN work/up
    python3 tools/sys8fw.py pack    work/up -o out/SYSTEM8_UP.BIN
    python3 tools/sys8fw.py verify  firmware/SYSTEM8_UP.BIN        # unpack + pack == original
    python3 tools/sys8fw.py symbols work/up --elf-dir work/elf       # RTTI classes, vtables, ELF

What is decoded: the tar container, the PRELOAD_PLUGOUT wrapper, the
"SYSTEM-8:Plugout" regions (LZSS) of slot 0 and the factory PLUG-OUTs.
What is NOT: the `App1_Main` OS file, which is encrypted with a 64-bit block
cipher whose key is not in the updater (docs/BASE_FIRMWARE.md). It is carried
through unchanged unless you supply a replacement with `--os`.
```

| option | help | default |
|---|---|---|
| `up` |  |  |
| `up` |  |  |
| `dir` |  |  |
| `dir` |  |  |
| `-o, --output` |  |  |
| `--os` | replacement App1_Main file (already encrypted) |  |
| `up` |  |  |
| `dir` |  |  |
| `--elf-dir` |  |  |
| `--json` |  |  |
| `dir` |  |  |
| `--out` |  |  |
| `dir` |  |  |
| `--image` | file name, e.g. p1y01_p0s.region0.bin |  |
| `--at` |  |  |
| `--offset` |  |  |
| `--hex` |  |  |
| `--file` |  |  |
| `dir` |  |  |
| `--out` |  |  |
| `--objdump` |  | `arm-none-eabi-objdump` |

### `tools/tools_usage.py`

```
Write docs/TOOLS.md: every tool of the repository with its usage and parameters.

    python3 tools/tools_usage.py            rewrite docs/TOOLS.md
    python3 tools/tools_usage.py --check    exit code 1 if docs/TOOLS.md is out of date (for tests / CI)

The hand-written guide (docs/TOOLS_GUIDE.md: builds, simulation, checks, install) is copied first; then, for each
Python tool in the directories below, the module docstring and the command-line options found in its argparse calls
(add_argument). Files are read with ast, never imported or run. Build scripts (build.sh) contribute the comment
block at their top or their usage() text.
```

Environment: `NAME` (default default)

## DSP assembler and disassembler (`asm/`)

### `asm/asm.py`

```
Assembler: dspasm source -> program words (DSP_ISA_SPEC.md §3, §4, §12).

    result = Assembler().assemble(text)
    result.words        flat 16-bit words in execution order
    result.errors       [(line, message)] — rejected, never guessed
    result.notes        [(line, message)] — assembled but flagged

The language is what `disasm.program` writes back:

    label:  unit [| unit ...]          ; comment
    mac_p1p2 r0, r0, r1, r2, r1
    raw @0x060 0fe0 0201              ; verbatim body words, optional class

Directives, each on the line before the instruction it changes:

    .class 0x0040        the class of the next instruction (needed when two
                         classes fit the units, or for a `raw` line)
    .free @n 0xMASK      bits of word n (0 = header) that the variant leaves
                         free and that the listing restores
    .slot @k 0xNNNN      slot word k, which no operand reads (spec 4.4)
    .words 0xNNNN ...    words kept as they are (a truncated tail)

Immediates (spec 4.4): `#v@b:w` is a value read at byte b of the slot array,
w bits wide. A MAC coefficient `#v` is read in order after Y's immediate, or
at the byte `@b` when Y is a register. An immediate with no byte is placed by
the assembler at the first byte where its bits agree with the stream.

Confidence policy is enforced **here**, not by the caller: a line whose
variant is not in `Table.EMIT_DEFAULT` (exact, measured) is rejected unless
`allow_inferred=True` is passed, which admits `inferred` and `fitted`. `raw`
words and `.words` are never emitted by default either: they are unknown
encodings kept verbatim, so re-assembling them takes the same explicit option.

Ambiguity is rejected: if more than one class can hold the units, or a field
that must be preserved has no source, the line fails with the reason.
See docs/REVIEW.md for the 2026-10-20 review that set these rules.
```

### `asm/codec.py`

```
Generic codec: the only place that knows how a table entry maps to words.

Nothing here knows a mnemonic. A unit is decoded by testing each variant's
`when` conditions; a word is only decoded when the chosen variant explains
*every* field of the layout, otherwise the caller keeps it verbatim.

    codec.decode(form, words)                -> (Variant, fields) | None
    codec.encode(form, variant, args)        -> words

Operands travel as `Arg` (parsed source). An immediate operand carries its own
width (`bits`) and byte (`at_byte`); an instruction may have several (a MAC
reads up to three, spec 4.4). Placing the bits in the slot array is the
assembler's job (asm.py), not this module's.
```

### `asm/disasm.py`

```
Disassembler: program words -> dspasm source (spec §3, §4, §12).

    text, report = disasm.program(words, table)

Honesty rules:

* A word the table does not explain becomes `raw` with its words verbatim —
  never an invented mnemonic (spec §14).
* `raw` also carries the class when the class would not follow from the
  units, so re-assembling always reproduces the original header.
* Each line ends with a status tag (`exact`, `measured`, `inferred`,
  `fitted`, `raw`) so confirmed and inferred decodings stay distinguishable.
* `report` counts what happened and lists every instruction that stayed
  verbatim, with the reason.

Layout (§4.3): the units fill a class's prefix (up to its last T); every word
after that is the slot array. Its first M words are the N words of the MACs;
the words a transfer points at are references; the rest is the immediate bit
stream (§4.4). `layout.py` holds that rule, and the assembler uses the same
one, so the two cannot disagree about which word is which.

Each line is checked before it is printed: the listing re-assembles it, and a
line that does not give back its own words is kept verbatim instead. That is
the reason `report["reassembly"]` exists; on the corpus it is 0.
```

### `asm/dspbuild.py`

```
General build pipeline for our DSP microcode, SYSTEM-1 and SYSTEM-8 (docs/MICROCODE_OPTIMIZATION.md §6).

A generator only writes the plain program: one op or one transfer per line, load delays as "mov rN, rN" fillers,
';' comments. Everything else is here, the same for every plug-out:

    import dspbuild
    b = dspbuild.build(lines, dspbuild.S1)        # framing S1 (SYSTEM-1 section) or S8 (SYSTEM-8 slot / main)
    b.words, b.text, b.plain_words                # optimized words with the framing, its text, the plain words
    dspbuild.write('s1m0', out_dir, b)            # s1m0.s / s1m0.hex + check_optimized warnings

build() runs, in order (each can be turned off for one call, or for every build with DSP_OPT=0):
  1. phwrap: the manual 11-op phase wrap of our early programs (addxy t, x, #-1 / max t, t, r5 / (mul t, t, #256 /
     min t, t, r6) x4 / mul t, t, #2 / subxy x, t, x) becomes ph_wrap x, r3, x (Roland's form), when the temporary
     t is dead afterwards. It gives the same result except for a phase of exactly 1.0 (manual: 1.0, ph_wrap: -1.0).
  2. schedule: asm/dspsched.py (dependencies, packing, op fusion, reordering).
  3. assemble, with the framing words around the code.
  4. verify (DSP_VERIFY=0 to skip): the scheduled program against the plain one (after phwrap) in the emulator's C
     core, on 4 random states (registers, banks 0 / 1 / 5, bank 6, IOMD: values on the quant grid, as a program
     or the ARM writes them) for 6 samples each; every register, flag and memory word must be equal. A difference
     raises. Programs already verified are remembered by their words (asm/build/verified.txt).

    python3 asm/dspbuild.py FILE.s [--s1|--s8] [-o DIR]    a hand-written plain .s -> optimized .s / .hex
```

### `asm/dspsched.py`

```
List scheduler for our DSP microcode (SYSTEM-1 and SYSTEM-8: same ISA). docs/MICROCODE_OPTIMIZATION.md §5.

    import dspsched
    packed = dspsched.schedule(lines)          # one-unit lines (with "mov rN, rN" fillers) -> instruction texts

The input is the plain program the generators write: one op or one transfer per line, a load delay written as a
"mov rN, rN" filler, comments with ';'. The output does the same thing in fewer instructions: independent ops share
an instruction ("op1 | op2 | op3"), loads and stores go into op instructions, the fillers are gone, and the units are
reordered as far as the dependencies allow (pack() of make_synth.py only merged a line into the instruction before
it, in program order).

Semantics the schedule keeps (the emulator's, measured on the hardware; DSP_HARDWARE.md, pack() comment):
  * every unit of an instruction reads before any unit writes;
  * a loaded register is usable from the next instruction on; a store reads the register as the instruction starts;
  * memory: a load after a store to the same address comes one instruction later; a store after a load may share
    the load's instruction; two stores to one address keep their order (one instruction apart).

Dependencies (i before j in program order -> minimum instruction distance):
  register read after write (op or load result) 1; write after read 0; write after write 1;
  same address (normalised: banks 0 / 1 / 5 repeat every 0x800, IOMD every 0x2000): store -> load 1, load -> store 0,
  store -> store 1. Anything the scheduler does not understand (an unknown op, a condition) is a barrier: everything
  before it comes first, it gets an instruction of its own, everything after it comes after.

Limits per instruction (the ones proven on the device with pack() / FUSE: C-S1SY8 .. C-S1SY11): at most 3 ops, all of
the one-destination kinds in OPS; at most 3 transfers, no address twice, bank-0 loads <= 2, bank-0 stores <= 2, any
other bank <= 1 load and <= 1 store; and the instruction must assemble (asm.Assembler).

Priority: the longest dependency path to the end of the program (critical path), then program order. Validation:
the scheduled program must give the same samples as the unscheduled one in the emulator (make_synth.py --pack-check,
test_sched.py).
```

### `asm/isa.py`

```
The ISA table as objects. Pure interpreter: nothing here knows a mnemonic.

    table = isa.Table.load()          # reads isa.json next to this file
    table.classes[0x048]              # -> Class(layout='A', status='measured', ...)
    table.mnemonics['mul']            # -> Variant(...)
    table.constraints                 # per-form rules the codec applies

Everything the codec needs comes from the JSON: field positions, variant
selectors, fixed bits, operand wiring and status tags.
```

### `asm/layout.py`

```
Program layout rules shared by the disassembler and the assembler.

A class of isa.json fixes a layout (spec §12). Its *prefix* (see prefix_length)
is what the units fill; the rest is the slot array, whose length an instruction
chooses. An instruction can be longer (more immediate words) or shorter than the
layout's letters. Both tools must agree on how the letters map to body words, or
a round trip breaks at the first unusual instruction.
```

### `asm/s1rules.py`

```
The SYSTEM-1 access rules, checked on scheduled programs (one instruction per line, units joined by ' | ').

Breaking one of them hung the device (docs/examples_system_1_jx_README.md §5, measured by on-device bisection):
    R2  bank 5: at most 1 load and 1 store per instruction
    R3  bank 0: at most 2 loads and 2 stores per instruction
    R4  bank 6: at most 2 loads per instruction
    R5  one bank-5 access per instruction (load or store)                         [our rule, inferred]
    R6  at most 2 loads of banks 5 + 6 + IOMD per instruction (the S1 never has more)  [inferred]
    R7  a slave DSP stores to bank 5 only in A100..A1FF (A000..A0FF is what the ARM writes)
    R8  ops sharing an instruction: only the groups proven on the device (asm/proven_forms.txt)
and, as a warning: an instruction shape (accesses per bank, number of ops) that is not part of a shape of a program
that ran on the device (PROVEN_PROGRAMS: C-S1V8, C-S1SY13).

    import s1rules
    s1rules.check('s1m0', lines)              -> list of warnings; SystemExit on a broken rule
    python3 asm/s1rules.py FILE.s ...           the same from the command line
```

### `asm/slots.py`

```
The slot array of one instruction (DSP_ISA_SPEC.md §4.3).

    The slot array = every word after the last T.  Slot k = the (k+1)-th word.
    N words come first (one per M). Among the remaining slots, the ones a
    memory transfer (mode fe) points to are references; the ones nobody points
    to are the immediate bit stream, in order.

Both directions use this module. Each word of the array gets at most one owner
(N word, reference, jump offset, `.slot` word); a second claim with a different
value is a conflict, reported, never resolved by order. Immediate bits are not
owners: they are laid over the array and must agree with the owner they fall on.

Two facts from §4.3 that a count of R words cannot express:

* a transfer addresses slot `Y >> 1` -- an *index*, which may be larger than
  the number of references (the corpus has transfers pointing at slot 7 while
  only four references exist), and two transfers may share one slot;
* the N words are part of the array, so slot 0 is the first M's second word
  whenever the class has an M.
```

## Emulator (C core, native code) (`examples/from_scratch/emu/`)

### `examples/from_scratch/emu/allplugouts.py`

```
The data-flow facts of docs/log/VOICE_ALLOCATION_LOG.md §6, checked on every Roland PLUG-OUT image in the repo (study only).

    python3 allplugouts.py            images: work/unpack/*/region1.bin and the JX-3P of engine/build/jx

For each image: the six main programs (descriptors with slot 0x0100, in image order = DSP0..5, re/alloc_scan.py) and
the slot programs (0x6000 / 0xB000). Per image it prints
  * pair window: loads / stores of bank 5 0x400-0x7FF per main (A4xx);
  * bank 1: the words that slot programs store, and how many of them the mains read as stored or at +1;
  * MXM: main copies `M 0nn -> Z 4nn` with the same nn, and with another nn (re/listing.forwards rule);
  * IOMD words written by each main.
Nothing is copied: the programs are disassembled in memory from the local images.
```

### `examples/from_scratch/emu/cchain.py`

```
chain.Slave / chain.Bmc2 on the C core (dspcore.c): same constructor, same ARM-side calls, same results.

    import cchain
    s = cchain.Slave(voices.programs('v4'), chain.Model())
    s.p_bank(0, 0x21, 0.5); s.step(); s.iomd.get(0x6020, 0.0)
    s.run(n, (0x6020,))          # n samples in C, returns {word: [values]} (no per-sample Python call)

The shared library is built with cc the first time (and whenever dspcore.c / dspcore.h is newer). The instruction
semantics are those of re/dspisa.py, decoded once by dspcomp.py; test_core.py checks them against dspisa/chain.py.

Slave(..., native=True, threads=True): programs translated to native code (dspjit.py, cached in build/jit/) and the
independent DSP groups of a sample on their own threads (for run() / step(n) with n >= 64). Both give the same bits
as the interpreter (native=False, threads=False).
```

### `examples/from_scratch/emu/chain.py`

```
Emulator of one BMC slave (six DSPs) and of the two slaves, for the voice chain studies of docs/VOICE_ALLOCATION.md.

Every memory access of re/dspisa.py goes through dspisa.ACCESS, so the banks are modelled here, per DSP:

    bank 0 (0nnn)  ring of 0x800 words: what is stored at R in sample n is read at R+1 in sample n+1
                   [medido, DSP_HARDWARE.md 7.2]
    bank 1 (2nnn)  0x800 words; a ring like bank 0 when Model.ring1 (hypothesis of question D), else plain memory;
                   Model.shared1 = pairs of DSPs with one bank 1 (question E)
    bank 3 (6nnn)  IOMD: one block for the six DSPs; writes persist [medido, 8.3]. Ownership is not modelled.
    bank 4 (8nnn)  MXM: point-to-point, latched once per sample, 1 sample latency, no hold (a word nobody wrote
                   in the previous sample reads 0) [medido en el maestro, 8.2]. Inputs of DSP r (r >= 2):
                   M[i] <- Z[0x400 + i - Model.mxm_shift] of the even DSP of the previous pair, O[i] <- Z of the
                   odd one; S[0x600 + i] of DSP r <- writes of DSP r+2 to M (r even) or O (r odd) [listado].
    bank 5 (Annn)  0x000-0x3FF: own persistent memory (DPC) [medido]. 0x400-0x7FF: Model.dp_pairs -> one window
                   per pair (0,1), (2,3), (4,5) (DP01 / DP23 / DP45), latched like MXM when Model.dp_delay = 1
                   (hypothesis of question B); otherwise own memory.
    bank 6 (Cnnn)  parameters, read only: Slave.params (default 0).

Inside one sample the DSPs run in Model.order; in DSP2 / DSP3 the main program runs before the slot programs
when Model.main_first (question F). Registers, flags and hidden registers persist per DSP.

What is NOT modelled: the ARM timing (engine writes are injected between samples), the effect slots, bank 2
(delay lines), the wave bus beyond the two-slave link of Model.link (question C), the real IOMD ownership.

The emulator contains no Roland code: the programs are passed in by the caller. Today the tests pass ours (v3/make_v3.py,
v4/make_v4.py); running Roland's programs, loaded at run time from the local copy, is allowed for study (the
no-Roland-microcode rule applies only to our own plug-outs, docs/log/VOICE_ALLOCATION_LOG.md §7).
```

### `examples/from_scratch/emu/dspcomp.py`

```
Front end of the C core (dspcore.c): every instruction of a chain.Program becomes a list of units and conditions.

The decoding is re/dspisa.py's own (dspisa.Layout: template, slot area, immediate stream, units, jumps); this module
only replays the immediate reads of the A / B / M units in run()'s order and packs the result into ctypes arrays.
What depends on run-time values (registers, flags, hidden registers, memory) is left to the C core.
```

### `examples/from_scratch/emu/dspjit.py`

```
Native translation of DSP programs for the C core (docs/EMULATOR.md §3.6 point 2), and the thread groups.

Every program becomes one C function: the instruction words are constants, jumps are gotos, the conditions are
tests of a local flags word, and the units call the inline functions of dspcore.h that the interpreter uses (with
constant words gcc folds them to the few operations each unit really does). The translation adds nothing to the
semantics; it only drops work whose result is known:

* quant(): dspisa quantises all eight registers after every A/B/M unit. quant is idempotent, so a register needs it
  only if it may hold an unquantised value: one written by a unit (superset by unit type), or loaded since the last
  unconditional unit, or anything at a jump target / program start;
* the result and flags of a unit are kept only when an X9bY of the same instruction reads them.

The functions of one program set go into one shared library, built by cc and cached by the hash of its source in
emu/build/jit/. test_core.py compares the translation with the interpreter and with chain.py.

groups(): DSPs that share an IOMD word (one writes a word another reads or writes, static addresses: a transfer of
mode fe; the indexed modes only reach banks 0 and 5) or bank 1 (Model.shared1) must run on one thread, in model
order; the other DSPs of a sample are independent (MXM and the pair window are latched).
```

Environment: `DSP_CC`

### `examples/from_scratch/emu/gen_qmask.py`

```
Generate qmask.h: for each biased exponent field of a double, the mask of the bits quant() keeps (dspcore.h).

    python3 examples/from_scratch/emu/gen_qmask.py

k = 20 + 4s - E low bits are cleared (s = clamp((E + 7) >> 2, 0, 3), E = unbiased exponent); k > 52 -> 63 (a signed
zero); E >= 9 (|v| >= 512, inf, nan) -> nothing cleared. test_core.py test_quant checks quant against quant_ref.
```

### `examples/from_scratch/emu/listing.py`

```
Bus references of Roland's JX-3P programs, read from their listings (study only; nothing is copied or executed).

The programs come from engine/build/programs.h (local, copied from the user's PLUG-OUT by engine/programs.py; MAIN0
and MAIN1 carry the envelope patches of programs.py, which do not touch any bus reference) and are disassembled with
asm/disasm.py, as dsp/dump_main.py does.

    python3 listing.py MAIN4                  loads and stores of banks 1, 3, 4, 5 per instruction
    python3 listing.py --summary              the cross-program facts used by docs/log/VOICE_ALLOCATION_LOG.md §6
```

### `examples/from_scratch/emu/sigcheck.py`

```
Signature check: no Roland microcode in a build or in our programs (docs/log/VOICE_ALLOCATION_LOG.md §7).

    python3 sigcheck.py FILE...      FILE = a built region (.bin) or a program (.hex / .txt, 16-bit words)

The Roland programs are the ones engine/programs.py copies at build time (MICROCODE: MAIN0-5, DCO, FX), read from
engine/build/programs.h (local, not in git). A .bin is searched for every 16-byte window (4 aligned 32-bit words, as
stored in the image) of every Roland program; a .hex for every run of 8 consecutive 16-bit words. Windows that also
occur in our own programs (generic markers such as `0a02 ff70`) are ignored. Exit status 1 if anything matches.
```

### `examples/from_scratch/emu/voices.py`

```
Program sets (ours only) and the ARM-side voice writes of v3/engine_v3.c, for chain.py.

python3 voices.py v3|v4 [SAMPLES]        one voice at a time on slave 0: level and frequency at 6020
```

## DSP research tools (SYSTEM-8) (`examples/from_scratch/re/`)

### `examples/from_scratch/re/alloc_mxm.py`

```
Bus references of every main program of a Roland region 1 image: MXM (bank 4, 8nnn), IOMD (6nnn) and bank 5
(annn), read and written, from the disassembly (asm/disasm.py). Used for docs/VOICE_ALLOCATION.md.

    python3 alloc_mxm.py region1.bin BASE_HEX OFFSET:SIZE [OFFSET:SIZE ...]
    e.g. python3 alloc_mxm.py region1.bin 60A00000 015270:5066 016918:5018
The program words are read as dump_main.py does (uint32 little-endian, upper half first).
```

### `examples/from_scratch/re/alloc_scan.py`

```
Descriptor table of a Roland region 1 image: every DSP program descriptor (slot address, size, size, program
pointer) with its slot. Used for docs/VOICE_ALLOCATION.md.

    python3 alloc_scan.py region1.bin 60A00000     # BASE = the slot run address of the region 1 image (slot 1)
```

### `examples/from_scratch/re/deps.py`

```
Which parameters change what the JX-3P engine writes when parameter X
changes (reference measurement for the model's combined outputs).

usage: python3 re/deps.py out.json [slave]
```

### `examples/from_scratch/re/draft_model.py`

```
Draft of the engine's parameter model from the measured responses
(re/measure.py): one expression per (source id, curve), targets grouped.
Prints Python for engine/model_spec.py, to be curated by hand.

usage: python3 re/draft_model.py meas.json > draft.py
```

### `examples/from_scratch/re/dsp_corpus.py`

```
Collect DSP microcode blobs from SYSTEM-8 images and print word statistics.

usage: python3 dsp_corpus.py outdir name=image.bin [name=image.bin ...]

A program is found through its descriptor {u32 dsp_addr, u32 bytes, u32 bytes,
u32 src} (dsp_addr < 0x10000 and even) whose src points at a blob starting with
70 ff 02 0a. The load address of each image is guessed from those descriptors.
Unique blobs are written to outdir as <name>_<dsp_addr>_<src>.bin.
```

### `examples/from_scratch/re/dsp_corpus_all.py`

```
Collect EVERY DSP program found in Roland images (2026-10-17): descriptor scan (dsp_corpus.scan) plus a
signature scan (a 4-byte-aligned `0a02 ff70` that is not an end marker, parsed instruction by instruction up to
the end marker `0a02 ff70 0b02 00f8 0001`). Unique programs (by word sequence) go to OUTDIR as
<source>_<offset>.txt (16-bit words in execution order); prints which ones are new w.r.t. work/corpus.

    python3 dsp_corpus_all.py OUTDIR name=image.bin ...
```

### `examples/from_scratch/re/dsp_progs.py`

```
List / extract the DSP microcode that the JX-3P engine (region 1, slot 1) hands to the slave.

usage: python3 dsp_progs.py region1.bin [outdir]

Runs the main object's vt+8 (DSP load) in emu1 for every DCO1/DCO2 waveform and
effect type, logs the OS table calls +0x28..+0x6C and prints the descriptors
{DSP address, bytes, bytes, src}. With outdir, writes each program blob there.
```

### `examples/from_scratch/re/dsp_s1.py`

```
DSP programs of SYSTEM-1 images (2026-10-17): no descriptor table like the SYSTEM-8; programs start with
`0a02 ff71` (or follow an end marker) and end with `0a02 ff70 0b02 00f8 0001`. Segments the image between end
markers and keeps the segments that parse as whole instructions.

    python3 dsp_s1.py OUTDIR name=image.bin ...
```

### `examples/from_scratch/re/dsp_wdiff.py`

```
Align two DSP microcode word streams and classify the words that differ.

usage: python3 dsp_wdiff.py a.bin b.bin      two programs (e.g. DCO1 vs DCO2 copy)
       python3 dsp_wdiff.py a.bin --halves   a program made of two voice copies

Words that differ between copies of the same code are operands; the output
groups them by high byte and delta, e.g. "c0 +0x16" = parameter ids, voice
stride 0x16. Words that never change are operations.
```

### `examples/from_scratch/re/dspasm.py`

```
Minimal assembler for the established DSP subset (DSP_ISA_SPEC.md) + our own SAW oscillator.

    python3 dspasm.py saw dco2 [--fs 48000] [-o prog.txt]   # oscillator for the DCO2 (or dco1) slot
    python3 dspasm.py sim prog.txt --pitch 4.75 [--n 400]    # run it in dspisa's simulator, print period

Encoders (each returns one instruction as a list of 16-bit words):
    a(op, d, x, y=None, imm=None)   unit A op (class 0x048); y = register (negative int = negated) or imm (float)
    b(op, d, x, y)                  unit B op (class 0x050), d >= 8 forms (min = op 0, max = op 2), x in r0..r3
    ld(r, addr), st(r, addr)        transfer with one reference (class 0x060)
Every instruction is checked by decoding it again with dspisa.
```

| option | help | default |
|---|---|---|
| `cmd` |  |  |
| `arg` |  |  |
| `--fs` |  | `96000.0` |
| `--level` |  | `1.0` |
| `--pitch` |  | `4.75` |
| `--n` |  | `400` |
| `--slot` |  | `dco2` |
| `-o` |  |  |

### `examples/from_scratch/re/dspaudio.py`

```
DSP ISA test bench on the hardware: test programs in the DCO2 slot, recorded by USB audio.

    python3 dspaudio.py install b11.batch       # build plugins/C-DSPT, install in slot 3 (--force)
        (then select PLG3 on the panel: after an install the device is back on SYSTEM-8)
    python3 dspaudio.py recall b11              # eight notes, one per voice (VOICE_NOTES)
    python3 dspaudio.py rec name [--note 60]    # one note, one recording
    python3 dspaudio.py show work/dspaudio/x.raw

A batch maps names to test programs (`t0=x.txt`, `t1=`, and program-name
prefixes such as `dco1_=z_dco1.txt`; files in re/dspexp/audio): 16-bit words in
execution order (hex, '#' comments), as printed by dspdis.py. engine/build.sh
with DSPTEST=1 MC_TEST=<spec> puts them in place (programs.py): the DCO2 slot of
DSP2 runs t0 and the one of DSP3 t1, loaded at start-up (engine.c -DDSPTEST).
Each test program has a voice A path (outputs [2221] [221d]) and a voice B path
([2229] [2225]), so one install carries four tests; the assigner gives eight
different notes eight voices (2 slaves x DSP2/3 x A/B; map: calibration batch
b11). Live program swaps (+0x70) from note events were tried and were not
reliable (programs silent or the previous one kept).
Recordings: work/dspaudio/<name>.raw (float32 interleaved L R, 96 kHz).
```

| option | help | default |
|---|---|---|
| `cmd` |  |  |
| `arg` | build/install: spec or .batch; rec: name; recall: batch name; show: .raw |  |
| `--note` |  | `60` |
| `--ch` |  | `0` |
| `--secs` |  | `2.0` |

### `examples/from_scratch/re/dspb0dly.py`

```
Slave test of the indexed bank-0 delay (DSP_USOS §2): out = (saw + saw delayed k samples)/2.

python3 dspb0dly.py 92 out.txt; python3 dspown.py load out.txt; python3 dspharm.py dly 72   # 2026-10-17: f0 x 2
```

### `examples/from_scratch/re/dspb0fix.py`

```
Fixed table in bank 0 of idle master DSP3, one program; the write block is switched off by a jump
(one 32-bit word changes). h6 = T + step*n, n = bank-5 sample counter.

    python3 dspb0fix.py 1      # 2026-10-17: table kept 3 s; one plain bank-0 store wipes it (< 10 ms)
```

### `examples/from_scratch/re/dspb5alias.py`

```
Bank-5 aliasing (2026-10-17): indexed stores at 0x3b0, 0x7b0, 0xbb0, 0xfb0, 0x13b0, 0x23b0, read back in the same
sample and later: persistent words = index mod 0x400; bit 10 set = same-sample only.

    python3 dspb5alias.py
```

### `examples/from_scratch/re/dspb5idx.py`

```
Bank-5 indexed mode `da` on idle master DSP3 (2026-10-17): wrap / truncation / latency of h6, and the indexed
store `9daY` (read back by a plain load and by the ARM). Bank 5 saved and restored.

    python3 dspb5idx.py
```

### `examples/from_scratch/re/dspb5map.py`

```
Bank 5 beyond 0x3FF, h7 (`db`/`df`) and the time-varying modes d8/d9/dc/dd on idle master DSP3 (2026-10-17).

python3 dspb5map.py
```

### `examples/from_scratch/re/dspb5osc.py`

```
Slave test: oscillator reading a table from bank 5 with the indexed mode `0daY` (h6 = base + phase*N).
F = fill table (plain bank-5 stores, every sample) + play; P = play only (table must persist).
    python3 dspb5osc.py F|P [-N 16] [--base 0x100] [--wave tri|sq] -o file
    python3 dspown.py load file; python3 dspharm.py NAME 48 60     # measured 2026-10-17: tri / sq harmonics exact
```

| option | help | default |
|---|---|---|
| `mode` |  |  |
| `-N` |  | `16` |
| `--base` |  | `256` |
| `--wave` |  | `tri` |
| `-o` |  |  |

### `examples/from_scratch/re/dspbits.py`

```
Unit-B ops with D < 8 (nibbles 0-b): bitwise ops on two's-complement fixed point, results modulo 512 (a negative
result reads as 512 + x: the old `7ffxxxxx` format). Master DSP3 probes with random registers, then every candidate
function is checked (2026-10-17).

    python3 dspbits.py [word ...]       # default 0001 0201 0401 0601 0801 0a01 0101
```

### `examples/from_scratch/re/dspbudget.py`

```
Time budget of a master DSP (default: idle master DSP3) and the cost of multi-op instructions, without audio.

    python3 dspbudget.py single 100 400 800 ...     # K single-op fillers (A: r4 = r5) before a counter
    python3 dspbudget.py aab 100 300 ...            # K A/A/B fillers (r4 = r5 | r3 = r5 | r2 = max(r1, r5))
    python3 dspbudget.py skip 500 1500 ...          # a jump over K single-op fillers (do skipped words cost?)
    python3 dspbudget.py loop 100 200 ...           # a loop of K iterations (4 instructions each) before the counter

The program is K fillers followed by a counter kept in the ring memory (load [S + 1], add 2^-16, wrap to [-1, 1), store [S]):
while the whole program fits in one sample period the counter grows by 96000 * 2^-16 = 1.46 per second; when the
fillers push the counter past the end of the budget it stops (or slows). Comparing the K where that happens for
single-op and A/A/B fillers gives the cost of an A/A/B instruction. The context's original program is saved and
restored (dspmaster.Session, area enlarged to the longest program).
```

| option | help | default |
|---|---|---|
| `kind` |  |  |
| `k` |  |  |

### `examples/from_scratch/re/dspbus.py`

```
Where do bank-3 (IOMD, `6nnn`) and bank-4 (MXM, `8nnn`) stores go? Master DSP3 stores markers every sample to
the given references; the ARM scans the MXM windows of all six DSPs and the IOMD block for them (IEEE or RAW
encodings). Marker of reference R: 0.125 + R/65536 (unique, exact in both formats).

    python3 dspbus.py 85F0 60D8          # references to mark (8 consecutive words each)
```

Environment: `BUSDSP`

### `examples/from_scratch/re/dspbus2.py`

```
Inter-DSP bus test: master DSP3 (main program) stores markers (dspbus.marker) to one MXM zone, master DSP4
(slot 0xF7DC, replacing the global delay for a moment) loads candidate references of all four zones and stores
them to bank 5 [A3C0..]; which loads return DSP3's markers shows the routing as seen by a DSP.

    python3 dspbus2.py 85F0             # DSP3 writes 85F0..85F7
```

### `examples/from_scratch/re/dspbusclk.py`

```
MXM latency with a common clock (2026-10-17): DSP3 sends its delay-ring write position (`f5c`, -1/16384 per
sample) on MXM Z 85F0; DSP4 (slot 0xF7DC) reads O 83F0 and its own `f5c`, stores the difference in samples.
If the position is a shared clock, the difference is the bus latency.
```

### `examples/from_scratch/re/dspbuslat.py`

```
Inter-DSP transfers on the master, measured (2026-10-17): DSP3 (main program) keeps a sample counter (+2^-16 per
sample, bank 5) and publishes it on MXM (Z 85F0) and IOMD (6030); DSP4 (slot 0xF7DC, the global delay replaced for
a moment) copies MXM O 83F0 -> IOMD 6032 and IOMD 6030 -> IOMD 6033; DSP3 reads 6032/6033 at the start and at the
end of its program and stores counter - received (= round-trip latency in samples).

    python3 dspbuslat.py [--dsp4-first]
```

### `examples/from_scratch/re/dspbussync.py`

```
Timing of the MXM bus between master DSP3 (main) and DSP4 (slot 0xF7DC), 2026-10-17. DSP3 writes its sample
counter to Z 85F0 at the start and to 85F1 at the end of its program (N no-ops apart); DSP4 reads O 83F0/83F1 at
the start and at the end of its program (N no-ops apart) and also its own copy of DSP3's counter is not available,
so differences between the four reads (in samples) tell whether the bus is live or latched per sample.

    python3 dspbussync.py [--nops 200]
```

| option | help | default |
|---|---|---|
| `--nops` |  | `[10, 200]` |

### `examples/from_scratch/re/dspclass.py`

```
Measure corpus instructions on a master DSP and fit what each one computes (exact, dspfit search).

    python3 dspclass.py list                       # header classes by frequency, with sample instructions
    python3 dspclass.py run 0x0a8 [-k 4] [-n 6]    # the k most frequent instructions of class 0x0a8
    python3 dspclass.py run all -k 3               # every class, by frequency
    python3 dspclass.py ins "1503 2987 7505" ...   # given instructions

For each instruction: its memory references (one per transfer, dspdis.fields) are moved to a free area
of master DSP4 bank 0 ([0790+k]) and preloaded with distinct values; r0..r7 get random values; the
instruction runs; r0..r7 and the memory words are read back (dspmaster.Session, dsppeek transport).
Every output that changed is fitted over r0..r7 and m0..m(k-1) (up to 3 monomials of degree <= 2).
```

| option | help | default |
|---|---|---|
| `cmd` |  |  |
| `args` |  |  |
| `-k` |  | `4` |
| `-n` |  | `7` |
| `--raw` | append raw probes (jsonl) here |  |

Environment: `PAD` (default '0')

### `examples/from_scratch/re/dspcond.py`

```
Snapshot the 16 condition bits of the C words on a DSP context (DSPCTX, default efdc here).

    DSPCTX=efdc python3 dspcond.py [-n 200]

Program: for each condition index k (two programs: 0-7, 8-15), r(k mod 8) = cond_k ? 1 : 0 through
`2d06 r=0 r=1 <2k>12 0000 3000` (op 2 runs iff the condition holds), then r0..r7 -> bank 0. Many single reads
(no majority) give snapshots of the condition bits at different times; prints how often each pattern appears.
```

| option | help | default |
|---|---|---|
| `-n` |  | `200` |

### `examples/from_scratch/re/dspcondsnap.py`

```
Read the 16 condition bits on master DSP3 after a prefix instruction (to find what writes them).

python3 dspcondsnap.py "1803 0040 b9b2" --r3 -0.5
```

| option | help | default |
|---|---|---|
| `prefix` |  |  |
| `--r3` |  | `0.5` |

### `examples/from_scratch/re/dspcopy.py`

```
DSP bank-5 queue -> delay ring copier, verified with the tap of age 409602 (2026-10-20, master DSP4 slot 0xF7DC).

Each sample: load bank5[SRC] -> [4300] (ring input), rotate the 16-word queue at SRC, tap u = 25 -> bank 5 OUT.
The ARM then reads OUT (bank 5 is coherent) and checks that every value is one of the 16 queued words.

    python3 dspcopy.py
```

### `examples/from_scratch/re/dspcount.py`

```
How often is a C (condition) word true? Count on the master DSP (dspmaster.Session).

    python3 dspcount.py 0e12 1312 [--pre "0a02 b042"] [--secs 1]

A load in the instruction right after a store returns the stored word (pipeline), hence the 0001 no-ops.
Program (runs on every execution of the slot): [07a1] += 2^-16 (all executions) and
[07a0] += 2^-16 when the condition word selects the first of two loads
(2d06 1050 1052 <C> 0001 0000: r1 = 2^-16 or 0). Two reads <secs> apart give the executions per second and the
fraction of executions where the condition held.
```

| option | help | default |
|---|---|---|
| `words` |  |  |
| `--pre` |  | `` |
| `--secs` |  | `1.0` |

### `examples/from_scratch/re/dspcreal.py`

```
Which ops of a real Roland instruction run, for chosen flag values (2026-10-17). Master DSP3: r0 = 0, each
`--set G=v` does cmp r0 + v into group G (1..5), two no-ops, then r1..r7 = distinct values, the instruction, and
r0..r7 are read back. Memory references are moved to scratch addresses 0x700.. (dspverify.layout).

    python3 dspcreal.py "b247 3044 3045 ee0f 1220 0e14 b9b3" --set 1=1 2=-1 [--set ...]   (several --set = cases)
```

| option | help | default |
|---|---|---|
| `ins` |  |  |
| `--set` |  | `[]` |

### `examples/from_scratch/re/dspcsweep.py`

```
Sweep the target byte of a C (condition) word: which ops of the instruction run, for a true and a false context
condition (master DSP3, DSPCTX=dsp3: condition 1 true, condition 0 false).

    python3 dspcsweep.py 0x168 [--from 0 --to 255]  > work/dspclass/csweep_168.txt

Each op of the test instruction copies a distinct register (r1 <- r5, r2 <- r6, r3 <- r7, ...), all destinations start
at 0, so the registers that changed tell which ops ran. Output per target byte: enabled ops for condition true / false.
```

| option | help | default |
|---|---|---|
| `cls` |  |  |
| `--from` |  | `0` |
| `--to` |  | `255` |

### `examples/from_scratch/re/dspdis.py`

```
First DSP microcode disassembler: instruction boundaries + annotations.

    python3 dspdis.py prog.bin [dsp]          # dsp = unit for C0nn/A0nn names (model_spec.py)
    python3 dspdis.py --fields prog.bin       # split into ops / transfers / bridge / immediates

Known structure (NOTES.md "ISA del DSP"):
* words in execution order (dspmc.py); an instruction is `w[0] & 0x1F` words
  long (all 84 unique programs of the corpus tile exactly with this rule)
* operand words `C0nn` = parameter block id nn (+0xA0), `A0nn` = bank 5 id nn
  (+0xB0), `02nn`/`04nn`/`06nn` = local data, `22nn` = mailbox to the slot
  programs
* some instructions end in a 32-bit immediate in the DSP's own number format
  (dspnum.py): 30000000 = 1.0, 01555555 = 1/192 (envelope tick), 2775b5ea /
  45488e4f / a69d3813 = the LFO rate curve of the VST (0.466 e^(5.28 x) - 0.413).
  `imm?` marks the last two words decoded that way (a guess per instruction)
```

### `examples/from_scratch/re/dspedm.py`

```
External delay memory (EDM, 0x200000 words per BMC) through the ARM indirect port (2026-10-17, master):
address -> sys+0x40, 0xA0000000 -> sys+0x44 (read command; bit 31 clears when done), data at 0x41158000 (IEEE),
+4, +8 (RAW). IDM: sys+0x50/+0x54, port +0x10. Writes with the driver's sequence (address, then data to the
port; s_boot 0x0111210a) did NOT land yet. The f7dc delay line lives at EDM 0x110000-0x1FFFFF (983040 words).

    python3 dspedm.py read ADDR [N]          # hex address, N words
    python3 dspedm.py scan [STEP]            # nonzero words over the whole EDM (STEP words apart, default 0x1000)
```

### `examples/from_scratch/re/dspedmloop.py`

```
Delay line as loop memory (2026-10-17, master DSP4 f7dc): fill it with a counter for 11 s, then write back the
tap of age a (u = 25 -> 409602 samples) every sample; tap(a) and tap(2a) then agree (to a few samples) for 45 s.
The DMA writes [4300] every sample even without a store (repeats the last value), so stored data must be refreshed.

    python3 dspedmloop.py
```

### `examples/from_scratch/re/dspexact.py`

```
Bit-exact arithmetic of the DSP (2026-10-17): random full-precision RAW operands, the op, the RAW result read
exactly from bank 5 (master DSP3), compared with exact rational models of the RAW encoding and rounding.

    python3 dspexact.py add mul mac ... [-n 64]

RAW: value = +-M / 2^(32 - 4s), s = 0..3 (bits 30-29), M = 29 bits; the encoder picks the smallest s that fits.
```

| option | help | default |
|---|---|---|
| `ops` |  |  |
| `-n` |  | `64` |

### `examples/from_scratch/re/dspf5.py`

```
Unit-B `{8+d}f5Y` (bits 7-6 = 01, sub 1) returns a time-varying value (2026-10-17). Master DSP3: r1 = 0 (or
--init), the op, r1 -> bank 5 every sample; read it N times over a few seconds and fit value vs time.

    python3 dspf5.py [word ...]      # default 9f5c 9f5d 9f5e 9f5f 9f54 9f50
```

Environment: `DSPCTX` (default 'dsp3')

### `examples/from_scratch/re/dspfit.py`

```
Fit the formula of an instruction from exact probes on a master DSP (dspprobe.py).

    python3 dspfit.py "0902 3{op}41" --op 0-7 -n 6        # {op} -> B nibble = op << 1 (X = r1 here)
    python3 dspfit.py "0803 3{op}41 2{op2}4" ...          # {op2}: op2 << 1 in bits 11-9 of w2

Operands: D = old r3 (destination r3), X = r1, Y/Z = r2, W = r4 (the template must use these).
For every op value, N probes with random r0..r7 from +-k/8, then least squares on monomials of
D, X, Z, W up to degree 3; prints the formula (coefficients rounded to 1/64, residual) and the
other registers the instruction changed.
```

| option | help | default |
|---|---|---|
| `template` |  |  |
| `--op` | values for {op} (B nibble = op << 1) | `0-7` |
| `--op2` | values for {op2} (w2 nibble = op2 << 1) | `0` |
| `-n` |  | `7` |
| `--seed` |  | `1` |
| `--deg` |  | `3` |
| `--terms` |  | `3` |
| `--all` | fit over r0..r7 (degree 2 is enough) |  |

### `examples/from_scratch/re/dspflagsaw.py`

```
SAW variants that use flags (X9bY + cmp), C selects, jumps and loops, for the audio check on the slaves
(2026-10-17). Same pitch formula as dspasm.saw(); only the phase update changes:

    sel   wrap: cmp r1 - 1 -> flags3; r1 -= 2 under a C word (runs only if r1 >= 1)
    jmp   wrap: cmp r1 - 1 -> flags3; conditional jump over r1 -= 2 (taken if r1 < 1)
    loop  increment / 4 added 4 times in a loop (backward conditional jump), then the jmp wrap

    python3 dspflagsaw.py sel|jmp|loop [-o file]
```

| option | help | default |
|---|---|---|
| `mode` |  |  |
| `--slot` |  | `dco2` |
| `--iters` |  | `4` |
| `-o` |  |  |

### `examples/from_scratch/re/dspharm.py`

```
Record notes (USB audio), print f0 and harmonic magnitudes 1..8 relative to h1.

python3 dspharm.py NAME [notes...]
```

### `examples/from_scratch/re/dsphops.py`

```
Unit-B nibble-f forms that use the hidden registers (2026-10-17): load h4..h7 from registers with `40TY`
(h4 <- r0 = 0.5, h5 <- r1 = 0.25, h6 <- r2 = 0.125, h7 <- r6 = 0.875), then the op, on master DSP3 (dspprobe).

    python3 dsphops.py [word ...]      # default: the forms seen in JX_b000
```

### `examples/from_scratch/re/dsphreg.py`

```
Hidden-register writes `40TY` (C position) on master DSP4 slot 0xF7DC: for each low byte TY, reset h4/h5, apply
the word (immediate 5100 [0000]) and read h4/h5 with A op 0 sub 4/5; results in bank 5. Batches of 8 (time budget).

    python3 dsphreg.py [TY ...]        # hex low bytes, default a grid over T, mode, Y
```

### `examples/from_scratch/re/dspidx.py`

```
Indexed transfers (mode byte 28) on master DSP3: markers in bank 0 [03D0..03D7], h6 set by `0b02 4080`, then
`0c02 028Y` loads / `8280` stores; results in bank 5 (DPC3 RAW).

    python3 dspidx.py
```

### `examples/from_scratch/re/dspidxring.py`

```
Is the indexed transfer (mode 28, h6) ring-relative like normal bank-0 references? On master DSP3: a counter
kept with indexed transfers, c = [h6 + a] + INC -> [h6], for a = 0 and a = 1; ring-relative addressing counts only
with a = 1 (96000/s), absolute addressing only with a = 0. Output to bank 5 (DPC3 RAW).

    python3 dspidxring.py
```

### `examples/from_scratch/re/dspiomd.py`

```
IOMD (bank 3) between master DSPs (2026-10-17): DSP3 stores salted markers to the words that slave programs use
as outputs (20-27, 60-67, B0-B7); DSP4 (slot 0xF7DC) loads the words they use as inputs (00-07, 10-17, E0-E3) and
the output words; the ARM reads the IOMD RAW block. Shows whether IOMD links DSPs or only goes out.
```

### `examples/from_scratch/re/dspisa.py`

```
DSP instruction set (as measured on the master's DSP4, NOTES.md "Formato de instrucción"): decoder,
disassembler and simulator.

    python3 dspisa.py dis prog.bin|prog.txt          # disassemble (blob in memory order, or hex words in execution order)
    python3 dspisa.py sim "0902 3c01" [--regs ...]    # simulate one instruction on r0..r7
    python3 dspisa.py cov work/corpus                # decoding coverage over a directory of .bin programs

An instruction = header (class << 5 | length) + words. The class is a template (TEMPLATES): one letter per
body position:
    A  ALU op (16 bits)          M  MAC first word (w1)        N  MAC second word (w2)
    T  transfer                  R  reference (slot)           I  immediate bits (rest of the instruction)
Slots (for transfers XfeY: slot Y/2) are counted from the first position after the transfers: N words
first, then R words. Operand modes 1/2/3 take 16/24/32 bits from the immediate bit stream: MAC operands in order,
every A/B op from the start of the stream (two A/B ops with immediates share the same bits, measured 2026-10-16).
```

| option | help | default |
|---|---|---|
| `cmd` |  |  |
| `arg` |  |  |
| `--regs` |  | `0.5,0.25,0.125,-0.375,0.0625,-0.75,0.875,-0.0625` |

### `examples/from_scratch/re/dspjump.py`

```
Jumps (2026-10-17): A op 0, X = 2 with an imm16 (`0090 nnnn`) = jump forward nnnn words from the end of the
instruction (hypothesis from the corpus: every offset lands on an instruction boundary). Master DSP3 probes:
registers r0..r7 = 0.5 .. (dspprobe), then the test lines, then r0..r7 stored.

    python3 dspjump.py
```

### `examples/from_scratch/re/dsplive.py`

```
Live DSP microcode pokes over MIDI (engine.c -DDSPLIVE): change the test programs without reinstalling.

    python3 dsplive.py install b14.batch     # build C-DSPT with DSPTEST + DSPLIVE, install in slot 3
        (then select PLG3 on the panel, as with dspaudio.py)
    python3 dsplive.py poke d2a=r_08.txt d3b=n_pass.txt   # rewrite those test paths in the running DSPs
    python3 dsplive.py restore               # back to the programs as installed
    python3 dsplive.py recall NAME           # dspaudio recall (eight notes) + report against b11

Transport (no region 0 changes): pitch bend carries 14 data bits (voice id 0x465), then CC1 carries
the command (voice id 0x467); the stock-path region 0 forwards both to the slaves, whose engine acts
when 0x467 changes (bit 6 toggles every command):
    CC1 = seq<<6 | cmd<<4 | ext     cmd 0 ADDR (dsp 2 + (ext & 1), word index = bend; ext & 2: both DSPs,
                                               ext & 4: bank 5 instead of the program,
                                               ext & 8: OSC1 = DCO1 slot 0x6000 instead of OSC2 = DCO2 0xB000)
                                    cmd 1 DATA (program: word = bend | ext<<14; bank 5: 18-bit value*2^17,
                                               stored at the index, index++)
                                    cmd 2 CTRL (ext 0 restore, ext 1 nop)
    (both-DSP and bank-5 modes 2026-10-17: Link.write(..., both=True), Link.write_b5; need a lab built after that)
Words = 16-bit microcode words in execution order, written into the DCO2 slot of DSP2 (t0, paths
d2a/d2b) or DSP3 (t1, d3a/d3b). Both slaves receive the same pokes. Only changed words are sent;
the state of what is loaded is kept in work/dsplive/state.json.
```

| option | help | default |
|---|---|---|
| `cmd` |  |  |
| `args` |  |  |
| `--full` | poke: send every word, not only changed ones |  |
| `--delay` |  |  |

Environment: `DSPLIVE`, `PEEK`

### `examples/from_scratch/re/dspmacfit.py`

```
Fit the formula of one instruction over r0..r7 (monomials up to degree 2) from probes with random registers
on master DSP3 (dspprobe). Prints the changed registers' formulas.

    python3 dspmacfit.py "0803 8c0a 9281" -n 14
```

| option | help | default |
|---|---|---|
| `ins` |  |  |
| `-n` |  | `14` |
| `--seed` |  | `1` |
| `--dump` | print every probe (inputs -> changed registers) |  |
| `--values` | comma-separated pool of register values |  |

### `examples/from_scratch/re/dspmaster.py`

```
Run a test program on a DSP of the MASTER and read its data bank, live (dsppeek.py transport).

    python3 dspmaster.py prog.txt [--read 0x400 8] [--dsp 4] [--slot 0xEFDC] [--wait 0.05]

The program (16-bit words in execution order, '#' comments, as in re/dspexp/audio) replaces the
global-effect program at <slot> of master DSP <dsp> for a moment: the original words are read,
the test program is written (an empty program first, then the body, then the head, so the DSP
never runs a half-written program), we wait, read bank 0 (RAW view) twice, and put the original
program back (tail first, head last). Values are printed as RAW words and decoded floats.
While it runs, that global effect (reverb/delay of the SYSTEM-8) is replaced.
```

| option | help | default |
|---|---|---|
| `prog` |  |  |
| `--restore` | put back the program saved by the last Session |  |
| `--read` |  | `['0x400', '8']` |
| `--dsp` |  | `4` |
| `--slot` |  | `0xEFDC` |
| `--wait` |  | `0.05` |

Environment: `DSPCTX` (default 'dsp3')

### `examples/from_scratch/re/dspmc.py`

```
DSP microcode helpers: blob bytes -> 16-bit words in execution order.

The DSP executes each 32-bit little-endian word upper half first. A blob of
4n+2 bytes is followed in the image by the halfword 0x0001, which is its real
last instruction; the blob's own last halfword (0x0000) is never written to
program memory (verified on the master DSP4, docs/CUSTOM_PLUGOUTS.md).
```

### `examples/from_scratch/re/dspmodescan.py`

```
Scan transfer mode bytes (loads, X=0) on idle master DSP3 (2026-10-17): bank 5 filled with markers i/2 (ARM
writes, restored afterwards), one reference word, h6 / h7 set (in words). r0 preset -7 = no load.

    python3 dspmodescan.py [REF Y H6 H7 [MODES]]     # hex; default A005 0 10 20, all modes but 9b
    python3 dspmodescan.py A005 3 15 27 d0,d1,da,db  # -> da = bank5[h6+Y], db = bank5[h7+Y], dN = bank5[N*16+Y]
```

### `examples/from_scratch/re/dspmtap.py`

```
N delay-tap requests per sample on master DSP4 f7dc, each to its own result triplet (h5 = 0x301 + 3k);
input = counter c (+INC per sample). Prints age of each tap (samples; includes the ARM read delay, compare differences) and its fraction.

    python3 dspmtap.py "[0.1*k for k in range(1,9)]" "[0.25+0.125*k for k in range(24)]"   # 2026-10-17: 24 taps OK, frac 1/64
```

### `examples/from_scratch/re/dspmxm.py`

```
MXM routing on the master, both directions at once (2026-10-17). DSP3 (main) and DSP4 (slot 0xF7DC) each store
unique salted markers to their four MXM zones (M 81F0, O 83F0, Z 85F0, S 87F0, words F0..F1) and load the same
eight words, storing what they read to bank 5; the ARM also scans the MXM windows of DSP2..5. Prints, for every
read and every window word, which writer/zone the value came from.
```

### `examples/from_scratch/re/dspmxsum.py`

```
Is MXM a mixing bus? (2026-10-17) DSP3 writes a to S 87F0 (word 0x1F0 of MXM5) and DSP4 writes b to O 83F0 (the
same word); both read the word back (DSP3 as S, DSP4 as O) and the ARM reads MXM5 word 0x1F0. Also DSP3 writes the
same S word twice in one sample (a, then a2).
```

### `examples/from_scratch/re/dspnlfit.py`

```
Nonlinear exact fit of measured instructions (raw probes from dspclass.py --raw).

    python3 dspnlfit.py work/dspclass/bunit_raw.jsonl [--ops D=3,X=1,Y=2]

For every output that changed: the smallest sum of up to 3 terms c * atom (c from least squares) that
reproduces every probe exactly. Atoms: 1, r0..r7, and over the operands D, X, Y: |a|, min(a,b), max(a,b),
a*b, sign(a), step(a) (a >= 0), |a|*b.
```

| option | help | default |
|---|---|---|
| `raw` |  |  |
| `--ops` |  | `D=3,X=1,Y=2` |
| `--wrap` | read results > 256 as v - 512 |  |

### `examples/from_scratch/re/dspnum.py`

```
The DSP's internal number format ("RAW" view of its data banks).

Measured on hardware (2026-10-14): 74 floats written through the IEEE view of
master DSP4's bank 5 (DPC4 0x41124E00) and read back through the SD31 / RAW
views (bulk_dump.py --tri, CONV=...). All of them fit:

    bit 31      sign (sign-magnitude)
    bits 30-29  exponent s = 0..3, one step = x16
    bits 28-0   magnitude M (29 bits)
    value = +-M / 2^(32 - 4 s)

s = 0: [0, 0.125) (= the SD31 value x 2), 1: [0.125, 2), 2: [2, 32), 3: [32, 512).
The IEEE view converts both ways; SD31 is Q31 saturated to [-1, 1).
```

### `examples/from_scratch/re/dspown.py`

```
Load an own program into the DCO2 slot of the slaves (DSP2 and DSP3) through dsplive, and measure it by
USB audio.

    python3 dspown.py load dspexp/own/saw_dco2.txt      # needs the DSPLIVE plugin installed + PLG3 selected
    python3 dspown.py notes NAME [--notes 48 60 72]       # record one note each, print the measured frequency
    python3 dspown.py restore                            # back to the installed test programs
```

| option | help | default |
|---|---|---|
| `cmd` |  |  |
| `arg` |  |  |
| `--notes` |  | `[48, 60, 69, 72]` |

Environment: `OWN_BOTH` (default '1'), `OWN_FULL`

### `examples/from_scratch/re/dsppeek.py`

```
Live peek / poke of master memory (incl. the master's DSPs) through our region 0 (model.c -DPEEK).

    python3 dsppeek.py ping
    python3 dsppeek.py read 0x4188EFDC 16            # words (hex + DSP RAW value)
    python3 dsppeek.py write 0x41804400 0x30000000 ...

Built into the DSPLIVE plugin (dsplive.py install). Transport: pitch bend = 14 data bits, then
CC1 = seq<<6 | 3<<4 | cmd on channel 1; region 0 runs the command on the OS CPU (master BMC
core 0) and answers reads / pings with SysEx F0 7D 02|03 ... F7 on SYSTEM-8 MIDI 1.
The slaves never see these CC1s (region 0 does not forward them), so the slave DSPs cannot be
read this way: only master memory (DSP n of the master: program 0x41480000 + n*0x100000,
bank 0 RAW view 0x41404000 + n*0x100000).
```

| option | help | default |
|---|---|---|
| `cmd` |  |  |
| `args` |  |  |

### `examples/from_scratch/re/dspports.py`

```
Listen to the DSP 'ports' (X9bY writes, Xa9Y reads) on the lab plugin (DCO2 slot of the slaves, live pokes).

python3 dspports.py write 0 1 2 3 4 5 6 7     # noise -> port Y, outputs silent
python3 dspports.py read  0 1 2 3 4 5 6 7     # port Y -> outputs
```

### `examples/from_scratch/re/dspprobe.py`

```
Exact instruction probes on a master DSP (dspmaster.py + dsppeek.py): registers before -> after.

    python3 dspprobe.py "0902 0c01" ["0803 0c01 0000" ...]     # one probe per argument (';' = more lines)

Each probe: r0..r7 = REGS (0903 d050 imm16), the instruction lines, then r0..r7 stored to bank 0
[STORE..STORE+7] with one 0c03 per register (7809 with different registers only kept the last one).
Prints the registers that changed, as floats (exact RAW words in --raw).
```

| option | help | default |
|---|---|---|
| `probes` |  |  |
| `--raw` |  |  |
| `--regs` | comma-separated r0..r7 values |  |

### `examples/from_scratch/re/dsprdly.py`

```
Delay-tap measurement inside Roland's own delay program (master DSP4 slot 0xF7DC, m1_tcm_f7dc_01134238), so the
bank-2 transfers happen at the instruction positions the delay hardware expects.

Changes to Roland's program: the delay input r0 (stored to [4300] at halfword 58) is replaced by a slow counter
c (ring state [07C0], c += 2^-20 per sample, also stored to [A3C3]); after the tap-1 interpolation (halfword 164)
r0 (lerp), r1 ([4302]) and r2 ([4303]) go to [A3C0..A3C2]; r3/r4 (tap delays after f6c) to [A3C4]/[A3C5], r3/r7 before halfword 141 to [A3C6]/[A3C7]; the
audio output (halfword 289) is zeroed. Bank 5 (DPC4, plain memory) is read by the ARM at 0x41126000 (RAW).

    python3 dsprdly.py --reads 10 --gap 1 [--d3 12.5]   # --d3: force the tap-1 delay r3 (before f6c)
```

| option | help | default |
|---|---|---|
| `--reads` |  | `10` |
| `--gap` |  | `1.0` |
| `--hold` |  | `0.5` |
| `--d3` |  | `[None]` |
| `--drop` | halfword starts to replace by 0001, e.g. "68,87,107" or "2-53" | `` |
| `--remove` | delete the dropped instructions (changes timing) |  |
| `--d4` |  |  |

### `examples/from_scratch/re/dsprun.py`

```
Run a DSP program on the master's DSP4 (hardware) and read its data bank.

    python3 dsprun.py experiment.py           # experiment file defines PROG, DATA, READ

The program replaces the global-effect program at DSP4 0xEFDC (program memory
0x4188EFDC) for a moment: the region 0 init hook (examples/midi_send,
init_bulk.s) saves the original words, writes DATA (DT40 word index -> float,
through the IEEE view 0x41800000), writes the program (first as an empty task,
then the body, then the head, so the DSP never runs a half-written program),
waits, copies READ = (first word, count) of DT40 (RAW view) twice to RAM
(snapshots DELAY loop turns apart), puts the original program back and dumps
the snapshots. DATA goes through the RAW view too (dspnum.encode).

An experiment file:

    PROG = '''
    0a02 ff70
    1e05 0fe0 1fe2 0400 0401
    0a02 ff70 0b02 00f8 0001
    '''
    DATA = {0x400: 1.0, 0x401: 2.0}
    READ = (0x400, 32)

DATA5 / READ5 instead use bank 5 (DPC4, ids = program refs A000 + id): the
ARM cannot write DT0 (writes to its RAW or IEEE view do not stay; verified),
bank 5 it can.
```

### `examples/from_scratch/re/dspseed.py`

```
SFU seeds bit-exact (2026-10-17): recip seed `cf06` (r4 = sign 2^-e, r5 ~ 1/m for r6 = m 2^e) and sqrt split
`cf16` (r6 <- m in [1,4), r5 ~ 1/sqrt m, r7 = sign 2^-k): sample the mantissa range and print the seed as RAW
words, to find the table (step width, values).

    python3 dspseed.py recip|rsqrt [-n 256] > work/seed_recip.txt
```

| option | help | default |
|---|---|---|
| `kind` |  |  |
| `-n` |  | `256` |

### `examples/from_scratch/re/dspshape.py`

```
Unit-B nibble e, D >= 8, bits 7-6 = 11 with mode bits != 00 (2026-10-17): phase-shaping ops. Probes with random
registers on master DSP3 and tests candidate functions of X (= r(d)), Y (= rY with the bit-3 sign) and |Y|.

    python3 dspshape.py [word ...]
```

### `examples/from_scratch/re/dspslots.py`

```
Map the word positions of an instruction class (template) on a master DSP: which position acts as
an ALU op, which as a transfer, and which make the instruction illegal.

    python3 dspslots.py 0x0a0 [0x158 ...] [--len 5]      # header = class << 5 | len
    python3 dspslots.py all --top 40

For each body position p (all other words 0000, neutral as op / transfer / immediate):
  probes 3c01 and 5c01: as an A-op they give r3 = .75 / r5 = .75 (r0 + r1); as a transfer they load
  r3 / r5 from memory; as a slot (reference) they change what the filler transfers (0000 = load r0
  from slot 0) read; as an immediate they change a value.
Printed: the registers that changed, for each probe; 'X' = nothing stored (program stopped). Uses dspprobe.probe (r0..r7 = REGS).
```

| option | help | default |
|---|---|---|
| `classes` |  |  |
| `--len` |  |  |
| `--top` |  | `40` |

### `examples/from_scratch/re/dsptap.py`

```
Delay-tap experiment (NOTES.md "Tomas de retardo"): own delay line through the delay-memory DMA.

Every sample: counter c += INC (ring state, wraps in [-1, 1)), store c to the write port [W], optionally set the
f6c base (`1d06 4022 7fe0 c1fc <imm32>`), r3 = D, `bf6c` (r3 += base), the result destination `0b03 4050 <W+1>`,
the request `3a90`, then load [W+1], [W+2], [W+3] (sample, older sample, fraction) and store them, c and c - tap
to bank 5 [A3C0..] (read by the ARM, coherent). Works only where the ARM connected bank 2 to the delay memory:
master DSP4 slot 0xF7DC (W = 4300, write position 324; `--base 324` gives tap = x(n - 2 - 16384 D)).
Measured 2026-10-16 (TAPINC=-20, --base 324): D = 1 -> 16386 samples, D = 0.5 -> 8194.

    TAPINC=-20 python3 dsptap.py --f7dc --W 0x4300 --base 324 --D 1 0.5 --hold 1 --reads 2
    env: TAPREQ=0c02|1b04 (request form), TAPDEST=0 (no destination word: requests ignored), TAPTWO=1
```

| option | help | default |
|---|---|---|
| `--D` |  | `[1.0]` |
| `--mode` |  | `both` |
| `--W` |  | `16384` |
| `--pads` |  | `1` |
| `--base` | set the f6c base first (Roland: 324) |  |
| `--hold` | seconds before reading | `0.2` |
| `--gap` | seconds between reads | `0.0` |
| `--reads` | single (non-majority) reads per case | `1` |
| `--f7dc` | run in master DSP4 slot 0xF7DC (Roland delay, base 4300) |  |

Environment: `TAPDEST` (default '1'), `TAPINC` (default -12), `TAPREQ` (default '0c02'), `TAPTWO`, `TAPWRITE`

### `examples/from_scratch/re/dsptemplates.py`

```
Infer the template of each instruction class from the position map (dspslots.py output) and the corpus.

    python3 dsptemplates.py work/dspclass/slots_top60.txt [more ...] > work/dspclass/templates.txt

Position labels from the map (probes 3c01 / 5c01): A (r3/r5 = .75), M (1.5 = MAC first word), T (loads a
memory value into r3/r5), '?' (no effect of its own). The '?' positions are resolved with the class's most
frequent corpus instruction: after the transfers come the MAC second words (N, one per M), then the
references (R, one per transfer), then immediates (I: as many words as the operand modes of the A/M/N words
need); '?' before the transfers (or left over) are C (another unit, not decoded yet).
```

### `examples/from_scratch/re/dspvalid4.py`

```
Validate re/dspisa.py and dsp/sim_main.py on the master's DSP4, live (dspmaster.py / dsppeek.py transport).

The test program (dspexp/master/m02_sim_check.txt) is stateless: every value it stores is a function of constants,
so the stored words do not depend on how many samples have run. It replaces the global-effect slot 0xEFDC of DSP4
while it runs (dspmaster restores it), reads bank 0 words 0x700-0x703 and compares them with the simulator.

    python3 dspvalid4.py            (needs the lab plugout installed and PLG3 selected)

Bank 0 is a ring, so a word read at an absolute address only holds this program's value in the sample where it was
stored. The DSP reads are therefore compared as a set: each device word must equal one of the simulator's stored
values, and the simulator's values must be the exact RAW encodings of the device's arithmetic.
```

Environment: `DSPCTX`

### `examples/from_scratch/re/dspvar.py`

```
Which words of an instruction are operands (change between copies of the same code)?

    python3 dspvar.py work/corpus/*.bin

Copies of the same code: the two voice halves of a program (found by shifting
the instruction sequence against itself) and programs whose instruction head
sequences are identical (DCO1/DCO2, effect instances). For aligned instructions
with equal words in all but some positions, those positions are operands.
Prints, per instruction head, the body positions seen varying, with examples.
```

### `examples/from_scratch/re/dspverify.py`

```
Verify instruction semantics (and so the template) on master DSP3 against dspisa's simulation (2026-10-17).
For each instruction: memory references are moved to 0x700.. and preloaded, r0..r7 get random values, the flags of
groups 1-5 are set by cmp + X9bY, then the instruction runs; registers and every written reference are compared
with dspisa.run on the same state.

    python3 dspverify.py "954c 2476 ..." ["..." ...] [-n 3]
    python3 dspverify.py --inferred [-n 3]       # one Roland instance of every class marked 'inferred'
```

| option | help | default |
|---|---|---|
| `ins` |  |  |
| `--inferred` |  |  |
| `-n` |  | `3` |

### `examples/from_scratch/re/dspx9b.py`

```
Test whether X9bY writes the condition bits (agent hypothesis): on master DSP3, r3 = v, `1803 0040 b9bP`
(port P), `pad` no-ops, then a C select on condition cc: r1 = 0 if the condition holds, 1 if not.

    python3 dspx9b.py --port 1 --conds 4 5 6 7 --pads 0 1 2
```

| option | help | default |
|---|---|---|
| `--port` |  | `1` |
| `--prefix` | instruction with {port}, e.g. "1b03 be42 b9b{port:x}" |  |
| `--conds` |  | `[4, 5, 6, 7]` |
| `--pads` |  | `[0, 1, 2]` |

### `examples/from_scratch/re/dspx9bflags.py`

```
X9bY as "set condition group (X & 7) from the result of unit Y of the same instruction" (hypothesis from the
corpus: every X9bY is followed by C words on group X & 7, and its instruction has an op in unit Y). Master DSP3
main (DSPCTX=dsp3): for each value v, r1 = v, then the test instruction (default Roland's `1b03 8e09 b9b1`,
r0 = -r1), pads, then the 16 condition bits through C selects -> bank 5 (A200 + 16k + cc, DPC3 RAW view).

    python3 dspx9bflags.py [--ins "1b03 8e09 b9b1"] [--pads 1] [--values 0.5 -0.5 0 ...]
```

| option | help | default |
|---|---|---|
| `--ins` |  | `1b03 8e09 b9b1` |
| `--pre` | extra instructions after setting r0 = 0, r1 = v | `[]` |
| `--pads` |  | `1` |
| `--values` |  | `[0.5, -0.5, 0.0, 2.0, -2.0, 0.0625, -0.0625, 1e-06]` |
| `--reg` | register that gets v | `1` |
| `--high` | conditions 16-31 instead of 0-15 |  |

Environment: `DSPCTX` (default 'dsp3')

### `examples/from_scratch/re/dspx9bscan.py`

```
Where does `X9bY` write? Master DSP3 writes a unique marker with `1803 0040 b9bY` (r3) every sample; the ARM scans
the MXM windows, IOMD, and DSP3's banks 0 (RAW), 1, 2 (IEEE) and 5 (RAW) for it.

    python3 dspx9bscan.py 1 2 3          # ports
```

### `examples/from_scratch/re/emu0.py`

(no docstring)

### `examples/from_scratch/re/emu1.py`

(no docstring)

### `examples/from_scratch/re/fitcurves.py`

```
Fit the measured responses (re/measure.py) with simple formula families.
```

### `examples/from_scratch/re/jxpatch.py`

```
Patch parameter ids of the JX-3P 'patch' struct: id = 0x6CD + nibble_offset/2.
```

### `examples/from_scratch/re/knotfit.py`

```
Least-squares knots (piecewise linear at given x, optionally in log2) for a
measured curve: prints the expression for engine/model_spec.py and its error.

usage: python3 re/knotfit.py meas.json ID TARGET [log|lin]
```

### `examples/from_scratch/re/measure.py`

```
Measure the JX-3P engine's DSP writes for every parameter value (reference
data for our own parameter model; nothing of it is built into the PLUG-OUT).

usage: python3 re/measure.py engine/build/proj/region1.bin out.json
```

### `examples/from_scratch/re/probe.py`

```
probe(x, vx, others) -> what the JX-3P engine writes for x = vx with the
other parameters set first (reference measurements).
```

### `examples/from_scratch/re/r0list.py`

```
Dump the region 0 record list (SYNINFO +0x28) of a stock PLUG-OUT.
python3 re/r0list.py region0.bin [base]
```

### `examples/from_scratch/re/review_probes.py`

```
Hardware probes for the open items of DSP_ISA_DISCREPANCIES.md (2026-10-20), on master DSP3 via dspprobe.

    python3 review_probes.py            # run all, print expected / measured, write work/review_probes.json

Each probe is a list of instructions: ('asm', text) is assembled with asm/ (allow_inferred), ('hex', words)
is taken as it is. The registers start at dspprobe.REGS; the probe stores r0..r7 and prints what changed.
Needs the lab plugin installed in slot 3 and PLG3 selected (see DSP_ISA_DISCREPANCIES.md).
```

### `examples/from_scratch/re/run_jx.py`

(no docstring)

### `examples/from_scratch/re/spec_blocks.py`

```
Measure the per-slave / per-state blocks of the model and print them as
literals for engine/model_spec.py (PART table, 0x737 table, effect curves).
```

### `examples/from_scratch/re/sweepfit.py`

```
Sweep one parameter 0..255 in a context and print fitted model lines.

usage: python3 re/sweepfit.py ID [ctx_id=value ...] [slave=N]
```

### `examples/from_scratch/re/tdis.py`

(no docstring)

### `examples/from_scratch/re/vst_anchor.py`

```
Anchor VST state constants in the DSP microcode.

    python3 vst_anchor.py work/ghidra/out/init.c work/corpus/JX_*.bin

init.c = Ghidra output of CJx3pSim's initialisation (0x180391630) passed
through ghidra/annotate.py. Every `st_<off> = 0x<float bits>;` (both sample
rate sets) is encoded in the DSP's RAW format (dspnum.py) and searched as a
32-bit immediate (two consecutive words, execution order) in the programs;
prints offset, value, and where it occurs. Trivial values (0, +-1, 0.5, ...)
are skipped: they are everywhere.
```

### `examples/from_scratch/re/vst_rtti.py`

```
MSVC x64 RTTI of a VST3 DLL: class name -> vtables (with their methods).

    python3 vst_rtti.py plugins/JX-3P/JX-3P.vst3 [name-filter]

TypeDescriptor ".?AV<name>@@" <- CompleteObjectLocator (signature 1, RVA of the
type descriptor at +12, RVA of itself at +20) <- the qword before each vtable.
Static analysis only (the DLL is never loaded or run).
```

### `examples/from_scratch/re/vst_sim.py`

```
The native DSP model (CJx3pSim) of the JX-3P VST3: named parameters and the
functions that use them. Static analysis only (the DLL is never loaded or run).

    python3 vst_sim.py params plugins/JX-3P/JX-3P.vst3       # index, offset, name
    python3 vst_sim.py fields plugins/JX-3P/JX-3P.vst3 0x1803b2530:0x829c ...

CJx3pSim registers its parameters in one function (0x180382bf0 in 1.0.7): for
each, a 40-byte entry {name, 0, 0, 0, 1, &field} is appended to the table at
sim+0x38; the control classes (CDSPJx3p*, the same as on the PLUG-OUT) write
entry i via 0x18040ba10 (table[i].field = value). Voice v uses entries
0x6e*v + (0..0x6d), fields at +0x3f80*v.
```

### `examples/from_scratch/re/wav2dco.py`

```
Convert a single-cycle WAV into an oscillator program for the DCO1/DCO2 slot of the SYSTEM-8 DSPs.

    python3 wav2dco.py cycle.wav [-n 64] [-o prog.txt] [--slot dco2] [--check]

The cycle is resampled to N points (FFT resampling = band-limited, drops harmonics above N/2), DC removed, peak
normalised to --level; the program (re/dspasm.py `wavetable`) interpolates linearly between the points, with
the points hardcoded as immediates (the DSP has no indexed reads). Load it live with
`python3 dspown.py load prog.txt` (lab plugin: only the DCO2 slot is live-pokable, so use --slot dco2 for that).
```

| option | help | default |
|---|---|---|
| `wav` |  |  |
| `-n` | table points (default 32; ~4 instructions per point for both voices) | `32` |
| `--slot` |  | `dco2` |
| `--level` |  | `0.9` |
| `--keep-dc` |  |  |
| `--cycle` | cycle length in samples (default: the whole file) |  |
| `--frame` | with --cycle: take frame FRAME (multi-frame wavetables) | `0` |
| `--pad-mid` | no-ops between voice A and B (time budget probe) | `0` |
| `--pad-mid-aab` | A/A/B fillers between voice A and B | `0` |
| `--check` | simulate and compare with the table |  |
| `-o` |  |  |

### `examples/from_scratch/re/wt5.py`

```
All-in-one WAV wavetable -> bank-5 oscillator on the slaves (2026-10-17): converts a WAV (Serum-style multi-frame
table, `clm ` chunk, or a single cycle; PCM or float) to K frames of N points and sends it live to bank 5 of DSP2/DSP3
of both slaves (DCO2 slot, through dsplive), then leaves the player running.

    python3 wt5.py send TABLE.wav                          # 1 frame (the first), max size: 437 points
    python3 wt5.py send TABLE.wav --frames 2 --frame 1     # 2 frames of 218 points, play the last one
    python3 wt5.py send TABLE.wav --frames 8 --frame 3.5   # 8 x 53, fixed morph between frames 3 and 4
    python3 wt5.py send TABLE.wav --frames 8 --scan 0.25   # LFO scans all frames (NOT verified by audio yet)
    python3 wt5.py send TABLE.wav ... --dry                # only write the programs (work/wt5/)
    python3 wt5.py info TABLE.wav                          # frames in the file, what would be used

Needs the slave lab (DSPLIVE plugin in slot 3, PLG3 selected) and no other MIDI tool running. Time (15 ms per MIDI
command): table 438 commands (~6.6 s) + player ~280 words (~4 s, less when only some words change); --code ~2 min.

Transfer (default, needs a lab built with the 2026-10-17 engine): the table goes as DATA, one MIDI command per point
(18-bit fixed point, 2^-17 steps) for DSP2 and DSP3 at once; the slave engine (DSPLIVE) converts each value to RAW
and stores it into bank 5 itself; then the player program is loaded (both DSPs per command, only the changed words).
--code: the old way for older labs: the table travels as fill programs (move + store per point; bank 5 takes ONE
store per instruction) of --chunk points each, run with the player for a moment, then the player alone.
Conversion: frames picked evenly over the file, FFT-resampled to N points (band-limited), DC removed, one common
peak of 0.9.
Layout: frame k at BASE + k*(N+1): N points + a copy of point 0 (wrap). Space 0x048..0x1FD = 438 words
(Roland's slave programs use A000-A047, A200-A205, A300-A303): K*(N+1) <= 438 (1 x 437, 2 x 218, 4 x 108, 8 x 53,
16 x 26).
Player per voice (~45 instructions): SAW phase -> pos in [0, N), idx = ceil(pos) - 1, frac = pos - idx (SFU
ceiling `cf36`), h6 = frame k, h7 = frame k+1, four bank-5 loads (`0daY`, `0dbY`), linear interpolation inside the
frame and between frames. Frame position fixed (--frame) or a triangle LFO (--scan HZ, state in bank 5 A1FE).
```

| option | help | default |
|---|---|---|
| `cmd` |  |  |
| `wav` |  |  |
| `--frames` |  | `1` |
| `--points` | default: as many as fit |  |
| `--frame` |  | `0.0` |
| `--scan` | LFO rate (Hz) scanning all frames |  |
| `--osc` | which oscillator plays the table: 1 (DCO1 slot), 2 (DCO2 slot, default), both | `2` |
| `--code` | send the table as fill programs (labs built before the bank-5 command, ~2 min) |  |
| `--chunk` | --code: table words written per fill program | `200` |
| `--dry` | only write the programs |  |

### `examples/from_scratch/re/wt5_diag.py`

```
Host-side diagnostic of what `wt5.py send` actually sends (no hardware needed, except --peek).

    python3 wt5_diag.py TABLE.wav [--frames K] [--points N] [--frame F] [--scan HZ]
    python3 wt5_diag.py TABLE.wav --peek        # + read bank 5 back from the master DSPs (needs the lab + dsppeek)

Four independent checks:

1. SIMULATOR. Builds the programs exactly as wt5.py does and runs them with dspasm.simulate (dspisa) into a memory
   preloaded with the same table stored at 0xA000 + BASE: the output must be the expected frame waveform
   (resample to N points, linear interpolation, between frames). This is what NOTES.md calls "simulador exacto".
2. SHADOW. Replays the program writes dspown.load would send (dspown's EMPTY head, runs against state.json, 32-word
   chunks, head one 32-bit word at a time, start word last) into a shadow of the DCO2 slot and decodes it: the DSP
   must see the same words wt5.py compiled (a truncated/torn head shows up here).
3. DATA. Shows the bank-5 stream wt5.py sends (one command per point, 18-bit fixed point 2^-17, RAW conversion of
   engine.c's live_raw18) and *where the engine stores it*: the slave's DPC RAW window 0x41114000 + 0x4000*(n-2)
   + 0x2000 - the same window whose writes kill the DSPs on the device (T1/T2 of re/wt5_isolate.py).
4. PEEK (optional). Reads bank 5 back from the master DSPs at 0x41404000 + n*0x100000 + 4*(A000+addr) and compares
   with the expected RAW words; reports how many words differ (0 = the table landed).
```

| option | help | default |
|---|---|---|
| `wav` |  |  |
| `--frames` |  | `1` |
| `--points` |  |  |
| `--frame` |  | `0.0` |
| `--scan` |  |  |
| `--peek` | read bank 5 back from the master DSPs (dsppeek.py) |  |
| `--n` | simulated samples | `4000` |

### `examples/from_scratch/re/wt5_isolate.py`

```
Isolate the two halves of `wt5.py send` on the hardware: program writes (DATA, cmd 1) vs bank-5 writes
(DATA with ext bit 2). Only ADDR (cmd 0) carries the bank-5 bit, so the interesting difference between a program
write that works (dspown) and a bank-5 write that may not is the ADDR that preceded it — or the DPC access itself.

Prerequisite: lab in slot 3 (DSPLIVE with bank-5 commands), PLG3 on the panel, no other MIDI tool running.
A silent result means DSP2/DSP3 are not producing output any more (the note still sounds if the rest of the
engine is alive).

  T1  bank-5 write of a few words, then a program that returns a constant (no bank-5 reads)
  T2  bank-5 write of the whole region 0x048..0x1FD (438 words, as wt5.py sends), then the same program
  T3  no bank-5 access from the ARM at all: `wt5.py send WAV --code` (table written by the DSP microcode)

usage: python3 re/wt5_isolate.py [--only T1,T2,T3] [--note 60]
```

| option | help | default |
|---|---|---|
| `--only` |  | `T1,T2,T3` |
| `--note` |  | `60` |

### `examples/from_scratch/re/wtcheck.py`

```
Check a wavetable oscillator on the device by USB audio: play a scale, compare the harmonics of every note
with the table's (re/wav2dco.py), report the notes where another program sounds.

    python3 wtcheck.py NAME cycle.wav [-n 32] [--cycle 2048 --frame K] [--notes 48 60]
```

| option | help | default |
|---|---|---|
| `name` |  |  |
| `wav` |  |  |
| `-n` |  | `32` |
| `--cycle` |  |  |
| `--frame` |  | `0` |
| `--notes` |  | `[48, 60]` |
| `--harmonics` | compare harmonics 1..HARMONICS (the patch filter damps the high ones) |  |

## Own DSP program generators (SYSTEM-8) (`examples/from_scratch/dsp/`)

### `examples/from_scratch/dsp/analyze_iomd.py`

```
Peak analysis of a capture of the IOMD test (make_iomd_test.py): which tones reach the output.

    arecord -D hw:SYSTEM8,0 -f FLOAT_LE -r 96000 -c 2 -d 6 cap.raw
    python3 analyze_iomd.py cap.raw
Expected: 440 Hz (word 60B0), 660 Hz (60B1), 880 Hz (6020). Level per tone in dB relative to the strongest.
```

### `examples/from_scratch/dsp/check_saw.py`

```
Check saw_dco2.s: assemble it with asm/asm.py and compare its simulated output with re/dspasm.py's SAW.

python3 examples/from_scratch/dsp/check_saw.py      (from the repository root)
```

### `examples/from_scratch/dsp/dump_main.py`

```
Listing of a Roland main program (engine/build/programs.h, from the JX-3P) with its bus references:
IOMD (bank 3, 6nnn), MXM (bank 4, 8nnn) and bank 5 (Annn). Used for docs/WAVE_BUS.md.

    python3 dump_main.py MAIN5 > main5.txt      (needs engine/build, i.e. engine/build.sh run once)
```

### `examples/from_scratch/dsp/make_bmap.py`

```
Channel map of the slave output words (MAIN5 copies [2204]-[221D] into IOMD B0-B7 at the end of each sample,
see docs/WAVE_BUS.md). The DSP5 slot writes one fixed sawtooth per word: B0 = 110 Hz ... B7 = 495 Hz, 55 Hz steps.
A single capture then shows which word reaches which output channel (both slaves run the same program).

    python3 make_bmap.py      -> bmap.s, bmap.hex
```

Environment: `BMAP_WORDS`

### `examples/from_scratch/dsp/make_chain.py`

```
Two-DSP chain for the gate plug-out, all our own asm (asm/asm.py), no Roland code and no envelope or LFO.

    DSP2 (DCO1 slot): SAW at the pitch the ARM writes to bank 5 id 0x21 ([A021]); the phase is the only state.
                      Output -> MXM Z 0x404, which DSP4 reads as its M 0x005 (routing of docs/DSP_ARM_SIDE.md).
    DSP4 (DCO1 slot): output = SAW from DSP2 times the gate the ARM writes to bank 5 id 0x22 ([A022]),
                      stored in IOMD 6020 (canal 1) and 6021 (canal 0) (docs/WAVE_BUS.md).

The gate is 0 or 1 with no smoothing: the note starts and stops at once.

    python3 make_chain.py          -> dsp2_osc.s/.hex, dsp4_gate.s/.hex
    python3 make_chain.py --sim    -> run both programs in re/dspisa.py: pitch 440 Hz, gate pulse on DSP4
```

### `examples/from_scratch/dsp/make_iomd_test.py`

```
Hardware test of the slave's output path (docs/FROM_SCRATCH.md §11).

Writes three fixed-pitch sawtooths, each sample, to three candidate IOMD words of the slave's output bus:
  60B0 = 440 Hz (IOMD0 B0, DSP5 store), 60B1 = 660 Hz (IOMD0 B1), 6020 = 880 Hz (IOMD0 20, DSP4 store).
The program is our own asm (asm/asm.py), registered in the DSP5 DCO1 slot (engine.c -DIOMDTEST).

    python3 make_iomd_test.py          -> iomd_test.s (source), iomd_test.hex (words, execution order)
    python3 make_iomd_test.py --sim    -> also runs the words in re/dspisa.py for 300 samples and prints the periods
```

### `examples/from_scratch/dsp/make_poly.py`

```
Four-voice version of the DSP5 voice (make_voice.py is the one-voice version). Symmetric for both slaves:
each slave's DSP5 runs these four voices, the ARM writes voice k's pitch to bank 5 id 0x60+k and its gate to 0x68+k.

Per voice: AR envelope (rate KA attack / KR release, branch free), SAW at its pitch with the shared LFO
vibrato, times the envelope. The four voices are summed (scale 1/4) into IOMD 6020 (canal 1) and 6021 (canal 0).

    python3 make_poly.py          -> poly.s, poly.hex
    python3 make_poly.py --opt    -> also poly_opt.s / poly_opt.hex: the same voice with ph_wrap (Roland's form)
                                     instead of the manual wraps, scheduled by asm/dspsched.py
                                     (docs/MICROCODE_OPTIMIZATION.md); poly.hex (tested on the device) stays
    python3 make_poly.py --sim    -> four voices in re/dspisa.py: each gate pulse must give its own pitch
```

### `examples/from_scratch/dsp/make_saw_hex.py`

```
Hex words of our DCO2 SAW for engine/programs.py (MC_TEST=dco2_saw=<file>): the slot program with its
start and end words, in execution order. Source: saw_dco2.s (asm/asm.py).

    python3 make_saw_hex.py   -> saw_dco2.hex
```

### `examples/from_scratch/dsp/make_solo.py`

```
Eight SAW voices with note gate in the main program of DSP4, all our own asm (asm/asm.py). Nothing of Roland.

Per voice k (0..7), like make_poly.py without envelope and LFO:
    pitch  bank 5 id 0x70+k ([A070+k]): (note - 12) / 12
    gate   bank 5 id 0x78+k ([A078+k]): 1 while the key is held
    phase  ring state at 0670+4k: stored at S, read at S+1 next sample
Output: (sum of saw_k * gate_k) / 8 in IOMD 6020 (canal 1) and 6021 (canal 0).

The ids of the pitches (0x70..0x77) and of the gates (0x78..0x7F) do not overlap; the engine's note_gate
writes DSP4 id 0x22 only, and the model writes no bank-5 id of DSP4 (checked in engine/build/model.h).

    python3 make_solo.py          -> solo.s, solo.hex
    python3 make_solo.py --sim    -> voice 6 alone: 880 Hz only while its gate is on
```

### `examples/from_scratch/dsp/make_voice.py`

```
Mono voice for the DSP5 DCO1 slot, all our own asm (asm/asm.py), no Roland code.

saw with pitch from the ARM (bank 5 id 0x21, read [A021]) and gate (bank 5 id 0x22, [A022]):
  envelope  AR: rising toward the gate with rate KA, falling with KR (branch free, min/max)
  LFO       sawtooth at LFO_HZ, vibrato of DEPTH (pitch units: 1 = 12 semitones)
  VCA       output = saw * envelope, stored in IOMD 6020 (the word that reaches the audio output)

python3 make_voice.py          -> voice.s (source) and voice.hex (words, execution order)
python3 make_voice.py --sim    -> run the words in re/dspisa.py with a gate pulse and print the result
```

Environment: `ALLOUT`, `DRONE`

### `examples/from_scratch/dsp/patch_ids.py`

```
Patch ids of the JX-3P panel parameters, from the plugin's own Script.xml (no hardware needed).

    python3 patch_ids.py                 -> base check and the ENV, PIT and FLT tables
    python3 patch_ids.py "ENV2 ATTACK"   -> id of one name

Method (2026-10-21, docs/log/WAVE_BUS_LOG.md §4l-§4m):
  1. Read every <value> that has <name>, <range> and <default> in document order (system fields without range are skipped).
  2. Find the base b such that id = b + index matches the INIT patch of model/params.py (100 % of its entries).
  3. Check the names against the model (engine/model_spec.py: env_value uses 0x6ef / 0x6f4 for ENV1 / ENV2).
```

### `examples/from_scratch/dsp/sim_main.py`

```
Run one Roland main program (engine/build/programs.h, copied from the JX-3P) sample by sample on re/dspisa.py.

Only what is measured is simulated. Anything else stops the run with an error that names the missing piece:
  - the condition-flag context of the DSP (--context; only the measured ones, DSP_HARDWARE.md §6)
  - a hidden register h4-h7 the program reads before loading it with 40TY (--hidden)
  - a bank-0, bank-5 or bank-6 word that was never given (--input); the ARM writes banks 5 and 6
  - reads of banks that depend on other DSPs or on the ARM: 1, 2, 3 (IOMD), 4 (MXM), 7
  - bank-5 words above 0x3FF, bank-0 references above 0x7FF, jumps into the middle of an instruction
  - undecoded ops, and ops whose effect is not modelled (dspisa prints them as unknown / reads 0 / ...)
Writes to banks 3 and 4 are collected per sample (outputs to the wave bus and the other DSPs), not read back.
Registers r0-r7 at power-up are not measured: they start at --regs (default 0). --check-init reruns the program
with other random register values and reports the samples where a recorded word changes.

    python3 sim_main.py MAIN0 --context <ctx> --input init.txt --n 1500 --record 0261,025B

Input file: one "t addr value" per line (addr hex, absolute; value = the DSP's RAW value, as dspisa uses it).
  t = 0 is the state before sample 0 (bank 0, 5 and 6 words); t > 0 is written before sample t (banks 5 and 6 only).
Ring: bank 0 is a 0x800-word ring. Between samples every word moves +1 (mod 0x800): stored at S in sample n, read at
S+1 in sample n+1 (DSP_HARDWARE.md §7.2). Nothing else is reset between samples: flags, hidden registers, r0-r7 and
memory carry over (measured, DSP_HARDWARE.md §2).
```

| option | help | default |
|---|---|---|
| `name` |  |  |
| `--context` | condition-flag context (dspisa.CONTEXT_BITS key) |  |
| `--input` | "t addr value" lines (see the docstring) |  |
| `--n` |  | `1500` |
| `--regs` | r0..r7 at power-up, comma separated (default 0) |  |
| `--hidden` | h4..h7 start values, e.g. 4=288,5=0.0039 | `` |
| `--record` | bank-0 addresses printed per sample (hex) | `0261,025B` |
| `--check-init` | rerun with random r0-r7 and report the samples where a recorded word differs |  |

## SYSTEM-1 study tools (`system-1/`)

### `system-1/asgn_sites.py`

```
asgn_sites.py -- list the places in the master app's voice allocator that encode "4 voices".

Disassembles the allocator range of the SYSTEM-1 master app 1.30 (0x60009500-0x6000A400:
the code behind the allocator object at 0x604A26B0, called from the CAssign-like object
0x600A195C, vtable 0x60081008) with arm-none-eabi-objdump and prints every instruction that
uses one of the constants that a 6-voice patch has to revisit:

    #4    voice count in loops / compares (cmp rX, #4; movs rX, #4)
    #3    "& 3" round robin index and "+ 3 & 3" (previous voice)
    #288  0x120 + voice = per-voice parameter id (group 2), sent to the slave

    python3 system-1/asgn_sites.py [work/sys1_v130/app_master.bin] [START END]
```

### `system-1/dsp_fit.py`

```
Does a SYSTEM-8 PLUG-OUT's DSP microcode fit in the SYSTEM-1? Time per DSP, traffic between DSPs, best placement.

    python3 system-1/dsp_fit.py                       # JX-3P, 4 voices (one SYSTEM-8 slave), DCO SAW/SAW, no effect
    python3 system-1/dsp_fit.py --dco1 sqr --dco2 noise --fx ch
    python3 system-1/dsp_fit.py --dco2 none           # one DCO only (DCO2 slot empty)
    python3 system-1/dsp_fit.py --voices 2            # only the voice-0/1 chain (DSP0 -> DSP2 -> DSP4)
    python3 system-1/dsp_fit.py --budget 900 --reserve-ma 60 --samples 4000

Study only (docs/log/VOICE_ALLOCATION_LOG.md §7 scope): Roland's programs are read at run time from the local copy
`examples/from_scratch/engine/build/programs.h` (made by engine/build.sh from plugins/JX-3P), run in the C emulator
(`examples/from_scratch/emu/cchain.py`) and disassembled with `emu/listing.py`. Nothing is copied into the repo.
MAIN0/MAIN1 of programs.h carry the same-size envelope patches of engine/programs.py; the count is the same.

What it measures (system-1/DSP_FIT.md):
 1. per program: bytes, static instructions, executed instructions per sample (min / mean / max over the run, with
    random bank-5 inputs and parameters changed every 250 samples, so the jumps take both ways);
 2. links between the six SYSTEM-8 DSPs, in words per sample: MXM (Z 84nn -> M 80nn / O 82nn of DSP n+2, S 86nn
    back), the pair window A400-A7FF, and the audible IOMD output 6020/6021;
 3. placement on the SYSTEM-1 (slave A/B, master A/B): every assignment of the units (one main each; DCO and FX
    slots on the chip of their main, because they talk to it through its bank 1) that keeps each DSP within the
    budget and the chip-to-chip words within the IOMD bus (slave -> master 6006-600D = 8, master -> slave
    600E-6021 = 20). Prints the best one (least loaded DSP peak) or why nothing fits.
```

| option | help | default |
|---|---|---|
| `--dco1` |  | `saw` |
| `--dco2` |  | `saw` |
| `--fx` | effect slot of DSP4 (default: none) |  |
| `--voices` |  | `4` |
| `--budget` | instructions per sample and DSP (SYSTEM-8: 925-945) | `925` |
| `--reserve-ma` | master A instructions taken by the OS output program at 0x82 [abierto] | `40` |
| `--samples` |  | `2000` |
| `--seed` |  | `1` |

### `system-1/extract_fw.py`

```
extract_fw.py -- extract the SYSTEM-1 OS images (master app, slave app, updater).

Two sources, both with the same region format as the PLUG-OUTs ("SYSTEM-1:..." 0x40-byte
header + LZSS, docs/CUSTOM_PLUGOUTS.md §2-3):

  * firmware/sys1_sys_v130/SYSTEM1_UPD.BIN  Roland's updater, 4 MiB flash image, NOT encrypted:
        0x000000 loader "ESC91.000" (raw)
        0x008000 "SYSTEM-1:Updater"  (2014-03-03)
        0x030000 "SYSTEM-1:Appli"    master app 1.30 (2016-10-26)
        0x130000 "SYSTEM-1:Appli"    slave app 1.30
  * plugins/SYSTEM-1/Script/Code1.Dat       the SYSTEM-1 plug-in's OS update 1.20 (2015-09-24),
        "UPD_POUT_INFO" container, START 70 20 00 00 (PLUG-OUTs use 70 10 00 00):
        flash blocks 0..839 master app, 2048..2312 slave app

Output (work/ is not in git):
    work/sys1_v130/{updater,app_master,app_slave}.bin
    work/sys1_v120/{app_master,app_slave}.bin

    python3 system-1/extract_fw.py
```

### `system-1/fnmatch.py`

```
fnmatch.py -- find a function of one SYSTEM-1 OS build in another build.

The bytes at ADDR (N bytes) in IMAGE_A are turned into a pattern where the
PC-relative parts are wildcards (32-bit BL/B.W, `ldr rX, [pc, #imm]`, `adr`), and the
pattern is searched in IMAGE_B (halfword aligned). Both images load at 0x60000000.

    python3 system-1/fnmatch.py work/sys1_v120/app_master.bin work/sys1_v130/app_master.bin         0x60006606:28 0x60006534:64

Used to carry addresses found in 1.20 (the plug-in's image) over to 1.30 (the device).
```

### `system-1/s1_progs.py`

```
DSP programs of a SYSTEM-1 PLUG-OUT: where each one goes, its cost, and what it exchanges (study only).

    python3 system-1/s1_progs.py                      # SH-101, SH-2, PROMARS (work/unpack/<NAME>_S1_Code1)
    python3 system-1/s1_progs.py work/unpack/SH-2_S1_Code1 --listing

For each program of region 0 (master) and region 1 (slave), found as {address, bytes, pointer} entries of the
program sets: the address (0x0100 main, 0x1000 master B, 0x1900/0x3100 voice sections), static instructions,
instructions executed per sample in the C emulator (random bank-5 / parameter / IOMD inputs; equal to static when
the program has no jumps), the IOMD words it loads and stores (600E-6021 master -> slave, 6006-600D and
6026-602D slave -> master) and the bank-5 words it stores. Used by system-1/DSP_FIT.md §4 to compare how far each
mono synth could be made polyphonic. Needs the unpacked Code1.Dat:
`python3 tools/plugout_build.py unpack plugins/<NAME>/Script/Code1.Dat work/unpack/<NAME>_S1_Code1`.
```

### `system-1/vfp_count.py`

```
Count ARM VFP (floating-point unit) instructions in binary images: does the code use an FPU?

    python3 system-1/vfp_count.py IMAGE...
    python3 system-1/vfp_count.py            # the default set: SYSTEM-8 OS and engines vs SYSTEM-1 OS and PLUG-OUTs

Scans every halfword pair as Thumb-2: coprocessor instructions (first halfword ECxx/EDxx/EExx, the forms a
compiler emits; FFxx/EFxx match mostly data) whose coprocessor
field is 10 or 11 (VFP single / double). Data words also match by chance, so a count of about 1 per 10 KiB is the
noise floor; real VFP code gives tens per 10 KiB. Result and interpretation: system-1/ARCHITECTURE.md §1.1.
```

### `system-1/voice_sets.py`

```
voice_sets.py -- DSP program sets of the SYSTEM-1 slave app and their cost per sample.

The slave app loads DSP programs in "sets". A set is a run of 12-byte entries
{u32 program address (bytes), u32 length (bytes), u32 pointer to the program}:

    0x0000  head (data at 0x0000, then code)   0x100  main part (empty in voice sets)
    0x1900  voice 0           0x3100 voice 1
    0x4900  voice 2           0x6100 voice 3
    0x7FF2  7-halfword stub that runs on into the head at 0x0000 (program memory
            wraps at 0x8000 bytes)

i.e. four voice slots of 0x1800 bytes in one program memory. Programs are stored as
32-bit words with the two halfwords swapped (as everywhere else in the corpus).
Cost = number of DSP instructions (1 cycle each, DSP_HARDWARE.md; budget about 925-945
per sample, the exact edge depends on the DSP).

    python3 system-1/voice_sets.py [work/sys1_v130/app_slave.bin] [--entries]
```

## C-S1V4 / C-S1V8 (SYSTEM-1) (`examples/system_1/`)

### `examples/system_1/build.sh`

```
C-S1V4: the C-V4 from-scratch PLUG-OUT ported to the SYSTEM-1 (docs/examples_system_1_README.md).
examples/system_1/build.sh                -> plugins/C-S1V4/Script/Code1.Dat (and Code.Dat, the same file)
TONE=1 NAME=C-S1TONE examples/system_1/build.sh   bring-up build: voice 0 sounds all the time (no notes needed)
PARAMLOG=1 TONE=1 NAME=C-S1LOG ...   diagnostic: the model sends every parameter it receives to the PC (model.c)
Needs arm-none-eabi-gcc/ld/objcopy and Python 3. No Roland file is read: all code and microcode are ours.
```

### `examples/system_1/make_s1.py`

```
C-S1V4: our own DSP programs for the SYSTEM-1 port of C-V4 (no Roland bytes; docs/examples_system_1_README.md).

The SYSTEM-1 has two DSPs per BMC (A and B, queues 0x0100FC30 / 0x0100FC34), and in PLUG-OUT mode the audio goes
slave DSP A/B -> IOMD 6006..600D -> master DSP B (program at 0x1000) -> bank 5 A118/A119 -> the OS's own output
program of master DSP A (at 0x82: A118/A119 -> DC filter -> x C004 -> 6000/6001). Found in the SH-2 and SH-101
images and the OS 1.30 (study only, nothing copied; docs/examples_system_1_README.md section 2).

C-V4's flow on SYSTEM-8 was DSP0/1 pitch -> DSP2/3 saw slot -> DSP4/5 gate -> 6020. Here one DSP does it all:

    slave A  s1m0 @0x0100: 4 voices: pitch A030+v (ARM, octaves: (note-12)/12), saw (the C-V4 saw, phase in the
                           bank-0 ring: store P, load P+1), x gate A038+v (ARM, 0/1), sum x 0.25 -> A150
    slave B  s1m1 @0x0100: A150 x C000 (the DSP's output level, set by the OS) -> 6006 (L), 6007 (R); 6008..600D = 0
    master B m1   @0x1000: 6006 x C01C -> A118, 6007 x C01C -> A119; A11A..A11D = 0 (C01C = volume, model.c)
    closes   s1c0/s1c1 @0x7FF2 (slave A / B): frame counter E000 -> A004 / A005, as every SYSTEM-1 PLUG-OUT has
             s1e @0x5FF2/0x6FF2 (master B): empty section, as the OS's own PLUG-OUT-mode set has

    python3 make_s1.py            -> s1*.s / s1*.hex here, build/s1_programs.h
    python3 make_s1.py --tone     -> the same, but s1m0 plays voice 0 at 4.75 (A) all the time (bring-up test:
                                     sound without notes or ARM parameters)
    python3 make_s1.py --sim      -> also simulate one chord through slave A, slave B and master B
```

Environment: `VOICES` (default '4')

### `examples/system_1/paramlog.py`

```
Listen to a PARAMLOG build and list the parameters the device hands to the PLUG-OUT model (docs/examples_system_1_README.md §6).

The PARAMLOG build of C-S1V4 (PARAMLOG=1 build.sh) sends one SysEx per call of the model's vt+0x30:
    F0 7D 06 <id >> 7> <id & 7F> <flag> <value: bits 31-28, 27-21, 20-14, 13-7, 6-0> F7
Everything else the device sends (CC echoes of the panel, ...) is shown too, interleaved, so a knob's id can be
matched with its MIDI CC.

    python3 examples/system_1/paramlog.py                 listen 60 s on every SYSTEM-1 port, then summarize
    python3 examples/system_1/paramlog.py -t 120 --raw    also print every message as it arrives
    python3 examples/system_1/paramlog.py --file LOG...   decode saved `amidi -d` output instead
    python3 examples/system_1/paramlog.py --device SYSTEM-8   the same on a SYSTEM-8 (a build that sends F0 7D 06)

Uses `amidi -d` (alsa-utils). Do not run it while another tool talks to the device.
```

| option | help | default |
|---|---|---|
| `-t, --time` | seconds to listen (default 60) | `60` |
| `--raw` | print every message too |  |
| `--file` | decode these amidi -d logs instead of listening |  |
| `--device` | port name to listen on (SYSTEM-8 for the SYSTEM-8) | `SYSTEM-1` |

## C-S1SY (SYSTEM-1) (`examples/system_1_synth/`)

### `examples/system_1_synth/build.sh`

```
usage: [VAR=value ...] sh examples/system_1_synth/build.sh [--help]

Builds the C-S1SY PLUG-OUT for the SYSTEM-1 -> plugins/<NAME>/Script/Code1.Dat (and Code.Dat, the same file).
Install: python3 tools/plugout_tool.py install plugins/<NAME> --slot 1 --force --no-patches -y
Then select the PLUG-OUT on the panel.

Options (environment variables):
  STEP=n        incremental step (default 8). Each one adds one thing to the previous one:
                  1 DCO2 (second saw, +7 cents)          2 OSC1/OSC2 levels from the panel
                  3 per-voice transport DSP A -> B       4 amp ADSR (DSP B)
                  5 OSC ranges + OSC2 fine tune          6 4-pole low-pass (cutoff, resonance, key track)
                  7 waves: selector position 2 = square, 3 = triangle, others saw
                  8 filter ADSR (faders 0x182..0x185) and env amount (0x181)
                  9 LFO on DSP A: rate 0x162, shape 0x160, delay 0x161, -> pitch 0x163 / filter 0x164 /
                    amp 0x165 (works on the device, 2026-10-23; DSP A 693 instructions)
                 10 the saw with ph_wrap (Roland's form) instead of the 11-op manual wrap: DSP A 517
                 11 LFO per voice (own phase restarted at each note, own S&H), -> filter / -> amp bipolar
                    (128 = 0, below = inverted LFO); op fusion on (FUSE): DSP A 576, B 603 (works on the device)
                 12 the list scheduler asm/dspsched.py instead of pack(): DSP A 553, B 471 (emulator only)
                 13 OS delay / reverb send-return in master B (C-S1SY13, last fully working build)
                  0 the first all-at-once design (silent on the device; reference only)
  SCHED=0|1     schedule with asm/dspsched.py instead of pack() (default 1 from STEP 12)
  FUSE=0|1      fuse independent ops into one instruction as Roland (default 1 from STEP 11, docs/
                MICROCODE_OPTIMIZATION.md); every build runs tools/check_optimized.py on the programs
  FOLD=1        (with STEP=13) the triangle of each oscillator wavefolded by its COLOR knob (0x168 / 0x16D);
                FOLD_MAX=x = gain at COLOR 255 (default 5). Doc §7.5
  PACK=0|1      pack loads / stores into the ops' instructions as Roland does (default 1). PACK=0 = one unit
                per instruction: STEP=7 PACK=0 is over the time budget of slave DSP A and hangs the device
  NAME=...      plug-out name shown on the device and plugins/ folder (default C-S1SY); max 16 characters
  GMAX=x        filter coefficient limit (default 0.7, steps >= 6): caps the cutoff (0.7 = 10.7 kHz). Above
                ~0.85 the loop oscillates at 48 kHz (Nyquist) with full resonance: inaudible, but it eats the
                filter's headroom (emulator, 2026-10-23). It does not give audible self-oscillation
  RESCOMP=1     resonance compensated by cutoff (steps >= 6): the filter self-oscillates with full resonance
                from ~80 Hz to ~2.8 kHz cutoff, as an analog ladder (works on the device 2026-10-23; the
                oscillation keeps sounding after note off, and needs oscillator level to start; DSP B 708
                instead of 660 instructions = most tried). Without it it never self-oscillates
  RES_MAX=x     feedback at the top of the resonance knob (default 3.6; 4.4 with RESCOMP; 4 = threshold)
  VOICES=n      voices (default 8; the OS forwards at most 8)
  MODEL_ID=0x.. model id in the SYNINFO (default 0x0010)
  PARAMLOG=1    diagnostic: region 0 sends every parameter the OS hands it to the PC as SysEx F0 7D 06;
                capture with python3 examples/system_1/paramlog.py -t 90 --raw (select the PLUG-OUT first)
  PAD=n         budget test: n harmless fillers at the end of slave DSP A's program
  STAGE=n       only with STEP=0: how much of the all-at-once design (1..4)

Examples:
  sh examples/system_1_synth/build.sh                                (STEP=8, packed -> plugins/C-S1SY)
  GMAX=0.9 NAME=C-S1SYG sh examples/system_1_synth/build.sh
  RESCOMP=1 NAME=C-S1SYR sh examples/system_1_synth/build.sh
  PARAMLOG=1 NAME=C-S1SYL sh examples/system_1_synth/build.sh
  STEP=7 python3 examples/system_1_synth/make_synth.py --sim       (DSP programs + simulation only)
```

### `examples/system_1_synth/make_synth.py`

```
C-S1SY: an 8-voice subtractive synth for the SYSTEM-1, all our own microcode (docs/examples_system_1_synth_README.md).

Extends C-S1V8 (examples/system_1): per voice two independent oscillators (wave + level each, DCO2 detuned), an
ADSR VCA, a 4-pole low-pass filter (cutoff, resonance, key track) and a second ADSR for the cutoff.

    slave A  s1m0 @0x0100  per voice: DCO1 + DCO2 (saw / pulse / triangle mix from the ARM) -> A068+v,
                           filter ADSR -> A070+v
    slave B  s1m1 @0x0100  first: sum of last sample's filter outputs x C000 -> 6006 / 6007 (6008..600D = 0);
                           per voice: amp ADSR, x = A068+v x amp env x comp, cutoff (base + amount x A070+v +
                           key track x pitch) -> g = 2 pi f / fs, ladder of 4 one-pole stages with feedback k
    master B s1mm1 @0x1000 6006 / 6007 x volume (C01C) -> A118 / A119 (unchanged from C-S1V4)

Built step by step on the hardware (STEP=n, STEPS below; default 7 = everything up to the waves, packed as Roland
does: pack()). The design above (STEP=0, STAGE=1..4) is the first all-at-once attempt: silent on the device, kept
as reference. The filter ADSR is still to come.

    python3 make_synth.py           -> s1*.s / s1*.hex here, build/s1_programs.h
    python3 make_synth.py --sim     -> also simulate notes through slave A, slave B and master B (emulator C core,
                                       seconds; SIM=py: the Python reference, minutes)
    python3 make_synth.py --pack-check -> packed / fused programs against the unpacked ones (C only, seconds)
    python3 make_synth.py --sim-check -> also the C core against the Python reference, sample by sample (≈ 25 s;
                                         only when the emulator, simulate() or the instruction forms change)
```

Environment: `CHDIAG`, `CHFRAC` (default 'mod2'), `ENS` (default '2'), `FOLD` (default '0'), `FOLD_MAX` (default '5.0'), `FUSE` (default '1' if STEP >= 11 else '0'), `GMAX` (default '0.7'), `M1` (default ''), `PACK` (default '1'), `PAD` (default '0'), `RESCOMP` (default '0'), `RES_MAX` (default '4.4' if RESCOMP else '3.6'), `SCHED` (default '1' if STEP >= 12 else '0'), `SIM`, `SIM_CHECK_N` (default '600'), `SIM_JOBS` (default '0'), `SIM_N` (default '9600'), `SIM_PACK_N` (default '19200'), `STAGE` (default '4'), `STEP` (default '8'), `VOICES` (default '8')

### `examples/system_1_synth/paramlog.py`

```
Listen to a PARAMLOG build and list the parameters the device hands to the PLUG-OUT model (docs/examples_system_1_README.md §6).

The PARAMLOG build of C-S1V4 (PARAMLOG=1 build.sh) sends one SysEx per call of the model's vt+0x30:
    F0 7D 06 <id >> 7> <id & 7F> <flag> <value: bits 31-28, 27-21, 20-14, 13-7, 6-0> F7
Everything else the device sends (CC echoes of the panel, ...) is shown too, interleaved, so a knob's id can be
matched with its MIDI CC.

    python3 examples/system_1/paramlog.py                 listen 60 s on every SYSTEM-1 port, then summarize
    python3 examples/system_1/paramlog.py -t 120 --raw    also print every message as it arrives
    python3 examples/system_1/paramlog.py --file LOG...   decode saved `amidi -d` output instead
    python3 examples/system_1/paramlog.py --device SYSTEM-8   the same on a SYSTEM-8 (a build that sends F0 7D 06)

Uses `amidi -d` (alsa-utils). Do not run it while another tool talks to the device.
```

| option | help | default |
|---|---|---|
| `-t, --time` | seconds to listen (default 60) | `60` |
| `--raw` | print every message too |  |
| `--file` | decode these amidi -d logs instead of listening |  |
| `--device` | port name to listen on (SYSTEM-8 for the SYSTEM-8) | `SYSTEM-1` |

## C-S1JX (SYSTEM-1) (`examples/system_1_jx/`)

### `examples/system_1_jx/build.sh`

```
C-S1JX: the JX-3P's own DSP microcode (one DCO, 4 voices) on the SYSTEM-1 (docs/examples_system_1_jx_README.md).
examples/system_1_jx/build.sh            -> plugins/C-S1JX/Script/Code1.Dat (and Code.Dat, the same file)
SLAVE_TEST=1 NAME=C-S1JXS ...             diagnostic: JX-3P on the slave only, minimal master program (model.c)
RUNTIME_MODEL=1 ...                       the parameter model evaluated on the ARM (soft float) instead of the table
V4_PROGS=1 NAME=C-S1JX4 ...              diagnostic: C-S1V4's DSP programs with our ARM code (model.c, engine.c)
NO_MAIN1=1 ...                           diagnostic: slave B main = an empty program instead of MAIN1
NOJUMP=UNIT[,UNIT] ...                   diagnostic: the unit's jumps neutralized (s1jx.nojump)
TRUNC=UNIT:lo:hi ...                     diagnostic: only instructions lo..hi-1 of a unit (s1jx.truncate)
NO_STATE=1 ...                           diagnostic: no start-up writes
SEC_A_EMPTY=1 ...                        diagnostic: an empty program of ours in the slave A section
NO_MAIN5=1 ...                           diagnostic: slave A main = an empty program instead of MAIN5
NO_SEC_A=1 ...                           diagnostic: slave A section (MAIN3) left off
NO_SEC_B=1 ...                            diagnostic: slave B section (DCO1 of voices 2/3) left off (engine.c)
Needs plugins/JX-3P (the microcode is read from it at build time; build/ and the plugin folder hold Roland bytes:
never commit them), arm-none-eabi-gcc/ld/objcopy and Python 3. Run test_s1jx.py before installing.
```

### `examples/system_1_jx/dspreloc.py`

```
Memory references of a DSP program and their relocation (same decoder as the emulator: re/dspisa.Layout).

    import dspreloc as R
    p = R.Prog(words, 'MAIN2')          # words in execution order
    p.refs                              # [Ref(ins, op 'ld'/'st', reg, addr, pos)]; pos = index of the address word
    p.footprint()                       # {bank: (loads, stores)} of literal addresses
    p.ring_reach(0x0)                   # largest k such that a load at A reads what the program stored at A-k
    q = p.relocate(lambda r: new_addr)  # new word list: only the address words change (same lengths, jumps kept)

Every memory transfer of the JX-3P programs is a literal one (mode fe, address in a slot word); other transfer modes
(X28Y ring-relative, bank-5 indexed, hidden registers) are reported by `Prog.other` and refused by relocate().
```

### `examples/system_1_jx/model_full.py`

```
Parameter model of the from-scratch engine: what the engine writes to the
slave DSPs for every parameter value. Our own description, written from
measurements of the JX-3P's behaviour (re/measure.py, deps.py, probe.py,
sweepfit.py, knotfit.py): formulas where one fits exactly, otherwise
least-squares knots (marked with their error). Nothing here is copied code;
the numbers are the measured response.

    out(source, targets, expr, when=None)

source  parameter id whose change triggers the write (vt+0x30), None = start-up only
targets 'P<dsp>:<id>[/r<ramp>f<flag>]' parameter block (+0xA0 on change, +0xA4 at
        start-up), 'B<dsp>:<id>' bank 5 (+0xB0); several separated by spaces
expr    expression over the current parameter values (expr.py); P(id), SLAVE
when    optional condition: the write happens only if it is not 0

Several sources may write the same DSP id; the last write wins, as on the
JX-3P. At start-up / full refresh the sources are evaluated in REFRESH_ORDER.
```

Environment: `DSPTEST`

### `examples/system_1_jx/s1jx.py`

```
C-S1JX: the JX-3P's own DSP microcode (one DCO) on the SYSTEM-1, 4 voices (system-1/DSP_FIT.md, docs in
docs/examples_system_1_jx_README.md).

The JX-3P's six SYSTEM-8 slave DSPs become the four DSPs of the SYSTEM-1, one chain of two voices per chip:

    S8 DSP  program            SYSTEM-1
    DSP0    MAIN0  LFO/env v0,1  master B @0x1000  MAIN0 + DCO1(v0,1) + tail (output x VOLUME -> A118/A119)
    DSP2    MAIN2  + DCO1        master A @0x0100  MAIN2 + MAIN4 (one program)
    DSP4    MAIN4  VCF/VCA, mix
    DSP1    MAIN1  LFO/env v2,3  slave B  @0x0100  MAIN1;  section 0x1900: DCO1(v2,3)
    DSP3    MAIN3  + DCO1        slave A  @0x0100  MAIN5;  section 0x1900: MAIN3
    DSP5    MAIN5  VCF/VCA v2,3
    DCO2 is not used: its outputs read as 0 (the OSC2 controls do nothing).

Every literal address of the Roland programs is rewritten (dspreloc.py), nothing else changes:
  * bank 0 / bank 1 rings: each program gets its own range on its chip (disjoint even if A and B share them);
  * bank 5 (shared by A and B of a chip): each program's parameter block, and mailbox words for what went through
    MXM (Z 84nn -> M 80nn / O 82nn, S 86nn back), through the DSP2/3 <-> DCO bank-1 interface, and for constants
    (ZERO: read 0, DUMP: written, never read);
  * pair window A4xx (DSP0->1, DSP4<->5) and the S8 IOMD inputs/outputs: chip to chip by IOMD 600E.. (down) and
    6006.. (up); MAIN4's 6020/6021 -> OUT_L/OUT_R; the unknown S8 inputs (60e0-60e2, 6000-6006, 6010-6017) -> ZERO;
  * bank 6 (per DSP): an id offset where two programs share a DSP.
The ARM side (model.c, engine.c) writes the JX-3P model's parameters through the tables of build/s1jx.h.

    python3 s1jx.py                 -> build/s1jx.h (programs + tables), build/plan.txt
    python3 s1jx.py --check [n]     -> also: emulate the SYSTEM-1 layout next to the SYSTEM-8 chain (C emulator)

The microcode is read at build time from plugins/JX-3P (engine/programs.py); build/ is not committed.
```

Environment: `DCO1` (default 'saw'), `NOJUMP` (default ''), `TRUNC` (default '')

### `examples/system_1_jx/sigdiff.py`

```
Instruction signatures of the C-S1JX programs that no Roland SYSTEM-1 program has (docs/examples_system_1_jx_README.md
section 12).

    python3 sigdiff.py              # the final programs of the plan (as the build makes them)
    python3 sigdiff.py --units      # per unit (MAIN0-5, DCO1@2/3), with handoff and split, before concatenation
    python3 sigdiff.py --dat plugins/C-S1JXS/Script/Code1.Dat     # the programs of an installed build
    python3 sigdiff.py --corpus     # only the reference sets: what the SYSTEM-1 programs use
    options: --suspects (only features no good device build has), --sig (also whole signatures)

Reference ("seen on the SYSTEM-1") = every DSP program in the program sets of the SYSTEM-1 OS 1.30
(work/sys1_v130/app_master.bin, app_slave.bin; system-1/extract_fw.py) and of the SYSTEM-1 PLUG-OUTs SH-101, SH-2,
PROMARS (plugins/<NAME>_S1_Code1/Script/Code1.Dat or work/unpack/<NAME>_S1_Code1). Those programs run on the device,
so a feature they use is known to be accepted by it. A feature of ours that none of them uses is a suspect, not a
proof: the device may accept it (the SYSTEM-8 Roland programs, a second reference, run it on the same DSP family).

Features of one instruction (each one checked on its own, then the whole signature):
  cl      class (header >> 5)
  cls     class and length
  op      each operation mnemonic (disassembler, asm/disasm.py); `raw` if the instruction is kept verbatim
  acc     per-instruction access counts per (bank, ld/st): `b0 st x2`
  mode    transfer mode byte other than fe (indexed, hidden registers, flags)
  jump    009Y units (jumps) and C units (conditions)
  pred    per C unit: its condition code (`cc N`) and the kind of unit it can skip (an op, or a transfer: `skips st
          b0`); 40TY hidden-register loads
  ntr     number of transfers
  sig     class + sorted ops + sorted accesses: the whole shape of the instruction
```

### `examples/system_1_jx/timing.py`

```
When each bank-5 mailbox word between the two DSPs of a SYSTEM-1 chip is written and read inside the sample, against
the latency the SYSTEM-8 had (docs/examples_system_1_jx_README.md section 13).

    python3 timing.py [samples]     # default 300 samples of the emulated SYSTEM-1 layout, random inputs

The two DSPs of a chip (master A/B, slave A/B) run AT THE SAME TIME on the device and share bank 5. A word that one
writes and the other reads is seen in the same sample only if the write comes before the read in cycles (1 cycle per
instruction, docs/DSP_HARDWARE.md). The emulator runs the DSPs one after the other, so it cannot show this: here the
executed instruction sequence of every DSP comes from the emulator's trace, and the cycle of each instruction is its
position in that sequence plus the programs the OS runs before ours (frame header at 0x0000; master A: the output
program at 0x82). [inferred] both DSPs of a chip start the sample on the same cycle; the exact visibility edge
(pipeline) is a few cycles, hence the margin.

Latency the S8 had, per mailbox kind (s1jx.py Plan keys):
  MZ / MS  MXM: 1 sample (latched bus, DSP_HARDWARE.md)
  DCO      main -> its DCO slot: 0 (same S8 DSP, the main ran first); DCO -> main: 1 (the main reads it at +1 of the
           bank-1 ring, the value of the sample before)
  REG      r2 / r3 handoff (main -> DCO): 0
Verdict per word: OK (S1 latency = S8), LATE (S1 one sample later), EARLY (S1 sees this sample's value, S8 the one
before) or RACE (writes and reads interleave, or the margin is under MARGIN cycles: depends on the exact timing).
```

## Mutable Instruments port toolkit (docs/MUTABLE_SDK.md) (`sdk/mport/`)

### `sdk/mport/bench.py`

```
A component alone on one emulated DSP (docs/MUTABLE_SDK.md §6): the port measured without a host program.

    import bench
    out, info = bench.run(SawSwarm(), note=69, knobs={'ratio': 128, 'hp': 0}, n=9600)

Layout of the bench program: pitch word at bank 5 A030 (written as the ARM writes it, float32), the component's
parameters from C040 up (in the order of Component.params), its state words from bank-0 0x0100 up (2 words each:
the ring pair S / S+1), its outputs stored to bank 5 A1F0 up. inc = the host's increment of the pitch
(Kernel.increment). The program is scheduled with asm/dspsched.py (sched=False: plain, one unit per instruction) and
runs on the C core of the emulator (examples/from_scratch/emu/cchain.py).
```

### `sdk/mport/cli.py`

```
sdk/mport command line (docs/MUTABLE_SDK.md §8).

python3 sdk/mport/cli.py list                         ported components
python3 sdk/mport/cli.py check saw_swarm              port vs the original C++ (and the integer model)
python3 sdk/mport/cli.py lines saw_swarm [--plain]    the bench program: scheduled (default) or one unit per line
python3 sdk/mport/cli.py lut braids ws_moderate_overdrive --odd 9 [--xmax 0.875]
                                                      polynomial fit of a resources.cc table, error in LSB
```

| option | help | default |
|---|---|---|
| `component` |  |  |
| `-n` | samples per case (default 9600 = 100 ms) | `9600` |
| `--window` | samples of the sample-by-sample SNR (default 960) | `960` |
| `component` |  |  |
| `--plain` | one unit per line, unscheduled |  |
| `module` |  |  |
| `table` |  |  |
| `--kind` | Interpolate88 (x in -1..1) or 824 (0..1) | `88` |
| `--odd` | odd polynomial of this degree |  |
| `--deg` | full polynomial degree (without --odd) | `5` |
| `--xmin` |  | `-1.0` |
| `--xmax` |  | `1.0` |

### `sdk/mport/compare.py`

```
Port against reference: the numbers port-check prints (docs/MUTABLE_SDK.md §6).
```

### `sdk/mport/component.py`

```
What a ported component is (docs/MUTABLE_SDK.md §5).

A Mutable Instruments render function splits in two:

  control rate  what Render() computes once per block from pitch_ and parameter_[] (increments, coefficients,
                table reads at a knob position). Here: Python functions of the knob value (0..255), evaluated at
                build time into ARM tables; the ARM writes the result as DSP parameters C0xx (FCEC) when the knob
                moves. -> Param
  audio rate    the per-sample loop. Here: microcode written with a Kernel (kernel.py), with the state of the loop
                in bank-0 ring words (load S+1, store S) and the inputs / outputs as values. -> Component.kernel()

    class MyOsc(Component):
        name, source = 'my_osc', 'braids/digital_oscillator.cc RenderMyOsc'
        params = (Param('k', lambda v: ..., count=1),)
        state = ('phase',)
        inputs, outputs = ('inc',), ('out',)
        def kernel(self, k, ins, prm, st):          # ins / outputs: {name: Value}; prm[name](i) -> Value
            ...                                     # st[name] -> kernel.Ring
            return {'out': y}

A component may also give parts (methods that take the same arguments) so that a host puts them on different DSPs:
Braids' SAW_SWARM runs its seven saws and the waveshaper on slave DSP A and its high-pass on slave DSP B in the
MUTABLE-SYSTEM example (examples/mutable_system).
```

### `sdk/mport/kernel.py`

```
Per-sample kernels written with values instead of registers -> our DSP microcode (docs/MUTABLE_SDK.md §3).

A ported component describes its sample loop as operations on values; the kernel picks the registers, writes one
unit per line (the form every generator of this repo writes: make_synth.py, make_poly.py) and puts the load-delay
fillers where the plain program needs them. asm/dspsched.py then packs and fuses it as for any other program.

    k = Kernel('swarm v0', regs=(0, 1, 2, 3, 4, 7))     # r5 / r6 belong to the host program (C-S1SY: 0 and 1)
    p = k.state(0x0500)                                  # ring word: load 0x0501 (last sample), store 0x0500
    inc = k.mul(k.load(0xA030), 0.0091)
    q = k.wrap(k.add(p.value, inc))
    p.write(q)
    k.store(q, 0xA151)
    lines = k.lines()                                    # 'ld r0, [0501]', 'mov r5, r5', 'mul r1, r0, #0.0091', ...

Operations (x: a value; y: a value, -value (negated register) or a number = immediate):

    mul(x, y) = x y            add(x, y) = x + y           sub(x, y) = x - y
    mad(acc, x, y) = acc + x y        ymac(acc, x, y) = y + x acc (Horner step, op 'ymacd')
    min(x, y), max(x, y)       mov(y)                     wrap(x): into [-1, 1)
    tri(x) = 1 - 2 |wrap(-x - 1.5)|   clamp(x) to [-1, 1]  (the phase ops of unit B, Roland's form X = r3)

Semantics kept from the hardware (DSP_ISA.md §9): all units of an instruction read before they write (a result may
take the register of an operand that dies there); a loaded register is usable one instruction later (filler in the
plain program, removed by the scheduler). Only op kinds already run on the SYSTEM-1 are emitted (the OPS set of
dspsched; asm/proven_forms.txt for the fused pairs).
```

### `sdk/mport/lut.py`

```
Lookup tables of the Mutable Instruments sources -> what our DSP can run (docs/MUTABLE_SDK.md §4).

The modules read their curves from int16 / uint16 tables in resources.cc (generated by resources/*.py). On our DSP
an audio-rate table read costs an indexed load and a hidden-register set per point; a short polynomial costs a few
fused ops and no memory. So:

  * audio-rate curve (waveshaper, sine): fit_odd() / fit() -> coefficients for Kernel.poly_odd() / horner(), with
    the error in LSB of the original int16 output;
  * control-rate curve (a coefficient that only changes with a knob or the note): evaluate it in Python at build
    time, at every knob value, and give the ARM a float table (as make_synth.py header() does).

    t = lut.table('braids', 'ws_moderate_overdrive')        # list of ints, as in resources.cc
    f = lut.interp88(t)                                     # Braids' Interpolate88 as a function of x in [-1, 1)
    c, err = lut.fit_odd(f, 7, x_max=0.875)                 # odd polynomial of degree 7, err = max |error| (LSB)
```

Environment: `EURORACK` (default os.path.expanduser('~/git/eurorack')

### `sdk/mport/mi.py`

```
Mutable Instruments' fixed-point conventions -> the number model of our DSP (docs/MUTABLE_SDK.md §2).

Our DSP computes with signed numbers up to +-512 (29-bit mantissa, results truncated toward zero; DSP_HARDWARE.md),
at 96 kHz like Braids. Each MI quantity has one place in that model:

  MI (Braids / Plaits)                          here
  sample int16 (-32768..32767)                  float -1..1 (x = s / 32768)
  phase uint32 (0..2^32), saw = phase >> 19     phase -1..1 wrapped by ph_wrap: p = phase / 2^31 - 1
  phase increment uint32                        2 f / fs (the step of p)
  pitch int16, MIDI note << 7 (1/128 semitone)  our pitch word: (note - 12) / 12 octaves (4.75 = A 440)
  parameter (TIMBRE / COLOR) 0..32767           SYSTEM-1 knob 0..255 -> param(v) = v x 32767 / 255
  a * b >> 15 (Q15 product)                     a x b with a, b as floats (/ 32768)
  CLIP(x) to int16                              ph_clamp (to -1..1)
  table lookups                                 lut.py: polynomial (audio rate) or ARM table (control rate)

Control-rate code of a Render() call (what Braids computes once per block of 24 samples from pitch_ and parameter_)
runs at build time in Python, at every knob value, and reaches the DSP as parameters C0xx from ARM tables.
```

### `sdk/mport/reference.py`

```
Reference renders from the original Mutable Instruments C++ (docs/MUTABLE_SDK.md §6).

The eurorack checkout (EURORACK, default ~/git/eurorack) is compiled as it is, with the small stmlib stand-ins of
sdk/mport/ref/stmlib (the submodule is empty there). The binary goes to sdk/mport/build/ (not in git) and is rebuilt
when a source is newer.

    import reference
    x = reference.braids(shape=1, note=69, p1=16000, p2=10922, n=9600)    # numpy float array, int16 / 32768
```

## s1cc glue: kernel placement and emulator check (docs/S1CC.md) (`sdk/s1cc/tools/`)

### `sdk/s1cc/tools/s1check.py`

```
s1cc check: a built kernel on the emulator against the native reference of the same C / C++ (docs/S1CC.md §10).

    sdk/s1cc/s1cc check OUTDIR [--note 57] [--n 9600] [name=knob ...] [--show]

Bench: slave DSP A (emulator DSP 0) runs stage A, slave DSP B (DSP 1) stage B, one shared bank 5 (the SYSTEM-1
slave pair); pitch word at A030, gate A038 = 1, trigger A060 = 1 (one note start, as the reference); stage A's
output to A151 (= stage B's input), exports at A1A0 + i, inputs for B at A1B0 + i, B's output to A1C0; state words
in each DSP's bank 0 from 0x0100 (2 words each), scratch words from 0x0300; parameters from C040 in .param order, on both DSPs. Programs
scheduled with asm/dspsched.py (only the proven op groups unless --all-forms). Prints the SNR over the first
10 ms and over all, the band levels port - reference, and the instruction counts.
```

| option | help | default |
|---|---|---|
| `outdir` |  |  |
| `--note` |  | `57` |
| `--n` |  | `9600` |
| `--all-forms` | schedule with every op group (not only the proven) |  |
| `--show` | print the scheduled programs |  |
| `--trace` | the kernel must be built with --trace (s1cc build --trace): report the first C operation whose value differs |  |
| `knobs` | name=knob (0..255; default 128) |  |

### `sdk/s1cc/tools/s1kernel.py`

```
Reading an s1cc kernel (.s1k, docs/S1CC.md §8) and placing it at real addresses: the glue between s1cc (C++)
and the repo's Python assembler / scheduler / emulator. Used by s1check.py and by examples that embed a kernel
(examples/mutable_system OSC=...).

    k = Kernel.read('voice.s1k')
    lines_a = k.place('A', {'IN:pitch': 0xA030, 'S0': 0x0100, ..., 'OUT': 0xA151})   # one-unit lines
    k.param_values('ratio', knob=128, tables=read_tables('tables.txt'))             # floats the ARM writes
```

## MUTABLE-SYSTEM (SYSTEM-1) (`examples/mutable_system/`)

### `examples/mutable_system/build.sh`

```
usage: [VAR=value ...] sh examples/mutable_system/build.sh [--help]

Builds MUTABLE-SYSTEM for the SYSTEM-1 -> plugins/<NAME>/Script/Code1.Dat (and Code.Dat, the same file):
C-S1SY13F (examples/system_1_synth: LFO, ADSRs, 4-pole low-pass, OS delay / reverb) with Braids' SAW_SWARM
(7 detuned saws, tanh waveshaper, high-pass) in place of DCO1 + DCO2, ported with sdk/mport
(docs/examples_mutable_system_README.md, docs/MUTABLE_SDK.md).
Install: python3 tools/plugout_tool.py install plugins/<NAME> --slot 1 --force --no-patches -y
Then select the PLUG-OUT on the panel.

Panel: OSC1 COLOR = TIMBRE (detune of the swarm), OSC2 COLOR = COLOR (high-pass, open at 0), OSC1 RANGE =
octave, OSC1 MIX = level; the OSC2 controls and both wave selectors do nothing. Everything after the oscillator
is C-S1SY13's.

Options (environment variables):
  KERNEL=DIR    a voice compiled from C / C++ by s1cc (sdk/s1cc/s1cc build ... -o DIR) instead of the oscillator:
                its stage A on DSP A, stage B first in the voice of DSP B, its S1_PARAM tables on their panel ids
                (docs/S1CC.md §12, e.g. KERNEL=sdk/s1cc/build/braids_saw_swarm_s1 NAME=MUTABLE-S1CC)
  SWARM=0|1     1 (default without KERNEL): SAW_SWARM written in Python (sdk/mport); 0: C-S1SY's DCO1 + DCO2
  WS_DEGREE=n   degree of the odd polynomial that replaces Braids' ws_moderate_overdrive table (default 9:
                21 LSB worst case; 7: 97 LSB, 2 instructions per voice less)
  STEP=n        C-S1SY step the example builds on (default 13; examples/system_1_synth/build.sh --help)
  NAME=...      plug-out name shown on the device and plugins/ folder (default MUTABLE-SYSTEM); max 16 characters
  GMAX=x, RESCOMP=1, RES_MAX=x, VOICES=n, MODEL_ID=0x.., PARAMLOG=1, PAD=n, PACK=0|1, SCHED=0|1, FUSE=0|1
                as in examples/system_1_synth/build.sh (SWARM schedules with the proven op groups only,
                asm/proven_forms.txt, like FOLD and step 14)

Examples:
  sh examples/mutable_system/build.sh                                 (-> plugins/MUTABLE-SYSTEM)
  python3 examples/mutable_system/make_mutable.py --sim --pack-check  (DSP programs + emulator checks only)
  python3 sdk/mport/cli.py check saw_swarm                            (the port against Braids' C++)
```

### `examples/mutable_system/make_mutable.py`

```
MUTABLE-SYSTEM: C-S1SY13F (examples/system_1_synth) with Braids' SAW_SWARM in place of DCO1 + DCO2, ported with
the sdk/mport toolkit (docs/examples_mutable_system_README.md, docs/MUTABLE_SDK.md). A copy of make_synth.py: the
steps below are the history of C-S1SY, kept as they were; SWARM=1 (default) is what this example adds:

    slave A  per voice: LFO (step 11) -> pitch -> increment -> 7 saws + waveshaper (sdk component SawSwarm.osc)
             x OSC1 level x tremolo -> A151+v; the increment -> A188+v for DSP B
    slave B  per voice: SAW_SWARM high-pass (SawSwarm.hp_f / hp, COLOR) on A151+v -> amp ADSR, filter ADSR,
             4-pole low-pass (unchanged)
    knobs    OSC1 COLOR (0x168) = Braids TIMBRE (detune) -> A C070..C075 (6 increment ratios);
             OSC2 COLOR (0x16D) = Braids COLOR (high-pass) -> B C060 (H); OSC1 range, OSC1 level as before

The original header of make_synth.py follows.

C-S1SY: an 8-voice subtractive synth for the SYSTEM-1, all our own microcode (docs/examples_system_1_synth_README.md).

Extends C-S1V8 (examples/system_1): per voice two independent oscillators (wave + level each, DCO2 detuned), an
ADSR VCA, a 4-pole low-pass filter (cutoff, resonance, key track) and a second ADSR for the cutoff.

    slave A  s1m0 @0x0100  per voice: DCO1 + DCO2 (saw / pulse / triangle mix from the ARM) -> A068+v,
                           filter ADSR -> A070+v
    slave B  s1m1 @0x0100  first: sum of last sample's filter outputs x C000 -> 6006 / 6007 (6008..600D = 0);
                           per voice: amp ADSR, x = A068+v x amp env x comp, cutoff (base + amount x A070+v +
                           key track x pitch) -> g = 2 pi f / fs, ladder of 4 one-pole stages with feedback k
    master B s1mm1 @0x1000 6006 / 6007 x volume (C01C) -> A118 / A119 (unchanged from C-S1V4)

Built step by step on the hardware (STEP=n, STEPS below; default 7 = everything up to the waves, packed as Roland
does: pack()). The design above (STEP=0, STAGE=1..4) is the first all-at-once attempt: silent on the device, kept
as reference. The filter ADSR is still to come.

    python3 make_synth.py           -> s1*.s / s1*.hex here, build/s1_programs.h
    python3 make_synth.py --sim     -> also simulate notes through slave A, slave B and master B (emulator C core,
                                       seconds; SIM=py: the Python reference, minutes)
    python3 make_synth.py --pack-check -> packed / fused programs against the unpacked ones (C only, seconds)
    python3 make_synth.py --sim-check -> also the C core against the Python reference, sample by sample (≈ 25 s;
                                         only when the emulator, simulate() or the instruction forms change)
```

Environment: `CHDIAG`, `CHFRAC` (default 'mod2'), `ENS` (default '2'), `FOLD` (default '0'), `FOLD_MAX` (default '5.0'), `FUSE` (default '1' if STEP >= 11 else '0'), `GMAX` (default '0.7'), `KERNEL` (default ''), `M1` (default ''), `PACK` (default '1'), `PAD` (default '0'), `RESCOMP` (default '0'), `RES_MAX` (default '4.4' if RESCOMP else '3.6'), `SCHED` (default '1' if STEP >= 12 else '0'), `SIM`, `SIM_CHECK_N` (default '600'), `SIM_JOBS` (default '0'), `SIM_N` (default '9600'), `SIM_PACK_N` (default '19200'), `STAGE` (default '4'), `STEP` (default '13'), `SWARM` (default '0' if KERNEL else '1'), `VOICES` (default '8'), `WS_DEGREE` (default '9')

### `examples/mutable_system/paramlog.py`

```
Listen to a PARAMLOG build and list the parameters the device hands to the PLUG-OUT model (docs/examples_system_1_README.md §6).

The PARAMLOG build of C-S1V4 (PARAMLOG=1 build.sh) sends one SysEx per call of the model's vt+0x30:
    F0 7D 06 <id >> 7> <id & 7F> <flag> <value: bits 31-28, 27-21, 20-14, 13-7, 6-0> F7
Everything else the device sends (CC echoes of the panel, ...) is shown too, interleaved, so a knob's id can be
matched with its MIDI CC.

    python3 examples/system_1/paramlog.py                 listen 60 s on every SYSTEM-1 port, then summarize
    python3 examples/system_1/paramlog.py -t 120 --raw    also print every message as it arrives
    python3 examples/system_1/paramlog.py --file LOG...   decode saved `amidi -d` output instead
    python3 examples/system_1/paramlog.py --device SYSTEM-8   the same on a SYSTEM-8 (a build that sends F0 7D 06)

Uses `amidi -d` (alsa-utils). Do not run it while another tool talks to the device.
```

| option | help | default |
|---|---|---|
| `-t, --time` | seconds to listen (default 60) | `60` |
| `--raw` | print every message too |  |
| `--file` | decode these amidi -d logs instead of listening |  |
| `--device` | port name to listen on (SYSTEM-8 for the SYSTEM-8) | `SYSTEM-1` |

## SYSTEM-S1CC: only an s1cc kernel (SYSTEM-1) (`examples/system_s1cc/`)

### `examples/system_s1cc/build.sh`

```
usage: KERNEL=DIR [VAR=value ...] sh examples/system_s1cc/build.sh [--help]

Builds SYSTEM-S1CC for the SYSTEM-1 -> plugins/<NAME>/Script/Code1.Dat (and Code.Dat, the same file): the voice is
an s1cc kernel and nothing else (S1_DSP_A on the slave's DSP A, S1_DSP_B on its DSP B); the master only applies
VOLUME and sends the mix through the OS delay / reverb (docs/examples_system_s1cc_README.md).
Install: python3 tools/plugout_tool.py install plugins/<NAME> --slot 1 --force --no-patches -y
Then select the PLUG-OUT on the panel.

Panel: the kernel's S1_PARAM tables on their panel ids (sdk/s1cc/include/s1panel.h; row 128 until the control
moves); VOLUME, DELAY and REVERB as the OS does them; every other control does nothing.

Options (environment variables):
  KERNEL=DIR    required: an s1cc build directory (sdk/s1cc/s1cc build VOICE.c ... -o DIR)
  VOICES=n      1..8 voices (default 8); fewer voices leave more instructions for each
  BUDGET=n      instructions per sample a slave DSP may have (default 708, the most tried on the device; 826 hung it)
  NAME=...      plug-out name shown on the device and plugins/ folder (default SYSTEM-S1CC); max 16 characters
  MODEL_ID=0x.. (default 0x0010), PARAMLOG=1 (model.c sends the parameters to the PC)

Examples:
  sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_env.c
  KERNEL=sdk/s1cc/build/simple_saw_env sh examples/system_s1cc/build.sh          (-> plugins/SYSTEM-S1CC)
  KERNEL=sdk/s1cc/build/simple_saw_env python3 examples/system_s1cc/make_s1cc.py --sim   (programs + emulator)
```

### `examples/system_s1cc/make_s1cc.py`

```
SYSTEM-S1CC: a SYSTEM-1 PLUG-OUT that is only an s1cc kernel (docs/examples_system_s1cc_README.md).

No oscillator, envelope, filter or LFO of its own: everything a voice does is the kernel's C (sdk/s1cc). Around it
only what the device needs, in the forms MUTABLE-SYSTEM / C-S1SY13 proved on the hardware:

    slave A  s1m0 @0x0100   per voice v: stage A of the kernel (S1_DSP_A); pitch / gate / trigger read from bank 5
                            (A030+v / A038+v / A060+v, written by engine.c); its output -> A151+v
    slave B  s1m1 @0x0100   first the mix: sum of last sample's voice outputs x gain x C000 -> 6006 / 6007
                            (6008..600D = 0); then per voice v: stage B of the kernel (S1_DSP_B) on x = A151+v ->
                            its output word; without S1_DSP_B the mix reads A151+v
    master B s1mm1 @0x1000  6006 / 6007 x VOLUME (C01C) -> OS effect send 2000 / 2002 (delay, reverb); the effect's
                            return 2001 / 2003 -> A118 / A119 (C-S1SY13's master, unchanged)
    knobs                   the kernel's S1_PARAM tables, by panel id: DSP A parameters from C070, DSP B from C060

    KERNEL=dir (required)   an s1cc build directory (sdk/s1cc/s1cc build ... -o dir)
    VOICES=n                1..8 (default 8): fewer voices, more instructions for each
    BUDGET=n                instructions per sample a slave DSP may have (default 708, the most tried on the device;
                            826 hung it: docs/examples_system_1_synth_README.md §4); above it the build stops
    OUTDIR=dir              where the programs and build/s1_programs.h go (default: this folder)

    python3 make_s1cc.py           -> s1*.s / s1*.hex here, build/s1_programs.h
    python3 make_s1cc.py --sim     -> also the whole chain on the emulator (C core) against the kernel's native
                                      reference (sdk/s1cc refdsp), and the silence without notes
```

Environment: `BUDGET` (default '708'), `KERNEL` (default ''), `OUTDIR` (default HERE), `SIM_N` (default '9600'), `VOICES` (default '8')
