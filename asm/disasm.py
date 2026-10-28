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

Layout (§4.3): the units fill a class's prefix (up to its last T); every word
after that is the slot array. Its first M words are the N words of the MACs;
the words a transfer points at are references; the rest is the immediate bit
stream (§4.4). `layout.py` holds that rule, and the assembler uses the same
one, so the two cannot disagree about which word is which.

Each line is checked before it is printed: the listing re-assembles it, and a
line that does not give back its own words is kept verbatim instead. That is
the reason `report["reassembly"]` exists; on the corpus it is 0.
"""

import asm as assembler
import codec
import isa
import layout as layout_rules
import slots as slots_mod

WORD = 0xFFFF


# ---------------------------------------------------------------------------
# immediates (§4.4)
# ---------------------------------------------------------------------------

class ImmStream:
    """the slot array as one MSB-first bit string, read by immediates

    The words are the whole slot array (spec 4.4), references included, so an
    explicit byte offset indexes the array directly. `refs` are the slots a
    transfer uses as references: a sequential read steps over them (see
    layout.stream_bits). Positions past the end read zeros, like the hardware.
    """

    def __init__(self, words, refs=()):
        self.words = list(words)
        self.bits = "".join(format(w, "016b") for w in self.words) + "0" * 64
        self.refs = set(refs)
        self.cursor = 0            # where the last read stopped
        self.covered = {}          # bit position -> bit, for every bit a read took
        self.overflow = False      # a read ran past the last word (spec 4.4)

    def take(self, at_bit, n):
        """`n` bits as a 32-bit RAW-aligned value (the top bits of a word).

        `at_bit` is the explicit bit position (a byte Y is 8 * Y), or None for a
        sequential read that continues from the last one.
        """
        if not n:
            return 0
        pos, self.cursor = layout_rules.stream_bits(n, self.cursor, self.refs, at_bit)
        if any(p >= 16 * len(self.words) for p in pos):
            self.overflow = True
        out = "".join(self.bits[p] if p < len(self.bits) else "0" for p in pos)
        self.covered.update(zip(pos, out))
        return (int(out, 2) << (32 - n)) & 0xFFFFFFFF

    def unread_words(self, array, owned):
        """the `.slot` directives for the stream: every slot word that the
        reads do not give, and every word past the last one a read touches
        (so the assembler makes the same array). `owned` are slot indices that
        hold a reference, an N word or a jump."""
        last = max((p // 16 for p in self.covered), default=-1)
        out = []
        for k, value in enumerate(array):
            if k in owned:
                continue
            from_reads = 0
            for j in range(16):
                bit = self.covered.get(k * 16 + j)
                if bit == "1":
                    from_reads |= 1 << (15 - j)
            if value != from_reads or k > last:
                out.append((k, value))
        return out


def fmt_number(v):
    """a number as source text: the shortest decimal that reads back as `v`"""
    if v == 0:
        return "0"
    if v == int(v) and abs(v) < 1e9:
        return "%d" % int(v)
    return repr(v)


def imm_text(table, bits, w):
    """source text of an immediate of `bits` bits, given as the top-aligned
    32-bit stream value `w` (spec 4.4).

    The text is the shortest decimal that encodes back to the same `bits` bits:
    0.2 stands for the 24-bit field 233333 (the RAW word of 0.2 is 23333333).
    Only when no decimal does is the field kept as raw bits (`raw.hhhh`).
    """
    target = w >> (32 - bits)
    value = table.raw_decode(w)
    for digits in range(1, 18):
        x = float("%.*g" % (digits, value))
        try:
            encoded = table.raw_encode(x)
        except ValueError:
            continue
        if encoded >> (32 - bits) == target:
            return fmt_number(x)
    return "raw.%0*x" % (bits // 4, target)


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
    def __init__(self, text, status, words, reason=None, free=()):
        self.text, self.status, self.words, self.reason = text, status, words, reason
        # bits the encoding leaves free that the text cannot express:
        # [(body index, mask)]
        self.free = list(free)
        # `.slot` directives: slot words no operand reads (spec 4.4)
        self.slots = []


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
    prefix = layout_rules.prefix_length(table.classes[cls].layout)
    units = letters[:prefix]

    # ---- the slot array (§4.3): the same positions the assembler uses -----
    positions = layout_rules.array_positions(letters, prefix)
    array = [body[i] for i in positions]
    n_words = layout_rules.n_words(letters, prefix)
    if any(letters[i] != "N" for i in positions[:n_words]):
        return raw(ins, "the N words are not first in the slot array")
    slot = slots_mod.SlotArray(positions[0] if positions else 0, n_words, len(array))
    slot.take_n(array[:n_words])

    # every transfer names the slot it addresses: `Y >> 1` (§5.3). Pointing at
    # a slot is what makes it a reference -- not its position in the array.
    try:
        for i, ch in enumerate(units):
            if ch == "T":
                w = body[i]
                if not uses_slot(table, (w >> 4) & 0xFF):
                    continue
                k = (w & 0xF) >> 1
                if k >= len(array):
                    return raw(ins, "transfer %04x addresses slot %d, the array has %d"
                               % (w, k, len(array)))
                slot.claim_reference(k, array[k], "T%04x" % w)
            elif ch == "C" and jump_slot_of(table, body[i]) is not None:
                k = jump_slot_of(table, body[i])
                if k >= len(array):
                    return raw(ins, "jump %04x names slot %d, the array has %d"
                               % (body[i], k, len(array)))
                slot.claim(k, slots_mod.Occupant(slots_mod.Occupant.JUMP, array[k]))
    except slots_mod.SlotError as e:
        return raw(ins, str(e))

    # The immediate stream is the whole slot array, N and reference words
    # included (spec 4.4). Only a sequential read steps over the references.
    refs = {i for i, occ in enumerate(slot.slots)
            if occ is not None and occ.kind == slots_mod.Occupant.REF}
    stream = ImmStream(array, refs)

    parts, statuses = [], []
    n_pos = [j for j in range(prefix, len(letters)) if letters[j] == "N"]
    for i, ch in enumerate(units):
        w = body[i]
        if ch == "A":
            text = alu_text(table, w, "A", stream)
        elif ch == "B":
            text = alu_text(table, w, "B", stream)
        elif ch == "M":
            # the N word of this MAC is the next N position, in order
            k = sum(1 for j in range(i) if units[j] == "M")
            w2 = body[n_pos[k]] if k < len(n_pos) else 0
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
    if stream.overflow:
        # the hardware would read the next instruction's words here (spec 4.4):
        # the line does not say what those are, so it stays verbatim
        return raw(ins, "an immediate reads past the last word of the instruction")
    owned = {i for i, occ in enumerate(slot.slots) if occ is not None}
    dec = Decoded(" | ".join(parts), worst(statuses), list(ins),
                  free=free_bits_in(table, ins, letters, prefix))
    dec.slots = stream.unread_words(array, owned)
    return dec


def jump_slot_of(table, w):
    """the slot a jump word names (009Y, spec 5.4b), or None for any other word"""
    for form in forms_for(table, "C"):
        if form.form == "jump" and codec.decode(form, [w])[0] is not None:
            return (w & 0xF) >> 1
    return None


def free_bits_in(table, ins, letters, prefix):
    """the bits of an instruction that the decoding variant declares free but
    that are set in the original words, as [(body index, mask)].

    They have no defined meaning (spec: "bits with no semantic value"), so the
    text cannot express them; the listing hands them to the assembler as
    `.free @n mask`, which puts them back in word n. Only bits of the variant
    that actually decoded the word count -- a bit another variant leaves free
    is a real operand here. A MAC coefficient's C field is free only when the
    coefficient is read in order (Y is an immediate); otherwise it holds the
    byte it is read at (spec 4.4).
    """
    body = ins[1:]
    out = []
    n_pos = [i for i in range(prefix, len(letters)) if letters[i] == "N"]
    m_count = 0
    for i, ch in enumerate(letters[:prefix]):
        if i >= len(body) or ch not in "ABM":
            continue
        w = body[i]
        if ch == "M":
            nxt = body[n_pos[m_count]] if m_count < len(n_pos) else 0
            m_count += 1
            form = forms_for(table, "M")[0]
            v, fields = codec.decode(form, [w, nxt])
            words = [(i, w), (n_pos[m_count - 1] if m_count <= len(n_pos) else i, nxt)]
        else:
            v = None
            for name in table.units.get(ch, []):
                form = table.forms_by_name[name]
                v, _ = codec.decode(form, [w])
                if v is not None:
                    break
            words = [(i, w)]
            fields = {}
        if v is None:
            continue
        y_is_imm = ch == "M" and table.immediate_width(fields.get("modeY", 0)) != 0
        if ch == "M" and not y_is_imm and w & 0x8:
            out.append((i, 0x8))          # bit 3 of a register Y is free (DISCREPANCIES D3)
        for fname, src in v.fields.items():
            if not (isinstance(src, dict) and src.get("free")):
                continue
            if fname == "C" and v.coef_bits and not y_is_imm:
                continue                  # the byte the coefficient is read at
            spec = form.field_spec(fname)
            width = spec["msb"] - spec["lsb"] + 1
            mask = ((1 << width) - 1) << spec["lsb"]
            pos, value = words[min(spec.get("_word", 0), len(words) - 1)]
            if value & mask:
                out.append((pos, value & mask))
    return out


def assembles_to(table, text, ins, free=(), pin=None, slots=()):
    """does this one line, assembled back, give exactly the words `ins`?

    The listing's own check (the honesty rule): a line the assembler cannot
    reproduce must not be printed as if it were decoded. `pin` names the class
    with a `.class` directive, which is what resolves a class ambiguity.
    """
    lines = []
    if pin is not None:
        lines.append(".class %#06x" % pin)
    lines.extend(".free @%d %#06x" % (pos + 1, mask) for pos, mask in free)
    lines.extend(".slot @%d %04x" % (k, v) for k, v in slots)
    lines.append(text)
    r = assembler.Assembler(table, allow_inferred=True).assemble("\n".join(lines))
    return r.ok and r.words == list(ins)


def raw(ins, reason):
    return Decoded("raw " + " ".join("%04x" % w for w in ins[1:]), "raw", list(ins), reason)


# ---------------------------------------------------------------------------
# unit texts: (text, status) or None
# ---------------------------------------------------------------------------

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
        yspec = v.fields.get("Y")
        if isinstance(yspec, dict) and yspec.get("or"):
            # a fixed bit 3 selects the variant (SFU forms, spec 6.4): not a negation
            ytxt = "r%d" % (fields.get("Y", 0) & ~yspec["or"] & 7)
        else:
            ytxt = _reg_text(fields.get("Y", 0))
    args = []
    shows = v.shows if v.shows is not None else form.operand_names
    for name in shows:
        if name not in v.fields and name != "R":     # R is the op field (see below)
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
            if name == "R":                # the third register of a D >= 8 op: the op field
                args.append("r%d" % (fields.get("op", 0) & 7))
            else:
                args.append("r%d" % (fields.get(name, 0) & 7))
    return "%s %s" % (v.mnemonic, ", ".join(args)), v.status


def _reg_text(y):
    """a Y nibble as a register operand: bit 3 negates (spec 5.1)"""
    return "-r%d" % (y & 7) if y & 8 else "r%d" % (y & 7)


def mac_text(table, w1, w2, stream):
    """the text of a MAC pair, or None (spec 5.2, 4.4, 7).

    The hardware reads the MAC's immediates in this order: Y at byte Y, then the
    coefficient right after Y's immediate (or at byte C when Y is a register),
    then W at byte W.
    """
    form = forms_for(table, "M")[0]
    v, fields = codec.decode(form, [w1, w2])
    if v is None:
        return None
    y_bits = table.immediate_width(fields.get("modeY", 0))
    w_bits = table.immediate_width(fields.get("modeW", 0))
    c_bits = v.coef_bits or 0
    imm = {}
    if y_bits:
        imm["Y"] = "#%s@%d:%d" % (imm_text(table, y_bits, stream.take(fields["Y"] * 8, y_bits)),
                                  fields["Y"], y_bits)
    if c_bits:
        if y_bits:
            c = stream.take(None, c_bits)                   # right after Y's immediate
            imm["C"] = "#" + imm_text(table, c_bits, c)
        else:
            byte = fields["C"]                              # at byte C (Y is a register)
            imm["C"] = "#%s@%d" % (imm_text(table, c_bits, stream.take(byte * 8, c_bits)), byte)
    if w_bits:
        imm["W"] = "#%s@%d:%d" % (imm_text(table, w_bits, stream.take(fields["W"] * 8, w_bits)),
                                  fields["W"], w_bits)
    args = []
    for name in (v.shows or form.operand_names):
        if name in imm:
            args.append(imm[name])
            continue
        spec = form.fields.get(name, {})
        val = fields.get(name, 0)
        negate = spec.get("negate_bit")
        if negate is not None and val & (1 << negate):
            args.append("-r%d" % (val & ((1 << negate) - 1)))
        else:
            # a MAC Y register: bit 3 is not a negation (DISCREPANCIES D3); it is a free bit
            args.append("r%d" % (val & 7 if name == "Y" else val))
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
    if v.kinds.get("R") == "group":
        # a condition group (X9bY, spec 5.3): the field holds 8 + group
        return "%s %d, %d" % (v.mnemonic, r - 8, w & 0xF), v.status
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
    words = list(array) if array is not None else []
    if slot >= len(words):
        return None
    word = words[slot]
    off = word - 0x10000 if word & 0x8000 else word
    return "jmp .%+d@%d" % (off, slot), "measured"


def hidden_text(table, form, w, body=None, pos=None, letters=None, stream=None, array=None, refs=None):
    """spec 9.0: 40TY loads h4..h7. The value is an integer word of the stream
    (value * 16384), read after skipping Y words; or a register (bits 5-4
    select which)"""
    v, fields = codec.decode(form, [w])
    if v is None:
        return None
    mode = fields.get("mode", 0)
    if mode:
        bits = table.immediate_width(mode)
        y = fields.get("Y", 0)                              # the byte it is read at
        n = stream.take(8 * y, bits) >> (32 - bits)         # the integer word
        value = "#%s@%d:%d" % (hidden_value_text(n), y, bits)
    else:
        value = _reg_text(fields.get("Y", 0))
    return "hset h%d, %s" % (v.hidden_reg, value), v.status


def hidden_value_text(n):
    """an integer word of a hidden register as its exact decimal (n / 16384)"""
    text = "%.14f" % (n / 16384.0)
    return text.rstrip("0").rstrip(".") if "." in text else text


# the renderer of each C-position family, keyed by the family name the table
# uses (spec 5.4 condition, 5.4b jump, 9.0 hidden register)
C_HANDLERS = {
    "condition": cond_text,
    "jump": jump_text,
    "hidden_reg": hidden_text,
}


def words_tail(words):
    """words kept as they are, with no instruction structure claimed (`.words`)"""
    return ".words " + " ".join("%04x" % x for x in words)


def program(words, table, title="dsp"):
    """flat words -> (source text, report)"""
    report = {"instructions": 0, "decoded": 0, "raw": 0, "statuses": {},
              "verbatim": []}
    body = []
    i = 0
    while i < len(words):
        n = words[i] & 0x1F
        if n == 0:
            # a header of length 0 says nothing about the words after it
            body.append(words_tail(words[i:]))
            report["verbatim"].append((i, words[i], "length 0: the rest kept as .words"))
            break
        ins = words[i:i + n]
        if len(ins) < n:
            # the dump ends inside an instruction: keep its words as they are
            body.append(words_tail(words[i:]))
            report["verbatim"].append((i, words[i], "truncated stream, kept as .words"))
            break
        if len(ins) == 1:
            # a header-only word (class 0x000, length 1) is the one-word no-op
            # (spec 4.1, 14): `raw` with no body, pinned to its class
            body.append(".class 0x0000")
            body.append("raw                                            ; (exact)  %04x" % ins[0])
            report["instructions"] += 1
            report["decoded"] += 1
            report["statuses"]["exact"] = report["statuses"].get("exact", 0) + 1
            i += 1
            continue
        dec = disassemble_insn(table, ins)
        pin = None
        if dec.status != "raw" and not assembles_to(table, dec.text, ins, dec.free,
                                                    slots=dec.slots):
            # the units may fit more than one class: name this one, if that
            # resolves it; otherwise the line is not printed as decoded
            if assembles_to(table, dec.text, ins, dec.free, pin=ins[0] >> 5,
                            slots=dec.slots):
                pin = ins[0] >> 5
            else:
                dec = raw(ins, "the assembler cannot reproduce this line; kept verbatim")
                report["reassembly"] = report.get("reassembly", 0) + 1
        report["instructions"] += 1
        report["statuses"][dec.status] = report["statuses"].get(dec.status, 0) + 1
        if dec.status == "raw":
            report["raw"] += 1
            report["verbatim"].append((i, ins[0], dec.reason))
        else:
            report["decoded"] += 1
        # `.class` is a directive: written when the class is not the one the
        # units alone select, and always for a raw line (its words are verbatim,
        # so the header must come from the directive)
        if pin is not None or dec.status == "raw":
            body.append(".class %#06x" % (ins[0] >> 5))
        for pos, mask in dec.free:
            body.append(".free @%d %#06x" % (pos + 1, mask))
        for k, value in dec.slots:
            body.append(".slot @%d %04x" % (k, value))
        body.append("%-46s ; %-8s %s" % (dec.text, "(%s)" % dec.status,
                                         " ".join("%04x" % x for x in ins)))
        i += n
    head = ["; %s" % title,
            "; %d words, %d instructions: %d decoded, %d kept verbatim"
            % (len(words), report["instructions"], report["decoded"], report["raw"]),
            "; status: " + ", ".join("%s %d" % (k, v) for k, v in sorted(report["statuses"].items()))]
    return "\n".join(head + [";"] + body) + "\n", report
