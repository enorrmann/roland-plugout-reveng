"""Assembler: dspasm source -> program words (DSP_ISA_SPEC.md §3, §4, §12).

    result = Assembler().assemble(text)
    result.words        flat 16-bit words in execution order
    result.errors       [(line, message)] — rejected, never guessed
    result.notes        [(line, message)] — assembled but flagged

The language is what `disasm.program` writes back:

    label:  unit [| unit ...]          ; comment
    mac_p1p2 r0, r0, r1, r2, r1
    raw @0x060 0fe0 0201              ; verbatim body words, optional class

Directives, each on the line before the instruction it changes:

    .class 0x0040        the class of the next instruction (needed when two
                         classes fit the units, or for a `raw` line)
    .free @n 0xMASK      bits of word n (0 = header) that the variant leaves
                         free and that the listing restores
    .slot @k 0xNNNN      slot word k, which no operand reads (spec 4.4)
    .words 0xNNNN ...    words kept as they are (a truncated tail)

Immediates (spec 4.4): `#v@b:w` is a value read at byte b of the slot array,
w bits wide. A MAC coefficient `#v` is read in order after Y's immediate, or
at the byte `@b` when Y is a register. An immediate with no byte is placed by
the assembler at the first byte where its bits agree with the stream.

Confidence policy is enforced **here**, not by the caller: a line whose
variant is not in `Table.EMIT_DEFAULT` (exact, measured) is rejected unless
`allow_inferred=True` is passed, which admits `inferred` and `fitted`. `raw`
words and `.words` are never emitted by default either: they are unknown
encodings kept verbatim, so re-assembling them takes the same explicit option.

Ambiguity is rejected: if more than one class can hold the units, or a field
that must be preserved has no source, the line fails with the reason.
See asm/REVIEW.md for the 2026-10-20 review that set these rules.
"""

import re

import codec
import isa
import layout
import slots as slots_mod

WORD = 0xFFFF
MAX_LEN = 31


# operand kinds that take an immediate, and kinds that never do (spec 5.2)
IMM_KINDS = frozenset(("y_operand", "coef_imm"))
IMM_ONLY_KINDS = frozenset(("coef_imm",))


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
    if kind in ("y_operand", "coef_imm"):
        if kind == "coef_imm" and not t.startswith("#"):
            raise codec.CodecError("the coefficient of this MAC is an immediate (#value)")
        if t.startswith("#"):
            body = t[1:]
            at = width = None
            m = re.fullmatch(r"(?P<v>[^@:]+)(?:@(?P<b>\d+))?(?::(?P<w>\d+))?", body)
            if m is None:
                raise codec.CodecError("%r is not an immediate (#value, #value@byte:bits)" % text)
            body = m.group("v")
            if m.group("b") is not None:
                at = int(m.group("b"))
            if m.group("w") is not None:
                width = int(m.group("w"))
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
    if kind in ("number", "units", "group"):
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
        m = re.fullmatch(r"[.]([+-]\d+)(?:@(\d+))?", t)
        if not m:
            raise codec.CodecError(
                "%r is not a jump offset: write `.%%+d` words from the instruction "
                "end (spec 5.4b); symbolic labels need a linker" % t)
        return codec.Arg("label", int(m.group(1)), t,
                         at_byte=int(m.group(2)) if m.group(2) else None)
    raise codec.CodecError("operands of kind %r are not supported" % kind)


# ---------------------------------------------------------------------------
# one unit of a line
# ---------------------------------------------------------------------------

class Unit:
    def __init__(self, variant, args, text):
        self.variant, self.args, self.token = variant, args, text
        self.form = variant.form

    def shown_args(self):
        """(name, Arg) for each operand the source wrote, in source order"""
        names = self.variant.shows if self.variant.shows is not None else self.form.operand_names
        out = []
        for name in names:
            i = self.form.operand_names.index(name)
            if self.args[i] is not None:
                out.append((name, self.args[i]))
        return out

    def imm_args(self):
        """(name, Arg) for each immediate operand, in the order the hardware reads
        them (spec 4.4): a MAC's Y, then its coefficient, then W"""
        return [(n, a) for n, a in self.shown_args() if a.is_immediate]

    def _label_arg(self):
        for name in self.form.operand_names:
            if self.form.kinds.get(name) == "label":
                i = self.form.operand_names.index(name)
                if i < len(self.args) and self.args[i] is not None:
                    return self.args[i]
        return None

    def jump_offset(self):
        """a jump's offset in words (§5.4b): it lives in a slot word"""
        arg = self._label_arg()
        return None if arg is None else int(arg.value)

    def jump_slot(self):
        """the slot a jump names (`jmp .+3@2`), or None"""
        arg = self._label_arg()
        return None if arg is None else arg.at_byte

    def slot_refs(self):
        """the reference words this unit addresses: [(value, slot or None)]

        `slot` is the `@k` the source names (the slot the reference occupies),
        None when the layout pass may choose (§4.3)."""
        if not self.variant.uses_slot:
            return []
        out = []
        for name in self.form.operand_names:
            if self.form.kinds.get(name) != "ref":
                continue
            i = self.form.operand_names.index(name)
            if i < len(self.args) and self.args[i] is not None:
                arg = self.args[i]
                out.append((int(arg.value), arg.at_byte))
        return out


# ---------------------------------------------------------------------------
# the assembler
# ---------------------------------------------------------------------------

class Assembler:
    def __init__(self, table=None, allow_inferred=False):
        self.table = table or isa.Table.load()
        self.allow_inferred = allow_inferred
        self.notes = []

    # -- public ------------------------------------------------------------
    def assemble(self, text):
        result = Result()
        self.notes = []
        lines = text.splitlines()
        pending_class = None
        pending_free = []
        pending_slots = []
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
                # not checked here: a `raw` line may name a class the table does not
                # know (its words are verbatim); an instruction is checked when built
                continue
            # `.words 0xNNNN ...`: words kept as they are (a truncated tail of a
            # dump, spec 3): no instruction structure is claimed for them
            m = re.match(r"\s*\.words\s+(?P<w>.+?)\s*$", line)
            if m:
                if not self.allow_inferred:
                    result.errors.append((n, ".words carries no known encoding; "
                                            "re-assembling it needs --allow-inferred"))
                    result.rejected += 1
                    continue
                try:
                    result.words.extend(int(x, 16) & WORD for x in m.group("w").split())
                except ValueError:
                    result.errors.append((n, "not a word list: %r" % m.group("w")))
                    result.rejected += 1
                continue
            m = re.match(r"\s*\.slot\s+@(?P<k>\d+)\s+(?P<v>0x[0-9a-fA-F]+|[0-9a-fA-F]{1,4})\s*$", line)
            if m:
                pending_slots.append((int(m.group("k")), int(m.group("v"), 16) & WORD))
                continue
            # `.free @n 0xMASK`: the bits of word n that its variant leaves free
            # (word 0 is the header); a bare mask applies to every free word
            m = re.match(r"\s*\.free\s+(?:@(?P<n>\d+)\s+)?(?P<v>0x[0-9a-fA-F]+|[0-9]+)\s*$", line)
            if m:
                try:
                    mask = int(m.group("v"), 0) & WORD
                except ValueError:
                    result.errors.append((n, "not a bit mask: %r" % m.group("v")))
                    result.rejected += 1
                    continue
                pending_free.append((int(m.group("n")) if m.group("n") else None, mask))
                continue
            # the legacy hint in a trailing comment: `; @0x0040 (exact) 0803 ...`
            hint = re.search(r";\s*@(0x[0-9a-fA-F]+)", raw)
            pint = pending_class
            if hint and pint is None:
                pint = int(hint.group(1), 16)
            pending_class = None
            try:
                words = self.line(n, line, pint, pending_free, pending_slots)
                pending_free = []
                pending_slots = []
            except AsmError as e:
                result.errors.append((e.line or n, e.msg))
                result.rejected += 1
                pending_free = []
                pending_slots = []
                continue
            except codec.CodecError as e:
                result.errors.append((n, str(e)))
                result.rejected += 1
                continue
            if words is None:             # a note, no words
                continue
            result.words.extend(words)
            result.assembled += 1
        result.notes = list(self.notes)
        return result

    # -- one line ----------------------------------------------------------
    def line(self, lineno, text, pinned_class=None, free=(), fixed_slots=()):
        m = re.match(r"^\s*(?:([A-Za-z_.][\w.]*)\s*:)?\s*(.*)$", text)
        body = m.group(2).strip()
        if not body:
            return None
        tokens = split_units(body)
        if tokens and tokens[0].split()[0] == "raw":
            return self._raw(lineno, body, pinned_class)
        units = [self._unit(lineno, t) for t in tokens]
        return self._instruction(lineno, units, pinned_class, free=free,
                                 fixed_slots=fixed_slots)

    def _unit(self, lineno, token):
        mnemonic, raw_ops = tokenize(token)
        if mnemonic not in self.table.variants_by_mnemonic:
            near = [m for m in sorted(self.table.mnemonics) if m.startswith(mnemonic[:3])][:3]
            hint = (" (similar: %s)" % ", ".join(near)) if near else ""
            raise AsmError(lineno, "unknown mnemonic %r%s" % (mnemonic, hint), token)
        variant = self._variant_for(lineno, mnemonic, raw_ops, token)
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

    def _variant_for(self, lineno, mnemonic, raw_ops, token):
        """the variant of `mnemonic` that the operands the source wrote describe.

        A mnemonic can name several variants (`ld` with its modes, `hset` with
        its register, a MAC with a register or an immediate coefficient). An
        immediate operand needs a variant whose operand takes one, a register
        one that takes a register, and `hset hN` the variant of register hN.
        The first variant that agrees is taken; the line is checked against the
        words the variant gives, so a wrong guess cannot reach the output.
        """
        for v in self.table.variants_by_mnemonic[mnemonic]:
            shown = v.shows if v.shows is not None else v.form.operand_names
            if len(shown) != len(raw_ops):
                continue
            if v.hidden_reg is not None and raw_ops and \
                    raw_ops[0].strip().lower() != "h%d" % v.hidden_reg:
                continue
            ok = True
            for name, given in zip(shown, raw_ops):
                kind = v.kinds.get(name) or v.form.kinds.get(name)
                if given.strip().startswith("#"):
                    ok = kind in IMM_KINDS
                else:
                    ok = kind not in IMM_ONLY_KINDS
                if not ok:
                    break
            if ok:
                return v
        # no variant agrees: report the operand count of the first one
        first = self.table.variants_by_mnemonic[mnemonic][0]
        shown = first.shows if first.shows is not None else first.form.operand_names
        if len(shown) != len(raw_ops):
            raise AsmError(lineno, "%s takes %d operand(s) (%s), %d given"
                           % (mnemonic, len(shown), ", ".join(shown), len(raw_ops)), token)
        raise AsmError(lineno, "%s: no variant takes these operands" % mnemonic, token)

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
        if not ops and cls != 0:
            # class 0x000 is the header-only one-word no-op (spec 4.1)
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
        for cls, c in sorted(self.table.classes.items()):
            letters = layout.letters_for(self.table, cls, len(body))
            if letters is None or len(letters) != len(body):
                continue
            if layout.fits_exact(self.table, letters, body, layout.prefix_length(c.layout)):
                hits.append(cls)
        return hits[0] if len(hits) == 1 else None

    # -- a normal instruction ---------------------------------------------
    def _instruction(self, lineno, units, pinned_class=None, free=(), fixed_slots=()):
        """the header-less body of one instruction (spec 4.1-4.4).

        The class is one whose unit prefix the units fill exactly. For each such
        class the slot array is made as small as the line allows: the smallest
        size at which every reference, jump and immediate bit fits (`_build`).
        Two classes that both fit are an error unless the source names one.
        """
        if pinned_class is not None and pinned_class not in self.table.classes:
            raise AsmError(lineno, "class %#05x is not in the table" % pinned_class)
        ordered = sorted(units, key=lambda u: self.table.unit_rank.get(
            _letter(self.table, u.form), 0))
        for u in ordered:
            self._size_immediates(lineno, u)
        letters = [_letter(self.table, u.form) for u in ordered]
        n_mac = sum(1 for u in ordered if u.form.form == "mac")
        choices, failure = [], None      # failure: the error of the largest slot array tried
        for cls, c in sorted(self.table.classes.items()):
            if pinned_class is not None and cls != pinned_class:
                continue
            prefix = layout.prefix_length(c.layout)
            # the units fill the class's prefix exactly, and its slot array has a
            # N position for every MAC (spec 4.2)
            if list(c.layout[:prefix]) != letters or c.layout[prefix:].count("N") < n_mac:
                continue
            for extra in self._extra_sizes(cls, prefix, c):
                try:
                    body = self._build(lineno, cls, prefix, ordered, extra, free, fixed_slots)
                except (AsmError, slots_mod.SlotError) as e:
                    if failure is None or extra >= failure[0]:
                        failure = (extra, e)
                    continue
                choices.append((cls, body))
                break
        if not choices:
            if failure is not None:
                e = failure[1]
                raise AsmError(lineno, getattr(e, "msg", None) or str(e))
            raise AsmError(lineno, "no class in the table holds these units "
                                   "(%s)" % "".join(letters))
        if len(choices) > 1:
            raise AsmError(lineno, "ambiguous: classes %s all hold these units; "
                                   "name one with `.class` or in the trailing "
                                   "comment (`; @0x%04x`)"
                           % (", ".join("%#05x" % c for c, _ in choices), choices[0][0]))
        cls, body = choices[0]
        return [isa.header_of(cls, len(body) + 1)] + body

    def _extra_sizes(self, cls, prefix, c):
        """slot-array sizes to try for one class, smallest first"""
        return [extra for extra in range(0, MAX_LEN - prefix)
                if c.allows(prefix + extra)]

    def _size_immediates(self, lineno, u):
        """the width of every immediate operand (spec 4.4)"""
        for name, arg in u.imm_args():
            arg.bits = self._width(lineno, arg, u.variant, name)

    def _width(self, lineno, arg, variant, name):
        """an immediate's width: the one the source states (`#v@b:32`), else the
        variant's own (a coefficient's, or `immediate_bits`), else the narrowest
        that holds the value exactly.

        A hidden-register value is an integer word (spec 9.0), not a RAW number,
        so its narrowest width is the one that holds that integer."""
        value = arg.value
        fixed = arg.width or (variant.coef_bits if name == "C" else None) or variant.immediate_bits
        if variant.form.form == "hidden_reg":
            if isinstance(value, _RawImm):
                raise AsmError(lineno, "a hidden-register value is written as a number")
            n = _hidden_int(value)
            if n is None:
                raise AsmError(lineno, "%r is not a multiple of 1/16384 (spec 9.0)" % (value,))
            if fixed:
                if n >> fixed:
                    raise AsmError(lineno, "%s: %r needs more than %d bits" % (variant.mnemonic, value, fixed))
                return fixed
            return 16 if n < 1 << 16 else 24 if n < 1 << 24 else 32
        if isinstance(value, _RawImm):
            need = len(value.bits) * 4
            if fixed:
                if need > fixed:
                    raise AsmError(lineno, "raw.%s does not fit in %d bits" % (value.bits, fixed))
                return fixed
            return 16 if need <= 16 else 24 if need <= 24 else 32
        try:
            raw = self.table.raw_encode(value)
        except ValueError as e:
            raise AsmError(lineno, str(e))
        if fixed:
            if raw & ((1 << (32 - fixed)) - 1):
                # the hardware keeps the top bits: say what is stored when that is
                # a real loss (a decimal such as 0.2 is exact to its own digits)
                kept = self.table.raw_decode(raw & ~((1 << (32 - fixed)) - 1))
                if abs(kept - value) > 1e-6 * max(abs(value), 1e-6):
                    self.notes.append((lineno, "%s: %r is stored in %d bits as %r"
                                       % (variant.mnemonic, value, fixed, kept)))
            return fixed
        if not raw & 0xFFFF:
            return 16
        if not raw & 0xFF:
            return 24
        return 32

    def _spots(self, letters, prefix, ordered):
        """body position(s) of each unit (id -> [position]): a MAC has an M
        position in the prefix and an N word in the slot array; every other
        unit one position of its letter"""
        free = {}
        for i, ch in enumerate(letters[:prefix]):
            free.setdefault(ch, []).append(i)
        for lst in free.values():
            lst.reverse()                 # pop() takes the first position
        n_free = [i for i in range(prefix, len(letters)) if letters[i] == "N"]
        spots = {}
        for u in ordered:
            if u.form.form == "mac":
                if not free.get("M") or not n_free:
                    raise AsmError(0, "class has no M/N position left for %s" % u.variant.mnemonic)
                spots[id(u)] = [free["M"].pop(), n_free.pop(0)]
            else:
                letter = _letter(self.table, u.form)
                if not free.get(letter):
                    raise AsmError(0, "class has no %s position left for %s"
                                   % (letter, u.variant.mnemonic))
                spots[id(u)] = [free[letter].pop()]
        return spots

    def _build(self, lineno, cls, prefix, ordered, extra, free=(), fixed_slots=()):
        """the body of one instruction in one class with `extra` slot words.

        Everything the line fixes is written first (unit words, the N words,
        the transfers' and jumps' slot fields); then the immediates are laid
        over the slot array in the hardware's order (spec 4.4). A bit that two
        things claim with different values, or a word that a reference or a
        jump already holds, is an error, never a silent choice.
        """
        letters = layout.letters_for(self.table, cls, prefix + extra)
        if letters is None:
            raise AsmError(lineno, "class %#05x cannot hold %d body words" % (cls, prefix + extra))
        body = [0] * len(letters)
        spots = self._spots(letters, prefix, ordered)

        # ---- the units' own words --------------------------------------
        for u in ordered:
            words = codec.encode(u.form, u.variant, u.args)
            for pos, w in zip(spots[id(u)], words):
                body[pos] = w
        # `.free`: the bits the variants leave free, restored from the source
        free_at = {}
        for pos, mask in self._free_spots(ordered, spots):
            free_at[pos] = free_at.get(pos, 0) | mask
        for n, mask in free:
            if n is None:                        # a bare mask: every free word
                stray = mask & ~sum(free_at.values())
                if stray:
                    raise AsmError(lineno, "0x%04x is not a free bit of this instruction" % stray)
                for pos, allowed in free_at.items():
                    body[pos] |= mask & allowed
                continue
            pos = n - 1
            stray = mask & ~free_at.get(pos, 0)
            if stray or pos < 0 or pos >= len(body):
                raise AsmError(lineno, "0x%04x is not a free bit of word %d of this instruction"
                               % (stray or mask, n))
            body[pos] |= mask

        # ---- the slot array (§4.3) -------------------------------------
        positions = layout.array_positions(letters, prefix)
        size = len(positions)
        n_words = layout.n_words(letters, prefix)
        if any(letters[p] != "N" for p in positions[:n_words]):
            raise AsmError(lineno, "class %#05x: the N words are not first in the slot array" % cls)
        arr = slots_mod.SlotArray(positions[0] if positions else len(body), n_words, size)
        arr.take_n([body[p] for p in positions[:n_words]])
        for k, value in fixed_slots:      # `.slot`: words stated outright
            arr.claim(k, slots_mod.Occupant(slots_mod.Occupant.FIXED, value))

        reads = self._reads(ordered, spots, body)
        cover = set()                     # slots an explicit immediate reads
        for r in reads:
            if r.at is not None:
                for bit in range(r.at * 8, r.at * 8 + r.bits):
                    cover.add(bit // 16)

        # references: the slot each transfer names, or the next free one
        for u in ordered:
            for value, slot in u.slot_refs():
                k = slot if slot is not None else self._next_free(arr, cover)
                arr.claim_reference(k, value, u.variant.mnemonic)
                self._write_slot(body, u, spots, k, "slot")
        # jumps: one slot word each, after the references (spec 5.4b)
        for u in ordered:
            off = u.jump_offset()
            if off is None:
                continue
            slot = u.jump_slot()
            k = slot if slot is not None else self._next_free(arr, cover)
            arr.claim(k, slots_mod.Occupant(slots_mod.Occupant.JUMP, off & WORD))
            self._write_slot(body, u, spots, k, "offset_slot")

        # ---- the immediate bits (spec 4.4) -----------------------------
        bitmap = {}
        refs = {i for i, occ in enumerate(arr.slots)
                if occ is not None and occ.kind == slots_mod.Occupant.REF}
        cursor = 0
        for r in reads:
            if r.search:
                # no byte was named: the first byte whose bits agree with what
                # is already there (a word the slot array fixes counts too)
                r.at = self._first_fit(lineno, r, arr, bitmap, size)
                body[r.pos] = isa.put_bits(body[r.pos], r.spec, r.at)
            pos, cursor = layout.stream_bits(r.bits, cursor, refs,
                                             None if r.at is None else r.at * 8)
            for j, bit in zip(pos, r.bitstring):
                if j // 16 >= size:
                    raise slots_mod.SlotError("an immediate runs past the slot array (%d words)"
                                              % size)
                occ = arr.slots[j // 16]
                want = bit == "1"
                if occ is not None:
                    have = (occ.value >> (15 - j % 16)) & 1
                    if bool(have) != want:
                        raise AsmError(lineno, "%s: its immediate bit %d disagrees with slot %d (%s)"
                                       % (r.mnemonic, j, j // 16, occ.kind))
                    continue
                if bitmap.get(j, want) != want:
                    raise AsmError(lineno, "%s: two immediates disagree at bit %d of the stream"
                                   % (r.mnemonic, j))
                bitmap[j] = want

        # ---- write the array ---------------------------------------------
        for k, pos in enumerate(positions):
            occ = arr.slots[k]
            if occ is not None:
                body[pos] = occ.value & WORD
                continue
            word = 0
            for j in range(16):
                if bitmap.get(k * 16 + j):
                    word |= 1 << (15 - j)
            body[pos] = word
        self._check_len(lineno, body)
        return body

    def _reads(self, ordered, spots, body):
        """every immediate of the line, in the order the hardware reads them"""
        reads = []
        for u in sorted(ordered, key=lambda u: spots[id(u)][0]):
            for name, arg in u.imm_args():
                bits = _imm_bitstring(self.table, arg.value, arg.bits,
                                      hidden=u.form.form == "hidden_reg")
                at, field = self._read_byte(u, name, arg, spots, body)
                search = at is None and field is not None
                pos = spec = None
                if field is not None:
                    pos = spots[id(u)][field[0]]
                    spec = u.form.field_spec(field[1])
                reads.append(_Read(u.variant.mnemonic, at, bits, search, pos, spec))
        return reads

    def _read_byte(self, u, name, arg, spots, body):
        """-> (byte, field): the byte an immediate is read at, and the field that
        holds that byte.

        byte None with a field: no byte was named, the layout pass chooses one.
        byte None without a field: read in order after the previous immediate
        (a MAC coefficient after Y's, spec 4.4); hidden-register loads too.
        """
        form = u.form
        if form.form == "hidden_reg":
            # spec 9.0: Y names the byte of the stream the value is read at, as
            # for an ALU op (`1d06 4022 7fe0 c1fc 5100 0000`: Y = 2 reads 5100)
            return (arg.at_byte if arg.at_byte is not None else 0), None
        if form.form == "mac":
            words = [body[spots[id(u)][0]], body[spots[id(u)][1]]]
            if name == "C":
                y_is_imm = any(n == "Y" for n, _ in u.imm_args())
                if y_is_imm and arg.at_byte is None:
                    return None, None
                byte = arg.at_byte if arg.at_byte is not None else codec._bits_of(form, words, "C")
                return byte, None
            word = 1 if name == "W" else 0
            field = (word, name)
            if arg.at_byte is not None:
                return arg.at_byte, field
            return None, field
        field = (0, "Y")
        if arg.at_byte is not None:
            return arg.at_byte, field
        return None, field

    @staticmethod
    def _first_fit(lineno, r, arr, bitmap, size):
        """the first byte at which an immediate's bits agree with the stream"""
        for at in range(16):
            ok = True
            for j, bit in zip(range(at * 8, at * 8 + r.bits), r.bitstring):
                if j // 16 >= size:
                    ok = False
                    break
                want = bit == "1"
                occ = arr.slots[j // 16]
                if occ is not None:
                    if bool((occ.value >> (15 - j % 16)) & 1) != want:
                        ok = False
                        break
                elif bitmap.get(j, want) != want:
                    ok = False
                    break
            if ok:
                return at
        raise AsmError(lineno, "%s: no byte of the slot array holds its immediate" % r.mnemonic)

    @staticmethod
    def _next_free(arr, cover):
        """the first slot nobody owns and no explicit immediate reads"""
        for i, occ in enumerate(arr.slots):
            if occ is None and i not in cover:
                return i
        raise slots_mod.SlotError("the slot array is full (%d words)" % arr.size)

    def _write_slot(self, body, u, spots, k, field_kind):
        """write slot index k into the field of unit u that names it (`Y >> 1`)"""
        pos = spots[id(u)][0]
        for fname, src in u.variant.fields.items():
            if isinstance(src, dict) and src.get(field_kind):
                spec = u.form.field_spec(fname)
                shift = spec.get("slot_shift", 1)
                body[pos] = isa.put_bits(body[pos], spec, k << shift)
                return

    def _free_spots(self, ordered, spots):
        """(body position, bit mask) of every bit the variants leave free"""
        out = []
        for u in ordered:
            imm = dict(u.imm_args())
            y_is_imm = "Y" in imm
            for fname, src in u.variant.fields.items():
                if not (isinstance(src, dict) and src.get("free")):
                    continue
                if fname == "C" and u.variant.coef_bits:
                    # free only when read in order after Y's immediate; a
                    # coefficient read at a byte keeps that byte in C
                    if not y_is_imm or imm["C"].at_byte is not None:
                        continue
                spec = u.form.field_spec(fname)
                width = spec["msb"] - spec["lsb"] + 1
                mask = ((1 << width) - 1) << spec["lsb"]
                idx = min(spec.get("_word", 0), len(spots[id(u)]) - 1)
                out.append((spots[id(u)][idx], mask))
        return out

    def _check_len(self, lineno, body):
        if len(body) + 1 > MAX_LEN:
            raise AsmError(lineno, "instruction is %d words; the header allows %d"
                           % (len(body) + 1, MAX_LEN))


class _Read:
    """one immediate read of the line: where it comes from and which bits it is

    at      the byte it is read at, when the source (or the disassembler) names it
    search  True when the source names no byte and the layout pass may choose
            one: the byte is then written into the operand's Y/W field at `pos`
    """

    def __init__(self, mnemonic, at, bitstring, search=False, pos=None, spec=None):
        self.mnemonic = mnemonic
        self.at = at
        self.bitstring = bitstring
        self.bits = len(bitstring)
        self.search = search
        self.pos, self.spec = pos, spec      # body word and field spec of that byte


def _imm_bitstring(table, value, bits, hidden=False):
    """an immediate as a `bits`-long bit string, most significant first.

    A RAW immediate (spec 4.4) is the top bits of its RAW word. A hidden-register
    value (spec 9.0) is the integer word itself, value * 16384."""
    if isinstance(value, _RawImm):
        hexs = (value.bits + "0" * 8)[:bits // 4]
        return "".join(format(int(c, 16), "04b") for c in hexs)
    if hidden:
        return format(_hidden_int(value), "0%db" % bits)
    raw = table.raw_encode(value) & 0xFFFFFFFF
    return format(raw, "032b")[:bits]


def _hidden_int(value):
    """a hidden-register value as its integer word, or None when it has no such
    word (spec 9.0: values are integers = value * 16384)"""
    n = value * 16384
    if n != int(n) or n < 0:
        return None
    return int(n)


def _letter(table, form):
    for letter, names in table.units.items():
        if form.name in names:
            return letter
    raise AsmError(0, "the table does not place %r" % form.name)


def assemble(text, table=None, allow_inferred=False):
    """convenience: source text -> Result"""
    return Assembler(table, allow_inferred).assemble(text)
