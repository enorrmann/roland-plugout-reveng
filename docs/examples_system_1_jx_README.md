# C-S1JX: el microcódigo del JX-3P en el SYSTEM-1, un DCO, 4 voces (`examples/system_1_jx/`)

Estado: 2026-10-23. **En pausa: todavía se cuelga.** Se encontraron cinco causas de cuelgue y una sexta inferida
(§5); queda al menos una más en MAIN1 (esclavo B), §10.7. Las pruebas sin hardware
(11 de microcódigo, 4 de ARM, 20 de la cadena del emulador) pasan. Análisis sin equipo: §12 (firmas contra el
SYSTEM-1), §13 (timing entre DSP), §14 (el OS no valida los programas).

Es un PLUG-OUT de prueba pedido por el usuario: usa **el código DSP de Roland** (los programas del JX-3P, leídos en la
build desde `plugins/JX-3P`) y nuestro código ARM. Por eso `build/` y `plugins/C-S1JX*/` llevan bytes de Roland: **no
se suben al repo** (`examples/system_1_jx/build/` está en `.gitignore`; ojo: el commit `60c3b42` la incluyó).

Leyenda: **[medido]** en el equipo · **[emulador]** · **[unicorn]** (ARM emulado) · **[código]** · **[inferido]** ·
**[abierto]**.

## 1. Uso

```sh
python3 examples/system_1_jx/test_s1jx.py       # microcódigo: 11 pruebas (≈ 40 s)
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
(`dspref.layout`, el decodificador C) para saber en qué media palabra está cada dirección.

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

| 6 | **Bancos 5, 6 e IOMD juntos: como mucho 2 cargas por instrucción** [inferido, **no confirmado**] | ningún programa del SYSTEM-1 ni ninguna build que corrió bien tiene más; el JX-3P tiene 4 (MAIN0 41/287, MAIN1 48/294: un parámetro `keep_b5` + 2 del banco 6) y C-S1JXK (MAIN1 1–110, con la 48) colgó. Pero C-S1JXB y C-S1JXS **con** la regla siguen colgando (§10.7). En programas con saltos se cumple con un prólogo (`Plan._stage`, `test_stage`) |

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
| `split` | `split_bank5` no cambia ningún store: cada unidad antes de partir (con el relevo) contra la final, bit a bit 3000 muestras. Con `units` y `handoff`: los programas finales calculan lo mismo que los de Roland **en el emulador** (§13: el timing entre DSP no). Control negativo: con las cargas sobrantes antes del store, falla en la muestra 0 |
| `nojump` | `NOJUMP=MAIN0/MAIN1` **no** es el programa de Roland: los stores difieren (MAIN0 654 de 3000 muestras, MAIN1 362, semilla 0; con la semilla 5 no se nota). Las builds `NOJUMP` sólo sirven para diagnosticar cuelgues |
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

**Superado por §10.7** (los pasos de abajo se hicieron el 2026-10-23; quedan como registro).

Sigue colgándose el esclavo completo con todas las reglas. Partes ya medidas como buenas: el esclavo A completo
(MAIN5 + MAIN3). Por probar, una instalación cada una:

1. `SLAVE_TEST=1 NO_SEC_B=1 NAME=C-S1JXB` (esclavo A + MAIN1 con saltos reales, ya con la regla 4). Si no cuelga, el
   problema es el DCO1 de la sección B; si cuelga,
2. `NOJUMP=MAIN1` sobre lo mismo (C-S1JXJ): separa los saltos del resto de MAIN1; si sigue colgando, bisección de
   MAIN1 con `NOJUMP=MAIN1 TRUNC=MAIN1:1:N` a partir de la instrucción 27.
3. Con el esclavo bueno: la build completa (el maestro con el JX-3P, sin probar con las reglas).

Cortes guiados por §12 en vez de mitades (MAIN1, `SLAVE_TEST=1 NO_SEC_B=1`), cada uno separa un grupo de sospechosos:

| build | instrucciones de MAIN1 | sospechosos que añade |
|---|---|---|
| `TRUNC=MAIN1:1:111` | 1–110 | sólo combinaciones clase/longitud nuevas (41, 60, 68, 84, 108); clases ya vistas |
| `TRUNC=MAIN1:1:184` | 1–183 | clases nuevas `0x5f0`, `0x6c8`, `0x5a4`, `0x568` y `cc 19` (120); ningún salto |
| completo | 1–526 | los 8 saltos (desde la 184) |

Si la primera cuelga, la clase/longitud cuenta y se biseca dentro de 27–110; si sólo cuelga la tercera, son los
saltos. El DCO1 (sección B) se prueba igual: sus primeras instrucciones sospechosas son la 29 (clase/longitud) y la 58
(clase `0x584`).

Antes de cada instalación: `python3 examples/system_1_jx/test_s1jx.py` (y actualizar `DEVICE` en `test_s1jx.py` con
el resultado de cada build que se reconstruya).

### 10.7 Sexta tanda: cortes guiados por §12 y regla 6 (2026-10-23; abierto)

| build | qué llevaba | resultado |
|---|---|---|
| C-S1JXB (2.ª) | esclavo A + MAIN1 completo (reglas 1–5), sin sección B | cuelgue ⇒ MAIN1 |
| C-S1JXK | ídem con `TRUNC=MAIN1:1:111` (sin saltos, sin `cc 19`, sin clases nuevas) | cuelgue ⇒ entre 27 y 110; la 48 tiene 3 cargas de parámetros ⇒ regla 6 [inferido] |
| C-S1JXS (8.ª) | esclavo completo con las reglas 1–6 (prólogo en MAIN1) | cuelgue |
| C-S1JXB (3.ª) | esclavo A + MAIN1 con las reglas 1–6, sin sección B | **cuelgue al instalar** |

Estado: MAIN1 sigue colgando con todas las reglas conocidas; la regla 6 no basta (o no es la causa). Lo que sigue
sin probar de MAIN1 1–110 (§12, firma completa no vista en el SYSTEM-1 ni en el equipo): 39, 41, 49, 59, 60, 64, 65,
67, 68, 81, 82, 84, 106, 107, 108 (`raw`, sin decodificar), 110. Para retomar: `TRUNC=MAIN1:1:111` con el prólogo
(¿basta la regla 6 para ese tramo?) y, si cuelga, bisección de 27–110. El usuario paró aquí: el equipo no resulta
fiable (cuelga incluso al instalar) y el avance es lento.

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
| `examples/system_1_jx/sigdiff.py` | §12: rasgos de instrucción no vistos en el SYSTEM-1 |
| `examples/system_1_jx/timing.py` | §13: latencia de los buzones entre los dos DSP de un chip |
| `examples/from_scratch/emu/chain.py` | `access_violations` / `STRICT`: el emulador rechaza programas fuera de los límites |

Usa `examples/system_1/syninfo.h`, `link.ld` y `make_s1.py` (cierres) de C-S1V4 sin cambiarlos.

## 12. Firmas de instrucción contra el SYSTEM-1 (`sigdiff.py`)

Responde "¿qué usa el C-S1JX que ningún programa de Roland del SYSTEM-1 usa?" sin instalar nada. Sirve para elegir
los cortes de la bisección, no sustituye a la medida: un rasgo no visto en el SYSTEM-1 es **sospechoso, no prueba**.

```sh
python3 examples/system_1_jx/sigdiff.py --suspects     # programas finales; sólo lo que tampoco corrió bien en el equipo
python3 examples/system_1_jx/sigdiff.py --units        # por unidad, antes de concatenar
python3 examples/system_1_jx/sigdiff.py --dat plugins/C-S1JXS/Script/Code1.Dat   # una build instalada
python3 examples/system_1_jx/sigdiff.py --corpus       # qué usa el SYSTEM-1
```

Referencias:

| referencia | qué | programas |
|---|---|---|
| SYSTEM-1 | los sets de programas del OS 1.30 (`work/sys1_v130/app_master.bin`, `app_slave.bin`) y de SH-101, SH-2, PROMARS para SYSTEM-1 | 216 distintos |
| S8 | los programas del JX-3P en el SYSTEM-8 | 7 |
| dev | lo que **corrió en el equipo sin colgar**: la última versión instalada de cada build que `test_s1jx.DEVICE` marca como no colgada (`plugins/<NOMBRE>`, local), sólo las ubicaciones que ejecutaba | C-S1JXM, P, Q, R, 5, 6, H, H1 |

Rasgos por instrucción (cada uno por separado, y la firma completa): clase (`cabecera >> 5`); clase y longitud;
mnemónicos de las operaciones (`asm/disasm.py`; `raw` si no se decodifica); número de accesos por (banco, ld/st);
modo de transferencia distinto de `fe`; saltos `009Y`; por cada unidad C, su código de condición y si puede saltarse
una operación o una transferencia (de qué banco); número de transferencias.

El SYSTEM-1 **nunca** usa [código, 2026-10-23]: saltos; `setflags` (`X9bY`); `mad`; códigos de condición fuera de
{0, 4, 5, 7, 9, 11, 12, 13, 15, 17, 20, 21}; más de 1 acceso al banco 5, más de 2 al banco 0 o al banco 6 por
instrucción (reglas 2–4 de §5); 8 clases que el JX-3P sí usa (`0x340`, `0x4b4`, `0x4b8`, `0x568`, `0x584`, `0x5a4`,
`0x5f0`, `0x6c8`). `setflags`, `mad` y las clases `0x0d8`, `0x188`, `0x220`, `0x2f0`, `0x398` sí corrieron en el
equipo (esclavo A, C-S1JXM): no causan cuelgues.

Resultado sobre los programas finales (2026-10-23, `--suspects`):

| ubicación | rasgos no vistos en el SYSTEM-1 | de ellos, tampoco en "dev" |
|---|---|---|
| maestro A (MAIN0 + MAIN2) | 29 | 24: clases `0x568`, `0x5a4`, `0x5f0`, `0x6c8`; 15 combinaciones clase/longitud; **saltos** (8); `cc 19` |
| maestro B (MAIN4 + DCO1 + cola) | 25 | 18: clases `0x340`, `0x4b8`, `0x584`, `0x6c8`, `0x4b4`; 13 clase/longitud |
| esclavo A (MAIN5; MAIN3) | 8 | **0** (coherente con el equipo: el esclavo A no cuelga) |
| esclavo B (MAIN1) | 28 | 24: los mismos que MAIN0 (MAIN0 y MAIN1 son el mismo programa con otras direcciones) |
| esclavo B (DCO1) | 20 | 18: los mismos que el DCO1 del maestro B |

Lectura para el cuelgue abierto del esclavo B (§10.5): los candidatos son los **saltos** de MAIN1 (instrucciones 184,
187, 237, 240, 430, 433, 483, 486), **`cc 19`** (120, 366), las clases nuevas de MAIN1 (desde la 111) y las del DCO1
(desde la 58). MAIN1 1–26 corrió sin colgar; la primera instrucción sospechosa de MAIN1 es la 41 (sólo por clase/longitud;
la clase `0x180` sí aparece en el SYSTEM-1) [inferido].

## 13. Timing entre los dos DSP de un chip (`timing.py`)

Los dos DSP de un chip **corren a la vez** y comparten el banco 5. Un buzón escrito por uno y leído por el otro se ve
en la misma muestra sólo si el store va antes que la carga, en ciclos. El emulador ejecuta los DSP uno detrás de otro,
así que **no puede ver esto**, y las pruebas de §7 tampoco (comparan cada programa por separado).

`python3 examples/system_1_jx/timing.py [muestras]`: el ciclo de cada instrucción es su posición en la secuencia
ejecutada (traza del emulador, saltos incluidos), más la cabecera del OS (4 instrucciones) y, en el maestro A, el
programa de salida del OS (40). Se supone [inferido] que los dos DSP de un chip empiezan la muestra en el mismo ciclo.
Margen: 8 ciclos. Latencia que tenía el SYSTEM-8: MXM 1 muestra; principal → su DCO 0 (mismo DSP, el principal antes);
DCO → principal 1; relevo `r2`/`r3` 0.

Resultado (2026-10-23, 100 muestras): **45 palabras con un timing distinto al del SYSTEM-8**:

| palabras | dónde | S1 / S8 | efecto |
|---|---|---|---|
| entradas del DCO (`DCO` principal → DCO) y relevo `r2`/`r3` | maestro (MAIN2 → DCO1@2) y esclavo (MAIN3 → DCO1@3) | 1 / 0 (**tarde**) o carrera | el DCO calcula con los valores de la muestra anterior; el paso de Newton empieza desde `r2`/`r3` de la muestra anterior |
| MXM MAIN1 → MAIN3 (`MZ 1`) | esclavo | 0 / 1 (**antes**) | MAIN3 ve el valor de esta muestra |
| 10 entradas del DCO | MAIN2 → DCO1@2, MAIN3 → DCO1@3 | **carrera** (escrituras y lecturas se cruzan) | depende del ciclo exacto y del camino de los saltos de MAIN1 |
| MXM MAIN2 → MAIN4, salidas del DCO → principal | | igual que el SYSTEM-8 | — |

No explica cuelgues. Sí explica que el C-S1JX pueda **no sonar igual** aunque no se cuelgue: son desfases de una
muestra (10 µs) en señales de control, probablemente inaudibles salvo en el DCO (fase/Newton) [inferido]. Corregirlo
pide cambiar dónde corre cada programa (no el código de Roland). Por ejemplo, MAIN3 como principal del esclavo A y
MAIN5 en la sección: MAIN3 escribiría hacia el ciclo 60–190, antes de que el DCO1@3 lea (≥ 455). Sin probar: cambia
el esclavo A, que hoy funciona en el equipo; MAIN5 (5016 bytes) cabe en la sección (6 KiB).

## 14. ¿Impone el OS los límites de acceso? (firmware 1.30 del esclavo)

No, en lo encontrado. Las rutinas del esclavo que escriben la memoria de los DSP (`0x6005362E`, `0x60053708`,
`0x60053852`, destinos `0x40150000` / `0x40170000` = DSP A / B) copian bloques de `0x200` palabras con el DMA
(`0x60055D32` / `0x60055CF0`) **sin leer el contenido** [código]. `0x6000345C`–`0x60003612` es una prueba de memoria
(escribe 0 y un patrón, y compara) [código]. Por tanto los límites de §5 son del **hardware del DSP**, no una
validación del OS, y no hay una tabla de reglas que leer en el firmware. Abierto: no se siguió el camino completo
desde la cola de `FC30` (`0x60003ECF`, cola `0x600603E0`) hasta estas rutinas.
