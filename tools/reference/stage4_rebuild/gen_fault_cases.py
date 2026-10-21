"""Writes the Stage 4 fault-trace cases (plan STAGE_04 3.15) and, from this reference, the traces.

  python3 gen_fault_cases.py cases  <dir>   writes fault_cases.txt (inputs only)
  python3 gen_fault_cases.py traces <dir>   writes fault_traces.txt computed by this reference

The golden fault_traces.txt committed to the repository is the one produced by the ORIGINAL
C++ at 89ea9b6 (orig_trace/orig_fault_trace.cpp). This reference must produce it byte for byte.

The cases (Migration Addendum M1.4, part 2):
  1. FR1: Stage 3 demonstration 2's true world with one injected event, demo-1 (F1 at (4, 3),
     blocked->free, step 1);
  2. FR4: the empty event list, C1 and C0 on D and on D2 with the hand-written belief D;
  3. the demonstration experiment: layouts 500000-500003 x run seeds 1-2, C1 and C0 (16 runs),
     with its event list (fault master seed 4242);
  4. the 60 further demonstration-seed runs: layouts 500000-500009 x run seeds 1-3, C1 and C0,
     with the event list of that 30-run set (master seed 4242). The fault-free runs of these
     layouts are already Stage 3's 20 demonstration-seed cases (without faults the run seed
     changes nothing);
  5. synthetic fault fixtures (D-070), generated here with seed 20261023: random valid worlds
     (no experiment generator and no experiment seed range), with fault events drawn by this
     file's own generator, including cases the experiment generator never makes (several
     events at one step, late steps, repeated F3 on one action, events without a cell), and
     hand-written wrong beliefs and models from the Stage 3 catalogue.
The event lists of 3 and 4 come from the carried-over Stage 4 reference (make_events); Claude
checked them against the original C++ generator (orig_trace/orig_material.cpp)."""
import os
import random
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'stage3'))
sys.path.insert(0, os.path.join(HERE, '..', 'stage4'))
import gen_golden as g3  # noqa: E402
import ref_parts as rp  # noqa: E402

rw = rp.rw
SEED = 20261023
C1_SYNTHETIC = 300
C0_SYNTHETIC = 100
TRAP_C1 = 60
TRAP_C0 = 10
# The candidates of wall_f5_case that reach A3's wall check, found by running the original C++
# (orig_trace/orig_fault_trace.cpp) on candidates 1-1000.
WALL_F5_PICKS = [191, 229, 264, 289, 302, 371, 380, 430]
DIRS = ['North', 'East', 'South', 'West']


class FaultCase:
    def __init__(self, base, layout_seed, run_seed, events):
        self.base = base
        self.layout_seed = layout_seed
        self.run_seed = run_seed
        self.events = events     # dicts: id, layout, run, index, step, type, target, common, ...

    def text(self):
        lines = self.base.text()
        extra = ['run %d %d' % (self.layout_seed, self.run_seed)] + [event_line(e) for e in self.events]
        return lines[:-1] + extra + [lines[-1]]


def event_line(e):
    head = 'event %s %d %d %d %d %s %s %d' % (e['id'], e['layout'], e['run'], e['index'], e['step'],
                                             e['type'], e['target'], 1 if e['common'] else 0)
    t = e['type']
    if t in ('F1', 'F4'):
        cell = 'cell none' if e['cell'] is None else 'cell %d %d' % e['cell']
        return head + ' ' + cell + (' ' + e['dir'] if t == 'F1' else '')
    if t == 'F2':
        return head + ' ' + ' '.join(e['order'])
    if t == 'F3':
        return head + ' %s %s' % (e['action'], e['rot'])
    return head + ' -'


def from_r4(ev, layout, run, index):
    e = dict(ev)
    e.update(layout=layout, run=run, index=index)
    return e


# ---------------- 1-4: the named and demonstration cases ----------------
def named_fault_cases():
    d = g3.world(g3.D)
    d2 = g3.with_wall(d, 4, 3)
    demo1 = {'id': 'demo-1', 'layout': 0, 'run': 0, 'index': 1, 'step': 1, 'type': 'F1', 'target': 'R1',
             'common': False, 'cell': (4, 3), 'dir': 'blocked->free'}
    out = [FaultCase(g3.case('FR1-demo2-injected', 'C1', (200, 5, 100, 3), d2), 0, 0, [demo1])]
    out.append(FaultCase(g3.case('FR4-C1-D', 'C1', (200, 5, 100, 3), d), 0, 0, []))
    out.append(FaultCase(g3.case('FR4-C1-D2', 'C1', (200, 5, 100, 3), d2, belief=d), 0, 0, []))
    out.append(FaultCase(g3.case('FR4-C0-D', 'C0', (200, 5, 100, 3), d), 0, 0, []))
    out.append(FaultCase(g3.case('FR4-C0-D2', 'C0', (200, 5, 100, 3), d2, belief=d), 0, 0, []))
    return out


def demonstration_set(prefix, last_seed, run_seeds):
    import ref_stage4 as r4
    runs = []
    for seed in range(500000, last_seed + 1):
        s = r4.make_layout(seed)[0]
        for run in range(1, run_seeds + 1):
            runs.append((seed, run, s))
    lists = r4.make_events(runs)
    out = []
    for (seed, run, s), events in zip(runs, lists):
        truth = g3.world(r4.layout_text(s))
        evs = [from_r4(e, seed, run, j + 1) for j, e in enumerate(events)]
        for agent in ('C1', 'C0'):
            name = '%s-%d-%d-%s' % (prefix, seed, run, agent)
            out.append(FaultCase(g3.case(name, agent, (200, 5, 100, 3), truth), seed, run, evs))
    return out


# ---------------- 5: synthetic fault fixtures ----------------
def interior(w):
    return [(x, y) for y in range(1, 9) for x in range(1, 9)]


def true_dead(w):
    return [c for c in g3.dead_cells(w) if c in interior(w) and c != w.destination]


def near(rng, w, base):
    x = min(8, max(1, base[0] + rng.randint(-2, 2)))
    y = min(8, max(1, base[1] + rng.randint(-2, 2)))
    return (x, y)


def f1_cell(rng, w, direction):
    want = direction == 'blocked->free'
    cands = [c for c in interior(w) if c != w.destination and w.walls[c[1]][c[0]] == want]
    if not cands or rng.random() < 0.03:
        return None
    if rng.random() < 0.6:
        for _ in range(20):
            c = near(rng, w, rng.choice([w.agent, w.object, w.object]))
            if c in cands:
                return c
    return rng.choice(cands)


def random_event(rng, w, name, index, step, kind=None, f3_action=None):
    t = kind or rng.choice(['F1', 'F2', 'F3', 'F4', 'F5'])
    target = rng.choice(['R1', 'R2', 'both'])
    e = {'id': '%s-%d' % (name, index), 'layout': 0, 'run': 0, 'index': index, 'step': step, 'type': t,
         'target': target, 'common': target == 'both'}
    if t == 'F1':
        e['dir'] = rng.choice(['free->blocked', 'blocked->free'])
        e['cell'] = f1_cell(rng, w, e['dir'])
    elif t == 'F2':
        order = DIRS[:]
        rng.shuffle(order)
        e['order'] = order
    elif t == 'F3':
        e['action'] = f3_action or rng.choice(DIRS)
        e['rot'] = rng.choice(['left', 'right'])
    elif t == 'F4':
        dead = true_dead(w)
        e['cell'] = rng.choice(dead) if dead and rng.random() > 0.03 else None
    return e


def events_standard(rng, w, name):
    steps = sorted(rng.sample(range(1, 11), 3))
    return [random_event(rng, w, name, j + 1, steps[j]) for j in range(3)]


def events_dense(rng, w, name):
    count = rng.randint(4, 7)
    steps = sorted(rng.choice(range(1, 21)) for _ in range(count))
    action = rng.choice(DIRS)
    out = []
    for j, step in enumerate(steps):
        kind = 'F3' if rng.random() < 0.25 else None
        out.append(random_event(rng, w, name, j + 1, step, kind, action if kind else None))
    return out


def events_late(rng, w, name):
    steps = sorted(rng.sample(range(5, 41), 3))
    return [random_event(rng, w, name, j + 1, steps[j]) for j in range(3)]


def events_object(rng, w, name):
    steps = sorted(rng.sample(range(1, 16), 3))
    return [random_event(rng, w, name, j + 1, steps[j], rng.choice(['F2', 'F5', 'F5'])) for j in range(3)]


def events_process(rng, w, name):
    steps = sorted(rng.sample(range(1, 13), 3))
    return [random_event(rng, w, name, j + 1, steps[j], rng.choice(['F3', 'F4'])) for j in range(3)]


def events_wall_f5(rng, w, name):
    """F1 walls near the object at every second step, and F5 events at every third: A3's wall
    check (the object's previous square believed to be a wall)."""
    out = []
    near_object = [c for c in interior(w) if c != w.destination and not w.walls[c[1]][c[0]]
                   and rp.manhattan(c, w.object) <= 3]
    for step in range(2, 14, 2):
        if near_object:
            e = random_event(rng, w, name, len(out) + 1, step, 'F1')
            e['dir'] = 'free->blocked'
            e['cell'] = rng.choice(near_object)
            out.append(e)
    for step in range(3, 20, 3):
        out.append(random_event(rng, w, name, len(out) + 1, step, 'F5'))
    out.sort(key=lambda e: e['step'])
    for j, e in enumerate(out):
        e['index'] = j + 1
        e['id'] = '%s-%d' % (name, j + 1)
    return out


EVENT_FAMILIES = [events_standard, events_standard, events_dense, events_late, events_object, events_process,
                  events_wall_f5]
SETTINGS = [(200, 5, 100, 3)] * 4 + [(200, 3, 60, 3), (50, 5, 60, 3), (20, 5, 40, 3), (8, 5, 30, 2),
                                     (400, 5, 60, 3), (200, 2, 45, 1)]


def synthetic_case(rng, n, agent):
    name = 'synthetic-F%s-%03d' % (agent[1], n)
    truth = g3.random_truth(rng)
    settings = rng.choice(SETTINGS)
    belief = None
    model = None
    kind = rng.random()
    if agent == 'C1' and kind < 0.15:
        belief = g3.wall_belief(rng, truth)
    elif agent == 'C1' and kind < 0.25:
        belief = g3.object_belief(rng, truth)
    elif kind < 0.45:
        model = g3.random_model(rng)
    family = EVENT_FAMILIES[(n - 1) % len(EVENT_FAMILIES)]
    events = family(rng, truth, name)
    base = g3.case(name, agent, settings, truth, belief, model)
    return FaultCase(base, 0, 0, events)


def trap_case(rng, n, agent):
    """The object starts in a true dead square, and an F4 event at step 1 or 2 marks that square
    "not dead": the trap claims of a C1 that notices (plan 3.4)."""
    name = 'synthetic-T%s-%03d' % (agent[1], n)
    while True:
        truth = g3.random_truth(rng, need_solvable=False)
        cells = [c for c in true_dead(truth) if c != truth.agent]
        if not cells:
            continue
        stuck = truth.copy()
        stuck.object = rng.choice(cells)
        if rw.is_valid(stuck) and stuck.object != stuck.destination:
            break
    e = random_event(rng, stuck, name, 1, rng.choice([1, 1, 2]), 'F4')
    e['cell'] = stuck.object
    events = [e] + [random_event(rng, stuck, name, j, rng.randint(1, 12)) for j in (2, 3)]
    events.sort(key=lambda x: x['step'])
    for j, x in enumerate(events):
        x['index'] = j + 1
        x['id'] = '%s-%d' % (name, j + 1)
    budget = rng.choice([8, 12, 20, 30, 60, 200, 200, 1000, 100000])
    settings = (budget, rng.randint(1, 5), rng.choice([15, 30, 45]), rng.randint(0, 5))
    model = None
    if rng.random() < 0.3:
        a = rng.choice(DIRS)
        model = ['pushturn', a, rng.choice([x for x in DIRS if x != a])]
    return FaultCase(g3.case(name, agent, settings, stuck, model=model), 0, 0, events)


def boxed_case(rng, n, agent):
    """The object in a pocket with walls on three sides and the agent on the fourth: an F2 event
    at step 1 finds no direction (plan 3.4)."""
    name = 'synthetic-B%s-%03d' % (agent[1], n)
    while True:
        truth = g3.random_truth(rng, need_solvable=False)
        free = [c for c in interior(truth) if not truth.walls[c[1]][c[0]]]
        rng.shuffle(free)
        found = None
        for c in free:
            open_sides = [d for d in DIRS if not truth.walls[rw.moved(c, d)[1]][rw.moved(c, d)[0]]]
            if len(open_sides) == 1:
                found = (c, rw.moved(c, open_sides[0]))
                break
        if found is None:
            continue
        w = truth.copy()
        w.object, w.agent = found
        if w.destination in found:
            continue
        if rw.is_valid(w):
            break
    e = random_event(rng, w, name, 1, 1, 'F2')
    events = [e] + [random_event(rng, w, name, j, rng.randint(2, 10)) for j in (2, 3)]
    return FaultCase(g3.case(name, agent, (200, rng.randint(1, 5), 30, 3), w), 0, 0, events)


def wall_f5_case(k):
    """Candidate k of a separate stream (seed SEED * 1000 + k): F1 walls near the object at every
    second step and F5 events at every third, C1 for odd k and C0 for even k."""
    rng = random.Random(SEED * 1000 + k)
    agent = 'C1' if k % 2 else 'C0'
    name = 'synthetic-W%s-%04d' % (agent[1], k)
    truth = g3.random_truth(rng)
    return FaultCase(g3.case(name, agent, (200, 5, 40, 3), truth), 0, 0, events_wall_f5(rng, truth, name))


def all_cases():
    rng = random.Random(SEED)
    out = named_fault_cases()
    out += demonstration_set('demo', 500003, 2)
    out += demonstration_set('seeds', 500009, 3)
    out += [synthetic_case(rng, n, 'C1') for n in range(1, C1_SYNTHETIC + 1)]
    out += [synthetic_case(rng, n, 'C0') for n in range(1, C0_SYNTHETIC + 1)]
    out += [trap_case(rng, n, 'C1') for n in range(1, TRAP_C1 + 1)]
    out += [trap_case(rng, n, 'C0') for n in range(1, TRAP_C0 + 1)]
    out += [boxed_case(rng, n, agent) for n, agent in enumerate(['C1', 'C1', 'C0', 'C0'], 1)]
    out += [wall_f5_case(k) for k in WALL_F5_PICKS]
    return out


def main():
    mode, folder = sys.argv[1], sys.argv[2]
    cases = all_cases()
    if mode == 'cases':
        lines = []
        for c in cases:
            lines += c.text()
        name = 'fault_cases.txt'
    else:
        import ref_fault_trace as rft
        lines = []
        for c in cases:
            lines += rft.trace(c)
        name = 'fault_traces.txt'
    with open(os.path.join(folder, name), 'w', newline='\n') as f:
        f.write('\n'.join(lines) + '\n')


if __name__ == '__main__':
    main()
