"""The bounds-safe reader of the Stage 4 fault-trace cases and the gate, part 2
(plan STAGE_04, 3.15): the exact order of checks and the exact problem messages.

read_fault_cases(lines) -> (ok, cases, problem); cases are ref_fault_trace.FaultCase objects
gate(case_lines, golden_lines, required) -> '' or the first failure

A fault case is a Stage 3 case block (read by the Stage 3 reader, unchanged) whose last lines,
just before 'end', are one 'run <layoutSeed> <runSeed>' line and then any number of
'event ...' lines. Every line is checked to exist before it is read, numbers are at most 9
digits, and nothing ever raises."""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'stage3'))
import ref_fault_trace as rft  # noqa: E402
import ref_material as rmat  # noqa: E402
import ref_trace_reader as rtr  # noqa: E402

ACTIONS = ['North', 'East', 'South', 'West']
TYPES = ['F1', 'F2', 'F3', 'F4', 'F5']
TARGETS = ['R1', 'R2', 'both']
DIRECTIONS = ['free->blocked', 'blocked->free']
ROTATIONS = ['left', 'right']
GOLDEN_FAULT_CASES = 563


def _cell(e, words, at):
    """Reads 'cell X Y' or 'cell none' at index at; returns the next index, or -1."""
    if at >= len(words) or words[at] != 'cell' or at + 1 >= len(words):
        return -1
    if words[at + 1] == 'none':
        return at + 2
    if at + 2 >= len(words):
        return -1
    x = rtr._int(words[at + 1])
    y = rtr._int(words[at + 2])
    if x is None or y is None:
        return -1
    e.has_cell = True
    e.cell = (x, y)
    return at + 3


def _details(e, words):
    """Reads the type's details from index 9; True when they fill the line exactly."""
    t = e.type
    if t == 'F1':
        at = _cell(e, words, 9)
        if at < 0 or at + 1 != len(words) or words[at] not in DIRECTIONS:
            return False
        e.direction = words[at]
        return True
    if t == 'F2':
        if len(words) != 13 or any(w not in ACTIONS for w in words[9:13]):
            return False
        e.order = words[9:13]
        return True
    if t == 'F3':
        if len(words) != 11 or words[9] not in ACTIONS or words[10] not in ROTATIONS:
            return False
        e.action = words[9]
        e.rotation = words[10]
        return True
    if t == 'F4':
        return _cell(e, words, 9) == len(words)
    return len(words) == 10 and words[9] == '-'


def parse_event_line(text):
    """The event of one 'event ...' line, or None when it is malformed."""
    words = rtr.split_words(text)
    if len(words) < 10 or words[0] != 'event' or words[1] == '':
        return None
    e = rmat.FaultEvent()
    e.id = words[1]
    numbers = [rtr._int(w) for w in words[2:6]]
    if None in numbers:
        return None
    e.layout_seed, e.run_seed, e.index, e.step = numbers
    if words[6] not in TYPES or words[7] not in TARGETS or words[8] not in ('0', '1'):
        return None
    e.type = words[6]
    e.target = words[7]
    e.common = words[8] == '1'
    if not _details(e, words):
        return None
    return e


def _first_word(text):
    return rtr.split_words(text)[0]


def read_fault_cases(lines):
    """1. The Stage 3 reader reads the file without its run and event lines (its problems are
    reported as it reports them, line numbers counted in that shorter file).
    2. Every case block starts with its header (no run or event line between blocks or after the
    last one), and its lines from the first run or event line up to 'end' must be one run line
    and then only event lines."""
    stripped = [line for line in lines if _first_word(line) not in ('run', 'event')]
    ok, bases, problem = rtr.read_cases(stripped)
    if not ok:
        return False, [], problem
    cases = []
    i = 0
    for number, base in enumerate(bases, 1):
        prefix = 'case %d: ' % number
        if _first_word(lines[i]) != 'case':
            return False, [], 'line %d: expected a case header' % (i + 1)
        j = i
        while lines[j] != 'end':
            j += 1
        k = i
        while k < j and _first_word(lines[k]) not in ('run', 'event'):
            k += 1
        if k >= j:
            return False, [], prefix + 'missing run line'
        words = rtr.split_words(lines[k])
        seeds = [rtr._int(w) for w in words[1:]] if len(words) == 3 and words[0] == 'run' else [None]
        if None in seeds:
            return False, [], prefix + 'malformed run line'
        fc = rft.FaultCase()
        fc.base = base
        fc.layout_seed, fc.run_seed = seeds
        for line in lines[k + 1:j]:
            event = parse_event_line(line)
            if event is None:
                return False, [], prefix + 'malformed event line'
            fc.events.append(event)
        cases.append(fc)
        i = j + 1
    if i < len(lines):
        return False, [], 'line %d: expected a case header' % (i + 1)
    return True, cases, ''


def gate(case_lines, golden_lines, required):
    """The fault gate on the reference: '' when every case matches (the order of plan 3.15)."""
    if not case_lines or not golden_lines:
        return 'a golden file is missing or empty'
    ok, cases, problem = read_fault_cases(case_lines)
    if not ok:
        return problem
    ok, blocks, problem = rtr.split_traces(golden_lines)
    if not ok:
        return problem
    if len(cases) != required:
        return 'case file has %d cases, required %d' % (len(cases), required)
    if len(blocks) != len(cases):
        return 'trace file has %d cases, the case file %d' % (len(blocks), len(cases))
    for fc, (name, expected) in zip(cases, blocks):
        if name != fc.base.name:
            return 'trace %s: expected case %s' % (name, fc.base.name)
        difference = rtr.first_difference(fc.base.name, expected, rft.trace(fc))
        if difference:
            return difference
    return ''
