# `sdk/s1cc/examples/mutable_ported`: osciladores y filtros de Mutable Instruments escritos para el DSP

Ports en C de los osciladores y filtros de Braids, Plaits y stmlib (Emilie Gillet, MIT), **reescritos** para los
DSP esclavos del SYSTEM-1 tal como los compila s1cc ([S1CC.md](S1CC.md)). No son envoltorios del código original
(eso es `sdk/s1cc/examples/mutable/`, [§7](#7-relación-con-los-envoltorios)): cada algoritmo está escrito en la
forma que el DSP ejecuta barata. Cada archivo `mp_*.c` es un kernel completo para SYSTEM-S1CC
([examples_system_s1cc_README.md](examples_system_s1cc_README.md)): el oscilador en el DSP A, filtro y envolventes
del anfitrión en el DSP B. Son 33 kernels y todos entran con **8 voces**.

Estado (2026-10-28): verificados contra su referencia nativa (`s1cc check`) y en la cadena completa del emulador
(`make_s1cc.py --sim`); **ninguno probado en el equipo todavía**. 14 leen OSC1 / OSC2 COLOR con su COLOR MOD
(`s1_color`, §4).

## 1. Uso

```sh
sdk/s1cc/examples/mutable_ported/check_all.sh                     # todos: s1cc build + check + presupuesto de 8 voces
sdk/s1cc/examples/mutable_ported/check_all.sh mp_csaw.c           # uno
sdk/s1cc/s1cc build sdk/s1cc/examples/mutable_ported/mp_csaw.c --regs 0,1,2,3,4,5,6,7 --check
KERNEL=sdk/s1cc/build/mp_csaw NAME=MP-CSAW sh examples/system_s1cc/build.sh          # -> plugins/MP-CSAW (8 voces)
python3 tools/plugout_tool.py install plugins/MP-CSAW --slot 1 --force --no-patches -y
```

`check_all.sh` escribe en `${OUT:-/tmp/mutable_ported}` e imprime, por kernel: instrucciones por voz de A y B, SNR
contra la referencia (10 ms, nota 57, perillas en 128) y los totales de los programas del esclavo con
`VOICES` (por defecto 8; el máximo probado en el equipo es 708 por DSP).

## 2. Archivos

| archivo | qué es |
|---|---|
| `include/mp_dsp_a.h` | piezas del DSP A: `mp_dt` / `mp_rdt`, `mp_frac`, `mp_step`, `mp_advance`, `mp_saw_bl`, `mp_pulse_bl`, `mp_sin` / `mp_cos`, `mp_fold`, `mp_noise` |
| `include/mp_filter.h` | piezas del DSP B: SVF de Chamberlin con doble muestreo, ladder de 4 polos, un polo, low-pass gate, `mp_soft` |
| `include/mp_dsp_b.h` | programa opcional del DSP B (VCF + VCA) y su panel: filtro elegido con `MP_FILTER`, envolventes, VCA. Cada kernel incluye `mp_dsp_a.h` y, si no trae su propio DSP B, `mp_dsp_b.h` |
| `mp_*.c` | un kernel por oscilador o filtro (§4, §5) |
| `check_all.sh` | build, verificación y presupuesto de todos |

## 3. Reglas de escritura para el DSP

| regla | por qué | cómo |
|---|---|---|
| sin divisiones en la voz | `x / y` usa `recipseed`, probado en el SYSTEM-8 pero no en el SYSTEM-1 (W910); además cuesta ~13 ops | lo que depende de una perilla, en la tabla `S1_PARAM` (corre en el PC); lo que depende de la nota, de `s1_dt()` / `s1_rdt()` |
| `dt` y `1/dt` por nota del anfitrión | calcularlos son dos `exp2` (~25 ops por voz) | `s1_dt()`, `s1_rdt()`: dos cargas de palabras que `engine.c` escribe con la nota |
| fases float en [0, 1) con `if (p >= 1) p -= 1` | s1cc la guarda en [−1, 1) y el wrap es un `ph_wrap` | `mp_advance`; para pasos que pueden pasar 1 (relación × nota aguda), `mp_step` (≤ 1/2) |
| sin signos en los bordes | una función signo se difumina en el borde: una muestra de error cuando un período cae justo ahí | `mp_saw_bl` (§3.1) |
| ramas cortas | el DSP calcula los dos lados de cada `if` | formas sin ramas (`mp_saw_bl`), `select` de constantes (baratos) |
| filtros sin `tan()` ni división | coeficientes por muestra con la envolvente | coeficientes lineales en w (a 96 kHz alcanza), SVF con doble muestreo |
| amplitud ≤ ~0,7 por voz | SYSTEM-S1CC suma 8 voces con ganancia 2/(1,5·NV) | perilla `level` en OSC1 MIX |

### 3.1 Sierra limitada en banda sin signo

`mp_saw_bl(t, dt, rdt)` da `2·frac(t) − 1` con polyBLEP de dos muestras en el borde `t = 0` (el de
`stmlib/dsp/polyblep.h`). Con `w` la fase llevada a −½…½ por un wrap y `u = clamp(w / dt, −1, 1)`, la sierra ingenua
más el residuo de stmlib (`2u − u² − 1` después del borde, `u² + 2u + 1` antes) es exactamente

```c
2w − u (2 − |u|)
```

Lejos del borde `u = ±1` y queda `2w ∓ 1 = 2·frac(t) − 1`; cerca, la corrección de stmlib. No aparece el signo del
lado del borde. Unas 8 ops: wrap, `min`/`max` de `w` contra ±dt (así `u` ∈ [−1, 1]), un producto, `|u|`, un producto
y una resta. Todo lo que tiene saltos se arma con ella:

| forma | expresión |
|---|---|
| pulso de ancho `pw` | `mp_saw_bl(t − pw) − mp_saw_bl(t) + 2 pw − 1` |
| indicador `[frac(t) < pw]` | `pw − (s0 − s1) / 2`, con `s0 = mp_saw_bl(t)`, `s1 = mp_saw_bl(t − pw)` |
| salto de altura H en el borde de otra fase (sync) | `H/2 · (mp_saw_bl(p) − (2p − 1))` |

### 3.2 DSP B común (`include/mp_dsp_b.h`)

Es opcional: el kernel que lo incluye recibe este `S1_DSP_B`. Uno que trae su propio DSP B no lo incluye y escribe
`S1_DSP_B` con las piezas de `include/mp_filter.h` (`mp_vowel.c`, §5). SYSTEM-S1CC corre el `S1_DSP_B` que tenga el kernel
y no agrega filtro; sin `S1_DSP_B`, el DSP B sólo mezcla.

```
x (DSP A) → filtro (MP_FILTER) → × s1_env(0) (AMP) → salida
corte: w = w0(FILTER CUTOFF) · 2^(s1_env(1) · ENV AMOUNT + KEY TRACK · (pitch − 4,75)), ≤ 1
```

| `MP_FILTER` | filtro | B por voz |
|---|---|---|
| `MP_SVF_LP` (por defecto), `MP_SVF_BP`, `MP_SVF_HP` | SVF de Chamberlin (`svf.h` de Braids, en float), dos pasos por muestra; estable con f + q < 2: q 1,4…0,02, f ≤ 0,55 (~17 kHz) | 38–39 |
| `MP_LADDER` | 4 polos `y += a (x − y)`, realimentación k 0…3,9 por un recorte suave, compensación 1 + k/2 | 43 |
| `MP_LPG` | low-pass gate (Plaits `fx/low_pass_gate.h`): la envolvente AMP abre VCA y paso bajo (w ∝ g²) | 44 |
| `MP_NONE` | sólo VCA | — |

Panel: FILTER CUTOFF 20 Hz…17 kHz, RESONANCE, FILTER ENV AMOUNT ±8 octavas (128 = 0), KEY TRACK 0…1; AMP y FILTER
A/D/S/R son las ADSR del anfitrión ([examples_system_s1cc_README.md](examples_system_s1cc_README.md) §3.1).

## 4. Osciladores

Instrucciones por voz (emulador, 2026-10-28, después de la normalización de `min` / `max` y de `s1_color`) y totales
de los programas del esclavo con 8 voces. Los kernels con COLOR MOD leen el color con `s1_color`, y SYSTEM-S1CC les
agrega LFO, envolvente de pitch y sub oscilador por voz en el DSP A (~43 instrucciones por voz,
[examples_system_s1cc_README.md](examples_system_s1cc_README.md) §3.2): de ahí sus totales de A. SNR contra la
referencia nativa del mismo C (10 ms, nota 57). En los kernels con ruido el SNR no aplica (el hash da otra secuencia
en el DSP): se comparan las bandas, todas dentro de ±0,4 dB.

| kernel | origen | perillas (además de OSC1 MIX = nivel) | COLOR MOD | A / voz | 8 voces A / B |
|---|---|---|---|---|---|
| `mp_saw` | sierra polyBLEP (Braids / Plaits VA) | — | — | 20 | 155 / 590 |
| `mp_square` | pulso con ancho (Braids SQUARE) | OSC1 COLOR ancho | OSC1 | 35 | 584 / 590 |
| `mp_csaw` | Braids CSAW (sierra con muesca) | OSC1 COLOR ancho, OSC2 COLOR profundidad | OSC1, OSC2 | 45 | 703 / 590 |
| `mp_variable_shape` | Plaits VariableShapeOscillator | OSC1 COLOR forma, OSC2 COLOR ancho | OSC1, OSC2 | 44 | 704 / 590 |
| `mp_tri_fold` | Braids TRIANGLE_FOLD | OSC1 COLOR ganancia | OSC1 | 13 | 407 / 590 |
| `mp_sine_fold` | Braids SINE_FOLD | OSC1 COLOR ganancia | OSC1 | 18 | 461 / 590 |
| `mp_saw_swarm` | Braids SAW_SWARM (7 sierras) | OSC1 COLOR desafinación | — | 32 | 249 / 590 |
| `mp_triple_saw` | Braids TRIPLE_SAW | OSC1 / OSC2 COLOR intervalos | — | 57 | 451 / 590 |
| `mp_triple_sine` | Braids TRIPLE_SINE | OSC1 / OSC2 COLOR intervalos | — | 37 | 283 / 590 |
| `mp_fm` | FM de 2 operadores (Braids FM, Plaits FM) | OSC1 COLOR relación, OSC2 COLOR índice | OSC2 | 29 | 543 / 582 |
| `mp_feedback_fm` | Braids FEEDBACK_FM | OSC1 COLOR realimentación | OSC1 | 18 | 455 / 590 |
| `mp_phase_distortion` | Plaits phase distortion (CZ) | OSC1 COLOR rodilla | — | 33 | 273 / 590 |
| `mp_formant` | Plaits FormantOscillator | OSC1 COLOR formante, OSC2 COLOR fase | OSC2 | 22 | 501 / 590 |
| `mp_vosim` | Braids VOSIM | OSC1 / OSC2 COLOR formantes | — | 33 | 273 / 590 |
| `mp_harmonic` | Plaits HarmonicOscillator (8 armónicos) | OSC1 COLOR brillo, OSC2 COLOR pares | — | 83 | 680 / 590 |
| `mp_sync` | Braids SAW_SYNC | OSC1 COLOR relación | OSC1 | 40 | 631 / 590 |
| `mp_super_square` | Plaits SuperSquareOscillator (2 cuadradas: desafinadas, o la segunda en sync) | OSC1 COLOR forma | — | 70 | 547 / 590 |
| `mp_chord` | Plaits chord engine (4 sierras en un acorde) | OSC1 COLOR acorde | — | 80 | 627 / 590 |
| `mp_sub` | Braids SAW_SUB | OSC1 COLOR nivel del sub | — | 47 | 378 / 590 |
| `mp_ring` | ring mod de dos senos | OSC1 COLOR relación | OSC1 | 38 | 607 / 590 |
| `mp_nes_triangle` | Plaits chiptune (triángulo de 4 bits) | OSC1 COLOR escalones | — | 16 | 152 / 590 |
| `mp_toy` | Braids TOY / CRUSHER | OSC1 COLOR frecuencia de muestreo, OSC2 COLOR bits | — | 33 | 259 / 590 |
| `mp_noise` | Braids FILTERED_NOISE (B: pasa banda) | — | — | 10 | 90 / 575 |
| `mp_clocked_noise` | Braids CLOCKED_NOISE | OSC1 COLOR reloj | — | 28 | 234 / 590 |
| `mp_kick` | bombo (Braids KICK, Plaits) | OSC1 COLOR barrido, OSC2 COLOR tiempo | OSC1 | 24 | 509 / 590 |
| `mp_snare` | caja (B: pasa altos) | OSC1 COLOR ruido / tono | OSC1 | 29 | 528 / 582 |
| `mp_hihat` | hi-hat metálico (6 cuadradas del 808, B: pasa altos) | OSC1 COLOR ruido | — | 79 | 626 / 582 |
| `mp_bell` | Braids STRUCK_BELL (4 parciales) | OSC1 COLOR decaimiento, OSC2 COLOR inarmonicidad | — | 70 | 562 / 590 |

La percusión reinicia su barrido / sus parciales en cada comienzo de nota (`s1_trig()`); el decaimiento del nivel es
la envolvente AMP (DECAY / RELEASE cortos, SUSTAIN 0).

## 5. Filtros

| kernel | filtro | oscilador | B / voz |
|---|---|---|---|
| (todos los de §4) | SVF paso bajo | — | 38–39 |
| `mp_filter_ladder` | ladder de 4 polos | sierra | 43 |
| `mp_filter_bp` | SVF pasa banda | pulso | 38 |
| `mp_filter_hp` | SVF pasa altos | sierra | 38 |
| `mp_lpg` | low-pass gate | triángulo plegado | 44 |
| `mp_vowel` | 3 formantes en paralelo (SVF pasa banda; vocal a-e-i-o-u en OSC1 COLOR, OSC2 COLOR corrimiento, RESONANCE ancho); su propio DSP B, sólo la envolvente AMP | pulso | 53 (8 voces: B 559) |

## 6. Lo que agregaron al compilador y al anfitrión

| cambio | dónde | efecto |
|---|---|---|
| `s1_dt()` / `s1_rdt()` | `s1dsp.h`, `graph.cpp`, `lower.cpp`, `s1rt.c`, `s1check.py`; SYSTEM-S1CC: `A040+v`, `A048+v` (`engine.c`, tablas `dt_bits` / `rdt_bits`) | −25 ops por voz en todo oscilador limitado en banda |
| fases float en [−1, 1) | `chooseStateReps` (`isPhaseWrap`) | el wrap es 1 op (7 fases de `mp_saw_swarm`: 63 → 43 instrucciones) |
| W910 | `lower.cpp` | aviso de división por `recipseed` |
| `min` / `max` normalizados | `lower.cpp` (`minmax`) | un valor recortado deja de quedar con escala diminuta: SVF del DSP B 42 → 39 por voz (8 voces 614 → 590) y su SNR 55–65 → 80–100 dB; el ladder sigue igual (+1 de planificación en 8 voces) |
| `s1_color(i)` | `s1dsp.h`, `graph.cpp` (E119 / E120), `lower.cpp` (rango 0…1), `s1rt.c` (`color0=` / `color1=`, barrido en el perfilado), `s1check.py`; SYSTEM-S1CC: fuentes y mezcla en el DSP A, [examples_system_s1cc_README.md](examples_system_s1cc_README.md) §3.2 | OSC1 / OSC2 COLOR con su COLOR MOD en 14 kernels (§4) |
| tests | `tests/test_s1cc.py`: `mp_saw`, `mp_csaw` (con `color0` / `color1`), `mp_harmonic`, `mp_filter_ladder` | regresiones del compilador; bandas comparadas sólo donde hay sonido |

Detalle de las transformaciones de s1cc: [S1CC.md](S1CC.md) §4.5–4.7 y §7.2.1.

## 7. Relación con los envoltorios

`sdk/s1cc/examples/mutable/` compila el C original de Braids / Plaits (envoltorios). Es exacto respecto al módulo,
pero caro: CSAW de Braids ocupa 160 instrucciones por voz (4 voces) contra 44 de `mp_csaw` (8 voces), y algunas
formas no entran (tablas leídas por fase, `recipseed`). `BRAIDS-CSAW` (envoltorio, 4 voces) se instaló el
2026-10-27 y dio ruido y clicks en notas graves en el equipo (captura USB: muestras sueltas en el tramo plano); la
sospecha principal es `recipseed`. Los ports de esta carpeta no dividen.

## 8. Abierto

* Ninguno probado en el equipo; primero `mp_saw` y `mp_csaw` (comparar con la captura de `BRAIDS-CSAW`). `MP-CSAW`
  armado el 2026-10-28 con COLOR MOD: probar cada fuente.
* COLOR MOD sin `s1_color` en los kernels pesados (A > ~45 por voz con el anfitrión: `mp_harmonic`, `mp_chord`,
  `mp_hihat`, `mp_bell`, `mp_super_square`, `mp_triple_saw`, `mp_sub`) y en los de tabla no afín barata
  (`mp_saw_swarm`, `mp_triple_*`, `mp_toy`, `mp_vosim`, `mp_nes_triangle`, `mp_phase_distortion`,
  `mp_clocked_noise`, el ratio de `mp_fm`, `mp_formant` OSC1, `mp_kick` OSC2): con 6 voces entrarían, o con una
  forma más barata.
* `s1_dt` / `s1_rdt` son palabras nuevas del banco 5 (`A040+v`, `A048+v`, en el rango que C-S1SY usaba para notas):
  sin probar en el equipo.
* Formas que faltan: tablas de ondas (índice calculado), cuerdas / guía de onda (retardo variable), granular.
