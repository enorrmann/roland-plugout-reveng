# Bus de audio entre los DSP del SYSTEM-8 (slaves → maestro)

Documento vivo: cada hallazgo lleva su estado y la prueba que lo sostiene. Se actualiza a medida que avanzan las
pruebas (última revisión: 2026-10-21). Mapa de memoria general: `DSP_ARM_SIDE.md`; ISA: `DSP_ISA_SPEC.md`;
arquitectura general: `ARCHITECTURE.md`. Este documento es el de **por dónde sale el audio**.

Etiquetas: **[medido]** = observado en el equipo o por audio USB; **[desconocido]** = sin medir;
**[inferido]** = deducido de código de Roland (listado estático) sin prueba en el equipo.

## 0. Ruta de señal que el OS usa y espera (resumen unificado, 2026-10-21)

Objetivo: construir un plug-out sin programas de Roland, sabiendo qué espera el OS en cada paso.
Antes se mantenían dos documentos sobre lo mismo; este es el único.

### 0.1 Flujo de la voz (asignación por componente)

Regla del usuario (2026-10-21): cada componente conserva el DSP y la ranura que Roland le asigna. El DSP de un
componente no cambia; se reemplaza su programa en el mismo DSP y ranura.

| componente | DSP | ranura | fuente |
|---|---|---|---|
| entrada de tono y puerta de la voz (ENV) | DSP0 / DSP1 | `0x0100` (principal) | [código] `engine.c` `note_pitch`/`note_gate` escriben banco 5 de DSP `u = lv>>1` |
| envolvente y LFO | DSP0 / DSP1 | `0x8000` | [código] `engine.c` `slots[0..1]` |
| oscilador DCO1 | DSP2 / DSP3 | `0x6000` | [código] `engine.c` `dco1_prog`; [medido] §4g |
| oscilador DCO2 | DSP2 / DSP3 | `0xB000` | [código] `engine.c` `dco2_prog` |
| filtro y VCA (velocidad) | DSP4 / DSP5 | `0x6000` / `0xB000` (efecto DSP4) | [código] `engine.c` `note_gate` escribe `d = 4+u` |
| salida al bus de onda | DSP4 (IOMD0 `0x20–0x24`) y DSP5 (`0xB0–0xB7`) | IOMD | [documentado] `DSP_ARM_SIDE.md`; [medido] `6020` llega a la salida |

### 0.2 Caminos entre DSP (MXM, banco 4)

Fuente [documentado] `DSP_ARM_SIDE.md`:
* DSP0 guarda Z `404–416` → DSP2 carga M `004–016`.
* DSP2 guarda Z `404–41A` → DSP4 carga M `005–01A`.
* DSP1 guarda Z `400–410` → DSP3 carga O `200–210`; DSP3 Z `400–414` → DSP5 O `201–214`.
* Retroalimentación: DSP2 guarda M `017–019` → DSP0 carga S `617–619`.

[medido] Roland MAIN2 escribe `[8404]` (`dsp/dump_main.py MAIN2`). El par DSP2→DSP4 no está medido en el equipo.

### 0.3 Entradas y salidas del oscilador (direcciones de programa)

Fuente: `re/dspasm.py` `SLOTS` (desde los programas SAW de Roland):

| | voz A | voz B |
|---|---|---|
| DCO1: tono de entrada | `[2201]` | `[220E]` |
| DCO1: salidas | `[221F]`, `[221B]` | `[2227]`, `[2223]` |
| DCO1: estado (fase) | `0470` | `0480` |
| DCO2: tono de entrada | `[2207]` | `[2214]` |
| DCO2: salidas | `[2221]`, `[221D]` | `[2229]`, `[2225]` |

El programa principal (MAIN2) escribe `[2201]`/`[2207]` desde el banco y el MXM, y lee las salidas de DCO para
enviarlas a DSP4. **Nuestro SAW de prueba leía `[A021]` y escribía `[8404]`**: no sigue esta ruta. [código]

### 0.4 Banco 5: lo que escribe el ARM

| id de banco 5 | DSP | significado | fuente |
|---|---|---|---|
| `0x21` | u = lv>>1 (DSP0 o DSP1) | altura: `(nota−12)/12` | [código] `note_pitch` (engine.c) |
| `0x22` | u | puerta de envolvente (1 al pulsar) | [código] `note_gate` (engine.c) |
| `0x22` | 4+u (DSP4/5) | velocidad del VCA `vel/127` | [código] `note_gate` (engine.c) |
| `0x60+lv`, `0x68+lv` | DSP5 | altura y puerta de la voz de `mini/` (`dsp/poly.hex`) | [código] `engine_mini.c` `voice_gate` |

### 0.5 Qué código de Roland carga el arranque

* `mini/engine_mini.c` (el que suena) carga en `e_load` los **programas principales `MAIN0`–`MAIN5` de Roland**,
  los programas de DCO de Roland en DSP2/DSP3 (`DCO1_SAW`, `DCO1_SQR`, `DCO1_FOLD`, `DCO2_*`) y los efectos de
  Roland en DSP4 (`fx_prog`, `swap_fx`). Su voz de DSP5 sí es nuestra (`poly.hex`). [código]
* `gate/engine_gate.c` (el que rompe) no carga ningún programa de Roland, pero tiene vacíos los seis principales.

Ver §4l para lo medido sobre cada una de estas piezas.

## 1. Piezas

| pieza | qué es | fuente |
|---|---|---|
| IOMD (banco 3, `6nnn`) | 256 palabras de E/S compartidas por los seis DSP de un BMC. IOMD0 `00–BF` (DSP4/5), IOMD1 `C0–DF` (DSP2/3), IOMD2 `E0–FF` (DSP0/1) | [medido] direcciones (`DSP_ARM_SIDE.md`, tabla de bancos) |
| MXM (banco 4, `8nnn`) | bus entre DSP de un BMC: cada DSP guarda una zona Z/S y el DSP vecino la carga en su zona O/M, con latencia de 1 muestra | [medido] (`DSP_USOS.md` §5b); tabla de zonas `DSP_ARM_SIDE.md` |
| Bus de onda | conexión externa entre BMC (pines), por donde los slaves envían la voz al maestro | [desconocido] correspondencia palabra → pin (`DSP_HARDWARE.md` §11) |
| Banco 5 (`Annn`) | memoria persistente de cada DSP; el ARM escribe parámetros con `+0xB0` | [medido] (`DSP_USOS.md` §2) |

## 2. Qué hace el programa principal DSP5 (MAIN5 de Roland)  [medido en el listado]

Fuente: `examples/from_scratch/dsp/dump_main.py MAIN5` (desensambla `engine/build/programs.h`, es decir el
microcódigo del JX-3P copiado en el build). Son sus 721 líneas; las referencias de bus son:

**Al inicio de cada muestra** carga entradas en `[2204]`–`[221D]` desde MXM y desde IOMD:

| destino | origen |
|---|---|
| `[2204]`, `[220C]`, `[2210]`, `[2211]`, `[2216]`–`[2219]` | ← MXM `8206`, `820D`, `8213`, `8214` y bank 5 `A405`–`A407` |
| `[221A]`–`[221C]`, `[2212]`–`[2215]` | ← IOMD `6010`–`6017` |

**Al final de la muestra** copia esas palabras a IOMD (la salida):

| IOMD | copia de | estado |
|---|---|---|
| `60B0` | `[2216]` | [medido en listado] |
| `60B1` | `[2217]` | [medido en listado] |
| `60B2` | `[2218]` | [medido en listado] |
| `60B3` | `[2219]` | [medido en listado] |
| `60B4` | `[2210]` | [medido en listado] |
| `60B5` | `[2211]` | [medido en listado] |
| `60B6` | `[2204]` | [medido en listado] |
| `60B7` | `[220C]` | [medido en listado] |

**Corrección 2026-10-22 (`VOICE_ALLOCATION.md` §6):** MAIN5 escribe además `A400`–`A403` (banco 5, buzón de la
pareja) y MAIN4 los suma a `6020`/`6021`: DSP5 procesa las voces 2 y 3 y las entrega a DSP4 [listado]. La frase
siguiente sólo vale para las palabras `60B0`–`60B7`.

Conclusión de esta sección: **DSP5 no genera la voz; la reenvía**. Las ocho palabras de salida son una copia de lo
que llega por MXM y por IOMD `6010`–`6017`. Lo que genera la voz está antes, en DSP2/3 y DSP4.

**Caveat**: `[2204]`–`[221D]` también son direcciones de la ranura DCO (ver `CUSTOM_OSC.md`). En DSP5 no hay ranura
DCO2, así que aquí son sólo memoria de trabajo del programa principal.

## 3. Qué hace DSP4 (MAIN4 de Roland)  [medido en el listado]

* Lee IOMD `6000`–`6006` (entrada) y escribe `6020`–`6024` y `6060`–`6066` (salida).
* **Corrección importante**: `6020` es una palabra de **DSP4**. Durante meses creímos que la voz de DSP5 salía por
  `6020`; en realidad es la palabra de salida de DSP4 (ver §5).

## 4. Mediciones del equipo (audio USB, `arecord -D hw:SYSTEM8,0 -f FLOAT_LE -r 96000 -c 2`)

Todas con `C-IOMD` / `C-MINI`/`C-MINI2` en slot 3 (PLG3 seleccionado) salvo que se indique.

| fecha | prueba | resultado | estado |
|---|---|---|---|
| 2026-10-21 | DSP5 DCO1 escribe SAW fijos en `60B0` (440), `60B1` (660), `6020` (880) | sólo **880 Hz en el canal derecho** de la captura (armónicos 1/n). `60B0` y `60B1` no llegan | [medido] (`FROM_SCRATCH.md` §11) |
| 2026-10-21 | mismo programa, reinstalado tras reinicio | igual: 880 Hz, rms 0.01269, canal izquierdo en silencio | [medido] |
| 2026-10-21 | engine propio (`C-MINI2`), voz en DSP5 con puerta desde el ARM | sólo suenan algunas notas (60 y 64 en una ejecución; 67–76 en otra), siempre en un canal | [medido], **causa no resuelta** |
| 2026-10-21 | drone (envolvente fija) de `C-MINI2` | tono constante 329.7 Hz = última nota de la voz; una sola voz audible | [medido] |
| 2026-10-21 | voz escribe también en `60B0` | sin cambio claro en el canal izquierdo | [medido], sin conclusión |
| 2026-10-21 | mapa de canales (`dsp/make_bmap.py`): ocho SAW fijos en `6020`–`6024` y `6060`–`6062`, una instalación sin notas | **`6020` → canal 1 (110 Hz)** y **`6021` → canal 0 (165 Hz)**, con armónicos 1/n; el resto de palabras no aparece (≈ −100 dB) | [medido] |
| 2026-10-21 | mapa de canales en `60B0`–`60B7` (primera versión, 110–495 Hz) | silencio total en los dos canales | [medido]; hipótesis: MAIN5 sobrescribe B0–B7 al final de cada muestra (§2) |
| 2026-10-21 | voz de `C-MINI2` escribe la misma señal en `6020` (canal 1) y `6021` (canal 0) | **estéreo**: los dos canales con rms 0.01034; notas 67–76 con afinación exacta (±0.01 st); 48–64 en silencio en esta ejecución | [medido] |

**Lo que no sabemos de estas pruebas**
* Qué canal de la captura corresponde a qué salida física: el usuario oye por la izquierda lo que la captura
  registra en el canal derecho (`ch1`). Hay que fijar el mapeo antes de sacar conclusiones de L/R.
* Qué slave (voces 0–3 o 4–7) produce cada tono: los dos slaves ejecutan el mismo programa y la asignación de
  notas entre slaves depende del historial del OS.

## 4b. Barrido de todas las palabras IOMD que usa Roland (2026-10-21)

Conjunto: las 38 direcciones `6000`–`60FF` que referencian MAIN0–MAIN5 del JX-3P (`dump_main.py`; MAIN0–3 no
referencian IOMD). Método: `dsp/make_bmap.py` con `BMAP_WORDS=…` escribe ocho SAW fijos (110, 165, … 495 Hz) en
ocho palabras, una instalación por lote, PLG3 seleccionado, sin notas. `dsp/analyze_bmap.py` mide el nivel de cada
frecuencia en cada canal. Como un SAW tiene armónicos 1/n (−6, −9.5, −12 dB), un nivel a 2×, 3× o 4× la
fundamental de otra palabra **no** es prueba de esa palabra: se compara con la predicción antes de decidir.

| palabra | escrita por / leída por (Roland) | resultado | estado |
|---|---|---|---|
| `6020` | escribe DSP4 (MAIN4) | sale por **canal 1** (110 Hz, armónicos exactos) | [medido] |
| `6021` | escribe DSP4 (MAIN4) | sale por **canal 0** (165 Hz, armónicos exactos) | [medido] |
| `6022` | escribe DSP4 | silencio (ningún 220 Hz por encima de la predicción) | [medido] |
| `6023` | escribe DSP4 | silencio (275 Hz ausente) | [medido] |
| `6024` | escribe DSP4 | **ambiguo**: 330 Hz en canal 0 a −45.5 dB frente a −46.9 dB predichos por el armónico de 165 Hz (1.4 dB de exceso); puede ser una contribución de `6024` | [medido, sin resolver] |
| `6060`, `6061`, `6062` | escribe DSP4 | silencio | [medido] |
| `6063` | escribe DSP4 | sale por **los dos canales** a la vez, 110 Hz a −23.9 dB (armónicos exactos) | [medido] |
| `6064`, `6065`, `6066` | escribe DSP4 | silencio (220, 275 y 165 Hz ausentes o en la predicción de armónicos) | [medido] |
| `6000`–`6006` | leída por DSP4 | silencio | [medido] |
| `6010`–`6017` | leída por DSP5 | silencio | [medido] |
| `6040`, `6046` | referenciadas por MAIN4/MAIN5 | silencio | [medido] |
| `60B0`–`60B7` | escribe DSP5 (MAIN5, copia de `[2204]`–`[221D]`) | silencio en los dos canales | [medido] |
| `60E0`–`60E2` | leída/usada por DSP0/1 (IOMD2) | silencio (los niveles de 330 y 440 Hz son exactamente los armónicos de `6063`) | [medido] |

**Lo que dicen los datos**
* Sólo `6020`, `6021` y `6063` son audibles en esta prueba (el 330 de `6024` queda por resolver).
* `6020`/`6021` van a canales distintos; `6063` va a los dos. Eso sugiere que las palabras de salida son estéreo
  por pares o que `6063` es un bus común. No está claro todavía.
* **Conflicto con la hipótesis de sobrescritura**: MAIN4 escribe `6021` y `6063` cada muestra igual que escribe
  `6022`–`6024`, `6060`–`6062` y `6064`–`6066`, pero las dos primeras sí llegan al audio y las otras no. Así que
  "MAIN4 sobrescribe" no explica el silencio. La explicación más simple es que sólo las palabras conectadas al bus
  llegan al audio, pero eso queda sin medir.
* El silencio de `60B0`–`60B7` no permite separar dos casos: (a) no están conectadas al bus, o (b) MAIN5 las
  sobrescribe después de nuestro slot. Para separarlos hace falta medir el orden de ejecución dentro de la muestra.

## 4c. Orden de ejecución dentro de la muestra (2026-10-21)

* **Escrituras finales de MAIN4** (`6020`–`6024`, `6060`–`6066`, líneas 728–741 del listado): **incondicionales**,
  porque están después de la última instrucción condicional del programa (`dump_main.py MAIN4`). Se ejecutan en cada muestra.
* **Slot de DSP5 frente a MAIN4 (DSP4)** [medido por inferencia]: el tono del slot sobrevive en `6021` y `6063`, que
  MAIN4 escribe en cada muestra. Por tanto, la escritura del slot de DSP5 queda **después** de la de DSP4 en la muestra.
  Caveat: son dos DSP distintos; el orden lo fija el planificador del BMC, no un programa.
* **Slot de DSP5 frente a MAIN5 (mismo DSP)** [no medido]: MAIN5 escribe `60B0`–`60B7` al final, y ninguna de esas
  palabras llega al audio. Sin una palabra audible que MAIN5 escriba, el orden no se puede observar por audio. Hace
  falta trazar un camino de datos desde una entrada leída por MAIN5 (`6010`–`6017`, `[2204]`–`[221D]`) hasta una palabra audible.

## 4d. Lo que dice el binario del OS (2026-10-21)

Fuentes: `dumps/os_full.bin` (imagen maestra), `work/os/s_boot/s_boot.bin` (imagen de los slaves, con
`tools/os_analyze.py --parts`). Advertencia: el análisis por función de `work/os/functions.json` mezcla datos y código
(funciones de cientos de KB); sólo se usan rutinas verificadas por desensamblado.

* **Base IOMD igual en los seis DSP**: tabla de la imagen maestra en `0x60172B4C` (seis entradas `0x4113C000`) y tamaño
  `0x100` (`0x60172B68`). [medido en el binario]
* **`swap` del slave** (`s_boot` `0x01105F94`): secuencia `sync(1)` → escritura de programa y datos → ajuste de
  parámetros → `sync(3)` → reactivación de la tarea. [medido en el binario]
* **`sync(n)`** (`s_boot` `0x011043B0`): pone una bandera, espera a que el DSP complete su trama (`wfi` y un límite de
  1000 unidades) y llama al servicio `0x01111C58` con id 5. Así los cambios de programa entran en un límite de trama.
  [medido en el binario]
* **Lo que no está**: el orden en que el planificador del DSP ejecuta main y slots dentro de la muestra. Ninguna
  rutina estudiada lo fija; probablemente depende de los números de tarea (`task` = 2 para DCO1, 3 para DCO2), pero no
  está verificado. Próximo paso: seguir `0x01111C58` (servicio 5) y las escrituras al control `0x41104318 + n·0x1000`.

## 4e. Slave 1 no llega al audio por IOMD (2026-10-21)

Prueba: `C-MINI2` con un drone por slave, sin notas. El slave 0 toca 261.6 Hz (nota 60) y el slave 1 toca 147.4 Hz
(nota 48). Cada slave escribe su señal en **todas** las palabras `6020`–`60FF` (`make_voice.py`, `ALLOUT=1`).

| medición | resultado |
|---|---|
| slave 0 (261.6 Hz) | presente en los dos canales, −40 dB, en la captura |
| slave 1 (147.4 Hz) | **ausente** en los dos canales (−103.6 dB) |

Conclusión [medido]: ninguna palabra de IOMD `6020`–`60FF` lleva la voz del slave 1 hasta la salida. Como la voz de
DSP5 es el mismo código en los dos slaves, el fallo está en algo que sólo depende del slave: que el slot de DSP5 no
corra en el slave 1 (tarea o `swap`), que la escritura de banco 5 (`+0xB0`) no llegue a su DSP5, o que su salida no
pase por IOMD (otro camino hacia el maestro, quizá el bus de onda por pines, `DSP_HARDWARE.md` §11).

**Causa de las notas que no sonaban (resuelta 2026-10-21)**: no era el slave 1, sino nuestro filtro de voces. El
engine (`mini/engine_mini.c`) aceptaba sólo las voces `0–3` de su slave (`v = id − 0x429 − first`), así que las notas
del asignador en las voces `4–7` se descartaban. Al aceptar las ocho voces en el slave 0 (voz monofónica de última
nota), las notas 48, 50, 60, 64, 67, 69, 72 y 76 suenan todas con la afinación exacta (−0.02 a +0.02 semitonos) en
los dos canales. [medido]

El slave 1 sigue sin llegar a la salida (§4e); con la voz en el slave 0 no hace falta para sonar.

**Medición por audio (2026-10-21, `C-SLV`: el slave 1 sube una octava, `gate/diag/build_slv.sh`):** notas 48, 57, 64
y 72, una a una. Nivel de cada armónico frente al suelo del espectro:

| nota | f0 | 2f0 | 3f0 | 4f0 | 2f0/f0 |
|---|---|---|---|---|---|
| 48 | ×204 | ×88 | ×40 | ×20 | 0.43 |
| 57 | ×181 | ×39 | ×21 | ×11 | 0.21 |
| 64 | ×118 | ×32 | ×14 | ×9 | 0.27 |
| 72 | ×78 | ×21 | ×12 | ×11 | 0.27 |

Lectura [medido]: la fundamental de la nota está (es la del slave 0) y **no aparece una fundamental a 2f0 de nivel
comparable**, que es lo que daría el slave 1 con la octava subida. Solo se oye el slave 0. No excluye que el slave 1
suene muy por debajo. Las razones 2f0/f0 y 3f0/f0 no son las de un SAW ideal (0.5 y 0.33): hay un filtrado o una
ventana que no se ha caracterizado, así que no se usan para más conclusiones.

**Estado 2026-10-21 (plug-out `C-SOLO`, ocho voces en el slave 0):** las ocho voces suenan con el motor propio y sin
código de Roland (`gate/engine_solo.c`, `dsp/make_solo.py`). El slave 1 sigue sin ruta medida hasta la salida: ninguna
palabra de IOMD `6020`–`60FF` la lleva (§4e). Para repartir voces entre los dos slaves hace falta antes esa ruta; si el
slave 1 llegara a la salida con las ocho voces, cada nota sonaría duplicada. El reparto correcto sería slave 0 con
`0–3` y slave 1 con `4–7`, con el filtro por `first` de `mini/`. La ruta del slave 1 sigue abierta: candidatos sin medir
son los pines del bus de onda (`DSP_HARDWARE.md` §11) y el banco 5 del DSP5 del slave 1.

Lo que no podemos decir aún: cuál de las tres causas es. La siguiente medida debe distinguirlas: por ejemplo, un slot
del slave 1 que escriba un valor conocido en un banco que sólo el ARM del slave 1 pueda leer (con el lab o con
`dsplive`), o un drone del slave 1 que cambie de frecuencia al recibir su nota (si la frecuencia cambia, el DSP corre).

## 4f. Polifonía (2026-10-21)

Diseño simétrico: cada slave tiene cuatro voces (`lv` = voz − primera voz del slave; slave 0 = voces 0–3, slave 1 =
voces 4–7). En DSP5 de cada slave corre `dsp/make_poly.py` (cuatro voces AR + SAW + LFO compartido, sumadas a
`6020` y `6021`), y el ARM escribe el tono de la voz `lv` en el banco 5 id `0x60+lv` y su puerta en `0x68+lv`
(`mini/engine_mini.c`). Simulación (`make_poly.py --sim`): dos voces a la vez, 440 y 220 Hz, presentes.

Medición en el equipo (`C-MINI2`, dos acordes, 3 s cada uno):

| acorde | resultado |
|---|---|
| 55 y 72 (de 48, 55, 72) | **suenan a la vez** (−46 y −47 dB): **polifonía confirmada en el slave que suena** |
| 48 (del mismo acorde) | silencio |
| 60, 64, 67 | silencio |

El 392 Hz que aparece (−53 dB) es el segundo armónico del 196 Hz (−52 dB predicho), no una nota del acorde.

Lectura [inferida, no medida]: las notas en silencio cayeron en voces del slave 1, que no llega a la salida (§4e). La
asignación de voces la hace el OS y no la controlamos desde MIDI, así que no sabemos qué slave recibió cada nota.

## 4g. Regla de flujo y resultado con la cadena de Roland (2026-10-21)

**Regla (indicación del usuario, "muy importante")**: cada componente de la voz conserva el DSP y la ranura que Roland
le asignó. El DSP de un componente no cambia: un oscilador sigue en DSP2/3, un LFO o una envolvente siguen en DSP0/1,
el filtro y el VCA siguen en DSP4/5. Dentro de esa asignación:

* **Reemplazar** el programa de un componente en su mismo DSP y ranura está permitido (p. ej. el SAW propio en el
  DCO1 de DSP2).
* **Quitar** un componente deja vacía su ranura en su mismo DSP. No pasa a otro DSP.
* **Mover** una función a otro DSP, o reunir dos funciones en un mismo DSP, no está permitido.

Asignación de Roland (`engine/engine.c`, `e_load`; `docs/ARCHITECTURE.md`): DSP0/1 ranura `0x8000` (envolventes y LFO);
DSP2/3 ranuras DCO1 `0x6000` y DCO2 `0xB000` (osciladores); DSP4 ranura DCO1 `0x6000` (filtro/VCA) y `0xB000` (efecto
del PLUG-OUT); DSP5 ranura DCO1 `0x6000` (filtro/VCA). El flujo que controla el OS es DSP0/1 → DSP2/3 → DSP4/5 → IOMD
→ bus de onda.

Por qué: el engine `mini` del 2026-10-21 puso oscilador y VCA juntos en DSP5. Eso rompe la asignación.
[Corrección 2026-10-21, §4l: la frase anterior sobre ranuras DCO vacías venía de una versión del motor; el motor `mini`
real carga los DCO de Roland en DSP2/3. Lo que sí está medido es el mudo con el principal de DSP4 vacío.] `engine/engine.c` y `build.sh` quedan sin cambios.
El plug-out `gate/` (2026-10-21) sigue la regla: oscilador SAW en DSP2 (ranura DCO1), puerta en DSP4 (ranura DCO1, la
función de VCA), sin LFO ni envolvente (DSP0/1 vacíos).

Resultado (`C-SUB2` = engine original + SAW propio en DCO1 y DCO2, `MC_TEST`): las ocho notas de prueba (48–76) suenan
en los dos canales, con el mismo desfase constante de afinación (−11.45 st respecto a la nota esperada, ~+0.55 st
respecto a Roland; pendiente, como se acordó). [medido]

## 4h. Afinación: era el fine tune del OSC2, no el SAW (2026-10-21)

Medida con `C-SUB3` (DCO1 de Roland, DCO2 propio), notas 69 y 81 (limpias: la nota 45 y la 57 baten entre los dos
osciladores y su pico no es fiable):

| fine tune OSC2 | nota 69 | nota 81 | lectura |
|---|---|---|---|
| 0 | −11.42 st | −11.45 st | ~+0.55 st (~52 cents) por encima de lo esperado |
| −128 | −11.93 st | −11.93 st | ~+0.07 st (~7 cents) respecto al patch con octava abajo |

Conclusión [medido]: el desfase era de la escala del parámetro fine tune (−128 … +128) del OSC2 con el valor 0, no del
SAW propio. Queda un residuo de ~7 cents con −128: pendiente de medir la escala exacta del parámetro (no es parte
de la cadena de Roland que tocamos). El fine tune del OSC2 también afecta al DCO1 (mismo programa en la configuración
de Roland), como indicó el usuario.

## 4i. Memoria compartida entre DSP: no existe; mecanismos de comunicación (2026-10-21)

**No hay memoria compartida** entre DSP. Cada DSP tiene sus bancos privados (DT0 ring, DT1, DT2, banco 5, programa).
La comunicación pasa por cuatro mecanismos, todos con **latencia de una muestra** salvo el banco 5:

| mecanismo | qué es | quién escribe / lee | propiedades | fuente |
|---|---|---|---|---|
| **MXM** (banco 4, `8nnn`) | bus punto a punto entre DSP del mismo BMC; zonas M `0x000–1FF`, O `0x200–3FF`, Z `0x400–5FF`, S `0x600–7FF` | DSP n escribe Z/S; el DSP vecino lee M/O | **doble buffer**: el lector ve lo escrito en la muestra n−1 durante todo su programa; **no suma** (gana la última escritura); **no guarda** (si nadie escribe, 0) | `DSP_HARDWARE.md` §8.2 [medido DSP3↔DSP4]; `DSP_ARM_SIDE.md` (routing) |
| **IOMD** (banco 3, `6nnn`) | palabras de E/S del BMC, cada una con un DSP dueño | el dueño escribe; el ARM y el bus leen | las escrituras de quien no es dueño no aparecen | `DSP_HARDWARE.md` §8.3 |
| **Banco 5** (`Annn`) | parámetros persistentes; el ARM escribe con `+0xB0` | el ARM escribe; el DSP lee (y a veces escribe) | persistente | `DSP_USOS.md` §2 |
| **Bus de onda** (pines entre BMC) | voces de los slaves al maestro | DSP5 de cada slave → maestro | correspondencia palabra→pin **[desconocido]** | `DSP_ARM_SIDE.md` |

**Importante**: las direcciones `8nnn` son **ventanas locales de cada DSP**. `8405` en DSP0 y `8405` en DSP2 no son la
misma palabra. El emparejamiento está en el routing: DSP0 Z `404–416` → DSP2 M `004–016`, y DSP2 Z `404–41A` → DSP4 M
`005–01A` (`DSP_ARM_SIDE.md`).

### Traza de la envolvente (estática, MAIN0 → DSP2 → DSP4)

1. **DSP0 (MAIN0)** escribe en su Z `8405`–`8416` (y `8404`) las salidas de las envolventes y el LFO (líneas 613–636
   del listado, `dsp/dump_main.py MAIN0`).
2. **DSP2 (MAIN2)** lee su M `8006`–`8015` y lo **reenvía**: `st r0,[8405]` con lo que lee de `8005`… (línea 9 del
   listado). Además multiplica `[222c]` por parámetros del panel `[C025]`/`[C024]` (líneas 45–56): ahí los valores son
   moduladores escalados por el panel (probablemente profundidad de modulación de tono).
3. **DSP4 (MAIN4)** lee su M `8005`–`8016` al inicio (líneas 7–24) y lo copia a `[220E]`–`[2219]`. Esas palabras se usan en
   operaciones MAC y multiplicaciones (líneas 81, 89, 291–293, 387, 401, 599). En la línea 293 aparece
   `mul … #0.9998364`: un suavizado de un polo, típico de un filtro o de una rampa, **no un VCA**.
4. **Pendiente**: identificar cuál de estas palabras es la envolvente de **amplitud**: la que multiplica el audio
   (los DCO) antes de IOMD. No está resuelto por el listado; requiere seguir las multiplicaciones que reciben el audio.

Lo que no se ha medido: la latencia exacta de cada tramo (la doble memoria de MXM sí está medida en DSP3↔DSP4) y el
valor real de cada palabra durante una nota.

## 4j. Salida de DSP4 y envolvente de amplitud (traza estática, 2026-10-21)

Traza desde la salida hacia atrás (`dsp/dump_main.py MAIN4`, líneas citadas del listado):

| paso | líneas | qué hace |
|---|---|---|
| `[6020]` ← `[02D0]` ← `[02CF]` × `[C000]` | 725–728 | salida izquierda: nivel de Roland (parámetro del panel) |
| `[02CF]` ← `[02CC]` × `[02CE]` (`[02CE]` = `[C04B]`) | 718–725 | mezcla × parámetro del panel |
| `[02B8]`/`[02B9]` = `[C049]`/`[C04A]`, `[02BA]` = 0 | 688–690 | ganancias de salida: **parámetros del panel** (nivel y paneo), no envolvente |
| `[02B2]` × `[029A]`, luego × `[029D]` | 659–662 | **dos multiplicaciones por envolventes** sobre la mezcla |

Las dos envolventes tienen la misma estructura (comparaciones con `cmp`, `min`/`max`, tasas `0.00359` y `0.003464`):
* `[029A]` (líneas 579–595): alimentada por `[A038]` (líneas 577–578) = **curva de velocidad** de `note_gate`.
* `[029D]` (líneas 596–600): alimentada por `[A039]` = **puerta** de `note_gate` (1.0 al pulsar).

**Hipótesis [inferida]**: la envolvente de **amplitud** es la de `[029D]`, porque es la que recibe la puerta. La de
`[029A]` escala con la velocidad. Se confirma con una prueba de audio (reemplazar sólo esa cadena por una versión con
puerta fija y ver que el volumen deja de seguir las notas). Pendiente.

## 4k. Pruebas de la envolvente de amplitud (2026-10-21)

Método: `MC_TEST=main4=<probe>` con el engine original (`C-PROBE`, `C-PROBE2`), una nota sostenida 2 s y soltada,
captura por USB. El probe cambia **una** instrucción de MAIN4 por otra del mismo tamaño (3 o 2 palabras).

| probe | cambio en MAIN4 | resultado |
|---|---|---|
| `C-PROBE` | `[029D]` forzado a 1.0 (`max r0,r0,#0` → `mov r0,#1.0`, 3 palabras) | el sonido se corta al soltar igual: `[029D]` no da la forma de la liberación |
| `C-PROBE2` | `[029A]` quitado de la cadena (`mul r0,r0,r1` → `mov r0,r0`, 2 palabras) | igual: el sonido se corta al soltar; `[029A]` tampoco |

Lo que muestran las capturas (ventanas de 20 ms):
* **Ataque y liberación instantáneos**: el nivel pasa de 0 a su valor completo, y de nuevo a 0, en unos 40 ms. No hay
  cola de liberación. Con el patch INIT, la envolvente de amplitud tiene tiempos de ataque y liberación en cero.
* **Latencia de ~0.5 s** entre el MIDI y el sonido, en los dos sentidos (la misma en ataque y liberación). No es una
  envolvente: es la cadena o la captura.
* **Conclusión**: ni `[029A]` ni `[029D]` son la envolvente de amplitud que se ve aquí; con el patch INIT ninguna
  envolvente muestra forma, así que estas pruebas **no pueden identificarla**. Hace falta un patch con tiempos de
  envolvente (ataque y liberación en el panel) para que la forma aparezca.
* La hipótesis de §4j queda **rechazada**: `[029D]` no es la amplitud.

Siguiente paso: un patch con ataque y liberación largos (panel) y repetir la captura con `C-SUB3` (sin probe) para ver
la forma real; después comparar cada candidata con esa forma.

### 4k-bis. Patch con envolvente (panel: attack 255, decay 255, sustain 0, release 0; 2026-10-21)

Con estos valores la forma aparece: **ataque lento** (rms 0.0002 a 1.5 s, 0.0032 a 5.5 s), mantenimiento al nivel
alcanzado y **liberación instantánea** (release 0). Medido con `C-PROBE2` (sin la multiplicación por `[029A]`): el ataque
**sigue** presente, así que `[029A]` no es la envolvente de amplitud. Pendiente: medir con `C-PROBE` (`[029D]` = 1.0);
si el ataque desaparece, `[029D]` es la de amplitud.

### 4k-ter. Resultado: `[029D]` es la envolvente de amplitud (2026-10-21)

Mismo patch (attack 255, decay 255, sustain 0, release 0), `C-PROBE` (`[029D]` = 1.0):

| medida | con Roland (`C-SUB3`, referencia) | con `[029D]` = 1.0 |
|---|---|---|
| ataque | lento: 0.0002 a 1.5 s → 0.0032 a 5.5 s | **instantáneo**: 0.0032 desde el primer medio segundo |
| liberación | instantánea (release 0) | instantánea |

[medido] Conclusión: `[029D]` es la **envolvente de amplitud** (VCA). Su cadena (MAIN4, líneas 596–600): puerta
`[A039]` → `[029C]` → `mac_p1p2c1` (tasa `0.003464`) → `max r0,r0,#0` → `[029D]`. Se multiplica en la mezcla de
salida (`[02B2]` × `[029A]` × `[029D]`, §4j). `[029A]` queda descartada como amplitud (§4k-bis).

Pendiente para reemplazarla dentro del flujo: reproducir la misma palabra `[029D]` con el mismo intervalo de muestra
y la misma escala, a partir de la puerta `[A039]` y los tiempos del panel (`[C0xx]`), sin cambiar el número de palabras
del programa (o ajustando los saltos).

### 4k-quater. Bloque de la envolvente de amplitud, simulado (2026-10-21)

Bloque original de MAIN4 (instrucciones 408–413 del listado, `programs.h` regenerado desde el plugin JX-3P sin
sustituciones), simulado con `re/dspisa.py` (puerta `[A039]` = 1.0 desde la muestra 100, anillo de `[029E]`):

* Ataque: `[029D]` sube de forma **exponencial**, 0.0069 a 101, 0.50 a 300, 0.956 a 1000 (constante ~0.00346, unas
  290 muestras). Liberación: 0.995 → 0.837 en 50 muestras (misma constante).
* Por tanto `[029D]` es un seguidor de un polo: `e ← e + k·(puerta − e)` con `k` fijo.

**Discrepancia con el equipo** (importante): con el patch de attack 255 el ataque medido dura ~4 s y es casi
lineal (0.0002 a 1.5 s, 0.0032 a 5.5 s). Ese tiempo **no** sale de este bloque (`k` fijo, puerta abrupta). Hipótesis
[inferida]: el tiempo lo pone una rampa del OS aplicada al parámetro del panel con `+0xA0` (`engine.c` usa
`p_ramp(…, ramp, …)` con tiempo de rampa) sobre una entrada de este bloque. Pendiente: confirmarlo antes de
reemplazar la envolvente, porque un reemplazo sin esa rampa no reproduciría el tiempo del panel.

### 4k-quinquies. Origen del tiempo de ataque (estático, 2026-10-21)

Confirmado en el listado de MAIN0 (`dsp/dump_main.py MAIN0`):
* Hay **dos** envolventes en MAIN0: ENV1 (ataque `C02F`, resto `C030`/`C031`/`C032`, estado `[024E]`, líneas 233–249) y
  ENV2 (ataque `C033`, resto `C034`–`C036`, estado `[025C]`, líneas 289–305).
* La envolvente de amplitud de DSP4 (`[029D]`, §4k-ter) recibe una **entrada constante**: `CONST` del modelo
  (`engine/model_spec.py`) escribe `B4:0x39` = 1. Por tanto **no** puede ser ella la que da el tiempo del panel.
* El tiempo del panel llega al sonido por una de las dos envolventes de MAIN0 (salidas MXM `8405`–`8416` → DSP2 → DSP4).
  **Pendiente**: cuál. Se mide moviendo el ataque de cada envolvente en el panel con `C-SUB3`.

#### Medida: origen del ataque lento (panel, 2026-10-21)

| panel | ataque observado |
|---|---|
| env1 = 255, env2 = 0 | **instantáneo**: 0.0032 desde 1.5 s; sin ataque lento |
| env1 = 0, env2 = 255 | **lento**: 0.0002 a 1.5 s → 0.0032 a 5.5 s, igual que con el patch original |

[medido] Con env2 en 0 el ataque lento desaparece aunque env1 esté en 255: **el ataque lento lo da env2** (ENV2 de MAIN0,
ataque `C033`). Confirmado con el caso contrario (env1 = 0, env2 = 255): el ataque lento vuelve. **El tiempo de ataque sale de env2 (ENV2 de MAIN0, `C033`)** [medido, dos sentidos].

**Contradicción abierta**: si env2 llega a DSP4 por `[029D]`, forzar `[029D]` a 1.0 debería dar el mismo efecto que env2
en 0, pero el constante `[A039]` = 1 daría un ataque de ~3 ms, no de 4 s. O la puerta `[A039]` no es constante, o el
ataque entra por otra palabra de DSP4. Sin resolver.

### 4k-sexies. Traza de ENV2 hasta la salida (2026-10-21)

1. **MAIN0**: ENV2 (bloque líneas 289–305 del listado) calcula el valor en `r0`; líneas 46–48 lo guardan en `[025B]` y en
   `[0261]`; `[0261]` sale por **MXM `840A`** (línea 618 del listado de MAIN0).
2. **DSP2**: relee `800A` y lo reenvía a su Z `840A` (líneas 13–14 del listado de MAIN2). Ese es el emparejamiento
   `800k` ↔ `840k` de §4i.
3. **DSP4**: carga `800A` en `[2212]` (línea 12). Desde ahí entra en un filtro de primer orden con coeficientes
   `0.9998364` / `0.9996728` y estado `[0254]` (líneas 291–294), luego en un bloque con límites (`min` 5.5, escala 0.6)
   que termina en `[025B]` de DSP4 (líneas 336–352). La multiplicación con el audio está más abajo; no la he seguido.

**Medida que separa filtro de volumen** (captura de `C-SUB3` con env1 = 0, env2 = 255, sin reinstalar):
| t (s) | rms | centroide espectral |
|---|---|---|
| 1.5 | 0.00021 | 2364 Hz |
| 3.0 | 0.00135 | 2379 Hz |
| 5.5 | 0.00321 | 2389 Hz |

El nivel sube 15 veces y el centroide no se mueve: **el ataque es de volumen (VCA), no de filtro**. Por tanto la
envolvente de amplitud es **ENV2 de MAIN0**, y su salida llega a DSP4 por MXM.

**Resuelto**: el bloque `[029D]` no es la amplitud del ataque (su entrada es constante y su ataque sería de ~3 ms).
**Pendiente**: el punto exacto de multiplicación en DSP4 (dónde el audio se multiplica por la envolvente de ENV2), y
por qué forzar `[029D]` a 1.0 eliminó el ataque en la prueba de `C-PROBE` (no explicado; la prueba puede haber
cambiado algo más de lo previsto).

### 4k-septies. Punto de multiplicación de ENV2 en DSP4 (2026-10-21)

Listado de MAIN4 (`dsp/dump_main.py MAIN4`):

| líneas | operación | qué es |
|---|---|---|
| 352–353 | `[025B]` ← ENV2 (resultado del filtro de §4k-sexies); luego `[025B]` × `[0243]` | ENV2 × ganancia `[0243]` |
| 354–356 | `[025C]` × `[0246]` → `[025D]` | × ganancia `[0246]` (escrita cerca de `[0298]`, sin identificar) |
| 665 | `[025D]` × `[02B4]` | **ENV2 multiplica la mezcla de salida** |

`[02B4]` = mezcla × `[029A]` × `[029D]` (§4j, líneas 659–662). Por tanto el VCA final es:

**salida = mezcla × `[029A]` × `[029D]` × ENV2 × `[0243]` × `[0246]`** (y luego × niveles del panel `[C000]`, `[C04B]`).

[inferido del listado, medido en el efecto de volumen de §4k-sexies]. Consecuencia: ENV2 es uno de varios factores. La
prueba de `[029D]` = 1.0 no debería quitar el ataque de ENV2; la contradicción de §4k-sexies sigue abierta.

**Pendiente**: origen de `[0243]` y `[0246]`, y la prueba de cada factor con el mismo patch de ataque para confirmar
la forma (ENV2 solo, sin los demás factores).

### 4k-octies. Pruebas de [0243] y [0246] (2026-10-21)

Método: `C-p0243` y `C-p0246` (`MC_TEST=main4=…`, MAIN4 original con la multiplicación que consume cada palabra
sustituida por `mov r0,r0`, mismo tamaño). Patch de ataque: env1 = 0, env2 = 255.

| probe | resultado (rms 1.5 → 6.0 s) |
|---|---|
| sin cambios (referencia) | lento: 0.0002 → 0.0032 |
| `[0243]` consumida = 1 | **plano**: 0.0033 desde 1.5 s |
| `[0246]` consumida = 1 | **plano**: 0.0032 desde 1.5 s |

**Interpretación insegura**: quitar cualquiera de las dos elimina la rampa. Eso no encaja con dos factores
independientes (quitar uno debería dejar la rampa del otro). Hipótesis: `[0243]` y `[0246]` forman parte de un mismo
lazo de realimentación (ambas leen estados que escribe el propio bloque), y sustituir una consumidora cambia el
comportamiento del lazo, no sólo un factor. **El método de quitar multiplicaciones no aísla estos factores.**

Lo que sí queda firme: la rampa del ataque pasa por DSP4 en el tramo ENV2 → `[0243]`/`[0246]` → `[025D]` → salida
(§4k-septies). Para separar los factores hace falta medir valores intermedios o reemplazar ENV2 en su origen (MAIN0).

### 4l. Nombres oficiales de las envolventes (plugin JX-3P, `plugins/JX-3P/Script/Script.xml`) (2026-10-21)

El XML del plugin (no usado hasta ahora; debió consultarse antes) define las secciones de parámetros:

| sección XML | parámetros de envolvente | papel |
|---|---|---|
| `AMP` | `ENV2 ATTACK`, `ENV2 DECAY`, `ENV2 SUSTAIN`, `ENV2 RELEASE` (y `BRILLIANCE`) | **envolvente de amplitud** |
| `FLT` | `ENV1 ATTACK`, `ENV1 DECAY`, `ENV1 SUSTAIN`, `ENV1 RELEASE`, `VCF ENV1 MOD` | **envolvente de filtro** |
| `PIT` | `PITCH ATTACK`, `PITCH DECAY`, `DCO ENV1 MOD` | envolvente de tono |

Coincide con la medida (§4k-quinquies): el ataque lento es **ENV2 = AMP**. En MAIN0, ENV2 lee `C033`–`C036` (§4k-quinquies)
y ENV1 lee `C02F`–`C032`. Los ids numéricos de cada campo no están en el XML (se deducen del orden dentro de la
sección); la correspondencia `C033` ↔ `ENV2 ATTACK` se confirma por la medida, no por el XML.

### 4m. Ids verificados de ENV2 (2026-10-21)

Verificación en tres vías, con `plugins/JX-3P/Script/Script.xml` (campos con nombre, rango y default), el INIT de
`model/params.py` y las rutas de `engine/model_spec.py`:

* **Índice del patch**: `id = 0x6A7 + índice` (en el orden de los valores del XML que tienen rango y default). Esta base
  coincide con los 35 valores del INIT (100 %).
* **Campos verificados**: `ENV1 ATTACK` = `0x6EF`, `ENV1 DECAY` = `0x6F0`, `ENV1 SUSTAIN` = `0x6F1` (default 255, INIT 255),
  `ENV1 RELEASE` = `0x6F2`; **`ENV2 ATTACK` = `0x6F4`**, `ENV2 DECAY` = `0x6F5`, `ENV2 SUSTAIN` = `0x6F6` (default 255,
  INIT 255), `ENV2 RELEASE` = `0x6F7`.
* **Rutas en el modelo** (`model_spec.py`, líneas 158–200): el id del patch `0x6F4+stage` escribe en los parámetros del
  DSP `0x33+stage` (`C033` ataque, `C034` decay, `C035` sustain, `C036` release). Para ENV1: `0x6EF+stage` → `0x2F+stage`.
* **MAIN0** lee `C033`–`C036` en el bloque de ENV2 (líneas 289–305), con el mismo orden. Coincide.

Nota de método: un primer recuento dio `ENV2 ATTACK` en el índice 81 porque la expresión regular incluía nombres de
sistema sin rango ni default. El recuento correcto usa nombre, rango y default a la vez.

### 4n. Cómo obtener los parámetros e ids del plugin y mapearlos al código (2026-10-21)

Cadena completa, de un nombre del panel a una instrucción del DSP:

| paso | fuente | herramienta / comando | qué da |
|---|---|---|---|
| 1. nombre, rango, default, sección | `plugins/JX-3P/Script/Script.xml` (`<structType>` → `<value>`) | `dsp/patch_ids.py` | "ENV2 ATTACK", rango 0–255, default 0, sección `AMP` |
| 2. id del patch | base `0x6A7` + índice del XML; base verificada contra el INIT de `model/params.py` (35/35) | `dsp/patch_ids.py` | `ENV2 ATTACK` = `0x6F4` |
| 3. qué parámetro del DSP escribe ese id | `engine/model_spec.py`, líneas 158–200: `out(pid, ramp(targets), expr)` | leer `out(...)` con ese id | `0x6F4+stage` → `0x33+stage` (`C033`…`C036`) |
| 4. dónde lo lee el DSP | listados del programa: `dsp/dump_main.py MAIN0` (buscar `[c033]`) | `grep "\[c033\]"` en el listado | MAIN0 líneas 289–305 |
| 5. qué hace el DSP con él | el mismo listado, con sus stores y cargas siguientes | seguir el registro hasta `st` a MXM o IOMD | ENV2 → `[0261]` → MXM `840A` |

**Convenciones que hay que recordar**
* `C0nn` en el listado = parámetro del DSP `0xnn` (escrito por el ARM con `+0xA0`, con rampa, o por el modelo).
* `A0nn` = banco 5 del DSP (escrito por el ARM con `+0xB0`, p. ej. puerta `0x22`, tono `0x21`).
* `[8nnn]` / `[84nn]` = ventana MXM **local** del DSP (no es la misma palabra en otro DSP; ver §4i).
* `[6nnn]` = IOMD del BMC.
* En el XML los ids dependen del orden: si el XML cambia (otra versión del plugin), recalcular la base con
  `dsp/patch_ids.py` antes de confiar en ningún número.

**Verificación mínima antes de usar un id**: (a) la base reproduce el INIT; (b) el default del XML coincide con el
INIT o con el valor que el modelo usa; (c) la medida del equipo cambia como dice la sección del XML (p. ej. el
ataque de `AMP` = ENV2, §4k-bis).

**Herramientas de este documento**: `dsp/patch_ids.py` (ids), `dsp/dump_main.py` (listados con referencias de bus),
`dsp/make_bmap.py` y `dsp/analyze_bmap.py` (medida de palabras de salida), `dsp/sim_main.py` (simulación de un programa
completo: **todavía no validada**, las salidas de MAIN0 salen en cero; no usar para conclusiones hasta validarla).

### 4o. Simulador de MAIN0 y puerta de ENV2 (2026-10-21, sin validar)

`dsp/sim_main.py` ejecuta MAIN0 completo (con saltos y anillo). Resultados:
* Con **todas** las entradas de banco 5 que lee MAIN0 (`A020`–`A04F`) en 1.0, `[0261]` (ENV2) sale 0.02. Antes salía 0
  porque las entradas estaban a cero: el cero anterior no era un fallo del programa.
* Búsqueda de una sola entrada a 1.0 (`A020`–`A03F`, 200 muestras): **sólo `A022`** hace responder `[0261]` (0.02).
  `A022` es la puerta de la voz 0 en el engine (`p_bank(u, 0x22 + 0x12·k, gate)`, `engine.c`).
* **Pero**: con `A022` a 1.0 desde la muestra 100 (no desde el inicio), `[0261]` sigue en 0 hasta la muestra 259.
  Por tanto el 0.02 depende del estado inicial y **no** es una respuesta a la puerta. El simulador no reproduce todavía
  la envolvente y **no** sirve para confirmar la puerta.

Pendiente: confirmar `A022` en el equipo (una prueba de MAIN0 que anule esa lectura y mida si el ataque de ENV2 desaparece
en las notas de la voz 0), y corregir el simulador (estado inicial de los estados de ENV2) antes de usarlo.

### 4p. Prueba en el equipo de la puerta de ENV2 (2026-10-21, no concluyente)

Probe `C-G22`: en MAIN0 la única lectura de `A022` (palabra 44, copia la puerta a `[0207]`) se cambia por `A02B`
(constante 1.0 del engine). Resultado con una nota (69) de 3 s:
* Antes de la nota ya hay sonido de fondo: rms 0.00214 constante (la envolvente está abierta).
* Con la nota sube a 0.0038–0.0043 y **al soltar vuelve al fondo**: la nota sigue apagándose.

Conclusión: el encendido y apagado de la nota **no** pasa sólo por `A022` → `[0207]`. La prueba no confirma que `A022`
sea la puerta de ENV2 y no la descarta. Pendiente: encontrar el otro camino de la puerta (otra lectura de `A022` o
de `A03x`/`A02x` en DSP2/3 por MXM) antes de identificar la puerta de ENV2.

## 4l. Diagnóstico del mudo y de la voz de `mini` (2026-10-21)

### 4l.1 Error de diagnóstico corregido: voz equivocada

Las variantes de diagnóstico `C-D4`, `C-GT3`, `C-MO`, `C-MO1` y `C-MO2` usaron `dsp/voice.hex` (voz monofónica que lee
`[A021]`/`[A022]`). El motor `mini/` escribe la nota en el banco 5 de DSP5 con los ids `0x60+lv` y `0x68+lv`, que
lee `dsp/poly.hex` (`mini/build.sh` copia `poly.hex` a `voice.txt`). Con `voice.hex` la voz nunca recibía nota, así que
su silencio no dice nada sobre los DCO de DSP2/3. Las conclusiones previas sobre un "DCO2 de Roland necesario" eran
falsas y quedan anuladas.

Microcódigo de `dsp/poly.s`: lee solo su propia memoria, el banco 5 de DSP5 (`[A060]`–`[A06B]`) y el estado; escribe
solo `[6020]` y `[6021]`. No lee IOMD ni MXM, así que no depende de lo que escriben los DCO de DSP2/3. [código]

### 4l.2 Tabla de variantes en el equipo

| variante | DSP2/3 DCO1 | DSP2/3 DCO2 | principales DSP0..5 | voz DSP5 | motor | resultado |
|---|---|---|---|---|---|---|
| C-MINI2 | Roland | Roland | Roland ×6 | `poly.hex` | `mini` | suena con notas; no rompe |
| C-D4 | Roland | Roland | Roland ×5, DSP4 constante | `voice.hex` (incorrecta) | `mini` | sin nota en la voz; no rompe |
| C-GATE, C-GT0/2/3 | nuestro o vacío | vacío o nuestro | vacíos ×6 | `voice.hex` o ninguna | `gate` | **rompen a todos** |
| C-MO, C-MO1, C-MO2 | nuestro o Roland | nuestro o vacío | Roland ×6 | `voice.hex` (incorrecta) | `mini` | sin nota en la voz; no rompen |
| C-MO3 | nuestro | nuestro sin salida | Roland ×6 | `poly.hex` (correcta) | `mini` | **suena: 48, 57, 64, 72, 76 afinadas; no rompe** (rms 3.6× menor que C-MINI2, sin explicar) |
| C-M4 | Roland | Roland | Roland ×5, DSP4 constante | `poly.hex` | `mini` | suena con algunas notas; no rompe |
| C-M4E | Roland | Roland | vacíos ×5, DSP4 constante | `poly.hex` | `mini` | **suena: 60, 64, 67, 72 afinadas; 48, 52, 55, 57, 76 no suenan; no rompe** |

### 4l.3 Qué está medido y qué no

* **Mudo total:** todas las variantes que rompen a los demás tienen el **programa principal de DSP4 vacío**. Ninguna
  variante con el de DSP4 ocupado (constante o Roland) rompe. Es un patrón fuerte; la causa exacta no se ha aislado.
  Además, el esqueleto `gate/` no llama a `send_state`, y eso también difiere de `mini/`. [medido por correlación]
* **DSP0, DSP1, DSP2, DSP3 y DSP5 con el principal vacío** (C-M4E) no rompen el sistema. [medido]
* **Nuestro oscilador en DSP2/DSP3** (DCO1 y DCO2 sin salida) no rompe y no impide que suene la voz de DSP5. [medido, C-MO3]
* **La rampa de nivel del DCO** (`+0xA0`) es un bloque de parámetros del hardware, no lo ejecuta el programa de DCO
  (`docs/CUSTOM_PLUGOUTS.md`). Descartada como explicación del silencio de la voz.
* **Nivel:** `C-MO3` suena unas 3.6 veces más bajo que `C-MINI2` (rms 0.0031 frente a 0.0111). No explicado.
* **Notas silenciosas:** el mismo patrón de notas que suenan y no suenan en `C-MINI2` aparece en `C-M4E`. Es el
  comportamiento de asignación de voces que se dejó fuera de este análisis.

### 4l.4 Qué falta medir

1. Aislar qué principal de DSP4 necesita el sistema (¿el valor que escribe? ¿su programa?).
2. Probar `send_state` en el esqueleto `gate/` y ver si el mudo desaparece sin ningún principal de Roland.
3. Medir la ruta `DSP2 Z 0x404 → DSP4 M 0x005` con una constante (no el SAW).
4. Construir el plug-out sin Roland en el arranque: principal de DSP4 con la puerta, DCO nuestro en DSP2, resto vacío.

## 4q. C-SOLO con filtro de voces por `first`: sólo suenan 4 notas (2026-10-21)

Cambio en `gate/engine_solo.c` (`e_param`): cada slave acepta sólo sus cuatro voces, `v = id − 0x429 − first` y
`v = id − 0x43A − first`, con descarte fuera de 0–3 (igual que `mini/engine_mini.c`). Compilado con
`gate/build_solo.sh` e instalado como `C-SOLO` en el slot 3 (`tools/plugout_tool.py install --force`, PLG3).

Resultado **[reportado por el usuario al oído, no medido con captura]**: suenan sólo 4 notas.

Lectura [inferida de §4e, no medida]: las voces 0–3 son las del slave 0, que es el que llega al audio. Las voces 4–7
son del slave 1, que no tiene ninguna palabra IOMD `6020`–`60FF` que llegue a la salida. El filtro no crea el mudo:
lo deja al descubierto. Antes de este cambio, las ocho notas sonaban en el slave 0 porque el slave 0 las calculaba
todas, y el filtro sólo cambia qué notas llegan a él.

**Grabación de C-SOLO (2026-10-21, `mini/play_test.py`, 16 notas de 48 a 74, 1.5 s cada una) [medido]:**

| nota | resultado |
|---|---|
| 48, 50 | mudas |
| 52, 53, 55, 57 | suenan (+0.45 a +0.49 st) |
| 59, 60, 62, 64 | mudas |
| 65, 67, 69, 71 | suenan (+0.46 a +0.49 st) |
| 72, 74 | mudas |

Suenan ocho notas en dos bloques de cuatro, no cuatro. Lectura [inferida]: el asignador reparte las notas por voz
global (ver abajo) y los bloques que suenan son los de las voces del slave 0. El desfase de +0.47 st no coincide con la
afinación exacta de §4l; queda sin explicar.

**Asignación de voces en el ARM de Roland [medido en el desensamblado]:** el asignador está en la región 0 del maestro
(`base-engine/elf-asm/p1y01_p0m.region0.S`, `p1y01_p0m_r0_init`, bucle desde `0x61718514`). Por cada voz `v` de una
máscara envía `0x429+v` (nota), `0x42D+v`, `0x43A+v` (velocidad) y `0x43E+v`. Los ids son globales (0–7): no hay ids
por slave. El slave filtra por `first = 4·slave`. Nuestro `model/model.c` envía `0x429+v`, `0x43A+v`, `0x44B+v` y
`0x45C`; no envía `0x42D+v` ni `0x43E+v`, cuyo significado no está identificado.

Lo que sigue abierto: la ruta del slave 1 hasta el audio (§4e).

**Prueba de búsqueda en `6000`–`601F` (C-S1W, 2026-10-21) — resultado: el slave 1 no llega al audio por `6000`–`601F`.**

Medida [medido, 12 s de `arecord -t raw`, PLG3 seleccionado]: **440 Hz en los dos canales** (pico 70.9 dB; su segundo
y tercer armónico en 880 y 1320 Hz, como un SAW). **660 Hz ausente**: pico 13.1 dB, igual que el suelo en 550 Hz
(13.3 dB), unos 58 dB por debajo del 440. Lo que el slave 1 escribe en `6000`–`601F` no sale por el audio. No excluye
que el slave 1 no esté ejecutando su voz (ver lo que sigue).

Lo que esto deja sin decidir: si el DSP5 del slave 1 corre (tarea o `swap`) o si su salida llega al maestro por
una palabra fuera de `6000`–`60FF`. La prueba de §4e ya excluía `6020`–`60FF`, así que `6000`–`60FF` completa queda
descartada para la salida del slave 1 en las condiciones de esta prueba.

Diseño de la prueba:
* Build: `gate/diag/build_s1w.sh` (engine `gate/diag/engine_s1w.c` = copia de `mini/engine_mini.c` con un drone fijo por
  slave: 440 Hz en el slave 0, 660 Hz en el slave 1, puerta siempre abierta, sin notas).
* Programa de voz: `dsp/make_voice.py` con `DRONE=1 ALLOUT=6000-601F`: la misma señal en las 32 palabras `6000`–`601F`,
  además de `6020`/`6021`.
* Criterio: si aparece **660 Hz**, el slave 1 llega al audio por una palabra de `6000`–`601F`. Si aparece sólo 440 Hz,
  el slave 1 no llega por ninguna palabra de `6000`–`60FF` y la ruta está en el maestro (fuera de estas palabras).
* Efecto colateral: `build_s1w.sh` sobrescribe `re/dspexp/own/voice.txt` (el archivo que lee `mini/build.sh`); `mini/build.sh`
  lo regenera al compilar.

**Pendiente para la siguiente sesión (2026-10-22):**
1. Decidir entre la causa A (el DSP5 del slave 1 no corre: tarea o `swap`) y la causa B (corre, pero su salida no llega
   al maestro por ninguna palabra de `6000`–`60FF`). Propuesta: en el slave 1, escribir un valor conocido en un banco
   que sólo lea su ARM (con `dsplive`, o un drone que cambie de frecuencia al recibir su nota).
2. Sin medir: si el slave 1 recibe su pitch (`p_bank(5, 0x21)`) en C-S1W; no lo verificamos, así que la ausencia de
   660 Hz también podría deberse a que el tono no se fijó.
3. Pendiente de la comparación con Roland: significado de `0x42D+v` y `0x43E+v` en el asignador de la región 0.
4. Sin resolver: desfase de +0.47 st en C-SOLO frente a la afinación exacta medida antes (§4l).
5. No commit: el repo tiene cambios sin commitear en `docs/WAVE_BUS.md`, `dsp/make_voice.py`, `gate/engine_solo.c`,
   `gate/diag/` (nuevo), `plugins/C-S1W` y `plugins/C-SOLO`. Hay además un `osc_src/NOTES.md` sin seguimiento que no creé.

## 5. Por qué 6020 sonaba

Medido (mapa de canales, §4): `6020` llega al canal 1 de la captura y `6021` al canal 0. La prueba anterior de 880 Hz
sólo escribía `6020`, por eso sonaba un solo canal. Lo que sigue siendo hipótesis (ver §4b: la sobrescritura por MAIN4 no lo explica):

IOMD es compartida por los seis DSP del BMC (`DSP_ARM_SIDE.md`). Si nuestro DSP5 escribe `6020` y DSP4 también la
escribe (MAIN4), el valor visible es el último escrito en la muestra. El tono de 880 Hz limpio indica que, en esa
prueba, DSP4 no estaba escribiendo algo que lo sobrescribiera. **[inferido]**: el orden de ejecución entre DSP4 y
DSP5 dentro de la muestra no está medido. Pendiente: medirlo (`dsp/make_bmap.py`, §6).

## 6. Pendiente (en orden)

1. **Slot de DSP5 frente a MAIN5** (§4c): trazar un camino de datos desde una entrada que MAIN5 lee hasta una palabra
   audible, para observar el orden sin depender de `60B0`–`60B7`.
2. **`6024`**: confirmar o descartar la contribución de 330 Hz (un lote con `6024` a otra frecuencia).
3. **Estéreo**: por qué `6020` y `6021` van a canales distintos y `6063` a los dos. Mapear con las notas.
4. **Qué slave llega a qué palabra** (los dos slaves ejecutan el mismo programa; hay que distinguirlos).
5. **Origen de `[2204]`–`[221D]`**: qué valores escribe DSP4 en MXM `8206`/`820D`/`8213`/`8214`.
6. **Tuning**: el SAW propio suena 52 cents por encima del de Roland (proporción 0.516 constante; no cambia con el
   mix ni con el fine tune del OSC2; DCO1 y DCO2 usan el mismo programa).
7. **Notas**: voz estéreo en `6020` (canal 1) y `6021` (canal 0) con puerta desde el ARM, y comprobar cuáles notas
   suenan y por qué canal.

## 7. Cómo reproducir

```
python3 examples/from_scratch/dsp/dump_main.py MAIN5 > main5.txt   # listado de referencias de bus
python3 examples/from_scratch/dsp/make_bmap.py                      # programa de mapa de canales
```

## 8. Historial

* 2026-10-21 (método): documentado cómo obtener ids y mapearlos al código (§4n); dsp/patch_ids.py; sim_main.py sin validar.
* 2026-10-21 (ids): ENV2 = patch 0x6F4–0x6F7, DSP C033–C036 (verificado en XML, INIT y modelo; §4m).
* 2026-10-21 (XML): el plugin define ENV2 = AMP (amplitud), ENV1 = FLT (filtro), PIT (tono) en Script.xml (§4l).
* 2026-10-21 (factores): quitar [0243] o [0246] aplana el ataque; el método no aísla factores en lazo (§4k-octies).
* 2026-10-21 (VCA): salida = mezcla × [029A] × [029D] × ENV2 × [0243] × [0246] × niveles del panel (§4k-septies).
* 2026-10-21 (volumen): el ataque es VCA (centroide constante); la envolvente de amplitud es ENV2 de MAIN0 (§4k-sexies).
* 2026-10-21 (origen del ataque): medido con env2 = 0: sin ataque lento; el ataque lento lo da env2 (ENV2 de MAIN0, C033).
* 2026-10-21 (bloque): la envolvente de amplitud simulada es un seguidor de un polo con k fijo (0.00346); el tiempo del
  panel (~4 s) no sale de este bloque: hipótesis de rampa del OS (§4k-quater).
* 2026-10-21 (amplitud): envolvente de amplitud = [029D] (MAIN4), confirmada: al fijarla a 1.0 el ataque desaparece (§4k-ter).
* 2026-10-21 (probes): [029D] y [029A] no son la envolvente de amplitud (la forma no cambia; con INIT no hay forma). Latencia
  MIDI→sonido ~0.5 s (§4k).
* 2026-10-21 (salida DSP4): VCA = mezcla × [029A] × [029D]; candidata a amplitud [029D] (puerta A039) (§4j).
* 2026-10-21 (memoria compartida): no existe; MXM (doble buffer), IOMD (dueño), banco 5 y bus de onda; traza
  MAIN0 → DSP2 → DSP4 de la envolvente (§4i); amplitud aún sin identificar.
* 2026-10-21 (afinación): el desfase de ~52 cents era de la escala del fine tune del OSC2; con −128 queda ~7 cents (§4h).
* 2026-10-21 (regla de flujo): la cadena de Roland no se cambia; `C-SUB2` (SAW propio en DCO1/DCO2) suena en las ocho notas (§4g).
* 2026-10-21 (polifonía): cuatro voces por slave (simétrico); dos notas a la vez suenan en el slave que llega a la salida (§4f).
* 2026-10-21 (notas): las notas que no sonaban eran descartadas por el filtro de voces del engine; con las ocho voces en
  el slave 0, las ocho notas suenan con afinación exacta.
* 2026-10-21 (slave 1): drone por slave: el slave 1 no llega a ninguna palabra IOMD `6020`–`60FF` (§4e).
* 2026-10-21 (estéreo): voz en `6020` + `6021`: los dos canales llevan la misma señal; las notas que suenan dependen
  de la asignación de voces entre slaves (no determinista entre ejecuciones).
* 2026-10-21 (OS): binario del OS: base IOMD igual en los seis DSP; `sync` espera la trama del DSP; el orden de tareas
  no está en estas rutinas (§4d).
* 2026-10-21 (madrugada): orden de ejecución: slot de DSP5 después de MAIN4 (medido por supervivencia en `6021`/`6063`);
  slot frente a MAIN5 sin medir (§4c).
* 2026-10-21 (noche): barrido de las 38 palabras IOMD de Roland (§4b): `6020`→canal 1, `6021`→canal 0, `6063`→ambos;
  `60B0`–`60B7` en silencio. Corrección: la hipótesis de sobrescritura por MAIN4/MAIN5 no explica el silencio de
  `6022`–`6024`, `6060`–`6066` (MAIN4 las escribe igual que `6021`/`6063`).
* 2026-10-21 (tarde): mapa de canales en `6020`–`6024`/`6060`–`6062`: `6020` = canal 1, `6021` = canal 0.
* 2026-10-21 (C-SOLO con filtro `first`): instalado; el usuario reporta que sólo suenan 4 notas (§4q).
* 2026-10-21 (C-S1W): drone fijo 440 Hz slave 0 / 660 Hz slave 1 con la voz en `6000`–`601F`: 440 Hz en los dos
  canales, 660 Hz ausente (−58 dB sobre el suelo). El slave 1 no llega al audio por `6000`–`60FF` (§4q).
* 2026-10-21: primera versión. Se documentan MAIN5 y MAIN4 (listado estático), la prueba IOMD y la prueba del engine
  propio. Antes de este documento la correspondencia de palabras IOMD estaba marcada como [desconocido] en
  `DSP_HARDWARE.md`; ahora está parcialmente medida.
