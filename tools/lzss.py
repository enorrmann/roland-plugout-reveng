#!/usr/bin/env python3
"""
lzss.py -- the LZSS codec used inside PLUG-OUT firmware regions.

It is Haruhiko Okumura's classic LZSS (lzss.c, 1989) with the ring buffer
initialised to 0x00 instead of spaces:

    N = 4096 ring buffer, F = 18 max match, THRESHOLD = 2
    flag byte, LSB first: 1 = literal byte, 0 = reference
    reference = 2 bytes: pos = b0 | (b1 & 0xF0) << 4, len = (b1 & 0x0F) + 3
    the first byte is written at ring position N - F

encode() is a port of Okumura's binary-tree encoder; it reproduces Roland's
compressed streams byte for byte (checked on every region of the three plugins
and of SYSTEM8_UP.BIN).

Pure stdlib.
"""

from __future__ import annotations

N = 4096
F = 18
THRESHOLD = 2
FILL = 0x00
_NIL = N


def decode(src: bytes, size: int | None = None) -> bytes:
    """Decompress `src`; stop after `size` output bytes when given."""
    ring = bytearray([FILL]) * N
    r = N - F
    out = bytearray()
    i, n = 0, len(src)
    limit = size if size is not None else 1 << 62
    while i < n and len(out) < limit:
        flags = src[i]
        i += 1
        for bit in range(8):
            if i >= n or len(out) >= limit:
                break
            if flags >> bit & 1:
                c = src[i]
                i += 1
                out.append(c)
                ring[r] = c
                r = (r + 1) & (N - 1)
            else:
                if i + 1 >= n:
                    i = n
                    break
                b0, b1 = src[i], src[i + 1]
                i += 2
                pos = b0 | (b1 & 0xF0) << 4
                for k in range((b1 & 0x0F) + THRESHOLD + 1):
                    c = ring[(pos + k) & (N - 1)]
                    out.append(c)
                    ring[r] = c
                    r = (r + 1) & (N - 1)
    if size is not None:
        del out[size:]
    return bytes(out)


def encode(src: bytes) -> bytes:
    """Okumura's encoder (InsertNode/DeleteNode binary trees), fill 0x00."""
    text = bytearray([FILL]) * (N + F - 1)
    lson = [_NIL] * (N + 1)
    rson = [_NIL] * (N + 257)
    dad = [_NIL] * (N + 1)
    match = [0, 0]                      # position, length

    def insert(r: int) -> None:
        cmp = 1
        p = N + 1 + text[r]
        rson[r] = lson[r] = _NIL
        match[1] = 0
        while True:
            if cmp >= 0:
                if rson[p] != _NIL:
                    p = rson[p]
                else:
                    rson[p] = r
                    dad[r] = p
                    return
            else:
                if lson[p] != _NIL:
                    p = lson[p]
                else:
                    lson[p] = r
                    dad[r] = p
                    return
            i = 1
            while i < F:
                cmp = text[r + i] - text[p + i]
                if cmp:
                    break
                i += 1
            if i > match[1]:
                match[0], match[1] = p, i
                if i >= F:
                    break
        # replace node p by r
        dad[r], lson[r], rson[r] = dad[p], lson[p], rson[p]
        dad[lson[p]] = r
        dad[rson[p]] = r
        if rson[dad[p]] == p:
            rson[dad[p]] = r
        else:
            lson[dad[p]] = r
        dad[p] = _NIL

    def delete(p: int) -> None:
        if dad[p] == _NIL:
            return
        if rson[p] == _NIL:
            q = lson[p]
        elif lson[p] == _NIL:
            q = rson[p]
        else:
            q = lson[p]
            if rson[q] != _NIL:
                while rson[q] != _NIL:
                    q = rson[q]
                rson[dad[q]] = lson[q]
                dad[lson[q]] = dad[q]
                lson[q] = lson[p]
                dad[lson[p]] = q
            rson[q] = rson[p]
            dad[rson[p]] = q
        dad[q] = dad[p]
        if rson[dad[p]] == p:
            rson[dad[p]] = q
        else:
            lson[dad[p]] = q
        dad[p] = _NIL

    out = bytearray()
    code = bytearray([0])
    mask = 1
    s, r, pos = 0, N - F, 0
    length = 0
    while length < F and pos < len(src):
        text[r + length] = src[pos]
        length += 1
        pos += 1
    if length == 0:
        return b""
    for i in range(1, F + 1):
        insert(r - i)
    insert(r)
    while True:
        if match[1] > length:
            match[1] = length
        if match[1] <= THRESHOLD:
            match[1] = 1
            code[0] |= mask
            code.append(text[r])
        else:
            code.append(match[0] & 0xFF)
            code.append(((match[0] >> 4) & 0xF0) | (match[1] - (THRESHOLD + 1)))
        mask = (mask << 1) & 0xFF
        if mask == 0:
            out += code
            code = bytearray([0])
            mask = 1
        last, i = match[1], 0
        while i < last and pos < len(src):
            c = src[pos]
            pos += 1
            delete(s)
            text[s] = c
            if s < F - 1:
                text[s + N] = c
            s = (s + 1) & (N - 1)
            r = (r + 1) & (N - 1)
            insert(r)
            i += 1
        while i < last:
            i += 1
            delete(s)
            s = (s + 1) & (N - 1)
            r = (r + 1) & (N - 1)
            length -= 1
            if length:
                insert(r)
        if length <= 0:
            break
    if len(code) > 1:
        out += code
    return bytes(out)
