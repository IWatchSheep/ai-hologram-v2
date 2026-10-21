"""Claude's deliberate failure checks for Stage 2 (plan STAGE_02, Section 7; rule R11),
tried on the reference first. Each mutation is undone before the next."""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import checks  # noqa: E402
import ref_model as rm  # noqa: E402

rw = rm.rw
print('baseline failing:', checks.failing())

# DF2a-1 (Cursor): tryUse compares with > instead of >=, allowing one use too many.
original_try_use = rm.BudgetCounter.try_use


def try_use_one_too_many(self):
    if self.used > self.limit:
        return False
    self.used += 1
    return True


rm.BudgetCounter.try_use = try_use_one_too_many
print('DF2a-1 (budget allows limit + 1) failing:', checks.failing())
rm.BudgetCounter.try_use = original_try_use

# DF2a-2 (Cursor): the world model ignores a supplied action model.
original_init = rm.WorldModel.__init__


def init_ignoring_model(self, action_model=None):
    self.action_model = rw.apply_action


rm.WorldModel.__init__ = init_ignoring_model
print('DF2a-2 (custom action model ignored) failing:', checks.failing())
rm.WorldModel.__init__ = original_init

# DF2b-1 (Cursor): walls missing from a prediction are ignored
# (the original Stage 2 check: "pb != ob" becomes "pb and not ob").
original_compare = rm.compare_states


def compare_ignoring_missing_walls(predicted, observed):
    a, o, d, _ = original_compare(predicted, observed)
    walls = []
    for y in range(rw.GRID):
        for x in range(rw.GRID):
            pb = predicted.walls[y][x]
            ob = observed.walls[y][x]
            if pb and not ob:
                walls.append(((x, y), pb, ob))
    return a, o, d, walls


rm.compare_states = compare_ignoring_missing_walls
print('DF2b-1 (missing walls ignored) failing:', checks.failing())

# DF2b-2 (Cursor): walls are reported column by column instead of row by row.


def compare_column_first(predicted, observed):
    a, o, d, _ = original_compare(predicted, observed)
    walls = []
    for x in range(rw.GRID):
        for y in range(rw.GRID):
            pb = predicted.walls[y][x]
            ob = observed.walls[y][x]
            if pb != ob:
                walls.append(((x, y), pb, ob))
    return a, o, d, walls


rm.compare_states = compare_column_first
print('DF2b-2 (walls column-first) failing:', checks.failing())
rm.compare_states = original_compare

# DF2c-1 (Cursor): the prediction demo's budget limit is 4 instead of 10.
original_demo = rm.prediction_demo_text


def demo_with_budget(limit):
    real = rm.BudgetCounter

    class Small(real):
        def __init__(self, _limit):
            real.__init__(self, limit)
    rm.BudgetCounter = Small
    try:
        return original_demo()
    finally:
        rm.BudgetCounter = real


rm.prediction_demo_text = lambda: demo_with_budget(4)
print('DF2c-1 (demo budget 4) failing:', checks.failing())
rm.prediction_demo_text = original_demo

# DF2c-2 (owner): the difference text says "seen" instead of "observed".
original_describe = rm.describe_differences
rm.describe_differences = lambda diff: [x.replace(', observed ', ', seen ')
                                        for x in original_describe(diff)]
print('DF2c-2 (difference text says seen) failing:', checks.failing())
rm.describe_differences = original_describe
print('baseline failing again:', checks.failing())
