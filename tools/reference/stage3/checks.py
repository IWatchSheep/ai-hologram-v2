"""The Stage 3 checks (plan STAGE_03, Section 6), mirrored on the reference.

Each check has the id the C++ test uses. Running this file prints every result and
`failing: none (<n> checks)`. df_check.py mutates the reference and reports which ids fail.
`python3 checks.py --fast` skips the two gate checks G4 and G5 (they take minutes)."""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import ref_agents as ra  # noqa: E402
import ref_verify as rc  # noqa: E402
import ref_parts as rp  # noqa: E402
import ref_trace as rt  # noqa: E402
import ref_trace_reader as rr  # noqa: E402

rw = rp.rw
GOLDEN = os.path.join(HERE, 'golden')

D = ['##########', '#........#', '#........#', '#..A.O..D#', '#........#',
     '#........#', '#...##...#', '#........#', '#........#', '##########']
LP = ['##########', '#....D##.#', '#.O......#', '#....A#..#', '#.#......#',
      '#.....##.#', '#........#', '#........#', '#........#', '##########']
L2 = ['##########', '#....#...#', '#.O..#.A.#', '#....#...#', '#D...#...#',
      '######...#', '#........#', '#........#', '#........#', '##########']
LT = ['##########', '#O.......#', '#........#', '#........#', '#....A...#',
      '#........#', '#........#', '#.......D#', '#........#', '##########']
LG = ['##########', '#........#', '#........#', '#..A....*#', '#........#',
      '#........#', '#...##...#', '#........#', '#........#', '##########']
LTA = ['##########', '#O...A...#', '#........#', '#........#', '#........#',
       '#........#', '#........#', '#.......D#', '#........#', '##########']
BOXED = ['##########', '#.#......#', '##A#.....#', '#.#..O..D#', '#........#',
         '#........#', '#...##...#', '#........#', '#........#', '##########']
SMALL = ['##########', '#........#', '#........#', '#..AO.D..#', '#........#',
         '#........#', '#........#', '#........#', '#........#', '##########']


def world(rows):
    w = rw.parse_world(rows)[0]
    assert w is not None
    return w


def demo():
    return world(D)


def edit(w, agent=None, obj=None, walls=(), free=()):
    c = w.copy()
    if agent is not None:
        c.agent = agent
    if obj is not None:
        c.object = obj
    for x, y in walls:
        c.walls[y][x] = True
    for x, y in free:
        c.walls[y][x] = False
    assert rw.is_valid(c), 'every fixture is a valid world'
    return c


def obs(step, state, action=None, outcome=None):
    return rc.Observation(step, state, action, outcome)


def two_obs(true_before, action):
    after, outcome = rw.apply_action(true_before, action)
    return [obs(1, true_before), obs(2, after, action, outcome)]


def predictor(budget, model=None):
    p = rp.StepPredictor(model or rp.standard_rule, budget)
    p.start_step()
    return p


def counting(calls, base=None):
    def model(state, action):
        calls.append((state.agent, state.object, action))
        return (base or rp.standard_rule)(state, action)
    return model


# ---------------- 3a: footprint and log text ----------------
FP1_ACTIONS = ['East'] * 5 + ['North', 'West', 'West', 'West', 'South', 'South', 'East', 'East',
                              'North', 'North', 'North', 'North', 'West', 'West'] + ['South'] * 6 \
    + ['East', 'East', 'East', 'West', 'North']


def run_step(belief, truth, action):
    predicted = rw.apply_action(belief, action)
    observed = rw.apply_action(truth, action)
    cells = rp.compute_footprint(belief, action, observed[0], observed[1])
    comparison = rp.compare_footprint(predicted[0], predicted[1], observed[0], observed[1], cells)
    return cells, predicted[1], observed[1], comparison


def diff(cell, pw, pa, po, ow, oa, oo):
    return {'cell': cell, 'pw': pw, 'pa': pa, 'po': po, 'ow': ow, 'oa': oa, 'oo': oo}


FP_ROWS = {
    'FP2': (lambda: (demo(), demo()), 'North', [(3, 3), (3, 2)], 'Moved', 'Moved', []),
    'FP3': (lambda: (demo(), edit(demo(), walls=[(4, 3)])), 'East', [(3, 3), (4, 3)], 'Moved', 'Blocked',
            [diff((3, 3), False, False, False, False, True, False),
             diff((4, 3), False, True, False, True, False, False)]),
    'FP4': (lambda: (edit(demo(), walls=[(4, 3)]), demo()), 'East', [(3, 3), (4, 3)], 'Blocked', 'Moved',
            [diff((3, 3), False, True, False, False, False, False),
             diff((4, 3), True, False, False, False, True, False)]),
    'FP5': (lambda: (edit(demo(), agent=(4, 3), walls=[(6, 3)]), edit(demo(), agent=(4, 3))), 'East',
            [(4, 3), (5, 3), (6, 3)], 'Blocked', 'Pushed',
            [diff((4, 3), False, True, False, False, False, False),
             diff((5, 3), False, False, True, False, True, False),
             diff((6, 3), True, False, False, False, False, True)]),
    'FP6': (lambda: (edit(demo(), agent=(4, 3), obj=(5, 4)), edit(demo(), agent=(4, 3))), 'East',
            [(4, 3), (5, 3), (6, 3)], 'Moved', 'Pushed', [diff((6, 3), False, False, False, False, False, True)]),
    'FP7': (lambda: (edit(demo(), agent=(4, 3), obj=(5, 4), walls=[(6, 3)]),
                     edit(demo(), agent=(4, 3), walls=[(6, 3)])), 'East',
            [(4, 3), (5, 3), (6, 3)], 'Moved', 'Blocked',
            [diff((4, 3), False, False, False, False, True, False),
             diff((5, 3), False, True, False, False, False, True)]),
    'FP8': (lambda: (edit(demo(), agent=(4, 3)), edit(demo(), agent=(4, 3), obj=(5, 4))), 'East',
            [(4, 3), (5, 3), (6, 3)], 'Pushed', 'Moved', [diff((6, 3), False, False, True, False, False, False)]),
    'FP9': (lambda: (demo(), edit(demo(), agent=(3, 4))), 'North', [(3, 3), (3, 2)], 'Moved', 'Moved',
            [diff((3, 3), False, False, False, False, True, False),
             diff((3, 2), False, True, False, False, False, False)]),
}


def check_footprint():
    c = {}
    belief = demo()
    truth = demo()
    every_step = True
    counts = {'Moved': 0, 'Pushed': 0, 'Blocked': 0, 'InvalidState': 0}
    for action in FP1_ACTIONS:
        after, outcome = rw.apply_action(truth, action)
        rp.apply_footprint_update(belief, rp.compute_footprint(belief, action, after, outcome), after)
        every_step = every_step and belief.same(after)
        counts[outcome] += 1
        truth = after.copy()
    c['FP1'] = every_step and (counts['Moved'], counts['Pushed'], counts['Blocked']) == (23, 3, 4)
    for cid, (make, action, cells, pout, oout, diffs) in FP_ROWS.items():
        b, t = make()
        got = run_step(b, t, action)
        c[cid] = got[0] == cells and got[1] == pout and got[2] == oout and got[3] == (pout != oout, diffs)
    belief = edit(demo(), walls=[(2, 7)])
    cells, _, _, comparison = run_step(belief, demo(), 'East')
    after = rw.apply_action(demo(), 'East')[0]
    rp.apply_footprint_update(belief, cells, after)
    c['FP10'] = (cells == [(3, 3), (4, 3)] and comparison == (False, [])
                 and belief.agent == after.agent and belief.object == after.object
                 and all(rw.is_wall(belief, x) == rw.is_wall(after, x) for x in cells)
                 and rw.is_wall(belief, (2, 7)))
    a = edit(demo(), agent=(4, 3))
    b = edit(demo(), agent=(4, 3), obj=(5, 4))
    oa = rw.apply_action(a, 'East')[1]
    ob = rw.apply_action(b, 'East')[1]
    cells = rp.footprint_for_two(a, oa, b, ob, 'East')
    c['FP11a'] = (oa, ob) == ('Pushed', 'Moved') and cells == [(4, 3), (5, 3), (6, 3)] and \
        rp.footprint_for_two(b, ob, a, oa, 'East') == cells
    c['FP11b'] = rp.footprint_for_two(b, ob, b.copy(), ob, 'East') == [(4, 3), (5, 3)]
    outside = [(-1, 3), (10, 3), (3, -1), (3, 10)]
    copy = demo()
    rp.apply_footprint_update(copy, outside, demo())
    c['FP12a'] = (rp.compare_footprint(demo(), 'Moved', demo(), 'Moved', outside) == (False, [])
                  and all(rw.is_wall(demo(), x) for x in outside) and copy.same(demo()))
    west = edit(demo(), agent=(1, 3))
    after, outcome = rw.apply_action(west, 'West')
    c['FP12b'] = rp.compute_footprint(west, 'West', after, outcome) == [(1, 3), (0, 3)] and outcome == 'Blocked'
    pushed_after, pushed = rw.apply_action(edit(demo(), agent=(4, 3)), 'East')
    c['FP13'] = (rc.footprint_of(edit(demo(), agent=(4, 3)), 'East', obs(2, pushed_after, 'East', pushed))
                 == [(4, 3), (5, 3), (6, 3)]
                 and rc.observed_outcome(obs(2, demo())) == 'Blocked'
                 and rc.footprint_of(demo(), 'North', obs(2, demo())) == [(3, 3), (3, 2)])
    return c


def lg_lines():
    """The lines the log functions write now (called at check time)."""
    return {
        'LG1': [ra.plan_search_line(4), ra.plan_fallback_line(10), ra.plan_active_test_line(1, 2), ra.PLAN_CAUTIOUS],
        'LG2': [ra.step_line(1, 'East', 'Moved', 'Blocked', False), ra.step_line(12, 'South', 'Pushed', 'Pushed', True),
                ra.safe_stop_line(3, 'no safe action')],
        'LG3': [ra.verified_line(1), ra.rejected_line(12)],
        'LG4': [ra.investigation_wall_line((4, 3), False, True), ra.investigation_wall_line((6, 3), True, False),
                ra.investigation_object_line((5, 4), (5, 3)), ra.investigation_model_line('East'),
                ra.ANOMALY_AGENT, ra.DEFERRED_LINE],
        'LG5': [ra.correction_wall_line(1, (4, 3), True), ra.correction_wall_line(2, (6, 3), False),
                ra.correction_object_line(3, (5, 3)), ra.correction_model_line(4, 'West'),
                ra.correction_trap_line(5, (1, 1)), ra.UNEXPLAINED],
        'LG6': [ra.surprise_line((2, 2)), ra.FULL_STATE, ra.consequence_mismatch_line((1, 1)), ra.ANOMALY_INVALID],
        'LG7': ra.result_lines('Stopped -- unresolved', 0, 2, 1, 3, 2, 414),
    }

LG_EXPECTED = {
    'LG1': ['Plan: search found a plan of 4 actions',
            'Plan: fallback toward the best state found (10 actions)',
            'Plan: active test of correction 1 (2 actions)',
            'Plan: cautious mode (investigation unfinished)'],
    'LG2': ['Step 1: East | predicted Moved, observed Blocked | MISMATCH',
            'Step 12: South | predicted Pushed, observed Pushed | match',
            'Step 3: safe stop (no safe action)'],
    'LG3': ['  - correction 1: VERIFIED', '  - correction 12: REJECTED'],
    'LG4': ['  - investigation: wall at (4, 3) believed free, recorded blocked (source S2)',
            '  - investigation: wall at (6, 3) believed blocked, recorded free (source S2)',
            '  - investigation: object believed at (5, 4), recorded at (5, 3) (source S2)',
            '  - investigation: belief matched the record; action model for East suspected (source S1)',
            '  - anomaly: believed agent position differs from the record',
            '  - investigation: budget too small for verification; retried next step'],
    'LG5': ['  - correction 1: wall at (4, 3) set to blocked: CONSISTENT',
            '  - correction 2: wall at (6, 3) set to free: CONSISTENT',
            '  - correction 3: object position set to (5, 3): CONSISTENT',
            '  - correction 4: action model for West restored to the standard rule: CONSISTENT',
            '  - correction 5: (1, 1) classified as a dead square: CONSISTENT',
            '  - investigation: no candidate passed V1 (source S5); unexplained update'],
    'LG6': ['  - consequence surprise: search exhausted with the object at (2, 2)',
            '  - full-state dead end (dead-square classification consistent; no claim)',
            '  - consequence mismatch: (1, 1) violates the dead-square rule (source S1)',
            '  - anomaly: invalid hypothetical state skipped'],
    'LG7': ['Result: Stopped -- unresolved after 0 world steps, 2 safe stops', 'Mismatches: 1',
            'Corrections: 3 (2 verified)', 'Model evaluations used: 414'],
}


def check_log():
    lines = lg_lines()
    return {cid: lines[cid] == LG_EXPECTED[cid] for cid in lines}


# ---------------- 3b: dead squares, heuristic, observers H4 ----------------
DS2_GRID = [
    [-2, -2, -2, -2, -2, -2, -2, -2, -2, -2],
    [-2, -3, -3, -3, -3, -3, -3, -3, -3, -2],
    [-2, -3, 7, 6, 5, 4, 3, 2, 1, -2],
    [-2, -3, 6, 5, 4, 3, 2, 1, 0, -2],
    [-2, -3, 7, 6, 5, 4, 3, 2, 1, -2],
    [-2, -3, 8, 7, 6, 5, 4, 3, 2, -2],
    [-2, -3, 9, 8, -2, -2, 5, 4, 3, -2],
    [-2, -3, 10, 9, 8, 7, 6, 5, 4, -2],
    [-2, -3, -3, -3, -3, -3, -3, -3, -3, -2],
    [-2, -2, -2, -2, -2, -2, -2, -2, -2, -2]]
DS2_CELL_OPERATIONS = 160


class Recorder:
    """An observer that records every report (IE-1 H3, H4)."""

    def __init__(self):
        self.events = []
        self.on_evaluation = lambda s, a, variant, p: self.events.append(
            ('evaluation', s.agent, s.object, a, variant, p[1]))
        self.on_dead_read = lambda cell, value: self.events.append(('dead', cell, value))
        self.on_heuristic = lambda s, value: self.events.append(('heuristic', s.agent, s.object, value))


def check_dead_squares():
    c = {}
    counts = rp.Counts()
    m = rp.compute_dead_squares(demo(), [], [], counts)
    c['DS1'] = (m.dead[1][1] and not m.dead[3][8] and m.push[3][8] == 0
                and not any(m.dead[y][x] for y in range(10) for x in range(10) if demo().walls[y][x]))
    grid_ok = True
    dead_count = 0
    for y in range(10):
        for x in range(10):
            want = DS2_GRID[y][x]
            dead_count += m.dead[y][x]
            if want == -2:
                grid_ok = grid_ok and demo().walls[y][x] and not m.dead[y][x]
            elif want == -3:
                grid_ok = grid_ok and m.dead[y][x] and m.push[y][x] == -1
            else:
                grid_ok = grid_ok and not m.dead[y][x] and m.push[y][x] == want
    c['DS2a'] = grid_ok and dead_count == 22
    c['DS2b'] = counts.analysis_runs == 1 and counts.cell_operations == DS2_CELL_OPERATIONS
    m2 = rp.compute_dead_squares(world(L2), [], [], rp.Counts())
    c['DS3a'] = not m2.dead[2][2] and m2.push[2][2] == 3
    counts = rp.Counts()
    over = rp.compute_dead_squares(demo(), [(1, 1)], [], counts)
    c['DS4'] = (not rp.local_check_says_not_dead(demo(), over, (1, 1), counts)
                and rp.local_check_says_not_dead(demo(), over, (5, 3), counts)
                and rp.local_check_says_not_dead(demo(), over, (8, 3), counts) and counts.local_checks == 3)
    differ = [(x, y) for y in range(10) for x in range(10) if m.dead[y][x] != over.dead[y][x]]
    c['DS5a'] = differ == [(1, 1)] and m.push == over.push
    c['DS5b'] = rp.compute_dead_squares(demo(), [(1, 1)], [(1, 1)], rp.Counts()).dead[1][1]
    counts = rp.Counts()
    cache = rp.DeadSquareCache()
    runs = []
    for belief, not_dead in [(demo(), []), (demo(), []), (edit(demo(), agent=(2, 3)), []),
                             (edit(demo(), walls=[(2, 7)]), []), (edit(demo(), walls=[(2, 7)]), [(1, 1)]),
                             (edit(demo(), walls=[(2, 7)]), [(1, 1)])]:
        cache.get(belief, not_dead, [], counts)
        runs.append(counts.analysis_runs)
    c['DS6'] = runs == [1, 1, 1, 2, 3, 3]
    calls = []
    counts = rp.Counts()
    m = rp.compute_dead_squares(demo(), [], [], counts)
    c['DS7'] = (rp.local_check_says_not_dead(demo(), m, (5, 3), counts) and rp.heuristic(demo(), m, counts) == 4
                and calls == [])
    counts = rp.Counts()
    c['DS8a'] = rp.heuristic(demo(), m, counts) == 4 and rp.heuristic(edit(demo(), agent=(4, 3)), m, counts) == 3
    c['DS8b'] = rp.heuristic(edit(demo(), obj=(1, 1)), m, counts) == rp.INF
    over_counts = rp.Counts()
    c['DS8c'] = rp.heuristic(edit(demo(), agent=(5, 5), obj=(1, 1)), over, over_counts) == 16
    c['DS8d'] = counts.heuristic_calls == 3 and over_counts.heuristic_calls == 1
    rec = Recorder()
    m = rp.compute_dead_squares(demo(), [], [], rp.Counts())
    silent = rp.read_dead_status(m, (1, 1)), rp.read_dead_status(m, (5, 3))
    m.observer = rec
    watched = rp.read_dead_status(m, (1, 1)), rp.read_dead_status(m, (5, 3))
    c['OB1'] = silent == (True, False) and watched == (True, False) and \
        rec.events == [('dead', (1, 1), True), ('dead', (5, 3), False)]
    rec = Recorder()
    m.observer = rec
    counts = rp.Counts()
    rp.heuristic(demo(), m, counts)
    rp.heuristic(edit(demo(), obj=(1, 1)), m, counts)
    rp.local_check_says_not_dead(demo(), m, (2, 2), counts)
    c['OB2'] = rec.events == OB2_EVENTS
    cache = rp.DeadSquareCache()
    plain = cache.get(demo(), [], [], rp.Counts())
    plain_dead = [row[:] for row in plain.dead]
    plain_observer = plain.observer
    cache.observer = rec
    watched = cache.get(demo(), [], [], rp.Counts())
    c['OB3'] = plain_observer is None and watched.observer is rec and watched.dead == plain_dead
    return c


OB2_EVENTS = [('dead', (5, 3), False), ('heuristic', (3, 3), (5, 3), 4),
              ('dead', (1, 1), True), ('heuristic', (3, 3), (1, 1), rp.INF),
              ('dead', (2, 1), True), ('dead', (3, 2), False)]


# ---------------- 3c: step predictor, agent model, planner, observers H3 ----------------
def check_predictor():
    c = {}
    s = demo()
    p = predictor(10)
    first = p.predict(s, 'East')
    ok1 = first is not None and p.used_this_step() == 1 and p.remaining() == 9 and p.is_cached(s, 'East')
    second = p.predict(s, 'East')
    c['SP1'] = ok1 and second[2].same(first[2]) and second[1] == first[1] and p.used_this_step() == 1
    p.start_step()
    c['SP2a'] = not p.is_cached(s, 'East') and p.predict(s, 'East') is not None and p.used_this_step() == 1
    walled = edit(s, walls=[(2, 7)])
    ok = not p.is_cached(walled, 'East')
    p.predict(walled, 'East')
    ok = ok and p.used_this_step() == 2 and p.is_cached(walled, 'East')
    p.clear_cache()
    c['SP2b'] = ok and not p.is_cached(walled, 'East')
    p = predictor(5)
    p.predict(s, 'East')
    p.start_step()
    c['SP3'] = p.remaining() == 5 and p.used_this_step() == 0 and p.used_total == 1
    p = predictor(1)
    first = p.predict(s, 'East')
    refused = p.predict(s, 'South')
    c['SP4a'] = first is not None and refused is None and p.used_this_step() == 1 and p.remaining() == 0 \
        and not p.is_cached(s, 'South')
    c['SP4b'] = p.predict_variant(s, 'South', rp.standard_rule) is None and p.used_this_step() == 1
    blocked = lambda w, a: (w, 'Blocked')
    p = predictor(10)
    ordinary = p.predict(s, 'East')
    variant = p.predict_variant(s, 'East', blocked)
    again = p.predict_variant(s, 'East', blocked)
    cached = p.predict(s, 'East')
    south = p.predict_variant(s, 'South', blocked)
    c['SP5'] = (ordinary[1] == 'Moved' and variant[1] == 'Blocked' and again is not None
                and cached[1] == 'Moved' and south is not None and p.used_this_step() == 4
                and not p.is_cached(s, 'South'))
    calls = []
    p = predictor(10, counting(calls))
    p.predict(s, 'East')
    p.predict(s, 'East')
    p.predict_variant(s, 'East', counting(calls))
    c['SP6'] = len(calls) == 2 and p.used_this_step() == 2 and p.used_total == 2
    rec = Recorder()
    p = predictor(3)
    p.observer = rec
    p.predict(s, 'East')
    p.predict(s, 'East')
    p.predict_variant(s, 'North', blocked)
    p.predict(s, 'South')
    p.predict(s, 'West')
    c['OB4'] = rec.events == [('evaluation', (3, 3), (5, 3), 'East', False, 'Moved'),
                              ('evaluation', (3, 3), (5, 3), 'North', True, 'Blocked'),
                              ('evaluation', (3, 3), (5, 3), 'South', False, 'Moved')] and p.used_total == 3
    return c


def push_north(state, action):
    standard = rw.apply_action(state, action)
    if action != 'East' or standard[1] != 'Pushed':
        return standard
    wrong = standard[0].copy()
    wrong.object = rw.moved(state.object, 'North')
    return wrong, 'Pushed'


def check_agent_model():
    c = {}
    s = edit(demo(), agent=(4, 3))
    c['AM1'] = rp.AgentActionModel().current()(s, 'East')[0].object == (6, 3)
    m = rp.AgentActionModel(push_north)
    c['AM2'] = not m.is_restored('East') and m.current()(s, 'East')[0].object == (5, 2)
    snapshot = m.current()
    m.restore('East')
    c['AM3'] = (m.is_restored('East') and m.current()(s, 'East')[0].object == (6, 3)
                and m.base()(s, 'East')[0].object == (5, 2) and snapshot(s, 'East')[0].object == (5, 2))
    m.unrestore('East')
    c['AM4'] = not m.is_restored('East') and m.current()(s, 'East')[0].object == (5, 2)
    m = rp.AgentActionModel(push_north)
    m.restore('East')
    m.set_base(lambda w, a: (w, 'Blocked'))
    restored_kept = m.is_restored('East') and m.current()(s, 'East')[0].object == (6, 3)
    blocked_west = m.current()(s, 'West')[1] == 'Blocked'
    m.set_base(None)
    c['AM5'] = restored_kept and blocked_west and m.current()(s, 'West')[1] == 'Moved'
    return c


def goal_reached(s):
    return s.object == s.destination


def main_search(root, budget, reserve, model=None):
    counts = rp.Counts()
    m = rp.compute_dead_squares(root, [], [], counts)
    p = predictor(budget, model)
    result = rp.search(root, m, goal_reached, lambda s: rp.heuristic(s, m, counts), p, reserve)
    return result, p, m


def actions(plan):
    return [a for a, _ in plan]


def none_dead(result, m):
    return not any(m.dead[d.state.object[1]][d.state.object[0]] for d in result.discovered)


def active_walk(root, targets, budget=1000000):
    m = rp.compute_dead_squares(root, [], [], rp.Counts())

    def goal(s):
        return s.agent in targets

    def estimate(s):
        if m.dead[s.object[1]][s.object[0]]:
            return rp.INF
        return min(rp.manhattan(s.agent, t) for t in targets)
    p = predictor(budget)
    return rp.search(root, m, goal, estimate, p, 1), p, m


def check_planner():
    c = {}
    r1, p1, m1 = main_search(demo(), 200, 1)
    c['PL1'] = (r1.kind == 'Found' and actions(r1.plan) == ['East'] * 4 and p1.used_this_step() == 16
                and r1.plan[-1][1].object == (8, 3))
    r2, p2, m2 = main_search(world(LP), 1000000, 1)
    c['PL2a'] = (r2.kind == 'Found' and actions(r2.plan) == ['West', 'West', 'West', 'North', 'West', 'North',
                                                             'East', 'East', 'East']
                 and p2.used_this_step() == 60 and r2.improved == 1 and r2.reopened == 0)
    rs, _, ms = main_search(world(SMALL), 200, 1)
    c['PL2b'] = rs.kind == 'Found' and actions(rs.plan) == ['East', 'East']
    d2 = edit(demo(), walls=[(4, 3)])
    a, pa, _ = main_search(d2, 10, 1)
    b, pb, _ = main_search(d2, 10, 3)
    c['PL4a'] = a.kind == 'OutOfBudget' and pa.used_this_step() == 9 and pa.remaining() == 1
    c['PL4b'] = b.kind == 'OutOfBudget' and pb.used_this_step() == 7 and pb.remaining() == 3
    tiny, _, _ = main_search(d2, 1, 1)
    c['PL5a'] = actions(rp.fallback(a)) == ['North', 'East'] and actions(rp.fallback(b)) == ['North', 'East']
    c['PL5b'] = tiny.kind == 'OutOfBudget' and len(tiny.discovered) == 1 and rp.fallback(tiny) == []
    r6, p6, m6 = main_search(world(L2), 1000000, 1)
    c['PL6'] = r6.kind == 'Exhausted' and p6.used_this_step() == 156 and len(r6.discovered) == 39
    r7, p7, m7 = active_walk(edit(d2, agent=(7, 1)), [(4, 4), (3, 3), (4, 2)])
    c['PL7'] = (r7.kind == 'Found' and actions(r7.plan) == ['South', 'West', 'West', 'West']
                and r7.plan[-1][1].agent == (4, 2) and p7.used_this_step() == 16)
    c['PL3'] = none_dead(r1, m1) and none_dead(r2, m2) and none_dead(rs, ms) and none_dead(r6, m6) and \
        none_dead(r7, m7)
    same = True
    for root in (demo(), world(LP)):
        x, px, _ = main_search(root, 1000000, 1)
        y, py, _ = main_search(root, 1000000, 1)
        same = same and x.kind == y.kind and px.used_this_step() == py.used_this_step() and \
            [(act, st.agent, st.object) for act, st in x.plan] == [(act, st.agent, st.object) for act, st in y.plan]
    c['PL8'] = same
    match = True
    for root in (demo(), world(LP)):
        calls = []
        x, px, _ = main_search(root, 1000000, 1, counting(calls))
        match = match and x.kind == 'Found' and len(calls) == px.used_this_step()
    c['PL9'] = match
    lg = world(LG)
    mg = rp.compute_dead_squares(lg, [], [], rp.Counts())
    rg, pg, _ = main_search(lg, 200, 1)
    c['PL10'] = rp.heuristic(lg, mg, rp.Counts()) == 4 and rg.kind == 'Found' and rg.plan == [] and \
        pg.used_this_step() == 0
    return c


# ---------------- 3d: records, investigation, verification ----------------
def candidates_text(result):
    out = []
    for cand in result.candidates:
        if cand.element.kind == 'wall':
            out.append(('wall', cand.element.cell, cand.blocked))
        else:
            out.append(('object', cand.position))
    return out


def investigate(belief, true_before, action, model=None, budget=200, reserve=1, history=None):
    p = predictor(budget)
    result = rc.investigate_mismatch(belief, action, history or two_obs(true_before, action), p,
                                     model or rp.standard_rule, reserve)
    return result, p


def always_moved_east(state, action):
    if action != 'East':
        return rw.apply_action(state, action)
    n = state.copy()
    n.agent = rw.moved(state.agent, 'East')
    return n, 'Moved'


def east_history():
    h = [obs(1, demo())]
    for step in range(2, 5):
        after, outcome = rw.apply_action(h[-1].state, 'East')
        h.append(obs(step, after, 'East', outcome))
    return h


def wall_record():
    r = rc.Correction()
    r.id = 1
    r.element = rc.Element('wall', (4, 3))
    r.new_blocked = True
    r.old_blocked = False
    r.step_consistent = 1
    return r


def model_record():
    r = rc.Correction()
    r.id = 2
    r.element = rc.Element('model', (0, 0), 'East')
    r.step_consistent = 1
    return r


def object_record():
    r = rc.Correction()
    r.id = 3
    r.element = rc.Element('object')
    r.new_position = (5, 3)
    r.old_position = (5, 4)
    r.step_consistent = 1
    return r


def check_investigation():
    c = {}
    r, p = investigate(demo(), edit(demo(), walls=[(4, 3)]), 'East')
    c['IV1'] = (r.outcome == 'BeliefCandidatesConsistent' and candidates_text(r) == [('wall', (4, 3), True)]
                and r.suspected == 'S2' and not r.anomaly and p.used_this_step() == 1)
    t2 = edit(demo(), agent=(4, 3))
    r, _ = investigate(edit(t2, walls=[(6, 3)]), t2, 'East')
    c['IV2'] = r.outcome == 'BeliefCandidatesConsistent' and candidates_text(r) == [('wall', (6, 3), False)] \
        and r.suspected == 'S2'
    r, _ = investigate(edit(demo(), agent=(4, 3)), edit(demo(), agent=(4, 3), obj=(5, 4)), 'East')
    c['IV3'] = r.outcome == 'BeliefCandidatesConsistent' and candidates_text(r) == [('object', (5, 4))] \
        and r.suspected == 'S2'
    t4 = edit(demo(), agent=(4, 3))
    r, _ = investigate(edit(t4, obj=(5, 4)), t4, 'East')
    c['IV4'] = r.outcome == 'BeliefCandidatesConsistent' and candidates_text(r) == [('object', (5, 3))] \
        and r.suspected == 'S2'
    r, p = investigate(t4, t4, 'East', model=push_north)
    c['IV5'] = r.outcome == 'ModelSuspected' and r.candidates == [] and r.suspected == 'S1' and \
        p.used_this_step() == 0
    r, p = investigate(edit(t4, obj=(5, 4), walls=[(6, 3)]), t4, 'East')
    c['IV6'] = (r.outcome == 'BeliefCandidatesConsistent'
                and candidates_text(r) == [('wall', (6, 3), False), ('object', (5, 3))]
                and r.suspected == 'S2' and p.used_this_step() == 1)
    r, _ = investigate(demo(), edit(demo(), walls=[(4, 3)]), 'East', model=always_moved_east)
    c['IV7'] = r.outcome == 'NoCandidatePassed' and r.suspected == 'S5'
    r, _ = investigate(demo(), edit(demo(), agent=(3, 4)), 'North')
    c['IV9'] = r.anomaly
    r, p = investigate(demo(), edit(demo(), walls=[(4, 3)]), 'East', budget=1, reserve=1)
    c['IV10'] = r.outcome == 'Deferred' and p.used_this_step() == 0
    r, p = investigate(edit(demo(), agent=(4, 3), obj=(5, 4)), edit(demo(), agent=(4, 3), obj=(4, 4)), 'East',
                       history=[obs(1, edit(demo(), agent=(4, 3), obj=(5, 3))),
                                obs(2, edit(demo(), agent=(4, 3), obj=(5, 3)), 'East', 'Blocked')])
    c['IV11'] = r.outcome == 'NoCandidatePassed' and r.suspected == 'S5' and p.used_this_step() == 1
    h = east_history()
    o2 = h[2]
    c['VF1'] = rc.object_history_check((6, 3), o2) and not rc.object_history_check((5, 3), o2)
    r, _ = investigate(edit(o2.state, obj=(6, 4)), None, 'East', history=h)
    c['VF2'] = r.outcome == 'BeliefCandidatesConsistent' and candidates_text(r) == [('object', (6, 3))]
    t = edit(demo(), walls=[(4, 3)])
    hist = two_obs(t, 'East')
    c['VF3a'] = rc.wall_history_check((4, 3), True, hist)
    lacking = [obs(1, demo()), hist[1]]
    c['VF3b'] = not rc.wall_history_check((4, 3), True, lacking)
    work = rc.start_model_v1('East', h)
    c['VF4a'] = work.pending == [1, 2, 3] and rc.start_model_v1('West', h).pending == []
    calls = []
    p = predictor(200, counting(calls))
    c['VF4b'] = rc.continue_model_v1(work, h, p, 1) == 'Done' and p.used_this_step() == 3 and calls == []
    altered = list(h)
    altered[2] = obs(3, edit(h[2].state, obj=(6, 4)), 'East', 'Pushed')
    c['VF4c'] = rc.continue_model_v1(rc.start_model_v1('East', altered), altered, predictor(200), 1) == 'Failed'
    work = rc.start_model_v1('East', h)
    p = predictor(3)
    first = rc.continue_model_v1(work, h, p, 1)
    ok = first == 'Pending' and p.used_this_step() == 2 and work.pending == [3]
    p.start_step()
    c['VF5'] = ok and rc.continue_model_v1(work, h, p, 1) == 'Done' and p.used_this_step() == 1
    east, north = predictor(200), predictor(200)
    wall_east = rc.alternative_prediction(wall_record(), demo(), 'East', east, rp.standard_rule, rp.standard_rule)
    wall_north = rc.alternative_prediction(wall_record(), demo(), 'North', north, rp.standard_rule,
                                           rp.standard_rule)
    c['VF7a'] = wall_east is not None and east.used_this_step() == 1 and wall_north is None and \
        north.used_this_step() == 0
    east, west = predictor(200), predictor(200)
    model_east = rc.alternative_prediction(model_record(), demo(), 'East', east, rp.standard_rule,
                                           rp.standard_rule)
    model_west = rc.alternative_prediction(model_record(), demo(), 'West', west, rp.standard_rule,
                                           rp.standard_rule)
    c['VF7b'] = model_east is not None and east.used_this_step() == 1 and model_west is None and \
        west.used_this_step() == 0
    b = edit(demo(), agent=(4, 4), obj=(5, 3))
    near, far = predictor(200), predictor(200)
    obj_near = rc.alternative_prediction(object_record(), b, 'East', near, rp.standard_rule, rp.standard_rule)
    obj_far = rc.alternative_prediction(object_record(), b, 'West', far, rp.standard_rule, rp.standard_rule)
    c['VF7c'] = (obj_near is not None and obj_near[1] == 'Pushed' and obj_near[2].agent == (5, 4)
                 and obj_near[2].object == (6, 4)
                 and near.used_this_step() == 1 and obj_far is None and far.used_this_step() == 0)
    calls = []
    r, p = investigate(demo(), edit(demo(), walls=[(4, 3)]), 'East', model=counting(calls))
    c['VF8a'] = r.outcome == 'BeliefCandidatesConsistent' and len(calls) == 1 and p.used_this_step() == 1
    old_calls, current_calls = [], []
    p = predictor(200)
    alt = rc.alternative_prediction(model_record(), demo(), 'East', p, counting(current_calls), counting(old_calls))
    c['VF8b'] = alt is not None and len(old_calls) == 1 and current_calls == [] and p.used_this_step() == 1
    base = ('East', 'Moved', demo())
    c['VF8c'] = (not rp.predictions_differ(base, base) and rp.predictions_differ(base, ('East', 'Blocked', demo()))
                 and rp.predictions_differ(base, ('East', 'Moved', edit(demo(), agent=(4, 3)))))
    lt = world(LT)
    counts = rp.Counts()
    trap_map = rp.compute_dead_squares(lt, [(1, 1)], [], counts)
    history = [obs(1, lt)]
    c['VF9a'] = rc.trap_v1_check(lt, trap_map, (1, 1), history, counts)
    calls = []
    p = predictor(200)
    result, anomalies = rc.trap_v2_test(lt, trap_map, (1, 1), p, counting(calls), 1, 0)
    c['VF9b'] = result == 'Verified' and p.used_this_step() == 2 and [a for _, _, a in calls] == ['North', 'West']
    tight = predictor(2)
    waiting, _ = rc.trap_v2_test(lt, trap_map, (1, 1), tight, rp.standard_rule, 1, 0)
    c['VF9c'] = waiting == 'Waiting' and tight.used_this_step() == 0
    dm = rp.compute_dead_squares(demo(), [], [], rp.Counts())
    rejected, _ = rc.trap_v2_test(demo(), dm, (5, 3), predictor(200), rp.standard_rule, 1, 0)
    c['VF9d'] = not rc.trap_v1_check(demo(), dm, (5, 3), [obs(1, demo())], rp.Counts()) and rejected == 'Rejected'
    c['VF10'] = anomalies == 0 and rw.is_valid(edit(lt, agent=(1, 2))) and rw.is_valid(edit(lt, agent=(2, 1)))
    l2 = world(L2)
    m2 = rp.compute_dead_squares(l2, [], [], rp.Counts())
    c['VF11'] = rc.check_consequence(l2, m2, [obs(1, l2)], rp.Counts()) == 'FullStateDeadEnd'
    c['VF12'] = rc.check_consequence(lt, trap_map, history, rp.Counts()) == 'Mismatch'
    later = edit(lt, agent=(6, 7), obj=(8, 7))
    c['VF13'] = (rc.check_consequence(lt, trap_map, [obs(1, lt), obs(2, later)], rp.Counts()) == 'NotApplicable'
                 and rc.check_consequence(lt, rp.compute_dead_squares(lt, [], [], rp.Counts()), history,
                                          rp.Counts()) == 'NotApplicable'
                 and rc.check_consequence(world(LG), rp.compute_dead_squares(world(LG), [], [], rp.Counts()),
                                          [obs(1, world(LG))], rp.Counts()) == 'NotApplicable')
    c['HC1'] = (rc.trap_history_passes((1, 1), (8, 7), [obs(1, lt)])
                and not rc.trap_history_passes((1, 1), (8, 7), [obs(1, lt), obs(2, later)])
                and rc.trap_history_passes((1, 1), (8, 7), [obs(1, later), obs(2, lt)]))
    belief = edit(demo(), walls=[(4, 3)])
    c['AT1'] = rc.active_test_targets(belief, (4, 3)) == [((4, 4), 'North'), ((3, 3), 'East'), ((4, 2), 'South')]
    r = rc.Correction()
    c['RC1'] = (r.id == 0 and r.element.kind == 'wall' and r.status == 'CONSISTENT' and not r.tested
                and r.step_consistent == 0 and rc.InvestigationResult().outcome == 'ModelSuspected'
                and rc.InvestigationResult().suspected == 'S5')
    return c


# ---------------- 3e: C1 and the run loop ----------------
def settings(b=200, s=5, t=100, v=3):
    return ra.Settings(b, s, t, v)


def run_lines(truth, agent, limits):
    return ra.c1_run_lines(truth, agent, limits)[2]


def new_c1(truth, limits, belief=None, model=None, not_dead=()):
    return ra.C1(obs(1, truth), limits, belief, model, not_dead)


C1_1 = ['Plan: search found a plan of 4 actions',
        'Step 1: East | predicted Moved, observed Moved | match',
        'Step 2: East | predicted Pushed, observed Pushed | match',
        'Step 3: East | predicted Pushed, observed Pushed | match',
        'Step 4: East | predicted Pushed, observed Pushed | match',
        'Result: Success after 4 world steps, 0 safe stops', 'Mismatches: 0', 'Corrections: 0 (0 verified)',
        'Model evaluations used: 19']
C1_2 = ['Plan: search found a plan of 4 actions',
        'Step 1: East | predicted Moved, observed Blocked | MISMATCH',
        '  - investigation: wall at (4, 3) believed free, recorded blocked (source S2)',
        '  - correction 1: wall at (4, 3) set to blocked: CONSISTENT',
        'Plan: fallback toward the best state found (10 actions)',
        'Step 2: North | predicted Moved, observed Moved | match',
        'Step 3: East | predicted Moved, observed Moved | match',
        'Step 4: East | predicted Moved, observed Moved | match',
        'Plan: active test of correction 1 (2 actions)',
        'Step 5: West | predicted Moved, observed Moved | match',
        'Step 6: South | predicted Blocked, observed Blocked | match',
        '  - correction 1: VERIFIED',
        'Plan: search found a plan of 12 actions',
        'Step 7: East | predicted Moved, observed Moved | match',
        'Step 8: South | predicted Pushed, observed Pushed | match',
        'Step 9: South | predicted Pushed, observed Pushed | match',
        'Step 10: West | predicted Moved, observed Moved | match',
        'Step 11: South | predicted Moved, observed Moved | match',
        'Step 12: East | predicted Pushed, observed Pushed | match',
        'Step 13: East | predicted Pushed, observed Pushed | match',
        'Step 14: East | predicted Pushed, observed Pushed | match',
        'Step 15: South | predicted Moved, observed Moved | match',
        'Step 16: East | predicted Moved, observed Moved | match',
        'Step 17: North | predicted Pushed, observed Pushed | match',
        'Step 18: North | predicted Pushed, observed Pushed | match',
        'Result: Success after 18 world steps, 0 safe stops', 'Mismatches: 1', 'Corrections: 1 (1 verified)',
        'Model evaluations used: 414']
C1_5 = ['Step 1: safe stop (no safe action)', 'Step 2: safe stop (no safe action)',
        'Result: Stopped -- unresolved after 0 world steps, 2 safe stops', 'Mismatches: 0',
        'Corrections: 0 (0 verified)', 'Model evaluations used: 8']
C1_6C_2 = ['Step 1: safe stop (investigation unfinished, no cautious action)',
           'Step 2: safe stop (investigation unfinished, no cautious action)',
           'Result: Stopped -- unresolved after 0 world steps, 2 safe stops', 'Mismatches: 0',
           'Corrections: 0 (0 verified)', 'Model evaluations used: 2']
C1_6C_5 = ['Step 1: safe stop (investigation unfinished, no cautious action)',
           'Step 2: safe stop (investigation unfinished, no cautious action)',
           '  - correction 1: action model for East restored to the standard rule: CONSISTENT',
           'Step 3: safe stop (budget too small to plan)', 'Step 4: safe stop (budget too small to plan)',
           'Step 5: safe stop (budget too small to plan)',
           'Result: Stopped -- unresolved after 0 world steps, 5 safe stops', 'Mismatches: 0',
           'Corrections: 1 (0 verified)', 'Model evaluations used: 3']
C1_9 = ['Plan: fallback toward the best state found (1 actions)',
        'Step 1: East | predicted Moved, observed Moved | match',
        'Plan: fallback toward the best state found (2 actions)',
        'Step 2: North | predicted Moved, observed Moved | match',
        'Step 3: East | predicted Moved, observed Moved | match',
        'Plan: fallback toward the best state found (2 actions)',
        'Step 4: East | predicted Moved, observed Moved | match',
        'Step 5: South | predicted Moved, observed Moved | match',
        'Plan: fallback toward the best state found (2 actions)',
        'Step 6: North | predicted Moved, observed Moved | match',
        'Result: Step limit after 6 world steps, 0 safe stops', 'Mismatches: 0', 'Corrections: 0 (0 verified)',
        'Model evaluations used: 798']
C1_12B = ['Step 1: safe stop (belief shows goal reached)', 'Step 2: safe stop (belief shows goal reached)',
          'Result: Stopped -- unresolved after 0 world steps, 2 safe stops', 'Mismatches: 0',
          'Corrections: 0 (0 verified)', 'Model evaluations used: 0']
VF6_LINES = ['Plan: fallback toward the best state found (10 actions)',
             'Step 1: North | predicted Moved, observed Moved | match',
             'Step 2: East | predicted Moved, observed Moved | match',
             'Plan: active test of correction 1 (1 actions)',
             'Step 3: South | predicted Blocked, observed Moved | MISMATCH',
             '  - correction 1: REJECTED',
             '  - investigation: wall at (4, 3) believed blocked, recorded free (source S2)',
             '  - correction 2: wall at (4, 3) set to free: CONSISTENT',
             'Plan: search found a plan of 3 actions',
             'Step 4: East | predicted Pushed, observed Pushed | match',
             'Step 5: East | predicted Pushed, observed Pushed | match',
             'Step 6: East | predicted Pushed, observed Pushed | match',
             'Result: Success after 6 world steps, 0 safe stops', 'Mismatches: 1', 'Corrections: 2 (0 verified)',
             'Model evaluations used: 216']
SP7_EXPECTED = {1: ('Stopped -- unresolved', 0, 3), 2: ('Stopped -- unresolved', 0, 3),
                3: ('Stopped -- unresolved', 0, 3), 4: ('Stopped -- unresolved', 0, 3),
                5: ('Stopped -- unresolved', 2, 3), 6: ('Success', 4, 0), 7: ('Success', 4, 0),
                8: ('Step limit', 30, 0), 9: ('Step limit', 30, 0), 10: ('Step limit', 30, 0),
                11: ('Success', 12, 0)}


def push_turn_north(state, action):
    return rt.build_model(['pushturn', 'East', 'North'])(state, action)


class Watch:
    """Records what the run loop's watchers see (plan 3.11)."""

    def __init__(self, agent):
        self.agent = agent
        self.delivered = []
        self.decisions = []
        self.used = []
        self.beliefs_after_check = []
        self.committed_actions = True
        self.finals = []

    def on_step(self, given, decision):
        self.delivered.append(given)
        self.decisions.append(decision)
        self.used.append(self.agent.predictor.used_this_step())
        if decision.is_action and not self.agent.committed.has:
            self.committed_actions = False

    def on_final(self, given):
        self.finals.append(given)


def seed_walls(agent, cells, new, old):
    for i, cell in enumerate(cells):
        r = rc.Correction()
        r.id = i + 1
        r.element = rc.Element('wall', cell)
        r.new_blocked = new
        r.old_blocked = old
        agent.corrections.append(r)
    agent.next_id = len(cells) + 1


def seed_v1(agent, count):
    s = demo()
    for i in range(1, count + 1):
        s, outcome = rw.apply_action(s, 'East')
        agent.history.append(obs(i + 1, s, 'East', outcome))
    agent.pending_v1 = rc.ModelV1Work('East', range(1, count + 1))


def same_obs(x, y):
    return x.step == y.step and x.state.same(y.state) and x.last_action == y.last_action and \
        x.last_outcome == y.last_outcome


def check_single_agent():
    c = {}
    a = new_c1(demo(), settings())
    lines = run_lines(demo(), a, settings())
    c['C1-1'] = lines == C1_1
    c['C1-10'] = lines[4] == 'Step 4: East | predicted Pushed, observed Pushed | match' and a.step == 4
    d2 = edit(demo(), walls=[(4, 3)])
    a = new_c1(d2, settings(), belief=demo())
    lines = run_lines(d2, a, settings())
    c['C1-2'] = lines == C1_2 and lines.index('Plan: active test of correction 1 (2 actions)') == 8
    beliefs_true = True
    for truth in (demo(), world(LP), d2):
        a = new_c1(truth, settings())
        checked = []

        def on_step(given, decision, a=a, checked=checked):
            if a.step > 1:
                checked.append(a.belief.same(given.state))

        def on_final(given, a=a, checked=checked):
            checked.append(a.belief.same(given.state))
        outcome, _ = ra.run_c1(truth, a, settings(), on_step, on_final)
        beliefs_true = beliefs_true and outcome == 'Success' and all(checked) and not a.mismatches
    c['C1-3'] = beliefs_true
    a = new_c1(demo(), settings())
    w = Watch(a)
    ra.run_c1(demo(), a, settings(), w.on_step, w.on_final)
    c['C1-4'] = len(a.history) == 5 and all(same_obs(a.history[i], (w.delivered + w.finals)[i]) for i in range(5))
    a = new_c1(world(LT), settings(200, 2))
    lines = run_lines(world(LT), a, settings(200, 2))
    c['C1-5'] = lines == C1_5 and a.step == 2 and len(a.history) == 2 and a.history[1].last_action is None
    a = new_c1(demo(), settings())
    a.predictor.start_step()
    pick = a.choose_cautious()
    c['C1-6a'] = pick is not None and pick[0] == 'North' and a.predictor.used_this_step() == 2
    a = new_c1(demo(), settings(2))
    seed_v1(a, 2)
    first = a.decide(obs(1, demo()))
    c['C1-6b'] = not first.is_action and first.reason == 'investigation unfinished, no cautious action' and \
        a.pending_v1 is not None and a.pending_v1.pending == [2]
    after_two = a.history[2].state
    a.decide(obs(2, after_two))
    c['VF5b'] = (a.pending_v1 is None and len(a.corrections) == 1 and a.model.is_restored('East')
                 and '  - correction 1: action model for East restored to the standard rule: CONSISTENT' in a.log)
    a = new_c1(demo(), settings(2, 2))
    seed_v1(a, 3)
    lines = run_lines(demo(), a, settings(2, 2))
    c['C1-6c-1'] = lines == C1_6C_2 and a.pending_v1.pending == [3] and not a.model.is_restored('East')
    a = new_c1(demo(), settings(2, 5))
    seed_v1(a, 3)
    lines = run_lines(demo(), a, settings(2, 5))
    c['C1-6c-2'] = lines == C1_6C_5 and a.pending_v1 is None and a.model.is_restored('East')
    committed = True
    for truth, belief in ((demo(), None), (d2, demo())):
        a = new_c1(truth, settings(), belief=belief)
        w = Watch(a)
        ra.run_c1(truth, a, settings(), w.on_step, w.on_final)
        committed = committed and w.committed_actions and max(w.used) <= 200
    c['C1-7'] = committed
    a = new_c1(demo(), settings(200, 5, 6), model=push_turn_north)
    c['C1-9'] = run_lines(demo(), a, settings(200, 5, 6)) == C1_9
    a = new_c1(d2, settings(1), belief=demo())
    a.step = 1
    a.committed.has = True
    a.committed.prediction = ('East', 'Moved', rw.apply_action(demo(), 'East')[0].copy())
    a.committed.belief_before = demo()
    a.committed.step = 1
    blocked, outcome = rw.apply_action(d2, 'East')
    first = a.decide(obs(2, blocked, 'East', outcome))
    second = a.decide(obs(3, blocked))
    c['C1-11a'] = (not first.is_action and first.reason == 'budget too small for verification'
                   and not second.is_action and second.reason == 'budget too small for verification'
                   and ra.DEFERRED_LINE in a.log and len(a.mismatches) == 1 and not a.corrections)
    a = new_c1(demo(), settings(0))
    a.plan = [('East', demo())]
    a.plan_kind = 'goal'
    stopped = a.decide(obs(1, demo()))
    c['C1-11b'] = not stopped.is_action and stopped.reason == 'no budget for a committed prediction'
    lg = world(LG)
    a = new_c1(lg, settings())
    outcome, steps, lines = ra.c1_run_lines(lg, a, settings())
    c['C1-12a'] = outcome == 'Success' and steps == 0 and a.predictor.used_total == 0 and a.step == 0
    wrong = edit(demo(), obj=(8, 3))
    a = new_c1(demo(), settings(200, 2), belief=wrong)
    c['C1-12b'] = run_lines(demo(), a, settings(200, 2)) == C1_12B
    a = new_c1(demo(), settings(), belief=edit(demo(), walls=[(4, 3)]))
    seed_walls(a, [(4, 3)], True, False)
    c['VF6'] = run_lines(demo(), a, settings()) == VF6_LINES
    for budget in range(1, 12):
        limits = settings(budget, 3, 30)
        a = new_c1(demo(), limits)
        seed_walls(a, [(4, 3), (4, 2), (3, 4)], False, True)
        w = Watch(a)
        outcome, steps = ra.run_c1(demo(), a, limits, w.on_step, w.on_final)
        reasons = [d.reason for d in w.decisions if not d.is_action]
        small_ok = budget > 4 or (reasons == ['budget too small to plan'] * 3 and steps == 0)
        c['SP7-B%d' % budget] = ((outcome, steps, a.safe_stops) == SP7_EXPECTED[budget] and small_ok
                                 and max(w.used) <= budget and w.committed_actions)
    a = new_c1(d2, settings(), belief=demo())
    w = Watch(a)
    outcome, steps = ra.run_c1(d2, a, settings(), w.on_step, w.on_final)
    plain = new_c1(d2, settings(), belief=demo())
    plain_outcome, plain_steps = ra.run_c1(d2, plain, settings())
    c['RL1'] = (len(w.decisions) == plain.step == 18 and len(w.finals) == 1 and w.finals[0].step == 19
                and (outcome, steps) == (plain_outcome, plain_steps) and a.log == plain.log)
    a = new_c1(d2, settings(), belief=demo())
    seen = []
    a.activation_hook = lambda step: seen.append((step, len(a.log), a.plan_kind))
    run_lines(d2, a, settings())
    b = new_c1(d2, settings(1), belief=demo())
    b.step = 1
    b.committed.has = True
    b.committed.prediction = ('East', 'Moved', rw.apply_action(demo(), 'East')[0].copy())
    b.committed.belief_before = demo()
    b.committed.step = 1
    deferred_seen = []
    b.activation_hook = lambda step: deferred_seen.append(step)
    b.decide(obs(2, blocked, 'East', outcome))
    c['OB5'] = ([s for s, _, _ in seen] == list(range(1, 19)) and seen[1] == (2, 4, 'goal')
                and deferred_seen == [2])
    a = new_c1(d2, settings(), belief=demo())
    rec = Recorder()
    a.predictor.observer = rec
    a.dead_cache.observer = rec
    lines = run_lines(d2, a, settings())
    reported = len([e for e in rec.events if e[0] == 'evaluation'])
    c['OB6'] = lines == C1_2 and reported == 414
    return c


# ---------------- 3f: C0 and the demonstrations ----------------
C0_4 = ['Result: Success after 4 world steps, 0 safe stops', 'Mismatches: 0', 'Corrections: 0 (0 verified)',
        'Model evaluations used: 16']
C0_5 = ['Result: Step limit after 100 world steps, 0 safe stops', 'Mismatches: 0', 'Corrections: 0 (0 verified)',
        'Model evaluations used: 207']
C0_5_FIRST_TEN = ['East', 'East', 'East', 'East', 'North', 'North', 'East', 'South', 'North', 'South']
AD1_TEXT = ('\nSingle-agent demonstrations use illustrative settings: budget 200 per step,\n'
            "safe-stop limit 5, step limit 100, active-test delay 3 (not the experiment's values).\n"
            '\nC1 demo 1: correct belief\n' + ''.join(line + '\n' for line in C1_1)
            + '\nC1 demo 2: belief wrongly has no wall at (4, 3)\n' + ''.join(line + '\n' for line in C1_2)
            + "Note: demo 2's wrong belief is hand-written; it is not fault injection.\n")


def new_c0(truth, limits, belief=None, model=None):
    return ra.C0(obs(1, truth), limits, belief, model)


def agent_demo_text():
    """The text runAgentDemos writes (plan 3.12)."""
    out = ('\nSingle-agent demonstrations use illustrative settings: budget 200 per step,\n'
           "safe-stop limit 5, step limit 100, active-test delay 3 (not the experiment's values).\n\n")
    d = demo()
    d2 = edit(demo(), walls=[(4, 3)])
    out += 'C1 demo 1: correct belief\n' + ''.join(
        line + '\n' for line in run_lines(d, new_c1(d, settings()), settings()))
    out += '\nC1 demo 2: belief wrongly has no wall at (4, 3)\n' + ''.join(
        line + '\n' for line in run_lines(d2, new_c1(d2, settings(), belief=demo()), settings()))
    return out + "Note: demo 2's wrong belief is hand-written; it is not fault injection.\n"


def check_naive_agent():
    c = {}
    d2 = edit(demo(), walls=[(4, 3)])
    first = obs(1, d2)
    a = new_c0(d2, settings(), belief=demo())
    b = new_c0(d2, settings(), belief=demo())
    env = rw.Environment(d2)
    current = first
    same = True
    for _ in range(20):
        da = a.decide(current)
        db = b.decide(first)
        same = same and da.action == db.action
        after, outcome = env.step(da.action)
        current = obs(a.step + 1, after, da.action, outcome)
    c['C0-1'] = same and a.predictor.used_total == b.predictor.used_total
    a = new_c0(demo(), settings())
    env = rw.Environment(demo())
    current = obs(1, demo())
    taken, used = [], []
    for _ in range(4):
        d = a.decide(current)
        taken.append(d.action)
        used.append(a.predictor.used_this_step())
        after, outcome = env.step(d.action)
        current = obs(a.step + 1, after, d.action, outcome)
    c['C0-2'] = taken == ['East'] * 4 and used == [16, 0, 0, 0] and a.searches == 1
    a = new_c0(world(LTA), settings())
    d = a.decide(obs(1, world(LTA)))
    c['C0-3a'] = d.action == 'East' and a.predictor.used_this_step() == 4 and a.searches == 1 and \
        a.belief.agent == (6, 1)
    a = new_c0(world(BOXED), settings())
    d = a.decide(obs(1, world(BOXED)))
    c['C0-3b'] = d.action == 'North' and a.predictor.used_this_step() == 4 and a.belief.same(world(BOXED))
    a = new_c0(demo(), settings())
    c['C0-4'] = ra.c0_run_lines(demo(), a, settings())[2] == C0_4
    a = new_c0(d2, settings(), belief=demo())
    taken = []
    outcome = ra.c0_run_lines(d2, a, settings(), ) if False else None
    w_actions = []
    out = ra.run_c0(d2, a, settings(), lambda given, d: w_actions.append(d.action))
    lines = ra.result_lines(out[0], out[1], 0, 0, 0, 0, a.predictor.used_total)
    c['C0-5'] = lines == C0_5 and a.searches == 97 and w_actions[:10] == C0_5_FIRST_TEN
    a = new_c0(demo(), settings())
    seen = []
    a.activation_hook = lambda step: seen.append((step, len(a.plan), a.predictor.used_this_step()))
    ra.run_c0(demo(), a, settings())
    c['OB7'] = seen == [(1, 0, 0), (2, 3, 0), (3, 2, 0), (4, 1, 0)]
    c['AD1'] = agent_demo_text() == AD1_TEXT
    return c


# ---------------- 3g: the equivalence gate ----------------
def read(path):
    with open(path) as f:
        lines = f.read().split('\n')
    return lines[:-1] if lines and lines[-1] == '' else lines


CASE_D = ['case demo1', 'agent C1', 'settings 200 5 100 3', 'world'] + D + ['belief true', 'model standard',
                                                                         'notdead 0', 'end']


def replace(lines, index, new):
    out = list(lines)
    out[index] = new
    return out


READER_CASES = {
    'TN1': (['case'] + CASE_D[1:], (False, 0, 'line 1: expected a case header')),
    'TN2': (CASE_D[:1], (False, 0, 'case 1: missing agent line')),
    'TN3': (replace(CASE_D, 1, 'agent C2'), (False, 0, 'case 1: malformed agent line')),
    'TN4': (replace(CASE_D, 2, 'settings 200 5 100'), (False, 0, 'case 1: malformed settings line')),
    'TN5': (CASE_D[:8], (False, 0, 'case 1: missing world lines')),
    'TN6': (replace(CASE_D, 4, '####.#####'),
            (False, 0, 'case 1: world does not parse: Every border cell must be a wall (#).')),
    'TN7': (replace(CASE_D, 14, 'belief maybe'), (False, 0, 'case 1: malformed belief line')),
    'TN8': (replace(CASE_D, 15, 'model swap East'), (False, 0, 'case 1: malformed model line')),
    'TN9': (replace(CASE_D, 16, 'notdead 1 1'), (False, 0, 'case 1: malformed notdead line')),
    'TN10': (CASE_D[:17] + ['seed wall 4 3 blocked open', 'end'], (False, 0, 'case 1: malformed seed line')),
    'TN11': (CASE_D[:17], (False, 0, 'case 1: missing end line')),
    'TN12': (CASE_D + CASE_D, (True, 2, '')),
}

SPLIT_CASES = {
    'TN13': (['case demo1', 'step 1'], (False, 0, 'trace demo1: missing end line')),
    'TN14': (['step 1'], (False, 0, 'line 1: expected a case header')),
}


def check_reader():
    c = {}
    for cid, (lines, expected) in READER_CASES.items():
        ok, cases, problem = rr.read_cases(lines)
        c[cid] = (ok, len(cases) if ok else 0, problem) == expected
    for cid, (lines, expected) in SPLIT_CASES.items():
        ok, blocks, problem = rr.split_traces(lines)
        c[cid] = (ok, len(blocks) if ok else 0, problem) == expected
    c['TN15'] = rr.first_difference('x', ['case x', 'step 1', 'decision East', 'end'],
                                    ['case x', 'step 1', 'decision West', 'end']) == \
        'case x, step 1, line 3: expected [decision East] actual [decision West]'
    return c


MC_FIXTURE = edit(demo(), agent=(4, 3))


def check_models():
    c = {}
    s = MC_FIXTURE
    stuck = rt.build_model(['stuck', 'East'])
    c['MC1'] = stuck(s, 'East')[1] == 'Blocked' and stuck(s, 'East')[0].same(s) and stuck(s, 'North')[1] == 'Moved'
    nopush = rt.build_model(['nopush', 'East'])
    c['MC2'] = nopush(s, 'East')[1] == 'Blocked' and nopush(demo(), 'East')[1] == 'Moved'
    slip = rt.build_model(['slip', 'North'])
    c['MC3'] = slip(s, 'North')[0].agent == (4, 1) and slip(edit(demo(), agent=(4, 2)), 'North')[0].agent == (4, 1)
    swap = rt.build_model(['swap', 'East', 'South'])
    c['MC4'] = swap(s, 'East')[0].agent == (4, 4) and swap(s, 'West')[0].agent == (3, 3)
    turn = rt.build_model(['pushturn', 'East', 'North'])
    blocked_turn = turn(edit(demo(), agent=(4, 1), obj=(5, 1)), 'East')
    c['MC5'] = (turn(s, 'East')[0].object == (5, 2) and turn(s, 'East')[0].agent == (5, 3)
                and turn(s, 'East')[1] == 'Pushed' and blocked_turn[1] == 'Blocked')
    c['TW1'] = (rt.hex_walls(demo()) == 'ffe0180601806018c601807ff'
                and rt.state_text(demo()) == 'agent (3, 3) object (5, 3) destination (8, 3) walls '
                                             'ffe0180601806018c601807ff')
    return c


def check_gate(fast):
    if fast:
        return {}
    cases = read(os.path.join(GOLDEN, 'c1_cases.txt'))
    return {'G4': rr.gate(cases, read(os.path.join(GOLDEN, 'c1_traces.txt'))) == '',
            'G5': rr.gate(cases, read(os.path.join(GOLDEN, 'c1_observers.txt')), True) == ''}


def all_checks(fast=False):
    c = {}
    for part in (check_footprint, check_log, check_dead_squares, check_predictor, check_agent_model,
                 check_planner, check_investigation, check_single_agent, check_naive_agent, check_reader,
                 check_models):
        c.update(part())
    c.update(check_gate(fast))
    return c


def failing(fast=False):
    return [k for k, v in all_checks(fast).items() if not v]


if __name__ == '__main__':
    fast = '--fast' in sys.argv
    results = all_checks(fast)
    for k, v in results.items():
        print(k, 'PASS' if v else 'FAIL')
    bad = [k for k, v in results.items() if not v]
    print('failing:', bad if bad else 'none', f'({len(results)} checks)')
