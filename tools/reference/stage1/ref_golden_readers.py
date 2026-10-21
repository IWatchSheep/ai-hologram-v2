"""The golden-file readers (plan STAGE_01 v1.2, Section 6.7), mirrored on the reference.

Each reader takes the file's lines (already split, '\\r' removed) and returns
(ok, cases, problem). `problem` is '' when ok, otherwise exactly one of the messages
below (line numbers count from 1). The C++ readers in tests/golden_*_reader.cpp follow
this data flow and these messages exactly."""
import re

import ref_world as rw

INTEGER = re.compile(r'^-?[0-9]+$')


def parse_integer(text):
    """The one integer test (plan R4): optional '-', then digits only."""
    if INTEGER.match(text):
        return True, int(text)
    return False, 0


def format_parse_result(rows):
    w, error = rw.parse_world(rows)
    if w is None:
        return 'ERROR ' + error
    text = (f'OK {w.agent[0]} {w.agent[1]} {w.object[0]} {w.object[1]} '
            f'{w.destination[0]} {w.destination[1]}')
    if rw.render_world(w) != rows:
        text += ' ROUNDTRIP-DIFFERS'
    return text


def read_case_header(line):
    """'case <n> lines <k>' with integers, k >= 0 -> (True, n, k)."""
    parts = line.split(' ')
    if len(parts) != 4 or parts[0] != 'case' or parts[2] != 'lines':
        return False, 0, 0
    ok_n, n = parse_integer(parts[1])
    ok_k, k = parse_integer(parts[3])
    if not ok_n or not ok_k or k < 0:
        return False, 0, 0
    return True, n, k


def check_parse_golden_lines(lines):
    cases = 0
    i = 0
    while i < len(lines):
        ok, n, k = read_case_header(lines[i])
        if not ok:
            return False, cases, f'line {i + 1}: invalid case header'
        i += 1
        if i + k > len(lines):
            return False, cases, f'case {n}: missing input lines'
        rows = lines[i:i + k]
        i += k
        if i >= len(lines):
            return False, cases, f'case {n}: missing expect line'
        if not lines[i].startswith('expect '):
            return False, cases, f'case {n}: malformed expect line'
        expected = lines[i][len('expect '):]
        i += 1
        actual = format_parse_result(rows)
        cases += 1
        if actual != expected:
            return False, cases, f'case {n}: expected [{expected}] actual [{actual}]'
    return True, cases, ''


def read_case_left(left):
    """'<id> <ax> <ay> <ox> <oy> <Action>' -> (True, id, agent, object, action)."""
    parts = left.split(' ')
    if len(parts) != 6 or parts[5] not in rw.ACTIONS:
        return False, '', (0, 0), (0, 0), ''
    numbers = []
    for text in parts[1:5]:
        ok, value = parse_integer(text)
        if not ok:
            return False, '', (0, 0), (0, 0), ''
        numbers.append(value)
    return True, parts[0], (numbers[0], numbers[1]), (numbers[2], numbers[3]), parts[5]


def format_rule_case(world, left, agent, obj, action):
    w = world.copy()
    w.agent, w.object = agent, obj
    new, outcome = rw.apply_action(w, action)
    goal = 1 if rw.is_goal(new) else 0
    return (f'{left} -> {outcome} {new.agent[0]} {new.agent[1]} '
            f'{new.object[0]} {new.object[1]} {goal}')


def check_rules_golden_lines(lines):
    layouts = {}
    cases = 0
    i = 0
    while i < len(lines):
        line = lines[i]
        if line.startswith('layout '):
            lid = line[len('layout '):]
            if i + 1 + rw.GRID > len(lines):
                return False, cases, f'line {i + 1}: truncated layout {lid}'
            w, error = rw.parse_world(lines[i + 1:i + 1 + rw.GRID])
            if w is None:
                return False, cases, f'line {i + 1}: layout {lid} does not parse: {error}'
            layouts[lid] = w
            i += 1 + rw.GRID
            continue
        if ' -> ' not in line:
            return False, cases, f'line {i + 1}: malformed case line'
        left = line.split(' -> ', 1)[0]
        ok, lid, agent, obj, action = read_case_left(left)
        if not ok:
            return False, cases, f'line {i + 1}: malformed case line'
        if lid not in layouts:
            return False, cases, f'line {i + 1}: unknown layout {lid}'
        actual = format_rule_case(layouts[lid], left, agent, obj, action)
        cases += 1
        if actual != line:
            return False, cases, f'line {i + 1}: expected [{line}] actual [{actual}]'
        i += 1
    return True, cases, ''
