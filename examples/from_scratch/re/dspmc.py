"""DSP microcode helpers: blob bytes -> 16-bit words in execution order.

The DSP executes each 32-bit little-endian word upper half first. A blob of
4n+2 bytes is followed in the image by the halfword 0x0001, which is its real
last instruction; the blob's own last halfword (0x0000) is never written to
program memory (verified on the master DSP4, docs/CUSTOM_PLUGOUTS.md).
"""
import struct

def words(blob, tail=b'\x01\x00'):
    """Execution-order words. A 4n+2 blob gets `tail` (the halfword that follows
    it in the image) and loses its own last halfword, which is never loaded;
    pass tail=None to keep the blob as it is."""
    padded = len(blob) % 4 and tail is not None
    if padded:
        blob = blob + tail
    w = list(struct.unpack('<%dH' % (len(blob) // 2), blob[:len(blob) // 2 * 2]))
    for i in range(0, len(w) - 1, 2):
        w[i], w[i + 1] = w[i + 1], w[i]
    if padded:
        w.pop()
    return w

def blob_words(path):
    return words(open(path, 'rb').read())
