# SYSTEM-1: asignación de voces a los DSP — especificación técnica

| | |
|---|---|
| Versión | 2.0 (2026-10-22). Reescritura como especificación de la versión 1 (bitácora: [log/VOICE_ALLOCATION_LOG.md](log/VOICE_ALLOCATION_LOG.md)) |
| Alcance | Cómo llega una nota a una voz, qué DSP calcula cada voz en el sinte de serie, en los PLUG-OUTs de Roland y en los nuestros, la memoria de programa (principal y secciones) y el número de voces. Transporte entre DSP: [WAVE_BUS.md](WAVE_BUS.md) |
| Equipo | SYSTEM-1, OS 1.30. SYSTEM-8: [../docs/VOICE_ALLOCATION.md](../docs/VOICE_ALLOCATION.md) |
| Código de referencia | C-S1V4 / C-S1V8 (`examples/system_1/`, medidos); C-S1JX (`examples/system_1_jx/`, sin instalar) |

Evidencia: **[M]** medido · **[L]** leído en el OS o en los PLUG-OUTs de Roland · **[C]** nuestro código ·
**[E]** emulador · **[I]** inferido · **[A]** abierto. Bloques `dsp`: ensamblan (`python3 tools/doccheck.py
system-1/VOICE_ALLOCATION.md`).

---

## 1. Resumen

| | valor | evidencia |
|---|---|---|
| voces del sinte de serie | 4 | [L] |
| voces de un PLUG-OUT | las que reparta su asignador: **8 medidas** (C-S1V8) | [M] |
| ids de voz (grupo 2) | `0x110 + v` nota, `0x120 + v` velocidad (0 = soltar), `0x128 + v` disparo | [L], [M] |
| quién recibe las voces | el esclavo: motor de la región 1, `vt+0x30 param(id, 0, valor)` por cada id cambiado | [M] |
| DSP de voz | esclavo A y B (2) | [L] |
| modulación por voz (sinte de serie) | en el **maestro** (20 palabras por IOMD hacia el esclavo) | [L] |

---

## 2. De la tecla a la voz

### 2.1 Camino

```
tecla / MIDI ─► OS del maestro ─► asignador (SYNINFO+0x2C de la región 0; vtable de 9:
                                   reset, changed, note_off, note_on, cc, part 0x81, bend, -, -)
                 └─ servicios FC04 set(grupo, id, v), FC08 flush(grupo), FC0C touch, FC00 get
OS ─► esclavo: despachador 0x6004CE64 → motor vt+0x30 (id, 0, valor) para cada id cambiado de los grupos 0–3
OS ─► maestro: modelo (SYNINFO+0x30) vt+0x30 (id, flag, valor) para los parámetros (panel, VOLUME 0x100)
```

### 2.2 Ids de voz (grupo 2) [L], [M]

| id | contenido | bloque | evidencia |
|---|---|---|---|
| `0x110 + v` | nota | 16 ids (`0x110`–`0x11F`) | [M] v = 0–7 |
| `0x120 + v` | velocidad; 0 = soltar | 8 ids (`0x120`–`0x127`) | [M] v = 0–7 |
| `0x128 + v` | disparo (1/0) | 8 ids | [L] |
| `0x130` | bend | — | [L] |
| `0x131` | modulación | — | [L] |
| `0x134` | expresión | — | [L] |

### 2.3 Asignador y motor de un PLUG-OUT propio

El asignador de C-S1V4/C-S1V8 es la regla poly de [../docs/VOICE_ALLOCATION.md](../docs/VOICE_ALLOCATION.md) §2.3
con `NV` voces. El motor del esclavo recibe los ids y escribe el banco 5:

```c
/* examples/system_1/engine.c */
static void e_param(engine_t *e, int id, int flag, int value)    /* vt+0x30 */
{
    (void)flag;
    if (id >= 0x110 && id < 0x110 + NV)
        voice_pitch(e, id - 0x110, value);       /* FD6C(0, {0, 0, 0x30 + v}, pitch_bits[nota]) */
    else if (id >= 0x120 && id < 0x120 + NV)
        voice_gate(e, id - 0x120, value);        /* FD6C(0, {0, 0, 0x38 + v}, 1.0 / 0.0) */
}
```

`pitch_bits[n]` = bits IEEE de `(n − 12)/12` (sin FPU, tabla calculada en el PC por `make_s1.py`).

---

## 3. Sinte de serie [L]

### 3.1 Reparto

| DSP | principal | secciones de voz | qué |
|---|---|---|---|
| maestro | modulación (`0x60097A7C`, 696 instr.) y mezclador (`0x60095B0C`) | — | envolventes/LFO por voz → 20 palabras `600E`–`6021` al esclavo |
| esclavo A | `0x600185B0` (241 instr.) | 4, una por voz | prepara las entradas de las secciones (banco 1), recoge sus salidas, pasa datos y audio a B (`A100`–`A10D`) |
| esclavo B | `0x60018F44` (237 instr.) | 4, una por voz | alimenta y recoge sus secciones; salidas `6006`–`600D` × `C000` |

Los programas de voz vienen en sets (48 de voz, en parejas idénticas, 24 variantes; `system-1/voice_sets.py`); los
cuatro de un set sólo cambian en las direcciones de datos de cada voz.

### 3.2 Palabras del banco 1 por voz (sección ↔ principal)

| voz | escritas por el principal de A | escritas por el principal de B | leídas de vuelta |
|---|---|---|---|
| 0 | `2004`–`2007` | `2000`–`2003` | `2005`, `2007`, `2009` |
| 1 | `200E`–`2011` | `200A`–`200D` | `200F`, `2013` |
| 2 | `2014`–`2017` | `2010`–`2013` | `2015`, `2017`, `2019` |
| 3 | `201E`–`2021` | `201A`–`201D` | `201F`, `2023` |

Que A y B escriban en el mismo banco 1 sin pisarse indica que lo comparten [I]. Qué es cada palabra (tono, nivel,
forma): [A].

---

## 4. Memoria de programa: principal y secciones

Cada DSP del SYSTEM-1 tiene 32 KiB de programa ([../docs/DSP_PROGRAM_MEMORY.md](../docs/DSP_PROGRAM_MEMORY.md) §3):

```
0x0000  cabecera del OS (no cargar) 0x1900  sección 2      0x4900  sección 4
0x0100  PRINCIPAL (hasta 0x18FF, 6 KiB)   0x3100  sección 3      0x6100  sección 5
                                                                 0x7FF2  cierre
```

* El principal corre siempre; las secciones, sólo si su bit está puesto. El principal corre antes que las secciones
  [I] (los programas de Roland lo suponen).
* `FC44(índice, on)` enciende o apaga una sección; `FC40` coloca un programa de voz en una sección; el OS las apaga
  todas al recargar [L]. Índices:

| índice | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|---|---|---|---|---|---|---|---|---|
| DSP | A | A | B | B | A | A | B | B |
| sección | `0x1900` | `0x4900` | `0x1900` | `0x4900` | `0x3100` | `0x6100` | `0x3100` | `0x6100` |

* Los programas se cargan en **sets** `{dirección, bytes, puntero}` terminados en `{0, 0, 0}`, encolados con `FC30`
  (DSP A) o `FC34` (DSP B) desde `load` (maestro: vt[2] del modelo; esclavo: vt+0x08 del motor) [L], [M]. **Nunca
  con la entrada `0x0000`** (la cabecera del OS): encolarla cuelga el equipo [M] ([WAVE_BUS.md](WAVE_BUS.md) §5.3,
  con la secuencia de `load` de los PLUG-OUTs de Roland). Ejemplo,
  esclavo de C-S1JX (principal + sección encendida):

```c
/* examples/system_1_jx/engine.c */
static struct s1_prog set_a[] = {
    { 0x1900, MC_SA_SEC_BYTES, mc_SA_SEC },         /* sección 2 */
    { 0x0100, MC_SA_MAIN_BYTES, mc_SA_MAIN },       /* principal */
    { 0x7FF2, MC_CLOSE_A004_BYTES, mc_CLOSE_A004 }, /* cierre */
    { 0, 0, 0 },
};

static void e_load(engine_t *e)                                           /* vt+0x08 */
{
    for (int i = 0; i < 8; i++)
        ((slot_fn)SVC(0x44))(i, 0);                 /* todas las secciones apagadas */
    ((s1_queue_fn)SVC(0x30))(set_a);                /* DSP A */
    ((s1_queue_fn)SVC(0x34))(set_b);                /* DSP B */
    ((slot_fn)SVC(0x44))(SEC_A_1900, 1);            /* índice 0: A, 0x1900 */
    ((slot_fn)SVC(0x44))(SEC_B_1900, 1);            /* índice 2: B, 0x1900 */
    jx_send_state();
}
```

---

## 5. PLUG-OUTs: quién hace qué

| | voces | esclavo A | esclavo B | maestro A | maestro B | evidencia |
|---|---|---|---|---|---|---|
| SH-101 | 1 | principal 575 instr. (todo el sinte); secciones apagadas | 12 instr.: `A10A`–`A10C` × `C000` → `6006`–`600A` | 410 instr.: modulación → `600E`–`6021` | 436 instr.: mezcla | [L] |
| SH-2 | 1 | principal 108 + secciones `0x1900` (353) y `0x3100` (245) | 62 instr.: → `6006`–`600B`, `6026`–`602D` | 367 instr. | 406 instr.: mezcla, volumen (`C01C`…), saturación → `A118`/`A119` | [L] |
| PROMARS | 1 | principal 373 + sección `0x1900` (344) | principal 4 + sección `0x1900` (69) | 426 instr. | 313 instr. | [L] |
| **C-S1V4** | 4 | 4 sierras en el principal (137 instr.) → `A150` | `A150` × `C000` → `6006`/`6007` | (OS) | `6006`/`6007` × `C01C` → `A118`/`A119` | [M] |
| **C-S1V8** | 8 | 8 sierras (269 instr.) | ídem | (OS) | ídem | [M] |
| **C-S1JX** | 4 | MAIN5 + sección MAIN3 (voces 2, 3) | MAIN1 + sección DCO1 (voces 2, 3) | MAIN2 + MAIN4 (voces 0, 1) | MAIN0 + DCO1 + cola (voces 0, 1) | [C], [E] |

Instrucciones de los PLUG-OUTs de Roland: ejecutadas por muestra en el emulador (no tienen saltos: estático =
ejecutado), `system-1/s1_progs.py` ([DSP_FIT.md](DSP_FIT.md) §4).

### 5.1 C-S1V4: palabras por voz [M]

| voz | tono (ARM → banco 5 del esclavo) | puerta | fase (banco 0, anillo) | salida |
|---|---|---|---|---|
| v (0–3; 0–7 en C-S1V8) | `A030 + v`, `(nota − 12)/12` | `A038 + v` (1.0 / 0.0) | escribe `0300 + 0x10·v`, lee `+1` | suma × 1/NV → `A150` |

Voz 0 del principal del esclavo A (`examples/system_1/s1m0.s`; la fórmula de la sierra está explicada en
[../docs/VOICE_ALLOCATION.md](../docs/VOICE_ALLOCATION.md) §5.2):

```dsp
mov r5, #0.0
mov r6, #1.0
mov r7, #0.0             ; acumulador de la mezcla
ld r1, [0301]            ; fase de la voz 0
ld r0, [a030]            ; tono de la voz 0 (ARM)
mov r4, r4
add r0, #-4.75
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
addxy r2, r1, #-1.0
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
st r1, [0300]
ld r4, [a038]            ; puerta de la voz 0 (ARM)
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1         ; a la mezcla; tras las 4 voces: r7 x 0,25 -> st [a150]
```

### 5.2 C-S1JX: el JX-3P repartido por chip [C], [E]

Cada chip ejecuta una cadena de dos voces del JX-3P (las voces locales 0/1 del SYSTEM-8 en el maestro, 2/3 en el
esclavo); el reparto, el remapeo de memoria y las pruebas están en
[../docs/examples_system_1_jx_README.md](../docs/examples_system_1_jx_README.md) §3–§6. Como el maestro también ejecuta
voces, **las dos regiones escriben el banco 5**: cada una evalúa el modelo completo y escribe sólo las palabras de su
chip:

```c
/* examples/system_1_jx/jx.h: S8 DSP d par = maestro (JX_CHIP 0), impar = esclavo (JX_CHIP 1) */
static void dsp_bank(int dsp, int id, float v)
{
    if ((dsp & 1) != JX_CHIP || id < 0x20 || id >= 0x50)
        return;
    uint16_t w = b5_index[dsp][id - 0x20];          /* tabla generada por s1jx.py */
    if (w != 0xFFFF)
        b5_write(w, fbits(v));                      /* FD6C(w >> 8, {0, 0, w & 0xFF}, bits) */
}
```

---

## 6. Número de voces

| | SYSTEM-1 |
|---|---|
| el OS entrega a un asignador de PLUG-OUT más de 4 voces | **sí, 8**: C-S1V8 suena con 8 notas sostenidas y el esclavo recibe `0x114`–`0x117` / `0x124`–`0x127` [M] |
| más de 8 | el bloque de velocidades acaba en `0x127` (`0x128` empieza el de disparo); las notas tienen hueco para 16. Haría falta otra forma de mandar la puerta [A] |
| tiempo de DSP | esclavo A: entre 682 y 826 instrucciones por muestra [M] (C-S1SY, `docs/examples_system_1_synth_README.md` §4); C-S1V8 usa 269 en el esclavo A |
| voces del JX-3P con su microcódigo | 4 completas no caben (3748 de ≈ 3660 instrucciones); 4 con un DCO sí (3098) [E] ([DSP_FIT.md](DSP_FIT.md)) |
| voces de SH-101 / SH-2 / PROMARS con su microcódigo | 2 como mucho [E] ([DSP_FIT.md](DSP_FIT.md) §4) |

---

## 7. Abierto

| # | qué |
|---|---|
| 1 | qué es cada palabra del banco 1 por voz y cada una de las 20 de modulación `600E`–`6021` |
| 2 | en qué DSP del maestro corren la modulación y el mezclador de serie |
| 3 | más de 8 voces (otra puerta) |
| 4 | medida por voz (como `voicelog` del SYSTEM-8): la captura de parámetros (`examples/system_1/paramlog.py`) es la base |

## 8. Evidencia

| dato | fuente |
|---|---|
| ids de voz, asignador de serie (`0x604A26B0`), sitios de "4 voces" | [VOCES_6.md](VOCES_6.md) §3–§4; [log/VOICE_ALLOCATION_LOG.md](log/VOICE_ALLOCATION_LOG.md) §2 |
| C-S1V4 y C-S1V8 medidos | [../docs/examples_system_1_README.md](../docs/examples_system_1_README.md) §4, §7 |
| secciones, índices de `FC44` | [SH101_POLY.md](SH101_POLY.md) §2; [../docs/DSP_PROGRAM_MEMORY.md](../docs/DSP_PROGRAM_MEMORY.md) §3 |
| costes de los PLUG-OUTs de Roland | [DSP_FIT.md](DSP_FIT.md) §4 |
