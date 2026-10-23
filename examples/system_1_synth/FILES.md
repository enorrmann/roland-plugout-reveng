# examples/system_1_synth: qué es cada archivo

Documento principal: [../../docs/examples_system_1_synth_README.md](../../docs/examples_system_1_synth_README.md).

## Programas DSP generados (`*.s` = fuente legible, `*.hex` = palabras que se cargan)

Los genera `make_synth.py` (con `STEP=n`); no se editan a mano. Cada SYSTEM-1 tiene dos chips BMC (maestro y esclavo)
con dos DSP cada uno (A y B). El nombre dice dónde corre cada programa:

| archivo | chip | DSP | dirección | qué hace | lo carga |
|---|---|---|---|---|---|
| `s1m0.s` | esclavo | A | `0x0100` (programa principal) | las 8 voces: DCO1 + DCO2 (+ copia de puerta, disparo y tono a `A1xx` para el DSP B); `m0` = main del DSP 0 (A) | `engine.c`, cola `FC30` |
| `s1m1.s` | esclavo | B | `0x0100` (programa principal) | por voz: ADSR del VCA y filtro paso bajo; mezcla → IOMD `6006`/`6007` hacia el maestro; `m1` = main del DSP 1 (B) | `engine.c`, cola `FC34` |
| `s1c0.s` | esclavo | A | `0x7FF2` (cierre, final de la memoria de programa) | contador de trama `E000` → `A004`, como todo PLUG-OUT del SYSTEM-1; `c0` = close del DSP A | `engine.c`, cola `FC30` |
| `s1c1.s` | esclavo | B | `0x7FF2` (cierre) | contador de trama `E000` → `A005`; `c1` = close del DSP B | `engine.c`, cola `FC34` |
| `s1mm1.s` | maestro | B | `0x1000` | `6006`/`6007` del esclavo × VOLUME (`C01C`) → `A118`/`A119`, que lee el programa de salida del OS en el DSP A del maestro; `mm1` = master, main del DSP 1 (B) | `model.c`, cola `FC34` del maestro |

El DSP A del maestro ejecuta el programa de salida del propio OS (en `0x82`): no es nuestro.

## Código ARM y build

| archivo | qué es |
|---|---|
| `make_synth.py` | generador de los programas DSP y de `build/s1_programs.h` (tablas: tonos, volumen, envolventes, filtro); `--sim` simula |
| `engine.c` | región 1, ARM del esclavo: carga `s1m0`/`s1c0` en el DSP A y `s1m1`/`s1c1` en el DSP B; notas → banco 5; panel → banco 5 (`FD6C`) o parámetros `C0xx` (`FCEC`) |
| `model.c` | región 0, ARM del maestro: asignador de 8 voces, carga `s1mm1`, VOLUME; `PARAMLOG=1` manda los parámetros al PC |
| `syninfo.h` | la estructura SYNINFO del SYSTEM-1 y los servicios del OS |
| `link.ld` | mapa de las regiones (SYNINFO, patches, código) |
| `build.sh` | build completa → `plugins/<NAME>/Script/Code1.Dat` (`STEP=n`, `NAME=...`, `PARAMLOG=1`) |
| `paramlog.py` | copia de `examples/system_1/paramlog.py` (la captura se lanza con la de `examples/system_1/`) |
| `build/` | intermedios (no en git) |
