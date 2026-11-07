# Volume IV: Firmware Containers, OS & SysEx Protocol

# Chapter 3: Fixed Vector OS Service Tables (Syscalls & APIs)

> **Document ID**: TRM-BMC-VOL4-003  
> **Classification**: Normative Software Architecture Reference  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 3.1 Overview of Operating System Service Tables

Communication between user synthesizer engines (Plug-Out Region 1) and the underlying hardware platform is governed by fixed, memory-mapped jump tables populated by the operating system during early boot:

- **Roland SYSTEM-8 Platform**: Fixed jump table located at physical address **`0x0113FC00`** in Slave TCM.
- **Roland SYSTEM-1 Platform**: Fixed jump table located at physical address **`0x0100FC00`** in Master and Slave TCM.

Calling services across these vector tables ensures complete binary compatibility across instrument firmware releases without requiring static symbol relinking.

```
+---------------------------------------------------------------------------------------------------+
|                               FIXED VECTOR SYSCALL DISPATCH ARCHITECTURE                          |
|                                                                                                   |
|  [ User Microcode / Engine Driver ]                                                               |
|        │                                                                                          |
|        │ Resolves function pointer at Base + Offset:                                              |
|        │   #define OS(off) (*(void *volatile *)(0x0113FC00u + (off)))                             |
|        ▼                                                                                          |
|  +---------------------------------------------------------------------------------------------+  |
|  | Fixed Jump Table Vector: 0x0113FC00 (S8) / 0x0100FC00 (S1)                                   |  |
|  |  [ +0x00 ]: Parameter Query Service (os_get / FC00)                                         |  |
|  |  [ +0x28 ]: Microcode Descriptor Registration (reg_fn)                                      |  |
|  |  [ +0x30 ]: Microcode Queue Dispatcher (FC30)                                               |  |
|  |  [ +0x44 ]: Hardware Slot / Section Gating (FC44)                                           |  |
|  |  [ +0x70 ]: Dynamic Microcode Slot Hot-Swap (swap_fn)                                       |  |
|  |  [ +0xA0 ]: Hardware Linear Parameter Ramp (ramp_fn / FCE4)                                 |  |
|  |  [ +0xA4 ]: Immediate Hardware Parameter Injection (set_fn / FCEC)                          |  |
|  |  [ +0xB0 ]: Bank 5 Floating-Point Word Write (bank_fn / FD6C)                               |  |
|  |  [ +0xC0 ]: 96 kHz Audio Frame Synchronizer Barrier (sync_fn)                               |  |
|  |  [ +0xF0 ]: Slave ASIC Node ID Identification (slave_fn)                                    |  |
|  |  [ +0x23C]: Parameter Hardware Ramp Completion Interrupt (FE3C)                             |  |
|  +---------------------------------------------------------------------------------------------+  |
|        │                                                                                          |
|        ▼                                                                                          |
|  [ Underlying RTOS / Hardware MMIO Registers / DSP Subsystems ]                                   |
+---------------------------------------------------------------------------------------------------+
```

---

## 3.2 Roland SYSTEM-8 Service Table (`0x0113FC00`) Specification `[VERIFIED]`

On the SYSTEM-8 Slave BMC cores, services are accessed via byte offsets relative to `0x0113FC00`:

| Table Offset | Mnemonic | C Method Signature | Description & Operational Semantics |
|---|---|---|---|
| `+0x00` | `os_get` | `int (*)(int part, int grp, int id)` | Queries active parameter value from the OS parameter kernel. |
| `+0x28` .. `+0x6C` | `reg_fn` | `void (*)(const void *desc)` | Registers program and data descriptors (`dspdesc_t`) with OS loader. |
| `+0x70` | `swap_fn`| `void (*)(int dsp, const void *prog, const void *data, void *ctx, void *ctx2, uint32_t slot, uint32_t task, uint32_t x0, uint32_t x1)` | Dynamically swaps microcode for a running slot without halting the audio frame loop. |
| `+0xA0` | `ramp_fn`| `void (*)(int dsp, int id, int ramp, int flag, int sp, float v)` | Writes parameter into Bank 6 (`C0id`) with hardware linear ramp interpolation (4 ms to 97 ms). |
| `+0xA4` | `set_fn` | `void (*)(int dsp, int id, float v)` | Immediately writes a 32-bit float value to Bank 6 (`C0id`). |
| `+0xB0` | `bank_fn`| `void (*)(int dsp, const uint32_t *p_id, float v)` | Converts float `v` to 32-bit RAW and writes to Bank 5 word `A000 + *p_id`. |
| `+0xC0` | `sync_fn`| `void (*)(int a, int b, int c, int d)` | Stalls execution until the next 96 kHz DSP audio frame strobe completes. |
| `+0xF0` | `slave_fn`| `int (*)(void)` | Hardware node identification: Returns `0` on Slave BMC 0, `1` on Slave BMC 1. |

### 3.2.1 C Header Declarations (SYSTEM-8 Reference)

```c
#define OS_TABLE_BASE 0x0113FC00u
#define OS_SVC(off) (*(void *volatile *)(OS_TABLE_BASE + (off)))

typedef void (*s8_bank_write_fn)(int dsp, const uint32_t *p_id, float val);
typedef void (*s8_param_set_fn)(int dsp, int id, float val);
typedef void (*s8_param_ramp_fn)(int dsp, int id, int ramp, int flag, int sp, float val);
typedef int  (*s8_slave_id_fn)(void);

static inline void dsp_write_bank5(int dsp, uint32_t word_offset, float value) {
    ((s8_bank_write_fn)OS_SVC(0xB0))(dsp, &word_offset, value);
}

static inline void dsp_write_param_immediate(int dsp, int param_id, float value) {
    ((s8_param_set_fn)OS_SVC(0xA4))(dsp, param_id, value);
}

static inline int dsp_get_slave_index(void) {
    return ((s8_slave_id_fn)OS_SVC(0xF0))() ? 1 : 0;
}
```

---

## 3.3 Roland SYSTEM-1 Service Table (`0x0100FC00`) Specification `[VERIFIED]`

On the SYSTEM-1 architecture, vector services are located at base `0x0100FC00` and invoked via standard C function pointers:

| Syscall ID | Offset | Mnemonic | C Method Signature | Description & Operational Semantics |
|---|---|---|---|---|
| `SVC 0x00` | `+0x00` | `FC00` | `uint32_t (*)(int grp, int id)` | Retrieves parameter state. |
| `SVC 0x04` | `+0x04` | `FC04` | `void (*)(int grp, int id, uint32_t val)` | Modifies parameter value in Group `grp`. |
| `SVC 0x08` | `+0x08` | `FC08` | `void (*)(int grp)` | Flushes parameter changes to DSP buffers. |
| `SVC 0x0C` | `+0x0C` | `FC0C` | `void (*)(int grp, int id)` | Marks parameter as dirty/touched. |
| `SVC 0x30` | `+0x30` | `FC30` | `void (*)(const struct s1_prog *set)` | Queues microcode descriptor set for **DSP A**. |
| `SVC 0x34` | `+0x34` | `FC34` | `void (*)(const struct s1_prog *set)` | Queues microcode descriptor set for **DSP B**. |
| `SVC 0x40` | `+0x40` | `FC40` | `void (*)(int index, const void *code)` | Places voice microcode into section slot $0..7$. |
| `SVC 0x44` | `+0x44` | `FC44` | `void (*)(int index, int state)` | Enables (`state=1`) or disables (`state=0`) section $0..7$. |
| `SVC 0xE4` | `+0xE4` | `FCE4` | `void (*)(int sel, int id, int target, int y, int z)` | Triggers Bank 6 linear hardware ramp (4 ms to 97 ms). |
| `SVC 0xEC` | `+0xEC` | `FCEC` | `void (*)(int sel, int id, uint32_t ieee_bits)` | Immediately writes 32-bit value to Bank 6 (`C0id`). |
| `SVC 0x16C`| `+0x16C`| `FD6C` | `void (*)(int sel, const uint32_t *obj, uint32_t bits)`| Writes IEEE float bits into Bank 5 (`A000` / `A100`). |
| `SVC 0x23C`| `+0x23C`| `FE3C` | `void (*)(int sel, int id, void (*fn)(void))` | Registers interrupt callback executed on ramp completion. |
| `SVC 0x240`| `+0x240`| `FE40` | `void (*)(int sel, int id)` | Unregisters ramp completion interrupt callback. |

### 3.3.1 C Header Declarations (SYSTEM-1 Reference)

```c
#define S1_OS_BASE 0x0100FC00u
#define S1_SVC(off) (*(void *volatile *)(S1_OS_BASE + (off)))

typedef void (*s1_b5_fn)(int sel, const uint32_t *obj, uint32_t bits);
typedef void (*s1_p_imm_fn)(int sel, int id, uint32_t bits);
typedef void (*s1_p_ramp_fn)(int sel, int id, int target, int y, int z);
typedef void (*s1_slot_fn)(int idx, int state);

static inline void s1_write_bank5_word(int sel, int word_idx, uint32_t ieee_bits) {
    uint32_t obj[3] = { 0, 0, (uint32_t)word_idx };
    ((s1_b5_fn)S1_SVC(0x16C))(sel, obj, ieee_bits);
}

static inline void s1_write_param_immediate(int sel, int id, uint32_t ieee_bits) {
    ((s1_p_imm_fn)S1_SVC(0xEC))(sel, id, ieee_bits);
}

static inline void s1_gate_section(int section_index, int enable) {
    ((s1_slot_fn)S1_SVC(0x44))(section_index, enable);
}
```
