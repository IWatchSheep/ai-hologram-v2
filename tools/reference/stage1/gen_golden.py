"""Generates the Stage 1 golden-value inputs and, from this reference, the expected outputs.

  python3 gen_golden.py cases   <dir>   writes rules_cases.txt and parse_cases.txt (inputs only)
  python3 gen_golden.py golden  <dir>   writes world_rules_golden.txt and world_parse_golden.txt
                                        computed by ref_world.py
  python3 gen_golden.py merge   <dir>   merges parse_cases.txt with the original tool's parse
                                        output (orig_parse_out.txt) into orig_world_parse_golden.txt

The golden files committed to the repository are the ones produced by the ORIGINAL C++
(orig_world_trace.cpp, built from IWatchSheep/ai-hologram 7587f38); this reference must
produce byte-identical files (plan STAGE_01, Section 6.6)."""
import os
import random
import sys

import ref_world as rw

LAYOUTS = {
    'L1': rw.DEMO_LAYOUT,
    'L2': ['##########', '#A...#...#', '#.##.#.#.#', '#.#..D.#.#', '#.#.####.#',
           '#...#....#', '###.#.##.#', '#.....#O.#', '#.##....##', '##########'],
    'L3': ['##########', '#D.......#', '#........#', '#..#..#..#', '#........#',
           '#........#', '#..#..#..#', '#........#', '#.......D#', '##########'],
}
# L3 has two D: it is used only as a broken layout in the parse cases.
RULE_LAYOUTS = ['L1', 'L2']
L1_PLUS = ['##########', '#........#', '#........#', '#..O....+#', '#........#',
           '#........#', '#...##...#', '#........#', '#........#', '##########']


def layout_world(rows):
    """Walls and destination of a layout; agent and object are set per case."""
    w, error = rw.parse_world(rows)
    assert w is not None, error
    return w


def rule_cases():
    positions = [(x, y) for y in range(rw.GRID) for x in range(rw.GRID)]
    positions += [(-1, 3), (10, 3), (3, -1), (3, 10), (-1, -1), (10, 10), (11, 0), (0, -2)]
    lines = []
    for lid in RULE_LAYOUTS:
        for (ax, ay) in positions:
            objects = [(ax, ay - 1), (ax + 1, ay), (ax, ay + 1), (ax - 1, ay), (ax, ay), (5, 5)]
            for (ox, oy) in objects:
                for action in rw.ACTIONS:
                    lines.append(f'{lid} {ax} {ay} {ox} {oy} {action}')
    return lines


def write_rule_cases(path):
    with open(path, 'w') as f:
        for lid in RULE_LAYOUTS:
            f.write(f'layout {lid}\n')
            for row in (LAYOUTS[lid] if lid != 'L1' else LAYOUTS['L1']):
                f.write(row + '\n')
        for line in rule_cases():
            f.write(line + '\n')


def rules_golden_text():
    out = []
    worlds = {}
    for lid in RULE_LAYOUTS:
        worlds[lid] = layout_world(LAYOUTS[lid])
        out.append(f'layout {lid}')
        out.extend(LAYOUTS[lid])
    for line in rule_cases():
        lid, ax, ay, ox, oy, action = line.split()
        w = worlds[lid].copy()
        w.agent = (int(ax), int(ay))
        w.object = (int(ox), int(oy))
        new, outcome = rw.apply_action(w, action)
        goal = 1 if rw.is_goal(new) else 0
        out.append(f'{line} -> {outcome} {new.agent[0]} {new.agent[1]} '
                   f'{new.object[0]} {new.object[1]} {goal}')
    return ''.join(x + '\n' for x in out)


ALPHABET = '#.AOD*+x?'


def parse_inputs():
    """Deterministic list of inputs (lists of lines) for the parser."""
    rng = random.Random(20261020)
    bases = [LAYOUTS['L1'], LAYOUTS['L2'], LAYOUTS['L3'], L1_PLUS]
    cases = [[], [''] * 10, ['#' * 1000] * 10, LAYOUTS['L1'][:9], LAYOUTS['L1'] + ['#' * 10]]
    cases += [list(b) for b in bases]
    for _ in range(1000):
        rows = [list(r) for r in rng.choice(bases)]
        kind = rng.random()
        for _ in range(rng.choice([1, 1, 1, 2, 3])):
            y = rng.randrange(10)
            x = rng.randrange(10)
            rows[y][x] = rng.choice(ALPHABET)
        rows = [''.join(r) for r in rows]
        if kind < 0.05:
            rows.pop(rng.randrange(len(rows)))
        elif kind < 0.10:
            rows.insert(rng.randrange(len(rows)), '#' * 10)
        elif kind < 0.15:
            i = rng.randrange(10)
            rows[i] = rows[i][:-1] if rng.random() < 0.5 else rows[i] + '.'
        cases.append(rows)
    return cases


def write_parse_cases(path):
    with open(path, 'w') as f:
        for n, rows in enumerate(parse_inputs(), 1):
            f.write(f'case {n} lines {len(rows)}\n')
            for r in rows:
                f.write(r + '\n')


def parse_result_line(n, rows):
    w, error = rw.parse_world(rows)
    if w is None:
        return f'case {n}: ERROR {error}'
    extra = '' if rw.render_world(w) == rows else ' ROUNDTRIP-DIFFERS'
    return (f'case {n}: OK {w.agent[0]} {w.agent[1]} {w.object[0]} {w.object[1]} '
            f'{w.destination[0]} {w.destination[1]}{extra}')


def merged_parse_text(results):
    out = []
    for (n, rows), result in zip(enumerate(parse_inputs(), 1), results):
        out.append(f'case {n} lines {len(rows)}')
        out.extend(rows)
        out.append('expect ' + result.split(': ', 1)[1])
    return ''.join(x + '\n' for x in out)


def main():
    mode, folder = sys.argv[1], sys.argv[2]
    if mode == 'cases':
        write_rule_cases(os.path.join(folder, 'rules_cases.txt'))
        write_parse_cases(os.path.join(folder, 'parse_cases.txt'))
    elif mode == 'golden':
        with open(os.path.join(folder, 'world_rules_golden.txt'), 'w') as f:
            f.write(rules_golden_text())
        results = [parse_result_line(n, rows) for n, rows in enumerate(parse_inputs(), 1)]
        with open(os.path.join(folder, 'world_parse_golden.txt'), 'w') as f:
            f.write(merged_parse_text(results))
    elif mode == 'merge':
        with open(os.path.join(folder, 'orig_parse_out.txt')) as f:
            results = f.read().splitlines()
        with open(os.path.join(folder, 'orig_world_parse_golden.txt'), 'w') as f:
            f.write(merged_parse_text(results))


if __name__ == '__main__':
    main()
