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
3. **`tapreq` en el maestro B** (§3.3, la prueba del eco de 0,5 s): si funciona, PLUCKED / cuerdas a cualquier altura
   con 6 voces (excitación por `6008`–`600D`), chorus del *string machine*, búfer granular. Después, `tapreq` en un
   esclavo (§3.2) para BOWED / BLOWN / FLUTED.
4. **Tramas por el tick**: el motor *speech* de Plaits (LPC) con `S1_FRAMES`.
5. Chorus / ensemble del *string machine* en el maestro B (la memoria de retardo que el OS ya usa para su delay):
   sólo si el OS deja espacio en esa memoria [abierto].
