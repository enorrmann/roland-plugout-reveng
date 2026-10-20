"""Program layout rules shared by the disassembler and the assembler.

A class of isa.json fixes a layout (spec §12). Its *prefix* (see prefix_length)
is what the units fill; the rest is the slot array, whose length an instruction
chooses. An instruction can be longer (more immediate words) or shorter than the
layout's letters. Both tools must agree on how the letters map to body words, or
a round trip breaks at the first unusual instruction.
"""


def prefix_length(layout):
    """how many body words a class's units fill before its slot array (spec 4.2-4.3).

    The units occupy the positions up to and including the last T (or, for a
    class with no transfer, up to its first N, I or R). Everything after that
    is the slot array, whatever letter the layout gives it: a C or an R there
    is a slot word, not a unit."""
    last_t = layout.rfind("T")
    if last_t >= 0:
        return last_t + 1
    for i, ch in enumerate(layout):
        if ch in "NIR":
            return i
    return len(layout)


def letters_for(table, cls, body_len):
    """one letter per body word of `body_len` words, or None if the class cannot
    hold that many. Positions past the class layout are immediate words."""
    c = table.classes.get(cls)
    if c is None:
        return None
    if body_len < prefix_length(c.layout):
        return None
    letters = list(c.layout[:body_len])
    return letters + ["I"] * (body_len - len(letters))


def slot_count(letters, prefix):
    """number of slot words in the body (§4.3)"""
    return len(letters) - prefix


def n_words(letters, prefix):
    """the N words are the first M words of the slot array (§4.3)"""
    return min(letters[prefix:].count("N"), slot_count(letters, prefix))


def fits_exact(table, letters, body, prefix):
    """do these body words decode as the units their letters name?

    Used by the assembler to find the class of a `raw` line. A word of a unit
    position must decode as that unit; a slot-array word can be anything."""
    import codec
    for ch, w in zip(letters[:prefix], body):
        if ch in "RI":
            continue
        if ch in "AB":
            if not any(codec.decode(table.forms_by_name[n], [w])[0]
                       for n in table.units.get(ch) or []):
                return False
        elif ch == "T":
            if codec.decode(table.forms_by_name[table.units["T"][0]], [w])[0] is None:
                return False
        elif ch == "C":
            if not any(codec.decode(table.forms_by_name[n], [w])[0]
                       for n in table.units.get("C", [])):
                return False
    return True


def array_positions(letters, prefix):
    """body indices of the slot array, in slot order (spec 4.3): every word
    after the units. Both tools use this, so they read the same words."""
    return list(range(prefix, len(letters)))


def stream_bits(n, cursor, refs=(), at_bit=None):
    """bit positions (in the slot array, MSB first) of an `n`-bit read.

    `at_bit` given: an explicit read at that bit (a byte Y of the array is
    8 * Y), taken literally, references included. `at_bit` None: a sequential
    read from `cursor`, which steps over every reference word that starts on a
    word boundary (spec 4.4: "reading past the first word skips the words that
    a transfer uses as a reference").

    Returns (positions, next_cursor). Keep next_cursor for the next sequential
    read of the same instruction.
    """
    if at_bit is not None:
        pos = list(range(at_bit, at_bit + n))
        return pos, at_bit + n
    refs = set(refs)
    out, pos = [], cursor
    while len(out) < n:
        if pos // 16 in refs and pos % 16 == 0:
            pos += 16
            continue
        out.append(pos)
        pos += 1
    return out, pos
