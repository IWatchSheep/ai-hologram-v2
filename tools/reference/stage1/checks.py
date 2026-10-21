"""The Stage 1 checks (plan STAGE_01, Section 6), mirrored on the reference.

Each check has the id the C++ test uses. Running this file prints every result and
`failing: none (<n> checks)`. df_check.py mutates ref_world and reports which ids fail."""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ref_world as rw  # noqa: E402

DEMO = rw.DEMO_LAYOUT
STAR = ['##########', '#........#', '#........#', '#..A....*#', '#........#',
        '#........#', '#...##...#', '#........#', '#........#', '##########']
PLUS = ['##########', '#........#', '#........#', '#..O....+#', '#........#',
        '#........#', '#...##...#', '#........#', '#........#', '##########']
OPEN = ['##########', '#........#', '#........#', '#........#', '#...A....#',
        '#........#', '#........#', '#......O.#', '#D.......#', '##########']
GOLDEN_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'golden')


def parsed(rows):
    return rw.parse_world(rows)[0]


def with_line(rows, index, text):
    out = list(rows)
    out[index] = text
    return out


def error_of(rows):
    return rw.parse_world(rows)[1]


def at(rows, agent, obj):
    w = parsed(rows).copy()
    w.agent, w.object = agent, obj
    return w


def check_parse():
    d = parsed(DEMO)
    c = {}
    c['T1'] = d is not None and error_of(DEMO) == ''
    c['T2'] = d.agent == (3, 3) and d.object == (5, 3) and d.destination == (8, 3)
    c['T3'] = (rw.is_wall(d, (0, 0)) and rw.is_wall(d, (4, 6)) and rw.is_wall(d, (5, 6))
               and not rw.is_wall(d, (1, 1)) and not rw.is_wall(d, (4, 5)))
    c['T4'] = error_of(DEMO[:9]) == rw.MSG_LINES
    c['T5'] = error_of(DEMO + ['#' * 10]) == rw.MSG_LINES
    c['T6'] = error_of(with_line(DEMO, 4, '#.......#')) == rw.MSG_LENGTH
    c['T7'] = error_of(with_line(DEMO, 4, '#.........#')) == rw.MSG_LENGTH
    c['T8'] = error_of(with_line(DEMO, 9, '####.#####')) == rw.MSG_BORDER
    c['T9'] = error_of(with_line(DEMO, 3, '#....O..D#')) == rw.MSG_AGENTS
    c['T10'] = error_of(with_line(DEMO, 1, '#A.......#')) == rw.MSG_AGENTS
    c['T11'] = error_of(with_line(DEMO, 3, '#..A....D#')) == rw.MSG_OBJECTS
    c['T12'] = error_of(with_line(DEMO, 1, '#O.......#')) == rw.MSG_OBJECTS
    c['T13'] = error_of(with_line(DEMO, 3, '#..A.O...#')) == rw.MSG_DESTS
    c['T14'] = error_of(with_line(DEMO, 1, '#D.......#')) == rw.MSG_DESTS
    c['T15'] = error_of(with_line(DEMO, 2, '#...x....#')) == rw.MSG_UNKNOWN
    s = parsed(STAR)
    c['T16'] = s is not None and s.object == (8, 3) and s.destination == (8, 3) and s.agent == (3, 3)
    p = parsed(PLUS)
    c['T17'] = p is not None and p.agent == (8, 3) and p.destination == (8, 3) and p.object == (3, 3)
    c['T18'] = error_of([]) == rw.MSG_LINES
    c['T19'] = error_of([''] * 10) == rw.MSG_LENGTH
    c['T20'] = error_of(['#' * 1000] * 10) == rw.MSG_LENGTH
    return c


def check_render():
    return {'RT1': rw.render_world(parsed(DEMO)) == DEMO,
            'RT2': rw.render_world(parsed(STAR)) == STAR,
            'RT3': rw.render_world(parsed(PLUS)) == PLUS}


def check_state():
    d = parsed(DEMO)
    c = {'V1': rw.is_valid(d)}
    w = d.copy()
    w.agent = (4, 6)
    c['V2'] = not rw.is_valid(w)
    w = d.copy()
    w.agent = d.object
    c['V3'] = not rw.is_valid(w)
    w = d.copy()
    w.walls[0][5] = False
    c['V4'] = not rw.is_valid(w)
    a = d.copy()
    a.agent = (10, 3)
    b = d.copy()
    b.object = (3, -1)
    c['V5'] = not rw.is_valid(a) and not rw.is_valid(b)
    c['V6a'] = rw.is_wall(d, (-1, 0))
    c['V6b'] = rw.is_wall(d, (10, 0))
    c['V6c'] = rw.is_wall(d, (0, -1))
    c['V6d'] = rw.is_wall(d, (0, 10))
    c['N1'] = [rw_action_name(a) for a in rw.ACTIONS] == ['North', 'East', 'South', 'West']
    return c


def rw_action_name(a):
    return a


def check_rules():
    c = {}
    expected_agent = {'North': (4, 3), 'East': (5, 4), 'South': (4, 5), 'West': (3, 4)}
    for tag, action in zip('abcd', rw.ACTIONS):
        w = parsed(OPEN)
        new, outcome = rw.apply_action(w, action)
        c['R1' + tag] = (new.agent == expected_agent[action] and outcome == 'Moved'
                         and new.object == (7, 7))
    w = at(OPEN, (1, 1), (7, 7))
    new, outcome = rw.apply_action(w, 'North')
    c['R2'] = outcome == 'Blocked' and new.same(w)
    w = at(OPEN, (4, 4), (5, 4))
    new, outcome = rw.apply_action(w, 'East')
    c['R3'] = outcome == 'Pushed' and new.agent == (5, 4) and new.object == (6, 4)
    w = at(OPEN, (6, 4), (7, 4))
    w = at(OPEN, (7, 4), (8, 4))
    new, outcome = rw.apply_action(w, 'East')
    c['R4'] = outcome == 'Blocked' and new.agent == (7, 4) and new.object == (8, 4)
    w = at(OPEN, (3, 8), (2, 8))
    new, outcome = rw.apply_action(w, 'West')
    c['R5'] = outcome == 'Pushed' and rw.is_goal(new)
    w = at(OPEN, (2, 7), (5, 5))
    new, outcome = rw.apply_action(w, 'West')
    w2 = new
    new, outcome = rw.apply_action(w2, 'South')
    c['R6'] = outcome == 'Moved' and new.agent == (1, 8) and not rw.is_goal(new)
    w = at(OPEN, (4, 4), (5, 4))
    new, outcome = rw.apply_action(w, 'West')
    c['R7'] = outcome == 'Moved' and new.object == (5, 4)
    walls = parsed(OPEN).walls
    m, _ = rw.apply_action(parsed(OPEN), 'North')
    p, _ = rw.apply_action(at(OPEN, (4, 4), (5, 4)), 'East')
    b, _ = rw.apply_action(at(OPEN, (7, 4), (8, 4)), 'East')
    c['R8a'] = m.walls == walls
    c['R8b'] = p.walls == walls
    c['R8c'] = b.walls == walls
    w = at(OPEN, (4, 4), (5, 4))
    before = w.copy()
    rw.apply_action(w, 'East')
    c['R9'] = w.same(before)
    w = at(OPEN, (2, 7), (1, 8))
    new, outcome = rw.apply_action(w, 'East')
    c['R10'] = rw.is_goal(w) and outcome == 'Moved' and new.agent == (3, 7) and new.object == (1, 8)
    w = at(OPEN, (10, 4), (7, 7))
    new, outcome = rw.apply_action(w, 'West')
    c['R11a'] = outcome == 'InvalidState' and new.same(w)
    w = at(OPEN, (0, 4), (7, 7))
    new, outcome = rw.apply_action(w, 'East')
    c['R11b'] = outcome == 'InvalidState' and new.same(w)
    c['N2'] = True
    return c


def check_environment():
    c = {}
    start = parsed(OPEN)
    env = rw.Environment(start)
    c['E1'] = env.state.same(start) and env.step_count == 0
    env.step('North')
    env.step('North')
    env.step('North')
    env.step('North')
    c['E2'] = env.step_count == 4 and env.state.agent == (4, 1)
    actions = ['East', 'East', 'South', 'South', 'South', 'East', 'West', 'North']
    env = rw.Environment(start)
    by_hand = start.copy()
    for a in actions:
        env.step(a)
        by_hand, _ = rw.apply_action(by_hand, a)
    c['E3'] = env.state.same(by_hand) and env.step_count == len(actions)
    bad = at(OPEN, (0, 4), (7, 7))
    env = rw.Environment(bad)
    _, outcome = env.step('East')
    c['E4'] = outcome == 'InvalidState' and env.state.same(bad) and env.step_count == 1
    return c


def golden_rules_ok():
    import gen_golden
    with open(os.path.join(GOLDEN_DIR, 'world_rules_golden.txt')) as f:
        expected = f.read()
    lines = expected.splitlines()
    cases = [x for x in lines if ' -> ' in x]
    return gen_golden.rules_golden_text() == expected and len(cases) == 5184


def golden_parse_ok():
    import gen_golden
    with open(os.path.join(GOLDEN_DIR, 'world_parse_golden.txt')) as f:
        expected = f.read()
    results = [gen_golden.parse_result_line(n, rows)
               for n, rows in enumerate(gen_golden.parse_inputs(), 1)]
    count = sum(1 for x in expected.splitlines() if x.startswith('case '))
    return gen_golden.merged_parse_text(results) == expected and count == 1009


DEMO_TEXT = ('\nInitial world:\n' + ''.join(r + '\n' for r in DEMO) + '\n'
             'Step 1: East -> Moved\nStep 2: East -> Pushed\nStep 3: East -> Pushed\n'
             'Step 4: East -> Pushed\nStep 5: East -> Blocked\n\nFinal world:\n'
             + ''.join(r + '\n' for r in with_line(DEMO, 3, '#......A*#'))
             + 'Goal reached: yes\nSteps taken: 5\n')


def all_checks():
    c = {}
    c.update(check_parse())
    c.update(check_render())
    c.update(check_state())
    c.update(check_rules())
    c.update(check_environment())
    c['G1'] = golden_rules_ok()
    c['G2'] = golden_parse_ok()
    c['D1'] = rw.scripted_demo() == DEMO_TEXT
    return c


def failing():
    return [k for k, v in all_checks().items() if not v]


if __name__ == '__main__':
    results = all_checks()
    for k, v in results.items():
        print(k, 'PASS' if v else 'FAIL')
    bad = failing()
    print('failing:', bad if bad else 'none', f'({len(results)} checks)')
