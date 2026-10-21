# Pendientes y falencias del ensamblador `asm/` (2026-10-21)

Origen: uso real de `asm/asm.py` para escribir el SAW propio (`examples/from_scratch/dsp/`). Cada punto se
comprobó con una llamada concreta al ensamblador; la columna «Corrección» dice qué hace falta.
La suite de tests (`asm/tests`) **no se ha ejecutado** en esta sesión (la ejecución se interrumpió).

## Falencias comprobadas

| # | Falencia | Prueba | Corrección necesaria |
|---|---|---|---|
| 1 | `mov` elige 16 bits en silencio: el valor se redondea y sólo hay una nota (`is stored in 16 bits as …`). | `mov r3, #0.041666666666666664` → 0.041656494 | Elegir 32 bits cuando el valor no es exacto en 16, o hacer del aviso un error. |
| 2 | La forma de 32 bits (`#v:32`, también `#v@0:32`) funciona pero **no está en la cabecera** de `asm.py` ni en `DSP_ISA_SPEC.md` §4.4. | `mov r3, #…:32` → `0x3070 0xaaaa 0xaaab` | Documentarla en el docstring y en la especificación. |
| 3 | `add rD, rX, rY` (tres operandos) se rechaza con `add takes 2 operand(s)`, sin sugerir `addxy`. | `add r0, r1, r2` | Mensaje con la alternativa: «para rD ← rX + Y usa `addxy`». |
| 4 | `sub` es idéntico a `add` (`rD ← rD + Y`, según `isa.json`): no resta nada útil con inmediatos. | `sub r1, #0.5` acepta y equivale a `add` | Renombrar, o avisar; la resta correcta es `subxy rD, rX, rY` (sólo registros). |
| 5 | `noop` se rechaza con `the table does not place 'empty'`, aunque `isa.json` lo lista como `noop`; `nop` da `field 'D' needs operand 'D'`. | `noop` y `nop` | Una sola forma aceptada y documentada: la línea vacía de la especificación §4 (`0001`). |
| 6 | Un `.class 0x00f0` delante de `ld` se rechaza (`no class in the table holds these units (T)`); sin `.class` la línea sí ensambla. | `.class 0x00f0` + `ld r0, [0400]` | Revisar la tabla de clases de transferencias (`T`) y la lectura de la pista `.class`. |
| 7 | **No hay etiquetas**: `jmp nowhere` → `not a jump offset … symbolic labels need a linker`. Los saltos hay que contarlos a mano (`jmp .+2`). | `jmp nowhere` | Dos pasadas con etiquetas y cálculo del desplazamiento (spec §5.4b). |
| 8 | `cond` exige tres operandos (`cc, skip_true, skip_false`), sin ejemplo en la cabecera. | `cond r0` | Documentar la forma y comprobarla contra el corpus. |
| 9 | **Sin comprobación de dependencias de carga**: `ld r0, [0400]` seguido de `mul r1, r0, …` ensambla sin aviso. El dato de `r0` sólo está listo a partir de la segunda instrucción (`re/dspasm.py`, `docs/NOTES.md`). | `ld r0,[0400]` + `mul r1,r0,#2.0` → ok, sin aviso | Aviso o error por uso inmediato de un registro recién cargado. |
| 10 | **Sin comprobación del anillo de memoria**: un `st` a `S` sólo se lee como `S+1` en la muestra siguiente. Nada lo avisa. | — | Aviso al leer `S` después de escribirlo en el mismo programa. |
| 11 | **Sin presupuesto de instrucciones**: unas 925–945 por muestra (`DSP_HARDWARE.md` §1); pasarse deja el equipo inestable hasta apagarlo. | — | Contar instrucciones por programa y por DSP, y avisar al superar el límite. |
| 12 | El simulador (`re/dspisa.py` + `re/dspasm.py simulate`) vive fuera de `asm/`. `dsp/make_iomd_test.py` duplica el bucle de simulación. | — | Mover un simulador de muestras a `asm/` que use las mismas reglas de anillo y salto. |
| 13 | Al ensamblar con `add`/`mov` con inmediato, el ensamblador elige un formato que puede no coincidir con `dspasm` (p. ej. `ymacd`). Lo comprobamos sólo por simulación. | `check_saw.py`: diferencia 0 con el SAW de `dspasm` | Mantener la prueba de equivalencia como regresión. |

## Relacionado (ya abierto en `docs/DSP_ISA_DISCREPANCIES.md`)

* **D8**: el coeficiente de un MAC con inmediato en Y puede leerse en la posición equivocada en diseños sin
  referencias. Afecta a cualquier programa con `mac_p1p2c*` y Y inmediato. No usar esas formas hasta cerrar D8.
* **D10**: lecturas más allá del final de una instrucción (el hardware lee la siguiente). Relevante si el
  ensamblador debe reproducir código de Roland.

## Orden sugerido

1. Falencias 1, 2 y 3: afectan a cualquier programa nuevo (precisión y mensajes).
2. Etiquetas (7) y la forma `cond` (8): sin ellas no hay LFO ni envolvente con ramas.
3. Comprobaciones de hazard y anillo (9, 10) y presupuesto (11): antes de cargar nada en el equipo.
4. Simulador dentro de `asm/` (12) y la prueba de equivalencia como test (13).
5. Revisar 4, 5 y 6 (nombres y clases), y ejecutar la suite completa de `asm/tests` antes de cerrar.
