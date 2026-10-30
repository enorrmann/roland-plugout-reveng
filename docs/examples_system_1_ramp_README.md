# C-S1RAMP: medición de las rampas de parámetros del SYSTEM-1 (`FCE4`)

`examples/system_1_ramp/`: C-S1V4 (`examples/system_1`) más una medición, en el ARM del maestro, del servicio
`FCE4`. Ese servicio escribe un parámetro del banco 6 (`C0id`) con rampa del hardware.
Medido el 2026-10-29 en el SYSTEM-1 (OS 1.30).

## 1. Uso

```sh
sh examples/system_1_ramp/build.sh                                         # -> plugins/C-S1RAMP
python3 tools/plugout_tool.py install plugins/C-S1RAMP --slot 1 --force --no-patches -y
python3 examples/system_1_ramp/ramp.py -t 600 --log work/captures/FECHA/ramp.txt   # PLG1 seleccionado
```

| control | id | qué hace |
|---|---|---|
| LFO RATE | `0x162` | cada escalón nuevo de 16 lanza una rampa del parámetro `C030` del DSP B del maestro, con `y` = perilla / 16 |
| LFO DELAY TIME | `0x161` | modo `z` = perilla / 32 |
| LFO → PITCH | `0x163` | < 128: sube de 0 a 1; si no, baja de 1 a 0 |
| LFO SHAPE | `0x160` | ≥ 3: arranca la cadena de fin de rampa (§2.1), reporte `F0 7D 08`; < 3: la detiene |
| cualquier parámetro | — | reporte SysEx `F0 7D 07` del último caso (`ramp.py` lo decodifica) |

El ARM fija el valor inicial con `FCEC` y lanza `FCE4`. Después lee el valor actual (palabra `+8` del bloque) en un
bucle, con el contador de ciclos del Cortex-M3 (DWT `CYCCNT`), hasta que cruza 0,999 o pasan 2,5 s. Mientras dura el
bucle, el panel queda congelado. Los ms suponen el SysTick del OS a 1 kHz (`LOAD + 1` = 156 000 ciclos por ms, o sea
un ARM de 156 MHz) [A].

El programa del DSP B del maestro (`make_s1.py prog_m1`) también mide los cruces y los deja en el banco 5
(`A1E0`–`A1EA`). Pero **el ARM del maestro lee 0 en todo el banco 5** por las cuatro vías probadas: la ventana
`0x40101000`, `FD78` (lectura directa, `sel` 0 / 1) y `FD74` (a través del conversor `0x40040550`). También da 0 en
`A118`/`A119`, las palabras de audio que escribe ese mismo programa [M]. Por eso la medición la hace el ARM.

## 2. Servicio `FCE4` (maestro 1.30 `0x01008CD4`; esclavo `0x0100976C`) [código]

```c
typedef void (*ramp_fn)(int sel, int id, uint32_t destino_ieee, int y, int z);   /* SVC(0xE4) */
/* bloque del parámetro: 0x4014C000 (DSP A) / 0x4016C000 (DSP B) + 16·id */
[+0] = 0x80000000 | z << 28 | y << 24      /* control; se lee de vuelta con los bits 30-29 en 1 */
[+4] = destino                             /* las dos palabras se escriben dos veces */
/* FCEC (inmediato): [+0] = 0xE0000000, [+4] = [+8] = valor, [+0xC] = 0 */
```

Con `z ≠ 0`, `FCE4` primero intenta registrar una función del OS en la tabla de fin de rampa: `0x20003534 + 4`
(`+0x404` para el DSP B) `+ 4·id`, y solo si la entrada está libre. Esa función (`0x01007F63` / `0x01007F7B` en el
maestro; `0x010089FB` / `0x01008A13` en el esclavo) levanta una bandera de evento del OS y libera la entrada. Hay
otra variante en `0x01008D44`, que escribe `0x40000000 | z << 28` sin destino; no está medida.

### 2.1 Función propia al terminar la rampa [código 2026-10-29; medido 2026-10-30]

El OS da a los PLUG-OUT el registro de la tabla. Lo que sigue está en el esclavo 1.30; el maestro tiene los
mismos servicios en las mismas posiciones (`0x0100D86C` / `0x0100D878`):

| servicio | qué hace | código |
|---|---|---|
| `FE3C(sel, id, fn)` (`SVC 0x23C`) | pone `fn` en `tabla[id]` si está libre; devuelve 1 / 0. **id < 0x100**: 256 entradas, sin control de rango [M] | `0x0100C4F4` → `0x01009754` → `0x01008A6A` |
| `FE40(sel, id)` (`SVC 0x240`) | libera `tabla[id]` | `0x0100C500` → `0x010089EC` |
| `FE34(id)` / `FE38(id)` (`SVC 0x234` / `0x238`) | llama `tabla[id](id)` del DSP A / B a mano | `0x0100AA0A` / `0x0100AA10` |

La interrupción de fin de rampa del DSP B (`0x0100B008`, puntero en el vector `0x6005A64C`; DSP A `0x0100AFF4`) lee
un registro del hardware, toma el `id` de los bits 4–11, lo borra y llama al despachador `0x01008A4C`:
`if (tabla[id]) tabla[id](id)`. **El despachador no libera la entrada.** Entonces una función nuestra registrada
con `FE3C` se llama en cada fin de rampa con `z = 1`, y puede lanzar la rampa siguiente (`FCE4` con `z = 1`; su
intento de registrar la función del OS falla porque la entrada está ocupada). Eso es un secuenciador por
interrupción, **sin tick**: lo que hace falta para envolventes y LFO en el ARM. [A]: un solo registro de `id` por
DSP, y se desconoce qué pasa si dos rampas terminan juntas.

**Medido en el equipo (2026-10-30, captura `work/captures/2026-10-30/chain_out.txt`)** [M]
* **Montaje.** C-S1RAMP con LFO SHAPE ≥ 3 registra `chain_cb` en `C030` del DSP B del maestro. Esa función lanza la
  rampa contraria (un triángulo 0 ↔ 1 con `y = 0`, `z = 1`) y anota la hora (DWT `CYCCNT`) de cada llamada.
* **Registro.** `FE3C` devolvió 1 y la entrada quedó con nuestra dirección (`0x60809737`).
* **Llamadas.** 6225 sin que el equipo se colgara: **una cada 4,00 ms ± 0,02**, la duración de la rampa con
  `y = 0` (§3).
* **Liberación.** Con LFO SHAPE < 3, `FE40` devolvió 1, la entrada volvió a 0 y las llamadas se detuvieron.
* **Conclusión.** Desde la interrupción se puede llamar a `FCE4`. El ARM encadena rampas sin tick y sin perder
  ninguna.
* **Falta medir.** Otras `y`, varios parámetros encadenados a la vez (¿dos fines simultáneos?) y lo mismo en el
  esclavo, donde corre el motor. El código del esclavo es el mismo (§2.1).

## 3. Resultados [M]

* **`+8` es el valor actual de la rampa** y el ARM lo puede leer. **`+C` es el incremento por paso** que calcula el
  hardware.
* **La rampa es lineal**: llega a 0,5 a la mitad del tiempo, igual con todos los `y`.
* El hardware da **un paso cada 1/3 ms** (3 kHz = 96 kHz / 32). Duración = (1 / `+C`) pasos × 1/3 ms, exacto en los
  16 valores de `y`.
* **La duración depende solo de `y`, no de la distancia** [M, 2026-10-30, cadena de §2.1]: con `y = 0`,
  4,00 ms para amplitudes 1, ½, ¼ y ⅛; con `y = 1`, 8,15 ms para 1, ½ y ¼. El hardware calcula el incremento
  (`+C`) = distancia / pasos. Captura: `work/captures/2026-10-30/dist_out.txt`.
* **`z = 1` da los mismos tiempos que `z = 0`.** Al leer el reporte, la entrada de la tabla de fin ya está en 0.
* Cada `y` se midió dos veces (subiendo y bajando la perilla); las dos dan lo mismo con ±0,05 ms de diferencia.

Rampa de 0 a 1 (captura `work/captures/2026-10-29/ramp_out.txt`):

| y | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| ms | 3,98 | 8,13 | 12,20 | 16,25 | 20,08 | 24,38 | 28,43 | 32,50 | 35,91 | 40,14 | 48,77 | 56,84 | 64,96 | 71,81 | 80,25 | 97,44 |
| `+C`·2048 | 172 | 84 | 56 | 42 | 34 | 28 | 24 | 21 | 19 | 17 | 14 | 12 | 10,5 | 9,5 | 8,5 | 7 |

## 4. Abierto

| # | qué |
|---|---|
| 1 | ~~si la duración depende de la distancia~~: **no**, medido 2026-10-30 (§3) |
| 2 | qué cambian los modos `z` = 2–7, y la bajada |
| 3 | qué pasa al cambiar el destino a mitad de rampa: ¿arranca desde el valor actual? |
| 4 | si el DSP ve la escalera de 3 kHz o un valor interpolado por muestra |
| 5 | por qué el ARM del maestro lee 0 en el banco 5 |
| 6 | la cadena de fin de rampa con varios parámetros a la vez, y en el esclavo (con uno, en el maestro, anda: §2.1) |

## 5. Qué significa para envolventes y LFO

La rampa más larga dura ~0,1 s, así que un segmento largo de una envolvente se arma con varias rampas seguidas.
Con la función de fin de rampa (§2.1, medida), cada tramo lanza el siguiente sin tick. Ahorro medido en el emulador con
MP-CSAW2 a 8 voces, reemplazando el código por la lectura de un parámetro (detalle en
[S1_OPTIMIZATIONS.md](S1_OPTIMIZATIONS.md)):

| al ARM | DSP B (de 701) |
|---|---|
| las 16 ADSR | −213 |
| el LFO por voz | −119 |
| las dos cosas | −332 |

La rampa avanza en escalones de 1/3 ms. Para la amplitud puede hacer falta suavizarla con un polo en el DSP
(2 ops por voz). Está sin medir de oído.
