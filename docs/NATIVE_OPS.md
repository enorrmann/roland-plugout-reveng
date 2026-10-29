# Ops nativas del DSP: estado, uso y plan

Objetivo (decisión del usuario, 2026-10-28): **probar todas las ops nativas del DSP en el equipo y usarlas todas**
en s1cc y en el anfitrión de SYSTEM-S1CC, para aprovechar el DSP al máximo, empezando por las que más aportan.
Este documento dice, para cada op, en qué estado está y qué falta. La semántica exacta está en
[DSP_HARDWARE.md](DSP_HARDWARE.md) §5–§8; el uso desde C, en [S1CC.md](S1CC.md) §7.6.

## 1. Estados

| estado | significa |
|---|---|
| **S8 exacta** | medida en el maestro del SYSTEM-8 con la sonda (`re/dspnative.py` o `re/dspexact.py`), igual bit a bit al emulador |
| **S1 probada** | corrió en un PLUG-OUT del SYSTEM-1 sin colgarlo y sonó como el emulador |
| **emulador** | la referencia C (`emu/dspdec.h`, `emu/dspcore.h`, `emu/dspjit.py`; `re/dspisa.py` archivado el 2026-10-29) la modela como el equipo |
| **s1cc** | intrínseco (`s1dsp.h`) y / o uso automático en `lower.cpp` / `kernel.cpp` |
| **anfitrión** | la usa `examples/system_s1cc/make_s1cc.py` (LFO, ADSR, COLOR MOD, mezclador) |

## 2. Estado por op (2026-10-29, tarde)

| op | qué hace | S8 exacta | emulador | s1cc | anfitrión | S1 probada |
|---|---|---|---|---|---|---|
| `shl` / `shr` / `shrn` | X·2^k, complemento a 2 con 32 bits de fracción (hacia −∞ en 2^−32, envuelve en ±512), luego RAW trunca | sí (2026-10-29: 15 360 casos) | sí (corregido dos veces) | automático (productos por potencias de 2 > 256), `s1_ldexp` | no | **no** |
| `floor` / `ceil` (SFU) | piso / techo | sí (`floor` era «negmask») | sí (corregido) | `floorf`, `s1_floor`, `s1_ceil` | no | **no** |
| `sign` (SFU) | −1 / 0 / +1 | sí | sí | `s1_sign` | no | **no** |
| `recipseed` + `newton` | 1/x de Goldschmidt: 4 ops, 10⁻⁸; `recipseed` pisa rY con m | sí | sí (corregido) | división `x / y`, `s1_recip` | no | **no** |
| `expsplit` (SFU) | e^⌊x⌋ de una ROM propia + frac | sí (ROM leída) | sí (corregido) | `exp2` de rango > ±8 octavas, `s1_exp2`, `s1_exp` | no | **no** |
| `log2split` (SFU) | x = m·2^e, m ∈ [0,75, 1,5); Y < 0 en complemento a 2 (e de Y + 512) | sí (2026-10-29, Y < 0 en todo el rango) | sí (corregido) | `s1_log2` | no | **no** |
| `add3` / `sum8` (A) | D + X + Y / X + Y + R | sí | sí | `s1_sum3` | no | **no** |
| MAC, coeficiente en registro | X·Y + C·Z (+ W), /2 sólo con P2 | sí | sí | `s1_mac2` (necesita 5–6 registros) | no (4 registros por voz) | **no** |
| `ph_wrap` / `ph_clamp` / `ph_tri` | fase, recorte, triángulo | sí | sí | `s1_wrap`, `s1_clamp`, `s1_tri` | sí | **sí** |
| `min` / `max` | | sí | sí | `fminf`, `fmaxf`, comparaciones | sí | **sí** |
| `absadd` / `subabs` | \|X\| + Y, Y − \|X\| | sí | sí | **no** (`fabs` sale como `max r, r, -r`) | no | no |
| `minn` / `maxn` | min(−X, Y), max(−X, Y) | sí | sí | **no** | no | no |
| `frexp` | m ∈ [0,5, 1), exponente | sí | sí | **no** | no | no |
| `atanhsplit` (SFU) | separación para log por atanh | sí | sí | no (lo cubre `log2split`) | no | no |
| `sqrtsplit` (SFU) | m = \|Y\|·4^k ∈ [1, 4) en rY, semilla 1/√m (9 bits) en r(d+1), ±2^−k en r(d+3) | sí (2026-10-29) | sí | no | no | no |
| MAC con coeficiente inmediato | X·Y + K·Z (+ W); K en el byte C (+8 con bit 9) | sí (2026-10-29, 20 formas; D8 cerrada) | sí | no | — | no |
| dos MAC en una instrucción (clase 0x080) | dos MAC de un producto, en paralelo; con P2 no escribe ninguna | sí (2026-10-29) | sí | no | no | no |
| banderas + condición (`X9bY`, `cond`) | saltear unidades según el resultado de otra | en curso 2026-10-29: instrucciones de Roland con condiciones en el maestro, exactas salvo clases 0x590 / 0x190 (plantilla) | sí | no | no | no |
| saltos y bucles (`009Y`) | | sí (S8, slaves por audio) | sí | no | no | no |
| cargas indexadas (`X28Y` banco 0; `da`/`de`, `db`/`df` banco 5; `hset` h4–h7) | dirección = ⌊h·16384⌋ + Y | sí (maestro exacto, slaves por audio) | sí | no | no | no |
| línea de retardo (`tapreq`, `f5c`, `f6Y`) | taps del banco 2 | sí (maestro, DSP4 `f7dc`) | sí | no | no | no |
| rampas de parámetros (`FCE4`, ARM) | el ARM rampa un parámetro del DSP | — | no | no | no | servicio del S1 sin mapear |

## 3. Plan, en orden de aporte

Cada paso sigue el mismo camino: (1) medir en el SYSTEM-8 si no está exacta (`re/dspnative.py`: agregar la prueba a
`TESTS`); (2) emulador igual al equipo; (3) intrínseco en s1cc y uso automático donde acorta; (4) uso en el
anfitrión (`make_s1cc.py`); (5) prueba en el SYSTEM-1 (un PLUG-OUT como NATIVE-CHECK, o `TRY_FORMS` para grupos
de ops) y recién entonces marcarla probada aquí y en `asm/proven_forms.txt`.

1. **Probar en el SYSTEM-1 lo ya hecho.**
   * `plugins/NATIVE-CHECK` (`sdk/s1cc/examples/native_ops/native_check.c`): sierra limpia = las ops del kernel
     están bien; OSC1 COLOR elige la op (0–27 todas; 28 `s1_recip`, 56 `s1_log2` + `s1_exp`, 84 `s1_floor`,
     112 `s1_sign`, 140 `s1_mac2`, 168 `s1_sum3`, 196 `shl`, 224 `expsplit` + `s1_ceil`). Si se cuelga al elegirlo:
     apagar y prender, y bisecar con builds de una op cada uno.
   * `plugins/BRAIDS-CSAW` reconstruido con la división corregida (los clicks de 2026-10-27 venían de leer y
     después de `recipseed`).
   * `MP-CSAW2` con 7 voces y `MOD_DSP=B`: instalado el 2026-10-28, sin confirmar de oído.
   * Cualquier kernel reconstruido desde el 2026-10-28 ya trae `shl` automático: NATIVE-CHECK primero.
2. **Cargas indexadas** (el mayor aporte).
   * COLOR MOD: en vez de pesar 5 fuentes (10 `mad` en cadena por voz), las fuentes de cada voz en bloques de 8
     palabras del banco 5 y una carga `da` (color 0, h6) / `db` (color 1, h7) con h = base de la fuente elegida
     (`hset` modo 0 desde un registro cargado de un parámetro; una vez por muestra, fuera de las voces). Por voz:
     2 cargas + 2 `mad` + 2 stores. Es lo que falta para MP-CSAW2 con 8 voces (el DSP B usa 782 de 708).
     Cuidado: una instrucción entre `40TY` y la transferencia; las envolventes viven en el banco 0 y habría que
     copiarlas al banco 5 (un acceso al banco 5 por instrucción en el S1, regla R5).
   * Tablas leídas por fase (wavetables, tablas `const` con índice calculado): hoy van a islas (polinomios); con
     `da` / `X28Y` serían una carga. Ver [bank5-tables](DSP_HARDWARE.md) §7.4 y `re/wt5.py` (wavetables en el S8).
   * Líneas de retardo por voz (chorus de CRUSHER, flanger): `X28Y` en el banco 0, relativo al anillo.
3. **Banderas, condiciones y saltos.** Selecciones (`a < b ? x : y`) sin aritmética; código a tasa de control
   (LFO, colores, envolventes cada N muestras con un contador y un salto). Probados en los slaves del S8 por audio;
   en el S1, nada. Primero un PLUG-OUT de prueba con un salto que saltee la mitad de las voces.
4. **MAC con coeficiente inmediato.** Medir en el S8 (D8: dónde lee el inmediato cuando Y es registro). Con ella
   la MAC sirve con 4 registros (la constante no ocupa uno): colores, filtros, mezclas. Después la MAC /2 y dos MAC
   por instrucción.
5. **`absadd` / `subabs` / `minn` / `maxn` automáticos.** El polyBLEP `u·(2 − |u|)` y `max(min(w, dt), −dt)` de
   `mp_saw_bl`, los triángulos `1 − 2·|x|`: una op menos por borde en todos los osciladores.
6. **`frexp` (`s1_frexp`), `sqrtsplit` (modelarla: dspnative con otros d / Y y la tabla Q completa) → `s1_sqrt`,
   `s1_rsqrt`; `atanhsplit`.**
7. **Taps de retardo (`tapreq`) y rampas `FCE4`** (mapear el servicio en el OS del S1).

## 4. Cómo retomar

```sh
# SYSTEM-8 (medir): el laboratorio con la sonda en el slot 3, seleccionar PLG3
python3 examples/from_scratch/re/dsplive.py install l00.batch
python3 examples/from_scratch/re/dsppeek.py ping                 # pong 0x1234
python3 examples/from_scratch/re/dspnative.py -n 48              # todas las pruebas; --list, --sim, nombres

# emulador y compilador
python3 examples/from_scratch/emu/test_core.py                   # 12/12
python3 -m unittest discover -s sdk/s1cc/tests                   # 6/6
for f in asm/tests/test_*.py; do python3 $f; done
sh sdk/s1cc/examples/mutable_ported/check_all.sh                 # voces por kernel (tabla de examples_mutable_ported_README.md §4)

# SYSTEM-1 (probar)
python3 tools/plugout_tool.py install plugins/NATIVE-CHECK --slot 1 --force --no-patches -y
```

Un solo programa a la vez sobre el equipo (MIDI): no correr dos herramientas juntas.

## 5. Registro

* **2026-10-29.** La referencia pasó a ser el C (`emu/dspref.py`; `re/dspisa.py` archivado, [EMULATOR.md](EMULATOR.md)
  §1.1). Medido en el maestro del S8 y corregido en la referencia: MAC con coeficiente inmediato (20 formas, regla
  del byte C + 8·bit 9 en 19 clases: D8 cerrada; `asm/asm.py` y `asm/disasm.py` la usan), dos MAC por instrucción,
  /2 sólo con P2, redondeo de `shl`/`shr` (complemento a 2 con 32 bits de fracción), `log2split` con Y < 0 en todo
  el rango, `sqrtsplit` completo (tabla Q de 9 bits, salida r(d+3)). Batería completa de `dspnative.py`: todas las
  pruebas 0 diferencias.

* **2026-10-28.** Medidas exactas en el maestro del S8: `shl`/`shr`/`shrn`, `floor`, `ceil`, `sign`, `recipseed`,
  `expsplit`, `log2split`, `atanhsplit`, `frexp`, `newton`, `add3`, `sum8`, MAC de coeficiente en registro (+, −,
  /2), 24–96 casos por op, con y sin instrucción vacía antes de leer. Cinco diferencias del emulador corregidas
  ([EMULATOR.md](EMULATOR.md) §3.7). Bug de s1cc encontrado y corregido (la división leía la entrada pisada por
  `recipseed`). Intrínsecos y uso automático en s1cc; `asm/isa.json` (formas SFU, `floor`, `shl` con número, MAC
  medidas), `asm/dspsched.py` (`_native`). `MOD_DSP=auto` en SYSTEM-S1CC: ningún kernel de mutable_ported perdió
  voces, 8 ganaron. NATIVE-CHECK y BRAIDS-CSAW construidos, sin instalar.
