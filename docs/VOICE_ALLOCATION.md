# SYSTEM-8: asignación de voces a los DSP — especificación técnica

| | |
|---|---|
| Versión | 2.0 (2026-10-22). Reescritura como especificación de la versión 1 (bitácora: [log/VOICE_ALLOCATION_LOG.md](log/VOICE_ALLOCATION_LOG.md)) |
| Alcance | Cómo llega una nota del teclado a una voz, qué DSP calcula cada parte de cada voz, qué palabras escribe el ARM para cada voz, la interfaz de las ranuras de oscilador, las dos familias de PLUG-OUTs de Roland y el límite de voces. Los mecanismos de transporte (MXM, anillos, buzón, IOMD) están en [WAVE_BUS.md](WAVE_BUS.md) |
| Equipo | SYSTEM-8 (maestro + 2 slaves, 6 DSP por slave). SYSTEM-1: [../system-1/VOICE_ALLOCATION.md](../system-1/VOICE_ALLOCATION.md) |
| Código de referencia | asignador `examples/from_scratch/model/model.c`; motores `examples/from_scratch/engine/engine.c` (JX-3P con microcódigo de Roland) y `v3/engine_v3.c` (C-V4, microcódigo propio) |

Evidencia: **[M]** medido en el equipo · **[L]** leído en el microcódigo o el ARM de Roland · **[C]** nuestro código ·
**[E]** emulador · **[I]** inferido · **[A]** abierto. *(log §n)* = sección de la bitácora que lo sostiene. Bloques
`dsp`: ensamblan (`python3 tools/doccheck.py docs/VOICE_ALLOCATION.md`). Notación de direcciones y tiempos:
[WAVE_BUS.md](WAVE_BUS.md) §1.

---

## 1. Resumen

| | valor | evidencia |
|---|---|---|
| voces | 8: 4 por slave (slave 0: 0–3, slave 1: 4–7) | [M] |
| quién asigna | la región 0 del PLUG-OUT (en el maestro), a petición del OS | [C], [L] |
| cómo llega una nota a un slave | parámetros de voz del grupo 2: `0x429 + v` nota, `0x43A + v` velocidad; **los dos slaves reciben los ocho** | [M] |
| reparto dentro de un slave (familia A) | dos cadenas de tres DSP: voces locales 0/1 en DSP0 → DSP2 → DSP4, voces 2/3 en DSP1 → DSP3 → DSP5 | [L], [M] con C-V4 |
| voces por DSP de oscilador | 2 (voz A y voz B), en las ranuras DCO1 `0x6000` y DCO2 `0xB000` | [L] |
| límite real | el tiempo de DSP (≈ 925 instrucciones por muestra y DSP), no un número de voces | [M] (§8) |

---

## 2. De la tecla a la voz

### 2.1 Camino

```
tecla / MIDI ─► OS del maestro ─► región 0: asignador (vt[3] note_on, vt[2] note_off, …)
                                    └─ set(parte, 2, 0x429+v, nota); set(parte, 2, 0x43A+v, velocidad) ─► OS
OS ─► los dos slaves: motor de la región 1, vt+0x30 param(id, valor) por cada id de voz cambiado
      slave s: voz local lv = v − 4·s; si 0 ≤ lv < 4, escribe tono y puerta en sus DSP (§4)
```

### 2.2 Ids de voz (grupo 2)

| id | contenido | lo envía | evidencia |
|---|---|---|---|
| `0x429 + v` (v = 0–7) | nota MIDI de la voz v | asignador | [L], [M] |
| `0x43A + v` | velocidad 1–127; **0 = soltar** | asignador | [L], [M] |
| `0x44B + v` | indicador de legato de portamento | nuestro asignador | [C] |
| `0x45C` | voz con la nota más aguda | asignador | [L], [C] |
| `0x42D + v`, `0x43E + v` | los envía el asignador de Roland; significado desconocido | Roland | [L], [A] |

Los ids están en bloques de 17 (`0x429`, `0x43A`, `0x44B`, `0x45C`): hay hueco para más de 8 voces (§8) [I].

### 2.3 Asignador (región 0)

El asignador es un objeto del PLUG-OUT; el OS lo llama con cada tecla. Extracto de nuestra regla poly ("misma nota >
voz libre más antigua > voz soltada con hold > voz más antigua"), común a C-V4 y a los ports del SYSTEM-1:

```c
/* examples/system_1/model.c (la misma regla que examples/from_scratch/model/model.c) */
static void key(struct assign *a, int note, int vel)
{
    ...
    int same = -1, idle = -1, held = -1, v;
    for (int k = NV - 1; k >= 0; k--) {          /* order[]: la más recientemente usada primero */
        int c = a->order[k];
        if (a->v[c].note == note) same = c;
        if (idle < 0 && !a->v[c].vel) idle = c;
        if (held < 0 && a->v[c].held == 1) held = c;
    }
    if (same >= 0) v = same;
    else if (idle >= 0) v = idle;
    else if (held >= 0) v = held;
    else v = a->order[NV - 1];
    gate_off(a, v);                     /* una tecla nueva siempre empieza con la puerta cerrada */
    play(a, v, note, vel);              /* set(2, ID_NOTE + v, note); set(2, ID_VEL + v, vel) */
    use(a, v);
}
```

Consecuencia práctica: con todas las voces libres, la primera nota va a la voz **más antigua del orden**, no a la 0.
Para saber qué voz tocó qué nota en una medida se usa `examples/from_scratch/voicelog/voicelog.py` (registro de los
ids de voz).

### 2.4 Motor (región 1): filtro de voces por slave

```c
/* examples/from_scratch/engine/engine.c, e_param (vt+0x30) */
if (id >= 0x429 && id < 0x429 + 8) {
    v = id - 0x429 - e->first;                   /* first = 4 * slave */
    if (v >= 0 && v < 4) note_pitch(e, v, value);
    return;
}
if (id >= 0x43A && id < 0x43A + 8) {
    v = id - 0x43A - e->first;
    if (v >= 0 && v < 4) note_gate(e, v, value);
    else active_flags(e);                         /* una voz del otro slave */
    return;
}
```

Si un motor acepta las ocho voces en los dos slaves, cada nota suena dos veces (el slave 1 llega a la salida a
través del slave 0, [WAVE_BUS.md](WAVE_BUS.md) §5.2) [M] *(log WB §4e)*.

---

## 3. Reparto por DSP (familia A: JX-3P, JUNO-60, JUNO-106, JUPITER-4, JUPITER-8)

### 3.1 Funciones

| DSP | voces locales | función | programa principal (JX-3P) | ranuras |
|---|---|---|---|---|
| DSP0 | 0, 1 | LFO, envolventes ENV1 (filtro) y ENV2 (amplitud), portamento, tono | MAIN0, 5066 B | `0x8000`: vacía en nuestra reproducción (`engine.c`); en Roland [A] |
| DSP1 | 2, 3 | ídem | MAIN1, 5018 B | `0x8000`: ídem |
| DSP2 | 0, 1 | osciladores: prepara las entradas de las ranuras, mezcla sus salidas | MAIN2, 2068 B | DCO1 `0x6000` (tarea 2), DCO2 `0xB000` (tarea 3) |
| DSP3 | 2, 3 | ídem | MAIN3, 2018 B | DCO1, DCO2 |
| DSP4 | 0, 1 + mezcla | filtro (VCF 4 polos), VCA, **mezcla de salida** del slave | MAIN4, 5174 B | efecto `0xB000` (OD, DS, CH, FZ, CR, PH) |
| DSP5 | 2, 3 | filtro y VCA; entrega las voces a DSP4 | MAIN5, 5016 B | `0x6000`: vacía en nuestra reproducción; en Roland [A] |

[L] por tamaños y punteros de la tabla de descriptores (`re/alloc_scan.py`, *log §1, §2*); [M] con C-SAWPOLY (cadena
de Roland, voces audibles) y con C-V4 (microcódigo propio con la misma disposición, las 8 voces).

### 3.2 Flujo de datos de una cadena (voces locales 0/1)

| salto | palabras | qué viaja | evidencia |
|---|---|---|---|
| ARM → DSP0 | banco 5 `A021` (tono A), `A033` (tono B), `A022`/`A034` (puerta) | nota y puerta | [C], [L] |
| DSP0 → DSP2 | `Z 8404`–`8416` → `M 8004`–`8016` (19 palabras) | tonos con modulación, envolventes, LFO | [L] |
| DSP0 → DSP1 | buzón `A404`, `A409`–`A40E` | datos comunes (LFO) para la otra cadena | [L] |
| DSP2 → ranuras | banco 1 `[2201]`…`[2219]` | entradas de DCO1/DCO2 (§5) | [L] |
| ranuras → DSP2 | `[221B]`…`[2229]`, leídas a +1 | audio de los osciladores | [L], [M] |
| DSP2 → DSP4 | `Z 8404`–`841A` → `M 8004`–`801A` | audio de las dos voces (`8416`–`8419`), tonos y envolventes reenviados | [L] |
| DSP4 → DSP2 | `M 801B`–`801D` → `S 861B`–`861D` | retorno | [L] |
| DSP2 → DSP0 | `M 8017`–`8019` → `S 8617`–`8619` | retorno | [L] |
| DSP4 ↔ DSP5 | buzón `A404`–`A407` (→ DSP5), `A400`–`A403` (→ DSP4) | voces 2/3 ya filtradas hacia la mezcla | [L], [M] |
| DSP4 → salida | IOMD `6020`/`6021` | mezcla del slave | [M] |

La cadena de las voces 2/3 es la misma con DSP1 → DSP3 (`Z 8400`–`8410` → `O 8200`–`8210`) → DSP5 (`Z 8400`–`8414` →
`O 8200`–`8214`), sin retornos S [L].

### 3.3 Coste de tiempo (JX-3P, 4 voces por slave) [E]

| DSP | programas | instrucciones ejecutadas por muestra (máx.) |
|---|---|---|
| DSP0 / DSP1 | MAIN0 / MAIN1 | 516 / 514 |
| DSP2 / DSP3 | MAIN2 + DCO1 SAW + DCO2 SAW | 839 / 834 |
| DSP4 / DSP5 | MAIN4 / MAIN5 | 532 / 513 |

Medido con `system-1/dsp_fit.py` (emulador; ver [../system-1/DSP_FIT.md](../system-1/DSP_FIT.md)). DSP2/3 están al
90 % del presupuesto: por eso Roland pone **dos** voces por DSP de oscilador.

---

## 4. Palabras que escribe el ARM por voz

Fórmulas de `engine.c` (`note_pitch`, `note_gate`, `active_flags`), verificadas contra el motor del JX-3P
(`engine/test_emu.py`, sección de notas; resultado en `NOTES.md`) [C], [L]. `u = lv >> 1`, `k = lv & 1`.

| voz local | tono (octavas, `(nota − 12)/12`) | puerta de envolvente | VCA: puerta / curva de velocidad / velocidad |
|---|---|---|---|
| 0 | DSP0 `A021` | DSP0 `A022` | DSP4 `A02A` / `A029` / `A022` |
| 1 | DSP0 `A033` | DSP0 `A034` | DSP4 `A039` / `A038` / `A031` |
| 2 | DSP1 `A021` | DSP1 `A022` | DSP5 `A029` / `A028` / `A021` |
| 3 | DSP1 `A033` | DSP1 `A034` | DSP5 `A038` / `A037` / `A030` |
| todas | — | — | "alguna voz suena": DSP0 y DSP1 `A029`, `A03B` |

```c
/* examples/from_scratch/engine/engine.c */
static void note_pitch(engine_t *e, int lv, int note)
{
    int u = lv >> 1, k = lv & 1;
    e->note[lv] = note;
    p_bank(u, 0x21 + 0x12 * k, (float)(note - 12) / 12.0f);
}

static void note_gate(engine_t *e, int lv, int vel)
{
    int u = lv >> 1, k = lv & 1, d = 4 + u, s = 0xf * k - u;   /* los ids de DSP5 van uno más abajo */
    vel = clampi(vel, 0, 127);
    if (vel > 0) {
        p_bank(d, 0x2a + s, 1.0f);
        p_bank(d, 0x29 + s, vel_curve[vel]);
        p_bank(d, 0x22 + s, (float)vel / 127.0f);
        p_bank(u, 0x22 + 0x12 * k, 1.0f);
        e->on[lv] = 1;
    } else {
        p_bank(u, 0x22 + 0x12 * k, 0.0f);
        e->on[lv] = 0;
    }
    active_flags(e);
}
```

Un tono de `4.75` octavas es La 440 Hz (nota 69). Los parámetros del patch (filtro, envolventes, LFO, mezcla…) no
son por voz: los escribe el modelo de parámetros (`engine/model_spec.py`; la versión completa, con filtro y
envolventes, es `examples/system_1_jx/model_full.py`, ver [examples_system_1_jx_README.md](examples_system_1_jx_README.md) §8).

---

## 5. Interfaz de las ranuras de oscilador (DSP2/DSP3)

### 5.1 Palabras

Banco 1, propio de cada DSP, en anillo ([WAVE_BUS.md](WAVE_BUS.md) §4.2). El principal escribe las entradas antes de
que corran las ranuras; lee cada salida una muestra después en la palabra +1.

| | DCO1 voz A | DCO1 voz B | DCO2 voz A | DCO2 voz B |
|---|---|---|---|---|
| tono (octavas) | `[2201]` | `[220E]` | `[2207]` | `[2214]` |
| otras entradas | `[2203]`–`[2206]` | `[2210]`–`[2213]` | `[2209]`–`[220C]` | `[2216]`–`[2219]` |
| salidas (escritas) | `[221F]`, `[221B]` | `[2227]`, `[2223]` | `[2221]`, `[221D]` | `[2229]`, `[2225]` |
| salidas (leídas por el principal) | `[2220]`, `[221C]` | `[2228]`, `[2224]` | `[2222]`, `[221E]` | `[222A]`, `[2226]` |

Entradas de DCO2 para el **Cross Mod** del panel [M] (`CUSTOM_OSC.md` §2): `[2209]`/`[2216]` = sync activado,
`[220A]`/`[2217]` = METAL, `[220B]`/`[2218]` = un impulso por ciclo de DCO1 (pulsos de sync).

Reglas:
* Las dos salidas de cada voz llevan valores distintos en Roland (sin decodificar); escribir la misma señal en las dos
  funciona [M].
* Registros: las ranuras ven lo que dejó el programa anterior del mismo DSP. El DCO1 de Roland **lee `r2`/`r3`
  antes de escribirlos** (en el emulador depende de lo que deja MAIN2) [E]: no reordenar ni separar MAIN2 y DCO1 sin
  conservar esos registros ([examples_system_1_jx_README.md](examples_system_1_jx_README.md) §4.1).
* A una ranura registrada el motor le asigna siempre un programa (uno nulo propio, `gate/dco2_null.hex`, si no hay
  oscilador) en vez de dejarla sin programa [C].

### 5.2 Ejemplo: oscilador de sierra en la ranura DCO2 (C-V4, voz A)

Nuestro programa `examples/from_scratch/re/dspexp/own/saw_dco2.txt` (voz A; la voz B es igual con `[2214]`, fase
`0680` y salidas `[2229]`/`[2225]`). Tono `p` en octavas → incremento de fase `2^(p − 4,75) · 880/96000` (La 440 en
una fase de −1 a 1) con una serie de `e^x` elevada a la 16; fase en el anillo del banco 0.

```dsp
mov r5, #0.0
mov r6, #1.0
ld r1, [0671]              ; fase de la muestra anterior (anillo, WAVE_BUS §4.1)
ld r0, [2207]              ; tono que escribió el principal en esta muestra
add r0, #-4.75             ; octavas respecto de La 440
mul r2, r0, #0.04332169878499658     ; x = (p - 4,75) · ln 2 / 16
mov r3, #0.041666666666666664:32     ; e^x por Horner: 1/24, 1/6, 1/2, 1, 1
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3             ; (e^x)^16 = 2^(p - 4,75)
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667    ; · 880 / 96000
addxy r1, r3, r1           ; fase += incremento
addxy r2, r1, #-1.0        ; si fase >= 1: restar 2 (saturación en cuatro pasos)
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
st r1, [0670]              ; fase para la muestra siguiente
st r1, [2221]              ; salida de DCO2, voz A (el principal la lee en 2222)
st r1, [221d]
```

Afinado a ±1,2 cent de la nota 36 a la 96 [M] (`CUSTOM_OSC.md`).

---

## 6. Familia B (SH-101, SH-2, SYSTEM-100, PROMARS, base del SYSTEM-8)

| PLUG-OUT | main1 | main5 | qué hace |
|---|---|---|---|
| SH-101, SH-2, SYSTEM-100 | 200 B | 84 B | main0 lee `8613`–`8617`, `8627`–`862B`; main2 lee `8000`–`8026`; main4 escribe IOMD `6020`… |
| PROMARS | 200 B | 84 B | igual (main4 5640 B) |
| SYSTEM-8 base (`Code8_0`) | 5566 B | 4900 B | main0/main1 leen `8613`/`860F` y escriben `8400`…; main5 escribe `60B0`… |

[L] *(log §2.2, §6.2)*. main1 y main5 son plantillas: otra organización de voces (los tres primeros son monofónicos).
No usan el buzón de pareja para la voz. **No sirven de referencia para cuatro voces por slave.**

Tamaños de los principales de la familia A (bytes, main0–main5): JX-3P 5066/5018/2068/2018/5174/5016; JUNO-60
4610/4564/1548/1512/6122/7406; JUNO-106 4982/4934/3000/2950/5344/5236; JUPITER-4 4374/4328/1482/1446/6474/7464;
JUPITER-8 5838/5790/3040/2990/5038/4912 [L].

---

## 7. Implementación de referencia: C-V4 (microcódigo propio)

C-V4 reproduce la disposición de la familia A con programas mínimos propios (`examples/from_scratch/v4/make_v4.py`,
`build_v4.sh`), y suena con las 8 voces en los dos slaves, cada una con su nota (−3,2 a +2,6 cent) [M] *(log §6.7)*.

| DSP | programa | hace |
|---|---|---|
| DSP0 | `v4m0` | `A021`/`A033` → `Z 8404`/`8405` ([WAVE_BUS.md](WAVE_BUS.md) §4.3) |
| DSP1 | `v4m1` | `A021`/`A033` → `Z 8400`/`8401` |
| DSP2 | `v4m2` + sierra en DCO2 | `M 8004`/`8005` → `[2207]`/`[2214]`; `[2222]`/`[222A]` → `Z 8404`/`8405` (WAVE_BUS §4.2) |
| DSP3 | `v4m3` + sierra en DCO2 | `O 8200`/`8201` → ranura; salidas → `Z 8400`/`8401` |
| DSP4 | `v4m4` | voces 0/1 × puerta `A02A`/`A039`, + `A400`/`A401` + IOMD `6000`–`6003` → `6020`/`6021` (WAVE_BUS §5.1) |
| DSP5 | `v4m5` | voces 2/3 × puerta `A029`/`A038` → buzón `A400`/`A401` (WAVE_BUS §4.4) |

Ejemplo del principal de DSP1 (la cadena de las voces 2/3 usa la zona Z hacia el DSP impar):

```dsp
; v4m1 (DSP1): tono de las voces 2 y 3 -> DSP3 (zona O)
ld r1, [a021]
mov r4, r4
st r1, [8400]      ; -> DSP3 O 200
ld r1, [a033]
mov r4, r4
st r1, [8401]      ; -> DSP3 O 201
```

Errores que C-V4 corrigió respecto de C-V3 *(log §5, §6.3)*: desfase MXM supuesto (voz 1 muda); salidas de la ranura
leídas sin el +1 del anillo; voces de DSP5 enviadas a `60B0` (inaudible); el DSP4 del slave 0 sin sumar `6000`–`6003`
(slave 1 mudo).

---

## 8. Límite de voces

No hay ninguna medida de un límite de 8 en el hardware [M]. Cada capa que fija "8" es software:

| capa | qué fija hoy | ¿cambiable en un PLUG-OUT? | evidencia |
|---|---|---|---|
| región 0 (`model/model.c`) | `NV_MAX 8`, ids `0x429 + v`, `0x43A + v` con v = 0–7 | sí | [C] |
| OS del maestro | llama al asignador con un número de voces y reenvía los ids de voz | **desconocido** si reenvía ids por encima de `0x430` | [A] |
| motores | `lv = v − 4 · slave` | sí | [C] |
| microcódigo | 2 voces por DSP de oscilador | sí | [L], [C] |
| enlace entre slaves | 4 palabras IOMD (`6000`–`6003`), cada una puede llevar una mezcla | no limita | [L], [M] |
| **tiempo de DSP** | ≈ 925–945 instrucciones por muestra y DSP; pasarse cuelga el slave hasta apagarlo | **límite físico** | [M] (DSP_HARDWARE §1) |

Una sierra como la de §5.2 cuesta 33 instrucciones por voz [C]: un DSP de oscilador tendría tiempo para ≈ 25. Los
osciladores de Roland cuestan ≈ 160 por voz (DCO1 SAW: 325 para dos voces) [E].

Prueba que decidiría el punto abierto (no hecha): región 0 con `NV_MAX 12` y un motor que toque un tono fijo al
recibir `0x431` (voz 8). Si suena, el OS reenvía ids por encima de la voz 7. En el SYSTEM-1 sí se midió el
equivalente (8 voces con ids `0x114`–`0x117`, [../system-1/VOICE_ALLOCATION.md](../system-1/VOICE_ALLOCATION.md) §6).

---

## 9. Abierto

| # | qué | estado |
|---|---|---|
| 1 | significado de `0x42D + v` y `0x43E + v` (asignador de Roland) | [A] |
| 2 | el `e_load` de Roland (cómo indexa su tabla de programas `0x60A2581C`) | [A]; nuestro orden `main_prog[d]` → DSP d está [M] |
| 3 | semántica completa de MAIN0–MAIN5 (más allá de sus referencias de bus y de la envolvente de amplitud) | [A]; parte en `DSP_MAIN_PROGRAMS.md` |
| 4 | si el OS entrega más de 8 voces en el SYSTEM-8 | [A] (§8) |
| 5 | por qué las voces 3 y 4 no tienen mensaje de nota en el registro de C-V4 | [C]: nuestro asignador no reenvía una nota igual; sin efecto |

---

## 10. Evidencia y referencias antiguas

| dato | sección de la bitácora ([log/VOICE_ALLOCATION_LOG.md](log/VOICE_ALLOCATION_LOG.md)) |
|---|---|
| método (descriptores, tamaños, listados) | §0, §1 |
| familias A y B en todas las imágenes | §2.1, §2.2 |
| tabla de punteros del JX-3P | §2.4 |
| salidas DCO y palabras `2220`–`222A` | §2.5, §6.3 D |
| C-SAWPOLY (cadena de Roland en el equipo) | §4 |
| errores cometidos | §5 |
| modelo del emulador, preguntas A–H, C-V4 medido | §6.1–§6.7 |
| comprobación de firmas (`sigcheck.py`) | §7 |
| límite de voces | §8 |

Las referencias "`VOICE_ALLOCATION.md` §n" anteriores al 2026-10-22 apuntan a esas secciones de la bitácora.
