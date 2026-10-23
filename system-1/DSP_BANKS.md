# Bancos de memoria de los DSP del SYSTEM-1: qué hay, qué se espera y qué usan los programas oficiales

Especificación de estudio. Para cada banco de datos: su semántica, qué palabras usan el OS (1.30) y los PLUG-OUTs
oficiales del SYSTEM-1 (SH-2, SH-101, PROMARS) en cada DSP, y las reglas que seguimos en C-S1SY.

Fuentes:
* `work/sys1_v130/app_{master,slave}.bin` (base `0x60000000`) y `work/unpack/<NOMBRE>_S1_Code1/region{0,1}.bin`;
* los programas de cada set con `system-1/s1_progs.py` (`sets`, `words`), desensamblados con `asm/disasm.py`;
* rangos de transferencias `ld`/`st` de modo `fe` contados por programa (2026-10-23).

Leyenda: **[código]** · **[medido]** · **[inferido]** · **[abierto]**. Estado: 2026-10-23.

## 1. Bancos y semántica

| banco | direcciones | semántica | quién lo escribe | estado |
|---|---|---|---|---|
| 0 | `0000`–`07FF` (11 bits) | **anillo** por DSP: lo guardado en S se lee en S+k k muestras después ([WAVE_BUS.md](WAVE_BUS.md)); también por índice `ldidx` (`hset h6`) | el propio DSP | [medido]: fase de las sierras (S/S+1, C-S1V4); **línea de retardo de 750 muestras con carga normal y con `ldidx`: el mismo peine** (C-S1SY15X, captura USB, §4) |
| 1 | `2000`–`23FF` | anillo como el 0; buzón entre principal y secciones de voz; **en el maestro B, envío/retorno del efecto del OS** ([FX_CHAIN.md](FX_CHAIN.md) §4) | DSP A / B del mismo chip | [medido] para el efecto (C-S1SY13) |
| 2 | `4000`–`5FFF` | ventana DMA de la **memoria de retardo** (`tapreq`, `h4`/`h5`) | el efecto del OS en el maestro B, el contador del esclavo | [código] |
| 3 | `6000`–`7FFF` | **IOMD**: entre DSP y entre chips; `6000`/`6001` salida, `6006`–`600D` esclavo → maestro, `600E`–`6021` maestro → esclavo | — | [medido] `6006`/`6007`, `6000`/`6001` |
| 5 | `A000`–`A3FF` (+ `A400`–`A7FF` ventana de pareja) | memoria simple; el ARM la escribe con `FD6C` (`sel` 0 → `A0xx`, `sel` 1 → `A1xx`); los DSP A y B del esclavo comparten `A1xx` | ARM y DSP | [medido] (C-S1SY: `A030`–`A06F` notas, `A151`–`A1FF` transporte A → B) |
| 6 | `C000`–`C1FF` | **parámetros** por DSP, de sólo lectura para el DSP; el ARM los escribe con `FCEC(sel, id, bits)` (bloques de 16 bytes en `0x4014C000` / `0x4016C000 + 16·id` del maestro) | ARM | [medido] |
| 7 | `E000`–`E0FF` | contadores / estado del hardware: `E000` contador de trama (los cierres `7FF2`), `E013`, `E01A`, `E020`, `E021` (el efecto y el arranque del OS) | hardware | [código] |

## 2. Qué usan los programas oficiales [código]

`ld`/`st` de modo `fe` por banco (rangos agrupados), sin contar las transferencias indexadas:

### 2.1 Maestro

| programa | banco 0 | banco 1 | banco 2 | IOMD | banco 5 | banco 6 |
|---|---|---|---|---|---|---|
| OS, efecto delay + reverb (set 12, `0x0`) | `0000`–`0033` | `2000`–`2002` | `4001`–`402D` | — | `A000`–`A003` | `C000`–`C00C` |
| OS, salida (set 11, `0x82`) | — | `2000`–`2008` | — | st `6000`–`6001` | ld `A118`–`A119` | `C001`–`C004` |
| OS, modulación de serie (set 14, `0x100`) | `0018`–`016A` | — | — | st `600E`–`6021` | ld `A038`–`A048`, st `A100`–`A117` | `C008`–`C068` |
| OS, mezclador de serie (set 15, `0x1000`) | `007C`–`0153` | `2000`–`2003` | — | ld `6006`–`600D` | st `A118`–`A11D` | `C01C`–`C040` |
| SH-2 / SH-101 / PROMARS, principal A `0x100` | `0018`–`00C7` | ld `200A`–`2012`, st `2013`–`2023` | — | ld `6006`–`602D`, st `600E`–`6021` | `A005`–`A15D` | `C008`–`C032` |
| SH-2 / SH-101 / PROMARS, principal B `0x1000` | `007C`–`013F` | `2000`–`2003` (envío / retorno del efecto) | — | ld `6006`–`600D` | st `A118`–`A11D` | `C01C`–`C037` |
| C-S1SY (paso 13), principal B `0x1000` | — | `2000`–`2003` | — | ld `6006`/`6007` | st `A118`–`A11D` | `C01C` |

### 2.2 Esclavo

| programa | banco 0 | IOMD | banco 5 | banco 6 |
|---|---|---|---|---|
| OS, principal / cierres (sets 0, 2) | `0000`–`000D` | st `6006`–`600D` | `A000` | `C000`–`C005` |
| OS, voces de serie (secciones `0x1900`/`0x3100`/`0x4900`/`0x6100`) | `00E0`–`00FC`, `01A8`–`01C0`, `0270`–`028C`, `0338`–`0350` (una zona por sección) | — | — | — |
| SH-2 principal A `0x100` | `0018`–`0047` | ld `600E`–`601E`, st `6009`–`600D` | `A003`–`A143` | `C003`–`C006` |
| SH-2 secciones `0x1900` / `0x3100` | `00E0`–`0118` / `01A8`–`01E5` | — | — | `C058`–`C05C` / `C062`–`C065` |
| SH-101 principal A `0x100` (todo en el principal) | `0018`–`009C` | ld `600E`–`6021` | `A003`–`A115` | `C003`–`C00F` |
| PROMARS principal A / sección `0x1900` | `0018`–`0084` / `00E0`–`0135` | — | — | `C003`–`C013` / `C058`–`C05C` |
| C-S1SY A `0x100` | voces `0300`–`037F`, ruido `0240`–`028D` | — | `A030`–`A06F`, `A151`–`A1FF` | `C044`–`C065` |
| C-S1SY B `0x100` | voces `0400`–`04FF`; paso ≥ 14: `0080`–`0095`, línea `0100`–`02FD` | st `6006`–`600D` | `A151`–`A1FF` | `C040`–`C05A` |

Observaciones:
* Todos los programas guardan su estado del banco 0 por encima de `0018` (lo de abajo es del OS: `0000`–`0017`).
  Cada sección de voz tiene su zona propia.
* El banco 1 del maestro B (`2000`–`2003`) sólo lo tocan el mezclador (de serie o el del PLUG-OUT) y el efecto:
  es el envío/retorno. En el esclavo, el buzón de las secciones de voz ([VOICE_ALLOCATION.md](VOICE_ALLOCATION.md)
  §3.2).
* Ningún programa oficial del SYSTEM-1 usa `hset h6` / `ldidx` en el maestro. El SYSTEM-100 del SYSTEM-8 los usa
  para leer señales de control retrasadas.

## 3. Reglas para nuestros programas

1. Estado del banco 0 por encima de `0017`. **Nada guarda en S+1 … S+kmax de una línea de retardo** (el anillo:
   cualquier guardado en ese rango pisaría muestras de la línea).
2. En el maestro B, el banco 1 `2000`–`2003` es el envío/retorno del efecto del OS: escribir el envío en
   `2000`/`2002`, leer el retorno en `2001`/`2003` ([FX_CHAIN.md](FX_CHAIN.md) §4).
3. Parámetros nuevos: en un rango libre del DSP (C-S1SY: maestro B `C01C`; esclavo B `C040`–`C05A`; esclavo A
   `C044`–`C065`).

## 4. Medida: la línea de retardo y `ldidx` en el esclavo B [medido, 2026-10-23]

C-S1SY15X (`CHDIAG=1`):
* el esclavo B guarda la mezcla en `0100` cada muestra;
* L = (x + `[0100 + 750]`)/2, leída con una carga normal del anillo;
* R = (x + toma)/2, la misma toma leída con `mov r5, #(0x100 + 750 + 0,5)/16384` + `hset h6, r5` + `ldidx r2, 0`.

Captura USB (`arecord -D hw:SYSTEM1,0 -f S32_LE -r 96000 -c 2`) de La2 (110 Hz):
* L = R muestra a muestra;
* armónicos 1–12: 1,00 0,36 0,10 0,05 0,14 0,17 0,15 0,13 0,09 0,03 0,02 0,05. Una sierra por el peine de 750
  muestras da 1,00 0,35 0,09 0,06 0,13 0,16 0,16 0,13 0,08 0,03 0,02 0,05.

**El anillo del banco 0 guarda al menos 750 muestras y `ldidx` lee lo mismo que la carga normal.**
