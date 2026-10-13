# p1y01_m0c0a_up (App1_Main, the OS) - NOT DECODED

Encrypted payload; only the clear header and trailer can be read. No classes, vtables, presets or labels can be extracted until the payload is decrypted (BASE_FIRMWARE.md section 2).

## Clear header (0x60 bytes)

| offset | value | note |
|---|---|---|
| 0x00 | `App1_Main` | image name |
| 0x10 | `2022/04/28 09:41` | build date |
| 0x20 | `0.010001` | tool/version tag |
| 0x28 | `0x60000000` | load address |
| 0x30 | `0x00060040` | then 3 x `FFFFFFFF` |
| 0x40 | `0x00060060` | |
| 0x44 | `0x2cda70` | payload size |
| 0x48 | `0x60000000` | |
| 0x50 | `0x81d0ee9e` | probable plaintext check value |
| 0x54 | `0xb6020001` | |

Raw header:

```
41707031 5f4d6169 6e000000 00000000 32303232 2f30342f 32382030 393a3431 302e3031 30303031 00000060 00000000 40000600 ffffffff ffffffff ffffffff 60000600 70da2c00 00000060 00000000 9eeed081 010002b6 00000000 ffffffff
```

## Clear trailer (0x40 bytes)

`000000000000000000000000f6969295526f6c616e642050315930315f4330415645522e312e3332424c442e3133364532322030343238203039203431203534`

Text: `����Roland P1Y01_C0AVER.1.32BLD.136E22 0428 09 41 54`

## Payload statistics

* file 2939664 bytes = 0x60 + 0x2cda70 payload + 0x40 trailer
* payload SHA-256 `70997ef99d63cb91923884b5362d0ae248eafb5e0730b6937d790b3d5ded23e7`
* byte entropy 7.697 bits/byte; 8-byte blocks: 367438 total, 165942 distinct
* most repeated 8-byte ciphertext blocks (identical plaintext under ECB):

| block (hex) | count | first offset | last offset |
|---|---|---|---|
| `6b4c9a852c732831` | 60889 | `0x95728` | `0x2cdac8` |
| `93b181af5fd8eec9` | 4666 | `0x107510` | `0x270da8` |
| `445d8d01c8185564` | 3350 | `0x97da8` | `0x2b9108` |
| `e39f2867b6c68b52` | 3088 | `0x967b8` | `0x2cd780` |
| `465e927150f5218d` | 2053 | `0xd3090` | `0x26ebc8` |

(offsets are file offsets)

## Cipher conclusions

* 64-bit block cipher in ECB mode (equal plaintext blocks give equal ciphertext; 8-byte halves repeat independently of their 16-byte neighbours).
* Full diffusion; no XOR/rotate/linear structure; not the plug-out chained XOR.
* Keys derived from header, trailer and product strings failed for Blowfish, CAST5, DES/3DES, IDEA, TEA and XTEA. The key is in the device bootloader.
