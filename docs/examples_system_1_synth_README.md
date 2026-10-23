# C-S1SY: sintetizador sustractivo de 8 voces para el SYSTEM-1 (`examples/system_1_synth/`)

Estado: 2026-10-23. **Pasos 1–7 funcionan en el equipo** [medido, a oído]:
* 8 voces, cada una con DCO1 + DCO2;
* forma de onda, nivel y range por oscilador, más fine tune en OSC2;
* ADSR de VCA;
* filtro paso bajo de 4 polos con cutoff, resonancia y key track.

Falta la ADSR del filtro. Extiende C-S1V8 ([examples_system_1_README.md](examples_system_1_README.md) §7). Todo el
código ARM y el microcódigo son nuestros; los programas de Roland sólo se leyeron para estudiar los mecanismos. Qué
es cada archivo: `examples/system_1_synth/FILES.md`.

Etiquetas: **[medido]** = probado en el equipo, **[sim]** = simulador `re/dspisa.py`, **[código]** = visto en los
binarios de Roland (estudio), **[inferido]**, **[abierto]**.

## 1. Uso

```sh
sh examples/system_1_synth/build.sh --help             # todas las opciones
sh examples/system_1_synth/build.sh                    # STEP=7 empaquetado -> plugins/C-S1SY/Script/Code1.Dat
python3 tools/plugout_tool.py install plugins/C-S1SY --slot 1 --force --no-patches -y
```

Después de instalar hay que seleccionar el PLUG-OUT en el panel. Opciones de la build: `STEP` (paso, por defecto 7),
`PACK` (empaquetado, por defecto 1), `GMAX` (tope del filtro, por defecto 0,7), `NAME`, `PARAMLOG`, `PAD`, `VOICES`.

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
| 8 | ADSR del filtro | — | pendiente |

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
| `0x181` / `0x186` | filter env amount / key track | 0–255 | paso 6 (key track) |
| `0x182`–`0x185` | ADSR del filtro | 0–255 | paso 8 |
| `0x187`–`0x18A` | ADSR del VCA | 0–255 | paso 4 |
| — | HPF cutoff | no llegó nada (ni parámetro ni CC) [abierto] | — |

## 9. Mejoras anotadas

* **Curvas y tiempos de las envolventes parametrizables** (pedido 2026-10-23): hoy los faders van de 1 ms a 10 s
  con una ley exponencial fija y el ataque apunta a 1,25 (`make_synth.py`: `env_time`, `attack_rate`,
  `decay_rate`). Se puede hacer configurable:
  * el rango de tiempos y la ley del fader (tablas distintas generadas en el PC), para imitar los tiempos de
    respuesta de distintos sintes;
  * la curva: el objetivo del ataque (1,25 da un ataque casi lineal y un valor cercano a 1, uno más exponencial), y
    caídas lineales en lugar de exponenciales.

  Todo se elige en el ARM, con tablas y un parámetro `C0xx` más para el objetivo del ataque. El coste en el DSP es
  de cero o una instrucción.
* **Simulador más rápido**: el actual (`re/dspisa.py`) tarda minutos para unos miles de muestras de 8 voces. Hasta
  mejorarlo, las pruebas se hacen directamente en el equipo.
* **Buscar el presupuesto exacto del SYSTEM-1** (entre 682 y 826) con `PAD=n`, si hace falta apurar.
