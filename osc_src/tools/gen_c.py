#!/usr/bin/env python3
"""gen_c.py -- Ghidra's decompilation of ``m0_core0`` as C that compiles.

The layer is code plus one data blob:

* ``c/code/``     the decompiled functions, as C.  One ``.c`` per contiguous
                  run of functions; every function keeps its own name and its
                  own prototype, and every address it uses stays absolute.
* ``c/data/``     the image's other bytes -- tables, strings, and the literal
                  pools the decompiler folded away -- as one blob, in the
                  order the image has it (``.osc_data``).
* ``c/osc_d.h``   one macro per named place (``OSC_P``) and per function
                  (``OSC_F``), so a fixed address is typed where it is used.
* ``c/fwd.h``     the prototypes, one per function.

The decompiler reorders memory: it folds read-only data into the text, gives
every string an address of its own and drops what nothing references, so its C
can never reproduce the image byte for byte.  What it does keep is the *use* of
those addresses, and every place has a name:

    *DAT_600003e0 = x   ->   *(volatile uint32_t *)OSC_P(D_600003E0) = x

The place is emitted with the image's bytes at exactly that address, and the
link is ``-N`` (no layout: nothing moved, nothing aligned).  So every absolute
address in the generated code still means what it meant in the firmware, and
the code needs no relocation to run where it ran.

    python3 tools/gen_c.py --dump ../work/c_dump/m0_core0 -o c

Ghidra is needed for the dump only (``tools/ghidra_dump.py``, ``make ghidra``);
the generated C builds with ``arm-none-eabi-gcc`` alone (``tools/build_c.py``).
"""
from __future__ import annotations

import argparse
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

BASE = 0x60000000
M0_END = 0x60130E00
TEXT_END = 0x60094800          # code; after it the image is tables/strings/data
BLOCK = 0x10000                # naming block of the data area
SEP = re.compile(r"^///// ([0-9a-fA-F]+) (\S+)$")

# Ghidra's data symbols: the class in front gives the width, the trailing hex
# digits are the address, and ``_<symbol>_<k>`` a repeat count.
SYM_RE = re.compile(
    r"(?<![0-9A-Za-z_$])_?(?:pcRam|DAT|PTR|UNK|BYTE|WORD|DWORD|QWORD|CHAR|UCHAR|s|code|LAB|SUB|joined|"
    r"uRam|iRam|cRam|sRam|bRam|ram|ROM|EXT|in_DAT|in_PTR|in_UNK|in_BYTE|in_WORD)"
    r"_?[A-Za-z0-9_$]*?[0-9a-fA-F]{6,8}(?:_[0-9A-Za-z_$]*?)?(?:_[0-9]{1,2})?(?![0-9A-Za-z_$])")
# the same without the underscore requirement, for Ghidra's RAM names
SYM_RE2 = re.compile(
    r"(?<![0-9A-Za-z_$])(?:_?(?:pcRam|DAT|PTR|UNK|BYTE|WORD|DWORD|QWORD|CHAR|UCHAR|s|code|LAB|"
    r"SUB|joined|uRam|iRam|cRam|sRam|bRam|ram|ROM|EXT|in_DAT|in_PTR|in_UNK|in_BYTE|in_WORD)"
    r"_?[A-Za-z0-9_$]*?|[uibcs]Ram[0-9a-fA-F]*?)[0-9a-fA-F]{6,8}"
    r"(?:_[0-9A-Za-z_$]*?)?(?:_[0-9]{1,2})?(?![0-9A-Za-z_$])")
# the address and the repeat count of a symbol, read back from its name
ADDR_RE = re.compile(r"_?([0-9a-fA-F]{6,8})(?:_[A-Za-z0-9_$]*?)?(?:_([0-9]{1,2}))?$")
SYM_CLASS = re.compile(r"^_?([A-Za-z]+)")
SIZES = {"BYTE": 1, "CHAR": 1, "UCHAR": 1, "WORD": 2, "DWORD": 4, "QWORD": 8,
         "UNK": 1, "DAT": 4, "PTR": 4, "s": 1, "uRam": 4, "iRam": 4, "cRam": 1,
         "sRam": 2, "bRam": 1, "ram": 4, "EXT": 4, "code": 4, "ROM": 4}
FUN_RE = re.compile(r"(?<![0-9A-Za-z_$])(?:thunk_)?(?:FUN|fn)_([0-9a-fA-F]{8})(?![0-9A-Za-z_$])")
NOARGS = re.compile(r"^([A-Za-z_][A-Za-z0-9_ ]*?\b[A-Za-z_][A-Za-z0-9_]*)( *\( *\))", re.M)


def log(msg):
    print("gen_c: " + msg, file=sys.stderr)


# --------------------------------------------------------------- addresses ---
def m0_addr(off):
    """``m0 | g``: the 64 KiB block and the offset inside it.

    The image is at 0x60000000 and its data at 0x60100000-0x61000000, so the
    offset from 0x60000000 is unique and fits 7 hex digits: a place's name
    keeps its address (``D_<block><offset>``) without exposing the base.
    """
    return "%07X" % off


def addr_of(off):
    """The name's block/offset back to the offset from 0x60000000."""
    return off


def ctype(size):
    return {1: "uint8_t", 2: "uint16_t", 4: "uint32_t", 8: "uint64_t"}[size]


# --------------------------------------------------------------- the dump ----
class Dump:
    def __init__(self, path):
        self.path = path
        self.units = self._units(os.path.join(path, "units.txt"))
        self.funcs = self._funcs(os.path.join(path, "funcs.txt"))
        self.decomp = self._decomp(os.path.join(path, "decomp.txt"))

    @staticmethod
    def _units(path):
        out = {}
        with open(path) as f:
            for line in f:
                parts = line.rstrip("\n").split(" ", 4)
                if len(parts) < 4:
                    continue
                try:
                    addr, ln = int(parts[0], 16), int(parts[1])
                    raw = bytes.fromhex(parts[2])
                except ValueError:
                    continue
                out.setdefault(addr, (raw, parts[3] == "D"))
        return out

    @staticmethod
    def _funcs(path):
        out = []
        with open(path) as f:
            for line in f:
                p = line.split()
                if len(p) == 2:
                    try:
                        out.append((int(p[0], 16), p[1]))
                    except ValueError:
                        pass
        return out

    @staticmethod
    def _decomp(path):
        out, cur, buf = [], None, []
        with open(path, errors="replace") as f:
            for line in f:
                m = SEP.match(line.rstrip("\n"))
                if m:
                    if cur is not None:
                        out.append((cur[0], cur[1], "".join(buf)))
                    cur, buf = (int(m.group(1), 16), m.group(2)), []
                elif cur is not None:
                    buf.append(line)
        if cur is not None:
            out.append((cur[0], cur[1], "".join(buf)))
        return out

    # -- what the dump says about the bytes --------------------------------
    def byte_map(self):
        """'I' (instruction), 'D' (data unit) or None for every byte."""
        n = M0_END - BASE
        m = bytearray(b"-" * n)
        for (a, (raw, isd)) in self.units.items():
            o = a - BASE
            if 0 <= o < n and o + len(raw) <= n:
                m[o:o + len(raw)] = (b"D" if isd else b"I") * len(raw)
        return bytes(m)


def image_bytes():
    with open(os.path.join(ROOT, "..", "dumps", "os_full.bin"), "rb") as f:
        return f.read()


# ---------------------------------------------------------------- preflight --
def preflight(d, img):
    """The dump itself: coverage, entries, and what the C layer will need."""
    starts = sorted({a for a, _ in d.funcs})
    log("preflight: %d listing units, %d function entries, %d decompiled"
        % (len(d.units), len(starts), len(d.decomp)))
    log("preflight: %d entries with several names" % (len(d.funcs) - len(starts)))
    m = d.byte_map()
    gaps = [i for i in range(len(m)) if m[i:i + 1] == b"-"]
    log("preflight: %d byte(s) of m0_core0 not in the listing" % len(gaps))
    ins = m[:TEXT_END - BASE].count(b"I")
    dat = m[:TEXT_END - BASE].count(b"D")
    log("preflight: text: %d instruction bytes, %d data bytes (literal pools,"
        " tables) in the code area" % (ins, dat))
    missing = [a for a, _ in d.funcs if a not in d.units]
    if missing:
        log("preflight: %d entries with no instruction (first %#x)"
            % (len(missing), missing[0]))
    noargs = sum(1 for _a, _n, t in d.decomp if NOARGS.search(t[:400]))
    log("preflight: %d decompiled function(s) take no argument" % noargs)
    return not gaps and not missing


# ------------------------------------------------------------ name model ----
class Names:
    """The C name of every function and every place the code touches."""

    def __init__(self, d, img):
        self.d = d
        self.img = img
        self.fn = {}                 # address -> fn_<m0|g> (all entries)
        for a, _name in d.funcs:
            self.fn.setdefault(a, "fn_" + m0_addr(a - BASE))
        self.places = {}             # name -> (addr, size)
        self.sym = {}                # Ghidra symbol -> C name
        self.stored = self._stored_through()
        self.sig = {}                # function address -> its C definition

    def _stored_through(self):
        """Image addresses the code stores a word holding them into."""
        s = set()
        for (a, (raw, isd)) in self.d.units.items():
            if not isd and len(raw) == 4:
                v = int.from_bytes(raw, "little")
                if BASE <= v < BASE + M0_END - BASE:
                    s.add(v)
        return s

    RAM_BASE = 0x60000000      # address 0x01000000-0x01FFFFFF names RAM

    def place(self, addr, size=4, k=0):
        """``D<width>_<address>``: the width is in the name, so OSC_P4 applies,
        and the offset from 0x60000000 is the address (7 hex digits)."""
        name = "D%d_%s" % (size, m0_addr(addr - BASE if addr >= BASE else addr))
        if k:
            name += "_%d" % k
        if name not in self.places:
            self.places[name] = (addr, size * (k or 1))
        return name

    def by_symbol(self, sym):
        """Ghidra symbol -> C name (a function or a place), memoised."""
        if sym in self.sym:
            return self.sym[sym]
        # 0x010507E0 is the kernel's RAM for the OS and 0x610507E0 the same
        # place through the image's window: one place, one name
        name = None
        m = FUN_RE.fullmatch(sym)
        if m:
            name = "fn_" + m0_addr(int(m.group(1), 16) - BASE)
        else:
            m = SYM_RE.fullmatch(sym)
            if m:
                cm = SYM_CLASS.match(sym)
                cls = cm.group(1) if cm else "DAT"
                am = ADDR_RE.search(sym)
                addr = int(am.group(1), 16) if am else -1
                k = int(am.group(2)) if am and am.group(2) else 0
                if cls == "LAB":
                    # a jump into a function body outside the image (the
                    # kernel in RAM): a tripwire named by its own address
                    name = "LAB_%08x" % (addr if 0 <= addr < 0x01000000 else addr & 0xffffffff)
                elif 0 <= addr < 0x10000000 and addr < BASE:
                    # the kernel's RAM for the OS (0x010507E0 and the like):
                    # variables the kernel initialises, not part of the image
                    name = "RAM_%08X" % addr
                elif BASE <= addr < BASE + 0x10000000:
                    name = self.place(addr, SIZES.get(cls, 1), k)
                elif 0x10000000 <= addr < 0x60000000:
                    # a MMIO register: an address, not a place of the image
                    name = "IO_%08X" % addr
                else:
                    # any other absolute address the code computes with (a
                    # discarded function pointer, a bit pattern): a value
                    name = "NUM_%08X" % (addr & 0xffffffff)
        if name is not None:
            self.sym[sym] = name
        return name


# ---------------------------------------------------------------- rewriter ---
class Rewriter:
    """One decompiled function -> compilable C."""

    def __init__(self, names, argc=None):
        self.names = names
        self.calls = set()
        self.used = set()
        self.argc = argc or {}

    def place_token(self, sym):
        """A place Ghidra named -> the accessor for the place's width."""
        name = self.names.by_symbol(sym)
        if name is None or name.startswith("fn_"):
            return sym
        if name.startswith("LAB_"):
            return name
        if name.startswith("RAM_"):
            # the kernel's RAM: not part of the image, and not something this
            # C can place; the accessor stops the program
            return "OSC_PA(OSC_RAM(0x%s))" % name[4:]
        if name.startswith("IO_"):
            # a device register at its absolute address
            return "OSC_PA(0x%s)" % name[3:]
        if name.startswith("NUM_"):
            # a plain number the decompiler printed as an address
            return "0x%s" % name[4:]
        self.used.add(name)
        return "OSC_P%s(%s)" % (name[1], name)

    def fn_name(self, sym):
        """Ghidra's name of a function -> the C name it calls."""
        m = re.search(r"_([0-9a-fA-F]{8})$", sym)
        if m and sym.split("_")[0].startswith("LAB"):
            # a jump into a function outside the image (the kernel): the
            # header's tripwire, named by that address
            return "LAB_%08x" % int(m.group(1), 16)
        if m:
            addr = int(m.group(1), 16)
            if addr in self.names.fn:
                self.calls.add(self.names.fn[addr])
                return self.names.fn[addr]
        name = self.names.by_symbol(sym)
        if name is None:
            # a jump target inside a function (`LAB_<addr>`) cannot be a
            # function: it keeps its name, and the header turns it into a
            # tripwire that stops the program
            return sym
        if name.startswith("fn_"):
            self.calls.add(name)
        return name

    def calls_through_words(self, code):
        """`(*(T *)x)(a)` -> `OSC_CALL(x)(a)`: a call of the word C sees.

        The decompiled C writes a call through a word as ``(*cast)(args)``;
        the word is the text inside the cast.  ``(*(code **)(p + 4))()``
        names the word ``*(p + 4)``, which is the address the code calls.
        """
        out = code
        start = 0
        while True:
            i = out.find("(*(void (*)(void))", start)
            if i < 0:
                i = out.find("(*(code *)", start)
            j = out.find("(*(code **)", start)
            if i < 0 and j < 0:
                break
            if i < 0 or (j >= 0 and j < i):
                i = j
                close = out.find(")", i + 11)
                body = out[i + 11:close].strip()
            else:
                head = 9 if out.startswith("(*(code *)", i) else 18
                close = out.find(")", i + head)
                body = out[i + head:close].strip()
            # the closing parenthesis of the whole cast: after it comes the
            # argument list
            end = close + 1
            while end < len(out) and out[end] in " \t\n":
                end += 1
            if not out.startswith("(", end):
                start = i + 1
                continue
            out = out[:i] + "OSC_CALL(" + body + ")" + out[end:]
            start = i + 1
        return out

    def trim_calls(self, code):
        def one(m):
            name, args = m.group(1), m.group(2)
            n = self.argc.get(name)
            if n is None:
                return m.group(0)
            parts = args.split(",")
            if len(parts) <= n:
                return m.group(0)
            return name + "(" + ",".join(parts[:n]) + ")"
        return re.sub(r"\b(fn_[0-9A-F]{7})\(([^;()]*)\)", one, code)

    def run(self, text):
        code = text
        # 1. places.  A read is `*DAT_x` (or `*&DAT_x` when the decompiler
        #    kept the pointer); the place itself is a fixed address, so the
        #    accessor for its width is the right lvalue and the `*` goes away.
        #    An address, `&DAT_x` or a bare `DAT_x`, stays the name, which
        #    *is* the address.
        def place(m):
            deref, amp, sym = m.group(1), m.group(2), m.group(3)
            name = self.names.by_symbol(sym)
            if name is None:
                return m.group(0)
            if name.startswith("fn_"):
                self.calls.add(name)
                return (deref or amp or "") + name + "_kr"
            if name.startswith("LAB_"):
                return name
            if name.startswith("RAM_"):
                return "OSC_PA(OSC_RAM(0x%s))" % name[4:]
            if name.startswith("IO_"):
                return "OSC_PA(0x%s)" % name[3:]
            if name.startswith("NUM_"):
                return "0x%s" % name[4:]
            self.used.add(name)
            if deref is None and amp is None:
                # the decompiler wrote the address where it read the place
                return "OSC_P%s(%s)" % (name[1], name)
            if deref:
                return "OSC_P%s(%s)" % (name[1], name)
            return name

        code = re.sub(r"(\*\s*&?\s*|&\s*|(?<![0-9A-Za-z_$]))(" + SYM_RE.pattern + ")",
                      lambda m: place(_Three(m.group(1), None, m.group(2))),
                      code)
        # 2. Ghidra's function names: `fn_600001c2` -> the C name.  The
        #    decompiled function is fn_00001C2 for the image address
        #    0x600001C2, and Ghidra's `LAB_<addr>` / `FUN_<addr>` of an address
        #    *inside* a function is that function's name only when the image
        #    has a function there; the call-site rewriter below resolves them.
        code = FUN_RE.sub(lambda m: self.fn_name(m.group(0)), code)
        # 2a. a place Ghidra named by an address of its own (a RAM window)
        #     is the same place, and takes the accessor for its width
        def reread(m):
            return self.place_token(m.group(0))
        code = SYM_RE2.sub(reread, code)
        # an absolute address the body writes to is a place: `0x70A4F540 = v`
        code = re.sub(r"(?m)^(\s*)(0x[0-9A-Fa-f]{8})\s*=", r"\1OSC_PA(\2) =", code)
        # 2a. a call whose result is used is a call to a value-returning
        #     function, whatever its own body returns (the word in r0 is the
        #     value the original code takes)
        # a call site whose value is dropped or used: `fn_..._kr` (the ABI's
        # few words) is the K&R declaration, `_v` the value-returning one
        code = re.sub(r"\(\s*void\s*\)\s*(?=fn_[0-9A-F]{7}\b)", "(long long)", code)
        code = re.sub(r"\b(fn_[0-9A-F]{7})(?=\s*\()", r"\1_kr", code)
        # 2b. a call with more arguments than the prototype cannot compile:
        #     the excess ones are dropped (the callee reads them as garbage,
        #     which is what would happen with the ABI)
        code = self.trim_calls(code)
        # 3. labels that point into a function (switch cases, jump targets)
        code = re.sub(r"\b(switchD|caseD)_", r"sw_\1_", code)
        code = re.sub(r"(?m)^(\s*)(LAB_[0-9a-fA-F]{8}):", r"\1sw_\2:", code)
        code = re.sub(r"\bgoto (LAB_[0-9a-fA-F]{8})\b", r"goto sw_\1", code)
        code = re.sub(r"\bcode_r0x([0-9a-fA-F]{8})\b", r"jr_\1", code)
        # the decompiler's "this is a function pointer" cast, applied to the
        # 32-bit word of an instruction Ghidra could not express
        code = re.sub(r"\(\s*code\s*\*\s*\)", "(void (*)(void))", code)
        # a jump into the middle of a function keeps its (unspellable) target
        # as a tripwire: C cannot take a label's address
        code = re.sub(r"&\s*(LAB_[0-9a-fA-F]{8})", r"\1", code)
        # a jump into the middle of a function is written as a call through a
        # cast expression; C cannot take the address of a label, so the call
        # goes through a statement expression

        # 3b. ``(T *)(frame + off)._3_1_``: a byte of a word, as a value
        code = re.sub(r"\(([^()]*)\)\._([0-7])_1_", r"((\1) >> 8*\2)", code)
        code = re.sub(r"\b([A-Za-z_][A-Za-z0-9_]*)\.(_[0-7]_[1248]_)", r"\1", code)
        # 3c. a call through a word: `(*(code *)x)(a)` and `(*x)(a)` are
        #     `(x)(a)` in the decompiler's notation, and C cannot call a
        #     word, so the call goes through OSC_CALL (osc.h).  The chain
        #     `(...)(a)(b)` is the same thing twice.
        code = re.sub(r"(?<![0-9A-Za-z_$*])\*\s*([A-Za-z_0-9][A-Za-z0-9_]*)\s*\)\s*\(", 
                      r"OSC_CALL(\1))(", code)
        # `(**(code **)x)(a)`: the function pointer is the word at x
        code = re.sub(r"\(\s*\*\s*\*\s*\(\s*code \*\*\s*\)\s*", "OSC_CALL_AT(", code)
        code = self.calls_through_words(code)
        code = re.sub(r"\)\s*\(", ") OSC_CALL (", code) if False else code
        # 4. Ghidra's undefined widths are the operands of a 32-bit core
        code = re.sub(r"\bundefined\b", "uint32_t", code)
        code = re.sub(r"\bundefined1\b", "uint8_t", code)
        code = re.sub(r"\bundefined2\b", "uint16_t", code)
        code = re.sub(r"\bundefined4\b", "uint32_t", code)
        code = re.sub(r"\bundefined8\b", "uint64_t", code)
        # an untyped cast `(uint *)x` and the like: the word is what moves
        code = re.sub(r"\(\s*(?:uint|ushort|uchar|byte|code|longlong|ulonglong)\s*\*\s*\)",
                      "(uint32_t *)", code)
        code = re.sub(r"\(\s*(?:uint|ushort|uchar|byte|code|longlong|ulonglong)\s*\)",
                      "(uint32_t)", code)

        # the decompiled body's `return;` in a non-void function: the value
        # is whatever is in r0, so make it explicit
        if "uint32_t" in code.split("\n", 1)[0] or re.search(r"(?m)^uint32_t \w+\(", code):
            code = re.sub(r"(?m)^(\s*)return;", r"\1return (uint32_t)0;", code)
        # 6. the definition keeps Ghidra's parameter list; a no-argument
        #    function that some call passes arguments to must accept them
        code = re.sub(r"(?m)^(void (fn_[0-9A-F]{7}))\(\s*void\s*\)",
                      lambda m: (m.group(1) + "(" + ", ".join(
                          "uint32_t a%d" % i for i in range(self.argc[m.group(2)]))
                          + ")") if m.group(2) in self.argc else m.group(1) + "(void)",
                      code)
        return code


# ----------------------------------------------------------------- passes ----
def is_fn_name(name):
    return name.startswith("fn_")


def sized(name, size):
    """``sizeof`` of the place: the macro keeps the type (OSC_P -> lvalue)."""
    return "sizeof(%s)" % name


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as f:
        f.write(text)


class _One:
    """One whole match, for a rewriter that looks at the text only."""
    __slots__ = ("s",)

    def __init__(self, s):
        self.s = s

    def group(self, i=None):
        return self.s


class _Three:
    """(dereference, address-of, symbol) of one match of ``place``."""
    __slots__ = ("a", "b", "c")

    def __init__(self, a, b, c):
        self.a, self.b, self.c = a, b, c

    def group(self, i):
        return (self.a, self.b, self.c)[i - 1]


def callable_name(gn):
    """Ghidra's name of a function -> the C name the call sites use."""
    try:
        return "fn_" + m0_addr(int(gn[-8:], 16) - BASE)
    except ValueError:
        return gn


def pass_preamble(d, names, out):
    """``fwd.h``: what a decompiled body needs on its own.

    The C is C89-style and K&R: the decompiler read every function's
    parameters from that function alone, so a body and its call sites disagree
    about how many there are.  The prototypes therefore stay out of the way
    (each function is declared with an empty parameter list, C's "takes
    whatever the caller passes"), and the functions are told apart by name:

        uint32_t fn_00001C2();

    That is enough for the compiler to type every call and for the linker to
    place the code at its address.  The types inside each body are what Ghidra
    inferred, unchanged.
    """
    lines = ["/* generated by tools/gen_c.py -- %d functions of m0_core0 */"
             % len(names.fn),
             "#ifndef OSC_FWD_H", "#define OSC_FWD_H", ""]
    decl = []
    for addr in sorted(names.fn):
        name = names.fn[addr]
        # the declaration is the body's own when there is a body, and a plain
        # K&R one for the entries without a decompiled body
        # C's empty parameter list: "a definition follows, with its own
        # parameters".  A call with any number of arguments is then legal, and
        # the definition in code/ only has to agree with itself.  There is no
        # return type either, which a definition cannot contradict
        # a call whose result is used names the value-returning view of the
        # function (the word the callee leaves in r0)
        # a call is `name(...)`: the macro gives it the K&R declaration's
        # semantics (any number of arguments), and a body written for a value
        # takes the word the callee leaves in r0
        lines.append("#define %s %s_kr" % (name, name))
        lines.append("int %s_kr();" % name)
        lines.append("#define %s_v %s_val" % (name, name))
        lines.append("long long %s_val();" % name)
    lines += ["", "#endif /* OSC_FWD_H */", ""]
    write(os.path.join(out, "fwd.h"), "\n".join(lines))
    log("preamble: %d function(s) -> fwd.h" % len(names.fn))


def pass_code(d, names, out):
    """``code/``: the decompiled functions, one .c per run of neighbours."""
    code_dir = os.path.join(out, "code")
    os.makedirs(code_dir, exist_ok=True)
    for old in os.listdir(code_dir):
        if old.endswith(".c") or old.endswith(".h"):
            os.remove(os.path.join(code_dir, old))
    items = []
    for a, gname, text in d.decomp:
        name = names.fn.get(a) or ("fn_" + m0_addr(a - BASE))
        rw = Rewriter(names)
        # the definition keeps Ghidra's parameters and return type, and takes
        # its name from its address; a K&R definition is its own prototype,
        # which fwd.h only then has to agree with -- and the blank parameter
        # list of fwd.h accepts any call
        pm = re.search(r"(?m)^([A-Za-z_][A-Za-z0-9_ *]*?)\b%s\s*\(([^)]*)\)"
                       % re.escape(gname), text)
        ret = (pm.group(1).strip() if pm else "void")
        ret = re.sub(r"\b(?:uint|undefined4)\b", "uint32_t", ret)
        # Ghidra's parameter list, kept whole: `undefined4 param_1,uint
        # param_2,undefined4 *param_3` names a word, a word and a pointer, and
        # the body uses them that way (the decompiler never took an address of
        # a typed parameter, so this is the use the body was read from)
        ptext = (pm.group(2).strip() if pm else "")
        if ptext in ("", "void"):
            plist = []
        else:
            plist = [p.strip() for p in ptext.split(",")]
        plist = [re.sub(r"\b(undefined4|uint)\b", "uint32_t", p).strip() for p in plist]
        own = [re.sub(r"\*|\[.*", " ", p).split()[-1] for p in plist]
        own = [p for p in own if re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", p)]
        # the file declares the definition first, with the body's own
        # signature: the two only have to agree with each other
        head = ("%s %s_impl(%s)" % (ret, name, ", ".join(plist))
                if plist else "%s %s_impl(void)" % (ret, name))
        names.sig[a] = ("%s %s(%s);" % (ret, name, ", ".join(plist))
                        if plist else "%s %s(void);" % (ret, name))
        body_prefix = ""
        body = re.sub(r"(?m)^[A-Za-z_][A-Za-z0-9_ *]*?\b%s\s*\([^)]*\)"
                      % re.escape(gname), lambda m: head, text, count=1)
        body = rw.run(body)
        # the definition's name is not the macro the calls use; the file
        # declares it before defining it, so it never meets fwd.h's `()` view
        body = "#undef %s\n#pragma GCC diagnostic push\n#pragma GCC diagnostic ignored \"-Wincompatible-pointer-types\"\n%s;\n%s\n#pragma GCC diagnostic pop\n" % (name, head, body)
        items.append((a, name, body, rw))
    items.sort()
    runs = []
    for a, name, body, rw in items:
        if runs and a - runs[-1][0][0] <= 0x10000:
            runs[-1].append((a, name, body, rw))
        else:
            runs.append([(a, name, body, rw)])
    # one file per function: the build compiles, links and reports each on
    # its own (a run of neighbours would hide which function failed why)
    for a, name, body, rw in items:
        head = ['/* generated by tools/gen_c.py -- %#x %s */' % (a, name),
                '#include "osc.h"', '#include "fwd.h"', ""]
        write(os.path.join(code_dir, name + ".c"), "\n".join(head) + body + "\n")
    log("code: %d function(s) -> %s" % (len(items), code_dir))
    return items


def place_address(name):
    """The image offset a place name stands for (``D_<block><offset>``)."""
    return int(name.split("_", 1)[1], 16)


def pass_data(d, names, out, items):
    """``data/``: every byte the C does not express, as one ordered blob."""
    img = image_bytes()
    dmap = d.byte_map()
    used = set()
    for _a, _name, _body, rw in items:
        used |= rw.used
    # a place is named by its address: any 4-aligned, not-yet-emitted byte
    have = bytearray(M0_END - BASE)
    for name, (addr, size) in names.places.items():
        if not is_fn_name(name):
            o = addr - BASE
            have[o:o + size] = b"\1" * size
    lines = ['/* generated by tools/gen_c.py -- the bytes of m0_core0 the C does',
             ' * not spell out (the decompiler folded them into the code): literal',
             ' * pools, tables and strings, in the image\'s order. */',
             '#include "osc.h"', ""]
    n = 0
    k = 0
    while True:
        # the next run of bytes nobody else emits
        start = have.find(b"\0", k) if False else -1
        start = next((i for i in range(k, len(have)) if not have[i]), None)
        if start is None:
            break
        end = start
        while end < len(have) and not have[end]:
            end += 1
        chunk = img[start:end]
        name = "osc_data_%d" % n
        lines.append('/* %#x..%#x, %d bytes */' % (BASE + start, BASE + end,
                                                   len(chunk)))
        lines.append('const uint8_t %s[%d] __attribute__((section(".osc_data"), '
                     'used, aligned(1))) =' % (name, len(chunk)))
        lines.append('    "%s";' % "".join("\\x%02x" % b for b in chunk))
        lines.append("")
        have[start:end] = b"\1" * (end - start)
        n += 1
        k = start
    write(os.path.join(out, "data", "osc_data.c"), "\n".join(lines))
    log("data: %d blob(s), %d byte(s) not spelled by the C"
        % (n, M0_END - BASE - sum(1 for b in have if b)))


def pass_dtable(d, names, out, items):
    """``osc_d.h``: one macro per place and per function."""
    for _a, _name, _body, rw in items:
        pass                          # the places are already registered
    lines = ['/* generated by tools/gen_c.py -- the fixed addresses the OS uses.',
             ' *',
             ' * OSC_P4(D4_01003E0) is the 4-byte place at 0x600003E0: the name',
             ' * carries its width and its address, and the macro is what makes',
             ' * the name usable where the decompiler wrote the place. */',
             '#ifndef OSC_D_H', '#define OSC_D_H', '#include "osc.h"', ""]
    for name in sorted(names.places, key=lambda n: names.places[n][0]):
        addr, _size = names.places[name]
        lines.append("#define %-18s ((uintptr_t)%#010Xu)" % (name, addr))
    lines += ["", "#endif /* OSC_D_H */", ""]
    write(os.path.join(out, "osc_d.h"), "\n".join(lines))
    log("dtable: %d place(s), %d function(s) -> osc_d.h"
        % (len(names.places), len(names.fn)))


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dump", required=True, help="ghidra/Dump.java output directory")
    ap.add_argument("-o", "--out", default=os.path.join(ROOT, "c"))
    ap.add_argument("--func", type=lambda s: int(s, 0), help="one function (iteration)")
    ap.add_argument("--preflight", action="store_true", help="check the dump only")
    ap.add_argument("--only", choices=["preflight", "fwd", "code", "data", "dtable"],
                    help="one pass (iteration)")
    args = ap.parse_args(argv)
    d = Dump(args.dump)
    img = image_bytes()
    if args.preflight or args.only == "preflight":
        return 0 if preflight(d, img) else 1
    names = Names(d, img)
    # the signatures are known once the code is generated, so fwd.h comes last
    steps = [args.only] if args.only else ["code", "data", "dtable", "fwd"]
    if args.func is not None:
        for a, gname, text in d.decomp:
            if a == args.func:
                rw = Rewriter(names)
                print(rw.run(text))
        return 0
    items = None
    for step in steps:
        if step == "fwd":
            pass_preamble(d, names, args.out)
        elif step == "code":
            items = pass_code(d, names, args.out)
        elif step == "data":
            if items is None:
                # the data layer needs to know which places the code names
                items = []
                for a, gname, text in d.decomp:
                    rw = Rewriter(names)
                    rw.run(text)
                    items.append((a, None, None, rw))
            pass_data(d, names, args.out, items)
        elif step == "dtable":
            if items is None:
                items = []
                for a, gname, text in d.decomp:
                    rw = Rewriter(names)
                    rw.run(text)
                    items.append((a, None, None, rw))
            pass_dtable(d, names, args.out, items)
    return 0


if __name__ == "__main__":
    sys.exit(main())
