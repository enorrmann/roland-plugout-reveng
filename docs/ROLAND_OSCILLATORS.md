# Osciladores de Roland en los PLUG-OUTs del SYSTEM-8 y del SYSTEM-1

Especificación de estudio (sólo lectura, nunca se copian bytes: [clean-room-boundary.md](clean-room-boundary.md)).
Cómo calculan las formas de onda los programas de oscilador de Roland y qué instrucciones usan para que quepan en el
presupuesto de cada muestra. Las reglas que salen de aquí para nuestro microcódigo están en
[MICROCODE_OPTIMIZATION.md](MICROCODE_OPTIMIZATION.md).

Estado: 2026-10-23. Fuentes:
* SYSTEM-8: el corpus local `work/corpus/*.bin`, que no está en git;
* SYSTEM-1: `work/unpack/<NOMBRE>_S1_Code1`, listado con `python3 system-1/s1_progs.py <dir> --listing`.

Todo se desensambló con `asm/disasm.py`. Las estadísticas de §4 salen de contar mnemónicos en esos listados.

Etiquetas: **[código]** = leído en los binarios; **[medido]** = probado en el equipo; **[inferido]**.

## 1. Esquema común (todos los modelos, los dos equipos) [código]

Todos los osciladores siguen los mismos pasos, con las mismas constantes en el SYSTEM-8 y en el SYSTEM-1:

| paso | qué hace | instrucciones |
|---|---|---|
| 1. tono → incremento | `x = p − 4,75` (p = tono en octavas, 4,75 = La 440), `x·ln 2`, separación en exponente y mantisa, polinomio de e^y | `addxy … #-4.75`, `mul … #0.69314718`, `expsplit`, `newton r2, #1.375001`, cadena `mac_p1p2c3` / `mac_p1p2c2` (Horner con dos coeficientes por instrucción) |
| 2. límite | incremento × constante de muestreo, tope (JX-3P: 0,249 por submuestra) | `mul … #0.0022916666`, `min … #0.248999998` |
| 3. 1 / incremento | ancho de las transiciones, en muestras: `256·inc`, semilla de recíproco + 3 Newton | `mul r6, r0, #256`, `recipseed r4, r6`, `newton r4, r6` ×3 |
| 4. fase | `fase += inc`, envuelta a [−1, 1) | `addxy rF, rI, rF` + `ph_wrap rF, r3, rF` |
| 5. forma | triángulo de la fase, escalado por el recíproco (pendiente = 1/inc), recortado a ±1: una rampa de una muestra de ancho en cada discontinuidad | `ph_tri r3, r3, -r3`, `mul r3, r3, #256`, `mul r3, r3, r4`, `ph_clamp r3, r3, -r3` |
| 6. suavizado | la rampa pasa por sen(π/2·x), un polinomio de grado 7: el escalón queda con banda limitada (tipo BLEP) | `mul … #1.570796013`, `mac_p1` / `ymacd` con 2,742e−6, −1,984e−4, 8,333e−3, −1/6 |
| 7. mezcla | sierra = fase − corrección; cuadrada / pulso = `sign` de la fase desplazada, con la misma corrección; los ondas se suman con los niveles (bank 6 `C0xx`) | `ph_add`, `sign r1, -r3`, `mac_p1 r2, r5, r6, r2` |

Diferencias entre equipos:
* **SYSTEM-8, JX-3P y Juno:** el paso 4–6 se repite **4 veces por muestra** (submuestras) y las 4 se diezman con un
  filtro (`mac_p1p2c3r`, `mac_p1p2`).
* **SYSTEM-1, SH-2:** cada voz calcula **una vez por muestra** las formas que el panel mezcla (sierra, cuadrada /
  pulso, sub) y aplica el suavizado a cada discontinuidad: 4–6 bloques recíproco + sen por voz.

## 2. SYSTEM-8: el DCO del JX-3P (`JX_6000_60a1b594`, 325 instrucciones) [código]

Ranura DCO (DCO1 / DCO2, programas de 325 y 357 instrucciones). Interfaz de la ranura: [CUSTOM_OSC.md](CUSTOM_OSC.md) §2.

1. Entrada `[2201]` (tono). Pasos 1–3: unas 20 instrucciones, una vez por muestra.
2. Por submuestra, unas 26 instrucciones, ×4:
   * `mac_p1 r3, r3, #0.5, #0.5 | mov r7, r3 | st … | ld … | ld …`: media fase y cinco transferencias en una sola
     instrucción;
   * `ph_tri`, luego `mul r5, r3, #256 | ph_add r1, r3, r7` y `mul r5, r5, r4 | ph_wrap r1, r3, r1`: cada
     instrucción lleva una op A y una op B;
   * `ph_clamp`, y el polinomio del seno en 4 instrucciones dobles: `mac_p1 … | mul …`, `ymacd … | mul …`;
   * sync y cruce: `cmp`, `setflags`, `cond` y `sum8`, todo en la misma instrucción que la aritmética.
3. Salidas `[221f]` / `[221b]`.

46 % de las instrucciones llevan dos o tres operaciones; sólo 23 de 325 son únicamente transferencias.

## 3. SYSTEM-1: la voz del SH-2 (sección `0x1900` del esclavo, 353 instrucciones) [código]

Un listado de una sección de voz. Las constantes son las del JX-3P:

```
ld r0, [0037] | ld r1, [0026]                         ; tono de la voz, nivel
addxy r0, r0, #-4.75 | mov r4, r1
mul r1, r0, #0.69314718
expsplit r0, -r1
mac_p1 r3, r1, #2.56533e-05, #0.0002821868 | newton r2, #1.375001
mac_p1p2c3 r3, r3, r2, #0.0007054673, r1, #0.0063492062   ; ... 4 más: e^y por Horner, 2 términos / instrucción
...
addxy r3, r4, r0 | st r4, [0106] | st r6, [0113] | ld r5, [0104]
ph_wrap r3, r3, r3 | st r1, [0110] | st r5, [0107]     ; fase envuelta: una op
mul r6, r0, #256 | ld r1, [0105]
recipseed r4, r6                                       ; 1 / (256 inc): semilla + 3 Newton
newton r4, r6
newton r4, r6
newton r4, r6
mac_p1 r3, r1, #0.5, #0.5 | st r4, [0112]
mul r1, r1, #-1 | ph_tri r3, r3, -r3                  ; op A + op B en una instrucción (clase 0x00b0)
mul r3, r3, #256 | ld r4, [0112]
mul r3, r3, r4
mul r3, r3, #0.25
ph_clamp r3, r3, -r3                                   ; rampa de una muestra en la discontinuidad
mul r3, r3, #1.570796013
mul r4, r3, r3
mac_p1 r2, r4, #0, #2.742e-06 | mul r5, r4, r3         ; sen(π/2 x): MAC + mul en una instrucción
mac_p1 r7, r4, #-0.0001984, #0.00833333 | mul r6, r4, r4
ymacd r2, r6, r7 | mul r7, r4, r5
```

(Texto del listado de `s1_progs.py`, sin las anotaciones `@` de los inmediatos; no es un bloque `dsp` porque usa
formas que nuestro ensamblador todavía no escribe.)

La sección hace ese bloque para cada discontinuidad de la mezcla: 12 `recipseed`, 37 `newton`, 14 `ph_tri` y 12
`ph_clamp`. El 38 % de las instrucciones llevan dos o más operaciones.

## 4. Todos los programas de oscilador [código]

`instr.` = instrucciones; `fusionadas` = instrucciones con dos o más operaciones (sin contar `ld`/`st`/`cond`);
`sólo transf.` = instrucciones sólo con `ld`/`st`. Columnas de mnemónicos: veces que aparecen.

SYSTEM-8 (corpus):

| programa | instr. | fusionadas | sólo transf. | `ph_wrap` | `ph_tri` | `ph_clamp` | `recipseed` | `newton` | `mac_p1*` | `ymacd` |
|---|---|---|---|---|---|---|---|---|---|---|
| JX-3P DCO `JX_6000_60a1b594` | 325 | 150 | 23 | 24 | 8 | 8 | 2 | 8 | 42 | 8 |
| JX-3P DCO `JX_6000_60a1d5f4` | 357 | 116 | 29 | 16 | 8 | 8 | 10 | 32 | 20 | 0 |
| Juno-106 `J106_6000_60a1d790` | 523 | 310 | 23 | 8 | 8 | 24 | 2 | 8 | 90 | 40 |
| Juno-60 `J60_6000_60a1c780` | 510 | 250 | 30 | 8 | 24 | 24 | 10 | 32 | 88 | 24 |
| Jupiter-4 `JP4_6000_60a1c450` | 616 | 338 | 40 | 8 | 24 | 24 | 10 | 32 | 112 | 48 |
| SH-1000 [inferido: prefijo `S100`] `S100_6000_60a16bc0` | 272 | 109 | 23 | 4 | 12 | 9 | 5 | 16 | 54 | 9 |
| SH-101 `SH_0100_60a12bc0` (en el principal, [CUSTOM_OSC.md](CUSTOM_OSC.md) §11) | 610 | 149 | 145 | 4 | 16 | 17 | 16 | 49 | 68 | 19 |

SYSTEM-1 (secciones de voz del esclavo):

| PLUG-OUT / programa | instr. | fusionadas | `ph_wrap` | `ph_tri` | `ph_clamp` | `recipseed` | `newton` | `mac_p1*` | `ymacd` |
|---|---|---|---|---|---|---|---|---|---|
| SH-2 sección `0x1900` | 353 | 135 | 4 | 14 | 12 | 12 | 37 | 59 | 13 |
| SH-2 sección `0x3100` | 245 | 71 | 4 | 10 | 8 | 8 | 25 | 30 | 9 |
| SH-101 principal `0x0100` | 575 | 137 | 4 | 16 | 17 | 16 | 49 | 68 | 18 |
| PROMARS sección `0x1900` | 344 | 161 | 4 | 12 | 12 | 5 | 16 | 57 | 28 |

Ningún oscilador de Roland envuelve la fase a mano: todos usan `ph_wrap` (o `ph_mod2` / `ph_abs` para fases
0..1) [código].

## 5. Instrucciones que usa Roland para ahorrar ciclos [código]

| técnica | forma | ejemplo |
|---|---|---|
| fase envuelta en una op | `ph_wrap rD, r3, rY` (X siempre r3) | `ph_wrap r3, r3, r3` |
| formas desde la fase | `ph_tri rD, r3, -rY` (triángulo), `ph_clamp rD, r3, -rY` (±1), `ph_abs`, `ph_add`, `ph_mod2` (fase 0..1), `sign rD, -rY` | `mul r1, r1, #-1 \| ph_tri r3, r3, -r3` |
| op A + op B | clase `0x00b0`: una op de unidad A con una de unidad B (`ph_*`, `min`/`max`, `sign`, `cmp`) | `mul r5, r5, r4 \| ph_wrap r1, r3, r1` |
| MAC + op A | `mac_p1 … \| mul …`, `ymacd … \| mul …` | polinomio del seno |
| Horner de 2 términos | `mac_p1p2c3`, `mac_p1p2c2`: dos coeficientes por instrucción | e^y en 5 instrucciones |
| hasta 5 transferencias por instrucción | `ld`/`st` en la misma instrucción que las ops (límites: [../examples/from_scratch/emu/chain.py](../examples/from_scratch/emu/chain.py) `access_violations`) | `mac_p1 … \| mov r7, r3 \| st … \| ld … \| ld …` |
| selección sin saltos | `cond c, …` / `setflags` / `cmp` en la instrucción de la op | sync del JX-3P |
| recíproco | `recipseed` + 3 `newton`, sin dividir | ancho de las rampas |

Coste: en el SYSTEM-8, una instrucción A/A/B (clase `0x160`) cuesta lo mismo que una de una sola op
[medido, [DSP_ISA_SPEC.md](DSP_ISA_SPEC.md) "time budget"]. En el SYSTEM-1, C-S1SY11 (576 instrucciones fusionadas en
el DSP A, 649 sin fusionar) funciona en el equipo [medido, 2026-10-23]: la fusión no lo pasa de presupuesto.
