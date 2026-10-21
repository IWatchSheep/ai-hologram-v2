"""Reference for the Stage 3 rebuild, part 4: the trace cases, the hand-written model
catalogue, and the trace and observer text of the equivalence gate (plan STAGE_03,
Section 3.13; Migration Addendum M1.4). The original-code tool orig_trace/orig_c1_trace.cpp
writes the same text from the original C++."""
import ref_agents as ra
import ref_verify as rc
import ref_parts as rp

rw = rp.rw


# ---------------- the model catalogue (plan 3.13) ----------------
def build_model(spec):
    kind = spec[0] if spec else 'standard'
    if kind == 'standard':
        return None
    a = spec[1]
    if kind == 'stuck':
        def stuck(s, act):
            standard = rw.apply_action(s, act)
            if act != a or standard[1] == 'InvalidState':
                return standard
            return s, 'Blocked'
        return stuck
    if kind == 'nopush':
        def nopush(s, act):
            standard = rw.apply_action(s, act)
            if act != a or standard[1] != 'Pushed':
                return standard
            return s, 'Blocked'
        return nopush
    if kind == 'slip':
        def slip(s, act):
            standard = rw.apply_action(s, act)
            if act != a or standard[1] != 'Moved':
                return standard
            beyond = rw.moved(standard[0].agent, act)
            if rw.is_wall(s, beyond) or beyond == s.object:
                return standard
            wrong = standard[0].copy()
            wrong.agent = beyond
            return wrong, 'Moved'
        return slip
    if kind == 'swap':
        b = spec[2]

        def swap(s, act):
            return rw.apply_action(s, b if act == a else act)
        return swap
    d = spec[2]  # pushturn A D

    def pushturn(s, act):
        standard = rw.apply_action(s, act)
        if act != a or standard[1] != 'Pushed':
            return standard
        target = rw.moved(s.object, d)
        if rw.is_wall(s, target):
            return s, 'Blocked'
        wrong = s.copy()
        wrong.agent = s.object
        wrong.object = target
        return wrong, 'Pushed'
    return pushturn


# ---------------- cases ----------------
class Case:
    def __init__(self):
        self.name = ''
        self.agent = 'C1'
        self.settings = ra.Settings()
        self.world = None
        self.belief = None
        self.model = ['standard']
        self.not_dead = []
        self.walls = []          # (cell, new_blocked, old_blocked)
        self.v1 = None           # (action, count)
        self.committed = None    # action
        self.plan = None         # action

    def text(self):
        """The case block of the cases file."""
        out = ['case ' + self.name, 'agent ' + self.agent,
               'settings %d %d %d %d' % (self.settings.budget, self.settings.safe_stop_limit,
                                         self.settings.max_world_steps, self.settings.active_test_delay),
               'world'] + rw.render_world(self.world)
        if self.belief is None:
            out.append('belief true')
        else:
            out += ['belief given'] + rw.render_world(self.belief)
        out.append('model ' + ' '.join(self.model))
        out.append('notdead %d' % len(self.not_dead) + ''.join(' %d %d' % c for c in self.not_dead))
        for cell, new, old in self.walls:
            out.append('seed wall %d %d %s %s' % (cell[0], cell[1], ra.blocked_text(new), ra.blocked_text(old)))
        if self.v1 is not None:
            out.append('seed v1 %s %d' % self.v1)
        if self.committed is not None:
            out.append('seed committed ' + self.committed)
        if self.plan is not None:
            out.append('seed plan ' + self.plan)
        out.append('end')
        return out


def read_cases(lines):
    cases = []
    i = 0
    c = None
    while i < len(lines):
        w = lines[i].split()
        i += 1
        if not w:
            continue
        if w[0] == 'case':
            c = Case()
            c.name = w[1]
        elif w[0] == 'agent':
            c.agent = w[1]
        elif w[0] == 'settings':
            c.settings = ra.Settings(int(w[1]), int(w[2]), int(w[3]), int(w[4]))
        elif w[0] == 'world':
            c.world = rw.parse_world(lines[i:i + 10])[0]
            i += 10
        elif w[0] == 'belief' and w[1] == 'given':
            c.belief = rw.parse_world(lines[i:i + 10])[0]
            i += 10
        elif w[0] == 'model':
            c.model = w[1:]
        elif w[0] == 'notdead':
            c.not_dead = [(int(w[k]), int(w[k + 1])) for k in range(2, len(w) - 1, 2)]
        elif w[0] == 'seed' and w[1] == 'wall':
            c.walls.append(((int(w[2]), int(w[3])), w[4] == 'blocked', w[5] == 'blocked'))
        elif w[0] == 'seed' and w[1] == 'v1':
            c.v1 = (w[2], int(w[3]))
        elif w[0] == 'seed' and w[1] == 'committed':
            c.committed = w[2]
        elif w[0] == 'seed' and w[1] == 'plan':
            c.plan = w[2]
        elif w[0] == 'end':
            cases.append(c)
    return cases


def make_c1(c):
    first = rc.Observation(1, c.world)
    agent = ra.C1(first, c.settings, c.belief, build_model(c.model), c.not_dead)
    next_id = 1
    for cell, new, old in c.walls:
        r = rc.Correction()
        r.id = next_id
        next_id += 1
        r.element = rc.Element('wall', cell)
        r.new_blocked = new
        r.old_blocked = old
        agent.corrections.append(r)
    agent.next_id = next_id
    if c.v1 is not None:
        action, count = c.v1
        s = c.world
        for i in range(1, count + 1):
            s, outcome = rw.apply_action(s, action)
            agent.history.append(rc.Observation(i + 1, s, action, outcome))
        agent.pending_v1 = rc.ModelV1Work(action, range(1, count + 1))
    if c.plan is not None:
        agent.plan = [(c.plan, agent.belief.copy())]
        agent.plan_kind = 'goal'
    if c.committed is not None:
        b = agent.belief.copy()
        state, outcome = rw.apply_action(b, c.committed)
        agent.step = 1
        agent.committed.has = True
        agent.committed.prediction = (c.committed, outcome, state.copy())
        agent.committed.belief_before = b
        agent.committed.step = 1
    return agent


def make_c0(c):
    return ra.C0(rc.Observation(1, c.world), c.settings, c.belief, build_model(c.model))


# ---------------- trace text ----------------
def hex_walls(s):
    digits = '0123456789abcdef'
    out = ''
    for j in range(25):
        v = 0
        for k in range(4):
            i = 4 * j + k
            v = v * 2 + (1 if s.walls[i // 10][i % 10] else 0)
        out += digits[v]
    return out


def state_text(s):
    return 'agent %s object %s destination %s walls %s' % (ra.pos(s.agent), ra.pos(s.object),
                                                           ra.pos(s.destination), hex_walls(s))


def yes_no(b):
    return 'yes' if b else 'no'


def element_text(e):
    if e.kind == 'model':
        return 'model ' + e.action
    if e.kind == 'object':
        return 'object'
    return e.kind + ' ' + ra.pos(e.cell)


def observation_line(o):
    line = 'observation %d %s' % (o.step, state_text(o.state))
    if o.last_action is not None:
        line += ' last %s %s' % (o.last_action, o.last_outcome if o.last_outcome is not None else 'Blocked')
    else:
        line += ' last none'
    return line


def mismatch_lines(m):
    differ, cells = m.differences
    out = ['mismatch step %d action %s outcomes %s suspected %s anomaly %s' % (
        m.step, m.action, 'differ' if differ else 'same', m.suspected, yes_no(m.anomaly))]
    b = lambda v: '1' if v else '0'
    for d in cells:
        out.append('  cell %s wall %s%s agent %s%s object %s%s' % (ra.pos(d['cell']), b(d['pw']), b(d['ow']),
                                                                 b(d['pa']), b(d['oa']), b(d['po']), b(d['oo'])))
    for e in m.candidates:
        out.append('  candidate ' + element_text(e))
    return out


def correction_line(c):
    line = 'correction %d %s' % (c.id, element_text(c.element))
    if c.element.kind == 'wall':
        line += ' new %s old %s' % (ra.blocked_text(c.new_blocked), ra.blocked_text(c.old_blocked))
    elif c.element.kind == 'object':
        line += ' new %s old %s' % (ra.pos(c.new_position), ra.pos(c.old_position))
    return line + ' %s since %d tested %s' % (c.status, c.step_consistent, yes_no(c.tested))


class C1Printer:
    def __init__(self):
        self.log_shown = 0
        self.mismatches_shown = 0
        self.corrections_shown = []   # the last printed line of each record

    def common(self, out, a):
        out.append('belief ' + state_text(a.belief))
        out.append('plan %s %d' % (a.plan_kind, len(a.plan)))
        line = 'work v1 '
        if a.pending_v1 is not None:
            line += a.pending_v1.action + ''.join(' %d' % k for k in a.pending_v1.pending)
        else:
            line += 'none'
        line += ' | deferred %s | trap %s' % (yes_no(a.deferred is not None),
                                              str(a.waiting_trap[0]) if a.waiting_trap is not None else 'none')
        out.append(line)
        out.append('evaluations %d total %d safe stops %d reserve %d' % (
            a.predictor.used_this_step(), a.predictor.used_total, a.safe_stops, a.reserve()))
        while self.log_shown < len(a.log):
            out.append('log ' + a.log[self.log_shown])
            self.log_shown += 1
        while self.mismatches_shown < len(a.mismatches):
            out += mismatch_lines(a.mismatches[self.mismatches_shown])
            self.mismatches_shown += 1
        for i, c in enumerate(a.corrections):
            line = correction_line(c)
            if i >= len(self.corrections_shown):
                self.corrections_shown.append('')
            if line != self.corrections_shown[i]:
                out.append(line)
                self.corrections_shown[i] = line

    def step(self, out, a, given, d):
        out.append('step %d' % a.step)
        out.append(observation_line(given))
        out.append('decision ' + d.action if d.is_action else 'decision stop ' + d.reason)
        if a.committed.has:
            p = a.committed.prediction
            out.append('committed %s %s %s tests%s' % (p[0], p[1], state_text(p[2]),
                                                       ''.join(' %d' % i for i in a.committed.tested)
                                                       if a.committed.tested else ' none'))
        else:
            out.append('committed none')
        self.common(out, a)

    def final(self, out, a, given):
        out.append('final')
        out.append(observation_line(given))
        self.common(out, a)


def result_line(outcome, world_steps, decision_steps, safe_stops, mismatches, corrections, verified, evaluations):
    return ('result %s | world steps %d | decision steps %d | safe stops %d | mismatches %d | corrections %d'
            ' | verified %d | evaluations %d' % (outcome, world_steps, decision_steps, safe_stops, mismatches,
                                                 corrections, verified, evaluations))


class Tally:
    def __init__(self):
        self.reset()

    def reset(self):
        self.own = 0
        self.variant = 0
        self.dead_reads = 0
        self.dead_true = 0
        self.heuristics = 0
        self.infinite = 0
        self.sum = 0

    def line(self):
        return ('observer evaluations %d variant %d dead reads %d true %d heuristics %d infinite %d sum %d' % (
            self.own, self.variant, self.dead_reads, self.dead_true, self.heuristics, self.infinite, self.sum))


class Observer:
    """The IE-1 watch-only observer (H3, H4): it only counts."""

    def __init__(self, tally):
        def on_evaluation(state, action, variant, prediction):
            if variant:
                tally.variant += 1
            else:
                tally.own += 1

        def on_dead_read(cell, value):
            tally.dead_reads += 1
            if value:
                tally.dead_true += 1

        def on_heuristic(state, value):
            tally.heuristics += 1
            if value >= rp.INF:
                tally.infinite += 1
            else:
                tally.sum += value
        self.on_evaluation = on_evaluation
        self.on_dead_read = on_dead_read
        self.on_heuristic = on_heuristic


def trace_c1(c, observers=False):
    a = make_c1(c)
    out = ['case ' + c.name]
    printer = C1Printer()
    tally = Tally()
    if observers:
        obs = Observer(tally)
        a.predictor.observer = obs
        a.dead_cache.observer = obs

    def on_step(given, d):
        if observers:
            out.append('step %d' % a.step)
            out.append(tally.line())
            tally.reset()
        else:
            printer.step(out, a, given, d)

    def on_final(given):
        if observers:
            out.append('final')
            out.append(tally.line())
            tally.reset()
        else:
            printer.final(out, a, given)
    outcome, steps = ra.run_c1(c.world, a, c.settings, on_step, on_final, c.committed)
    out.append(result_line(outcome, steps, a.step, a.safe_stops, len(a.mismatches), len(a.corrections),
                           a.verified_count(), a.predictor.used_total))
    out.append('end')
    return out


def trace_c0(c, observers=False):
    a = make_c0(c)
    out = ['case ' + c.name]
    tally = Tally()
    if observers:
        obs = Observer(tally)
        a.predictor.observer = obs
        a.dead_cache.observer = obs

    def on_step(given, d):
        out.append('step %d' % a.step)
        if observers:
            out.append(tally.line())
            tally.reset()
            return
        out.append(observation_line(given))
        out.append('decision ' + d.action)
        out.append('belief ' + state_text(a.belief))
        out.append('plan %d searches %d' % (len(a.plan), a.searches))
        out.append('evaluations %d total %d' % (a.predictor.used_this_step(), a.predictor.used_total))
    outcome, steps = ra.run_c0(c.world, a, c.settings, on_step)
    out.append(result_line(outcome, steps, a.step, 0, 0, 0, 0, a.predictor.used_total))
    out.append('end')
    return out


def trace(c, observers=False):
    return trace_c0(c, observers) if c.agent == 'C0' else trace_c1(c, observers)
