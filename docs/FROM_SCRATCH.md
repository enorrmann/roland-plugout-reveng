# Plug-out propio desde cero (C-V4): cómo está hecho y cómo reconstruirlo

Estado: 2026-10-22. **C-V4 funciona en el equipo**: ocho voces (cuatro por slave) con su nota, sueltas y en acordes,
afinadas a ±3 cent (`VOICE_ALLOCATION.md` §6.7). Todo el código y todo el microcódigo de DSP son nuestros. La
comprobación de firmas (`emu/sigcheck.py`) no encuentra bytes de Roland en la build.

Este documento sustituye a `examples/from_scratch/MINI_PLUGOUT.md` (diario del 2026-10-21; lo que sigue vigente de él
está en §11). Etiquetas: **[medido]** = en el equipo; **[código]** = está en nuestro código; **[listado]** = leído del
microcódigo de Roland desensamblado (sólo estudio); **[inferido]** = deducción no medida.

Rutas relativas a `examples/from_scratch/` salvo que se diga otra cosa.

---

## 1. Qué hay que construir

Un PLUG-OUT del SYSTEM-8 es un fichero `Code8_<slot>.Dat` (uno por slot, 1–3) con **dos regiones de código ARM**:

| región | corre en | qué hace | nuestro código |
|---|---|---|---|
| 0 ("modelo") | BMC maestro | SYNINFO, lista de parámetros del panel, memoria de patches, **asignador de voces** (qué nota va a qué voz) | `model/` |
| 1 ("motor") | los dos BMC esclavos (slave 0: voces 0–3, slave 1: voces 4–7) | carga los programas de los seis DSP del slave y traduce notas y parámetros a escrituras en el DSP | `v3/engine_v3.c` |

Y, dentro de la región 1, **el microcódigo de los DSP**: un programa principal por DSP (seis) y los programas de ranura
(osciladores DCO1/DCO2 en DSP2 y DSP3; efecto en DSP4). Es lo que genera el sonido.

```
 PC ──USB-MIDI──► maestro (región 0: asignador) ──parámetros 0x429+v (nota), 0x43A+v (velocidad)──►
      slave 0 / slave 1 (región 1: engine_v3.c) ──banco 5 de cada DSP──► DSP0..DSP5 ──► audio
```

C-V4 usa la región 0 propia de `model/` (la de C-SAW), el motor `v3/engine_v3.c` sin cambios y los seis principales de
`v4/`. Los programas de ranura son los de v3: nuestra sierra en DCO2 y nuestro programa nulo en DCO1.

## 2. El flujo de una voz en C-V4

Cada slave tiene seis DSP. Las voces locales 0 y 1 viven en DSP0 → DSP2 → DSP4; las 2 y 3 en DSP1 → DSP3 → DSP5 →
DSP4. Es la misma disposición que usa Roland en JX-3P, JUNO-60, JUNO-106, JUPITER-4 y JUPITER-8 (`VOICE_ALLOCATION.md`
§6.2).

```
ARM: tono  → banco 5 de DSP0 (voz 0: A021, voz 1: A033)   y de DSP1 (voz 2: A021, voz 3: A033)
ARM: puerta→ banco 5 de DSP4 (voz 0: A02A, voz 1: A039)   y de DSP5 (voz 2: A029, voz 3: A038)

DSP0 v4m0: A021 → Z 404, A033 → Z 405 ─MXM─► DSP2 M 004 / 005
DSP1 v4m1: A021 → Z 400, A033 → Z 401 ─MXM─► DSP3 O 200 / 201
DSP2 v4m2: M 004 → [2207], M 005 → [2214]  ─► ranura DCO2 (sierra): [2207] → [2221], [2214] → [2229]
           [2222] → Z 404, [222A] → Z 405 (las salidas, una muestra después) ─MXM─► DSP4 M 004 / 005
DSP3 v4m3: lo mismo con O 200/201 → Z 400 / 401 ─MXM─► DSP5 O 200 / 201
DSP5 v4m5: O 200 × puerta(A029) → A400,  O 201 × puerta(A038) → A401  (buzón de la pareja DSP4/DSP5)
DSP4 v4m4: M 004 × puerta(A02A) + M 005 × puerta(A039) + A400 + A401 + IOMD 6000..6003 → 6020, 6021 (audio)
           y las cuatro voces sueltas → 6060..6063 (las que el otro slave suma por 6000..6003)
```

Las reglas del hardware que hacen que esto funcione, todas comprobadas en el equipo con C-V4 o antes:

| regla | consecuencia en el código | evidencia |
|---|---|---|
| **MXM** (banco 4, `8nnn`): lo que un DSP guarda en Z `4nn` lo lee el DSP de la pareja siguiente en M `nn` (si el escritor es par) u O `nn` (si es impar), una muestra después; si nadie escribe, se lee 0 | escribir **cada muestra**; leer con el **mismo** número de palabra | [medido] maestro; C-V4 |
| **Banco 1** (`2nnn`) y banco 0 son **anillos**: lo guardado en R en la muestra n se lee en R+1 en la n+1 | el principal escribe la entrada del oscilador (`[2207]`) y la ranura la lee en la misma muestra; la ranura escribe la salida (`[2221]`) y el principal la lee **en `[2222]`** en la muestra siguiente | [medido] con C-V4 |
| El **principal corre antes** que las ranuras de su DSP | por eso la entrada se lee en la misma dirección | [inferido] del anillo + C-SAW afinado |
| **Banco 5 `A400`–`A7FF`** es un buzón compartido por la pareja (0,1), (2,3), (4,5) | DSP5 deja sus voces en `A400`/`A401`, DSP4 las suma | [medido] con C-V4 |
| Sólo **`6020`/`6021`** (IOMD de DSP4) llegan al audio; `60B0`–`60B7` no | la mezcla final va a `6020` y `6021` | [medido] `WAVE_BUS.md` §4b |
| El **slave 1** no llega al audio por su propio `6020`; llega al slave 0 por IOMD `6000`–`6003` | DSP4 suma `6000`–`6003` y escribe `6060`–`6063` | [medido] con C-V4 (qué palabra exacta: abierto) |

Los errores de v3 eran justo tres de estas reglas: leía M 005/006 en DSP4 (desfase inventado), mandaba las voces 2 y 3 a
`60B0` y no sumaba al otro slave (`VOICE_ALLOCATION.md` §6.3).

## 3. El microcódigo: ficheros `.s`, `.hex` y por qué hay ensamblador dentro de los `.py`

### 3.1 Qué es cada formato

| extensión | qué contiene | quién lo genera | quién lo usa |
|---|---|---|---|
| `.s` | código fuente en el lenguaje ensamblador de `asm/` (`ld r1, [8004]`, `mul r1, r1, r4`, `st r1, [6020]`). Legible, para revisar | `make_v3.py` / `make_v4.py` | personas; `asm/asm.py` lo puede volver a ensamblar |
| `.hex` | el programa ensamblado: **una palabra de 16 bits por línea**, en orden de ejecución, con una línea de comentario `#` | `make_v3.py` / `make_v4.py` (llaman a `asm/asm.py`) | `engine/programs.py` (lo mete en `programs.h`), el emulador, `sigcheck.py` |
| `.txt` (en `re/dspexp/own/`) | lo mismo que `.hex`, varias palabras por línea | `re/dspasm.py` (la sierra) | `programs.py`, `make_v3.saw_dco2_words()` |

Cada programa empieza con `0a02 ff70` (marca de inicio; no hace nada [medido], Roland la pone igual) y termina con
`0b02 00f8 0001` (la secuencia de fin que usan todos los programas de Roland). `make_v3.words()` las añade.

Ejemplo completo, `v4/v4m0.s` (DSP0):

```
; v4m0: voices 0 and 1 pitch -> MXM Z 404, 405 (DSP2 M 004, 005)
ld r1, [a021]
mov r4, r4
st r1, [8404]
ld r1, [a033]
mov r4, r4
st r1, [8405]
```

y su `v4/v4m0.hex` (aquí en una línea; en el fichero, una palabra por línea), con cada instrucción anotada:

```
0a02 ff70                inicio
0c03 1fe0 a021           ld r1, [a021]     (cabecera 0c03 = clase y largo, 1fe0 = carga a r1 desde la referencia, a021)
0902 4044                mov r4, r4
0c03 9fe0 8404           st r1, [8404]     (9fe0 = store de r1)
0c03 1fe0 a033           ld r1, [a033]
0902 4044                mov r4, r4
0c03 9fe0 8405           st r1, [8405]
0b02 00f8 0001           fin
```

El formato de las instrucciones está en `DSP_ISA_SPEC.md`.

El `mov r4, r4` entre la carga y su uso es una instrucción de margen: una carga tarda una instrucción en estar
disponible para las operaciones [medido, `DSP_HARDWARE.md` §7.3]. Otras reglas que respetan los programas: como mucho
una carga y un store al banco 5 por instrucción; hasta 6 transferencias por instrucción.

En la build, `engine/programs.py` empaqueta cada `.hex` en un array C `mc_<NOMBRE>[]` de palabras de 32 bits (la mitad
alta primero, como en la imagen de Roland) más `MC_<NOMBRE>_BYTES`, dentro de `engine/build/programs.h`.

### 3.2 Los nombres `v4m0` … `v4m5`: qué significan y dónde corre cada uno

El nombre tiene tres partes: **`v4`** = versión del plug-out (v3, v4…), **`m`** = programa principal ("main", el que
corre en la ranura `0x0100` de un DSP en cada muestra), **el dígito** = **el DSP donde corre**, 0 a 5. Así `v4m2` es el
principal de la versión 4 que corre en el DSP2. Es la misma numeración que los principales de Roland (`MAIN0` …
`MAIN5`, DSP0 … DSP5).

| programa | DSP | voces locales | qué hace |
|---|---|---|---|
| `v4m0` | DSP0 | 0, 1 | toma del banco 5 el tono de las voces 0 y 1 (`A021`, `A033`) y lo manda a DSP2 por MXM (Z `404`/`405`) |
| `v4m1` | DSP1 | 2, 3 | lo mismo para las voces 2 y 3, hacia DSP3 (Z `400`/`401`) |
| `v4m2` | DSP2 | 0, 1 | pasa los dos tonos a las entradas de la ranura DCO2 (`[2207]`, `[2214]`), lee las dos sierras (`[2222]`, `[222A]`) y las manda a DSP4 |
| `v4m3` | DSP3 | 2, 3 | lo mismo para las voces 2 y 3, hacia DSP5 |
| `v4m4` | DSP4 | 0, 1 (+ 2, 3 y el otro slave) | aplica la puerta a las voces 0 y 1, suma las voces 2 y 3 que llegan de DSP5 por `A400`/`A401` y el otro slave por IOMD `6000`–`6003`, y escribe la salida `6020`/`6021` y las voces sueltas `6060`–`6063` |
| `v4m5` | DSP5 | 2, 3 | aplica la puerta a las voces 2 y 3 y las deja en el buzón `A400`/`A401` para DSP4 |

Cosas a tener en cuenta:

* **El dígito sí decide dónde corre**, pero no por el nombre del fichero: decide el **índice del array** en el motor.
  `e_load` registra `main_prog[d]` en el DSP `d` (servicio del OS `+0x40 + 4·d`, ranura `0x0100`) [código,
  `engine_v3.c`]. Que el índice `d` es el DSP `d` está medido con C-SAWPOLY y con C-V4 (con otro orden no sonaría).
* **Los dos slaves corren los mismos seis programas.** `v4m0` corre en el DSP0 del slave 0 (voces globales 0 y 1) y en
  el DSP0 del slave 1 (voces 4 y 5). Lo que cambia entre slaves es qué notas les manda el motor (`lv = voz − 4·slave`).
* **En la build se cargan con los nombres de v3.** `engine_v3.c` pide `V3M0` … `V3M5` (arrays `mc_V3M0` …).
  `build_v4.sh` hace que esos nombres contengan los v4 (`OWN_PROGS=v3m0=v4/v4m0.hex,…`), así que `v4m2.hex` acaba en
  `mc_V3M2`, que es `main_prog[2]`, que corre en DSP2.
* **Programas de ranura**, que no siguen esta regla de nombres: `v3saw` (sierra, ranura DCO2 `0xB000` de DSP2 y DSP3)
  y `dco2_null` (programa nulo, ranura DCO1 `0x6000` de DSP2 y DSP3; el nombre viene de que se escribió para DCO2).
  En DSP0/DSP1 (ranura `0x8000`) y DSP4/DSP5 (ranuras `0x6000` y efecto `0xB000`) van programas vacíos del propio
  motor (`mc_empty`).
* `v4bm0` … `v4bm5` (sólo con `make_v4.py --no-ring`): la variante C-V4B. Misma regla: el dígito es el DSP.

### 3.3 Por qué el ensamblador está dentro de los `.py`

`make_v3.py` y `make_v4.py` no contienen programas fijos: son **generadores**. Cada principal se escribe como una
función de Python que recibe direcciones y devuelve líneas de ensamblador. Por ejemplo `prog_pitch(name, desc, out0,
out1)` produce el programa de DSP0 **y** el de DSP1 cambiando sólo la palabra Z de destino; `prog_dco_pair` hace DSP2 y
DSP3. Después `words()` llama a `asm.Assembler().assemble(texto)` (el ensamblador de `asm/`, en el mismo proceso) y
escribe los `.s` y `.hex`.

Las razones:

1. **Una sola fuente para las direcciones.** Los ids de tono y puerta (`PITCH_A`, `GATE`), las entradas y salidas del
   oscilador (`DCO_IN`, `DCO_READ`), el buzón (`PAIR`) y las palabras IOMD son constantes de Python. Las usan a la vez
   los programas, el emulador (`emu/`) y las pruebas (`emu/test_chain.py`). Si una dirección cambia, cambia en todos.
2. **Programas casi iguales sin copiar y pegar.** Los pares DSP0/DSP1, DSP2/DSP3 y DSP4/DSP5 se diferencian en dos o
   tres direcciones. `make_v4.py` sólo redefine las funciones que cambian respecto de v3 y reutiliza el resto
   (`v3.prog_pitch`, `v3.words`).
3. **Variantes por opción.** `make_v4.py --no-ring` genera v4b (lee `[2221]`/`[2229]` en vez de `[2222]`/`[222A]`)
   con la misma función y otro parámetro.
4. **El emulador ensambla en memoria.** `emu/voices.py` llama a `make_v4.sources()` y ensambla sin pasar por ficheros,
   así una prueba puede modificar un programa (p. ej. `roland_layout_dsp2()` en `test_chain.py`).

Los `.s` se escriben para leerlos y revisarlos. **No se editan a mano**: se regeneran con `python3 make_v4.py`, y la
build lo hace sola. Si algún día un programa crece y deja de ser paramétrico, puede vivir como `.s` y ensamblarse con
`asm/asm.py` directamente; el `.hex` resultante entra en la build igual.

La sierra (`re/dspexp/own/saw_dco2.txt` = `v3/v3saw.hex`) es otro generador: `re/dspasm.py saw dco2` calcula sus
constantes (incremento de fase por la frecuencia de muestreo de 96 kHz) y escribe las palabras.

## 4. El motor ARM (región 1): `v3/engine_v3.c`

Compila para Cortex-M4 (`arm-none-eabi-gcc`), sin biblioteca estándar. Habla con el OS del slave sólo por su tabla de
servicios en `0x0113FC00` [código]:

| servicio | uso |
|---|---|
| `+0x00` `get(part, grp, id)` | leer un parámetro (p. ej. la nota `0x429+v`) |
| `+0x28/+0x40/+0x58 + 4·dsp` | registrar ranuras, programas y bloques de datos de cada DSP (en `e_load`) |
| `+0x70` `swap` | cambiar el programa de una ranura y activar su tarea (DCO1 = tarea 2, DCO2 = 3) |
| `+0xA0` `p_ramp` | escribir un parámetro con rampa (banco 6) |
| `+0xB0` `p_bank(dsp, &id, v)` | escribir la palabra `A000 + id` del banco 5 de un DSP: **así llegan tono y puerta** |
| `+0xC0` `sync` | esperar al límite de trama del DSP antes de un cambio |
| `+0xF0` | índice del slave (0 o 1) |

Piezas del fichero:

* **Descriptores** `dspdesc_t {addr, n1, n2, src, os[2]}`: dirección de la ranura (`0x0100` principal, `0x6000` DCO1,
  `0xB000` DCO2/efecto, `0x8000` en DSP0/1), tamaño dos veces y puntero al array de `programs.h`. Los dos últimos
  campos los usa el OS; un descriptor no puede compartirse entre DSP.
* `main_prog[6]` = `V3M0`…`V3M5` (en C-V4 esos nombres contienen los v4: `build_v4.sh` los mapea), `dco1_prog` =
  `DCO2NULL`, `dco2_prog` = `V3SAW`, `fx_prog` = vacíos.
* **`e_load`** (vtable `+0x08`): registra en cada DSP sus ranuras, programas y datos.
* **`e_start`** (`+0x0C`): manda el estado de los parámetros, activa las ranuras con `swap` y pone las puertas a 0.
* **`e_param`** (`+0x30`): `0x429 + voz` = nota → `voice_pitch` (banco 5 de DSP `lv>>1`, id `0x21 + 0x12·(lv&1)`,
  valor `(nota − 12)/12`, así 4.75 = La 440); `0x43A + voz` = velocidad → `voice_gate` (banco 5 de DSP `4 + (lv>>1)`,
  id `0x2A + 0xF·(lv&1) − (lv>>1)`, 1.0 o 0.0). `lv` = voz menos `4·slave`.
* El resto de parámetros pasa por la tabla `model.h`, que genera `engine/gen_model.py` desde `engine/model_spec.py`
  (el modelo de parámetros del JX-3P reproducido). Los principales de v4 no leen esos parámetros; queda para cuando
  haya filtro, envolvente y LFO.
* **Exportaciones**: `plugout_init` (devuelve el descriptor de creación) y la estructura `syninfo` en la sección
  `.syninfo`, que el OS busca al cargar la región.

## 5. La región 0: `model/`

Documentada en `examples_from_scratch_model_README.md`. Lo esencial:

* `model.c`: SYNINFO, descriptor, crear/destruir/seleccionar y el **asignador** (17 entradas de vtable). Envía a los
  slaves la nota (`0x429 + voz`) y la velocidad (`0x43A + voz`, 0 = soltar).
* `params.py`: la lista de parámetros del panel (ids del JX-3P, porque el motor los usa) y la memoria de 64 patches
  INIT dentro de la propia región (por eso la región 0 no puede ser pequeña).
* `VOICELOG=1` compila un registro de diagnóstico: cada nota y velocidad asignada sale por SysEx
  `F0 7D 05 <tipo> <voz> <valor> <nota> F7` en el MIDI 1 del equipo. `voicelog/voicelog.py` lo cruza con el audio. La
  nota sólo se envía si cambia (`retune`), así que una voz que repite nota no manda mensaje de nota.

## 6. Build paso a paso

```sh
cd examples/from_scratch
VOICELOG=1 NAME=C-V4 sh v4/build_v4.sh        # -> plugins/C-V4/Script/Code8_{1,2,3}.Dat
python3 emu/sigcheck.py engine/build/engine_0x60A00000.bin engine/build/engine_0x60C00000.bin
```

Requisitos: `arm-none-eabi-gcc/ld/objcopy`, Python 3 y el PLUG-OUT de Roland `plugins/JX-3P` en el disco (ver §10,
dependencia que queda).

Qué hace cada script, en orden:

1. **`v4/build_v4.sh`**: `python3 make_v4.py` (escribe `v4m0..5.s/.hex`); fija `NAME` (C-V4, o C-V4B con `V4B=1`) y
   `OWN_PROGS=v3m0=v4/v4m0.hex,…,v3saw=v3/v3saw.hex,dco2null=gate/dco2_null.hex`; llama a `v3/build_v3.sh`.
2. **`v3/build_v3.sh`**: regenera los v3 (no se usan en C-V4) y crea `v3/build_engine.sh`, una copia de
   `engine/build.sh` que compila `v3/engine_v3.c` en vez de `engine/engine.c` (regla: `engine/` no se modifica).
3. **`v3/build_engine.sh`** (copia de `engine/build.sh`):
   1. `model/build.sh` compila la región 0 (con `-DVOICELOG` si `VOICELOG=1`) en dos direcciones (slot 1 y 2) y
      deduce las reubicaciones comparando las dos.
   2. `engine/programs.py plugins/JX-3P engine/build` escribe `programs.h` con los programas de `OWN_PROGS`.
   3. `engine/gen_model.py` escribe `model.h`.
   4. `arm-none-eabi-gcc` compila el motor; `ld` lo enlaza en `0x60A00000` y `0x60C00000`; las palabras que difieren
      son las reubicaciones.
   5. Arma el proyecto (`engine/build/projC/`: `plugout.json`, `region0.bin`, `region1.bin`, `relocs.json`) y
      `tools/plugout_build.py set-id` / `plugin` lo convierte en `plugins/<NAME>/Script/Code8_{1,2,3}.Dat`.
4. **`emu/sigcheck.py`**: confirma que la build no contiene bytes de Roland (sale con 1 si los encuentra).

## 7. Instalar y medir

```sh
python3 tools/plugout_tool.py install plugins/C-V4 --slot 3 --force --no-patches -y
# en el panel: seleccionar PLG3 sin tocar notas
cd examples/from_scratch/voicelog
python3 voicelog.py                                   # 8 notas sueltas
python3 voicelog.py --groups 48,52,55,60 64,67,72,76  # dos acordes
```

Reglas: una instalación cada vez y, después de cada una, volver a seleccionar PLG3; nunca dos herramientas que hablen
con el equipo (MIDI/ALSA) a la vez. `voicelog.py` graba el audio con `arecord -D hw:SYSTEM8,0` a 96 kHz.

## 8. Verificar sin el equipo: el emulador

`emu/` (detalle en `VOICE_ALLOCATION.md` §6):

```sh
cd examples/from_scratch/emu
python3 voices.py v4            # cada voz sola: nivel y frecuencia en 6020
python3 test_chain.py           # 20 pruebas (A–H y regresión), unos 20 minutos
python3 test_chain.py A D       # sólo algunas
```

`chain.py` ejecuta nuestros programas con el simulador de instrucciones `re/dspisa.py` y modela los bancos, MXM, el
buzón de pareja, el IOMD y el enlace entre slaves. Cada regla del hardware es un campo de `Model`, así que una prueba
puede comparar el modelo medido con la alternativa. **Antes de instalar un cambio de microcódigo, hay que pasarlo por
el emulador**: un programa que el emulador da mudo casi seguro estará mudo en el equipo.

## 9. Mapa de ficheros

**Lo que forma C-V4**

| fichero | qué es |
|---|---|
| `v4/make_v4.py` | generador de los seis principales de C-V4 (y de v4b con `--no-ring`) |
| `v4/v4m0.s` … `v4m5.s` | fuente ensamblador generado de cada principal (DSP0 … DSP5) |
| `v4/v4m0.hex` … `v4m5.hex` | los mismos programas ensamblados (entran en la build) |
| `v4/build_v4.sh` | build de C-V4 / C-V4B |
| `v3/engine_v3.c` | el motor ARM (región 1) |
| `v3/v3saw.hex` | la sierra de la ranura DCO2 (copia de `re/dspexp/own/saw_dco2.txt`, generada por `re/dspasm.py`) |
| `gate/dco2_null.hex` | programa nulo propio para la ranura DCO1 (no saca nada; la ranura no puede quedar vacía) |
| `model/` | la región 0 (`model.c`, `params.py`, `storage.S`, `link.ld`, `build.sh`) |
| `engine/programs.py`, `engine/gen_model.py`, `engine/model_spec.py`, `engine/expr.py`, `engine/link.ld`, `engine/build.sh` | piezas de build compartidas (no se modifican) |

**v3 (versión anterior, medida: sólo sonaba la voz 0)**

| fichero | qué es |
|---|---|
| `v3/make_v3.py` | generador de v3; también las funciones y constantes que reutiliza v4 |
| `v3/v3m0..5.s/.hex` | los principales de v3 (se conservan: la regresión comprueba que no cambien) |
| `v3/build_v3.sh` | build de C-V3 / base de la build de v4 |
| `v3/build_engine.sh` | **generado** por `build_v3.sh` (copia de `engine/build.sh` apuntando a `engine_v3.c`) |

**Herramientas**

| fichero | qué es |
|---|---|
| `emu/chain.py`, `emu/voices.py`, `emu/test_chain.py` | emulador de los DSP de los slaves, programas y motor, pruebas |
| `emu/listing.py`, `emu/allplugouts.py` | lectura de las referencias de bus de los programas de Roland (estudio, nada se copia) |
| `emu/sigcheck.py` | comprobación de firmas: sin bytes de Roland |
| `voicelog/voicelog.py` | medida en el equipo: voz asignada a cada nota y si se oye, con su frecuencia |
| `re/dspisa.py`, `re/dspdis.py`, `asm/asm.py` | simulador, desensamblador y ensamblador del DSP |

**Generados por la build (no se editan, no van al repositorio)**: `engine/build/` (`programs.h`, `model.h`, ELF/BIN,
`projC/`), `model/build/`, `plugins/C-V4/`.

## 10. Qué falta y deudas conocidas

* **Sonido**: hoy cada voz es una sierra con puerta (sin envolvente, filtro ni LFO). El siguiente paso natural es una
  envolvente en DSP4/DSP5 (la puerta ya está ahí) y después filtro y LFO. Los parámetros del panel siguen siendo los
  del JX-3P y casi ninguno hace nada.
* **Dependencia de la build**: `programs.py` sigue copiando los 17 programas de Roland a `programs.h` (comprueba su
  SHA-256 en `plugins/JX-3P`), aunque `engine_v3.c` no los usa y el compilador los descarta (lo confirma
  `sigcheck.py`). Para una build totalmente independiente, `programs.py` debería poder trabajar sólo con
  `OWN_PROGS`. No se ha cambiado porque `engine/` no se toca sin permiso.
* **Enlace entre slaves**: C-V4 suma `6000`–`6003` y escribe `6060`–`6063`; no se sabe cuál de las dos cosas usa el
  hardware ni qué palabra va a qué entrada.
* **Mono**: la afinación en modo mono no se ha medido con C-V4 (con C-V3L daba ±40 cent).
* **Nivel**: la mezcla suma hasta ocho sierras de amplitud 1 sin escalar; con acordes grandes puede saturar.

## 11. Historia (de `MINI_PLUGOUT.md`, 2026-10-21)

1. **Sierra propia en `asm/`** (`dsp/saw_dco2.s`): igual bit a bit que la de `re/dspasm.py` en simulación
   (`dsp/check_saw.py`; una constante 1/24 en 16 bits daba 1.45·10⁻⁶ de error, con `#…:32` 0).
2. **Prueba de salida** (`dsp/make_iomd_test.py`, `engine.c -DIOMDTEST`, plugin C-IOMD, sin notas): tres sierras fijas
   desde la ranura DCO1 de DSP5 [medido]:

   | palabra | resultado |
   |---|---|
   | `6020` | sale: 880 Hz, armónicos 1/n, canal derecho de la captura |
   | `60B0` | no sale (440 Hz ausente en los dos canales) |
   | `60B1` | no sale (660 Hz ausente) |

   El barrido completo de palabras IOMD está en `WAVE_BUS.md` §4b.
3. C-MINI / C-MINI2 / C-SUB3: voz monofónica con los principales de Roland; sólo el slave 0 llegaba al audio.
4. C-SAWPOLY (2026-10-22): cadena de Roland con nuestras sierras: ocho voces, el slave 1 sí llega [medido]. Contenía
   microcódigo de Roland, sólo para medir.
5. C-V3 / C-V3L: principales propios; sólo sonaba la voz 0. El emulador explicó por qué (`VOICE_ALLOCATION.md` §6.3).
6. **C-V4**: los tres arreglos; ocho voces [medido].

Decisiones de entonces que siguen en pie: puerta y tono por el banco 5 con los ids del JX-3P (los usa el asignador de la
región 0); quitar opciones del panel = quitar registros `P(...)` de `model/params.py` (no probado en el equipo).
