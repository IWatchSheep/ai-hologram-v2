"""Reference for the Stage 2 rebuild: belief, prediction, budget, world model, comparison
and difference text (plan STAGE_02, Section 3).

Reference contract (R10): it follows the original C++ (IWatchSheep/ai-hologram, src/model,
src/compare and src/demo/prediction_demo at 7587f38, unchanged from stage-2 to 89ea9b6).
The world itself comes from tools/reference/stage1/ref_world.py."""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'stage1'))
import ref_world as rw  # noqa: E402


class BudgetCounter:
    def __init__(self, limit):
        self.limit = 0 if limit < 0 else limit
        self.used = 0

    def try_use(self):
        if self.used >= self.limit:
            return False
        self.used += 1
        return True

    def remaining(self):
        return self.limit - self.used

    def reset(self):
        self.used = 0


class WorldModel:
    """action_model(world, action) -> (world, outcome); None means the default rules."""

    def __init__(self, action_model=None):
        self.action_model = action_model if action_model is not None else rw.apply_action

    def predict(self, belief, action, budget):
        """belief is a World. Returns (action, outcome, state) or None when the budget refuses."""
        if not budget.try_use():
            return None
        state, outcome = self.action_model(belief, action)
        return action, outcome, state


def compare_states(predicted, observed):
    """Returns (agent, object, destination, walls): each position item is
    (differs, predicted, observed); walls is a list of (cell, predicted_blocked,
    observed_blocked) in row-then-column order."""
    def pos(p, o):
        return (p != o, p, o)
    walls = []
    for y in range(rw.GRID):
        for x in range(rw.GRID):
            pb = predicted.walls[y][x]
            ob = observed.walls[y][x]
            if pb != ob:
                walls.append(((x, y), pb, ob))
    return (pos(predicted.agent, observed.agent), pos(predicted.object, observed.object),
            pos(predicted.destination, observed.destination), walls)


def has_differences(diff):
    return diff[0][0] or diff[1][0] or diff[2][0] or len(diff[3]) > 0


def compare_prediction(prediction, observed_state, observed_outcome):
    """Returns (predicted outcome, observed outcome, state difference)."""
    return prediction[1], observed_outcome, compare_states(prediction[2], observed_state)


def matches(comparison):
    return comparison[0] == comparison[1] and not has_differences(comparison[2])


def position_text(p):
    return f'({p[0]}, {p[1]})'


def describe_differences(diff):
    lines = []
    for name, item in zip(('agent', 'object', 'destination'), diff[:3]):
        if item[0]:
            lines.append(f'  - {name}: predicted {position_text(item[1])}, '
                         f'observed {position_text(item[2])}')
    for cell, pb, ob in diff[3]:
        lines.append(f'  - wall at {position_text(cell)}: predicted '
                     f'{"blocked" if pb else "free"}, observed {"blocked" if ob else "free"}')
    return lines


def step_text(n, action, comparison):
    verdict = 'match' if matches(comparison) else 'MISMATCH'
    return (f'Step {n}: {action} | predicted {comparison[0]}, observed {comparison[1]} | '
            f'{verdict}')


def prediction_demo_text():
    """The text runPredictionDemos writes."""
    out = ''
    for title, wrong_wall in (('Prediction demo 1: correct belief', False),
                              ('Prediction demo 2: belief wrongly includes a wall at (6, 3)',
                               True)):
        truth, _ = rw.parse_world(rw.DEMO_LAYOUT)
        env = rw.Environment(truth)
        belief = truth.copy()
        if wrong_wall:
            belief.walls[3][6] = True
        model = WorldModel()
        budget = BudgetCounter(10)
        out += '\n' + title + '\n'
        mismatches = 0
        for n, action in enumerate(['East'] * 5, 1):
            prediction = model.predict(belief, action, budget)
            if prediction is None:
                out += 'No prediction available; stopping this demo.\n'
                break
            observed_state, observed_outcome = env.step(action)
            comparison = compare_prediction(prediction, observed_state, observed_outcome)
            out += step_text(n, action, comparison) + '\n'
            out += ''.join(x + '\n' for x in describe_differences(comparison[2]))
            if not matches(comparison):
                mismatches += 1
            belief = observed_state.copy()
        out += f'Mismatches: {mismatches}\nModel evaluations used: {budget.used}\n'
    out += 'Note: in these demos the belief is replaced by the observation after every step.\n'
    out += 'How beliefs are corrected is decided in Stage 3.\n'
    return out
