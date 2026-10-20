#!/usr/bin/env python3
"""decode.py -- rebuild the bytes that sd_txt.s sends as CC witnesses.

    python3 decode.py CAPTURE.txt [--out FILE.bin]

CAPTURE.txt is the output of `amidi -p hw:3,0,0 -d` during the install
(one "BF cc vv" line per CC). Prints the read count (CC 0x67), the attempts
(CC 0x69) and the bytes: each byte is CC 0x68 (low nibble) then CC 0x6A (high
nibble). --out writes the reconstructed bytes.
"""
import argparse
import sys


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("capture")
    ap.add_argument("--out")
    args = ap.parse_args()

    cc = []
    with open(args.capture) as f:
        for line in f:
            p = line.split()
            if len(p) == 3 and p[0] == "BF":
                cc.append((int(p[1], 16), int(p[2], 16)))

    count = [v for c, v in cc if c == 0x67]
    attempts = [v for c, v in cc if c == 0x69]
    nib = [(c, v) for c, v in cc if c in (0x68, 0x6A)]
    print("read count (0x67):", count, " 0x7F = fopen failed")
    print("attempts   (0x69):", attempts)

    data = bytearray()
    for i in range(0, len(nib) - 1, 2):
        (c0, lo), (c1, hi) = nib[i], nib[i + 1]
        if (c0, c1) != (0x68, 0x6A):
            print(f"out of order at message {i}: {c0:#x} {c1:#x}", file=sys.stderr)
            return 1
        data.append(lo | (hi << 4))

    print("bytes:", len(data))
    print("ascii:", "".join(chr(b) if 32 <= b < 127 else "." for b in data))
    print("hex:  ", " ".join(f"{b:02x}" for b in data))
    if args.out:
        open(args.out, "wb").write(data)
        print("wrote", args.out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
