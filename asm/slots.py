"""The slot array of one instruction (DSP_ISA_SPEC.md §4.3).

    The slot array = every word after the last T.  Slot k = the (k+1)-th word.
    N words come first (one per M). Among the remaining slots, the ones a
    memory transfer (mode fe) points to are references; the ones nobody points
    to are the immediate bit stream, in order.

Both directions use this module. Each word of the array gets at most one owner
(N word, reference, jump offset, `.slot` word); a second claim with a different
value is a conflict, reported, never resolved by order. Immediate bits are not
owners: they are laid over the array and must agree with the owner they fall on.

Two facts from §4.3 that a count of R words cannot express:

* a transfer addresses slot `Y >> 1` -- an *index*, which may be larger than
  the number of references (the corpus has transfers pointing at slot 7 while
  only four references exist), and two transfers may share one slot;
* the N words are part of the array, so slot 0 is the first M's second word
  whenever the class has an M.
"""


class SlotError(Exception):
    """the instruction cannot be laid out without losing information"""


class Occupant:
    """what a slot word holds: a reference, an immediate, an N word, a jump
    offset, or a word stated outright with `.slot` (spec 4.4: unread stream words)"""

    N = "N"
    REF = "R"
    IMM = "I"
    JUMP = "J"
    FIXED = "F"            # a word the source states outright (`.slot`), read by nothing

    def __init__(self, kind, value=None, by=()):
        self.kind, self.value, self.by = kind, value, tuple(by)

    def __repr__(self):
        return "Occupant(%s, %r, by=%r)" % (self.kind, self.value, self.by)


class SlotArray:
    """the slot array of one instruction: its size and who owns each word"""

    def __init__(self, base, n_words, size):
        """`base` = body index of slot 0; `size` = number of slot words"""
        self.base = base
        self.n_words = n_words
        self.size = size
        self.slots = [None] * size

    # -- discovery ---------------------------------------------------------
    def take_n(self, values):
        """the N words come first, one per M (§4.3)"""
        for i, v in enumerate(values[:self.n_words]):
            self.slots[i] = Occupant(Occupant.N, v, ("M%d" % i,))

    def claim(self, index, occ):
        """record an occupant; a position already taken by something different
        is a conflict, not something to overwrite"""
        if not 0 <= index < self.size:
            raise SlotError("slot %d is outside the array (%d words)"
                            % (index, self.size))
        have = self.slots[index]
        if have is None:
            self.slots[index] = occ
            return
        if have.kind == occ.kind and have.value == occ.value:
            self.slots[index] = Occupant(have.kind, have.value, have.by + occ.by)
            return
        raise SlotError("slot %d is claimed twice: %s and %s"
                        % (index, have, occ))

    def claim_reference(self, index, value, by):
        self.claim(index, Occupant(Occupant.REF, value, (by,)))

# ---------------------------------------------------------------------------
# reference indices named by the source
# ---------------------------------------------------------------------------

