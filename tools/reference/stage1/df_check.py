"""Claude's deliberate failure checks for Stage 1 (plan STAGE_01, Section 7; rule R11),
tried on the reference first. Each mutation is undone before the next."""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import checks  # noqa: E402
import ref_world as rw  # noqa: E402

print('baseline failing:', checks.failing())

# DF1b-1 (sub-stage 1b): a push into a wall still moves the agent.
original_apply = rw.apply_action


def apply_blocked_push_moves_agent(w, action):
    if not rw.is_valid(w):
        return w, 'InvalidState'
    target = rw.moved(w.agent, action)
    if rw.is_wall(w, target):
        return w, 'Blocked'
    nxt = w.copy()
    nxt.agent = target
    if target != w.object:
        return nxt, 'Moved'
    beyond = rw.moved(w.object, action)
    if rw.is_wall(w, beyond):
        return nxt, 'Blocked'
    nxt.object = beyond
    return nxt, 'Pushed'


rw.apply_action = apply_blocked_push_moves_agent
print('DF1b-1 (blocked push moves the agent) failing:', checks.failing())
rw.apply_action = original_apply

# DF1a-1 (sub-stage 1a): the border test forgets the bottom row (y == 9).
original_cell = rw.parse_cell


def parse_cell_no_bottom_border(c, x, y, w, counts):
    """ref_world.parse_cell with 'or y == GRID - 1' removed from the border test."""
    border = x == 0 or x == rw.GRID - 1 or y == 0
    if border and c != '#':
        return rw.MSG_BORDER
    if c == '#':
        w.walls[y][x] = True
    elif c == '.':
        pass
    elif c == 'A':
        counts['agents'] += 1
        w.agent = (x, y)
    elif c == 'O':
        counts['objects'] += 1
        w.object = (x, y)
    elif c == 'D':
        counts['dests'] += 1
        w.destination = (x, y)
    elif c == '*':
        counts['objects'] += 1
        counts['dests'] += 1
        w.object = (x, y)
        w.destination = (x, y)
    elif c == '+':
        counts['agents'] += 1
        counts['dests'] += 1
        w.agent = (x, y)
        w.destination = (x, y)
    else:
        return rw.MSG_UNKNOWN
    return ''


rw.parse_cell = parse_cell_no_bottom_border
print('DF1a-1 (no border check on the bottom row) failing:', checks.failing())
rw.parse_cell = original_cell

# DF1a-2 (sub-stage 1a): isWall says "not a wall" for positions outside the grid.
original_is_wall = rw.is_wall


def is_wall_outside_open(w, p):
    if not rw.is_inside(p):
        return False
    return w.walls[p[1]][p[0]]


rw.is_wall = is_wall_outside_open
print('DF1a-2 (outside the grid is not a wall) failing:', checks.failing())
rw.is_wall = original_is_wall

# DF1c-1 (sub-stage 1c): the demo uses four East actions instead of five.
rw.DEMO_ACTIONS = ['East'] * 4
print('DF1c-1 (demo with four actions) failing:', checks.failing())
rw.DEMO_ACTIONS = ['East'] * 5

# DF1c-2 (owner, sub-stage 1c): the Pushed case reports the outcome Moved.
def apply_pushed_reported_as_moved(w, action):
    new, outcome = original_apply(w, action)
    return new, ('Moved' if outcome == 'Pushed' else outcome)


rw.apply_action = apply_pushed_reported_as_moved
print('DF1c-2 (Pushed reported as Moved) failing:', checks.failing())
rw.apply_action = original_apply
print('baseline failing again:', checks.failing())
