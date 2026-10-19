"""Assembler: dspasm source -> program words (DSP_ISA_SPEC.md §3, §4, §12).

    result = Assembler().assemble(text)
    result.words        flat 16-bit words in execution order
    result.errors       [(line, message)] — rejected, never guessed
    result.notes        [(line, message)] — assembled but flagged

The language is what `disasm.program` writes back:

    label:  unit [| unit ...]          ; comment
    mac_p1p2 r0, r0, r1, r2, r1
    raw @0x060 0fe0 0201              ; verbatim body words, optional class

Confidence policy is enforced **here**, not by the caller: a line whose
variant is not in `Table.EMIT_DEFAULT` (exact, measured) is rejected unless
`allow_inferred=True` is passed, which admits `inferred` and `fitted`. `raw`
words are never emitted by default either: they are unknown encodings kept
verbatim, so re-assembling them takes the same explicit option.

Ambiguity is rejected: if more than one class can hold the units, or a field
that must be preserved has no source, the line fails with the reason.
"""

import re

import codec
import isa
import layout
import slots as slots_mod

WORD = 0xFFFF
MAX_LEN = 31


class AsmError(Exception):
    """one rejected line"""

    def __init__(self, line, msg, text=None):
        self.line, self.msg, self.text = line, msg, text
        Exception.__init__(self, "line %d: %s" % (line, msg) if line else msg)


class Result:
    def __init__(self):
        self.words = []
        self.errors = []
        self.notes = []
        self.assembled = 0
        self.rejected = 0

    @property
    def ok(self):
        return not self.errors

    def summary(self):
        return ("assembled %d instruction(s), rejected %d, %d note(s)"
                % (self.assembled, self.rejected, len(self.notes)))


# ---------------------------------------------------------------------------
# tokens
# ---------------------------------------------------------------------------

def split_units(body):
    return [u.strip() for u in body.split("|") if u.strip()]


def tokenize(unit):
    """'mac r0, r1' -> ('mac', ['r0', 'r1']); brackets do not split on commas"""
    unit = unit.strip()
    if not unit:
        return None, []
    head, _, rest = unit.partition(" ")
    ops, cur, depth = [], "", 0
    for ch in rest:
        if ch == "[":
            depth += 1
        elif ch == "]":
            depth -= 1
        if ch == "," and depth == 0:
            ops.append(cur.strip())
            cur = ""
        else:
            cur += ch
    if cur.strip():
        ops.append(cur.strip())
    return head, ops


def parse_register(text):
    """r0..r7, optionally negated (-r3 sets Y bit 3, spec 5.1)"""
    t = text.strip()
    neg = t.startswith("-")
    if neg:
        t = t[1:].strip()
    if len(t) < 2 or t[0] not in "rR" or not t[1:].isdigit():
        raise codec.CodecError("%r is not a register (r0..r7)" % text)
    n = int(t[1:])
    if not 0 <= n <= 7:
        raise codec.CodecError("register out of range: %r" % text)
    if neg and n == 0:
        # -r0 is legal in the encoding (bit 3 set, register 0): the spec's
        # measured XOR and MAC forms produce it, so it is not an error
        return -8 - 0
    return -n if neg else n


def parse_immediate(text, whole=None):
    """an immediate: a number, or `raw.NNNN` for bits that are not a canonical
    RAW word (the disassembler writes those when the value would not round-trip)"""
    t = text.strip()
    m = re.fullmatch(r"(?i)raw\.([0-9a-f]+)", t)
    if m:
        # the bits stand for the *top* of a 16/24/32-bit immediate: the value is
        # what decoding those bits as a RAW word gives (spec 4.4)
        bits = m.group(1)
        if len(bits) > 8:
            raise codec.CodecError("%r is not an immediate word pattern" % t)
        return _RawImm(bits)
    return parse_number(t)


class _RawImm(float):
    """an immediate given as raw bits; the assembler writes those bits verbatim"""

    def __new__(cls, bits):
        self = float.__new__(cls, 0.0)
        self.bits = bits
        return self

    def __repr__(self):
        return "raw.%s" % self.bits


def parse_number(text):
    t = text.strip()
    try:
        if t.lower().startswith("0x"):
            return int(t, 16)
        # a bare hexadecimal word (the disassembler writes `0fe0`, `0a02`)
        if re.fullmatch(r"[0-9a-fA-F]{4}", t) and re.search(r"[a-fA-F]", t):
            return int(t, 16)
        if t.lower().startswith("0b"):
            return int(t[2:], 2)
        if re.fullmatch(r"-?\d+", t):
            return int(t, 10)
        return float(t)
    except ValueError:
        raise codec.CodecError("%r is not a number" % text)


def parse_operand(kind, text):
    """-> codec.Arg"""
    t = text.strip()
    if not t:
        raise codec.CodecError("empty operand")
    if kind in ("reg", "dst_reg", "x_reg", "reg_x2"):
        return codec.Arg("reg", parse_register(t), t)
    if kind == "y_operand":
        if t.startswith("#"):
            body = t[1:]
            at = width = None
            m = re.fullmatch(r"(?P<v>.+?)@(?P<b>\d+)(?::(?P<w>\d+))?", body)
            if m:
                body, at = m.group("v"), int(m.group("b"))
                width = int(m.group("w")) if m.group("w") else None
            arg = codec.Arg("imm", parse_immediate(body, t), t, at_byte=at)
            arg.width = width
            return arg
        return codec.Arg("reg", parse_register(t), t)
    if kind == "ref":
        slot = None
        m = re.fullmatch(r"\[(?P<v>[^\]]+)\]@(?P<s>\d+)", t)
        if m:
            t, slot = "[%s]" % m.group("v"), int(m.group("s"))
        if not (t.startswith("[") and t.endswith("]")):
            raise codec.CodecError("%r is not a reference ([0x0200])" % t)
        inner = t[1:-1].strip()
        # references are written in hex by the disassembler ([0400] = 0x0400)
        if re.fullmatch(r"[0-9a-fA-F]{1,4}", inner):
            return codec.Arg("ref", int(inner, 16), t, at_byte=slot)
        return codec.Arg("ref", parse_number(inner), t, at_byte=slot)
    if kind == "cond":
        neg = t.startswith("!")
        if neg:
            t = t[1:]
        m = re.fullmatch(r"(?i)(?:cond)?(\d{1,2})", t)
        if not m:
            raise codec.CodecError("%r is not a condition (cond0..cond31, !condN)" % text)
        n = int(m.group(1))
        if not 0 <= n <= 31:
            raise codec.CodecError("condition %d is out of range (0..31)" % n)
        return codec.Arg("cond", (n, neg), text)
    if kind in ("number", "units"):
        return codec.Arg("num", int(parse_number(t)), t)
    if kind == "word":
        return codec.Arg("word", int(parse_number(t)) & WORD, t)
    if kind == "hidden_reg":
        m = re.fullmatch(r"h([4-7])", t.lower())
        if not m:
            raise codec.CodecError("%r is not a hidden register (h4..h7)" % t)
        return codec.Arg("num", int(m.group(1)), t)
    if kind == "label":
        # a jump target: the disassembler writes the offset itself (`.+3`), and
        # that is the only form the table can place without a linker
        m = re.fullmatch(r"[.]([+-]\d+)", t)
        if not m:
            raise codec.CodecError(
                "%r is not a jump offset: write `.%%+d` words from the instruction "
                "end (spec 5.4b); symbolic labels need a linker" % t)
        return codec.Arg("label", int(m.group(1)), t)
    raise codec.CodecError("operands of kind %r are not supported" % kind)


# ---------------------------------------------------------------------------
# one unit of a line
# ---------------------------------------------------------------------------

class Unit:
    def __init__(self, variant, args, text):
        self.variant, self.args, self.token = variant, args, text
        self.form = variant.form
        self.words = []
        self.read = None

    def immediate(self):
        """the immediate value this unit carries, or None (spec 4.4)"""
        for name in self.form.immediate_operands:
            i = self.form.operand_names.index(name)
            if i < len(self.args) and self.args[i] is not None and self.args[i].is_immediate:
                return self.args[i].value
        return None

    def jump_offset(self):
        """a jump's offset in words (§5.4b): it lives in a slot word"""
        for name in self.form.operand_names:
            if self.form.kinds.get(name) != "label":
                continue
            i = self.form.operand_names.index(name)
            if i < len(self.args) and self.args[i] is not None:
                return int(self.args[i].value)
        return None

    def slots(self):
        """the reference words this unit addresses, in order (§4.3)"""
        if not self.variant.uses_slot:
            return []
        out = []
        for name in self.form.operand_names:
            if self.form.kinds.get(name) != "ref":
                continue
            i = self.form.operand_names.index(name)
            if i < len(self.args) and self.args[i] is not None:
                out.append(int(self.args[i].value))
        return out


# ---------------------------------------------------------------------------
# the assembler
# ---------------------------------------------------------------------------

class Assembler:
    def __init__(self, table=None, allow_inferred=False):
        self.table = table or isa.Table.load()
        self.allow_inferred = allow_inferred

    # -- public ------------------------------------------------------------
    def assemble(self, text):
        result = Result()
        lines = text.splitlines()
        pending_class = None
        pending_free = 0
        for n, raw in enumerate(lines, 1):
            line = raw.split(";")[0].rstrip()
            if not line.strip():
                continue
            # `.class 0x0558`: a directive that pins the next instruction's class
            m = re.match(r"\s*\.class\s+(?P<c>0x[0-9a-fA-F]+|[0-9]+)\s*$", line)
            if m:
                try:
                    pending_class = int(m.group("c"), 0)
                except ValueError:
                    result.errors.append((n, "not a class: %r" % m.group("c")))
                    result.rejected += 1
                    continue
                if pending_class not in self.table.classes:
                    result.errors.append((n, "class %#05x is not in the table" % pending_class))
                    result.rejected += 1
                    pending_class = None
                continue
            m = re.match(r"\s*\.free\s+(?P<v>0x[0-9a-fA-F]+|[0-9]+)\s*$", line)
            if m:
                try:
                    pending_free = int(m.group("v"), 0) & 0xFFFF
                except ValueError:
                    result.errors.append((n, "not a bit mask: %r" % m.group("v")))
                    result.rejected += 1
                continue
            # the legacy hint in a trailing comment: `; @0x0040 (exact) 0803 ...`
            hint = re.search(r";\s*@(0x[0-9a-fA-F]+)", raw)
            pint = pending_class
            if hint and pint is None:
                pint = int(hint.group(1), 16)
            pending_class = None
            try:
                words = self.line(n, line, pint, pending_free)
                pending_free = 0
            except AsmError as e:
                result.errors.append((e.line or n, e.msg))
                result.rejected += 1
                pending_free = 0
                continue
            except codec.CodecError as e:
                result.errors.append((n, str(e)))
                result.rejected += 1
                continue
            if words is None:             # a note, no words
                continue
            result.words.extend(words)
            result.assembled += 1
        return result

    # -- one line ----------------------------------------------------------
    def line(self, lineno, text, pinned_class=None, free_bits=0):
        m = re.match(r"^\s*(?:([A-Za-z_.][\w.]*)\s*:)?\s*(.*)$", text)
        body = m.group(2).strip()
        if not body:
            return None
        tokens = split_units(body)
        if tokens and tokens[0].split()[0] == "raw":
            return self._raw(lineno, body, pinned_class)
        units = [self._unit(lineno, t) for t in tokens]
        return self._instruction(lineno, units, pinned_class, free_bits)

    def _unit(self, lineno, token):
        mnemonic, raw_ops = tokenize(token)
        variant = self.table.mnemonics.get(mnemonic)
        if variant is None:
            near = [m for m in sorted(self.table.mnemonics) if m.startswith(mnemonic[:3])][:3]
            hint = (" (similar: %s)" % ", ".join(near)) if near else ""
            raise AsmError(lineno, "unknown mnemonic %r%s" % (mnemonic, hint), token)
        self._check_confidence(lineno, variant, token)
        form = variant.form
        # the operands a variant *shows* are the ones the source writes
        # (`shows` in isa.json); the form's list is the positional map of fields
        shown = variant.shows if variant.shows is not None else list(form.operand_names)
        if len(raw_ops) > len(shown):
            raise AsmError(lineno, "%s takes %d operand(s) (%.*s), %d given"
                           % (mnemonic, len(shown),
                              min(len(", ".join(shown)), 60), ", ".join(shown),
                              len(raw_ops)), token)
        if len(raw_ops) < len(shown):
            raise AsmError(lineno, "%s needs %d operand(s) (%.*s), %d given"
                           % (mnemonic, len(shown),
                              min(len(", ".join(shown)), 60), ", ".join(shown),
                              len(raw_ops)), token)
        args = [None] * len(form.operand_names)
        for name, given in zip(shown, raw_ops):
            i = form.operand_names.index(name)
            args[i] = self._arg(lineno, variant, name, given, token)
        return Unit(variant, args, token)

    def _arg(self, lineno, variant, name, given, token):
        kind = variant.kinds.get(name) or variant.form.kinds.get(name)
        try:
            return parse_operand(kind, given)
        except codec.CodecError as e:
            raise AsmError(lineno, "%s: %s" % (variant.mnemonic, e), token)

    def _check_confidence(self, lineno, variant, token):
        if self.table.emittable(variant, self.allow_inferred):
            return
        if variant.status in isa.EMIT_OPT_IN:
            msg = ("%s is %s: emit it with allow_inferred=True "
                   "(which admits inferred and fitted variants)"
                   % (variant.mnemonic, variant.status))
        else:
            msg = "%s is %s: never emittable" % (variant.mnemonic, variant.status)
        raise AsmError(lineno, msg, token)

    # -- raw ---------------------------------------------------------------
    def _raw(self, lineno, token, pinned_class=None):
        # `raw @0x060 0fe0 0201`: whitespace (or commas) separate the words
        body = token.strip()
        head = body.split(None, 1)
        rest = head[1] if len(head) > 1 else ""
        ops = [o for o in rest.replace(",", " ").split() if o]
        if not self.allow_inferred:
            raise AsmError(lineno, "raw words carry no known encoding; "
                                   "re-assembling them needs --allow-inferred", token)
        cls = pinned_class
        if ops and ops[0].startswith("@"):
            try:
                cls = int(ops[0][1:], 16)
            except ValueError:
                raise AsmError(lineno, "%r is not a class" % ops[0], token)
            ops = ops[1:]
        if not ops:
            raise AsmError(lineno, "raw needs at least one body word", token)
        body = []
        for o in ops:
            t = o.strip()
            try:
                if re.fullmatch(r"[0-9a-fA-F]{1,4}", t):
                    body.append(int(t, 16))          # a word is written in hex
                else:
                    body.append(int(parse_number(t)) & WORD)
            except codec.CodecError as e:
                raise AsmError(lineno, "raw: %s" % e, token)
        if len(body) > MAX_LEN - 1:
            raise AsmError(lineno, "raw holds at most %d words" % (MAX_LEN - 1), token)
        if cls is None:
            cls = self._class_from_words(body)
            if cls is None:
                raise AsmError(lineno, "raw: no class in the table gives exactly these "
                                       "words; write `raw @0x0NN ...`", token)
        return [isa.header_of(cls, len(body) + 1)] + body

    def _class_from_words(self, body):
        """a class whose layout explains these words exactly"""
        hits = []
        for cls in sorted(self.table.classes):
            letters = layout.letters_for(self.table, cls, len(body))
            if letters is None or len(letters) != len(body):
                continue
            if layout.fits_exact(self.table, letters, body):
                hits.append(cls)
        return hits[0] if len(hits) == 1 else None

    # -- a normal instruction ---------------------------------------------
    def _instruction(self, lineno, units, pinned_class=None, free_bits=0):
        # spec 4.4: every A/B op that takes an immediate reads it at *its own*
        # Y byte of the one bit stream, so one instruction can present several
        # values to several ops. Each is written into the stream separately.
        carriers = [u for u in units if u.immediate() is not None]
        value = carriers[0].immediate() if carriers else None
        bits = None
        imm_words = 0
        if carriers:
            bits = self._width(lineno, value, carriers[0].variant,
                               given_offset=self._carrier_offset(carriers[0]) is not None,
                               stated=self._carrier_width(carriers[0]))
            for u in carriers[1:]:
                bits = max(bits, self._width(lineno, u.immediate(), u.variant,
                                             given_offset=self._carrier_offset(u) is not None,
                                             stated=self._carrier_width(u)))
            # spec 4.4: the I words must cover every carrier's byte offset
            need = 0
            for u in carriers:
                i = [n for n in u.form.immediate_operands
                     if u.args[u.form.operand_names.index(n)] is not None
                     and u.args[u.form.operand_names.index(n)].is_immediate][0]
                arg = u.args[u.form.operand_names.index(i)]
                w = self._width(lineno, arg.value, u.variant,
                                given_offset=arg.at_byte is not None,
                                stated=getattr(arg, "width", None))
                at = arg.at_byte or 0
                need = max(need, at * 8 + w)
            imm_words = (need + 15) // 16     # local: never leaks to the next line

        # one call decides the class *and* the body length (nothing recomputes it)
        imm_words = imm_words if bits else 0
        choices = self.classify(units, imm_words)
        ordered = sorted(units, key=lambda u: self.table.unit_rank.get(
            _letter(self.table, u.form), 0))
        if not choices:
            raise AsmError(lineno, "no class in the table holds these units with "
                                   "%d slot word(s)%s"
                           % (max(self._slots_needed(ordered),
                                  sum(1 for u in ordered if u.jump_offset() is not None)),
                              "" if not bits else " and %d immediate word(s)" % imm_words))
        if len(choices) > 1 and pinned_class is not None:
            choices = [c for c in choices if c[0] == pinned_class] or choices
        if len(choices) > 1:
            raise AsmError(lineno, "ambiguous: classes %s all hold these units; "
                                   "name one with `.class` or in the trailing "
                                   "comment (`; @0x%04x`)"
                           % (", ".join("%#05x" % c for c, _ in choices), choices[0][0]))
        cls, letters = choices[0]
        return self._emit(lineno, cls, letters, ordered, value, bits, free_bits)

    def classify(self, units, imm_words):
        """-> [(class, letters)] , the one place that sizes an instruction.

        Everything an instruction needs is decided here together: which class
        can hold these units, and how many body words it has. Callers must not
        recompute the length -- that is what made the two disagree.
        """
        rank = self.table.unit_rank
        ordered = sorted(units, key=lambda u: rank.get(_letter(self.table, u.form), 0))
        letters = [_letter(self.table, u.form) for u in ordered]
        # spec 4.2: the M of a MAC sits with the ops, its N in the slot array
        n_after = sum(1 for u in ordered if u.form.form == "mac")
        if n_after:
            last_t = max((i for i, ch in enumerate(letters) if ch == "T"), default=-1)
            at = len(letters) if last_t < 0 else last_t + 1
            letters = letters[:at] + ["N"] * n_after + letters[at:]
        # every slot word the line fills: references plus any hole the source
        # names explicitly, plus one per jump (spec 4.3, 5.4b)
        slots = self._slots_needed(ordered)
        jumps = sum(1 for u in ordered if u.jump_offset() is not None)
        # The slot array covers the references, the jumps, and the bytes the
        # immediates are read at (§4.3, §4.4): one span, three consumers.
        slot_words = max(slots, jumps, imm_words)
        out = []
        for cls, c in sorted(self.table.classes.items()):
            lay = c.layout
            nr = lay.count("R")
            core = lay[:len(lay) - nr] if nr else lay
            # the class's core must match the units, position for position
            if list(core) != letters[:len(core)]:
                continue
            if len(letters) > len(core):
                continue
            # the body is: the core, then the slot array the line actually
            # needs, then the immediate words it actually needs. A class keeps
            # R words for the slot array; one without R keeps them in its tail.
            body = len(core) + slot_words
            if not c.allows(body):
                continue
            want = layout.letters_for(self.table, cls, body)
            if want is None or len(want) != body:
                continue
            # the core positions must still read as the units say, and the
            # slot array must have room for every word the line fills
            if want[:len(letters)] != letters:
                continue
            out.append((cls, want))
        return out

    def _width(self, lineno, value, variant=None, given_offset=False, stated=None):
        """the immediate width (spec 4.4).

        `immediate_bits` on a variant is the width the spec's own example uses
        for the plain form (11.2 writes `mul` as imm32). When the source states
        the byte offset the stream already exists, so the narrowest width that
        holds the value is what reproduces those bits.
        """
        if value is None:
            return None
        if stated:
            return stated          # the source said how wide the stream is
        if isinstance(value, _RawImm):
            need = len(value.bits) * 4
            want = getattr(variant, "immediate_bits", None)
            # the bits were given literally: take the narrowest width that holds
            # them, but never narrower than the variant requires
            bits = max(need, want or 0) if want else (
                16 if need <= 16 else 24 if need <= 24 else 32)
            if bits == 16:
                bits = 16
            elif bits <= 24:
                bits = 24
            else:
                bits = 32
            if len(value.bits) * 4 > bits:
                raise AsmError(lineno, "raw.%s does not fit in %d bits"
                               % (value.bits, bits))
            return bits
        try:
            raw = self.table.raw_encode(value)
        except ValueError as e:
            raise AsmError(lineno, str(e))
        want = None if given_offset else getattr(variant, "immediate_bits", None)
        if want:
            if want == 16 and raw & 0xFFFF:
                raise AsmError(lineno, "%s takes an imm16 and %r does not fit"
                               % (variant.mnemonic, value))
            if want == 24 and raw & 0xFF:
                raise AsmError(lineno, "%s takes an imm24 and %r does not fit"
                               % (variant.mnemonic, value))
            return want
        if not raw & 0xFFFF:
            return 16
        if not raw & 0xFF:
            return 24
        return 32

    def _slots_needed(self, units):
        """how many slot words the line needs (§4.3).

        That is the highest byte anything addresses: each reference and each
        jump occupies one word, and an immediate read at byte b (spec 4.4)
        reaches word b/2.
        """
        top = -1
        refs = 0
        for u in units:
            for name in u.form.operand_names:
                if u.form.kinds.get(name) != "ref":
                    continue
                arg = u.args[u.form.operand_names.index(name)]
                if arg is None:
                    continue
                refs += 1
                top = max(top, arg.at_byte if arg.at_byte is not None else refs - 1)
        # an immediate may be read past the references: cover its byte range
        for u in units:
            if u.immediate() is None:
                continue
            at = self._carrier_offset(u) or 0
            w = self._width(0, u.immediate(), u.variant,
                            given_offset=at is not None,
                            stated=self._carrier_width(u)) or 16
            top = max(top, (at * 8 + w - 1) // 16)
        for u in units:
            if u.jump_offset() is not None:
                top = max(top, refs)
        return top + 1 if top >= 0 else 0

    def _explicit_slots(self, units):
        """the slot each transfer must occupy, in transfer order.

        A reference may name its slot (`[8000]@1`); two transfers can share a
        value and still sit in different slots, so this is a list, not a map.
        """
        out = []
        for u in units:
            for name in u.form.operand_names:
                if u.form.kinds.get(name) != "ref":
                    continue
                arg = u.args[u.form.operand_names.index(name)]
                if arg is not None:
                    out.append(arg.at_byte)
        return out

    def _emit(self, lineno, cls, letters, units, value, bits, free_bits=0):
        explicit = self._explicit_slots(units)
        """place every unit at the body positions its letter owns (spec 4.2).

        The letters are the authority: an A/B/C/T word goes to the next free
        position with its letter, and a MAC's two words go to an M and an N.
        """
        body = [0] * len(letters)
        free = {}
        for i, ch in enumerate(letters):
            free.setdefault(ch, []).append(i)
        for lst in free.values():
            lst.reverse()                  # pop() takes the first position
        n_slots = free.pop("N", [])
        for u in units:
            words, read = codec.encode(u.form, u.variant, u.args, bits)
            if read is not None and u.immediate() is None:
                raise AsmError(lineno, "%s expects an immediate" % u.variant.mnemonic)
            if u.form.form == "mac":
                spots = [free["M"].pop(), n_slots.pop()]
            else:
                letter = _letter(self.table, u.form)
                if not free.get(letter):
                    raise AsmError(lineno, "class %#05x has no %s position left"
                                   % (cls, letter))
                spots = [free[letter].pop()]
            for spot, w in zip(spots, words):
                body[spot] = w
        # `.free 0x...`: restore bits the variant declares free (no defined
        # meaning, so a round trip must keep them). Only bits that really are
        # free in these units are accepted; anything else is an error.
        if free_bits:
            allowed = 0
            for spot, w in self._free_spots(cls, letters, units):
                allowed |= w
            stray = free_bits & ~allowed
            if stray:
                raise AsmError(lineno, "0x%04x is not a free bit of this instruction "
                                       "(free bits: 0x%04x)" % (stray, allowed))
            for spot, mask in self._free_spots(cls, letters, units):
                body[spot] |= free_bits & mask
        # ---- the slot array (§4.3): discover every occupant, then write ----
        body = self._place_slots(lineno, cls, letters, units, body, explicit, bits)
        self._check_len(lineno, body)
        return [isa.header_of(cls, len(body) + 1)] + body

    def _free_spots(self, cls, letters, units):
        """(body position, bit mask) for every field the variants leave free"""
        spots = []
        free = {}
        for i, ch in enumerate(letters):
            free.setdefault(ch, []).append(i)
        for lst in free.values():
            lst.reverse()
        n_slots = list(free.get("N", []))
        for u in units:
            if u.form.form == "mac":
                positions = [free["M"].pop(), n_slots.pop()]
            else:
                letter = _letter(self.table, u.form)
                if not free.get(letter):
                    continue
                positions = [free[letter].pop()]
            for fname, src in u.variant.fields.items():
                if not (isinstance(src, dict) and src.get("free")):
                    continue
                spec = u.form.field_spec(fname)
                width = spec["msb"] - spec["lsb"] + 1
                mask = ((1 << width) - 1) << spec["lsb"]
                idx = spec.get("_word", 0)
                spots.append((positions[min(idx, len(positions) - 1)], mask))
        return spots

    def _place_slots(self, lineno, cls, letters, units, body, explicit, bits):
        """Give every word after the last T exactly one owner (§4.3).

        Discovery happens first: the N words take the first slots, then each
        transfer claims the slot its Y field names, then a jump claims one.
        Whatever nobody claims is the immediate stream. Only after the whole
        map exists is the array written, so no two rules can disagree by order
        of execution -- a conflict is an error.
        """
        start = layout.slot_start(letters)
        if start is None:
            # The slot array is "every word after the last T" (§4.3), so a class
            # with no T has no array: its N word was encoded with the MAC and is
            # already in place. Only the I positions take the immediate stream,
            # and a jump's offset takes the last of them (spec 5.4b).
            spots = [i for i, ch in enumerate(letters) if ch == "I"]
            imm = self._immediate_words(units, bits, len(spots))
            for spot, w in zip(spots, imm):
                body[spot] = w
            offsets = [u.jump_offset() for u in units if u.jump_offset() is not None]
            for j, off in enumerate(offsets):
                if j < len(spots):
                    body[spots[len(spots) - 1 - j]] = off & WORD
            return body

        size = len(letters) - start
        nn = layout.n_words(letters)
        # the N words were encoded by codec.encode into the MAC's own positions
        n_values = [body[i] for i, ch in enumerate(letters) if ch == "N"]
        arr = slots_mod.SlotArray(start, nn, size)
        try:
            arr.take_n(n_values)
            # references: the slot each transfer names (`Y >> 1`), or the next
            # free one when the source does not say (§5.3)
            ti = 0
            for u in units:
                for ref in u.slots():
                    named = explicit[ti] if ti < len(explicit) else None
                    if named is None:
                        named = self._next_free(arr)
                    arr.claim_reference(named, ref, u.variant.mnemonic)
                    ti += 1
            # jump offsets: one slot each, after the references
            for u in units:
                off = u.jump_offset()
                if off is None:
                    continue
                arr.claim(self._next_free(arr), slots_mod.Occupant(slots_mod.Occupant.JUMP, off))
        except slots_mod.SlotError as e:
            raise AsmError(lineno, "class %#05x: %s" % (cls, e))

        # write, from the finished map
        imm_words = self._immediate_words(units, bits, len(arr.free_slots()))
        k = 0
        for i in range(size):
            if arr.slots[i] is None:
                body[start + i] = imm_words[k] if k < len(imm_words) else 0
                k += 1
            else:
                body[start + i] = arr.slots[i].value & WORD
        # point each transfer's Y field at the slot it owns
        positions = [i for i, ch in enumerate(letters) if ch == "T"]
        ti = 0
        for u in units:
            for fname, src in u.variant.fields.items():
                if not (isinstance(src, dict) and src.get("slot")):
                    continue
                spec = u.form.field_spec(fname)
                shift = spec.get("slot_shift", 1)
                named = explicit[ti] if ti < len(explicit) else None
                if named is None:
                    named = self._next_free_reference(arr, u.slots()[0])
                body[positions[ti]] = isa.put_bits(body[positions[ti]], spec,
                                                  named << shift)
                ti += 1
        return body

    @staticmethod
    def _next_free(arr):
        """the first slot nobody owns (N words and references already taken)"""
        for i, occ in enumerate(arr.slots):
            if occ is None:
                return i
        raise slots_mod.SlotError("the slot array is full (%d words)" % arr.size)

    @staticmethod
    def _next_free_reference(arr, value):
        return arr.slots.index(next(o for o in arr.slots
                                    if o is not None and o.kind == slots_mod.Occupant.REF
                                    and o.value == value))

    @staticmethod
    def _carrier_width(u):
        """the width the source states (`#v@b:32`), if any"""
        for name in u.form.immediate_operands:
            arg = u.args[u.form.operand_names.index(name)]
            if arg is not None and arg.is_immediate:
                return getattr(arg, "width", None)
        return None

    @staticmethod
    def _carrier_offset(u):
        """the byte offset the source names for this unit's immediate, if any"""
        for name in u.form.immediate_operands:
            arg = u.args[u.form.operand_names.index(name)]
            if arg is not None and arg.is_immediate:
                return arg.at_byte
        return None

    def _immediate_words(self, units, bits, count):
        """the immediate stream as words (spec 4.4), `count` words long"""
        carriers = [u for u in units if u.immediate() is not None]
        if not carriers or not bits:
            return [0] * count
        stream = [0] * count
        for u in carriers:
            at = 0
            for name in u.form.immediate_operands:
                arg = u.args[u.form.operand_names.index(name)]
                if arg is not None and arg.is_immediate and arg.at_byte is not None:
                    at = arg.at_byte
            ub = self._width(0, u.immediate(), u.variant,
                             given_offset=self._carrier_offset(u) is not None)
            stream = self._put_bits(stream, at, self._imm_bitstring(u.immediate(), ub))
        return stream

    def _write_slot_fields(self, units, refs, nn, body, letters, start=None, explicit=None):
        """point each {"slot": true} field at its unit's reference (§4.3).

        Each transfer word is a separate body position, in order, so the k-th
        transfer takes the k-th slot.
        """
        positions = [i for i, ch in enumerate(letters) if ch == "T"]
        # The slot array starts with the N words (§4.3). Slot k belongs to the
        # k-th *distinct* reference: two transfers may share one (the corpus
        # has `a2c2` reusing slot 1), so the index advances only for a new
        # address. References are matched by value, which keeps a repeated
        # reference pointing where it originally pointed.
        order = []
        for u in units:
            for ref in u.slots():
                if ref not in order:
                    order.append(ref)
        ti = 0
        for u in units:
            own = u.slots()                  # the references this unit asks for
            if not own:
                continue
            for fname, src in u.variant.fields.items():
                if not (isinstance(src, dict) and src.get("slot")):
                    continue
                tg = ti
                slot = nn + (explicit[tg] if tg < len(explicit or []) and explicit[tg] is not None
                             else order.index(own[0]))
                spec = u.form.field_spec(fname)
                shift = spec.get("slot_shift", 1)
                pos = positions[ti]
                try:
                    body[pos] = isa.put_bits(body[pos], spec, slot << shift)
                except ValueError:
                    raise AsmError(0, "slot %d does not fit the Y field of %s "
                                       "(spec 5.3: slot index = Y >> 1, so at most %d)"
                                   % (slot, u.variant.mnemonic,
                                      (1 << (spec["msb"] - spec["lsb"] + 1 - shift)) - 1))
                ti += 1

    @staticmethod
    def _imm_word_count(bits):
        """I words an immediate needs: 16 bits = 1, 24 = 2, 32 = 2 (spec 4.4)"""
        return 0 if not bits else (1 if bits == 16 else 2)

    @staticmethod
    def _put_bits(stream, at_byte, bitstring):
        """OR a value's bits into the stream starting at a byte position (§4.4).
        The stream is a list of 16-bit words, laid out most significant first."""
        total = "".join(format(w, "016b") for w in stream)
        start = at_byte * 8
        buf = list(total)
        for i, ch in enumerate(bitstring):
            pos = start + i
            if pos >= len(buf):
                buf.extend("0" * (pos + 1 - len(buf)))
            if ch == "1":
                buf[pos] = "1"
        out = []
        for i in range(len(stream)):
            chunk = "".join(buf[i * 16:(i + 1) * 16]).ljust(16, "0")
            out.append(int(chunk, 2))
        return out

    def _imm_bitstring(self, value, bits):
        """an immediate as a `bits`-long bit string, most significant first
        (spec 4.4: the value is the top bits of the RAW word)"""
        if isinstance(value, _RawImm):
            hexs = (value.bits + "0" * 8)[:bits // 4]
            return "".join(format(int(c, 16), "04b") for c in hexs)
        raw = self.table.raw_encode(value) & 0xFFFFFFFF
        return format(raw, "032b")[:bits]

    def _imm_words(self, value, bits):
        """the I words of an immediate (spec 4.4): the *top* `bits` bits of the
        RAW word, most significant first, padded with zeros"""
        if isinstance(value, _RawImm):
            # the bits were given literally: pad on the right to the width
            hexs = (value.bits + "0" * 8)[:bits // 4]
            n = bits // 16
            return [int(hexs[4 * i:4 * i + 4], 16) for i in range(n)]
        raw = self.table.raw_encode(value) & 0xFFFFFFFF
        if bits == 16:
            return [(raw >> 16) & WORD]
        if bits == 24:
            return [(raw >> 16) & WORD, (raw >> 8) & WORD]
        return [(raw >> 16) & WORD, raw & WORD]

    def _check_len(self, lineno, body):
        if len(body) + 1 > MAX_LEN:
            raise AsmError(lineno, "instruction is %d words; the header allows %d"
                           % (len(body) + 1, MAX_LEN))


def _letter(table, form):
    for letter, names in table.units.items():
        if form.name in names:
            return letter
    raise AsmError(0, "the table does not place %r" % form.name)


def assemble(text, table=None, allow_inferred=False):
    """convenience: source text -> Result"""
    return Assembler(table, allow_inferred).assemble(text)
