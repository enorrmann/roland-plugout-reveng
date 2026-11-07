# MUTABLE-SYSTEM: qué más se puede portar de `eurorack/` (formas de onda, filtros, efectos, moduladores)

Estado: 2026-10-24. Inventario del repositorio de Mutable Instruments (`~/git/eurorack`, commit `f4cfa76`) para
portarlo con `sdk/mport` al microcódigo del SYSTEM-1, sobre el ejemplo `examples/mutable_system/`.

Etiquetas:
* **[medido]**: portado y comparado con el original.
* **[código]**: leído en la fuente de Mutable (qué tablas, qué estado y qué operaciones usa).
* **[estimado]**: coste calculado con los costes medidos de §1.2. No está compilado.
* **[abierto]**: depende de algo que todavía no funciona en el equipo (§1.3).

Lo único portado y medido es **SAW_SWARM** de Braids. Está instalado en el slot 1 (2026-10-24), sin escuchar
todavía.

## 1. Recursos y reglas que deciden si algo cabe

### 1.1 Presupuesto (instrucciones por muestra, 96 kHz)

| dónde | usado hoy (MUTABLE-SYSTEM) | máximo probado | libre |
|---|---|---|---|
| esclavo DSP A (8 voces: LFO + oscilador) | 638, de las que unas 270 son LFO y transporte y unas 46 por voz son el oscilador | 682 (693 funcionó en el paso 9) | **≈ 51 instr. por voz** para el oscilador, si se reemplaza SAW_SWARM |
| esclavo DSP B (8 voces: paso alto, ADSR ×2, filtro de 4 polos) | 564 | 708 | **≈ 18 instr. por voz**, o ≈ 144 una vez por muestra |
| maestro DSP B (después de la mezcla, antes del delay / reverb del OS) | 7 | sin medir: los pasos 14/14A colgaron por formas de op no probadas | efectos globales [abierto] |

Una instrucción lleva hasta 3 ops fusionadas y 3 transferencias, pero sólo con las combinaciones de
`asm/proven_forms.txt` (`ph_wrap` todavía no se fusiona con nada). Por eso un componente cuesta entre 1/2 y 2/3 de
sus unidades.

### 1.2 Costes medidos de las piezas básicas (unidades = ops + transferencias, antes de planificar)

| pieza | unidades | fuente |
|---|---|---|
| incremento de fase desde el tono (`exp2` + escala) | 12 | `Kernel.increment`, C-S1SY [medido] |
| sierra (fase += inc, `ph_wrap`, guardar) | 3–4 | SAW_SWARM [medido] |
| cuadrada / triángulo desde la fase | 1–2 (`ph_clamp(256·p)`, `ph_tri`) | C-S1SY paso 7 [medido] |
| curva por polinomio impar de grado 9 (tablas `ws_*`) | 7 | SAW_SWARM [medido] |
| seno desde el triángulo (polinomio impar de grado 7) | 5 | [estimado] (C-S1SY usa uno de grado 3) |
| filtro de estado (Chamberlin, 2 polos) | 7 + coeficiente 7 | SAW_SWARM paso alto [medido] |
| ruido (LCG + Weyl), global | 8 | C-S1SY paso 11 [medido] |
| un polo `y += g (x − y)` | 2–3 | C-S1SY filtro [medido] |

### 1.3 Memoria y operaciones (qué hay y qué falta)

| recurso | estado | qué componentes lo necesitan |
|---|---|---|
| estado en el anillo del banco 0 (2048 palabras por DSP, `ld S+1` / `st S`) | [medido] | todos |
| retardo **fijo** de hasta 2047 muestras (21 ms): carga simple `[S + k]` o `ldidx` con `h6` constante | [medido] 15X ([../system-1/DSP_BANKS.md](../system-1/DSP_BANKS.md) §4) | reverbs y difusores de tomas fijas |
| retardo **variable** (índice calculado → `hset h6`) | **[abierto]**: el chorus del paso 15 da tomas en silencio | cuerdas Karplus, comb, chorus, ensemble, pitch shifter, tablas de ondas leídas por fase |
| tablas en el banco 5 (`A0xx`/`A1xx`, compartido A↔B, ≈ 0x400 palabras) | fijas [medido en el SYSTEM-8]; leer por fase = índice calculado [abierto] | wavetables, formantes |
| tablas de control (ARM, `FCEC` → `C0xx`) | [medido] | todo lo que Mutable calcula por bloque (incrementos, coeficientes, tablas de knob) |
| recíproco, raíz, log, exp por semilla de la unidad B (SFU) | decodificados en el SYSTEM-8, **no probados en el SYSTEM-1** | resonancia por `1/(1+…)`, `tan()` de la SVF de stmlib, envolventes exponenciales |
| saltos / bucles (`009Y`) | probados en los esclavos del SYSTEM-8; no usados en el SYSTEM-1 | nada imprescindible (todo se escribe sin saltos) |

Retardo por voz: 2048 palabras entre 8 voces son 256 muestras por voz. Eso da 375 Hz como nota más grave de una
cuerda Karplus a 96 kHz. Con el banco 1 (también es un anillo) serían 512 muestras y 187 Hz [estimado].

### 1.4 Reglas de port (de `sdk/mport`, [MUTABLE_SDK.md](MUTABLE_SDK.md))

* **Lo que Mutable calcula una vez por bloque** (incrementos, coeficientes, lecturas de tabla en la posición del
  knob) se calcula en Python, en la build, para los 256 valores de cada knob → tabla del ARM → `C0xx`.
* **Lo que va por muestra** se escribe con `Kernel`. Una tabla leída por muestra pasa a polinomio (`lut.fit_odd`)
  mientras el índice calculado siga abierto.
* **Frecuencia de muestreo.** Braids corre a 96 kHz, como el equipo. Plaits y Rings corren a 48 kHz y Elements a
  32 kHz: sus coeficientes por muestra se recalculan para 96 kHz (`kSampleRate` en `*/dsp/dsp.h`) [código]. Los
  filtros de Plaits usan `tan()` en el ARM del módulo; aquí va en la tabla del ARM.
* **Referencia.** Hace falta stmlib, y en este checkout el submódulo está vacío. `sdk/mport/ref/stmlib` sólo tiene
  lo que usa Braids. Plaits, Rings y Elements necesitan además `stmlib/dsp/{dsp,filter,polyblep,
  parameter_interpolator,units}.h`: o se clona stmlib, o se amplían los sustitutos.

## 2. Formas de onda / osciladores

Coste por voz [estimado] = las instrucciones del DSP A, ya planificadas. "Cabe" = ≤ 51 por voz (§1.1).

### 2.1 Braids (`braids/analog_oscillator.cc`, `digital_oscillator.cc`, `macro_oscillator.cc`), 96 kHz, enteros

| modelo | qué es [código] | coste / voz [estimado] | cabe | bloqueo / nota |
|---|---|---|---|---|
| **SAW_SWARM** | 7 sierras desafinadas + tanh + paso alto | 46 (A) + 9 (B) [medido] | sí | **hecho** |
| CSAW | sierra con muesca (cambia la forma con TIMBRE/COLOR), polyblep | 15 | sí | polyblep = 4–6 ops más por discontinuidad |
| MORPH | triángulo → sierra → cuadrada → pulso, con filtrado | 20 | sí | sale de la fase con `ph_tri` y `ph_clamp` |
| SAW_SQUARE | mezcla sierra/cuadrada con PW | 15 | sí | |
| SINE_TRIANGLE | seno ↔ triángulo + plegado | 18 | sí | `wav_sine` → polinomio |
| BUZZ | pulsos tipo impulse train (suma de cosenos por tabla `wav`) | 20 | sí | tabla → fórmula cerrada |
| SQUARE_SUB / SAW_SUB | onda + sub una octava abajo | 18 | sí | sub = `ph_clamp` de la fase / 2 |
| SQUARE_SYNC / SAW_SYNC | dos osciladores con hard sync | 22 | sí | sync sin saltos: `fase −= d·fase`, como el reinicio del LFO de C-S1SY |
| TRIPLE_SAW / SQUARE / TRIANGLE / SINE | 3 osciladores con intervalo (COLOR) | 15–25 | sí | variante barata de SAW_SWARM |
| TRIPLE_RING_MOD | 3 senos multiplicados + `ws_moderate_overdrive` | 25 | sí | todo polinomios |
| SAW_COMB | sierra en un filtro peine | 15 + retardo | — | **[abierto]**: retardo variable |
| TOY | bit-crush y AND de fase (circuit bending) | 12 | sí | ops de bits de la unidad B: decodificadas, no usadas en el SYSTEM-1 |
| DIGITAL_FILTER_LP/PK/BP/HP | "filtro resonante" sintetizado con fases y `wav_sine` (no es un filtro real) | 25 | sí | |
| VOSIM | VOSIM: senos² con ventana `lut_bell` | 25 | sí | `lut_bell` → polinomio |
| VOWEL | formantes por tablas `wav_formant_*`, voz con ruido | 50+ | justo | tablas de formantes por fase = índice calculado, o por senos |
| VOWEL_FOF | FOF: 5 formantes con ventanas | 60+ | no | demasiado por voz |
| HARMONICS | aditiva de 12 senos | 12 × 6 = 70 | no | con 4–6 armónicos sí |
| FM / FEEDBACK_FM / CHAOTIC_FEEDBACK_FM | FM de 2 operadores con seno | 20–25 | sí | **buen candidato**: dos senos por polinomio; feedback = muestra anterior en el anillo |
| PLUCKED / BOWED / BLOWN / FLUTED | guía de onda (Karplus, arco, soplo, flauta) | 20–35 + retardo | — | **[abierto]**: retardo variable y 256 muestras por voz |
| STRUCK_BELL / STRUCK_DRUM | banco de resonadores (senos o SVFs) excitado | 40–80 | justo / no | campana con 4–6 modos sí |
| KICK / SNARE / CYMBAL | percusión: SVF excitada, ruido | 25–40 | sí | necesita disparo por nota (`A060+v`, ya está) |
| WAVETABLES / WAVE_MAP / WAVE_LINE / WAVE_PARAPHONIC | tablas `wt_waves` (8 bits, 129 puntos) | 10 + lectura | — | **[abierto]**: índice calculado; alternativa: wavetables propias de C-WTLIVE (SYSTEM-8) |
| FILTERED_NOISE / TWIN_PEAKS_NOISE | ruido por SVF / 2 resonadores | 20–30 | sí | ruido global + SVF por voz |
| CLOCKED_NOISE | ruido muestreado a una frecuencia (S&H) | 12 | sí | como el S&H del LFO |
| GRANULAR_CLOUD / PARTICLE_NOISE | granos de seno con envolvente al azar / partículas filtradas | 40+ | justo | número de granos por voz limitado |
| DIGITAL_MODULATION | portadora modulada por bits de datos (QPSK, etc.) | 25 | sí | curiosidad |

Por qué no las analógicas tal cual: Braids usa tablas de BLEP en enteros. Aquí la versión natural es polyblep
polinómica sobre `ph_wrap`: la fracción de cruce es `fase / incremento` (con un recíproco por voz, una vez por
muestra) [estimado].

### 2.2 Plaits (`plaits/dsp/`), 48 kHz, float: el que mejor encaja con nuestro DSP de punto flotante

| componente | archivo | coste / voz [estimado] | cabe | nota |
|---|---|---|---|---|
| VariableShapeOscillator (sierra ↔ pulso ↔ triángulo, PW, sync) | `oscillator/variable_shape_oscillator.h` | 30 | sí | polyblep; base de un "VA" completo |
| VariableSawOscillator | `oscillator/variable_saw_oscillator.h` | 20 | sí | |
| SuperSquareOscillator | `oscillator/super_square_oscillator.h` | 25 | sí | |
| ZOscillator (formante con sync tipo CZ) | `oscillator/z_oscillator.h` | 30 | sí | |
| FormantOscillator, VOSIM | `oscillator/formant_oscillator.h`, `vosim_oscillator.h` | 25 | sí | |
| HarmonicOscillator (aditiva) | `oscillator/harmonic_oscillator.h` | 6 por armónico | con 6 | Chebyshev de cosenos: barato por armónico |
| NesTriangleOscillator | `oscillator/nes_triangle_oscillator.h` | 8 | sí | triángulo cuantizado a 4 bits |
| StringSynthOscillator | `oscillator/string_synth_oscillator.h` | 35 | sí | divisoras tipo string machine |
| WavetableOscillator / GrainletOscillator | `oscillator/wavetable_oscillator.h`, `grainlet_oscillator.h` | — | — | **[abierto]** índice calculado / grainlet ≈ 30, sí |
| motor *virtual analog* (2 VariableShape) | `engine/virtual_analog_engine.cc` | 55 | justo | con un solo oscilador cabe |
| motor *waveshaping* (pendiente + plegador por tablas) | `engine/waveshaping_engine.cc` | 30 | sí | tablas del plegador → polinomios o `ph_tri` (como FOLD de C-S1SY) |
| motor *FM* (2 op con feedback) | `engine/fm_engine.cc` | 25 | sí | **candidato fuerte** |
| motor *swarm* (8 osciladores con envolvente por grano) | `engine/swarm_engine.cc` | 70+ | no | 3–4 granos sí |
| motor *chord*, *additive*, *wavetable* | `engine/chord_engine.cc`, `additive_engine.cc`, `wavetable_engine.cc` | 60+ | no | polifonía dentro de la voz: no cabe con 8 voces |
| motor *phase distortion* (CZ) | `engine2/phase_distortion_engine.cc` | 25 | sí | **candidato** |
| motor *wave terrain* | `engine2/wave_terrain_engine.cc` | 30 | sí | terrenos que son funciones (seno de sumas): polinomios |
| motor *chiptune* | `engine2/chiptune_engine.cc` | 20 | sí | |
| motor *six op* (DX7) | `engine2/six_op_engine.cc` | 6 × 15 | no | 2–3 operadores sí |
| motor *string machine* | `engine2/string_machine_engine.cc` | 60 + chorus | no | |
| motores de percusión (bombo, caja, hat analógicos y sintéticos) | `drums/*.h` | 25–45 | sí | |
| motores *string* / *modal* (física) | `physical_modelling/*` | 60+ | no | retardo variable [abierto] / modos: pocos |
| motor *speech* (LPC, SAM, formantes) | `speech/*` | — | no | tablas grandes y control complejo |
| motor *grain* / *particle* / *noise* | `engine/grain_engine.cc`, `particle_engine.cc`, `noise_engine.cc` | 30–40 | sí | |

### 2.3 Otros módulos

| módulo | componente | coste / voz [estimado] | nota |
|---|---|---|---|
| Stages | `variable_shape_oscillator.h` (osciladores en modo audio) | 25 | casi igual al de Plaits |
| Warps | `dsp/oscillator.cc` (oscilador interno: seno, triángulo, sierra, cuadrada, ruido filtrado) | 15 | |
| Edges | ondas cuadradas de chip con PW (8 bits, firmware AVR) | 8 | sencillo |
| Peaks | bombo / caja / hats analógicos (enteros) | 25–35 | como los de Braids |
| Tides 2 | generador de pendientes polifónico (rampa con forma) | 20 | también como LFO / envolvente |

## 3. Filtros

| componente | origen | coste [estimado] | dónde | nota |
|---|---|---|---|---|
| **Svf de Braids** (Chamberlin, enteros, con *punch*) | `braids/svf.h` | 14 por voz | B | **ya portada** (el paso alto de SAW_SWARM); LP/BP/HP es la misma salida con otra palabra |
| Svf de stmlib (TPT, `tan()` del corte, LP/BP/HP/notch/peak) | `stmlib/dsp/filter.h` (no está en este checkout) | 12 por voz | B | más estable que Chamberlin con corte alto; `g = tan(π fc/fs)` en la tabla del ARM o por polinomio |
| OnePole / DCBlocker (stmlib) | `stmlib/dsp/filter.h` | 3 por voz | A o B | |
| **LowPassGate** (vactrol: VCA + paso bajo acoplados) | `plaits/dsp/fx/low_pass_gate.h` | 15 por voz | B | **candidato fuerte**: sustituto "Buchla" del VCA + filtro, con el ADSR de amplitud como control |
| VCF de *virtual analog VCF* (SVF con saturación) | `plaits/dsp/engine2/virtual_analog_vcf_engine.cc` | 18 por voz | B | segundo modo del filtro (pedido pendiente: 2 polos / 4 polos, §9.1 del README de C-S1SY) |
| banco de resonadores modales (hasta 64 SVF pasabanda) | `rings/dsp/resonator.cc`, `elements/dsp/resonator.cc` | 8 por modo | global | por voz no cabe; global, 8–12 modos sobre la mezcla |
| banco de filtros del vocoder (20 bandas) | `warps/dsp/filter_bank.cc`, `vocoder.cc` | 200+ | global | sin entrada de audio externa en el SYSTEM-1: no tiene sentido [inferido] |
| Ripples, Blades, Shelves | — | — | — | **no hay código**: son módulos analógicos (sólo `hardware_design/`) |

## 4. Efectos

Van una vez por muestra sobre la mezcla: en el esclavo B, después de sumar las voces (≈ 144 instrucciones libres),
o en el maestro B [abierto].

| componente | origen | coste [estimado] | bloqueo | nota |
|---|---|---|---|---|
| **SampleRateReducer** (decimación con S&H) | `plaits/dsp/fx/sample_rate_reducer.h` | 8 | — | **candidato fuerte** para el pote CRUSHER (`0x18C`), que hoy no hace nada |
| bit-crush (RESOLUTION de Braids) | `braids/braids.cc` (máscara de bits) | 4 | ops de bits no probadas en el SYSTEM-1 | alternativa sin bits: cuantizar con `mul` + truncado del DSP |
| Overdrive | `plaits/dsp/fx/overdrive.h` | 8 | — | polinomio; por voz o global |
| SIGNATURE de Braids (`ws_*` por módulo) | `braids/signature_waveshaper.h` | 7 | — | polinomio, como el de SAW_SWARM |
| Ensemble / Chorus | `plaits/dsp/fx/ensemble.h`, `rings/dsp/fx/chorus.h`, `ensemble.h` | 40–60 | **[abierto]**: retardo variable | es lo que falló en el paso 15 de C-S1SY |
| Diffuser (4 allpass de tomas fijas) | `plaits/dsp/fx/diffuser.h`, `clouds/dsp/fx/diffuser.h` | 25 | — | **se puede hoy** con tomas fijas (carga simple `[S + k]`); longitudes × 2 por los 96 kHz |
| Reverb (Griesinger/Dattorro, FxEngine) | `rings/dsp/fx/reverb.h`, `elements/dsp/fx/reverb.h`, `clouds/dsp/fx/reverb.h` | 60–80 | las líneas de Rings suman más de 2048 palabras; 2 tomas moduladas por LFO [abierto] | versión corta (líneas ÷ 4, sin modulación) cabe en un banco 0; el OS ya da su reverb |
| Pitch shifter | `clouds/dsp/fx/pitch_shifter.h` | 30 | [abierto] retardo variable | |
| Limiter / compresor | `rings/dsp/limiter.h`, `warps/dsp/limiter.h`, `streams/compressor.cc` | 10–20 | — | seguidor de envolvente + ganancia: un `1/x` (recíproco) o una tabla de control |
| Modulador de Warps (crossfade, fold, ring mod analógico/digital, XOR, comparador, frequency shifter) | `warps/dsp/modulator.cc` | 10–40 | XOR / comparador = ops de bits | ring mod / fold **entre OSC1 y OSC2 por voz** (≈ 10): candidato; frequency shifter (allpass en cuadratura, `quadrature_transform.h`) ≈ 40 global |
| Granulares y loopers (Clouds, Beads) | `clouds/dsp/granular_processor.cc`, `beads/dsp/*` | — | — | **no**: necesitan segundos de memoria (la línea larga del OS no es nuestra) |

## 5. Moduladores (LFO, envolventes, aleatorios)

Casi todo esto va a la frecuencia de control. Puede ir en el ARM o en el DSP con pocas instrucciones.

| componente | origen | dónde | nota |
|---|---|---|---|
| envolventes de decay / LPG | `plaits/dsp/envelope.h` | DSP B | alternativa a los ADSR para percusión |
| Segment generator (rampas, etapas encadenadas) | `stages/segment_generator.cc` | ARM o DSP | envolventes con curvas: la mejora pendiente de C-S1SY ("envolventes toscas", §9) |
| generador de pendientes polifónico | `tides2/` | DSP | LFO / envolvente con forma |
| generadores aleatorios (smooth random, Bernoulli, distribución con *spread*) | `marbles/random/*`, `plaits/dsp/noise/smooth_random_generator.h` | ARM | fuente de modulación nueva (COLOR mod, LFO S&G) |
| Lorenz | `streams/lorenz_generator.cc` | ARM o DSP | LFO caótico |
| Dust / partículas | `plaits/dsp/noise/dust.h`, `particle.h` | DSP | ruido de impulsos |

Grids (secuenciador), Yarns (MIDI), Kinks, Links, Shades, Veils y Volts no aportan DSP portable: son
secuenciadores, firmware de control o módulos analógicos.

## 6. Orden propuesto

Ordenado por lo que suena distinto, por lo que cuesta y por lo que se puede hacer hoy. Cada fila es una
instalación (regla de C-S1SY: una cosa por paso).

| # | qué | por qué | coste / riesgo |
|---|---|---|---|
| 1 | Strike de SAW_SWARM (fases al azar en cada nota) | completa el port actual | +7 por voz en A |
| 2 | **SampleRateReducer en CRUSHER** | el pote no hace nada; efecto típico de Braids | 8, global en B |
| 3 | **FM / Feedback FM de Braids o motor FM de Plaits** en el selector de onda de OSC1 | un timbre nuevo con poco código | ≈ 25 por voz |
| 4 | **Low-pass gate de Plaits** como modo del VCA | carácter Buchla con los ADSR ya hechos | ≈ 15 por voz en B |
| 5 | Phase distortion (Plaits) o Z oscillator | timbres CZ | 25–30 por voz |
| 6 | Modulador de Warps (ring mod / fold) entre dos osciladores | requiere volver a 2 osciladores por voz | ≈ 10 + el segundo oscilador |
| 7 | Diffuser de tomas fijas en el esclavo B | espacio propio sin el índice calculado | ≈ 25 global |
| — | todo lo que lleva retardo variable o tablas leídas por fase (Karplus, comb, chorus, wavetables) | primero hay que resolver el índice calculado (`hset h6` con un valor de registro: paso 15Y de C-S1SY) | — |

Para elegir el modelo con el selector de onda (0–5) en el DSP se suman todos los modelos con coeficientes 0/1,
como hace hoy `step_wave` con la sierra, la cuadrada y el triángulo. Pero entonces el coste es la **suma** de los
modelos, no el máximo. Con el presupuesto de §1.1 caben dos o tres modelos baratos juntos. Para más hace falta
cargar otro programa en el DSP al cambiar de modelo [abierto]: el ARM vuelve a llamar a `FC30`/`FC34` con otro
programa; no se ha probado mientras suena.
