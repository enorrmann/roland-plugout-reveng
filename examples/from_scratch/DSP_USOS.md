# Para qué sirven las operaciones halladas

Guía práctica para quien escriba microcódigo propio (o el futuro compilador / SDK). **La especificación que manda
es `DSP_ISA_SPEC.md`**; acá sólo se resume qué se puede construir con cada cosa, con secuencias mínimas tal como se
midieron. Cada sección dice **dónde se midió** (DSP y contexto). Lo del 2026-10-16 está verificado leyendo
memoria con el peek (`re/dsptap.py`, `re/dsprdly.py`, `re/dspidx.py`, `re/dsphreg.py`, `re/dspidxring.py`,
`re/dspbus.py`, `re/dspbus2.py`); el 2026-10-17 se oyeron en los slaves el retardo indexado (§2) y el banco 5
indexado (§2b); la línea de retardo del §1 todavía no se oyó. Historia y evidencia cruda: `NOTES.md` ("Tomas de retardo", "Registros ocultos").

Reglas que valen para todo (de `DSP_ISA_SPEC.md`):
* ≈ 925–945 instrucciones por muestra y por DSP (96 kHz); una instrucción A/A/B cuesta un ciclo. Pasarse puede dejar
  el equipo inestable hasta apagarlo.
* Una carga necesita una instrucción en el medio antes de que una op use el registro.
* Los inmediatos son enteros en las palabras `40TY` (valor × 16384) y números RAW en las ops A/B/MAC.

---

## 1. Línea de retardo (DMA del banco 2) — *medido: maestro DSP4, ranura 0xF7DC*

**Qué hace.** Cada muestra se escribe una muestra en la memoria de retardo y se pide leer otra de hace N muestras
(con parte fraccionaria). El hardware devuelve dos muestras vecinas y la fracción, listas para interpolar.

**Usos.**
* Delay / eco (con realimentación: sumar la toma escalada a la entrada antes de escribir).
* Chorus / flanger: retardo modulado por un LFO; la fracción da interpolación sub-muestra sin "zipper".
* Filtros peine (comb) de realimentación y feed-forward, all-pass de Schroeder → bloques de reverb.
* Karplus-Strong (cuerda pulsada): retardo = período, realimentación con promedio de dos muestras (las dos
  muestras vecinas que ya entrega la toma sirven de filtro paso bajo).
* Multi-tap: varias tomas por muestra (Roland usa dos: `[4301..]` y `[4304..]`). **Medido 2026-10-17: 24 tomas por
  muestra funcionan** (cada una con su `0b03 4050 <dest>`, resultados en tríos `[4301+3k..]`; `re/dspmtap.py`), el
  límite no apareció.
* Idea: pitch shifter de dos tomas con rampas cruzadas.
* Retardos largos y exactos a la muestra: se midió hasta 8.6 s (u = 50.648).

**Secuencia mínima** (por muestra; `W = 4300` es la ventana de esta ranura):

```
8fe0 4300 ...        [4300] <- r0                  escribe la entrada (store fe cualquiera; Roland: 1b06 c132 8fe0 4300 …)
1d06 4022 7fe0 c1fc 5100 0000                      h4 (base) = 0x510000/16384 = 324 (= posición de escritura → retardo = u)
                                                   (la carga de r7 <- [c1fc] es de Roland; el 4022 saltea esas 2 palabras)
0903 3050 <imm16>    r3 = u                        retardo en unidades de 16384 muestras (u = muestras/16384)
0a02 bf6c            r3 = (r3 + h4) mod 512        dirección de lectura
0b03 4050 0301       h5 = 0x301                    los resultados irán a [4301..4303]  (¡obligatorio!)
0c02 3a90            pedido de toma con la dirección en r3   (o el 1b04 c510 3a90 0008 de Roland)
0c03 0fe0 4301 …     r0 <- [4301] (x[n-L-⌊k⌋]), r1 <- [4302] (una muestra más vieja), r2 <- [4303] (fracción)
0001                 latencia de carga
0803 0c0a 9281       r0 = r0 + r2·(r1 − r0)        interpolación lineal (MAC)
```

Retardo total = 16384·u + L muestras, con L = 2 (programa corto propio) o 3 (programa de Roland): depende de la
posición dentro del programa. Base 324 ⇒ el retardo es directamente u.

**Límites / cuidado.**
* Sólo se conoce cableada la ranura `0xF7DC` del DSP4 del maestro: usarla **reemplaza el delay global de
  Roland**. En el DSP3 nada de esto tiene efecto. No se sabe todavía cómo conseguir una ventana en otra ranura ni
  en los slaves (donde corren los chorus del J60/JX).
* La escritura de `[4300]` cae en la posición 324 (fija, la pone el ARM); una base menor que 324 lee "el futuro"
  (ceros). Largo de la línea desconocido: u = 100 ya no devuelve nada.
* **La fracción tiene 6 bits (1/64), truncada** (2026-10-17: 0.4 → 25/64, 0.8 → 51/64, 0.2 → 12/64): la
  interpolación lineal de la toma tiene pasos de 1/64 de muestra.
* Leer toda la WMEM (0x41300000) desde el ARM con un programa propio en esta ranura colgó el maestro una vez.

---

## 2. Transferencias indexadas `X28Y` + índice h6 — *medido: maestro DSP3 (exacto)*

**Qué hace.** `0b02 4080` copia r0 al registro índice h6; `X28Y` carga (X 0–7: rX ← banco0[⌊h6·16384⌋ + Y]) o
guarda (X 8–15: banco0[…] ← r(X−8)). Índice truncado, módulo 0x800, sin sumar la palabra R. Caben varias por
instrucción (`1e04 0281 1282` → r0 = [h6+1], r1 = [h6+2]). **Es relativo al anillo** como cualquier referencia
del banco 0 (`re/dspidxring.py`: un contador con load `[h6+1]` / store `[h6]` cuenta 96 000/s; con `[h6]`/`[h6]`
no cuenta).

**Uso principal: línea de retardo variable dentro del banco 0, en cualquier DSP (también los slaves, sin DMA).**
Como el anillo avanza una palabra por muestra, una carga indexada de `[S + k]` devuelve lo que se guardó en `[S]`
hace k muestras. Guardando la entrada en `[S]` cada muestra y eligiendo k con h6 se obtiene un retardo de
1 a < 2048 muestras (< ~21 ms a 96 kHz), modulable muestra a muestra:
* chorus / flanger / vibrato (k modulado por un LFO),
* filtros peine y all-pass cortos, Karplus-Strong de tonos agudos (período < 2048 muestras ⇒ notas > ~47 Hz),
* interpolación lineal con dos cargas vecinas `[h6]`, `[h6+1]` y la fracción calculada aparte del índice
  (el índice se trunca).
Es probablemente lo que hace Roland en el S-100 (`r0 <- [a024]`, `0b02 4080`, `0c02 0280`, con h6 sacado de un
parámetro).

**Secuencia mínima** (entrada en r1, k = retardo en muestras en r0 como k/16384, base S):

```
0c03 9fe0 <S>        [S] <- r1                    escribe la entrada (store fe normal)
<ops>                r0 = (S + k)/16384           índice = referencia de la muestra de hace k
0b02 4080            h6 = r0
1e04 2280 3281       r2 <- [h6], r3 <- [h6+1]     muestras de hace k y k+1
0001                 latencia
<lerp MAC>           out = r2 + frac·(r3 − r2)    (frac = parte fraccionaria de k, calculada aparte)
```

**Límites / cuidado.**
* Hay que **reservar el rango de referencias S..S+kmax** del banco 0: ningún otro código de ese DSP puede guardar
  ahí (los programas de Roland comparten el banco y su anillo).
* **Tablas fijas: usar el banco 5 (§2b).** En el banco 0 una tabla fija sólo sobrevive si **ningún** programa del
  DSP guarda en el banco 0: compensando el anillo con un contador de muestras del banco 5 (h6 = T + n) 8 marcadores
  duraron 3 s en el DSP3 del maestro (ocioso); un solo store común los borró en < 10 ms (barre las 2048 palabras).
  `re/dspb0fix.py`. Sirve sólo en un DSP entero propio con el estado en el banco 5.
* **Verificado por audio en los slaves** (2026-10-17, ranura DCO2, `re/dspb0dly.py`): SAW + SAW retrasada 92
  muestras con `[h6]` indexado → la nota 72 sale una octava arriba (1045.9 Hz: el fundamental se cancela); con
  k = 0 vuelve la SAW (523.6 Hz). h6 y el modo `28` funcionan igual en los slaves.

---

## 2b. Banco 5 indexado: `da`/`de` (h6), `db`/`df` (h7), `d0`–`d7` (directo) — *medido 2026-10-17: maestro DSP3 exacto, slaves por audio*

**Qué hace.** Modos de transferencia sobre el **banco 5, memoria simple (sin anillo)**, sin palabra de referencia:

| palabra | efecto |
|---|---|
| `XdaY` / `XdeY` | X 0–7: rX ← banco5[⌊h6·16384⌋ + Y]; X 8–15: banco5[…] ← r(X−8) |
| `XdbY` / `XdfY` | lo mismo con **h7** (`40c0` = h7 ← r0) |
| `Xd0Y` … `Xd7Y` | banco5[N·16 + Y] (dirección directa 0x00–0x7F en la palabra) |

* Índice truncado (h6 = 0x10.75 → 0x10); una instrucción entre `4080` y la carga alcanza.
* **1024 palabras persistentes**: índice mod 0x400 (0x800, 0xC00, 0x1000… son alias; `re/dspb5alias.py`). Con el
  bit 10 (0x400–0x7FF) lo guardado se lee sólo en la misma muestra y no queda: Roland usa A400–A427 como temporales.
  El ARM ve 0x000–0x3FF (DPC*n* RAW) y puede escribirlo (en el maestro).
* Límite del banco 5: **1 carga y 1 store por instrucción** (con 2 o más stores no queda ninguno): dos lecturas
  vecinas para interpolar = dos instrucciones; llenar una tabla = `move` + `store` por punto.
* **Herramienta lista: `re/wt5.py`** (WAV → tabla → envío en vivo a los slaves → reproductor con interpolación y
  morph), ver `CUSTOM_OSC.md` "Wavetable verdadera en el banco 5".
* `d8`/`d9`/`dc`/`dd` leen direcciones que cambian con el tiempo: [desconocido], no usar. Otros modos (`80`–`8f`,
  `b0`–`bf`, `c0`–`cf`, `f0`–`ff`, `e0`–`ef`) leen otras cosas, `90`–`af` y `f5`–`f7` no cargan (`re/dspmodescan.py`).

**Uso en Roland** (refs `Annn` del corpus): A000–A047, A200–A205, A300–A303, A4xx. **Libre en general:
0x048–0x1FF y 0x306–0x3BF** (en el maestro A3C0.. lo usan nuestras pruebas).

**Usos.**
* **Wavetables verdaderas**: tabla en el banco 5, índice = fase · N + base → h6, una carga (`0daY`), interpolación
  con una segunda carga `[h6+1]` y la MAC lerp. Costo por voz ≈ 10 instrucciones, sin importar N (hasta ~440 puntos
  contiguos), contra 1 instrucción por punto de la versión con inmediatos.
* Morph entre cuadros (dos lecturas con h6 y h7 + lerp), varias tablas por DSP, LUT de waveshaping, curvas
  (envolventes, velocidad, afinación), secuencias.
* Estado persistente sin el anillo: con el estado en el banco 5 también es posible una tabla fija en el banco 0.

**Verificado en los slaves por audio** (`re/dspb5osc.py`, ranura DCO2): tabla de 16 puntos en A100 → triángulo
(armónicos 1, 0, 0.111, 0, 0.042 = 1/n² impares) y cuadrada (1, 0, 0.315, 0, 0.180 = 1/n impares); con un programa
que sólo lee (sin escribir la tabla) el sonido sigue igual > 10 s: la tabla persiste y las voces A y B la comparten.

**Secuencia mínima** (fase r1 ∈ [−1, 1), tabla de N puntos en la base B):

```
r0 = r1 · (N/2)/16384                  (A mul imm)
r0 = r0 + (B + N/2)/16384              (A add imm)
0b02 4080                              h6 = r0
0001                                   (una instrucción en el medio)
0c02 2da0                              r2 <- banco5[h6]
0001 0001                              latencia
```

**Cuidado.** Llenar la tabla desde el DSP (en los slaves el ARM no llega): N stores con inmediatos, una vez (saltar
el bloque con un salto cuando ya está lleno) o repartido en varias muestras. El banco 5 lo comparten todos los
programas del DSP: respetar las direcciones de Roland.

## 3. Registros ocultos h4–h7 y palabras `40TY` — *medido: maestro DSP4 `f7dc` y DSP3*

**Qué hace.** Una palabra en posición de C con byte alto `0x40` carga un registro oculto: T (bits 7–6) = h4/h5/h6/h7,
modo (bits 5–4) = rY (mod 512) / imm16 / imm24, Y = registro o palabras del flujo de inmediatos a saltear.
La op A 0 con sub-código 4/5 suma h4/h5: **rD = D + Y + h4** (o h5).

**Usos.**
* h4/h5 como **constantes que no ocupan registro** (ni inmediato en cada uso): cargar una vez por muestra y sumar
  con la op A 0 sub 4/5. Ojo: h4 y h5 también los usa el motor de retardo.
* `40TY` en modo 0 copia un registro a h4..h7 (con wrap mod 512).
* h6 = índice del modo `28` (§2) y de `da`/`de` (§2b). **h7 = índice de `db`/`df`** (§2b; `40c0` = h7 ← r0).

**Cuidado.** Los valores por defecto los deja otra tarea (DSP4: h4 = 288, h5 = 0x105/16384); un programa propio
debe fijarlos antes de usarlos. Modo 3 (imm32) carga 0.

## 4. `{8+d}f6c`: sumar y envolver — *medido: maestro DSP4 / DSP3*

rd = (rd + h4) mod 512. Además de dar direcciones de retardo, es un **sumar-con-wrap** en una instrucción
(acumuladores de fase con período 512, contadores que no se saturan). Aplicado dos veces: 302.648 → 78.648.

## 5. Persistencia de cada banco — *medido: maestro DSP3 (contador load W / store W)*

| banco | comportamiento | sirve para |
|---|---|---|
| 0 (`0nnn`) | anillo: lo guardado en S se lee en S+1 la muestra siguiente | estado (load S+1, store S), retardos cortos gratis (S+k), indexado (§2) |
| 2 (`4nnn`) | dos páginas alternadas por muestra (se relee 2 muestras después) | ventana DMA de la memoria de retardo (§1) |
| 4 (`8nnn`, MXM) | no persiste entre muestras | bus de audio/control entre DSP (§5b) |
| 5 (`Annn`) | memoria simple persistente, el ARM la lee coherente (DPC*n* RAW) | estado persistente, tablas pequeñas, **salidas de depuración** leídas por el peek |
| 3 (`60nn`) | no se relee; el ARM lo ve | E/S (IOMD, §5b), canal de salida/depuración |

Truco de depuración: guardar resultados de prueba en el banco 5 (`A3C0..`, libre en el maestro) y leerlos desde el
ARM; las lecturas sueltas del banco 0 caen en otras referencias por el anillo.

## 5b. Buses IOMD y MXM — *medido: maestro DSP3 → ARM / DSP4 (`re/dspbus.py`, `re/dspbus2.py`)*

* **IOMD**: un store `[60nn]` aparece en la palabra nn del bloque IOMD (0x4113C000 IEEE / 0x4113E000 RAW), legible
  por el ARM. Usos: canal de salida / depuración sin pasar por el anillo, y la E/S del wave bus (por donde salen las
  voces de los slaves).
* **MXM**: lo que el DSP3 guarda en su zona Z (`85nn`) o S (`87nn`) lo carga el DSP4 de su zona O (`83nn`, mismo
  nn). Las zonas M/O del DSP3 no llegan al DSP4. Usos: **bus propio entre DSP**: repartir un efecto en dos DSP
  (p. ej. delay en uno, reverb en otro), mandar un LFO o una señal de modulación a otro DSP, sumar voces.
* Cuidado: la latencia del bus no está medida; el mapa en los slaves y la escritura desde el ARM tampoco.

## 5c. Banderas: `X9bY` + C — *medido: maestro DSP3 y DSP4 `efdc` (2026-10-17, `re/dspx9bflags.py`)*
* `X9bY` copia las banderas del resultado de la unidad Y de su instrucción al grupo X−8 (9/a/b → cond 4–7 / 8–11 /
  12–15): c0 = ≠0, c1 = ≥0, c2 = sin overflow, c3 = ≤0. Una instrucción de latencia; persisten hasta que se
  reescriben. La op B `{8+d}e{0Y}` (bits 7–6 = 00) **compara** (banderas de r(d) + Y) sin escribir registros.
* Usos: **selección dependiente de los datos** con una sola instrucción (`x12`: corre op 1 u op 2 según la
  condición): if/else en el DSP sin min/max, histéresis (Schmitt trigger para sync o detección de cruces),
  comparadores de envolvente (ataque / release distintos), sample & hold disparado por un cruce, contadores con
  reset (divisores, secuenciadores), detección de overflow de una fase (c2), elegir entre dos ramas de un
  waveshaper por tramos.
* **Saltos y bucles** (`009Y`, desplazamiento con signo en palabras desde el final de la instrucción; la C lo
  hace condicional): saltear bloques enteros (ahorra presupuesto: una rama no cuesta tiempo), `switch` por modo
  (como el JX), bucles para filtros/órdenes variables, sumar N muestras/voces, iterar Newton hasta converger,
  polinomios de grado variable. Confirmado por audio en los slaves (SAW con bucle de 4 vueltas idéntica).
  Cuidado: cargar con programa vacío primero (`dspown.load` ya lo hace).
* `f5c` = posición de escritura de la línea de retardo (cuenta 1/16384 por muestra): direcciones de retardo sin
  depender de h4; también sirve de reloj de muestras libre (87 s de período).
* Secuencia mínima: `cmp` (o cualquier op en la unidad Y) + `X9bY` → una instrucción cualquiera → instrucción con C.
  Cuidado: el grupo queda escrito para los programas siguientes del mismo DSP (Roland lo reescribe antes de usarlo).

## 6. Hallazgos anteriores, en una línea

* **min / max / |x| (unidad B)**: selección sin saltos, clamps, wrap de fase (`dspasm.wrap`), rectificación,
  half-wave, detección de umbrales.
* **SFU**: `cf06` semilla de recíproco + Goldschmidt → división; `8f19` separación exponencial → 2^x, e^x
  (pitch, envolventes exponenciales); `8f29` signo; `8f39` máscara de negativo; `cf36` techo → cuantización,
  bitcrusher, índices enteros.
* **MAC lerp** (`0803 0c0a 9281`): interpolación, crossfade, mezcla wet/dry, filtros de un polo.
* **MAC con D ≥ 8 → r(D−8) = resultado/2** (`0803 8c0a 9281`): mitad gratis (promedios, Karplus-Strong, mezclas
  50 %, evitar saturar). **op2 = 010 → c = imm16** (`0804 0809 5480 3000` = r0·r1 + 1.0·r5): coeficiente
  constante en 16 bits. **No** poner una op A con inmediato dentro de una clase MAC: lee la palabra N.
* **SFU nuevas**: `cf16` separación para raíz (rY ← m ∈ [1,4), r5 ≈ 1/√m ~1 %, r7 = signo·2^−k → √|x| = r7·m·r5;
  refinar r5 con Newton) → seguidores RMS/envolvente, compresores, paneo/crossfade de igual potencia, magnitud de
  cuadratura. `cf26` separación log2 (rY ← m ∈ [0.75,1.5), r4 ← e; negativo → r4 = 9 como marca) → dB para
  medidores/compresores, frecuencia → pitch, ganancia en dominio log; junto con `8f19` (exp) da pow(x, y) y curvas
  musicales. `8f09` parcial.
* **Ops B nuevas (D ≥ 8)**: `b242` min(−X, Y) y `b642` max(−X, Y) → clamps simétricos (|x| ≤ a con una op cada
  lado sin negar antes), rectificación negativa; `bc42` **frexp** (X = m·2^e, m ∈ [0.5,1), rY ← e) → normalizar
  antes de un polinomio, control automático de nivel por potencias de 2, contar bits / octava de un valor.
* **A/A/B en un ciclo** (clase 0x160): tres ops por instrucción → wavetables de un punto por instrucción, cadenas
  de polinomios.
* **Inmediatos compartidos**: varias ops A/B de una instrucción leen el mismo inmediato; planificar una constante
  por instrucción.
* **Bits de condición de contexto**: especializar código por ranura / voz (fijos por contexto, no dependen de
  datos). Para decisiones según datos, usar min/max.

---

## Ideas combinando

* **Chorus/flanger propio** en el DSP4 del maestro: LFO (fase con `f6c` o wrap) → u modulado → dos tomas
  (estéreo) → lerp → mezcla con MAC.
* **Delay con realimentación filtrada** (tape echo): toma → one-pole (MAC) → saturación (min/max o polinomio) →
  suma a la entrada.
* **Reverb chica**: 4 combs + 2 all-pass usando varias tomas de la misma línea.
* **Karplus-Strong** en la línea de retardo (cuerda pulsada polifónica limitada por número de tomas).
* **Chorus / vibrato en los slaves** con la línea indexada del banco 0 (§2), sin depender del DMA del maestro.
* **Waveshaper / LUT y wavetable indexada con morph**: tablas en el banco 5 (§2b), h6 y h7 para dos cuadros.
* **Compresor / limitador**: RMS con `cf16` → dB con `cf26` → curva → ganancia con `8f19`.
* **Efecto repartido en dos DSP** por MXM (DSP3 Z/S → DSP4 O), o un LFO común enviado por el bus.

## Qué falta verificar antes de usarlo

Hecho el 2026-10-17: §2 por audio en los slaves, tablas fijas (banco 5 indexado, §2b), indexado y h6 en los slaves,
cantidad de tomas (≥ 24) y fracción (1/64), h7 (índice de `db`/`df`). `X9bY` = banderas (§5c). Latencia MXM = 1
muestra (`DSP_HARDWARE.md` §8).

1. §1 (línea de retardo) **por audio**: está medido exacto con el peek, falta oírlo por la salida.
2. Cómo obtener una ventana de retardo fuera de `0xF7DC` (configuración del ARM) y el largo real de la línea.
3. MXM/IOMD en los slaves, escritura del ARM en MXM; MAC op2 = 011; resto de `8f09`.
4. Modo imm32 de `40TY` (carga 0), otros bytes bajos de `40xx`; modos `d8`/`d9`/`dc`/`dd` del banco 5; qué es la
   zona 0x400–0x7FF del banco 5 (sólo la misma muestra).
5. Banco 5 en los slaves: qué direcciones usan los programas de cada DSP (el corpus da la unión) y si el ARM del
   slave puede escribirlo (para cargar tablas sin código).
