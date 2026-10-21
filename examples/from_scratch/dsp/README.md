# DSP de la voz propia: microcódigo en el lenguaje de `asm/`

Estado: **sólo el oscilador SAW (DCO2) está escrito y verificado en simulación**. No está instalado ni
probado en el equipo, y todavía no lo llama ningún engine propio. Diario completo y decisiones:
`../MINI_PLUGOUT.md`.

## Ficheros

| fichero | qué es |
|---|---|
| `saw_dco2.s` | fuente del SAW de los dos caminos de voz (A y B) del slot DCO2, en la sintaxis de `asm/asm.py` |
| `check_saw.py` | ensambla la fuente y compara su salida simulada con `re/dspasm.py saw('dco2')` |

Comprobación (desde la raíz del repositorio):

```
python3 examples/from_scratch/dsp/check_saw.py
```

Resultado actual: `assembled 62 instruction(s), rejected 0, 0 note(s)`; diferencia máxima con el SAW de
`dspasm` 0 a 300 muestras y tonos 4.75, 6.0 y 8.0.

## Precisión de la constante 1/24

`mov r3, #1/24` con 16 bits daba un error de 1.45·10⁻⁶ a tono alto. Se usa la forma de 32 bits
`mov r3, #0.041666666666666664:32` (sintaxis no documentada en `asm/`, ver `docs/ASM_PENDIENTES.md` falencia 2).
Con ella `check_saw.py` da diferencia **0** y el umbral es 1e-6.

## Qué hace el programa (en cada muestra, por voz)

El programa corre una vez por muestra (96 kHz). Para la voz A (la B usa las direcciones de su columna):

1. `mov r5, #0.0` y `mov r6, #1.0` al principio (constantes de máximo y mínimo).
2. `ld r1, [0671]`: fase de la muestra anterior. La memoria de datos es un anillo: lo que se guarda en `S` se lee
   en `S+1` de la muestra siguiente, por eso el estado se carga de `S+1` y se guarda en `S`
   (`docs/DSP_ISA_SPEC.md` §9).
3. `ld r0, [2207]`: tono `p` que escribe el programa principal de Roland (ver «Entradas» abajo).
4. `x = p − 4.75`; `2^(x/16)` con la serie de Taylor (`ymacd`) elevada a 16 (cuatro `mul r3, r3, r3`).
   Incremento de fase = `2^x · 2·440/96000`.
5. Fase nueva = fase + incremento; luego se envuelve a [−1, 1) sin ramas: `q = max(fase − 1, 0)`, cuatro veces
   `q = min(256·q, 1)`, y `fase −= 2q`.
6. Se guarda la fase en `[0670]` (estado) y la salida en `[2221]` y `[221d]`.

Igual que en `dspasm.saw`: la salida es la fase, que ya es la diente de sierra de amplitud 1.

## Entradas, salidas y estado (direcciones medidas en el equipo, `docs/CUSTOM_OSC.md` §2)

| | voz A | voz B |
|---|---|---|
| tono de entrada | `[2207]` | `[2214]` |
| salidas | `[2221]`, `[221d]` | `[2229]`, `[2225]` |
| estado (fase) | `[0670]` (lee `[0671]`) | `[0680]` (lee `[0681]`) |

Estas direcciones vienen de `re/dspasm.py` `SLOTS['dco2']`. **Importante**: el tono `[2207]` hoy lo escribe el
programa principal de Roland (MAIN2/3), que no es nuestro. Hasta que haya un programa principal propio, el
SAW no recibe tono de nuestro engine y no puede sonar en un plugout sin Roland.

## Mnemónicos de `asm/` usados y su semántica (`asm/isa.json`)

| línea | significado | estado en `isa.json` |
|---|---|---|
| `mov rD, #v` | rD ← v (inmediato de 16 bits) | exact |
| `add rD, #v` | rD ← rD + v | exact |
| `mul rD, rX, #v` / `mul rD, rX, rY` | rD ← rX · Y | exact |
| `ymacd rD, rX, #v` | rD ← v + rX · rD | exact |
| `addxy rD, rX, rY` / `addxy rD, rX, #v` | rD ← rX + Y | exact |
| `subxy rD, rX, rY` | rD ← Y − rX | exact |
| `max`, `min` | max/min de dos registros | exact |
| `ld rR, [dir]` / `st rR, [dir]` | memoria directa | exact |

`add` y `sub` en `asm/` son `rD ← rD + Y` (el `sub` no resta). Para restar se usa `subxy`. Esto contrasta con
`re/dspasm.py`, donde `a('add', d, x, imm=v)` escribe `rd ← rx + v`; por eso el programa usa mnemónicos
explícitos y no una traducción literal.

## Cómo se comprobó

* Se simuló con `re/dspisa.py` mediante el mismo bucle que `dspasm.simulate` (anillo, saltos, un solo
  ejecutable por muestra). Ambos programas dan la misma diente de sierra dentro de la tolerancia de arriba.
* Prueba de salida en el equipo (`make_iomd_test.py`, `analyze_iomd.py`, `docs/MINI_PLUGOUT.md` §4): sólo la
  palabra `6020` llega a la salida; `60B0` y `60B1` no. El SAW propio aún no tiene nota ni puerta.

## Prueba de salida (IOMD)

`make_iomd_test.py` escribe tres SAW fijos (440, 660 y 880 Hz) desde el slot DCO1 del DSP5 a `60B0`, `60B1`
y `6020`. Se registra con `engine/build.sh` usando `IOMDTEST=1 OWN_PROGS=iomd5=iomd_test.txt NAME=C-IOMD`.
Captura de PLG3: sólo 880 Hz en el canal derecho, armónicos 1/n. Ver `docs/MINI_PLUGOUT.md` §4.
