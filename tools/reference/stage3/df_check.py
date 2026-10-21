"""The Stage 3 deliberate failure checks (plan STAGE_03, Section 7), tried on the reference
first (rule R11). Each one changes the reference exactly as the C++ change will, runs the
checks that exist at that sub-stage, reports which ids fail, and undoes the change.

  python3 df_check.py          all of them (the 3g ones run the gate: a few minutes)
  python3 df_check.py --fast   without the two 3g gate checks"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import checks  # noqa: E402
import ref_agents as ra  # noqa: E402
import ref_verify as rc  # noqa: E402
import ref_parts as rp  # noqa: E402

rw = rp.rw

# The checks that exist after each sub-stage (plan Section 7).
PARTS = {
    '3a': [checks.check_footprint, checks.check_log],
    '3b': [checks.check_dead_squares],
    '3c': [checks.check_predictor, checks.check_agent_model, checks.check_planner],
    '3d': [checks.check_investigation],
    '3e': [checks.check_single_agent],
    '3f': [checks.check_naive_agent],
    '3g': [checks.check_reader, checks.check_models],
}
ORDER = ['3a', '3b', '3c', '3d', '3e', '3f', '3g']


def failing_at(stage, gate=False):
    results = {}
    for s in ORDER[:ORDER.index(stage) + 1]:
        for part in PARTS[s]:
            results.update(part())
    if gate:
        results.update(checks.check_gate(False))
    return [k for k, v in results.items() if not v]


def patched(module, name, replacement):
    """Context: replaces module.name for the duration of one check run."""
    class Patch:
        def __enter__(self):
            self.old = getattr(module, name)
            setattr(module, name, replacement)

        def __exit__(self, *args):
            setattr(module, name, self.old)
    return Patch()


# DF3a-1: the footprint forgets rule (iii) (observedAfter.object == c1).
def footprint_without_iii(belief_before, action, observed_after, observed_outcome):
    c0 = belief_before.agent
    c1 = rw.moved(c0, action)
    cells = [c0, c1]
    if belief_before.object == c1 or observed_outcome == 'Pushed':
        cells.append(rw.moved(c1, action))
    return cells


# DF3b-1: the overrides are applied in the other order ("dead" corrections first).
def dead_squares_wrong_order(belief, not_dead, dead_corrections, counts):
    m = ORIGINAL_DEAD(belief, [], [], counts)
    rp._apply_overrides(m, belief, dead_corrections, True)
    rp._apply_overrides(m, belief, not_dead, False)
    return m


ORIGINAL_DEAD = rp.compute_dead_squares


# DF3c-1: the planner may spend the reserve (remaining() < reserve instead of <=).
class ReservePredictor(rp.StepPredictor):
    pass


def search_spending_reserve(root, m, goal_test, heuristic_fn, predictor, reserve):
    return ORIGINAL_SEARCH(root, m, goal_test, heuristic_fn, predictor, reserve - 1)


ORIGINAL_SEARCH = rp.search


# DF3d-1: the investigation checks an object candidate against the OLDEST observation
# (history.front()) instead of o_t-1.
def history_checks_oldest(candidates, history, before):
    return ORIGINAL_HISTORY_CHECKS(candidates, history, history[0])


ORIGINAL_HISTORY_CHECKS = rc._history_checks_pass


# DF3e-1: the reserve forgets the pending corrections (always 1).
def reserve_always_one(self):
    return 1


# DF3f-1: C0 searches again at every decision step.
ORIGINAL_C0_DECIDE = ra.C0.decide


def c0_always_searching(self, observation):
    self.plan = []
    return ORIGINAL_C0_DECIDE(self, observation)


# DF3g-1: after a consequence mismatch, the trap V2 test waits for the next decision step
# instead of being tried at once (plan 3.7 B, 3.8).
def handle_exhausted_waiting(self, m):
    original = self.attempt_trap_v2
    self.attempt_trap_v2 = lambda cid, cell: setattr(self, 'waiting_trap', (cid, cell))
    try:
        ORIGINAL_EXHAUSTED(self, m)
    finally:
        self.attempt_trap_v2 = original


ORIGINAL_EXHAUSTED = ra.C1.handle_exhausted


# DF3g-2 (the owner): the active-test plan line says "check" instead of "test".
def active_check_line(cid, k):
    return 'Plan: active check of correction %d (%d actions)' % (cid, k)


def run(fast):
    out = []
    with patched(rp, 'compute_footprint', footprint_without_iii):
        out.append(('DF3a-1 (footprint without rule iii)', failing_at('3a')))
    with patched(rp, 'compute_dead_squares', dead_squares_wrong_order):
        out.append(('DF3b-1 (overrides in the other order)', failing_at('3b')))
    with patched(rp, 'search', search_spending_reserve):
        out.append(('DF3c-1 (search may spend the reserve)', failing_at('3c')))
    with patched(rc, '_history_checks_pass', history_checks_oldest):
        out.append(('DF3d-1 (object V1 against the oldest observation)', failing_at('3d')))
    with patched(ra.C1, 'reserve', reserve_always_one):
        out.append(('DF3e-1 (reserve always 1)', failing_at('3e')))
    with patched(ra.C0, 'decide', c0_always_searching):
        out.append(('DF3f-1 (C0 searches at every step)', failing_at('3f')))
    with patched(ra.C1, 'handle_exhausted', handle_exhausted_waiting):
        out.append(('DF3g-1 (trap V2 test waits a step)', failing_at('3g', gate=not fast)))
    with patched(ra, 'plan_active_test_line', active_check_line):
        out.append(('DF3g-2 (owner: "active check" in the plan line)', failing_at('3g', gate=not fast)))
    out.append(('baseline failing again', failing_at('3g', gate=not fast)))
    return out


if __name__ == '__main__':
    for name, bad in run('--fast' in sys.argv):
        print(name, 'failing:', bad)
