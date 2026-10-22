# Asignación de voces de Roland: qué DSP hace qué (estudio, 2026-10-22)

Objetivo: saber, con datos, qué programa corre en cada DSP y cómo viaja una voz (tono, puerta, osciladores,
salida), para escribir programas principales propios sin repetir errores. Este documento separa lo **verificado**
de lo **pendiente**. Ninguna afirmación de aquí se apoya en una suposición sin marcar.

Niveles de evidencia (§0–§5; §6 tiene su propia leyenda): **[datos]** = leído de la imagen de la región 1 de un plug-out; **[listado]** = leído del
microcódigo desensamblado (`dump_main.py`); **[ARM]** = leído del código ARM de la región 1; **[medido]** = audio en
el equipo.

## 0. Método

1. `tools/plugout_build.py unpack` de cada `Code8_<n>.Dat` (slot 1, 2, 3) → `region1.bin`.
2. `re/alloc_scan.py region1.bin 60A00000` busca los descriptores de programa `dspdesc` (dirección de ranura, tamaño
   dos veces, puntero al programa). La base `0x60A00000` es la dirección de ejecución de la región 1 del slot 1; se
   comprobó porque los punteros de los descriptores caen dentro de la imagen con ese desplazamiento.
3. El tamaño de cada descriptor de programa principal se compara con los tamaños de `engine/build/programs.h`
   (nombres MAIN0…MAIN5 de JX-3P): la identidad de tamaño da el nombre.
4. Los listados de los programas (`dsp/dump_main.py`) dan las palabras de MXM y de banco 5 que lee y escribe cada uno.
5. El código ARM de la región 1 (`arm-none-eabi-objdump`) confirma qué ids escribe el motor en cada DSP.

## 1. JX-3P (referencia de este estudio)

| DSP | programa principal | bytes | lee de | escribe a | ranuras de oscilador o efecto |
|---|---|---|---|---|---|
| DSP0 | MAIN0 | 5066 | banco 5 `a021`, `a033` (tono), `a022`, `a034` (envolvente) [listado] | MXM Z `8404`…`8416` → DSP2 M `8004`…`8016` [listado] | ninguna (ranura `8000` vacía en `engine.c`, nuestra reproducción; no verificado en ARM) |
| DSP1 | MAIN1 | 5018 | banco 5 `a021`, `a033` [listado] | MXM Z `8400`…`8410` → DSP3 O `8200`…`8210` [listado] | ninguna |
| DSP2 | MAIN2 | 2068 | MXM M `8004`…`8016` (de DSP0) y M `861b`…`861d` [listado] | MXM Z `8404`…`841a` → DSP4 M `8005`…`8016` [listado] | DCO1 (SAW 4104 B, SQR 3976 B) y DCO2 (SAW, SQR, NOISE 384 B) [datos] |
| DSP3 | MAIN3 | 2018 | MXM O `8200`…`8210` (de DSP1) [listado] | MXM Z `8400`…`8414` → DSP5 O `8201`…`8214` [listado] | DCO1 y DCO2 [datos] |
| DSP4 | MAIN4 | 5174 | MXM M `8005`…`801a` (de DSP2) [listado]; banco 5 `a022`…`a03e` (puerta, unidad 4) [listado] | IOMD `6020`… (salida) [listado, WAVE_BUS.md] | ranura `B000` = efecto (OD, DS, CH, FZ, CR, PH) [datos] |
| DSP5 | MAIN5 | 5016 | MXM O `8201`…`8214` (de DSP3) [listado]; banco 5 `a020`…`a045` (puerta, unidad 5) [listado] | IOMD `60B0`… (no se oye) y **`A400`–`A403` → DSP4** (ventana de pareja, §6) [listado] | ninguna (ranura `6000` vacía en `engine.c`; no verificado en ARM) |

Verificación:
* **[datos]** Los seis descriptores de programa principal están en el orden MAIN0…MAIN5 en la imagen, y cada tamaño
  coincide con el de su nombre (5066, 5018, 2068, 2018, 5174, 5016). El índice del array es el DSP.
* **[listado]** Lo que lee cada programa es consistente con lo que escribe el anterior: DSP2 lee lo que escribe DSP0,
  DSP4 lee lo que escribe DSP2, DSP3 lee lo que escribe DSP1, DSP5 lee lo que escribe DSP3. Es la cadena de MXM.
* **[ARM, parcial]** El motor escribe el tono en el banco 5 del DSP `u = lv>>1`, id `0x21 + 0x12·(lv&1)`. Dos
  sitios de `movs r1,#0x21` (p. ej. `0x60A099BC`, con `asrs #1` sobre la voz) y de `movs r1,#0x33` (`0x60A0A66A`)
  son compatibles con eso. No he leído la función completa que los llama, así que el ARM está **parcialmente**
  verificado.
* La puerta del motor (`note_gate`) escribe en el banco 5 de DSP `4+u` (DSP4 para voces 0,1; DSP5 para voces 2,3).
  Coincide con los programas: MAIN4 lee puertas de unidad 4 y MAIN5 de unidad 5.

**Consecuencia:** en Roland, las voces 0 y 1 pasan por DSP0 → DSP2 → DSP4, y las voces 2 y 3 por DSP1 → DSP3 → DSP5.
El filtro/VCA de la pareja 2 está en **DSP5**, no en DSP4.
**Añadido 2026-10-22 (§6):** DSP5 entrega las voces 2 y 3 a DSP4 por `A400`–`A403`, y DSP4 las suma a la salida
`6020`/`6021`. Los rangos MXM de la tabla son rangos de uso: el número de palabra se conserva (Z `4nn` → M/O `nn`).

## 2. Todos los plug-outs de Roland (estudio 2026-10-22)

Método: `re/alloc_scan.py` (descriptores de programa y base de la imagen), `re/alloc_mxm.py` (referencias de MXM,
banco 5 e IOMD de cada principal, desde el desensamblado). Los seis principales de cada imagen se leen en orden de
dirección. **Corrección:** antes dije que JUNO-60 tenía 3 principales y que JUPITER-8 no tenía ninguno; eran fallos del
filtro de búsqueda. Con el filtro relajado hay 6 en ambos.

Hay dos familias de topología.

### 2.1 Familia A: polifónica, cuatro voces por slave (JX-3P, JUNO-60, JUNO-106, JUPITER-4, JUPITER-8)

La misma cadena en las cinco imágenes (`main0`…`main5` = DSP0…DSP5):

| principal | lee | escribe | notas |
|---|---|---|---|
| main0 (DSP0) | MXM `8617`–`8619` (S) | MXM Z `8404`–`8416` → DSP2 M | banco 5 `a02e`/`a02f`/`a040`/`a041` (JX-3P) y `a404`…`a40e` |
| main1 (DSP1) | — | MXM Z `8400`–`8410` → DSP3 O | sin lecturas de MXM |
| main2 (DSP2) | MXM M `8004`–`8016` (de DSP0), `861b`–`861d` | Z `8017`–`8019` (reenvío a DSP0 S) y Z `8404`–`841a` → DSP4 M | sin banco 5 escrito |
| main3 (DSP3) | MXM O `8200`–`8210` (de DSP1) | Z `8400`–`8414` → DSP5 O | sin banco 5 escrito |
| main4 (DSP4) | MXM M `8005`–`801a` (de DSP2) | M `801b`–`801d` (reenvío) + **IOMD `6020`–`6024`, `6060`–`6066`** | banco 5 `a404`–`a407` |
| main5 (DSP5) | MXM O `8201`–`8214` (de DSP3) | **IOMD `60B0`–`60B7`** | banco 5 `a400`–`a403` |

Tamaños (bytes): JX-3P 5066/5018/2068/2018/5174/5016; JUNO-60 4610/4564/1548/1512/6122/7406; JUNO-106
4982/4934/3000/2950/5344/5236; JUPITER-4 4374/4328/1482/1446/6474/7464; JUPITER-8 5838/5790/3040/2990/5038/4912.

Lo que esto **verifica** para la familia A: el par DSP0→DSP2→DSP4 (voces 0,1) y el par DSP1→DSP3→DSP5 (voces 2,3)
es el mismo en los cinco plug-outs. Las salidas por IOMD son las mismas: DSP4 escribe `6020`… y DSP5 escribe `60B0`….
No es casualidad de JX-3P: es la topología de la familia.

### 2.2 Familia B: otra arquitectura (SH-101, SH-2, SYSTEM-100, PROMARS, base SYSTEM-8)

| plug-out | main1 (bytes) | main5 (bytes) | cadena |
|---|---|---|---|
| SH-101 | 200 | 84 | main0 lee `8613`–`8617` y `8627`–`862b`; main2 lee `8000`–`8026`; main4 escribe IOMD `6020`… |
| SH-2 | 200 | 84 | igual que SH-101 |
| SYSTEM-100 | 200 | 84 | igual que SH-101 |
| PROMARS | 200 | 84 | igual que SH-101 (main4 `5640` B) |
| SYSTEM-8 base (Code8_0) | 5566 | 4900 | main0 y main1 leen zonas `8613`/`860f` y escriben `8400`…; main5 escribe `60B0`… |

En esta familia main1 y main5 son plantillas muy pequeñas. Eso es coherente con sintetizadores monofónicos (SH-101,
SH-2) o con otra organización de voces. **No sirve como referencia de la asignación de cuatro voces por slave.**

### 2.3 Consecuencia para nuestro trabajo

Nuestra referencia para la asignación polifónica es la familia A. Nuestros `solo`/`solov2` no la respetaban (ver
sección 5).

Tabla de los descriptores de ranuras DCO y efectos (no principales) por plug-out: JX-3P 16, JUNO-60 8, JUNO-106 9,
JUPITER-4 10, JUPITER-8 26, SH-101 0, SH-2 2, SYSTEM-100 2, PROMARS 2, SYSTEM-8 base 84. Su asignación a DSP no está
verificada en todos los casos (falta el orden del `e_load` ARM de cada imagen).

## 2.4 Pendientes resueltos y no resueltos (2026-10-22)

**Resuelto**
* En la imagen de JX-3P (región 1, slot 1, base `0x60A00000`) hay una tabla de punteros en la dirección `0x60A2581C`
  (desplazamiento de fichero `0x2581C`). Tiene pares `{programa, datos}` por DSP, con paso de 8 bytes:
  `prog0, data0, prog1, data1, …, prog5, data5`. Los programas apuntados son los de MAIN0…MAIN5 (sección 1), y los
  datos de DSP0…DSP5 se apuntan en el mismo orden. Esta es la estructura `main_prog[]`/`main_data[]` de `engine.c`.

**No resuelto**
* **El código ARM que lee la tabla** no está localizado. Solo he encontrado un `ldr` que carga el literal `0x60A25818`,
  que contiene `0x60A25804`. La desensamblado lineal de `objdump` mezcla código y datos en esa zona y no es fiable para
  recuperar fronteras de función. Hace falta un desensamblado guiado por funciones antes de afirmar cómo se indexa la
  tabla. Por eso el índice DSP de cada principal en JX-3P sigue siendo deducido de la estructura, no leído del código.
* Lo mismo para los otros plug-outs de la familia A (JUNO-60, JUNO-106, JUPITER-4, JUPITER-8): solo se ha leído su
  tabla de descriptores, no su código de carga.
* **Semántica de los programas**: no se ha decodificado qué calcula cada principal. Solo sus referencias de bus.
* **Familia B** (SH-101, SH-2, SYSTEM-100, PROMARS, base SYSTEM-8): solo tablas y referencias de bus, sin análisis
  funcional.
* **Interfaz de las ranuras DCO de DSP2/DSP3** (qué entradas `[22nn]` leen y qué salidas esperan): no está escrita en
  forma completa; sale de los listados de MAIN2 y MAIN3 y de `CUSTOM_OSC.md` §2, y no se ha cruzado con los
  programas de ranura de Roland.

**Consecuencia:** la asignación de JX-3P está bien respaldada por datos y por el microcódigo (sección 1), y
`C-SAWPOLY` la reproduce en el equipo (sección 4). Lo que falta para escribir MAIN0–5 propios es la semántica de
cada programa, no la asignación.

## 2.5 Osciladores DCO1 y DCO2 de Roland y las palabras `2220`–`222A` (2026-10-22)

Desensamblado de los programas de ranura de JX-3P (`dsp/dump_main.py DCO1_SAW | DCO1_SQR | DCO2_SAW | DCO2_NOISE`).
Solo estudio: ningún programa de Roland entra en las pruebas nuevas.

**Escrituras de salida de los osciladores [datos: listado]**

| programa de ranura | voz A | voz B |
|---|---|---|
| DCO1 SAW y DCO1 SQR | `[221F]` (r0), `[221B]` (r1) | `[2227]` (r0), `[2223]` (r1) |
| DCO2 SAW y DCO2 NOISE | `[2221]` (r0), `[221D]` (r1) | `[2229]` (r0), `[2225]` (r1) |

Coincide con `CUSTOM_OSC.md` §2. Nuestro SAW de DCO2 escribe `[2221]`/`[2229]`, igual que el de Roland.

**Entradas de tono [datos: listado]:** MAIN2 escribe `[2207]` (voz A DCO2) y `[2214]` (voz B DCO2), y `[220E]` (voz B DCO1).
MAIN3 escribe las mismas direcciones. Nuestro MAIN2 escribe `[2207]`/`[2214]`, igual.

**Lo que escribe y lee MAIN4 en `[221B]`, `[221D]`, `[2229]` [listado]:** MAIN4 escribe esas palabras al principio de
su programa (copias de `M 8017`, `M 8013`, etc.) y las lee después. **No son las salidas de los osciladores**, aunque
coincidan en dirección. Corrige una afirmación anterior de este documento.

**Palabras `[2220]`, `[2222]`, `[2224]`, `[2226]`, `[2228]`, `[222A]` [listado]:**
* MAIN2 las lee (líneas 25, 27, 106, 129, 131, 138, 210 del listado).
* **Ningún programa de ranura de JX-3P ni los principales MAIN0–5 las escriben.** Tampoco MAIN3, MAIN4 o MAIN5.
* ~~Así que las escribe algo que no está en el microcódigo de DSP: lo más probable, el ARM…~~ **Corregido en §6.3 D:**
  son las salidas de las ranuras (`[221B]`–`[2229]`) leídas una muestra después a +1, porque el banco 1 es un anillo
  [listado, inferido].
* Nuestra cadena no usa estas palabras. Por eso no explican el fallo de nuestro plug-out; sí explican por qué el camino
  de Roland (MAIN2 → DCO) es distinto del nuestro en este punto. **(Corregido en §6.3 A/D: sí importan; v3 lee las
  palabras impares, que con anillo devuelven `[221D]`/`[2225]` de 4 muestras antes.)**

**Banco 1 compartido por pares [inferido, contradicho en parte] — corregido en §6.3 E: DSP2 y DSP3 tienen bancos 1
distintos (tabla del ARM).** Texto original: `DSP_ARM_SIDE.md` dice que el banco 1 (`2nnn`) se
comparte por el par de DSP (líneas 20 y 190). Si DSP2 y DSP3 lo compartieran, sus escrituras de `[2207]`, `[2214]`,
`[2221]`, `[2229]` se pisarían. Argumento indirecto contra la compartición: en `C-V3` la voz 0 (DSP2 voz A) suena con la nota correcta, aunque DSP3
escribe también `[2207]` en cada muestra. Si compartieran el banco, la voz 0 tocaría la nota de DSP3. **Inferido, no
medido.** Hay que confirmarlo con una prueba directa (p. ej. escribir un valor distinto en `[2207]` de DSP3 y ver si
cambia la voz 0) antes de darlo por cierto.

**Consecuencia para la búsqueda de la voz 1 muda (resuelto en §6.3 A: MAIN4 de v3 lee M 006, que nadie escribe):** las salidas de voz B (`[2229]`) y entradas (`[2214]`) son las que
escribe Roland. El fallo de la voz 1 no se explica por direcciones distintas de las de Roland en esas palabras. Queda
por revisar: la puerta (`0x39` en DSP4, que Roland también lee en la línea 588 de MAIN4) y el estado de fase de la
voz B (`0x680`/`0x681`, que coincide con `CUSTOM_OSC.md` §2).

## 3. Lo que no está verificado en JX-3P

* El orden del array de descriptores → índice DSP está deducido por el tamaño, no por la función `e_load` completa.
* El destino exacto de cada palabra de MXM dentro de los DSP (p. ej. `[2200]` frente a `[2201]`) lo fija el programa
  de la ranura DCO; no está documentado con el mismo detalle en todos los casos.

## 4. Medición con la cadena de Roland (`C-SAWPOLY`, instalado en el slot 3, 2026-10-22)

La cadena de Roland (principales de JX-3P sin tocar) y nuestro SAW en las ranuras DCO. Resultado con `voicelog.py`:

| | voces que suenan |
|---|---|
| slave 0 (voces 0–3) | 4 de 4 |
| slave 1 (voces 4–7) | 3 de 4 (la nota 55, voz 5, en dos ejecuciones secuenciales) |
| ocho notas a la vez | 8 de 8 |

Con esto queda **medido** que el slave 1 llega al audio con la cadena de Roland. El silencio del slave 1 en
`gate/solo` y `solov2` era de nuestra cadena (programas principales vacíos y tono escrito directo en DSP2/3).

**Ojo:** `C-SAWPOLY` contiene microcódigo de Roland (los principales y los efectos, copiados en la construcción).
Sirve para medir la asignación. No es válido como plug-out sin microcódigo de Roland (regla de la memoria
`no-roland-microcode-in-tests`).

## 5. Errores que cometimos (para no repetirlos)

1. Dar por cierto que el tono llega a DSP2/3 directamente. En Roland entra por DSP0 (voces 0,1) y DSP1 (voces 2,3).
2. Poner la puerta de las cuatro voces en DSP4. En Roland, voces 2 y 3 van a DSP5.
3. Quitar los programas principales de DSP0…DSP3 (`solo`/`solov2`). Eso rompe la cadena de MXM y el DSP4 no recibe lo
   que espera.
4. Asumir que el microcódigo es común entre plug-outs. La tabla de la sección 2 muestra que no.
5. Inferir la asignación sin mirar la tabla de descriptores. Esta tabla existe y se puede leer.

## 6. Flujo de datos del slave y por qué C-V3 no suena (estudio con emulador, 2026-10-22)

Etiquetas de esta sección: **[medido]** = en el equipo; **[listado]** = leído del microcódigo desensamblado (solo
estudio, nada se copia); **[código]** = leído de nuestro código o del código ARM; **[documentado]** = en otro
documento del repo, sin repetir la prueba; **[emulador]** = predicción de `examples/from_scratch/emu/chain.py`;
**[inferido]** = deducción, con la prueba que la confirmaría.

Herramientas nuevas (todas en `examples/from_scratch/emu/`):

| herramienta | qué hace |
|---|---|
| `chain.py` | emulador de un slave (seis DSP) y de dos slaves (`Bmc2`). Bancos por DSP, orden dentro de la muestra, MXM con cerrojo de 1 muestra, ventana de pareja del banco 5, IOMD. Cada hipótesis es un campo de `Model` |
| `voices.py` | conjuntos de programas (solo nuestros: v3, v4, v4b) y las escrituras de tono y puerta de `engine_v3.c` |
| `test_chain.py` | pruebas por pregunta (A–H) y de regresión; `python3 test_chain.py [A G …]` |
| `listing.py` | referencias de bus de los programas de Roland, leídas del listado (`--summary`) |
| `allplugouts.py` | los mismos hechos en todas las imágenes de Roland del repo (`work/unpack/*`, JX-3P) |
| `sigcheck.py` | comprobación de firmas: que una build o un programa no contenga bytes de Roland (§7) |

**Corrección sobre el emulador anterior:** el `emu/chain.py` previo descartaba la memoria que devuelve
`dspisa.run` (`regs, _, _ = run(...)`), así que ninguna escritura llegaba a la memoria y todas las voces daban 0. Se
reescribió con el gancho `dspisa.ACCESS`, que ve cada acceso y lo dirige al banco correcto.

### 6.1 Modelo del hardware que usa el emulador

| pieza | comportamiento en `Model()` (por defecto) | evidencia |
|---|---|---|
| banco 0 | anillo de 0x800: lo guardado en R en la muestra n se lee en R+1 en la n+1 | [medido] `DSP_HARDWARE.md` §7.2 |
| banco 1 (`2nnn`) | **también anillo** (`ring1`) | [listado] §6.3 D; [medido] con C-V4 (§6.7) |
| banco 1, compartición | cada DSP el suyo; DSP0 y DSP1 tienen la misma base en la tabla del ARM | [listado] tabla ARM §6.3 E |
| orden en un DSP | principal antes que las ranuras (`main_first`), DCO1 antes que DCO2 | [inferido] de C-SAW afinado [documentado] + listado, §6.3 F |
| MXM | Z `4nn` del DSP par/impar de la pareja anterior → M/O `nn` del DSP siguiente, **mismo nn**; cerrojo de 1 muestra; sin retención | [medido en el maestro] `DSP_HARDWARE.md` §8.2; [listado] §6.3 G |
| MXM retorno | escrituras en M/O de DSP r+2 → S `6nn` de DSP r | [documentado] `DSP_ARM_SIDE.md` §3.1 |
| banco 5 `A000`–`A3FF` | memoria propia persistente | [medido] |
| banco 5 `A400`–`A7FF` | **ventana compartida por la pareja** (0,1), (2,3), (4,5), cerrojo de 1 muestra (`dp_pairs`, `dp_delay`) | [listado] §6.3 B; [medido] con C-V4 voces 2 y 3 (§6.7); el cerrojo de 1 muestra no está medido |
| IOMD | un bloque para los seis DSP; lo escrito persiste; sólo `6020`/`6021` del slave 0 se oyen | [medido] `WAVE_BUS.md` §4b |
| enlace entre slaves | `Model.link`: palabras IOMD del slave 1 → entradas del slave 0, 1 muestra tarde | [medido] que existe (C-V4, §6.7); [inferido] qué palabras |

Lo que el emulador no modela: el lado ARM (las escrituras del motor se inyectan entre muestras), las ranuras de
efecto, el banco 2, la propiedad de las palabras IOMD, el bus de onda real y la latencia de carga de las ops (el
simulador `dspisa` hace visible una carga en la instrucción siguiente; nuestros programas dejan una instrucción de
margen igual que en el equipo).

### 6.2 Flujo de una voz en Roland (familia A: JX-3P, JUNO-60, JUNO-106, JUPITER-4, JUPITER-8)

```
ARM ──banco 5 (tono 0x21/0x33)──► DSP0 (voces 0,1) ──Z 404..416──► DSP2 M 004..016 ──► [2200..221A] ──► ranuras DCO1/DCO2
ARM ──banco 5 (tono 0x21/0x33)──► DSP1 (voces 2,3) ──Z 400..410──► DSP3 O 200..210 ──► [2200..221A] ──► ranuras DCO1/DCO2
DSP0 ──A404..A40E (ventana DP01)──► DSP1
ranuras DSP2: salidas [221B][221D][221F][2221] (voz A), [2223][2225][2227][2229] (voz B)
      └─(banco 1, anillo)─► MAIN2 lee [221C]…[222A] en la muestra siguiente ─► Z 416..419 ──► DSP4 M 016..019
ranuras DSP3: lo mismo ─► MAIN3 ─► Z 411..414 ──► DSP5 O 211..214
DSP4 (filtro/VCA voces 0,1) ──A404..A407──► DSP5      DSP5 (filtro/VCA voces 2,3) ──A400..A403──► DSP4
DSP4: suma 8 fuentes: sus 2 voces, A400/A401 (voces 2,3 de DSP5) e IOMD 6000..6003 ──► 6020 / 6021 (audio)
      y escribe las voces sueltas en 6060..6063; DSP5 escribe 60B0..60B7 (no se oyen)
```

Detalle [listado] (`emu/listing.py --summary`, `emu/allplugouts.py`):
* MAIN2 copia `M 0nn → Z 4nn` con el **mismo** nn (004–00B, 00C–012, 013–015; 19 de 20 copias; la otra es
  `M 016 → Z 41A`). Igual en los cinco plug-outs de la familia A (18 o 19 copias con el mismo nn, 1 distinta).
* MAIN2 calcula Z 416–419 desde el banco 0 (`[0236]`, `[026E]`, `[022B]`, `[0263]`); MAIN4 copia M 016 → `[221C]`,
  M 017 → `[221D]`, M 018 → `A404`, M 019 → `A405`. MAIN3 calcula Z 411–414 y MAIN5 copia O 211–214.
* MAIN4 suma ocho cargas (`[2228]`, `[2233]`, `[223C]`, `[2245]`, `[2248]`, `[224B]`, `[224C]`, `[224D]`, con `addxy`
  y coeficientes `C044`–`C048`). Al principio de la muestra guarda `A400` → `[2234]`, `A401` → `[223D]`, `6000` →
  `[2246]`, `6001` → `[2249]`, `6002` → `[224C]`, `6003` → `[224D]`, y justo antes de sumar guarda `[221E]` y `[2229]`
  (sus dos voces [inferido]). Con el anillo, cada carga es una de esas ocho palabras guardadas, cada una una vez, con
  retardos de 10, 10, 8, 8, 2, 2, 0 y 0 muestras. Sin anillo, seis de las ocho cargas leerían palabras que nadie
  escribe. Es el segundo indicio fuerte del anillo.
* MAIN5 escribe `A400`–`A403` al final de su programa y MAIN4 los lee al principio; MAIN4 escribe `A404`–`A407` y
  MAIN5 los lee. MAIN0 escribe `A404`–`A40E` y MAIN1 los lee. **En ningún caso un programa lee lo que escribe él
  mismo en esa zona**: es un buzón de pareja. Familia A completa: igual (JUNO-60/JUPITER-4 con más palabras).
* Familia B (SH-101, SH-2, SYSTEM-100, PROMARS) no usa ese buzón para la voz (main0 y main5 escriben `A400`… y nadie
  los lee); no sirve de referencia.

### 6.3 Preguntas A–H

Prueba = nombre de la función en `emu/test_chain.py`. Todas pueden fallar: comprueban un dato del listado o una
predicción del emulador para el modelo elegido **y** para el alternativo.

**A. ¿Por qué la voz 1 (DSP2 voz B) no suena?**
* Hipótesis: el MAIN4 de v3 lee `M 005` y `M 006` suponiendo que DSP2 Z 404 llega a DSP4 M 005 (desfase de 1). El
  MXM conserva el número de palabra (G), así que `M 005` = Z 405 (sierra de la voz B) y `M 006` no la escribe nadie.
  La puerta de la voz 1 (`0x39`) multiplica 0. La puerta `0x39` en DSP4 y la fase `0x680`/`0x681` están bien
  [código: `engine_v3.c` `voice_gate`; listado: el SAW guarda la fase de B en `[0680]` y la lee en `[0681]`].
* Pruebas: `test_A_v3_voice1_silent` (voz 1 → 0), `test_A_v3_voice0_gate_plays_voice1_saw` (la puerta de la voz 0
  deja pasar la sierra de la voz B: suena **con el tono de la voz 1**), `test_A_v3_same_with_and_without_ring`.
* Resultado [emulador]: las tres pasan. La voz 0 "suena" en v3 sólo porque su puerta abre la sierra de la voz 1
  (con el modelo elegido; ver F para la variante "ranura primero").
* Medida que lo confirma (propuesta, no hecha): C-V3L en poly, voz 1 con nota 57 y luego voz 0 con nota 69 (o
  `voicelog.py --groups 57 69`): se oye 220 Hz (la nota de la voz 1) mientras la puerta de la voz 0 está abierta. Si
  se oye 440 Hz, el modelo está mal. En unison todas las voces tienen la misma nota, por eso allí la afinación es
  exacta [medido].
* Arreglo (v4): MAIN4 lee `M 004`/`M 005`.

**B. ¿Por qué DSP3 (voces 2 y 3) no suena? ¿Llega 60B0 al audio?**
* `60B0`–`60B7` no llegan al audio [medido, `WAVE_BUS.md` §4b, barrido con ocho SAW]. v3 manda las voces 2 y 3 a
  `60B0`/`60B1` (DSP5). Roland no usa 60B0 para el audio: DSP5 deja sus voces en `A400`–`A403` y DSP4 las suma en
  `6020`/`6021` [listado].
* Contradicción del prompt resuelta: en `C-SAWPOLY` las voces 2 y 3 suenan porque el MAIN5 de Roland las pasa a DSP4
  por la ventana de pareja, no por 60B0.
* Pruebas: `test_B_v3_voices23_only_reach_60b0` (v3: señal en 60B0, nada en 6020), `test_B_listing_pair_window`,
  `test_B_v4_pair_window_decides` (v4: voz 2 suena con la ventana; muda sin ella).
* Resultado: pasan. Medida que lo confirma: C-V4, voces 2 y 3 (notas sueltas). Si suenan, la ventana de pareja queda
  medida; si no, `dp_pairs` es falso.

**C. ¿Por qué el slave 1 entero no suena, si con la cadena de Roland sí?**
* v3 produce la voz del slave 1 en su propio `6020` [emulador]. Si ese `6020` se oyera, la voz 4 sonaría, y no suena
  [medido, C-V3L]. Así que el `6020` del slave 1 no va directo al audio.
* Hipótesis C1 [inferido]: el slave 1 llega al slave 0 por IOMD y el MAIN4 del slave 0 lo suma: Roland suma
  `6000`–`6003` en su mezcla y escribe sus cuatro voces sueltas en `6060`–`6063` [listado]. v3 no hace ninguna de las
  dos cosas.
* Pruebas: `test_C_listing_main4_mixes_iomd_inputs`, `test_C_v3_slave1_silent_needs_a_link` (con C1: v3 slave 1
  mudo, v4 slave 1 audible).
* Lo que no se sabe: qué palabras del slave 1 llegan a qué entradas del slave 0 (C1 supone `6060`–`6063` →
  `6000`–`6003`; también encaja `6020`/`6021` → `6000`/`6001`). El barrido de `WAVE_BUS.md` §4b oyó `6063` en los dos
  canales sin que nadie lo sumara, lo que C1 no explica. **Abierto.**
* Medida propuesta: C-V4 con notas del slave 1 (voces 4–7; `voicelog.py` dice qué slave). Si suenan, C1 o su variante
  es correcta. Para fijar la palabra: un MAIN4 que sume una sola entrada `60nn` por instalación y el motor con un
  tono distinto por slave (el ARM conoce `e->slave` y puede escribirlo en el banco 5).

**D. ¿Quién escribe `[2220]`–`[222A]`?**
* Nadie [listado]: son las salidas de las ranuras (`[221B]`–`[2229]`, impares) leídas una muestra después en la
  dirección +1, porque **el banco 1 es un anillo como el banco 0**. Las ocho salidas DCO se leen todas a +1 y ninguna
  como se escriben; ningún programa escribe las pares. Lo mismo en los cinco plug-outs de la familia A. MAIN0 usa el
  mismo idioma (guarda `[2200]` al final, lee `[2201]` en la muestra siguiente).
* No es el motor ARM: `engine.c` y `engine_v3.c` sólo escriben el banco 5 (`p_bank`, OS `+0xB0`) y los parámetros
  (`p_ramp`, `+0xA0`) [código]. Que ningún otro código del OS escriba el banco 1 no está comprobado; el patrón exacto
  +1 de las ocho salidas lo hace innecesario.
* Pruebas: `test_D_listing_slot_outputs_read_plus_one`, `test_D_ring_decides_v4_and_v4b`.
* Consecuencia para v3: leer `[2221]`/`[2229]` devuelve, con anillo, `[221D]`/`[2225]` de hace 4 muestras (la misma
  sierra, porque nuestro SAW escribe también esas palabras). Por eso v3 funciona igual con o sin anillo y no sirve
  para decidirlo.
* Medida que lo confirma: C-V4 (lee `[2222]`/`[222A]`) suena ⇒ anillo; C-V4 mudo y C-V4B (lee `[2221]`/`[2229]`)
  suena ⇒ no hay anillo.

**E. ¿Comparten banco 1 DSP2 y DSP3?**
* No, según la tabla de bancos del OS del slave (`s_boot` `0x0111C580`): bases de DT1 `0x41510000` (DSP0),
  `0x41510000` (DSP1), `0x41610000`, `0x41710000`, `0x41810000`, `0x41910000` [listado]. Sólo DSP0 y DSP1 comparten
  base. **Corrige** `DSP_ARM_SIDE.md` (decía "compartido por el par" en general).
* Además, MAIN2 y MAIN3 escriben las mismas direcciones `[2200]`–`[221A]` en cada muestra; compartidas se pisarían
  [listado].
* Pruebas: `test_E_arm_table`, `test_E_shared_bank1_would_cross_voices` (si se compartiera, la voz 0 de v4 tocaría la
  sierra de la voz 2).
* Abierto: si DSP0 y DSP1 comparten de verdad el banco 1 (la tabla puede ser un alias del ARM). Medida: leer
  `0x41410000` y `0x41510000` desde el ARM con un programa que escriba un valor distinto en `[2210]` de DSP0 y DSP1.

**F. ¿Corre el principal antes que la ranura?**
* Sí, si el banco 1 es anillo. MAIN2 de Roland escribe en `[2207]` el tono de DCO2 (acotado a [−4, 10]) y en
  `[2206]` el parámetro `A02A`, que no es un tono [listado]. C-SAW (principales de Roland, nuestro SAW leyendo
  `[2207]`) está afinado de la nota 36 a la 96 [documentado, `CUSTOM_OSC.md`]. Con anillo, una ranura que corriera
  antes leería en `[2207]` el `[2206]` de la muestra anterior y no estaría afinada. Sin anillo el orden no cambia el
  tono (la ranura leería la misma palabra una muestra tarde).
* Pruebas: `test_F_csaw_layout_needs_main_first` (programa propio con la disposición de MAIN2: afinado con principal
  primero, a más de 100 cent con ranura primero) y `test_F_unison_does_not_decide`: el unison afinado de C-V3L
  [medido] **no** decide F, porque con ranura primero la voz B del SAW lee `[2214]` = el `[2207]` de 13 muestras antes
  (el tono de la voz A) y la puerta de la voz 0 de v3 toca la voz B.
* Consecuencia para la medida del paso 1 (§6.5): con anillo y principal primero se oye la nota de la voz 1; con anillo
  y ranura primero se oiría la de la voz 0 (y C-SAW no estaría afinado); sin anillo, la de la voz 1.
* El orden DCO1 → DCO2 [inferido]: nuestro programa nulo de DCO1 y el SAW de DCO2 guardan la fase en `[0670]`; si el
  nulo corriera después, la voz A del SAW no avanzaría.

**G. ¿Llega DSP2 Z 0405 a DSP4 M 0006?**
* No: llega a M 0005. MXM conserva el número de palabra [medido en el maestro, `DSP_HARDWARE.md` §8.2] y Roland
  copia `M 0nn → Z 4nn` sin desfase [listado]. El "DSP2 Z 404–41A → DSP4 M 005–01A" de §1 son rangos de uso, no un
  desfase: MAIN4 no usa M 004.
* Pruebas: `test_G_listing_forwards_keep_the_word`, `test_G_shift_would_make_voice1_heard` (con desfase 1, la voz 1
  de v3 sonaría; no suena [medido]).

**H. ¿Afinación de mono ±40 cent?**
* La cadena no desafina [emulador]: v4 voz 0 y v3 (voz 1 con la misma nota) a menos de 1 cent para las notas 48, 64
  y 76 (`test_H_chain_is_in_tune`). El SAW solo está afinado a ±1.2 cent de la nota 36 a la 96 [documentado, `CUSTOM_OSC.md`].
* Por A, en v3 lo que se oye con la puerta de la voz 0 es el tono de la **voz 1**. En mono, el tono que se oye depende
  de qué escribe el OS en el tono de la voz 1, no de la nota de la voz 0. [inferido] Si en mono el OS manda la nota
  también a la voz 1 (como en unison), la afinación sería exacta; un error de ±40 cent indica otra fuente (cambio de
  tono durante la ventana de medida, o glide del modo mono).
* Medida propuesta: `voicelog.py` en mono con notas largas (`--hold 3`), mirando los mensajes de tono de las voces 0 y
  1 del registro y la frecuencia en tres tramos de la nota. Con C-V4 (sin el cruce de A) el error debería desaparecer
  si la causa era el tono de la voz 1.

### 6.4 Correcciones a afirmaciones anteriores

| dónde | decía | corrección | evidencia |
|---|---|---|---|
| §2.5 | `[2220]`–`[222A]` las escribe algo fuera del microcódigo (¿ARM?) | son las salidas de las ranuras vistas una muestra después (banco 1 anillo) | [listado], [inferido] |
| §2.5 | DSP2/DSP3 comparten banco 1 "inferido, contradicho en parte" | no lo comparten (tabla del ARM); sólo DSP0/DSP1 tienen la misma base | [listado] |
| §2.5 | la voz 0 de C-V3 suena con su nota | la puerta de la voz 0 abre la sierra de la voz 1 | [emulador], falta medirlo (A) |
| §1, tabla | DSP5 escribe IOMD 60B0… | también, y sus voces van a DSP4 por `A400`–`A403`; 60B0 no se oye | [listado], [medido] |
| `WAVE_BUS.md` §2 y `DSP_MAIN_PROGRAMS.md` §1 | "DSP5 no genera la voz; la reenvía" | DSP5 procesa las voces 2 y 3 y las entrega a DSP4 por la ventana de pareja | [listado] |
| `DSP_HARDWARE.md` §7.1 | banco 5 `0x400`–`0x7FF` "sólo dentro de la muestra" | es la ventana de la pareja: lo que escribe un DSP lo lee el otro | [listado], [inferido] |
| `make_v3.py` | DSP2 Z 404/405 → DSP4 M 005/006 | → M 004/005 | [medido en maestro], [listado] |
| `emu/chain.py` previo | "solo la voz 2 está comprobada" | no comprobaba nada: perdía las escrituras | [código] |

### 6.5 Candidato v4 y plan de medida (C-V4 instalado y medido el 2026-10-22, ver §6.7)

`examples/from_scratch/v4/make_v4.py` genera MAIN0–5 propios con el flujo de §6.2: lee las salidas del SAW a +1,
DSP4 lee M 004/005, DSP5 deja las voces 2 y 3 en `A400`/`A401`, DSP4 suma `A400`/`A401` e IOMD `6000`–`6003` y
escribe `6060`–`6063`. `build_v4.sh` lo empaqueta con `engine_v3.c` sin tocar nada de v3 ni de `engine/`
(`V4B=1` → la variante sin anillo). Firma limpia (§7). El emulador da las cuatro voces a su tono
(`test_regression_v4_chain_all_voices`).

Riesgo: si `6000`–`6003` del slave 0 traen algo distinto de lo supuesto (p. ej. su propia salida), v4 puede
realimentarse. Roland las multiplica por coeficientes del banco 6 (`C044`–`C048`) que no conocemos.

Orden de medida propuesto (una instalación cada vez; tras cada una el usuario vuelve a elegir PLG3; nunca dos
herramientas de MIDI a la vez):

| paso | build | medida | resultado que confirma / descarta |
|---|---|---|---|
| 1 | C-V3L (ya instalado) | `voicelog.py --groups 57 69`, poly | 220 Hz (nota de la voz 1) con la puerta de la voz 0 ⇒ A confirmado; 440 Hz ⇒ anillo con ranura primero, en contra de C-SAW: revisar el modelo |
| 2 | C-V4 (`VOICELOG=1`) | `voicelog.py` (8 notas sueltas y un acorde) | voces 0–3 ⇒ A, D, B, G; voces 4–7 ⇒ C1 — **hecho: 8/8, §6.7** |
| 3 | C-V4B, sólo si en el paso 2 no suena nada | igual | suena ⇒ banco 1 sin anillo (D falso) — **no hace falta** |
| 4 | C-V4 en mono | `voicelog.py --hold 3` | afinación < 5 cent ⇒ H era el cruce de A |

### 6.6 Qué es seguro y qué sigue abierto (actualizado con §6.7)

**Medido con C-V4:** las ocho voces suenan con su nota en los dos slaves, sueltas y en acordes. Con eso quedan
confirmados en el equipo, en conjunto:
* **A / G:** DSP4 debe leer M 004/005: el número de palabra MXM se conserva.
* **D:** el banco 1 es un anillo. C-V4 sólo lee las salidas del SAW en `[2222]`/`[222A]`, que nadie escribe; sin
  anillo estaría mudo (`test_D_ring_decides_v4_and_v4b`).
* **B:** la ventana de pareja `A400`/`A401` lleva las voces de DSP5 a DSP4. Sin ella las voces 2 y 3 estarían mudas.
* **C:** el slave 1 se oye cuando el MAIN4 del slave 0 suma IOMD `6000`–`6003` (o cuando el slave 1 escribe
  `6060`–`6063`; C-V4 hace las dos cosas).
* **F:** principal antes que ranura, como consecuencia de D y de C-SAW.

**Seguro sin medida nueva:** la topología y los buzones de Roland (§6.2), iguales en los cinco plug-outs de la
familia A [listado]; que 60B0 no se oye [medido]; la tabla del ARM con un banco 1 propio para DSP2–DSP5 [listado].

**Sigue abierto:**
* Qué palabras exactas del slave 1 llegan a qué entradas `6000`–`6003` del slave 0. Para separarlo, harían falta dos
  builds: una que sólo escriba `6060`–`6063` y otra que sólo sume `6000`/`6001`.
* La predicción de A para C-V3L (paso 1: la puerta de la voz 0 tocaría la nota de la voz 1). Ya no es necesaria para
  arreglar, pero confirmaría el diagnóstico de v3. Quedó sin medir porque C-V3L fue reemplazado en el slot 3.
* H (mono ±40 cent): sin medir con C-V4 (paso 4; hay que poner el equipo en mono).
* Por qué `6063` se oyó en los dos canales en el barrido de `WAVE_BUS.md` §4b. Si DSP0 y DSP1 comparten de verdad el
  banco 1.

### 6.7 Medida de C-V4 en el equipo (2026-10-22)

Build: `VOICELOG=1 NAME=C-V4 sh v4/build_v4.sh`. `emu/sigcheck.py` da limpios `engine_0x60A00000.bin` y
`engine_0x60C00000.bin`. Se comprobó que la ranura `V3M4` de la build contiene `v4m4.hex`. Instalación:
`tools/plugout_tool.py install plugins/C-V4 --slot 3 --force --no-patches -y` (reemplaza C-V3L). El usuario
seleccionó PLG3 y lo escuchó: "parece todo ok".

`voicelog.py` (notas sueltas, 1.5 s cada una) [medido]:

| nota | voz (registro) | slave | esperado Hz | oído Hz | audible |
|---|---|---|---|---|---|
| 48 | 0 | 0 | 130.8 | 130.9 | sí |
| 52 | 6 | 1 | 164.8 | 164.5 | sí |
| 55 | 2 | 0 | 196.0 | 196.3 | sí |
| 60 | 3 | 0 | 261.6 | 261.8 | sí |
| 64 | 4 | 1 | 329.6 | 330.0 | sí |
| 67 | 5 | 1 | 392.0 | 391.8 | sí |
| 72 | 1 | 0 | 523.3 | 523.6 | sí |
| 76 | 7 | 1 | 659.3 | 659.0 | sí |

`voicelog.py --groups 48,52,55,60 64,67,72,76` (dos acordes de cuatro notas): las ocho notas audibles, voces y slaves
iguales que arriba, frecuencias a ±0.4 Hz de la tabla. Resumen de las dos ejecuciones: slave 0 4/4, slave 1 4/4.

Lectura:
* Cada una de las ocho voces (local 0–3 en cada slave) suena con su nota. Las diferencias son de −3.2 a +2.6 cent,
  dentro de la resolución de la medida por FFT (±1.2 cent en `CUSTOM_OSC.md` con ventanas más largas).
* Frente a C-V3L (sólo la voz 0 audible), los cambios de v4 son los de §6.5, y cada uno era necesario según el
  emulador (`test_chain.py`: sin anillo, sin ventana de pareja o sin la suma de `6000`–`6003`, la voz o el slave
  correspondiente queda mudo).
* No hubo realimentación ni ruido por sumar `6000`–`6003` (el riesgo de §6.5): línea base rms 1·10⁻⁵ antes de las
  notas.
* En el registro, las voces 3 y 4 no tienen mensaje de nota (`kind 0`), sólo de velocidad [medido]. [inferido] El OS
  no reenvía la nota si la voz ya la tenía de una ejecución anterior. El tono oído es el correcto, así que el motor ya tenía el valor.

## 7. Comprobación de firmas (sin bytes de Roland)

`examples/from_scratch/emu/sigcheck.py FICHERO…` busca en una build (`.bin`) cada ventana de 16 bytes (4 palabras de
32 bits, como están en la imagen) de los 17 programas de Roland de `engine/programs.py`, y en un programa (`.hex`)
cada serie de 8 palabras de 16 bits. Las ventanas que también están en nuestros programas (marcas genéricas como
`0a02 ff70`) no cuentan. Sale con 1 si encuentra algo. Control positivo: un trozo de MAIN4 se detecta en los dos
formatos.

Resultado 2026-10-22: `v3m0`–`v3m5`, `v4m0`–`v4m5`, `v3saw`, `dco2_null` y `engine/build/engine_0x60A00000.bin` /
`engine_0x60C00000.bin` (la última build del motor) limpios. `hyb_*.bin` (la build híbrida antigua) contiene los 17
programas, como era de esperar. Hay que ejecutarlo sobre la build de v4 antes de instalarla.
