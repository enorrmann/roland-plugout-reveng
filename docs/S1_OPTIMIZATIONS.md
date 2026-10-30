# Optimizaciones del SYSTEM-1 (SYSTEM-S1CC): catálogo

Qué se puede sacar de los DSP del esclavo del SYSTEM-1 y cuánto, medido con el emulador sobre builds reales.
Referencia: MP-CSAW2 (`sdk/s1cc/build/mp_csaw2`, compilado con `--regs-a 0,1,2,3 --regs-b 0,1,2,3`),
`CMOD_IDX=1 MOD_DSP=B`, 8 voces. Límites medidos: DSP A 703 y DSP B 708 instrucciones por muestra. Estado al
2026-10-29. Lo que se mide en el equipo está en [NATIVE_OPS.md](NATIVE_OPS.md) y en los README de cada ejemplo.

Leyenda: **[M]** medido en el equipo · **[sim]** medido en el emulador · **[est]** estimado · **[código]** leído
en el OS.

## 1. Punto de partida [sim]

| DSP | instrucciones (8 voces) | quién las usa |
|---|---|---|
| A | 664 (498 + sección 166) | dos CSAW por voz (kernel, 81 por voz), tono modulado (`exp2`), mezclador |
| B | 701 (512 + sección 189) | LFO por voz, 2 ADSR por voz, envolvente de pitch, COLOR MOD, kernel B (37 por voz) |

## 2. Candidatas, de mayor a menor ahorro

| # | qué | ahorro | estado | qué falta |
|---|---|---|---|---|
| 1 | envolventes ADSR en el ARM, con rampas encadenadas (§3) | **B −213** | [sim] el ahorro; **[M] el mecanismo** (2026-10-30) | escribir `ARM_ENV`; probar 16 parámetros a la vez en el esclavo |
| 2 | LFO por voz en el ARM (§3) | **B −119** | [sim] | ídem; forma y fase por voz en el ARM |
| 3 | tono modulado (LFO y envolvente de pitch → `dt`) en el ARM, con rampa por voz (§4) | **A −115** | [sim] (`LFO_PITCH=0`) | 8 parámetros del DSP A con rampa en vez de `A040+v` |
| 4 | envolventes a media tasa con un salto: la mitad de las voces en muestras pares, la otra en impares (§5) | B ≈ −105 | [est] | primer salto en el S1 (en el S8 está medido) |
| 5 | LFO global (sin reinicio por nota): la voz 0 lo calcula y las demás copian su valor (fade, → amp y → filtro siguen por voz) | B −83 | [sim] | opción del panel; pierde el KEY TRIGGER |
| 6 | grupos de ops nuevos (`TRY_FORMS`, [examples_system_s1cc_README.md](examples_system_s1cc_README.md) §4.1) | 0–20 por DSP | [M] por grupo | seguir la bisección |

**Las 1 + 2 + 3 juntas**: DSP B de 701 a 369 (−47 %), DSP A de 664 a 549. Ese lugar va para el kernel: dos
kernels B más largos, o filtros por voz.

## 3. Envolventes y LFO en el ARM: rampas encadenadas [M]

* **Rampa del hardware** [M]
  * `FCE4(sel, id, destino, y, z)`: lineal, un paso cada 1/3 ms, de 4 a 97 ms de 0 a 1 según `y`.
  * `+8` del bloque es el valor actual y `+C` el incremento por paso.
  * Cada `y` es un tiempo fijo (4,00 ms con `y = 0`) para cualquier distancia [M, 2026-10-30]: cada tramo dura
    exactamente lo que se elige.
  * Detalle en [examples_system_1_ramp_README.md](examples_system_1_ramp_README.md).
* **Sin tick** [M, 2026-10-30]
  * `FE3C(sel, id, fn)` registra una función propia en la tabla de fin de rampa.
  * La interrupción del DSP llama `fn(id)` en cada fin de rampa con `z = 1` y no libera la entrada.
  * `fn` lanza el tramo siguiente.
  * Medido en el maestro: 6225 llamadas seguidas, una cada 4,00 ms ± 0,02, sin colgar.
* **Hecho: `ARM_ENV` / `ARM_LFO`** en SYSTEM-S1CC ([examples_system_s1cc_README.md](examples_system_s1cc_README.md) §3.7).
  MP-CSAW2-ARM: 8 voces, DSP B 375 instrucciones. Instalado el 2026-10-30.
* **SYSTEM-8**: su ARM tiene FPU. Al portarlo, `float` en vez de Q24. La rampa del S8 es `+0xA0`, con otro formato
  de entrada ([WAVE_BUS.md](WAVE_BUS.md) §4.6); habría que buscar el equivalente de `FE3C` en su OS.
* **Diseño (`ARM_ENV`)**
  * Por voz y envolvente, un parámetro del DSP B (`C0id`, 16 ids) y un estado en el ARM: etapa, nivel y destino.
  * En la nota on, `FCE4` hacia el primer punto del ataque.
  * En cada fin de rampa, el punto siguiente de la etapa: tramos de ≤ 97 ms, y la curva exponencial aproximada por
    rectas. El error de una cuerda de largo h sobre una exponencial de constante τ es ≈ (h/τ)²/8. Con tramos de 4 ms
    da < 1 % para tiempos de 0,1 s o más (τ ≥ 15 ms), y 95 % para 1 ms.
  * **Límite: una rampa dura como mínimo 4 ms**: la duración depende solo de `y`, no de la distancia [M]. Los ataques
    y decaimientos de menos de ~10 ms no se pueden hacer con rampas: un salto inmediato (`FCEC`) hace click. Para
    eso hay dos opciones: dejar en el DSP un polo que siga al valor del ARM (2–3 ops por envolvente), o aceptar
    4 ms como mínimo.
  * En la nota off, release desde el valor actual (`+8`).
  * En el DSP B, la ADSR (43 líneas) pasa a ser `ld` + `st` en la tabla de COLOR MOD.
* **LFO**: igual, con un parámetro por voz. La forma sale de una tabla en el ARM (cualquier forma, sin costo en
  el DSP) y la fase va por voz.
* **Riesgos**
  * La función corre en la interrupción.
  * Hay un solo registro de `id` por DSP: no se sabe qué pasa si dos rampas terminan juntas.
  * Los escalones de 3 kHz en la amplitud: un polo en el DSP los suaviza con 2 ops por voz.

## 4. Tono modulado en el ARM [sim]

Hoy el DSP A calcula 2^y (Taylor + tres cuadraturas, ~14 instrucciones por voz con 8 voces), con y = LFO + PITCH
ENV + RANGE. El ARM puede mandar `dt` (y `rdt`) ya modulado con rampa. La modulación es lenta (< 40 Hz), así que
escalones de 1/3 ms con interpolación lineal en `dt` sobran. Hace falta que la cadena de §3 ande, porque el LFO y
la envolvente de pitch avanzan solos. `LFO_PITCH=0` da el ahorro: DSP A 664 → 549.

## 5. Media tasa con saltos [est]

Un contador por muestra y un salto condicional que saltea las envolventes de las voces 0–3 en las muestras pares y
de las 4–7 en las impares. Las tasas se duplican, y el peor caso por muestra baja a la mitad. Lo salteado no cuesta
y el salto tomado cuesta 1 ciclo ([DSP_HARDWARE.md](DSP_HARDWARE.md) §6). Saltos y condiciones están medidos en el
S8; en el S1 no hay ninguno probado y Roland no los usa ahí. Primer paso: un PLUG-OUT con un salto que saltee la
mitad de las voces ([NATIVE_OPS.md](NATIVE_OPS.md) §3, punto 3). Si §3 funciona, esto deja de hacer falta para las
envolventes, pero sirve para cualquier código a tasa de control.

## 6. Tablas (LUT) en el DSP [sim]

`LFO_LUT`, `ENV_CURVE` ([examples_system_s1cc_README.md](examples_system_s1cc_README.md) §3.6).

* **No ahorran.** `LFO_LUT` +23, `ENV_CURVE` +109 (DSP B, 8 voces).
* **Dan formas.** Un seno verdadero y curvas de envolvente a elección, con todas las envolventes independientes
  sobre la misma tabla.
* **El tamaño no es el problema.** Las 128 palabras que escribe el ARM alcanzan; lo que cuesta son las ~7
  instrucciones de cada búsqueda.
* **Interpolar cuesta 4 de esas 7.** Sin interpolar haría falta N ≥ 256 para no oír escalones, y no hay lugar.
* **Del lado del ARM** (§3), una tabla no cuesta nada en el DSP.

## 7. Lo aprendido del planificador (`asm/dspsched.py`, 2026-10-29)

* `ldb5` / `ldb5h7` (banco 5 en h6 / h7 + Y) se planifican como cargas del banco 5: dos instrucciones después de su
  `hset`, ninguna otra transferencia del banco 5 en la instrucción. `asm/s1rules.py` las cuenta en R2 / R5.
* `hset` va **solo** en su instrucción, pero ya **no es una barrera**: solo se ordena con el registro que lee y con
  las cargas indexadas de su h. h6 y h7 se siguen por separado, así que dos búsquedas pueden estar en curso a la vez.
* Si solo quedan esperas (la distancia 2 después de un `hset`), el planificador pone un relleno (`mov r0, r0`).
  Antes se trababa.
* Al intercalar dos voces (`PAIRS`), un `hset` con sus cargas va entero (`make_s1cc.py _chunks`): el h es uno solo
  por DSP.
* Una búsqueda por voz dentro del código de la voz es una cadena en serie de ~10 instrucciones. Juntas en un bloque
  global, alternando h6 / h7, cuestan ~7 por búsqueda. COLOR MOD (`colors_idx_all`) ya estaba hecho así.

## 8. Trampa: compilar los kernels con 4 registros

Los kernels de SYSTEM-S1CC usan r0–r3 (el anfitrión intercala dos voces: r0–r3 y r4–r7). Un kernel compilado sin
`--regs-a 0,1,2,3 --regs-b 0,1,2,3` (con 8 registros, como el que dejan algunas pruebas de s1cc) da otro código y
más instrucciones. El 2026-10-29, `sdk/s1cc/build/mp_csaw2` había quedado compilado así y MP-CSAW2 dejó de entrar
en 8 voces (A 844). Recompilarlo como `sdk/s1cc/examples/mutable_ported/check_all.sh` lo resolvió.

## 9. Pruebas en el equipo pendientes, en orden

1. ~~C-S1RAMP, cadena de fin de rampa~~: **anda** (2026-10-30). Falta con 16 parámetros a la vez en el esclavo:
   eso se ve directamente con `ARM_ENV`.
2. **MP-CSAW2-LUT** (`plugins/`): primera carga indexada del banco 5 en el S1; formas del LFO y la curva `sq`.
3. ~~C-S1RAMP, distancia~~: la duración depende solo de `y` (2026-10-30).
4. **PLUG-OUT con un salto** (§5).
