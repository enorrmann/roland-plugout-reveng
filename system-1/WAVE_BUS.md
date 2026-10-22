# SYSTEM-1: bus de audio entre DSP y chips (esclavo → maestro → salida)

Estado: 2026-10-22. Es el equivalente de `docs/WAVE_BUS.md` (SYSTEM-8) para el SYSTEM-1. Quién toca qué voz está
en [VOICE_ALLOCATION.md](VOICE_ALLOCATION.md). Fuentes: OS 1.30, SH-2/SH-101/PROMARS para SYSTEM-1 (sólo leídos),
C-S1V4 (`docs/examples_system_1_README.md`).

Leyenda: **[medido]** · **[código]** · **[inferido]** · **[abierto]**.

## 0. Resumen

```
                 ESCLAVO                                       MAESTRO
  DSP A ──banco 5 A100–A10D──► DSP B ──IOMD 6006–600D──► DSP B (PLUG-OUT: @0x1000) ──A118/A119──► DSP A (@0x82, del OS) ──► 6000/6001 ──► salida
     ▲                                                     (de serie: mezclador 6006–600D × C005–C00E)
     └──────────── IOMD 600E–6021 (20 palabras de modulación por voz) ◄── programa de modulación del maestro
```

| camino | palabras | quién escribe → quién lee | estado |
|---|---|---|---|
| esclavo A → esclavo B | banco 5 `A100`–`A10D` (de serie), `A10A`–`A10C` (SH-101), `A150` (C-S1V4) | principal de A → principal de B | [código], [medido] con C-S1V4 |
| esclavo → maestro | IOMD `6006`–`600D` (8 palabras) | principal del DSP B del esclavo → DSP B del maestro | [medido] con C-S1V4 (`6006`/`6007`) |
| maestro → esclavo | IOMD `600E`–`6021` (20 palabras) | modulación del maestro → principal del DSP A del esclavo | [código] |
| maestro B → maestro A | banco 5 `A118`/`A119` (L/R); `A11A`–`A11D` | DSP B → programa de salida del OS en el DSP A | [medido] con C-S1V4 |
| maestro A → salida | IOMD `6000` (L), `6001` (R) | programa de salida → convertidor / efectos del OS | [medido] (suena) |

## 1. Mecanismos (los mismos del SYSTEM-8)

* **IOMD** (`6nnn`, banco 3): palabras compartidas entre los DSP de un BMC y, por el bus de onda, entre el esclavo y el
  maestro. El esclavo escribe `6006`–`600D` y el maestro las lee con el mismo número; el maestro escribe
  `600E`–`6021` y el esclavo las lee [código]. Funciona: C-S1V4 manda su audio por `6006`/`6007` [medido].
* **Banco 5 de la pareja** (`Annn`): los DSP A y B de un BMC ven las mismas palabras. `A1xx` hace de buzón: lo que
  escribe A lo lee B y al revés (SH-2: A escribe `A10A`, B lo lee; maestro: B escribe `A118`, A lo lee) [código];
  C-S1V4 usa `A150` en el esclavo [medido]. El ARM escribe en el banco 5 con `FD6C(sel, obj, float)`: dirección
  `0x40101000 + 0x400·sel + 4·obj[2]` en el esclavo; `sel 0` = `A000+` [medido con C-S1V4], `sel 1` = `A100+`
  [inferido].
* **Banco 6** (`Cnnn`, parámetros del DSP, sólo lectura para el DSP): los escribe el ARM con `FCEC(sel, id, float)`
  (inmediato) o `FCE4` (con rampa); `sel` 0/1 = DSP A/B. C-S1V4 escribe `C01C` del maestro B con `FCEC(1, 0x1C, …)`
  [medido: el VOLUME funciona].
* **Banco 1** (`2nnn`): entradas y salidas de las secciones de voz, con el anillo de una muestra
  ([VOICE_ALLOCATION.md](VOICE_ALLOCATION.md) §3.1) [código].
* **Banco 0** (`0nnn`): datos propios del programa, también anillo (escribir `R`, leer `R+1` en la muestra
  siguiente): la fase de las sierras de C-S1V4 [medido: suenan afinadas].
* En el SYSTEM-1 no aparece el MXM (banco 4, `8nnn`) en los programas mirados: con dos DSP por BMC no hay cadena
  DSP0 → DSP2 → DSP4 [código, sólo en los programas leídos].

## 2. Qué corre en el maestro según el modo [código]

| | DSP A | DSP B |
|---|---|---|
| sinte de serie | modulación por voz (`0x60097A7C`, escribe `600E`–`6021`) y mezcla (`0x60095B0C`: `6006`–`600D` × `C005`–`C00E`, × `C00F` × `C010` → `6000`/`6001`); reparto exacto A/B [abierto] | |
| modo PLUG-OUT, sets que encola el OS (`0x6001ECA2`) | programa de salida en **`0x82`**: `A118`/`A119` → filtro de continua (`C002`, `C003`) → × `C004` → `6000`/`6001` | en **`0x1000`** un programa que pone `A118`–`A11D` a 0 |
| después, `load` del modelo del PLUG-OUT | SH-2: su modulación en `0x100` (el `0x82` del OS sigue) | SH-2: su mezcla en `0x1000`; **C-S1V4**: `6006`/`6007` × `C01C` → `A118`/`A119` |

Consecuencia: un PLUG-OUT que no carga su programa del DSP B del maestro **queda mudo** (el del OS escribe ceros), y el
volumen del panel lo tiene que aplicar el PLUG-OUT: el `C004` del programa de salida del OS no sigue al VOLUME
[medido: C-S1TONE sonaba fuerte con cualquier VOLUME].

## 3. Salidas del esclavo `6006`–`600D` [código]

El mezclador de serie del maestro las suma por parejas, L y R:

| L | R | ganancias |
|---|---|---|
| `6006` | `6007` | `C005`, `C006` |
| `600A` | `600B` | `C007`, `C008` |
| `600C` | `600D` | `C009`, `C00A` |
| `6008` | `6009` | `C00D`, `C00E` |

El principal B de serie escribe las ocho, todas × `C000` (su nivel de salida, lo gobierna el OS). Qué fuente va en cada
pareja (¿voces? ¿osciladores, sub, ruido?) está [abierto]. En modo PLUG-OUT las lee el programa del PLUG-OUT en el
maestro B, así que el reparto es libre: SH-2 usa `6006`–`600B`, C-S1V4 `6006`/`6007` y pone las demás a 0.

## 4. Medidas (2026-10-22)

| prueba | resultado |
|---|---|
| C-S1TONE: voz 0 fija (La), sin notas | suena: carga de programas, `A150`, `6006`/`6007`, maestro B `0x1000`, `A118`/`A119` correctos [medido] |
| C-S1TONE con el VOLUME | no cambia: el VOLUME no pasa por el programa de salida del OS [medido] |
| C-S1LOG (captura de parámetros) | VOLUME = id `0x100`, 0–255 [medido] |
| C-S1V4: notas, acordes de 4, VOLUME | todo correcto, a oído [medido] |

Sin hacer: captura de audio por USB (afinación exacta, nivel, si la salida es estéreo idéntica), barrido de qué otras
palabras IOMD llegan a la salida (como `docs/WAVE_BUS.md` §4b del SYSTEM-8).
