# C-S1SY: sintetizador sustractivo de 8 voces para el SYSTEM-1 (`examples/system_1_synth/`)

Estado: 2026-10-23. Extiende C-S1V8 ([examples_system_1_README.md](examples_system_1_README.md) §7) con DCO2
independiente, ADSR de VCA, filtro paso bajo y ADSR de filtro. Todo el código ARM y el microcódigo son nuestros.
Se construye **por pasos**: cada paso añade una sola cosa al último que sonó en el equipo.

Etiquetas: **[medido]** = probado en el equipo (a oído), **[sim]** = simulador `re/dspisa.py`, **[código]** = visto en
los binarios de Roland (sólo estudio), **[abierto]**.

## 1. Uso

```sh
STEP=5 NAME=C-S1SY5 sh examples/system_1_synth/build.sh          # -> plugins/C-S1SY5/Script/Code1.Dat
PARAMLOG=1 STEP=5 NAME=C-S1SY5 sh examples/system_1_synth/build.sh   # + captura de parámetros (§5)
python3 tools/plugout_tool.py install plugins/C-S1SY5 --slot 1 --force --no-patches -y
```

`make_synth.py` genera los programas DSP (`s1*.s`, `s1*.hex`, `build/s1_programs.h` con las tablas). `STEP=0`
construye el primer diseño, que lo metía todo de una vez y no sonó (§4).

## 2. Pasos

| paso | qué añade | DSP A / B (instr.) | resultado |
|---|---|---|---|
| 1 | DCO2 por voz: segunda copia de la sierra de C-S1V8, +7 cents, nivel 0,5 fijo | 541 / 13 | [medido] suena |
| 2 | niveles OSC1/OSC2 del panel (`0x174`/`0x175`) → banco 5 `A040`/`A041` (ARM `FD6C` sel 0) | 581 / 13 | [medido] mandos ok |
| 3 | transporte por voz: A guarda cada voz en `A151+v` y copia la puerta `A038+v` → `A160+v`; B multiplica y mezcla | 570 / 53 | [medido] suena |
| 4 | ADSR del VCA en el DSP B; faders `0x187`–`0x18A` → parámetros `C040`–`C043` del DSP B (`FCEC(1, …)`); disparo `A060+v` copiado por A → `A168+v`; estado de B en su banco 0 `0x0400 + 0x10·v` | 594 / 351 | [medido] funciona |
| 5 | OSC1/OSC2 range (`0x16A`/`0x16F`, 64'…2' = −3…+2 octavas) y OSC2 fine tune (`0x170`, ±1 semitono) → parámetros `C044`–`C046` del DSP A (`FCEC(0, …)`); desaparece el +7 cents fijo | 674 / 351 | [sim] 440,37 Hz; sin probar |
| 6+ | formas de onda, filtro paso bajo, ADSR de filtro | — | pendiente |

## 3. Interfaz con el panel

Ids capturados con PARAMLOG (2026-10-23; llegan también al motor del esclavo, `vt+0x30`, [medido] en el paso 2):

| id | control | valores |
|---|---|---|
| `0x162` | LFO rate | 0–255 |
| `0x167` / `0x16C` | OSC1 / OSC2 wave | 0–5 |
| `0x16A` / `0x16F` | OSC1 / OSC2 range | 0–5 (64'…2') |
| `0x170` | OSC2 fine tune | 0–255, centro 128 |
| `0x174` / `0x175` | OSC1 / OSC2 mix | 0–255 |
| `0x17D` / `0x17F` | cutoff / resonancia | 0–255 |
| `0x181` / `0x186` | env amount / key track | 0–255 |
| `0x182`–`0x185` | ADSR del filtro | 0–255 |
| `0x187`–`0x18A` | ADSR del VCA | 0–255 |
| `0x168` / `0x16D` | OSC1 / OSC2 color (CC 50 / 55) | 0–255 |
| `0x176` / `0x177` | sub mix / noise level (CC 18 / 19) | 0–255 |
| `0x17A` | pitch env amount (CC 22) | 0–255 |
| `0x17B` / `0x17C` | pitch env attack / decay (CC 23 / 24) | 0–255 |
| — | HPF cutoff | no llegó nada (ni parámetro ni CC) en la captura del 2026-10-23 [abierto] |

Cómo llegan los valores a cada DSP, como en el SH-2 de Roland ([código], `system-1/s1_progs.py --listing`):

| camino | quién lo lee | estado |
|---|---|---|
| ARM `FCEC(sel, id, bits)` → parámetro `C0id` del DSP A (`sel 0`) o B (`sel 1`) | ese DSP | [medido] en el esclavo B (paso 4) |
| ARM `FD6C(0, …)` → banco 5 `A0xx` | DSP A | [medido] |
| ARM `FD6C(0, …)` → banco 5 `A0xx` | DSP B | **no llega**: el paso 3 con B leyendo `A038+v` no sonó [medido] |
| DSP A → banco 5 `A1xx` → DSP B | DSP B | [medido] (`A150`, `A151+v`, `A160+v`, `A168+v`) |

El SH-2 tiene los mandos en `C0xx`, el intercambio entre DSP en `A1xx` y el estado en el banco 0 bajo
(`0018`–`01E6`).

## 4. Diseño completo previsto y el primer intento (`STEP=0`)

`STEP=0` (= `STAGE=4`) metía todo en una instalación:
* DSP A: dos osciladores con `ph_wrap`/`ph_tri`, mezcla de sierra, pulso y triángulo, y ADSR de filtro.
* DSP B: ADSR de VCA, corte exponencial y filtro de 4 polos con realimentación.
* Parámetros escritos en `A040`–`A05F` con `FD6C sel 0` y leídos por el DSP B.

En simulación el diseño funciona, pero en el equipo no sonó (no se colgó). Las causas posibles eran varias a la vez:
* el DSP B lee `A0xx`: comprobado en el paso 3 que no funciona;
* instrucciones nuevas en el SYSTEM-1;
* la estructura de salida.

Las funciones de ese diseño (`osc`, `adsr`, `prog_s1m0/1`) quedan como referencia para los pasos siguientes.

Nota del ensamblador: `ph_tri rD, rX, rY` se codifica como `ph_wrap`; hay que escribir `ph_tri rD, rX, -rY`, y lo
mismo con `ph_clamp` (lo indica `asm/isa.json`: la variante se elige por el bit de negación de Y).

## 5. Captura de parámetros

Igual que en C-S1V4 (§6 de su documento): una build con `PARAMLOG=1`, luego
`python3 examples/system_1/paramlog.py -t 90 --raw` mientras se mueve un control cada vez.

## 6. Mejoras anotadas

* **Curvas y tiempos de las envolventes parametrizables** (pedido 2026-10-23): hoy los faders van a tiempos de
  1 ms a 10 s con una ley exponencial fija y el ataque apunta a 1,25 (`make_synth.py`, `env_time`, `attack_rate`,
  `decay_rate`). Se puede hacer configurable:
  * el rango de tiempos y la ley del fader (tablas distintas generadas en el PC), para imitar los tiempos de
    respuesta de distintos sintes;
  * la curva: el objetivo del ataque (1,25 da un ataque casi lineal; un valor mayor, más lineal; cercano a 1, más
    exponencial), y caídas lineales en lugar de exponenciales.

  Todo se elige en el ARM: tablas y un parámetro `C0xx` más para el objetivo del ataque. El coste en el DSP es
  nulo o de una instrucción.
