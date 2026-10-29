# SYSTEM-1: comunicación entre DSP y ruta de audio — especificación técnica

| | |
|---|---|
| Versión | 2.0 (2026-10-22). Reescritura como especificación de la versión 1 (bitácora: [log/WAVE_BUS_LOG.md](log/WAVE_BUS_LOG.md)) |
| Alcance | Cómo intercambian datos los cuatro DSP del SYSTEM-1 (A y B del maestro, A y B del esclavo) y el ARM, y por dónde llega el audio de un PLUG-OUT a la salida. Qué DSP hace qué con cada voz: [VOICE_ALLOCATION.md](VOICE_ALLOCATION.md) |
| Equipo | SYSTEM-1, OS 1.30, PLUG-OUT en el slot 1. SYSTEM-8: [../docs/WAVE_BUS.md](../docs/WAVE_BUS.md) (misma ISA, otra topología) |
| Documentos base | [ARCHITECTURE.md](ARCHITECTURE.md), [../docs/DSP_PROGRAM_MEMORY.md](../docs/DSP_PROGRAM_MEMORY.md), [../docs/examples_system_1_README.md](../docs/examples_system_1_README.md) (interfaz PLUG-OUT) |
| Código de referencia | C-S1V4 (`examples/system_1/`, medido en el equipo); C-S1JX (`examples/system_1_jx/`, sin instalar) |

Evidencia: **[M]** medido en el equipo · **[L]** leído en el OS 1.30 o en los PLUG-OUTs de Roland del SYSTEM-1
(SH-2, SH-101, PROMARS) · **[C]** nuestro código · **[E]** emulador · **[I]** inferido · **[A]** abierto. Bloques
`dsp`: ensamblan (`python3 tools/doccheck.py system-1/WAVE_BUS.md`). Sintaxis y notación de direcciones:
[../docs/WAVE_BUS.md](../docs/WAVE_BUS.md) §1.

---

## 1. Diferencias con el SYSTEM-8 que cambian el diseño

| | SYSTEM-8 | SYSTEM-1 |
|---|---|---|
| BMC | maestro + 2 slaves | maestro + **1** esclavo |
| DSP por BMC | 6 en cadena (MXM) | **2 (A y B)**, sin MXM en ningún programa leído [L] |
| comunicación entre los DSP de un chip | MXM + buzón de pareja `A400`–`A7FF` | **banco 5 compartido** por A y B (`A000`–`A1FF`) [M] |
| entre chips | IOMD `6000`–`6003` (slave 1 → slave 0) | IOMD: esclavo → maestro `6006`–`600D` y `6026`–`602D`; maestro → esclavo `600E`–`6021` [L], [M] en parte |
| salida | `6020`/`6021` del slave 0 | maestro: `A118`/`A119` → programa del OS en el maestro A → `6000`/`6001` [M] |
| ARM | con FPU | **sin FPU** ([ARCHITECTURE.md](ARCHITECTURE.md) §1.1) |

---

## 2. Topología

```
 ESCLAVO (voces)                                          MAESTRO
 ┌─────────────────────────────┐                          ┌──────────────────────────────────────────────┐
 │ DSP A ◄──banco 5 común──► DSP B │──IOMD 6006–600D──────►  │ DSP B @0x1000 (PLUG-OUT) ──A118/A119──► DSP A │──► 6000/6001 ──► salida
 │   ▲                          │──IOMD 6026–602D──────►  │                     banco 5 común        @0x82 (OS) │
 │   └───────────IOMD 600E–6021 ◄──────────────────────────│ (modulación por voz, sinte de serie)          │
 └─────────────────────────────┘                          └──────────────────────────────────────────────┘
   ARM del esclavo: región 1 (motor)                        ARM del maestro: OS + región 0 (modelo, asignador)
```

---

## 3. Modelo de memoria

| banco | alcance | entre muestras | acceso del ARM | evidencia |
|---|---|---|---|---|
| 0 (`0nnn`) | por DSP o por chip [A]; los diseños propios usan rangos distintos en A y B | anillo (escribir R, leer R+1) | — | anillo [M] con C-S1V4 |
| 1 (`2nnn`) | compartido por A y B [I] (el principal A de serie escribe `2004`…, el B `2000`…, sin pisarse) | anillo | — | [L], [I] |
| 3 (`6nnn`) IOMD | por chip; palabras de enlace entre chips | persiste | — | [M] en parte (§4.4) |
| 5 (`A000`–`A1FF`) | uno por chip; `A1xx` compartido por A y B; **`A0xx` escrito por el ARM (`FD6C` sel 0) lo ve el DSP A pero no el B** | persiste | `FD6C(sel, obj, bits)` (§4.2) | [M] (`A150`, `A151+v`… de A a B; B leyendo `A038+v` del ARM: mudo, C-S1SY paso 3) |
| 6 (`Cnnn`) | por DSP | persiste | `FCEC(sel, id, bits)` inmediato, `FCE4` con rampa [A] | [M] (`C01C` del maestro B; `C040`–`C04B` del esclavo B y `C044`–`C055` del esclavo A, C-S1SY) |

Palabras del banco 5 reservadas por el sistema [L]: `A004`, `A005`, `A01D` (contador de trama de la cabecera del OS
y de los cierres, §5.3); `A006`–`A009` (el ARM del esclavo de serie); `A118`–`A11D` en el maestro (salida, §5.1).

---

## 4. Mecanismos

### 4.1 Banco 5 compartido: de un DSP al otro del mismo chip [M]

Lo que escribe el DSP A en el banco 5 lo lee el DSP B, y al revés. Es el sustituto de MXM y del buzón de pareja
del SYSTEM-8. La latencia (0 o 1 muestra, según el orden de A y B en la muestra) no está medida [A]; ningún diseño
probado depende de ella.

Ejemplo, C-S1V4: el DSP A suma las voces en `A150`; el DSP B (`examples/system_1/s1m1.s`) la lleva al maestro:

```dsp
; s1m1 (esclavo B, principal @0x0100): A150 x C000 -> 6006 / 6007; las otras salidas a 0
ld r7, [c000]      ; nivel de salida del DSP B (lo pone el OS)
ld r0, [a150]      ; mezcla que escribió el DSP A
mov r4, r4
mul r1, r0, r7
st r1, [6006]      ; al maestro, izquierda
st r1, [6007]      ; al maestro, derecha
mov r0, #0.0
st r0, [6008]
st r0, [6009]
st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d]
```

### 4.2 Banco 5 desde el ARM: `FD6C` [M], [L]

Mismo código en el maestro y en el esclavo (OS 1.30: maestro `0x010094B6`, esclavo `0x01009F4E`, desensamblados el
2026-10-22) [L]:

```
FD6C(sel, obj, bits):   idx = obj[2]
    sel 0: *(0x40101000 + 4·idx)         = bits     -> palabra A000 + idx
    sel 1: *(0x40101400 + 4·idx)         = bits     -> palabra A100 + idx
    otro sel: no hace nada
```

`bits` es el patrón IEEE-754 de un `float` en un registro entero (sin FPU). Uso en C-S1V4:

```c
/* examples/system_1/engine.c */
typedef void (*bank_fn)(int sel, const uint32_t *obj, uint32_t value);
static void b5(int idx, uint32_t bits)                        /* palabra A000 + idx */
{
    uint32_t obj[3] = { 0, 0, (uint32_t)idx };
    ((bank_fn)SVC(0x16C))(0, obj, bits);
}
```

Con dos `sel` el ARM alcanza `A000`–`A1FF`: es el banco 5 utilizable por un PLUG-OUT.

### 4.3 Banco 6 desde el ARM: `FCEC` [M]

`FCEC(sel, id, bits)`: parámetro `C0id` del DSP A (`sel 0`) o B (`sel 1`) del chip que lo llama, valor inmediato.
Ejemplos medidos: VOLUME → `C01C` del maestro B; en el esclavo, la ADSR y el filtro de C-S1SY → `C040`–`C04B` del
DSP B y los ranges/ondas → `C044`–`C055` del DSP A. Es el camino de Roland para los mandos (el SH-2 lee sus mandos en
`C0xx`) y el único fiable hacia el DSP B del esclavo: `A0xx` escrito con `FD6C sel 0` no le llega (§3).

```c
/* examples/system_1/model.c */
typedef void (*env_fn)(int sel, int id, uint32_t value);
static void set_volume(int v)
{
    volume = v < 0 ? 0 : v > 255 ? 255 : v;
    ((env_fn)SVC(0xEC))(1, M_VOLUME, volume_bits[volume]);     /* M_VOLUME = 0x1C */
}
```

`FCE4(sel, id, destino, y, z)` (con rampa): rampa lineal del hardware, un paso cada 1/3 ms, de 4 a 97 ms de 0 a 1
según `y`; `+8` del bloque = valor actual, `+C` = incremento [M] ([../docs/examples_system_1_ramp_README.md](../docs/examples_system_1_ramp_README.md)).

### 4.4 IOMD entre chips

| palabras | sentido | quién (Roland) | evidencia |
|---|---|---|---|
| `6006`–`600D` | esclavo → maestro | esclavo A o B escriben; maestro A o B leen (SH-101: esclavo A `600B`–`600D`, esclavo B `6006`–`600A`) | [L]; `6006`/`6007` [M] con C-S1V4 |
| `6026`–`602D` | esclavo → maestro | esclavo B escribe (SH-2: `6026`–`602D`; PROMARS: `6026`, `6027`); maestro A lee | [L] (`system-1/s1_progs.py`) |
| `600E`–`6021` | maestro → esclavo (20 palabras) | maestro A (modulación) y B (PROMARS: `6010`) escriben; esclavo A y B leen | [L] |
| `6000`/`6001` | maestro A → salida | programa de salida del OS | [M] |

Latencia entre chips: ≥ 1 muestra [I]. En el sinte de serie, el mezclador del maestro suma `6006`–`600D` por parejas
L/R con ganancias `C005`–`C00E` [L] (§5.2); en modo PLUG-OUT el reparto es libre.

### 4.5 Anillos (bancos 0 y 1)

La misma semántica que en el SYSTEM-8 ([../docs/WAVE_BUS.md](../docs/WAVE_BUS.md) §4.1, §4.2). Ejemplo medido, fase
de una sierra de C-S1V4 (`s1m0.s`, voz 0): `ld r1, [0301]` … `st r1, [0300]`.

---

## 5. Ruta de audio

### 5.1 Modo PLUG-OUT [M], [L]

```
esclavo A/B ──IOMD 6006…──► maestro B (programa del PLUG-OUT @0x1000) ──A118/A119──► maestro A @0x82 (OS):
    A118/A119 → filtro de continua (C002, C003) → × C004 → 6000 / 6001 → salida
```

* Al pasar a modo PLUG-OUT el OS encola en el maestro A el programa de salida en `0x82` y en el maestro B, en
  `0x1000`, un programa que pone `A118`–`A11D` a 0. Después llama a `load` del modelo de la región 0 [L].
* **Un PLUG-OUT que no carga su programa del maestro B queda mudo** [M] (C-S1TONE). El VOLUME del panel tampoco pasa
  por el programa del OS: lo aplica el PLUG-OUT (`C01C`) [M].

Programa mínimo del maestro B (C-S1V4, `examples/system_1/s1mm1.s`), medido:

```dsp
; s1mm1 (maestro B @0x1000): 6006 / 6007 del esclavo x VOLUME (C01C) -> A118 / A119
ld r0, [6006]
ld r1, [6007]
ld r7, [c01c]      ; VOLUME, escrito por el ARM del maestro con FCEC(1, 0x1C, ...)
mov r4, r4
mul r0, r0, r7
mul r1, r1, r7
st r0, [a118]      ; lo lee el programa de salida del OS en el maestro A
st r1, [a119]
mov r2, #0.0
st r2, [a11a]
st r2, [a11b]
st r2, [a11c]
st r2, [a11d]
```

### 5.2 Sinte de serie [L]

| DSP | programa | qué hace |
|---|---|---|
| maestro | `0x60097A7C` (696 instr.) | modulación por voz: lee del ARM `A038`–`A048`, escribe 20 palabras `600E`–`6021` al esclavo y otras 20 por el banco 5 (`A100`–`A113`) |
| maestro | `0x60095B0C` (58 instr.; variantes `0x600959DC`, `0x60095A24`) | mezclador: `6006`–`600D` × `C005`–`C00E`, filtro de continua, × `C00F` × `C010` → `6000`/`6001` |
| esclavo A, principal | `0x600185B0` (241 instr.) | lee `600E`–`6021` y `A006`–`A009`; alimenta las secciones de voz (banco 1); pasa datos y audio a B por `A100`–`A10D` |
| esclavo B, principal | `0x60018F44` (237 instr.) | lee `A100`–`A10D`; recoge sus secciones; escribe `6006`–`600D` × `C000` |

En qué DSP del maestro corre cada uno de los dos programas del maestro: [A].

### 5.3 Cabecera `0x0000` y cierre `0x7FF2`: qué se carga en `load` [M], [L]

Cada DSP empieza su muestra en una **cabecera** en `0x0000` y la termina con un **cierre** en `0x7FF2`; los dos
mantienen un contador de trama en el banco 5 (`A01D` maestro A, `A004` maestro B y esclavo A, `A005` esclavo B). La
cabecera la pone el OS.

**Regla: un PLUG-OUT nunca encola la entrada `0x0000` en `load`.** Hacerlo cuelga el equipo al seleccionar el
PLUG-OUT (error fatal del OS, todas las luces del panel encendidas, sólo sale apagando) [M]: la cabecera es código que
el DSP está ejecutando. Los PLUG-OUTs de Roland tienen la entrada `0x0000` en sus tablas de programas (por eso
aparece al leer las imágenes), pero sus motores pasan a `FC30`/`FC34` un puntero a la **entrada siguiente** [L].
Secuencia real de `load`, observada ejecutando el código de Roland en Unicorn (`examples/system_1_jx/test_arm.py`,
arnés `Region` con mapeo bajo demanda):

| PLUG-OUT | región | llamadas en `load` (direcciones de cada set encolado) |
|---|---|---|
| SH-101 | 1 (esclavo) | `FC44(0…7, 0)`; `FC30 {0x100, 0x7FF2}`; `FC34 {0x100, 0x7FF2}` |
| SH-2 | 1 | `FC44(0, 1)`, `FC44(4, 1)`, el resto 0; `FC30 {0x100, 0x7FF2, 0x3100}`, `FC30 {0x1900, 0x100, 0x7FF2, 0x3100}`, `FC30 {0x3100}`; `FC34 {0x100, 0x7FF2}` |
| PROMARS | 1 | `FC44(0, 1)`, `FC44(2, 1)`, el resto 0; `FC30 {0x100, 0x7FF2}`, `FC30 {0x1900, 0x100, 0x7FF2}`; `FC34 {0x100, 0x1900, 0x7FF2}`, `FC34 {0x1900, 0x7FF2}` |
| SH-101, SH-2, PROMARS | 0 (maestro) | `FC30 {0x100, 0x6DF2, 0x7FF2}`, `FC30 {0x6DF2, 0x7FF2}`; `FC34 {0x1000, 0x7FF2}`, `FC34 {0x5FF2, 0x6FF2, 0x1000, 0x7FF2}`, `FC34 {0x6FF2, 0x1000, 0x7FF2}`; dos `FC38` y tres `FC3C` (colas de datos: puntero a la región 0, `0x110001D` / `0x110001F`, un tercer valor) |

Ninguno encola `0x0000`. Los varios `FC30`/`FC34` de un mismo `load` son sets que se solapan (sufijos de una misma
tabla); qué hacen las colas de datos `FC38`/`FC3C` no está estudiado [A] (nuestros PLUG-OUTs no las usan y
funcionan).

Para referencia, la cabecera de Roland (la misma en los tres; nuestra versión ensamblada, `s1jx.frame_header`, es
idéntica byte a byte). **No hay que cargarla**:

```dsp
; cabecera @0x0000 del DSP cuyo contador es A004 (la pone el OS; NO encolarla)
ld r0, [a004]
cmp r0, r0, #0
ld r0, [e021]
cond !12, 1, 0 | st r0, [a004] | st r0, [a004]
```

El cierre sí se encola (C-S1V4, Roland):

```dsp
; cierre @0x7FF2 (precedido de una media palabra 0000): contador de trama E000 -> A004
ld r0, [e000]
st r0, [a004]
```

Cómo se encontró: bisección en el equipo con builds de diagnóstico de C-S1JX
([../docs/examples_system_1_jx_README.md](../docs/examples_system_1_jx_README.md) §10).

---

## 6. Ejemplo completo: una nota en C-S1V4 (voz 0)

| paso | dónde | qué | mecanismo |
|---|---|---|---|
| 1 | maestro, región 0 | el asignador elige la voz v y hace `set(2, 0x110 + v, nota)`, `set(2, 0x120 + v, vel)` | [VOICE_ALLOCATION.md](VOICE_ALLOCATION.md) §2 |
| 2 | esclavo, región 1 | `param(0x110 + v)` → `FD6C(0, {0, 0, 0x30 + v}, (nota − 12)/12)`; `param(0x120 + v)` → `0x38 + v` = 1.0 | §4.2 |
| 3 | esclavo A, `s1m0` | sierra de la voz v con tono `A030+v`, × puerta `A038+v`, suma × 0,25 → `A150` | §4.1, §4.5 |
| 4 | esclavo B, `s1m1` | `A150` × `C000` → `6006`/`6007` | §4.1, §4.4 |
| 5 | maestro B, `s1mm1` | × `C01C` → `A118`/`A119` | §5.1 |
| 6 | maestro A, OS `@0x82` | filtro de continua, × `C004` → `6000`/`6001` | §5.1 |

---

## 7. Reglas de diseño

1. Cargar siempre un programa propio en el maestro B (`0x1000`) que escriba `A118`/`A119`; si no, mudo.
2. Aplicar el VOLUME (`0x100`) en el PLUG-OUT (p. ej. `C01C` del maestro B).
3. Banco 5 utilizable: `A020`–`A1FF` (salvo `A118`–`A11F` en el maestro); dos programas del mismo chip no deben
   compartir palabras sin querer (es un banco común).
4. Escribir el banco 5 desde el ARM sólo con `sel` 0/1 (`A000`–`A1FF`).
5. Esclavo → maestro: `6006`–`600D` (8 palabras) y `6026`–`602D` (8 más); maestro → esclavo: `600E`–`6021` (20).
6. **Nunca encolar `0x0000`** (cuelga el equipo, §5.3). Encolar programas, finales de sección vacíos y el cierre
   `0x7FF2`, como los PLUG-OUTs de Roland en su `load`.
7. Presupuesto **medido en el esclavo A: entre 682 y 826** instrucciones por muestra (682 funciona; 826 cuelga el
   equipo al seleccionar el PLUG-OUT, también con 144 rellenos `mov r4, r4`) [M], menos que las ≈ 925 del SYSTEM-8.
   Empaquetar transferencias con las operaciones, como Roland, lo hace caber: una carga se usa en la instrucción
   siguiente [M] (`docs/examples_system_1_synth_README.md` §4–5).
8. **Límites de acceso por instrucción** (incumplirlos cuelga el equipo [M]): banco 5 ≤ 1 carga y ≤ 1 store (mejor:
   un solo acceso), banco 0 ≤ 2 cargas y ≤ 2 stores, banco 6 ≤ 2 cargas. Los DSP del esclavo guardan sólo en
   `A100`–`A1FF` [I]. El emulador rechaza los programas que los incumplen (`emu/chain.py access_violations`).
   Detalle: [../docs/examples_system_1_jx_README.md](../docs/examples_system_1_jx_README.md) §5.

---

## 8. Abierto

| # | qué |
|---|---|
| 1 | si el banco 0 es por DSP o por chip; si el banco 1 se comparte de verdad entre A y B |
| 2 | latencia del banco 5 entre A y B, y del IOMD entre chips |
| 3 | qué son `6026`–`602D` en el sinte de serie y si llegan al audio en modo PLUG-OUT |
| 4 | `FCE4`: medido (lineal, 3 kHz); falta la dependencia de la distancia y los modos `z` ≥ 2 |
| 5 | captura de audio por USB en el SYSTEM-1 (afinación exacta, estéreo); barrido de palabras IOMD como el del SYSTEM-8 |

---

## 9. Evidencia

| dato | fuente |
|---|---|
| cadena de audio de C-S1TONE/C-S1V4, VOLUME | [log/WAVE_BUS_LOG.md](log/WAVE_BUS_LOG.md) §4; [../docs/examples_system_1_README.md](../docs/examples_system_1_README.md) §2.2, §6 |
| `FD6C` idéntico en maestro y esclavo; `6026`–`602D` | esta revisión (2026-10-22), `system-1/s1_progs.py`, [DSP_FIT.md](DSP_FIT.md) §4 |
| `load` de Roland (no encola `0x0000`); el cuelgue al encolarla | §5.3; [../docs/examples_system_1_jx_README.md](../docs/examples_system_1_jx_README.md) §10 |
| programas de serie del maestro y del esclavo | [log/WAVE_BUS_LOG.md](log/WAVE_BUS_LOG.md) §2, §3; [VOCES_6.md](VOCES_6.md) |
