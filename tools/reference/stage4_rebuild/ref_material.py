"""Reference for the Stage 4 rebuild, part 1: the experiment's material (plan STAGE_04, 3.1-3.3).

Seeded random numbers (SplitMix64), the experimenter's search over true states, the layout
generator, and the fault-event lists with their text. Reference contract (R10): it follows the
original C++ (src/experiment at 89ea9b6: seeded_random, true_search, layout_generator,
fault_events), function by function and draw by draw. It works on the rebuild's own reference
types (../stage1 world, ../stage3 dead squares)."""
import os
import sys
from collections import deque

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'stage3'))
import ref_parts as rp  # noqa: E402

rw = rp.rw
MASK = (1 << 64) - 1
ORDER = ['North', 'East', 'South', 'West']
TYPES = ['F1', 'F2', 'F3', 'F4', 'F5']


# ---------------- 3.1 seeded random numbers ----------------
class SeededRandom:
    def __init__(self, seed):
        self.state = seed & MASK

    def next(self):
        self.state = (self.state + 0x9E3779B97F4A7C15) & MASK
        z = self.state
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & MASK
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & MASK
        return z ^ (z >> 31)

    def below(self, n):
        threshold = ((1 << 64) - n) % n
        while True:
            r = self.next()
            if r >= threshold:
                return r % n

    def shuffle(self, values):
        for i in range(len(values) - 1, 0, -1):
            j = self.below(i + 1)
            values[i], values[j] = values[j], values[i]

    def pick_distinct(self, values, k):
        values = list(values)
        for i in range(k):
            j = i + self.below(len(values) - i)
            values[i], values[j] = values[j], values[i]
        return values[:k]


# ---------------- 3.2 the experimenter's search and the layout generator ----------------
def shortest_solution_length(world):
    """Breadth-first search over true (agent, object) states, actions in the order North, East,
    South, West. Returns the number of world steps of a shortest solution, or None."""
    if world.object == world.destination:
        return 0
    seen = {(world.agent, world.object)}
    queue = deque([(world.copy(), 0)])
    while queue:
        state, depth = queue.popleft()
        for action in ORDER:
            nxt, outcome = rw.apply_action(state, action)
            if outcome in ('Blocked', 'InvalidState'):
                continue
            key = (nxt.agent, nxt.object)
            if key in seen:
                continue
            seen.add(key)
            if nxt.object == nxt.destination:
                return depth + 1
            queue.append((nxt.copy(), depth + 1))
    return None


def true_dead_interior(world):
    """Interior cells (y 1..8 outer, x 1..8 inner) other than the destination that are dead in
    the true analysis with no overrides."""
    m = rp.compute_dead_squares(world, [], [], rp.Counts())
    return [(x, y) for y in range(1, 9) for x in range(1, 9)
            if m.dead[y][x] and (x, y) != world.destination]


class GeneratedLayout:
    def __init__(self):
        self.found = False
        self.world = rw.World()
        self.attempts = 0
        self.shortest = 0


def generate_layout(layout_seed):
    random = SeededRandom(layout_seed)
    result = GeneratedLayout()
    for attempt in range(1, 10001):
        world = rw.World()
        for y in range(10):
            for x in range(10):
                border = x in (0, 9) or y in (0, 9)
                world.walls[y][x] = True if border else random.below(100) < 12
        free = [(x, y) for y in range(10) for x in range(10) if not world.walls[y][x]]
        if len(free) < 3:
            continue
        world.agent, world.object, world.destination = random.pick_distinct(free, 3)
        if not rw.is_valid(world):
            continue
        shortest = shortest_solution_length(world)
        if shortest is None or shortest < 12 or shortest > 40:
            continue
        if not true_dead_interior(world):
            continue
        result.found = True
        result.world = world
        result.attempts = attempt
        result.shortest = shortest
        return result
    result.attempts = 10000
    return result


# ---------------- 3.3 the fault-event list ----------------
class FaultEvent:
    def __init__(self):
        self.id = ''
        self.layout_seed = 0
        self.run_seed = 0
        self.index = 0
        self.step = 0
        self.type = 'F1'
        self.target = 'R1'
        self.common = False
        self.has_cell = False
        self.cell = (0, 0)
        self.direction = 'free->blocked'
        self.order = list(ORDER)
        self.action = 'North'
        self.rotation = 'left'


def common_mode_count(count):
    return (2 * count + 5) // 10


def run_stream_seed(master, layout_seed, run_seed):
    return ((master << 40) + (layout_seed << 8) + run_seed) & MASK


def candidates_for(event, layout):
    dead = None
    if event.type == 'F4':
        dead = rp.compute_dead_squares(layout, [], [], rp.Counts()).dead
    out = []
    for y in range(1, 9):
        for x in range(1, 9):
            wanted = dead[y][x] if dead is not None else False
            if event.type == 'F1':
                wanted = (event.direction == 'blocked->free') == layout.walls[y][x]
            if (x, y) != layout.destination and wanted:
                out.append((x, y))
    return out


def assign_type_targets(events, fault_type, random):
    idx = [i for i, e in enumerate(events) if e.type == fault_type]
    perm = list(idx)
    random.shuffle(perm)
    common = common_mode_count(len(idx))
    for p, at in enumerate(perm):
        if p < common:
            events[at].target = 'both'
            events[at].common = True
        else:
            events[at].target = 'R1' if (p - common) % 2 == 0 else 'R2'
            events[at].common = False
    if fault_type == 'F1':
        perm2 = list(idx)
        random.shuffle(perm2)
        for p, at in enumerate(perm2):
            events[at].direction = 'free->blocked' if p % 2 == 0 else 'blocked->free'


def fill_event_detail(event, layout, random):
    if event.type in ('F1', 'F4'):
        cells = candidates_for(event, layout)
        if cells:
            event.has_cell = True
            event.cell = cells[random.below(len(cells))]
    elif event.type == 'F2':
        order = list(ORDER)
        random.shuffle(order)
        event.order = order
    elif event.type == 'F3':
        event.action = ORDER[random.below(4)]
        event.rotation = ['left', 'right'][random.below(2)]


def make_events(runs, types):
    events = []
    for i, fault_type in enumerate(types):
        layout_seed, run_seed = runs[i // 3]
        e = FaultEvent()
        e.layout_seed = layout_seed
        e.run_seed = run_seed
        e.index = i % 3 + 1
        e.type = fault_type
        e.id = '%d-%d-%d' % (layout_seed, run_seed, e.index)
        events.append(e)
    return events


def generate_fault_events(runs, layouts, master_seed):
    """runs: [(layout seed, run seed)]; layouts: the true worlds, one per run."""
    if len(runs) != len(layouts):
        return []
    experiment = SeededRandom(master_seed)
    types = [TYPES[i % 5] for i in range(3 * len(runs))]
    experiment.shuffle(types)
    events = make_events(runs, types)
    for fault_type in TYPES:
        assign_type_targets(events, fault_type, experiment)
    result = []
    for r, (layout_seed, run_seed) in enumerate(runs):
        stream = SeededRandom(run_stream_seed(master_seed, layout_seed, run_seed))
        steps = sorted(stream.pick_distinct(list(range(1, 11)), 3))
        run_events = []
        for j in range(3):
            e = events[3 * r + j]
            e.step = steps[j]
            fill_event_detail(e, layouts[r], stream)
            run_events.append(e)
        result.append(run_events)
    return result


def cell_text(cell):
    return '(%d, %d)' % cell


def detail_text(e):
    if e.type == 'F1':
        return (cell_text(e.cell) if e.has_cell else 'none') + ' ' + e.direction
    if e.type == 'F2':
        return ' '.join(e.order)
    if e.type == 'F3':
        return '%s %s' % (e.action, e.rotation)
    if e.type == 'F4':
        return cell_text(e.cell) if e.has_cell else 'none'
    return '-'


def fault_event_text(e):
    return '%s step %d %s target %s | %s' % (e.id, e.step, e.type, e.target, detail_text(e))
