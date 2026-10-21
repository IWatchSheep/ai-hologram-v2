"""Writes the Stage 3 trace cases and, from this reference, the expected traces.

  python3 gen_golden.py cases     <dir>   writes c1_cases.txt (inputs only)
  python3 gen_golden.py traces    <dir>   writes c1_traces.txt computed by this reference
  python3 gen_golden.py observers <dir>   writes c1_observers.txt computed by this reference

The golden files committed to the repository are the ones produced by the ORIGINAL C++
(orig_trace/orig_c1_trace.cpp): c1_traces.txt from 7587f38 (tag stage-3) and
c1_observers.txt from 89ea9b6 (stage-3 plus IE-1). This reference must produce
byte-identical files (plan STAGE_03, Section 3.13).

The cases (plan 3.13):
  1. the named Stage 3 scenarios of Migration Addendum M1.4 (demonstrations, C1-3, C1-5,
     C1-6c, C1-9, C1-11, C1-12, SP7, VF6, the LT, L2 and LP layouts, and the C0 cases);
  2. the demonstration-seed layouts 500000-500009 (rule R21), fault-free, C1 and C0, with the
     protocol's settings (B 200, S_max 5, T_max 100, T_v 3). The layouts are written out as
     text; they were produced by the carried-over Stage 4 reference (make_layout), which gives
     the same layouts as the original C++ generator at 89ea9b6;
  3. synthetic cases, generated here with seed 20261022: random valid worlds that are not
     experiment material (no experiment generator and no experiment seed range is used), with
     hand-written wrong beliefs, wrong action models from the model catalogue, "not dead"
     overrides and seeded corrections, at budgets from 0 to 5000."""
import os
import random
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'stage4'))
import ref_agents as ra  # noqa: E402
import ref_parts as rp  # noqa: E402
import ref_trace as rt  # noqa: E402

rw = rp.rw
SEED = 20261022
C1_SYNTHETIC = 630
C0_SYNTHETIC = 100

D = ['##########', '#........#', '#........#', '#..A.O..D#', '#........#',
     '#........#', '#...##...#', '#........#', '#........#', '##########']
LT = ['##########', '#O.......#', '#........#', '#........#', '#....A...#',
      '#........#', '#........#', '#.......D#', '#........#', '##########']
LP = ['##########', '#....D##.#', '#.O......#', '#....A#..#', '#.#......#',
      '#.....##.#', '#........#', '#........#', '#........#', '##########']
L2 = ['##########', '#....#...#', '#.O..#.A.#', '#....#...#', '#D...#...#',
      '######...#', '#........#', '#........#', '#........#', '##########']
LG = ['##########', '#........#', '#........#', '#..A....*#', '#........#',
      '#........#', '#...##...#', '#........#', '#........#', '##########']
LTA = ['##########', '#O...A...#', '#........#', '#........#', '#........#',
       '#........#', '#........#', '#.......D#', '#........#', '##########']
BOXED = ['##########', '#.#......#', '##A#.....#', '#.#..O..D#', '#........#',
         '#........#', '#...##...#', '#........#', '#........#', '##########']


TRAP_REJECT_1 = ['##########', '#........#', '#A.....#.#', '#.....D..#', '#........#',
                 '#.....#..#', '#..#.....#', '#........#', '#.O..#...#', '##########']
TRAP_REJECT_2 = ['##########', '#........#', '#.A.....O#', '#.#....#.#', '##D.#...##',
                 '#........#', '#........#', '#.#......#', '#...#...##', '##########']
TRAP_WAIT_1 = ['##########', '#.#....#A#', '#O......##', '#........#', '#....#.#.#',
               '#.#....D.#', '#.#...#..#', '#..##....#', '#.....#..#', '##########']
TRAP_WAIT_2 = ['##########', '#........#', '##......##', '#......#A#', '#...#...##',
               '#...#..#.#', '#........#', '#...##...#', '#.D.O#.#.#', '##########']


def world(rows):
    return rw.parse_world(rows)[0]


def with_wall(w, x, y, value=True):
    c = w.copy()
    c.walls[y][x] = value
    return c


def case(name, agent, settings, truth, belief=None, model=None, not_dead=(), walls=(), v1=None,
         committed=None, plan=None):
    c = rt.Case()
    c.name = name
    c.agent = agent
    c.settings = ra.Settings(*settings)
    c.world = truth
    c.belief = belief
    c.model = model or ['standard']
    c.not_dead = list(not_dead)
    c.walls = list(walls)
    c.v1 = v1
    c.committed = committed
    c.plan = plan
    return c


def named_cases():
    d = world(D)
    d2 = with_wall(d, 4, 3)
    wrong_goal = d.copy()
    wrong_goal.object = (8, 3)
    sp7_walls = [((4, 3), False, True), ((4, 2), False, True), ((3, 4), False, True)]
    out = [
        case('demo1', 'C1', (200, 5, 100, 3), d),
        case('demo2', 'C1', (200, 5, 100, 3), d2, belief=d),
        case('C1-3-LP', 'C1', (200, 5, 100, 3), world(LP)),
        case('C1-3-D2', 'C1', (200, 5, 100, 3), d2),
        case('C1-5-LT', 'C1', (200, 2, 100, 3), world(LT)),
        case('C1-6c-smax2', 'C1', (2, 2, 100, 3), d, v1=('East', 3)),
        case('C1-6c-smax5', 'C1', (2, 5, 100, 3), d, v1=('East', 3)),
        case('C1-9', 'C1', (200, 5, 6, 3), d, model=['pushturn', 'East', 'North']),
        case('C1-11-deferred', 'C1', (1, 5, 100, 3), d2, belief=d, committed='East'),
        case('C1-11-nobudget', 'C1', (0, 5, 100, 3), d, plan='East'),
        case('C1-12a', 'C1', (200, 5, 100, 3), world(LG)),
        case('C1-12b', 'C1', (200, 2, 100, 3), d, belief=wrong_goal),
        case('VF6', 'C1', (200, 5, 100, 3), d, belief=with_wall(d, 4, 3), walls=[((4, 3), True, False)]),
        case('LT-notdead', 'C1', (100000, 5, 100, 3), world(LT), not_dead=[(1, 1)]),
        case('LT-notdead-B200', 'C1', (200, 3, 100, 3), world(LT), not_dead=[(1, 1)]),
        case('L2', 'C1', (200, 3, 100, 3), world(L2)),
        case('trap-rejected-1', 'C1', (100000, 3, 15, 5), world(TRAP_REJECT_1),
             model=['pushturn', 'East', 'North'], not_dead=[(2, 8)]),
        case('trap-rejected-2', 'C1', (100000, 5, 30, 5), world(TRAP_REJECT_2),
             model=['pushturn', 'South', 'West'], not_dead=[(8, 2)]),
        case('trap-waiting-1', 'C1', (7, 4, 45, 2), world(TRAP_WAIT_1), not_dead=[(1, 2)]),
        case('trap-waiting-2', 'C1', (5, 4, 15, 3), world(TRAP_WAIT_2), not_dead=[(4, 8)]),
        case('L2-large', 'C1', (100000, 3, 100, 3), world(L2)),
        case('C0-4', 'C0', (200, 5, 100, 3), d),
        case('C0-5', 'C0', (200, 5, 100, 3), d2, belief=d),
        case('C0-3a-LTa', 'C0', (200, 5, 20, 3), world(LTA)),
        case('C0-3b-boxed', 'C0', (200, 5, 20, 3), world(BOXED)),
        case('C0-LP', 'C0', (200, 5, 100, 3), world(LP)),
        case('C0-L2', 'C0', (200, 5, 30, 3), world(L2)),
        case('C0-9', 'C0', (200, 5, 30, 3), d, model=['pushturn', 'East', 'North']),
    ]
    for budget in range(1, 12):
        out.append(case('SP7-B%d' % budget, 'C1', (budget, 3, 30, 3), d, walls=sp7_walls))
    return out


def demo_seed_cases():
    import ref_stage4 as r4
    out = []
    for seed in range(500000, 500010):
        s = r4.make_layout(seed)[0]
        truth = world(r4.layout_text(s))
        out.append(case('seed%d-C1' % seed, 'C1', (200, 5, 100, 3), truth))
        out.append(case('seed%d-C0' % seed, 'C0', (200, 5, 100, 3), truth))
    return out


# ---------------- synthetic cases ----------------
def solvable(w, limit=40):
    """Shortest number of world steps to the goal by breadth-first search over true states
    (generator only; never visible to an agent), or None."""
    if w.object == w.destination:
        return 0
    seen = {(w.agent, w.object)}
    frontier = [w]
    for depth in range(1, limit + 1):
        nxt = []
        for s in frontier:
            for a in rp.ORDER:
                t, outcome = rw.apply_action(s, a)
                if outcome not in ('Moved', 'Pushed') or (t.agent, t.object) in seen:
                    continue
                if t.object == t.destination:
                    return depth
                seen.add((t.agent, t.object))
                nxt.append(t)
        frontier = nxt
    return None


def random_truth(rng, need_solvable=True):
    while True:
        w = rw.World()
        density = rng.choice([0.06, 0.1, 0.14, 0.2])
        for y in range(10):
            for x in range(10):
                w.walls[y][x] = x in (0, 9) or y in (0, 9) or rng.random() < density
        free = [(x, y) for y in range(10) for x in range(10) if not w.walls[y][x]]
        if len(free) < 6:
            continue
        w.agent, w.object, w.destination = rng.sample(free, 3)
        if rng.random() < 0.4:
            dx, dy = rng.choice([(0, -1), (1, 0), (0, 1), (-1, 0)])
            beside = (w.agent[0] + dx, w.agent[1] + dy)
            if beside in free and beside != w.destination:
                w.object = beside
        if not rw.is_valid(w) or w.object == w.destination:
            continue
        if not need_solvable or solvable(w) is not None:
            return w


def near_cells(rng, w, count):
    """Interior cells near the agent, the object or the destination."""
    cells = []
    for _ in range(count):
        base = rng.choice([w.agent, w.object, w.object, w.destination])
        x = min(8, max(1, base[0] + rng.randint(-2, 2)))
        y = min(8, max(1, base[1] + rng.randint(-2, 2)))
        cells.append((x, y))
    return cells


def toggled(w, cells):
    b = w.copy()
    for x, y in cells:
        b.walls[y][x] = not b.walls[y][x]
    return b


def missing_walls(rng, truth, count):
    """Truly blocked interior cells near the agent, the object or the destination, which the
    belief will wrongly show as free."""
    blocked = [(x, y) for y in range(1, 9) for x in range(1, 9) if truth.walls[y][x]]
    if not blocked:
        return []
    key = lambda c: min(rp.manhattan(c, truth.agent), rp.manhattan(c, truth.object),
                        rp.manhattan(c, truth.destination)) + rng.random() * 3
    return sorted(blocked, key=key)[:count]


def wall_belief(rng, truth, count=None):
    for _ in range(50):
        k = count if count is not None else rng.choice([1, 1, 2])
        cells = missing_walls(rng, truth, k) if rng.random() < 0.7 else near_cells(rng, truth, k)
        b = toggled(truth, cells)
        if rw.is_valid(b) and not b.same(truth):
            return b
    return None


def object_belief(rng, truth):
    for _ in range(50):
        b = truth.copy()
        b.object = near_cells(rng, truth, 1)[0]
        if rng.random() < 0.3:
            b = toggled(b, near_cells(rng, truth, 1))
        if rw.is_valid(b) and not b.same(truth):
            return b
    return None


def noisy_belief(rng, truth):
    for _ in range(50):
        b = truth.copy()
        kind = rng.random()
        free = [(x, y) for y in range(1, 9) for x in range(1, 9) if not b.walls[y][x]]
        if kind < 0.5:
            b.agent = rng.choice(free)
        else:
            b.destination = rng.choice(free)
        if rng.random() < 0.5:
            b = toggled(b, near_cells(rng, truth, 1))
        if rw.is_valid(b) and not b.same(truth):
            return b
    return None


def random_model(rng):
    kind = rng.choice(['stuck', 'nopush', 'slip', 'swap', 'pushturn'])
    a = rng.choice(rp.ORDER)
    if kind in ('stuck', 'nopush', 'slip'):
        return [kind, a]
    b = rng.choice([x for x in rp.ORDER if x != a])
    return [kind, a, b]


def dead_cells(belief):
    m = rp.compute_dead_squares(belief, [], [], rp.Counts())
    return [(x, y) for y in range(10) for x in range(10) if m.dead[y][x]]


def settings_for(rng, budgets):
    return (rng.choice(budgets), rng.randint(1, 5), rng.choice([15, 30, 45, 60]), rng.randint(0, 5))


NORMAL = [200, 200, 200, 400, 1000]
SMALL = [0, 1, 2, 3, 4, 5, 6, 8, 10, 12, 16, 24, 40]
MIXED = SMALL + NORMAL + [5000]


def family_wall(rng, n):
    truth = random_truth(rng)
    return case(n, 'C1', settings_for(rng, MIXED), truth, wall_belief(rng, truth))


def family_object(rng, n):
    truth = random_truth(rng)
    return case(n, 'C1', settings_for(rng, MIXED), truth, object_belief(rng, truth))


def family_model(rng, n):
    truth = random_truth(rng)
    return case(n, 'C1', settings_for(rng, MIXED), truth, model=random_model(rng))


def family_mixed(rng, n):
    truth = random_truth(rng)
    belief = rng.choice([wall_belief, object_belief])(rng, truth)
    return case(n, 'C1', settings_for(rng, MIXED), truth, belief, random_model(rng))


def family_noisy(rng, n):
    truth = random_truth(rng)
    model = random_model(rng) if rng.random() < 0.3 else None
    return case(n, 'C1', settings_for(rng, MIXED), truth, noisy_belief(rng, truth), model)


def family_trap(rng, n):
    for _ in range(100):
        truth = random_truth(rng, need_solvable=False)
        cells = [c for c in dead_cells(truth) if c != truth.agent and c != truth.destination]
        if not cells:
            continue
        stuck = truth.copy()
        stuck.object = rng.choice(cells)
        if not rw.is_valid(stuck):
            continue
        not_dead = [stuck.object] + (rng.sample(cells, 1) if rng.random() < 0.3 else [])
        budgets = [8, 12, 20, 30, 60] if rng.random() < 0.3 else [200, 1000, 100000]
        model = None
        if rng.random() < 0.4:
            a = rng.choice(rp.ORDER)
            model = ['pushturn', a, rng.choice([x for x in rp.ORDER if x != a])]
        return case(n, 'C1', settings_for(rng, budgets), stuck, model=model, not_dead=not_dead)
    return None


def family_trap_model(rng, n):
    """A stuck object with a wrong push model at a large budget: a trap claim that the
    model-based V2 test can reject (plan 3.8)."""
    c = family_trap(rng, n)
    a = rng.choice(rp.ORDER)
    c.model = ['pushturn', a, rng.choice([x for x in rp.ORDER if x != a])]
    c.settings.budget = rng.choice([200, 100000])
    return c


def family_trap_tight(rng, n):
    """A stuck object at a budget that an exhaustive search nearly uses up: the trap V2
    test may have to wait for a later step (plan 3.6 step 3)."""
    c = family_trap(rng, n)
    c.settings.budget = rng.choice([6, 8, 10, 12, 14, 16, 20, 24])
    return c


def family_seeded(rng, n):
    """Seeded CONSISTENT wall corrections, some right and some wrong, with the belief
    agreeing with each correction's new value."""
    truth = random_truth(rng)
    belief = truth.copy()
    walls = []
    for cell in near_cells(rng, truth, rng.choice([1, 2, 3])):
        if cell in [w[0] for w in walls] or cell in (truth.agent, truth.object, truth.destination):
            continue
        right = rng.random() < 0.5
        old = truth.walls[cell[1]][cell[0]]
        new = old if right else not old
        old_claimed = not new
        belief.walls[cell[1]][cell[0]] = new
        walls.append((cell, new, old_claimed))
    if not walls or not rw.is_valid(belief):
        return family_wall(rng, n)
    return case(n, 'C1', settings_for(rng, MIXED), truth, None if belief.same(truth) else belief,
                walls=walls)


def family_tight(rng, n):
    """Several missing walls at very small budgets: deferred investigations (plan 3.4)."""
    truth = random_truth(rng)
    settings = (rng.choice([2, 2, 3, 3, 4, 5, 6]), rng.randint(2, 5), rng.choice([30, 60]), rng.randint(0, 5))
    return case(n, 'C1', settings, truth, wall_belief(rng, truth, rng.choice([2, 3, 4])))


def blocked_neighbour(truth):
    """An action whose first cell is a truly blocked interior cell, or None."""
    for a in rp.ORDER:
        c1 = rw.moved(truth.agent, a)
        if 1 <= c1[0] <= 8 and 1 <= c1[1] <= 8 and truth.walls[c1[1]][c1[0]]:
            return a, c1
    return None


def seeded_bump(rng):
    """A truth with a blocked interior cell next to the agent, and a belief that misses it."""
    while True:
        truth = random_truth(rng, need_solvable=False)
        found = blocked_neighbour(truth)
        if found is None:
            continue
        a, c1 = found
        belief = toggled(truth, [c1])
        if rw.is_valid(belief):
            return truth, belief, a


def family_deferred(rng, n):
    """A seeded committed action into a wall the belief misses, at a budget too small for the
    investigation's re-prediction (plan 3.4): the investigation is deferred and retried."""
    truth, belief, a = seeded_bump(rng)
    walls = []
    if rng.random() < 0.5:
        far = [(x, y) for y in range(1, 9) for x in range(1, 9)
               if rp.manhattan((x, y), truth.agent) > 4 and (x, y) not in (truth.object, truth.destination)]
        for cell in rng.sample(far, rng.randint(1, 2)):
            now = belief.walls[cell[1]][cell[0]]
            walls.append((cell, now, not now))
    reserve = 1 + len(walls)
    settings = (rng.choice([0, 1, reserve, reserve, reserve + 1]), rng.randint(1, 5), 30, rng.randint(0, 5))
    return case(n, 'C1', settings, truth, belief, walls=walls, committed=a)


def family_unexplained(rng, n):
    """The same seeded bump, with a wrong model for that action: the corrected belief still
    predicts wrongly, so no candidate passes (source S5, an unexplained update)."""
    truth, belief, a = seeded_bump(rng)
    kind = rng.choice(['slip', 'swap', 'swap'])
    model = [kind, a] if kind == 'slip' else ['swap', a, rng.choice([x for x in rp.ORDER if x != a])]
    return case(n, 'C1', settings_for(rng, MIXED), truth, belief, model, committed=a)


FAMILIES = [family_deferred, family_unexplained, family_wall, family_wall, family_object, family_model, family_mixed, family_mixed,
            family_noisy, family_trap, family_trap_model, family_trap_tight, family_seeded, family_tight]


def synthetic_c1(rng, n):
    family = FAMILIES[(n - 1) % len(FAMILIES)]
    c = None
    while c is None:
        c = family(rng, 'synthetic-C1-%03d' % n)
    return c


def synthetic_c0(rng, n):
    truth = random_truth(rng)
    budget = rng.choice([0, 1, 3, 10, 30, 200, 200, 1000])
    settings = (budget, 5, rng.choice([10, 20, 40, 60]), 3)
    belief = rng.choice([None, wall_belief, object_belief])
    belief = belief(rng, truth) if belief is not None else None
    model = random_model(rng) if rng.random() < 0.3 else None
    return case('synthetic-C0-%03d' % n, 'C0', settings, truth, belief, model)


def all_cases():
    rng = random.Random(SEED)
    out = named_cases() + demo_seed_cases()
    out += [synthetic_c1(rng, n) for n in range(1, C1_SYNTHETIC + 1)]
    out += [synthetic_c0(rng, n) for n in range(1, C0_SYNTHETIC + 1)]
    return out


def main():
    mode, folder = sys.argv[1], sys.argv[2]
    cases = all_cases()
    if mode == 'cases':
        lines = []
        for c in cases:
            lines += c.text()
        name = 'c1_cases.txt'
    else:
        lines = []
        for c in cases:
            lines += rt.trace(c, mode == 'observers')
        name = 'c1_%s.txt' % mode
    with open(os.path.join(folder, name), 'w', newline='\n') as f:
        f.write('\n'.join(lines) + '\n')


if __name__ == '__main__':
    main()
