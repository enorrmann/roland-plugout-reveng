# Volume IV: Firmware Containers, OS & SysEx Protocol

# Chapter 1: Binary Container Formats, Firmware Update Images & Compression

> **Document ID**: TRM-BMC-VOL4-001  
> **Classification**: Normative Specification  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 1.1 Firmware Distribution Packages

Roland distributes synthesizer engines, system firmware, and Plug-Out models across two primary binary distribution containers:
1. **Instrument Firmware Update Archives**: `SYSTEM8_UP.BIN` (Roland SYSTEM-8, typical size ~30–45 MB) and `SYSTEM1_UPD.BIN` (Roland SYSTEM-1, typical size ~10–15 MB).
2. **Plug-Out Model Containers**: `Code*.Dat` archives embedded inside Roland Cloud VST3/AU instrument packages (`Script/Code8_1.Dat`, `Script/Code8_2.Dat`, `Script/Code8_3.Dat` for SYSTEM-8; `Script/Code1.Dat` for SYSTEM-1).

```
+---------------------------------------------------------------------------------------------------+
|                                 FIRMWARE DISTRIBUTION CONTAINERS                                  |
|                                                                                                   |
|  [ SYSTEM8_UP.BIN / SYSTEM1_UPD.BIN ]                   [ Code*.Dat (Plug-Out Images) ]           |
|  - Encrypted Outer Header                               - Chained 32-bit XOR Cipher Block         |
|  - Multi-Segment Archive:                               - 128-byte Header: "PLUGOUT_SYS_INF"      |
|    * p1y01_p0m.bin (Master μT-Kernel OS Image)          - START Record (Slot Selection, Auth)     |
|    * p1y01_p0s.bin (Slave s_boot & s_data)              - Data Records: 2048 to 4096 Packets      |
|    * p1y01_m0c0a_up.bin (STM32 Panel SubCPU)              (512-byte payload packed 7-in-8)        |
|  - Compression: LZSS Sliding Window Engine              - END Record (Cryptographic Hash/Trailer) |
+---------------------------------------------------------------------------------------------------+
```

---

## 1.2 The `Code*.Dat` Plug-Out Container Format

Plug-Out archives located in `Script/Code*.Dat` contain the complete execution images (Region 0 Master Model, Region 1 Slave Engine, and Voice DSP Microcode) formatted for direct serial transfer over USB-MIDI.

### 1.2.1 Chained XOR Decryption Algorithm `[VERIFIED]`

The binary image is encrypted using a rolling, chained 32-bit exclusive-OR (XOR) stream cipher initialized with a hardwired 32-bit seed (`0xBACADABE`):

$$\text{plain}[0] = \text{cipher}[0] \oplus \text{0xBACADABE}$$

$$\text{plain}[i] = \text{cipher}[i] \oplus \text{cipher}[i-1], \quad \forall i \ge 1$$

```c
/* Chained 32-bit XOR Container Decryption (C Reference Implementation) */
void decrypt_plugout_container(uint32_t *buffer, size_t word_count)
{
    if (word_count == 0) return;
    uint32_t prev = 0xBACADABEu;
    for (size_t i = 0; i < word_count; i++) {
        uint32_t curr = buffer[i];
        buffer[i] = curr ^ prev;
        prev = curr;
    }
}
```

### 1.2.2 Decrypted Container Header Layout (`0x00`–`0x7F`) `[VERIFIED]`

Once decrypted, the 128-byte header defines image metadata, versioning, and record offsets:

| Offset | Data Type | Field Name | Functional Definition |
|---|---|---|---|
| `0x00` | `uint32_t` | `header_size` | Length of container header; invariant constant `0x00000080` (128 bytes). |
| `0x04` | `char[16]` | `magic_signature`| ASCII Identifier: `"PLUGOUT_SYS_INF"` (null-terminated). |
| `0x14` | `char[16]` | `model_name` | ASCII Synthesizer Model String (e.g. `"JX-3P           "`). |
| `0x24` | `uint16_t` | `model_id` | Roland Model Identifier (e.g. `0x0008` = JX-3P, `0x000A` = JUNO-60). |
| `0x26` | `uint16_t` | `version` | BCD Encoded Version Number (`0x0120` = v1.20). |
| `0x30` | `uint32_t` | `start_offset` | Byte offset to the complete SysEx START message. |
| `0x34` | `uint32_t` | `start_size` | Length of SysEx START message (typically 14 bytes). |
| `0x40` | `uint32_t` | `end_offset` | Byte offset to the complete SysEx END message. |
| `0x44` | `uint32_t` | `end_size` | Length of SysEx END message (typically 18 bytes). |
| `0x50` | `uint32_t` | `data_offset` | Byte offset to first data record. |
| `0x54` | `uint32_t` | `data_stride` | Fixed record stride; invariant constant `0x00000260` (608 bytes). |
| `0x58` | `uint32_t` | `record_count` | Total data records: **4,096** (`Code8_*.Dat`) or **2,048** (`Code1.Dat`). |

---

## 1.3 7-in-8 Data Encoding & Packet Structuring

Because standard MIDI transmission is constrained to 7-bit data bytes ($0 \le \text{byte} \le 127$), binary firmware blocks must be packed using Roland's 7-in-8 encoding scheme:

### 1.3.1 Encoding Mechanics
- Every group of **7 source bytes** (56 bits) is expanded into **8 MIDI data bytes** (7 bits each).
- Each data block encapsulates exactly **512 raw binary payload bytes**, which expand into **586 encoded MIDI transmission bytes**.
- Combined with Roland address bytes (`70 30 00 09`), model header, and checksum, each full SysEx data packet totals **599 bytes**.

```
Source Data (7 Bytes / 56 Bits):
[ b0 ][ b1 ][ b2 ][ b3 ][ b4 ][ b5 ][ b6 ]

Encoded MIDI Stream (8 Bytes / 7 Bits each):
[ m0 ][ m1 ][ m2 ][ m3 ][ m4 ][ m5 ][ m6 ][ m7 ]
where MSBs of b0..b6 are gathered into header bits of m0..m7.
```

### 1.3.2 Record Types & Region Partitioning
Data records are partitioned into two equal regions padded with null records:
- **`70 30 00 09`** (599 bytes): Active binary firmware block.
- **`70 30 01 09`** (14 bytes): Filler padding record (Payload byte `0x01`).

---

## 1.4 System Update Images (`SYSTEM8_UP.BIN` / `SYSTEM1_UPD.BIN`)

The full instrument update package contains compressed system binaries unpacked by the bootloader during flash updates:

```
+---------------------------------------------------------------------------------------------------+
|                                  SYSTEM UPDATE ARCHIVE PARTITION                                  |
|                                                                                                   |
|  Offset Range           Compressed Size      Expanded Size        Identity / Destination          |
|  0x000000 - 0x02498E    149,902 Bytes        1,166,152 Bytes      p1y01_p0m.bin (Master OS Core 0)|
|  0x024990 - 0x071720    314,736 Bytes        536,308 Bytes        p1y01_p0s.bin (Slave Engine)    |
|  0x071730 - 0x0825B0    69,248 Bytes         128,000 Bytes        p1y01_m0c0a_up.bin (STM32 Panel)|
+---------------------------------------------------------------------------------------------------+
```

### 1.4.1 Official SYSTEM-8 Firmware Revision Matrix `[STATIC_ANALYSIS]`

Analysis of official updaters across releases v1.30, v1.31, and v1.32 demonstrates internal engine growth and OS payload characteristics:

| Archive Member | Partition Contents | v1.30 Decompressed | v1.31 Decompressed | v1.32 Decompressed | Architectural Evolution |
|---|---|---|---|---|---|
| `p0m` | Slot 0 Model (SYSTEM-8 v1.20) | 1,166,152 B | 1,166,152 B | 1,166,152 B | Unchanged across revisions |
| `p0s` | Slot 0 Engine | 536,220 B | 536,308 B | 536,308 B | Expanded +88 B in v1.31 (voice handling fixes) |
| `pl1m` | JUPITER-8 Model v1.06 / Engine | 1,160,460 / 342,404 B | 1,160,460 / 342,424 B | 1,160,460 / 342,424 B | Engine expanded +20 B in v1.31 (arpeggio fix) |
| `pl1s` | JUNO-106 Model / Engine | v1.05: 1,157,772 / 262,160 B | v1.06: 1,157,780 / 262,192 B | 1,157,780 / 262,192 B | Model v1.06; engine +32 B (mono/unison note-off) |
| `pl2m` | JX-3P Model v1.00 / Engine | 1,159,484 / 275,492 B | 1,159,484 / 275,508 B | 1,159,484 / 275,508 B | Engine expanded +16 B in v1.31 |
| `App1_Main` | Encrypted Master OS Kernel | 2,938,080 B (cipher) | 2,938,640 B (cipher) | 2,939,664 B (cipher) | OS expanded across all versions; v1.32 adds JUPITER-4 support |

### 1.4.1a `App1_Main` Payload Cipher: 3-Key 3DES `[VERIFIED]`

Only the **payload** is encrypted. File bytes `[0x60 : 0x60 + size@0x44)`; the `0x60`-byte
clear header and the 0x40-byte clear trailer are always plaintext, which is what makes an
encrypt/decrypt round trip byte-exact.

Container layout:

| Offset | Contents |
|---|---|
| `0x00` | `"App1_Main"` + NUL |
| `0x10` | build date, ASCII (`2022/04/28 09:41`) |
| `0x20` | tool/version tag (`0.010001`) |
| `0x28` | load address (`0x60000000`, master BMC SDRAM) |
| `0x30` | `0x00060040`; `0x34`–`0x40` are `FFFFFFFF` |
| `0x40` | `0x00060060` |
| `0x44` | encrypted payload size (v1.32: `0x2CDA70`) |
| `0x48` | load address again |
| `0x50` | payload check value (not CRC-32/sum32/Adler/SHA of the ciphertext) |
| `0x54` | crypto control: bytes `01 00 MM KK` = enabled / mode / boot ROM key slot |
| `0x60` | **encrypted payload begins** |

Cipher:

- **3-key 3DES in ECB over 64-bit blocks**, operated through the hardware AXI FIFO word
  order: each 64-bit block is two 32-bit words, big-endian packed and word-swapped
  ($[W_1{:}W_0]$) on both ingress and egress.
- **Key source**: the silicon Boot ROM master key table at `0x012023B0`, entry
  `0x012023B0 + key_idx * 24`. In decrypt mode the hardware key registers load the six
  32-bit words in inverted stage order: `Key = pack(">IIIIII", w1, w0, w3, w2, w5, w4)`.

Known key slots (addresses are public; the 24-byte key material is **not**, and is
deliberately not reproduced in this documentation):

| Slot | Address | Protects |
|---|---|---|
| `0xB6` | `0x012034C0` | MC-101 `App1_Main`; SYSTEM-8 `App1_Main` |
| `0xB5` | `0x012034A8` | MC-101 `App0_UPD` recovery updater |

The SYSTEM-8 `App1_Main` container names slot **`0xB6`** and mode `0x02` (3-key 3DES) in
all of v1.30, v1.31 and v1.32. The mechanism and the key table are shared with the
Roland ESC2 firmware family (MC-101).

> **Key handling.** The key material is private. It is never stored in this repository,
> in `tools/`, in the test suite, or in these documents. Supply it at the point of use:
> `--key <48 hex digits>`, `--key-file <path|->`, or `--bootrom <silicon dump>` (which
> reads the table entry directly). `tools/app1_crypto.py` prints only a SHA-256
> fingerprint of the key, never the key itself.

Verification `[VERIFIED]`:

- The ported cipher reproduces the MC-101 `App1_Main` reference plaintext exactly.
- Decrypting the v1.32 payload yields a valid Cortex-M image (`0x680148B0`, `ldr r0,[pc,#…]`)
  whose **code segment matches `dumps/os_full.bin` byte for byte** up to the first
  live-RAM variable; the remaining 2.63 % are runtime-mutated `.data` values.
- Re-encrypting the decrypted payload reproduces Roland's shipped `SYSTEM8_UP.BIN`
  byte for byte (`cmp` clean), so a patched OS can be repackaged and installed.

Tool: [`tools/app1_crypto.py`](../tools/app1_crypto.py) (`info`, `decrypt`, `encrypt`,
`selftest`), or `tools/sys8fw.py pack --os-plain <decrypted image> --os-key <hex>` to
rebuild the whole updater with a modified OS. `info` needs no key; the other commands
require one.

```bash
# needs no key
python3 tools/app1_crypto.py info firmware/sys8_sys_v132/untared/p1y01_m0c0a_up.bin

# decrypt / re-encrypt (key supplied per invocation, or via $APP1_KEY / a bootrom dump)
python3 tools/app1_crypto.py decrypt <container> -o os.bin --key-file ~/.roland-key
python3 tools/app1_crypto.py encrypt os.bin --header-from <container> \
        -o out.bin --key-file ~/.roland-key
```

### 1.4.2 LZSS Compression Algorithm Specification `[STATIC_ANALYSIS]`

System images are compressed using an optimized **LZSS (Lempel-Ziv-Storer-Szymanski)** sliding-window algorithm:
- **Sliding Window Size**: 4,096 bytes (12-bit distance offset $d \in [0, 4095]$).
- **Match Length**: 18 bytes (4-bit length field $l \in [3, 18]$, bias = 3).
- **Control Flags**: 8-bit flag byte preceding every 8 tokens:
  - Bit = 1: **Literal Byte** (Transmitted raw).
  - Bit = 0: **Window Match Reference** (2 bytes: 12 bits distance offset + 4 bits length).

```c
/* Standard LZSS Decompression Implementation */
void lzss_decompress(const uint8_t *src, uint8_t *dst, size_t dst_len)
{
    uint8_t window[4096] = {0};
    size_t win_pos = 4078;
    size_t out_pos = 0;
    size_t in_pos = 0;
    uint16_t flags = 0;

    while (out_pos < dst_len) {
        if (((flags >>= 1) & 0x100) == 0) {
            flags = src[in_pos++] | 0xFF00;
        }
        if (flags & 1) {
            uint8_t c = src[in_pos++];
            dst[out_pos++] = c;
            window[win_pos] = c;
            win_pos = (win_pos + 1) & 0x0FFF;
        } else {
            uint8_t b1 = src[in_pos++];
            uint8_t b2 = src[in_pos++];
            size_t match_offset = b1 | ((b2 & 0xF0) << 4);
            size_t match_len    = (b2 & 0x0F) + 3;
            for (size_t k = 0; k < match_len && out_pos < dst_len; k++) {
                uint8_t c = window[(match_offset + k) & 0x0FFF];
                dst[out_pos++] = c;
                window[win_pos] = c;
                win_pos = (win_pos + 1) & 0x0FFF;
            }
        }
    }
}
```
