# SH-101 con 4 voces en el SYSTEM-1: análisis

Pedido del 2026-10-22: tomar el PLUG-OUT SH-101 (`plugins/SH-101/Script/Code1.Dat`, versión 14.FA, para SYSTEM-1)
y hacer que toque 4 notas de polifonía. **Nada parcheado ni instalado todavía.**

Leyenda: **[código]** visto en el binario · **[inferido]** · **[abierto]**.

## 1. Qué es el SH-101 hoy

* Sinte monofónico. En el panel del SYSTEM-1, el selector de modo del VCA queda en `MONO` (manual, tabla de
  controles) [código/manual]. `Script.xml` no tiene parámetros de modo poly; solo `CHORD MEMORY Note (voice 1…16)`.
* Región 1 (motor, slave, `0x60200000`): **dos programas DSP**, ambos sin las ranuras de voz [código]:

| Set (entradas `{addr, len, ptr}`) | Programa | Tamaño | DSP |
|---|---|---|---|
| `0x60221FBC`: `0x0` cabecera, **`0x100` principal**, `0x7FF2` cierre | `0x60220B44` | 2611 medias palabras, **575 instrucciones**, 1 sección de voz (`ef75`) | DSP0 |
| `0x60224194`: igual | `0x60224130` | 41 medias palabras, 12 instrucciones | DSP1 |

* Los dos sets se referencian desde los descriptores `0x6021F7EC` (`{…, 3, set, …}`) y `0x60220960` [código].

## 2. Cómo carga programas el OS del slave (1.30)

Tabla de servicios del slave en `0x0100FC00` (copiada de `0x6005A6B0`, 0x288 bytes) [código]:

| Servicio | Función | Qué hace |
|---|---|---|
| `0x0100FC30` | `0x60003ECF` | encola un descriptor de programa en la cola de **DSP0** (`0x600603E0`) |
| `0x0100FC34` | `0x60003ED9` | ídem **DSP1** (`0x600603F0`) |
| `0x0100FC38` / `FC3C` | `0x60003EE3` / `0x60003EED` | ídem DSP2 / DSP3 (`0x60060400` / `0x60060410`) |
| `0x0100FC28` / `FC2C` | `0x60003EBD` / `0x60003EC5` | ídem DSP4 / DSP5 (`0x60060420` / `0x60060430`) |
| `0x0100FC40` | `0x60003F22` | coloca un programa de voz: `FC40(índice 0–7, set del programa, 3 sets de datos)` → (A/B, ranura, sección) por su propia cola (`0x600602d8`) [código]; un programa propio colocado así corre [medido 2026-10-28] |
| `0x0100FC44` | `0x60003F96` | activa/desactiva una de esas 8 ranuras: el bit `1 << sección` de `[0x60018520 + A/B·4 + 0x68]` es de **apagado**; `FC44(i, 1)` lo borra (enciende), `FC44(i, 0)` lo pone [código] |

La asignación de cada DSP a una cola sale del orden de los desplazamientos (`-0x40` … `+0x10` sobre `0x60060420`)
y es [inferida]; el resto es [código].

Índice de `0x0100FC40` → (A/B, ranura, sección):

| índice | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|---|---|---|---|---|---|---|---|---|
| A/B | 0 | 0 | 1 | 1 | 0 | 0 | 1 | 1 |
| ranura | `0x1900` | `0x4900` | `0x1900` | `0x4900` | `0x3100` | `0x6100` | `0x3100` | `0x6100` |
| sección | 2 | 4 | 2 | 4 | 3 | 5 | 3 | 5 |

Lectura [inferido]: el SYSTEM-1 de serie tiene **dos DSP de oscilador** (A/B = OSC1/OSC2; por eso los sets de voz
vienen en parejas idénticas, `VOCES_6.md` §2), cada uno con 4 ranuras de voz llamadas desde su principal. Es el
mismo esquema de ranuras de los DCO del SYSTEM-8.

El motor del SH-101 (`0x602016F8`) [código]:

1. llama a `FC44(i, 0)` para i = 0…7 (cambia las 8 ranuras de voz del sinte de serie);
2. encola su set grande en DSP0 (`FC30`) y el pequeño en DSP1 (`FC34`).

Es decir: **el SH-101 entero (oscilador, filtro, envolventes…) es un programa de 575 instrucciones en un solo DSP**,
y no carga nada en DSP2–DSP5 [código]. Lo que corre en esos DSP mientras el SH-101 está activo está [abierto].

## 3. Qué haría falta para 4 voces

El presupuesto es de ≈ 925 instrucciones por DSP y muestra. Una voz del SH-101 cuesta 575, así que **no caben dos
voces en un DSP**. Cuatro voces necesitan cuatro DSP, uno por voz [inferido].

1. **Microcódigo**: cargar copias del programa de 575 instrucciones en otros DSP (DSP2–DSP5 vía `FC38/FC3C/FC28/FC2C`)
   y cambiar en cada copia las direcciones de salida para que no se pisen (en MXM gana la última escritura, no suma;
   `docs/DSP_HARDWARE.md`). Alguien tiene que sumar las 4 salidas: el programa de 12 instrucciones de DSP1, que
   ahora reenvía la salida de DSP0 (lee `6006`–`600A`/`8fe0`… por los bancos 3/4), ampliado a 4 entradas.
2. **Motor (región 1)**: cada parámetro de patch que hoy se escribe en DSP0 se escribe también en los otros 3 DSP; el
   tono y la puerta de cada voz, solo en su DSP. Hay que encontrar el envoltorio de escritura de parámetros (servicios
   `0x0100FCxx` con unidad DSP) y duplicarlo o redirigirlo.
3. **Modelo (región 0)**: el asignador del SH-101 es mono. Hay que pasarlo a 4 voces y enviar nota/puerta por voz
   (los ids por voz del SYSTEM-1 de serie son `0x110+v`, `0x120+v`, `0x128+v`; los del SH-101 están por ver).
4. **Comprobación**: emulador de DSP (`examples/from_scratch/emu/`) con el programa del SH-101 y luego, en el equipo,
   instalar en el slot 1 (`plugout_tool.py install … --slot 1`; sustituye al SH-2 que hay ahora).

Lo [abierto] que decide la viabilidad:

* que DSP2–DSP5 estén libres (o se puedan vaciar) con el SH-101 activo;
* que la memoria de datos de cada DSP sea propia, para que cada copia tenga su estado (filtro, envolventes) sin
  cambios;
* que el programa no use recursos de un solo DSP (memoria de retardo, IOMD con dueño);
* el formato de las escrituras de parámetros del motor.

## 4. Siguiente paso

Desensamblar con `re/dspisa.py` los dos programas del SH-101 (575 y 12 instrucciones) y listar sus referencias a los
bancos 1–5 (entradas, salidas, parámetros). Así se sabe qué hay que reubicar por copia y si DSP1 puede mezclar 4
entradas.
