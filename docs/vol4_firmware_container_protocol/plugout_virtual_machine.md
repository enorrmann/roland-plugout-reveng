# Volume IV: Firmware Containers, OS & SysEx Protocol

# Chapter 2: Plug-Out Architecture, Region 0/1 Model & Lifecycle

> **Document ID**: TRM-BMC-VOL4-002  
> **Classification**: Normative Software Architecture Reference  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 2.1 The Two-Tier Plug-Out Virtual Machine

The Roland Plug-Out architecture divides a software synthesizer into two physically and logically decoupled binary regions executed on distinct processor nodes:

1. **Region 0: Master Control Model ("The Model")**:
   - Executes on the Master BMC ARM Core 0 within the μT-Kernel OS address space.
   - Hosts the parameter schema (`SYNINFO`), MIDI Continuous Controller (CC) mappings, menu descriptors, patch bank structures, and the polyphonic note assignment state machine (`CAssignPlugout`).
2. **Region 1: Slave DSP Engine Driver ("The Engine")**:
   - Executes on the Slave BMC ARM core(s).
   - Manages DSP program loading, voice microcode execution gating, and real-time parameter conversion from semitones/linear controls into Roland 32-bit RAW floating-point registers.

```
+---------------------------------------------------------------------------------------------------+
|                                   PLUG-OUT TWO-REGION ARCHITECTURE                                |
|                                                                                                   |
|  [ MASTER BMC: ARM Core 0 ]                                                                       |
|   Physical Address: 0x61600000 + (slot * 0x200000) (S8) / 0x60800000 (S1)                         |
|   ┌────────────────────────────────────────────────────────────────────────────────────────────┐  |
|   │ REGION 0: Parameter Model & Dispatcher                                                     │  |
|   │  - SYNINFO Descriptor Block (+0x000..+0x200)                                               │  |
|   │  - Descriptor Factory Entry (+0x24): create(), destroy(), select()                         │  |
|   │  - Parameter Schema Walker (+0x28): Linked list of Group & Control Type Records            │  |
|   │  - CAssignPlugout Allocator: Handles noteOn, noteOff, CC, PitchBend, Modulation           │  |
|   └─────────────────────────────────────────────┬──────────────────────────────────────────────┘  |
|                                                 │ Parameter Updates via USB / Mailbox             |
|                                                 ▼                                                 |
|  [ SLAVE BMC: ARM Core ]                                                                          |
|   Physical Address: 0x60800000 + (slot * 0x200000) (S8) / 0x60200000 (S1)                         |
|   ┌────────────────────────────────────────────────────────────────────────────────────────────┐  |
|   │ REGION 1: Engine Driver & Microcode Engine                                                 │  |
|   │  - Engine Driver Object (+0x30): load(), refresh(), param(id, value)                       │  |
|   │  - Microcode Image Tables: MAIN0..MAIN5 Programs, DCO1/DCO2 Oscillator Slots               │  |
|   │  - Hardware Memory Dispatch: Writes Bank 5 & Bank 6 via OS Jump Table 0x0113FC00           │  |
|   └────────────────────────────────────────────────────────────────────────────────────────────┘  |
+---------------------------------------------------------------------------------------------------+
```

---

## 2.2 Region 0: Model Architecture & Parameter Schema

### 2.2.1 The `SYNINFO` Descriptor Block (`+0x00`–`+0x40`) `[VERIFIED]`

The entry point of Region 0 begins with a standardized synchronization block:
- **Offset `+0x00`**: Magic header check word.
- **Offset `+0x24`**: Pointer to the `init()` factory function.
  The Master OS invokes `init()` multiple times during system startup (`LoadPlugout 0x60051D32`). This function **must be idempotent** and return a valid pointer to the Plug-Out Descriptor Array.
- **Offset `+0x28`**: Pointer to the Parameter Registration Type List.

### 2.2.2 Plug-Out Descriptor Structure `[VERIFIED]`

The descriptor array returned by `init()` contains 7 pointer words:

```c
typedef struct {
    uint32_t slot_id;                   /* [0] Slot Index (Assigned by OS: 1, 2, or 3) */
    void *(*create)(void);              /* [1] Constructor Callback */
    void  (*destroy)(void *obj);        /* [2] Destructor Callback */
    void *part_a_obj;                   /* [3] Primary Part Object Pointer */
    void *part_b_obj;                   /* [4] Secondary / Upper Part Object Pointer */
    uint32_t reserved;                  /* [5] Reserved (Must be 0) */
    void  (*select)(void *obj, int mode); /* [6] Slot Select / Mode Change Notification */
} plugout_desc_t;
```

### 2.2.3 Parameter Record Walker (`+0x28`) `[VERIFIED]`

The parameter registration table at `+0x28` defines panel controls for the UI and patch manager. It consists of serialized type records terminated by `-1`:

| Record Type | Payload Size | Fields & Structural Content |
|---|---|---|
| **Type 0** | Variable | Group Descriptor: `{ int32_t type, uint32_t size, char group_name[16] }`. |
| **Type 1** | `0x4C` Bytes | Parameter Descriptor: `{ id, default_val, min, max, step, curve, disp_mode, name[16] }`. |
| **Type 2** | 8 Bytes | List Header Descriptor: `{ id, item_count }`. |
| **Type 3** | 20 Bytes | Text Label Descriptor: `{ char label[20] }`. |
| **`-1`** | 4 Bytes | Sentinel Terminator: Signals end of registration table. |

> [!WARNING]
> If the table at `+0x28` begins with zero (`size = 0`), the OS parameter walker (`0x600525B4`) enters an infinite loop, causing the host to freeze during the "Receiving PLG" handshake.

---

## 2.3 The Note Allocator Interface (`CAssignPlugout`) `[VERIFIED]`

Polyphonic voice allocation is governed by the `CAssignPlugout` C++ object located in Region 0. The Master OS dispatches musical events across its 17-method virtual function table (`vtable` at `0x619197C8` in JX-3P):

```c
struct CAssignPlugout_VTable {
    void (*reset)(void *this);                          /* Slot 0: Reset voice states */
    void (*refresh)(void *this, int mode);              /* Slot 1: Mode change refresh */
    void (*note_off)(void *this, int note);             /* Slot 2: MIDI Note Off */
    void (*note_on)(void *this, int note, int vel);     /* Slot 3: MIDI Note On */
    void (*control_change)(void *this, int cc, int val);/* Slot 4: MIDI CC Handler */
    void (*param_update)(void *this, int id, float val);/* Slot 5: Parameter Injection */
    void (*pitch_bend)(void *this, int bend);           /* Slot 6: Pitch Wheel (-8192..8191) */
    void (*aftertouch)(void *this, int pressure);       /* Slot 7: Channel Pressure */
    /* Slots 8 to 16: Portamento, Legato, Hold-1, and Arpeggiator Clocks */
};
```

When `note_on` is invoked, the allocator selects an idle voice according to priority rules (Same note retrigger > Oldest released voice > Oldest voice with hold > Oldest sounding voice) and updates parameter IDs `0x429 + v` (Note) and `0x43A + v` (Velocity).

---

## 2.4 Region 1: Engine Driver Interface

Region 1 executes on the Slave ARM cores. Its primary interface is the **Engine Object VTable** (22 functional entry points):

| VTable Offset | Method Name | Parameters | Functional Description |
|---|---|---|---|
| `vt + 0x00` | `reset` | `(engine_t *e)` | Clears internal voice filter states and modulation accumulators. |
| `vt + 0x04` | `deinit` | `(engine_t *e)` | Releases hardware hooks, restores OS cyclic handler pointers. |
| `vt + 0x08` | `load` | `(engine_t *e)` | Dispatches microcode programs to DSP program memory (`FC30`/`FC34`). |
| `vt + 0x0C` | `start` | `(engine_t *e)` | Enables DSP slot task bits and audio execution pipelines. |
| `vt + 0x10` | `stop` | `(engine_t *e)` | Mutes output stages and clears active voice gates. |
| `vt + 0x30` | `param` | `(engine_t *e, int id, float val)` | Real-time parameter callback. Injects data into Bank 5/6. |
| `vt + 0x70` | `swap_slot` | `(engine_t *e, int slot, void *code)` | Hot-swaps microcode for active oscillator or FX slots. |
