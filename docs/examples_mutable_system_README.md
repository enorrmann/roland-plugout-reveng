# MUTABLE-SYSTEM: C-S1SY13F con SAW_SWARM de Braids (`examples/mutable_system/`)

Estado: 2026-10-24. Construido y validado en el emulador. **Instalado en el slot 1** (reemplazó al SH-2 de
Roland que estaba en ese slot), **sin escuchar todavía** en el equipo [abierto]. Desde el mismo día el oscilador
también puede venir de una voz en C / C++ compilada con s1cc (`KERNEL=`, §8, [S1CC.md](S1CC.md)).

Es el sinte de 8 voces C-S1SY13F ([examples_system_1_synth_README.md](examples_system_1_synth_README.md)) con los
dos osciladores básicos por voz (DCO1 + DCO2) reemplazados por **SAW_SWARM** de Braids: 7 sierras desafinadas, un
waveshaper tanh y un paso alto. Está portado con `sdk/mport` ([MUTABLE_SDK.md](MUTABLE_SDK.md)). Todo lo que viene
después del oscilador no cambia: LFO por voz, ADSR de VCA y de filtro, filtro de 4 polos, delay y reverb del OS.
Qué más se puede portar: [examples_mutable_system_CANDIDATES.md](examples_mutable_system_CANDIDATES.md).

Etiquetas: **[medido]** en el equipo, **[sim]** en el emulador, **[código]**, **[inferido]**, **[abierto]**.

## 1. Uso

```sh
sh examples/mutable_system/build.sh --help
sh examples/mutable_system/build.sh                     # -> plugins/MUTABLE-SYSTEM/Script/Code1.Dat
amidi -l                                                # ¿está el SYSTEM-1?
python3 tools/plugout_tool.py install plugins/MUTABLE-SYSTEM --slot 1 --force --no-patches -y
```

Después de instalar hay que seleccionar el PLUG-OUT en el panel.

Emulador:

```sh
python3 examples/mutable_system/make_mutable.py --sim          # La 440, 8 notas, silencio
python3 examples/mutable_system/make_mutable.py --pack-check   # empaquetado = sin empaquetar, 7 paneles (≈ 7 s)
python3 examples/mutable_system/make_mutable.py --sim-check    # además núcleo C = referencia Python (minutos)
python3 sdk/mport/cli.py check saw_swarm                       # el oscilador contra el C++ de Braids
```

Opciones de la build (`build.sh --help`):

| opción | por defecto | qué hace |
|---|---|---|
| `SWARM` | 1 | 1: SAW_SWARM; 0: DCO1 + DCO2 de C-S1SY |
| `WS_DEGREE` | 9 | grado del polinomio del waveshaper (7: 97 LSB de error, unas 2 instrucciones menos por voz) |
| `STEP` | 13 | paso de C-S1SY sobre el que se construye |
| `NAME` | `MUTABLE-SYSTEM` | nombre en el equipo y carpeta en `plugins/` |
| `GMAX`, `RESCOMP`, `RES_MAX`, `VOICES`, `MODEL_ID`, `PARAMLOG`, `PAD`, `PACK`, `SCHED`, `FUSE` | — | como en C-S1SY |

## 2. Panel

| control (id) | qué hace aquí | destino |
|---|---|---|
| OSC1 COLOR (`0x168`) | **TIMBRE** de Braids: detune de las sierras (0 = casi unísono, 255 = ±0,8 semitonos en las exteriores) | DSP A `C070`–`C075` (6 razones de incremento, `swarm_ratio_bits`) |
| OSC2 COLOR (`0x16D`) | **COLOR** de Braids: corte del paso alto (0 = nota − 64 semitonos, abierto; 85 ≈ la nota; 255 = nota + 64 semitonos) | DSP B `C060` (H, `swarm_hp_bits`) |
| OSC1 RANGE (`0x16A`) | octava (64'…2') | DSP A `C044` |
| OSC1 MIX (`0x174`) | nivel del enjambre | banco 5 `A040` |
| OSC1/OSC2 WAVE, OSC2 RANGE, OSC2 FINE, OSC2 MIX | **sin efecto**: el ARM sigue escribiendo sus parámetros, pero el programa no los lee | — |
| resto (LFO, filtro, ADSRs, delay, reverb, VOLUME) | como C-S1SY13 | [examples_system_1_synth_README.md](examples_system_1_synth_README.md) §3.1 |

Valores iniciales, hasta que se mueve la perilla: OSC1 COLOR = 128 (detune medio), OSC2 COLOR = 0 (paso alto
abierto). Las tablas salen de `Param.table()` del componente (`make_mutable.py header()`).

**VOLUME al instalar.** Observado en el equipo el 2026-10-27: MUTABLE-S1CC no sonaba hasta mover la perilla de
VOLUME, en cualquier posición inicial. Hipótesis: `m_load` (`model.c`) escribe `C01C` con `FCEC` justo después de
encolar la carga del programa del maestro B, y la carga encolada llega después y deja `C01C` en 0. Cambio:
`a_note_on` vuelve a escribir el último VOLUME en cada nota (una escritura `FCEC`). Sin verificar en el equipo.
C-S1SY y los demás `model.c` iguales no tienen este cambio.

## 3. Arquitectura

| DSP | por voz | instrucciones |
|---|---|---|
| esclavo A `s1m0` | LFO (`step_lfo_voice`, sin cambios) → tono + range → incremento (`exp2`) → 7 sierras → waveshaper → × nivel × trémolo → `A151+v`; incremento → `A188+v` (`swarm_voice`) | **638** (C-S1SY13F: 585; máximo probado 682, 693 funcionó en el paso 9) |
| esclavo B `s1m1` | paso alto de SAW_SWARM sobre `A151+v` (`swarm_hp_voice`) → ADSR de VCA → ADSR de filtro → filtro de 4 polos (sin cambios) | **564** (C-S1SY13F: 471; máximo probado 708) |
| maestro B `s1mm1` | volumen → envío / retorno del delay y la reverb del OS | 7 |

* **Paso alto en el DSP B.** El DSP A no tiene sitio para él. Además es lineal, así que da igual que vaya antes o
  después del nivel y del trémolo. El único cambio es que el recorte a ±1 de la SVF actúa sobre la señal ya
  escalada.
* **Planificación.** Como con `FOLD` y en el paso 14, sólo se fusionan los grupos de `asm/proven_forms.txt`
  (`step_program`: `STEP >= 14 or FOLD or SWARM`). Sin esa restricción el DSP A queda en 598 [sim], pero con
  combinaciones de ops que nunca corrieron en el equipo (por ejemplo `ph_wrap` con otra op).
* **`check_optimized`.** Avisa en el DSP A de 25 `transfer-only` y 56 `fusable`. Son los pares que la lista de
  formas probadas no deja fusionar: justificado por la regla 7 de
  [MICROCODE_OPTIMIZATION.md](MICROCODE_OPTIMIZATION.md). En el DSP B avisa de 1 `transfer-only` y 24 `fusable`.

### 3.1 Mapa de memoria (lo que añade a C-S1SY13)

| qué | dirección | escribe → lee |
|---|---|---|
| razones de las sierras k = −3, −2, −1, 1, 2, 3 | DSP A `C070`–`C075` | ARM (`FCEC(0, …)`) → A |
| H del paso alto | DSP B `C060` | ARM (`FCEC(1, …)`) → B |
| incremento del centro de la voz v | banco 5 `A188+v` | A → B (misma muestra) |
| fases de las 7 sierras de la voz v | banco 0 del DSP A, `0x0400 + 0x10·v` + 0, 2, …, 12 (anillo) | A |
| `lp` / `bp` del paso alto | banco 0 del DSP B, bloque de voz (`0x0400 + 0x20·v`) + `0x16` / + `0x18` (anillo) | B |
| salida del paso alto | bloque de voz de B + `0x1A` (se escribe y se lee en la misma muestra) | B → ADSR de VCA |

Ya no se usan: las fases DCO1/DCO2 (bloque de A + 0/+2) y los parámetros de onda / fold de A (`C050`–`C057`).

### 3.2 Código generado (paso alto, voz 0, DSP B, planificado)

Copiado de `examples/mutable_system/s1m1.s`. Las demás unidades de cada instrucción (`st` a `6008`–`600D`,
`addxy r7` de la mezcla) son de la parte de C-S1SY con la que el planificador lo intercala:

```dsp
mul r7, r7, #0.16666666666666666 | mul r3, r1, r2 | st r0, [6009]
mul r7, r7, r6 | mov r6, #1.0 | st r0, [600a]
min r4, r3, #0.39269908169872414 | st r7, [6006]
max r7, r4, #0.000267552393860559 | st r7, [6007]
mul r1, r7, r7 | st r0, [600b]
mul r2, r1, #-0.3333231608072917 | ld r1, [0419] | st r0, [600c]
addxy r3, r2, #1.99993896484375 | ld r2, [0417] | st r0, [600d] | ld r0, [a151]
mul r4, r3, r7
mad r2, r4, r1
mad r0, r1, #-1.99993896484375
ph_clamp r3, r3, -r2
subxy r7, r3, r0 | st r3, [0416] | ld r0, [0405]
mad r1, r4, r7 | ld r4, [a160]
ph_clamp r2, r3, -r7 | st r1, [0418] | ld r3, [a168] | ld r1, [0401]
```

## 4. Validación [sim]

| prueba | resultado |
|---|---|
| `--pack-check`: 7 paneles (defecto, filtro + envolventes, 3 LFO, swarm ancho con paso alto, swarm cerrado con paso alto abierto), 19 200 muestras, 8 notas | idéntico bit a bit |
| `--sim-check`: núcleo C contra la referencia Python, mismos 7 paneles, 600 muestras | idéntico bit a bit |
| cadena completa contra el C++ de Braids, niveles por banda alineados en 200 Hz–1 kHz (notas 69 / 57 / 45) | ±0,4 dB hasta 4 kHz; −1,9 dB (4–12 kHz) y −10 a −15 dB (12–40 kHz) por el filtro de 4 polos con `GMAX` 0,7 (≈ 10,7 kHz) |
| oscilador solo (`cli.py check saw_swarm`) | [MUTABLE_SDK.md](MUTABLE_SDK.md) §7.4 |

**Corrección de la referencia Python.** En `simulate_py`, sólo en esta copia, el maestro no modelaba el anillo del
banco 1. El retorno del efecto (`2001`/`2003`) quedaba en 0 y la salida era siempre silencio desde el paso 13. Ahora
el retorno toma el envío de la muestra anterior, como en el núcleo C. `make_synth.py` de C-S1SY sigue con ese
hueco: su `--sim-check` del paso 13 compara contra silencio.

## 5. Qué escuchar en la primera prueba

1. **Al seleccionar el PLUG-OUT, no tiene que colgarse.** Hay dos riesgos:
   * el DSP A está en 638 instrucciones, por debajo de las 682 probadas;
   * aparecen formas de instrucción que se usan por primera vez solas en el DSP A del SYSTEM-1 (`mad` con
     inmediato 0,125, `max` con inmediato, polinomios con `ymacd` encadenados). Ninguna es fusión nueva.
2. **Al tocar una nota:** una sierra gruesa con detune medio.
3. **OSC1 COLOR:** de casi unísono (batido lento) a un coro ancho.
4. **OSC2 COLOR:** desde 0 (todo el cuerpo) hasta un sonido fino y agudo. Por encima de unos 200, y en notas
   agudas, el paso alto se acerca a su límite de estabilidad y puede sonar con ruido. Pasa igual en Braids
   ([MUTABLE_SDK.md](MUTABLE_SDK.md) §7.3).
5. **El resto del panel** tiene que funcionar como en C-S1SY13F.

Si cuelga, primero probar `SWARM=1 WS_DEGREE=7` (unas 16 instrucciones menos en el DSP A). Si sigue colgando,
volver a C-S1SY13F (`STEP=13 FOLD=1 NAME=C-S1SY13F sh examples/system_1_synth/build.sh`).

## 6. Siguientes pasos

Una cosa por instalación ([examples_mutable_system_CANDIDATES.md](examples_mutable_system_CANDIDATES.md) §6):
* **strike:** fases al azar en cada nota, unas 7 instrucciones por voz en el DSP A;
* **SampleRateReducer de Plaits** en el pote CRUSHER;
* **FM de Braids / Plaits** en el selector de onda;
* **low-pass gate de Plaits** como modo del VCA;
* **build con `SCHED` sin restricciones** (598 en el DSP A), para probar en el equipo las formas fusionadas
  nuevas y añadirlas a `asm/proven_forms.txt`.

## 7. Archivos

`examples/mutable_system/FILES.md`. Lo nuevo respecto a `examples/system_1_synth`:

| archivo | cambio |
|---|---|
| `make_mutable.py` | copia de `make_synth.py`: `SWARM`, `swarm_voice`, `swarm_hp_voice`, tablas, parámetros de simulación, paneles, corrección de `simulate_py` |
| `engine.c` | `P_C1` → 6 razones al DSP A, `P_C2` → H al DSP B (`#if SWARM`); nombre por defecto |
| `model.c` | sólo el nombre por defecto |
| `build.sh` | ayuda, `make_mutable.py`, `NAME=MUTABLE-SYSTEM` |
| `s1*.s`, `s1*.hex`, `build/` | generados |

## 8. Voz compilada desde C / C++ (`KERNEL=`)

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/braids_saw_swarm_s1.cc ~/git/eurorack/braids/resources.cc \
    -I sdk/mport/ref -I ~/git/eurorack
KERNEL=sdk/s1cc/build/braids_saw_swarm_s1 NAME=MUTABLE-S1CC sh examples/mutable_system/build.sh
python3 tools/plugout_tool.py install plugins/MUTABLE-S1CC --slot 1 --force --no-patches -y
```

Con `KERNEL=<salida de s1cc>` el oscilador sale del kernel en vez de `swarm_voice` (`SWARM` pasa a 0). La voz
del DSP A es `step_lfo_voice` + `kernel_voice_a`; la del DSP B empieza con `kernel_voice_b`. Mapa por voz v:

| qué | dirección |
|---|---|
| tono del kernel (`V9_PM` + range de OSC1) | `V8_PHASE[v] + 2` del DSP A (se escribe y se lee en la misma muestra) |
| puerta / disparo | `A038+v` / `A060+v` |
| estado y temporales del código del DSP A | banco 0 del DSP A, `0x0400 + 0x20·v` (2 palabras cada uno, hasta 16) |
| estado y temporales del código del DSP B | banco 0 del DSP B, `0x0600 + 0x20·v` |
| salida del código del DSP A | `V8_PHASE[v]` → × nivel de OSC1 × trémolo → `A151+v` (= entrada de B) |
| salida del código del DSP B | bloque de voz de B + `0x1A` → ADSR de VCA, como el paso alto de SAW_SWARM |
| `s1_export(i)` | `A188 + 8·i + v` |
| entradas que B toma de A | `A1A0 + 8·j + v` |
| tablas `S1_PARAM` | parámetros del DSP A desde `C070`, del DSP B desde `C060`, en el orden del kernel |

En el ARM, `make_mutable.py header()` escribe las tablas de cada `S1_PARAM`, ya divididas por su escala y en la
rejilla del registro: `kparam_panel`, `kparam_count`, `kparam_a`, `kparam_b`, `kparam_off`, `kparam_bits`.
`engine.c` las escribe con `kparam_set` cuando llega su `panel_id` (también si ese id está en la tabla del panel) y
todas en `panel_all`.

Resultado con `braids_saw_swarm_s1.cc` [sim, 2026-10-24]:
* DSP A 626 instrucciones y DSP B 620. La versión en Python (`SWARM=1`) usaba 638 / 564; los máximos probados son
  682 / 708.
* `--pack-check` y `--sim-check` idénticos en 7 paneles.
* La salida coincide con la de `SWARM=1` a ±0,03 dB por banda; por debajo de 200 Hz difiere ~1 dB, porque el kernel
  reproduce el SVF entero de Braids.
* La build avisa si un programa pasa de los máximos probados.
