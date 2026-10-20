"""The ISA table as objects. Pure interpreter: nothing here knows a mnemonic.

    table = isa.Table.load()          # reads isa.json next to this file
    table.classes[0x048]              # -> Class(layout='A', status='measured', ...)
    table.mnemonics['mul']            # -> Variant(...)
    table.constraints                 # per-form rules the codec applies

Everything the codec needs comes from the JSON: field positions, variant
selectors, fixed bits, operand wiring and status tags.
"""

import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT = os.path.join(HERE, "isa.json")

STATUSES = ("exact", "measured", "inferred", "fitted", "unknown", "raw")

# Confidence policy (what the assembler may emit by default). The spec defines
# [exact] as reproduced with random operands and zero error, [measured] as
# limited but consistent measurements. Anything weaker needs an explicit opt-in.
EMIT_DEFAULT = ("exact", "measured")
EMIT_OPT_IN = ("inferred", "fitted")
EMIT_NEVER = ("unknown", "raw")

# a variant is decodable if its status is in this set (the disassembler decodes
# everything it can name, but tags what it found)
DECODABLE = ("exact", "measured", "inferred", "fitted")


class TableError(Exception):
    """isa.json is inconsistent"""


# ---------------------------------------------------------------------------
# bit fields
# ---------------------------------------------------------------------------

def header_of(cls, length):
    """the header word of §4.1: class << 5 | length"""
    return (cls << 5) | (length & 0x1F)


def get_bits(word, spec):
    msb, lsb = spec["msb"], spec["lsb"]
    return (word >> lsb) & ((1 << (msb - lsb + 1)) - 1)


def put_bits(word, spec, value):
    msb, lsb = spec["msb"], spec["lsb"]
    width = msb - lsb + 1
    if not 0 <= value < (1 << width):
        raise ValueError("%d does not fit in bits %d..%d" % (value, msb, lsb))
    mask = ((1 << width) - 1) << lsb
    return (word & ~mask & 0xFFFF) | (value << lsb)


# ---------------------------------------------------------------------------
# table objects
# ---------------------------------------------------------------------------

class ClassDef:
    """one template class of spec §12"""

    def __init__(self, cls, entry):
        self.cls = cls
        self.layout = entry["layout"]
        self.status = entry.get("status", "unknown")
        # the instruction sizes measured for this class (header excluded); used
        # to tell apart classes whose layouts give the same unit prefix
        self.body_lengths = entry.get("body_lengths")

    def allows(self, body_len):
        return self.body_lengths is None or body_len in self.body_lengths

    def __repr__(self):
        return "Class(%#05x, %r)" % (self.cls, self.layout)


class Variant:
    """one mnemonic of one instruction family"""

    def __init__(self, form, entry, index=0):
        self.form = form
        self.index = index
        self.mnemonic = entry["mnemonic"]
        self.status = entry.get("status", form.status)
        self.when = entry.get("when", {})
        self.fields = entry.get("fields", {})
        self.result = entry.get("result", "")
        self.uses_slot = entry.get("uses_slot", False)
        self.special_d = entry.get("special_d", False)
        self.hidden_reg = entry.get("hidden_reg")
        self.shows = entry.get("shows")
        # per-variant operand kinds, overriding the form's (a variant may take
        # an immediate where the form only documents a register, or the reverse)
        self.kinds = dict(entry.get("kinds", {}))
        # the immediate width this variant takes (spec 4.4); None = narrowest
        self.immediate_bits = entry.get("immediate_bits")
        # a MAC coefficient immediate (op2 mode 1/2/3 = imm16/24/32, spec 5.2):
        # the width is fixed by the variant, the coefficient is read after Y's
        # immediate (or at the byte its C field names, when Y is a register)
        self.coef_bits = entry.get("coef_bits")
        # True when bits 5-4 are a function selector here, not an operand mode
        self.mode_is_selector = entry.get("mode_is_selector", False)
        # where the claim comes from: a spec section, a measurement, both
        self.evidence = entry.get("evidence", [])
        self.evidence_note = entry.get("evidence_note")

    @property
    def decodable(self):
        return self.status in DECODABLE

    def __repr__(self):
        return "Variant(%s.%s %s)" % (self.form.name, self.mnemonic, self.status)


class Form:
    """one entry of "instructions": the fields it uses, its operands, variants"""

    def __init__(self, table, entry):
        self.table = table
        self.entry = entry
        self.name = entry["name"]
        self.form = entry["form"]
        self.status = entry.get("status", "unknown")
        self.words = entry.get("words", 1)
        # the whole form reuses bits 5-4 as a selector (nibble-E/F families)
        self.mode_is_selector = entry.get("mode_is_selector", False)
        self.operands = entry.get("operands", [])
        self.operand_names = [o["name"] for o in self.operands]
        self.kinds = {o["name"]: o["kind"] for o in self.operands}
        self.immediate_operands = [o["name"] for o in self.operands if o.get("immediate")]
        self.layout_names = set(table.word.get(self.form, {}) or {})
        if self.form == "hidden_reg":
            # spec 9.0 names the pattern 40TY: bits 15-12 are fixed at 4
            table.word["hidden_reg"]["tag"] = {"msb": 15, "lsb": 12}
            self.layout_names.add("tag")
        self.local_fields = {k: v for k, v in entry.get("local_fields", {}).items()
                             if not k.startswith("$")}
        self.fields = table.field_map(self.form, [
            f for v in entry.get("variants", []) for f in v.get("fields", {})] +
            [f for v in entry.get("variants", []) for f in v.get("when", {})])
        self.fields.update(self.local_fields)
        self.variants = [Variant(self, v, i) for i, v in enumerate(entry.get("variants", []))]
        self.by_mnemonic = {}
        for v in self.variants:
            self.by_mnemonic.setdefault(v.mnemonic, []).append(v)
        self.mnemonics = [v.mnemonic for v in self.variants]

    # -- bit access --------------------------------------------------------
    def field_spec(self, name):
        if name in self.fields:
            return self.fields[name]
        raise TableError("%s: no field %r in layout %r"
                         % (self.name, name, self.entry.get("layout")))

    def get(self, word, name):
        """field value from one word (use codec._bits_of for a MAC pair)"""
        return get_bits(word, self.field_spec(name))

    def put(self, word, name, value):
        return put_bits(word, self.field_spec(name), value)

    def __repr__(self):
        return "Form(%s, %s)" % (self.name, self.form)


class Table:
    def __init__(self, data, path=None):
        self.data, self.path = data, path
        self.word = data["word"]
        self.immediate_rules = data["immediate"]
        self.number = data["number_format"]
        self.operand_kinds = data["operand_kinds"]
        self.classes = {int(k, 16): ClassDef(int(k, 16), v)
                        for k, v in data["classes"].items() if not k.startswith("$")}
        self.forms = [Form(self, e) for e in data["instructions"]]
        self.mnemonics = {}             # the first variant of each mnemonic
        self.variants_by_mnemonic = {}  # all of them, in table order
        self.forms_by_mnemonic = {}
        for f in self.forms:
            for v in f.variants:
                self.mnemonics.setdefault(v.mnemonic, v)
                self.variants_by_mnemonic.setdefault(v.mnemonic, []).append(v)
                self.forms_by_mnemonic.setdefault(v.mnemonic, f)
        self.units = {k: v for k, v in data.get("units", {}).items() if not k.startswith("$")}
        # the canonical order of positions (spec 4.2); kept apart from `units`
        self.unit_rank = data.get("units", {}).get("$rank",
                                                   {"A": 0, "B": 0, "M": 0, "C": 1, "T": 2})
        self.forms_by_name = {f.name: f for f in self.forms}
        self._validate()

    # -- loading -----------------------------------------------------------
    @classmethod
    def load(cls, path=None):
        path = path or DEFAULT
        with open(path, "r", encoding="utf-8") as fh:
            try:
                data = json.load(fh)
            except ValueError as e:
                raise TableError("%s: not valid JSON: %s" % (path, e))
        return cls(data, path)

    # -- helpers -----------------------------------------------------------
    def field_map(self, form_name, used=()):
        """the fields a form's words have.

        The layout block of the family, restricted to the fields the form's
        variants actually name (a unit-A word has no nibble field, a unit-B
        word has no `op`), plus the per-entry local fields.
        """
        w = self.word
        if form_name == "alu":
            base = dict(w["alu"])
            synth = {k: v for k, v in w.get("synthetic", {}).items() if not k.startswith("$")}
            base.update(synth)
            keep = set(used)
            out = {}
            for k, v in base.items():
                if k in keep or k in ("D", "X", "mode", "Y"):
                    out[k] = v
            return out
        if form_name == "mac":
            return dict(w["mac"])
        return {k: v for k, v in w.get(form_name, {}).items() if not k.startswith("$")}

    def emittable(self, variant, allow_inferred=False):
        """may the assembler emit this variant? (confidence policy)"""
        if variant.status in EMIT_DEFAULT:
            return True
        if variant.status in EMIT_OPT_IN:
            return allow_inferred
        return False

    def variant_for(self, mnemonic):
        v = self.mnemonics.get(mnemonic)
        if v is None:
            raise KeyError("no mnemonic %r in the table" % mnemonic)
        return v

    def form_for(self, mnemonic):
        v = self.variant_for(mnemonic)
        return v.form

    # -- checks ------------------------------------------------------------
    def _validate(self):
        problems = []
        seen = {}
        for f in self.forms:
            for v in f.variants:
                if v.mnemonic in seen and seen[v.mnemonic].form is not f:
                    problems.append("mnemonic %r appears in two forms (%s, %s)"
                                    % (v.mnemonic, seen[v.mnemonic].form.name, f.name))
                seen.setdefault(v.mnemonic, v)
                if v.status not in STATUSES:
                    problems.append("%s.%s: unknown status %r" % (f.name, v.mnemonic, v.status))
                for field, src in v.fields.items():
                    if field not in f.fields:
                        problems.append("%s.%s: writes field %r that the layout has not"
                                        % (f.name, v.mnemonic, field))
                    if isinstance(src, str) and src not in f.operand_names:
                        problems.append("%s.%s: field %r takes %r, which is not an operand"
                                        % (f.name, v.mnemonic, field, src))
                for field in v.when:
                    if field not in f.fields:
                        problems.append("%s.%s: 'when' tests field %r that the layout has not"
                                        % (f.name, v.mnemonic, field))
        for cls, c in self.classes.items():
            bad = set(c.layout) - set("ABMNCTRI")
            if bad:
                problems.append("class %#05x: bad layout letters %s" % (cls, sorted(bad)))
            if c.status not in STATUSES:
                problems.append("class %#05x: unknown status %r" % (cls, c.status))
        if problems:
            raise TableError("%s:\n  %s" % (self.path or "isa.json", "\n  ".join(problems)))

    # -- number format (spec 2) -------------------------------------------
    def raw_encode(self, value):
        n = self.number
        max_mag = n.get("max_magnitude", 512)
        if value != value:
            raise ValueError("NaN is not a RAW value")
        if abs(value) >= max_mag:
            raise ValueError("%r out of range (|v| must be < %g)" % (value, max_mag))
        if value == 0:
            return 0
        neg = value < 0
        m = abs(value)
        for s in n.get("exponents", [0, 1, 2, 3]):
            mant = int(round(m * 2 ** (32 - 4 * s)))
            if mant < 2 ** 29:
                return ((1 << n.get("sign_bit", 31)) if neg else 0) | (s << 29) | mant
        raise ValueError("%r out of range" % value)

    def raw_decode(self, w):
        s = (w >> 29) & 3
        v = (w & 0x1FFFFFFF) / 2.0 ** (32 - 4 * s)
        return -v if w >> 31 else v

    def immediate_width(self, mode):
        return self.immediate_rules["modes"].get(str(mode), 0)

