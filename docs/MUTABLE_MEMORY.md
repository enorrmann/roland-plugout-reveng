# Osciladores de Mutable que necesitan memoria: dónde podrían vivir en el SYSTEM-1

Especificación de estudio (2026-11-03). Para cada forma de `~/git/eurorack` que no se puede escribir sin memoria
(línea de retardo o tabla de datos), qué memoria del SYSTEM-1 le sirve, cuánto entra, qué falta en s1cc y en el
anfitrión, y en qué orden conviene hacerlo. Las formas sin memoria están portadas en
`sdk/s1cc/examples/mutable_ported` ([examples_mutable_ported_README.md](examples_mutable_ported_README.md)).

Etiquetas: **[medido]** (en el equipo), **[código]** (leído en el firmware / la fuente), **[estimado]** (cuenta),
**[abierto]**.

## 1. Qué necesita memoria

| forma | origen | memoria | tamaño por voz |
|---|---|---|---|
| PLUCKED, BOWED, BLOWN, FLUTED | Braids (`digital_oscillator.cc`) | línea de retardo variable (guía de onda, Karplus) | 1 período de la nota más grave: 96000 / f muestras |
| SAW_COMB | Braids | línea de retardo variable (peine) | ídem |
| motor *string* (Karplus + dispersión) | Plaits `physical_modelling/string.cc` | línea de retardo variable + filtro de dispersión | ídem (Plaits: 1024 a 48 kHz) |
| motor *string machine* (chorus / ensemble) | Plaits `engine2/string_machine_engine.cc` | línea de retardo con tomas moduladas | ~10–20 ms: 1000–2000 muestras, **una por PLUG-OUT** (después de la mezcla) |
| Rings, Elements | `rings/dsp`, `elements/dsp` | varias líneas (cuerdas, tubo, difusor) | 1–4 k muestras por voz |
| Clouds / Beads | `clouds/dsp`, `beads/dsp` | búfer de audio de segundos | 100 k+ muestras, global |
| WAVETABLES, WAVE_MAP, WAVE_LINE, WAVE_PARAPHONIC | Braids (`wt_waves`, 129 bytes por onda, 256 ondas) | tabla leída por fase | 129–256 palabras por onda; 2 ondas por voz para el morph |
| motor *wavetable* | Plaits (`wav_integrated_waves`, 4 bancos × 64 ondas × 132) | ídem | ídem |
| motor *wave terrain*, terrenos 5–8 | Plaits | tablas de 64 × 64 o de ondas | 4 k palabras |
| motor *speech* (LPC, SAM) | Plaits `speech/` | tramas de coeficientes (datos del ROM) | **a tasa de control**: una trama cada 10–25 ms |

Lo que **no** necesita memoria quedó portado aunque el original use tablas: las tablas de función (seno, plegador,
`ws_*`, `lut_svf_cutoff`, campanas) son curvas, y s1cc las convierte en polinomios o en tablas de perilla.

## 2. Las memorias del SYSTEM-1 que un kernel podría usar

| memoria | tamaño | quién escribe | lectura con índice calculado | estado |
|---|---|---|---|---|
| banco 0 del esclavo (anillo) | 2048 palabras por DSP (`0000`–`0017` del OS), compartidas con todo el estado del kernel | el DSP | **sí**: `hset h6` + `ldidx` (relativo al anillo) | [medido] línea de 750 muestras con carga normal y con `ldidx` (C-S1SY15X, [../system-1/DSP_BANKS.md](../system-1/DSP_BANKS.md) §4); `hset` desde un registro en MP-CSAW2-8 (COLOR MOD, 2026-10-29) |
| banco 1 del esclavo (anillo) | 1024 palabras (`2000`–`23FF`) | el DSP | `ldidx` no lo direcciona [código] | buzón principal ↔ secciones ([../system-1/VOICE_ALLOCATION.md](../system-1/VOICE_ALLOCATION.md) §3.2) |
| banco 5 (`A000`–`A3FF`) | 1024 palabras, compartidas por el DSP A y el B del esclavo; libres hoy ~`A070`–`A150` y `A200`–`A3FF` [código, SYSTEM-S1CC] | el ARM (`FD6C`) y el DSP | `da` / `db` (h6 / h7) | [medido] en los esclavos del SYSTEM-8 ([DSP_HARDWARE.md](DSP_HARDWARE.md) §7.4); en el S1 armado (MP-CSAW2-LUT, `LFO_LUT`), **sin probar** |
| memoria de retardo (banco 2, `tapreq`) | megapalabras (S8: 2 M por BMC) | el DSP (entrada `4300`) | tomas con base `h4` y resultado `h5`; ≥ 24 tomas por muestra, fracción 1/64 | en el S1 sólo la usa el efecto del OS en el **maestro** B ([../system-1/FX_CHAIN.md](../system-1/FX_CHAIN.md) §3); en los esclavos del S1 **sin medir** [abierto] |
| parámetros (banco 6, `C060`–`C0FF`) | 160 palabras por DSP | el ARM (`FCEC`) | no (el kernel lee ids fijos) | [medido] 1000 escrituras/s por id exactas ([S1_ARM_CLOCK.md](S1_ARM_CLOCK.md) §2) |
| memoria del ARM (región 1 del PLUG-OUT) | ≤ 1 MiB comprimido por región (~0,94 MB de PCM) | — | el ARM la copia a bancos 5 / 6 | [medido] en el S8 ([NOTES.md](NOTES.md), «Muestras largas») |

## 3. Las dos ideas del pedido

### 3.1 Stream desde la memoria del ARM con el reloj de 1 ms

El tick del ARM del esclavo ([S1_ARM_CLOCK.md](S1_ARM_CLOCK.md)) llama nuestra función cada 1,00 ms; desde ahí
`FCEC` escribe parámetros del DSP y `FD6C` palabras del banco 5.

| uso | palabras por ms | entra | por qué |
|---|---|---|---|
| audio (muestras de una voz) | 96 por voz (768 con 8 voces) | **no** [estimado] | probado: ~33 `FCEC` por ms sin molestar al OS (8 voces × 2 envolventes + LFO). 96 por voz es 3× eso para **una** voz. Además el DSP tendría que leer la palabra de la muestra n (un contador y una carga indexada del banco 5) y el ARM ir un bloque adelante con su reloj sin deriva respecto del de audio: un fallo de un tick es un hueco audible |
| realimentación (Karplus, cuerdas) | — | **no** | la muestra de salida vuelve a la línea en el mismo período: el ARM está al menos un tick (96 muestras) atrás |
| tramas de control (LPC / SAM del motor *speech*, formantes de VOWEL_FOF) | 10–20 coeficientes cada 10–25 ms | **sí** [estimado] | como las envolventes del anfitrión (`ARM_ENV`): `FCEC` a `C080`… por voz |
| reescribir una wavetable del banco 5 al mover COLOR | 256 palabras cada vez que cambia la onda | **sí**, repartidas en ticks [estimado] | ~30 palabras por ms → una onda nueva en ~9 ms; el DSP lee siempre de una tabla fija (doble búfer: el ARM llena la otra mitad y cambia un parámetro de base) |

Conclusión: el reloj sirve para **datos que cambian a tasa de control** (tramas de voz, cambio de onda, posiciones de
grano), no para audio ni para retardos.

### 3.2 La línea de retardo

**Banco 0 con `ldidx`** (lo que ya funciona en el esclavo). La línea es un tramo del anillo: el kernel guarda la
entrada en `S + b` cada muestra y lee `S + b + d` con `hset h6 = (b + d)/16384` y `ldidx` (d calculado: la altura de
la nota; dos lecturas y una interpolación lineal para el retardo fraccionario). Nada más del DSP puede guardar en
`S + b + 1 … S + b + dmax` ([../system-1/DSP_BANKS.md](../system-1/DSP_BANKS.md) §3, regla 1).

| voces | palabras libres por voz (2048 − OS − estado de kernel y anfitrión) | nota más grave de una cuerda |
|---|---|---|
| 8 | ~140 | 690 Hz (Fa5) |
| 6 | ~225 | 430 Hz (La4) |
| 4 | ~385 | 250 Hz (Si3) |
| 2 | ~870 | 110 Hz (La2) |

[estimado desde `make_s1cc.py bank0_top`: con `mp_saw` y 8 voces el estado del DSP A llega a `0x0384` (cada palabra
de estado ocupa dos del anillo, `S` y `S + 1`), unas 100 palabras por voz más unas 100 fijas]

Para notas más graves sin más memoria: la línea a media tasa (la cuerda de Plaits corre a 48 kHz): escribir una
muestra cada dos con la condición `X9bY` (sin probar en el S1) o promediando de a pares, y leer con `d/2`. Duplica el
alcance (8 voces: 345 Hz) con un filtro de lazo a 48 kHz, como el original.

**Memoria de retardo (`tapreq`)** en los esclavos: si existe, es la única que da cuerdas a cualquier altura, chorus /
ensemble, Rings / Elements y búferes de Clouds. En el S1 sólo se vio en el maestro; el OS no la usa en los esclavos y
ningún PLUG-OUT oficial tampoco ([../system-1/FX_CHAIN.md](../system-1/FX_CHAIN.md) §2). En los esclavos del S8 hay
tomas en los programas `p0s_*` (ventana sin medir). Prueba que falta, en este orden: (1) un kernel de un esclavo con
la secuencia de `tapreq` del maestro ([DSP_ISA_SPEC.md](DSP_ISA_SPEC.md) §9.1: `[4300] ← x`, la instrucción de base,
`0b03 4050 0301`, `3a90`) y la toma leída a una palabra del transporte; (2) captura USB de un peine fijo de 5000
muestras (imposible en el banco 0). Si la toma da silencio, la memoria de retardo queda sólo para el maestro: un
chorus / ensemble global (string machine) en el maestro B, sí; cuerdas por voz, no.

### 3.3 La memoria de retardo grande, desde el maestro

Lo medido y leído [código salvo donde dice]:

* La memoria de retardo (EDM, banco 2 de los DSP, `tapreq`) sólo responde **donde el ARM conectó la ventana DT2**
  ([DSP_HARDWARE.md](DSP_HARDWARE.md) §8.4; en el S8 una sola ranura del maestro, DSP4 `f7dc`). En el SYSTEM-1 la
  conecta el OS para su efecto delay + reverb, programa `0x0` del **maestro B** ([../system-1/FX_CHAIN.md](../system-1/FX_CHAIN.md)
  §3: base `h4` = 256, interpolación entre dos muestras, escritura `4001`). Ningún programa de esclavo del S1 usa el
  banco 2, y el mapa de registros del BMC del S1 (`0x401xxxxx`) no tiene las constantes del S8 (`0x41158000`,
  `0xC0200000`): no sabemos todavía configurar la ventana nosotros.
* El esclavo manda al maestro 8 palabras IOMD por muestra: `6006`/`6007` (la mezcla de SYSTEM-S1CC) y
  `6008`–`600D`, que hoy van en 0 ([../system-1/DSP_BANKS.md](../system-1/DSP_BANKS.md) §1). Son **6 canales libres
  de audio por muestra, esclavo → maestro** [medido: `6006`/`6007`; el resto, mismo mecanismo].
* El ARM del maestro (región 0, `model.c`) asigna las voces: sabe la nota de cada voz y puede escribir parámetros del
  maestro B con `FCEC`.

**Medido 2026-11-03: MP-EDMKS, una cuerda Karplus en la memoria de retardo, sin microcódigo propio en el maestro.**
`sdk/s1cc/examples/edm/edm_pluck.c` (golpe de ruido de 1–16 ms por nota en el esclavo) + `EDM_KS=1`
(`examples/system_s1cc/build.sh`, `model.c`): en cada nota la región 0 escribe

| palabra | valor | qué es |
|---|---|---|
| banco 5 `A101`, `A102` (`FD6C(sel 1, idx 1 / 2)`) | (período − 3) / 16384 | tiempos del delay del OS (izquierdo / derecho), en unidades de 16384 muestras; `A103` = 2 × `A101` |
| maestro B `C001` / `C002` (`FCEC(1, …)`) | 1 / 1 | delay encendido / nivel |
| maestro B `C004` | 0,985 | realimentación (el OS pone 0,6 con DELAY > 0) |

Cómo se encontró: con `FD6C(sel 0)` (`A0xx`) y escribiendo en `0x4016A000` la línea quedaba en 961 muestras (ecos
cada 961 muestras, realimentación 0,97 por vuelta: la línea del delay con DELAY TIME en 0). Un build `PARAMLOG`
(`edm_dump`) leyó `A101` = 0,05847 = 958 / 16384 en la página `A1xx` de `FD6C`: 3 muestras de latencia del lazo.
`0x4016A000` lee 0. Capturas: `sdk/s1cc/examples/edm/ks_probe.sh` (nota por MIDI canal 5 del equipo + `arecord`, autocorrelación).

| nota | esperado | medido (0,8 s después de la nota) |
|---|---|---|
| 36 | 65,41 Hz | 65,48 Hz |
| 55 | 196,00 Hz | 195,92 Hz |
| 60 | 261,63 Hz | 261,58 Hz |
| 72 | 523,25 Hz | 521,74 Hz |

Límites de esta forma: una sola cuerda (la línea del OS, la última nota fija la altura); el programa del OS suaviza
el tiempo, así que la altura se desliza desde la nota anterior durante ~0,5 s; DELAY / DELAY TIME del panel
vuelven a escribir sus valores; la reverb sigue en la cadena. Para cuerdas por voz hace falta el programa propio del
maestro B (abajo), con los mismos `tapreq` que ahora sabemos que la ventana acepta en ese DSP.

**Medido 2026-11-03: pruebas de `tapreq` propio en el maestro B** (`examples/system_s1cc/microcode/edm.s`,
opciones `EDM_TAP` / `EDM_ENTRY` / `EDM_STEP` / `EDM_MAIN` de `make_s1cc.py`; captura con
`sdk/s1cc/examples/edm/ks_probe.sh`):

| prueba | qué | resultado |
|---|---|---|
| EDMTAP 1 | nuestro principal `0x1000`: 2 `tapreq` de 0,5 s (con y sin contador de posición), resultados en `4040` / `4043` | suena el golpe seco, **sin eco** |
| EDMTAP 2 | ídem con resultados en `402E`; el otro canal lee la palabra de resultado del delay del OS (`4005`) | el canal de `4005` trae el eco de 10 ms del OS: **las cargas del banco 2 funcionan desde `0x1000`**; nuestro pedido, nada |
| EDMTAP 3 | nuestros dos pedidos al tiempo exacto del OS (`A101`) | nada en ningún canal: **`tapreq` sólo se atiende en el programa de `0x0`** (como en el S8, donde sólo una ranura tenía la línea) |
| EDMOWN (`0x0` propio, sin las llamadas de entrada) | reemplaza el efecto | **se colgó** al seleccionar |
| EDMENT 4 (`0x0` propio: cabecera + llamadas a `0x1000`, `0x5FF2`, `0x6FF2` + `tapreq` + salto a `0x7FF2`) | | **se apagó** al seleccionar |
| EDMENT A (ídem sin `tapreq`) | | no se cuelga, **silencio total** |
| EDMENT B (A + principal con la mezcla directa a `A118` / `A119`) | | silencio: el principal no corre |
| EDMENT A + cierres propios `0x5FF2`, `0x6FF2`, `0x7FF2` (orden del set del SH-101) | | silencio |
| control: sólo la cabecera de 4 instrucciones en `0x0` (15 palabras, como el `0x0` de 30 bytes de los PLUG-OUTs de serie) | | **silencio** |

Después de las pruebas EDMENT, MP-EDMKS zumbaba a fondo de escala (95 Hz, pico 1,0, sin tocar) y luego quedaba casi
mudo: el maestro B no vuelve solo a su estado al elegir otro PLUG-OUT. **Apagar y encender** lo arregló: MP-EDMKS
otra vez en silencio sin notas y la cuerda a 130,79 Hz (nota 48) y 260,87 Hz (nota 60). Desde entonces `model.c`
pone la realimentación en 0 al cargar y la escribe recién con la primera nota (por defecto 0,975).

Lectura: **ya estaba medido** ([../system-1/WAVE_BUS.md](../system-1/WAVE_BUS.md) §5.3, regla 1 de
[examples_system_1_jx_README.md](examples_system_1_jx_README.md) §5): un PLUG-OUT **nunca encola `0x0000`**; es la
cabecera que el DSP está ejecutando. Los motores de Roland la tienen en sus tablas pero pasan a `FC30` / `FC34` la
entrada siguiente (ejecutados en Unicorn). Las pruebas EDMOWN / EDMENT la incumplieron: de ahí el cuelgue, el
apagado y los silencios (y el estado roto hasta apagar). No se puede poner código propio en `0x0`.

Consecuencias (2026-11-03):
* `tapreq` propio: **no** por ahora. Sólo lo atiende el programa de `0x0` (el efecto del OS); falta saber qué lo
  habilita (en el S8: la tarea / ranura que el ARM conecta, [DSP_PROGRAM_MEMORY.md](DSP_PROGRAM_MEMORY.md) §2.2) y si
  una sección nuestra puede recibirlo.
* Lo que sí funciona: los tiempos de las dos tomas del OS (`A101` / `A102`, `FD6C sel 1`), su realimentación y nivel
  (`C004`, `C001`, `C002`), y **leer sus resultados `4004`–`4009` desde nuestro principal** (prueba EDMTAP 2).
* El deslizamiento de la altura: el efecto suaviza el tiempo con `r2 = A101 (1 − r7) + anterior · r7`, con
  `r7 = min(0,0287 + r7 anterior, 0,9998875)` (sube a 0,99989 en ~35 muestras y queda) → τ = 92,6 ms, medido ~90 ms.
  Es una constante del código del OS; se cambia pisando el set del OS desde `create()` (§3.3.2). Leído de las transferencias del programa (`st r7,
  [0009]` / `ld r6, [000a]`); el emulador no ejecuta su instrucción de clase `0x06f0`.
* Para atacar el deslizamiento sin código propio en `0x0`: escribir `A101` con sobreimpulso desde un tick del ARM del
  maestro (el gancho de 1 ms existe en el esclavo, [S1_ARM_CLOCK.md](S1_ARM_CLOCK.md); en el maestro sin buscar).

#### 3.3.1 Lectura estática del OS 1.30 del maestro (2026-11-03, sin el equipo)

| hallazgo | dónde | estado |
|---|---|---|
| `FC30` / `FC34` / `FC38` / `FC3C` = la misma rutina (`0x6001CF6C`, un `push` en un vector) sobre cuatro colas: programas maestro A, programas maestro B, **datos** maestro A, **datos** maestro B. `FC40` en el maestro es `bx lr` (no hace nada) | tabla de servicios `0x600AB5DC` → `0x6001ED77`… | [código] |
| los sets de datos son `{dirección, bytes, puntero}` como los de programas. Roland (SH-2, SH-101, PROMARS) encola en el maestro: `{0x110, 8 bytes = 0, 0}` y `{0x17F8, 8 bytes = 0x807FFFFF, 0}` | Unicorn: `load` del SH-2 (región 0) | [medido en Unicorn] |
| `{0x17F8: 0x807FFFFF, 0}` acompaña a **todos** los programas del OS (más de 50 sets en el esclavo): es una inicialización común, **no** la llave de la línea | búsqueda en `app_master.bin` / `app_slave.bin` | [código] |
| el OS sí encola su set del maestro B **con `0x0`**: `{0x0 efecto 0x552 bytes, 0x5FF2, 0x6FF2, 0x1000 principal por defecto 0x38 bytes, 0x7FF2}` + datos `{0x118, 0}, {0x17F8, 0}, {0x0, 0x110}…`, desde descriptores `{cantidad, programas, datos}` en `0x6006025C` | `0x600971F0`, `0x60060278` | [código] |
| el deslizamiento de la cuerda: coeficiente fijo 0,9998875 en el efecto (τ 92,6 ms) | §3.3 | [código + medido] |

Hipótesis que quedan (todas necesitan el equipo):
1. **`tapreq` sólo se ejecuta en el programa de nivel superior** (el de `0x0`, que el DSP recorre sin llamada), no en
   los llamados (`cond 25, 10` → `0x1000`, donde nuestro pedido no hizo nada). El OS también salta (`cond 0, 8`, sin
   retorno) al cierre `0x7FF2`, que los PLUG-OUTs sí encolan (`FC34 {…, 0x1000, 0x7FF2}`). Prueba barata y dentro de
   las reglas: nuestro `0x7FF2` del maestro B con el contador de trama **más** un `tapreq` de 0,5 s sobre la línea que
   escribe el OS, leído en la muestra siguiente por el principal. Si da eco, tenemos `tapreq` propio sin tocar `0x0`.
   Riesgo: el cierre tiene 14 bytes hasta el fin de la memoria de programa (`0x8000`); el pedido entra (3 instrucciones
   de 2–3 palabras) sólo si el cierre puede crecer hacia atrás (empezar antes de `0x7FF2`): eso es lo primero a medir.
2. Si no: `tapreq` depende de la ranura / tarea que el ARM conecta (como en el S8); buscar en el OS del maestro la
   escritura del registro equivalente al `+0x310` del S8 ([DSP_ARM_SIDE.md](DSP_ARM_SIDE.md) §2.2) para el efecto.
3. Encolar un set completo como el del OS (con `0x0`) **no**: lo prohíbe la regla 1 y las pruebas EDMENT lo confirmaron.

#### 3.3.2 Pisar el set del OS desde la región 0 (medido 2026-11-03)

`EDM_GLIDE=1` (con `EDM_KS`, build MP-EDMKG; `model.c edm_glide`) cambia en la RAM del OS la constante de
suavizado del efecto: palabra `0x60097404` (dentro del programa `0x0`, 0x552 bytes en `0x60097364`, halfwords
`2fff 8a00` = `mov r6, #0.9998875@0:24`) → `0x28000000` (0,5, `asm/asm.py`), sólo si tiene el valor de 1.30.

| hallazgo | estado |
|---|---|
| El OS encola su set del maestro B (con `0x0`) **sólo al arrancar** desde sus descriptores en RAM; elegir el PLUG-OUT o pasar por el modo SYSTEM-1 no lo recarga (parche sin efecto hasta apagar) | [medido] |
| `create()` del PLUG-OUT del slot corre **al arrancar, antes** de esa carga, aunque el equipo arranque en modo SYSTEM-1: tras apagar y encender, el DSP tiene el valor parcheado aunque esa variante sólo lo escribió en la RAM | [medido: lectura del DSP] |
| Resultado: **la cuerda cambia de altura en < 10 ms** (antes ~140 ms, τ 90 ms): 367 → 734 → 184 muestras | [medido, `seq.wav`] |
| Memoria de programa del maestro B mapeada en **`0x40170000` + dirección en bytes** (en `+0` el efecto, `0C030FE0 A0000A03…`; en `+0x1000` nuestro principal); maestro A en `0x40150000`. Mismo formato que la imagen (dos palabras de 16 bits por palabra de 32, la alta primero); la primera palabra difiere (`00010001` en el DSP, `00000001` en la RAM) | [medido: lectura] |
| El ARM puede escribir esa memoria en vivo (`0x401700A0` leyó el valor escrito); su efecto en el audio no quedó medido (el retraso del USB, abajo, confundió la prueba) | [medido: lectura] / [abierto] |
| **El audio USB llega hasta ~2,7 s tarde** respecto del MIDI con el USB conectado (de siempre, según el usuario): las sondas buscan el inicio de la nota en vez de usar tiempos fijos (`ks_probe.sh`) | [medido] |

Consecuencia: **el `0x0` propio es posible sin encolarlo**. La región 0, en `create()`, pisa el programa del efecto
(o el descriptor `{0x0, 0x552, 0x60097364}` en `0x600971F0`, apuntándolo a un programa nuestro de la región 0) y el OS
lo carga él mismo al arrancar, con su secuencia. Así un `0x0` propio tiene `tapreq` (sólo ese programa lo recibe,
§3.3) y la regla de no encolar `0x0000` se respeta. Requisitos: mantener lo que el OS espera del efecto (cabecera,
llamadas `cond 25/26/27`, salto a `0x7FF2`, contador `A004`); el cambio sólo aparece **después de apagar y encender**
con el PLUG-OUT instalado, y vale para todos los modos hasta apagar (el modo SYSTEM-1 también corre ese `0x0`).

Del OS (lectura estática, agente 2026-11-03, sin medir): el ARM tiene un **puerto indirecto a la EDM**
(`0x40100050` dirección, `+0x54` control, datos en `+0x800`, `+0x7C` bit 8 para escribir; lo usa el autotest de
arranque `0x6001D51A`), es decir, la región 0 podría llenar tablas en la EDM. Ningún registro del ARM habilita `tapreq`
por programa: todo es por DSP. El OS marca los programas de un set reescribiendo su primera palabra `…ff71` → `…ff70`.

#### 3.3.3 `tapreq` propio: llamada desde el efecto del OS (medido 2026-11-03)

**Funciona.** `EDM_CALL=1` (build MP-EDMCALL: `KERNEL=sdk/s1cc/build/edm_pluck NAME=MP-EDMCALL EDM_KS=0 EDM_CALL=1
sh examples/system_s1cc/build.sh`) pide una toma propia de la línea del OS **desde dentro del programa de `0x0`**,
sin encolar `0x0` y sin microcódigo de Roland en el PLUG-OUT:

| pieza | dónde | qué |
|---|---|---|
| bloque | `microcode/edm_call.s` `edm_call_block`, escrito en el maestro B en el byte `0x0C00` (libre: el efecto termina en `0x552`, el principal empieza en `0x1000`) | las dos instrucciones del OS que la llamada reemplaza (copiadas **del DSP** en el momento) + `mov r2, #edad` / `hset h4, #256` / `hset h5, #(402E − 4000)/16384` / `f6 r2, -r4` / `tapreq r2, 0` / `cond 0, 15, 8` (retorno) |
| sitio | `edm_call_site`, sobre el segundo `tapreq` del efecto y la instrucción siguiente (5 medias palabras en el byte `0x164`, OS 1.30) | `cond 0, 10, 0` con ranura `0x0600` (llamada a `0x0C00`) + `mov r2, r2` de relleno |
| región 0 | `model.c edm_call`, en `load` | escribe el bloque y después el sitio en `0x40170000 + byte`, sólo si el sitio tiene el FNV-1a de 1.30 (si ya está parcheado o es otro OS, no toca nada) |
| principal | `edm.s master_edm` con `EDM_REQ` 0 | lee `402E`–`4030` (x0, x1, fracción) una muestra después y suma `0,7 ×` la toma al canal izquierdo |
| la línea | `EDM_TAP` en `model.c` | el delay del OS encendido con nivel y realimentación 0: sólo escribe la línea con nuestro envío |

Resultado: un golpe de ruido seco en los dos canales y **su eco sólo en el izquierdo a 48 007 muestras** (48 000
pedidas + 7 de latencia envío → línea → toma → lectura), ganancia 0,074 respecto del seco, sin repeticiones. El
`r4` del sitio es el contador de posición del efecto (su segunda toma usa la misma forma `f6 r2, -r4`), así que la
dirección es la edad en unidades de 16384 muestras.

Consecuencias:
* El `0x0` del OS sirve de **trampolín**: cualquier cantidad de pedidos propios en un bloque llamado desde ahí (las
  llamadas `cond 0, 10, 0` / retorno `cond 0, 15, 8` funcionan en el maestro B). Cuerdas por voz, chorus, granular:
  los pedidos en el bloque, la lógica en nuestro principal (lee los resultados una muestra después).
* El parche vive hasta apagar y vale en todos los modos; con otro PLUG-OUT el bloque sigue llamándose (pide una toma
  que nadie lee: inofensivo). Al apagar el OS vuelve a cargar su efecto.
* Pendiente: medir cuántos pedidos entran por muestra en el maestro B, y si conviene el arranque (tabla del OS,
  §3.3.2) en vez de la escritura en vivo (esta escribe con el DSP corriendo: tres palabras seguidas con
  interrupciones apagadas; no se observó ningún problema).

**Escritura propia de la EDM: todavía no** (pruebas MP-EDMW, `EDM_CALL=write`, `edm_call.s edm_call_block_w`,
2026-11-03; captura de 14–16 s con el retraso del USB descontado):

| prueba | escritura | lectura | resultado |
|---|---|---|---|
| 1 | nuestro envío `st → [4034]` con `h4 = 296` | izq.: `h4 = 296`, 0,5 s; der.: línea del OS, `h4 = 256`, 0,25 s | izq. **nada**; der. eco a 24 006 muestras |
| 2 | ídem, delay del OS apagado (`C001 = 0`) | izq.: `h4 = 256`, 0,5 s; der.: `h4 = 296`, 0,25 s | nada en ninguno (16 s) |
| 3 | sólo la del OS (`[4001]`, `C001 = 1`) | izq.: `h4 = 296`, 0,5 s; der.: `h4 = 256`, 0,25 s | der. eco (24 364 desde el inicio medido); izq. **nada** en 15 s |

Lectura: la base `h4` no es un corrimiento sobre un anillo que avanza solo (con 40 unidades de diferencia el eco
habría caído a ~7,3 s), y guardar en otra palabra par del banco 2 no escribe donde la leemos. El efecto lleva su
propia posición `p` (`r4`, `+[E013]` por muestra, vuelta a 35,15625) y pide en `base + edad − p`: la escritura de
`[4001]` cae en `base − p`, pero cómo recibe el hardware esa dirección no está visto (candidatas: la instrucción de
clase `0x06f0` sin decodificar antes de la primera toma, o un mapa de la ventana del banco 2 que pone el ARM: los
mapas 1–3 por DSP del controlador, `0x40100100…`). Siguiente paso posible: leer la EDM desde el ARM con el puerto
indirecto (`0x40100050` / `+0x54` / `+0x800`) para ubicar dónde cae la línea del OS.

**Resuelto en §3.3.4**: la dirección de cada escritura la pone la tabla de mapas del DSP, no el programa.

Lo que ya se puede sin escritura propia: todo lo que **lee** la línea del OS con tomas propias (chorus / ensemble,
multitoma, granular sobre lo que entra al delay), con la señal que nuestro principal manda a `2000` / `2002`.

Diseño: **las voces con memoria terminan en el maestro B.**

```
esclavo A/B (por voz)  excitación / señal seca de la voz v  ->  6008 + v  (hasta 6 voces)
maestro B (nuestro, 0x1000)  por voz: línea en la EDM: entrada = excitación + realimentación filtrada,
                             toma variable (tapreq) a la longitud de la nota (parámetro del ARM del maestro),
                             interpolada; suma de voces -> envío al efecto del OS (2000 / 2002)
```

| forma | qué corre en el esclavo | qué corre en el maestro B | voces |
|---|---|---|---|
| PLUCKED, cuerda de Plaits, SAW_COMB | el pulso / ruido de ataque y su envolvente | Karplus: línea + filtro de lazo de un polo + dispersión opcional, ~15 instr. por voz [estimado] | 6, a cualquier altura (la línea puede tener segundos) |
| BOWED, BLOWN, FLUTED | el arco / soplo (no lineal) necesita la muestra de la cuerda en la misma muestra: **no** se parte entre chips | — | sólo si el esclavo tiene la ventana |
| chorus / ensemble del *string machine* | las sierras divide-down (`mp_string_synth`) | 2–3 tomas moduladas por un LFO sobre la mezcla, ~20 instr. en total | todas: es un efecto global |
| Clouds / Beads (granular sobre un búfer) | la voz | búfer de segundos y granos leídos con tomas | global |
| Rings / Elements | la excitación | resonador modal (sin memoria: puede ir en el esclavo, `mp_modal`) o de cuerdas (en el maestro) | 4–6 |

Límites y riesgos:
* El maestro B ya ejecuta el efecto del OS (107 instr.) y nuestra mezcla; su presupuesto no está medido (los pasos
  14/14A de C-S1SY colgaron por formas de op no probadas). Cada `tapreq` cuesta un pedido y una lectura; en el S8
  entran ≥ 24 pedidos por muestra.
* La línea propia no puede pisar la del delay del OS: hay que saber qué tramo de la EDM usa (base 256, largo según
  DELAY TIME).
* **Lo primero que hay que medir**: si un `tapreq` de *nuestro* programa del maestro B (`0x1000`) lee la EDM o si la
  ventana sólo está conectada para el programa del OS en `0x0` (en el S8 dependía de la ranura).

Prueba propuesta (un PLUG-OUT de prueba, captura USB): en el principal del maestro B de SYSTEM-S1CC, escribir la
mezcla en la línea con la secuencia del programa del OS (mismo `h4`, una base lejos de la del OS), leer una toma fija
de 0,5 s y sumarla a la salida. Eco de 0,5 s en la captura = la EDM es nuestra desde el maestro B. Silencio = hay que
encontrar la conexión de la ventana en el ARM del maestro (o colocar nuestro código en la ranura `0x0`, reemplazando
el efecto del OS).

#### 3.3.4 Líneas propias: la tabla de mapas del banco 2 (medido 2026-11-03)

**Funciona.** Qué palabra del banco 2 se escribe en / se lee de qué dirección de la memoria de retardo no lo dice
el programa: lo dice una **tabla por DSP**, la del maestro B en `0x4016E000` (la del A en `0x4014E000`), 8 bytes por
entrada, 768 entradas (`0x1800` bytes):

| palabra 0 | palabra 1 | qué |
|---|---|---|
| `0x80000000 \| dirección` | `palabra << 16` | escritura: lo que el DSP guarda en `[4000 + palabra]` va a la dirección |
| `dirección` | `palabra << 16 \| largo` (en el DSP: largo 0) | lectura: `[4000 + palabra]` trae la muestra de esa dirección |
| `0x807FFFFF` | `0x007FFFFF` | libre |

Las direcciones son relativas a un puntero que avanza solo: una lectura en `escritura + d` da la muestra escrita
`d` muestras antes. La unidad es la muestra (256 en las unidades de `h4` = `0x400000`).

| hallazgo | dónde | estado |
|---|---|---|
| El OS carga su tabla para el maestro B con un set de datos `{0x0, 0x110 bytes}` (34 entradas, origen `0x60097928`) + `{0x110, 8 bytes = 0}` + `{0x17F8, 0x807FFFFF 0}`, desde el descriptor `0x60060280` | OS 1.30 | [código] |
| Entradas del OS: `[4001]` → `0x400000` (la escritura del delay, base 256); `[400a]` → `0x48CA00` (entrada de la reverb, 576 000 muestras después: el largo máximo del delay y la vuelta de `p`); las líneas de la reverb: palabra par escribe, impar lee (`[400e]` → `0x48F340`, `[400f]` ← `0x48FB5C`, …) | `0x60097928` | [código] |
| En el DSP la tabla viva no es igual a la copia de la RAM: las lecturas del OS pierden el largo y tres de ellas están corridas −2 / +1 muestras (`0049aa4c 002c23e9` → `0049aa4a 002c0000`); las nuestras, cargadas igual, quedan con la dirección exacta y largo 0 | PARAMLOG `edm_ecf_dump` | [medido] |
| **Escribir la tabla directo desde el ARM** (stores a `0x4016E000 + 8k`, con el DSP corriendo, direcciones `0x4A0000` / `0x4C0000`) **apagó el equipo** al elegir el PLUG-OUT. Con el set de datos por `FC3C` (la cola de datos del maestro B) y direcciones dentro del tramo del delay del OS: sin problema. No se separó cuál de las dos cosas causó el apagado | MP-EDMECF | [medido] |
| `[4034]` → `0x440000`, `[4035]` ← `0x44BB80` (48 000) y `[4036]` → `0x460000`, `[4037]` ← `0x465DC0` (24 000), escritas y leídas por el **principal en `0x1000`** con `st` / `ld` comunes: eco izquierdo a **48 001** muestras y derecho a **24 001**, ganancia 0,700 (la pedida) | `ks_probe`-style, `p2.wav` | [medido] |

Cómo se hace (`EDM_ECF=1`, build MP-EDMECF: `KERNEL=sdk/s1cc/build/edm_pluck NAME=MP-EDMECF EDM_KS=0 EDM_ECF=1
sh examples/system_s1cc/build.sh`):

| pieza | qué |
|---|---|
| `model.c edm_ecf`, en `load` | lee la tabla (entradas 0 y 33 del OS 1.30; 34–38 en cero, libres o nuestras) y encola `{0x110, 40 bytes}` con `FC3C`: dos pares escritura / lectura + la entrada en cero detrás. Si ya están, no encola |
| `microcode/edm_ecf.s master_ecf` | principal del maestro B: `st` del envío a `[4034]` y `[4036]`, `ld` de `[4035]` y `[4037]`, × 0,7 a la salida |
| duración | hasta apagar (el OS vuelve a cargar su tabla al arrancar); con otro PLUG-OUT las entradas siguen pero nadie usa esas palabras |

Consecuencias:
* **Cada voz puede tener su línea**: una entrada de escritura y una o dos de lectura por voz, en palabras libres del
  banco 2 (`4034` en adelante). No hace falta `tapreq` ni tocar el programa de `0x0`.
* Largo fijo por entrada. Para la altura de una cuerda: (a) `tapreq` sobre la línea propia (con la convención del OS,
  `h4` = dirección de escritura / 16384, `f6 r2, -r4`; sin medir sobre una línea propia); (b) reescribir la entrada de
  lectura en cada nota con otro set de datos `FC3C` (el set se aplica con la cola: latencia sin medir); (c) varias
  lecturas fijas e interpolación.
* El tramo usado (`0x440000`–`0x465DC0`) está dentro del delay del OS: sus tomas de más de 262 144 muestras (2,7 s)
  leerían nuestras líneas. Falta medir cuánto mide la EDM del S1 más allá de `0x49B0DA` (la prueba en `0x4A0000`
  fue la que apagó el equipo, junto con la escritura directa).


#### 3.3.5 Una cuerda Karplus-Strong por voz (MP-EDMSTR: **funciona, 6 voces**, 2026-11-04)

**Estado (2026-11-04): funciona en el equipo, 6 voces, «suena muy lindo» (usuario).** Build:
`sdk/s1cc/s1cc build sdk/s1cc/examples/edm/edm_string.c && KERNEL=sdk/s1cc/build/edm_string sh
examples/system_s1cc/build.sh` (`EDM_STR=1 EDM_STR_ONE=1` vienen del `S1_BUILD` del kernel) → `plugins/MP-EDMSTR`
(el build dice «8 voces»: las del esclavo; el asignador de la región 0 usa las 6 cuerdas).

La forma que funciona (`EDM_STR_ONE`, la última de la tabla de pruebas de abajo):

| pieza | qué |
|---|---|
| tabla de mapas | sólo las escrituras, una por voz (`[4040 + 8v]` → `0x440000 + 0x4000 v`), encoladas con `FC3C` en `load`: la tabla **no cambia después** |
| lecturas | un `tapreq` por voz en el bloque del trampolín (`edm_call.s edm_str_block1`, §3.3.3): `ld r2, [C030 + v]` (la edad, que la región 0 escribe con `FCEC` en cada nota), `hset h4` = base de la línea, `hset h5` → `4080 + 8v`, `f6 r2, -r4`, `tapreq` |
| lazo | `edm_str.s master_str1`: `z = x0 + g (x1 − x0)` (`g` en `C020 + v`), `s += b (z − s)` (estado en el banco 0 `0300 + 2v`), `w = x + fb s` |
| afinación | `make_s1cc.py str1_tune`: `d` y `g` tales que `d + 1` + el retardo de fase de los dos filtros en la nota = el período; tablas por 16 pasos de FILTER CUTOFF (`b` 0,25 … 1); mover CUTOFF reafina las voces |
| verificación sin el equipo | `sdk/s1cc/examples/edm/str1_sim.py`: ±0,5 cents (la nota 91 con `b` 0,25 se apaga antes de medirse) |

Pruebas en el equipo (2026-11-04), en orden:

| build | qué cambia | resultado |
|---|---|---|
| MP-EDMSTR | 4 lecturas por tabla por voz, retoque por `FC3C` en cada nota | suena, **altura fija**: la cola sólo se vacía al elegir el PLUG-OUT |
| MP-EDMSTRL | ídem + volcado PARAMLOG de la tabla | confirma: las entradas no cambian después de `load` |
| MP-EDMSTRW | dirección de lectura escrita directo en la tabla en cada nota | **colgado** en la primera nota |
| MP-EDMSTRT | lecturas por `tapreq` (2 por voz), 8 voces: 16 pedidos | **colgado** al elegirlo |
| MP-EDMSTR1 | ídem, 1 voz (2 pedidos) | funciona |
| MP-EDMSTR4 | 4 voces (8 pedidos) | **colgado** al elegirlo |
| MP-EDMSTR1P | 1 voz + el bloque rellenado (sin ejecutar) al tamaño del de 4 voces | funciona: el tamaño no es |
| MP-EDMSTR2 | 2 voces (4 pedidos, bases 272 / 273) | funciona |
| MP-EDMSTR3 | 3 voces (6 pedidos) | funciona |
| MP-EDMSTR6 | `EDM_STR_ONE`: 6 voces, 1 pedido cada una | **funciona** |

El DELAY del panel (el efecto del OS, su línea en `0x400000`) **sigue funcionando junto con las cuerdas** [medido
2026-11-04, usuario]: nuestras líneas (`0x440000`–`0x454000`) y su toma conviven.

**Límite medido: 6 `tapreq` nuestros por muestra en el maestro B funcionan, 8 cuelgan** (más los 2 del delay del
OS). `make_s1cc.py` rechaza más de 6. Bajo el límite, las opciones `EDM_STR_TAP`, `EDM_STR_NV` y `EDM_STR_PAD`
quedan para repetir las pruebas.

**SAW_COMB (MP-SAWCOMB, `EDM_STR_COMB=1`): funciona en el equipo** (2026-11-04, «suena bien»). Kernel
`sdk/s1cc/examples/edm/edm_sawcomb.c`: la sierra de `mp_saw.c` con el filtro y el VCA de `mp_dsp_b.h` en el esclavo;
en el maestro B el peine de Braids (`digital_oscillator.cc RenderComb`) sobre las mismas líneas: retardo = período
de la nota + OSC1 COLOR (±64 semitonos, el TIMBRE de Braids), realimentación = OSC2 COLOR (bipolar, curva
`0,97 tanh(2x)/tanh 2` en lugar de `ws_moderate_overdrive`), sin filtro en el lazo (`b` = 1), salida `x + z`
(`master_str1` con `.if STR_COMB`). Emulador: impulso 0,5 → ecos 0,5, 0,4, 0,32 al período (366,94 muestras, nota 60).
Los dos COLOR los escribe la región 0 desde la perilla: **no se modulan** (COLOR MOD no llega al maestro).

**Cuerda de Plaits (MP-PLSTRING, `EDM_STR_PL=1`): funciona en el equipo** (2026-11-04, «suena hermoso»). Kernel
`edm_plstring.c` (golpe de ruido de 5 ms por un polo: OSC1 COLOR = brillo de la excitación, el TIMBRE del motor de
cuerdas de Plaits); maestro `edm_str.s master_pstr`, 6 voces:

| pieza | qué | por qué |
|---|---|---|
| amortiguación | dos polos iguales `b` en serie a 24 … 84 semitonos sobre la nota (FILTER CUTOFF, 8 pasos) | el SVF de Plaits con Q 0,5 es eso; con 12 semitonos (el mínimo de Plaits) sólo se oía el golpe |
| dispersión | tres allpass de primer orden `y = a (u − y1) + x1` (OSC2 COLOR, 8 pasos) | Plaits usa un allpass sobre otra línea: otro `tapreq` por voz, más que los 6 por muestra. Con `a` fijo el estiramiento iba de nada (graves) a semitonos (agudos): `a` por nota para que el 8º parcial suba `300 amt³` cents (medio: +60, ¾: +236, máximo: +420, igual en todas las notas) |
| salida | la cuerda sola, sin el golpe | como Plaits |
| afinación | `make_s1cc.py pl_loop`: bisección con la fase de cada etapa sumada (sin vuelta de 2π) | emulador `pl_sim.py`: ±2,5 cents (resolución) |

Medido en el equipo (OSC2 COLOR, nota 48, `disp.wav`): mínimo, parciales a +1 cent; máximo de la versión de 50
cents: 2º +5, 4º +18, 8º +55 (lo calculado) → poco audible; de ahí la curva `300 amt³`.

**BLOWN (MP-BLOWN, `EDM_STR_BLOWN=1`): funciona** (2026-11-04, «suena bien»). Braids `RenderBlown` (caña + tubo, 6
voces) en `master_blown`; el soplo se hace en el esclavo (`edm_blown.c`). Cambios contra Braids, medidos:

| qué | Braids | aquí | por qué |
|---|---|---|---|
| presión | 0,8 fija | OSC1 MIX 0,35 … 0,9 | |
| ruido del soplo | ±0,68 … 0,29 de la presión | OSC1 COLOR 0 … 0,25 | Braids en el equipo: «cuadrado con ruido», ruido a −30 dB entre armónicos |
| pérdida del tubo | ninguna (el promedio de dos muestras) | un polo en la reflexión a 18 … 66 semitonos sobre la nota (OSC2 COLOR, 8 pasos) | sin ella la caña da una cuadrada a cualquier presión (H3 −10, H5 −16, H7 −15 dB, medido); con ella: H3 −20 / H5 −38 (suave) … −11 / −18 (clarinete) … −10 / −14 (cuadrada) |
| cuerpo | `lut_flute_body_filter` por nota + COLOR | la misma tabla (`30 · 2^(i/12) / 4096`) en la nota | |

Afinación: el lazo (línea + promedio + polo) = medio período (la reflexión invierte), `blown_loop`; emulador
`blown_sim.py`: ±4 cents, armónicos pares a −45 … −68 dB.

**BOWED (MP-BOWED, `EDM_STR_BOWED=1`): funciona** (2026-11-04, 3 voces: **dos líneas por voz**, mástil y puente =
6 `tapreq`, el límite). Braids `RenderBowed` en `master_bowed`: el arco (`edm_bowed.c`: 0,16 × OSC1 MIX × la
envolvente AMP) contra la velocidad de la cuerda; fricción `min(1, (|Δv| s + 0,75)^−4)` (`lut_bowing_friction` es
la tabla de STK, ajustada exacta) con `1/y` por semilla cuadrática + 2 pasos de Newton (error 7·10⁻⁴); `absadd`
(`|x| + Y`) en una op. OSC1 COLOR = presión del arco (`s = p0/32`), OSC2 COLOR = posición (parte del puente
`(6 … 69)/256`, 8 pasos).

| medido en el equipo | qué | arreglo |
|---|---|---|
| «metálico, como ring mod» | el cuerpo de Braids (bicuadrático a 493 Hz, `out = t − y1`: pasabanda) dejaba la fundamental de C3 15 dB bajo el 7º armónico | los mismos polos como pasabajos de ganancia 1 (`gin = 1 − p1 − p2`): la fundamental arriba |
| período 2 muestras largo en todas las notas | una muestra más de latencia por línea que la del modelo (`a + 1 + g`) | `bowed_loop` resta 2: C2 +0,8, C3 0,0, C4 +1,2, C5 +0,6 cents |
| pico 1,0 con ganancia 2,4 | | 1,2: pico 0,62 |

El 6º armónico muy bajo con OSC2 COLOR al centro es el de la cuerda real frotada a 1/6. Pendiente: medir si las
formas de una línea (MP-EDMSTR, MP-PLSTRING, MP-BLOWN) también quedan una muestra largas (lo calculado con el
emulador era exacto; en el equipo no se midió la afinación fina).

Plan para COLOR MOD en el maestro (sin implementar, 2026-11-04):
* Transporte: el esclavo manda al maestro B 8 palabras probadas, `6006`–`600D`; 6 ya llevan el audio de las voces.
  `6026`–`602D` las lee el **maestro A** en los sintes de serie ([../system-1/WAVE_BUS.md](../system-1/WAVE_BUS.md)),
  no sirven para el B. Quedan `600C` / `600D`: **multiplexar** los 12 colores (6 voces × 2) en ellas, `600C` =
  valor, `600D` = número de ranura / 16; cada color se renueva cada 12 muestras (8 kHz).
* Esclavo (DSP B, `host_b.s`): los colores de cada voz (`s1_color(0/1)` del kernel por `s1_export`, o las palabras
  de COLOR MOD del anfitrión) en un bloque del banco 0; un contador de ranura en el estado; `hset h6` desde el
  registro + `ldidx` (la forma de `color.s colors_idx`, medida en el S1) → `600C`, el contador → `600D`.
* Maestro B (`edm_str.s`): por ranura j, `sel = max(0, 1 − K |tag − j/16|)`, `c_j += sel (valor − c_j)` (estado en
  el banco 0, ~8 instrucciones × 12); por voz `edad = base_v · 2^(−10,67 (c0 − 0,5))` (`base_v` = período / 16384 por
  `FCEC` en cada nota; la división exponencial del SFU como la emite s1cc `exp2split`), recortada a
  `[0, 0,9997]` (las líneas están a 16 384 muestras), y la fracción del propio `tapreq` (`R + 2`, 1/64) en lugar de
  `g`; `fb = 0,97 r (1,5 − 0,5 r²)`, `r = 2 c1 − 1`. El bloque del trampolín lee la edad del banco 0 (escrita por
  el principal en la misma muestra) en lugar de `C030 + v`. Unas 190 instrucciones más en el maestro B (el SH-2
  tiene 406 en el mismo lugar).

Lo que sigue de esta sección describe el primer diseño (lecturas por tabla), que quedó como registro.
Variante con volcado de la tabla en cada nota: `NAME=MP-EDMSTRL PARAMLOG=1` (id `0x3D00` estado, `0x3D01 + k`
entradas 33–44, 73–75; `examples/system_1/paramlog.py`).

| pieza | dónde | qué |
|---|---|---|
| excitación | `edm_string.c` (esclavo, DSP A del kernel) | golpe de ruido de 1–16 ms (OSC1 COLOR) × OSC1 MIX en cada nota, como `edm_pluck.c` |
| transporte | `host_b.s head_b`, `.if EDM_STR` | sin mezcla: la salida de la voz v × `C000` → IOMD `6006 + v` (8 palabras, las que el mezclador de serie del OS lee en el maestro: [código], §1 de [../system-1/WAVE_BUS.md](../system-1/WAVE_BUS.md)) |
| líneas | `model.c edm_str`, en `load` | una cola de datos `FC3C` `{0x110, 41 × 8 bytes}`: por voz una escritura `[4040 + 8v]` → `0x440000 + 0x4000 v` y cuatro lecturas `[4041/43/45/47 + 8v]` ← esa dirección `+ d … + d + 3`; una entrada en cero detrás (entradas 34–74 de la tabla). Palabras pares escriben, impares leen (la convención del OS) |
| altura | `model.c edm_str_note`, en cada nota | otra cola `FC3C` con las cuatro lecturas de la voz (`d` de la nota) y `f` en `C020 + v` (`FCEC`) |
| lazo | `microcode/edm_str.s master_str` (principal del maestro B, 220 instrucciones, una por línea como `edm_ecf.s`) | `F0 = a (y0 + y2) + c y1`, `F1 = a (y1 + y3) + c y2` (filtro de fase lineal `[a, 1 − 2a, a]`, 1 muestra), `z = F0 + f (F1 − F0)`, `w = x + fb z` → `[4040 + 8v]`; la suma × 0,5 × VOLUME → envío del efecto |
| panel (región 0) | `edm_str_param` | AMP DECAY → `fb` con la tecla apretada (`1 − 10^−(1 + 3v/255)`, 0,9 … 0,9999 por vuelta), AMP RELEASE → `fb` suelta (`1 − 10^−(0,3 + 3v/255)`), FILTER CUTOFF → `a = 0,25 (1 − v/255)` (`C01D`, `1 − 2a` en `C01E`) |

Período: las lecturas dan `y_k = w[n − d − 1 − k]` (la lectura llega una muestra después de su entrada: 48 001 para
48 000, §3.3.4), el filtro está centrado en `y1` y la interpolación suma `f`: **período = d + 2 + f**
(`make_s1cc.py str_period`; nota 0: d = 11 739, cada línea tiene 16 384 muestras de lugar).

Verificado sin el equipo: `sdk/s1cc/examples/edm/str_sim.py` corre `build/s1mm1.s` en el emulador C con un modelo
de la tabla (las lecturas como arriba): 8 cuerdas a la vez (notas 48 … 84), alturas a ±0,9 cents (resolución de la
FFT). Prueba en el equipo: `sdk/s1cc/examples/edm/str_probe.sh` (seis notas de a una, cada una en otra voz, y un
acorde de tres; imprime la altura de cada una).

**Medido 2026-11-04: no cambia de altura.** Todas las notas suenan igual y un acorde suena como una nota. MP-EDMSTRL
(volcado de la tabla en cada nota): las entradas cargadas en `load` están (estado 1, `[4041]` ← `0x44016C` = `0x440000`
+ 364, la nota 60), pero **después de cada nota siguen iguales**: la cola de datos `FC3C` sólo se vacía al elegir el
PLUG-OUT. Leído en el OS 1.30: `0x6001ECA2` vacía las cuatro colas (`FC30`–`FC3C`, vectores `{capacidad, cantidad,
lectura, datos}` en `0x604C0C70`–`0x604C0CA0`), encola los sets del OS para el modo PLUG-OUT, llama a `load` (vt+0x08) y
después el OS carga todo; durante la ejecución nada vacía las colas [código + medido].

**MP-EDMSTRW** (`EDM_STR_DIRECT=1`): en cada nota, la palabra de dirección de las cuatro lecturas de la voz escrita
directo en la tabla (un `str` de 32 bits por entrada, dentro del tramo cargado): **el equipo se colgó en la primera
nota** (2026-11-04; hubo que apagarlo). Con la escritura directa de §3.3.4 (apagado) son dos fallas: **la tabla no se
escribe desde el ARM con el DSP corriendo**, ni una palabra. Queda: la altura por `tapreq` desde el trampolín (§3.3.3),
con la tabla fija (sólo las escrituras de cada voz) y dos pedidos por voz (edad y edad + 2: `y0`…`y3` + `f` como ahora),
que depende de que `tapreq` sirva sobre una línea propia (MP-EDMIDX, §3.3.6).

Qué puede fallar en el equipo (cada punto es una pregunta de la prueba):
1. **La cola `FC3C` en medio de la ejecución**: §3.3.4 sólo la usó en `load`. Si las colas de cada nota no se
   aplican, todas las voces quedan en la nota 60 (la carga) y sólo cambia `f`: suena casi la misma altura. MP-EDMSTRL
   lo muestra (las entradas 35–38 deben cambiar de una nota a la siguiente).
2. **Cuántas entradas atiende la tabla por muestra**: el OS usa 34; con 8 voces son 75. Si el DMA no llega, se
   oirán líneas mudas o ruido en las voces altas (las entradas que pasan del límite).
3. `6008`–`600D` desde nuestro esclavo hasta nuestro maestro: leído en el OS, no medido con un PLUG-OUT propio.
4. Presupuesto del maestro B: efecto del OS (107) + 220 nuestras; el SH-2 tiene 406 en el mismo lugar.

Límites: la flexión de altura (bend) no mueve la cuerda (sólo la nota); las entradas quedan hasta apagar y siguen
escribiendo `0x440000`–`0x460000` con otro PLUG-OUT (el delay del OS las lee sólo con más de 2,7 s).

#### 3.3.6 ¿La memoria de retardo como memoria indexada? (WAV, wavetable, granular; análisis 2026-11-03)

Pregunta: cargar un WAV en la EDM y leerlo avanzando la posición (wavetable, granular). Modelo de la memoria que
sale de §3.3.3 / §3.3.4 [medido salvo donde dice]:

* Las entradas de la tabla están en coordenadas de una **posición que el hardware mueve sola** (P): la escritura en
  `A` cae cada muestra en la dirección absoluta `P(t) + A`, y la lectura en `A + d` trae lo escrito `d` muestras
  antes. Es decir, **cada entrada de escritura recorre toda la EDM una vez por vuelta** de P (largo de la vuelta sin
  medir: ≥ 635 098 muestras, `0x400000`–`0x49B0DA`; la EDM entera son 2 097 152 palabras, abajo).
* `tapreq` del OS pide `base + edad − p`, con `p` su propio contador (`+[E013]` por muestra, vuelta a 576 000). Si el
  pedido fuera relativo a P como las entradas, `p` sobraría: lo más probable es que **el pedido sea una dirección
  absoluta** y `p` la copia por software de P [inferido; en el S8 `f5c` es la posición de escritura y las tomas
  restan esa posición, [DSP_USOS.md](DSP_USOS.md) §5]. Prueba MP-EDMIDX (abajo).
* `tapreq` sólo se atiende en el programa de `0x0`: los pedidos van en el bloque del trampolín (§3.3.3), que puede
  cargar la dirección de memoria (`ld r2, [...]` que nuestro principal escribió; en el sitio sólo `r2` está libre),
  ~5 instrucciones por pedido. Resultado una muestra después, dos muestras vecinas + fracción (en el S8 1/64,
  truncada; ≥ 24 pedidos por muestra).

| uso | qué necesita | estado |
|---|---|---|
| lectura en una posición calculada: granular, *scrub*, velocidad variable, lectura de una wavetable | `tapreq` con la dirección de un registro, en el trampolín | **posible**: el pedido con edad inmediata funciona (§3.3.3); de un registro es la misma instrucción. Sobre una línea propia, sin medir (MP-EDMIDX) |
| búfer grabado en vivo (Clouds / Beads, looper, granular sobre lo que se toca) | una escritura propia (§3.3.4) + pedidos con la edad de cada grano | **posible ya** con las piezas medidas: la edad crece una muestra por muestra con P; un grano que avanza a velocidad r pide `edad₀ + (1 − r) t` |
| congelar el búfer (*freeze*) | dejar de escribirlo: reencolar su entrada de escritura con otra dirección (`FC3C`) | sin probar. Dura hasta que pase el escritor siguiente de dirección menor: con la línea en `0x440000`, el del delay del OS (`0x400000`) llega a los 262 144 muestras (**2,73 s**) |
| **WAV fijo cargado desde el PLUG-OUT** | (1) escribirlo: por el puerto indirecto del ARM (`0x40100050` / `+0x54` / `+0x800`, lo usa el autotest de arranque; sin probar en el S1, en el S8 la escritura no se logró) o grabarlo por el DSP en tiempo real con datos que el ARM entrega a 96 palabras por ms (`FCEC` no da: ~33 por ms); (2) que **ningún escritor lo pise**: todos barren la EDM entera por vuelta, así que hay que sacar las escrituras del delay y la reverb del OS (reencolar sus entradas como libres, y volver a poner las de la RAM del OS, `0x60097928`, al salir) y no tener líneas propias | **abierto**: la parte más difícil; además el WAV entra en ~0,94 MB por región del PLUG-OUT (16 bits: ~4,9 s a 96 kHz) |
| wavetable chica (≤ 1024 palabras) | banco 5 + `ldb5` (§4) | más simple que la EDM; la EDM sólo para tablas grandes (Plaits: 4 × 64 × 132 = 33 792 palabras) |

Conclusión: **granular y lectura variable sobre un búfer que se graba en vivo sí** (falta medir el pedido sobre una
línea propia); **un WAV fijo todavía no**: hace falta escribir la EDM desde el ARM y apagar los escritores del OS.
Sin el puerto del ARM queda un llenado lento por el DSP [estimado, sin probar]: una entrada de escritura en `A` y
una de lectura en `A + V` (V = largo de la vuelta: la misma dirección absoluta una vuelta antes) hacen una línea que
se recircula a sí misma; el principal escribe la muestra del WAV que el ARM dejó en el banco 5 cuando le toca a esa
dirección y si no la que leyó (`ld` + `st`). El ARM entrega ~30 palabras por ms, así que cada vuelta llena ~1/3 de
las posiciones: el WAV entero en ~3–4 vueltas (≥ 20 s), con los escritores del OS apagados mientras dure. Falta
medir V (la vuelta de P) antes de intentarlo.

**El puerto del ARM a la EDM, leído del autotest de arranque** (`0x6001D51A`, OS 1.30; llamado una sola vez desde
la máquina de estados del arranque `0x60019FB0`, caso que también inicializa el objeto de la EDM `0x604BB40C`)
[código, sin medir]:

| paso | qué hace |
|---|---|
| antes | servicio del ROM `0x01008BDF(0x20003534, 1, i, 0x17, 0)` para i = 0…23 (24 rutas a `0x17`), `0x0100A211(0x20003534, k, 0)` para k = 0…3 (**los 4 mapas**: las ventanas de los DSP, guardadas en el objeto `+8…+0x14`), `0x01009F13(0x20003534, 0x40000000)` (el argumento es un float, 2,0: ver §3.3.7) |
| escritura de la palabra n | `[0x40100050] = 0x01000000 + 4n`; `[0x4010007C] \|= 0x100`; esperar `[0x40100008]` bit 30 en 0; `[0x40100800] = dato`; `[0x4010007C] &= ~0x100`; esperar otra vez |
| entre las dos pasadas | modo `0x41200000` |
| lectura de la palabra n | `[0x40100050] = 0x01000000 + 4n`; `[0x4010007C] \|= 0x100`; esperar; `[0x40100054] \|= 0x20000000`; esperar `[0x40100054]` bit 31 en 0; dato = `[0x40100800]`; `[0x4010007C] &= ~0x100`; esperar |
| datos | n = 0 y n = 2^k para k = 0…21, dato = n (prueba de las líneas de dirección y de datos) |
| después | los 4 mapas desde el objeto, las 24 rutas a `i → i`, modo `0x40000000` |

Consecuencias: **la EDM del S1 tiene 2^21 = 2 097 152 palabras** (21,8 s a 96 kHz, como un BMC del S8); el ARM la
escribe palabra por palabra con espera de ocupado, pero el OS lo hace **con los mapas de los DSP y las 24 rutas
desconectados**. Usarlo con el audio corriendo (en `load`, para cargar un WAV) significa repetir ese marco: un corte
del audio mientras dura y el riesgo de apagado que ya dio la escritura directa de la tabla (§3.3.4). Prueba a hacer
antes de cualquier escritura: una lectura de pocas palabras con el marco completo en un build PARAMLOG.

**Sonda MP-EDMIDX** (armada, sin probar): `KERNEL=sdk/s1cc/build/edm_pluck NAME=MP-EDMIDX EDM_KS=0 EDM_ECF=1
EDM_CALL=idx sh examples/system_s1cc/build.sh`; captura `sdk/s1cc/examples/edm/idx_probe.sh`.

| pieza | qué |
|---|---|
| escritura | `EDM_ECF`: `[4034]` → `0x440000` (unidades de `h4`: 272) recibe el envío izquierdo (`master_edm` con `EDM_WR`) |
| pedido izquierdo | `edm_call.s edm_call_block_idx`: `h4 = 272`, `r2 = 48000/16384`, `f6 r2, -r4`, `tapreq` → `402E` |
| pedido derecho | `h4 = 272`, `r2 = 0`, `f6 r2, -r2` (la base sola: **una dirección fija**), `tapreq` → `4031`; `h4` vuelve a 256 antes del retorno |
| salida | seco + 0,7 × cada toma, izquierda / derecha |

Lectura del resultado: izquierda con eco a ~48 007 = la convención del OS vale para una línea propia (edad − p).
Derecha − izquierda antes del eco: **escalones lentos** (cambian una vez por vuelta) = el pedido es absoluto, y
entonces una posición fija o que avanza a cualquier velocidad lee un búfer quieto (la base de wavetable / WAV);
**una copia del golpe a pocas muestras** = el pedido es relativo a la posición de escritura.

#### 3.3.7 El puerto del ARM a la EDM: sonda MP-EDMPORT (2026-11-04: el puerto con los DSP corriendo **mata el audio**)

Pregunta: ¿puede la región 0 (el ARM del maestro) **escribir una tabla o un sample entero en la EDM** con los DSP
corriendo, y que dure (≥ 1 s) para leerla después desde el DSP? Es el camino para samples y wavetables grandes sin
reducirlos (una tabla de Analog de 92 frames = 188 416 muestras; la EDM tiene ≥ 2^21 palabras).

Lectura del autotest `0x6001D51A` desensamblado (OS 1.30, 2026-11-04) [código], que corrige el cuadro de §3.3.6:
* `0x01009F13(obj, x)` (veneer `0x6003CD0E`) recibe un **float**: 2,0 antes de escribir, 10,0 entre las pasadas,
  2,0 al final; en otras partes del OS 1,0, 3,0 y 40,0. No es un "modo del controlador"; probablemente un tiempo
  (espera) [inferido, el ROM `0x01000000` del S1 no está volcado].
* La prueba recorre n = 0 y n = 2^k para k = 0…21 (`lsl r6=4, i−1` con i < 0x17): **22 líneas de dirección**. Si la
  prueba pasa en el arranque, la EDM tiene **≥ 2^22 = 4 194 304 palabras (43,7 s)**, no 2^21 [inferido: no se vio qué
  hace el OS si falla, `0x60040D5A(obj, 0x40, 0)` sólo apaga un bit de estado].
* Los mapas se restauran con `0x0100A211(obj, k, [0x604BB40C + 8 + 4k])` (`0x6001D4E4` con 0) y las rutas con
  `0x01008BDF(obj, 1, i, i, 0)`.

Sonda (`model.c edm_port_probe`, opción `EDM_PORT` de `build.sh`, requiere `PARAMLOG=1`). Escalón 1: la secuencia del
puerto **sin** desconectar mapas ni rutas (los DSP siguen sonando); toda espera tiene un límite (cuenta un
*timeout* en lugar de colgar el ARM).

| build | qué hace en cada nota | ids de PARAMLOG |
|---|---|---|
| MP-EDMPORTR (`EDM_PORT=r`) | lee 64 palabras `k · 0x10000` (k = 0…63: 4M palabras) y 16 palabras de `EDM_PORT_W0` (por defecto `0x380000`) | `0x3E00` (flag 0: notas, 1: timeouts), `0x3E40 + k`, `0x3E80 + i` |
| MP-EDMPORTW (`EDM_PORT=w`) | lo mismo; en la **primera** nota escribe 64 palabras `0x5A0000 + i` en `EDM_PORT_W0 + i` y las relee en el acto | además `0x3EC0 + i` |

Builds: `KERNEL=sdk/s1cc/build/mp_saw NAME=MP-EDMPORTR PARAMLOG=1 EDM_PORT=r sh examples/system_s1cc/build.sh`
(ídem `NAME=MP-EDMPORTW EDM_PORT=w`). Captura: `python3 examples/system_1/paramlog.py -t 20` mientras se tocan
notas (con DELAY del panel arriba, para que el delay del OS escriba la EDM).

Cómo se lee el resultado:

| observación | significa |
|---|---|
| timeouts > 0 | el puerto no responde con los DSP corriendo: escalón 2 = el marco del autotest (mapas y rutas desconectados durante el acceso, corte de audio) |
| R: palabras distintas de 0 que cambian de nota a nota en unos `k` | ahí escribe el delay / la reverb del OS: el tramo que barren los escritores y la relación entre `n` del puerto y las direcciones de la tabla de mapas (`0x400000`…) |
| R: el resto en 0 y fijo | memoria que nadie escribe: el lugar de una tabla persistente |
| W: `0x3EC0 + i` = `0x5A0000 + i` | **el ARM escribe la EDM** |
| W: `0x3E80 + i` sigue igual en las notas siguientes (segundos después) | la tabla **dura**; si se borra, un escritor del OS pasa por ahí (se ve en R cuál) |

**Medido 2026-11-04** (captura PARAMLOG, notas por MIDI canal 5):

| build | resultado |
|---|---|
| MP-EDMPORTR (80 lecturas por nota, sin restaurar registros) | sin timeouts, **todas las lecturas 0**; después de la primera nota **el equipo queda mudo** (PLUG-OUT y modo SYSTEM-1) hasta apagar |
| MP-EDMPORT0 (control: el mismo build sin la sonda) | suena, después de apagar y encender |
| MP-EDMPORTD (`EDM_PORT=d`: **una** lectura de la palabra 0, con `0x50` / `0x54` / `0x7C` restaurados) | lectura 0, sin timeouts; **mudo igual** (todo el equipo, hasta apagar) |

Conclusión: **el acceso del ARM a la EDM por el puerto con el DMA de los DSP activo traba la EDM y el audio**; es la
razón del marco del autotest (rutas y mapas desconectados) y coincide con las escrituras que nunca llegaron en el S8
(NOTES.md «Muestras largas»). No se intenta W en esta forma.

Registros del controlador `0x40100000` (32 palabras, antes / después de la lectura) [medido]:

| palabra | valor | interpretación |
|---|---|---|
| `+0x00` | `0x45534D39` («ESM9» en ASCII), fijo | identificador del chip [inferido] |
| `+0x48` | ~7 000 000 … 7 570 000, **decrece a 96 kHz** (−386 en ~4 ms entre antes y después; −69 749 entre dos notas a ~0,73 s) | contador de muestras hacia atrás: probablemente **P**, la posición que mueve la EDM (§3.3.6); su rango (< 2^23) coincide con las direcciones `0x4xxxxx` de la tabla de mapas [inferido] |
| `+0x4C`, `+0x60`, `+0x78` | cambian solos (123…713, ~1250, 256…135 185) | estado del DMA [abierto] |
| `+0x50` | `0x01000000` antes de la lectura | el registro de dirección del puerto (palabra 0) |
| `+0x64` | `0x5FF`, fijo | [abierto] |
| `+0x7C` | `0x11` (a veces `0x10011`) | control; bit 8 (`0x100`) = el pedido del ARM |
| el resto | 0 | — |

Escalón 2 (sin armar): el marco completo del autotest en `load` (rutas a `0x17`, mapas a 0, el valor de cada mapa
guardado en `0x604BB40C + 8 + 4k`, espera; acceso; mapas y rutas restaurados), primero una sola palabra.

Siguiente escalón si W funciona: la tabla del WAV desde la región 0 (16 bits, ~0,94 MB por región) en el tramo libre,
y la lectura desde el DSP con `tapreq` en el trampolín (§3.3.3) a una dirección fija (la forma del pedido derecho de
MP-EDMIDX, §3.3.6), hasta 6 pedidos por muestra (§3.3.5).

## 4. Tablas de datos (wavetables)

No son retardos: son datos fijos leídos por fase. El banco 0 no sirve (el anillo los movería, y una tabla fija del
banco 0 sólo es posible si el DSP no guarda nada en el banco 0 [medido en el S8]). El lugar es el **banco 5**:

* el ARM escribe las ondas con `FD6C` al cargar y al cambiar la onda (§3.1); 1024 palabras, ~600 libres con
  SYSTEM-S1CC: dos ondas de 256 puntos (morph de WAVE_MAP / Plaits *wavetable* entre dos ondas vecinas), o cuatro de
  128;
* el DSP lee `da` (h6) / `db` (h7) con h = base + fase · N, y una segunda carga para la interpolación lineal: ~8
  instrucciones por voz, 8 voces [estimado];
* COLOR por voz (COLOR MOD) no: las 8 voces comparten las dos ondas que el ARM puso; el morph entre ellas sí es por
  voz.

Bloqueo: la carga indexada del banco 5 (`ldb5` / `ldb5h7`) no está probada en el S1. `MP-CSAW2-LUT`
(`LFO_LUT=1`) es esa prueba y ya está armada ([NATIVE_OPS.md](NATIVE_OPS.md) §3, paso 2).

## 5. Qué falta en s1cc

| pieza | qué hace | dónde |
|---|---|---|
| `s1_delay_write(line, x)` / `s1_delay_read(line, d)` (`d` float, interpolado) | una línea por voz en el banco 0: el reservador del anillo le da `b`, saca el resto del estado de `S + b … S + b + dmax`; `hset h6` + `ldidx` | `s1dsp.h`, `graph.cpp` (hoy E303: índice calculado en un array de estado), `kernel.cpp` (reserva), `make_s1cc.py` (que el anfitrión no pise la línea) |
| `S1_TABLE(nombre, n)` en el banco 5 + `s1_table(nombre, fase)` | tabla que el ARM escribe (desde una función que corre en el PC, como `S1_PARAM`) y el DSP lee con `da` interpolando | `s1dsp.h`, `lower.cpp` (`ldb5`), `engine.c` (copia con `FD6C` en `load` y al mover la perilla) |
| `S1_FRAMES(nombre, panel_id, n)` | tramas de control que el tick escribe con `FCEC` (speech) | `engine.c` (tick), `make_s1cc.py` |
| emulador | el anillo del banco 0 ya es exacto; el banco 5 indexado también (`emu/dspcore.h`) | — |

## 6. Orden propuesto

1. **Probar `MP-CSAW2-LUT` en el equipo** (banco 5 indexado en el S1). Si funciona: `S1_TABLE` y las wavetables de
   Braids / Plaits (WAVETABLES, WAVE_MAP, WAVE_LINE, motor *wavetable*), 8 voces.
2. **`s1_delay_*` en el banco 0**: PLUCKED, SAW_COMB y la cuerda de Plaits con 4 voces (≥ 250 Hz), 2 voces (≥ 110 Hz), o el doble de alcance a media tasa [estimado].
3. **Cuerdas por voz en el maestro B**: MP-EDMSTR (§3.3.5) **funciona con 6 voces** (2026-11-04). Después MP-EDMIDX
   (§3.3.6) para saber si `tapreq` es absoluto: de eso dependen wavetable y granular en la EDM.
   Antes: **`tapreq` en el maestro B** (§3.3, la prueba del eco de 0,5 s): si funciona, PLUCKED / cuerdas a cualquier altura
   con 6 voces (excitación por `6008`–`600D`), chorus del *string machine*, búfer granular. Después, `tapreq` en un
   esclavo (§3.2) para BOWED / BLOWN / FLUTED.
4. **Tramas por el tick**: el motor *speech* de Plaits (LPC) con `S1_FRAMES`.
5. Chorus / ensemble del *string machine* en el maestro B (la memoria de retardo que el OS ya usa para su delay):
   sólo si el OS deja espacio en esa memoria [abierto].
