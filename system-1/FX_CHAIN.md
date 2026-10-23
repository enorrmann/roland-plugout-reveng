# Cadena de efectos (delay + reverb) del SYSTEM-1 en modo PLUG-OUT

Especificación de estudio: qué programas DSP del OS forman el delay y la reverb del panel, dónde corren, qué
palabras de memoria comparten con nuestro PLUG-OUT y qué parámetros los gobiernan. Fuentes:
* OS 1.30 (`work/sys1_v130/app_master.bin`, base `0x60000000`), listado con `system-1/s1_progs.py` (`sets`, `words`)
  y `asm/disasm.py`;
* los PLUG-OUTs SH-2, SH-101 y PROMARS (`work/unpack/<NOMBRE>_S1_Code1`), sólo leídos;
* la captura PARAMLOG del 2026-10-23 con C-S1SY12L.

Leyenda: **[código]** visto en los binarios · **[medido]** en el equipo · **[inferido]** · **[abierto]**.

Estado: 2026-10-23. Con C-S1SY ≤ 12 el efecto **no suena** [medido: reverb y delay al máximo, sin efecto
audible]. Causa: el principal del maestro B del PLUG-OUT tiene que hacer el envío y el retorno (§4). Prueba en curso
con el paso 13 (§5b).

## 1. Controles del panel [medido]

PARAMLOG con C-S1SY12L (`examples/system_1/paramlog.py`), en el orden en que se movieron:

| control | id | CC (canal 5) | valores |
|---|---|---|---|
| REVERB amount | `0x18D` | 91 (`B4 5B`) | 0–255 |
| DELAY amount | `0x18E` | 94 (`B4 5E`) | 0–255 |
| DELAY time | `0x18F` | 13 (`B4 0D`) | 0–255 |

Los tres llegan al modelo de la región 0 (maestro) por `vt+0x30(id, flag, valor)`, como el resto de los
parámetros del patch. `0x18B` (valores 103–128, CC 69) aparece otra vez intercalado; sin identificar.

El OS los trata como parámetros genéricos del grupo 3 [código]. `0x60007302`/`0x6000730c`/`0x6000731a` cargan el id
y saltan a `0x6000673c`; `0x6000733a`… saltan a `0x60007fee`, con `r0 = 3`. Es el mismo patrón que para
`0x18A`–`0x190`, y no hay ningún caso especial para los efectos. La tabla de `0x60013d6e` (CC → id) contiene
`0x18D`–`0x18F` en su sitio. El SH-2 tiene la misma tabla CC → id en su región 0 (`0x608094f2`: `0x18D`,
`0x608094fe`: `0x18F`).

## 2. Programas DSP del maestro en modo PLUG-OUT [código]

`s1_progs.sets()` sobre el OS encuentra 16 sets en el maestro. Los de modo PLUG-OUT (los encola `0x6001ECA2`,
[log/WAVE_BUS_LOG.md](log/WAVE_BUS_LOG.md) §2):

| set | DSP | dirección | tamaño | qué es |
|---|---|---|---|---|
| 11 | maestro A | `0x82` | 126 B, 12 instrucciones | **salida**: `A118`/`A119` → filtro de continua → banco 1 (para el efecto) → mezcla con el retorno del efecto → × `C004` → `6000`/`6001` |
| 11 | maestro A | `0x6DF2`, `0x7FF2` | 14 B cada uno | cierres |
| 12 | maestro B | **`0x0`** | 1362 B, **107 instrucciones** | **delay + reverb** (§3): la única que usa la memoria de retardo (40 accesos al banco 2, `tapreq`, `h4`/`h5`) |
| 12 | maestro B | `0x1000` | 56 B, 11 instrucciones | `A118`–`A11D` = 0. El `load` del PLUG-OUT lo sustituye (C-S1SY: `s1mm1`) |
| 12 | maestro B | `0x5FF2`, `0x6FF2`, `0x7FF2` | 14 B | cierres |

Los sets 14 y 15 son los del sinte de serie (maestro A `0x100`, 696 instrucciones de modulación; maestro B
`0x1000`, 606 de mezcla). Ninguno de los dos usa la memoria de retardo.

**Ningún PLUG-OUT de Roland lleva efecto propio** [código]. Ni el SH-2 ni el SH-101 ni el PROMARS tienen accesos al
banco 2 (`4xxx`), `tapreq` ni registros ocultos en sus programas, ni en el maestro ni en el esclavo. El maestro B del
SH-2 (`0x1000`, 406 instrucciones) es mezcla, filtro y saturación con `C01C`–`C024`. Los efectos del panel los hace
siempre el programa del OS en `0x0` del maestro B. Nuestro `load` sólo reemplaza `0x1000`, así que ese programa sigue
corriendo con C-S1SY [inferido: el set 12 se carga y nada lo quita].

## 3. El programa del efecto (maestro B `0x0`) [código]

Orden de ejecución por muestra; instrucciones numeradas como en el listado (`disasm.program`, sin la cabecera
`f7`):

| instr. | qué hace | parámetros / memoria |
|---|---|---|
| 1–10 | estado (`A000`, condiciones 25–27 de `E021`) | `A000`–`A003`, `E013`, `E020`, `E021` |
| 11–16 | **entrada**: lee `2000` / `2002` (el envío que deja el principal del maestro B en esa muestra, §4), filtro de una etapa | estado `0002`–`0006` |
| 17–45 | **delay**: lectura de la memoria de retardo por `tapreq` (base `h4` = 256, resultado en `h5`), interpolación entre dos muestras (`4005`/`4006`, `4008`/`4009`), realimentación con filtro de una etapa (0,994…, 0,996…), escritura `4001` | `C000`, `C001`, `C004`, `C005`, `C006`, `C007` (nivel, realimentación, tiempo [inferido por el uso]) |
| 46–53 | **mezcla del delay** con la entrada (`ymacd`, `C001`, `C002`, `C003`) | → `0000` / `0001` |
| 54–61 | **entrada de la reverb**: (L + R)/32, × `C009`, dos filtros de una etapa (0,989…, 0,9946…) | `C008`, `C009` |
| 62–76 | **líneas de la reverb** en el banco 2 (`400B`–`4015`): cuatro etapas `mac_p1w` en cascada, modulación con `ph_wrap` / `ph_abs` de una fase lenta (`0x22`) | `C00C` |
| 77–95 | **allpass** en cascada (coeficientes −0,579 / 0,486 / 0,877, −0,510 / 0,336) sobre `4017`–`402D`, salidas L/R por separado | — |
| 96–104 | **salida**: × `C00A`, × `C008`/`C00B`, mezcla con la entrada (`ymacd`), → `2000` / `2002` | `C008`, `C00A`, `C00B` |

Resumen: **entrada → delay (`C000`–`C007`) → reverb (`C008`–`C00C`) → `2000`/`2002`**. El significado exacto de
cada `C00x` (nivel, realimentación, tiempo) se infiere por dónde se usa, no está medido [inferido].

## 4. Envío y retorno: los hace el principal del maestro B [código, medido en el emulador]

**Corrección (2026-10-23, más tarde):** el efecto **no** toma su entrada del programa de salida del maestro A. Su
envío y su retorno viven en el **banco 1 del propio maestro B** y los hace el **principal de `0x1000`**. En modo
PLUG-OUT ese principal es el del PLUG-OUT.

* El mezclador de serie (maestro B `0x1000`, set 15) termina con
  `st r0, [2000] | st r1, [2002] | ld r0, [2001] | ld r1, [2003]` [código]:
  1. escribe la mezcla L/R en `2000`/`2002` (envío);
  2. el programa del efecto, en el mismo DSP, la lee en esa muestra y deja en `2000`/`2002` el resultado
     (seco + delay + reverb, §3);
  3. el principal lee ese resultado en la muestra siguiente en `2001`/`2003` (anillo del banco 1: lo escrito en S
     se lee en S+1) y lo pasa a la salida.
* El programa de salida del maestro A (`0x82`) usa `2000`/`2001` y `2002`/`2003` de **su** banco 1 sólo como estado
  del filtro de continua. Lo midió el emulador ejecutándolo solo, con los parámetros del §5:
  `6000 = A118 − [2001]`, `[2006] = A118 − [2001]`, `[2000]` = estado del filtro (fuga 0,99993), y
  `[2004]`/`[2005]` entran en la salida con `C002` = 0,0469. Los bancos 1 de los DSP A y B del maestro son
  distintos [inferido de lo anterior].

Cadena completa en modo PLUG-OUT:

```
esclavo A/B (nuestro)          -> 6006/6007
maestro B 0x1000 (PLUG-OUT):    6006/6007 x volumen -> 2000/2002 (envío)
maestro B 0x0    (OS, efecto):  2000/2002 -> delay (C000-C007) -> reverb (C008-C00C) -> seco + húmedo -> 2000/2002
maestro B 0x1000 (PLUG-OUT):    2001/2003 (resultado de la muestra anterior) -> A118/A119
maestro A 0x82   (OS, salida):  A118/A119 -> filtro de continua -> x C004 -> 6000/6001
```

**Por qué no sonaba:** `s1mm1` (C-S1V4, C-S1SY ≤ 12) escribía `6006`/`6007` × volumen directamente en
`A118`/`A119` y se saltaba el envío y el retorno. El SH-2 tampoco usa `2000`–`2003` en sus programas del maestro
[código]; si su efecto suena en el equipo [abierto], tiene que ser por otro camino.

## 5. Los parámetros los escribe el OS [medido, C-S1SYFX, 2026-10-23]

Build de diagnóstico `PARAMLOG=1` con `fx_dump()` en `model.c`. Lee los bloques de parámetros del maestro como los
escribe `set_param` del OS:
* `0x01008D26`: `sel` 0 → `0x4014C000`, `sel` 1 → `0x4016C000`, `+ 16·id`;
* palabras: `+0` modo (`0xE0000000` inmediato, `0xC0000000` en rampa), `+4` valor, `+8` valor actual o destino,
  `+0xC` paso de la rampa.

Los manda por SysEx al pasar REVERB / DELAY / DELAY time por 0, 64, 128, 192 y 255. **El OS escribe él mismo los
parámetros del efecto al mover los potes**; el modelo no tiene que hacer nada:

| parámetro | qué lo cambia | valores medidos (palabra `+4`, float) |
|---|---|---|
| maestro B `C009` (envío a la reverb) | REVERB `0x18D` | 0 → 0,0382 (64) → 0,1625 (128) → 0,464 (192) → 1,0 (255), ≈ (v/255)^2,5 |
| maestro B `C002` (nivel del delay) | DELAY `0x18E` | lineal: 0,25 (64), 0,75 (192), 1,0 (255) |
| maestro B `C001` | DELAY `0x18E` | 1,0 con el delay > 0, si no 0 |
| maestro B `C004` | DELAY `0x18E` | 0,6 con el delay > 0, si no 0 |
| DELAY time `0x18F` | — | ninguno de `C000`–`C010`. Con el delay a 0 no cambia nada. El efecto lee `A000`–`A003` de su banco 5, probablemente las posiciones de lectura [abierto] |

Fijos en modo PLUG-OUT:
* maestro B: `C000` = 1, `C003` = 1, `C007` = 1, `C008` = 1, `C00A` = 0,498, `C00B` = 1, `C00C` = 0,5;
  `C005`, `C006`, `C00D`–`C010` = 0;
* maestro A: `C000` = 0, `C001` = 1, `C002` = 0,0469, `C003` = 1, `C004` = 1, `C005`–`C007` = 0.

Servicios que usan los tres PLUG-OUTs de Roland y C-S1SY no: `FC30` (cola del DSP A del maestro), `FCE4`
(parámetro con rampa), `FD6C`, `FE14` y `FE28`. Lo que hacen en el OS:
* `FE14(x)` → `0x01009EEC`: pone o quita un bit de un registro del chip (máscara `0x10` si x ≠ 0, `0x08` si no);
* `FE28(v)` → `0x01009F3E`: escribe con rampa el parámetro `sel` 0, id 0 a partir de v (float, × 1,0, a punto fijo
  con desplazamiento −24).

El SH-2 los llama desde los métodos de ciclo de vida de su modelo (`0x60809DE8`…`0x60809E84`). `FE14` sólo con x = 0
en lo visto. Qué hacen exactamente: [abierto].

## 5b. Prueba en el equipo: C-S1SY paso 13

`make_synth.py` `STEP=13`: `s1mm1` pasa por el efecto, como el mezclador de serie.

```dsp
ld r0, [6006]
ld r1, [6007]
ld r7, [c01c]
mov r4, r4
mul r0, r0, r7
mul r1, r1, r7
st r0, [2000]
st r1, [2002]
ld r2, [2001]
ld r3, [2003]
mov r4, r4
st r2, [a118]
st r3, [a119]
```

(Seguido de las escrituras a 0 de `A11A`–`A11D`; el planificador lo deja en 7 instrucciones.) Si el principal
corriera **antes** que el efecto en cada muestra, el efecto no recibiría nada. Pero la salida seguiría sonando: el
principal leería su propio envío de la muestra anterior. El arreglo no puede dejar el equipo mudo.

Resultado: **funciona** [medido, C-S1SY13, 2026-10-23]: reverb, delay y delay time suenan; con los efectos a 0 suena seco.

## 6. Siguientes pasos

1. Escuchar C-S1SY13 (§5b): reverb, delay, delay time y el sonido seco con los efectos a 0.
2. Si suena: documentar el paso 13 en [../docs/examples_system_1_synth_README.md](../docs/examples_system_1_synth_README.md)
   y llevar el mismo `s1mm1` a C-S1V4 (`examples/system_1/make_s1.py`).
3. Si no suena: medir el orden principal / efecto dentro de la muestra. Por ejemplo, con un principal que escriba
   una marca en `2000` y la región 0 leyendo lo que vuelve. Después mirar `FE14` / `FE28` (§5) y el banco 5
   `A000`–`A003` del efecto (DELAY time).

## 7. El pote CRUSHER y AMP TONE: efecto propio del PLUG-OUT [medido / código]

* PARAMLOG con C-S1SYFX (volcado ampliado del maestro: B `C000`–`C03F`, A `C000`–`C00F`): CRUSHER `0x18C` (CC 12) y
  AMP TONE `0x18B` (CC 69, bipolar, 128 = centro) **no cambian ningún parámetro del maestro**. El OS no los
  implementa en modo PLUG-OUT.
* SH-2, SH-101 y PROMARS los implementan en su principal del maestro B (`0x1000`): leen `C01C`–`C037`, escritos por
  su modelo, con la máscara de crusher `bic r4, r1, -r5`. Sus manuales los llaman EFFECT DEPTH ("modifica el
  carácter tonal distorsionando la forma de onda") y AMP TONE (brillo). En el VST, `EFFECT TYPE` (0–5) elige el
  efecto; los nombres no aparecen en las `Script.xml` / `ScriptEdit.xml`, que sólo traen `EFX CRUSHER`.
* El botón LPF type sólo manda el CC 115, sin parámetro para el modelo.
* En el SYSTEM-8, cada PLUG-OUT lleva sus efectos en la ranura de efecto del DSP4 (JX-3P / Juno: OVERDRIVE,
  DISTORTION, CHORUS, FUZZ, CRUSHER, PHASER). El chorus/ensemble es global del maestro (`m1_tcm` `efdc`/`f7dc`) y el
  del Juno-60 está en `J60_b000_60a1f8e8`: LFO triangular, dos tomas interpoladas en la memoria de retardo, filtros
  de BBD. Los intentos de C-S1SY están en
  [../docs/examples_system_1_synth_README.md](../docs/examples_system_1_synth_README.md) §7.4.

