# `examples/system_s1cc`: SYSTEM-S1CC, un PLUG-OUT que es solo el kernel de s1cc

SYSTEM-S1CC es un PLUG-OUT del SYSTEM-1 cuya voz es **solo** un kernel compilado por s1cc ([S1CC.md](S1CC.md)).
No tiene oscilador, envolvente, filtro ni LFO propios: todo lo que hace una voz está en el C del kernel. Alrededor
solo hay lo que el equipo necesita, con las formas que ya funcionaron en MUTABLE-SYSTEM / C-S1SY13:
* la carga de los programas;
* las notas;
* las tablas de perillas;
* la mezcla de las voces;
* VOLUME y el envío a delay / reverb del OS.

Comparado con MUTABLE-SYSTEM ([examples_mutable_system_README.md](examples_mutable_system_README.md)), deja libres
las instrucciones del anfitrión C-S1SY13 (LFO, ADSR, filtro, mezcla): unas 234 del DSP A y 412 del DSP B.

## 1. Uso

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_env.c --check
KERNEL=sdk/s1cc/build/simple_saw_env sh examples/system_s1cc/build.sh               # -> plugins/SYSTEM-S1CC
KERNEL=sdk/s1cc/build/simple_saw_env python3 examples/system_s1cc/make_s1cc.py --sim   # programas + emulador
python3 tools/plugout_tool.py install plugins/SYSTEM-S1CC --slot 1 --force --no-patches -y
```

| variable | qué hace |
|---|---|
| `KERNEL=dir` | obligatoria: la carpeta de salida de `s1cc build` |
| `VOICES=n` | 1…8 voces (8 por defecto). Menos voces dejan más instrucciones a cada una |
| `BUDGET=n` | instrucciones por muestra permitidas en cada DSP del esclavo: 708 por defecto, el máximo probado en el equipo; con 826 se colgó ([examples_system_1_synth_README.md](examples_system_1_synth_README.md) §4). Por encima, el build se detiene |
| `NAME=…` | nombre del PLUG-OUT (`SYSTEM-S1CC`) |
| `MODEL_ID`, `PARAMLOG=1` | como en MUTABLE-SYSTEM |
| `OUTDIR=dir` | solo `make_s1cc.py`: dónde escribir los programas (los tests usan una carpeta temporal) |

## 2. Qué corre en cada DSP

```
esclavo A (s1m0 @0x0100)   por voz v: S1_DSP_A                        → A151+v
esclavo B (s1m1 @0x0100)   mezcla: Σ salidas × 2/(1,5·NV) × C000       → 6006 / 6007 (6008…600D = 0)
                           por voz v (si hay S1_DSP_B): S1_DSP_B(x = A151+v) → su palabra de salida
maestro B (s1mm1 @0x1000)  6006 / 6007 × VOLUME (C01C)                 → envío al delay / reverb del OS (2000 / 2002)
                           retorno del efecto (2001 / 2003)            → A118 / A119
```

* **Entradas de la voz.** `s1_pitch()`, `s1_gate()` y `s1_trig()` se leen directo del banco 5: `A030+v`, `A038+v`
  y `A060+v`, que escribe `engine.c`. A diferencia de MUTABLE-SYSTEM, **no** hay LFO ni RANGE: el tono es el de
  la nota.
* **Mezcla.** Con `S1_DSP_B`, el DSP B suma las salidas de B de la muestra anterior (palabras de anillo, S+1). Sin
  él, suma `A151+v`. La ganancia 2/(1,5·NV) es la de C-S1SY13; `C000` es el nivel que deja el OS.
* **Maestro.** Es el de C-S1SY13 STEP 13, sin cambios.
* **Memoria de estado.** Cada voz tiene un bloque en el banco 0 de su DSP, del tamaño que pide el kernel: estado +
  temporales + bandera de primera muestra. Esclavo A desde `0x0300`, esclavo B desde `0x0600`, salidas de B en
  `0x0400`+: las zonas que usaron nuestros programas probados en el equipo (C-S1V8, C-S1SY, MUTABLE-SYSTEM). No hay
  tope de 16 palabras como en MUTABLE-SYSTEM; el límite es que las `NV` voces entren hasta `0x07FF`. Lo de abajo de
  `0x0018` es del OS ([../system-1/DSP_BANKS.md](../system-1/DSP_BANKS.md)).
* **Parámetros.** Las tablas `S1_PARAM` van a `C070`… del DSP A y `C060`… del DSP B, por `panel_id`. El ARM
  escribe la fila 128 al cargar y la de la posición de la perilla cada vez que se mueve, y las vuelve a escribir
  en cada nota.
* **Planificación y reglas.** El planificador (`asm/dspsched.py`) solo arma:
  * grupos de operaciones probados en el equipo (`asm/proven_forms.txt`);
  * un acceso al banco 5 por instrucción (regla 5, `Ins.one_bank5`);
  * formas de instrucción (accesos por banco y cantidad de operaciones) que son parte de una forma de un programa
    nuestro que corrió en el equipo (C-S1V8, C-S1SY13; `Ins.shapes`).

  Después, `asm/s1rules.py` verifica cada programa contra todas las reglas del SYSTEM-1 (R2–R8, §7) y una regla rota
  detiene el build. `check_optimized.py` avisa fusiones posibles; se dejan por seguridad.

## 3. Qué tiene que hacer el kernel

Lo que en MUTABLE-SYSTEM ponía el anfitrión, aquí lo pone el C:

| tema | en MUTABLE-SYSTEM | en SYSTEM-S1CC |
|---|---|---|
| puerta / VCA | ADSR del anfitrión | **el kernel**: `s1_gate()` o una envolvente propia. Sin eso la voz suena siempre (`--sim` lo avisa) |
| filtro | paso bajo del anfitrión, después de `S1_DSP_B` | solo el del kernel, si lo tiene |
| LFO, RANGE | en el tono de `s1_pitch()` | no hay |
| nivel | OSC1 MIX | el kernel (una tabla `S1_PARAM`) |
| controles del panel | OSC1 / OSC2 COLOR libres | todos libres para `S1_PARAM`, salvo VOLUME, DELAY y REVERB, que son del OS / maestro |

Ejemplo: `sdk/s1cc/examples/simple_saw_wrapped/simple_saw_env.c`. Son dos sierras de la librería `saw.c`, con una
envolvente de ataque / release de un polo:

| control | tabla | qué hace |
|---|---|---|
| OSC1 COLOR | `level` | nivel |
| OSC2 COLOR | `detune` | desafinación de la segunda sierra, 0…+50 cents |
| AMP ATTACK (`0x187`) | `attack` | ataque, 1 ms…2 s |
| AMP RELEASE (`0x18A`) | `release` | release, 1 ms…2 s |

## 4. Presupuesto

Instrucciones por muestra del DSP A ≈ instrucciones del kernel A × `NV` (el empaquetado ahorra un poco). El DSP B
tiene la mezcla (12–16) más el kernel B × `NV`.

| kernel | voces | DSP A | DSP B | |
|---|---|---|---|---|
| `simple_saw_env` (43 por voz) | 8 | 337 | 16 | entra |
| `simple_saw_env` | 4 | 169 | 12 | entra |
| `plaits_variable_shape` (311 por voz) | 8 | 2481 | — | rechazado (> 708) |
| `plaits_variable_shape` | 2 | entra | — | pero suena sin notas: su wrapper no usa la puerta (§3) |

## 5. Verificación [sim, 2026-10-27]

`make_s1cc.py --sim` corre la cadena entera en el núcleo C del emulador: esclavo A, esclavo B y maestro, con el
banco 5 compartido entre A y B. La compara con la referencia nativa del kernel (`refdsp` de s1cc) × la ganancia de
la mezcla, con la latencia que mejor coincide.

`simple_saw_env`, 8 voces:

| caso | resultado |
|---|---|
| nota 45, voz 0 | latencia 1 muestra; SNR 97,4 dB (10 ms), 74,1 dB (todo) |
| nota 69, voz 0 | SNR 112,1 dB (10 ms), 23,9 dB (todo: la deriva de fase float32 / DSP, S1CC.md §11) |
| 8 notas | pico 0,217 |
| sin notas | pico 0,000000 |

`sdk/s1cc/tests/test_s1cc.py` (`TestSystemS1cc`) comprueba la cadena con 8 y con 2 voces (SNR 10 ms > 90 dB,
silencio sin notas) y que el build se detenga por encima del presupuesto.

**En el equipo [medido, 2026-10-27]: funciona** (`simple_saw_env`, 8 voces; tercera instalación, con el header
corregido, §7). Para repetir la prueba:
* instalar con `simple_saw_env` (43 instrucciones por voz, lejos del límite);
* elegir el PLUG-OUT y tocar sin mover nada: AMP ATTACK y RELEASE valen 128 (≈ 45 ms) hasta que se muevan;
* después, OSC1 / OSC2 COLOR, ATTACK, RELEASE y VOLUME.

## 6. Archivos

| archivo | qué es |
|---|---|
| `make_s1cc.py` | generador de los programas DSP y de `build/s1_programs.h` (programas, `pitch_bits`, `volume_bits`, tablas del kernel); `--sim` |
| `engine.c` | región 1, ARM del esclavo: el de MUTABLE-SYSTEM sin el panel de C-S1SY. Carga los programas; notas → banco 5; perillas → tablas del kernel |
| `model.c` | región 0, ARM del maestro: el de MUTABLE-SYSTEM (asignador de `NV` voces, carga `s1mm1`, VOLUME, que reenvía en cada nota) |
| `syninfo.h`, `link.ld` | copias de MUTABLE-SYSTEM |
| `build.sh` | build completo → `plugins/<NAME>/Script/Code1.Dat` |
| `s1m0.s`, `s1m1.s`, `s1mm1.s`, `s1c0.s`, `s1c1.s` (+ `.hex`) | los programas generados del último build |

## 7. Reglas del SYSTEM-1 en la herramienta (`asm/s1rules.py`)

Romper una de estas reglas colgó el equipo en pruebas anteriores
([examples_system_1_jx_README.md](examples_system_1_jx_README.md) §5). `make_s1cc.py` las verifica en cada build, y
`make_mutable.py` con `KERNEL=` o `SWARM=1`. También se pueden correr a mano: `python3 asm/s1rules.py FILE.s ...`.

| regla | qué limita, por instrucción | origen |
|---|---|---|
| R2 | banco 5: ≤ 1 carga y ≤ 1 store | medido (C-S1JXQ/R) |
| R3 | banco 0: ≤ 2 cargas y ≤ 2 stores | medido (C-S1JXH) |
| R4 | banco 6: ≤ 2 cargas | medido (C-S1JXH1) |
| R5 | banco 5: un solo acceso (carga o store) | inferido |
| R6 | ≤ 2 cargas de los bancos 5 + 6 + IOMD | inferido |
| R7 | un DSP del esclavo solo guarda en `A100`–`A1FF` del banco 5 | Roland nunca guarda en otra zona |
| R8 | operaciones juntas: solo grupos de `asm/proven_forms.txt` | medido (cuelgues de pasos de C-S1SY) |
| aviso | una forma de instrucción que no es parte de ninguna de C-S1V8 / C-S1SY13 | permitida por las reglas, nunca corrió |

**2026-10-27, causa de los cuelgues de SYSTEM-S1CC (encontrada).** Los dos primeros builds se colgaban al seleccionar
el PLUG-OUT (la instalación terminaba bien). `make_s1cc.py` escribía los programas en `s1_programs.h` como
`uint16_t` en orden de programa. El cargador del OS lee palabras de 32 bits con la primera media palabra en la mitad
**alta** (`(p[i] << 16) | p[i + 1]`, como `make_mutable.py c_array`). En el ARM little-endian, cada par llegaba
invertido al DSP: un programa sin sentido. El emulador no lo podía ver, porque simula las palabras del programa, no
el header que compila el ARM. Ahora `c_array` es el de MUTABLE, y hay dos controles:
* `TestSystemS1cc.test_header_layout` comprueba que cada programa del header empiece con `0x0a02ff71`;
* `build.sh`, después de compilar el ARM, busca cada programa (`.hex`) en el binario de su región (`region1`:
  esclavo; `region0`: maestro) con el formato que lee el cargador. Si falta alguno, el build se detiene antes de
  empaquetar; si lo encuentra con los pares invertidos, lo dice. Es la comparación entre lo que simula el emulador y
  lo que recibe el equipo.

**Antes de encontrar la causa** (mismo día) se revisaron y endurecieron las reglas de acceso. Lo que sigue queda como
registro.

**2026-10-27, primera instalación de SYSTEM-S1CC: el equipo se colgó.** El build de ese momento:
* rompía R5 en 7 instrucciones (`st r0, [a151] | ld r0, [a031]`: la salida de una voz junto al tono de la
  siguiente);
* tenía 2 formas nuevas (avisos);
* guardaba los bloques de voz desde `0x0100` del banco 0, una zona que nuestros programas no usaban en el
  esclavo A.

Ninguna regla se verificaba en la herramienta. Desde entonces el build cumple R2–R8, no tiene formas nuevas y usa
las zonas de memoria de los programas probados. La causa no está confirmada: R5 es inferida. Ese mismo día,
`make_mutable.py` con `KERNEL=` también rompía R5 (en el código del anfitrión, `ld [a040] | st [a169]`) y R8 (sin
lista blanca). Ahora usa la lista blanca y corre `s1rules`.
