# SYSTEM-8: comunicación entre DSP y ruta de audio — especificación técnica

| | |
|---|---|
| Versión | 2.0 (2026-10-22). Reescritura como especificación de la versión 1 (bitácora: [log/WAVE_BUS_LOG.md](log/WAVE_BUS_LOG.md)) |
| Alcance | Cómo intercambian datos los seis DSP de un BMC slave, el ARM y los otros BMC, y por dónde llega el audio de una voz a la salida. Qué DSP hace qué con cada voz está en [VOICE_ALLOCATION.md](VOICE_ALLOCATION.md) |
| Equipo | SYSTEM-8, OS 1.x, PLUG-OUT en el slot 3 (las pruebas). El SYSTEM-1 tiene su propio documento: [../system-1/WAVE_BUS.md](../system-1/WAVE_BUS.md) |
| Documentos base | ISA y tiempo: [DSP_HARDWARE.md](DSP_HARDWARE.md), [DSP_ISA.md](DSP_ISA.md); ventanas del ARM: [DSP_ARM_SIDE.md](DSP_ARM_SIDE.md); memoria de programa: [DSP_PROGRAM_MEMORY.md](DSP_PROGRAM_MEMORY.md) |
| Código de referencia | C-V4 (`examples/from_scratch/v4/`, microcódigo propio, 8 voces medidas en el equipo); motor `examples/from_scratch/v3/engine_v3.c` |

Nivel de evidencia de cada dato: **[M]** medido en el equipo · **[L]** leído en el microcódigo o el código ARM de
Roland (estático) · **[C]** nuestro código · **[E]** predicción del emulador (`examples/from_scratch/emu`) ·
**[I]** inferido, sin prueba directa · **[A]** abierto. Entre paréntesis, la sección de la bitácora que lo sostiene
(p. ej. *(log §4b)*).

Los bloques `dsp` de este documento son microcódigo en la sintaxis de `asm/`; todos ensamblan
(`python3 tools/doccheck.py docs/WAVE_BUS.md`). Los bloques `c` son extractos de código del repositorio.

---

## 1. Convenciones

### 1.1 Direcciones de memoria del DSP

Una referencia a memoria es una palabra de 16 bits `Bnnn`. Los bits 15–13 eligen el banco; el resto, la palabra
dentro del banco. En el texto se escriben en hexadecimal sin prefijo y, en las tablas de programas, entre corchetes
(`[2207]` = "la palabra `2207`").

| prefijo | banco | nombre | ejemplo |
|---|---|---|---|
| `0nnn` | 0 | DT0, RAM de datos (anillo) | `[0300]` |
| `2nnn` | 1 | DT1, buzón principal ↔ ranura (anillo) | `[2207]` |
| `4nnn` | 2 | DT2, ventana de la línea de retardo | `[4300]` |
| `6nnn` | 3 | IOMD, entradas y salidas del BMC | `[6020]` |
| `8nnn` | 4 | MXM, bus entre DSP del mismo BMC | `[8404]` |
| `Annn` | 5 | DPC, memoria persistente; `A400`–`A7FF` = buzón de pareja | `[A021]` |
| `Cnnn` | 6 | parámetros escritos por el ARM (sólo lectura para el DSP) | `[C000]` |
| `Ennn` | 7 | desconocido (`E000` = contador de trama) | `[E000]` |

### 1.2 Unidades de tiempo y numeración

* **Muestra**: 1/96 000 s. Cada DSP ejecuta todos sus programas una vez por muestra (§6). "La muestra n+1" es la
  ejecución siguiente.
* **Slave**: cada SYSTEM-8 tiene un BMC maestro y dos slaves (slave 0: voces 0–3, slave 1: voces 4–7). Cada slave tiene
  seis DSP, `DSP0`–`DSP5`, agrupados en **parejas** (0,1), (2,3), (4,5).
* **Voz local** `lv` = voz global − 4 · slave (0–3). En la familia A de Roland, `u = lv >> 1` elige la cadena
  (0: voces 0/1, 1: voces 2/3) y `k = lv & 1` la voz dentro de la cadena (A o B).

### 1.3 Sintaxis del microcódigo de los ejemplos

```dsp
ld r1, [a021]      ; carga: r1 <- palabra A021 (banco 5)
mov r4, r4         ; instrucción de relleno: la carga se ve en la instrucción siguiente (latencia de 1)
st r1, [8404]      ; guarda r1 en la palabra 8404 (MXM, zona Z)
```

Reglas del hardware que afectan a todos los ejemplos [M] ([DSP_HARDWARE.md](DSP_HARDWARE.md) §7.3): una carga es
visible una instrucción después; como máximo 2 cargas del banco 0, 1 carga y 1 store del banco 5 por instrucción;
hasta 6 transferencias por instrucción.

---

## 2. Topología

```
                    ┌──────────── BMC slave 0 (voces 0–3) ─────────────┐
 ARM del slave ───► │ banco 5 / banco 6 de cada DSP                     │
 (región 1)         │                                                   │
                    │  DSP0 ──MXM Z→M──► DSP2 ──MXM Z→M──► DSP4 ──► IOMD 6020/6021 ──► salida
                    │   │ A404… (pareja)  │ banco 1             ▲ A400…       ▲
                    │   ▼                 ▼ ranuras DCO1/DCO2    │ (pareja)    │ IOMD 6000–6003
                    │  DSP1 ──MXM Z→O──► DSP3 ──MXM Z→O──► DSP5 ┘             │
                    └──────────────────────────────────────────────────────── │ ─┘
                    ┌──────────── BMC slave 1 (voces 4–7) ───────────┐         │
                    │  la misma cadena; su salida entra al slave 0 ──┼─────────┘  [M] existe, palabras [A]
                    └────────────────────────────────────────────────┘
                    BMC maestro: OS, panel, MIDI; DSP4 efectos (ranuras 0xEFDC, 0xF7DC); DSP3 ocioso
```

La flecha "salida" no está caracterizada a nivel de pines: se sabe que `6020`/`6021` del slave 0 se oyen y que nada
más del slave se oye sin pasar por esas dos palabras (§5) [M].

---

## 3. Modelo de memoria por banco

| banco | palabras | alcance | entre muestras | lo ven otros DSP | acceso del ARM | evidencia |
|---|---|---|---|---|---|---|
| 0 (`0nnn`) | 0x800 | propio del DSP | **anillo**: lo guardado en R se lee en R+1 en la muestra siguiente | no | lectura (vistas IEEE/SD31/RAW, `0x41400000 + n·0x100000`), relativa al anillo | [M] (log §4i; DSP_HARDWARE §7.2) |
| 1 (`2nnn`) | 0x800 | propio del DSP; DSP0 y DSP1 tienen la misma base en la tabla del ARM | **anillo**, como el banco 0 | sólo DSP0↔DSP1 [I] | no usado | anillo [M] con C-V4; bases [L] `s_boot 0x0111C580` (VA log §6.3 D/E) |
| 2 (`4nnn`) | ventana | línea de retardo (DMA) | dos páginas alternas | no | configuración | [M] en el maestro; fuera de alcance aquí ([DSP_HARDWARE.md](DSP_HARDWARE.md) §8.4) |
| 3 (`6nnn`) | 0x100 | **un bloque por BMC**; cada palabra tiene un DSP dueño | persiste | sí (§4.7) | lectura RAW `0x4113E000` | [M] (log §4b, §4i) |
| 4 (`8nnn`) | 4 × 0x200 (M, O, Z, S) | canales punto a punto entre DSP | **cerrojo de 1 muestra**, sin retención | sí, sólo el par cableado (§4.3) | lectura | [M] en el maestro, [M] en los slaves con C-V4 |
| 5 (`A000`–`A3FF`) | 0x400 | propio del DSP | persiste | no | **escritura** con el servicio `+0xB0` (§4.5); lectura RAW `0x41114000 + n·0x4000 + 0x2000` | [M] |
| 5 (`A400`–`A7FF`) | 0x400 | **compartido por la pareja** | lo escribe un DSP, lo lee el otro; cerrojo [I] | sí, la pareja (§4.4) | — | [L]; DSP5→DSP4 [M] con C-V4 |
| 6 (`Cnnn`) | por DSP | parámetros, sólo lectura para el DSP | persiste; rampas del hardware | no | escritura `+0xA4` inmediata / `+0xA0` con rampa (§4.6) | [M] |

Consecuencia de diseño: **no hay memoria común a los seis DSP**. Lo que un DSP calcula llega a otro sólo por MXM
(cadena fija), por el buzón de pareja o por IOMD. El banco 5 propio y el banco 6 son la vía del ARM hacia cada DSP.

---

## 4. Mecanismos de transporte

### 4.1 Anillo del banco 0: estado de un programa entre muestras [M]

**Semántica.** La referencia R direcciona la palabra física `(R + p) mod 0x800`, y `p` avanza 1 en cada muestra.
Dentro de una muestra, lo guardado en R se relee en R; en la muestra siguiente aparece en R+1; tras k muestras, en
R+k.

**Patrón.** Para conservar un valor de una muestra a la siguiente: cargar de `S+1`, guardar en `S`. Ejemplo: la fase
de la sierra de C-S1V4 (voz 0, `examples/system_1/s1m0.s`; la misma ISA):

```dsp
ld r1, [0301]      ; fase de la muestra anterior (se guardó en 0300)
mov r4, r4
addxy r1, r1, r3   ; avanzar la fase (r3 = incremento)
st r1, [0300]      ; guardar: en la muestra siguiente estará en 0301
```

**Reglas.** Dos programas del mismo DSP comparten el anillo: sus rangos no deben solaparse, contando que un dato se
desplaza una palabra por muestra (si un programa lee `R+k`, reserve k palabras de margen). Un retardo de k muestras
es `st [R]` + `ld [R+k]`. El ARM no puede dejar una tabla fija en el banco 0 (también se desplaza).

### 4.2 Banco 1: buzón principal ↔ ranura (anillo) [M]

Las ranuras (programas DCO en `0x6000`/`0xB000` de DSP2/3, efectos en `0xB000` de DSP4) se comunican con el programa
principal del mismo DSP por el banco 1. El banco 1 también es un anillo, y **el principal corre antes que las
ranuras** dentro de la muestra (VA log §6.3 D, F). De ahí la temporización:

| paso | quién | palabra | cuándo |
|---|---|---|---|
| 1 | principal escribe la entrada de la ranura | `[2207]` | muestra n |
| 2 | la ranura lee la entrada | `[2207]` (misma palabra) | muestra n |
| 3 | la ranura escribe su salida | `[2221]` | muestra n |
| 4 | el principal lee la salida | **`[2222]`** (= `2221` + 1) | muestra n+1 |

Ejemplo: principal de DSP2 de C-V4 (`examples/from_scratch/v4/v4m2.s`), medido en el equipo:

```dsp
; v4m2 (DSP2, principal): tono de DCO2 desde MXM; salidas de la sierra -> MXM
ld r1, [8004]      ; tono de la voz A, de DSP0 (MXM, §4.3)
mov r4, r4
st r1, [2207]      ; entrada de tono de la ranura DCO2, voz A (paso 1)
ld r1, [8005]      ; tono de la voz B
mov r4, r4
st r1, [2214]      ; entrada de tono de DCO2, voz B
ld r1, [2222]      ; salida de la voz A que la ranura escribió en 2221 la muestra anterior (paso 4)
mov r4, r4
st r1, [8404]      ; hacia DSP4 (MXM)
ld r1, [222a]      ; salida de la voz B (escrita en 2229)
mov r4, r4
st r1, [8405]
```

Interfaz completa de las ranuras DCO de Roland: [VOICE_ALLOCATION.md](VOICE_ALLOCATION.md) §5.

Error típico (cometido en C-V3, VA log §6.3 A/D): leer la salida en `[2221]`. Con el anillo devuelve otra palabra
de hace varias muestras; en C-V3 coincidió con otra sierra y lo ocultó.

### 4.3 MXM (banco 4): canales entre DSP del mismo BMC [M]

**Zonas** (direcciones locales de cada DSP; `8405` en DSP0 y `8405` en DSP2 son palabras distintas):

| zona | direcciones | uso |
|---|---|---|
| M | `8000`–`81FF` | entrada desde el DSP **par** de la pareja anterior |
| O | `8200`–`83FF` | entrada desde el DSP **impar** de la pareja anterior |
| Z | `8400`–`85FF` | salida hacia la pareja siguiente |
| S | `8600`–`87FF` | entrada de retorno: lo que el DSP de dos posiciones más adelante escribe en sus M/O |

**Cableado de los slaves** (el número de palabra `nn` se conserva):

| escribe | lee | evidencia |
|---|---|---|
| DSP0 `Z 84nn` | DSP2 `M 80nn` | [M] C-V4 |
| DSP1 `Z 84nn` | DSP3 `O 82nn` | [M] C-V4 |
| DSP2 `Z 84nn` | DSP4 `M 80nn` | [M] C-V4 |
| DSP3 `Z 84nn` | DSP5 `O 82nn` | [M] C-V4 |
| DSP2 `M 80nn` (store) | DSP0 `S 86nn` | [L] |
| DSP4 `M 80nn` (store) | DSP2 `S 86nn` | [L] |

**Temporización y reglas** [M en el maestro, DSP3↔DSP4]:
* **Cerrojo de 1 muestra**: el lector ve, durante todo su programa de la muestra n, lo escrito en la muestra n−1, sea
  cual sea el orden de los DSP.
* **Sin suma**: dos escrituras en la misma muestra → queda la última.
* **Sin retención**: si en una muestra nadie escribe la palabra, el lector recibe 0. Hay que escribirla en cada
  muestra.

Ejemplo: tono de las voces 0/1 de DSP0 a DSP2 (C-V4, `v4m0.s`; lo recibe el `v4m2.s` de §4.2 con `ld [8004]`):

```dsp
; v4m0 (DSP0, principal): tono de las voces 0 y 1 -> DSP2
ld r1, [a021]      ; tono de la voz A, escrito por el ARM (§4.5)
mov r4, r4
st r1, [8404]      ; Z 404 -> DSP2 M 004, una muestra después
ld r1, [a033]      ; tono de la voz B
mov r4, r4
st r1, [8405]      ; Z 405 -> DSP2 M 005
```

Error típico (C-V3, VA log §6.3 A/G): suponer un desfase (`Z 404` → `M 005`). DSP4 leía `M 006`, que nadie
escribía (0), y la voz 1 quedaba muda.

### 4.4 Buzón de pareja (banco 5, `A400`–`A7FF`) [L, M]

**Semántica.** Las palabras `A400`–`A7FF` las comparten los dos DSP de una pareja: lo que escribe uno lo lee el otro.
En ningún programa de Roland un DSP lee lo que escribe él mismo en esa zona [L]. El emulador modela un cerrojo de 1
muestra, como MXM [I]; la latencia real no está medida [A].

**Uso en la familia A de Roland** [L]:

| escribe | palabras | lee | qué |
|---|---|---|---|
| DSP0 (MAIN0) | `A404`–`A406`, `A409`–`A40E` | DSP1 (MAIN1) lee `A404`, `A409`–`A40E` | datos comunes de modulación de las dos cadenas |
| DSP4 (MAIN4) | `A404`–`A407` | DSP5 (MAIN5) | datos de DSP4 a DSP5 |
| DSP5 (MAIN5) | `A400`–`A403` | DSP4 (MAIN4) | **voces 2 y 3 procesadas** → mezcla de salida |

Ejemplo: DSP5 entrega sus dos voces a DSP4 (C-V4, `v4m5.s`), [M] con C-V4 (voces 2, 3, 6 y 7 audibles):

```dsp
; v4m5 (DSP5, principal): voces 2 y 3 con puerta -> buzón de pareja
ld r4, [a029]      ; puerta de la voz 2 (banco 5 propio de DSP5, la escribe el ARM)
ld r1, [8200]      ; sierra de la voz 2 (de DSP3 por MXM, zona O)
mov r4, r4
mul r1, r1, r4
st r1, [a400]      ; a DSP4
ld r4, [a038]      ; puerta de la voz 3
ld r2, [8201]
mov r4, r4
mul r2, r2, r4
st r2, [a401]      ; a DSP4
```

Error típico (C-V3, VA log §6.3 B): mandar las voces de DSP5 a IOMD `60B0`, que no llega a la salida (§5).

### 4.5 Banco 5 propio: parámetros por voz del ARM (`+0xB0`) [M]

El ARM del slave escribe una palabra del banco 5 de un DSP con el servicio del OS en `0x0113FC00 + 0xB0`:

```c
/* examples/from_scratch/v3/engine_v3.c */
#define OS(off) (*(void *volatile *)(0x0113FC00u + (off)))
typedef void (*bank_fn)(int dsp, const uint32_t *id, float v);          /* +0xB0 */
static void p_bank(int dsp, uint32_t id, float v) { ((bank_fn)OS(0xB0))(dsp, &id, v); }

/* voz local lv (0..3): banco 5 del DSP lv>>1, id 0x21 + 0x12*(lv&1) */
static void voice_pitch(int lv, int note)
{
    p_bank(lv >> 1, 0x21 + 0x12 * (lv & 1), (float)(note - 12) / 12.0f);
}
```

* Argumentos: `dsp` 0–5 del slave que ejecuta el código; `id` por puntero (índice de palabra, `A000 + id`); valor
  `float` (el OS lo convierte al formato RAW del DSP).
* El DSP lee la palabra con `ld rX, [A0id]`; persiste hasta la siguiente escritura. No hay cerrojo de muestra: el DSP
  ve el valor nuevo en la muestra en que el ARM lo escribió o en la siguiente [I].
* Lista completa de ids por voz: [VOICE_ALLOCATION.md](VOICE_ALLOCATION.md) §4.

### 4.6 Banco 6: parámetros del DSP (`+0xA4`, `+0xA0`) [M]

```c
/* examples/from_scratch/engine/engine.c */
typedef void (*set_fn)(int dsp, int id, float v);                                /* +0xA4: inmediato */
typedef void (*ramp_fn)(int dsp, int id, int ramp, int flag, int sp, float v);   /* +0xA0: con rampa */
static void p_set(int dsp, int id, float v) { ((set_fn)OS(0xA4))(dsp, id, v); }
static void p_ramp(int dsp, int id, int ramp, int flag, int sp, float v)
{
    ((ramp_fn)OS(0xA0))(dsp, id, ramp, flag, sp, v);
}
```

El DSP lee `C0id` (sólo lectura). Con `+0xA0` el hardware lleva el valor hasta el destino en una rampa (`ramp` =
duración codificada; `flag` = esperar el fin); se usa para niveles y tiempos del panel. Ejemplo de uso: nivel de
salida de MAIN4, `C000`, que multiplica la mezcla antes de `6020` [L].

### 4.7 IOMD (banco 3): entradas y salidas del BMC [M]

**Semántica.** 256 palabras `6000`–`60FF` por BMC. No es memoria compartida: cada palabra tiene **un DSP dueño**
(lo configura el ARM); las escrituras de otro DSP no aparecen; lo que escribe el dueño persiste y el ARM lo ve [M]
(log §4i). Bloques de propiedad: `00`–`BF` DSP4/5, `C0`–`DF` DSP2/3, `E0`–`FF` DSP0/1 [M] (direcciones).

**Palabras que usan los programas de voz de Roland** y su efecto medido (barrido de log §4b: un SAW de frecuencia
distinta en cada palabra, captura USB):

| palabras | escribe / lee (Roland) | llega a la salida | evidencia |
|---|---|---|---|
| `6020` | escribe DSP4 | **sí**, canal 1 de la captura | [M] |
| `6021` | escribe DSP4 | **sí**, canal 0 | [M] |
| `6063` | escribe DSP4 | **sí**, los dos canales (−23,9 dB); por qué, [A] | [M] |
| `6024` | escribe DSP4 | dudoso: +1,4 dB sobre el armónico previsto | [M], [A] |
| `6022`, `6023`, `6060`–`6062`, `6064`–`6066` | escribe DSP4 | no | [M] |
| `60B0`–`60B7` | escribe DSP5 (copias de `[2204]`–`[221D]`) | no | [M] |
| `6000`–`6006` | lee DSP4 | — (entradas: llega aquí el slave 1, §5.2) | [M] |
| `6010`–`6017` | lee DSP5 | — (entradas; origen [A]) | [M] |
| `60E0`–`60E2` | lee DSP0 | — (entradas; origen [A]) | [M] |

En la captura USB (`arecord -D hw:SYSTEM8,0 -f FLOAT_LE -r 96000 -c 2`), el canal 1 es lo que se oye por la salida
izquierda en estas pruebas [M] (confirmar en cada sesión).

### 4.8 Resumen de latencias

| tramo | latencia | evidencia |
|---|---|---|
| ARM → banco 5 / banco 6 → DSP | 0–1 muestras (según cuándo escribe el ARM) | [I] |
| principal → ranura (banco 1, misma palabra) | 0 | [M] (C-V4, C-SAW) |
| ranura → principal (banco 1, palabra + 1) | 1 | [M] |
| DSP → DSP (MXM) | 1 exacta | [M] |
| DSP → DSP de la pareja (`A4xx`) | 1 en el emulador | [I] |
| DSP → IOMD → DSP del mismo BMC | 0 o 1 según el orden de los DSP | [I] |
| slave 1 → slave 0 (IOMD) | ≥ 1 | [I] |

---

## 5. Ruta de audio

### 5.1 Slave 0

Lo único que se oye de un slave es lo que el **DSP4 del slave 0** escribe en `6020`/`6021` [M]. El DSP4 de Roland
mezcla ahí ocho fuentes [L] (VA log §6.2):

| fuente | llega a DSP4 por | guardada en | leída en la mezcla |
|---|---|---|---|
| voces 0 y 1 (su propio filtro/VCA) | — | `[221E]`, `[2229]` | `[2228]` … (anillo) |
| voces 2 y 3 (de DSP5) | buzón `A400`, `A401` | `[2234]`, `[223D]` | |
| slave 1 | IOMD `6000`–`6003` | `[2246]`, `[2249]`, `[224C]`, `[224D]` | |

Las ocho cargas se suman con coeficientes `C044`–`C048` y la mezcla se multiplica por `C04B` (nivel) y `C000`
antes de `6020`/`6021`. La versión mínima de esta mezcla es la de C-V4 (`v4m4.s`), medida con las ocho voces:

```dsp
; v4m4 (DSP4, principal): voces 0, 1 con puerta + voces 2, 3 (A400/A401) + slave 1 (6000-6003) -> 6020/6021
ld r4, [a02a]      ; puerta de la voz 0
ld r1, [8004]      ; sierra de la voz 0 (DSP2 Z 404)
mov r4, r4
mul r1, r1, r4
st r1, [6060]      ; voz suelta (Roland también las escribe en 6060-6063)
ld r4, [a039]      ; puerta de la voz 1
ld r2, [8005]
mov r4, r4
mul r2, r2, r4
st r2, [6061]
addxy r1, r1, r2
ld r2, [a400]      ; voz 2, ya con su puerta (DSP5, §4.4)
mov r4, r4
st r2, [6062]
addxy r1, r1, r2
ld r2, [a401]      ; voz 3
mov r4, r4
st r2, [6063]
addxy r1, r1, r2
ld r2, [6000]      ; slave 1
mov r4, r4
addxy r1, r1, r2
ld r2, [6001]
mov r4, r4
addxy r1, r1, r2
ld r2, [6002]
mov r4, r4
addxy r1, r1, r2
ld r2, [6003]
mov r4, r4
addxy r1, r1, r2
st r1, [6020]      ; salida (canal 1 de la captura)
st r1, [6021]      ; salida (canal 0)
```

### 5.2 Slave 1

El slave 1 **no** tiene salida propia audible: ninguna palabra `6000`–`60FF` que escriba se oye sola [M]
(log §4e, §4q: un drone en todas las palabras del slave 1 no se oyó). Se oye cuando el DSP4 del slave 0 suma sus
entradas `6000`–`6003` [M] (C-V4: voces 4–7 audibles). **Qué palabra del slave 1 llega a cada entrada no se sabe**
[A]; C-V4 escribe sus voces en `6060`–`6063` y su mezcla en `6020`/`6021`, y con eso funciona. Para fijarlo hace falta
una build que sólo escriba `6060`–`6063` y otra que sólo escriba `6020`/`6021`.

### 5.3 Maestro

El maestro recibe la mezcla del slave 0 y aplica sus efectos (DSP4, ranuras `0xEFDC`, `0xF7DC`) [L]; su DSP3 está
ocioso [M]. Cómo llega la palabra al maestro (bus de onda, pines) no está caracterizado [A] (log §1, DSP_HARDWARE §11).

---

## 6. Orden de ejecución dentro de la muestra

| relación | orden | evidencia |
|---|---|---|
| principal y ranuras de un DSP | principal primero | [M] (C-SAW afinado, C-V4; VA log §6.3 F) |
| DCO1 y DCO2 de un DSP | DCO1 antes que DCO2 | [I] (VA log §6.3 F) |
| ranura de DSP5 y principal de DSP4 | la ranura de DSP5 escribe después | [M] por supervivencia en `6021`/`6063` (log §4c) |
| principal de DSP5 y su ranura | desconocido | [A] |
| entre DSP en general | lo fija el planificador del BMC; MXM y el buzón no dependen de él (cerrojo) | [I] |

Todos los programas de un DSP comparten `r0`–`r7`, las banderas y los registros ocultos: lo que deja uno lo ve el
siguiente, también entre muestras [M] ([DSP_PROGRAM_MEMORY.md](DSP_PROGRAM_MEMORY.md) §1).

---

## 7. Ejemplo completo: una nota de la voz 0 en C-V4

Recorrido de una nota desde el ARM del slave 0 hasta `6020`, con la muestra en que ocurre cada paso (t = muestra en
que el ARM escribe el tono):

| t | dónde | qué | mecanismo |
|---|---|---|---|
| t | ARM, `voice_pitch(0, nota)` | `p_bank(0, 0x21, (nota−12)/12)` | §4.5 |
| t | DSP0, `v4m0` | `ld [a021]` → `st [8404]` | §4.3 |
| t+1 | DSP2, `v4m2` | `ld [8004]` → `st [2207]` | cerrojo MXM |
| t+1 | DSP2, ranura DCO2 (sierra) | lee `[2207]`, escribe `[2221]` | §4.2 |
| t+2 | DSP2, `v4m2` | `ld [2222]` → `st [8404]` | anillo del banco 1 |
| t+3 | DSP4, `v4m4` | `ld [8004]` × puerta `[a02a]` → `st [6020]` | cerrojo MXM |

Retardo total: 3 muestras (31 µs). La puerta (`p_bank(4, 0x2a, 1.0)`) llega directamente al DSP4: el tono y la
puerta no viajan juntos, así que un tono escrito tras la puerta se oye con hasta 3 muestras de la nota anterior.
`engine_v3.c` reescribe el tono al abrir la puerta por eso.

---

## 8. Reglas de diseño

1. Respetar la cadena fija: DSP0/1 → DSP2/3 → DSP4/5 → `6020`/`6021` del slave 0. MXM sólo va hacia delante (Z) y
   dos posiciones atrás (S).
2. Escribir cada palabra MXM en **cada** muestra (sin retención) y leer el mismo número de palabra que se escribió.
3. Leer las salidas de una ranura en la palabra **+1**, una muestra después (anillo del banco 1).
4. Pasar las voces de DSP5 a DSP4 por el buzón `A400`–`A403`; no por IOMD `60B0`.
5. El DSP4 del slave 0 debe sumar `6000`–`6003` si el slave 1 tiene que oírse.
6. **El principal de DSP4 no puede estar vacío**: con él vacío el equipo queda mudo entero [M] (log §4l). Los otros
   cinco principales pueden estar vacíos sin romper nada [M].
7. Reservar márgenes en los anillos si dos programas comparten DSP (§4.1).
8. Comprobar cada cambio en el emulador (`examples/from_scratch/emu/test_chain.py`), con pruebas que puedan fallar.

---

## 9. Abierto

| # | qué | cómo decidirlo |
|---|---|---|
| 1 | qué palabras del slave 1 entran por `6000`–`6003` del slave 0 | dos builds: una sólo con `6060`–`6063`, otra sólo con `6020`/`6021` |
| 2 | por qué `6063` se oye en los dos canales y `6024` dudoso | lote con `6024` a otra frecuencia; mapa con notas |
| 3 | latencia real del buzón de pareja | dsppeek en el maestro con dos DSP de una pareja |
| 4 | origen de las entradas `6010`–`6017` (DSP5) y `60E0`–`60E2` (DSP0) | leer IOMD con el ARM durante una nota |
| 5 | si DSP0 y DSP1 comparten de verdad el banco 1 | valores distintos en `[2210]` de cada uno, leídos por el ARM |
| 6 | orden de MAIN5 y su ranura | ruta de datos de una entrada de MAIN5 a una palabra audible |

---

## 10. Evidencia y referencias antiguas

| dato | sección de la bitácora |
|---|---|
| barrido de las 38 palabras IOMD | [log/WAVE_BUS_LOG.md](log/WAVE_BUS_LOG.md) §4b |
| orden ranura DSP5 / MAIN4 | log §4c |
| slave 1 sin salida propia; prueba C-S1W | log §4e, §4q |
| mecanismos (versión 2026-10-21, corregida) | log §4i |
| principal de DSP4 vacío = mudo; variantes | log §4l |
| envolvente de amplitud del JX-3P (ENV2, `[029D]`, factores del VCA) | log §4j–§4p |
| afinación (fine tune de OSC2) | log §4h |
| anillo del banco 1, buzón, MXM sin desfase, slave 1 por `6000`–`6003` | [log/VOICE_ALLOCATION_LOG.md](log/VOICE_ALLOCATION_LOG.md) §6.3, §6.7 |

Las referencias "`WAVE_BUS.md` §n" anteriores al 2026-10-22 apuntan a esas secciones de la bitácora.
