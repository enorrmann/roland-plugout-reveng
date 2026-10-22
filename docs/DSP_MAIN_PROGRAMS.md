# Programas principales de los DSP (MAIN0–MAIN5): qué son y dónde corren

Estado: 2026-10-21 (correcciones del 2026-10-22 marcadas; flujo completo en `VOICE_ALLOCATION.md` §6.2). Descripción de los seis programas principales del microcódigo de voz del JX-3P (PLUG-OUT de
Roland), con qué parte de la voz hace cada uno y en qué DSP corre. Las etiquetas: **[medido]** = observado en el
equipo o por audio; **[inferido]** = deducido del listado estático de Roland, sin prueba en el equipo;
**[abierto]** = sin resolver.

## 1. Qué es cada programa

Un "programa principal" es el código que corre en cada muestra (96 kHz) en un DSP y que llama a las ranuras de
efecto u oscilador. Los programas de ranura (DCO1, DCO2, efectos) se cargan aparte con `+0x70` y los llama el
principal por medio de la mailbox `22nn` (ver `CUSTOM_OSC.md` §2).

| programa | bytes | palabras (orden de ejecución) | función [inferido salvo indicación] |
|---|---|---|---|
| MAIN0 | 0x13CA | 2533 | envolventes ENV1 y ENV2, LFO (constantes del render del VST en `NOTES.md` "ISA del DSP"), tono de la voz; sus salidas van por MXM (`8405`–`8416`) a DSP2 |
| MAIN1 | 0x139A | 2509 | igual que MAIN0 para la segunda voz del par (A/B con otras direcciones) |
| MAIN2 | 0x0814 | — | DSP2 (osciladores): reenvía lo que lee de MXM (`8005`…) a su Z `8405`…; escribe la sincronía y el metal de DCO2 (`[2209]`, `[220a]`, `[220b]`) [medido por audio] |
| MAIN3 | 0x07E2 | — | DSP3: como MAIN2, para el otro par de voces del slave [inferido] |
| MAIN4 | 0x1436 | — | DSP4 (filtro y VCA): copia `8005`–`8016` a `[220E]`–`[2219]`; la envolvente de amplitud `[029D]`; **escribe la salida a IOMD** `6020`–`6024` y `6060`–`6066` (incondicional, al final) [medido, `WAVE_BUS.md` §3] |
| MAIN5 | 0x1398 | — | DSP5: filtro/VCA de las voces 2 y 3; las entrega a DSP4 por el buzón de pareja `A400`–`A403`, que MAIN4 suma a la salida [listado, `VOICE_ALLOCATION.md` §6]; además copia a IOMD `60B0`–`60B7` (silenciosas [medido]) |

Los tamaños y las palabras de MAIN0/MAIN1 se comprobaron con el SHA-256 de `engine/programs.py`
(coinciden con el PLUG-OUT JX-3P 1.01, `plugins/JX-3P/Script/Code8_1.Dat`).

## 2. Dónde corre cada uno

Un BMC esclavo tiene seis DSP (DSP0..DSP5) que comparten IOMD y MXM con sus vecinos. El reparto de la voz es:

| DSP | programa principal | ranura de oscilador | ranura de efecto | rol |
|---|---|---|---|---|
| DSP0 | MAIN0 | — | — | envolventes, LFO, tono (en el listado) |
| DSP1 | MAIN1 | — | — | envolventes, LFO (segunda pareja de voces) |
| DSP2 | MAIN2 | DCO1 `0x6000`, DCO2 `0xB000` | — | osciladores, voces A y B |
| DSP3 | MAIN3 | DCO1 `0x6000`, DCO2 `0xB000` | — | osciladores, voces A y B |
| DSP4 | MAIN4 | efecto en `0xB000` (PLUG-OUT) | — | filtro y VCA, salida a IOMD |
| DSP5 | MAIN5 | — | — | voces 2 y 3 → DSP4 por `A400`–`A403`; copia a IOMD `60B0`–`60B7` |

Fuente: `docs/ARCHITECTURE.md` (tabla de slaves) y `docs/WAVE_BUS.md` §2–§3 (listados de MAIN4/MAIN5).

**Qué no está resuelto:**
- ~~**[abierto]** En qué esclavo corre MAIN0 y en cuál MAIN1~~ **Resuelto 2026-10-22:** los dos slaves corren los seis
  principales; MAIN0 en el DSP0 y MAIN1 en el DSP1 de **cada** slave (voces locales 0–1 y 2–3) [código `e_load`;
  medido con C-SAWPOLY y C-V4, `VOICE_ALLOCATION.md` §6].
- **[abierto]** El camino completo de la envolvente de amplitud: ENV2 de MAIN0 → DSP2 → DSP4 (`[0261]` → MXM `840A`
  → `[029D]`), con una contradicción señalada en `WAVE_BUS.md` (§4k-ter, línea 348).
- **[abierto]** Por qué sólo algunas palabras de salida llegan al audio (`WAVE_BUS.md` §5).

## 3. Envolventes y LFO dentro de MAIN0/MAIN1

Localización en el listado de MAIN0 (`dsp/dump_main.py MAIN0`, numeración de instrucciones de ese listado):
- ENV1: ataque `C02F`, decay/sustain `C030`/`C031`, release `C032`, estado `[024E]`, líneas 233–249.
- ENV2: bloque en las líneas 289–305; ataque `C033`, salida en `r0` que se guarda en `[025B]` y `[0261]`
  (líneas 46–48); `[0261]` sale por MXM `840A` (línea 618).
- LFO: rate `A028` ← `0x3EF`/`0x6CF`, forma `A02C` ← `0x6CD`, delay `A02D` ← `0x6CE` (`NOTES.md` "ISA del DSP").
- Salidas de envolventes y LFO a MXM `8405`–`8416` (líneas 613–636).

Experimentos previos (`WAVE_BUS.md` §4k–§4p): con env2 = 0 desaparece el ataque lento aunque env1 = 255, y al revés
vuelve; el ataque lento lo da ENV2 [medido, dos sentidos].

## 4. Cómo reproducir el listado

```
cd examples/from_scratch/engine && python3 -c "..."   # extrae main0/main1 de work/jup8/region1.bin o del
                                                       # region1.bin desempaquetado de plugins/JX-3P
cd examples/from_scratch/re && python3 dspdis.py main0.bin > main0.dis
```

`engine/programs.py` comprueba el SHA-256 de cada programa antes de usarlo, así que el listado sale del mismo binario
que se instala. `re/dspmc.py` `words()` da las palabras en orden de ejecución (mitad alta primero).

## 5. Qué toca cada cosa al cambiar un programa

- `MC_TEST=main0=fichero` (y `main1=`, …) sustituye el programa en `engine/programs.py`; el tamaño registrado sigue
  al fichero. Un programa más largo que el registrado no se ejecuta entero (`engine/build.sh`).
- Regla vigente del usuario (2026-10-21): cada componente conserva el DSP y la ranura que Roland le asignó (oscilador
  en DSP2/3, LFO y envolvente en DSP0/1, filtro y VCA en DSP4/5); se reemplaza el programa en su mismo DSP y ranura, y
  no se mueve ni se reúne con otra función (`docs/WAVE_BUS.md` §4g). Un intento de desactivar ENV/LFO sin tocar MAIN0/MAIN1
  (dejar el código, no usarlo) va primero; reemplazar MAIN0/MAIN1 es el paso siguiente, no el primero.
