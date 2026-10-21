# Bus de audio entre los DSP del SYSTEM-8 (slaves → maestro)

Documento vivo: cada hallazgo lleva su estado y la prueba que lo sostiene. Se actualiza a medida que avanzan las
pruebas (última revisión: 2026-10-21). Mapa de memoria general: `DSP_ARM_SIDE.md`; ISA: `DSP_ISA_SPEC.md`;
arquitectura general: `ARCHITECTURE.md`. Este documento es el de **por dónde sale el audio**.

Etiquetas: **[medido]** = observado en el equipo o por audio USB; **[desconocido]** = sin medir;
**[inferido]** = deducido de código de Roland (listado estático) sin prueba en el equipo.

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
| 2026-10-21 | DSP5 DCO1 escribe SAW fijos en `60B0` (440), `60B1` (660), `6020` (880) | sólo **880 Hz en el canal derecho** de la captura (armónicos 1/n). `60B0` y `60B1` no llegan | [medido] (`MINI_PLUGOUT.md` §4) |
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

**Regla (indicación del usuario, "muy importante")**: no cambiar la cadena de DSP de Roland. El flujo lo controla el OS:
DSP0/1 (envolventes, LFO, tono) → DSP2/3 (osciladores) → DSP4/5 (filtro y VCA) → IOMD → bus de onda. Sólo se reemplazan
los programas de oscilador (ranuras DCO1/DCO2) y, después, el resto de programas uno a uno, siempre dentro de ese flujo.

Por qué: el engine `mini` (voz sola en DSP5, ranuras DCO vacías en DSP2/3) perdió notas y polifonía, y no tiene
sentido seguir reemplazando la cadena. `engine/engine.c` y `build.sh` quedan sin cambios.

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
* 2026-10-21: primera versión. Se documentan MAIN5 y MAIN4 (listado estático), la prueba IOMD y la prueba del engine
  propio. Antes de este documento la correspondencia de palabras IOMD estaba marcada como [desconocido] en
  `DSP_HARDWARE.md`; ahora está parcialmente medida.
