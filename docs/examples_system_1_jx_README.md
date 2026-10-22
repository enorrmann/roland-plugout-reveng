# C-S1JX: el microcódigo del JX-3P en el SYSTEM-1, un DCO, 4 voces (`examples/system_1_jx/`)

Estado: 2026-10-22. **Construido y probado sin hardware** (emulador DSP + Unicorn). **Sin instalar en el equipo.**
Es un PLUG-OUT de prueba pedido por el usuario: usa **el código DSP de Roland** (los programas del JX-3P, leídos en
la build desde `plugins/JX-3P`) y nuestro código ARM. Por eso `build/` y `plugins/C-S1JX/` llevan bytes de Roland:
**no se suben al repo** (la regla de [VOICE_ALLOCATION.md](VOICE_ALLOCATION.md) §7 sigue valiendo para nuestros
PLUG-OUTs propios; este es la excepción pedida).

Leyenda: **[medido]** en el equipo · **[emulador]** · **[unicorn]** (ARM emulado) · **[código]** · **[inferido]** ·
**[abierto]**.

## 1. Uso

```sh
python3 examples/system_1_jx/test_s1jx.py       # microcódigo: 7 pruebas (≈ 10 s)
sh examples/system_1_jx/build.sh                # -> plugins/C-S1JX/Script/Code1.Dat (y Code.Dat)
python3 examples/system_1_jx/test_arm.py        # código ARM de las dos regiones: 4 pruebas (≈ 1 min)
python3 tools/plugout_tool.py install plugins/C-S1JX --slot 1 --force --no-patches -y   # sustituye al PLUG-OUT del slot 1
```

Necesita `plugins/JX-3P` (JX-3P 1.01; `engine/programs.py` comprueba los SHA-256), `arm-none-eabi-gcc` y Python
con `unicorn` para `test_arm.py`. Para volver: `plugout_tool.py install plugins/SH-2 --slot 1 --force -y` (o
`plugins/C-S1V8`).

Prueba en el equipo propuesta (no hecha): seleccionar el PLUG-OUT sin tocar teclas; tocar una nota (la primera va a
la voz 3, que suena en el esclavo), luego un acorde de cuatro (voces 0/1 en el maestro, 2/3 en el esclavo); mover
VOLUME. Sonido esperado: el INIT PATCH del JX-3P con DCO1 en SAW, filtro abierto, envolvente de órgano.

## 2. Por qué así

`system-1/DSP_FIT.md`: el JX-3P completo (6 DSP del SYSTEM-8, 4 voces) necesita 3748 instrucciones por muestra y
el SYSTEM-1 tiene ≈ 3660 en sus 4 DSP: no cabe. **Sin DCO2** son 3098 y caben (`dsp_fit.py --dco2 none`). Los
controles de OSC2 no hacen nada (sus salidas se leen como 0).

## 3. Reparto (una cadena de dos voces por chip)

| DSP del SYSTEM-1 | dónde | programas (orden) | instrucciones/muestra [emulador] |
|---|---|---|---|
| maestro A | `0x0100` | MAIN2 + relevo + MAIN4 (un programa) | 719 (+ el programa de salida del OS en `0x82`) |
| maestro B | `0x1000` | MAIN0 + relevo + DCO1 (voces 0, 1) + cola nuestra | 854 |
| esclavo A | `0x0100` / sección `0x1900` | MAIN5 / MAIN3 + relevo | 697 |
| esclavo B | `0x0100` / sección `0x1900` | MAIN1 / relevo + DCO1 (voces 2, 3) | 839 |

En el SYSTEM-8: DSP0/2/4 = voces 0 y 1, DSP1/3/5 = voces 2 y 3. Aquí los pares van a chips distintos: maestro =
voces 0/1, esclavo = 2/3. Un principal del esclavo cabe en 6 KiB (`0x0100`–`0x18FF`); el segundo programa va a la
sección `0x1900`, encendida con `FC44(0, 1)` (A) y `FC44(2, 1)` (B), como hacen SH-2 y PROMARS. En el maestro se
concatenan (se quita el `0b02 00f8 0001` del primero; los saltos son relativos).

Sets que se encolan, con el mismo patrón que los PLUG-OUTs de Roland del SYSTEM-1 [código]: cabecera de trama en
`0x0000` (`A01D` maestro A, `A004` maestro B / esclavo A, `A005` esclavo B; la ensamblamos nosotros, mismas palabras),
programas, finales de sección vacíos (`0x6DF2` maestro A; `0x5FF2`, `0x6FF2` maestro B) y cierre en `0x7FF2`.

## 4. Cómo se transforma el microcódigo (`s1jx.py`, `dspreloc.py`)

Sólo se reescriben **direcciones literales** (todas las transferencias de estos programas son de modo `fe`; no hay
modos indexados, registros ocultos ni `X28Y`); ningún largo cambia. `dspreloc.Prog` usa el decodificador del emulador
(`dspisa.Layout`) para saber en qué media palabra está cada dirección.

| qué usaba en el SYSTEM-8 | en el SYSTEM-1 |
|---|---|
| banco 0 y banco 1 (anillos) por DSP | un rango propio por programa en su chip, separados ≥ `0x20` (alcance máximo del anillo: 16) — vale tanto si A y B comparten esos bancos como si no |
| banco 5 propio (`A020`–`A045`: parámetros del ARM y estado) | un bloque por programa en el banco 5 del chip (compartido por A y B), `A020`–`A1FF` sin `A118`–`A11F` (salida del maestro) |
| MXM: `Z 84nn` → `M 80nn` / `O 82nn` del DSP+2; `S 86nn` de vuelta | palabras de buzón en el banco 5 del chip |
| ranura DCO ↔ su principal por el banco 1 (`2201`… entradas, `221b`/`221f`/`2223`/`2227` salidas, leídas a +1) | buzón en el banco 5; las salidas de **DCO2** que lee MAIN2/3 → `ZERO` |
| ventana de pareja `A4xx` (DSP0→1 7 palabras, DSP4↔5 4+4) | IOMD entre chips: 11 hacia el esclavo (`600E`…), 4 hacia el maestro (`6006`…) |
| IOMD de entrada de fuera del slave (`60e0`–`60e2`, `6000`–`6006`, `6010`–`6017`) | `ZERO` [abierto: en el SYSTEM-8 vienen de otro chip; ver §7] |
| IOMD de salida: MAIN4 `6020`/`6021` | `OUT_L`/`OUT_R` en el banco 5 del maestro → cola nuestra en el maestro B: × `C01C` (VOLUME) → `A118`/`A119` |
| otras salidas IOMD (`6022`…, `6060`…, `60B0`…) | `DUMP` |
| banco 6 (`Cnnn`) por DSP | `+0x50` a MAIN4 (comparte el maestro A con MAIN2 y el OS usa `C002`–`C004`); el resto igual |

`build/plan.txt` lista cada palabra y su dueño.

### 4.1 El relevo de registros (`r2`, `r3`)

En el SYSTEM-8 cada ranura DCO corría justo después de su principal, en el mismo DSP. El DCO1 empieza con un paso de
Newton (`newton r2, …`) que **lee `r2`/`r3` antes de escribirlos**: con la semántica del emulador depende de los
valores que deja MAIN2 [emulador, `test_scramble`]. En el SYSTEM-1 el DCO corre en el otro DSP del chip, después de
otro programa. Solución (código nuestro, sin tocar el de Roland): al final de MAIN2/MAIN3 `st r2`, `st r3` a dos
palabras del buzón; al principio del DCO1 `ld r2`, `ld r3`. `test_handoff` comprueba que las salidas del DCO son
iguales bit a bit a las del orden del SYSTEM-8 aunque en medio corra un programa que cambie todos los registros.
Si en el equipo esa dependencia no existiera, el relevo no hace daño.

## 5. Puesta en marcha: lo que hace falta además del microcódigo

Lo que el SYSTEM-8 ponía por su cuenta y aquí tiene que poner el PLUG-OUT (encontrado al poner en marcha la cadena
en el emulador):

| qué | por qué | dónde |
|---|---|---|
| **el modelo de parámetros completo** (`model_full.py`) | el `engine/model_spec.py` actual no tiene salidas de filtro (`0x6EA` corte, `0x6EC` resonancia, `0x6F3`, `0x6F8`, `0x2DB`/`0x7E4`…) ni de envolventes (`0x6EF`–`0x6F7`); se quitaron para experimentos (§8). Sin ellas el corte queda en 0 y la cadena es muda | `examples/system_1_jx/model_full.py` = `model_spec.py` del commit `013d3fe` |
| **valores de los parámetros**: INIT PATCH | en el SYSTEM-1 el OS no tiene los ids del JX-3P (`0x6CD`…); los ponemos fijos: los `default` del `Script.xml` (`re/jxpatch.py`), el resto a 0 | `s1jx.default_vals()` → `jx_defaults` |
| **`0x7E4` y `0x74D`** (parámetros float del OS del SYSTEM-8) | `0x7E4` es el corte que el JX-3P escribe el último; a 0 cierra el filtro. `0x74D` es la velocidad del LFO | iguales a `0x6EA/255` y `0x6CF/255` |
| **`P4:0x4A` = 1** | nivel de salida de MAIN4: lo pone `engine.c` en `unmute`, no el modelo | `jx_send_state`, `s1jx.ENGINE_EXTRA` |
| **cabeceras, finales de sección y cierres** de los DSP | el patrón de todos los PLUG-OUTs del SYSTEM-1 | `s1jx.frame_header`, `make_s1.close`/`empty_section` |
| **secciones del esclavo encendidas** | el OS las apaga al cargar | `FC44(0, 1)`, `FC44(2, 1)` en `e_load` |
| **relevo `r2`/`r3`** | §4.1 | `Plan._handoff` |
| **VOLUME** | `0x100` → `C01C` del maestro B, (v/255)² | `set_volume` |
| **estado en las dos regiones** | cada región evalúa todo el modelo y escribe sólo lo de su chip; se reenvía todo en `load`, en `refresh` (esclavo) y antes de la primera nota | `jx.h` |

Las rampas (`+0xA0` del SYSTEM-8) se escriben como valor inmediato (`FCEC`): el servicio con rampa del SYSTEM-1
(`FCE4`) no está mapeado. Puede oírse "zipper" al mover parámetros; hoy ninguno se mueve (§7).

## 6. Pruebas (independientes; no necesitan el equipo)

`test_s1jx.py` (microcódigo):

| prueba | qué demuestra |
|---|---|
| `identity` | todas las transferencias se decodifican y la reubicación identidad devuelve las mismas palabras |
| `units` | cada programa original contra el reubicado, solo en un DSP, entradas externas iguales y aleatorias: salidas iguales **bit a bit** durante 3000 muestras (los 8 programas) |
| `scramble` | cada programa no depende de registros/banderas que deje otro programa (puede compartir DSP) |
| `handoff` | §4.1; y falla si se quita el relevo (comprobado) |
| `memory` | rangos de anillo, ids de banco 6, palabras de banco 5 e IOMD sin solapes |
| `sizes` | cada programa cabe en su zona |
| `budget` | instrucciones por muestra ≤ 925 en cada DSP (maestro A ≤ 885) |

`test_arm.py` (código ARM en Unicorn, servicios del OS simulados): sin instrucciones VFP; los sets encolados son los
de `s1jx.py`; tras `load` las 205 palabras de cada chip coinciden con el modelo en Python (≤ 0,1 %) y no se escribe
nada más; cuatro notas usan las cuatro voces y cada chip escribe las suyas.

`s1jx.py --check` (audio de la cadena del SYSTEM-8 contra la del SYSTEM-1 en el emulador) hoy sale **inconcluso**:
ver §7.

El modelo completo se comprobó contra el motor del JX-3P con `engine/test_model.py` (sin tocarlo, importando
`model_full` como `model_spec`): **0 diferencias** en cambios (72 fuentes × 4 contextos × 2 slaves) y en arranque;
error máximo de aproximación 2 %.

## 7. Riesgos y abierto

| # | qué | cómo se vería | estado |
|---|---|---|---|
| 1 | el emulador no reproduce el filtro de MAIN4 (corte 0 → silencio; 200 → 511; 230 → 0,001; ver [EMULATOR.md](EMULATOR.md) §5.2) | — | por eso no hay prueba de audio; las pruebas de equivalencia no dependen de ello |
| 2 | presupuesto del SYSTEM-1 distinto de 925 | ruido o equipo inestable (apagar y encender) | [abierto]; maestro B al 92 % |
| 3 | zonas de programa del maestro (`0x0100`–`0x1D50` en A, `0x1000`–`0x342C` en B) | mudo o cuelgue | [inferido] de los finales de sección de Roland |
| 4 | secciones del esclavo (`FC44`) | voces 2/3 mudas o sin DCO | [código], usado por SH-2/PROMARS |
| 5 | latencias: lo que en el SYSTEM-8 iba por MXM (1 muestra) ahora va por el banco 5 (0 o 1) o IOMD (≥ 1) | cambios pequeños de fase/tiempo | [abierto], sin efecto esperado audible |
| 6 | entradas IOMD a 0 (`60e0`–`60e2` de MAIN0: LFO u otra señal del maestro del SYSTEM-8) | LFO u otra modulación ausente | [abierto] |
| 7 | MAIN0 lee `2201` = su propio `2200` de la muestra anterior (en el SYSTEM-8, el de MAIN1 de hace dos) | — | [inferido] equivalente |
| 8 | ningún control del panel está mapeado a ids del JX-3P (salvo VOLUME); los de OSC2 tampoco hacen nada | el panel no cambia el sonido | pendiente: mapear con PARAMLOG (`docs/examples_system_1_README.md` §6) y `jx_param` |
| 9 | primera nota al principio: el estado se reenvía antes de la primera nota | — | — |

## 8. Lo que se eliminó del modelo de `engine/` y dónde está

`examples/from_scratch/engine/model_spec.py` perdió partes en experimentos del 2026-10-21 (detalle en
[examples_from_scratch_engine_README.md](examples_from_scratch_engine_README.md), "Removed from the model"):

| commit | qué se quitó | para qué |
|---|---|---|
| `b50213f` "pyc" | ENV-2 (amplitud) y su `GATE` de VCA Mode | C-NOENV: la VCA toma la puerta de la nota |
| `0396d20` "env1" | ENV-1 (todas las envolventes) | ídem |
| `b440296` "mini" | VCF: corte, resonancia, env/LFO/key mod, bend/aftertouch al filtro | mini (sin filtro) |

La versión completa es la de `013d3fe` y **funciona** contra el JX-3P (§6). Con el `model_spec.py` actual,
`engine/test_model.py 4` da 92 (fuente, destino) distintos y 84 ids distintos en el arranque. C-S1JX usa su
propia copia (`model_full.py`) y no toca `engine/`.

## 9. Ficheros

| fichero | qué |
|---|---|
| `examples/system_1_jx/s1jx.py` | plan (ubicación, memoria), transformación, `build/s1jx.h`, `build/model.h`, `build/plan.txt`, `--check` |
| `examples/system_1_jx/dspreloc.py` | referencias a memoria de un programa DSP y su reubicación; `concat` |
| `examples/system_1_jx/model_full.py` | el modelo de parámetros completo (copia de `013d3fe`) |
| `examples/system_1_jx/jx.h` | modelo + voces + remapeo de escrituras, común a las dos regiones |
| `examples/system_1_jx/model.c` / `engine.c` | región 0 (maestro: asignador, carga, VOLUME) / región 1 (esclavo: carga, secciones, voces) |
| `examples/system_1_jx/build.sh` | build completa (`NAME`, `MODEL_ID` = `0x0011`) |
| `examples/system_1_jx/test_s1jx.py`, `test_arm.py` | §6 |

Usa `examples/system_1/syninfo.h`, `link.ld` y `make_s1.py` (cierres) de C-S1V4 sin cambiarlos.
