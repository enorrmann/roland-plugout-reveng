"""Write the generated parts of the ISA chapter from asm/isa.json (isa.json is the reference; the chapter follows it).

    python3 asm/isadoc.py            # rewrite the generated blocks of docs/vol2_dsp_core_isa/instruction_set_reference.md
    python3 asm/isadoc.py --stdout   # print the blocks instead

Two blocks, each between `<!-- isadoc:NAME -->` and `<!-- /isadoc:NAME -->` in the chapter:

* `classes` (§12): every class of isa.json with its layout and status, plus what the local corpus of Roland
  programs (work/corpus_all, work/corpus_s1) shows for it: lengths, uses, most frequent instruction. Without the
  corpus the corpus columns are left empty. Corpus classes missing from isa.json are listed under the table.
* `mnemonics` (§15): every variant of every form, in isa.json order, with the field values that select it, its
  result and status, followed by the forms' `$doc` texts.

Run it after every change to isa.json. The text outside the blocks is not touched. Nothing in asm/ reads the chapter;
this tool only writes it.
"""
import collections
import glob
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)
import isa  # noqa: E402

DOC = os.path.join(ROOT, "docs", "vol2_dsp_core_isa", "instruction_set_reference.md")
CORPORA = ("corpus_all", "corpus_s1")
HEX_FIELDS = ("mode",)


def corpus_instances():
    """class -> list of instructions (tuples of words, header included); {} when the corpus is not here"""
    out = collections.defaultdict(list)
    for d in CORPORA:
        for path in sorted(glob.glob(os.path.join(ROOT, "work", d, "*.txt"))):
            words = [int(t, 16) for t in open(path).read().split()]
            i = 0
            while i < len(words) and words[i] & 0x1F:
                n = words[i] & 0x1F
                out[words[i] >> 5].append(tuple(words[i:i + n]))
                i += n
    return out


def classes_block(table):
    corpus = corpus_instances()
    rows = []
    for cls, c in table.classes.items():
        insts = corpus.get(cls, [])
        lengths = sorted({len(w) for w in insts})
        common_len = collections.Counter(len(w) for w in insts).most_common(1)
        n = common_len[0][0] if common_len else len(c.layout) + 1
        top = collections.Counter(insts).most_common(1)
        rows.append((-len(insts), cls, "| `%#05x` | `%04x` | `%s` + I… | %s | %s | %s | %s |" % (
            cls, isa.header_of(cls, n), c.layout or "(none)", c.status,
            ", ".join(map(str, lengths)), len(insts) if corpus else "",
            "`%s`" % " ".join("%04x" % w for w in top[0][0]) if top else "")))
    out = ["| class | header (most frequent length) | layout (isa.json) | status | lengths in corpus | uses "
           "| most frequent instruction |", "|---|---|---|---|---|---|---|"]
    out += [r[2] for r in sorted(rows)]
    out.append("")
    out.append("%d classes in isa.json%s." % (len(table.classes), "" if corpus else
               "; corpus not present (work/corpus_all, work/corpus_s1), corpus columns empty"))
    missing = sorted(set(corpus) - set(table.classes) - {0})
    if missing:
        out.append("Classes of the corpus that isa.json does not have (their instructions stay `raw`): "
                   + ", ".join("`%#05x` (%d)" % (c, len(corpus[c])) for c in missing) + ".")
    return out


def _value(name, v):
    if isinstance(v, int):
        return ("%#x" % v) if name in HEX_FIELDS else str(v)
    return str(v)


def selected_by(form, v):
    parts = []
    when = dict(v.when)
    tag = form.entry.get("when_tag")
    if tag is not None and "tag" not in when:
        when["tag"] = tag
    for k, x in when.items():
        if isinstance(x, dict):
            for op, y in x.items():
                if op == "lt":
                    parts.append("%s<%s" % (k, _value(k, y)))
                elif op == "ge":
                    parts.append("%s≥%s" % (k, _value(k, y)))
                elif op == "in":
                    parts.append("%s∈{%s}" % (k, ",".join(_value(k, z) for z in y)))
                else:
                    parts.append("%s %s %s" % (k, op, y))
        else:
            parts.append("%s=%s" % (k, _value(k, x)))
    return ", ".join(parts)


def cell(text):
    return str(text).replace("|", "\\|").replace("\n", " ")


def mnemonics_block(table):
    out = ["| group | mnemonic | selected by | result | status |", "|---|---|---|---|---|"]
    n = 0
    for form in table.forms:
        for v in form.variants:
            n += 1
            out.append("| `%s` | `%s` | %s | %s | %s |" % (
                form.name, v.mnemonic, cell(selected_by(form, v)), cell(v.result), v.status))
    out.append("")
    out.append("%d variants in %d form groups. Notes of the forms (`$doc` in isa.json):" % (n, len(table.forms)))
    for form in table.forms:
        for k, x in form.entry.items():
            if k.startswith("$doc"):
                text = " ".join(x) if isinstance(x, list) else str(x)
                out.append("* `%s` %s: %s" % (form.name, k, cell(text)))
    return out


def replace_block(text, name, lines):
    pat = re.compile(r"(<!-- isadoc:%s -->\n).*?(<!-- /isadoc:%s -->)" % (name, name), re.S)
    if not pat.search(text):
        raise SystemExit("%s: no <!-- isadoc:%s --> block" % (DOC, name))
    return pat.sub(lambda m: m.group(1) + "\n".join(lines) + "\n" + m.group(2), text)


def main():
    table = isa.Table.load()
    blocks = {"classes": classes_block(table), "mnemonics": mnemonics_block(table)}
    if "--stdout" in sys.argv:
        for name, lines in blocks.items():
            print("<!-- isadoc:%s -->\n%s\n" % (name, "\n".join(lines)))
        return 0
    text = open(DOC, encoding="utf-8").read()
    for name, lines in blocks.items():
        text = replace_block(text, name, lines)
    open(DOC, "w", encoding="utf-8").write(text)
    print("wrote %s (%d classes, %d mnemonic rows)" % (os.path.relpath(DOC, ROOT), len(table.classes),
                                                      len(blocks["mnemonics"]) - 2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
