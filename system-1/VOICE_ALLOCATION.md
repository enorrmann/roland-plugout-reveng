# SYSTEM-1: asignación de voces (qué DSP hace qué)

Estado: 2026-10-22. Es el equivalente de `docs/VOICE_ALLOCATION.md` (SYSTEM-8) para el SYSTEM-1. Fuentes: OS 1.30
(`work/sys1_v130/app_master.bin`, `app_slave.bin`), los PLUG-OUTs SH-2, SH-101 y PROMARS para SYSTEM-1 (sólo
leídos) y nuestro C-S1V4 (`examples/system_1/`, `docs/examples_system_1_README.md`). El camino del audio entre
chips está en [WAVE_BUS.md](WAVE_BUS.md).

Leyenda: **[medido]** en el equipo · **[código]** visto en los binarios · **[inferido]** · **[abierto]**.

## 1. Resumen

| | SYSTEM-8 | SYSTEM-1 |
|---|---|---|
| voces | 8: cuatro por slave, dos slaves | **4 de serie, un solo slave** (una sola imagen de esclavo y una región 1) [código]; un PLUG-OUT puede usar **8** [medido, C-S1V8] |
| DSP por BMC | 6 (DSP0–DSP5) | **2 (A y B)** por BMC: colas `0x0100FC30` (A) y `0x0100FC34` (B) [código] |
| id de nota / velocidad | `0x429+v` / `0x43A+v` | **`0x110+v` / `0x120+v`** (grupo 2), disparo `0x128+v` [código], [medido] con C-S1V4 |
| dónde se calcula la modulación por voz | slave (DSP0/1) | **maestro**: un programa del maestro manda 20 palabras por IOMD al esclavo (§3.2) [código] |
| oscilador | ranuras DCO1/DCO2 de DSP2/3 | **secciones de voz** `0x1900`…`0x6100` de los DSP A y B del esclavo (§3.1) [código] |

## 2. Del teclado a la voz

```
tecla / MIDI ─► OS del maestro ─► asignador (de serie, o el de la región 0 del PLUG-OUT)
                                    └─ set(2, 0x110+v, nota), set(2, 0x120+v, velocidad) ─┬─► OS del maestro (DSP del maestro)
                                                                                          └─► esclavo: motor vt+0x30(id, 0, valor)
```

* **Asignador de serie**: objeto en `0x604A26B0` (puntero en `0x600094F4`), código `0x60009500`–`0x6000A400`. Cuatro
  voces fijas: bucles `cmp #4`, índices `& 3`, máscara de voces de un byte (bits 0–3). Modos según el parámetro
  `0x192` del grupo 3 [código] (detalle y sitios en [VOCES_6.md](VOCES_6.md) §3–4).
* **Ids de voz** (grupo 2): `0x110+v` nota, `0x120+v` velocidad (0 = soltar), `0x128+v` disparo (1/0), `0x130`
  bend, `0x131` modulación, `0x134` expresión. Los bloques tienen hueco para 16, 8 y 8 voces [código].
* **En modo PLUG-OUT** el OS usa el asignador que devuelve `SYNINFO+0x2C` de la región 0 (vtable de 9 entradas:
  `reset`, `changed`, `note_off`, `note_on`, `cc`, `part 0x81`, `bend`, dos vacías) [código]. El SH-2 es mono (sólo
  la voz 0) [código]; C-S1V4 es poly de cuatro voces y manda sólo nota y velocidad [medido: notas y acordes suenan].
* **El esclavo recibe los ids de voz**: su despachador (`0x6004CE64`) llama al motor con cada id cambiado de los
  grupos 0–3, sin decirle el grupo [código]; el motor de serie lee `get(2, 0x120..0x123)` en su refresco [código];
  C-S1V4 usa `0x110+v` / `0x120+v` en el esclavo [medido].
* **El modelo del maestro** (`SYNINFO+0x30`, vt+0x30) recibe también los parámetros (panel, sistema). Así se encontró
  el VOLUME = id `0x100` (`docs/examples_system_1_README.md` §6) [medido].

## 3. Sinte de serie: reparto por DSP

### 3.1 Esclavo [código]

Memoria de programa de cada DSP: cabecera `0x0000`, principal `0x0100`, secciones de voz `0x1900`, `0x3100`,
`0x4900`, `0x6100` (`0x1800` bytes cada una), cierre `0x7FF2` (`docs/examples_system_1_README.md` §2.4).

| DSP | principal | secciones |
|---|---|---|
| A | `0x600185B0` (241 instrucciones): lee la modulación de cada voz del maestro (IOMD `600E`–`6021`) y `A006`–`A009` (del ARM), prepara las entradas de las secciones en el banco 1, recoge sus salidas y pasa datos y audio a B por `A100`–`A10D` | 4 programas de voz, uno por voz |
| B | `0x60018F44` (237 instrucciones): lee `A100`–`A10D`, alimenta y recoge sus secciones (banco 1) y escribe las ocho salidas `6006`–`600D` × `C000` | 4 programas de voz, uno por voz |

Los programas de voz vienen en sets (48 de voz, en parejas idénticas, 24 variantes, `voice_sets.py`); los cuatro de un
set sólo cambian en las direcciones de datos de cada voz. Índice del servicio `FC40`/`FC44` → sección
([SH101_POLY.md](SH101_POLY.md) §2):

| índice | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|---|---|---|---|---|---|---|---|---|
| DSP | A | A | B | B | A | A | B | B |
| sección | `0x1900` | `0x4900` | `0x1900` | `0x4900` | `0x3100` | `0x6100` | `0x3100` | `0x6100` |

Palabras del banco 1 de cada voz (`2nnn`; el principal escribe en la muestra n y la sección lee en la n, la salida
vuelve una muestra después, como en el anillo del SYSTEM-8):

| voz | escritas por el principal de A | escritas por el principal de B | leídas de vuelta |
|---|---|---|---|
| 0 | `2004`–`2007` | `2000`–`2003` | `2005`, `2007`, `2009` |
| 1 | `200E`–`2011` | `200A`–`200D` | `200F`, `2013` |
| 2 | `2014`–`2017` | `2010`–`2013` | `2015`, `2017`, `2019` |
| 3 | `201E`–`2021` | `201A`–`201D` | `201F`, `2023` |

Que A y B escriban en el mismo banco 1 sin pisarse indica que los dos DSP de un BMC comparten la base del banco 1
(como DSP0/DSP1 en el SYSTEM-8) [inferido]. Qué es cada palabra (tono, nivel, forma) está [abierto].

### 3.2 Maestro [código]

* `0x60097A7C` (696 instrucciones): lee del ARM los datos de cada voz en el banco 5 (`A038`–`A048`), calcula la
  modulación y escribe **20 palabras por IOMD hacia el esclavo** (`600E`–`6021`) y otras 20 por el buzón de la pareja
  (`A100`–`A113`). Es decir, en el SYSTEM-1 las envolventes/LFO por voz se hacen en el maestro [inferido del
  código]. En qué DSP (A o B) corre está [abierto].
* `0x60095B0C` (58 instrucciones; variantes `0x600959DC`, `0x60095A24`): mezclador. Suma las ocho salidas del esclavo
  `6006`–`600D` con ganancias `C005`–`C00E`, filtro de continua, × `C00F` × `C010` → `6000`/`6001`. El OS fija
  `C00F` y `C010` con el nivel del patch y un desplazamiento de parte (ids de patch `0x16D`/`0x169` y de parte `0xB5`; `0x60021020`, `0x60021078`) [inferido de qué id lleva a cada función].
* En modo PLUG-OUT el OS cambia el maestro (§4 y [WAVE_BUS.md](WAVE_BUS.md) §2).

## 4. Los PLUG-OUTs del SYSTEM-1

| | voces | esclavo A | esclavo B | maestro |
|---|---|---|---|---|
| SH-101 | 1 | todo el sinte (575 instrucciones) en el principal; las 8 secciones apagadas | 12 instrucciones: `A10A`–`A10C` × `C000` → `6006`–`600A` | sus propios programas en A y B |
| SH-2 | 1 | principal + secciones `0x1900` y `0x3100` encendidas (`FC44(0,1)`, `FC44(4,1)`) | 62 instrucciones: niveles → `6006`–`600B`, `6026`–`602D` | A: modulación (367 instr.), B en `0x1000`: mezcla, volumen (`C01C`…`C037`), saturación → `A118`/`A119` |
| **C-S1V4** | **4** | las 4 voces (137 instrucciones) en el principal; secciones apagadas | `A150` × `C000` → `6006`/`6007` | sólo B en `0x1000`: `6006`/`6007` × `C01C` (VOLUME) → `A118`/`A119` |

Voces de C-S1V4 [código], [medido a oído]:

| voz | nota (ARM → banco 5 del esclavo) | puerta | fase (banco 0, anillo) | sale por |
|---|---|---|---|---|
| 0 | `A030` | `A038` | escribe `0300`, lee `0301` | suma × 0.25 → `A150` → DSP B → `6006`/`6007` |
| 1 | `A031` | `A039` | `0310` / `0311` | ídem |
| 2 | `A032` | `A03A` | `0320` / `0321` | ídem |
| 3 | `A033` | `A03B` | `0330` / `0331` | ídem |

El ARM escribe nota y puerta con `FD6C(0, {0, 0, 0x30+v / 0x38+v}, float)`; la nota va en octavas,
`(nota − 12) / 12` [medido: afinado a oído, sin medir con instrumento].

### 4.1 Ocho voces (C-S1V8) [medido, a oído]

El mismo diseño con `NV = 8`: asignador poly de 8 en la región 0, ids `0x110`–`0x117` / `0x120`–`0x127`, tono
`A030`–`A037` y puerta `A038`–`A03F` en el banco 5 del esclavo, las ocho sierras en el principal del DSP A (269
instrucciones). Ocho notas sostenidas suenan todas: **el OS reenvía al esclavo los ids de las voces 4–7** igual que
los de 0–3, aunque el sinte de serie sólo use cuatro. Detalle: `docs/examples_system_1_README.md` §7.

## 5. Abierto

* Qué es cada palabra del banco 1 por voz y cada una de las 20 de modulación `600E`–`6021`.
* En qué DSP del maestro corren el programa de modulación y el mezclador de serie.
* ~~Si el OS entrega más de 4 voces a un asignador de PLUG-OUT~~: **sí, 8** (C-S1V8, §4). Más de 8: el bloque de
  velocidades se acaba en `0x127` [abierto].
* Medida por voz (qué voz suena con cada nota) como `voicelog` del SYSTEM-8: no hecha; la captura de parámetros
  (`examples/system_1/paramlog.py`) es la herramienta para hacerlo.
