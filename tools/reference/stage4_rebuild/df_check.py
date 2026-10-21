"""The Stage 4 deliberate failure checks (rule R11), tried on the reference first.

Each check makes one small change to the reference source (the same change the C++ check
makes), runs the checks that exist at that sub-stage (and Stage 3's when the change is in Stage 3
code), and prints the ids that fail. The plan (Section 7) lists the result as
the expected failures. `python3 df_check.py` takes several minutes (each change runs the gate)."""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import checks  # noqa: E402
import ref_faults  # noqa: E402
import ref_material  # noqa: E402
import ref_scorer  # noqa: E402
import ref_fault_csv  # noqa: E402

sys.path.insert(0, os.path.join(HERE, '..', 'stage3'))
import ref_parts  # noqa: E402

MUTATIONS = [
    ('DF4a-1', 'Cursor', ref_material,
     "        for i in range(len(values) - 1, 0, -1):\n            j = self.below(i + 1)",
     "        for i in range(len(values) - 1):\n            j = i + self.below(len(values) - i)",
     'shuffle runs from the front (i = 0 .. size-2, j = i + below(size - i)) instead of from the end'),
    ('DF4b-1', 'Cursor', ref_material, "    return (2 * count + 5) // 10", "    return count // 5",
     'the common-mode share rounds down instead of half up'),
    ('DF4c-1', 'Cursor', ref_faults,
     "        if previous == b.agent:\n            return False, 'the previous square is the believed agent square'\n",
     '', "F5 forgets A3's check of the believed agent square"),
    ('DF4d-1', 'Cursor', ref_scorer,
     "                for e in range(count):\n                    if not start_active[e]:",
     "                for e in range(count):\n                    if not self.active_now(view, e) or not self.scores[e].activated:",
     'a claim is judged against the end of its phase instead of the start'),
    ('DF4e-1', 'Cursor', ref_parts,
     "                and not read_dead_status(m, ahead)):",
     "                and not m.dead[ahead[1]][ahead[0]]):",
     'the local check reads the map directly instead of through readDeadStatus'),
    ('DF4f-1', 'Cursor', ref_fault_csv,
     "    delay = '' if score.strict_step is None else str(score.strict_step - activation.application_step)",
     "    delay = '' if score.strict_step is None else str(score.strict_step - e.step)",
     "the detection delay counts from the scheduled step instead of the activation step"),
    ('DF4g-1', 'Cursor', ref_scorer,
     "            if not self.awaiting(self.scores[i]) or event.type not in ('F1', 'F4'):",
     "            if not self.awaiting(self.scores[i]) or event.type != 'F4':",
     'dead-status reads are compared for F4 events only, not F1'),
    ('DF4g-2', 'owner', ref_faults, "'the object has not moved yet'", "'the object did not move yet'",
     'the F5 reason text "the object has not moved yet" becomes "the object did not move yet"'),
]


def mutate(module, old, new):
    path = module.__file__
    source = open(path).read()
    if source.count(old) != 1:
        raise SystemExit('mutation text not found exactly once in %s: %r' % (path, old))
    exec(compile(source.replace(old, new), path, 'exec'), module.__dict__)
    return source


def restore(module, source):
    exec(compile(source, module.__file__, 'exec'), module.__dict__)


ORDER = ['4a', '4b', '4c', '4d', '4e', '4f', '4g']


def groups_until(cid):
    """The checks that exist when the check is made: every sub-stage up to its own."""
    last = cid[2:4].lower()
    return ORDER[:ORDER.index(last) + 1]


def main():
    only = sys.argv[1:]
    for cid, who, module, old, new, what in MUTATIONS:
        if only and cid not in only:
            continue
        source = mutate(module, old, new)
        try:
            bad = checks.failing(False, groups_until(cid))
            if module is ref_parts:
                bad = checks.S3.failing(False) + bad
        finally:
            restore(module, source)
        print('%s (%s): %s\n  failing: %s' % (cid, who, what, ', '.join(bad) if bad else 'none'))


if __name__ == '__main__':
    main()
