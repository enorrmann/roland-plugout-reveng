# C-S1SY: sintetizador sustractivo de 8 voces para el SYSTEM-1 (`examples/system_1_synth/`)

Estado: 2026-10-23. **Último build que funciona por completo: C-S1SY13F** (`STEP=13 FOLD=1`) [medido, a oído]:
* 8 voces, cada una con DCO1 + DCO2;
* forma de onda, nivel y range por oscilador, más fine tune en OSC2; el triángulo se pliega con el pote COLOR (§7.5);
* ADSR de VCA;
* filtro paso bajo de 4 polos con cutoff, resonancia y key track;
* ADSR de filtro con cantidad bipolar;
* LFO por voz (6 formas, delay, → pitch / filtro / amplitud bipolares);
* delay y reverb del OS (paso 13).

Los pasos 14 y 15 (efectos propios en el pote CRUSHER) están abiertos (§7.4).

Extiende C-S1V8 ([examples_system_1_README.md](examples_system_1_README.md) §7). Todo el
código ARM y el microcódigo son nuestros; los programas de Roland sólo se leyeron para estudiar los mecanismos. Qué
es cada archivo: `examples/system_1_synth/FILES.md`.

Etiquetas: **[medido]** = probado en el equipo, **[sim]** = simulador (`re/dspisa.py` o su núcleo C, §1), **[código]** = visto en los
binarios de Roland (estudio), **[inferido]**, **[abierto]**.

## 1. Uso

```sh
sh examples/system_1_synth/build.sh --help             # todas las opciones
sh examples/system_1_synth/build.sh                    # STEP=8 empaquetado -> plugins/C-S1SY/Script/Code1.Dat
python3 tools/plugout_tool.py install plugins/C-S1SY --slot 1 --force --no-patches -y
```

Después de instalar hay que seleccionar el PLUG-OUT en el panel. Opciones de la build: `STEP` (paso, por defecto 8),
`PACK` (empaquetado, por defecto 1), `GMAX` (tope del filtro, por defecto 0,7), `NAME`, `PARAMLOG`, `PAD`, `VOICES`.

Simulación sin equipo:

```sh
python3 examples/system_1_synth/make_synth.py --sim         # La 440, 8 notas, silencio: ≈ 1,4 s
SIM=py python3 examples/system_1_synth/make_synth.py --sim  # lo mismo con el simulador de referencia (minutos)
python3 examples/system_1_synth/make_synth.py --pack-check  # empaquetado / fusión contra sin empaquetar, sólo C (≈ 3 s)
python3 examples/system_1_synth/make_synth.py --sim-check   # además núcleo C contra la referencia Python (≈ 25 s, sólo si cambia el emulador)
```

`--sim` corre en el núcleo C del emulador (`examples/from_scratch/emu/cchain.py`, programas traducidos a código
nativo), en un solo `cchain.Slave`: DSP 0 = esclavo A, DSP 1 = esclavo B, DSP 2 = master B. Con
`Model.shared5 = ((0, 1),)` A y B tienen un solo banco 5, el mismo modelo que la referencia Python (`simulate_py`,
`re/dspisa.py`). `Model.own_params` da un banco 6 a cada DSP. Las salidas `6006`/`6007` de B llegan al master por
el IOMD en la misma muestra ([EMULATOR.md](EMULATOR.md) §3.6.1). Desde el 2026-10-23, 1 s de audio tarda unos 0,4 s. Rendimiento: 16 400 muestras en 2,4 s, frente a 3 440 muestras en 3 min 24 s con
la referencia (unas 400 veces más rápido). `--sim-check` da salidas idénticas bit a bit en 600 muestras con dos notas
(una se suelta) [sim], con el panel por defecto y con filtro, resonancia y ambas envolventes activas. Después
compara, en el núcleo C, los programas empaquetados (`pack()`, §5) con los mismos sin empaquetar (`sources(False)`):
19 200 muestras, 8 notas, la mitad soltadas a mitad, los mismos dos paneles. Todo `--sim-check` tarda
unos 80 s, casi todos de la referencia Python. Las longitudes salen de `SIM_CHECK_N` (600) y `SIM_PACK_N` (19 200).

La referencia antes de este cambio tenía dos huecos en el paso 8, ya corregidos: no escribía los parámetros `C050`–`C053`
(ADSR del filtro) ni `C04C` (cantidad) del DSP B, y movía a mano el anillo con el paso de bloque 0x10 de B en vez
de 0x20. El núcleo C mueve los bancos 0 y 1 enteros como anillos (S+1 por muestra).

## 2. Pasos

Regla de trabajo: cada paso añade **una sola cosa** al último que sonó, y cada uno se prueba con una instalación. El
primer diseño, que lo metía todo de una vez (`STEP=0`, §6), no sonó, y no había forma de saber por qué.

| paso | qué añade | DSP A / B (instr.) | resultado |
|---|---|---|---|
| 1 | DCO2 por voz: segunda copia de la sierra de C-S1V8, +7 cents, nivel 0,5 fijo | 541 / 13 | [medido] |
| 2 | niveles OSC1/OSC2 (`0x174`/`0x175`) → banco 5 `A040`/`A041` (ARM `FD6C` sel 0, leídos por el DSP A) | 581 / 13 | [medido] |
| 3 | transporte por voz: A guarda cada voz en `A151+v` y copia la puerta `A038+v` → `A160+v`; B multiplica y mezcla | 570 / 53 | [medido]; la primera versión, con B leyendo `A038+v`, no sonó (§3) |
| 4 | ADSR del VCA en el DSP B; faders `0x187`–`0x18A` → `C040`–`C043` del DSP B (`FCEC(1, …)`); disparo `A060+v` copiado por A → `A168+v` | 594 / 351 | [medido] |
| 5 | OSC1/OSC2 range (`0x16A`/`0x16F`: 64'…2' = −3…+2 octavas) y OSC2 fine tune (`0x170`, ±1 semitono) → `C044`–`C046` del DSP A (`FCEC(0, …)`) | 674 / 351 | [medido] |
| 6 | filtro de 4 polos por voz en el DSP B, detrás del VCA: `0x17D`/`0x17F`/`0x186` → `C048` corte (octavas desde 440 Hz), `C049` k, `C04A` ganancia de entrada, `C04B` key track; el tono lo copia A → `A178+v`; saturación con `ph_clamp`; B saca la suma de la muestra anterior | 682 / 703 | [medido] |
| 7 | ondas: selector `0x167`/`0x16C` → coeficientes de sierra, cuadrada y triángulo, `C050`–`C055` del DSP A; posición 2 = cuadrada (`ph_clamp(256·fase)`), 3 = triángulo (`ph_tri`) | 826 / 703 | **[medido] cuelga** el equipo al seleccionarlo: presupuesto (§4) |
| 7 + `PAD` | prueba de presupuesto: el paso 6 con 144 `mov r4, r4` al final del DSP A | 826 / 703 | **[medido] cuelga** |
| 6 empaquetado | el paso 6 con las transferencias empaquetadas como Roland (§5) | 507 / 460 | [medido] igual que el 6; [sim] mismas muestras bit a bit |
| 7 empaquetado | el paso 7 empaquetado (`PACK=1`, por defecto) | 587 / 460 | [medido] ondas ok |
| 8 | ADSR del filtro por voz en el DSP B (empaquetado): faders `0x182`–`0x185` → `C050`–`C053` del DSP B, cantidad `0x181` → `C04C`; corte = base + cantidad × env + key track; estado en el bloque de voz de B + `0x10` (bloques de `0x20`) | 587 / 660 | [medido] |
| 8 bis | cantidad bipolar: `0x181` = 128 → 0; 0 → −8 octavas (envolvente invertida); 255 → +8 octavas (sólo cambia la tabla `amount_bits` del ARM) | 587 / 660 | instalado, sin confirmar |
| 9 | LFO global en el DSP A (§7.2): rate `0x162`, shape `0x160`, delay `0x161`, → pitch `0x163`, → filtro `0x164`, → amplitud `0x165` → `C058`–`C05D`, `C060`–`C064` del DSP A; el corte (base + LFO) pasa a B por `A180+v` | 693 / 652 (700 con `RESCOMP=1`) | [sim]; C-S1SY9 instalado, sin confirmar |
| 10 | sierra con `ph_wrap rD, r3, rY` (forma de Roland) en vez del wrap manual de 11 ops ([MICROCODE_OPTIMIZATION.md](MICROCODE_OPTIMIZATION.md)) | 517 / 652 | [sim] La 440 = 439,98 Hz como antes |
| 11 | LFO por voz (§7.3): fase propia reiniciada en cada nota y S&H propio, ruido global; → filtro y → amplitud bipolares (128 = 0); fusión de ops (`FUSE=1`) | 576 / 603 | [medido] C-S1SY11 "funciona muy bien" (2026-10-23): primera build con ops fusionadas en el SYSTEM-1 |
| 11 bis | → pitch bipolar; formas en el orden del panel: sine, triangle, saw, square, sample & glide, sample & hold (§7.3) | 640 / 603 | [sim] `--sim-check` idéntico |
| 12 | planificador de instrucciones `asm/dspsched.py` en lugar de `pack()` ([MICROCODE_OPTIMIZATION.md](MICROCODE_OPTIMIZATION.md) §5) | 553 / 471 | [sim] `--pack-check` y `test_s1_sched` idénticos; sin probar en el equipo |
| 13 | **delay y reverb del OS**: `s1mm1` hace el envío/retorno del efecto (maestro B banco 1 `2000`/`2002` → `2001`/`2003`), como el mezclador de serie ([../system-1/FX_CHAIN.md](../system-1/FX_CHAIN.md)) | 553 / 471 / 7 | [medido] "funciona perfecto" (C-S1SY13) |
| 13 + `FOLD` | paso 13 + triángulo con wavefolding en el pote COLOR de cada oscilador (`0x168`/`0x16D` → `C056`/`C057` del DSP A; §7.5) | 585 / 471 / 7 | [medido] C-S1SY13F "funciona" (2026-10-23); [sim] `--pack-check` idéntico — **último build que funciona por completo** |
| 14 (primer intento) | AMP TONE + ensemble de 3 tomas en el maestro B | 553 / 471 / 64 | [medido] **cuelga**; también cuelga 14A (sólo tone, maestro B) y 14D (tone, esclavo B); 14C (DSP del paso 13 + `model.c` del 14) funciona; 14E (sólo el tone, esclavo B) funciona pero cambia el volumen, no el brillo (§7.4) |
| 14F | paso 13 + sólo el ensemble en el esclavo B, planificado con la lista de formas probadas (`asm/proven_forms.txt`) | 553 / 534 / 7 | [medido] no cuelga; el ensemble "suena raro, no a ensemble" |
| 15 | paso 13 + chorus estilo Juno-60 en el esclavo B (pote CRUSHER) | 553 / 508 / 7 | [medido] no cuelga; **las tomas salen en silencio**: captura USB = sierra pura, L = R (§7.4) |
| 15X | diagnóstico: retardo fijo de 750 muestras, L por carga normal, R por `ldidx` | — | [medido] los dos iguales y con el peine: **la línea y `ldidx` funcionan** ([../system-1/DSP_BANKS.md](../system-1/DSP_BANKS.md) §4) |
| 15Y | el 15 con la fracción por `ph_mod2(2k)` en lugar del AND de bits | 553 / 512 / 7 | instalado 2026-10-23; el usuario: "no funciona el chorus" [abierto: confirmar con captura] |

## 3. Cómo llega cada valor a cada DSP (SYSTEM-1)

| camino | quién lo lee | estado |
|---|---|---|
| ARM `FCEC(sel, id, bits)` → parámetro `C0id` del DSP A (`sel 0`) o B (`sel 1`) del esclavo | ese DSP | [medido] A (paso 5) y B (paso 4) |
| ARM `FD6C(0, …)` → banco 5 `A0xx` | DSP A | [medido] (tono, puerta, niveles) |
| ARM `FD6C(0, …)` → banco 5 `A0xx` | DSP B | **no llega** [medido]: el paso 3 con B leyendo `A038+v` quedó mudo |
| DSP A → banco 5 `A1xx` → DSP B | DSP B | [medido] (`A150`, `A151+v`, `A160+v`, `A168+v`, `A178+v`) |
| ids del panel → `engine vt+0x30` del esclavo | ARM del esclavo | [medido] (paso 2): los ids de patch `0x16x`–`0x18x` también llegan |

Así lo hace Roland en el SH-2 [código] (`python3 system-1/s1_progs.py work/unpack/SH-2_S1_Code1 --listing`):
* los mandos llegan a los DSP como parámetros `C0xx`;
* el intercambio entre los DSP del esclavo pasa por `A1xx`;
* el estado de cada DSP va en su banco 0.

C-S1SY sigue ese esquema. Solo la puerta, el disparo y el tono pasan por una copia del DSP A, porque ese camino se
probó primero. El SH-2 los escribe desde el ARM con `FD6C sel 1` (`A1xx`); en nuestro caso eso no está probado.

### 3.1 Mapa: de un control del panel al microcódigo (estado del paso 13 + `FOLD`)

Para cambiar o añadir algo no hace falta leer el código: este mapa dice dónde está cada cosa. Se mantiene a mano;
si se cambia una dirección o un id en `make_synth.py` / `engine.c`, se cambia aquí en el mismo cambio.

**Camino de un control.** El OS llama a `engine vt+0x30` con (id, valor) → `engine.c` busca el id en
`panel_id[]` (índice `P_*`, valor inicial en `panel[]`) → `panel_apply(p)` convierte el valor 0–255 con una tabla
`*_bits` (floats generados por `make_synth.py header()` en `build/s1_programs.h`) → `b6(sel, id, bits)` (= `FCEC`,
parámetro `C0id` del DSP A con `sel 0`, del DSP B con `sel 1`) o `b5(idx, bits)` (banco 5 `A000 + idx`, sólo lo lee
el DSP A). Para un control nuevo: (1) añadir `P_X` al enum **antes de `P_N`**, su id al final de `panel_id[]` y su
valor inicial al final de `panel[]`; (2) un `case P_X:` en `panel_apply`; (3) la tabla en `header()`; (4) el
parámetro en `sim_params()` para que el simulador lo vea; (5) la carga `ld rN, [c0id]` en el microcódigo.

| control (id) | `P_*` | tabla ARM | destino | lo usa (función de `make_synth.py`) |
|---|---|---|---|---|
| OSC1 / OSC2 mix (`0x174`/`0x175`) | `P_L1`/`P_L2` | `level_bits` | banco 5 `A040`/`A041` | `step_s1m0` (× onda) |
| OSC1 / OSC2 wave (`0x167`/`0x16C`) | `P_W1`/`P_W2` | `step_wave_bits` (`STEP_WAVES`) | A `C050`–`C052` / `C053`–`C055` (sierra, cuadrada, triángulo) | `step_wave` |
| OSC1 / OSC2 color (`0x168`/`0x16D`) | `P_C1`/`P_C2` | `fold_bits` (`fold_gain`) | A `C056` / `C057` = g/2 (sólo `FOLD=1`) | `step_wave(prm, fold)` |
| OSC1 / OSC2 range (`0x16A`/`0x16F`) | `P_R1`/`P_R2` | `range_bits` | A `C044` / `C045` (octavas) | `step_s1m0` |
| OSC2 fine (`0x170`) | `P_F2` | `fine_bits` | A `C046` (octavas) | `step_s1m0` |
| cutoff (`0x17D`) | `P_CUT` | `cutoff_bits` | A `C05D` (+ LFO) → `A180+v` → B | `step_lfo_voice`, `step_filter_voice` |
| resonancia (`0x17F`) | `P_RES` | `res_bits`, `gain_bits` | B `C049` k, `C04A` ganancia de entrada | `step_filter_voice` |
| key track (`0x186`) | `P_KT` | `level_bits` | B `C04B` | `step_filter_voice` |
| filter env amount (`0x181`) | `P_FAMT` | `amount_bits` | B `C04C` | `step_filter_voice` |
| ADSR filtro (`0x182`–`0x185`) | `P_FA`…`P_FR` | `level_bits`, `attack_bits`, `decay_bits` | B `C050`–`C053` (S, A, D, R) | `step_adsr` en `step_filter_voice` |
| ADSR VCA (`0x187`–`0x18A`) | `P_AA`…`P_AR` (`default:`) | ídem | B `C040`–`C043` (S, A, D, R) | `step_adsr` en `step_filter_voice` |
| LFO rate / delay (`0x162`/`0x161`) | `P_LRATE`/`P_LDELAY` | `lfo_rate_bits`, `lfo_delay_bits` | A `C058` / `C059` | `step_lfo_voice` |
| LFO → pitch / filtro / amp (`0x163`–`0x165`) | `P_LPITCH`, `P_LFILT`, `P_LAMP` | `lfo_pitch_bits`, `lfo_filt_bits`, `lfo_amp_bits`, `lfo_ampabs_bits` | A `C05A`, `C05B`, `C05C`, `C05E` (\|M\|) | `step_lfo_voice` |
| LFO shape (`0x160`) | `P_LWAVE` | `lfo_wave_bits` (`LFO_WAVES11`) | A `C060`–`C065` | `step_lfo_voice` |
| AMP TONE / CRUSHER (`0x18B`/`0x18C`) | `P_TONE`/`P_ENS` | `tone_bits`, `ens_*_bits` | B `C058`–`C05A` (sólo pasos ≥ 14) | `step_s1m1` (pasos 14/15) |
| nota / puerta / disparo | `voice_pitch`/`voice_gate` (`engine.c`) | `pitch_bits` | banco 5 `A030+v` / `A038+v` / `A060+v` | `step_s1m0`, `step_lfo_voice` |

**Parámetros `C0xx` libres** (paso 13 + `FOLD`). Lo que leen los programas generados (`grep -o '\[c0..\]' s1m0.s s1m1.s`): DSP A
`C044`–`C046`, `C050`–`C05E`, `C060`–`C065`; DSP B `C000` (nivel de salida), `C040`–`C043`, `C049`–`C04C`,
`C050`–`C053` (`C048` ya no se lee desde el paso 9; `C058`–`C05A` sólo en los pasos 14/15). Todo lo demás de
`C001`–`C0FF` está libre en cada DSP.

**Banco 5** (compartido por el par de esclavos): ARM → A: `A030+v` tono, `A038+v` puerta, `A040`/`A041` niveles,
`A060+v` disparo. A → B: `A151+v` voz (DCO1 + DCO2 × trémolo), `A160+v` puerta, `A168+v` disparo, `A178+v` tono,
`A180+v` corte.

**Banco 0** (anillos: se escribe en S y se lee S+1 en la muestra siguiente). DSP A: bloque de voz
`0x0300 + 0x10·v` (+0/+2 fases DCO1/DCO2, +4 fade, +6 tono modulado, +8 fase LFO, +10 S&H, +12 último disparo,
+14 término de amplitud), S&G `0x0240 + 2v`, ruido y LFO global `0x0280`–`0x028C`. DSP B: bloque de voz
`0x0400 + 0x20·v` (+0 env VCA, +2 latch, +4 último disparo, +6…+12 las 4 etapas del filtro, +0x10… ADSR del filtro).

**Registros dentro de un oscilador del DSP A** (`v8_saw` + `step_wave`, llamados desde `step_s1m0`): r0 = tono (octavas − 4,75), r1 = fase
(= sierra), r2 = suma de las ondas, r3 / r4 = temporales y cargas de parámetros, r7 = suma de la voz. r5 / r6 no
se tocan dentro de la onda.

**Presupuesto** (instrucciones por muestra, máximo probado 708, cuelga a 826; §4): paso 13 = A 553 / B 471;
paso 13 + `FOLD` = A 585 / B 471. Cada build lo imprime (`s1m0 … instructions`).

## 4. Presupuesto de tiempo de los DSP del esclavo [medido]

| programa del DSP A | instrucciones (una unidad cada una) | resultado |
|---|---|---|
| paso 6 | 682 | funciona |
| paso 7 | 826 | cuelga al seleccionar el PLUG-OUT |
| paso 6 + 144 `mov r4, r4` | 826 | cuelga |

El límite del SYSTEM-1 está **entre 682 y 826** instrucciones por muestra en el DSP A del esclavo, más bajo que las
925–945 medidas en el SYSTEM-8 (`docs/DSP_HARDWARE.md` §1). No se buscó el valor exacto: cada intento por encima
cuelga el equipo y hay que apagarlo y encenderlo. Para diseñar se toma **682 como máximo probado**.

Con el empaquetado (§5), las 587 instrucciones del paso 7 funcionan, así que una instrucción con hasta tres
transferencias cuesta como una sola [inferido: lo mismo que se midió en el SYSTEM-8 con A/A/B].

DSP B del esclavo:

| programa del DSP B | instrucciones | resultado |
|---|---|---|
| paso 6 sin empaquetar | 703 | funciona |
| paso 8 empaquetado + `RESCOMP=1` (C-S1SYR) | 708 | funciona (2026-10-23) |

**708 es el máximo probado del DSP B.** No se ha buscado su límite superior. Si el DSP B tiene el mismo límite que
el A, está entre 708 y 826.

## 5. Empaquetado de transferencias, como Roland [medido]

Roland pone en una misma instrucción hasta tres cargas o almacenamientos junto con una operación, y usa un registro
cargado **en la instrucción siguiente**, sin relleno (SH-2, programa principal del esclavo B):

```
ld r0, [a10a] | ld r7, [c000]
mul r1, r0, r7 | ld r0, [a10b]               ; usa r0 y r7 cargados en la instrucción anterior
mul r1, r0, r7 | st r1, [6006] | ld r0, [a100]
```

`make_synth.py` `pack()` hace lo mismo con nuestro código, escrito con una unidad por línea:
* Quita los rellenos `mov rN, rN` que teníamos detrás de cada carga.
* Une una carga a la instrucción anterior si la operación de esa instrucción no escribe el registro y no queda otra
  transferencia del mismo registro.
* Une un almacenamiento a la instrucción anterior si su operación no escribe el registro. Si no se puede, el
  almacenamiento abre una instrucción a la que se une la operación siguiente: todas las unidades leen antes de
  escribir, así que se guarda el valor anterior.
* Respeta los puertos de lectura: como máximo 2 cargas del banco 0 y 1 de cada otro banco por instrucción, 3
  transferencias en total, nunca la misma dirección dos veces.
* Solo acepta la unión si el ensamblador la codifica; las palabras salen idénticas a las de Roland, por ejemplo
  `3406 1407 9fe0 0fe2 6006 a100`.

Resultados:
* paso 6: el DSP A pasa de 682 a 507 instrucciones y el B de 703 a 460;
* paso 7: el DSP A pasa de 826 a 587;
* [sim] el paso 6 empaquetado da las mismas muestras que sin empaquetar (1600 muestras, tres notas, filtro y
  resonancia; diferencia 0,0);
* [sim] el paso 8 empaquetado (DSP A 587, B 660, master 9 instrucciones; sin empaquetar 826 / 1031 / 13) da salidas
  idénticas bit a bit a las de sin empaquetar (`make_synth.py --sim-check`, 2026-10-23: 19 200 muestras, 8 notas,
  4 soltadas; panel por defecto y panel con filtro, resonancia y ambas envolventes);
* [medido] el paso 6 y el paso 7 empaquetados funcionan igual en el equipo. Con eso queda confirmado que **una carga
  se puede usar en la instrucción siguiente**.

## 6. El primer intento, todo de una vez (`STEP=0`)

`STEP=0` (= `STAGE=4`) contenía:
* DSP A: dos osciladores con `ph_wrap`/`ph_tri` y ADSR de filtro;
* DSP B: ADSR de VCA y filtro;
* los parámetros en `A040`–`A05F`, escritos con `FD6C sel 0` y leídos también por el DSP B.

En simulación funcionaba; en el equipo no sonó. La causa encontrada después es la de §3: el DSP B no ve `A0xx`.
Además, en el paso 4 de ese diseño el DSP A habría superado el presupuesto (760 instrucciones sin empaquetar). Las
funciones de ese diseño quedan como referencia.

Nota del ensamblador: `ph_tri rD, rX, rY` se codifica como `ph_wrap`. Hay que escribir `ph_tri rD, rX, -rY`, y lo
mismo con `ph_clamp`: la variante se elige por el bit de negación de Y. Roland los usa con X = r3, por ejemplo
`ph_clamp r3, r3, -r3`, y C-S1SY también.

## 7. Filtro

* Cuatro etapas de un polo, cada una `y += g·(u − y)`, con realimentación `u = clamp(x − k·y4, −1, 1)`. `y4` es la
  salida de la muestra anterior. La saturación es `ph_clamp`.
* `g = 2π·440·2^c / 96000`, con `c` = corte + key track·(nota − La 440) en octavas. `2^c` se calcula como en los
  osciladores, con 4 términos de Taylor y 4 cuadrados.
* El tope de `g` es `GMAX`, 0,7 por defecto. [sim] Con k = 3,6 el bucle es inestable a la frecuencia de Nyquist si
  k·(g/(2 − g))⁴ ≥ 1, es decir, si g ≳ 0,83. Con 0,95 quedaba oscilando después de soltar la nota. Esa oscilación
  es a 48 kHz (Nyquist), no en la banda de audio [sim, núcleo C, 2026-10-23]: `GMAX` cambia el sonido pero no da
  autooscilación audible (§7.1).
* Ganancia de entrada `(1 + k/2)/2`: compensa la pérdida de graves con resonancia y deja margen a la saturación. La
  mezcla final multiplica por 2.

### 7.1 Autooscilación (`RESCOMP`)

Umbral del bucle lineal: hay oscilación si k > k_crit(g) = 4·c(g), con la frecuencia donde la fase de
z⁻¹·H(z)⁴ es −π, H(z) = g / (1 − (1 − g)·z⁻¹):

| corte | g | k_crit | frecuencia de oscilación |
|---|---|---|---|
| 50 Hz | 0,0033 | 4,01 | 50 Hz |
| 1 kHz | 0,065 | 4,29 | 1,07 kHz |
| 3 kHz | 0,196 | 5,04 | 3,74 kHz |
| 8 kHz | 0,524 | 9,60 | 17,8 kHz |
| — | 0,9 | 2,23 | 48 kHz |

En una escalera analógica (Juno-106) k_crit = 4 en todo el rango. Aquí la muestra de retardo de la
realimentación y la integración de Euler suben el umbral con el corte. Con `RES_MAX` = 3,6 el filtro no autooscila
nunca. El bucle lineal da la misma respuesta que el programa real en el núcleo C: nota de 50 ms, soltada; resonancia
255; nivel 0,5 s después de soltar [sim]:

| variante | corte 76 Hz | 239 Hz | 748 Hz | 2,34 kHz | 7,33 kHz | 19,9 kHz |
|---|---|---|---|---|---|---|
| por defecto (k 3,6) | se apaga | se apaga | se apaga | se apaga | se apaga | se apaga |
| `RES_MAX=4.3` | oscila | oscila | oscila | se apaga | se apaga | se apaga |
| `RES_MAX=5` | oscila | oscila | oscila | oscila | se apaga | se apaga |
| `RESCOMP=1` (k 4,4·c(g)) | oscila | oscila (242 Hz) | oscila (786 Hz) | oscila (2,77 kHz) | se apaga | se apaga |
| `GMAX=0.9` / `0.95` (k 3,6) | — | — | — | — | se apaga | 48 kHz |

`RESCOMP=1` multiplica k por c(min(g, 0,4)) = 1 + 0,778·g + 2,778·g², un ajuste de k_crit/4 con un error del 1,2 %
hasta g = 0,4. Son 6 instrucciones por voz en el DSP B: 708 instrucciones en vez de 660 (empaquetado), con
`--sim-check` idéntico. Por encima de g = 0,4 (corte de unos 6 kHz) no hay compensación completa y el filtro no
autooscila. La oscilación sale por encima del corte nominal (+1,5 % a 239 Hz, +5 % a 748 Hz, +18 % a 2,34 kHz), igual que el
umbral.

[medido, C-S1SYR, 2026-10-23, a oído] `RESCOMP=1 NAME=C-S1SYR sh examples/system_1_synth/build.sh` funciona en el
equipo y el filtro autooscila. Problemas abiertos:
* [abierto] la oscilación se queda trabada: sigue sonando después de soltar la nota. El filtro va detrás del VCA, así
  que el ADSR de amplitud no la apaga [inferido];
* [medido, a oído] el usuario prefiere el sonido sin `RESCOMP` ("suena feo"); las builds normales quedan con `RESCOMP=0`;
* [abierto] la oscilación depende de que haya volumen en los osciladores. En una escalera analógica arranca sola, por
  el ruido, aunque los osciladores estén a cero. Aquí, sin entrada, el estado del filtro se queda en 0.

### 7.2 LFO (paso 9)

Todo en el DSP A del esclavo; el DSP B sólo cambia una carga (`C048` → `A180+v`) y se queda en 700 instrucciones
con `RESCOMP=1`, por debajo de su máximo probado de 708 (§4).

* **Global**, una vez por muestra, antes de las voces:
  * fase `p += rate` en el anillo `0280`/`0281`, envuelta con `ph_wrap` a [−1, 1) (como Roland);
  * `r = p_sin_envolver − p` vale 2 en cada vuelta y 0 en el resto;
  * ruido `n = wrap(7,31275·n + w)` (`0282`), con un contador de Weyl `w = wrap(w + 0,618430…)` (`0286`, inmediato
    de 32 bits). Sin `w`, `n` se repetía cada 7194 muestras (13 Hz) [sim]. Con `w`, en 600 000 muestras
    hubo 596 989 valores distintos, en 10 franjas de entre 59 523 y 60 289 [sim];
  * S&H `h += r/2·(n − h)` (`0284`): toma un valor de ruido nuevo en cada vuelta;
  * forma `lfo = a·tri(p) + b·p + c·ph_clamp(256·p) + d·h + e·n`, con `a`–`e` = `C060`–`C064`. La tabla
    `lfo_wave_bits` asigna shape 0–5 a triángulo, sierra +, sierra −, cuadrada, S&H y ruido. [inferido] El orden
    del panel del SYSTEM-1 no está comprobado;
  * productos de la muestra: `LFO_P = lfo·P` (`0288`), `LFO_F = lfo·F` (`028A`), `LFO_M = M·(lfo − 1)` (`028C`).
* **Por voz** (fade `f` en el anillo `V8_PHASE[v] + 4`):
  * `f = min((f + paso)·puerta, 1)`: sube de 0 a 1 de forma lineal desde la puerta y vuelve a 0 al soltar;
  * tono `+= f·LFO_P`, guardado en `V8_PHASE[v] + 6` para el DCO2. El key track de B usa el tono sin LFO;
  * voz `×= 1 + f·LFO_M`, es decir, de 1 − 2M a 1: el trémolo baja desde el nivel pleno;
  * corte `= C05D + f·LFO_F` → `A180+v`. El ARM escribe el cutoff en `C05D` del DSP A, en vez de en `C048` del B.
* **Perillas** (tablas del ARM):
  * rate: 0,05·800^(v/255) Hz, de 0,05 a 40 Hz;
  * delay: 5·(v/255)² s hasta el pleno (0 = inmediato);
  * → pitch: (v/255)² octavas, hasta ±1;
  * → filtro: 4·v/255 octavas;
  * → amplitud: `M` = v/510.

  Rate y delay están en la rejilla de los registros (`on_grid`, múltiplos de 2^−32). Así el parámetro da los mismos
  bits si se usa justo después de cargarlo (empaquetado) o tras un relleno.

Medido en el núcleo C (`make_synth.py`, programas reales, 2026-10-23) [sim]:

| qué | resultado |
|---|---|
| rate 192 / 255 | 7,670 / 40,000 Hz (tabla 7,671 / 40,000) |
| formas | triángulo y sierras de −1 a +1; cuadrada ±1 con una rampa de 1/128 del período; S&H con escalones; ruido ±1 |
| fade, delay 64 / 128 / 255 | llega a 1 en 0,315 / 1,260 / 5,002 s (tabla 0,315 / 1,260 / 5,000); 0 al soltar la nota |
| → filtro al máximo, corte 128 | `A180+0` de −3,729 a +4,270 octavas (base +0,271 ± 4) |
| → pitch al máximo, La 440 | de +3,750 a +5,750 octavas (4,75 ± 1) |
| → amplitud 128 / 255, cuadrada | razón de RMS entre la mitad baja y la alta: 0,502 / 0,062 (esperado 0,498 / 0) |

`--sim-check` del paso 9 con dos paneles de LFO (triángulo y S&H, todas las profundidades activas): núcleo C =
referencia Python, y empaquetado = sin empaquetar, bit a bit.

Instalado sin probar todavía, 2026-10-23: `STEP=9 NAME=C-S1SY9 sh examples/system_1_synth/build.sh` (`RESCOMP=0`: DSP A 693,
B 652 instrucciones) [abierto].

### 7.3 LFO por voz (paso 11)

Cambios respecto a §7.2:
* **El LFO se calcula por voz** (`step_lfo_voice`).
  * La fase está en el bloque de la voz, en `V8_PHASE[v] + 8`. Vuelve a 0 cuando cambia el disparo de la voz:
    `d = (disparo − último)²` y `fase −= fase·d`.
  * El S&H va en `+10` y el último disparo en `+12`.
  * Sólo el ruido (con su contador de Weyl) sigue siendo global.
* **Las profundidades → filtro (`0x164`) y → amplitud (`0x165`) son bipolares**, con 128 = 0:
  * filtro: F = 4·(v − 128)/127 octavas;
  * amplitud: M = 0,5·(v − 128)/127, con la voz `×(1 + f·(M·lfo − |M|))`. `|M|` va en `C05E` del DSP A. Con M < 0
    es el mismo trémolo sobre el LFO invertido.
* **Las copias de puerta y disparo hacia el DSP B** se hacen con las mismas cargas del LFO: ya no se cargan dos
  veces.
* **11 bis** (2026-10-23, después de probarlo en el equipo):
  * → pitch (`0x163`) también bipolar: P = b·|b| octavas, con b = (v − 128)/127 (128 = 0, ±1 octava en los
    extremos, fino cerca del centro);
  * las formas siguen el orden del panel del SYSTEM-1 que dio el usuario: 0 sine, 1 triangle, 2 saw, 3 square,
    4 sample & glide, 5 sample & hold. Los coeficientes son `C060`–`C065` del DSP A;
  * sine = t·(1,5 − 0,5·t²) del triángulo t;
  * sample & glide: `g += k·(h − g)`, con `k = min(1, 4·rate)` (constante de tiempo = período / 8) y estado en
    `0240 + 2v`;
  * el DSP A pasa a 640 instrucciones.

Medido en el núcleo C (2026-10-23) [sim]:

| qué | resultado |
|---|---|
| formas (rate 192, → pitch al máximo) | sine: error máximo 0,020 frente a sen; triangle, saw, square de −1 a +1; S&G suave (salto máximo entre muestras 0,0008); S&H en escalones |
| → pitch 0 / 64 / 128 / 192 / 255 | ±1 / ±0,25 / 0 / ±0,25 / ±1 octavas; 0 y 64 con el signo invertido |
| fase por voz | voz 0 en t = 0 y voz 1 en t = 3000 empiezan cada una en 0,00016 (un incremento); voz 0 redisparada en t = 4000: de 0,639 a 0,00016 |
| → filtro 0 / 64 / 128 / 192 / 255, corte 128 | ±3,84 / ±1,93 / 0 / ±1,93 / ±3,84 octavas; 0 y 64 con el signo invertido |
| → amplitud 0 / 64 / 128 / 192 / 255, cuadrada | RMS mitad alta / baja: 0 / 0,0706, 0,035 / 0,0706, 0,0706 / 0,0706, 0,0706 / 0,035, 0,0706 / 0,0009 |

### 7.4 Efectos propios en el pote CRUSHER (pasos 14 y 15) [abierto]

Hallazgos, 2026-10-23:
* **CRUSHER (`0x18C`) y AMP TONE (`0x18B`) no los procesa el OS.** En los tres PLUG-OUTs oficiales del SYSTEM-1 los
  implementa el principal del maestro B: la máscara `bic` del crusher, con parámetros `C01C`–`C037` escritos por su
  modelo. Los manuales los llaman EFFECT DEPTH y AMP TONE ([../system-1/FX_CHAIN.md](../system-1/FX_CHAIN.md) §7).
  El botón LPF type sólo manda el CC 115 (`B4 73`), sin parámetro para el modelo.
* **Los cuelgues del paso 14 venían de formas de instrucción nuevas, no del banco 0.** Pasaba lo mismo en el maestro
  B y en el esclavo B, y 14C demostró que no eran las escrituras de parámetros. Desde el paso 14 el planificador sólo
  fusiona grupos de ops que ya corrieron en el equipo (`asm/proven_forms.txt`,
  [MICROCODE_OPTIMIZATION.md](MICROCODE_OPTIMIZATION.md) §5). Con eso 14F y 15 no cuelgan. Qué grupo concreto colgaba:
  [abierto].
* **AMP TONE** (14E: inclinación con un paso bajo de 1,2 kHz) cambia el nivel más que el brillo. Hay que rediseñarlo
  como un shelving de agudos o con un corte más alto.
* **Chorus 15:** las tomas son 0 en el equipo (captura USB con el pote al máximo: sierra pura, L = R), aunque la
  línea y `ldidx` funcionan (15X). La diferencia entre 15 y 15X es el índice calculado: LFO → k → fracción por AND
  de bits → `hset h6, r5` con el resultado. Hipótesis: el AND o el `hset` de un valor calculado no se comportan en el
  equipo como en el emulador. 15Y prueba la fracción sin AND (`ph_mod2(2k)`).

Siguiente prueba propuesta: el índice calculado con un LFO pero sin fracción (k entero, sin interpolar). Así se
separa el `hset` de un registro calculado del resto. Medir siempre con captura USB:

```sh
arecord -q -D hw:SYSTEM1,0 -f S32_LE -r 96000 -c 2 -t raw -d 8 ch.raw    # mientras se toca una nota grave
```

### 7.5 Triángulo con wavefolding en COLOR (`FOLD=1`) [medido]

Estado: funciona en el equipo (C-S1SY13F, 2026-10-23, a oído). Install: `python3 tools/plugout_tool.py install
plugins/C-S1SY13F --slot 1 --force --no-patches -y`.

Variante del paso 13 (no incluye los pasos 14 y 15). Build: `STEP=13 FOLD=1 NAME=C-S1SY13F sh
examples/system_1_synth/build.sh`. La posición del selector que da el triángulo (la 3.ª, `STEP_WAVES[2]`) da ahora
el triángulo plegado; el pote COLOR del mismo oscilador sube el plegado. Las otras ondas no cambian.

* Ganancia: g = 1 + (`FOLD_MAX` − 1) · color / 255, con `FOLD_MAX` = 5 por defecto (variable de build). El ARM
  escribe g/2 en `C056` (OSC1) / `C057` (OSC2) del DSP A (`fold_bits`).
* Plegado: `ph_tri(R) = 1 − 2|wrap(−R − 1,5)|` ([DSP_ISA.md](DSP_ISA.md)) es la reflexión en ±1 de 2R, es decir,
  un plegador con cualquier número de pliegues. Por eso salida = `ph_tri(g · tri / 2)`, y con color 0 (g/2 = 0,5) da
  el triángulo sin cambios. Coste: 1 carga + 2 ops por oscilador (+32 instrucciones en el DSP A).

```dsp
ld r4, [c056]          ; g / 2
ph_tri r3, r3, -r1     ; triángulo de la fase
mul r3, r3, r4
ld r4, [c052]          ; coeficiente del triángulo (selector)
ph_tri r3, r3, -r3     ; plegado
mad r2, r3, r4
```

* La planificación de `FOLD=1` fusiona sólo los grupos de ops de `asm/proven_forms.txt` (como el paso 14), porque
  `mul` + `ph_tri` en una sola instrucción nunca se probó en el equipo.
* Medido en el simulador (OSC1 solo, La 220, `FOLD_MAX` 5). Armónicos en dB respecto al fundamental:

  | color | 3.º | 5.º | 7.º | centroide (armónicos) | pico |
  |---|---|---|---|---|---|
  | 0 | −19 | −28 | −34 | 1,67 (triángulo: 1/n²) | 0,082 |
  | 128 | +43 | −14 | −17 | 3,96 | 0,081 |
  | 255 | +7 | +143 | +1 | 5,96 | 0,079 |

  El nivel de pico no cambia. El fundamental baja cuando el pliegue lo cancela (por eso los armónicos salen por
  encima de 0 dB). Sin antialiasing: con g alto y notas agudas aparece aliasing.
* Pendiente: la fuente de modulación del color (`0x169`/`0x16E`: LFO, envolventes) todavía no se usa; sólo manual.

## 8. Interfaz con el panel

Ids capturados con PARAMLOG (2026-10-23; `python3 examples/system_1/paramlog.py -t 90 --raw` con una build
`PARAMLOG=1`, **con el PLUG-OUT seleccionado**: si no, sólo llegan los ecos de CC):

| id | control | valores | usado en |
|---|---|---|---|
| `0x160` | LFO shape (CC 35) | 0–5 | — |
| `0x161` | LFO delay / fade time (CC 27) | 0–255 | — |
| `0x162` | LFO rate (CC 29) | 0–255 | — |
| `0x163` | LFO → pitch (CC 26) | 0–255 | — |
| `0x164` | LFO → filtro (CC 28) | 0–255 | — |
| `0x165` | LFO → amplitud (CC 30) | 0–255 | — |
| `0x167` / `0x16C` | OSC1 / OSC2 wave | 0–5 | paso 7 |
| `0x168` / `0x16D` | OSC1 / OSC2 color (CC 50 / 55) | 0–255 | `FOLD=1`: fold del triángulo |
| `0x169` / `0x16E` | OSC1 / OSC2 color mod source (CC 60 / 63), 2026-10-23 | 0–5 = manual, LFO, pitch env, filter env, amp env, sub osc (igual en ambos; CC 0, 25, 51, 76, 102, 127) | — |
| `0x16A` / `0x16F` | OSC1 / OSC2 range | 0–5 (64'…2') | paso 5 |
| `0x170` | OSC2 fine tune | 0–255, centro 128 | paso 5 |
| `0x174` / `0x175` | OSC1 / OSC2 mix | 0–255 | paso 2 |
| `0x176` / `0x177` | sub mix / noise level (CC 18 / 19) | 0–255 | — |
| `0x17A` | pitch env amount (CC 22) | 0–255 | — |
| `0x17B` / `0x17C` | pitch env attack / decay (CC 23 / 24) | 0–255 | — |
| `0x17D` / `0x17F` | cutoff / resonancia | 0–255 | paso 6 |
| `0x181` / `0x186` | filter env amount (bipolar, 128 = 0) / key track | 0–255 | pasos 8 / 6 |
| `0x182`–`0x185` | ADSR del filtro | 0–255 | paso 8 |
| `0x187`–`0x18A` | ADSR del VCA | 0–255 | paso 4 |
| — | HPF cutoff | no llegó nada (ni parámetro ni CC) [abierto] | — |
| `0x18D` / `0x18E` / `0x18F` | REVERB amount (CC 91) / DELAY amount (CC 94) / DELAY time (CC 13), 2026-10-23 | 0–255 | — ([../system-1/FX_CHAIN.md](../system-1/FX_CHAIN.md)) |
| `0x18B` | sin identificar: 128–145, con CC 69 (64–72) detrás; llegó dos veces durante la captura del LFO, entre otros controles [abierto] | — |

Los ids del LFO se capturaron con C-S1SYRL (`RESCOMP=1 PARAMLOG=1`, 2026-10-23). Orden de la captura: rate, → filtro,
→ amplitud, fade time, → pitch, shape. Cada control se recorrió de 0 a 255 (shape de 0 a 5). Log:
`paramlog.py -t 120 --raw`.
Segunda captura, mismo día: shape, → pitch, → filtro, delay time → `0x160`, `0x163`, `0x164`, `0x161`. Así que
"fade time" y "delay time" son el mismo control, `0x161`. `0x18B` no volvió a llegar.

## 9. Mejoras anotadas

* **Curvas y tiempos de las envolventes parametrizables: prioridad.** Al oído las envolventes de C-S1SY8 se notan
  **algo toscas** (2026-10-23). Hoy, en `make_synth.py` (`env_time`, `attack_rate`, `decay_rate`, `step_adsr`):
  * cada fader va de 1 ms a 10 s con una sola ley exponencial: `0.001 · 10000^(v/255)`;
  * el ataque es exponencial hacia 1,25 y se corta en 1;
  * la caída y el release son exponenciales y llegan a 1/1000 en el tiempo del fader;
  * el sustain es lineal, `v/255`.

  Qué se puede hacer configurable, casi todo en el ARM:
  * **Rango y ley de cada fader por separado** (A, D, R). Por ejemplo, un ataque de 0,5 ms a 5 s y un release de
    2 ms a 15 s. La ley puede ser exponencial, cuadrática o por tramos, como en los sintes reales. Son tablas
    distintas generadas por `make_synth.py` con opciones de build.
  * **Curva del ataque**: el objetivo (1,25 hoy) como parámetro `C0xx`. Cerca de 1 da una curva más exponencial,
    tipo RC; más alto, una más lineal. Coste en el DSP: cambiar `#1.25` por una carga.
  * **Curva de caída y release**: exponencial (hoy) o lineal, con un paso constante por muestra y un tope en 0 o
    en S. Una curva intermedia mezcla las dos con un parámetro. Coste: unas 3–4 instrucciones por envolvente.
  * **Suavizado del parámetro**: hoy un cambio de fader salta de golpe en el DSP. Podría seguir una rampa, con
    `FCE4`, el servicio con rampa que Roland usa y que no está mapeado, o con un paso de suavizado en el DSP.
  * **Perfiles**: conjuntos de tablas con nombre para imitar distintos sintes (rápido / medio / lento, tipo
    SH-101, tipo JX-3P), elegidos con una opción de build o más adelante con un control del panel.

* **Simulador más rápido**: el actual (`re/dspisa.py`) tarda minutos para unos miles de muestras de 8 voces. Hasta
  mejorarlo, las pruebas se hacen directamente en el equipo.
* **Buscar el presupuesto exacto del SYSTEM-1** (entre 682 y 826) con `PAD=n`, si hace falta apurar.

### 9.1 Más mejoras posibles (lista para la siguiente sesión)

Por orden aproximado de coste. Cada una se hace como un paso nuevo, con una instalación.

* **Controles ya mapeados y sin usar** (§8):
  * **Sub oscilador** (`0x176`): onda cuadrada una octava abajo, sacada de la fase del DCO1 con `ph_clamp`.
    Unas 4 instrucciones por voz en el DSP A.
  * **Ruido** (`0x177`): un generador LCG global en el DSP B, que se suma antes del filtro. Unas 5 instrucciones más
    1 por voz.
  * **Color OSC1/OSC2** (`0x168`/`0x16D`): ancho de pulso de la cuadrada (umbral `ph_clamp(256·(fase − t))`). Es
    un parámetro `C0xx` más por oscilador y 1 instrucción.
  * **Pitch env** (`0x17A`–`0x17C`): una envolvente AD por voz que se suma al tono. El DSP A no tiene sitio para
    ella (587 de 682), así que habría que calcularla en el DSP B y pasarla por `A1xx`, con una muestra de retraso.
  * **LFO**: un LFO global en el DSP B. Ids capturados (§8): rate `0x162`, → filtro `0x164`, → amplitud `0x165`,
    delay / fade time `0x161`, → pitch `0x163`, shape `0x160` (0–5).
* **Botón de tipo de filtro del panel → 2 polos / 4 polos** (pendiente, pedido 2026-10-23):
  * Primero capturar su id con PARAMLOG, con el PLUG-OUT seleccionado. Puede ser uno de los selectores de patch de
    la SYNINFO (`0x171`, `0x172`, `0x178`, `0x179`, `0x17E`…), que todavía no se han visto.
  * En el DSP B, la salida del filtro se toma de `y2` (−12 dB/oct) o de `y4` (−24 dB/oct). Hay dos formas:
    * sin saltos: mezclar `y2·a + y4·(1 − a)` con `a` como parámetro `C0xx` de 0 o 1, por unas 3 instrucciones
      por voz;
    * más barato: guardar en la palabra que se suma a la salida la etapa elegida.
  * La realimentación de resonancia debería salir de la misma etapa, con k máxima distinta: un filtro de 2 polos
    con realimentación de `y2` oscila con k ≈ 2. Hay que ajustar `RES_MAX` y la compensación por modo.
  * Coste: un parámetro `C0xx` más en el DSP B, de 0 a 6 instrucciones por voz; el DSP B tiene margen (660).
* **HPF**: en la captura no llegó nada. Hay que repetirla y, si sigue sin llegar, mirar la tabla de controles
  `0xC0`–`0xE5` de la SYNINFO y cómo la configura el SH-2.
* **Velocidad**: hoy la puerta vale 0 o 1. La velocidad (`0x120+v`) podría escalar el VCA y/o la cantidad de la
  envolvente del filtro.
* **Presupuesto**: buscar el límite exacto del DSP A (entre 682 y 826) con `PAD=n`. Para ganar margen:
  * reescribir la sierra de C-S1V8 con `ph_wrap`, como Roland, que ahorra unas 10 instrucciones por oscilador;
  * mover la copia de puerta, disparo y tono del DSP A al ARM con `FD6C sel 1` (`A1xx`), como el SH-2.
* **Afinación**: la sierra de C-S1V8 lleva la constante calibrada en el SYSTEM-8, que podría estar unos 54 cents
  baja según `make_s1.py`. Conviene medir con un afinador o con captura USB.
* **Simulador**: hacerlo rápido para poder volver a validar antes de instalar. Se puede usar el núcleo C
  `emu/cchain.py` en lugar de `re/dspisa.py`.
* **Envolventes**: tiempos y curvas configurables (arriba).

### 9.2 Estado al cerrar la sesión del 2026-10-23

* Instalado en el SYSTEM-1, slot 1: **C-S1SY13F** (paso 13 + `FOLD=1`), confirmado en el equipo.
* Lo reconstruye `STEP=13 FOLD=1 NAME=C-S1SY13F sh examples/system_1_synth/build.sh` (opciones: `--help`; sin
  variables, `build.sh` sigue haciendo `STEP=8`).
* El USB del SYSTEM-1 se desconectó solo tres veces durante la sesión. Antes de cada instalación conviene comprobar
  `amidi -l`.
