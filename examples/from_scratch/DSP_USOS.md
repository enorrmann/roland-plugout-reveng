# Para qué sirven las operaciones halladas

Guía práctica para quien escriba microcódigo propio (o el futuro compilador / SDK). **La especificación que manda
es `DSP_ISA_SPEC.md`**; acá sólo se resume qué se puede construir con cada cosa, con secuencias mínimas tal como se
midieron. Cada sección dice **dónde se midió** (DSP y contexto). Nada de lo nuevo del 2026-10-16 se escuchó
todavía por audio: está verificado leyendo memoria con el peek (`re/dsptap.py`, `re/dsprdly.py`, `re/dspidx.py`,
`re/dsphreg.py`, `re/dspidxring.py`, `re/dspbus.py`, `re/dspbus2.py`). Historia y evidencia cruda: `NOTES.md` ("Tomas de retardo", "Registros ocultos").

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
* Multi-tap: varias tomas por muestra (Roland usa dos: `[4301..]` y `[4304..]`).
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
* La fracción se midió exacta en 0.25 / 0.5 / 0.75; la resolución fina no está medida.
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
* **Wavetables fijas en memoria: siguen abiertas.** Una tabla guardada una vez en el banco 0 no queda en su lugar:
  se corre una palabra por muestra y los stores de los demás programas barren todo el banco en 2048 muestras. Haría
  falta un destino indexado que no sea anillo (por ejemplo el banco 5, memoria simple); no se encontró todavía (las
  direcciones > 0x7FF vuelven al banco 0). Mientras tanto las wavetables siguen con inmediatos (`CUSTOM_OSC.md`:
  una A/A/B por punto, ≤ 290 puntos).
* Por la misma razón las LUT (waveshaping, curvas) no se pueden guardar fijas en el banco 0.
* No probado todavía en los slaves ni por audio.

---

## 3. Registros ocultos h4–h7 y palabras `40TY` — *medido: maestro DSP4 `f7dc` y DSP3*

**Qué hace.** Una palabra en posición de C con byte alto `0x40` carga un registro oculto: T (bits 7–6) = h4/h5/h6/h7,
modo (bits 5–4) = rY (mod 512) / imm16 / imm24, Y = registro o palabras del flujo de inmediatos a saltear.
La op A 0 con sub-código 4/5 suma h4/h5: **rD = D + Y + h4** (o h5).

**Usos.**
* h4/h5 como **constantes que no ocupan registro** (ni inmediato en cada uso): cargar una vez por muestra y sumar
  con la op A 0 sub 4/5. Ojo: h4 y h5 también los usa el motor de retardo.
* `40TY` en modo 0 copia un registro a h4..h7 (con wrap mod 512).
* h6 = índice del modo `28` (§2). h7 (`40c5` en un efecto del JX): uso desconocido.

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
* Waveshaper / LUT y wavetable indexada con morph: sólo si aparece un modo indexado sobre memoria no anillo.
* **Compresor / limitador**: RMS con `cf16` → dB con `cf26` → curva → ganancia con `8f19`.
* **Efecto repartido en dos DSP** por MXM (DSP3 Z/S → DSP4 O), o un LFO común enviado por el bus.

## Qué falta verificar antes de usarlo

1. Todo lo de §1–§2 **por audio** (nada se escuchó todavía).
2. Un destino indexado que no sea anillo (¿banco 5?) para tablas fijas; qué rango del banco 0 está libre en
   cada DSP para una línea indexada.
3. Indexado y registros ocultos en los **slaves** (laboratorio PLG3).
4. Cómo obtener una ventana de retardo fuera de `0xF7DC` (configuración del ARM) y el largo real de la línea.
5. Cuántas tomas por muestra admite el DMA; resolución de la fracción.
5b. Latencia del bus MXM, MXM/IOMD en los slaves, escritura desde el ARM; MAC op2 = 011; resto de `8f09`.
6. h7, modo imm32 de `40TY`, otros bytes bajos de `40xx`.
7. `X9bY` (sigue sin decodificar; no aparece en ninguna ventana visible desde el ARM; no usar).
