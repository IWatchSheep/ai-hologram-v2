"""The Stage 4 checks (plan STAGE_04, Section 6), mirrored on the reference.

Each check has the id the C++ test uses. Running this file prints every result and
`failing: none (<n> checks)`. df_check.py mutates the reference and reports which ids fail.
`python3 checks.py --fast` skips the slow checks: the whole-set material checks (LY5, FE2-FE4
on the pilot and confirmatory material) and the gate check G8."""
import importlib.util
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'stage3'))
import ref_agents as ra  # noqa: E402
import ref_fault_csv as rcsv  # noqa: E402
import ref_fault_demo as rdemo  # noqa: E402
import ref_fault_reader as rfr  # noqa: E402
import ref_fault_trace as rft  # noqa: E402
import ref_faults as rf  # noqa: E402
import ref_material as rmat  # noqa: E402
import ref_parts as rp  # noqa: E402
import ref_scorer as rs  # noqa: E402
import ref_verify as rc  # noqa: E402

rw = rp.rw
GOLDEN = os.path.join(HERE, 'golden')
SETTINGS = ra.Settings(200, 5, 100, 3)
D = ['##########', '#........#', '#........#', '#..A.O..D#', '#........#',
     '#........#', '#...##...#', '#........#', '#........#', '##########']


def _stage3_checks():
    spec = importlib.util.spec_from_file_location('stage3_checks', os.path.join(HERE, '..', 'stage3', 'checks.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


S3 = _stage3_checks()


def world(rows):
    return rw.parse_world(rows)[0]


def layout_d():
    return world(D)


def layout_d2():
    w = layout_d()
    w.walls[3][4] = True
    return w


def obs(step, state, action=None, outcome=None):
    return rc.Observation(step, state, action, outcome)


def c1(truth, belief=None):
    return ra.C1(obs(1, truth), SETTINGS, belief)


def c0(truth, belief=None):
    return ra.C0(obs(1, truth), SETTINGS, belief)


def event(id_='t', step=1, fault_type='F1', **fields):
    e = rmat.FaultEvent()
    e.id = id_
    e.step = step
    e.type = fault_type
    for k, v in fields.items():
        setattr(e, k, v)
    return e


def read(path):
    lines = open(path).read().split('\n')
    return lines[:-1] if lines and lines[-1] == '' else lines


# ---------------- 4a: seeded random numbers, the experimenter's search, layouts ----------------
def check_random():
    c = {}
    r = rmat.SeededRandom(0)
    c['RN1'] = [r.next() for _ in range(3)] == [16294208416658607535, 7960286522194355700, 487617019471545679]
    r = rmat.SeededRandom(1234567)
    c['RN2'] = [r.next() for _ in range(3)] == [6457827717110365317, 3203168211198807973, 9817491932198370423]
    r = rmat.SeededRandom(42)
    c['RN3'] = [r.below(10) for _ in range(10)] == [3, 1, 8, 4, 0, 2, 5, 8, 5, 4]
    r = rmat.SeededRandom(42)
    values = list(range(1, 11))
    r.shuffle(values)
    c['RN4'] = values == [1, 10, 6, 9, 7, 5, 8, 3, 2, 4]
    c['RN5'] = rmat.SeededRandom(7).pick_distinct(list(range(1, 11)), 3) == [8, 1, 5]
    a = rmat.SeededRandom(5)
    value = a.below(1)
    b = rmat.SeededRandom(5)
    b.next()
    c['RN6'] = value == 0 and a.next() == b.next()
    x = rmat.SeededRandom(99)
    y = rmat.SeededRandom(99)
    c['RN7'] = all(x.next() == y.next() for _ in range(1000)) and \
        rmat.SeededRandom(1).next() != rmat.SeededRandom(2).next()
    in_range = True
    seen = set()
    for n in (1, 2, 3, 7, 100, 1000):
        r = rmat.SeededRandom(2026)
        for _ in range(1000):
            v = r.below(n)
            in_range = in_range and 0 <= v < n
            if n == 7:
                seen.add(v)
    c['RN8'] = in_range and seen == set(range(7))
    return c


def dead_interior_count(w):
    return len(rmat.true_dead_interior(w))


def interior_walls(w):
    return sum(1 for y in range(1, 9) for x in range(1, 9) if w.walls[y][x])


LAYOUTS = [(500000, 3, 12, (7, 2), (6, 5), (2, 6), 25, 7), (500001, 6, 16, (8, 1), (4, 5), (8, 4), 23, 9),
           (500002, 1, 29, (4, 1), (4, 7), (8, 5), 27, 8), (500003, 3, 14, (1, 1), (4, 5), (7, 4), 26, 7),
           (900000, 2, 17, (7, 1), (7, 6), (2, 8), 19, 7), (900001, 4, 19, (4, 3), (3, 7), (5, 6), 26, 11)]
R500000 = ['##########', '#...#....#', '#.....#A.#', '##.......#', '#.#......#', '#.....O..#', '##D...#..#',
           '#........#', '#......#.#', '##########']
R900000 = ['##########', '#....#.A##', '##...#...#', '#........#', '#.....#..#', '#.#......#', '#......O.#',
           '#......#.#', '#.D......#', '##########']


def check_layouts(fast):
    c = {}
    d = layout_d()
    c['LY1a'] = rw.is_valid(d)
    c['LY1b'] = rmat.shortest_solution_length(d) == 4
    corner = d.copy()
    corner.object = (1, 1)
    c['LY1c'] = rw.is_valid(corner)
    c['LY1d'] = rmat.shortest_solution_length(corner) is None
    solved = d.copy()
    solved.object = (8, 3)
    c['LY1e'] = rw.is_valid(solved)
    c['LY1f'] = rmat.shortest_solution_length(solved) == 0
    bad = d.copy()
    bad.agent = (10, 3)
    c['LY1g'] = not rw.is_valid(bad) and rmat.shortest_solution_length(bad) is None
    for seed, attempts, shortest, agent, obj, dest, dead, walls in LAYOUTS:
        g = rmat.generate_layout(seed)
        c['LY2-%d' % seed] = (g.found and g.attempts == attempts and g.shortest == shortest
                              and g.world.agent == agent and g.world.object == obj
                              and g.world.destination == dest and dead_interior_count(g.world) == dead
                              and interior_walls(g.world) == walls)
    c['LY3a'] = rw.render_world(rmat.generate_layout(500000).world) == R500000
    c['LY3b'] = rw.render_world(rmat.generate_layout(900000).world) == R900000
    c['LY3c'] = all(rmat.generate_layout(500004 + i).attempts == a and rmat.generate_layout(500004 + i).shortest == s
                    for i, (a, s) in enumerate(zip([3, 1, 3, 8, 5, 2], [20, 16, 12, 12, 12, 16])))
    for seed in range(500000, 500010):
        g = rmat.generate_layout(seed)
        borders = all(g.world.walls[0][i] and g.world.walls[9][i] and g.world.walls[i][0] and g.world.walls[i][9]
                      for i in range(10))
        c['LY4-%d' % seed] = (g.found and rw.is_valid(g.world) and borders and 12 <= g.shortest <= 40
                              and rmat.shortest_solution_length(g.world) == g.shortest
                              and dead_interior_count(g.world) >= 1)
    if not fast:
        for name, first, count, attempts, longest in (('pilot', 900000, 40, 10, 35),
                                                      ('confirmatory', 100000, 400, 20, 38)):
            gs = [rmat.generate_layout(first + i) for i in range(count)]
            c['LY5-' + name] = (all(g.found for g in gs) and max(g.attempts for g in gs) == attempts
                                and min(g.shortest for g in gs) == 12 and max(g.shortest for g in gs) == longest)
    a = rmat.generate_layout(500002)
    b = rmat.generate_layout(500002)
    c['LY6'] = a.found == b.found and a.world.same(b.world) and a.attempts == b.attempts and a.shortest == b.shortest
    return c


# ---------------- 4b: fault-event lists ----------------
DEMO_TEXTS = ['500000-1-1 step 4 F1 target both | (6, 8) free->blocked', '500000-1-2 step 6 F3 target R1 | East right',
              '500000-1-3 step 10 F4 target R2 | (8, 2)', '500000-2-1 step 2 F1 target R1 | (1, 6) blocked->free',
              '500000-2-2 step 9 F2 target R2 | North South East West', '500000-2-3 step 10 F4 target R2 | (7, 1)',
              '500001-1-1 step 4 F2 target R1 | West North East South',
              '500001-1-2 step 6 F2 target R2 | West East South North', '500001-1-3 step 8 F5 target R2 | -',
              '500001-2-1 step 2 F1 target R1 | (1, 3) free->blocked',
              '500001-2-2 step 4 F2 target R1 | West East South North', '500001-2-3 step 10 F4 target both | (7, 1)',
              '500002-1-1 step 3 F3 target R1 | East right', '500002-1-2 step 6 F4 target R1 | (4, 1)',
              '500002-1-3 step 7 F1 target R2 | (2, 8) blocked->free', '500002-2-1 step 5 F3 target both | South right',
              '500002-2-2 step 8 F3 target R2 | West left', '500002-2-3 step 9 F5 target both | -',
              '500003-1-1 step 1 F5 target R1 | -', '500003-1-2 step 2 F4 target R1 | (1, 7)',
              '500003-1-3 step 8 F1 target R2 | (6, 8) free->blocked',
              '500003-2-1 step 1 F2 target both | East North South West', '500003-2-2 step 9 F3 target R2 | North right',
              '500003-2-3 step 10 F5 target R1 | -']


def material(first, count, run_seeds, master=4242):
    runs = []
    layouts = []
    for i in range(count):
        w = rmat.generate_layout(first + i).world
        for r in range(1, run_seeds + 1):
            runs.append((first + i, r))
            layouts.append(w)
    return runs, layouts, rmat.generate_fault_events(runs, layouts, master)


def texts(events):
    return [rmat.fault_event_text(e) for run in events for e in run]


def type_counts_ok(events, per_type, common, per_target):
    ok = True
    for t in rmat.TYPES:
        es = [e for run in events for e in run if e.type == t]
        ok = ok and len(es) == per_type and sum(1 for e in es if e.target == 'both') == common
        ok = ok and sum(1 for e in es if e.target == 'R1') == per_target
        ok = ok and sum(1 for e in es if e.target == 'R2') == per_target
    return ok


def directions_ok(events, half):
    f1 = [e for run in events for e in run if e.type == 'F1']
    cells = all(e.has_cell for run in events for e in run if e.type in ('F1', 'F4'))
    return cells and sum(1 for e in f1 if e.direction == 'free->blocked') == half and \
        sum(1 for e in f1 if e.direction == 'blocked->free') == half


def list_valid(runs, layouts, events):
    ok = len(events) == len(runs)
    for run, w in zip(events, layouts):
        dead = rp.compute_dead_squares(w, [], [], rp.Counts()).dead
        ok = ok and len(run) == 3
        for j, e in enumerate(run):
            ok = ok and e.index == j + 1 and 1 <= e.step <= 10 and (j == 0 or run[j - 1].step < e.step)
            inside = 1 <= e.cell[0] <= 8 and 1 <= e.cell[1] <= 8
            if e.has_cell and e.type == 'F1':
                wall = w.walls[e.cell[1]][e.cell[0]]
                ok = ok and inside and e.cell != w.destination and (wall if e.direction == 'blocked->free' else not wall)
            if e.has_cell and e.type == 'F4':
                ok = ok and inside and e.cell != w.destination and dead[e.cell[1]][e.cell[0]]
    return ok


def check_events(fast):
    c = {}
    runs, layouts, events = material(500000, 4, 2)
    c['FE1a'] = texts(events) == DEMO_TEXTS
    flat = [e for run in events for e in run]
    c['FE1b'] = all(e.common == (e.target == 'both') for e in flat) and sum(1 for e in flat if e.common) == 5
    c['FE1c'] = all(e.id == '%d-%d-%d' % (e.layout_seed, e.run_seed, e.index) and 1 <= e.index <= 3 for e in flat)
    c['FE5a'] = texts(material(500000, 4, 2)[2]) == texts(events) and len(texts(events)) == 24
    other = material(500000, 4, 2, 4243)[2]
    c['FE5b'] = rmat.fault_event_text(other[0][0]) == '500000-1-1 step 1 F3 target R1 | East left'
    c['FE5c'] = rmat.generate_fault_events([(500000, 1)], [], 4242) == []
    c['FE6a'] = [rmat.common_mode_count(n) for n in (2, 3, 5, 8, 48, 1200)] == [0, 1, 1, 2, 10, 240]
    c['FE6b'] = rmat.run_stream_seed(4242, 500000, 1) == (4242 << 40) + (500000 << 8) + 1
    c['FE4a'] = list_valid(runs, layouts, events)
    if not fast:
        p_runs, p_layouts, pilot = material(900000, 40, 2)
        c['FE2a'] = len(pilot) == 80 and type_counts_ok(pilot, 48, 10, 19)
        c['FE2b'] = directions_ok(pilot, 24)
        c['FE2c'] = [rmat.fault_event_text(e) for e in pilot[0]] == [
            '900000-1-1 step 1 F4 target both | (4, 1)', '900000-1-2 step 2 F5 target both | -',
            '900000-1-3 step 8 F3 target both | West right']
        f_runs, f_layouts, conf = material(100000, 400, 5)
        c['FE3a'] = len(conf) == 2000 and type_counts_ok(conf, 1200, 240, 480)
        c['FE3b'] = directions_ok(conf, 600)
        steps = [0] * 10
        for run in conf:
            for e in run:
                steps[e.step - 1] += 1
        c['FE3c'] = steps == [580, 620, 603, 578, 609, 577, 614, 597, 605, 617]
        c['FE3d'] = [rmat.fault_event_text(e) for e in conf[0]] == [
            '100000-1-1 step 2 F2 target both | South West East North',
            '100000-1-2 step 7 F2 target both | South North East West',
            '100000-1-3 step 8 F3 target R2 | South right'] and [rmat.fault_event_text(e) for e in conf[-1]] == [
            '100399-5-1 step 2 F5 target R2 | -', '100399-5-2 step 5 F4 target R1 | (8, 3)',
            '100399-5-3 step 6 F1 target both | (1, 3) free->blocked']
        c['FE4b'] = list_valid(p_runs, p_layouts, pilot)
        c['FE4c'] = list_valid(f_runs, f_layouts, conf)
    return c


# ---------------- 4c: applying faults, activation records, the run with faults ----------------
def apply(agent, e, history=()):
    return rf.try_apply_fault(e, agent, {}, list(history))


def check_apply():
    c = {}
    d = layout_d()
    c['FI1a'] = rw.is_valid(d)
    a = c1(d)
    c['FI1b'] = apply(a, event(has_cell=True, cell=(3, 3))) == (False, 'the wall would be under the believed agent')
    a = c1(d)
    c['FI1c'] = apply(a, event(has_cell=True, cell=(5, 3))) == (False, 'the wall would be under the believed object')
    a = c1(d)
    c['FI1d'] = apply(a, event(has_cell=True, cell=(6, 5)))[0] and a.belief.walls[5][6]
    a = c1(d)
    c['FI1e'] = apply(a, event(direction='blocked->free', has_cell=True, cell=(4, 6)))[0] and not a.belief.walls[6][4]
    a = c1(d)
    c['FI1f'] = apply(a, event(has_cell=False)) == (False, 'no cell of the required kind')
    order = ['North', 'East', 'South', 'West']
    a = c1(d)
    c['FI2a'] = apply(a, event(fault_type='F2', order=order))[0] and a.belief.object == (5, 2)
    a = c1(d)
    a.belief.object = (1, 1)
    a.belief.walls[1][2] = True
    a.belief.walls[2][1] = True
    c['FI2b'] = rw.is_valid(a.belief)
    c['FI2c'] = apply(a, event(fault_type='F2', order=order)) == (False, 'no direction passes the test')
    a = c1(d)
    a.belief.agent = (5, 2)
    c['FI2d'] = apply(a, event(fault_type='F2', order=order))[0] and a.belief.object == (6, 3)
    return c


def check_f3_f4():
    c = {}
    d = layout_d()
    a = c1(d)
    faulted = {}
    first = rf.try_apply_fault(event(fault_type='F3', action='East', rotation='right'), a, faulted, [])
    inp = d.copy()
    inp.agent = (4, 3)
    c['FI3a'] = rw.is_valid(inp)
    direct = rf.make_faulty_model(faulted)(inp, 'East')
    result = a.model.current()(inp, 'East')
    c['FI3b'] = (first[0] and result[1] == 'Pushed' and result[0].agent == (5, 3) and result[0].object == (5, 4)
                 and direct[0].object == (5, 4) and rw.apply_action(inp, 'East')[0].object == (6, 3))
    walled = inp.copy()
    walled.walls[4][5] = True
    blocked = a.model.current()(walled, 'East')
    c['FI3c'] = blocked[1] == 'Blocked' and blocked[0].same(walled)
    west = a.model.current()(inp, 'West')
    c['FI3d'] = west[1] == 'Moved' and west[0].agent == (3, 3)
    second = rf.try_apply_fault(event(fault_type='F3', action='North', rotation='left'), a, faulted, [])
    north_in = d.copy()
    north_in.agent = (5, 4)
    north = a.model.current()(north_in, 'North')
    c['FI3e'] = (second[0] and a.model.current()(inp, 'East')[0].object == (5, 4) and north[1] == 'Pushed'
                 and north[0].agent == (5, 3) and north[0].object == (4, 3))
    a.model.restore('East')
    c['FI3f'] = (a.model.current()(inp, 'East')[0].object == (6, 3)
                 and a.model.current()(north_in, 'North')[0].object == (4, 3))
    a.model.set_base(rf.make_faulty_model(faulted))
    c['FI3g'] = (a.model.is_restored('East') and a.model.current()(inp, 'East')[0].object == (6, 3)
                 and a.model.current()(north_in, 'North')[0].object == (4, 3))
    a = c1(d)

    def dead_count(m):
        return sum(1 for row in m.dead for v in row if v)
    before = dead_count(a.dead_cache.get(a.belief, a.not_dead, a.dead_overrides, a.counts))
    out = apply(a, event(fault_type='F4', has_cell=True, cell=(1, 1)))
    after = a.dead_cache.get(a.belief, a.not_dead, a.dead_overrides, a.counts)
    c['FI4a'] = out[0] and not after.dead[1][1] and before == 22 and dead_count(after) == 21
    a.dead_overrides.append((1, 1))
    c['FI4b'] = a.dead_cache.get(a.belief, a.not_dead, a.dead_overrides, a.counts).dead[1][1]
    c['FI4c'] = apply(a, event(fault_type='F4', has_cell=False)) == (False, 'the layout has no true dead square')
    return c


def check_f5_cache():
    c = {}
    d = layout_d()
    c['FI5a'] = rw.is_valid(d)
    s2 = d.copy()
    s2.agent = (4, 3)
    s3 = s2.copy()
    s3.agent = (5, 3)
    s3.object = (6, 3)
    s4 = s3.copy()
    s4.agent = (5, 2)
    c['FI5b'] = rw.is_valid(s2) and rw.is_valid(s3) and rw.is_valid(s4)
    o = [obs(1, d), obs(2, s2), obs(3, s3), obs(4, s4)]
    f5 = event(fault_type='F5')
    a = c1(d)
    a.belief = s3.copy()
    c['FI5c'] = apply(a, f5, o[:3]) == (False, 'the previous square is the believed agent square')
    a = c1(d)
    a.belief = s4.copy()
    c['FI5d'] = apply(a, f5, o)[0] and a.belief.object == (5, 3)
    a = c1(d)
    c['FI5e'] = apply(a, f5, o[:1]) == (False, 'the object has not moved yet')
    a = c1(d)
    b = s4.copy()
    b.walls[3][5] = True
    a.belief = b
    c['FI5f'] = apply(a, f5, o) == (False, 'the previous square is believed to be a wall')
    a = c1(d)
    a.predictor.predict(a.belief, 'East')
    before = a.predictor.is_cached(a.belief, 'East')
    applied = apply(a, event(has_cell=True, cell=(6, 5)))
    c['FI6a'] = before and applied[0] and not a.predictor.is_cached(a.belief, 'East')
    a.predictor.predict(a.belief, 'East')
    again = a.predictor.is_cached(a.belief, 'East')
    refused = apply(a, event(has_cell=True, cell=(3, 3)))
    c['FI6b'] = again and not refused[0] and a.predictor.is_cached(a.belief, 'East')
    return c


def demo_material():
    return material(500000, 4, 2)


def demo_run(index, single=True, observe=True, mat=None):
    runs, layouts, events = mat if mat is not None else demo_material()
    w = layouts[index]
    agent = c1(w) if single else c0(w)
    r, _, _ = rf.run_with_faults(w, agent, SETTINGS, events[index], observe)
    return r, agent


def record(r, id_):
    return next(x for x in r.activations if x.event_id == id_)


def run_faults(truth, agent, events, observe=True, **watchers):
    return rf.run_with_faults(truth, agent, SETTINGS, events, observe, **watchers)[0]


def check_activation():
    c = {}
    mat = demo_material()
    r, _ = demo_run(6, mat=mat)
    first = record(r, '500003-1-1')
    c['FI7a'] = (first.status == 'applied' and first.application_step == 9 and rf.activation_delay(first) == 8
                 and rf.reported_as(first) == 'Activated after deferral')
    r, _ = demo_run(2, mat=mat)
    second = record(r, '500001-1-3')
    c['FI7b'] = (second.status == 'pending' and second.reason == 'the object has not moved yet'
                 and rf.reported_as(second) == 'Not activated: deferred, never applicable')
    d = layout_d()
    r = run_faults(d, c1(d), [event(step=10, fault_type='F2', order=['North', 'East', 'South', 'West'])])
    c['FI8a'] = (r.outcome == 'Success' and r.world_steps == 4 and len(r.activations) == 1
                 and rf.reported_as(r.activations[0]) == 'Not activated: run ended before the scheduled step')
    r = run_faults(d, c1(d), [event(has_cell=True, cell=(3, 3))])
    c['FI8b'] = (r.outcome == 'Success' and r.world_steps == 4 and r.evaluations == 19 and len(r.activations) == 1
                 and rf.reported_as(r.activations[0]) == 'Not activated: not applicable at the scheduled step'
                 and r.activations[0].reason == 'the wall would be under the believed agent')
    rec = rf.ActivationRecord(event(step=5))
    rec.status = 'applied'
    rec.application_step = 5
    ok = rf.activation_delay(rec) == 0 and rf.reported_as(rec) == 'Activated at its scheduled step'
    rec.application_step = 8
    ok = ok and rf.activation_delay(rec) == 3 and rf.reported_as(rec) == 'Activated after deferral'
    sched = rf.ActivationRecord(event(step=0))
    ok = ok and rf.activation_delay(sched) == 0 and \
        rf.reported_as(sched) == 'Not activated: run ended before the scheduled step'
    sched.status = 'not-applicable'
    ok = ok and rf.reported_as(sched) == 'Not activated: not applicable at the scheduled step'
    sched.status = 'pending'
    ok = ok and rf.activation_delay(sched) == 0 and rf.reported_as(sched) == 'Not activated: deferred, never applicable'
    c['FI8c'] = ok
    return c


def same_run(r, outcome, steps, lines, evaluations):
    return r.outcome == outcome and r.world_steps == steps and r.lines == lines and r.evaluations == evaluations


def plain_c1(truth, belief=None):
    a = c1(truth, belief)
    outcome, steps, lines = ra.c1_run_lines(truth, a, SETTINGS)
    return outcome, steps, lines, a.predictor.used_total


def plain_c0(truth, belief=None):
    a = c0(truth, belief)
    outcome, steps, lines = ra.c0_run_lines(truth, a, SETTINGS)
    return outcome, steps, lines, a.predictor.used_total


DEMO1 = event('demo-1', 1, 'F1', direction='blocked->free', has_cell=True, cell=(4, 3))


def check_fault_runs():
    c = {}
    d = layout_d()
    d2 = layout_d2()
    c['FR1a'] = rw.is_valid(d) and rw.is_valid(d2)
    r = run_faults(d2, c1(d2), [DEMO1])
    expected = plain_c1(d2, d)
    c['FR1b'] = (same_run(r, *expected) and r.evaluations == 414 and len(r.activations) == 1
                 and r.activations[0].status == 'applied' and r.activations[0].application_step == 1
                 and rf.reported_as(r.activations[0]) == 'Activated at its scheduled step')
    c['FR4a'] = rw.is_valid(d) and rw.is_valid(d2)
    for name, truth, belief, single in (('b', d, None, True), ('c', d2, d, True), ('d', d, None, False),
                                        ('e', d2, d, False)):
        agent = c1(truth, belief) if single else c0(truth, belief)
        r = run_faults(truth, agent, [])
        plain = plain_c1(truth, belief) if single else plain_c0(truth, belief)
        c['FR4' + name] = same_run(r, *plain) and agent.activation_hook is None
    c['IE-T1a'] = plain_c1(d)[2] == S3.C1_1 and plain_c1(d)[3] == 19
    c['IE-T1b'] = plain_c1(d2, d)[2] == S3.C1_2 and plain_c1(d2, d)[3] == 414
    mat = demo_material()
    first, _ = demo_run(5, mat=mat)
    second, _ = demo_run(5, mat=mat)
    c['FR5'] = (first.lines == second.lines and first.outcome == second.outcome
                and first.decision_steps == second.decision_steps
                and [(x.event_id, x.status, x.scheduled_step, x.application_step, x.reason) for x in first.activations]
                == [(x.event_id, x.status, x.scheduled_step, x.application_step, x.reason) for x in second.activations])
    goal = d.copy()
    goal.object = goal.destination
    c['FR6a'] = rw.is_valid(goal)
    r = run_faults(goal, c1(goal), [event(fault_type='F2', order=['North', 'East', 'South', 'West'])])
    c['FR6b'] = (r.outcome == 'Success' and r.world_steps == 0 and r.decision_steps == 0 and len(r.activations) == 1
                 and rf.reported_as(r.activations[0]) == 'Not activated: run ended before the scheduled step')
    calls = []
    r = run_faults(d2, c1(d2), [DEMO1], watch_decision=lambda o, dec, i, s: calls.append(('d', o.step)),
                   watch_final=lambda o, i, s: calls.append(('f', o.step)))
    plain = run_faults(d2, c1(d2), [DEMO1])
    c['FR7'] = (calls == [('d', k) for k in range(1, 19)] + [('f', 19)] and r.lines == plain.lines
                and r.evaluations == plain.evaluations)
    return c


def check_run_watchers():
    c = {}
    d = layout_d()
    d2 = layout_d2()
    seen = []
    a = c1(d2, d)
    outcome, steps = rf.run_c1_watched(d2, a, SETTINGS, lambda ag, o: seen.append((o.step, o.state.object)))
    lines = a.log + ra.result_lines(outcome, steps, a.safe_stops, len(a.mismatches), len(a.corrections),
                                     a.verified_count(), a.predictor.used_total)
    c['RL2'] = ([s for s, _ in seen] == list(range(1, 20)) and seen[-1][1] == d2.destination and lines == S3.C1_2)
    seen = []
    a = c0(d)
    outcome, steps = rf.run_c0_watched(d, a, SETTINGS, lambda ag, o: seen.append(o.step))
    c['RL3'] = seen == [1, 2, 3, 4] and outcome == 'Success' and steps == 4 and a.predictor.used_total == 16
    return c


# ---------------- 4d: scoring I and II ----------------
def check_scorer():
    c = {}
    d = layout_d()
    d2 = layout_d2()
    c['SC1a'] = rw.is_valid(d2)
    r = run_faults(d2, c1(d2), [DEMO1])
    s = r.event_scores[0]
    c['SC1b'] = (s.activated and s.activation_step == 1 and s.revealing and s.opportunity_step == 1
                 and s.mismatch_step == 2 and s.strict_step == 2 and s.end_state == 'corrected' and s.end_step == 2
                 and s.correcting_id == 1 and s.correction_status == 'VERIFIED')
    rs_ = r.score
    c['SC1c'] = (rs_.claims, rs_.false_alarms, rs_.wrong_accepted, rs_.mismatches, rs_.mismatches_without_cause,
                 rs_.labels_correct, rs_.labels_scored, rs_.justified_safe_stops) == (1, 0, 0, 1, 0, 1, 1, 0)
    c['SC2a'] = rw.is_valid(d) and rw.is_valid(d2)
    r = run_faults(d2, c1(d2, d), [])
    rs_ = r.score
    c['SC2b'] = (rs_.claims, rs_.false_alarms, rs_.wrong_accepted, rs_.mismatches, rs_.mismatches_without_cause,
                 rs_.labels_scored, rs_.labels_correct) == (1, 1, 0, 1, 1, 0, 0)
    c['SC3a'] = rw.is_valid(d) and rw.is_valid(d2)
    wall = rc.Correction()
    wall.id = 1
    wall.element = rc.Element('wall', (4, 3))
    wall.new_blocked = True
    c['SC3b'] = rs.claim_value_is_true(wall, d2, [], []) and not rs.claim_value_is_true(wall, d, [], [])
    model = rc.Correction()
    model.element = rc.Element('model', action='East')
    c['SC3c'] = rs.claim_value_is_true(model, d, [], [])
    trap = rc.Correction()
    trap.element = rc.Element('trap', (1, 1))
    ok = rs.claim_value_is_true(trap, d, [], [])
    trap.element = rc.Element('trap', (5, 3))
    c['SC3d'] = ok and not rs.claim_value_is_true(trap, d, [], [])
    second = d.copy()
    second.agent = (4, 3)
    third = d.copy()
    third.agent = (5, 3)
    third.object = (6, 3)
    c['SC3e'] = rw.is_valid(second) and rw.is_valid(third)
    history = [obs(1, d), obs(2, second, 'East', 'Moved'), obs(3, third, 'East', 'Pushed')]
    mismatch = rc.Mismatch(3, 'East', (False, []), [], 'S2', False)
    obj = rc.Correction()
    obj.element = rc.Element('object')
    obj.new_position = (5, 3)
    ok = rs.claim_value_is_true(obj, d, history, [mismatch])
    obj.new_position = (6, 3)
    c['SC3f'] = ok and not rs.claim_value_is_true(obj, d, history, [mismatch]) and \
        not rs.claim_value_is_true(obj, d, history, [])
    return c


def wall_record(cid, new, old, step):
    r = rc.Correction()
    r.id = cid
    r.element = rc.Element('wall', (4, 3))
    r.new_blocked = new
    r.old_blocked = old
    r.status = 'CONSISTENT'
    r.step_consistent = step
    return r


def check_scorer_life():
    c = {}
    d = layout_d()
    d2 = layout_d2()
    c['SC4a'] = rw.is_valid(d2)
    a = c1(d2)
    history = [obs(1, d2)]
    e = event('E', 2, 'F1', direction='blocked->free', has_cell=True, cell=(4, 3))
    scorer = rs.Scorer([e], d2, history)
    view = rs.view_of(a)
    records = [rf.ActivationRecord(e)]
    act = ra.Decision(True, 'North')
    scorer.end_phase_a(view, 1)
    scorer.after_activation(view, records, 1)
    scorer.after_decide(view, act)
    a.corrections.append(wall_record(1, True, True, 2))
    scorer.end_phase_a(view, 2)
    c['SC4b'] = scorer.run.false_alarms == 1 and scorer.run.claims == 1 and not scorer.scores[0].activated
    a.belief.walls[3][4] = False
    records[0].status = 'applied'
    records[0].application_step = 2
    scorer.after_activation(view, records, 2)
    s = scorer.scores[0]
    c['SC4c'] = s.activated and s.activation_step == 2 and s.active
    scorer.after_decide(view, act)
    a.corrections.append(wall_record(2, True, False, 3))
    a.belief.walls[3][4] = True
    scorer.end_phase_a(view, 3)
    c['SC4d'] = (s.strict_step == 3 and s.end_state == 'corrected' and s.end_step == 3 and s.correcting_id == 2
                 and scorer.run.false_alarms == 1)
    scorer.after_activation(view, records, 3)
    scorer.after_decide(view, act)
    a.corrections[1].status = 'REJECTED'
    a.belief.walls[3][4] = False
    scorer.end_phase_a(view, 4)
    c['SC4e'] = s.end_state == 'none' and s.active and s.correcting_id == 0
    scorer.after_activation(view, records, 4)
    scorer.after_decide(view, ra.Decision(False, 'North', 'test'))
    c['SC4f'] = scorer.run.justified_safe_stops == 1
    scorer.end_final_check(view)
    scorer.finish(view)
    c['SC4g'] = (s.end_state == 'none' and s.strict_step == 3 and scorer.run.claims == 2
                 and scorer.run.false_alarms == 1 and scorer.run.wrong_accepted == 0 and s.correction_status is None)
    c['SC6a'] = rw.is_valid(d) and rw.is_valid(d2)
    agent = c1(d2)
    r = run_faults(d2, agent, [DEMO1])
    expected = plain_c1(d2, d)
    c['SC6b'] = (r.lines == expected[2] and r.outcome == expected[0] and r.evaluations == expected[3]
                 and agent.activation_hook is None)
    return c


# ---------------- 4e: shadow evaluation ----------------
class FixedView:
    def __init__(self, belief, model=None, not_dead=None, dead=None):
        self.belief = belief
        self.model = model
        self.not_dead = not_dead
        self.dead_overrides = dead
        self.corrections = None
        self.mismatches = None
        self.committed = None


def prediction(model, state, action):
    s, o = model(state, action)
    return (action, o, s)


def check_shadow():
    c = {}
    belief = layout_d()
    truth = layout_d2()
    c['SH1-F1a'] = rw.is_valid(belief) and rw.is_valid(truth)
    e = event(fault_type='F1', has_cell=True, cell=(4, 3), direction='blocked->free')
    am = rp.AgentActionModel()
    view = FixedView(belief, am, [], [])
    model = am.current()
    c['SH1-F1b'] = (rs.evaluation_effective(e, belief, 'East', model, prediction(model, belief, 'East'), view,
                                            truth, truth.object)
                    and not rs.evaluation_effective(e, belief, 'North', model, prediction(model, belief, 'North'),
                                                    view, truth, truth.object))
    belief = layout_d()
    belief.object = (5, 2)
    truth = layout_d()
    c['SH1-F2a'] = rw.is_valid(belief) and rw.is_valid(truth)
    e = event(fault_type='F2', has_cell=True)
    view = FixedView(belief, am, [], [])
    state = belief.copy()
    state.agent = (4, 2)
    c['SH1-F2b'] = rw.is_valid(state)
    c['SH1-F2c'] = (rs.evaluation_effective(e, state, 'East', model, prediction(model, state, 'East'), view, truth,
                                            (5, 3))
                    and not rs.evaluation_effective(e, state, 'West', model, prediction(model, state, 'West'), view,
                                                    truth, (5, 3)))
    odd = belief.copy()
    odd.agent = (5, 3)
    c['SH1-F2d'] = rw.is_valid(odd)
    c['SH1-F2e'] = rs.evaluation_effective(e, odd, 'North', model, prediction(model, odd, 'North'), view, truth,
                                           (5, 3))
    belief = layout_d()
    c['SH1-F3a'] = rw.is_valid(belief)
    e = event(fault_type='F3', action='East', rotation='right')
    faulty = rf.make_faulty_model({'East': 'right'})
    am = rp.AgentActionModel(faulty)
    view = FixedView(belief, am, [], [])
    state = belief.copy()
    state.agent = (4, 3)
    c['SH1-F3b'] = rw.is_valid(state)
    standard = rp.standard_rule
    c['SH1-F3c'] = (rs.evaluation_effective(e, state, 'East', faulty, prediction(faulty, state, 'East'), view, belief,
                                            belief.object)
                    and not rs.evaluation_effective(e, state, 'East', standard, prediction(standard, state, 'East'),
                                                    view, belief, belief.object)
                    and not rs.evaluation_effective(e, state, 'West', faulty, prediction(faulty, state, 'West'),
                                                    view, belief, belief.object))
    c['SH1-F4a'] = rw.is_valid(belief)
    e = event(fault_type='F4', has_cell=True, cell=(1, 1))
    view = FixedView(belief, None, [(1, 1)], [])
    shadow = rs.shadow_map(e, view, belief)
    real = rp.compute_dead_squares(belief, [(1, 1)], [], rp.Counts())
    c['SH1-F4b'] = (shadow is not None and shadow.dead[1][1] and not real.dead[1][1]
                    and shadow.dead[2][2] == real.dead[2][2])
    state = belief.copy()
    state.agent = (3, 3)
    state.object = (1, 1)
    c['SH1-F4c'] = rw.is_valid(state)
    c['SH1-F4d'] = rp.heuristic(state, real, rp.Counts()) == 12 and rp.heuristic(state, shadow, rp.Counts()) == rp.INF
    return c


class Recorder:
    def __init__(self):
        self.cells = []
        self.values = []
        self.heuristics = []
        self.evaluations = 0
        self.on_evaluation = self._evaluation
        self.on_dead_read = self._dead
        self.on_heuristic = self._heuristic

    def _evaluation(self, *_):
        self.evaluations += 1

    def _dead(self, cell, value):
        self.cells.append(cell)
        self.values.append(value)

    def _heuristic(self, _, value):
        self.heuristics.append(value)


def check_observed_runs():
    c = {}
    d = layout_d()
    d2 = layout_d2()
    c['SH2a'] = rw.is_valid(d) and rw.is_valid(d2)
    for name, truth, belief, single in (('C1-D', d, None, True), ('C1-D2', d2, d, True), ('C0-D', d, None, False),
                                        ('C0-D2', d2, d, False)):
        recorder = Recorder()
        agent = c1(truth, belief) if single else c0(truth, belief)
        agent.predictor.observer = recorder
        agent.dead_cache.observer = recorder
        if single:
            outcome, steps, lines = ra.c1_run_lines(truth, agent, SETTINGS)
            plain = plain_c1(truth, belief)
        else:
            outcome, steps, lines = ra.c0_run_lines(truth, agent, SETTINGS)
            plain = plain_c0(truth, belief)
        c['SH2-' + name] = (outcome, steps, lines, agent.predictor.used_total) == plain
        c['SH3-' + name] = recorder.evaluations == agent.predictor.used_total
    mat = demo_material()
    for i in range(8):
        name = '%d-%d' % (500000 + i // 2, i % 2 + 1)
        on1, _ = demo_run(i, True, True, mat)
        on0, _ = demo_run(i, False, True, mat)
        off1, _ = demo_run(i, True, False, mat)
        off0, _ = demo_run(i, False, False, mat)
        c['SH3-' + name] = on1.observed_evaluations == on1.evaluations and on0.observed_evaluations == on0.evaluations
        c['SH4-' + name] = (primary(on1) == primary(off1) and primary(on0) == primary(off0)
                            and off1.observed_evaluations == 0 and off0.observed_evaluations == 0)
    c['SH5a'] = rw.is_valid(d2)
    r = run_faults(d2, c1(d2), [DEMO1])
    c['SH5b'] = r.event_scores[0].opportunity_step == 1
    return c


def primary(r):
    s = r.score
    return (r.outcome, r.evaluations, s.claims, s.false_alarms, s.wrong_accepted, s.justified_safe_stops,
            s.mismatches, s.mismatches_without_cause, s.labels_correct, s.labels_scored,
            [(x.strict_step, x.mismatch_step, x.end_state, x.end_step, x.correcting_id, x.correction_status)
             for x in r.event_scores])


def values_match(recorder, m):
    return all(rw.is_inside(cell) and m.dead[cell[1]][cell[0]] == v for cell, v in zip(recorder.cells, recorder.values))


def check_read_sites():
    c = {}
    d = layout_d()
    c['SH6.1a'] = rw.is_valid(d)
    m = rp.compute_dead_squares(d, [], [], rp.Counts())
    rec = Recorder()
    m.observer = rec
    value = rp.heuristic(d, m, rp.Counts())
    c['SH6.1b'] = (value == 4 and rec.cells == [(5, 3)] and rec.values == [False] and rec.heuristics == [4]
                   and values_match(rec, m))
    m = rp.compute_dead_squares(d, [], [], rp.Counts())
    rec = Recorder()
    m.observer = rec
    predictor = rp.StepPredictor(rp.standard_rule, 200)
    predictor.start_step()
    result = rp.search(d, m, lambda s: s.object == s.destination, lambda s: 0, predictor, 1)
    c['SH6.2'] = result.kind == 'Found' and rec.cells and not rec.heuristics and values_match(rec, m)
    rec = Recorder()
    a = c1(d)
    a.predictor.start_step()
    a.dead_cache.observer = rec
    pick = a.choose_cautious()
    a.dead_cache.observer = None
    c['SH6.3'] = pick is not None and any(cell == (5, 3) and not v for cell, v in zip(rec.cells, rec.values))
    m = rp.compute_dead_squares(d, [], [], rp.Counts())
    rec = Recorder()
    m.observer = rec
    outcome = rc.check_consequence(d, m, [obs(1, d)], rp.Counts())
    c['SH6.4'] = (outcome == 'FullStateDeadEnd' and rec.cells == [(5, 3), (5, 2)] and rec.values == [False, False]
                  and values_match(rec, m))
    m = rp.compute_dead_squares(d, [], [], rp.Counts())
    rec = Recorder()
    m.observer = rec
    c['SH6.5'] = (rp.local_check_says_not_dead(d, m, (5, 3), rp.Counts()) and rec.cells == [(5, 2)]
                  and rec.values == [False] and values_match(rec, m))
    m = rp.compute_dead_squares(d, [], [], rp.Counts())
    rec = Recorder()
    m.observer = rec
    predictor = rp.StepPredictor(rp.standard_rule, 200)
    predictor.start_step()
    anomalies = [0]
    result = rc.trap_v2_test(d, m, (5, 3), predictor, rp.standard_rule, 1, anomalies)[0]
    c['SH6.6'] = (result == 'Rejected' and any(cell == (6, 3) and not v for cell, v in zip(rec.cells, rec.values))
                  and values_match(rec, m))
    return c


# ---------------- 4f: CSV, the demonstration ----------------
CSV3_ROW = ('500001-1-3,C1,R,500001,1,F5,R2,0,,,,,,8,Not activated: deferred, never applicable,,,'
            'the object has not moved yet,,,,,,,,,,,,')


def check_csv():
    c = {}
    c['CSV1a'] = rcsv.RUN_HEADER == S3_RUN_HEADER
    c['CSV1b'] = rcsv.EVENT_HEADER == S3_EVENT_HEADER
    runs, layouts, events = demo_material()
    agent = c1(layouts[2])
    r, _, _ = rf.run_with_faults(layouts[2], agent, SETTINGS, events[2], True)
    c['CSV3a'] = rcsv.event_row('C1', events[2][2], r.activations[2], r.event_scores[2]) == CSV3_ROW
    c['CSV3b'] = agent.dead_cache.map.observer is None and agent.dead_cache.observer is None
    path = os.path.join('/tmp', 'stage4_csv_test.csv')
    text = 'a,b\n1,2\n'
    with open(path, 'w', newline='') as f:
        f.write(text)
    c['CSV4a'] = os.path.exists(path)
    c['CSV4b'] = open(path, newline='').read() == text
    os.remove(path)
    c['CSV4c'] = not os.path.exists(path)
    c['CSV4d'] = not os.path.isdir('no_such_folder_stage4')
    expected = read(os.path.join(HERE, '..', 'stage4', 'expected_output_stage4.txt'))[102:170]
    c['FD1'] = rdemo.fault_demo_lines() == expected
    return c


S3_RUN_HEADER = ('run_id,condition,layout_seed,run_seed,outcome,decision_steps,world_steps,safe_stops,'
                 'justified_safe_stops,mismatches,mismatches_without_fault_cause,source_labels_correct,'
                 'source_labels_scored,claims,false_alarms,wrong_accepted_corrections,evaluations,events_scheduled,'
                 'events_activated')
S3_EVENT_HEADER = ('event_id,condition,reasoner,layout_seed,run_seed,type,target,common_mode,cell,direction,order,'
                   'action,rotation,scheduled_step,reported_as,activation_step,activation_delay,not_activated_reason,'
                   'opportunity,opportunity_step,revealing_opportunity,mismatch_detected,mismatch_step,'
                   'strictly_detected,strict_step,detection_delay,end_state,end_step,correction_status,'
                   'verified_correction')


# ---------------- 4g: the fault-case reader, the trace text, the gate ----------------
BASE = ['case x', 'agent C1', 'settings 200 5 100 3', 'world'] + D + ['belief true', 'model standard', 'notdead 0']
EVENTS_ALL = ['event x-1 7 2 1 1 F1 R1 0 cell 4 3 blocked->free', 'event x-2 7 2 2 2 F1 both 1 cell none free->blocked',
              'event x-3 7 2 3 3 F2 R2 0 West North East South', 'event x-4 7 2 4 4 F3 R1 0 South left',
              'event x-5 7 2 5 5 F4 R2 0 cell 1 1', 'event x-6 7 2 6 6 F4 both 1 cell none',
              'event x-7 7 2 7 7 F5 R1 0 -']


def case_lines(name='x', middle=None, tail=None):
    lines = list(BASE)
    lines[0] = 'case ' + name
    return lines + (middle if middle is not None else ['run 7 2']) + (tail if tail is not None else []) + ['end']


READER_CASES = [
    ('FN1', case_lines()[:-1], 'case 1: missing end line'),
    ('FN2', case_lines(middle=[]), 'case 1: missing run line'),
    ('FN3', ['case x', 'agent C2'] + case_lines()[2:], 'case 1: malformed agent line'),
    ('FN4', case_lines(middle=['run 7']), 'case 1: malformed run line'),
    ('FN5', case_lines(tail=['event x-1 7 2 1 1 F6 R1 0 -']), 'case 1: malformed event line'),
    ('FN6', case_lines(tail=['event x-1 7 2 1 1 F1 R1 0 cell 4 blocked->free']), 'case 1: malformed event line'),
    ('FN7', case_lines(tail=['event x-1 7 2 1 1 F2 R1 0 North East South']), 'case 1: malformed event line'),
    ('FN8', case_lines(tail=['event x-1 7 2 1 1 F3 R1 0 North up']), 'case 1: malformed event line'),
    ('FN9', case_lines() + ['event x-1 7 2 1 1 F5 R1 0 -'], 'line 20: expected a case header'),
]


def check_reader():
    c = {}
    for cid, lines, problem in READER_CASES:
        c[cid] = rfr.read_fault_cases(lines) == (False, [], problem)
    ok, cases, problem = rfr.read_fault_cases(case_lines('x', tail=EVENTS_ALL) + case_lines('y', ['run 0 0']))
    good = ok and problem == '' and len(cases) == 2 and cases[1].events == [] and cases[1].base.name == 'y'
    if good:
        es = cases[0].events
        good = (cases[0].layout_seed == 7 and cases[0].run_seed == 2 and len(es) == 7
                and [e.type for e in es] == ['F1', 'F1', 'F2', 'F3', 'F4', 'F4', 'F5']
                and es[0].has_cell and es[0].cell == (4, 3) and es[0].direction == 'blocked->free'
                and not es[1].has_cell and es[1].common and es[1].target == 'both'
                and es[2].order == ['West', 'North', 'East', 'South'] and es[3].action == 'South'
                and es[3].rotation == 'left' and es[4].cell == (1, 1) and not es[5].has_cell
                and es[6].id == 'x-7' and es[6].step == 7 and es[6].index == 7 and es[6].layout_seed == 7)
    c['FN10'] = good
    rec = rf.ActivationRecord(event('a-1', 3))
    lines = [rft.activation_text(rec)]
    rec.status = 'pending'
    rec.reason = 'the object has not moved yet'
    lines.append(rft.activation_text(rec))
    rec.status = 'applied'
    rec.application_step = 5
    rec.reason = ''
    lines.append(rft.activation_text(rec))
    rec.status = 'not-applicable'
    rec.reason = 'no cell of the required kind'
    lines.append(rft.activation_text(rec))
    c['FT1'] = lines == ['activation a-1 scheduled', 'activation a-1 pending | the object has not moved yet',
                         'activation a-1 applied at 5', 'activation a-1 not-applicable | no cell of the required kind']
    score = rs.EventScore()
    score.activated = True
    score.activation_step = 2
    score.end_state = 'corrected'
    score.end_step = 4
    score.correcting_id = 3
    score.opportunity_step = 2
    score.revealing = True
    score.strict_step = 4
    c['FT2'] = rft.score_text('a-1', score) == ('score a-1 activated yes at 2 active no end corrected 4 by 3 '
                                                'opportunity 2 revealing yes mismatch - strict 4')
    return c


def first_blocks(lines, n):
    out = []
    for line in lines:
        out.append(line)
        if line == 'end':
            n -= 1
            if n == 0:
                break
    return out


def check_gate(fast):
    c = {}
    cases = read(os.path.join(GOLDEN, 'fault_cases.txt'))
    traces = read(os.path.join(GOLDEN, 'fault_traces.txt'))
    if not fast:
        c['G8'] = rfr.gate(cases, traces, rfr.GOLDEN_FAULT_CASES) == ''
    three_cases = first_blocks(cases, 3)
    three_traces = first_blocks(traces, 3)
    c['G9'] = rfr.gate(three_cases, three_traces, rfr.GOLDEN_FAULT_CASES) == 'case file has 3 cases, required 563'
    c['G10'] = rfr.gate(three_cases, three_traces, 3) == ''
    return c


GROUPS = {
    '4a': lambda fast: {**check_random(), **check_layouts(fast)},
    '4b': lambda fast: check_events(fast),
    '4c': lambda fast: {**check_apply(), **check_f3_f4(), **check_f5_cache(), **check_activation(),
                        **check_fault_runs(), **check_run_watchers()},
    '4d': lambda fast: {**check_scorer(), **check_scorer_life()},
    '4e': lambda fast: {**check_shadow(), **check_observed_runs(), **check_read_sites()},
    '4f': lambda fast: check_csv(),
    '4g': lambda fast: {**check_reader(), **check_gate(fast)},
}


def all_checks(fast=False, groups=None):
    c = {}
    for name, part in GROUPS.items():
        if groups is None or name in groups:
            c.update(part(fast))
    return c


def failing(fast=False, groups=None):
    return [k for k, v in all_checks(fast, groups).items() if not v]


if __name__ == '__main__':
    fast = '--fast' in sys.argv
    totals = {}
    results = {}
    for name, part in GROUPS.items():
        r = part(fast)
        totals[name] = len(r)
        results.update(r)
    for k, v in results.items():
        print(k, 'PASS' if v else 'FAIL')
    print('per sub-stage:', ', '.join('%s %d' % kv for kv in totals.items()))
    bad = [k for k, v in results.items() if not v]
    print('failing:', bad if bad else 'none', f'({len(results)} checks)')
