"""Generic codec: the only place that knows how a table entry maps to words.

Nothing here knows a mnemonic. A unit is decoded by testing each variant's
`when` conditions; a word is only decoded when the chosen variant explains
*every* field of the layout, otherwise the caller keeps it verbatim.

    codec.decode(table, form, word, slot=...)  -> (Variant, fields) | None
    codec.encode(table, form, variant, args, imm_bits) -> (words, ImmRead)

Operands travel as `Arg` (parsed source) or plain ints; immediate operands
carry the instruction's single immediate value (§4.4).
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

    __slots__ = ("kind", "value", "text", "at_byte", "width")

    def __init__(self, kind, value, text=None, at_byte=None):
        self.kind, self.value, self.text = kind, value, text if text is not None else str(value)
        # spec 4.4: the byte of the slot array an immediate is read at. None
        # means "wherever the operand's own Y field points".
        self.at_byte = at_byte
        # the immediate width the source states (`#v@b:32`), if any
        self.width = None

    def __repr__(self):
        return "Arg(%s, %r)" % (self.kind, self.value)

    @property
    def is_immediate(self):
        return self.kind == "imm"

    def number(self):
        if isinstance(self.value, int):
            return self.value
        raise CodecError("%r is not a number" % self.text)


class ImmRead:
    """how an instruction reads its immediate (§4.4)

    slot  True  read at byte `at_byte` of the slot array
          False read in order (hidden-register loads)
    """
    __slots__ = ("slot", "at_byte", "bits")

    def __init__(self, slot, at_byte=0, bits=16):
        self.slot, self.at_byte, self.bits = slot, at_byte, bits

    def __repr__(self):
        return "ImmRead(slot=%s, byte=%d, %d bits)" % (self.slot, self.at_byte, self.bits)


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

def encode(form, variant, args, imm_bits=None):
    """-> (words, ImmRead or None)

    `args` are the form's positional operands (Arg objects). Fields whose
    source is a slot reference or a jump target are left for the layout pass.
    """
    named = dict(zip(form.operand_names, args))
    if form.form == "raw":
        return [a.number() & WORD for a in args], None

    words = [0] * (2 if form.form == "mac" else 1)
    read = None
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
                value = 0
                if arg is not None and arg.is_immediate:
                    value = _mode_of(form, imm_bits or 16)
                    read = ImmRead(slot=form.form == "alu", bits=imm_bits or 16)
                words = _write(form, words, fname, value)
                continue
            if src.get("free"):
                # a bit the encoding leaves unused: write what the caller gave,
                # which is nothing, so it stays zero (the assembler restores the
                # original value when re-assembling a disassembled line)
                continue
            if "from" in src:
                arg = named.get(src["from"])
                if arg is None:
                    raise CodecError("%s: field %r needs operand %r"
                                     % (variant.mnemonic, fname, src["from"]))
                words = _write(form, words, fname, _extract(arg, src))
            elif "one_of" in src:
                val = named.get(src.get("from", "mode"))
                words = _write(form, words, fname,
                               (val.number() if isinstance(val, Arg) else 0) & src.get("mask", WORD))
            elif "from_mode_of" in src:
                arg = named[src["from_mode_of"]]
                bits = imm_bits or 16
                if arg.is_immediate:
                    if "mode" in form.fields:
                        words = _write(form, words, "mode", _mode_of(form, bits))
                    read = ImmRead(slot=form.form == "alu", bits=bits)
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
            if arg.is_immediate:
                bits = imm_bits or 16
                if "mode" in form.fields:
                    words = _write(form, words, "mode", _mode_of(form, bits))
                if arg.at_byte is not None and "Y" in form.fields:
                    words = _write(form, words, "Y", arg.at_byte)
                read = ImmRead(slot=form.form == "alu", bits=bits,
                               at_byte=arg.at_byte if arg.at_byte is not None else 0)
                continue
            words = _write(form, words, fname, _value(form, fname, arg))
    if read is not None and form.form == "alu":
        read.at_byte = form.get(words[0], "Y")
    return words, read


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
    for mode, width in form.table.immediate_rules["modes"].items():
        if width == bits:
            return int(mode)
    raise CodecError("the table has no immediate mode of %d bits" % bits)
