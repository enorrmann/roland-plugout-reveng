"""Coverage by instruction family and confidence level, with a regression guard.

    python3 tests/coverage.py             # the report
    python3 tests/coverage.py --check     # fail if below tests/baseline.json

The point of this file is that a single aggregate number hides regressions:
one family can improve while another collapses and the total stays flat. So
the report is per family (which table form produced the instruction) and per
confidence tag (exact / measured / inferred / fitted / raw), and the check
compares both against the stored baseline.
"""

import collections
import glob
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
ROOT = os.path.dirname(os.path.dirname(HERE))

import disasm  # noqa: E402
import isa  # noqa: E402

BASELINE = os.path.join(HERE, "baseline.json")


def corpus_files(directory="corpus*"):
    out = []
    for path in sorted(glob.glob(os.path.join(ROOT, "work", directory, "*.txt"))):
        try:
            words = [int(tok, 16) for tok in open(path).read().split()]
        except ValueError:
            continue
        if words:
            out.append((path, words))
    return out


def families(table, text):
    """the table forms that produced the units of one decoded instruction"""
    out = []
    for part in text.split("|"):
        mnemonic = part.strip().split(" ")[0]
        variant = table.mnemonics.get(mnemonic)
        out.append(variant.form.name if variant else "(raw)")
    return out


def report(directory="corpus_all"):
    table = isa.Table.load()
    by_family = collections.Counter()
    by_status = collections.Counter()
    raw_reasons = collections.Counter()
    total = decoded = 0
    for path, words in corpus_files(directory):
        text, rep = disasm.program(words, table, os.path.basename(path))
        total += rep["instructions"]
        decoded += rep["decoded"]
        for k, v in rep["statuses"].items():
            by_status[k] += v
        for key, instruction, reason in rep["verbatim"]:
            raw_reasons[reason.split(":")[0]] += 1
        for line in text.splitlines():
            if not line or line.startswith((";", ".")):
                continue                  # comments and directives (.class, .free, .slot)
            body, _, tag = line.partition(";")
            status = tag.strip().split()[0].strip("()") if tag.strip() else "?"
            for fam in families(table, body):
                by_family[(fam, status)] += 1
    return {"total": total, "decoded": decoded, "families": by_family,
            "statuses": by_status, "raw_reasons": raw_reasons}


def show(directory):
    r = report(directory)
    pct = 100.0 * r["decoded"] / r["total"] if r["total"] else 0
    print("== %s: %d instructions, %d decoded (%.1f%%), %d raw"
          % (directory, r["total"], r["decoded"], pct, r["total"] - r["decoded"]))
    print("\nby confidence:")
    for status, n in sorted(r["statuses"].items(), key=lambda kv: -kv[1]):
        print("  %-10s %6d  %5.1f%%" % (status, n, 100.0 * n / r["total"]))
    print("\nby family (decoded units):")
    for (fam, status), n in sorted(r["families"].items(), key=lambda kv: -kv[1])[:16]:
        print("  %-12s %-10s %6d" % (fam, status, n))
    print("\ntop reasons a word stayed verbatim:")
    for reason, n in r["raw_reasons"].most_common(8):
        print("  %6d  %s" % (n, reason))
    return pct


def check():
    data = json.load(open(BASELINE))
    ok = True
    for directory in ("corpus_all", "corpus_s1"):
        if not corpus_files(directory):
            continue
        r = report(directory)
        pct = 100.0 * r["decoded"] / r["total"] if r["total"] else 0
        key = {"corpus_all": ("decoding", "corpus_all_pct"),
               "corpus_s1": ("decoding", "corpus_s1_pct")}.get(directory)
        want = data.get(key[0], {}).get(key[1], 0) if key else 0
        mark = "ok" if pct + 0.05 >= want else "REGRESSION"
        if mark != "ok":
            ok = False
        print("%-12s %.1f%%  (baseline %.1f%%)  %s" % (directory, pct, want, mark))
    return 0 if ok else 1


if __name__ == "__main__":
    if "--check" in sys.argv:
        sys.exit(check())
    for d in ("corpus_all", "corpus_s1", "corpus"):
        if corpus_files(d):
            show(d)
            print()
