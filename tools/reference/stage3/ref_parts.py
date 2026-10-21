"""Reference for the Stage 3 rebuild, part 1: footprint, dead squares, heuristic,
step predictor, agent action model and planner (plan STAGE_03, Sections 3.2-3.5).

Reference contract (R10): it follows the original C++ data flow (IWatchSheep/ai-hologram,
src/agent at 89ea9b6 = stage-3 at 7587f38 plus the IE-1 hooks H1-H4), function by function,
in the same order, so that every evaluation, every dead-status read and every heuristic
value happens at the same moment as in the original. It is checked against golden traces
produced by a tool built from that original C++ (orig_trace/)."""
import heapq
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'stage2'))
import ref_model as rm  # noqa: E402

rw = rm.rw
GRID = rw.GRID
ORDER = ['North', 'East', 'South', 'West']
INF = 1000000000


def manhattan(a, b):
    return abs(a[0] - b[0]) + abs(a[1] - b[1])


def offset(action):
    return rw.OFFSETS[action]


# ---------------- footprint (plan 3.2) ----------------
def compute_footprint(belief_before, action, observed_after, observed_outcome):
    c0 = belief_before.agent
    c1 = rw.moved(c0, action)
    c2 = rw.moved(c1, action)
    cells = [c0, c1]
    if belief_before.object == c1 or observed_outcome == 'Pushed' or observed_after.object == c1:
        cells.append(c2)
    return cells


def compare_footprint(predicted_state, predicted_outcome, observed_state, observed_outcome, cells):
    """Returns (outcomes_differ, [cell differences]); each difference is a dict with the
    cell and the predicted and observed wall, agent-here and object-here values."""
    diffs = []
    for cell in cells:
        d = {'cell': cell,
             'pw': rw.is_wall(predicted_state, cell), 'ow': rw.is_wall(observed_state, cell),
             'pa': predicted_state.agent == cell, 'oa': observed_state.agent == cell,
             'po': predicted_state.object == cell, 'oo': observed_state.object == cell}
        if d['pw'] != d['ow'] or d['pa'] != d['oa'] or d['po'] != d['oo']:
            diffs.append(d)
    return predicted_outcome != observed_outcome, diffs


def footprint_for_two(first_belief, first_outcome, second_belief, second_outcome, action):
    c0 = first_belief.agent
    c1 = rw.moved(c0, action)
    c2 = rw.moved(c1, action)
    cells = [c0, c1]
    if (first_belief.object == c1 or second_belief.object == c1
            or first_outcome == 'Pushed' or second_outcome == 'Pushed'):
        cells.append(c2)
    return cells


def apply_footprint_update(belief, cells, observed):
    """Changes belief in place, exactly as the original (walls, then agent, then object,
    cell by cell)."""
    for cell in cells:
        if not rw.is_inside(cell):
            continue
        x, y = cell
        belief.walls[y][x] = observed.walls[y][x]
        if observed.agent == cell:
            belief.agent = observed.agent
        if observed.object == cell or belief.object == cell:
            belief.object = observed.object


# ---------------- dead squares (plan 3.3, IE-1 H4) ----------------
class Counts:
    def __init__(self):
        self.analysis_runs = 0
        self.cell_operations = 0
        self.local_checks = 0
        self.heuristic_calls = 0


class DeadMap:
    def __init__(self):
        self.dead = [[False] * GRID for _ in range(GRID)]
        self.push = [[0] * GRID for _ in range(GRID)]
        self.observer = None

    def copy_from(self, other):
        self.dead = [row[:] for row in other.dead]
        self.push = [row[:] for row in other.push]


def _apply_overrides(m, belief, cells, value):
    for cell in cells:
        if not rw.is_inside(cell) or rw.is_wall(belief, cell) or cell == belief.destination:
            continue
        m.dead[cell[1]][cell[0]] = value


def compute_dead_squares(belief, not_dead, dead_corrections, counts):
    m = DeadMap()
    m.push = [[-1] * GRID for _ in range(GRID)]
    counts.analysis_runs += 1
    queue = []
    if rw.is_inside(belief.destination):
        m.push[belief.destination[1]][belief.destination[0]] = 0
        queue.append(belief.destination)
    front = 0
    while front < len(queue):
        r = queue[front]
        front += 1
        rd = m.push[r[1]][r[0]]
        for direction in ORDER:
            counts.cell_operations += 1
            ox, oy = offset(direction)
            q = (r[0] - ox, r[1] - oy)
            s = (r[0] - 2 * ox, r[1] - 2 * oy)
            if not rw.is_inside(q) or rw.is_wall(belief, q) or rw.is_wall(belief, s):
                continue
            if m.push[q[1]][q[0]] < 0:
                m.push[q[1]][q[0]] = rd + 1
                queue.append(q)
    for y in range(GRID):
        for x in range(GRID):
            if not rw.is_wall(belief, (x, y)) and belief.destination != (x, y):
                m.dead[y][x] = m.push[y][x] < 0
    _apply_overrides(m, belief, not_dead, False)
    _apply_overrides(m, belief, dead_corrections, True)
    return m


def read_dead_status(m, cell):
    value = m.dead[cell[1]][cell[0]]
    if m.observer is not None and m.observer.on_dead_read is not None:
        m.observer.on_dead_read(cell, value)
    return value


def local_check_says_not_dead(belief, m, cell, counts):
    counts.local_checks += 1
    if cell == belief.destination:
        return True
    for direction in ORDER:
        ox, oy = offset(direction)
        behind = (cell[0] - ox, cell[1] - oy)
        ahead = (cell[0] + ox, cell[1] + oy)
        if (not rw.is_wall(belief, behind) and not rw.is_wall(belief, ahead)
                and not read_dead_status(m, ahead)):
            return True
    return False


class DeadSquareCache:
    """One map object, recomputed IN PLACE when the inputs change (as the original's
    DeadSquareCache::map, whose references stay valid across recomputations)."""

    def __init__(self):
        self.map = DeadMap()
        self.observer = None
        self._has = False
        self._walls = None
        self._dest = None
        self._not_dead = None
        self._dead = None

    def get(self, belief, not_dead, dead_corrections, counts):
        changed = (not self._has or belief.walls != self._walls or belief.destination != self._dest
                   or list(not_dead) != self._not_dead or list(dead_corrections) != self._dead)
        if changed:
            self.map.copy_from(compute_dead_squares(belief, not_dead, dead_corrections, counts))
            self._walls = [row[:] for row in belief.walls]
            self._dest = belief.destination
            self._not_dead = list(not_dead)
            self._dead = list(dead_corrections)
            self._has = True
        self.map.observer = self.observer
        return self.map


def heuristic(state, m, counts):
    counts.heuristic_calls += 1
    value = INF
    if not read_dead_status(m, state.object):
        pd = m.push[state.object[1]][state.object[0]]
        pushes = pd if pd >= 0 else manhattan(state.object, state.destination)
        walking = max(0, manhattan(state.agent, state.object) - 1)
        value = pushes + walking
    if m.observer is not None and m.observer.on_heuristic is not None:
        m.observer.on_heuristic(state, value)
    return value


# ---------------- agent action model (plan 3.8, IE-1 H2) ----------------
def standard_rule(state, action):
    return rw.apply_action(state, action)


class AgentActionModel:
    def __init__(self, base=None):
        self.base_model = base if base is not None else standard_rule
        self.restored = [False] * 4

    def restore(self, action):
        self.restored[ORDER.index(action)] = True

    def unrestore(self, action):
        self.restored[ORDER.index(action)] = False

    def is_restored(self, action):
        return self.restored[ORDER.index(action)]

    def set_base(self, model):
        self.base_model = model if model is not None else standard_rule

    def current(self):
        base = self.base_model
        marks = list(self.restored)

        def model(state, action):
            if marks[ORDER.index(action)]:
                return standard_rule(state, action)
            return base(state, action)
        return model

    def base(self):
        return self.base_model


# ---------------- step predictor (plan 3.4, IE-1 H3) ----------------
class StepPredictor:
    """A prediction is a tuple (action, outcome, state); states are never shared."""

    def __init__(self, model_fn, budget_per_step):
        self.model_fn = model_fn          # called at every evaluation (reads the current model)
        self.budget_per_step = budget_per_step
        self.budget = rm.BudgetCounter(budget_per_step)
        self.cache = []
        self.has_context = False
        self.ctx_walls = None
        self.ctx_dest = None
        self.used_total = 0
        self.observer = None

    def start_step(self):
        self.budget = rm.BudgetCounter(self.budget_per_step)
        self.clear_cache()

    def clear_cache(self):
        self.cache = []
        self.has_context = False

    def _find(self, state, action):
        for agent, obj, act, prediction in self.cache:
            if agent == state.agent and obj == state.object and act == action:
                return prediction
        return None

    def is_cached(self, state, action):
        if not self.has_context or state.walls != self.ctx_walls or state.destination != self.ctx_dest:
            return False
        return self._find(state, action) is not None

    def _evaluate(self, state, action, model):
        if not self.budget.try_use():
            return None
        new_state, outcome = model(state, action)
        return (action, outcome, new_state.copy())

    def predict(self, state, action):
        if self.has_context and (state.walls != self.ctx_walls or state.destination != self.ctx_dest):
            self.clear_cache()
        hit = self._find(state, action)
        if hit is not None:
            return hit
        result = self._evaluate(state, action, self.model_fn)
        if result is None:
            return None
        self.used_total += 1
        if not self.has_context:
            self.ctx_walls = [row[:] for row in state.walls]
            self.ctx_dest = state.destination
            self.has_context = True
        self.cache.append((state.agent, state.object, action, result))
        if self.observer is not None and self.observer.on_evaluation is not None:
            self.observer.on_evaluation(state, action, False, result)
        return result

    def predict_variant(self, state, action, model):
        result = self._evaluate(state, action, model if model is not None else standard_rule)
        if result is None:
            return None
        self.used_total += 1
        if self.observer is not None and self.observer.on_evaluation is not None:
            self.observer.on_evaluation(state, action, True, result)
        return result

    def remaining(self):
        return self.budget.remaining()

    def used_this_step(self):
        return self.budget.used


def predictions_differ(first, second):
    return (first[1] != second[1] or first[2].agent != second[2].agent
            or first[2].object != second[2].object)


# ---------------- planner (plan 3.5) ----------------
class Discovered:
    def __init__(self, state, best_g, h, parent, action, number):
        self.state = state
        self.best_g = best_g
        self.h = h
        self.parent = parent
        self.action = action
        self.number = number


class SearchResult:
    def __init__(self):
        self.kind = 'Exhausted'
        self.plan = []            # list of (action, expected state)
        self.discovered = []
        self.improved = 0
        self.reopened = 0


def _find_discovered(result, state):
    for i, known in enumerate(result.discovered):
        if known.state.agent == state.agent and known.state.object == state.object:
            return i
    return -1


def _build_plan(result, index):
    steps = []
    at = index
    while result.discovered[at].parent != -1:
        d = result.discovered[at]
        steps.append((d.action, d.state))
        at = d.parent
    steps.reverse()
    return steps


def search(root, m, goal_test, heuristic_fn, predictor, reserve):
    result = SearchResult()
    queue = []
    closed = []
    counter = [0]

    def add_entry(d, index):
        heapq.heappush(queue, (d.best_g + d.h, d.h, counter[0], index, d.best_g))
        counter[0] += 1

    result.discovered.append(Discovered(root, 0, heuristic_fn(root), -1, 'North', 0))
    closed.append(False)
    add_entry(result.discovered[0], 0)
    while queue:
        _, _, _, index, g = heapq.heappop(queue)
        if g != result.discovered[index].best_g or closed[index]:
            continue
        if goal_test(result.discovered[index].state):
            result.kind = 'Found'
            result.plan = _build_plan(result, index)
            return result
        closed[index] = True
        for action in ORDER:
            state = result.discovered[index].state
            new_g = result.discovered[index].best_g + 1
            if not predictor.is_cached(state, action) and predictor.remaining() <= reserve:
                result.kind = 'OutOfBudget'
                return result
            prediction = predictor.predict(state, action)
            if prediction is None:
                result.kind = 'OutOfBudget'
                return result
            if prediction[1] in ('Blocked', 'InvalidState') or read_dead_status(m, prediction[2].object):
                continue
            known = _find_discovered(result, prediction[2])
            if known >= 0 and new_g >= result.discovered[known].best_g:
                continue
            if known >= 0:
                d = result.discovered[known]
                result.improved += 1
                if closed[known]:
                    closed[known] = False
                    result.reopened += 1
                d.best_g = new_g
                d.parent = index
                d.action = action
                add_entry(d, known)
            else:
                d = Discovered(prediction[2], new_g, heuristic_fn(prediction[2]), index, action,
                               len(result.discovered))
                result.discovered.append(d)
                closed.append(False)
                add_entry(d, len(result.discovered) - 1)
    result.kind = 'Exhausted'
    return result


def fallback(result):
    best = -1
    for i, c in enumerate(result.discovered):
        if c.parent == -1 or c.h >= INF:
            continue
        if best < 0:
            best = i
            continue
        b = result.discovered[best]
        if (c.h, c.best_g, c.number) < (b.h, b.best_g, b.number):
            best = i
    if best < 0:
        return []
    return _build_plan(result, best)
