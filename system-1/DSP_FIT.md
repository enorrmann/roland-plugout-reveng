# ¿Cabe el microcódigo de un PLUG-OUT del SYSTEM-8 en el SYSTEM-1? (`dsp_fit.py`)

Estado: 2026-10-22. Herramienta de estudio, sin hardware: mide el tiempo de DSP y el tráfico entre DSP de los
programas de Roland del JX-3P y busca un reparto en los cuatro DSP del SYSTEM-1. Responde a la pregunta "¿se puede
instalar el JX-3P con su microcódigo en el SYSTEM-1, a 4 voces?".

Leyenda: **[medido]** en el equipo · **[emulador]** medido en `emu/cchain.py` · **[código]** · **[inferido]** · **[abierto]**.

## 1. Uso

```sh
python3 system-1/dsp_fit.py                          # JX-3P, 4 voces (un slave del SYSTEM-8), DCO SAW/SAW, sin efecto
python3 system-1/dsp_fit.py --dco1 sqr --dco2 noise --fx ch
python3 system-1/dsp_fit.py --voices 2               # sólo la cadena de las voces 0/1 (DSP0 -> DSP2 -> DSP4)
python3 system-1/dsp_fit.py --budget 945 --reserve-ma 0 --samples 4000
```

Necesita `examples/from_scratch/engine/build/programs.h` (lo crea `examples/from_scratch/engine/build.sh` a partir de
`plugins/JX-3P`). El microcódigo se lee de esa copia local en tiempo de ejecución; no se copia nada al repo
(alcance de la regla de [VOICE_ALLOCATION.md](../docs/VOICE_ALLOCATION.md) §7). Tarda unos 10 s con 1500 muestras.
Código de salida 0 si cabe, 1 si no.

| opción | por defecto | qué es |
|---|---|---|
| `--dco1` | `saw` | programa de la ranura DCO1 (`saw`, `sqr`) en DSP2 y DSP3 |
| `--dco2` | `saw` | ranura DCO2 (`saw`, `sqr`, `noise`, `none` = sin DCO2) |
| `--fx` | ninguno | ranura de efecto del DSP4 (`od`, `ds`, `ch`, `fz`, `cr`, `ph`) |
| `--voices` | 4 | 4 = los seis DSP de un slave; 2 = DSP0, DSP2, DSP4 |
| `--budget` | 925 | instrucciones por muestra y DSP. 925–945 está **[medido] en el SYSTEM-8**; en el SYSTEM-1 **[abierto]** |
| `--reserve-ma` | 40 | instrucciones del DSP A del maestro que se lleva el programa de salida del OS en `0x82` **[abierto]**, estimado |
| `--samples`, `--seed` | 2000, 1 | muestras emuladas y semilla de las entradas aleatorias |

## 2. Qué mide

1. **Tiempo por programa** [emulador]: cada programa se ejecuta en el emulador C con entradas aleatorias en el banco 5
   y en los parámetros (`C000`–`C03F`), que cambian cada 250 muestras para que los saltos vayan por los dos lados. Se
   dan las instrucciones ejecutadas por muestra (mínimo, media, máximo). El coste de una ranura es la diferencia entre
   el DSP con y sin ella (las ranuras corren después del principal y leen lo que éste escribe). También se dan bytes
   e instrucciones estáticas (`emu/listing.py`).
2. **Enlaces entre los DSP del SYSTEM-8** [código, desde los listados]: palabras por muestra de
   * MXM hacia delante: `Z 84nn` del DSP n leídas como `M 80nn` (n par) u `O 82nn` (n impar) por el DSP n+2;
   * MXM hacia atrás: `M`/`O` escritas por el DSP n y leídas como `S 86nn` por el DSP n−2;
   * ventana de la pareja `A400`–`A7FF` (DSP0↔1, 2↔3, 4↔5);
   * salida audible `6020`/`6021`.
3. **Reparto en el SYSTEM-1**: prueba todas las asignaciones de los programas principales a los cuatro DSP (esclavo
   A/B, maestro A/B); cada ranura (DCO, efecto) va a uno de los dos DSP del **mismo chip** que su principal, porque se
   hablan por el banco 1 (que A y B comparten [inferido], `VOICE_ALLOCATION.md` §3.1). Condiciones:
   * cada DSP ≤ `--budget` (el maestro A con `--reserve-ma` ya ocupado);
   * palabras esclavo → maestro ≤ 8 (IOMD `6006`–`600D`; la salida de audio cuenta aquí si sale del esclavo);
   * palabras maestro → esclavo ≤ 20 (`600E`–`6021`).
   Muestra el mejor reparto (primero los que cumplen el bus, luego el de menor pico) y cuántos enlaces MXM habría que
   reescribir: en los programas del SYSTEM-1 leídos no aparece el banco 4 [código], así que cada enlace MXM tendría
   que pasar a otro mecanismo (buzón `A1xx` dentro del chip, IOMD entre chips) parcheando direcciones en el
   microcódigo de Roland.

Lo que **no** mide: si los DSP del SYSTEM-1 tienen el mismo presupuesto, la latencia de `A1xx` y del IOMD en el
SYSTEM-1, ni el trabajo del ARM (el motor del JX-3P es Thumb con VFP y servicios del SYSTEM-8; en el SYSTEM-1 no hay
FPU y la interfaz es otra, `docs/examples_system_1_README.md` §3).

## 3. Resultados (2026-10-22) [emulador]

JX-3P 1.01, DCO SAW/SAW, sin efecto, 1500 muestras:

| DSP del S8 | programa | bytes | estáticas | ejecutadas (máx.) |
|---|---|---|---|---|
| DSP0 | MAIN0 (LFO/envolventes voces 0,1) | 5066 | 528 | 516 (media 453) |
| DSP1 | MAIN1 (voces 2,3) | 5018 | 526 | 514 (media 450) |
| DSP2 | MAIN2 + DCO1_SAW + DCO2_SAW | 2068 + 2×4104 | 189 + 2×325 | 839 |
| DSP3 | MAIN3 + DCO1_SAW + DCO2_SAW | 2018 + 2×4104 | 184 + 2×325 | 834 |
| DSP4 | MAIN4 (filtro/VCA voces 0,1, suma, salida) | 5174 | 532 | 532 |
| DSP5 | MAIN5 (voces 2,3) | 5016 | 513 | 513 |
| | **total, 4 voces** | | | **3748** |

Enlaces: DSP0→2 MXM 19, DSP1→3 MXM 17, DSP2→4 MXM 22, DSP3→5 MXM 20, DSP2→0 y DSP4→2 MXM S 3 cada uno, DSP0→1
`A4xx` 7, DSP4↔5 `A4xx` 4 + 4, salida 2.

**4 voces: no cabe.** 3748 instrucciones por muestra contra 3660 de los cuatro DSP (4 × 925 − 40), y aunque cupiera
la suma, los programas no se reparten: el mejor reparto deja dos DSP en ≈ 1025. Con `--budget 945 --reserve-ma 0`
tampoco. No depende del presupuesto del SYSTEM-1 salvo que fuera ≥ ≈ 1030.

**2 voces: cabe en el papel** (`--voices 2`, 1887 instrucciones):

| DSP del S1 | carga | programas |
|---|---|---|
| esclavo A | 721 | MAIN2, MAIN4 |
| esclavo B | 650 | DCO1_SAW, DCO2_SAW (de MAIN2, por el banco 1 compartido [inferido]) |
| maestro A | 516 + OS | MAIN0 |
| maestro B | 0 | (queda el programa de salida propio del PLUG-OUT) |

Bus: esclavo → maestro 5 de 8 (3 de `S` hacia MAIN0 + 2 de audio), maestro → esclavo 19 de 20 (la modulación de
MAIN0 hacia MAIN2). Habría que reescribir 4 enlaces MXM en el microcódigo de Roland y portar el motor ARM del JX-3P
(o escribir uno que dé al microcódigo las mismas entradas del banco 5). [abierto]: nada de esto está probado.

**Un DCO, 4 voces** (`--dco2 none`): 3098 instrucciones, cabe (maestro: MAIN2 + MAIN4 / MAIN0 + DCO1; esclavo:
MAIN3 + MAIN5 / MAIN1 + DCO1). Hecho como C-S1JX: [../docs/examples_system_1_jx_README.md](../docs/examples_system_1_jx_README.md).

Variantes por medir con la herramienta: `--dco1 sqr` (357 instrucciones por ranura), `--fx ch` (312 en DSP4).

## 4. Comparación: ¿es más fácil dar polifonía al SH-101, SH-2 o PROMARS? (`s1_progs.py`)

```sh
python3 system-1/s1_progs.py                         # los tres (work/unpack/<NOMBRE>_S1_Code1)
python3 system-1/s1_progs.py work/unpack/SH-2_S1_Code1 --listing
```

Para cada programa de los sets de la región 0 (maestro) y la región 1 (esclavo): dirección, instrucciones estáticas y
ejecutadas por muestra en el emulador, palabras IOMD que lee y escribe y palabras del banco 5 que escribe. Los tres
PLUG-OUTs son nativos del SYSTEM-1 (su código ARM ya corre allí, sin FPU), pero son **monofónicos**.

Medido 2026-10-22 [emulador; ningún programa tiene saltos, así que estático = ejecutado]:

| | esclavo A | esclavo B | maestro A (modulación) | maestro B (mezcla) | IOMD maestro → esclavo |
|---|---|---|---|---|---|
| SH-101 | 575 | 12 | 410 | 436 | 20 (`600E`–`6021`, todas) |
| SH-2 | 108 + 353 (`0x1900`) + 245 (`0x3100`) = 706 | 62 | 367 | 406 | 10 |
| PROMARS | 373 + 344 (`0x1900`) = 717 | 4 + 69 (`0x1900`) | 426 | 313 | 6 + 1 (`6010` desde el maestro B) |

Hallazgo de paso: el esclavo también escribe `6026`–`602D` (SH-2, PROMARS) y el maestro A las lee, así que el bus
esclavo → maestro tiene al menos 16 palabras (`6006`–`600D` y `6026`–`602D`), no 8 [código].

**Estimación de voces** [inferido], suponiendo lo peor: todo el programa del esclavo A y la modulación del maestro A
son por voz (se duplican), y el esclavo B y el maestro B son globales. Presupuesto 925 por DSP, 40 del OS en el
maestro A.

| | 2 voces | 4 voces | límite |
|---|---|---|---|
| SH-101 | esclavo A 575, B 12 + 575; maestro A 2 × 410 + 40 = 860; **IOMD 40 de 20** | no | el bus: su modulación ya llena las 20 palabras hacia el esclavo |
| SH-2 | esclavo A 706, B 62 + 706 = 768; maestro A 774; IOMD 20 de 20 | no | tiempo del esclavo (4 × 706 > 1850) |
| PROMARS | esclavo A 717, B 73 + 717 = 790; maestro A 892; IOMD 14 de 20 | no | tiempo del esclavo; el maestro A queda justo |
| JX-3P con un DCO (§3, `--dco2 none`) | — | **cabe en tiempo**: 3098 de 3660, bus 4/8 y 11/20 | trabajo: reubicar seis programas (dos por DSP), reescribir 6 enlaces MXM, portar el motor ARM |

Si parte del programa del esclavo fuera global (por ejemplo el principal del PROMARS, 373), el PROMARS podría
llegar a 3 voces (A: 717; B: 73 + 2 × (344 + 69)… ≈ 900) [abierto: no se sabe qué palabras son por voz].

Qué hace falta para 2 voces en los tres: un asignador poly (propio, cambiando el puntero `SYNINFO+0x2C` de la
región 0), una segunda copia de los programas de voz con su estado reubicado (banco 0/1 y palabras por voz del banco
5) y enganches en el código ARM de Roland (maestro y esclavo) para que escriba nota, puerta y modulación de la
segunda voz en las direcciones de la copia. Ninguno está hecho.
