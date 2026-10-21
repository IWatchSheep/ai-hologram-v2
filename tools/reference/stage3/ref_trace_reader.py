"""The bounds-safe readers of the Stage 3 equivalence gate (plan STAGE_03, 6.8): the exact
order of checks and the exact problem messages the C++ readers follow.

read_cases(lines)        -> (ok, cases, problem); cases are ref_trace.Case objects
split_traces(lines)      -> (ok, blocks, problem); blocks are (name, [lines from 'case' to 'end'])

Like the Stage 1 and 2 readers: every line is checked to exist before it is read, a number
is at most 9 digits (parse_integer), and nothing ever raises."""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'stage1'))
import ref_golden_readers as rgr  # noqa: E402
import ref_agents as ra  # noqa: E402
import ref_parts as rp  # noqa: E402
import ref_trace as rt  # noqa: E402

rw = rp.rw
ACTIONS = rp.ORDER


def split_words(text):
    """The C++ splitWords: split on single spaces (two spaces give an empty word)."""
    return text.split(' ')


def _int(text):
    """A non-negative integer of at most 9 digits, or None."""
    ok, value = rgr.parse_integer(text)
    if not ok or value < 0:
        return None
    return value


def _model_ok(words):
    if len(words) == 2 and words[1] == 'standard':
        return True
    if len(words) == 3 and words[1] in ('stuck', 'nopush', 'slip'):
        return words[2] in ACTIONS
    if len(words) == 4 and words[1] in ('swap', 'pushturn'):
        return words[2] in ACTIONS and words[3] in ACTIONS
    return False


def _notdead(words):
    """The cells of 'notdead <n> x1 y1 ... xn yn', or None."""
    if len(words) < 2 or words[0] != 'notdead':
        return None
    n = _int(words[1])
    if n is None or len(words) != 2 + 2 * n:
        return None
    cells = []
    for k in range(n):
        x = _int(words[2 + 2 * k])
        y = _int(words[3 + 2 * k])
        if x is None or y is None:
            return None
        cells.append((x, y))
    return cells


def _seed(c, words):
    """Applies one 'seed ...' line to case c; False when it is malformed."""
    if len(words) == 6 and words[1] == 'wall':
        x, y = _int(words[2]), _int(words[3])
        states = ('blocked', 'free')
        if x is None or y is None or words[4] not in states or words[5] not in states:
            return False
        c.walls.append(((x, y), words[4] == 'blocked', words[5] == 'blocked'))
        return True
    if len(words) == 4 and words[1] == 'v1' and c.v1 is None:
        k = _int(words[3])
        if words[2] not in ACTIONS or k is None:
            return False
        c.v1 = (words[2], k)
        return True
    if len(words) == 3 and words[1] == 'committed' and c.committed is None and words[2] in ACTIONS:
        c.committed = words[2]
        return True
    if len(words) == 3 and words[1] == 'plan' and c.plan is None and words[2] in ACTIONS:
        c.plan = words[2]
        return True
    return False


def _rows(lines, i, number, what):
    """Parses 10 world rows at index i; returns (world, problem)."""
    prefix = 'case %d: ' % number
    if i + 10 > len(lines):
        return None, prefix + 'missing %s lines' % what
    world, error = rw.parse_world(lines[i:i + 10])
    if world is None:
        return None, prefix + '%s does not parse: %s' % (what, error)
    return world, ''


def _read_one(lines, i, number):
    """Reads the case starting at index i; returns (case, next index, problem)."""
    prefix = 'case %d: ' % number
    words = split_words(lines[i])
    if len(words) != 2 or words[0] != 'case':
        return None, i, 'line %d: expected a case header' % (i + 1)
    c = rt.Case()
    c.name = words[1]
    i += 1
    if i >= len(lines):
        return None, i, prefix + 'missing agent line'
    words = split_words(lines[i])
    if words not in (['agent', 'C1'], ['agent', 'C0']):
        return None, i, prefix + 'malformed agent line'
    c.agent = words[1]
    i += 1
    if i >= len(lines):
        return None, i, prefix + 'missing settings line'
    words = split_words(lines[i])
    values = [_int(w) for w in words[1:]] if len(words) == 5 and words[0] == 'settings' else [None]
    if None in values:
        return None, i, prefix + 'malformed settings line'
    c.settings = ra.Settings(*values)
    i += 1
    if i >= len(lines) or lines[i] != 'world':
        return None, i, prefix + 'missing world line'
    c.world, problem = _rows(lines, i + 1, number, 'world')
    if problem:
        return None, i, problem
    i += 11
    if i >= len(lines) or lines[i] not in ('belief true', 'belief given'):
        return None, i, prefix + 'malformed belief line'
    if lines[i] == 'belief given':
        c.belief, problem = _rows(lines, i + 1, number, 'belief')
        if problem:
            return None, i, problem
        i += 10
    i += 1
    if i >= len(lines) or not _model_ok(split_words(lines[i])) or split_words(lines[i])[0] != 'model':
        return None, i, prefix + 'malformed model line'
    c.model = split_words(lines[i])[1:]
    i += 1
    cells = _notdead(split_words(lines[i])) if i < len(lines) else None
    if cells is None:
        return None, i, prefix + 'malformed notdead line'
    c.not_dead = cells
    i += 1
    while i < len(lines) and split_words(lines[i])[:1] == ['seed']:
        if not _seed(c, split_words(lines[i])):
            return None, i, prefix + 'malformed seed line'
        i += 1
    if i >= len(lines) or lines[i] != 'end':
        return None, i, prefix + 'missing end line'
    return c, i + 1, ''


def read_cases(lines):
    cases = []
    i = 0
    while i < len(lines):
        c, i, problem = _read_one(lines, i, len(cases) + 1)
        if problem:
            return False, 0, problem
        cases.append(c)
    return True, cases, ''


def split_traces(lines):
    """Splits a trace or observer file into blocks from 'case <name>' to 'end'."""
    blocks = []
    i = 0
    while i < len(lines):
        words = split_words(lines[i])
        if len(words) != 2 or words[0] != 'case':
            return False, [], 'line %d: expected a case header' % (i + 1)
        start = i
        while i < len(lines) and lines[i] != 'end':
            i += 1
        if i >= len(lines):
            return False, [], 'trace %s: missing end line' % words[1]
        blocks.append((words[1], lines[start:i + 1]))
        i += 1
    return True, blocks, ''


def first_difference(name, expected, actual):
    """'' when equal; otherwise the gate's message for the first different line:
    'case <name>, <step> line <k>: expected [...] actual [...]'."""
    step = 'start'
    for k in range(max(len(expected), len(actual))):
        e = expected[k] if k < len(expected) else '<missing>'
        a = actual[k] if k < len(actual) else '<missing>'
        if e != a:
            return 'case %s, %s, line %d: expected [%s] actual [%s]' % (name, step, k + 1, e, a)
        if e.startswith('step ') or e == 'final':
            step = e
    return ''


def gate(cases_lines, golden_lines, observers=False):
    """The equivalence gate on the reference: '' when every case matches."""
    ok, cases, problem = read_cases(cases_lines)
    if not ok:
        return problem
    ok, blocks, problem = split_traces(golden_lines)
    if not ok:
        return problem
    if len(blocks) != len(cases):
        return 'trace file has %d cases, the case file %d' % (len(blocks), len(cases))
    for c, (name, expected) in zip(cases, blocks):
        if name != c.name:
            return 'trace %s: expected case %s' % (name, c.name)
        difference = first_difference(c.name, expected, rt.trace(c, observers))
        if difference:
            return difference
    return ''
