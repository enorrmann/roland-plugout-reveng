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
| (voces) | no es una opción: `build.sh` prueba 8, 7, … y se queda con la mayor cantidad que entra en los límites (instrucciones por muestra y los 6 KiB del principal), y la anuncia al final (`SYSTEM-S1CC: 6 voices`) |
| `BUDGET_A=n`, `BUDGET_B=n`, `MAIN_BYTES=n` | los límites medidos: 703 instrucciones por muestra en el DSP A (826 se colgó), 708 en el DSP B, y 6144 bytes de cada programa principal, su región `0x0100`–`0x18FF` ([DSP_PROGRAM_MEMORY.md](DSP_PROGRAM_MEMORY.md) §3.1: 5854 bytes corrieron, 6534 y 6740 colgaron el equipo, 2026-10-28). Sólo para probar más, a riesgo propio |
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
| el tono (`s1_pitch`, `s1_increment`, `s1_dt`, `s1_rdt`), `s1_lfo_filter()`, `s1_lfo_amp()` | el LFO del anfitrión por voz en el DSP A y lo que modula; en `engine.c`, LFO SHAPE / RATE / DELAY TIME / TO PITCH / TO FILTER / TO AMP (§3.3). `LFO_PITCH=0`: sin LFO → pitch |
| `s1_color(0)` y / o `s1_color(1)` | además, envolvente de pitch y sub oscilador por voz y la mezcla de cada color; en el DSP B, las dos ADSR del anfitrión; en `engine.c`, PITCH ENV A / D, COLOR y COLOR MOD (§3.2) |
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
  escribe la fila de la posición de la perilla cada vez que llega, y las vuelve a escribir en cada nota. Al cargar,
  el modelo del maestro hace `touch` de los controles `0x160`–`0x18F` del grupo 3 y `flush`: el OS le manda al
  engine sus valores actuales (sin eso quedaban en la fila 128 hasta mover la perilla) [medido 2026-10-28].
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
| LFO, RANGE | en el tono de `s1_pitch()` | el LFO del anfitrión, por voz (§3.3): en el tono de `s1_pitch()` / `s1_dt()` / `s1_rdt()`, y `s1_lfo_filter()` / `s1_lfo_amp()` para el filtro y el VCA del kernel; RANGE no hay |
| nivel | OSC1 MIX | el kernel (una tabla `S1_PARAM`) |
| controles del panel | OSC1 / OSC2 COLOR libres | todos libres para `S1_PARAM`, salvo VOLUME, DELAY y REVERB, que son del OS / maestro; OSC1 / OSC2 COLOR con su COLOR MOD también como `s1_color` (§3.2) |

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

### 3.2 COLOR con su fuente de modulación (`s1_color`) [sim, 2026-10-28; sin probar en el equipo]

`s1_color(i)` en `S1_DSP_A` es OSC1 COLOR (i = 0) u OSC2 COLOR (i = 1), 0…1, con la fuente de su selector COLOR MOD
(`0x169` / `0x16E`), como en el SYSTEM-1: con MANUAL la perilla fija el color; con otra fuente, la perilla es la
profundidad y el color = perilla × fuente (0…1). El kernel elige qué hace con el color (un ancho de pulso, una
ganancia, una relación con `s1_exp2`…); lo que no es un `s1_color` sigue en tablas `S1_PARAM`, sin modulación.

| COLOR MOD | fuente | dónde |
|---|---|---|
| 0 MANUAL | la perilla | — |
| 1 LFO | el LFO del anfitrión de la voz con su DELAY TIME (§3.3), 0,5 + 0,5 · lfo | DSP A |
| 2 PITCH ENV | AD de la voz: PITCH ENV ATTACK / DECAY (`0x17B` / `0x17C`, 1 ms…10 s) | DSP A |
| 3 FILTER ENV | la ADSR 1 del anfitrión (§3.1) | DSP B → `A178+v` → DSP A |
| 4 AMP ENV | la ADSR 0 del anfitrión (§3.1) | DSP B → `A170+v` → DSP A |
| 5 SUB OSC | cuadrada una octava abajo de la nota, 0 / 1 | DSP A |

Si el kernel llama a `s1_color`, `make_s1cc.py` agrega:
* **DSP A, por voz, antes del kernel** (`hlfo_voice`), además del LFO de §3.3:
  * Envolvente de pitch: las cuentas de la ADSR del anfitrión (`adsr()`, probada) con la puerta en 1 y sustain 0.
    El latch (1 cuando el nivel pasó 0,999) se borra con cada disparo; ataque = 1 − latch; el nivel va hacia
    1,25 · ataque con tasa ataque · kA + latch · kD.
  * Sub oscilador: fase −1…1 + `s1_dt()` por muestra, cuadrada por `ph_clamp`.
  * Color i = Σ pesos (`C050 + 8 i`…) × (1, lf, pitch env, filter env, amp env, sub), a una palabra del bloque de la
    voz que el kernel lee como `s1_color(i)`.
* **DSP B:** las dos ADSR del anfitrión aunque el kernel no lea `s1_env`, y su nivel a `A170+v` / `A178+v`. El DSP A
  lo lee una muestra después.
* **ARM (`engine.c`, `cmod_*`):** las perillas → parámetros del DSP A, como las demás tablas. Se escriben al cargar,
  en cada nota y cuando se mueve la perilla.

| parámetro del DSP A | valor |
|---|---|
| `C048` / `C049` | tasas de decay / ataque de la envolvente de pitch (tablas `decay_bits` / `attack_bits`) |
| `C050 + 8 i` … `+5` | pesos del color i: manual, LFO, pitch env, filter env, amp env, sub. Con LFO o SUB OSC (−1…1): manual = ese peso = perilla / 2 |

Por defecto, hasta que se mueve un control: PITCH ENV A 0 / D 128, colores en 128 con MOD = MANUAL (el LFO: §3.3).

**Costo.** Con el LFO de §3.3, unas 66 instrucciones por voz en el DSP A, ya empaquetadas: `mp_csaw` entra con 6
voces (669; con 8, 889). Sin el LFO del anfitrión (antes de §3.3) eran ~43: 8 voces daban 703.

**Verificación.** En el emulador, la palabra de color de la voz 0 con cada fuente (perilla en 255, 0,6 s, la puerta
baja a la mitad):

| fuente | resultado |
|---|---|
| MANUAL | 1,000 constante |
| LFO, las 6 formas (antes de §3.3: sin fade, pesos 0,5) | seno, triángulo y sierra recorren 0…1; la cuadrada salta 0 / 1; S&H escalones del ruido en cada vuelta; S&G los sigue con glide |
| PITCH ENV, A 60 / D 160 | 0,706 a los 25 ms y cae a 0 en ~0,4 s; A 200: sube de forma gradual |
| FILTER ENV (S 128) | 0,82 → 0,50 (sustain), 0 después del release |
| AMP ENV (S 255) | 1,0 mientras dura la nota, 0 después |
| SUB OSC, nota 69 | 0 / 1 con período de 436 muestras (220 Hz) |

Primer intento de la envolvente de pitch: `(1 − e) · 65536` desborda el rango del DSP (±512) y da 0, y la bandera de
ataque nunca subía. El segundo, con una bandera continua que baja con `min`, quedaba en equilibrio en 0,997 y no
decaía. Por eso se usa la forma probada de la ADSR.

### 3.3 El LFO del anfitrión (`s1_lfo_filter`, `s1_lfo_amp`, el tono) [sim, 2026-10-28; sin probar en el equipo]

Un LFO por voz en el DSP A, antes del kernel (`hlfo_voice`), con los controles LFO del panel. Es `step_lfo_voice` de
MUTABLE-SYSTEM paso 11 (probado en el equipo), con el fade y sus tres destinos:

| control | panel | qué hace |
|---|---|---|
| LFO SHAPE | `0x160` | seno, triángulo, sierra, cuadrada, S&G, S&H (pesos `C042`–`C047`: 1 en la elegida) |
| LFO RATE | `0x162` | 0,05…40 Hz; la fase vuelve a 0 en cada comienzo de nota; S&H toma el ruido global en cada vuelta, S&G lo sigue (constante = período / 8) |
| DELAY TIME | `0x161` | fade f: 0 → 1 desde la puerta en 0…5 s; lf = f · lfo |
| LFO TO PITCH | `0x163` | bipolar (128 = 0), y = b·\|b\| octavas (±1): el anfitrión escribe pitch + y, `dt` · 2^y y `rdt` · 2^−y en palabras del bloque de la voz, y el kernel las lee como `s1_pitch()` / `s1_increment()`, `s1_dt()`, `s1_rdt()` |
| LFO TO FILTER | `0x164` | bipolar, ±4 octavas: `s1_lfo_filter()` = F · lf, para sumar al corte |
| LFO TO AMP | `0x165` | bipolar, M = ±0,5: `s1_lfo_amp()` = 1 + f · (M · lf − \|M\|), una ganancia 0…1 (trémolo hacia abajo desde el nivel lleno; M < 0, el LFO invertido) |

Las curvas son las de MUTABLE-SYSTEM. Por defecto: triángulo, RATE 128 (1,4 Hz), DELAY 0 y los tres destinos en 128,
es decir sin modulación.

* **Dónde.** El filtro y el VCA son del kernel, así que el anfitrión sólo da los términos: `s1_lfo_filter()` y
  `s1_lfo_amp()` se leen en los dos DSP (el compilador pasa a B por `INB` lo que B lee). `mp_dsp_b.h` de
  mutable_ported los usa. Un kernel que no los lee no los paga.
* **2^y.** Taylor de e^(y·ln2/2) hasta el cuarto grado, al cuadrado: 0,6 cents de error a 0,7 octavas. `dt · rdt`
  queda en 1,0000.
* **Parámetros del DSP A.** `C040` paso de fase, `C041` coeficiente del S&G, `C042`–`C047` formas, `C04A` paso del
  fade, `C04B` octavas a pitch, `C04C` octavas al filtro, `C04D` / `C04E` M y \|M\|. Palabras de banco 0 después de
  los bloques de voz: el ruido del S&H y su contador de Weyl (`step_noise`).
* **Cuándo.** Si el kernel lee el tono (casi todos), `s1_lfo_filter`, `s1_lfo_amp` o `s1_color`. `LFO_PITCH=0`
  deja el tono como lo escribe el ARM (ahorra ~18 instrucciones por voz).
* **Costo.** ~46 instrucciones por voz en el DSP A con pitch, filtro y amp (`mp_saw`: 155 → 521 con 8 voces); el
  DSP B, +4 por voz en `mp_dsp_b.h`. Los kernels de más de ~42 instrucciones por voz en A necesitan menos voces
  (tabla en [examples_mutable_ported_README.md](examples_mutable_ported_README.md) §4).

**Verificación** (emulador, `mp_saw`, palabras que el anfitrión le da al kernel en la voz 0):

| controles | resultado |
|---|---|
| todo en 128 | pitch, `dt`, `rdt` sin cambio; filtro 0; amp 1 |
| TO PITCH 255, triángulo 2,55 Hz | pitch ±1 octava; `dt` × 0,5…2, `rdt` el inverso (`dt · rdt` = 1,0000…1,00005) |
| TO PITCH 0, cuadrada | −1 / +1 octava, en escalón |
| TO FILTER 255, TO AMP 255 | filtro ±4,0 octavas; amp 0,03…0,99 |
| TO AMP 0 | amp con el LFO invertido |
| DELAY TIME 200, TO PITCH 200 | fade en rampa de 3,1 s; la profundidad crece con él |

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

### 4.1 Probar en el equipo grupos de ops que no están en la lista (`--forms`, `TRY_FORMS`)

El planificador sólo pone juntas en una instrucción las ops cuyo grupo está en `asm/proven_forms.txt`: los que
corrieron en el equipo (regla R8 de `asm/s1rules.py`). Los demás quedan en instrucciones separadas, y
`check_optimized` los avisa como `fusable`. Para sumar un grupo a la lista hay que probarlo en el equipo:

```sh
KERNEL=sdk/s1cc/build/mp_csaw VOICES=6 python3 examples/system_s1cc/make_s1cc.py --forms       # 1. candidatos
KERNEL=sdk/s1cc/build/mp_csaw VOICES=6 NAME=MP-CSAW-TRY TRY_FORMS='addxy:imm + mul:imm; mad:imm + mul:reg' \
    sh examples/system_s1cc/build.sh                                                           # 2. armar (o TRY_FORMS=all)
python3 tools/plugout_tool.py install plugins/MP-CSAW-TRY --slot 1 --force --no-patches -y      # 3. instalar y tocar
python3 tools/proven_forms.py promote examples/system_s1cc/build/try_forms.txt --by MP-CSAW-TRY  # 4. si funcionó
```

1. `--forms` lista, por programa del esclavo, los grupos fuera de la lista que el planificador usaría si pudiera,
   cuántas instrucciones ahorra cada uno solo y cuántas veces aparece con todos permitidos.
2. `TRY_FORMS` arma con esos grupos además de los probados. El resto de las reglas (R2–R7) se sigue verificando.
   `examples/system_s1cc/build/try_forms.txt` lista los grupos que el armado usó, y el build lo recuerda al final.
3. Prueba en el equipo: que no se cuelgue y que suene igual que el armado sin `TRY_FORMS` (notas, acordes, las
   perillas). La prueba exacta es una captura USB contra `make_s1cc.py --sim`.
4. `tools/proven_forms.py promote` agrega los grupos nuevos a la lista con la fecha y el PLUG-OUT que los corrió.
   Desde ahí todos los armados los usan. `tools/proven_forms.py list` muestra la lista.

Ejemplo (`mp_csaw`, 6 voces, con el LFO del anfitrión y COLOR MOD): DSP A 669 → 603 y DSP B 462 → 427 con
`TRY_FORMS=all` (14 grupos nuevos, 106 instrucciones). Los dos que más ahorran solos son `addxy:imm + mul:imm`
(12, DSP A) y `mad:imm + mul:reg` (18, DSP B). En el emulador, el armado con `all` da la misma envolvente y
silencio sin notas.

**[medido] 2026-10-28.** Lo que corrió en el equipo, y lo que lo colgó:

| armado (`mp_csaw`) | grupos nuevos | resultado | lista |
|---|---|---|---|
| `MP-CSAW-TRY` (6 voces, secuencial) | 14 | funciona | promovidos (29 → 43) |
| `MP-CSAW-T8` (8 voces, pares, secciones) | 22 (`TRY_FORMS=all`) | **cuelga** | — |
| `MP-CSAW-H2` | 12 de esos (sin dos inmediatos) | **cuelga** | — |
| `MP-CSAW-G2` | 8 de H2: A+A y A+B | funciona | promovidos (43 → 51) |
| `MP-CSAW-H1` | 8 con dos inmediatos (A+A) | funciona | promovidos (51 → 59) |

El cuelgue está en los 4 grupos que quedan de H2: `max + max`, `ph_wrap + ph_wrap`, `ph_clamp + ph_clamp` y
`min + min`, **dos ops de la unidad B en una instrucción** (Roland fusiona A+B y A+A+B). No se separó cuál. Están en
`asm/bad_forms.txt` con `min:imm + min:imm` (B+B, no probado): el planificador no los usa nunca, ni con
`TRY_FORMS=all`. La bisección: partir los grupos sospechosos en mitades, armar cada mitad con `TRY_FORMS`, y separar
primero por familia (dos inmediatos, dos ops B) para que cada prueba diga algo aunque cuelgue.

### 4.2 Voces de a pares (`PAIRS`) [medido 2026-10-28]

El código de una voz es una cadena de dependencias: aunque el DSP ejecuta hasta 3 ops por instrucción, 397 de las 447
instrucciones del DSP A de `mp_saw` tenían una sola. Con pares, cada DSP corre las voces de a dos: la par en r0–r3 y
la impar, el mismo código, en r4–r7, intercaladas. El planificador junta en una instrucción ops de las dos. Es la
forma que sugiere el hardware: en la unidad B el operando X nombra 4 registros y la paridad del opcode elige la mitad
([DSP_HARDWARE.md](DSP_HARDWARE.md) §5.2), y en el SYSTEM-8 cada DSP de oscilador lleva dos voces. Se activa solo
cuando el kernel está en r0–r3 (`PAIRS=0` / `1` lo fuerza); el DSP B, cuando su kernel también lo está:

```sh
sdk/s1cc/s1cc build KERNEL.c --regs-a 0,1,2,3 --regs-b 0,1,2,3 -o sdk/s1cc/build/X     # el kernel en 4 registros
KERNEL=sdk/s1cc/build/X sh examples/system_s1cc/build.sh
```

* **El anfitrión, en 4 registros.** El LFO por voz, COLOR MOD y las ADSR del DSP B usan r0–r3, con los valores que
  duran más (inicio de nota, lf, el sub) en palabras del bloque de la voz. Las ADSR vuelven a cargar el nivel para el
  último paso en lugar de tenerlo en un registro.
* **`ph_wrap rD, r3, rY`.** El `r3` del medio de `ph_wrap` / `ph_tri` / `ph_clamp` es parte del opcode, no un
  registro: con `r7` el DSP hace otra cosa (emulador; el primer armado de pares daba las voces impares mal desde la
  primera muestra). El remapeo a r4–r7 lo deja, y `asm/dspsched.py` ya no lo cuenta como lectura de r3.
* **Verificación.** Con el mismo kernel, el programa de pares da las mismas muestras que el secuencial (diferencia
  0 en 9600 muestras, 8 notas, LFO a tono / filtro / amp, COLOR MOD, notas que se sueltan y vuelven). Con grupos de
  dos inmediatos, 1,3·10⁻⁶ (−118 dB): la precisión de un inmediato al compartir la instrucción [inferido].
* **En el equipo:** `MP-CSAW-P6` (6 voces, pares) y los armados de §4.1 y §4.3 funcionan.

| `mp_csaw`, LFO y COLOR MOD | voces | DSP A | DSP B |
|---|---|---|---|
| secuencial, al empezar el día | 6 | 603 | 427 |
| pares + secciones, 59 grupos probados (`MP-CSAW`) | **8** | 629 (471 + 158 en la sección 2) | 431 |

### 4.3 Secciones [medido 2026-10-28]

Cada programa principal tiene 6 KiB (§1, [DSP_PROGRAM_MEMORY.md](DSP_PROGRAM_MEMORY.md) §3); pasarlos cuelga el equipo.
Cuando el principal de un DSP no entra, `make_s1cc.py` pone las voces (o pares) que no caben en sus secciones, en el
orden en que el DSP las corre después del principal: 2 (`0x1900`), 3 (`0x3100`), 4 (`0x4900`), 5 (`0x6100`). Programas
`s1a2`…`s1a5` (DSP A) y `s1b2`…`s1b5` (DSP B); `engine.c` coloca cada una con
`FC40(índice, set del programa, vacío, vacío, vacío)` y la enciende con `FC44(índice, 1)`, como la sinte de serie
coloca sus programas de voz.

* **Dan espacio, no tiempo:** el principal y sus secciones corren en la misma muestra, en el mismo DSP, y suman al
  mismo presupuesto (`BUDGET_A` / `BUDGET_B` cuentan todo).
* **`FC40`, no `FC30`.** Encolada con `FC30` en `0x1900`, la sección se copiaba pero no corría (`MP-CSAW-S` con la
  voz 6 muda, también encendiéndola en cada nota). Con `FC40` corre (`MP-CSAW-S`: las 7 voces suenan).
* **`FC44(i, 1)` enciende:** el bit que maneja es de apagado (`0x60003F96`). `engine.c` la vuelve a encender en
  cada `refresh` y en cada nota, como las tablas de perillas.
* El emulador corre las secciones después del principal (`--sim`).

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
