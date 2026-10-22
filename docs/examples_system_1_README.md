# C-S1V4: el plug-out propio C-V4 portado al SYSTEM-1 (`examples/system_1/`)

Estado: 2026-10-22. **C-S1V4 funciona en el equipo** [medido, a oído]: notas sueltas, acordes de cuatro notas y
el VOLUME del panel. Antes, C-S1TONE comprobó la carga y la cadena esclavo → maestro → salida; el VOLUME no actuaba
hasta que la captura de parámetros (§6) mostró que es el id `0x100`. Afinación sin medir con instrumento.

**C-S1V8 (8 voces) también funciona** [medido, a oído, 2026-10-22]: ocho notas sostenidas suenan todas, sin cortar
ninguna. El SYSTEM-1 de serie tiene 4 voces; el OS reenvía al esclavo los ids de las voces 4–7 sin cambios (§7). Es el C-V4 del SYSTEM-8
([FROM_SCRATCH.md](FROM_SCRATCH.md)) reescrito para la interfaz de PLUG-OUT del SYSTEM-1: cuatro voces de sierra
con puerta, polifónicas, todo el código ARM y todo el microcódigo nuestros (ningún byte de Roland; los programas de
Roland sólo se leyeron para estudiar la interfaz).

Etiquetas: **[código]** = visto en los binarios (OS 1.30, SH-2/SH-101/PROMARS para SYSTEM-1); **[sim]** = comprobado
con el simulador `re/dspisa.py`; **[inferido]**; **[abierto]**. Nada está **[medido]** todavía.

## 1. Uso

```sh
sh examples/system_1/build.sh                          # -> plugins/C-S1V4/Script/Code1.Dat (y Code.Dat)
TONE=1 NAME=C-S1TONE sh examples/system_1/build.sh     # prueba de arranque: la voz 0 suena siempre (La)
VOICES=8 NAME=C-S1V8 sh examples/system_1/build.sh     # 8 voces de polifonía (medido: funciona)
python3 examples/system_1/make_s1.py --sim             # simula los tres programas (unos 3 minutos)

python3 tools/plugout_tool.py info                     # SYSTEM-1: slot 1 = SH-2 hoy
python3 tools/plugout_tool.py install plugins/C-S1V4 --slot 1 --force --no-patches -y
```

El SYSTEM-1 tiene un solo slot: instalar C-S1V4 **sustituye al SH-2**. Se recupera con
`python3 tools/plugout_tool.py install plugins/SH-2 --slot 1 --force -y`.

Orden propuesto para la primera prueba (una instalación cada vez, como en el SYSTEM-8):

1. `C-S1TONE` (TONE=1): debe sonar un La continuo al seleccionar el PLUG-OUT, sin tocar teclas. Comprueba la carga
   de los programas, la cadena de audio esclavo → maestro y el programa del maestro. Si no suena, el fallo está en
   §2.2/§3.3, no en las notas.
2. `C-S1V4`: notas sueltas y acordes de hasta cuatro notas. Comprueba que el esclavo recibe `0x110+v`/`0x120+v` y
   la escritura del banco 5 (§3.4).
3. Afinación: medir con un afinador o con la captura USB. La sierra es la de C-V4, calibrada en el SYSTEM-8 a 96 kHz
   **[abierto]**: si el SYSTEM-1 usa otra frecuencia de muestreo, todo saldrá desplazado el mismo intervalo.

## 2. Qué es distinto del SYSTEM-8

### 2.1 Un PLUG-OUT del SYSTEM-1

`Code1.Dat` con 2048 registros (`tools/plugout_image.py` ya lo maneja; `pack` de un SH-2 desempaquetado da el mismo
fichero byte a byte). Dos regiones, en direcciones fijas (un solo slot, sin reubicaciones):

| región | carga | corre en | C-S1V4 |
|---|---|---|---|
| 0 | `0x60800000` | maestro | `model.c`: SYNINFO, asignador de 4 voces, objeto modelo que carga el DSP B del maestro, 64 patches vacíos en `+0x1000` |
| 1 | `0x60200000` | esclavo | `engine.c`: SYNINFO, motor que carga los DSP A y B del esclavo y escribe tono y puerta |

Los BMC del SYSTEM-1 no tienen FPU (ningún binario usa VFP) **[código]**: se compila para Cortex-M3 con
`-mfloat-abi=soft` y los valores en coma flotante son patrones de bits precalculados por `make_s1.py`.

### 2.2 La cadena de audio en modo PLUG-OUT [código]

Cada BMC tiene dos DSP (A y B) con su cola de programas: servicios `0x0100FC30` (A) y `0x0100FC34` (B); `FC38`/`FC3C`
son colas de bloques de datos; `FC28`/`FC2C` las rellena el propio OS. (`system-1/SH101_POLY.md` §2 suponía seis
DSP; el SH-2 y el reinicio de colas del OS indican dos por BMC.)

```
esclavo DSP A  s1m0 @0x0100 : 4 sierras x puerta -> suma x 0.25 -> A150   (banco 5 compartido por A y B)
esclavo DSP B  s1m1 @0x0100 : A150 x C000 -> IOMD 6006 (L), 6007 (R); 6008..600D = 0
maestro DSP B  s1mm1 @0x1000: IOMD 6006 / 6007 -> A118 / A119
maestro DSP A  (programa del OS en 0x82): A118 / A119 -> filtro de continua -> x C004 -> 6000 / 6001 (salida)
```

* El OS, al pasar a modo PLUG-OUT, encola en el maestro sus propios sets (`0x6001ECA2`): en el DSP A un programa en
  `0x82` que lleva `A118`/`A119` a `6000`/`6001`, y en el DSP B, en `0x1000`, un programa que pone `A118`–`A11D` a
  0. Después llama a `load` del modelo (vt+0x08). **Si el modelo no carga nada, el PLUG-OUT queda mudo**: por eso
  `model.c` carga su propio programa del DSP B en `0x1000`, como hace el SH-2 (el suyo lee `6006`–`600B` y escribe
  `A118`–`A11B`).
* `C000` es el nivel de salida del DSP B del esclavo; lo usan el programa de serie y los del SH-2/SH-101. Lo
  gobierna el OS (silencio en los cambios) **[inferido]**.
* El mezclador de serie del maestro (`0x60095B0C`, que suma `6006`–`600D` con ganancias `C005`–`C00E`) no se usa en
  modo PLUG-OUT **[inferido]** del set que el OS encola; por eso no dependemos de esas ganancias.

### 2.3 Programas DSP [código, sim]

Generados por `make_s1.py` con el ensamblador `asm/` (mismo formato `.s`/`.hex` que C-V4):

| programa | dónde | qué hace |
|---|---|---|
| `s1m0` | esclavo A `0x0100` | 4 voces: tono `A030+v` (octavas, `(nota−12)/12`), la sierra de C-V4 con la fase en el anillo del banco 0 (escribe `P`, lee `P+1`; `P = 0x300 + 0x10·v`), × puerta `A038+v`, suma × 0.25 → `A150`. 137 instrucciones |
| `s1m1` | esclavo B `0x0100` | `A150 × C000` → `6006`, `6007`; `6008`–`600D` = 0 |
| `s1mm1` | maestro B `0x1000` | `6006`/`6007` → `A118`/`A119`; `A11A`–`A11D` = 0 |
| `s1c0`, `s1c1` | esclavo A/B `0x7FF2` | cierre de la memoria de programa: contador de trama `E000` → `A004`/`A005`, como en todos los PLUG-OUTs del SYSTEM-1 |
| `s1e` | — | sección vacía (no se usa en la build; queda para cargar `0x5FF2`/`0x6FF2` si hiciera falta) |

Los programas del SYSTEM-1 empiezan con `0a02 ff71` (el SYSTEM-8 usa `ff70`) y terminan con
`0a02 ff70 0b02 00f8 0001`. El cierre son siete medias palabras: `0000`, `ld r0,[e000]`, `st r0,[a004]`; coinciden con
las de Roland porque es la misma instrucción (como los marcadores de inicio y fin).

`make_s1.py --sim` (sin hardware): La 440 → 440 cruces por segundo, un acorde de 4 notas llega a 0.78 de pico, sin
notas la salida es 0 exacto.

### 2.4 Dónde van las voces: programa principal o secciones de voz

Mapa completo, límites y otras variantes (también del SYSTEM-8): [DSP_PROGRAM_MEMORY.md](DSP_PROGRAM_MEMORY.md).

**C-S1V4 usa el programa principal** (`s1m0` en `0x0100` del DSP A del esclavo) para las cuatro voces, y apaga las
ocho secciones de voz del OS (`FC44(i, 0)`). Las secciones quedan sin usar.

La memoria de programa de cada DSP del SYSTEM-1 está dividida así [código]:

```
0x0000  cabecera (la pone el OS)
0x0100  principal          <- C-S1V4 / SH-101: aquí va todo
0x1900  sección de voz 2   <- sinte de serie / SH-2: un programa por sección
0x3100  sección de voz 3
0x4900  sección de voz 4
0x6100  sección de voz 5
0x7FF2  cierre (7 medias palabras)
```

Cada sección mide `0x1800` bytes y el OS la enciende o apaga con `FC44(índice, on)` (8 índices = 4 secciones × DSP
A/B); el principal corre siempre y, por lo visto en los programas de Roland, antes que las secciones [inferido]. El
tiempo de DSP es el mismo en los dos casos: unas 925 instrucciones por muestra y DSP, sumando principal y secciones.

| | principal (C-S1V4, SH-101) | secciones (sinte de serie, SH-2) |
|---|---|---|
| quién lo hace | SH-101: su sinte entero (575 instrucciones) en el principal del DSP A | serie: 4 programas de voz en A y 4 en B, iguales salvo las direcciones de datos de cada voz; SH-2: 2 secciones del DSP A |
| programas | uno, con las cuatro voces seguidas | uno por sección (4 copias casi iguales) |
| cambiar el sonido | recargar el principal | cambiar una sección (`FC40`) sin tocar el principal; encender/apagar voces con `FC44` |
| espacio | `0x0100`–`0x18FF` (6 KiB) para todo | `0x1800` bytes por sección |
| datos por voz | los elige el programa | cada copia con sus direcciones |

**De qué depende la elección**: no del hardware ni del OS (los dos caminos funcionan en los PLUG-OUTs de Roland),
sino de lo que haga el sonido. Para C-V4 (cuatro sierras con puerta, 137 instrucciones) un único programa es lo más
simple y deja `make_s1.py` igual de paramétrico. Las secciones convienen cuando cada voz es un programa grande que
se quiere cambiar por separado (por ejemplo, formas de onda distintas por oscilador, como el sinte de serie), o para
apagar voces y ahorrar ciclos. Pasar C-S1V4 a secciones sería: cuatro programas `s1v0`–`s1v3` en `0x1900`…`0x6100`
del DSP A, `FC44(0/1/4/5, 1)` según el índice → sección de `system-1/SH101_POLY.md` §2, y el principal sólo con la
suma; no está hecho.

## 3. La interfaz PLUG-OUT del SYSTEM-1 (OS 1.30)

### 3.1 SYNINFO [código]

Bloque de `0x28C` bytes al principio de las dos regiones (`syninfo.h`). El OS comprueba la marca en `0x000` y
`0x1B4` antes de llamar a `create` (maestro `0x600A95F8`, esclavo `0x60004D1E`).

| offset | contenido |
|---|---|
| `0x000` | `"SYSTEM1_SYNINFO"` |
| `0x010` | nombre (16) |
| `0x020` | id del modelo, versión (u16) |
| `0x024` / `0x028` | `create` / `destroy` |
| `0x02C` | maestro: el asignador de teclas; esclavo: 0 |
| `0x030` | maestro: el objeto modelo; esclavo: el motor |
| `0x034` | 39 × {u16 id, u16 activo, u32 valor}: controles del panel `0xC0`–`0xE5`, `0xEC` |
| `0x16C` | 9 × {u16 id, u16 activo, u8 permitido[4]}: selectores del patch (`0x171`, `0x172`, `0x178`, `0x179`, `0x17E`, `0x191`, `0x166`, `0x195`, `0x192`) |
| `0x1B4`, `0x1C4` | marca |
| `0x1D4` | 7 × 2 ids de patch de los botones (−1 = ninguno) |
| `0x1F0` | 3 × {u16 id, u16 activo, u8[4]} |
| `0x208` | marca (valida el número de patches) |
| `0x278` | número de patches (64): `NPATCH × 0x200` bytes en región 0 `+0x1000`, palabra k = id de patch `0x160+k` |
| `0x27C` | marca |

Valores de C-S1V4 (elegidos por nosotros): todos los controles activos con valor `0xFF` (salvo `0xC0` = 4, como el
SH-2; significado desconocido), todos los selectores con las cuatro posiciones permitidas, ningún botón, patches a
cero. **[abierto]**: qué hace exactamente cada tabla.

### 3.2 Maestro [código]

* Al pasar al PLUG-OUT (`0x6001E494`): `obj = get_b()`, `obj->vt[0]()`, y `get_a()` se instala como asignador de
  teclas (`0x60005AD4`).
* Asignador, vtable de 9 entradas (igual que las 9 primeras del SYSTEM-8): `reset`, `changed(grupo)`,
  `note_off(nota)`, `note_on(nota, vel)`, `cc(cc, valor)`, `part 0x81(valor)`, `bend(0..0x3FFF)`, dos vacías. No
  hay `set/get` en la vtable: el asignador llama directamente a los servicios `0x0100FC00` `get(grupo, id)`, `FC04`
  `set(grupo, id, v)`, `FC08` `flush(grupo)`, `FC0C` `touch(grupo, id)`.
* Ids de voz (grupo 2): nota `0x110+v`, velocidad `0x120+v` (0 = soltar), bend `0x130`, modulación `0x131`,
  expresión `0x134`.
* Modelo, vtable de 13 entradas; el OS llama a `vt[0]` (entrar), `vt[1]` (salir), `vt[2]` (cargar programas del
  maestro, después de encolar los suyos) y otras sin valor de retorno usado. C-S1V4 sólo implementa `vt[2]`.

### 3.3 Esclavo [código]

* Al pasar al PLUG-OUT (`0x6004E1A8`): `eng = get_b()`, `eng->vt[0]()`. Al recargar programas (`0x60003E3A`) el OS
  vacía las colas `FC30`–`FC3C`, desactiva las secciones de voz y llama a `eng->vt[2]` (load).
* Motor, vtable de 22 entradas: `+0x08` load, `+0x24` refresh(flag), `+0x30` `param(id, 0, valor)` (lo llama el
  despachador `0x6004CE64` para cada id cambiado de los grupos 0–3, sin el número de grupo), el resto sin uso en
  C-S1V4.
* `FC44(i, on)`: activa/desactiva la sección de voz i (0–7) del OS. El SH-101 las apaga todas; C-S1V4 también.
* Set de programas: entradas `{dirección en bytes, longitud en bytes, puntero}` terminadas en `{0, 0, 0}`; palabras
  de 32 bits con la primera media palabra en la mitad alta.

### 3.4 Banco 5 desde el ARM del esclavo [código, sin medir]

`FD6C(sel, obj, valor)` escribe `valor` en `0x40101000 + 0x400·sel + 4·obj[2]`. Los motores del SYSTEM-1 le pasan
patrones IEEE (`float`). Lectura: `sel 0` = palabras `A000+`, `sel 1` = `A100+` (el SH-2 escribe `A10E`–`A112` con
`sel 1`, que lee su DSP B) **[inferido]**. C-S1V4 usa `sel 0`, índices `0x30+v` (tono) y `0x38+v` (puerta): ningún
programa de serie ni de los PLUG-OUTs lee `A030`–`A03F` del esclavo.

## 4. Riesgos y preguntas abiertas

| # | qué | cómo se ve en el equipo | qué probar |
|---|---|---|---|
| 1 | ~~el OS rechaza el id de modelo `0x0010` o el SYNINFO~~ | **descartado**: aceptado e instalado [medido] | — |
| 2 | ~~el programa del maestro B no es el que suena~~ | **descartado**: C-S1TONE suena [medido] | — |
| 2b | ~~el volumen no actúa~~ | **resuelto**: `0x100` → `C01C` del maestro B con `FCEC(1, …)` funciona [medido] | — |
| 3 | ~~`FD6C` no es el banco 5 IEEE~~ | **descartado**: tono y puerta llegan [medido] | — |
| 4 | ~~el esclavo no recibe `0x110+v`/`0x120+v`~~ | **descartado**: las cuatro voces suenan [medido] | — |
| 5 | frecuencia de muestreo ≠ 96 kHz | todo desafinado el mismo intervalo | medir y corregir la constante de la sierra |
| 6 | ~~`C000` del esclavo B queda a 0~~ | **descartado** [medido] | — |

## 5. Ficheros

| fichero | qué es |
|---|---|
| `examples/system_1/make_s1.py` | generador de los programas DSP (`.s`/`.hex`, `build/s1_programs.h` con la tabla de tonos), `--tone`, `--sim` |
| `examples/system_1/syninfo.h` | estructura SYNINFO del SYSTEM-1, sets de programas, servicios |
| `examples/system_1/model.c` | región 0 |
| `examples/system_1/engine.c` | región 1 |
| `examples/system_1/link.ld` | SYNINFO en 0, patches en `+0x1000`, código en `CODE` |
| `examples/system_1/build.sh` | build completa → `plugins/<NAME>/Script/Code1.Dat` (`TONE=1`, `PARAMLOG=1`) |
| `examples/system_1/paramlog.py` | escucha y decodifica la captura de parámetros de una build `PARAMLOG` (§6) |
| `examples/system_1/s1*.s`, `s1*.hex` | programas generados (no se editan a mano) |

Generados (fuera de git): `examples/system_1/build/`, `plugins/C-S1V4/`.

## 6. Captura de parámetros (PARAMLOG): qué id manda cada control

Hecha el 2026-10-22 para encontrar el VOLUME; sirve para mapear cualquier control del panel o mensaje MIDI a su id
de parámetro, en el SYSTEM-1 y (con otro punto de enganche, §6.3) en el SYSTEM-8.

### 6.1 Cómo funciona

* El OS del maestro llama a `vt+0x30 (id, flag, valor)` del **objeto modelo** de la región 0 con cada parámetro que
  cambia (panel, MIDI, cambio de patch; `flag` = 0, 1 en los refrescos). Es la misma llamada que recibe el motor
  del esclavo, pero en el maestro, que **sí puede mandar MIDI**.
* La build `PARAMLOG=1` (`model.c`, `param_log`) reenvía cada llamada al PC como SysEx no comercial:
  `F0 7D 06 <id>>7> <id&7F> <flag> <valor: bits 31–28, 27–21, 20–14, 13–7, 6–0> F7`.
* Envío: la misma función que usa el OS 1.30 para sus respuestas DT1 (`0x60019D2A`):
  `send(0x605F20D4, buf, len, cable 3, máscara 1)` en `0x600065B0` (`system-1/FIRMWARE.md` §3.2). Antes se mira el
  byte de guarda `0x6009BF08` bit 0 (el objeto ya construido; lo está después de cualquier consulta de
  `plugout_tool.py info`); si no, el mensaje se descarta. **Verificado**: los mensajes llegan por `hw:N,0,0`
  (MIDI 1) [medido].
* Nada se escribe: sólo se observa. La build lleva `TONE=1` para que suene algo mientras tanto.

### 6.2 Cómo repetirla

```sh
PARAMLOG=1 TONE=1 NAME=C-S1LOG sh examples/system_1/build.sh
python3 tools/plugout_tool.py install plugins/C-S1LOG --slot 1 --force --no-patches -y
python3 examples/system_1/paramlog.py -t 120          # mientras tanto: seleccionar PLUG-OUT y mover UN control
python3 examples/system_1/paramlog.py --file LOG...   # decodificar logs de `amidi -p hw:N,0,x -d` guardados
```

Consejos: mover un solo control por captura (o anotar el orden), recorrerlo de punta a punta, y no correr otra
herramienta MIDI a la vez. `paramlog.py` muestra por id: número de llamadas, mínimo, máximo, primeros valores y los
mensajes MIDI que el equipo emitió justo después (eco de CC del panel). Con `--file` y varios logs el orden entre
puertos se pierde, así que esa última columna sólo vale con un único log.

Resultado del 2026-10-22 (C-S1LOG; se movieron el VOLUME y dos controles más):

| id | valores | eco MIDI | qué es |
|---|---|---|---|
| `0x100` | 0–255, 433 llamadas, sin CC | — | **VOLUME** del panel |
| `0x174` | 0–132 | CC 16 (canal 5) | otro control del panel (id de patch `0x160 + 0x14`) |
| `0x175` | 0–115 | CC 17 (canal 5) | otro control del panel (`0x160 + 0x15`) |
| `0x082` | 1 | — | una vez al entrar (parte; sin identificar) |

### 6.3 En el SYSTEM-8

El SYSTEM-8 no tiene ese objeto modelo con `vt+0x30` en la región 0 (su descriptor es `create/destroy/asignador/
select`, `examples/from_scratch/model/model.c`), y el motor del esclavo, que sí recibe `e_param(id, valor)`, no
puede mandar MIDI. Dos caminos, los dos con el envío ya probado de `VOICELOG` (router `0x6000345D`, SysEx por
USB 1 + DIN, `docs/BASE_FIRMWARE.md` §7.12):

1. **Diferencias en `changed(grupo)`** (vt[1] del asignador, que el OS llama al cambiar un grupo): guardar una copia
   de los valores de un rango de ids con `OS->get(parte, grupo, id)` y mandar sólo los que cambian. Sin enganches
   nuevos; cubre los grupos que notifica el OS.
2. Buscar en el OS del maestro el despachador de parámetros equivalente al `0x6004CE64` del esclavo del SYSTEM-1 y
   enganchar ahí (más trabajo, cubre todo).

Lo propio de cada equipo es sólo el punto de enganche y la función de envío; el formato `F0 7D 06 …` y
`paramlog.py` valen igual (en el SYSTEM-8: `paramlog.py --device SYSTEM-8`).

## 7. Ocho voces (C-S1V8, 2026-10-22)

`VOICES=8 NAME=C-S1V8 sh examples/system_1/build.sh`. El número de voces es un parámetro de la build
(`make_s1.py` lee `VOICES` y escribe `#define NV` en `build/s1_programs.h`; `model.c` y `engine.c` lo toman de ahí).

| | C-S1V4 | C-S1V8 |
|---|---|---|
| asignador (región 0) | poly de 4 | poly de 8 (la misma regla: misma nota > libre más antigua > soltada con hold > más antigua) |
| ids | `0x110`–`0x113`, `0x120`–`0x123` | `0x110`–`0x117`, `0x120`–`0x127` |
| banco 5 del esclavo | tono `A030`–`A033`, puerta `A038`–`A03B` | tono `A030`–`A037`, puerta `A038`–`A03F` |
| fase (banco 0) | `0300`–`0331` | `0300`–`0371` |
| `s1m0` (DSP A del esclavo) | 137 instrucciones | **269** de ≈ 925 por muestra |
| nivel por voz | 1/4 | 1/8 (un acorde de 8 no satura: 0,57 de pico en la simulación) |

Resultado [medido, a oído]: las ocho voces suenan a la vez y ninguna nota corta a otra. Por lo tanto:

* el OS del maestro **acepta 8 voces** de un asignador de PLUG-OUT y manda al esclavo los ids `0x114`–`0x117` y
  `0x124`–`0x127` como los de las voces 0–3 (el SYSTEM-1 de serie sólo usa 4);
* un solo DSP del esclavo hace las ocho voces sobrado de tiempo.

Límites para ir más allá [inferido]: el bloque de velocidades tiene 8 ids (`0x120`–`0x127`; `0x128` empieza el de
disparo), así que con estos ids el máximo es 8. Notas (`0x110`–`0x11F`) tendrían hueco para 16. Más de 8 voces
necesitaría otra forma de mandar la puerta (por ejemplo, usar la nota y un valor especial, o los ids de disparo
`0x128+v`) y comprobar que también llegan; el tiempo de DSP daría para ≈ 25 voces de sierra en el DSP A, y el DSP B
del esclavo está casi libre. Sin probar.
