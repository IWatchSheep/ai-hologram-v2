"""Reference for the Stage 3 rebuild, part 2: records, investigation and verification
(plan STAGE_03, Sections 3.7 and 3.8). Follows the original src/agent/investigation.cpp
and verification.cpp (89ea9b6) function by function."""
import ref_parts as rp

rw = rp.rw


class Observation:
    def __init__(self, step, state, last_action=None, last_outcome=None):
        self.step = step
        self.state = state.copy()
        self.last_action = last_action
        self.last_outcome = last_outcome


class Element:
    def __init__(self, kind='wall', cell=(0, 0), action='North'):
        self.kind = kind          # 'wall', 'object', 'model', 'trap'
        self.cell = cell
        self.action = action


class Correction:
    def __init__(self):
        self.id = 0
        self.element = Element()
        self.new_blocked = False
        self.old_blocked = False
        self.new_position = (0, 0)
        self.old_position = (0, 0)
        self.status = 'CONSISTENT'
        self.step_consistent = 0
        self.tested = False


class Mismatch:
    def __init__(self, step, action, differences, candidates, suspected, anomaly):
        self.step = step
        self.action = action
        self.differences = differences    # (outcomes_differ, [cell differences])
        self.candidates = candidates      # [Element]
        self.suspected = suspected
        self.anomaly = anomaly


class Candidate:
    def __init__(self, element, blocked=False, position=(0, 0)):
        self.element = element
        self.blocked = blocked
        self.position = position


class InvestigationResult:
    def __init__(self):
        self.outcome = 'ModelSuspected'   # BeliefCandidatesConsistent, NoCandidatePassed, Deferred
        self.candidates = []
        self.suspected = 'S5'
        self.anomaly = False


def observed_outcome(observation):
    return observation.last_outcome if observation.last_outcome is not None else 'Blocked'


def footprint_of(belief_before, action, after):
    return rp.compute_footprint(belief_before, action, after.state, observed_outcome(after))


def _find_candidates(belief_before, action, before, after):
    cells = rp.compute_footprint(belief_before, action, after.state, observed_outcome(after))
    candidates = []
    for cell in cells:
        if rw.is_wall(belief_before, cell) == rw.is_wall(before.state, cell):
            continue
        candidates.append(Candidate(Element('wall', cell), rw.is_wall(before.state, cell)))
    in_footprint = False
    for cell in cells:
        if belief_before.object == cell or before.state.object == cell:
            in_footprint = True
    if in_footprint and belief_before.object != before.state.object:
        candidates.append(Candidate(Element('object'), position=before.state.object))
    return candidates


def corrected_belief(belief, candidates):
    b = belief.copy()
    for c in candidates:
        if c.element.kind == 'wall':
            if rw.is_inside(c.element.cell):
                b.walls[c.element.cell[1]][c.element.cell[0]] = c.blocked
        elif c.element.kind == 'object':
            b.object = c.position
    return b


def wall_history_check(cell, blocked, history):
    for entry in history:
        if rw.is_wall(entry.state, cell) != blocked:
            return False
    return True


def object_history_check(value, before):
    return before.state.object == value


def _history_checks_pass(candidates, history, before):
    for c in candidates:
        if c.element.kind == 'wall' and not wall_history_check(c.element.cell, c.blocked, history):
            return False
        if c.element.kind == 'object' and not object_history_check(c.position, before):
            return False
    return True


def trap_history_passes(p, destination, history):
    for i in range(len(history)):
        if history[i].state.object != p:
            continue
        for j in range(i + 1, len(history)):
            if history[j].state.object == destination:
                return False
    return True


def _verify_candidates(belief_before, action, candidates, history, before, after, predictor,
                       current_model, reserve):
    if (not _history_checks_pass(candidates, history, before)
            or not rw.is_valid(corrected_belief(belief_before, candidates))):
        return 'NoCandidatePassed'
    if predictor.remaining() <= reserve:
        return 'Deferred'
    corrected = corrected_belief(belief_before, candidates)
    predicted = predictor.predict_variant(corrected, action, current_model)
    if predicted is None:
        return 'Deferred'
    outcome = observed_outcome(after)
    cells = rp.compute_footprint(corrected, action, after.state, outcome)
    differ, diffs = rp.compare_footprint(predicted[2], predicted[1], after.state, outcome, cells)
    if not differ and not diffs:
        return 'BeliefCandidatesConsistent'
    return 'NoCandidatePassed'


def investigate_mismatch(belief_before, action, history, predictor, current_model, reserve):
    result = InvestigationResult()
    if len(history) < 2:
        return result
    before = history[-2]
    after = history[-1]
    result.anomaly = belief_before.agent != before.state.agent
    result.candidates = _find_candidates(belief_before, action, before, after)
    if not result.candidates:
        result.outcome = 'ModelSuspected'
        result.suspected = 'S1'
        return result
    result.suspected = 'S2'
    result.outcome = _verify_candidates(belief_before, action, result.candidates, history, before,
                                        after, predictor, current_model, reserve)
    if result.outcome == 'NoCandidatePassed':
        result.suspected = 'S5'
    return result


def check_consequence(belief, m, history, counts):
    p = belief.object
    if not rw.is_inside(p):
        return 'NotApplicable'
    dead = rp.read_dead_status(m, p)
    if dead or p == belief.destination:
        return 'NotApplicable'
    if rp.local_check_says_not_dead(belief, m, p, counts):
        return 'FullStateDeadEnd'
    if not trap_history_passes(p, belief.destination, history):
        return 'NotApplicable'
    return 'Mismatch'


# ---------------- verification (plan 3.8) ----------------
class ModelV1Work:
    def __init__(self, action, indices):
        self.action = action
        self.pending = list(indices)


def start_model_v1(action, history):
    return ModelV1Work(action, [k for k in range(len(history)) if history[k].last_action == action])


def continue_model_v1(work, history, predictor, reserve):
    while work.pending:
        if predictor.remaining() <= reserve:
            return 'Pending'
        k = work.pending[0]
        if k < 1 or k >= len(history):
            return 'Failed'
        recorded = history[k]
        replay = predictor.predict_variant(history[k - 1].state, work.action, rp.standard_rule)
        if replay is None:
            return 'Pending'
        if (not replay[2].same(recorded.state) or recorded.last_outcome is None
                or replay[1] != recorded.last_outcome):
            return 'Failed'
        work.pending.pop(0)
    return 'Done'


def trap_v1_check(belief, m, p, history, counts):
    if rp.local_check_says_not_dead(belief, m, p, counts):
        return False
    return trap_history_passes(p, belief.destination, history)


def trap_v2_test(belief, m, p, predictor, current_model, reserve, anomalies):
    """Returns (result, anomalies) with result Verified, Rejected or Waiting."""
    pushes = []
    for direction in rp.ORDER:
        ox, oy = rp.offset(direction)
        stand = (p[0] - ox, p[1] - oy)
        if rw.is_wall(belief, stand):
            continue
        hypothetical = belief.copy()
        hypothetical.agent = stand
        hypothetical.object = p
        if not rw.is_valid(hypothetical):
            anomalies += 1
            continue
        pushes.append((hypothetical, direction))
    if predictor.remaining() - len(pushes) < reserve:
        return 'Waiting', anomalies
    rejected = False
    for state, direction in pushes:
        predicted = predictor.predict_variant(state, direction, current_model)
        if predicted is None:
            return 'Waiting', anomalies
        moved_to = predicted[2].object
        if moved_to != p and rw.is_inside(moved_to) and not rp.read_dead_status(m, moved_to):
            rejected = True
    return ('Rejected' if rejected else 'Verified'), anomalies


def alternative_prediction(correction, belief, action, predictor, current_model, old_model):
    c1 = rw.moved(belief.agent, action)
    c2 = rw.moved(c1, action)
    e = correction.element
    if e.kind == 'model':
        if e.action != action:
            return None
        if not rw.is_valid(belief):
            return None
        return predictor.predict_variant(belief, action, old_model)
    if e.kind == 'trap':
        return None
    alternative = belief.copy()
    if e.kind == 'wall':
        if e.cell != c1 and e.cell != c2:
            return None
        if not rw.is_inside(e.cell):
            return None
        alternative.walls[e.cell[1]][e.cell[0]] = correction.old_blocked
    else:
        if (correction.old_position != c1 and correction.old_position != c2
                and belief.object != c1 and belief.object != c2):
            return None
        alternative.object = correction.old_position
    if not rw.is_valid(alternative):
        return None
    return predictor.predict_variant(alternative, action, current_model)


def active_test_targets(belief, cell):
    targets = []
    for direction in rp.ORDER:
        ox, oy = rp.offset(direction)
        stand = (cell[0] - ox, cell[1] - oy)
        if not rw.is_wall(belief, stand) and stand != belief.object:
            targets.append((stand, direction))
    return targets
