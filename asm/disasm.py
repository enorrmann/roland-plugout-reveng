"""Disassembler: program words -> dspasm source (spec §3, §4, §12).

    text, report = disasm.program(words, table)

Honesty rules:

* A word the table does not explain becomes `raw` with its words verbatim —
  never an invented mnemonic (spec §14).
* `raw` also carries the class when the class would not follow from the
  units, so re-assembling always reproduces the original header.
* Each line ends with a status tag (`exact`, `measured`, `inferred`,
  `fitted`, `raw`) so confirmed and inferred decodings stay distinguishable.
* `report` counts what happened and lists every instruction that stayed
  verbatim, with the reason.

Layout (§4.3): the slot array is every body word after the last transfer.
Its first M words are the N words of the MACs; the words a load/store points
at are references; the rest is the immediate bit stream (§4.4).
"""

import codec
import layout as layout_rules
import slots as slots_mod

WORD = 0xFFFF


# ---------------------------------------------------------------------------
# immediates (§4.4)
# ---------------------------------------------------------------------------

class ImmStream:
    """the instruction's immediate words as one MSB-first bit string;
    positions past the end read zeros, like the hardware

    The words are the whole slot array (spec 4.4), so a byte offset indexes the
    array directly. `refs` names the words a transfer owns: sequential reading
    (a MAC's C right after Y's immediate) skips them, an explicit byte offset
    does not -- that is `re/dspisa.py`'s rule, derived on the `540c` example.
    """

    def __init__(self, words, refs=()):
        self.words = list(words)
        self.bits = "".join(format(w, "016b") for w in self.words) + "0" * 64
        self.refs = set(refs)
        self.pos = 0               # where the last take() stopped (sequential reads)

    def take(self, start, n):
        """`n` bits starting at bit `start`, or on from the last read if None

        Only a *sequential* read steps over reference words (spec 4.4: "C right
        after Y's immediate" of a MAC, measured on `540c …`). An explicit offset
        is the address the program wrote, so it reads whatever is there.
        """
        if not n:
            return 0
        if start is None:                       # sequential: step over references
            pos = self.pos
            out = ""
            while len(out) < n:
                if (pos // 16) in self.refs and pos % 16 == 0:
                    pos = (pos // 16 + 1) * 16
                    continue
                k = min(n - len(out), 16 - pos % 16)
                out += self.bits[pos:pos + k]
                pos += k
            self.pos = pos
        else:                                   # explicit offset: read it literally
            out = self.bits[start:start + n]
            if len(out) < n:
                out += "0" * (n - len(out))
            self.pos = start + n
        return (int(out, 2) << (32 - n)) & 0xFFFFFFFF


def fmt_number(v):
    if v == 0:
        return "0"
    if v == int(v) and abs(v) < 1e9:
        return "%d" % int(v)
    return "%.7g" % v


def canonical(table, w):
    """is `w` what the hardware writes for its own value (§2)?"""
    try:
        return table.raw_encode(table.raw_decode(w)) == w
    except ValueError:
        return False


def imm_text(table, bits, w):
    """source text of an immediate value; non-canonical bits are kept as raw"""
    if canonical(table, w):
        v = table.raw_decode(w)
        if bits == 32 or v * 2 ** (32 - bits) == int(v * 2 ** (32 - bits)):
            return fmt_number(v)
    if bits == 16:
        return "raw.%04x" % (w >> 16)
    if bits == 24:
        return "raw.%06x" % (w >> 8)
    return "raw.%08x" % (w & 0xFFFFFFFF)


# ---------------------------------------------------------------------------
# layout (§4.3, §12)
# ---------------------------------------------------------------------------

# §5.3: fe, ff, 3e, 0e, 10 and 12 are all the same plain access, and that is
# the only kind that addresses a reference word (§4.3).
SLOT_MODES = frozenset((0xFE, 0xFF, 0x3E, 0x0E, 0x10, 0x12))


def uses_slot(table, mode):
    """§4.3: a plain access addresses a reference word; the other mode bytes
    (28, a9, 9b, da..df, d0..d7) do not"""
    return mode in SLOT_MODES


# ---------------------------------------------------------------------------
# one instruction
# ---------------------------------------------------------------------------

class Decoded:
    def __init__(self, text, status, words, reason=None, need_class=False, free_bits=0):
        self.text, self.status, self.words, self.reason = text, status, words, reason
        # True when the units alone would fit more than one class, so the
        # listing must say which one this instruction uses
        self.need_class = need_class
        # bits the encoding leaves free that the text cannot express
        self.free_bits = free_bits


ORDER = ["exact", "measured", "inferred", "fitted", "raw"]


def worst(statuses):
    if not statuses:
        return "exact"          # class 0x000: the one-word no-op
    return max(statuses, key=ORDER.index)


def disassemble_insn(table, ins):
    cls, body = ins[0] >> 5, ins[1:]
    letters = layout_rules.letters_for(table, cls, len(body))
    if letters is None:
        return raw(ins, "class %#05x cannot hold %d body words" % (cls, len(body)))

    # ---- the slot array (§4.3): discovery first, then reading -------------
    start = layout_rules.slot_start(letters)
    if start is None:
        # no T: the array is the N and I positions in order. A class with no
        # slot position at all (CCC) keeps the offset words after its units.
        n_words = [w for w, ch in zip(body, letters) if ch == "N"]
        array = [w for w, ch in zip(body, letters) if ch in ("N", "I")]
        if not array:
            array = list(body[1:])
        # where each array word lives in the body, so a reference can be read
        # by its real index (§4.3: slot k is the k-th word of the array)
        owner = [i for i, ch in enumerate(letters) if ch in ("N", "I")]
        if not owner:
            owner = list(range(1, 1 + len(array)))
    else:
        array = list(body[start:])
        n_words = array[:min(letters.count("N"), len(array))]
        owner = list(range(start, len(body)))

    slot = slots_mod.SlotArray(start if start is not None else 0,
                               len(n_words), len(array))
    slot.take_n(n_words)

    # every transfer names the slot it addresses: `Y >> 1` (§5.3). Pointing at
    # a slot is what makes it a reference -- not its position in the array.
    for i, ch in enumerate(letters):
        if ch != "T":
            continue
        w = body[i]
        if not uses_slot(table, (w >> 4) & 0xFF):
            continue
        k = (w & 0xF) >> 1
        if k >= len(array):
            return raw(ins, "transfer %04x addresses slot %d, the array has %d"
                       % (w, k, len(array)))
        slot.claim_reference(k, array[k], "T%04x" % w)

    # The immediate stream is the *whole* slot array -- N words and references
    # included (spec 4.4: an op addresses its immediate by byte Y into that
    # array, so the indices must be the array's own). Only *sequential* reading
    # (a MAC's C right after Y's immediate) skips reference words; `refs` says
    # which ones. Derived and checked on `540c … 2333 33c4 c008` (spec 4.4):
    # with the I words alone, Y at byte 2 reads 0 instead of `3000`.
    stream = ImmStream(array, refs={i for i, occ in enumerate(slot.slots)
                                    if occ is not None and occ.kind != slots_mod.Occupant.N})
    imm_idx = [i for i in range(len(array)) if slot.is_immediate(i)]
    slot_offset = {i: n for n, i in enumerate(imm_idx)}

    parts, statuses = [], []
    mac_i = 0
    n_list = n_words
    for i, ch in enumerate(letters):
        w = body[i]
        if ch == "A":
            text = alu_text(table, w, "A", stream)
        elif ch == "B":
            text = alu_text(table, w, "B", stream)
        elif ch == "M":
            w2 = n_list[mac_i] if mac_i < len(n_list) else 0
            mac_i += 1
            text = mac_text(table, w, w2, stream)
        elif ch == "T":
            text = transfer_text(table, w, array)
        elif ch == "C":
            text = c_text(table, w, body, i, letters, stream, array)
        else:
            continue
        if text is None:
            return raw(ins, "body word %d (%s) has no mnemonic" % (i + 1, ch))
        parts.append(text[0])
        statuses.append(text[1])
    # does the text alone identify the class? Two classes sharing a core
    # (0x040 / 0x098) leave the assembler a choice it must not make alone.
    need = shares_core(table, cls)
    return Decoded(" | ".join(parts), worst(statuses), list(ins), need_class=need,
                   free_bits=free_bits_in(table, ins, letters))


def free_bits_in(table, ins, letters):
    """the bits of an instruction that the decoding variant declares free but
    that are set in the original words.

    They have no defined meaning (spec: "bits with no semantic value"), so the
    text cannot express them; the listing hands them to the assembler as
    `.free`, which puts them back. Only bits of the variant that actually
    decoded the word count -- a bit another variant leaves free is a real
    operand here.
    """
    body = ins[1:]
    out = 0
    n_slots = [w for w, ch in zip(body, letters) if ch == "N"]
    for i, ch in enumerate(letters):
        if i >= len(body) or ch not in "ABM":
            continue
        w = body[i]
        if ch == "M":
            nxt = n_slots.pop(0) if n_slots else 0
            form = forms_for(table, "M")[0]
            v, _ = codec.decode(form, [w, nxt])
            pairs = [(w, 0), (nxt, 1)]
        else:
            form = None
            v = None
            for name in table.units.get(ch, []):
                form = table.forms_by_name[name]
                v, _ = codec.decode(form, [w])
                if v is not None:
                    break
            pairs = [(w, 0)]
        if v is None or form is None:
            continue
        for fname, src in v.fields.items():
            if not (isinstance(src, dict) and src.get("free")):
                continue
            spec = form.field_spec(fname)
            width = spec["msb"] - spec["lsb"] + 1
            mask = ((1 << width) - 1) << spec["lsb"]
            word = pairs[min(spec.get("_word", 0), len(pairs) - 1)][0]
            out |= word & mask
    return out & 0xFFFF


def shares_core(table, cls):
    """True when another class of the table has the same core layout"""
    mine = table.classes[cls].layout
    for other, c in table.classes.items():
        if other != cls and c.layout == mine:
            return True
    return False


def raw(ins, reason):
    return Decoded("raw " + " ".join("%04x" % w for w in ins[1:]), "raw", list(ins), reason)


# ---------------------------------------------------------------------------
# unit texts: (text, status) or None
# ---------------------------------------------------------------------------

def _y(text):
    """the Y operand of an ALU word from its Y nibble and mode"""
    return text


def y_operand(table, word, value_text):
    mode = (word >> 4) & 3
    y = word & 0xF
    if mode:
        return "#" + value_text
    return "-r%d" % (y & 7) if y & 8 else "r%d" % (y & 7)


def alu_text(table, w, letter, stream):
    """the text of one ALU word, or None (spec 6.1 / 6.3 / 6.4).

    Unit B is split over several forms in the table (alu_b, sfu, f_words); the
    nibble at bits 11-8 decides which one owns a word, so try each in turn.
    """
    for form in forms_for(table, letter):
        v, fields = codec.decode(form, [w])
        if v is None:
            continue
        return _render_alu(table, form, v, fields, stream)
    return None


def forms_for(table, letter):
    """the forms that can describe one template position (spec 4.2), in the
    order the table says to try them"""
    names = table.units.get(letter)
    if not names:
        raise isa.TableError("the table has no forms for position %r" % letter)
    return [table.forms_by_name[n] for n in names]


def _render_alu(table, form, v, fields, stream):
    # An immediate operand is signalled by the mode field -- unless the form
    # reuses bits 5-4 as a function selector (nibble-E/F families).
    mode = 0 if (v.mode_is_selector or form.mode_is_selector) else fields.get("mode", 0)
    bits = table.immediate_width(mode)
    if bits:
        at = fields["Y"] * 8
        ytxt = ("#" + imm_text(table, bits, stream.take(at, bits))
                + "@%d:%d" % (fields["Y"], bits))
    else:
        ytxt = _reg_text(fields.get("Y", 0))
    args = []
    shows = v.shows if v.shows is not None else form.operand_names
    for name in shows:
        if name not in v.fields:
            continue
        if name == "D" and isinstance(v.fields.get("D"), dict) and "add" in v.fields["D"]:
            args.append("r%d" % ((fields.get("D", 0) - v.fields["D"]["add"]) & 7))
            continue
        if name == "X" and form.fields.get("X", {}).get("borrowed_by_nibble") is not None:
            # X[2] was encoded in the nibble (spec 5.1): put it back
            bit = form.fields["X"]["borrowed_by_nibble"]
            args.append("r%d" % (fields.get("X", 0) | ((fields.get("b11_8", 0) & 1) << bit)))
        elif name == "Y":
            args.append(ytxt)
        else:
            args.append("r%d" % (fields.get(name, 0) & 7))
    return "%s %s" % (v.mnemonic, ", ".join(args)), v.status


def _reg_text(y):
    """a Y nibble as a register operand: bit 3 negates (spec 5.1)"""
    return "-r%d" % (y & 7) if y & 8 else "r%d" % (y & 7)


def mac_text(table, w1, w2, stream):
    """the text of a MAC pair, or None (spec 5.2, 7)"""
    form = forms_for(table, "M")[0]
    v, fields = codec.decode(form, [w1, w2])
    if v is None:
        return None
    args = []
    for name in (v.shows or form.operand_names):
        spec = form.fields.get(name, {})
        mode_field = {"Y": "modeY", "W": "modeW"}.get(name)
        bits = table.immediate_width(fields[mode_field]) if mode_field else 0
        if bits:
            args.append("#" + imm_text(table, bits, stream.take(0, bits)))
        else:
            val = fields.get(name, 0)
            negate = spec.get("negate_bit")
            if negate is not None and val & (1 << negate):
                args.append("-r%d" % (val & ((1 << negate) - 1)))
            else:
                args.append("r%d" % val)
    return "%s %s" % (v.mnemonic, ", ".join(args)), v.status


def transfer_text(table, w, array):
    """the text of one transfer word (spec 5.3).

    A plain access (fe and its aliases) addresses the slot word Y >> 1 of the
    slot array (§4.3). The other mode bytes carry their Y field themselves.
    """
    form = forms_for(table, "T")[0]
    v, fields = codec.decode(form, [w])
    if v is None:
        return None
    r = (w >> 12) & 0xF
    if v.uses_slot:
        k = (w & 0xF) >> 1
        if k >= len(array):
            return None
        # `@k` names the slot itself: two transfers may share a slot, and a
        # transfer may point past the references (§4.3)
        return "%s r%d, [%04x]@%d" % (v.mnemonic, r & 7, array[k], k), v.status
    return "%s r%d, %d" % (v.mnemonic, r & 7, w & 0xF), v.status


def c_text(table, w, body, pos, letters, stream, array, refs=None):
    """a C-position word is a jump, a hidden-register load or a condition
    (spec 5.4, 5.4b, 9.0) — which one is decided by the table"""
    # which C-position word this is, decided by the table (spec 5.4, 5.4b, 9.0)
    for form in forms_for(table, "C"):
        if codec.decode(form, [w])[0] is None:
            continue
        handler = C_HANDLERS.get(form.form)
        if handler is None:
            return None
        return handler(table, form, w, body, pos, letters, stream, array, refs)
    return None


def cond_text(table, form, w, body=None, pos=None, letters=None, stream=None, array=None, refs=None):
    v, fields = codec.decode(form, [w])
    if v is None:
        return None
    return "cond %s%d, %d, %d" % ("!" if fields["neg"] else "", fields["cc"],
                                  fields["skip_true"], fields["skip_false"]), v.status


def jump_text(table, form, w, body=None, pos=None, letters=None, stream=None, array=None, refs=None):
    """spec 5.4b: the offset is the signed 16-bit value in the slot word Y>>1"""
    slot = (w & 0xF) >> 1
    order = sorted(refs or [])
    words = list(array) if array is not None else []
    if slot >= len(words):
        return None
    word = words[slot]
    off = word - 0x10000 if word & 0x8000 else word
    return "jmp .%+d" % off, "measured"


def hidden_text(table, form, w, body=None, pos=None, letters=None, stream=None, array=None, refs=None):
    """spec 9.0: 40TY loads h4..h7; the value is an immediate word index or a
    register (bits 5-4 select which)"""
    v, fields = codec.decode(form, [w])
    if v is None:
        return None
    mode = fields.get("mode", 0)
    if mode:
        bits = table.immediate_width(mode)
        # hidden-register loads read the stream in order (isa.json: sequential_read)
        value = "#" + imm_text(table, bits, stream.take(None, bits)) if stream else "#?"
    else:
        value = _reg_text(fields.get("Y", 0))
    return "hset h%d, %s" % (v.hidden_reg, value), v.status


# the renderer of each C-position family, keyed by the family name the table
# uses (spec 5.4 condition, 5.4b jump, 9.0 hidden register)
C_HANDLERS = {
    "condition": cond_text,
    "jump": jump_text,
    "hidden_reg": hidden_text,
}


def program(words, table, title="dsp"):
    """flat words -> (source text, report)"""
    report = {"instructions": 0, "decoded": 0, "raw": 0, "statuses": {},
              "verbatim": []}
    body = []
    i = 0
    while i < len(words):
        n = words[i] & 0x1F
        if n == 0:
            body.append("; cannot continue: length 0 at word %d (%04x)" % (i, words[i]))
            report["verbatim"].append((i, words[i], "length 0"))
            break
        ins = words[i:i + n]
        if len(ins) < n:
            body.append("; truncated instruction at word %d" % i)
            report["verbatim"].append((i, words[i], "truncated stream"))
            break
        dec = disassemble_insn(table, ins)
        report["instructions"] += 1
        report["statuses"][dec.status] = report["statuses"].get(dec.status, 0) + 1
        if dec.status == "raw":
            report["raw"] += 1
            report["verbatim"].append((i, ins[0], dec.reason))
        else:
            report["decoded"] += 1
        # `.class` is a directive (not a hidden comment convention): it is
        # needed exactly when the units would fit more than one class.
        if dec.need_class or dec.status == "raw":
            # a raw line reproduces its words verbatim: the class must be
            # written too, or the assembler cannot know the header
            body.append(".class %#06x" % (ins[0] >> 5))
        if dec.free_bits:
            body.append(".free %#06x" % dec.free_bits)
        body.append("%-46s ; %-8s %s" % (dec.text, "(%s)" % dec.status,
                                         " ".join("%04x" % x for x in ins)))
        i += n
    head = ["; %s" % title,
            "; %d words, %d instructions: %d decoded, %d kept verbatim"
            % (len(words), report["instructions"], report["decoded"], report["raw"]),
            "; status: " + ", ".join("%s %d" % (k, v) for k, v in sorted(report["statuses"].items()))]
    return "\n".join(head + [";"] + body) + "\n", report
