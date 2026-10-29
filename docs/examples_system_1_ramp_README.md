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

Con `z ≠ 0`, `FCE4` primero registra una función del OS en la tabla de fin de rampa: `0x20003534 + 4` (`+0x404` para
el DSP B) `+ 4·id`. La función de fin (`0x01007F63` / `0x01007F7B` en el maestro) atiende la interrupción y libera la
entrada. La función es del OS: el PLUG-OUT no puede pasar la suya. Hay otra variante en `0x01008D44`, que escribe
`0x40000000 | z << 28` sin destino; no está medida.

## 3. Resultados [M]

* **`+8` es el valor actual de la rampa** y el ARM lo puede leer. **`+C` es el incremento por paso** que calcula el
  hardware.
* **La rampa es lineal**: llega a 0,5 a la mitad del tiempo, igual con todos los `y`.
* El hardware da **un paso cada 1/3 ms** (3 kHz = 96 kHz / 32). Duración = (1 / `+C`) pasos × 1/3 ms, exacto en los
  16 valores de `y`.
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
| 1 | si la duración depende de la distancia (todo se midió de 0 a 1) o si el paso es fijo |
| 2 | qué cambian los modos `z` = 2–7, y la bajada |
| 3 | qué pasa al cambiar el destino a mitad de rampa: ¿arranca desde el valor actual? |
| 4 | si el DSP ve la escalera de 3 kHz o un valor interpolado por muestra |
| 5 | por qué el ARM del maestro lee 0 en el banco 5 |

## 5. Qué significa para envolventes y LFO

La rampa más larga dura ~0,1 s. Un segmento entero de una envolvente (ataques de segundos) no entra en una sola
rampa. La rampa sirve como **interpolador** de valores que el ARM calcula a tasa de control: con `y = 0` une dos
puntos separados 4 ms con una recta (12 pasos de 1/3 ms). Para eso el ARM necesita un llamado periódico. El motor
del esclavo solo recibe eventos (`init`, `load`, `refresh`, `param`) y todavía no se encontró un tick.
