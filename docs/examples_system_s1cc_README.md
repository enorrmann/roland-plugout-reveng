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
sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_env.c --regs 0,1,2,3,4,5,6,7 --check
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
| `ENV_TMIN=s`, `ENV_TMAX=s` | tiempos de las envolventes del anfitrión con el fader en 0 y en 255 (por defecto 0,001 y 10 s, exponencial entre medio, como C-S1SY). Solo si el kernel usa `s1_env` |
| `OUTDIR=dir` | solo `make_s1cc.py`: dónde escribir los programas (los tests usan una carpeta temporal) |

### 1.1 Lo decide el kernel

No hay opciones para cada función: `make_s1cc.py` lee el kernel (`voice.s1k`) y arma solo lo que este usa.

| el kernel tiene | `make_s1cc.py` agrega |
|---|---|
| `S1_DSP_B` | su código en el DSP B, por voz; sin él, el DSP B solo mezcla |
| `s1_env(0)` y / o `s1_env(1)` | una ADSR en ensamblador por cada envolvente que lee, en el DSP B, y la copia de puerta y disparo en el DSP A (§3.1); en `engine.c`, las perillas AMP / FILTER A, D, S, R |
| `s1_export(i, …)` | la palabra del banco 5 de la ranura `i` de cada voz (§2) |
| `S1_PARAM` | las tablas, escritas por el ARM en el `panel_id` de cada una |
| estado, derrames, primera muestra | el bloque de banco 0 de cada voz, del tamaño justo |

Por eso los tres PLUG-OUT de ejemplo son el mismo generador con otro `KERNEL` (y otro `NAME`, que solo cambia el
nombre del equipo y la carpeta de `plugins/`):

| PLUG-OUT | `KERNEL` (fuente en `sdk/s1cc/examples/simple_saw_wrapped/`) | qué arma por eso |
|---|---|---|
| `SYSTEM-S1CC` | `simple_saw_env.c`: ADSR en C en `S1_DSP_B` | sierras en A, la ADSR del kernel en B |
| `S1CC-HOSTENV` | `simple_saw_hostenv.c`: `s1_env(0)`, `s1_env(1)` | más las dos ADSR del anfitrión en B y sus perillas |
| `S1CC-EXPORT` | `simple_saw_export.c`: `s1_export(0, …)` | más la ranura 0 en `A188+v` |

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_hostenv.c --regs 0,1,2,3,4,5,6,7
KERNEL=sdk/s1cc/build/simple_saw_hostenv NAME=S1CC-HOSTENV sh examples/system_s1cc/build.sh   # -> plugins/S1CC-HOSTENV
```

Los tres usan el slot 1 y el mismo id de modelo: instalar uno reemplaza al anterior.

## 2. Qué corre en cada DSP

```
esclavo A (s1m0 @0x0100)   por voz v: S1_DSP_A                        → A151+v
esclavo B (s1m1 @0x0100)   mezcla: Σ salidas × 2/(1,5·NV) × C000       → 6006 / 6007 (6008…600D = 0)
                           por voz v (si hay S1_DSP_B): S1_DSP_B(x = A151+v) → su palabra de salida
maestro B (s1mm1 @0x1000)  6006 / 6007 × VOLUME (C01C)                 → envío al delay / reverb del OS (2000 / 2002)
                           retorno del efecto (2001 / 2003)            → A118 / A119
```

* **Entradas de la voz.** `s1_pitch()`, `s1_gate()` y `s1_trig()` se leen directo del banco 5: `A030+v`, `A038+v`
  y `A060+v`, que escribe `engine.c`. `s1_dt()` (f/96000) y `s1_rdt()` (96000/f/16384) en `A040+v` y `A048+v`: el
  ARM los escribe con el pitch desde tablas por nota (`dt_bits`, `rdt_bits`; 2026-10-28, sin probar en el equipo). A diferencia de MUTABLE-SYSTEM, **no** hay LFO ni RANGE: el tono es el de
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

Ejemplo: `sdk/s1cc/examples/simple_saw_wrapped/simple_saw_env.c`. Son dos sierras (librería `saw.c`) y una ADSR
(librería `env.c`), las dos en C puro; el envoltorio las adapta a la voz y mapea el panel como C-S1SY:

| control | tabla | qué hace |
|---|---|---|
| OSC1 MIX (`0x174`) | `mix1` | nivel de la primera sierra |
| OSC2 MIX (`0x175`) | `mix2` | nivel de la segunda sierra |
| OSC2 FINE (`0x170`) | `fine2` | segunda sierra ±1 semitono (128 = afinada) |
| AMP ATTACK / DECAY / RELEASE (`0x187`, `0x188`, `0x18A`) | `attack`, `decay`, `release` | tiempos 1 ms…2 s |
| AMP SUSTAIN (`0x189`) | `sustain` | nivel 0…1 |

Hasta que se mueven, todos valen 128: las dos sierras a 0,5, afinadas, ataque / decay / release ≈ 45 ms, sustain 0,5.

### 3.1 Envolventes del anfitrión (`s1_env`)

Si el kernel llama a `s1_env(0)` o `s1_env(1)` en `S1_DSP_B`, `make_s1cc.py` agrega por voz:
* **DSP A:** la copia de la puerta y del disparo a `A160+v` / `A168+v`. El DSP B las lee de ahí: en C-S1SY, leer lo
  que escribe el ARM daba silencio.
* **DSP B, antes del kernel:** una ADSR en ensamblador por cada envolvente que el kernel lee (`s1_env(0)`,
  `s1_env(1)` o las dos). Es `step_adsr` de C-S1SY, copiada sin cambios, que corre en el equipo desde el paso 4 de
  C-S1SY. Cada una deja su nivel 0…1 en una palabra del bloque de la voz, y el kernel la lee como `s1_env(i)`. Las
  envolventes no necesitan `s1_export`: ver [examples_simple_saw_wrapped_README.md](examples_simple_saw_wrapped_README.md) §7.2.
* **ARM (`engine.c`):** las perillas → parámetros del DSP B. Envolvente 0: AMP S / A / D / R → `C040`–`C043`;
  envolvente 1: FILTER S / A / D / R → `C044`–`C047`. Las tablas las genera `make_s1cc.py` con `ENV_TMIN` /
  `ENV_TMAX`. Valores por defecto hasta que se mueve una perilla: AMP 255 / 0 / 128 / 60, FILTER 0 / 0 / 128 / 60.

La ADSR sigue a la puerta misma (ataque = puerta − latch; el latch se borra con cada disparo), así que no le afecta
que el disparo y la puerta lleguen separados. Costo: unas 17 instrucciones por envolvente y voz en el DSP B, una vez
empaquetadas (8 voces, 2 envolventes y el kernel de `simple_saw_hostenv`, con su paso bajo: 471). Ejemplo y detalle de quién escribe
qué: [examples_simple_saw_wrapped_README.md](examples_simple_saw_wrapped_README.md) §7.

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_hostenv.c --regs 0,1,2,3,4,5,6,7 --check
KERNEL=sdk/s1cc/build/simple_saw_hostenv NAME=S1CC-HOSTENV sh examples/system_s1cc/build.sh
```

En la referencia nativa y en `s1cc check`, `s1_env()` es la puerta. `--sim` verifica las envolventes reales por su
forma (ataque, decay hacia el sustain, release) y no compara muestra a muestra.

## 4. Presupuesto

Instrucciones por muestra del DSP A ≈ instrucciones del kernel A × `NV` (el empaquetado ahorra un poco). El DSP B
tiene la mezcla (12–16) más el kernel B × `NV`.

| kernel | voces | DSP A | DSP B | |
|---|---|---|---|---|
| `simple_saw_env`: sierras en A (29 por voz), ADSR en B (46 por voz), 8 registros | 8 | 225 | 364 | entra, con margen |
| la misma voz con todo en `S1_DSP_A` (100 por voz) | 8 | 801 | 16 | rechazado (> 708) |
| idem | 6 | 601 | 14 | entra |
| `simple_saw_env` de antes (solo ataque / release, 44 por voz) | 8 | 345 | 16 | entra (el primero probado en el equipo) |
| `plaits_variable_shape` (311 por voz) | 8 | 2481 | — | rechazado |
| `plaits_variable_shape` | 2 | entra | — | pero suena sin notas: su wrapper no usa la puerta (§3) |

**Usar los dos DSP.** Cada voz pasa por el DSP A y después por el B, y cada uno tiene su propio presupuesto de
≈ 708 instrucciones. Repartir el trabajo (osciladores en `S1_DSP_A`, envolventes / filtros en `S1_DSP_B`) dobla lo
que entra. El costo extra es pasar a B lo que necesita de A: las entradas (`s1_gate()`, `s1_trig()`: una carga y un
store en A cada una) y `s1_export` / `s1_import`.

**Todos los registros.** `s1cc build` usa por defecto `--regs 0,1,2,3,4,7`, porque en MUTABLE-SYSTEM `r5` / `r6`
son del anfitrión. SYSTEM-S1CC no tiene anfitrión: compilar el kernel con `--regs 0,1,2,3,4,5,6,7` deja más valores
en registros y menos derrames ([S1CC.md](S1CC.md) §1.1). `make_s1cc.py` avisa si un kernel tiene derrames y no usa
`r5` / `r6`.

**Valores que cruzan de A a B.** `make_s1cc.py` informa, para `x`, las entradas copiadas, cada ranura de export y las
copias de puerta y disparo, si en el equipo llegan en la misma muestra, una más tarde o en carrera. Los dos DSP del chip
corren a la vez ([examples_simple_saw_wrapped_README.md](examples_simple_saw_wrapped_README.md) §8). Mapa de la zona
del banco 5 que escriben los DSP: `A151+v` `x`, `A160+v` / `A168+v` copias de puerta y disparo, `A188`–`A19F` ranuras
de export 0…2, `A1A0`–`A1B7` entradas para B, `A1B8`–`A1FF` ranuras 3…11.

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
