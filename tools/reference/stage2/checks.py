"""The Stage 2 checks (plan STAGE_02, Section 6), mirrored on the reference.

Each check has the id the C++ test uses. Running this file prints every result and
`failing: none (<n> checks)`. df_check.py mutates the reference and reports which ids fail."""
import os
import sys

import importlib.util

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import ref_model as rm  # noqa: E402

# The Stage 1 test layouts (DEMO, OPEN, at) come from tools/reference/stage1/checks.py,
# loaded by path so it cannot be confused with this file (same name).
_spec = importlib.util.spec_from_file_location(
    'stage1_checks', os.path.join(HERE, '..', 'stage1', 'checks.py'))
stage1_checks = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(stage1_checks)
import ref_prediction_reader as prd  # noqa: E402

rw = rm.rw
OPEN = stage1_checks.OPEN
DEMO = stage1_checks.DEMO
at = stage1_checks.at
GOLDEN = os.path.join(HERE, 'golden', 'prediction_golden.txt')


def parsed(rows):
    return rw.parse_world(rows)[0]


def check_budget():
    c = {}
    b = rm.BudgetCounter(3)
    c['BC1'] = b.used == 0 and b.remaining() == 3
    results = [b.try_use() for _ in range(4)]
    c['BC2'] = results == [True, True, True, False] and b.used == 3
    b.reset()
    c['BC3'] = b.used == 0 and b.try_use()
    z = rm.BudgetCounter(0)
    c['BC4a'] = not z.try_use() and z.used == 0 and z.remaining() == 0
    n = rm.BudgetCounter(-2)
    c['BC4b'] = not n.try_use() and n.used == 0 and n.remaining() == 0
    return c


def check_model():
    c = {}
    truth = parsed(OPEN)
    for tag, action in zip('abcd', rw.ACTIONS):
        p = rm.WorldModel().predict(truth.copy(), action, rm.BudgetCounter(1))
        state, outcome = rw.apply_action(truth, action)
        c['WM1' + tag] = p is not None and p[1] == outcome and p[2].same(state)
    b = rm.BudgetCounter(5)
    rm.WorldModel().predict(truth, 'North', b)
    rm.WorldModel().predict(truth, 'East', b)
    c['WM2'] = b.used == 2
    b = rm.BudgetCounter(1)
    b.try_use()
    belief = truth.copy()
    p = rm.WorldModel().predict(belief, 'East', b)
    c['WM3'] = p is None and b.used == 1 and belief.same(truth)
    belief = truth.copy()
    rm.WorldModel().predict(belief, 'East', rm.BudgetCounter(1))
    c['WM4'] = belief.same(truth)
    bad = at(OPEN, (0, 4), (7, 7))
    b = rm.BudgetCounter(1)
    p = rm.WorldModel().predict(bad, 'East', b)
    c['WM5'] = p is not None and p[1] == 'InvalidState' and p[2].same(bad) and b.used == 1
    always_blocked = rm.WorldModel(lambda w, a: (w, 'Blocked'))
    p = always_blocked.predict(truth, 'East', rm.BudgetCounter(1))
    c['WM6'] = p[1] == 'Blocked' and p[2].same(truth)
    p = rm.WorldModel().predict(truth, 'West', rm.BudgetCounter(1))
    c['WM7'] = p[0] == 'West'
    same = True
    for action in rw.ACTIONS:
        a = rm.WorldModel(None).predict(truth, action, rm.BudgetCounter(1))
        d = rm.WorldModel().predict(truth, action, rm.BudgetCounter(1))
        same = same and a[1] == d[1] and a[2].same(d[2])
    c['WM8'] = same
    return c


def changed(rows, agent=None, obj=None, dest=None, walls=()):
    w = parsed(rows).copy()
    if agent:
        w.agent = agent
    if obj:
        w.object = obj
    if dest:
        w.destination = dest
    for (x, y) in walls:
        w.walls[y][x] = not w.walls[y][x]
    assert rw.is_valid(w)
    return w


def check_compare():
    c = {}
    base = parsed(OPEN)
    d = rm.compare_states(base, base.copy())
    c['CM1'] = not rm.has_differences(d)
    d = rm.compare_states(base, changed(OPEN, agent=(4, 3)))
    c['CM2'] = d == ((True, (4, 4), (4, 3)), (False, (7, 7), (7, 7)),
                     (False, (1, 8), (1, 8)), [])
    d = rm.compare_states(base, changed(OPEN, obj=(7, 6)))
    c['CM3'] = d == ((False, (4, 4), (4, 4)), (True, (7, 7), (7, 6)),
                     (False, (1, 8), (1, 8)), [])
    d = rm.compare_states(base, changed(OPEN, dest=(2, 8)))
    c['CM4'] = d == ((False, (4, 4), (4, 4)), (False, (7, 7), (7, 7)),
                     (True, (1, 8), (2, 8)), [])
    walled = changed(OPEN, walls=[(5, 5)])
    c['CM5a'] = rm.compare_states(walled, base)[3] == [((5, 5), True, False)]
    c['CM5b'] = rm.compare_states(base, walled)[3] == [((5, 5), False, True)]
    d = rm.compare_states(changed(OPEN, walls=[(5, 4), (4, 5)]), changed(OPEN, agent=(4, 3)))
    c['CM6'] = (d[0] == (True, (4, 4), (4, 3)) and not d[1][0] and not d[2][0]
                and d[3] == [((5, 4), True, False), ((4, 5), True, False)])
    prediction = ('East', 'Blocked', base)
    moved_state, moved_outcome = rw.apply_action(base, 'East')
    comp = rm.compare_prediction(prediction, moved_state, moved_outcome)
    c['CM7'] = comp[0] != comp[1] and not rm.matches(comp)
    comp = rm.compare_prediction(('East', 'Moved', moved_state), moved_state, 'Moved')
    c['CM8'] = rm.matches(comp)
    comp = rm.compare_prediction(('East', 'Moved', changed(OPEN, walls=[(5, 5)])),
                                 base, 'Moved')
    c['CM9'] = comp[0] == comp[1] and not rm.matches(comp)
    observed = changed(OPEN, agent=(4, 3), obj=(7, 6), dest=(2, 8), walls=[(5, 4), (4, 5)])
    c['CM10'] = rm.describe_differences(rm.compare_states(base, observed)) == CM10_LINES
    return c


CM10_LINES = [
    '  - agent: predicted (4, 4), observed (4, 3)',
    '  - object: predicted (7, 7), observed (7, 6)',
    '  - destination: predicted (1, 8), observed (2, 8)',
    '  - wall at (5, 4): predicted free, observed blocked',
    '  - wall at (4, 5): predicted free, observed blocked',
]

E = ['case 1 action East differences 0']
READER_CASES = {
    'PN1': (['case 1 action Up differences 0'] + DEMO + DEMO + ['result Moved Moved match'],
            (False, 0, 'line 1: invalid case header')),
    'PN2': (E + DEMO[:5], (False, 0, 'case 1: missing world lines')),
    'PN3': (E + ['####.#####'] + DEMO[1:] + DEMO + ['result Moved Moved match'],
            (False, 0, 'case 1: belief does not parse: Every border cell must be a wall (#).')),
    'PN4': (E + DEMO + DEMO, (False, 0, 'case 1: missing result line')),
    'PN5': (E + DEMO + DEMO + ['outcome Moved Moved match'],
            (False, 0, 'case 1: malformed result line')),
    'PN6': (['case 1 action East differences 1'] + DEMO + DEMO + ['result Moved Moved match'],
            (False, 0, 'case 1: missing difference lines')),
    'PN7': (E + DEMO + DEMO + ['result Moved Moved match'], (True, 1, '')),
}


def check_reader():
    return {cid: prd.check_prediction_golden_lines(lines) == expected
            for cid, (lines, expected) in READER_CASES.items()}


def golden_ok():
    with open(GOLDEN) as f:
        lines = f.read().split('\n')
    if lines and lines[-1] == '':
        lines = lines[:-1]
    ok, cases, _ = prd.check_prediction_golden_lines(lines)
    return ok and cases == 1000


DEMO2_TEXT = (
    '\nPrediction demo 1: correct belief\n'
    'Step 1: East | predicted Moved, observed Moved | match\n'
    'Step 2: East | predicted Pushed, observed Pushed | match\n'
    'Step 3: East | predicted Pushed, observed Pushed | match\n'
    'Step 4: East | predicted Pushed, observed Pushed | match\n'
    'Step 5: East | predicted Blocked, observed Blocked | match\n'
    'Mismatches: 0\nModel evaluations used: 5\n'
    '\nPrediction demo 2: belief wrongly includes a wall at (6, 3)\n'
    'Step 1: East | predicted Moved, observed Moved | MISMATCH\n'
    '  - wall at (6, 3): predicted blocked, observed free\n'
    'Step 2: East | predicted Pushed, observed Pushed | match\n'
    'Step 3: East | predicted Pushed, observed Pushed | match\n'
    'Step 4: East | predicted Pushed, observed Pushed | match\n'
    'Step 5: East | predicted Blocked, observed Blocked | match\n'
    'Mismatches: 1\nModel evaluations used: 5\n'
    'Note: in these demos the belief is replaced by the observation after every step.\n'
    'How beliefs are corrected is decided in Stage 3.\n')


def all_checks():
    c = {}
    c.update(check_budget())
    c.update(check_model())
    c.update(check_compare())
    c['G3'] = golden_ok()
    c.update(check_reader())
    c['D2'] = rm.prediction_demo_text() == DEMO2_TEXT
    return c


def failing():
    return [k for k, v in all_checks().items() if not v]


if __name__ == '__main__':
    results = all_checks()
    for k, v in results.items():
        print(k, 'PASS' if v else 'FAIL')
    bad = failing()
    print('failing:', bad if bad else 'none', f'({len(results)} checks)')
