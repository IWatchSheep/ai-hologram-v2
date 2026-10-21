"""Reference for the Stage 1 rebuild: the grid world (plan STAGE_01, Section 3).

Reference contract (R10): it follows the C++ data flow of the original world code
(IWatchSheep/ai-hologram, src/world at 7587f38, unchanged from stage-1 to 89ea9b6):
the same checks in the same order, the same error messages, the same outcomes.
It is checked against a tool built from that original C++ (orig_world_trace.cpp)."""

GRID = 10
ACTIONS = ['North', 'East', 'South', 'West']
OFFSETS = {'North': (0, -1), 'East': (1, 0), 'South': (0, 1), 'West': (-1, 0)}

MSG_LINES = 'A world must contain exactly 10 lines.'
MSG_LENGTH = 'Every world line must contain exactly 10 characters.'
MSG_BORDER = 'Every border cell must be a wall (#).'
MSG_UNKNOWN = 'The world contains an unknown character.'
MSG_AGENTS = 'A world must contain exactly one agent (A or +).'
MSG_OBJECTS = 'A world must contain exactly one object (O or *).'
MSG_DESTS = 'A world must contain exactly one destination (D, * or +).'
MSG_INVALID = 'The parsed world state is not valid.'


class World:
    """walls[y][x]; agent, object, destination are (x, y) pairs (default (0, 0))."""

    def __init__(self):
        self.walls = [[False] * GRID for _ in range(GRID)]
        self.agent = (0, 0)
        self.object = (0, 0)
        self.destination = (0, 0)

    def copy(self):
        w = World()
        w.walls = [row[:] for row in self.walls]
        w.agent, w.object, w.destination = self.agent, self.object, self.destination
        return w

    def same(self, other):
        return (self.walls == other.walls and self.agent == other.agent
                and self.object == other.object and self.destination == other.destination)


def moved(p, action):
    dx, dy = OFFSETS[action]
    return (p[0] + dx, p[1] + dy)


def is_inside(p):
    return 0 <= p[0] < GRID and 0 <= p[1] < GRID


def is_wall(w, p):
    if not is_inside(p):
        return True
    return w.walls[p[1]][p[0]]


def is_valid(w):
    if not (is_inside(w.agent) and is_inside(w.object) and is_inside(w.destination)):
        return False
    for i in range(GRID):
        if not (w.walls[0][i] and w.walls[GRID - 1][i] and w.walls[i][0] and w.walls[i][GRID - 1]):
            return False
    return (not is_wall(w, w.agent) and not is_wall(w, w.object)
            and not is_wall(w, w.destination) and w.agent != w.object)


def is_goal(w):
    return w.object == w.destination


def apply_action(w, action):
    """Returns (new world, outcome); never changes w."""
    if not is_valid(w):
        return w, 'InvalidState'
    target = moved(w.agent, action)
    if is_wall(w, target):
        return w, 'Blocked'
    nxt = w.copy()
    nxt.agent = target
    if target != w.object:
        return nxt, 'Moved'
    beyond = moved(w.object, action)
    if is_wall(w, beyond):
        return w, 'Blocked'
    nxt.object = beyond
    return nxt, 'Pushed'


def parse_world(lines):
    """Returns (world, '') or (None, message), checking in the original order."""
    if len(lines) != GRID:
        return None, MSG_LINES
    if any(len(line) != GRID for line in lines):
        return None, MSG_LENGTH
    w = World()
    counts = {'agents': 0, 'objects': 0, 'dests': 0}
    for y in range(GRID):
        for x in range(GRID):
            error = parse_cell(lines[y][x], x, y, w, counts)
            if error:
                return None, error
    if counts['agents'] != 1:
        return None, MSG_AGENTS
    if counts['objects'] != 1:
        return None, MSG_OBJECTS
    if counts['dests'] != 1:
        return None, MSG_DESTS
    if not is_valid(w):
        return None, MSG_INVALID
    return w, ''


def parse_cell(c, x, y, w, counts):
    border = x == 0 or x == GRID - 1 or y == 0 or y == GRID - 1
    if border and c != '#':
        return MSG_BORDER
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
        return MSG_UNKNOWN
    return ''


def render_world(w):
    rows = []
    for y in range(GRID):
        row = ''
        for x in range(GRID):
            cell = (x, y)
            if w.walls[y][x]:
                ch = '#'
            elif w.agent == cell and w.destination == cell:
                ch = '+'
            elif w.object == cell and w.destination == cell:
                ch = '*'
            elif w.agent == cell:
                ch = 'A'
            elif w.object == cell:
                ch = 'O'
            elif w.destination == cell:
                ch = 'D'
            else:
                ch = '.'
            row += ch
        rows.append(row)
    return rows


class Environment:
    def __init__(self, world):
        self.state = world.copy()
        self.step_count = 0

    def step(self, action):
        new, outcome = apply_action(self.state, action)
        self.state = new.copy()
        self.step_count += 1
        return new, outcome


DEMO_LAYOUT = ['##########', '#........#', '#........#', '#..A.O..D#', '#........#',
               '#........#', '#...##...#', '#........#', '#........#', '##########']
DEMO_ACTIONS = ['East'] * 5


def scripted_demo():
    """The text runScriptedDemo writes (it starts with an empty line)."""
    world, error = parse_world(DEMO_LAYOUT)
    if world is None:
        return 'Demo layout is invalid: ' + error + '\n'
    env = Environment(world)
    out = '\nInitial world:\n' + ''.join(r + '\n' for r in render_world(env.state)) + '\n'
    for n, action in enumerate(DEMO_ACTIONS, 1):
        _, outcome = env.step(action)
        out += f'Step {n}: {action} -> {outcome}\n'
    out += '\nFinal world:\n' + ''.join(r + '\n' for r in render_world(env.state))
    out += 'Goal reached: ' + ('yes' if is_goal(env.state) else 'no') + '\n'
    out += f'Steps taken: {env.step_count}\n'
    return out
