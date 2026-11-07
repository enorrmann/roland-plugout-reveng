# Volume VII: Clean-Room Archive & Evidence Logs

# Chapter 2: Functional, Data & Performance Requirements Specification

> **Document ID**: TRM-BMC-VOL7-002  
> **Classification**: Normative Specification Standards  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 2.1 Architectural Requirements (REQ-ARCH)

- **`REQ-ARCH-01` [Multi-ASIC Heterogeneous Topology]**:
  The system shall support distributed heterogeneous processing across clustered Roland BMC ASICs. The Master ASIC shall govern UI, USB, and high-level synthesis modeling; Slave ASICs shall execute low-level DSP voice microcode and voice parameter translation.
- **`REQ-ARCH-02` [Synchronous 96 kHz Audio Frame Synchronization]**:
  The audio synthesis pipeline shall execute synchronously locked to a sample rate of $F_s = 96.000\text{ kHz}$ ($T_s = 10.4166\ \mu\text{s}$). Every DSP core shall restart execution at address `0x0100` at each 96 kHz frame pulse.
- **`REQ-ARCH-03` [Deterministic Single-Cycle DSP Execution]**:
  The DSP core pipeline shall execute every instruction word in exactly one instruction cycle, regardless of parallel unit combinations, except taken branches which consume two cycles.
- **`REQ-ARCH-04` [Load-to-Use Latency Constraint]**:
  The processor pipeline shall exhibit a 1-cycle load-to-use delay. A register targeted by a memory load transfer shall become valid only on the second instruction following the load.

---

## 2.2 Data Representation & Binary Formats (REQ-DATA)

- **`REQ-DATA-01` [Roland 32-bit RAW Floating-Point Format]**:
  Internal arithmetic shall represent numerical audio variables as 32-bit sign-magnitude floating-point values governed by the formula:
  $$v = (-1)^S \cdot \frac{M}{2^{32 - 4s}}, \quad S \in \{0, 1\}, \; s \in \{0, 1, 2, 3\}, \; M \in [0, 2^{29}-1]$$
- **`REQ-DATA-02` [Dynamic Range Modulo 512 Wrapping]**:
  Arithmetic operations generating magnitudes $|v| \ge 512.0$ shall wrap modulo 512.0 while preserving algebraic sign $S$ and asserting condition flag `V`.
- **`REQ-DATA-03` [Container Chained XOR Encryption]**:
  Container files in `Code*.Dat` archives shall be decrypted via a chained 32-bit exclusive-OR algorithm initialized with seed constant `0xBACADABE`:
  $$\text{plain}[0] = \text{cipher}[0] \oplus \text{0xBACADABE}, \quad \text{plain}[i] = \text{cipher}[i] \oplus \text{cipher}[i-1]$$
- **`REQ-DATA-04` [7-in-8 USB MIDI SysEx Packet Encoding]**:
  Firmware data payloads shall expand 7 raw bytes (56 bits) into 8 MIDI-safe 7-bit transmission bytes ($0 \le \text{byte} \le 127$). Each 512-byte raw block shall expand into 586 transmission bytes.
- **`REQ-DATA-05` [LZSS Archive Compression]**:
  System firmware update partitions (`SYSTEM8_UP.BIN`, `SYSTEM1_UPD.BIN`) shall decode using LZSS sliding-window decompression with a 4,096-byte circular window, 18-byte maximum match length, and 8-token flag bytes.

---

## 2.3 Functional Interface Requirements (REQ-FUNC)

- **`REQ-FUNC-01` [Two-Region Virtual Machine Separation]**:
  Synthesizer software shall partition into Region 0 (Master ARM Model, parameter registry, note allocator) and Region 1 (Slave ARM Engine, DSP microcode loader, voice callbacks).
- **`REQ-FUNC-02` [Parameter Group 2 Note Dispatch]**:
  Note events shall be dispatched from Master to Slaves via Parameter Group 2 records:
  - System-8: Pitch `0x429 + v`, Velocity `0x43A + v` ($v \in [0, 7]$).
  - System-1: Pitch `0x110 + v`, Velocity `0x120 + v` ($v \in [0, 7]$).
- **`REQ-FUNC-03` [Fixed Vector Syscall Table Execution]**:
  Engine microcode drivers shall invoke platform hardware services strictly through the fixed jump vector table (`0x0113FC00` on SYSTEM-8, `0x0100FC00` on SYSTEM-1).
- **`REQ-FUNC-04` [Bank 0 Circular Ring Pointer Progression]**:
  Bank 0 memory pointers shall advance by exactly 1 word per sample frame modulo 2048. Data written to reference $R$ in sample $n$ shall be read at $R+1$ in sample $n+1$.
- **`REQ-FUNC-05` [Wave Bus Output Summing]**:
  Polyphonic voice mixes shall reach physical audio DAC outputs only by streaming across IOMD registers `0x6020`/`0x6021` (SYSTEM-8) or `0x6006`/`0x6007` (SYSTEM-1).

---

## 2.4 Performance & Timing Deadlines (REQ-PERF)

- **`REQ-PERF-01` [SYSTEM-8 Frame Execution Envelope]**:
  The total instruction count for any SYSTEM-8 DSP core within a single sample frame shall not exceed **945 instruction cycles**.
- **`REQ-PERF-02` [SYSTEM-1 Frame Execution Envelope]**:
  The total instruction count for any SYSTEM-1 DSP core within a single sample frame shall not exceed **703 instruction cycles** (DSP A) or **708 instruction cycles** (DSP B).
- **`REQ-PERF-03` [SYSTEM-1 Main Program Storage Boundary]**:
  The Main Program binary on SYSTEM-1 shall not exceed **6,144 bytes (`0x18FF`)** in program SRAM.
- **`REQ-PERF-04` [Cyclic Handler Deadline]**:
  The real-time 1.00 ms cyclic ARM handler shall complete all execution within $50\ \mu\text{s}$ per tick to prevent task deadline jitter.
