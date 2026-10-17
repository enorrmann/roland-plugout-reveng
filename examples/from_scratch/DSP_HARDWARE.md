# DSP del SYSTEM-8 (chip BMC de Roland): referencia técnica del hardware

Documento único, sin historia, para quien quiera **escribir un ensamblador, un compilador o un emulador** del
DSP. Todo lo que dice está medido en el equipo salvo que se indique. Detalle exhaustivo (tabla de clases, ejemplos,
mediciones originales): `DSP_ISA_SPEC.md`. Diario de trabajo: `NOTES.md`. Estado: 2026-10-17.

Certeza: **[exacto]** = verificado bit a bit o en todos los casos; **[medido]** = medido en el equipo, casos
representativos; **[inferido]** = deducido del corpus o del lado ARM, sin medición directa; **[desconocido]**.

---

## 1. Visión general

| ítem | valor | certeza |
|---|---|---|
| Chips | 3 BMC (maestro + 2 slaves de 4 voces cada uno), iguales en hardware | [inferido] |
| DSP por BMC | 6 (DSP0..DSP5) + DSPM (sin estudiar) | [inferido] |
| Frecuencia de muestreo | 96 kHz; el programa de cada DSP corre entero una vez por muestra | [exacto] |
| Ciclos por muestra | ≈ 925–945 (≈ 90 MHz) en todos los DSP | [medido] |
| Costo | **1 ciclo por instrucción**, cualquiera sea su contenido (1 op, A/A/B, 1 o 2 MAC, hasta 6 transferencias, marcadores); salto tomado +1 ciclo; lo salteado por un salto no cuesta | [medido] |
| Pasarse del presupuesto | se pierde el final del programa; en los slaves deja el equipo inestable hasta apagarlo | [medido] |
| Reloj común | todos los DSP de un BMC comparten el contador de muestras (posición de escritura del retardo idéntica en DSP3 y DSP4) | [medido] |

Cada DSP ejecuta **varios programas por muestra**: un programa principal y "ranuras" (slots) que el ARM agrega
(osciladores, efectos). Los programas de un DSP **comparten r0–r7, las banderas y los registros ocultos**: nada
se reinicia entre programas ni entre muestras **[medido]**.

---

## 2. Estado visible del programador

| recurso | descripción | certeza |
|---|---|---|
| r0–r7 | 8 registros de 32 bits en formato RAW (§3) | [exacto] |
| h4–h7 | registros ocultos: h4 = base de retardo / suma de `A op0 sub4`; h5 = destino de tomas de retardo / `sub5`; h6 = índice de transferencias indexadas; h7 = sin lector conocido. Se cargan con palabras `40TY` (§5.6) | [medido] |
| banderas | 32 bits de condición = 8 grupos × 4 (§6). Grupos 1–5 escribibles; grupo 0 fijo `0111`; grupos 6–7 fijos `1111` | [medido] |
| PC | contador de programa en palabras de 16 bits; saltos relativos (§6.3) | [medido] |
| puntero de anillo | avanza una palabra por muestra; direcciona el banco 0 (§7.2) | [medido] |
| posición del retardo | contador global que baja 1/16384 por muestra (mod 512); legible con `f5c` (§5.3) | [medido] |

---

## 3. Formato numérico RAW **[exacto]**

```
bit 31     signo (1 = negativo)              valor = (−1)^signo · M / 2^(32 − 4·s)
bits 30-29 exponente s (0..3)
bits 28-0  mantisa M (29 bits, sin signo)
```

| s | rango de \|v\| | resolución |
|---|---|---|
| 0 | < 1/8 | 2^−32 |
| 1 | < 2 | 2^−28 |
| 2 | < 32 | 2^−24 |
| 3 | < 512 | 2^−20 |

**Aritmética (verificada bit a bit en el maestro, `re/dspexact.py`)**:
* Las ops calculan el resultado **exacto** (sin redondeos intermedios: `D + X·Y`, la MAC con dos productos más W,
  la MAC D ≥ 8 = resultado/2) y lo codifican **una sola vez**, **truncando hacia cero** al exponente s más chico
  en el que la mantisa entra en 29 bits. 48/48 casos al azar en suma, resta, producto, multiplicar-acumular, MAC
  y MAC/2.
* **Desborde**: si |resultado| ≥ 512, la magnitud se toma **módulo 512** y se conserva el signo (300 + 300 → 88;
  256 + 256 → 0 con bandera V). Exacto en 48/48.
* Las ops lógicas (§5.2) trabajan en complemento a 2 de punto fijo (23 bits de fracción, rango 512) y devuelven el
  resultado **módulo 512** (un negativo se lee como 512 + x: palabras `7ffxxxxx`).
* El ARM ve la memoria en tres vistas: float IEEE, Q31 ("SD31") y RAW; el DSP sólo usa RAW.

---

## 4. Programa e instrucciones

### 4.1 Imagen de programa **[exacto]**
* Flujo de palabras de 16 bits. En la memoria de programa cada palabra de 32 bits little-endian lleva dos
  palabras del flujo, **la mitad alta primero**.
* Programa de ranura: empieza con `0a02 ff70` y termina con `0a02 ff70 0b02 00f8 0001` (el `00f8` final es el
  retorno de la ranura). Programa principal del DSP3 del maestro: `0001 0001 … 0b03 0080 7fee 0001`. SYSTEM-1:
  empieza con `0a02 ff71`.
* `0001` = no-op de una palabra.

### 4.2 Instrucción
```
cabecera = clase[15:5] | largo[4:0]     largo = palabras, cabecera incluida (1..31)
cuerpo   = [ops A/B/M y palabras C/J] [transferencias T] [área de slots: N de las MAC, referencias R, inmediatos]
```
* La **clase** (11 bits) identifica una **plantilla** que dice qué es cada palabra del cuerpo; no es un campo de
  bits. Hay 139 clases conocidas (todas las del corpus de Roland), cada una medida o verificada en el equipo
  (`re/dsp_templates.txt`, tabla en `DSP_ISA_SPEC.md` §12). Una clase que no está en la tabla hay que medirla
  (`re/dspslots.py`, `re/dspverify.py`) antes de usarla. **[medido]**
* Las ops de una instrucción **leen todas antes de escribir** (r4 = r3 + r1 y r3 = … en la misma instrucción
  ven el r3 viejo). Un store en la misma instrucción guarda el valor **viejo** del registro. **[exacto]**

### 4.3 Área de slots e inmediatos **[medido, verificado sobre 100 instrucciones al azar]**
* Área de slots = las palabras después de la última T (sin T: las palabras N e I). Primero una palabra N por
  cada MAC; luego referencias e inmediatos mezclados.
* Una transferencia apunta a la **palabra de slot Y >> 1**. Un salto toma su desplazamiento igual.
* Un **inmediato** (modo 1 = 16 bits, 2 = 24, 3 = 32: los bits altos del RAW, el resto 0) de una op A/B empieza
  **Y bytes** dentro del área de slots (Y = el campo Y de 4 bits de la op). Varias ops pueden leer el mismo
  inmediato o distintos. En la MAC: Y en el byte Y, W en el byte W, el coeficiente C **a continuación** del
  inmediato de Y (o en el byte C si Y es registro). La lectura que pasa de una palabra a la siguiente **saltea
  las palabras usadas como referencia**. Si el inmediato se pasa del final de la instrucción, el hardware sigue
  leyendo las palabras siguientes del programa.

### 4.4 Palabras C (condición), J (salto) y `40TY` **[medido]**
```
C:  bits 15-14 = 00 | cc[13:9] (condición 0..31) | neg[8] | skipT[7:4] | skipF[3:0]
    cond = bandera[cc] XOR neg ; si cond: no se ejecuta la unidad skipT; si no: la unidad skipF
J:  0x009Y  = salto (unidad más, se cuenta como op)          40TY = carga de registro oculto (§5.6)
```
* **Unidades** = ops (A, B, M, J) en orden de posición, después las transferencias, numeradas desde 1.
* El campo elegido sólo actúa si el **bit 3 del otro campo es 0**.
* Campo 0 o fuera de rango: no hace nada, **salvo** en clases con tres ops A y una sola C (0x2c8 AAAC, 0x570,
  0x240 MAAC), donde **anula todas las ops**. Hay numeraciones especiales por clase (0x190: el campo 2 también es
  la op). Roland casi no usa campos fuera de rango (< 0.5 % de sus instrucciones); un compilador debe evitarlos.

---

## 5. Unidades de cálculo

Palabra de op (A y B): `D[15:12] op[11:9] X[8:6] mY[5:4] Y[3:0]`. mY = 0: Y es registro (bit 3 = negar,
bits 2–0 = registro); mY = 1/2/3: inmediato de 16/24/32 bits (§4.3). "Nibble" = bits 11–8 = op·2 + X[2].

### 5.1 Unidad A **[exacto]**

| op (D < 8) | rD ← |
|---|---|
| 1 | D + X + Y |
| 2 | X · Y |
| 3 | D + X · Y |
| 4 | Y + X · D |
| 5 | Y − X · D |
| 6 | X + Y |
| 7 | Y − X |
| 0, sub X = 1 | Y (mover / cargar inmediato) |
| 0, sub 2 | paso de Newton: rD ← D·r(D+1); r(D+1) ← 2 − r(D+1)·rY; rY ← r(D+1)·rY |
| 0, sub 4 / 5 | D + Y + h4 / h5 |
| 0, sub 6 / 7 | D + Y |
| 0, sub 0 / 3 | nada |
| D ≥ 8 | r(D−8) ← X + Y + r[op] (op indexa un tercer registro) |

### 5.2 Unidad B

| forma | resultado | certeza |
|---|---|---|
| D < 8, nibble 0 / 2 / 4 / 6 / 8 / a | X AND Y / (NOT X) AND Y / X OR Y / (NOT X) OR Y / X XOR Y / NOT(X XOR Y) — complemento a 2, resultado mod 512 | [medido] |
| D < 8, nibble c / e | \|X\| + Y / Y − \|X\| | [exacto] |
| D ≥ 8, nibble 0 / 2 / 4 / 6 | min(X, Y) / min(−X, Y) / max(X, Y) / max(−X, Y) | [exacto] |
| D ≥ 8, nibble 8 / a | \|X\| / \|Y\| | [exacto] |
| D ≥ 8, nibble c, modo 0 | frexp: X = m·2^e, m ∈ [0.5, 1): r(d) ← m, rY ← e | [medido] |
| D ≥ 8, nibble c, modo 1 / 2 / 3 | X·2^Y / X·2^−Y / X·2^−(16+Y) (Y = el número del campo) | [medido] |
| D ≥ 8, nibble e, bits 7–6 = 00 | **CMP**: banderas de r(d) + Y, no escribe | [medido] |
| D ≥ 8, nibble e, bits 7–6 = 01 | r(d) ← Y | [medido] |
| D ≥ 8, nibble e, bits 7–6 = 11 | según bits 5–4 y Y bit 3: 00 r(d) ± rY; 01 \|rY\| / clamp(rY, −1, 1); 10 wrap(rY) a [−1, 1) / triángulo 1 − 2\|wrap(−rY − 1.5)\|; 11 (rY mod 2)/2 / −rY | [medido] |
| D ≥ 8, nibble f | SFU (§5.3) | |
| nibbles impares | la misma op con X en r4–r7 | [exacto] |

r(d) = r(D − 8). Banderas de min/max: N = signo del resultado, Z = operandos iguales.

### 5.3 SFU (unidad B, D ≥ 8, nibble f)

Bits 7–6 = 00, función según bits 5–4 (modo) y Y bit 3:

| modo | Y bit 3 | función |
|---|---|---|
| 00 | 0 | semilla de recíproco: \|Y\| = m·2^e, m ∈ [0.5, 1): r(d) ← signo·2^−e, r(d+1) ← **R[k]** |
| 00 | 1 | separación atanh: x = rY; (x+1)/(2·2^k) ∈ [0.5, 1): r(d) ← (x−1)/(2·2^k), rY ← (x+1)/(2·2^k), r(d+1) ← R[k] de rY → (x−1)/(x+1) = r(d)·r(d+1) tras Newton (logaritmos) |
| 01 | 0 | separación para raíz: \|Y\| = m·4^−k, m ∈ [1, 4): rY ← m, r(d+1) ← **Q[j]**, r7 ← signo·2^−k |
| 01 | 1 | r(d) ← e^⌊Y⌋, rY ← Y − ⌊Y⌋ |
| 10 | 0 | separación log2: Y = m·2^e, m ∈ [0.75, 1.5): rY ← m, r(d) ← e (Y < 0: r(d) ← 9, rY ← Y/512) |
| 10 | 1 | r(d) ← signo(Y) (±1, 0) |
| 11 | 0 | r(d) ← ⌈Y⌉ |
| 11 | 1 | r(d) ← −1 si Y < 0, si no 0 |

**Tablas ROM (bit a bit, `re/dspseed.py`)**:
* **R** (recíproco, 64 entradas): k = 6 bits de la mantisa después del 1 inicial (tramos de 1/128 en [0.5, 1));
  `R[k] = round(4096 / (0.5 + (2k + 1)/256)) / 4096`. Coincide en 64/64. **[exacto]**
* **Q** (1/√, 48 entradas): j = ⌊(m − 1)·16⌋ para m ∈ [1, 4); valor = Q[j]/256 con
  Q = 252 245 238 232 226 221 216 211 207 203 199 195 192 188 185 182 179 177 174 172 169 167 165 163 161 159 157
  155 153 152 150 148 147 145 144 142 141 140 138 137 136 135 134 133 131 130 129 128. **[exacto, tabla literal]**

Bits 7–6 = 01 (sub-op en bits 5–4): `f4Y` r(d) ← r(Y & 7); `f5c` r(d) ← posición de escritura del retardo;
`f5Y` (Y ≠ c) lee 0 en nuestros contextos; `f6Y` (Y = c–f) r(d) ← (r(d) + h(Y−8)) mod 512; `f7Y` = marcadores
(§9). **[medido]**

### 5.4 MAC (dos palabras: w1 entre las ops, w2 = la N en el área de slots) **[exacto en lo medido]**
```
w1 = D op1 X mY Y        w2 = Z op2 C mW W
P1 = Y · rX              P2 = c · Z   (c = rC, o inmediato si op2 >> 1 ≠ 0; Z = ±rZ)
rD ← (op1 & 1 ? −P1 : P1) + (op1 & 2 ? W : 0) + (op1 & 4 ? P2 : 0)        op1 ∈ {0, 1}: no escribe
op2 = 000 y op1 ∈ {6, 7}: P1 = X (o X·Y si Y es inmediato), P2 = Z  (modo suma, sin productos)
D ≥ 8: r(D − 8) ← resultado / 2
```
Una instrucción puede tener dos MAC (clases MM…). Resultado exacto + truncado (§3).

### 5.5 Banderas desde las ops
`X9bY` (transferencia de modo `9b`): grupo X − 8 (X = 9…d → grupos 1…5) ← banderas del resultado de la unidad Y
de la misma instrucción. Por grupo: c0 = ≠ 0, c1 = ≥ 0, c2 = sin desborde (\|x\| < 512), c3 = ≤ 0. Valen para
A, B (min/max: Z = operandos iguales), MAC, SFU y CMP. **Latencia: una instrucción.** Persisten hasta que se
reescriben. **[medido; confirmado por audio en los slaves]**

### 5.6 Registros ocultos: `40TY` en posición de C **[medido]**
T = bits 7–6 → h4, h5, h6, h7; modo = bits 5–4 (0: h ← rY mod 512; 1/2: inmediato de 16/24 bits = valor·16384,
Y = palabras a saltear; 3: lee 0).

---

## 6. Control de flujo

* **Predicación**: palabras C (§4.4) con las banderas de §5.5. Selección de dos ops en una instrucción (`x12`:
  si verdadero saltea la 1, si no la 2).
* **Saltos**: palabra `009Y` en posición de C, desplazamiento con signo de 16 bits = palabra de slot Y >> 1,
  **en palabras desde el final de la instrucción**. Es una unidad: una C la vuelve condicional. Hacia atrás =
  bucles (medido 3/8/16 vueltas exactas; en los slaves, bucle de 4 vueltas por audio).
* **Costo**: salto tomado +1 ciclo; lo salteado no cuesta.
* **Riesgo**: reescribir en vivo un programa con saltos hacia atrás puede dejarlo "roto" a mitad de escritura y
  en un bucle infinito (colgó los slaves). Cargar siempre primero un programa vacío y la cabecera al final.

---

## 7. Memoria

### 7.1 Bancos (prefijo = bits 15–13 de la referencia)

| banco | referencias | contenido | comportamiento medido |
|---|---|---|---|
| 0 | `0nnn` | DT0: RAM de datos, 0x800 palabras | **anillo** (§7.2); 2 lecturas por instrucción |
| 1 | `2nnn` | DT1: memoria compartida por el par de DSP; buzón principal ↔ ranura (`22nn`) | [inferido] compartido; usado como entrada de las ranuras DCO |
| 2 | `4nnn` | DT2: ventana de la línea de retardo | dos páginas que alternan por muestra; DMA de retardo donde el ARM lo conectó (§8.4) |
| 3 | `6nnn` | IOMD: palabras de E/S (§8.3) | no es memoria: entradas y salidas con dueño |
| 4 | `8nnn` | MXM: bus entre DSP, zonas M/O/Z/S de 0x200 palabras | §8.2 |
| 5 | `Annn` | DPC: memoria simple persistente | 1 lectura por instrucción; el ARM la lee y escribe (mejor lugar para resultados de prueba) |
| 6 | `Cnnn` | parámetros que escribe el ARM | sólo lectura para el DSP |
| 7 | `Ennn` | ? (`e013` lo lee el JX como fuente) | [desconocido] |

### 7.2 Anillo del banco 0 **[medido]**
La referencia R direcciona la palabra física (R + ptr) mod 0x800, y ptr avanza 1 por muestra. En la misma muestra
lo guardado en R se relee en R; **en la muestra siguiente aparece en R + 1** (en R + k después de k muestras).
Estado entre muestras: cargar de S + 1, guardar en S. Retardo variable: transferencias indexadas `X28Y`
(bank0[⌊h6·16384⌋ + Y], también relativas al anillo).

### 7.3 Reglas de acceso **[medido]**
* Cargas por instrucción: banco 0 ≤ 2 (con más, sólo vale la última), banco 5 ≤ 1 (con más, ninguna). Los stores
  no cuentan.
* Latencia de carga: una instrucción (la op de la instrucción siguiente todavía ve el valor viejo).
* Una op y una carga al mismo registro en la misma instrucción se **suman**.
* Hasta 6 transferencias por instrucción.

---

## 8. Comunicación

### 8.1 Dentro de un DSP (entre el principal y las ranuras)
Registros compartidos (lo que deja un programa lo ve el siguiente), banco 1 (`22nn`, p. ej. entradas de las
ranuras DCO `2207`, `2209`–`220c`), banco 5 (`a1fe`/`a2fe` en las ranuras de efectos del maestro). **[medido /
inferido]**

### 8.2 Entre DSP del mismo BMC: MXM (banco 4) **[medido en el maestro, DSP3 ↔ DSP4]**
* Son loads y stores comunes a las zonas M (`80nn`/`81nn`), O (`82nn`/`83nn`), Z (`84nn`/`85nn`), S
  (`86nn`/`87nn`); la palabra nn se conserva.
* **Canales punto a punto con cableado fijo**: lo que el DSP3 escribe en Z o S lo lee el DSP4 en O; lo que el DSP4
  escribe en O lo lee el DSP3 en S. Lo que un DSP escribe en M / O va a la memoria de mezcla de su par (ventanas
  ARM MXM2/3 para el DSP3, MXM4/5 para el DSP4).
* **Se congela una vez por muestra** (doble buffer): durante todo su programa el lector ve el mismo valor, sin
  importar cuándo escribió el otro.
* **Latencia exacta: 1 muestra** (medida con el reloj común).
* **No suma**: dos escrituras en la misma muestra → gana la última. **No guarda**: si en una muestra nadie
  escribe, el lector recibe 0.
* Topología de los programas de voz (estático, igual en todos los sintetizadores): DSP0/1 → DSP2/3 → DSP4/5 →
  IOMD; zonas de entrada M ← Z del DSP par del par anterior, O ← Z del DSP impar; S = retorno desde el DSP dos
  posiciones adelante (DSP2 M → DSP0 S, DSP4 M → DSP2 S). **[inferido del corpus; la parte DSP3/DSP4 medida]**

### 8.3 IOMD (banco 3): E/S del BMC **[medido parcialmente]**
* No es memoria compartida: cada palabra es una entrada o una salida, con un DSP dueño configurado por el ARM. Las
  escrituras de un DSP que no es dueño no aparecen; las palabras escritas por su dueño **persisten** y el ARM las
  ve (vista RAW 0x4113E000).
* En los programas de voz de los slaves: entradas `00–06`, `10–17`, `e0–e3`; salidas `20–24`, `60–66`,
  `b0–b7` (hacia el bus de onda que lleva las voces al maestro). En el maestro, el bloque contiene datos vivos
  (por ejemplo `02`/`12` = 4.75, `04`/`14` alternando ±1, `c0`/`c1`, `e0`/`e1`, `f0`/`f1` señales).
* Qué palabra va a qué conexión externa (bus de onda, códec) **[desconocido]**.

### 8.4 Línea de retardo (DMA del banco 2) **[medido en el maestro, DSP4 ranura 0xF7DC]**
Escritura `[4300] ← x`; base con `40TY` + `f6c`; destino de resultados `0b03 4050 <a>` (sin esto el pedido se
ignora); pedido `Xa9Y`; en la misma muestra `[a]`, `[a+1]`, `[a+2]` = dos muestras vecinas + fracción para
interpolar. Unidad de dirección = 16384 muestras. Sólo funciona donde el ARM conectó la ventana. Detalles:
`DSP_ISA_SPEC.md` §9.1.

### 8.5 ARM ↔ DSP
El ARM escribe la memoria de programa (`0x41480000 + n·0x100000` + dirección de ranura, stores de 32 bits),
los parámetros (banco 6) y el banco 5; lee el banco 0 (vistas IEEE/SD31/RAW en `0x41400000 + n·0x100000`), el
banco 5, MXM e IOMD. Registros de control por DSP en `0x41104000 + n·0x1000`. Mapa completo de ventanas:
`DSP_ARM_SIDE.md` §1.

---

## 9. Marcadores `ff70` / `ff71` / `ef75` / `ef74` **[medido]**
`0a02 ff70` = inicio y fin de programa (SYSTEM-1: `ff71` al inicio). `ef75` abre y `ef74` cierra la sección de
cada voz en los programas de Roland (también fusionados como op B dentro de otras instrucciones). **No tienen
efecto** en registros, banderas, memoria ni audio; cuestan 1 ciclo; ningún código ARM los busca. Opcionales.

---

## 10. Modelo de ejecución para un emulador

```
por cada muestra n:
    congelar las entradas MXM (lo escrito en la muestra n−1) e IOMD; avanzar ptr del anillo y la posición del retardo
    para cada DSP, para cada programa (principal, ranuras) en orden:
        pc = inicio
        mientras no se llegue al retorno:
            ins = decodificar(pc) con la plantilla de su clase
            banderas visibles = las escritas hasta dos instrucciones atrás (latencia 1)
            unidades salteadas = C words (§4.4)
            leer todos los operandos (registros viejos; cargas con latencia 1; inmediatos §4.3)
            calcular cada op exacto y truncar a RAW (§3); MAC; SFU con tablas R y Q
            escribir resultados; stores con valores viejos; op + carga al mismo registro se suman
            si hay salto tomado: pc = fin de la instrucción + desplazamiento (+1 ciclo); si no pc = fin
    publicar lo escrito en MXM para la muestra n+1
```
Referencia en Python: `re/dspisa.py` (instrucción: `run`) y `re/dspasm.py` (`simulate`: programa completo con
anillo, banderas y saltos). Validación: `re/dspverify.py` ejecuta una instrucción real en el equipo y la compara
(100/100 instrucciones de Roland al azar coinciden).

**Qué ya es exacto**: decodificación de todas las clases del corpus, A, B, SFU (incluidas las tablas), MAC,
banderas, saltos, anillo, reglas de puertos, MXM.

**Qué falta para ser fiel al motor de Roland**: aritmética en enteros (hoy el simulador usa flotantes con la
regla de truncado; para bit-exactitud total hay que implementarlo sobre mantisas de 29 bits), la línea de retardo
completa (largo, otras ranuras), IOMD/bus de onda entre BMC, orden y planificación de los programas que pone el
ARM, y el lado ARM (cuándo y cómo se escriben parámetros y voces). Para tiempo real hace falta portarlo a C/C++.

---

## 11. Pendientes (sin uso en el código de Roland salvo indicación)

1. Clases de instrucción que no están en el corpus (la numeración de clases no está decodificada).
2. C con campos fuera de rango en clases de varias ops (Roland: < 0.5 % de las instrucciones, casos medidos).
3. `f5Y` con Y ≠ c (`cf54` del JX en los slaves lee 0 en nuestros contextos), h7, `89ba`, `00cY`, modo de
   transferencia `9e` y los "de otra memoria" (`e0`, `e1`, …), nibble e bits 7–6 = 10.
4. Línea de retardo: largo, ventanas en otras ranuras / DSP, cantidad de tomas.
5. IOMD: qué palabra conecta con qué; bus de onda entre BMC; DT1 compartido por el par (sin medir); MXM en los
   slaves (sólo estático) y zonas M/Z del maestro más allá del par DSP3/DSP4.
6. DSPM (sin estudiar).
7. Por qué un programa que se pasa del presupuesto deja inestables a los slaves.
