"""Generates the Stage 2 golden-value input and, from this reference, the expected file.

  python3 gen_golden.py cases  <dir>   writes prediction_cases.txt (inputs only)
  python3 gen_golden.py golden <dir>   writes prediction_golden.txt computed by ref_model.py

The golden file committed to the repository is the one produced by the ORIGINAL C++
(orig_trace/orig_prediction_trace.cpp, built from IWatchSheep/ai-hologram 7587f38);
this reference must produce a byte-identical file (plan STAGE_02, Section 3.6)."""
import os
import random
import sys

import ref_model as rm

rw = rm.rw
SEED = 20261021
COUNT = 1000


def random_truth(rng):
    """A valid random world: border walls, about 20% interior walls."""
    while True:
        w = rw.World()
        for y in range(rw.GRID):
            for x in range(rw.GRID):
                border = x in (0, rw.GRID - 1) or y in (0, rw.GRID - 1)
                w.walls[y][x] = border or rng.random() < 0.2
        free = [(x, y) for y in range(rw.GRID) for x in range(rw.GRID) if not w.walls[y][x]]
        if len(free) < 4:
            continue
        w.agent, w.object = rng.sample(free, 2)
        if rng.random() < 0.5:
            # Put the object next to the agent half the time, so pushes are common.
            dx, dy = rng.choice([(0, -1), (1, 0), (0, 1), (-1, 0)])
            beside = (w.agent[0] + dx, w.agent[1] + dy)
            if beside in free:
                w.object = beside
        w.destination = rng.choice([w.object] + free) if rng.random() < 0.1 else rng.choice(free)
        if rw.is_valid(w):
            return w


def perturb(rng, truth):
    """A valid belief: the truth, sometimes with wrong walls or wrong positions."""
    while True:
        b = truth.copy()
        for _ in range(rng.choice([0, 0, 1, 1, 2, 3])):
            kind = rng.random()
            free = [(x, y) for y in range(1, rw.GRID - 1) for x in range(1, rw.GRID - 1)
                    if not b.walls[y][x]]
            if kind < 0.4:
                x, y = rng.randrange(1, rw.GRID - 1), rng.randrange(1, rw.GRID - 1)
                b.walls[y][x] = not b.walls[y][x]
            elif kind < 0.6 and free:
                b.agent = rng.choice(free)
            elif kind < 0.85 and free:
                b.object = rng.choice(free)
            elif free:
                b.destination = rng.choice(free)
        if rw.is_valid(b):
            return b


def cases():
    rng = random.Random(SEED)
    out = []
    for n in range(1, COUNT + 1):
        truth = random_truth(rng)
        belief = perturb(rng, truth)
        action = rng.choice(rw.ACTIONS)
        toward = [a for a in rw.ACTIONS if rw.moved(truth.agent, a) == truth.object]
        if toward and rng.random() < 0.5:
            # Half the time, act toward an adjacent object, so pushes are common.
            action = toward[0]
        out.append((n, action, rw.render_world(belief), rw.render_world(truth)))
    return out


def write_cases(path):
    with open(path, 'w') as f:
        for n, action, b, t in cases():
            f.write(f'case {n} action {action}\n' + ''.join(r + '\n' for r in b + t))


def golden_text():
    lines = []
    for n, action, brows, trows in cases():
        belief, _ = rw.parse_world(brows)
        truth, _ = rw.parse_world(trows)
        prediction = rm.WorldModel().predict(belief, action, rm.BudgetCounter(1))
        observed_state, observed_outcome = rw.apply_action(truth, action)
        comparison = rm.compare_prediction(prediction, observed_state, observed_outcome)
        described = rm.describe_differences(comparison[2])
        verdict = 'match' if rm.matches(comparison) else 'MISMATCH'
        lines.append(f'case {n} action {action} differences {len(described)}')
        lines.extend(brows + trows)
        lines.append(f'result {comparison[0]} {comparison[1]} {verdict}')
        lines.extend(described)
    return ''.join(x + '\n' for x in lines)


def main():
    mode, folder = sys.argv[1], sys.argv[2]
    if mode == 'cases':
        write_cases(os.path.join(folder, 'prediction_cases.txt'))
    elif mode == 'golden':
        with open(os.path.join(folder, 'prediction_golden.txt'), 'w') as f:
            f.write(golden_text())


if __name__ == '__main__':
    main()
