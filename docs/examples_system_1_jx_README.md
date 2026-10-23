# C-S1JX: el microcódigo del JX-3P en el SYSTEM-1, un DCO, 4 voces (`examples/system_1_jx/`)

Estado: 2026-10-23. **En puesta en marcha en el equipo: todavía se cuelga.** Se encontraron y corrigieron cinco
causas de cuelgue (§5); queda al menos una más, en el esclavo B (MAIN1 o el DCO1), §10.5. Las pruebas sin hardware
(9 de microcódigo, 4 de ARM, 20 de la cadena del emulador) pasan.

Es un PLUG-OUT de prueba pedido por el usuario: usa **el código DSP de Roland** (los programas del JX-3P, leídos en la
build desde `plugins/JX-3P`) y nuestro código ARM. Por eso `build/` y `plugins/C-S1JX*/` llevan bytes de Roland: **no
se suben al repo** (`examples/system_1_jx/build/` está en `.gitignore`; ojo: el commit `60c3b42` la incluyó).

Leyenda: **[medido]** en el equipo · **[emulador]** · **[unicorn]** (ARM emulado) · **[código]** · **[inferido]** ·
**[abierto]**.

## 1. Uso

```sh
python3 examples/system_1_jx/test_s1jx.py       # microcódigo: 9 pruebas (≈ 30 s)
sh examples/system_1_jx/build.sh                # -> plugins/C-S1JX/Script/Code1.Dat (y Code.Dat)
python3 examples/system_1_jx/test_arm.py        # código ARM de las dos regiones: 4 pruebas (≈ 1 min)
python3 tools/plugout_tool.py install plugins/C-S1JX --slot 1 --force --no-patches -y   # sustituye al slot 1
```

Necesita `plugins/JX-3P` (1.01; `engine/programs.py` comprueba los SHA-256), `arm-none-eabi-gcc` y `unicorn`. Para
volver a algo que funciona: `plugout_tool.py install plugins/C-S1V8 --slot 1 --force --no-patches -y`. Si el equipo
queda colgado, apagarlo y encenderlo; `plugout_tool.py info` sirve para ver si responde por USB.

Builds de diagnóstico (todas con `NAME=…`):

| variable | efecto |
|---|---|
| `SLAVE_TEST=1` | JX-3P sólo en el esclavo; en el maestro un programa mínimo que saca las voces 2/3 (`Plan.test_tail`) |
| `NO_SEC_A=1`, `NO_SEC_B=1` | no enciende la sección `0x1900` del esclavo A / B |
| `NO_MAIN5=1`, `NO_MAIN1=1` | principal del esclavo A / B vacío |
| `SEC_A_EMPTY=1` | un programa vacío nuestro en la sección del esclavo A |
| `V4_PROGS=1` | programas DSP de C-S1V4 con nuestro código ARM |
| `TRUNC=UNIT:lo:hi` | sólo las instrucciones `lo`…`hi−1` de ese programa (bisección) |
| `NOJUMP=UNIT` | los saltos de ese programa neutralizados (`0090` → `0000`) |
| `NO_STATE=1` | sin escrituras de arranque |
| `RUNTIME_MODEL=1` | el modelo de parámetros evaluado en el ARM en lugar de la tabla precalculada |
| `MODEL_ID` | id del modelo (por defecto `0x0010`, el de C-S1V4) |

## 2. Por qué así

`system-1/DSP_FIT.md`: el JX-3P completo (6 DSP del SYSTEM-8, 4 voces) necesita 3748 instrucciones por muestra y
el SYSTEM-1 tiene ≈ 3660 en sus 4 DSP: no cabe. **Sin DCO2** son 3098 y caben (`dsp_fit.py --dco2 none`). Los
controles de OSC2 no hacen nada (sus salidas se leen como 0).

## 3. Reparto

| DSP del SYSTEM-1 | dónde | programas (orden) | instr./muestra [emulador] |
|---|---|---|---|
| maestro A | `0x0100` | MAIN0 + MAIN2 + relevo (un programa) | 710 (+ el programa de salida del OS en `0x82`) |
| maestro B | `0x1000` | MAIN4 + relevo + DCO1 (voces 0, 1) + cola nuestra | 879 |
| esclavo A | `0x0100` / sección `0x1900` | MAIN5 / MAIN3 + relevo | 703 |
| esclavo B | `0x0100` / sección `0x1900` | MAIN1 / relevo + DCO1 (voces 2, 3) | 848 |

Maestro = voces 0/1 del SYSTEM-8 (DSP0/2/4), esclavo = voces 2/3 (DSP1/3/5). El maestro se reorganizó el 2026-10-23
(antes A = MAIN2 + MAIN4, B = MAIN0 + DCO1) para que MAIN0 y MAIN2, que intercambian MXM en los dos sentidos,
compartan DSP (§4, regla del banco 5). Un principal del esclavo cabe en 6 KiB; el segundo programa va a la sección
`0x1900`, encendida con `FC44(0, 1)` (A) y `FC44(2, 1)` (B). En el maestro se concatenan (se quita el
`0b02 00f8 0001` del primero; los saltos son relativos).

Sets que se encolan en `load`: programas, finales de sección vacíos (`0x6DF2` maestro A; `0x5FF2`, `0x6FF2` maestro
B) y cierre `0x7FF2`. **Nunca la cabecera `0x0000`** (§5).

## 4. Transformación del microcódigo (`s1jx.py`, `dspreloc.py`)

Sólo se reescriben **direcciones literales** (todas las transferencias de estos programas son de modo `fe`) y se
**parten** algunas instrucciones de sólo transferencias (§4.2). `dspreloc.Prog` usa el decodificador del emulador
(`dspisa.Layout`) para saber en qué media palabra está cada dirección.

| qué usaba en el SYSTEM-8 | en el SYSTEM-1 |
|---|---|
| banco 0 (anillo) por DSP | un bloque contiguo del banco 0 por programa en su chip, con su parte del banco 0 y luego la del banco 1; huecos de `0x11` (> alcance máximo del anillo, 16); todo < `0x400` (el SYSTEM-1 usa hasta `0x377`) |
| banco 1 (anillo) por DSP | **al banco 0** (mismo comportamiento de anillo, medido en el SYSTEM-1). El SYSTEM-1 sólo usa `2000`–`2023`; no hay prueba de que el resto funcione |
| banco 5 propio: parámetros que el ARM escribe y el programa sólo lee | **banco 6**, un id libre por palabra en el DSP del programa (`Plan.p6`); salvo los que dejarían más de 2 cargas del banco 6 en una instrucción de un programa con saltos (`Plan.keep_b5`: `P0/P1:0x25` y `0x37`, en `A020`/`A021`) |
| banco 5 propio: palabras que el programa escribe (estado de MAIN0/MAIN1) | banco 5, **`A100`–`A1FF`** |
| MXM entre programas del **mismo** DSP del SYSTEM-1 | anillo del banco 0 con paso 2: store en `R`, load en `R+1` (exactamente la muestra de retraso de MXM) |
| MXM entre los dos DSP de un chip | buzón en el banco 5, `A100`–`A1FF` |
| ranura DCO ↔ su principal (banco 1) | buzón en el banco 5 (`A1xx`); las salidas de **DCO2** que lee MAIN2/3 → `ZERO` |
| ventana de pareja `A4xx` (entre chips en el SYSTEM-1) | IOMD: 11 palabras hacia el esclavo (`600E`…), 4 hacia el maestro (`6006`…) |
| IOMD de entrada de fuera del slave (`60e0`–`60e2`, `6000`–`6006`, `6010`–`6017`) | `ZERO` [abierto: en el SYSTEM-8 vienen de otro chip] |
| IOMD de salida: MAIN4 `6020`/`6021` | `OUT_L`/`OUT_R` (banco 5) → cola en el maestro B: × `C01C` (VOLUME) → `A118`/`A119` |
| otras salidas (`6022`…, `6060`…, `60B0`…, `A4xx` sin lector) | `DUMP`: una palabra aislada del banco 0 |
| `ZERO` | banco 6, id `0xFE` de cada DSP (el ARM le escribe 0) |
| banco 6 (`Cnnn`) por DSP | `+0x50` a un programa si comparte DSP y sus ids chocan; el resto igual |

`build/plan.txt` lista cada palabra y su dueño.

### 4.1 El relevo de registros (`r2`, `r3`)

El DCO1 empieza con un paso de Newton que **lee `r2`/`r3` antes de escribirlos**; en el SYSTEM-8 los dejaba MAIN2
(corrían seguidos en DSP2) [emulador, `test_scramble`]. Aquí el DCO1 corre en el otro DSP: al final de MAIN2/MAIN3
`st r2`, `st r3` a dos palabras del buzón; al principio del DCO1 `ld r2`, `ld r3` (`Plan._handoff`). `test_handoff`
comprueba que las salidas del DCO son iguales bit a bit a las del orden del SYSTEM-8.

### 4.2 Instrucciones partidas (`split_bank5`)

Al reubicar, una instrucción puede quedar fuera de los límites de §5 (p. ej. dos palabras del banco 1 que pasan al
banco 5). Si es sólo de transferencias y el programa no tiene saltos, se parte: los stores que sobran van en
instrucciones propias **antes** (guardan los mismos registros), las cargas que sobran **después** (la instrucción
original siguiente ve todos los valores, como antes). Hoy: MAIN2, MAIN3 y los dos DCO1. MAIN0/MAIN1 tienen saltos y
no se tocan (se evita el problema eligiendo direcciones: reparto del maestro, `keep_b5`).

## 5. Reglas encontradas en el equipo (causas de cuelgue)

Cada una colgó el SYSTEM-1 (error del OS, el panel con todas las luces encendidas o con luces raras; sólo sale
apagando) y se localizó por bisección (§10). Todas las comprueban `test_s1jx.py` (`access_limits`) y, salvo la 1 y la
5, el emulador (`chain.access_violations`: un programa que las incumple no se puede cargar).

| # | regla | evidencia |
|---|---|---|
| 1 | **No encolar la entrada `0x0000`** (cabecera del OS) en `load` | C-S1JX7; los motores de Roland la tienen en sus tablas pero no la encolan (ejecutados en Unicorn, [../system-1/WAVE_BUS.md](../system-1/WAVE_BUS.md) §5.3) |
| 2 | **Banco 5: como mucho una carga y un store por instrucción** (ya medido en el SYSTEM-8) | C-S1JXQ/R: MAIN3 con 8 violaciones colgaba, un programa vacío no |
| 3 | **Banco 0: como mucho 2 stores por instrucción** (y 2 cargas) | C-S1JXH, MAIN3 instrucción 55 (3 stores tras pasar el banco 1 al banco 0); en todo el código de Roland nunca hay más de 2 |
| 4 | **Banco 6: como mucho 2 cargas por instrucción** | C-S1JXH1, MAIN1 instrucción 48 (3 cargas tras pasar un parámetro al banco 6); nunca más de 2 en el código de Roland |
| 5 | **Nuestra regla: un solo acceso al banco 5 por instrucción; los DSP sólo guardan en `A100`–`A1FF`** | una carga con un store del banco 5 no aparece nunca en los programas del SYSTEM-8 y estaba en la mitad de MAIN3 que colgaba; todos los programas de Roland del esclavo del SYSTEM-1 guardan sólo en `A1xx` (`A000`–`A0FF` es lo que escribe el ARM con `FD6C sel 0`). Ninguna de las dos está aislada por una prueba propia [inferido] |

Nota: los límites 3 y 4 no estaban documentados; el SYSTEM-8 sólo había medido "los stores al banco 0 no cuentan"
para el límite del banco 5 ([DSP_HARDWARE.md](DSP_HARDWARE.md) §7.3).

## 6. Puesta en marcha: lo que hace falta además del microcódigo

| qué | por qué | dónde |
|---|---|---|
| **el modelo de parámetros completo** (`model_full.py`) | el `engine/model_spec.py` actual no tiene filtro ni envolventes (§9); sin ellos el corte queda en 0 | `model_full.py` = `model_spec.py` de `013d3fe` |
| **INIT PATCH** | el OS del SYSTEM-1 no tiene los ids del JX-3P; los `default` del `Script.xml` (`re/jxpatch.py`), el resto 0 | `s1jx.default_vals()` |
| **`0x7E4`, `0x74D`** | floats del OS del SYSTEM-8: corte (a 0 cierra el filtro) y velocidad del LFO | iguales a `0x6EA/255`, `0x6CF/255` |
| **`P4:0x4A` = 1** | nivel de salida de MAIN4 (lo pone `engine.c` en `unmute`) | `s1jx.ENGINE_EXTRA` |
| **estado precalculado** | el modelo se evalúa en el PC; el ARM copia una tabla (`jx_state0`/`jx_state1`): `load` pasa de ≈ 441 000 a ≈ 4 600 instrucciones, sin coma flotante | `s1jx.state_table`, `jx.h` |
| **`ZERO` = 0** | id `0xFE` del banco 6 de cada DSP | `state_table` |
| **secciones encendidas** | el OS las apaga al cargar | `FC44(0, 1)`, `FC44(2, 1)` |
| **relevo `r2`/`r3`** | §4.1 | `Plan._handoff` |
| **VOLUME** | `0x100` → `C01C` del maestro B, (v/255)² | `set_volume` |

Rampas (`+0xA0` del SYSTEM-8): se escriben inmediatas (`FCEC`); el servicio con rampa del SYSTEM-1 (`FCE4`) no está
mapeado.

## 7. Pruebas (independientes; no necesitan el equipo)

`test_s1jx.py`:

| prueba | qué demuestra |
|---|---|
| `identity` | las transferencias se decodifican; la reubicación identidad devuelve las mismas palabras |
| `units` | cada programa original contra el reubicado (sin relevo ni partición), solo en un DSP, entradas externas iguales: salidas iguales **bit a bit** 3000 muestras |
| `scramble` | ningún programa final depende de registros/banderas que deje otro |
| `handoff` | §4.1 (y falla sin el relevo) |
| `memory` | anillos, buzones de anillo, `DUMP`, ids del banco 6, banco 5 e IOMD sin solapes |
| `access_limits` | los programas finales cumplen §5 (reglas 2–5); la reubicación sin partir instrucciones es rechazada por el emulador (control positivo) |
| `device_history` | el criterio de §5 contra las builds instaladas (`plugins/`, local): coincide con el equipo en las 11 |
| `sizes`, `budget` | cada programa cabe en su zona; ≤ 925 instrucciones por muestra (maestro A ≤ 885) |

`test_arm.py` (ARM en Unicorn): sin VFP; sets encolados = los de `s1jx.py`, **nunca `0x0000`**; tras `load` cada
palabra (banco 5 y banco 6) coincide con el modelo en Python; notas en las cuatro voces.

`s1jx.py --check` (audio en el emulador) sale **inconcluso**: el emulador no reproduce el filtro de MAIN4
([EMULATOR.md](EMULATOR.md) §5.2).

El modelo completo coincide con el motor del JX-3P (`engine/test_model.py` con `model_full`): 0 diferencias.

## 8. Riesgos y abierto

| # | qué | estado |
|---|---|---|
| 1 | **queda un cuelgue en el esclavo B** (MAIN1 o el DCO1, §10.5) | [abierto], siguiente paso |
| 2 | el maestro con el JX-3P no se ha probado con las reglas de §5 | [abierto] |
| 3 | saltos: ningún programa del SYSTEM-1 usa saltos; MAIN0/MAIN1 tienen 8. Neutralizarlos no quitó el cuelgue de MAIN1 (C-S1JXJ), pero aquella versión tenía también el problema del banco 6 | [abierto] |
| 4 | presupuesto del SYSTEM-1 (925 medido en el SYSTEM-8); maestro B al 95 % | [abierto] |
| 5 | entradas IOMD a 0 (`60e0`–`60e2` de MAIN0: posiblemente el LFO del maestro del SYSTEM-8) | [abierto] |
| 6 | panel sin mapear (sólo VOLUME; OSC2 no hace nada) | pendiente: PARAMLOG ([examples_system_1_README.md](examples_system_1_README.md) §6) |
| 7 | el emulador no reproduce el filtro del JX-3P | [abierto], [EMULATOR.md](EMULATOR.md) §5.2 |

## 9. Lo que se eliminó del modelo de `engine/` y dónde está

| commit | qué se quitó de `examples/from_scratch/engine/model_spec.py` | para qué |
|---|---|---|
| `b50213f` "pyc" | ENV-2 (amplitud) y su `GATE` de VCA Mode | C-NOENV |
| `0396d20` "env1" | ENV-1 | ídem |
| `b440296` "mini" | VCF: corte, resonancia, env/LFO/key mod, bend/aftertouch al filtro | mini (sin filtro) |

La versión completa (`013d3fe`) **funciona** contra el JX-3P. Con el `model_spec.py` actual, `engine/test_model.py 4`
da 92 (fuente, destino) y 84 ids distintos. Detalle en
[examples_from_scratch_engine_README.md](examples_from_scratch_engine_README.md) ("Removed from the model").

## 10. Puesta en marcha en el equipo (2026-10-22 y 23)

Una instalación por fila, en orden. "Cabecera" = el set encolaba `0x0000`. "Esclavo A" = MAIN5 principal + MAIN3
sección; "esclavo B" = MAIN1 principal + DCO1 sección.

### 10.1 Primera tanda: la cabecera (regla 1)

| build | qué llevaba | resultado |
|---|---|---|
| C-S1JX (1.ª) | todo, modelo en el ARM, cabecera, id `0x0011` | cuelgue, luces raras |
| C-S1JXS | esclavo completo, maestro mínimo, cabecera | cuelgue, todas encendidas |
| C-S1JXB | ídem sin sección B | cuelgue |
| C-S1JXT | ídem con el estado precalculado | cuelgue |
| C-S1JX4 | programas de C-S1V4 + nuestro ARM, escrituras, cabecera | cuelgue |
| C-S1JX5 | ídem sin escrituras ni cabecera, id `0x0010` | **no** cuelga, algunas notas |
| C-S1JX6 | C-S1JX5 + escrituras | **no** cuelga (ruido de fondo) |
| C-S1JX7 | C-S1JX6 + cabecera | **cuelgue** ⇒ regla 1 |

### 10.2 Segunda tanda: banco 5 (regla 2)

| build | qué llevaba | resultado |
|---|---|---|
| C-S1JX (2.ª) | todo sin cabecera | cuelgue |
| C-S1JXS, C-S1JXB | esclavo completo / sin sección B | cuelgue |
| C-S1JXM | esclavo A sólo (sin MAIN1) | cuelgue |
| C-S1JXM (2.ª) | ídem con el banco 1 pasado al banco 0 | cuelgue |
| C-S1JXP | sólo MAIN5 | **no** cuelga |
| C-S1JXQ | sólo MAIN3 (sección) | cuelgue |
| C-S1JXR | sólo un programa vacío nuestro en la sección | **no** cuelga ⇒ MAIN3: 2 accesos al banco 5 por instrucción ⇒ regla 2 |

### 10.3 Tercera tanda: un acceso al banco 5, `A1xx` (regla 5)

| build | qué llevaba | resultado |
|---|---|---|
| C-S1JX (3.ª) | todo con la regla 2 | cuelgue, luces raras |
| C-S1JXS (3.ª) | esclavo completo con la regla 2 | cuelgue |
| C-S1JXM, C-S1JXQ (3.ª) | esclavo A / MAIN3 solo | cuelgue |
| C-S1JXH | MAIN3 1–47 (versión de entonces) | **no** cuelga; 1–95 cuelga |
| C-S1JX (4.ª), C-S1JXS (4.ª) | parámetros al banco 6, un acceso al banco 5, maestro reorganizado / esclavo | cuelgue |
| C-S1JXS (5.ª) | stores sólo en `A1xx` | cuelgue |

### 10.4 Cuarta tanda: bisección de MAIN3 (regla 3)

MAIN3 solo en la sección (C-S1JXQ, cuelga) partido con `TRUNC`: 1–96 cuelga, 1–48 no, 1–72 cuelga, 1–60 cuelga,
1–53 no, 1–56 cuelga, 1–54 no ⇒ **instrucción 55**: `st r7,[013f] | st r1,[013d] | st r3,[01a8] | ld r3,[c018]`, tres
stores al banco 0 (en el original el tercero era al banco 1) ⇒ regla 3. Con la regla: **MAIN3 solo no cuelga**
(C-S1JXQ) y **el esclavo A completo tampoco** (C-S1JXM) [medido].

### 10.5 Quinta tanda: el esclavo B (regla 4; abierto)

| build | qué llevaba | resultado |
|---|---|---|
| C-S1JXS (6.ª) | esclavo completo con las reglas 2, 3, 5 | cuelgue |
| C-S1JXB | esclavo A + MAIN1 (sin sección B) | cuelgue ⇒ MAIN1 |
| C-S1JXJ | ídem con los saltos de MAIN1 neutralizados | cuelgue |
| C-S1JXH1 | MAIN1 (sin saltos) 1–215 / 1–107 / 1–53 | cuelgue / cuelgue / cuelgue |
| C-S1JXH1 | MAIN1 1–26 | **no** cuelga ⇒ entre 27 y 53: instrucción 48, `ld [c015] \| ld [c022] \| ld [c024]`, tres cargas del banco 6 ⇒ regla 4 |
| C-S1JXS (7.ª) | esclavo completo con las reglas 2–5 (saltos reales) | **cuelgue** (al instalar) |

### 10.6 Siguientes pasos (para retomar)

Sigue colgándose el esclavo completo con todas las reglas. Partes ya medidas como buenas: el esclavo A completo
(MAIN5 + MAIN3). Por probar, una instalación cada una:

1. `SLAVE_TEST=1 NO_SEC_B=1 NAME=C-S1JXB` (esclavo A + MAIN1 con saltos reales, ya con la regla 4). Si no cuelga, el
   problema es el DCO1 de la sección B; si cuelga,
2. `NOJUMP=MAIN1` sobre lo mismo (C-S1JXJ): separa los saltos del resto de MAIN1; si sigue colgando, bisección de
   MAIN1 con `NOJUMP=MAIN1 TRUNC=MAIN1:1:N` a partir de la instrucción 27.
3. Con el esclavo bueno: la build completa (el maestro con el JX-3P, sin probar con las reglas).

Antes de cada instalación: `python3 examples/system_1_jx/test_s1jx.py` (y actualizar `DEVICE` en `test_s1jx.py` con
el resultado de cada build que se reconstruya).

## 11. Ficheros

| fichero | qué |
|---|---|
| `examples/system_1_jx/s1jx.py` | plan, transformación, partición de instrucciones, `build/s1jx.h`, `build/model.h`, `build/plan.txt`, variables de diagnóstico |
| `examples/system_1_jx/dspreloc.py` | referencias a memoria de un programa DSP y su reubicación; `concat` |
| `examples/system_1_jx/model_full.py` | el modelo de parámetros completo (copia de `013d3fe`) |
| `examples/system_1_jx/jx.h` | voces, escrituras remapeadas (banco 5 / banco 6), estado precalculado o modelo en el ARM |
| `examples/system_1_jx/model.c` / `engine.c` | región 0 (asignador, carga del maestro, VOLUME) / región 1 (carga del esclavo, secciones, voces) |
| `examples/system_1_jx/build.sh` | build y variantes de diagnóstico (§1) |
| `examples/system_1_jx/test_s1jx.py`, `test_arm.py` | §7 |
| `examples/from_scratch/emu/chain.py` | `access_violations` / `STRICT`: el emulador rechaza programas fuera de los límites |

Usa `examples/system_1/syninfo.h`, `link.ld` y `make_s1.py` (cierres) de C-S1V4 sin cambiarlos.
