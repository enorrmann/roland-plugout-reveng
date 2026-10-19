"""Program layout rules shared by the disassembler and the assembler.

A class of isa.json fixes a *core* layout (spec §12); an actual instruction
can be longer (more immediate words) or shorter (a transfer that needs no
reference). Both tools must agree on how the letters map to body words, or a
round trip breaks at the first unusual instruction.
"""


def letters_for(table, cls, body_len):
    """one letter per body word, or None if the table cannot place them"""
    c = table.classes.get(cls)
    if c is None:
        return None
    layout = c.layout
    nr = layout.count("R")
    core = layout[:len(layout) - nr] if nr else layout
    if body_len < len(core):
        return None
    keep = min(nr, body_len - len(core))
    return list(core) + ["R"] * keep + ["I"] * (body_len - len(core) - keep)


def slot_start(letters):
    """index of the first slot word: the body word after the last T (§4.3).

    "after the last T" means the last *used* transfer position. A class may have
    more T positions than the instruction uses (0x398 = A T T T R R): the words
    from the first unused T onward are the slot array, and an empty T position
    followed by an R is simply an R.
    """
    last = -1
    for i, ch in enumerate(letters):
        if ch == "T":
            last = i
    if last < 0:
        return None
    # an R right after the last T belongs to the array, not to the transfer area
    return last + 1


def slot_count(letters):
    """number of slot words in the body (§4.3)"""
    start = slot_start(letters)
    return 0 if start is None else len(letters) - start


def n_words(letters):
    """the N words are the first M words of the slot array (§4.3)"""
    return min(letters.count("N"), slot_count(letters))


def classes_for(table, letters):
    """every class whose layout can hold this exact sequence of positions"""
    out = []
    for cls, c in sorted(table.classes.items()):
        want = letters_for(table, cls, len(letters))
        if want == list(letters):
            out.append(cls)
    return out


def fits_exact(table, letters, body):
    """do these body words match the letters exactly?

    Used by the assembler to find the class of a `raw` line: every position
    must be explainable — a T word by the transfer table, a C word by one of
    the C forms, an A/B/M word by its ALU/MAC form, an R/I word by anything.
    """
    import codec
    for ch, w in zip(letters, body):
        if ch in "RI":
            continue
        if ch in "AB":
            form_names = table.units.get(ch) or []
            if not any(codec.decode(table.forms_by_name[n], [w])[0]
                       for n in form_names):
                return False
        elif ch == "T":
            if codec.decode(table.forms_by_name[table.units["T"][0]], [w])[0] is None:
                return False
        elif ch == "C":
            if not any(codec.decode(table.forms_by_name[n], [w])[0]
                       for n in table.units.get("C", [])):
                return False
        elif ch == "M":
            continue          # the w2 that completes it is the next word
        elif ch == "N":
            continue
    return True
