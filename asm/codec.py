"""Generic codec: the only place that knows how a table entry maps to words.

Nothing here knows a mnemonic. A unit is decoded by testing each variant's
`when` conditions; a word is only decoded when the chosen variant explains
*every* field of the layout, otherwise the caller keeps it verbatim.

    codec.decode(form, words)                -> (Variant, fields) | None
    codec.encode(form, variant, args)        -> words

Operands travel as `Arg` (parsed source). An immediate operand carries its own
width (`bits`) and byte (`at_byte`); an instruction may have several (a MAC
reads up to three, spec 4.4). Placing the bits in the slot array is the
assembler's job (asm.py), not this module's.
"""

import isa

WORD = 0xFFFF


class CodecError(ValueError):
    """an operand or a word the table cannot express"""


# ---------------------------------------------------------------------------
# operands
# ---------------------------------------------------------------------------

class Arg:
    """one parsed operand.

    kind    'reg' | 'imm' | 'ref' | 'cond' | 'units' | 'word' | 'num' | 'label'
    value   register number (negative = -rN), integer, or LabelRef
    text    source text, used when the value cannot be written back
    """

    __slots__ = ("kind", "value", "text", "at_byte", "width", "bits")

    def __init__(self, kind, value, text=None, at_byte=None):
        self.kind, self.value, self.text = kind, value, text if text is not None else str(value)
        # spec 4.4: the byte of the slot array an immediate is read at. None
        # means "in order" for a sequential read, or the operand's own Y field
        # for an ALU op (where the field is the byte).
        self.at_byte = at_byte
        # the immediate width the source states (`#v@b:32`), if any
        self.width = None
        # the width the assembler settled on (16, 24 or 32); set before encode
        self.bits = None

    def __repr__(self):
        return "Arg(%s, %r)" % (self.kind, self.value)

    @property
    def is_immediate(self):
        return self.kind == "imm"

    def number(self):
        if isinstance(self.value, int):
            return self.value
        raise CodecError("%r is not a number" % self.text)


# ---------------------------------------------------------------------------
# selectors
# ---------------------------------------------------------------------------

def match(when, form, words):
    """do the `when` conditions hold? `words` is one word or a MAC pair"""
    if isinstance(words, int):
        words = [words]
    for name, want in when.items():
        have = _bits_of(form, words, name)
        if isinstance(want, int):
            if have != want:
                return False
        elif isinstance(want, dict):
            if "in" in want and have not in want["in"]:
                return False
            if "lt" in want and have >= want["lt"]:
                return False
            if "ge" in want and have < want["ge"]:
                return False
            if "eq" in want and have != want["eq"]:
                return False
        else:
            raise isa.TableError("%s: cannot test %r against %r" % (form.name, name, want))
    return True


# ---------------------------------------------------------------------------
# decoding
# ---------------------------------------------------------------------------

def decode(form, words, slot_ok=True):
    """-> (Variant, fields) or (None, reason)

    `fields` maps each layout field to its value in the word. A variant is
    only accepted when it explains every field (fixed by the variant, taken
    from an operand, or zero); otherwise the word holds information the
    variant does not name and must stay verbatim.
    """
    reasons = set()
    for v in form.variants:
        if not v.decodable:
            continue
        if not match(v.when, form, words):
            continue
        ok, fields = _explained(form, v, words, slot_ok)
        if ok:
            return v, fields
        reasons.add(_why(v, form, words))
    return None, "; ".join(sorted(reasons)) if reasons else "no variant matches"


def _explained(form, v, words, slot_ok):
    fields = {}
    for name, spec in form.fields.items():
        if name.startswith("$"):
            continue
        value = _bits_of(form, words, name)
        src = v.fields.get(name)
        if src is None and name == "mode" and "mode" in form.fields and _writes_mode(form, v):
            # the immediate mode is written by whichever operand takes the
            # instruction's immediate (§4.4)
            fields[name] = value
            continue
        if src is None:
            if _covered(form, name, v):
                fields[name] = value      # bits another field already names
                continue
            if value != 0:
                return False, None
            fields[name] = 0
            continue
        if isinstance(src, int):
            if value != src:
                return False, None
            fields[name] = value
            continue
        if isinstance(src, dict):
            if src.get("operand_mode"):
                # the mode nibble of a register-or-immediate operand (spec 5.2):
                # it belongs to that operand, so any value is consistent
                fields[name] = value
                continue
            if src.get("free"):
                fields[name] = value      # any value: the variant does not name it
                continue
            if src.get("slot") or src.get("label_slot"):
                if not slot_ok:
                    return False, None
                fields[name] = value
                continue
            if src.get("one_of") and value not in src["one_of"]:
                return False, None
            fields[name] = value
            continue
        fields[name] = value
    return True, fields


def _bits_of(form, words, name):
    """a field's value; a MAC field says which of its two words holds it"""
    spec = form.field_spec(name)
    return isa.get_bits(words[spec.get("_word", 0)], spec)


# A field is an alias of another when it covers exactly the same bits and one of
# the two is the layout's own name for it. The aliases the table uses are
# declared here explicitly — no overlap is inferred.
def _writes_mode(form, v):
    """does this variant carry the immediate on one of its operands?"""
    return any(form.kinds.get(src) == "y_operand" for src in v.fields.values()
               if isinstance(src, str))


def _covered(form, name, variant=None):
    """True when `name` does not need a value of its own in this variant.

    The relationships are declared in the table (`word.field_relations`); the
    code only applies them:

      same_bits_as   two names for the same bits — naming either covers both
      selector_only  a selector: only the variants that test it own it, for the
                     others those bits belong to the field(s) listed in `of`
    """
    relations = form.table.word.get("field_relations", {})
    rel = relations.get(name)
    if not rel or name.startswith("$"):
        return False
    same = rel.get("same_bits_as")
    if same and same in form.fields:
        return variant is None or same in variant.fields
    if rel.get("selector_only"):
        if variant is not None and name in variant.fields:
            return False                       # this variant does own it
        return any(f in form.fields and (variant is None or f in variant.fields)
                   for f in rel.get("of", []))
    return False


def _why(v, form, words):
    free = [n for n in form.fields if not n.startswith("$")
            and n not in v.fields and _bits_of(form, words, n) and not _covered(form, n, v)]
    if free:
        return "%s leaves %s free" % (v.mnemonic, ", ".join(free))
    return "%s does not cover the fixed bits" % v.mnemonic


# ---------------------------------------------------------------------------
# encoding
# ---------------------------------------------------------------------------

def is_imm(arg):
    return arg is not None and arg.is_immediate


def encode(form, variant, args):
    """-> the words of one unit (two for a MAC)

    `args` are the form's positional operands (Arg objects). An immediate
    operand must have `bits` set (its width) and may have `at_byte` (the byte it
    is read at; None = read in order). This pass writes the mode nibbles and
    the byte fields only; the bits themselves are placed by the layout pass.
    Fields whose source is a slot reference or a jump target are left for it.
    """
    named = dict(zip(form.operand_names, args))
    if form.form == "raw":
        return [a.number() & WORD for a in args]

    words = [0] * (2 if form.form == "mac" else 1)
    for fname, src in variant.fields.items():
        if isinstance(src, int):
            words = _write(form, words, fname, src)
        elif isinstance(src, dict):
            if src.get("slot") or src.get("label_slot") or src.get("offset_slot"):
                continue                  # written by the layout pass
            if src.get("operand_mode"):
                # the mode nibble of a register-or-immediate operand (§5.2):
                # 0 for a register, else the immediate width's mode
                arg = named.get(src["operand_mode"])
                value = _mode_of(form, arg.bits) if is_imm(arg) else 0
                words = _write(form, words, fname, value)
                continue
            if src.get("free"):
                # a bit the encoding leaves unused: nothing is written, so it
                # stays zero (the assembler restores the original value from a
                # `.free` directive when re-assembling a disassembled line)
                continue
            if "from" in src:
                arg = named.get(src["from"])
                if arg is None:
                    raise CodecError("%s: field %r needs operand %r"
                                     % (variant.mnemonic, fname, src["from"]))
                spec = form.field_spec(fname)
                if spec.get("borrowed_by_nibble") is not None and src.get("bits") == [1, 0]:
                    # unit B: X[2] is written to the nibble (spec 5.1), so this
                    # field is only X[1:0]; bit 8 belongs to the nibble
                    idx = spec.get("_word", 0)
                    value = _extract(arg, src) & 3
                    words[idx] = (words[idx] & ~(3 << spec["lsb"])) | (value << spec["lsb"])
                    continue
                words = _write(form, words, fname, _extract(arg, src))
            elif "one_of" in src:
                val = named.get(src.get("from", "mode"))
                words = _write(form, words, fname,
                               (val.number() if isinstance(val, Arg) else 0) & src.get("mask", WORD))
            elif "from_mode_of" in src:
                arg = named[src["from_mode_of"]]
                if is_imm(arg):
                    if "mode" in form.fields:
                        words = _write(form, words, "mode", _mode_of(form, arg.bits))
                elif "mode" in form.fields:
                    words = _write(form, words, "mode", 0)
            else:
                raise CodecError("%s: no rule for field %r" % (variant.mnemonic, fname))
        else:
            # a condition operand carries both the number and the negation
            cond_arg = named.get("cc")
            if (src in ("cc", "neg") and cond_arg is not None
                    and cond_arg.kind == "cond"):
                n, neg = cond_arg.value
                words = _write(form, words, src, n if src == "cc" else (1 if neg else 0))
                continue
            arg = named.get(src)
            if arg is None:
                raise CodecError("%s: field %r needs operand %r"
                                 % (variant.mnemonic, fname, src))
            if is_imm(arg):
                # the operand sits in this field: the field is its byte
                if "mode" in form.fields:
                    words = _write(form, words, "mode", _mode_of(form, arg.bits))
                if arg.at_byte is not None:
                    words = _write(form, words, fname, arg.at_byte)
                continue
            words = _write(form, words, fname, _value(form, fname, arg))

    # a MAC coefficient read at a byte keeps that byte in its C field (spec 4.4);
    # read in order (after Y's immediate) the field stays free
    coef = named.get("C")
    if variant.coef_bits and is_imm(coef) and coef.at_byte is not None:
        words = _write(form, words, "C", coef.at_byte)
    return words


def _write(form, words, fname, value):
    spec = form.field_spec(fname)
    idx = spec.get("_word", 0)
    try:
        words[idx] = isa.put_bits(words[idx], spec, value)
    except ValueError as e:
        raise CodecError("%s.%s: %s" % (form.name, fname, e))
    return words


def _value(form, fname, arg):
    if arg.kind == "reg":
        val = arg.number()
        spec = form.field_spec(fname)
        # a field that encodes 8 + d (spec 6.3/6.4) shifts the register number
        val = val + spec.get("add", 0)
        if val < 0:
            # bit 3 (the top bit of a 4-bit operand field) negates the register
            # (spec 5.1 for Y, spec 5.2 for Z and W)
            if spec["msb"] - spec["lsb"] + 1 < 4:
                raise CodecError("%s: field %r has no room for a negated register"
                                 % (form.name, fname))
            return (1 << 3) | (-val)
        return val
    if arg.kind in ("num", "word", "cond"):
        return arg.number() if arg.kind != "cond" else arg.value
    raise CodecError("%s: a %r operand cannot go into field %r"
                     % (form.name, arg.kind, fname))


def _extract(arg, spec):
    """the bits an operand contributes to a field.

    `bits` lists which bits of the operand go into the field; `add` is added to
    the value first (a store writes r(R-8), so the field holds R+8), `or` sets
    fixed bits afterwards.
    """
    val = arg.number() + spec.get("add", 0)
    if "range" not in spec and "bits" not in spec:
        return val | spec.get("or", 0)      # the whole value goes in the field
    if "range" in spec:
        msb, lsb = spec["range"]
        out = (val >> lsb) & ((1 << (msb - lsb + 1)) - 1)
    else:
        out = 0
        for bit in spec.get("bits", []):
            out = (out << 1) | ((val >> bit) & 1)
    return out | spec.get("or", 0)


def _mode_of(form, bits):
    if bits is None:
        raise CodecError("%s: immediate has no width" % form.name)
    for mode, width in form.table.immediate_rules["modes"].items():
        if width == bits:
            return int(mode)
    raise CodecError("the table has no immediate mode of %d bits" % bits)
