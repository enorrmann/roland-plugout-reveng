> [!NOTE]
> **TRM Technical Reference Manual**: This implementation example is documented normatively in [Volume VI: Reference Designs & Applications](../../../../docs/vol6_reference_designs_applications/casio_cz_phase_distortion.md).

# `sdk/s1cc/examples/casio_cz`: CZ-SYSTEM, una voz de Casio CZ (distorsión de fase) para SYSTEM-S1CC

Kernel de s1cc ([Vol. V ch. 1](../../../../docs/vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md)) para el PLUG-OUT SYSTEM-S1CC
([examples/system_s1cc/README.md](../../../../examples/system_s1cc/README.md)). Reproduce la síntesis por distorsión de fase de
los Casio CZ: las formas de onda del DCO, el DCW como cantidad de distorsión con su envolvente, las dos líneas con
desafinación y las modulaciones ring y noise. Lo que el CZ tiene y el anfitrión de SYSTEM-S1CC ya da (envolventes,
LFO, filtro) lo pone el anfitrión.

Fuente de la descripción del CZ: Michael Rickard, *Casio CZ-1 Spelunking* (https://www.kasploosh.com/cz/:
«Your Basic Waveforms», «The Truth About Combination Waves», «Window Function Explained», «Types Of Modulation»).

Estado (2026-11-03): verificado contra su referencia nativa (`s1cc check`) y en la cadena completa del emulador
(`make_s1cc.py --sim`). 8 voces. **Sin probar en el equipo.**

## 1. Uso

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/casio_cz/cz.c --check          # S1_COMPILE ya pone --regs 0,1,2,3
KERNEL=sdk/s1cc/build/cz sh examples/system_s1cc/build.sh            # -> plugins/CZ-SYSTEM (S1_BUILD: NAME, MIX=0)
python3 tools/plugout_tool.py install plugins/CZ-SYSTEM --slot 1 --force --no-patches -y
```

Mismo slot e id de modelo que los demás armados de SYSTEM-S1CC: instalarlo reemplaza al anterior.

## 2. Panel

| CZ | control del SYSTEM-1 | qué hace |
|---|---|---|
| DCO1, forma de onda de la línea 1 | OSC1 WAVE | posiciones 0…4 = WAVE 1…5 del CZ (§3); posición 5 = WAVE 6 (resonancia, ventana sierra) |
| DCO2, forma de onda de la línea 2 | OSC2 WAVE | posiciones 0…4 = WAVE 1…5; posición 5 = WAVE 7 (resonancia, ventana triángulo) |
| DCW de cada línea | OSC1 / OSC2 COLOR | distorsión 0…1: 0 = seno (WAVE 1…5) |
| envolvente DCW | OSC1 / OSC2 COLOR MOD = FILTER ENV | DCW = COLOR × envolvente FILTER (A / D / S / R). También LFO, PITCH ENV, AMP ENV, SUB OSC (§3.2 del anfitrión) |
| LINE SELECT 1+2' | OSC1 MIX / OSC2 MIX | nivel de cada línea, 0…0,5 |
| OCTAVE | OSC1 RANGE | las dos líneas (lo aplica el anfitrión al tono) |
| octava y DETUNE de la línea 2 | OSC2 RANGE, OSC2 FINE | línea 2 = línea 1 × 2^(OSC2 RANGE − OSC1 RANGE) × 2^(±1/12) (FINE 128 = afinada) |
| RING modulation | SUB | línea 2' × ((1 − m) + m · línea 1), m = SUB / 255 |
| NOISE modulation | NOISE | línea 2' × ((1 − m) + m · ruido), m = NOISE / 255 |
| envolvente DCA | AMP A / D / S / R | ADSR 0 del anfitrión (VCA) |
| envolvente DCO | PITCH ENV AMOUNT / ATTACK / DECAY | envolvente de pitch del anfitrión |
| VIBRATO | LFO (SHAPE, RATE, DELAY TIME, TO PITCH) | LFO del anfitrión por voz |
| — (el CZ no tiene) | FILTER CUTOFF / RESONANCE / ENV AMOUNT / KEY TRACK | SVF pasa bajos de `mp_dsp_b.h`; CUTOFF 255 y RESONANCE 0 = el CZ sin filtro |

Antes de que llegue el panel (fila 128 de cada tabla): WAVE = posición 0, RANGE 8', FINE afinada, SUB y NOISE 0.
SUB y NOISE son del kernel: `S1_BUILD("MIX=0")` saca el mezclador SUB / NOISE del anfitrión. La fuente SUB OSC de
COLOR MOD sigue existiendo.

## 3. Formas de onda

### 3.1 Las del CZ

| WAVE del CZ | bits (kasploosh) | forma con DCW alto |
|---|---|---|
| 1 sierra | 000 SAW | subida rápida hasta la rodilla, bajada lenta |
| 2 cuadrada | 001 SQUARE | dos medios ciclos, cada uno rápido y después sostenido |
| 3 pulso | 010 PULSE | un ciclo rápido en el medio del período, sostenido el resto |
| 4 seno-pulso («double sine») | 100 SINE-PULSE | un ciclo rápido al comienzo del período |
| 5 sierra-pulso | 101 SAW-PULSE | sierra comprimida en la primera parte del período, después sostenida |
| 6 / 7 / 8 resonancia | 110 MULTI-SINE con ventana 001 SAW / 010 TRIANGLE / 011 TRAPEZOID | seno a 1 + 15 · DCW veces la frecuencia, reiniciado en cada período, por la ventana |

Con DCW = 0 las WAVE 1…5 dan un seno (en el CZ igual). WAVE 8 (ventana trapecio) no tiene posición en el panel.

### 3.2 Una fórmula para todas

El DSP no salta: cada rama de un `if` se paga entera. En lugar de calcular seis formas y elegir una, la voz calcula
una sola fórmula de distorsión de fase y el selector WAVE cambia sus coeficientes. Cada selector es una tabla
`S1_PARAM` de 6 palabras. Con p la fase del oscilador (0…1) y d el DCW (0…1):

```
k   = (1 + a·d)^4, a = 32^(1/4) − 1          pendiente del tramo rápido: 1…32; a lo sumo rdt/2
r   ≈ 1/k                                    cúbica de 1/(1 + a·d), 0,5 %, a la cuarta
B   ≈ 1/(2 − r)                              cúbica en r, 0,2 %: pendiente del tramo lento de la sierra
R   = 1 + SQ + 15·F·d                        a lo sumo rdt/4
q   = R·p + O·(1 − r),   n = floor(q),   h = q − n
phi = min((KA·k + F)·h,  1 − (BG + BE)·B + (BG·B + BE)·h,  1)
y   = −(1 − F·win(p) − 2·SQ·n) · cos(2π·(1 − SQ/2)·phi)
win(p) = p (OSC1: ventana sierra 1 − p)  o  |2p − 1| (OSC2: ventana triángulo)
```

| posición | WAVE | SQ | F | O | KA | BG | BE | phi |
|---|---|---|---|---|---|---|---|---|
| 0 | 1 sierra | 0 | 0 | 0 | 1 | 1 | 0 | min(k·h, 1 − B + B·h): rodilla en phi = 1/2, h = 1/(2k) |
| 1 | 2 cuadrada | 1 | 0 | 0 | 1 | 0 | 0 | min(k·h, 1) en cada medio ciclo (R = 2), medio coseno, el segundo invertido |
| 2 | 3 pulso | 0 | 0 | −1/2 | 1 | 0 | 0 | min(k·h, 1), corrido (1 − r)/2: el ciclo rápido centrado en p = 1/2 |
| 3 | 4 seno-pulso | 0 | 0 | 0 | 1 | 0 | 0 | min(k·h, 1): el ciclo rápido al comienzo |
| 4 | 5 sierra-pulso | 0 | 0 | 0 | 2 | 0 | 1 | min(2k·h, 1 − B + h, 1) |
| 5 | 6 / 7 resonancia | 0 | 1 | 0 | 0 | 0 | 0 | h, con R = 1 + 15·d y la ventana del oscilador |

* **phi es un mínimo de rectas:** continuo para cualquier coeficiente. Los límites de k y R y las aproximaciones de
  r y B cambian la forma, no abren saltos.
* **Seguimiento de teclado del DCW.** k ≤ rdt/2: el tramo rápido dura al menos 2 muestras. R ≤ rdt/4: el seno de la
  resonancia queda debajo de fs/4. En las notas agudas el DCW máximo baja, como en el CZ.
* **Curva del DCW.** k = (1 + a·d)^4 es casi exponencial (1…32) con tres ops, sin `exp2` ni recíproco.
* **Línea 2.** Su paso es dt × razón (RANGE y FINE) y su 1/dt sale de los inversos de las mismas tablas, sin
  división.
* **Ruido.** n = wrap(K·n + W), K = 7,31275, W = 0,618…: el mapa del ruido del LFO del anfitrión con W constante,
  dos ops.

## 4. Presupuesto

Kernel fusionado (`--fused`, por defecto): la voz entera en un solo programa, las voces 0…3 en el DSP A y las 4…7 en
el DSP B. Medido 2026-11-03, con el anfitrión completo (`ARM_ENV=1 ARM_LFO=1`, `MOD_DSP=B`):

| | instrucciones |
|---|---|
| kernel por voz (osciladores, ring / noise, SVF, VCA) | 179 |
| DSP A: `s1m0` + sección `s1a2` | 284 + 373 = 657 (presupuesto 703) |
| DSP B: `s1m1` + sección `s1b2` | 387 + 311 = 698 (presupuesto 708) |

Cómo se llegó de 6 a 8 voces:

| cambio | por voz | voces |
|---|---|---|
| fórmula genérica, tabla de 13 palabras | 233 | 6 (y 40 palabras de parámetros: más que las 32 de `C060`…`C07F`) |
| tabla de 6 palabras, el resto derivado | 215 | 6 |
| `s1_recip` en lugar del segundo `s1_exp2` | 208 | 6 |
| k = (1 + a·d)^4 y r por polinomio | 186 | 7 (8: DSP B 714) |
| ruido en dos ops; ring / noise con (1 − m) en la tabla | 179 | 8 |

Con 9 palabras por tabla (R0, S, G y 15·F cargados en vez de derivados) el kernel subió a 195: más valores vivos,
más derrames con 4 registros por voz. Con 8 registros (`--regs 0,…,7`, sin voces de a pares) no entran 8 voces
(A 858, B 890).

## 5. Verificación

**Referencia nativa** (`s1cc check`, kernel fusionado, `level2=0 cutoff=255 color0=255`, solo la línea 1): SNR
75…76 dB (10 ms) en las posiciones 0…4 y 65 dB en la resonancia (posición 5). La línea 2 sola: 31…44 dB (10 ms). Es la deriva de fase de dt × razón contra el float de la
referencia: MP-CSAW2, que funciona en el equipo, da 39 dB con su línea 2 en el mismo banco.

**Formas** (referencia nativa, nota 33, DCW 0 / 128 / 255): las seis posiciones de cada oscilador tienen la forma de
§3.1. Con DCW 0, seno en las WAVE 1…5.

**Cadena completa** (`make_s1cc.py --sim`, emulador, 8 voces):

| caso | resultado |
|---|---|
| WAVE 1…5, DCW 0 en las dos líneas | armónicos sobre 1 kHz a −80 dB: seno |
| envolvente AMP (A 60, D 128, S 128, R 60) | ataque, decay al sustain, release a 0 |
| 8 notas | pico 0,508 |
| sin notas | pico 0,000000 |
| COLOR 255, COLOR MOD = FILTER ENV (A 0, D 150, S 0) | centroide espectral 3695 → 441 Hz en 0,5 s: el DCW sigue a la envolvente hasta el seno |
| COLOR MOD = LFO (RATE 200) | el centroide oscila entre 1460 y 3940 Hz |
| SUB (ring) 255, NOISE 255, resonancia DCW 200 | cambian el espectro (centroides 2600, 14000 y 8900 Hz) |

`sdk/s1cc/tests/test_s1cc.py` (`TestExamples`, caso `casio_cz/cz.c`) arma el kernel sin fusionar y verifica la
línea 1 (sierra, cuadrada, resonancia) y la línea 2 (sierra-pulso) contra la referencia.

## 6. Diferencias con el CZ

| CZ | aquí | por qué |
|---|---|---|
| envolventes de 8 etapas (DCO, DCW, DCA) por línea | ADSR del anfitrión: AMP = DCA, FILTER = DCW (con COLOR MOD), PITCH ENV (AD) = DCO; las mismas para las dos líneas | las del anfitrión (pedido: usar envolventes y filtros del anfitrión) |
| ondas combinadas (dos formas alternadas ciclo a ciclo) | no | el panel tiene un selector por línea |
| WAVE 8 (ventana trapecio) | sin posición | 6 posiciones por selector: 5 formas + una resonancia por oscilador. La tabla de ventanas admite el trapecio (1,5 − p − 0,5·\|2p − 1\|): `win` y una columna más |
| ring / noise: activado o no | cantidad continua (SUB, NOISE) | cruce entre la línea 2 sin modular y modulada |
| 4 voces en 1+2' (8 con una línea) | 8 voces con las dos líneas siempre | presupuesto §4 |
| sin filtro | SVF del anfitrión | pedido; CUTOFF 255, RESONANCE 0 lo deja abierto |
| forma exacta del DCW en función del valor | k = (1 + a·d)^4, tramo rápido hasta 1/32 del ciclo | curva elegida, no medida en un CZ |

## 7. Archivos

| archivo | qué es |
|---|---|
| `sdk/s1cc/examples/casio_cz/cz.c` | el kernel: tablas de formas, `cz_osc`, líneas, ring / noise; DSP B de `mutable_ported/include/mp_dsp_b.h` (SVF + VCA) |
| `plugins/CZ-SYSTEM/` | el PLUG-OUT armado por `build.sh` |
