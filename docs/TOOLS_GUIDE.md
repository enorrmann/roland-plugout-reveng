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
python3 -m unittest discover -s sdk/s1cc/tests                     # ≈ 8 s
```

