# C-S1SY: sintetizador sustractivo de 8 voces para el SYSTEM-1 (`examples/system_1_synth/`)

Estado: 2026-10-23. **Pasos 1–8 funcionan en el equipo** [medido, a oído]:
* 8 voces, cada una con DCO1 + DCO2;
* forma de onda, nivel y range por oscilador, más fine tune en OSC2;
* ADSR de VCA;
* filtro paso bajo de 4 polos con cutoff, resonancia y key track;
* ADSR de filtro con cantidad bipolar.

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
python3 examples/system_1_synth/make_synth.py --sim         # La 440, 8 notas, silencio: unos 2 s
SIM=py python3 examples/system_1_synth/make_synth.py --sim  # lo mismo con el simulador de referencia (minutos)
python3 examples/system_1_synth/make_synth.py --sim-check   # núcleo C contra la referencia, muestra a muestra
```

`--sim` corre en el núcleo C del emulador (`examples/from_scratch/emu/cchain.py`, programas traducidos a código
nativo): el esclavo A es el DSP 0 de un `cchain.Slave`; el esclavo B y el master B son los DSP 0 y 1 de otro (las
salidas `6006`/`6007` de B llegan al master por el IOMD compartido en la misma muestra). El banco 5 se copia
A → B → A alrededor de cada muestra, el mismo modelo de un banco 5 por pareja que la referencia Python
(`simulate_py`, `re/dspisa.py`). Rendimiento: 16 400 muestras en 2,4 s, frente a 3 440 muestras en 3 min 24 s con
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

## 4. Presupuesto de tiempo del DSP A del esclavo [medido]

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
  k·(g/(2 − g))⁴ ≥ 1, es decir, si g ≳ 0,83. Con 0,95 quedaba oscilando después de soltar la nota. Por encima de
  0,7 el filtro puede autooscilar con el corte alto: se ajusta a oído con `GMAX=0.9 sh build.sh`.
* Ganancia de entrada `(1 + k/2)/2`: compensa la pérdida de graves con resonancia y deja margen a la saturación. La
  mezcla final multiplica por 2.

## 8. Interfaz con el panel

Ids capturados con PARAMLOG (2026-10-23; `python3 examples/system_1/paramlog.py -t 90 --raw` con una build
`PARAMLOG=1`, **con el PLUG-OUT seleccionado**: si no, sólo llegan los ecos de CC):

| id | control | valores | usado en |
|---|---|---|---|
| `0x162` | LFO rate | 0–255 | — |
| `0x167` / `0x16C` | OSC1 / OSC2 wave | 0–5 | paso 7 |
| `0x168` / `0x16D` | OSC1 / OSC2 color (CC 50 / 55) | 0–255 | — |
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
  * **LFO** (`0x162`): un LFO global en el DSP B hacia el tono y/o el corte. Faltan por capturar los destinos y la
    forma de onda.
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

* Instalado en el SYSTEM-1: **C-S1SY8** (`STEP=8`, empaquetado, `GMAX` 0,7, cantidad de la envolvente del filtro
  bipolar). El paso 8 está confirmado en el equipo; la cantidad bipolar está instalada pero sin confirmar.
* `sh examples/system_1_synth/build.sh` reconstruye exactamente eso (opciones: `--help`).
* El USB del SYSTEM-1 se desconectó solo tres veces durante la sesión. Antes de cada instalación conviene comprobar
  `amidi -l`.
