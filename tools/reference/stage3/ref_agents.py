"""Reference for the Stage 3 rebuild, part 3: the log text, C1 (the competent single
agent), C0 (the naive agent) and the run loop (plan STAGE_03, Sections 3.6-3.12).
Follows the original src/agent/single_agent*.cpp, naive_agent.cpp, run_loop.cpp and
agent_log.cpp (89ea9b6) function by function."""
import ref_verify as rc
import ref_parts as rp

rw = rp.rw


# ---------------- log text (plan 3.12) ----------------
def pos(p):
    return '(%d, %d)' % (p[0], p[1])


def blocked_text(b):
    return 'blocked' if b else 'free'


def plan_search_line(k):
    return 'Plan: search found a plan of %d actions' % k


def plan_fallback_line(k):
    return 'Plan: fallback toward the best state found (%d actions)' % k


def plan_active_test_line(cid, k):
    return 'Plan: active test of correction %d (%d actions)' % (cid, k)


PLAN_CAUTIOUS = 'Plan: cautious mode (investigation unfinished)'


def step_line(step, action, predicted, observed, matched):
    return 'Step %d: %s | predicted %s, observed %s | %s' % (
        step, action, predicted, observed, 'match' if matched else 'MISMATCH')


def safe_stop_line(step, reason):
    return 'Step %d: safe stop (%s)' % (step, reason)


def verified_line(cid):
    return '  - correction %d: VERIFIED' % cid


def rejected_line(cid):
    return '  - correction %d: REJECTED' % cid


def investigation_wall_line(cell, believed, recorded):
    return '  - investigation: wall at %s believed %s, recorded %s (source S2)' % (
        pos(cell), blocked_text(believed), blocked_text(recorded))


def investigation_object_line(believed, recorded):
    return '  - investigation: object believed at %s, recorded at %s (source S2)' % (pos(believed), pos(recorded))


def investigation_model_line(action):
    return '  - investigation: belief matched the record; action model for %s suspected (source S1)' % action


ANOMALY_AGENT = '  - anomaly: believed agent position differs from the record'
DEFERRED_LINE = '  - investigation: budget too small for verification; retried next step'
UNEXPLAINED = '  - investigation: no candidate passed V1 (source S5); unexplained update'
FULL_STATE = '  - full-state dead end (dead-square classification consistent; no claim)'
ANOMALY_INVALID = '  - anomaly: invalid hypothetical state skipped'


def correction_wall_line(cid, cell, blocked):
    return '  - correction %d: wall at %s set to %s: CONSISTENT' % (cid, pos(cell), blocked_text(blocked))


def correction_object_line(cid, p):
    return '  - correction %d: object position set to %s: CONSISTENT' % (cid, pos(p))


def correction_model_line(cid, action):
    return '  - correction %d: action model for %s restored to the standard rule: CONSISTENT' % (cid, action)


def surprise_line(cell):
    return '  - consequence surprise: search exhausted with the object at %s' % pos(cell)


def consequence_mismatch_line(cell):
    return '  - consequence mismatch: %s violates the dead-square rule (source S1)' % pos(cell)


def correction_trap_line(cid, cell):
    return '  - correction %d: %s classified as a dead square: CONSISTENT' % (cid, pos(cell))


def result_lines(outcome, world_steps, safe_stops, mismatches, corrections, verified, evaluations):
    return ['Result: %s after %d world steps, %d safe stops' % (outcome, world_steps, safe_stops),
            'Mismatches: %d' % mismatches,
            'Corrections: %d (%d verified)' % (corrections, verified),
            'Model evaluations used: %d' % evaluations]


# ---------------- C1 ----------------
class Settings:
    def __init__(self, budget=200, safe_stops=5, steps=100, delay=3):
        self.budget = budget
        self.safe_stop_limit = safe_stops
        self.max_world_steps = steps
        self.active_test_delay = delay


class Decision:
    def __init__(self, is_action, action='North', reason=''):
        self.is_action = is_action
        self.action = action
        self.reason = reason


class Committed:
    def __init__(self):
        self.has = False
        self.prediction = None      # (action, outcome, state)
        self.belief_before = None
        self.step = 0
        self.tested = []


def object_dead(state, m):
    return rw.is_inside(state.object) and rp.read_dead_status(m, state.object)


class C1:
    def __init__(self, first, settings, belief=None, base_model=None, not_dead=()):
        self.settings = settings
        self.belief = (belief if belief is not None else first.state).copy()
        self.history = [first]
        self.dead_cache = rp.DeadSquareCache()
        self.counts = rp.Counts()
        self.not_dead = list(not_dead)
        self.dead_overrides = []
        self.model = rp.AgentActionModel(base_model)
        self.predictor = rp.StepPredictor(lambda s, a: self.model.current()(s, a), settings.budget)
        self.plan = []
        self.plan_kind = 'none'
        self.committed = Committed()
        self.corrections = []
        self.mismatches = []
        self.pending_v1 = None
        self.deferred = None            # (belief_before, action, history copy)
        self.waiting_trap = None        # (correction id, cell)
        self.safe_stops = 0
        self.step = 0
        self.next_id = 1
        self.trap_anomalies = 0
        self.log = []
        self.activation_hook = None

    # -- small helpers --
    def reserve(self):
        r = 1
        for c in self.corrections:
            if c.status == 'CONSISTENT' and c.element.kind != 'trap':
                r += 1
        return r

    def current_map(self):
        return self.dead_cache.get(self.belief, self.not_dead, self.dead_overrides, self.counts)

    def find(self, cid):
        for c in self.corrections:
            if c.id == cid:
                return c
        return None

    def add_record(self, record):
        record.id = self.next_id
        self.next_id += 1
        self.corrections.append(record)
        return record

    def safe_stop(self, reason):
        self.safe_stops += 1
        self.committed.has = False
        self.log.append(safe_stop_line(self.step, reason))
        return Decision(False, 'North', reason)

    def apply_correction(self, c):
        e = c.element
        if e.kind == 'wall' and rw.is_inside(e.cell):
            self.belief.walls[e.cell[1]][e.cell[0]] = c.new_blocked
        elif e.kind == 'object':
            self.belief.object = c.new_position
        elif e.kind == 'model':
            self.model.restore(e.action)
            self.predictor.clear_cache()
        elif e.kind == 'trap':
            if e.cell not in self.dead_overrides:
                self.dead_overrides.append(e.cell)
            self.predictor.clear_cache()

    def revert_correction(self, c):
        e = c.element
        if e.kind == 'wall' and rw.is_inside(e.cell):
            self.belief.walls[e.cell[1]][e.cell[0]] = c.old_blocked
        elif e.kind == 'model':
            self.model.unrestore(e.action)
            self.predictor.clear_cache()
        elif e.kind == 'trap':
            self.dead_overrides = [cell for cell in self.dead_overrides if cell != e.cell]
            self.predictor.clear_cache()

    # -- the decision step (plan 3.6) --
    def decide(self, observation):
        self.step += 1
        self.predictor.start_step()
        if self.step != 1:
            self.history.append(observation)
        if self.committed.has:
            self.check_committed(observation)
        stopped = self.retry_deferred()
        if stopped is not None:
            if self.activation_hook is not None:
                self.activation_hook(self.step)
            return stopped
        self.continue_pending_work()
        if self.activation_hook is not None:
            self.activation_hook(self.step)
        return self.choose_and_commit()

    def final_check(self, observation):
        self.predictor.start_step()
        self.history.append(observation)
        if self.committed.has:
            self.check_committed(observation)

    # -- step 2: the check --
    def check_committed(self, observation):
        committed = self.committed
        self.committed = Committed()
        self.committed.has = False
        before = committed.belief_before
        action = committed.prediction[0]
        observed = rc.observed_outcome(observation)
        cells = rc.footprint_of(before, action, observation)
        comparison = rp.compare_footprint(committed.prediction[2], committed.prediction[1],
                                          observation.state, observed, cells)
        matched = not comparison[0] and not comparison[1]
        self.log.append(step_line(committed.step, action, committed.prediction[1], observed, matched))
        self.settle_tested(committed, matched)
        if matched:
            rp.apply_footprint_update(self.belief, cells, observation.state)
            return
        result = rc.investigate_mismatch(before, action, self.history, self.predictor,
                                         self.model.current(), self.reserve())
        self.mismatches.append(rc.Mismatch(self.step, action, comparison,
                                           [c.element for c in result.candidates],
                                           result.suspected, result.anomaly))
        self.handle_investigation(result, before, action, observation, True, list(self.history))

    def settle_tested(self, committed, matched):
        for cid in committed.tested:
            record = self.find(cid)
            if record is None or record.status != 'CONSISTENT':
                continue
            if matched:
                record.status = 'VERIFIED'
                self.log.append(verified_line(cid))
            else:
                record.status = 'REJECTED'
                self.log.append(rejected_line(cid))
                self.revert_correction(record)

    def handle_investigation(self, result, before, action, after, log_lines, history_used):
        if log_lines:
            self.log_investigation(result, before, action)
        cells = rc.footprint_of(before, action, after)
        if result.outcome == 'BeliefCandidatesConsistent':
            self.create_belief_corrections(result, before)
            rp.apply_footprint_update(self.belief, cells, after.state)
        elif result.outcome == 'NoCandidatePassed':
            self.log.append(UNEXPLAINED)
            rp.apply_footprint_update(self.belief, cells, after.state)
        elif result.outcome == 'ModelSuspected':
            rp.apply_footprint_update(self.belief, cells, after.state)
            self.pending_v1 = rc.start_model_v1(action, history_used)
        else:
            if log_lines:
                self.log.append(DEFERRED_LINE)
            self.deferred = (before.copy(), action, list(history_used))

    def log_investigation(self, result, before, action):
        if not result.candidates and result.outcome == 'ModelSuspected':
            self.log.append(investigation_model_line(action))
        for c in result.candidates:
            if c.element.kind == 'wall':
                self.log.append(investigation_wall_line(c.element.cell, rw.is_wall(before, c.element.cell),
                                                        c.blocked))
            else:
                self.log.append(investigation_object_line(before.object, c.position))
        if result.anomaly:
            self.log.append(ANOMALY_AGENT)

    def create_belief_corrections(self, result, before):
        for cand in result.candidates:
            record = rc.Correction()
            record.element = rc.Element(cand.element.kind, cand.element.cell, cand.element.action)
            record.new_blocked = cand.blocked
            record.old_blocked = rw.is_wall(before, cand.element.cell)
            record.new_position = cand.position
            record.old_position = before.object
            record.status = 'CONSISTENT'
            record.step_consistent = self.step
            added = self.add_record(record)
            self.apply_correction(added)
            if added.element.kind == 'wall':
                self.log.append(correction_wall_line(added.id, added.element.cell, added.new_blocked))
            else:
                self.log.append(correction_object_line(added.id, added.new_position))

    # -- step 3: deferred and pending work --
    def retry_deferred(self):
        if self.deferred is None:
            return None
        before, action, history = self.deferred
        result = rc.investigate_mismatch(before, action, history, self.predictor,
                                         self.model.current(), self.reserve())
        if result.outcome == 'Deferred':
            return self.safe_stop('budget too small for verification')
        self.deferred = None
        self.handle_investigation(result, before, action, history[-1], False, history)
        return None

    def attempt_trap_v2(self, cid, cell):
        record = self.find(cid)
        if record is None or record.status != 'CONSISTENT':
            self.waiting_trap = None
            return
        anomalies_before = self.trap_anomalies
        result, self.trap_anomalies = rc.trap_v2_test(self.belief, self.current_map(), cell, self.predictor,
                                                      self.model.current(), self.reserve(),
                                                      self.trap_anomalies)
        for _ in range(anomalies_before, self.trap_anomalies):
            self.log.append(ANOMALY_INVALID)
        if result == 'Verified':
            record.status = 'VERIFIED'
            self.log.append(verified_line(cid))
            self.waiting_trap = None
        elif result == 'Rejected':
            record.status = 'REJECTED'
            self.log.append(rejected_line(cid))
            self.revert_correction(record)
            self.waiting_trap = None
        else:
            self.waiting_trap = (cid, cell)

    def continue_pending_work(self):
        if self.waiting_trap is not None:
            self.attempt_trap_v2(self.waiting_trap[0], self.waiting_trap[1])
        if self.pending_v1 is None:
            return
        action = self.pending_v1.action
        progress = rc.continue_model_v1(self.pending_v1, self.history, self.predictor, self.reserve())
        if progress == 'Done':
            self.pending_v1 = None
            record = rc.Correction()
            record.element = rc.Element('model', (0, 0), action)
            record.status = 'CONSISTENT'
            record.step_consistent = self.step
            added = self.add_record(record)
            self.apply_correction(added)
            self.log.append(correction_model_line(added.id, action))
        elif progress == 'Failed':
            self.pending_v1 = None
            self.log.append(UNEXPLAINED)

    # -- steps 5-7: choose and commit --
    def choose_cautious(self):
        m = self.current_map()
        reserve = self.reserve()
        for action in rp.ORDER:
            need = 1 if self.predictor.is_cached(self.belief, action) else 2
            if self.predictor.remaining() - need < reserve:
                break
            current = self.predictor.predict(self.belief, action)
            if current is None:
                break
            standard = self.predictor.predict_variant(self.belief, action, rp.standard_rule)
            if standard is None:
                break
            if not rp.predictions_differ(current, standard) and not object_dead(current[2], m):
                return (action, current[2])
        return None

    def try_active_test(self, m, reserve):
        for record in self.corrections:
            if (record.status != 'CONSISTENT' or record.element.kind != 'wall'
                    or self.step - record.step_consistent < self.settings.active_test_delay):
                continue
            if self.try_test_for(record, m, reserve):
                return True
        return False

    def try_test_for(self, record, m, reserve):
        targets = rc.active_test_targets(self.belief, record.element.cell)
        if not targets:
            return False

        def goal(state):
            for target in targets:
                if state.agent == target[0]:
                    return True
            return False

        def estimate(state):
            if object_dead(state, m):
                return rp.INF
            best = rp.INF
            for target in targets:
                best = min(best, rp.manhattan(state.agent, target[0]))
            return best
        result = rp.search(self.belief, m, goal, estimate, self.predictor, reserve)
        if result.kind != 'Found':
            return False
        return self.finish_active_test(record, targets, result, reserve)

    def finish_active_test(self, record, targets, result, reserve):
        end = self.belief if not result.plan else result.plan[-1][1]
        for target in targets:
            if target[0] != end.agent:
                continue
            if not self.predictor.is_cached(end, target[1]) and self.predictor.remaining() <= reserve:
                continue
            test = self.predictor.predict(end, target[1])
            if test is None:
                continue
            self.plan = list(result.plan)
            self.plan.append((target[1], test[2]))
            self.plan_kind = 'test'
            self.log.append(plan_active_test_line(record.id, len(self.plan)))
            return True
        return False

    def commit_action(self):
        action = self.plan[0][0]
        predicted = self.predictor.predict(self.belief, action)
        if predicted is None:
            self.plan = []
            self.plan_kind = 'none'
            return self.safe_stop('no budget for a committed prediction')
        self.plan.pop(0)
        if not self.plan:
            self.plan_kind = 'none'
        tested = []
        for record in self.corrections:
            if record.status != 'CONSISTENT' or record.tested or self.predictor.remaining() <= 0:
                continue
            alternative = rc.alternative_prediction(record, self.belief, action, self.predictor,
                                                    self.model.current(), self.model.base())
            if alternative is not None and rp.predictions_differ(alternative, predicted):
                record.tested = True
                tested.append(record.id)
        self.committed = Committed()
        self.committed.has = True
        self.committed.prediction = predicted
        self.committed.belief_before = self.belief.copy()
        self.committed.step = self.step
        self.committed.tested = tested
        return Decision(True, action)

    def choose_and_commit(self):
        if self.pending_v1 is not None:
            cautious = self.choose_cautious()
            if cautious is not None:
                self.log.append(PLAN_CAUTIOUS)
                self.plan = [cautious]
                self.plan_kind = 'goal'
                return self.commit_action()
            return self.safe_stop('investigation unfinished, no cautious action')
        reserve = self.reserve()
        m = self.current_map()
        if self.plan_kind != 'test' and self.try_active_test(m, reserve):
            return self.commit_action()
        kept = self.try_keep_plan(m)
        if kept is not None:
            return kept
        return self.run_main_search(m, reserve)

    def try_keep_plan(self, m):
        if not self.plan:
            return None
        predicted = self.predictor.predict(self.belief, self.plan[0][0])
        if predicted is None:
            self.plan = []
            self.plan_kind = 'none'
            return self.safe_stop('no budget for a committed prediction')
        if predicted[2].same(self.plan[0][1]) and not object_dead(predicted[2], m):
            return self.commit_action()
        self.plan = []
        self.plan_kind = 'none'
        return None

    def run_main_search(self, m, reserve):
        def goal(state):
            return state.object == state.destination

        def estimate(state):
            return rp.heuristic(state, m, self.counts)
        result = rp.search(self.belief, m, goal, estimate, self.predictor, reserve)
        if result.kind == 'Found':
            if not result.plan:
                return self.safe_stop('belief shows goal reached')
            self.plan = list(result.plan)
            self.plan_kind = 'goal'
            self.log.append(plan_search_line(len(self.plan)))
        else:
            if result.kind == 'Exhausted':
                self.handle_exhausted(m)
            stopped = self.choose_fallback(result)
            if stopped is not None:
                return stopped
        predicted = self.predictor.predict(self.belief, self.plan[0][0])
        if predicted is None:
            self.plan = []
            self.plan_kind = 'none'
            return self.safe_stop('no budget for a committed prediction')
        return self.commit_action()

    def handle_exhausted(self, m):
        p = self.belief.object
        consequence = rc.check_consequence(self.belief, m, self.history, self.counts)
        if consequence == 'FullStateDeadEnd':
            self.log.append(surprise_line(p))
            self.log.append(FULL_STATE)
        elif consequence == 'Mismatch':
            self.log.append(surprise_line(p))
            self.log.append(consequence_mismatch_line(p))
            record = rc.Correction()
            record.element = rc.Element('trap', p)
            record.status = 'CONSISTENT'
            record.step_consistent = self.step
            added = self.add_record(record)
            self.apply_correction(added)
            self.log.append(correction_trap_line(added.id, p))
            self.attempt_trap_v2(added.id, p)

    def choose_fallback(self, result):
        best = rp.fallback(result)
        if not best:
            return self.safe_stop('budget too small to plan' if result.kind == 'OutOfBudget' else 'no safe action')
        self.plan = best
        self.plan_kind = 'fallback'
        self.log.append(plan_fallback_line(len(self.plan)))
        return None

    # -- counts --
    def verified_count(self):
        return len([c for c in self.corrections if c.status == 'VERIFIED'])


# ---------------- C0 (plan 3.10) ----------------
class C0:
    def __init__(self, first, settings, belief=None, base_model=None):
        self.settings = settings
        self.belief = (belief if belief is not None else first.state).copy()
        self.dead_cache = rp.DeadSquareCache()
        self.counts = rp.Counts()
        self.not_dead = []
        self.dead_overrides = []
        self.model = rp.AgentActionModel(base_model)
        self.predictor = rp.StepPredictor(lambda s, a: self.model.current()(s, a), settings.budget)
        self.plan = []
        self.step = 0
        self.searches = 0
        self.activation_hook = None

    def first_moving_or_north(self):
        for action in rp.ORDER:
            predicted = self.predictor.predict(self.belief, action)
            if predicted is not None and predicted[1] in ('Moved', 'Pushed'):
                return (action, predicted[2])
        return ('North', self.belief.copy())

    def plan_from_belief(self):
        m = self.dead_cache.get(self.belief, self.not_dead, self.dead_overrides, self.counts)
        result = rp.search(self.belief, m, lambda s: s.object == s.destination,
                           lambda s: rp.heuristic(s, m, self.counts), self.predictor, 0)
        self.searches += 1
        if result.kind == 'Found' and result.plan:
            self.plan = list(result.plan)
            return
        self.plan = rp.fallback(result)
        if not self.plan:
            self.plan = [self.first_moving_or_north()]

    def decide(self, observation):
        self.step += 1
        self.predictor.start_step()
        if self.activation_hook is not None:
            self.activation_hook(self.step)
        if not self.plan:
            self.plan_from_belief()
        action, expected = self.plan.pop(0)
        self.belief = expected.copy()
        return Decision(True, action)


# ---------------- the run loop (plan 3.11), with a watch-only step watcher ----------------
def run_c1(world, agent, settings, on_step=None, on_final=None, seeded_action=None):
    """Returns (outcome, world steps). on_step(observation, decision) and on_final(observation)
    are called after each decision and after the final check; they only watch."""
    env = rw.Environment(world)
    if world.object == world.destination:
        return 'Success', 0
    current = rc.Observation(1, world)
    world_steps = 0
    if seeded_action is not None:
        state, outcome = env.step(seeded_action)
        world_steps += 1
        current = rc.Observation(2, env.state, seeded_action, outcome)
    while True:
        decision = agent.decide(current)
        if on_step is not None:
            on_step(current, decision)
        if not decision.is_action:
            if agent.safe_stops >= settings.safe_stop_limit:
                return 'Stopped -- unresolved', world_steps
            current = rc.Observation(agent.step + 1, current.state)
            continue
        state, outcome = env.step(decision.action)
        world_steps += 1
        nxt = rc.Observation(agent.step + 1, env.state, decision.action, outcome)
        if env.state.object == env.state.destination:
            agent.final_check(nxt)
            if on_final is not None:
                on_final(nxt)
            return 'Success', world_steps
        if world_steps >= settings.max_world_steps:
            agent.final_check(nxt)
            if on_final is not None:
                on_final(nxt)
            return 'Step limit', world_steps
        current = nxt


def run_c0(world, agent, settings, on_step=None):
    env = rw.Environment(world)
    if world.object == world.destination:
        return 'Success', 0
    current = rc.Observation(1, world)
    world_steps = 0
    while True:
        decision = agent.decide(current)
        if on_step is not None:
            on_step(current, decision)
        state, outcome = env.step(decision.action)
        world_steps += 1
        nxt = rc.Observation(agent.step + 1, env.state, decision.action, outcome)
        if env.state.object == env.state.destination:
            return 'Success', world_steps
        if world_steps >= settings.max_world_steps:
            return 'Step limit', world_steps
        current = nxt


def c1_run_lines(world, agent, settings):
    """The RunResult lines of the original runSingleAgent for C1."""
    outcome, steps = run_c1(world, agent, settings)
    return outcome, steps, agent.log + result_lines(outcome, steps, agent.safe_stops, len(agent.mismatches),
                                                    len(agent.corrections), agent.verified_count(),
                                                    agent.predictor.used_total)


def c0_run_lines(world, agent, settings):
    outcome, steps = run_c0(world, agent, settings)
    return outcome, steps, result_lines(outcome, steps, 0, 0, 0, 0, agent.predictor.used_total)
