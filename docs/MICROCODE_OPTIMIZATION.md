# Microcódigo optimizado: reglas obligatorias (SYSTEM-1 y SYSTEM-8)

Especificación para todo microcódigo DSP propio, en los dos equipos (el ISA es el mismo). Cada instrucción cuesta un
ciclo del presupuesto de la muestra:
* esclavo del SYSTEM-1: máximo probado 708, cuelga a 826 ([examples_system_1_synth_README.md](examples_system_1_synth_README.md) §4);
* SYSTEM-8: unas 925–945 por DSP ([DSP_ISA_SPEC.md](DSP_ISA_SPEC.md)).

Una instrucción desperdiciada es una voz, una forma de onda o una modulación menos. Cómo lo hace Roland:
[ROLAND_OSCILLATORS.md](ROLAND_OSCILLATORS.md).

Estado: 2026-10-23.

## 1. Reglas (imperativas)

1. **Operaciones de fase en una sola op.** Envolver una fase es `ph_wrap rD, r3, rY` (forma de Roland, X = r3). El
   triángulo es `ph_tri rD, r3, -rY` y la cuadrada / el recorte a ±1 es `ph_clamp rD, r3, -rY`. Están prohibidas
   las cadenas `mul ×256` / `min` / `max` que hacen lo mismo en 11 instrucciones.
2. **Transferencias empaquetadas.** Los `ld`/`st` van en la instrucción de una op (`pack()`, como Roland), dentro de
   los límites por instrucción (`chain.access_violations`). Un `mov rN, rN` de relleno sólo se escribe en el
   código sin empaquetar y desaparece al empaquetar.
3. **Operaciones independientes fusionadas.** Dos o tres ops que no dependen entre sí van en una instrucción
   (`op1 | op2`, A+B, A+A+B). Todas leen antes de escribir. `pack()` con `FUSE=1` lo hace automáticamente.
4. **Sin cargas repetidas.** Un valor que ya está en un registro o en memoria de la misma muestra no se vuelve a
   cargar (por ejemplo, la puerta: una carga por voz y por muestra, que también sirve para la copia hacia el DSP B).
5. **Validación bit a bit en el emulador.** Toda optimización se compara con la versión sin optimizar en el núcleo
   C (`--sim-check`: muestras idénticas) antes de instalar.
6. **Parámetros en la rejilla de los registros.** Un parámetro que se usa justo después de cargarlo tiene que estar
   en la rejilla de cuantización (`on_grid()` en `make_synth.py`). Si no, la versión empaquetada y la sin empaquetar
   difieren en 1 LSB.
7. **Ningún build sin `check_optimized`.** Los avisos se corrigen o se justifican en el README del ejemplo.

## 2. `tools/check_optimized.py`

```sh
python3 tools/check_optimized.py examples/system_1_synth/s1m0.s     # .s, .hex o .txt (palabras)
python3 tools/check_optimized.py --strict FILE...                    # código de salida 1 si hay avisos
python3 tools/check_optimized.py -q FILE...                          # una línea por archivo
```

| aviso | qué detecta | corrección |
|---|---|---|
| `manual-wrap` | pares `mul rX, rX, #256` + `min rX, rX, …` repetidos | regla 1 |
| `filler` | `mov rN, rN` solo en una instrucción | regla 2 |
| `transfer-only` | instrucción sólo con `ld`/`st` | regla 2: mover a una op vecina |
| `fusable` | dos instrucciones seguidas de una op, independientes y codificables como una (lo comprueba el ensamblador) | regla 3 |

Lo ejecutan, después de generar los programas:

| equipo | build | programas que comprueba |
|---|---|---|
| SYSTEM-1 | `examples/system_1/build.sh` | `s1m0.s`, `s1m1.s`, `s1mm1.s` |
| SYSTEM-1 | `examples/system_1_synth/build.sh` | ídem |
| SYSTEM-8 | `examples/from_scratch/gate/build.sh` | `dsp2_osc.hex`, `dsp4_gate.hex`, `dco2_null.hex` |
| SYSTEM-8 | `examples/from_scratch/mini/build.sh` | `poly.hex` |
| SYSTEM-8 | `examples/wt_morph/build.sh`, `examples/wt_pwm/build.sh` | `prog/*.txt`, `wt2/dco2_w*.txt` |
| SYSTEM-8 | `examples/wt_live/build.sh` | `cal_d2.txt`, `cal_d3.txt` |

`examples/from_scratch/engine/build.sh` no se modifica (regla del proyecto). Sus programas propios llegan por
`OWN_PROGS` / `MC_TEST` desde los scripts de arriba, que son los que los comprueban. `examples/system_1_jx` sólo
reubica microcódigo de Roland y no se comprueba.

Estado al crear la herramienta (2026-10-23):

| programa | instrucciones | avisos |
|---|---|---|
| C-S1SY paso 8, DSP A (instalado) | 587 | 16 manual-wrap, 25 transfer-only, 49 fusable |
| C-S1SY paso 11, DSP A (`FUSE=1`) | 576 | 0 manual-wrap, 49 transfer-only, 25 fusable (de 74 sin `FUSE`) |
| C-S1SY paso 11, DSP B (`FUSE=1`) | 603 | 62 transfer-only (las 6 escrituras a 0 de `6008`–`600D` y las cargas de cada voz) |
| C-S1V4 `s1m0` | 269 | 8 manual-wrap, 16 filler, 33 transfer-only, 9 fusable |
| SYSTEM-8 `poly.hex` (C-MINI2) | 210 | 5 manual-wrap, 9 filler, 33 transfer-only, 5 fusable |
| SYSTEM-8 `dsp2_osc.hex` | 35 | 1 manual-wrap, 2 filler, 4 transfer-only, 2 fusable |
| SYSTEM-8 wavetables `dco1_w0.txt` | 344 | 2 manual-wrap, 10 transfer-only, 7 fusable |

[abierto] `pack()` sólo fusiona una op con la instrucción inmediatamente anterior. Quedan 25 pares en el DSP A del
paso 11, y las transferencias sueltas no se mueven hacia la op siguiente.

[abierto] Los programas del SYSTEM-8 (`make_v3`/`make_v4`, `make_chain`, `make_poly`, `wav2dco`) y C-S1V4 todavía
tienen el wrap manual: hay que pasarlos a `ph_wrap`, empaquetarlos y fusionarlos, validar en el emulador y probar
en el equipo.

## 3. Ejemplos (C-S1SY, `make_synth.py`)

Sierra con `ph_wrap` (paso 10): fase += incremento y envoltura en dos ops, en vez de 13.

```dsp
addxy r1, r3, r1
ph_wrap r1, r3, r1
st r1, [0300]
```

Transferencias y ops fusionadas (paso 11, salida de `pack()` con `FUSE=1`): tres ops en una instrucción, las
cargas que usa la instrucción siguiente.

```dsp
mad r7, r1, r4 | mul r3, r1, #256.0 | ld r4, [c062]
ph_clamp r3, r3, -r3
mad r7, r3, r4 | ph_tri r3, r3, -r1 | ld r4, [c060]
```

En la última instrucción, `mad` lee el r3 anterior (la cuadrada) y `ph_tri` escribe el triángulo en r3. Todas
las unidades leen antes de escribir.

## 4. Coste medido

| paso C-S1SY | DSP A (instr.) | DSP B (instr.) | qué cambia |
|---|---|---|---|
| 9 (LFO global, instalado) | 693 | 652 | — |
| 10 (sierra con `ph_wrap`) | 517 | 652 | −176: 11 instrucciones × 16 osciladores |
| 11 (LFO por voz, sin `FUSE`) | 649 | 652 | +132 por el LFO por voz |
| 11 (con `FUSE`, por defecto) | 576 | 603 | −73 / −49 por la fusión |

`--sim-check` del paso 11: núcleo C = referencia Python, y fusionado = sin empaquetar, bit a bit (4 paneles, entre
ellos dos de LFO) [sim, 2026-10-23].
